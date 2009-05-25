// $Id: GameServer.h 4951 2009-05-25 20:03:10Z OLiver $
//
// Copyright (c) 2005-2009 Settlers Freaks (sf-team at siedler25.org)
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

#ifndef GAMESERVER_H_
#define GAMESERVER_H_

#pragma once

#include "Singleton.h"
#include "Socket.h"

#include "GameMessageInterface.h"

#include "GlobalGameSettings.h"
#include "GamePlayerList.h"


class GameServerPlayer;
class GlobalGameSettings;
struct CreateServerInfo;
class GameMessage;
class AIBase;

class GameServer : public Singleton<GameServer>, public GameMessageInterface
{
public:
	GameServer(void);
	~GameServer(void);

	/// "Versucht" den Server zu starten (muss ggf. erst um Erlaubnis beim LobbyClient fragen)
	bool TryToStart(const CreateServerInfo& csi, const std::string& map_path, const MapType map_type);
	/// Startet den Server, muss vorher TryToStart aufgerufen werden!
	bool Start();

	void Run(void);
	void Stop(void);

	bool StartGame(void);

	bool TogglePause();

	void TogglePlayerNation(unsigned char client);
	void TogglePlayerTeam(unsigned char client);
	void TogglePlayerColor(unsigned char client);
	void TogglePlayerState(unsigned char client);
	void ChangeGlobalGameSettings(const GlobalGameSettings& ggs);

	/// L�sst einen Spieler wechseln (nur zu Debugzwecken)
	void ChangePlayer(const unsigned char old_id, const unsigned char new_id);

	/// Tauscht Spieler(positionen) bei Savegames in dskHostGame
	void SwapPlayer(const unsigned char player1, const unsigned char player2);

protected:

	void SendToAll(const GameMessage& msg);
	void KickPlayer(unsigned char playerid, unsigned char cause, unsigned short param);
	void KickPlayer(NS_PlayerKicked npk);

	void ClientWatchDog(void);
	void WaitForClients(void);
	void FillPlayerQueues(void);

	/// Sendet ein NC-Paket ohne Befehle
	void SendNothingNC(const unsigned int &id);

	/// Generiert einen KI-Namen
	void SetAIName(const unsigned player_id);

private:
	void OnNMSPong(const GameMessage_Pong& msg);
	void OnNMSServerType(const GameMessage_Server_Type& msg);
	void OnNMSServerPassword(const GameMessage_Server_Password& msg);
	void OnNMSServerChat(const GameMessage_Server_Chat& msg);
	void OnNMSPlayerName(const GameMessage_Player_Name& msg);
	void OnNMSPlayerToggleNation(const GameMessage_Player_Toggle_Nation& msg);
	void OnNMSPlayerToggleTeam(const GameMessage_Player_Toggle_Team& msg);
	void OnNMSPlayerToggleColor(const GameMessage_Player_Toggle_Color& msg);
	void OnNMSPlayerReady(const GameMessage_Player_Ready& msg);
	void OnNMSMapChecksum(const GameMessage_Map_Checksum& msg);
	void OnNMSGameCommand(const GameMessage_GameCommand& msg);


private:
	enum ServerState
	{
		SS_STOPPED = 0,
		SS_CONFIG,
		SS_GAME
	} status;

	class FramesInfo
	{
	public:

		FramesInfo();
		void Clear();

		/// Aktueller nwf
		unsigned int nr;
		/// L�nge der Network-Frames in ms (gf-L�nge * nwf_length des Clients)
		unsigned int nwf_length;
		/// Aktueller GF
		unsigned gf_nr;
		/// GF-L�nge in ms
		unsigned gf_length;

		unsigned int lasttime;
		unsigned int lastmsgtime;
		unsigned int pausetime;
		bool pause;
	} framesinfo;

	class ServerConfig
	{
	public:

		ServerConfig();
		void Clear();

		unsigned char servertype;
		unsigned char playercount;
		std::string gamename;
		std::string password;
		std::string mapname;
		unsigned short port;
	} serverconfig;

	class MapInfo
	{
	public:

		MapInfo();
		void Clear();

		unsigned int partcount;
		unsigned int ziplength;
		unsigned int length;
		unsigned int checksum;
		std::string name;
		unsigned char *zipdata;
		MapType map_type;
	} mapinfo;

	Socket serversocket;
	GameServerPlayerList players;
	GlobalGameSettings ggs;

	/// Alle KI-Spieler und ihre Daten (NULL, falls ein solcher Spieler nicht existiert)
	std::vector<AIBase*> ai_players;

};

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#define GAMESERVER GameServer::inst()

#endif
