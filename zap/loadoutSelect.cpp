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

#include "loadoutSelect.h"
#include "UI.h"
#include "gameConnection.h"
#include "game.h"
#include "glutInclude.h"
#include "shipItems.h"
#include <ctype.h>

namespace Zap
{

struct LoadoutItem
{
   U32 key;
   U32 button;
   U32 index;
   const char *text;
};

LoadoutItem gLoadoutModules[] = {
   { 'W', 0, ModuleBoost, "Turbo Boost" },
   { 'G', 1, ModuleShield, "Shield Generator" },
   { 'R', 2, ModuleRepair, "Repair Module", },
   { 'S', 3, ModuleSensor, "Enhanced Sensor" },
   { 'C', 4, ModuleCloak, "Cloak Field Modulator" },
   { 'E', 5, ModuleEngineer, "Engineering Bay" },
   { 0, 0, NULL },
};

LoadoutItem gLoadoutWeapons[] = {
   { 'W', 0, WeaponPhaser, "Phaser" },
   { 'G', 1, WeaponBounce, "Bouncer" },
   { 'R', 2, WeaponTriple, "Three-Way" },
   { 'S', 3, WeaponBurst, "Burster" },
   { 'E', 4, WeaponMineLayer, "Mine Dropper" },
   { 0, 0, NULL },
};

const char *gLoadoutTitles[] = {
   "Choose Primary Module:",
   "Choose Secondary Module:",
   "Choose Primary Weapon:",
   "Choose Secondary Weapon:",
   "Choose Tertiary Weapon:",
};

LoadoutHelper::LoadoutHelper()
{
   mVisible = false;
}

void LoadoutHelper::show(bool fromController)
{
   mVisible = true;
   mFromController = fromController;
   mCurrentIndex = 0;
}

void LoadoutHelper::render()
{
   S32 curPos = 300;
   const int fontSize = 15;

   glColor3f(0, 1, 0);

   UserInterface::drawStringf(20, curPos, fontSize, "%s", gLoadoutTitles[mCurrentIndex]); 
   curPos += fontSize + 4;

   LoadoutItem *list;

   if(mCurrentIndex < 2)
      list = gLoadoutModules;
   else
      list = gLoadoutWeapons;

   for(S32 i = 0; list[i].text; i++)
   {
      if((mCurrentIndex == 1 && mModule[0] == i) ||
         (mCurrentIndex == 3 && mWeapon[0] == i) ||
         (mCurrentIndex == 4 && (mWeapon[0] == i || mWeapon[1] == i)))
         glColor3f(0.3, 0.3, 0.3);
      else
         glColor3f(0.3, 1.0, 0.3);

      UserInterface::drawStringf(20, curPos, fontSize, "%c - %s", 
         (mFromController ? '1' + list[i].button : 
            list[i].key), list[i].text);
      curPos += fontSize + 4;
   }

}

void LoadoutHelper::processKey(U32 key)
{
   if(key == 27)
   {
      mVisible = false;
      return;
   }
   if(!mFromController)
      key = toupper(key);
   U32 index;
   LoadoutItem *list;
   if(mCurrentIndex < 2)
      list = gLoadoutModules;
   else
      list = gLoadoutWeapons;

   for(index = 0; gLoadoutModules[index].text; index++)
   {
      if(key == gLoadoutModules[index].key ||
         key == gLoadoutModules[index].button)
      {
         break;
      }
   }
   if(!gLoadoutModules[index].text)
      return;

   switch(mCurrentIndex)
   {
      case 0:
         mModule[0] = index;
         mCurrentIndex++;
         break;
      case 1:
         if(mModule[0] != index)
         {
            mModule[1] = index;
            mCurrentIndex++;
         }
         break;
      case 2:
         mWeapon[0] = index;
         mCurrentIndex++;
         break;
      case 3:
         if(mWeapon[0] != index)
         {
            mWeapon[1] = index;
            mCurrentIndex++;
         }
         break;
      case 4:
         if(mWeapon[1] != index && mWeapon[0] != index)
         {
            mWeapon[2] = index;
            mCurrentIndex++;
         }
         break;
   }
   if(mCurrentIndex == 5)
   {
      // do the processing thang
      Vector<U32> loadout;
      loadout.push_back(gLoadoutModules[mModule[0]].index);
      loadout.push_back(gLoadoutModules[mModule[1]].index);
      loadout.push_back(gLoadoutWeapons[mWeapon[0]].index);
      loadout.push_back(gLoadoutWeapons[mWeapon[1]].index);
      loadout.push_back(gLoadoutWeapons[mWeapon[2]].index);
      GameConnection *gc = gClientGame->getConnectionToServer();
      if(gc)
         gc->c2sRequestLoadout(loadout);
      mVisible = false;
   }
}

};