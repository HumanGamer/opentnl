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

#ifndef _SHIP_H_
#define _SHIP_H_

#include "moveObject.h"
#include "sparkManager.h"
#include "sfx.h"
#include "timer.h"
namespace Zap
{

class Item;

class Ship : public MoveObject
{
   typedef MoveObject Parent;
public:
   enum {
//      MaxVelocity = 500, // points per second
//      Acceleration = 800, // points per second per second
      MaxVelocity = 450, // points per second
      Acceleration = 2500, // points per second per second
      TurboMaxVelocity = 700, // points per second
      TurboAcceleration = 5000, // points per second per second

      CollisionRadius = 24,
      VisibilityRadius = 30,
      KillDeleteDelay = 1500,
      ExplosionFadeTime = 300,
      FireDelay = 100,
      MaxControlObjectInterpDistance = 200,
      TrailCount = 2,
      EnergyMax = 100000,
      EnergyRechargeRate = 6000, // How many percent/second
      EnergyTurboDrain = 15000,
      EnergyShieldDrain = 15000,
      EnergyShieldHitDrain = 20000,
      EnergyShootDrain = 500,
      EnergyCooldownThreshold = 15000,
   };

   enum MaskBits {
      InitialMask       = BIT(0),
      PositionMask      = BIT(1),
      MoveMask          = BIT(2),
      WarpPositionMask  = BIT(3),
      ExplosionMask     = BIT(4),
      HealthMask        = BIT(5),
      PowersMask        = BIT(6),
   };

   Timer mFireTimer;
   F32 mHealth;
   S32 mEnergy;
   StringTableEntry mPlayerName;
   bool mShield, mTurbo, mCooldown;
   SFXHandle mTurboNoise;

   U32     mSparkElapsed;
   S32     mLastTrailPoint[TrailCount];
   fxTrail mTrail[TrailCount];

   Color color; // color of the ship
   F32 mass; // mass of ship
   bool hasExploded;

   Vector<Point> posSegments;
   Vector<SafePtr<Item> > mMountedItems;

   void render();
   Ship(StringTableEntry playerName="", Point p = Point(0,0), F32 m = 1.0);

   F32 getHealth() { return mHealth; }

   void onGhostRemove();

   bool shouldToggleShieldOff() { return !mShield && mEnergy < EnergyCooldownThreshold; }
   void idle(IdleCallPath path);

   void processMove(U32 stateIndex);
   void processWeaponFire();
   void processEnergy();
   void updateTurboNoise();
   void emitMovementSparks();

   void controlMoveReplayComplete();

   void emitShipExplosion(Point pos);
   void setActualPos(Point p);

   virtual void damageObject(DamageInfo *theInfo);
   void kill(DamageInfo *theInfo);

   void writeControlState(BitStream *stream);
   void readControlState(BitStream *stream);

   U32 packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream);
   void unpackUpdate(GhostConnection *connection, BitStream *stream);

   void performScopeQuery(GhostConnection *connection);

   void processArguments(S32 argc, const char **argv);

   TNL_DECLARE_CLASS(Ship);
};

};

#endif
