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

#include "projectile.h"
#include "ship.h"
#include "sparkManager.h"
#include "sfx.h"

#include "glutInclude.h"

namespace Zap
{

TNL_IMPLEMENT_NETOBJECT(Projectile);


Projectile::Projectile(Point p, Point v, U32 t, Ship *shooter)
{
   mNetFlags.set(Ghostable);
   pos = p;
   velocity = v;
   liveTime = t;
   collided = false;
   alive = true;
   mShooter = shooter;
}

U32 Projectile::packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream)
{
   ((GameConnection *) connection)->writeCompressedPoint(pos, stream);
   writeCompressedVelocity(velocity, CompressedVelocityMax, stream);

   S32 index = -1;
   if(mShooter.isValid())
      index = connection->getGhostIndex(mShooter);
   if(stream->writeFlag(index != -1))
      stream->writeInt(index, GhostConnection::GhostIdBitSize);
   stream->writeFlag(collided);

   return 0;
}

void Projectile::unpackUpdate(GhostConnection *connection, BitStream *stream)
{
   ((GameConnection *) connection)->readCompressedPoint(pos, stream);
   readCompressedVelocity(velocity, CompressedVelocityMax, stream);

   if(stream->readFlag())
      mShooter = (Ship *) connection->resolveGhost(stream->readInt(GhostConnection::GhostIdBitSize));

   collided = stream->readFlag();
   if(collided)
      explode(NULL, pos);
   else
      pos += velocity * -0.020f;

   Rect newExtent(pos,pos);
   setExtent(newExtent);
   mCurrentMove.time = U32(connection->getOneWayTime());
   idle(GameObject::ClientIdleMainRemote);

   SFXObject::play(0, pos, velocity);
}

enum {
   NumSparkColors = 4,
};

Color SparkColors[NumSparkColors] = {
Color(1, 0, 1),
Color(1, 1, 1),
Color(0, 0, 1),
Color(1, 0, 0),
};

void Projectile::handleCollision(GameObject *hitObject, Point collisionPoint)
{
   collided = true;

   if(!isGhost())
   {
      DamageInfo theInfo;
      theInfo.collisionPoint = collisionPoint;
      theInfo.damageAmount = 0.21;
      theInfo.damageType = 0;
      theInfo.damagingObject = mShooter;
      theInfo.impulseVector = velocity;

      hitObject->damageObject(&theInfo);
   }

   liveTime = 0;
   explode(hitObject, collisionPoint);
}

void Projectile::explode(GameObject *hitObject, Point thePos)
{
   // Do some particle spew...
   if(isGhost())
   {
      SparkManager::emitExplosion(thePos, 0.4, SparkColors, NumSparkColors);

      Ship *s = dynamic_cast<Ship*>(hitObject);
      if(s && s->mShield)
         SFXObject::play(SFXBounceShield, thePos, velocity);
      else
         SFXObject::play(SFXPhaserImpact, thePos, velocity);
   }
}

void Projectile::idle(GameObject::IdleCallPath path)
{
   U32 deltaT = mCurrentMove.time;
   if(!collided)
   {
      Point endPos = pos + velocity * deltaT * 0.001;
      static Vector<GameObject *> disableVector;

      Rect queryRect(pos, endPos);

      float collisionTime;
      disableVector.clear();

      if(mShooter.isValid())
      {
         disableVector.push_back(mShooter);
         mShooter->disableCollision();
      }

      GameObject *hitObject;
      for(;;)
      {
         hitObject = findObjectLOS(MoveableType | BarrierType, MoveObject::RenderState, pos, endPos, collisionTime);
         if(!hitObject || hitObject->collide(this))
            break;
         disableVector.push_back(hitObject);
         hitObject->disableCollision();
      }

      for(S32 i = 0; i < disableVector.size(); i++)
         disableVector[i]->enableCollision();

      if(hitObject)
      {
         Point collisionPoint = pos + (endPos - pos) * collisionTime;
         handleCollision(hitObject, collisionPoint);
      }
      else
         pos = endPos;

      Rect newExtent(pos,pos);
      setExtent(newExtent);
   }

   if(path == GameObject::ClientIdleMainRemote)
      liveTime += deltaT;
   else if(alive)
   {
      if(liveTime <= deltaT)
      {
         getGame()->deleteObject(this, 500);
         liveTime = 0;
         alive = false;
      }
      else
         liveTime -= deltaT;
   }
}

void Projectile::render()
{
   if(collided)
      return;

   glColor3f(1,0,0.5);
   glPushMatrix();
   glTranslatef(pos.x, pos.y, 0);

   glPushMatrix();
   glRotatef((liveTime % 720) * 0.5, 0, 0, 1);

   glBegin(GL_LINE_LOOP);
   glVertex2f(-2, 2);
   glVertex2f(0, 6);
   glVertex2f(2, 2);
   glVertex2f(6, 0);
   glVertex2f(2, -2);
   glVertex2f(0, -6);
   glVertex2f(-2, -2);
   glVertex2f(-6, 0);
   glEnd();

   glPopMatrix();

   glRotatef(180 - (liveTime % 360), 0, 0, 1);
   glColor3f(0.5,0,1);
   glBegin(GL_LINE_LOOP);
   glVertex2f(-2, 2);
   glVertex2f(0, 8);
   glVertex2f(2, 2);
   glVertex2f(8, 0);
   glVertex2f(2, -2);
   glVertex2f(0, -8);
   glVertex2f(-2, -2);
   glVertex2f(-8, 0);
   glEnd();

   glPopMatrix();
}

};
