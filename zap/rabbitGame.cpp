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

#include "rabbitGame.h"
#include "ship.h"
#include "UIGame.h"
#include "sfx.h"
#include "flagItem.h"

#include "glutInclude.h"
#include <stdio.h>

namespace Zap
{

TNL_IMPLEMENT_NETOBJECT_RPC(RabbitGameType, s2cRabbitMessage, (U32 msgIndex, StringTableEntry clientName), (msgIndex, clientName),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   switch (msgIndex)
   {
   case RabbitMsgGrab:
      SFXObject::play(SFXFlagCapture);
      gGameUserInterface.displayMessage(Color(1.0f, 0.0f, 0.0f),
                  "%s GRABBED the Carrot!",
                  clientName.getString());
      break;
   case RabbitMsgRabbitKill:
      SFXObject::play(SFXShipHeal);
      gGameUserInterface.displayMessage(Color(1.0f, 0.0f, 0.0f),
                  "%s is a rabbid rabbit!",
                  clientName.getString());
      break;
   case RabbitMsgDrop:
      SFXObject::play(SFXFlagDrop);
      gGameUserInterface.displayMessage(Color(0.0f, 1.0f, 0.0f),
                  "%s DROPPED the Carrot!",
                  clientName.getString());
      break;
   case RabbitMsgRabbitDead:
      SFXObject::play(SFXShipExplode);
      gGameUserInterface.displayMessage(Color(1.0f, 0.0f, 0.0f),
                  "%s killed the rabbit!",
                  clientName.getString());
      break;
   case RabbitMsgReturn:
      SFXObject::play(SFXFlagReturn);
      gGameUserInterface.displayMessage(Color(1.0f, 0.0f, 1.0f),
                  "The Carrot has been returned!");
      break;
   case RabbitMsgGameOverWin:
      gGameUserInterface.displayMessage(Color(1.0f, 1.0f, 0.0f),
                  "%s is the top rabbit!",
                  clientName.getString());
      break;
   case RabbitMsgGameOverTie:
      gGameUserInterface.displayMessage(Color(1.0f, 1.0f, 0.0f),
                  "No top rabbit - Carrot wins by default!");
      break;
   }
}

//-----------------------------------------------------
// RabbitGameType
//-----------------------------------------------------
TNL_IMPLEMENT_NETOBJECT(RabbitGameType);

void RabbitGameType::processArguments(S32 argc, const char **argv)
{
   if (argc != 4)
      return;

   Parent::processArguments(argc, argv);

   mFlagReturnTimer = Timer(atoi(argv[2]) * 1000);
   mFlagScoreTimer = Timer(1.0f / atoi(argv[3]) * 60 * 1000); //secs per point
}

void RabbitGameType::spawnShip(GameConnection *theClient)
{
   Parent::spawnShip(theClient);
   ClientRef *cl = theClient->getClientRef();
   setClientShipLoadout(cl, theClient->getLoadout());
}

bool RabbitGameType::objectCanDamageObject(GameObject *damager, GameObject *victim)
{
   if(!damager)
      return true;

   GameConnection *c1 = damager->getOwner();
   GameConnection *c2 = victim->getOwner();

   if( (!c1 || !c2) || (c1 == c2))
      return true;

   Ship *damnShip = (Ship *) c1->getControlObject();
   Ship *victimShip = (Ship *) c2->getControlObject();

   if(!damnShip || !victimShip)
      return true;

   //only hunters can hurt rabbits and only rabbits can hurt hunters
   return shipHasFlag(damnShip) != shipHasFlag(victimShip);
}

Color RabbitGameType::getShipColor(Ship *s)
{
   GameConnection *gc = gClientGame->getConnectionToServer();
   if(!gc)
      return Color();
   Ship *co = (Ship *) gc->getControlObject();

   if(s == co || shipHasFlag(s) == shipHasFlag(co))
      return Color(0,1,0);
   return Color(1,0,0);
}

bool RabbitGameType::shipHasFlag(Ship *ship)
{
   if (!ship)
      return false;

   for (S32 k = 0; k < ship->mMountedItems.size(); k++)
   {
      if (dynamic_cast<FlagItem *>(ship->mMountedItems[k].getPointer()))
         return true;
   }
   return false;
}

void RabbitGameType::onClientScore(Ship *ship, S32 howMuch)
{
   GameConnection *controlConnection = ship->getControllingClient();
   ClientRef *cl = controlConnection->getClientRef();

   if(!cl)
      return;

   cl->score += howMuch;
   if (cl->score >= mTeamScoreLimit)
      gameOverManGameOver();
}

void RabbitGameType::idle(GameObject::IdleCallPath path)
{
   Parent::idle(path);
   if(path != GameObject::ServerIdleMainLoop || !mRabbitFlag)
      return;

   U32 deltaT = mCurrentMove.time;

   if (mRabbitFlag->isMounted())
   {
      if (mFlagScoreTimer.update(deltaT))
      {
         onFlagHeld(mRabbitFlag->getMount());
         mFlagScoreTimer.reset();
      }
   }
   else
   {
      if (!mRabbitFlag->isAtHome() && mFlagReturnTimer.update(deltaT))
      {
         mFlagReturnTimer.reset();
         mRabbitFlag->sendHome();
         static StringTableEntry returnString("The carrot has been returned!");
         for (S32 i = 0; i < mClientList.size(); i++)
            mClientList[i]->clientConnection->s2cDisplayMessageE(GameConnection::ColorNuclearGreen, SFXFlagReturn, returnString, NULL);
      }
   }
   Parent::idle(path);
}

void RabbitGameType::controlObjectForClientKilled(GameConnection *theClient, GameObject *clientObject, GameObject *killerObject)
{
   Parent::controlObjectForClientKilled(theClient, clientObject, killerObject);

   Ship *killerShip = NULL;
   GameConnection *ko = killerObject->getOwner();
   if(ko)
      killerShip = (Ship *) ko->getControlObject();

   Ship *victimShip = dynamic_cast<Ship *>(clientObject);

   if (killerShip)
   {
      if (shipHasFlag(killerShip))
      {
         //rabbit killed another person
         onFlaggerKill(killerShip);
      }
      else if (shipHasFlag(victimShip))
      {
         //someone killed the rabbit!  Poor rabbit!
         onFlaggerDead(killerShip);
      }
   }
}

void RabbitGameType::shipTouchFlag(Ship *ship, FlagItem *flag)
{
   s2cRabbitMessage(RabbitMsgGrab, ship->mPlayerName);

   flag->mountToShip(ship);
}

void RabbitGameType::flagDropped(Ship *theShip, FlagItem *theFlag)
{
   mFlagScoreTimer.reset();
   mFlagReturnTimer.reset();
   s2cRabbitMessage(RabbitMsgDrop, theShip->mPlayerName);
   Point vel = theShip->getActualVel();
   theFlag->setActualVel(vel);
}

void RabbitGameType::onFlagHeld(Ship *ship)
{
   onClientScore(ship, 1);
}

void RabbitGameType::addFlag(FlagItem *theFlag)
{
   mRabbitFlag = theFlag;
   theFlag->setScopeAlways();
}

void RabbitGameType::onFlaggerKill(Ship *rabbitShip)
{
   s2cRabbitMessage(RabbitMsgRabbitKill, rabbitShip->mPlayerName);
   onClientScore(rabbitShip, RabbidRabbitBonus);
}

void RabbitGameType::onFlaggerDead(Ship *killerShip)
{
   s2cRabbitMessage(RabbitMsgRabbitDead, killerShip->mPlayerName);
   onClientScore(killerShip, RabbitKillBonus);
}

};  //namespace Zap

