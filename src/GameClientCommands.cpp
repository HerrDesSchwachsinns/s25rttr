// $Id: GameClientCommands.cpp 6015 2010-02-13 15:09:58Z FloSoft $
//
// Copyright (c) 2005 - 2010 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Siedler II.5 RTTR.
//
// Siedler II.5 RTTR is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Siedler II.5 RTTR is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Siedler II.5 RTTR. If not, see <http://www.gnu.org/licenses/>.

///////////////////////////////////////////////////////////////////////////////
// Header
#include "main.h"
#include "GameClient.h"

#include "VideoDriverWrapper.h"
#include "noFlag.h"
#include "GameWorld.h"
#include "GameClientPlayer.h"

#include "GameServer.h"
#include "nobUsual.h"
#include "nobMilitary.h"
#include "nobBaseWarehouse.h"
#include "dskGameInterface.h"
#include "ClientInterface.h"
#include "GameCommands.h"
#include "GameMessages.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif




///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
void GameClient::Command_SetFlag2(int x, int y, unsigned char player)
{
	gw->SetFlag(x, y, player);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Chatbefehl, h�ngt eine Textnachricht in die Sende-Queue.
 *
 *  @param[in] text        Der Text
 *  @param[in] destination Ziel der Nachricht
 *
 *  @author FloSoft
 */
void GameClient::Command_Chat(const std::string& text, const ChatDestination cd)
{
	// Replaymodus oder kein Text --> nichts senden
	if(GameClient::inst().IsReplayModeOn() || text.length() == 0)
		return;

	send_queue.push(new GameMessage_Server_Chat(playerid,cd,text));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author FloSoft
 */
void GameClient::Command_ToggleNation()
{
	send_queue.push(new GameMessage_Player_Toggle_Nation
		(0xff,Nation((this->GetLocalPlayer()->nation+1)%NATION_COUNT)));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author FloSoft
 */
void GameClient::Command_ToggleTeam()
{
	send_queue.push(new GameMessage_Player_Toggle_Team(0xff,Team((this->GetLocalPlayer()->team+1)%TEAM_COUNT)));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  sendet den "Bereit"-Status.
 *
 *  @author FloSoft
 */
void GameClient::Command_ToggleReady()
{
	send_queue.push(new GameMessage_Player_Ready(0xFF, GetLocalPlayer()->ready ));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author FloSoft
 */
void GameClient::Command_ToggleColor()
{
	send_queue.push(new GameMessage_Player_Toggle_Color(0xFF,0xFF));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  wechselt einen Spieler.
 *
 *  @param[in] old_id Alte Spieler-ID
 *  @param[in] new_id Neue Spieler-ID
 *
 *  @author OLiver
 *  @author FloSoft
 */
void GameClient::ChangePlayer(const unsigned char old_id, const unsigned char new_id)
{
	// ID auch innerhalb der Spielerzahl?
	if(new_id >= players.getCount())
		return;

	// Gleiche ID - w�re unsinnig zu wechseln
	if(old_id == new_id)
		return;

	// old_id muss richtiger Spieler, new_id KI sein, ansonsten geht das nat�rlich nicht
	if( !(players[old_id].ps == PS_OCCUPIED && players[new_id].ps == PS_KI) )
		return;

	players[old_id].ps = PS_KI;
	players[new_id].ps = PS_OCCUPIED;

	// Wenn wir betroffen waren, unsere ID neu setzen und BQ neu berechnen
	if(playerid == old_id)
	{
		playerid = new_id;

		// BQ �berall neu berechnen
		for(unsigned y = 0; y < gw->GetHeight(); ++y)
		{
			for(unsigned x = 0; x < gw->GetWidth(); ++x)
				gw->SetBQ(x, y, new_id);
		}

		// Visuelle Einstellungen vom Spieler wieder holen
		GetVisualSettings();

		//// zum HQ hinscrollen
		//gw->MoveToMapObject(player->hqx,player->hqy);
		//GameClientPlayer *player = players[playerid]; // wegen GCC-Fehlermeldung auskommentiert
	}

	// GUI Bescheid sagen (um z.B. Schatten neu zu berechnen)
	if(ci)
		ci->CI_PlayersSwapped(old_id,new_id);
}

void GameClient::ChangeReplayPlayer(const unsigned new_id)
{
	unsigned old_id = playerid;

	if(old_id == new_id)
		// Unsinn auf den selben Spieler zu wechseln
		return;
	// Auch innerhalb der g�ltigen Spieler?
	if(new_id >= GameClient::inst().GetPlayerCount())
		return;
	// Und ein richtiger ehemaliger Spieler?
	if(GameClient::inst().GetPlayer(new_id)->ps != PS_KI &&
		GameClient::inst().GetPlayer(new_id)->ps != PS_OCCUPIED)
		return;


	playerid = new_id;

	// BQ �berall neu berechnen
	for(unsigned y = 0; y < gw->GetHeight(); ++y)
	{
		for(unsigned x = 0; x < gw->GetWidth(); ++x)
			gw->SetBQ(x, y, new_id);
	}

	// GUI Bescheid sagen (um z.B. Schatten neu zu berechnen)
	if(ci)
		ci->CI_PlayersSwapped(old_id,new_id);
}

