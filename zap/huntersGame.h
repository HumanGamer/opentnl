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

#ifndef _HUNTERSGAME_H_
#define _HUNTERSGAME_H_

#include "gameType.h"
#include "item.h"

namespace Zap
{

class Ship;
class HuntersFlagItem;
class HuntersNexusObject;

class HuntersGameType : public GameType
{
   typedef GameType Parent;
   enum Scores
   {
      KillScore    = 100,
      CapScore     = 200,
      YSaleBonus   = 125,
   };
   enum 
   {
      NexusCapDelay    = 15 * 1000,
      NexusReturnDelay = 60 * 1000,
      FlagCountDelay   = 1000,
   };
   Timer mNexusReturnTimer;
   Timer mNexusCapTimer;
   Timer mFlagCountTimer;

public:
   U32 mThisClientFlagCount;
   bool mCanNexusCap;

   HuntersGameType();
   void shipTouchNexus(Ship *theShip, HuntersNexusObject *theNexus);
   void updateClientFlagCount(GameConnection *con, U32 flagCount);

   void onGhostAvailable(GhostConnection *theConnection);
   void idle(GameObject::IdleCallPath path);
   void renderInterfaceOverlay(bool scoreboardVisible);
   void controlObjectForClientKilled(GameConnection *theClient, GameObject *clientObject, GameObject *killerObject);
   void gameOverManGameOver();

   enum {
      HuntersMsgScore,
      HuntersMsgYardSale,
      HuntersMsgGameOverWin,
      HuntersMsgGameOverTie,
   };

   TNL_DECLARE_RPC(s2cFlagCarryUpdate, (U32 flagCount));
   TNL_DECLARE_RPC(s2cSetNexusTimer, (U32 nexusTime, bool canCap));
   TNL_DECLARE_RPC(s2cHuntersMessage, (U32 msgIndex, StringTableEntryRef clientName, U32 flagCount));
   TNL_DECLARE_CLASS(HuntersGameType);
};

class HuntersFlagItem : public Item
{
   typedef Item Parent;

public:
   bool yardSaleFlag;
   HuntersFlagItem(Point pos = Point());

   void renderItem(Point pos);
   void onMountDestroyed();
   bool collide(GameObject *hitObject);

   TNL_DECLARE_CLASS(HuntersFlagItem);
};

class HuntersNexusObject : public GameObject
{
   typedef GameObject Parent;
   Rect nexusBounds;
public:
   HuntersNexusObject();

   void onAddedToGame(Game *theGame);
   void processArguments(S32 argc, const char **argv);

   void render();
   S32 getRenderSortValue() { return -1; }

   bool getCollisionPoly(Vector<Point> &polyPoints);
   bool collide(GameObject *hitObject);

   U32 packUpdate(GhostConnection *connection, U32 mask, BitStream *stream);
   void unpackUpdate(GhostConnection *connection, BitStream *stream);

   TNL_DECLARE_CLASS(HuntersNexusObject);
};

};

#endif  // _HUNTERSGAME_H_
