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

#include "gameObject.h"
#include "glutInclude.h"

using namespace TNL;

namespace Zap
{

GameObject::GameObject()
{
   mGame = NULL;
   mLastQueryId = 0;
   mObjectTypeMask = UnknownType;
   mDisableCollisionCount = 0;
   mInDatabase = false;
}

void GameObject::setControllingClient(GameConnection *c)
{
   mControllingClient = c;
}

void GameObject::deleteObject(U32 deleteTimeInterval)
{
   mObjectTypeMask = DeletedType;
   if(!mGame)
      delete this;
   else
      mGame->addToDeleteList(this, deleteTimeInterval);
}

Point GameObject::getRenderPos()
{
   return Point();
}

Point GameObject::getActualPos()
{
   return Point();
}

void GameObject::setActualPos(Point p)
{
}

F32 GameObject::getUpdatePriority(NetObject *scopeObject, U32 updateMask, S32 updateSkips)
{
   GameObject *so = (GameObject *) scopeObject;

   Point center = so->extent.getCenter();  

   Point nearest;
   if(center.x < extent.min.x)
      nearest.x = extent.min.x;
   else if(center.x > extent.max.x)
      nearest.x = extent.max.x;
   else
      nearest.x = center.x;

   if(center.y < extent.min.y)
      nearest.y = extent.min.y;
   else if(center.y > extent.max.y)
      nearest.y = extent.max.y;
   else
      nearest.y = center.y;

   Point deltap = nearest - center;

   F32 distance = (nearest - center).len();

   Point deltav = getActualVel() - so->getActualVel();

   F32 add = 0;

   // initial scoping factor is distance based.
   F32 distFactor = (500 - distance) / 500;

   // give some extra love to things that are moving towards the scope object
   if(deltav.dot(deltap) < 0)
      add = 0.7;

   // and a little more love if this object has not yet been scoped.
   if(updateMask == 0xFFFFFFFF)
      add += 0.5;
   return distFactor + add + updateSkips * 0.5f;
}

void GameObject::damageObject(DamageInfo *theInfo)
{

}

GameConnection *GameObject::getControllingClient()
{
   return mControllingClient;
}

void GameObject::setExtent(Rect &extents)
{
   if(mGame && mInDatabase)
   {
      // remove from the extents database for current extents
      mGame->getGridDatabase()->removeFromExtents(this, extent);
      // and readd for the new extent
      mGame->getGridDatabase()->addToExtents(this, extents);
   }
   extent = extents;
}

void GameObject::findObjects(U32 typeMask, Vector<GameObject *> &fillVector, Rect &ext)
{
   if(!mGame)
      return;
   mGame->getGridDatabase()->findObjects(typeMask, fillVector, ext);
}

GameObject *GameObject::findObjectLOS(U32 typeMask, U32 stateIndex, Point rayStart, Point rayEnd, float &collisionTime, Point &collisionNormal)
{
   if(!mGame)
      return NULL;
   return mGame->getGridDatabase()->findObjectLOS(typeMask, stateIndex, rayStart, rayEnd, collisionTime, collisionNormal);
}

void GameObject::addToDatabase()
{
   if(!mInDatabase)
   {
      mInDatabase = true;
      mGame->getGridDatabase()->addToExtents(this, extent);
   }
}

void GameObject::removeFromDatabase()
{
   if(mInDatabase)
   {
      mInDatabase = false;
      mGame->getGridDatabase()->removeFromExtents(this, extent);
   }
}

void GameObject::addToGame(Game *theGame)
{
   TNLAssert(mGame == NULL, "Error, already in a game.");
   theGame->addToGameObjectList(this);
   mGame = theGame;
   addToDatabase();
   onAddedToGame(theGame);
}

void GameObject::onAddedToGame(Game *)
{
}

void GameObject::removeFromGame()
{
   if(mGame)
   {
      removeFromDatabase();
      mGame->removeFromGameObjectList(this);
      mGame = NULL;
   }
}

bool GameObject::getCollisionPoly(Vector<Point> &polyPoints)
{
   return false;
}

bool GameObject::getCollisionCircle(U32 stateIndex, Point &point, float &radius)
{
   return false;
}

void GameObject::render()
{
}

void GameObject::render(U32 layerIndex)
{
   if(layerIndex == 0)
      render();
}

void GameObject::idle(IdleCallPath path)
{
}

void GameObject::writeControlState(BitStream *)
{
}

void GameObject::readControlState(BitStream *)
{
}

void GameObject::controlMoveReplayComplete()
{
}

void GameObject::writeCompressedVelocity(Point &vel, U32 max, BitStream *stream)
{
   U32 len = U32(vel.len());
   if(stream->writeFlag(len == 0))
      return;

   if(stream->writeFlag(len > max))
   {
      stream->write(vel.x);
      stream->write(vel.y);
   }
   else
   {
      F32 theta = atan2(vel.y, vel.x);
      stream->writeFloat(theta * FloatInverse2Pi, 10);
      stream->writeRangedU32(len, 0, max);
   }
}

void GameObject::readCompressedVelocity(Point &vel, U32 max, BitStream *stream)
{
   if(stream->readFlag())
   {
      vel.x = vel.y = 0;
      return;
   }
   else if(stream->readFlag())
   {
      stream->read(&vel.x);
      stream->read(&vel.y);
   }
   else
   {
      F32 theta = stream->readFloat(10) * Float2Pi;
      F32 magnitude = stream->readRangedU32(0, max);
      vel.set(cos(theta) * magnitude, sin(theta) * magnitude);
   }
}

void GameObject::processArguments(S32 argc, const char**argv)
{
}

bool GameObject::onGhostAdd(GhostConnection *theConnection)
{
   addToGame(gClientGame);
   return true;
}

void drawCircle(Point pos, F32 radius)
{
   glBegin(GL_LINE_LOOP);

   for(F32 theta = 0; theta < 2 * 3.1415; theta += 0.2)
      glVertex2f(pos.x + cos(theta) * radius, pos.y + sin(theta) * radius);

   glEnd();
}

};
