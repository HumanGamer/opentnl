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

#ifndef _CTFGAME_H_
#define _CTFGAME_H_

#include "gameType.h"
#include "item.h"

namespace Zap
{

class Ship;
class CTFFlagItem;

class CTFGameType : public GameType
{
   enum Scores
   {
      KillScore    = 100,
      ReturnScore  = 150,
      CapScore     = 500,
      CapTeamScore = 250,

   };
public:
   void shipTouchFlag(Ship *theShip, CTFFlagItem *theFlag);
   void renderInterfaceOverlay(bool scoreboardVisible);
   void updateClientScoreboard(S32 clientIndex);
   void controlObjectForClientKilled(GameConnection *theClient, GameObject *clientObject, GameObject *killerObject);
   void controlObjectForClientRemoved(GameConnection *theClient, GameObject *clientObject);
   U32 checkFlagDrop(GameObject *theObject);

   enum {
      MaxPing = 999,
   };

   static Vector<RangedU32<0, MaxPing> > mPingTimes; ///< Static vector used for constructing update RPCs
   static Vector<Int<24> > mScores;

   enum {
      CTFMsgReturnFlag,
      CTFMsgCaptureFlag,
      CTFMsgTakeFlag,
      CTFMsgDropFlag,
   };

   TNL_DECLARE_RPC(s2cCTFMessage, (U32 messageIndex, U32 clientId, U32 teamIndex));
   TNL_DECLARE_RPC(s2cKillMessage, (StringTableEntry victim, StringTableEntry killer));
   TNL_DECLARE_RPC(s2cScoreboardUpdate, (const Vector<RangedU32<0, MaxPing> > &pingTimes, const Vector<Int<24> > &scores));
   TNL_DECLARE_CLASS(CTFGameType);
};

class CTFFlagItem : public Item
{
   typedef Item Parent;
   U32 teamIndex;
   Point initialPos;
public:
   CTFFlagItem(Point pos = Point());
   void processArguments(S32 argc, const char **argv);
   void renderItem(Point pos);
   void sendHome();

   bool collide(GameObject *hitObject);
   bool isAtHome();
   U32 getTeamIndex() { return teamIndex; }

   U32 packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream);
   void unpackUpdate(GhostConnection *connection, BitStream *stream);

   TNL_DECLARE_CLASS(CTFFlagItem);
};

};


#endif