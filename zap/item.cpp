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

#include "item.h"
#include "ship.h"
#include "goalZone.h"
#include "glutInclude.h"

namespace Zap
{

Item::Item(Point p, bool collideable, float radius, float mass) : MoveObject(p, radius, mass)
{
   mIsMounted = false;
   mIsCollideable = collideable;
   mObjectTypeMask = MoveableType | ItemType | CommandMapVisType;
}

void Item::processArguments(S32 argc, const char **argv)
{
   if(argc < 2)
      return;
   Point pos;
   pos.read(argv);
   pos *= getGame()->getGridSize();
   for(U32 i = 0; i < MoveStateCount; i++)
      mMoveState[i].pos = pos;
   updateExtent();
}

void Item::render()
{
   // if the item is mounted, renderItem will be called from the
   // ship it is mounted to
   if(mIsMounted)
      return;

   renderItem(mMoveState[RenderState].pos);
}

void Item::mountToShip(Ship *theShip)
{
   TNLAssert(isGhost() || isInDatabase(), "Error, mount item not in database.");
   dismount();
   mMount = theShip;
   if(theShip)
      theShip->mMountedItems.push_back(this);

   mIsMounted = true;
   setMaskBits(MountMask);
}

void Item::onMountDestroyed()
{
   dismount();
}

void Item::dismount()
{
   if(mMount.isValid())
   {
      for(S32 i = 0; i < mMount->mMountedItems.size(); i++)
      {
         if(mMount->mMountedItems[i].getPointer() == this)
         {
            mMount->mMountedItems.erase(i);
            break;
         }
      }
   }
   mMount = NULL;
   mIsMounted = false;
   setMaskBits(MountMask);
}

void Item::setActualPos(Point p)
{
   mMoveState[ActualState].pos = p;
   mMoveState[ActualState].vel.set(0,0);
   setMaskBits(WarpPositionMask | PositionMask);
}

void Item::setActualVel(Point vel)
{
   mMoveState[ActualState].vel = vel;
   setMaskBits(WarpPositionMask | PositionMask);
}

Ship *Item::getMount()
{
   return mMount;
}

void Item::setZone(GoalZone *theZone)
{
   mZone = theZone;
   setMaskBits(ZoneMask);
}

GoalZone *Item::getZone()
{
   return mZone;
}

void Item::idle(GameObject::IdleCallPath path)
{
   if(!isInDatabase())
      return;

   if(mIsMounted)
   {
      if(mMount.isNull() || mMount->hasExploded)
      {
         if(!isGhost())
            dismount();
      }
      else
      {
         mMoveState[RenderState].pos = mMount->getRenderPos();
         mMoveState[ActualState].pos = mMount->getActualPos();
      }
   }
   else
   {
      float time = mCurrentMove.time * 0.001f;
      move(time, ActualState, false);
      if(path == GameObject::ServerIdleMainLoop)
      {
         // Only update if it's actually moving...
         if(mMoveState[ActualState].vel.len() > 0.001f)
            setMaskBits(PositionMask);

         mMoveState[RenderState] = mMoveState[ActualState];
         
      }
      else
         updateInterpolation();
   }
   updateExtent();
}

U32 Item::packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream)
{
   U32 retMask = 0;
   if(stream->writeFlag(updateMask & InitialMask))
   {
   }
   if(stream->writeFlag(updateMask & PositionMask))
   {
      ((GameConnection *) connection)->writeCompressedPoint(mMoveState[ActualState].pos, stream);
      writeCompressedVelocity(mMoveState[ActualState].vel, 511, stream);
      stream->writeFlag(updateMask & WarpPositionMask);
   }
   if(stream->writeFlag(updateMask & MountMask) && stream->writeFlag(mIsMounted))
   {
      S32 index = connection->getGhostIndex(mMount);
      if(stream->writeFlag(index != -1))
         stream->writeInt(index, GhostConnection::GhostIdBitSize);
      else
         retMask |= MountMask;
   }
   if(stream->writeFlag(updateMask & ZoneMask))
   {
      if(mZone.isValid())
      {
         S32 index = connection->getGhostIndex(mZone);
         if(stream->writeFlag(index != -1))
            stream->writeInt(index, GhostConnection::GhostIdBitSize);
         else
            retMask |= ZoneMask;
      }
      else
         stream->writeFlag(false);
   }
   return retMask;
}

void Item::unpackUpdate(GhostConnection *connection, BitStream *stream)
{
   bool interpolate = false;
   bool positionChanged = false;

   if(stream->readFlag())
   {
   }
   if(stream->readFlag())
   {
      ((GameConnection *) connection)->readCompressedPoint(mMoveState[ActualState].pos, stream);
      readCompressedVelocity(mMoveState[ActualState].vel, 511, stream);
      positionChanged = true;
      interpolate = !stream->readFlag();
   }
   if(stream->readFlag())
   {
      bool shouldMount = stream->readFlag();
      if(shouldMount)
      {
         Ship *theShip = NULL;
         if(stream->readFlag())
            theShip = (Ship *) connection->resolveGhost(stream->readInt(GhostConnection::GhostIdBitSize));
         mountToShip(theShip);
      }
      else
         dismount();
   }
   if(stream->readFlag())
   {
      bool hasZone = stream->readFlag();
      if(hasZone)
         mZone = (GoalZone *) connection->resolveGhost(stream->readInt(GhostConnection::GhostIdBitSize));
      else
         mZone = NULL;
   }
   
   if(positionChanged)
   {
      if(interpolate)
      {
         mInterpolating = true;
         move(connection->getOneWayTime() * 0.001f, ActualState, false);
      }
      else
      {
         mInterpolating = false;
         mMoveState[RenderState] = mMoveState[ActualState];
      }
   }
}

bool Item::collide(GameObject *otherObject)
{
   return mIsCollideable && !mIsMounted;
}

PickupItem::PickupItem(Point p, float radius) 
   : Item(p, false, radius, 1)
{
   mIsVisible = true;
}

void PickupItem::idle(GameObject::IdleCallPath path)
{
   if(!mIsVisible && path == GameObject::ServerIdleMainLoop)
   {
      if(mRepopTimer.update(mCurrentMove.time))
      {
         setMaskBits(PickupMask);
         mIsVisible = true;
      }
   }
   updateExtent();
}

U32 PickupItem::packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(connection, updateMask, stream);
   stream->writeFlag(updateMask & InitialMask);
   stream->writeFlag(mIsVisible);

   return retMask;
}

void PickupItem::unpackUpdate(GhostConnection *connection, BitStream *stream)
{
   Parent::unpackUpdate(connection, stream);
   bool isInitial = stream->readFlag();
   bool visible = stream->readFlag();

   if(!isInitial && !visible && mIsVisible)
      onClientPickup();
   mIsVisible = visible;
}


bool PickupItem::collide(GameObject *otherObject)
{
   if(mIsVisible && !isGhost() && otherObject->getObjectTypeMask() & ShipType)
   {  
      if(pickup((Ship *) otherObject))
      {
         setMaskBits(PickupMask);
         mRepopTimer.reset(getRepopDelay());
         mIsVisible = false;
      }
   }
   return false;
}


};

