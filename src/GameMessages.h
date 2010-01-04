// $Id: GameMessages.h 5853 2010-01-04 16:14:16Z FloSoft $
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
#ifndef GAMEMESSAGES_H_INCLUDED
#define GAMEMESSAGES_H_INCLUDED

#include "GameMessage.h"
#include "GameMessageInterface.h"
#include "GameProtocol.h"
#include "GamePlayerList.h"
#include "GameCommands.h"
#include "GlobalGameSettings.h"

/* 
 * das Klassenkommentar ist alles Client-Sicht, f�r Server-Sicht ist alles andersrum
 * 
 * Konstruktor ohne Parameter ist allgemein nur zum Empfangen (immer noop!)
 * run-Methode ist Auswertung der Daten
 * 
 * Konstruktor(en) mit Parametern (wenns auch nur der "reserved"-Parameter ist)
 * ist zum Verschicken der Nachrichten gedacht!
 */


/// Castet das allgemeine Message-Interface in ein GameMessage-Interface
inline GameMessageInterface *GetInterface(MessageInterface *callback)
{ 
	return dynamic_cast<GameMessageInterface*>(callback);
}

///////////////////////////////////////////////////////////////////////////////
/// eingehende Ping-Nachricht
class GameMessage_Ping : public GameMessage
{
public:
	GameMessage_Ping(void) : GameMessage(NMS_PING) {}
	GameMessage_Ping(const unsigned char player) : GameMessage(NMS_PING,player) {} 

	void Run(MessageInterface *callback) 
	{ 
		//LOG.write("<<< NMS_PING\n");
		GetInterface(callback)->OnNMSPing(*this);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// ausgehende Pong-Nachricht
class GameMessage_Pong : public GameMessage
{
public:
	GameMessage_Pong(void) : GameMessage(NMS_PONG) { }
	GameMessage_Pong(const unsigned char player) : GameMessage(NMS_PONG, player)
	{
		//LOG.write(">>> NMS_PONG\n");
	}
	void Run(MessageInterface *callback) 
	{ 
		//LOG.write("<<< NMS_PONG\n");
		GetInterface(callback)->OnNMSPong(*this);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// ausgehende Server-Typ-Nachricht
class GameMessage_Server_Type: public GameMessage
{
public:
	unsigned short type;
	std::string version;

public:
	GameMessage_Server_Type(void) : GameMessage(NMS_SERVER_TYPE) { }
	GameMessage_Server_Type(const unsigned short type,
		const std::string &version) : GameMessage(NMS_SERVER_TYPE,0xFF)
	{
		LOG.write(">>> NMS_SERVER_Type(%d, %s)\n", type, version.c_str());
		
		PushUnsignedShort(type);
		PushString(version);
	}
	GameMessage_Server_Type(const unsigned short &type)
		: GameMessage(NMS_SERVER_TYPE,0xFF)
	{
		LOG.write(">>> NMS_SERVER_Type(%s)\n", (type == 1 ? "true" : "false"));
		PushUnsignedShort(type);
	}
	void Run(MessageInterface *callback) 
	{ 
		type = PopUnsignedShort();

		if(GetLength() > sizeof(unsigned short))
		{
			version = PopString();

			LOG.write("<<< NMS_SERVER_Type(%d, %s)\n", type, version.c_str());
			GetInterface(callback)->OnNMSServerType(*this);
		}
		else
		{
			LOG.write("<<< NMS_SERVER_Type(%s)\n", (type == 1 ? "true" : "false"));
			GetInterface(callback)->OnNMSServerType(*this);
		}
	}
};


class GameMessage_Server_TypeOK: public GameMessage
{
public:
	/// Vom Server akzeptiert?
	unsigned err_code;

public:
	GameMessage_Server_TypeOK(void) : GameMessage(NMS_SERVER_TYPEOK) { }
	GameMessage_Server_TypeOK(const unsigned err_code) 
		: GameMessage(NMS_SERVER_TYPEOK,0xFF), err_code(err_code)
	{
		PushUnsignedInt(err_code);
	}

	void Run(MessageInterface *callback) 
	{ 
		err_code = PopUnsignedInt();
		GetInterface(callback)->OnNMSServerTypeOK(*this);
	}
};


///////////////////////////////////////////////////////////////////////////////
/// ein/ausgehende Server-Password-Nachricht
class GameMessage_Server_Password : public GameMessage
{
public:
	std::string password;

public:
	GameMessage_Server_Password(void) : GameMessage(NMS_SERVER_PASSWORD) { }
	GameMessage_Server_Password(const std::string &password) 
		: GameMessage(NMS_SERVER_PASSWORD,0xFF)
	{
		LOG.write(">>> NMS_SERVER_PASSWORD(%s)\n", "********");

		PushString(password);
	}
	void Run(MessageInterface *callback) 
	{ 
		password = PopString();

		LOG.write("<<< NMS_SERVER_PASSWORD(%s)\n", "********");
		GetInterface(callback)->OnNMSServerPassword(*this);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// ausgehende Server-Name-Nachricht
class GameMessage_Server_Name : public GameMessage
{
public:
	std::string name;

public:
	GameMessage_Server_Name(void) : GameMessage(NMS_SERVER_NAME) { }
	GameMessage_Server_Name(const std::string &name)
		: GameMessage(NMS_SERVER_NAME,0xFF)
	{
		LOG.write(">>> NMS_SERVER_NAME(%s)\n", name.c_str());

		PushString(name);
	}
	void Run(MessageInterface *callback) 
	{ 
		name = PopString();

		LOG.write("<<< NMS_SERVER_NAME(%s)\n", name.c_str());
		GetInterface(callback)->OnNMSServerName(*this);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// eingehende Server-Start-Nachricht
class GameMessage_Server_Start : public GameMessage
{
public:
	unsigned int random_init;
	unsigned int nwf_length;

public:
	GameMessage_Server_Start(void) : GameMessage(NMS_SERVER_START) { }
	GameMessage_Server_Start(const unsigned random_init,
		const unsigned nwf_length) : GameMessage(NMS_SERVER_START,0xFF)
	{
		LOG.write(">>> NMS_SERVER_START(%d, %d)\n", random_init, nwf_length);
	
		PushUnsignedInt(random_init);
		PushUnsignedInt(nwf_length);
	}
	void Run(MessageInterface *callback) 
	{ 
		random_init = PopUnsignedInt();
		nwf_length = PopUnsignedInt();

		LOG.write("<<< NMS_SERVER_START(%d, %d)\n", random_init, nwf_length);
		GetInterface(callback)->OnNMSServerStart(*this);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// ein/ausgehende Server-Chat-Nachricht
class GameMessage_Server_Chat : public GameMessage
{
public:
	ChatDestination destination;
	std::string text;

public:
	GameMessage_Server_Chat(void) : GameMessage(NMS_SERVER_CHAT) { }
	GameMessage_Server_Chat(const unsigned char player, 
		const ChatDestination destination, const std::string &text) : GameMessage(NMS_SERVER_CHAT, player)
	{
		LOG.write(">>> NMS_SERVER_CHAT(%d, %s)\n", destination, text.c_str());

		PushUnsignedChar(static_cast<unsigned char>(destination));
		PushString(text);
	}

	void Run(MessageInterface *callback) 
	{ 
		destination = ChatDestination(PopUnsignedChar());
		text = PopString();

		LOG.write("<<< NMS_SERVER_CHAT(%d, %s)\n", destination, text.c_str());
		GetInterface(callback)->OnNMSServerChat(*this);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// eingehende Server-Async-Nachricht
class GameMessage_Server_Async : public GameMessage
{
public:
	std::vector<int> checksums;

public:
	GameMessage_Server_Async(void) : GameMessage(NMS_SERVER_ASYNC) { }
	GameMessage_Server_Async(const std::vector<int> &checksums) 
		: GameMessage(NMS_SERVER_ASYNC,0xFF)
	{
		LOG.write(">>> NMS_SERVER_ASYNC(%d)\n", checksums.size());

		PushUnsignedInt(unsigned(checksums.size()));
		for(unsigned int i = 0; i < checksums.size(); ++i)
			PushSignedInt(checksums.at(i));
	}
	void Run(MessageInterface *callback) 
	{ 
		unsigned size = PopUnsignedInt();
		checksums.resize(size);
		for(unsigned int i = 0; i < size; ++i)
			checksums[i] = PopSignedInt();

		LOG.write("<<< NMS_SERVER_ASYNC(%d)\n", checksums.size());
		GetInterface(callback)->OnNMSServerAsync(*this);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// eingehende Player-ID-Nachricht
class GameMessage_Player_Id : public GameMessage
{
public:
	unsigned int playerid;

public:
	GameMessage_Player_Id(void) : GameMessage(NMS_PLAYER_ID) { }
	GameMessage_Player_Id(const unsigned int playerid) 
		: GameMessage(NMS_PLAYER_ID,0xFF)
	{
		LOG.write(">>> NMS_PLAYER_ID(%d)\n", playerid);

		PushUnsignedInt(playerid);
	}
	void Run(MessageInterface *callback) 
	{ 
		playerid = PopUnsignedInt();

		LOG.write("<<< NMS_PLAYER_ID(%d)\n", playerid);
		GetInterface(callback)->OnNMSPlayerId(*this);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// ausgehende Player-Name-Nachricht
class GameMessage_Player_Name : public GameMessage
{
public:
	/// Name des neuen Spielers
	std::string playername;

public:
	GameMessage_Player_Name(void) : GameMessage(NMS_PLAYER_NAME) { }
	GameMessage_Player_Name(const std::string &playername)
		: GameMessage(NMS_PLAYER_NAME,0xFF)
	{
		LOG.write(">>> NMS_PLAYER_NAME(%s)\n", playername.c_str());

		PushString(playername);
	}
	void Run(MessageInterface *callback) 
	{ 
		playername = PopString();

		LOG.write("<<< NMS_PLAYER_NAME(%s)\n", playername.c_str());
		GetInterface(callback)->OnNMSPlayerName(*this);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// eingehende Player-List-Nachricht
class GameMessage_Player_List : public GameMessage
{
public:
	GameServerPlayerList gpl;

public:
	GameMessage_Player_List(void) : GameMessage(NMS_PLAYER_LIST) { }
	GameMessage_Player_List(const GameServerPlayerList &gpl) 
		: GameMessage(NMS_PLAYER_LIST,0xFF)
	{
		LOG.write(">>> NMS_PLAYER_LIST(%d)\n", gpl.getCount());

		gpl.serialize(this);
	}
	void Run(MessageInterface *callback) 
	{ 
		gpl.deserialize(this);

		LOG.write("<<< NMS_PLAYER_LIST(%d)\n", gpl.getCount());
		for(unsigned int i = 0; i < gpl.getCount(); ++i)
		{
			const GamePlayerInfo *player = gpl.getElement(i);
			LOG.write("    %d: %s %d %d %d %d %d %d %s\n", i, player->name.c_str(), player->ps, player->rating, player->ping, player->nation, player->color, player->team, (player->ready ? "true" : "false") );
		}
		GetInterface(callback)->OnNMSPlayerList(*this);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// gehende -Nachricht
class GameMessage_Player_Toggle_State : public GameMessage
{
public:
	GameMessage_Player_Toggle_State(void) : GameMessage(NMS_PLAYER_TOGGLESTATE) { }
	GameMessage_Player_Toggle_State(const unsigned char player) 
		: GameMessage(NMS_PLAYER_TOGGLESTATE, player)
	{
		LOG.write(">>> NMS_PLAYER_TOGGLESTATE(%d)\n", player);
	}
	void Run(MessageInterface *callback) 
	{
		LOG.write("<<< NMS_PLAYER_TOGGLESTATE(%d)\n", player);
		GetInterface(callback)->OnNMSPlayerToggleState(*this);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// gehende -Nachricht
class GameMessage_Player_Toggle_Nation : public GameMessage
{
public:
	/// Das zu setzende Volk
	Nation nation;

public:
	GameMessage_Player_Toggle_Nation(void) : GameMessage(NMS_PLAYER_TOGGLENATION) { }
	GameMessage_Player_Toggle_Nation(const unsigned char player, const Nation nation)
		: GameMessage(NMS_PLAYER_TOGGLENATION, player), nation(nation)
	{
		PushUnsignedChar(static_cast<unsigned char>(nation));
		LOG.write(">>> NMS_PLAYER_TOGGLENATION\n");
	}
	void Run(MessageInterface *callback) 
	{ 
		nation = Nation(PopUnsignedChar());

		LOG.write("<<< NMS_PLAYER_TOGGLENATION\n");
		GetInterface(callback)->OnNMSPlayerToggleNation(*this);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// gehende Player-Toggle-Team-Nachricht
class GameMessage_Player_Toggle_Team : public GameMessage
{
public:
	/// Das zu setzende Team
	Team team;
public:
	GameMessage_Player_Toggle_Team(void) : GameMessage(NMS_PLAYER_TOGGLETEAM) { }
	GameMessage_Player_Toggle_Team(const unsigned char player, const Team team) 
		: GameMessage(NMS_PLAYER_TOGGLETEAM, player), team(team)
	{
		PushUnsignedChar(static_cast<unsigned char>(team));
		LOG.write(">>> NMS_PLAYER_TOGGLETEAM\n");
	}
	void Run(MessageInterface *callback) 
	{ 
		team = Team(PopUnsignedChar());

		LOG.write("<<< NMS_PLAYER_TOGGLETEAM\n");
		GetInterface(callback)->OnNMSPlayerToggleTeam(*this);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// gehende Player-Toggle-Color-Nachricht
class GameMessage_Player_Toggle_Color : public GameMessage
{
public:
	/// Das zu setzende Team
	unsigned char color;
public:
	GameMessage_Player_Toggle_Color(void) : GameMessage(NMS_PLAYER_TOGGLECOLOR) { }
	GameMessage_Player_Toggle_Color(const unsigned char player, const unsigned char color) 
		: GameMessage(NMS_PLAYER_TOGGLECOLOR, player), color(color)
	{
		PushUnsignedChar(color);

		LOG.write(">>> NMS_PLAYER_TOGGLECOLOR\n");
	}
	void Run(MessageInterface *callback) 
	{
		color = PopUnsignedChar();

		LOG.write("<<< NMS_PLAYER_TOGGLECOLOR\n");
		GetInterface(callback)->OnNMSPlayerToggleColor(*this);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// gehende Player-Kicked-Nachricht
class GameMessage_Player_Kicked : public GameMessage
{
public:
	unsigned char cause;
	unsigned short param;

public:
	GameMessage_Player_Kicked(void) : GameMessage(NMS_PLAYER_KICKED) { }
	GameMessage_Player_Kicked(const unsigned char player, const unsigned char cause, const unsigned short param) 
		: GameMessage(NMS_PLAYER_KICKED, player),
		 cause(cause), param(param)
	{
		PushUnsignedChar(cause);
		PushUnsignedShort(param);
	}

	void Run(MessageInterface *callback) 
	{ 
		cause = PopUnsignedChar();
		param = PopUnsignedShort();

		LOG.write("<<< NMS_PLAYER_KICKED\n");
		GetInterface(callback)->OnNMSPlayerKicked(*this);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// gehende Player-Ping-Nachricht
class GameMessage_Player_Ping : public GameMessage
{
public:
	unsigned short ping;
public:
	GameMessage_Player_Ping(void) : GameMessage(NMS_PLAYER_PING) { }
	GameMessage_Player_Ping(const unsigned char player, const unsigned short ping)
		: GameMessage(NMS_PLAYER_PING, player), ping(ping)
	{
		PushUnsignedShort(ping);
		LOG.write(">>> NMS_PLAYER_PING\n");
	}
	void Run(MessageInterface *callback) 
	{ 
		ping = PopUnsignedShort();
		LOG.write("<<< NMS_PLAYER_PING\n");
		GetInterface(callback)->OnNMSPlayerPing(*this);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// gehende Player-New-Nachricht
class GameMessage_Player_New : public GameMessage
{
public:
	/// Name des Neuen Spielers
	std::string name;
public:
	GameMessage_Player_New(void) : GameMessage(NMS_PLAYER_NEW) { }
	GameMessage_Player_New(const unsigned char player, const std::string& name)
		: GameMessage(NMS_PLAYER_NEW, player)
	{
		PushString(name);
		LOG.write(">>> NMS_PLAYER_NEW\n");
	}
	void Run(MessageInterface *callback) 
	{ 
		name = PopString();

		LOG.write("<<< NMS_PLAYER_NEW\n");
		GetInterface(callback)->OnNMSPlayerNew(*this);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// gehende Player-Ready-Nachricht
class GameMessage_Player_Ready : public GameMessage
{
public:
	/// Ist der Spieler bereit?
	bool ready;
public:
	GameMessage_Player_Ready(void) : GameMessage(NMS_PLAYER_READY) { }
	GameMessage_Player_Ready(const unsigned char player, const bool ready)
		: GameMessage(NMS_PLAYER_READY, player), ready(ready)
	{
		PushBool(ready);
		LOG.write(">>> NMS_PLAYER_READY\n");
	}
	void Run(MessageInterface *callback) 
	{ 
		ready = PopBool();
		LOG.write("<<< NMS_PLAYER_READY\n");
		GetInterface(callback)->OnNMSPlayerReady(*this);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// gehende Player-Swap-Nachricht
class GameMessage_Player_Swap : public GameMessage
{
public:
	/// Die beiden Spieler-IDs, die miteinander vertauscht werden sollen
	unsigned char player2;
public:
	GameMessage_Player_Swap(void) : GameMessage(NMS_PLAYER_SWAP) { }
	GameMessage_Player_Swap(const unsigned char player, const unsigned char player2) 
		: GameMessage(NMS_PLAYER_SWAP, player),  player2(player2)
	{
		PushUnsignedChar(player2);

		LOG.write(">>> NMS_PLAYER_SWAP\n");
	}
	void Run(MessageInterface *callback) 
	{ 
		player2 = PopUnsignedChar();

		LOG.write("<<< NMS_PLAYER_SWAP\n");
		GetInterface(callback)->OnNMSPlayerSwap(*this);
	}
};

class GameMessage_Map_Info : public GameMessage
{
public:
	/// Name der Karte
	std::string map_name;
	/// Kartentyp (alte Karte neue Karte, Savegame usw.)
	MapType mt;
	/// Anzahl der Teile, in die der Mapblock zerteilt wurde
	unsigned partcount;
	/// Gr��e der Zip-komprimierten Date
	unsigned ziplength;
	/// Gr��e der dekomprimierten Daten
	unsigned normal_length;

public:
	GameMessage_Map_Info(void) : GameMessage(NMS_MAP_INFO) { }
	GameMessage_Map_Info(const std::string& map_name, const MapType mt, const unsigned partcount,
		const unsigned ziplength, const unsigned normal_length) 
		: GameMessage(NMS_MAP_INFO,0xFF), map_name(map_name),  mt(mt), partcount(partcount), ziplength(ziplength),
		normal_length(normal_length)
	{
		PushString(map_name);
		PushUnsignedChar(static_cast<unsigned char>(mt));
		PushUnsignedInt(partcount);
		PushUnsignedInt(ziplength);
		PushUnsignedInt(normal_length);

		LOG.write(">>> NMS_PLAYER_SWAP\n");
	}
	void Run(MessageInterface *callback) 
	{ 
		map_name = PopString();
		mt = MapType(PopUnsignedChar());
		partcount = PopUnsignedInt();
		ziplength = PopUnsignedInt();
		normal_length = PopUnsignedInt();

		LOG.write("<<< NMS_PLAYER_SWAP\n");
		GetInterface(callback)->OnNMSMapInfo(*this);
	}
};

class GameMessage_Map_Data : public GameMessage
{
public:
	/// Kartendaten
	unsigned char * map_data;

public:
	GameMessage_Map_Data(void) : GameMessage(NMS_MAP_DATA) { }
	GameMessage_Map_Data(const unsigned char * const map_data, const unsigned length) 
		: GameMessage(NMS_MAP_DATA,0xFF)
	{
		PushRawData(map_data,length);

		LOG.write(">>> NMS_MAP_DATA\n");
	}
	void Run(MessageInterface *callback) 
	{ 
		map_data = GetDataWritable() + (GetLength()-GetNetLength());

		LOG.write("<<< NMS_MAP_DATA\n");
		GetInterface(callback)->OnNMSMapData(*this);
	}
};

class GameMessage_Map_Checksum : public GameMessage
{
public:
	/// Checksumme, die vom Client berechnt wurde
	unsigned checksum;

public:
	GameMessage_Map_Checksum(void) : GameMessage(NMS_MAP_CHECKSUM) { }
	GameMessage_Map_Checksum(const unsigned checksum) 
		: GameMessage(NMS_MAP_CHECKSUM,0xFF)
	{
		PushUnsignedInt(checksum);

		LOG.write(">>> NMS_MAP_DATA\n");
	}
	void Run(MessageInterface *callback) 
	{ 
		checksum = PopUnsignedInt();
		LOG.write("<<< NMS_MAP_DATA\n");
		GetInterface(callback)->OnNMSMapChecksum(*this);
	}
};

/// MapChecksum vom Server erfolgreich verifiziert?
class GameMessage_Map_ChecksumOK: public GameMessage
{
public:
	/// Vom Server akzeptiert?
	bool correct;

public:
	GameMessage_Map_ChecksumOK(void) : GameMessage(NMS_MAP_CHECKSUMOK) { }
	GameMessage_Map_ChecksumOK(const bool correct) 
		: GameMessage(NMS_MAP_CHECKSUMOK,0xFF), correct(correct)
	{
		PushBool(correct);
	}

	void Run(MessageInterface *callback) 
	{ 
		correct = PopBool();
		GetInterface(callback)->OnNMSMapChecksumOK(*this);
	}
};


class GameMessage_GGSChange : public GameMessage
{
public:
	/// GGS-Daten
	GlobalGameSettings ggs;

public:
	GameMessage_GGSChange(void) : GameMessage(NMS_GGS_CHANGE) { }
	GameMessage_GGSChange(const GlobalGameSettings& ggs) 
		: GameMessage(NMS_GGS_CHANGE, player)
	{
		ggs.Serialize(this);

		LOG.write(">>> NMS_GGS_CHANGE\n");
	}
	void Run(MessageInterface *callback) 
	{ 
		ggs.Deserialize(this);

		LOG.write("<<< NMS_GGS_CHANGE\n");
		GetInterface(callback)->OnNMSGGSChange(*this);
	}
};


class GameMessage_GameCommand : public GameMessage
{
public:
	/// Checksumme, die der Spieler �bermittelt
	unsigned checksum;
	/// Die einzelnen GameCommands
	std::vector<gc::GameCommand*> gcs;

public:

	GameMessage_GameCommand(void) : GameMessage(NMS_GAMECOMMANDS) { }
	GameMessage_GameCommand(const unsigned char player, const unsigned checksum, 
		const std::vector<gc::GameCommand*>& gcs) 
		: GameMessage(NMS_GAMECOMMANDS, player)
	{
		PushUnsignedInt(checksum);
		PushUnsignedInt(gcs.size());

		for(unsigned i = 0;i<gcs.size();++i)
		{
			PushUnsignedChar(gcs[i]->GetType());
			gcs[i]->Serialize(this);
		}

	}

	GameMessage_GameCommand(const unsigned char * const data, const unsigned length) 
		: GameMessage(NMS_GAMECOMMANDS,data,length),
			checksum(PopUnsignedInt()),
			gcs(PopUnsignedInt())
	{
		for(unsigned i = 0;i<gcs.size();++i)
		{
			gc::Type type = gc::Type(PopUnsignedChar());
			gcs[i] = gc::GameCommand::CreateGameCommand(type,this);
		}

	}

	void Run(MessageInterface *callback) 
	{ 
		checksum = PopUnsignedInt();
		gcs.resize(PopUnsignedInt());
		for(unsigned i = 0;i<gcs.size();++i)
		{
			gc::Type type = gc::Type(PopUnsignedChar());
			gcs[i] = gc::GameCommand::CreateGameCommand(type,this);
		}

		GetInterface(callback)->OnNMSGameCommand(*this);
	}
};


class GameMessage_Server_NWFDone : public GameMessage
{
public:
	GameMessage_Server_NWFDone(void) : GameMessage(NMS_SERVER_NWF_DONE) { }
	GameMessage_Server_NWFDone(const unsigned char player) : GameMessage(NMS_SERVER_NWF_DONE, player) { }

	void Run(MessageInterface *callback) 
	{ 
		LOG.write("<<< NMS_GGS_CHANGE\n");
		GetInterface(callback)->OnNMSServerDone(*this);
	}
};


class GameMessage_Pause : public GameMessage
{
public:
	/// Pausiert?
	bool paused;

public:
	GameMessage_Pause(void) : GameMessage(NMS_PAUSE) { }
	GameMessage_Pause(const bool paused) 
		: GameMessage(NMS_PAUSE,0xFF)
	{
		PushBool(paused);

		LOG.write(">>> NMS_GGS_CHANGE\n");
	}
	void Run(MessageInterface *callback) 
	{ 
		paused = PopBool();

		LOG.write("<<< NMS_GGS_CHANGE\n");
		GetInterface(callback)->OnNMSPause(*this);
	}
};

#endif //!GAMEMESSAGES_H_INCLUDED
