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

#include "gameType.h"
#include "ship.h"
#include "UIGame.h"
#include "gameNetInterface.h"
#include "glutInclude.h"

namespace Zap
{

TNL_IMPLEMENT_NETOBJECT(GameType);

GameType::GameType()
   : mScoreboardUpdateTimer(1000)
   , mGameTimer(DefaultGameTime)
   , mGameTimeUpdateTimer(30000) 
{
   mNetFlags.set(Ghostable);
   mGameOver = false;
   mTeamScoreLimit = DefaultTeamScoreLimit;
}

void GameType::processArguments(S32 argc, const char **argv)
{
   if(argc > 0)
      mGameTimer.reset(U32(atof(argv[0]) * 60 * 1000));
   if(argc > 1)
      mTeamScoreLimit = atoi(argv[1]);
}

void GameType::processClient(U32 deltaT)
{
   mGameTimer.update(deltaT);
}

void GameType::processServer(U32 deltaT)
{
   if(mScoreboardUpdateTimer.update(deltaT))
   {
      mScoreboardUpdateTimer.reset();
      for(S32 i = 0; i < mClientList.size(); i++)
         if(mClientList[i].clientConnection)
            mClientList[i].ping = (U32) mClientList[i].clientConnection->getRoundTripTime();

      for(S32 i = 0; i < mClientList.size(); i++)
         if(mGameOver || mClientList[i].wantsScoreboardUpdates)
            updateClientScoreboard(i);
   }

   if(mGameTimeUpdateTimer.update(deltaT))
   {
      mGameTimeUpdateTimer.reset();
      s2cSetTimeRemaining(mGameTimer.getCurrent());
   }

   for(S32 i = 0; i < mClientList.size(); i++)
   {
      if(mClientList[i].respawnTimer.update(deltaT))
         spawnShip(mClientList[i].clientConnection);
   }

   if(mGameTimer.update(deltaT))
   {
      gameOverManGameOver();
   }
}

void GameType::renderInterfaceOverlay(bool scoreboardVisible)
{
   if((mGameOver || scoreboardVisible) && mTeams.size() > 0)
   {
      U32 totalWidth = 780;
      U32 teamWidth = totalWidth / mTeams.size();
      U32 maxTeamPlayers = 0;
      countTeamPlayers();

      for(S32 i = 0; i < mTeams.size(); i++)
         if(mTeams[i].numPlayers > maxTeamPlayers)
            maxTeamPlayers = mTeams[i].numPlayers;

      U32 teamAreaHeight = 40;
      if(mTeams.size() < 2)
         teamAreaHeight = 0;

      U32 totalHeight = 580;
      U32 maxHeight = (totalHeight - teamAreaHeight) / maxTeamPlayers;
      if(maxHeight > 30)
         maxHeight = 30;

      totalHeight = teamAreaHeight + maxHeight * maxTeamPlayers;
      U32 yt = (600 - totalHeight) / 2;
      U32 yb = yt + totalHeight;

      for(S32 i = 0; i < mTeams.size(); i++)
      {
         U32 xl = 10 + i * teamWidth;
         U32 xr = xl + teamWidth - 2;

         Color c = mTeams[i].color;
         glEnable(GL_BLEND);
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

         glColor4f(c.r, c.g, c.b, 0.6);
         glBegin(GL_POLYGON);
         glVertex2f(xl, yt);
         glVertex2f(xr, yt);
         glVertex2f(xr, yb);
         glVertex2f(xl, yb);
         glEnd();

         glDisable(GL_BLEND);
         glBlendFunc(GL_ONE, GL_ZERO);

         glColor3f(1,1,1);
         if(teamAreaHeight)
         {
            glBegin(GL_LINES);
            glVertex2f(xl, yt + teamAreaHeight);
            glVertex2f(xr, yt + teamAreaHeight);
            glEnd();

            UserInterface::drawString(xl + 40, yt + 2, 30, mTeams[i].name.getString());

            UserInterface::drawStringf(xr - 140, yt + 2, 30, "%d", mTeams[i].score);
         }

         U32 curRowY = yt + teamAreaHeight + 1;
         U32 fontSize = maxHeight * 0.8f;
         for(S32 j = 0; j < mClientList.size(); j++)
         {
            if(mClientList[j].teamId == i)
            {
               UserInterface::drawString(xl + 40, curRowY, fontSize, mClientList[j].name.getString());

               static char buff[255] = "";
               dSprintf(buff, sizeof(buff), "%d", mClientList[j].score);

               UserInterface::drawString(xr - (120 + UserInterface::getStringWidth(fontSize, buff)), curRowY, fontSize, buff);
               UserInterface::drawStringf(xr - 70, curRowY, fontSize, "%d", mClientList[j].ping);
               curRowY += maxHeight;
            }
         }
      }
   }
   renderTimeLeft();
   renderTalkingClients();
}

void GameType::renderTimeLeft()
{
   glColor3f(1,1,1);
   U32 timeLeft = mGameTimer.getCurrent();

   U32 minsRemaining = timeLeft / (60000);
   U32 secsRemaining = (timeLeft - (minsRemaining * 60000)) / 1000;
   UserInterface::drawStringf(720, 577, 20, "%02d:%02d", minsRemaining, secsRemaining);
}

void GameType::renderTalkingClients()
{
   S32 y = 150;
   for(S32 i = 0; i < mClientList.size(); i++)
   {
      if(mClientList[i].voiceSFX->isPlaying())
      {
         Color teamColor = mTeams[mClientList[i].teamId].color;
         glColor3f(teamColor.r, teamColor.g, teamColor.b);
         UserInterface::drawString(10, y, 20, mClientList[i].name.getString());
         y += 25;
      }
   }
}

void GameType::gameOverManGameOver()
{
   // 17 days??? We won't last 17 hours!
   mGameOver = true;
   s2cSetGameOver(true);
   gServerGame->gameEnded();
}

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, s2cSetGameOver, (bool gameOver),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   mGameOver = gameOver;
}

void GameType::onAddedToGame(Game *theGame)
{
   theGame->setGameType(this);
   setInterface(theGame->getNetInterface());
   if(!isGhost())
      setScopeAlways();
}

bool GameType::processLevelItem(S32 argc, const char **argv)
{
   if(!stricmp(argv[0], "Team"))
   {
      if(argc < 5)
         return false;
      Team t;
      t.numPlayers = 0;

      t.name.set(argv[1]);
      t.color.read(argv + 2);
      mTeams.push_back(t);
   }
   else if(!stricmp(argv[0], "Spawn"))
   {
      if(argc < 4)
         return false;
      S32 teamIndex = atoi(argv[1]);
      Point p;
      p.read(argv + 2);
      p *= getGame()->getGridSize();
      if(teamIndex >= 0 && teamIndex < mTeams.size())
         mTeams[teamIndex].spawnPoints.push_back(p);
   }
   else
      return false;
   return true;
}

S32 GameType::findClientIndexByConnection(GameConnection *theConnection)
{
   for(S32 clientIndex = 0; clientIndex < mClientList.size(); clientIndex++)
      if(mClientList[clientIndex].clientConnection == theConnection)
         return clientIndex;
   return -1;
}

S32 GameType::findClientIndexByName(const StringTableEntry &name)
{
   for(S32 clientIndex = 0; clientIndex < mClientList.size(); clientIndex++)
      if(mClientList[clientIndex].name == name)
         return clientIndex;
   return -1;
}

void GameType::spawnShip(GameConnection *theClient)
{
   S32 clientIndex = findClientIndexByConnection(theClient);
   S32 teamIndex = mClientList[clientIndex].teamId;

   Point spawnPoint;
   S32 spawnIndex = Random::readI() % mTeams[teamIndex].spawnPoints.size();
   spawnPoint = mTeams[teamIndex].spawnPoints[spawnIndex];

   Ship *newShip = new Ship(mClientList[clientIndex].name, spawnPoint);
   newShip->addToGame(getGame());
   theClient->setControlObject(newShip);
}

void GameType::performProxyScopeQuery(GameObject *scopeObject, GameConnection *connection)
{
   static Vector<GameObject *> fillVector;
   fillVector.clear();

   if(connection->mInCommanderMap && mTeams.size() > 1)
   {
      S32 teamId = mClientList[findClientIndexByConnection(connection)].teamId;

      for(S32 i = 0; i < mClientList.size(); i++)
      {
         if(mClientList[i].teamId == teamId)
         {
            GameObject *co = mClientList[i].clientConnection->getControlObject();
            if(!co)
               continue;

            Point pos = co->getActualPos();

            Rect queryRect(pos, pos);
            queryRect.expand(Point(Game::PlayerHorizScopeDistance, Game::PlayerVertScopeDistance));
            findObjects(scopeObject == co ? AllObjectTypes : CommandMapVisType, fillVector, queryRect);
         }
      }
   }
   else
   {
      Point pos = scopeObject->getActualPos();

      Rect queryRect(pos, pos);
      queryRect.expand(Point(Game::PlayerHorizScopeDistance, Game::PlayerVertScopeDistance));
      findObjects(AllObjectTypes, fillVector, queryRect);
   }

   for(S32 i = 0; i < fillVector.size(); i++)
      connection->objectInScope(fillVector[i]);
}

void GameType::countTeamPlayers()
{
   for(S32 i = 0; i < mTeams.size(); i ++)
      mTeams[i].numPlayers = 0;

   for(S32 i = 0; i < mClientList.size(); i++)
      mTeams[mClientList[i].teamId].numPlayers++;
}

void GameType::serverAddClient(GameConnection *theClient)
{
   ClientRef cref;
   cref.name = theClient->playerName;

   cref.clientConnection = theClient;
   cref.wantsScoreboardUpdates = false;

   countTeamPlayers();

   U32 minPlayers = mTeams[0].numPlayers;
   S32 minTeamIndex = 0;

   for(S32 i = 1; i < mTeams.size(); i++)
   {
      if(mTeams[i].numPlayers < minPlayers)
      {
         minTeamIndex = i;
         minPlayers = mTeams[i].numPlayers;
      }
   }
   cref.teamId = minTeamIndex;
   mClientList.push_back(cref);

   s2cAddClient(cref.name, false);
   s2cClientJoinedTeam(cref.name, cref.teamId);
   spawnShip(theClient);
}

void GameType::controlObjectForClientKilled(GameConnection *theClient, GameObject *clientObject, GameObject *killerObject)
{
   GameConnection *killer = killerObject ? killerObject->getControllingClient() : NULL;
   S32 killerIndex = findClientIndexByConnection(killer);
   S32 clientIndex = findClientIndexByConnection(theClient);

   if(killerIndex != -1)
   {
      // Punish team killers slightly
      if(mTeams.size() > 1 && mClientList[killerIndex].teamId == mClientList[clientIndex].teamId)
         mClientList[killerIndex].score -= 1;
      else
         mClientList[killerIndex].score += 1;

      s2cKillMessage(mClientList[clientIndex].name, mClientList[killerIndex].name);
   }
   mClientList[clientIndex].respawnTimer.reset(RespawnDelay);
}

void GameType::controlObjectForClientRemoved(GameConnection *theClient, GameObject *clientObject)
{

}

void GameType::addClientGameMenuOptions(Vector<const char *> &menuOptions)
{
   if(mTeams.size() > 1)
      menuOptions.push_back("CHANGE TEAMS");
}

void GameType::processClientGameMenuOption(U32 index)
{
   if(index == 0)
      c2sChangeTeams();
}

void GameType::setTeamScore(S32 teamIndex, S32 newScore)
{
   mTeams[teamIndex].score = newScore;
   s2cSetTeamScore(teamIndex, newScore);
   if(newScore >= mTeamScoreLimit)
      gameOverManGameOver();
}

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, s2cSetTimeRemaining, (U32 timeLeft),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   mGameTimer.reset(timeLeft);
}

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, c2sChangeTeams, (),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhostParent, 0)
{
   if(mTeams.size() <= 1)
      return;

   GameConnection *source = (GameConnection *) NetObject::getRPCSourceConnection();
   S32 clientIndex = findClientIndexByConnection(source);

   // destroy the old ship
   GameObject *co = source->getControlObject();
   controlObjectForClientRemoved(source, co);
   if(co)
      getGame()->deleteObject(co, 0);

   U32 newTeamId = (mClientList[clientIndex].teamId + 1) % mTeams.size();
   mClientList[clientIndex].teamId = newTeamId;
   s2cClientJoinedTeam(mClientList[clientIndex].name, newTeamId);
   spawnShip(source);
}

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, s2cAddClient, (StringTableEntryRef name, bool isMyClient),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   ClientRef cref;
   cref.name = name;
   cref.teamId = 0;
   cref.wantsScoreboardUpdates = false;
   cref.ping = 0;
   cref.decoderState = create_lpc10_decoder_state();
   init_lpc10_decoder_state(cref.decoderState);
   cref.voiceSFX = new SFXObject(SFXVoice, NULL, 1, Point(), Point());

   mClientList.push_back(cref);

   if(isMyClient)
      mThisClientName = name;
   gGameUserInterface.displayMessage(Color(0.6f, 0.6f, 0.8f), "%s joined the game.", name.getString());
}

void GameType::serverRemoveClient(GameConnection *theClient)
{
   S32 clientIndex = findClientIndexByConnection(theClient);

   mClientList.erase(clientIndex);

   GameObject *theControlObject = theClient->getControlObject();
   controlObjectForClientRemoved(theClient, theControlObject);
   if(theControlObject)
      getGame()->deleteObject(theControlObject, 0);

   s2cRemoveClient(theClient->playerName);
}

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, s2cRemoveClient, (StringTableEntryRef name),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   S32 clientIndex = findClientIndexByName(name);
   destroy_lpc10_decoder_state(mClientList[clientIndex].decoderState);

   gGameUserInterface.displayMessage(Color(0.6f, 0.6f, 0.8f), "%s left the game.", name.getString());
   mClientList.erase(clientIndex);
}

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, s2cAddTeam, (StringTableEntryRef teamName, F32 r, F32 g, F32 b),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   Team team;
   team.name = teamName;
   team.color.r = r;
   team.color.g = g;
   team.color.b = b;
   mTeams.push_back(team);
}

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, s2cSetTeamScore, (U32 teamIndex, U32 score),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   mTeams[teamIndex].score = score;
}

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, s2cClientJoinedTeam, (StringTableEntryRef name, U32 teamIndex),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   S32 clientIndex = findClientIndexByName(name);
   mClientList[clientIndex].teamId = teamIndex;
   gGameUserInterface.displayMessage(Color(0.6f, 0.6f, 0.8f), "%s joined team %s.", mClientList[clientIndex].name.getString(), mTeams[teamIndex].name.getString());
}

void GameType::onGhostAvailable(GhostConnection *theConnection)
{
   NetObject::setRPCDestConnection(theConnection);

   for(S32 i = 0; i < mTeams.size(); i++)
   {
      s2cAddTeam(mTeams[i].name, mTeams[i].color.r, mTeams[i].color.g, mTeams[i].color.b);
      s2cSetTeamScore(i, mTeams[i].score);
   }

   // add all the client and team information
   for(S32 i = 0; i < mClientList.size(); i++)
   {
      s2cAddClient(mClientList[i].name, mClientList[i].clientConnection == theConnection);
      s2cClientJoinedTeam(mClientList[i].name, mClientList[i].teamId);
   }
   s2cSetTimeRemaining(mGameTimer.getCurrent());
   s2cSetGameOver(mGameOver);
   NetObject::setRPCDestConnection(NULL);
}

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, c2sSendChat, (bool global, const char *message),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhostParent, 0)
{
   GameConnection *source = (GameConnection *) getRPCSourceConnection();
   S32 clientIndex = findClientIndexByConnection(source);

   RefPtr<NetEvent> theEvent = TNL_RPC_CONSTRUCT_NETEVENT(this, 
      s2cDisplayChatMessage, (global, source->playerName, message));

   S32 teamId = 0;
   
   if(!global)
      teamId = mClientList[clientIndex].teamId;

   for(S32 i = 0; i < mClientList.size(); i++)
   {
      if(global || mClientList[i].teamId == teamId)
         if(mClientList[i].clientConnection)
            mClientList[i].clientConnection->postNetEvent(theEvent);
   }
}

extern Color gGlobalChatColor;
extern Color gTeamChatColor;

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, s2cDisplayChatMessage, (bool global, StringTableEntryRef clientName, const char *message),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   Color theColor = global ? gGlobalChatColor : gTeamChatColor;

   gGameUserInterface.displayMessage(theColor, "%s: %s", clientName.getString(), message);
}

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, c2sRequestScoreboardUpdates, (bool updates),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhostParent, 0)
{
   GameConnection *source = (GameConnection *) getRPCSourceConnection();
   S32 clientIndex = findClientIndexByConnection(source);
   mClientList[clientIndex].wantsScoreboardUpdates = updates;
   if(updates)
      updateClientScoreboard(clientIndex);
}

Vector<RangedU32<0, GameType::MaxPing> > GameType::mPingTimes; ///< Static vector used for constructing update RPCs
Vector<Int<24> > GameType::mScores;

void GameType::updateClientScoreboard(S32 clientIndex)
{
   mPingTimes.clear();
   mScores.clear();

   for(S32 i = 0; i < mClientList.size(); i++)
   {
      if(mClientList[i].ping < MaxPing)
         mPingTimes.push_back(mClientList[i].ping);
      else
         mPingTimes.push_back(MaxPing);
      mScores.push_back(mClientList[i].score);
   }

   NetObject::setRPCDestConnection(mClientList[clientIndex].clientConnection);
   s2cScoreboardUpdate(mPingTimes, mScores);
   NetObject::setRPCDestConnection(NULL);
}

TNL_DECLARE_ENUM(GameType::MaxPing);

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, s2cScoreboardUpdate, (const Vector<RangedU32<0, GameType::MaxPing> > &pingTimes, const Vector<Int<24> > &scores),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   for(S32 i = 0; i < mClientList.size(); i++)
   {
      if(i >= pingTimes.size())
         break;

      mClientList[i].ping = pingTimes[i];
      mClientList[i].score = scores[i];
   }
}

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, s2cKillMessage, (StringTableEntryRef victim, StringTableEntryRef killer),
   NetClassGroupGameMask, RPCGuaranteedOrdered, RPCToGhost, 0)
{
   gGameUserInterface.displayMessage(Color(1.0f, 1.0f, 0.8f), 
            "%s zapped %s", killer.getString(), victim.getString());
}

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, c2sVoiceChat, (bool echo, ByteBufferRef voiceBuffer),
   NetClassGroupGameMask, RPCUnguaranteed, RPCToGhostParent, 0)
{
   // in the naive implementation, we just broadcast this to everyone,
   // including the sender...

   GameConnection *source = (GameConnection *) getRPCSourceConnection();
   S32 clientIndex = findClientIndexByConnection(source);
   if(clientIndex != -1)
   {
      RefPtr<NetEvent> event = TNL_RPC_CONSTRUCT_NETEVENT(this, s2cVoiceChat, (mClientList[clientIndex].name, voiceBuffer));
      for(S32 i = 0; i < mClientList.size(); i++)
      {
         if((i != clientIndex || echo) && mClientList[i].clientConnection)
            mClientList[i].clientConnection->postNetEvent(event);
      }
   }
}

TNL_IMPLEMENT_NETOBJECT_RPC(GameType, s2cVoiceChat, (StringTableEntryRef clientName, ByteBufferRef voiceBuffer),
   NetClassGroupGameMask, RPCUnguaranteed, RPCToGhost, 0)
{
   S32 clientIndex = findClientIndexByName(clientName);
   if(clientIndex != -1)
   {
      S16 frame[LPC10_SAMPLES_PER_FRAME];
      int p = 0, decodeLen = 0;
      ByteBufferPtr playBuffer = new ByteBuffer(0);

      for(U32 i = 0; i < voiceBuffer.getBufferSize(); i += p)
      {
         int frameLen = vbr_lpc10_decode((unsigned char *) voiceBuffer.getBuffer() + i, frame, mClientList[clientIndex].decoderState, &p);
         playBuffer->resize((decodeLen + frameLen) * 2);

         memcpy(playBuffer->getBuffer() + decodeLen * 2, frame, frameLen * 2);
         decodeLen += frameLen;
      }

      //logprintf("Decoded buffer size %d", playBuffer->getBufferSize());
      mClientList[clientIndex].voiceSFX->queueBuffer(playBuffer);
   }
}

};