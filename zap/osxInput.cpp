//-----------------------------------------------------------------------------------
//
//   Torque Network Library - ZAP example multiplayer vector graphics space game
//   Copyright (C) 2004 GarageGames.com, Inc.
//   For more information see http://www.opentnl.org
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   For use in products that are not compatible with the terms of the GNU 
//   General Public License, alternative licensing options are available 
//   from GarageGames.com.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/errno.h>
#include <sysexits.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#ifdef MACOS_10_0_4
#include <IOKit/hidsystem/IOHIDUsageTables.h>
#else
/* The header was moved here in MacOS X 10.1 */
#include <Kernel/IOKit/hidsystem/IOHIDUsageTables.h>
#endif
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <CoreFoundation/CoreFoundation.h>
#include <carbon/carbon.h>

namespace Zap
{

bool gJoystickInit = false;

void getModifierState(bool &shiftDown, bool &controlDown, bool &altDown)
{
	UInt32 modKeys = GetCurrentEventKeyModifiers();
	shiftDown = (modKeys & shiftKey) != 0;
	controlDown = (modKeys & controlKey) != 0;
	altDown = (modKeys & optionKey) != 0;
}

struct ControllerElement
{
   IOHIDElementCookie cookie;
   long minValue;
   long maxValue;
};

struct Controller
{
   IOHIDDeviceInterface **device;
   Vector<ControllerElement> axes;
   Vector<ControllerElement> buttons;  
};

Controller gController;

static S32 getElementValue(ControllerElement *theElement)
{
   IOHIDEventStruct hidEvent;
   hidEvent.value = 0;

   IOReturn result = (*(gController.device))->getElementValue(gController.device, theElement->cookie, &hidEvent);
   if(result == kIOReturnSuccess)
   {
      if(hidEvent.value < theElement->minValue)
         theElement->minValue = hidEvent.value;
      if(hidEvent.value > theElement->maxValue)
         theElement->maxValue = hidEvent.value;
   }
   return hidEvent.value;
}

static void HIDGetElementsCFArrayHandler (const void * value, void * parameter)
{
	if (CFGetTypeID (value) != CFDictionaryGetTypeID ())
      return;

   CFTypeRef refElement = (CFTypeRef(value);
	long elementType, usagePage, usage;
	CFTypeRef refElementType = CFDictionaryGetValue (refElement, CFSTR(kIOHIDElementTypeKey));
	CFTypeRef refUsagePage = CFDictionaryGetValue (refElement, CFSTR(kIOHIDElementUsagePageKey));
	CFTypeRef refUsage = CFDictionaryGetValue (refElement, CFSTR(kIOHIDElementUsageKey));
   bool isButton = false, isAxis = false;

	if ((refElementType) && (CFNumberGetValue (refElementType, kCFNumberLongType, &elementType)))
	{
		/* look at types of interest */
		if ((elementType == kIOHIDElementTypeInput_Misc) || (elementType == kIOHIDElementTypeInput_Button) ||
			(elementType == kIOHIDElementTypeInput_Axis))
		{
			if (refUsagePage && CFNumberGetValue (refUsagePage, kCFNumberLongType, &usagePage) &&
				refUsage && CFNumberGetValue (refUsage, kCFNumberLongType, &usage))
			{
				switch (usagePage) /* only interested in kHIDPage_GenericDesktop and kHIDPage_Button */
				{
					case kHIDPage_GenericDesktop:
						{
							switch (usage) /* look at usage to determine function */
							{
								case kHIDUsage_GD_X:
								case kHIDUsage_GD_Y:
								case kHIDUsage_GD_Z:
								case kHIDUsage_GD_Rx:
								case kHIDUsage_GD_Ry:
								case kHIDUsage_GD_Rz:
								case kHIDUsage_GD_Slider:
								case kHIDUsage_GD_Dial:
								case kHIDUsage_GD_Wheel:
                           isAxis = true;
   								break;
							}							
						}
						break;
					case kHIDPage_Button:
                  isButton = true;
						break;
					default:
						break;
				}
			}
		}
		else if (kIOHIDElementTypeCollection == elementType)
      {
         CFTypeID type = CFGetTypeID (refElement);
	      if (type == CFArrayGetTypeID()) /* if element is an array */
	      {
		      CFRange range = {0, CFArrayGetCount (refElement)};
		      /* CountElementsCFArrayHandler called for each array member */
		      CFArrayApplyFunction (refElement, range, HIDGetElementsCFArrayHandler, NULL);
	      }
      }
	}
   ControllerElement *theElement = NULL;
   if(isButton)
   {
      gController.buttons.increment();
      theElement = &gController.buttons.last();
   }
   else if(isAxis)
   {
      gController.axes.increment();
      theElement = &gController.axes.last();
   }

	if (theElement) /* add to list */
	{
	   long number;
	   CFTypeRef refType;

	   refType = CFDictionaryGetValue (refElement, CFSTR(kIOHIDElementCookieKey));
	   if (refType && CFNumberGetValue (refType, kCFNumberLongType, &number))
		   theElement->cookie = (IOHIDElementCookie) number;
	   refType = CFDictionaryGetValue (refElement, CFSTR(kIOHIDElementMinKey));
	   if (refType && CFNumberGetValue (refType, kCFNumberLongType, &number))
		   theElement->minValue = number;

      refType = CFDictionaryGetValue (refElement, CFSTR(kIOHIDElementMaxKey));

	   if (refType && CFNumberGetValue (refType, kCFNumberLongType, &number))
		   theElement->maxValue = number;
	}
}


void InitJoystick()
{
   mach_port_t masterPort = NULL;
	io_iterator_t hidObjectIterator = NULL;

	IOReturn result = IOMasterPort (bootstrap_port, &masterPort);
   if(result != kIOReturnSuccess)
      return;
   
   CFMutableDictionaryRef hidMatchDictionary = IOServiceMatching (kIOHIDDeviceKey);
   if(!hidMatchDictionary)
      return;
	result = IOServiceGetMatchingServices (masterPort, hidMatchDictionary, &hidObjectIterator);
   if(result != kIOReturnSuccess)
      return;

   // find the first joystick/gamepad on the USB
   for(;;)
   {
      IOHIDDeviceInterface **device;
      io_object_t ioHIDDeviceObject = IOIteratorNext(hidObjectIterator);
      if(!ioHIDDeviceObject)
         break;

      CFMutableDictionaryRef hidProperties = 0;
      kern_result_t result = IORegistryEntryCreateCFProperties(ioHIDDeviceObject, &hidProperties, kCFAllocatorDefault, kNilOptions);
      if(result == KERN_SUCCESS && hidProperties)
      {
			CFTypeRef refCF = 0;
			refCF = CFDictionaryGetValue (hidProperties, CFSTR(kIOHIDPrimaryUsagePageKey));
         long usage, usagePage;
			CFNumberGetValue (refCF, kCFNumberLongType, &usagePage);
         refCF = CFDictionaryGetValue (hidProperties, CFSTR(kIOHIDPrimaryUsageKey));
			CFNumberGetValue (refCF, kCFNumberLongType, &usage);

   		if ( (device->usagePage == kHIDPage_GenericDesktop) &&
		     ((device->usage == kHIDUsage_GD_Joystick ||
		      device->usage == kHIDUsage_GD_GamePad)) )
         {
	         CFTypeRef refElementTop = CFDictionaryGetValue (hidProperties, CFSTR(kIOHIDElementKey));
	         if (refElementTop)
            {
	            CFTypeID type = CFGetTypeID (refElementTop);
	            if (type == CFArrayGetTypeID()) /* if element is an array */
	            {
		            CFRange range = {0, CFArrayGetCount (refElementTop)};
		            /* CountElementsCFArrayHandler called for each array member */
		            CFArrayApplyFunction (refElementTop, range, HIDGetElementsCFArrayHandler, NULL);

                  IOCFPlugInInterface ** ppPlugInInterface = NULL;
	
		            IOReturn result = IOCreatePlugInInterfaceForService (hidDevice, kIOHIDDeviceUserClientTypeID,
													            kIOCFPlugInInterfaceID, &ppPlugInInterface, &score);
		            if (result == kIOReturnSuccess)
		            {
			            // Call a method of the intermediate plug-in to create the device interface
			            plugInResult = (*ppPlugInInterface)->QueryInterface (ppPlugInInterface,
								            CFUUIDGetUUIDBytes (kIOHIDDeviceInterfaceID), (void *) &device);
                     if(device)
                     {
                        result = (*device)->open(device, 0);
                        gController.device = device;
                        gJoystickInit = true;
                     }
                     (*ppPlugInInterface)->Release (ppPlugInInterface);
                  }
               }
            }
         }
         CFRelease(hidProperties)
      }
      IOObjectRelese(ioHIDDeviceObject);
      if(gJoystickInit)
         break;
   }
	IOObjectRelease (hidObjectIterator); /* release the iterator */
}

bool ReadJoystick(F32 axes[MaxJoystickAxes], U32 &buttonMask)
{
   if(!gJoystickInit)
      return false;

   S32 i;
   for(i = 0; i < gController.axes.size(); i++)
   {
      if(i >= MaxJoystickAxes)
         break;
      ControllerElement *e = &gController.axes[i];
      S32 elementValue = getElementValue(e);
      S32 diff = e->maxValue - e->minValue;
      if(diff != 0)
      {
         axes[i] = (elementValue - e->minValue) * 2 / diff;
         axes[i] -= 1;
      }
      else
         axes[i] = 0;
   }
   for(; i < MaxJoystickAxes; i++)
      axes[i] = 0;

   buttonMask = 0;
   for(i = 0; i < gController.buttons.size(); i++)
   {
      ControllerElement *e = &gController.axes[i];
      S32 value = getElementValue(e);
      if(value)
         buttonMask |= (1 << i);
   }
}

void ShutdownJoystick()
{
   if(gController.device)
      (*(gController.device))->close(gController.device);
}

};