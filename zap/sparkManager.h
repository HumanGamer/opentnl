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

#ifndef _FXMANAGER_H_
#define _FXMANAGER_H_

#include "point.h"
#include "gameObject.h"

namespace Zap
{

namespace FXManager
{
   void emitSpark(Point pos, Point vel, Color color, F32 ttl=0);
   void emitExplosion(Point pos, F32 size, Color *colorArray, U32 numColors);
   void emitBurst(Point pos, Point scale, Color color1, Color color2);
   void emitTeleportInEffect(Point pos, U32 type);
   void tick( F32 dT);
   void render(U32 renderPass);
};

class FXTrail
{
private:
   struct TrailNode
   {
      Point pos;
      S32   ttl;
      bool  boosted;
      bool  invisible;
   };

   Vector<TrailNode> mNodes;

   U32 mDropFreq;
   S32 mLength;

   FXTrail *mNext;

   
   static FXTrail *mHead;
   void registerTrail();
   void unregisterTrail();


public:
   FXTrail(U32 dropFrequency = 32, U32 len = 15);
   ~FXTrail();

   /// Update the point this trail is attached to.
   void update(Point pos, bool boosted = false, bool invisible = false);

   void tick(U32 dT);

   void render();

   void reset();

   Point getLastPos();

   static void renderTrails();
};

};

#endif
