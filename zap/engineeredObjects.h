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

#ifndef _ENGINEEREDOBJECTS_H_
#define _ENGINEEREDOBJECTS_H_

#include "gameObject.h"
#include "item.h"
#include "barrier.h"

namespace Zap
{

extern void engClientCreateObject(GameConnection *connection, U32 object);

class EngineeredObject : public GameObject
{
   typedef GameObject Parent;
protected:
   F32 mHealth;
   S32 mTeam;
   Color mTeamColor;
   SafePtr<Item> mResource;
   Point mAnchorPoint;
   Point mAnchorNormal;

   enum MaskBits
   {
      InitialMask  = BIT(0),
      NextFreeMask = BIT(1),
   };
   
public:
   SafePtr<Ship> mOwner;

   EngineeredObject(S32 team = -1, Point anchorPoint = Point(), Point anchorNormal = Point());
   void setResource(Item *resource);
   bool checkDeploymentPosition();
   void computeExtent();

   S32 getTeam() { return mTeam; }

   U32 packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream);
   void unpackUpdate(GhostConnection *connection, BitStream *stream);

   void damageObject(DamageInfo *damageInfo);
   bool collide(GameObject *hitObject) { return true; }
};

class ForceFieldProjector : public EngineeredObject
{
   typedef EngineeredObject Parent;

   SafePtr<Barrier>  mField;
   Timer             mFieldDown;
   F32               mLength;

public:
   ForceFieldProjector(S32 team = -1, Point anchorPoint = Point(), Point anchorNormal = Point()) :
      EngineeredObject(team, anchorPoint, anchorNormal) { mNetFlags.set(Ghostable); mLength = 0.1f; }

   ~ForceFieldProjector();

   enum Constants
   {
      FieldDownTime = 200,

      FieldDownMask = BIT(2),
   };

   bool getCollisionPoly(Vector<Point> &polyPoints);
   void render();
   void idle(IdleCallPath path);

   TNL_DECLARE_CLASS(ForceFieldProjector);
};

class Turret : public EngineeredObject
{
   typedef EngineeredObject Parent;

   Timer mFireTimer;

public:
   Turret(S32 team = -1, Point anchorPoint = Point(), Point anchorNormal = Point()) :
      EngineeredObject(team, anchorPoint, anchorNormal) { mNetFlags.set(Ghostable); }

   bool getCollisionPoly(Vector<Point> &polyPoints);
   void render();
   void idle(IdleCallPath path);

   TNL_DECLARE_CLASS(Turret);
};

};

#endif