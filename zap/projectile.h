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

#ifndef _PROJECTILE_H_
#define _PROJECTILE_H_

#include "gameObject.h"

namespace Zap
{

class Ship;

class Projectile : public GameObject
{
public:
   Point pos;
   Point velocity;
   U32 liveTime;
   bool collided;
   bool alive;
   SafePtr<Ship> mShooter;

   Projectile(Point pos = Point(), Point vel = Point(), U32 liveTime = 0, Ship *shooter = NULL);

   U32 packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream);
   void unpackUpdate(GhostConnection *connection, BitStream *stream);

   void handleCollision(GameObject *theObject, Point collisionPoint);
   void process(U32 deltaT);
   void processServer(U32 deltaT);
   void processClient(U32 deltaT);

   virtual Point getRenderVel() { return velocity; }
   virtual Point getActualVel() { return velocity; }

   void render();

   TNL_DECLARE_CLASS(Projectile);
};

};
#endif
