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

#include "gameConnection.h"
#include "game.h"
#include "gameType.h"

#include "UIGame.h"
#include "UIMenus.h"

namespace Zap
{
// Global list of clients (if we're a server).
GameConnection GameConnection::gClientList;

TNL_IMPLEMENT_NETCONNECTION(GameConnection, NetClassGroupGame, true);

GameConnection::GameConnection()
{
   mNext = mPrev = this;
   setTranslatesStrings();
   mInCommanderMap = false;
}

GameConnection::~GameConnection()
{
   // unlink ourselves if we're in the client list
   mPrev->mNext = mNext;
   mNext->mPrev = mPrev;

   // Tell the user...
   logprintf("%s disconnected", getNetAddress().toString());
}

/// Adds this connection to the doubly linked list of clients.
void GameConnection::linkToClientList()
{
   mNext = gClientList.mNext;
   mPrev = gClientList.mNext->mPrev;
   mNext->mPrev = this;
   mPrev->mNext = this;
}

GameConnection *GameConnection::getClientList()
{
   return gClientList.getNextClient();
}

GameConnection *GameConnection::getNextClient()
{
   if(mNext == &gClientList)
      return NULL;
   return mNext;
}

TNL_IMPLEMENT_RPC(GameConnection, c2sRequestCommanderMap, (), NetClassGroupGameMask, RPCGuaranteedOrdered, RPCDirClientToServer, 1)
{
   mInCommanderMap = true;
}

TNL_IMPLEMENT_RPC(GameConnection, c2sReleaseCommanderMap, (), NetClassGroupGameMask, RPCGuaranteedOrdered, RPCDirClientToServer, 1)
{
   mInCommanderMap = false;
}

TNL_IMPLEMENT_RPC(GameConnection, c2sRequestLoadout, (const Vector<U32> &loadout), NetClassGroupGameMask, RPCGuaranteedOrdered, RPCDirClientToServer, 1)
{
   mLoadout = loadout;
   GameType *gt = gServerGame->getGameType();
   if(gt)
      gt->clientRequestLoadout(this, mLoadout);
}


void GameConnection::writeConnectRequest(BitStream *stream)
{
   Parent::writeConnectRequest(stream);

   stream->writeString(mClientName.getString());
}

bool GameConnection::readConnectRequest(BitStream *stream, const char **errorString)
{
   if(!Parent::readConnectRequest(stream, errorString))
      return false;

   if(gServerGame->isFull())
   {
      *errorString = "Server Full.";
      return false;
   }

   char buf[256];
   
   stream->readString(buf);
   size_t len = strlen(buf);

   if(len > 252)
      len = 252;
   U32 index = 0;

checkPlayerName:
   for(GameConnection *walk = getClientList(); walk; walk = walk->getNextClient())
   {
      if(!strcmp(walk->mClientName.getString(), buf))
      {
         dSprintf(buf + len, 3, ".%d", index);
         index++;
         goto checkPlayerName;
      }
   }

   mClientName = buf;
   return true;
}

void GameConnection::onConnectionEstablished()
{
   Parent::onConnectionEstablished();

   if(isInitiator())
   {
      setGhostFrom(false);
      setGhostTo(true);
      logprintf("%s - connected to server.", getNetAddressString());
      setFixedRateParameters(50, 50, 2000, 2000);
   }
   else
   {
      linkToClientList();
      gServerGame->addClient(this);
      setGhostFrom(true);
      setGhostTo(false);
      activateGhosting();
      logprintf("%s - client \"%s\" connected.", getNetAddressString(), mClientName.getString());
      setFixedRateParameters(50, 50, 2000, 2000);
   }
}

void GameConnection::onConnectionTerminated(NetConnection::TerminationReason r, const char *reason)
{
   if(isInitiator())
   {
      gMainMenuUserInterface.activate();
   }
   else
   {
      gServerGame->removeClient(this);
   }
}

void GameConnection::onConnectTerminated(TerminationReason r, const char *string)
{
   if(isInitiator())
      gMainMenuUserInterface.activate();
}

};

