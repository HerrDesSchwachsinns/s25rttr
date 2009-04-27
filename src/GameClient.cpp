// $Id: GameClient.cpp 4652 2009-03-29 10:10:02Z FloSoft $
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

///////////////////////////////////////////////////////////////////////////////
// Header
#include "main.h"
#include "GameClient.h"

#include "SocketSet.h"
#include "Loader.h"
#include "Settings.h"
#include "FileChecksum.h"
#include "VideoDriverWrapper.h"
#include "WindowManager.h"
#include "dskGameInterface.h"
#include "Random.h"
#include "GameServer.h"
#include "GameWorld.h"
#include "EventManager.h"
#include "GameObject.h"
#include "files.h"
#include "GlobalGameSettings.h"

#include "SerializedGameData.h"
#include "LobbyClient.h"
#include "GameFiles.h"
#include "Settings.h"
#include "files.h"
#include "ClientInterface.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

void GameClient::ClientConfig::Clear()
{
	server = "";
	gamename = "";
	password = "";
	mapfile = "";
	mapfilepath = "";
	servertyp = 0;
	port = 0;
	host = false;
}
void GameClient::MapInfo::Clear()
{
	map_type = MAPTYPE_OLDMAP;
	partcount = 0;
	ziplength = 0;
	length = 0;
	checksum = 0;
	title = "";
	delete [] zipdata;
	zipdata = 0;
}
void GameClient::FramesInfo::Clear()
{
	nr = 0;
	gf_length = 0;
	nwf_length = 0;
	frame_time = 0;
	lasttime = 0;
	lastmsgtime = 0;
	pausetime = 0;
	pause = false;
}
void GameClient::RandCheckInfo::Clear()
{
	rand = 0;
	last_rand = 0;
}
void GameClient::ReplayInfo::Clear()
{
	async = false;
	end = false;
	next_gf = 0;
	filename = "";
	all_visible = false;
}


///////////////////////////////////////////////////////////////////////////////
/// Konstruktor
GameClient::GameClient(void)
{
	ci = NULL;
	players = 0;

	clientconfig.Clear();
	framesinfo.Clear();
	randcheckinfo.Clear();
}

///////////////////////////////////////////////////////////////////////////////
/// Destruktor
GameClient::~GameClient(void)
{
	Stop();
	ExitGame();
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Verbindet den Client mit einem Server
 *
 *  @param server    Hostname des Zielrechners
 *  @param password  Passwort des Spieles
 *  @param servertyp Servertyp des Spieles (Direct/LAN/usw)
 *  @param host      gibt an ob wir selbst der Host sind
 *
 *  @return true, wenn Client erfolgreich verbunden und gestartet
 *
 *  @author OLiver
 *  @author FloSoft
 */
bool GameClient::Connect(const std::string& server, const std::string& password, unsigned char servertyp, unsigned short port, bool host)
{
	Stop();

	// Name und Password kopieren
	clientconfig.server = server;
	clientconfig.password = password;

	clientconfig.servertyp = servertyp;
	clientconfig.port = port;
	clientconfig.host = host;

	// Verbinden
	if(!so.Connect(server.c_str(), port))
	{
		puts("GameClient::Connect: ERROR: Connect failed!\n");
		return false;
	}

	state = CS_CONNECT;

	if(ci)
		ci->CI_NextConnectState(CS_WAITFORANSWER);

	// Es wird kein Replay abgespielt, sondern dies ist ein richtiges Spiel
	replay_mode = false;

	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// Hauptschleife des Clients
void GameClient::Run()
{
	if(state == CS_STOPPED)
		return;

	FillPlayerQueues();

	if(state == CS_GAME)
		ExecuteGameFrame();

	unsigned char count = 0;

	// send-queue abarbeiten
	while(send_queue.count() > 0)
	{
		// maximal 10 Pakete verschicken
		if(count > 10)
			break;

		GameMessage *m = send_queue.front();

		// maximal 1 gro�es Paket verschicken
		if(count > 0 && m->m_uiLength > 512)
			break;

		if(m->m_usID > 0)
		{
			if(!m->send(&so))
			{
				LOG.lprintf("Sending Message to server failed\n");
				ServerLost();
			}
		}
		send_queue.pop();
		++count;
	}

	// recv-queue abarbeiten
	while(recv_queue.count() > 0)
	{
		GameMessage *m = recv_queue.front();
		if(m->m_usID > 0)
		{
			if(state == CS_GAME)
				HandleGameMessage(m);
			else
				HandleMessage(m);
		}
		recv_queue.pop();
	}
}

///////////////////////////////////////////////////////////////////////////////
/// Stopfunktion
void GameClient::Stop()
{
	if(state != CS_STOPPED)
	{
		if(LOBBYCLIENT.LoggedIn()) // steht die Lobbyverbindung noch?
			LOBBYCLIENT.DeleteServer();

		LOG.lprintf("client state changed to stop\n");
	}

	// Nicht im Spiel --> Spieler l�schen
	// (im Spiel wird das dann von ExitGame �bernommen, da die Spielerdaten evtl noch f�r Statis-
	// tiken usw. ben�tigt werden
	if(state != CS_GAME)
	{
		delete [] players;
		players = 0;
	}

	state = CS_STOPPED;


	clientconfig.Clear();
	mapinfo.Clear();

	replayinfo.replay.StopRecording();

	// NFC-Queues aufr�umen
	nfc_queue.clear();

	so.Close();


}

///////////////////////////////////////////////////////////////////////////////
/// f�llt die Warteschlangen mit "Paketen"
void GameClient::FillPlayerQueues(void)
{
	SocketSet set;

	// erstmal auf Daten �berpr�fen
	set.Clear();

	// zum set hinzuf�gen
	set.Add(so);
	if(set.Select(0, 0) > 0)
	{
		// nachricht empfangen
		if(recv_queue.recv(&so) == -1)
		{
			LOG.lprintf("Receiving Message from server failed\n");
			ServerLost();
		}
	}

	// nun auf Fehler pr�fen
	set.Clear();

	// zum set hinzuf�gen
	set.Add(so);

	// auf fehler pr�fen
	if(set.Select(0, 2) > 0)
	{
		if(set.InSet(so))
		{
			// Server ist weg
			LOG.lprintf("Error on socket to server\n");
			ServerLost();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Startet ein Spiel oder Replay.
 *
 *  @param[in] random_init Initialwert des Zufallsgenerators.
 *
 *  @author OLiver
 */
void GameClient::StartGame(const unsigned int random_init)
{
	state = CS_GAME; // zu gamestate wechseln

	// Daten zur�cksetzen
	randcheckinfo.Clear();

	// framesinfo vorinitialisieren
	// bei gespeicherten Spielen mit einem bestimmten GF nat�rlich beginnen!
	framesinfo.nr = (mapinfo.map_type == MAPTYPE_SAVEGAME) ? mapinfo.savegame.start_gf : 0;
	framesinfo.pause = true;

	// Je nach Geschwindigkeit GF-L�nge einstellen
	framesinfo.gf_length = SPEED_GF_LENGTHS[ggs.game_speed];

	// Random-Generator initialisieren
	RANDOM.Init(random_init);

	// Spielwelt erzeugen
	gw = new GameWorld();
	em = new EventManager();
	GameObject::SetPointers(gw,em,players);
	for(unsigned i = 0;i<player_count;++i)
		players[i]->SetGameWorldPointer(gw);

	if(ci)
		ci->CI_GameStarted(gw);

	Savegame * savegame = (mapinfo.map_type == MAPTYPE_SAVEGAME) ? &mapinfo.savegame : 0;

	if(savegame)
	{
		savegame->sgd.PrepareDeserialization(em);
		gw->Deserialize(&savegame->sgd);
		em->Deserialize(&savegame->sgd);
		for(unsigned i = 0;i<GAMECLIENT.GetPlayerCount();++i)
			GetPlayer(i)->Deserialize(&savegame->sgd);
		// Visuelle Einstellungen ableiten
		GetVisualSettings();
	}
	else
		gw->LoadMap(clientconfig.mapfilepath);

	// Zeit setzen
	framesinfo.lasttime = VideoDriverWrapper::inst().GetTickCount();
	framesinfo.lastmsgtime = framesinfo.lasttime;

	if(!replay_mode)
	{
		WriteReplayHeader(random_init);

		char filename[256], time_str[80];
		unser_time_t temp = TIME.CurrentTime();
		TIME.FormatTime(time_str, "game_%Y-%m-%d_%H-%i-%s", &temp);
		sprintf(filename,"%s%s-%u.log",FILE_PATHS[47], time_str, rand()%100);

		game_log = fopen(filename,"a");
	}

	// Daten nach dem Schreiben des Replays ggf wieder l�schen
	if(mapinfo.zipdata)
	{
		delete [] mapinfo.zipdata;
		replayinfo.replay.map_data = 0;
		mapinfo.zipdata = NULL;
	}
}

void GameClient::RealStart()
{
	framesinfo.pause = false;

	/// Wenn Replay, evtl erstes Command vom Start-Frame auslesen, was sonst ignoriert werden w�rde
	if(replay_mode)
		ExecuteGameFrame_Replay();
}

void GameClient::ExitGame()
{
	// Spielwelt zerst�ren
	delete gw;
	delete em;
	gw = 0;
	em = 0;

	if(players)
	{
		// Player l�schen
		for(unsigned i = 0;i<player_count;++i)
			delete players[i];

		delete[] players;
	}

	players = NULL;
}


///////////////////////////////////////////////////////////////////////////////
/**
 *  f�hrt eine Nachricht aus.
 *
 *  @param[in] message Nachricht, welche ausgef�hrt werden soll
 *
 *  @author FloSoft
 */
void GameClient::HandleMessage(GameMessage *message)
{
	if(!message)
		return;

	switch(message->m_usID)
	{
	default:			break;
	case NMS_NULL_MSG:	break;

	case NMS_PING:	OnNMSPing(message);	break;

	case NMS_PLAYER_ID:				OnNMSPlayerId(message);				break;
	case NMS_PLAYER_LIST:			OnNMSPlayerList(message);			break;
	case NMS_PLAYER_NEW:			OnNMSPlayerNew(message);			break;
	case NMS_PLAYER_PING:			OnNMSPlayerPing(message);			break;
	case NMS_PLAYER_TOGGLESTATE:	OnNMSPlayerToggleState(message);	break;
	case NMS_PLAYER_TOGGLENATION:	OnNMSPlayerToggleNation(message);	break;
	case NMS_PLAYER_TOGGLETEAM:		OnNMSPlayerToggleTeam(message);		break;
	case NMS_PLAYER_TOGGLECOLOR:	OnNMSPlayerToggleColor(message);	break;
	case NMS_PLAYER_READY:			OnNMSPlayerReady(message);			break;
	case NMS_PLAYER_KICKED:			OnNMSPlayerKicked(message);			break;
	case NMS_PLAYER_SWAP:			OnNMSPlayerSwap(message);			break;

	case NMS_SERVER_TYP:		OnNMSServerTyp(message);		break;
	case NMS_SERVER_PASSWORD:	OnNMSServerPassword(message);	break;
	case NMS_SERVER_NAME:		OnNMSServerName(message);		break;
	case NMS_SERVER_CHAT:		OnNMSServerChat(message);		break;
	case NMS_SERVER_START:		OnNMSServerStart(message);		break;

	case NMS_MAP_NAME:		OnNMSMapName(message);		break;
	case NMS_MAP_INFO:		OnNMSMapInfo(message);		break;
	case NMS_MAP_DATA:		OnNMSMapData(message);		break;
	case NMS_MAP_CHECKSUM:	OnNMSMapChecksum(message);	break;

	case NMS_GGS_CHANGE:	OnNMSGGSChange(message);	break;

	case NMS_DEAD_MSG:		OnNMSDeadMsg(message);		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�hrt eine Ingame-Nachricht aus.
 *
 *  @param[in] message Nachricht, welche ausgef�hrt werden soll
 *
 *  @author FloSoft
 */
inline void GameClient::HandleGameMessage(GameMessage *message)
{
	if(!message)
		return;

	switch(message->m_usID)
	{
	default:			break;
	case NMS_NULL_MSG:	break;

	case NMS_PING:	OnNMSPing(message);	break;

	case NMS_PLAYER_KICKED:	OnNMSPlayerKicked(message);	break;
	case NMS_PLAYER_PING:	OnNMSPlayerPing(message);	break;

	case NMS_SERVER_CHAT:	OnNMSServerChat(message);	break;
	case NMS_SERVER_ASYNC:	OnNMSServerAsync(message);	break;

	case NMS_NFC_ANSWER:	OnNMSNfcAnswer(message);	break;
	case NMS_NFC_DONE:		OnNMSNfcDone(message);		break;
	case NMS_NFC_PAUSE:		OnNMSNfcPause(message);     break;

	case NMS_DEAD_MSG:		OnNMSDeadMsg(message);		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// Deadmessage
/// @param message	Nachricht, welche ausgef�hrt wird
inline void GameClient::OnNMSDeadMsg(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength != 0)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}

	LOG.write("<<< NMS_DEAD_MSG\n");

	ServerLost();
}

///////////////////////////////////////////////////////////////////////////////
/// Pingmessage
/// @param message	Nachricht, welche ausgef�hrt wird
inline void GameClient::OnNMSPing(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength != 0)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}

	//LOG.write("<<< NMS_PING\n");

	send_queue.push(NMS_PONG);

	//LOG.write(">>>NMS_PONG\n");
}

///////////////////////////////////////////////////////////////////////////////
/// player id
/// @param message	Nachricht, welche ausgef�hrt wird
inline void GameClient::OnNMSPlayerId(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength < 1)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		ServerLost();
		return;
	}

	playerid = *(unsigned char*)message->m_pData;

	LOG.write("<<< NMS_PLAYER_ID(%d)\n", playerid);

	if(playerid == 0xFF)
	{
		if(ci)
			ci->CI_Error(CE_SERVERFULL);

		Stop();
		return;
	}

	GameMessage *msg = send_queue.push(NMS_SERVER_TYP);
	char *data = (char*)msg->alloc(2 + (unsigned int)strlen(GetWindowVersion()));
	data[0] = clientconfig.servertyp;
	strcpy(&data[1], GetWindowVersion());

	LOG.write(">>>NMS_SERVER_TYP(%d, %s)\n", clientconfig.servertyp, GetWindowVersion());
}

///////////////////////////////////////////////////////////////////////////////
/// player liste
/// @param message	Nachricht, welche ausgef�hrt wird
inline void GameClient::OnNMSPlayerList(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength < 1)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}
	unsigned char *data = (unsigned char*)message->m_pData;
	unsigned char *count = (unsigned char*)&data[0];
	//GamePlayerInfo *list = (GamePlayerInfo*)&data[1];

	player_count = *count;

	LOG.write("<<< NMS_PLAYER_LIST(%d)\n", *count);


	//// Speicherlecks vermeiden!
	assert(!players);
	//if(players)
	//	delete[] players;

	players = NULL;

	if(player_count > 0)
	{
		players = new GameClientPlayer*[player_count];
		for(unsigned char i = 0; i < player_count; ++i)
			players[i] = new GameClientPlayer(i);
	}


	int j = 1;
	for(unsigned char i = 0; i < player_count; ++i)
	{
		unsigned char length_name,length_origin_name;
		length_name = data[j];
		length_origin_name = data[j+1];

		j+=2;

		players[i]->name = (char*)(&data[j]);
		j+=length_name;
		players[i]->origin_name = (char*)(&data[j]);
		j+=length_origin_name;

		memcpy(&players[i]->ps, &data[j], 4);
		memcpy(&players[i]->rating, &data[j+4], 2);
		players[i]->is_host = (data[j+6] == 1) ? true : false;
		memcpy(&players[i]->ping, &data[j+7], 2);
		memcpy(&players[i]->nation, &data[j+9], 1);
		memcpy(&players[i]->color, &data[j+10], 1);
		memcpy(&players[i]->team, &data[j+11], 1);
		players[i]->ready = (data[j+12]==1) ? true : false;

		if(ci)
			ci->CI_PSChanged(i,players[i]->ps);

		if(players[i]->ps == PS_KI)
		{
			players[i]->ready = true;
			if(ci)
				ci->CI_ReadyChanged(i,players[i]->ready);
		}

		LOG.write("    %d: %s %d %d %d %d %d %d %s\n", i, players[i]->name.c_str(), players[i]->ps, players[i]->rating, players[i]->ping, players[i]->nation, players[i]->color, players[i]->team, (players[i]->ready ? "true" : "false") );

		j += 13;
	}

	if(ci)
		ci->CI_NextConnectState(CS_FINISHED);
}

///////////////////////////////////////////////////////////////////////////////
/// player joined
/// @param message	Nachricht, welche ausgef�hrt wird
inline void GameClient::OnNMSPlayerNew(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength < 1)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}
	unsigned char *playerid = (unsigned char*)message->m_pData;

	LOG.write("<<< NMS_PLAYER_NEW(%d)\n", playerid[0]);

	if(playerid[0] != 0xFF)
	{
		char *name = (char*)&playerid[1];

		if(playerid[0] < player_count)
		{
			players[playerid[0]]->name = name;
			players[playerid[0]]->ps = PS_OCCUPIED;
			players[playerid[0]]->ping = 0;

			if(ci)
				ci->CI_NewPlayer(playerid[0]);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// player joined
/// @param message	Nachricht, welche ausgef�hrt wird
inline void GameClient::OnNMSPlayerPing(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength != 3)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}
	unsigned char playerid = *(unsigned char*)message->m_pData;
	unsigned short ping = *(unsigned short*)&((char*)message->m_pData)[1];

	//LOG.write("<<< NMS_PLAYER_PING(%d, %d)\n", playerid, ping);

	if(playerid != 0xFF)
	{
		if(playerid < player_count)
		{
			players[playerid]->ping = ping;

			if(ci)
				ci->CI_PingChanged(playerid,ping);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// player button gedr�ckt
/// @param message	Nachricht, welche ausgef�hrt wird
inline void GameClient::OnNMSPlayerToggleState(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength != 1)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}
	unsigned char playerid = *(unsigned char*)message->m_pData;

	LOG.write("<<< NMS_PLAYER_TOGGLESTATE(%d)\n", playerid);

	if(playerid != 0xFF)
	{
		if(playerid < player_count)
		{
			switch(players[playerid]->ps)
			{
			case PS_FREE:
				{
					players[playerid]->ps = PS_KI;
					// Baby mit einem Namen Taufen ("Name (KI)")
					char str[512];
					sprintf(str,_("Computer %u"),unsigned(playerid));
					players[playerid]->name = str;
					players[playerid]->name += _(" (AI)");
				} break;
			case PS_KI:     players[playerid]->ps = PS_LOCKED; break;
			case PS_LOCKED: players[playerid]->ps = PS_FREE;   break;
			default: break;
			}

			if(ci)
				ci->CI_PSChanged(playerid,players[playerid]->ps);
		}
		players[playerid]->ready = (players[playerid]->ps == PS_KI);

		if(ci)
			ci->CI_ReadyChanged(playerid,players[playerid]->ready);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// nation button gedr�ckt
/// @param message	Nachricht, welche ausgef�hrt wird
inline void GameClient::OnNMSPlayerToggleNation(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength != 2)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}
	unsigned char *data = (unsigned char*)message->m_pData;
	unsigned char playerid = data[0];
	Nation nation = Nation(data[1]);

	LOG.write("<<< NMS_PLAYER_TOGGLENATION(%d, %d)\n", playerid, nation);

	if(playerid != 0xFF)
	{
		if(playerid < player_count)
		{
			players[playerid]->nation = nation;

			if(ci)
				ci->CI_NationChanged(playerid,players[playerid]->nation);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// team button gedr�ckt
/// @param message	Nachricht, welche ausgef�hrt wird
inline void GameClient::OnNMSPlayerToggleTeam(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength != 2)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}
	unsigned char *data = (unsigned char*)message->m_pData;
	unsigned char playerid = data[0];
	unsigned char team = data[1];

	LOG.write("<<< NMS_PLAYER_TOGGLETEAM(%d, %d)\n", playerid, team);

	if(playerid != 0xFF)
	{
		if(playerid < player_count)
		{
			players[playerid]->team = team;

			if(ci)
				ci->CI_TeamChanged(playerid,players[playerid]->team);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// color button gedr�ckt
/// @param message	Nachricht, welche ausgef�hrt wird
inline void GameClient::OnNMSPlayerToggleColor(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength != 2)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}
	unsigned char *data = (unsigned char*)message->m_pData;
	unsigned char playerid = data[0];
	unsigned char color = data[1];

	LOG.write("<<< NMS_PLAYER_TOGGLECOLOR(%d, %d)\n", playerid, color);

	if(playerid != 0xFF)
	{
		if(playerid < player_count)
		{
			players[playerid]->color = color;

			if(ci)
				ci->CI_ColorChanged(playerid,players[playerid]->color);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Ready-state eines Spielers hat sich ge�ndert.
 *
 *  @param[in] message Nachricht, welche ausgef�hrt wird
 *
 *  @author FloSoft
 */
inline void GameClient::OnNMSPlayerReady(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength != 2)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}

	unsigned char *data = (unsigned char*)message->m_pData;
	unsigned char playerid = data[0];
	bool ready = (data[1] != 0 ? true : false);

	LOG.write("<<< NMS_PLAYER_READY(%d, %s)\n", playerid, (ready ? "true" : "false"));

	if(playerid != 0xFF)
	{
		if(playerid < player_count)
		{
			players[playerid]->ready = ready;

			if(ci)
				ci->CI_ReadyChanged(playerid,players[playerid]->ready);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// player gekickt
/// @param message	Nachricht, welche ausgef�hrt wird
inline void GameClient::OnNMSPlayerKicked(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength != sizeof(NS_PlayerKicked))
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}
	NS_PlayerKicked *npk = (NS_PlayerKicked*)message->m_pData;

	LOG.write("<<< NMS_PLAYER_KICKED(%d, %d, %d)\n", npk->playerid, npk->cause, npk->param);

	if(npk->playerid != 0xFF)
	{
		if(npk->playerid < player_count)
		{
			if(state == CS_GAME && GLOBALVARS.ingame)
			{
				// Im Spiel anzeigen, dass der Spieler das Spiel verlassen hat
				players[npk->playerid]->ps = PS_KI;
			}
			else
				players[npk->playerid]->clear();

			if(ci)
				ci->CI_PlayerLeft(npk->playerid);
		}
	}
}

inline void GameClient::OnNMSPlayerSwap(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength != 2)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}

	unsigned player1 = static_cast<unsigned char*>(message->m_pData)[0];
	unsigned player2 = static_cast<unsigned char*>(message->m_pData)[1];

	LOG.write("<<< NMS_PLAYER_SWAP(%u, %u)\n", player1,player2);

	players[player1]->SwapPlayer(*players[player2]);

	// Evtl. sind wir betroffen?
	if(playerid == player1)
		playerid = player2;
	else if(playerid == player2)
		playerid = player1;


	if(ci)
		ci->CI_PlayersSwapped(player1,player2);
}

///////////////////////////////////////////////////////////////////////////////
/// server typ
/// @param message	Nachricht, welche ausgef�hrt wird
inline void GameClient::OnNMSServerTyp(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength != 4)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}

	int servertypok = *(int*)message->m_pData;

	LOG.write("<<< NMS_SERVER_TYP(%d)\n", servertypok);

	switch(servertypok)
	{
	case 0: // ok
		break;
	case 1:
		{
			if(ci)
				ci->CI_Error(CE_INVALIDSERVERTYPE);
			Stop();
			return;
		} break;
	case 2:
		{
			if(ci)
				ci->CI_Error(CE_WRONGVERSION);
			Stop();
			return;
		} break;
	}

	GameMessage *msg = send_queue.push(NMS_SERVER_PASSWORD);
	msg->m_uiLength = unsigned(clientconfig.password.length()) + 1;
	msg->alloc(clientconfig.password.c_str());

	LOG.write(">>>NMS_SERVER_PASSWORD(********)\n");

	if(ci)
		ci->CI_NextConnectState(CS_QUERYPW);
}

///////////////////////////////////////////////////////////////////////////////
/// server passwort
/// @param message	Nachricht, welche ausgef�hrt wird
inline void GameClient::OnNMSServerPassword(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength != 1)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}

	bool passwordok = *(bool*)message->m_pData;

	LOG.write("<<< NMS_SERVER_PASSWORD(%d)\n", passwordok);

	if(passwordok == false)
	{
		if(ci)
			ci->CI_Error(CE_WRONGPW);

		Stop();
		return;
	}

	GameMessage *msg = send_queue.push(NMS_PLAYER_NAME);
	msg->m_uiLength = (unsigned int)(SETTINGS.name.length()) + 1;
	msg->alloc(SETTINGS.name.c_str());

	LOG.write(">>>NMS_PLAYER_NAME(%s)\n", SETTINGS.name.c_str());
}

///////////////////////////////////////////////////////////////////////////////
/// Servername
/// @param message	Nachricht, welche ausgef�hrt wird
inline void GameClient::OnNMSServerName(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength == 0)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}

	char *servername = (char *)message->m_pData;

	LOG.write("<<< NMS_SERVER_NAME(%s)\n", servername);

	clientconfig.gamename = servername;

	if(ci)
		ci->CI_NextConnectState(CS_QUERYPLAYERLIST);
}

///////////////////////////////////////////////////////////////////////////////
/// Chatnachricht
/// @param message	Nachricht, welche ausgef�hrt wird
inline void GameClient::OnNMSServerChat(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength < 2)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}

	unsigned char playerid = *(unsigned char*)message->m_pData;
	ChatDestination cd = ChatDestination(*((unsigned char*)message->m_pData+1));
	char *text = &((char*)message->m_pData)[2];

	LOG.write("<<< NMS_SERVER_CHAT(%u, %u, %s)\n", playerid, cd, text);

	if(playerid != 0xFF)
	{
		if(ci)
			ci->CI_Chat(playerid,cd,text);

		if(state == CS_GAME)
			/// Mit im Replay aufzeichnen
			replayinfo.replay.AddChatCommand(framesinfo.nr,playerid,cd,text);

	}
}


///////////////////////////////////////////////////////////////////////////////
/// server start
/// @param message	Nachricht, welche ausgef�hrt wird
inline void GameClient::OnNMSServerStart(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength != sizeof(NS_StartGameInfo))
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}

	NS_StartGameInfo *nsgi = (NS_StartGameInfo *)message->m_pData;

	LOG.write("<<< NMS_SERVER_START\n");


	// NWF-L�nge bekommen wir vom Server
	framesinfo.nwf_length = nsgi->nwf_length;

	StartGame(nsgi->random_init);


	// Nothing-Command f�r ersten Network-Frame senden
	SendNothingNC(0);
}

inline void GameClient::OnNMSServerAsync(GameMessage *message)
{
	if(message->m_uiLength != player_count * 4u)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}

	LOG.write("<<< NMS_SERVER_ASYNC\n");

	// Liste mit Namen und Checksummen erzeugen
	std::string checksum_list;

	for(unsigned i = 0; i < player_count; ++i)
	{
		char player_str[256];
		sprintf(player_str,"%s: %u ",players[i]->name.c_str(),static_cast<int*>(message->m_pData)[i]);
		checksum_list+=player_str;
	}

	// Fehler ausgeben (Konsole)!
	LOG.lprintf(_("The Game is not in sync. Checksums of some players don't match."));
	LOG.lprintf(checksum_list.c_str());
	LOG.lprintf("\n");

	// Messenger im Game
	if(ci && GLOBALVARS.ingame)
		ci->CI_Async(checksum_list);


	char filename[256], time_str[80];
	unser_time_t temp = TIME.CurrentTime();
	TIME.FormatTime(time_str, "async_%Y-%m-%d_%H-%i-%s", &temp);

	sprintf(filename,"%s%s-%u.log",FILE_PATHS[47], time_str, rand()%100);

	Random::inst().SaveLog(filename);

	LOG.lprintf("Async log saved at \"%s\"\n",filename);

	sprintf(filename,"%s%s.sav",FILE_PATHS[85], time_str);

	GameClient::inst().WriteSaveHeader(filename);

	// Pausieren

}

///////////////////////////////////////////////////////////////////////////////
/// Kartenname
/// @param message	Nachricht, welche ausgef�hrt wird
inline void GameClient::OnNMSMapName(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength == 0)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}

	char *mapfile = (char *)message->m_pData;

	LOG.write("<<< NMS_MAP_NAME(%s)\n", mapfile);

	// shortname
	clientconfig.mapfile = mapfile;
	// full path
	clientconfig.mapfilepath = FILE_PATHS[48] + clientconfig.mapfile;

	// karteninformationen anfordern
	send_queue.push(NMS_MAP_INFO);

	LOG.write(">>>NMS_MAP_INFO\n");
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  verarbeitet die MapInfo-Nachricht, in der die gepackte Gr��e,
 *  die normale Gr��e und Teilanzahl der Karte �bertragen wird.
 *
 *  @param message Nachricht, welche ausgef�hrt wird
 *
 *  @author FloSoft
 */
inline void GameClient::OnNMSMapInfo(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength != 16)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}

	unsigned int *infos = (unsigned int*)message->m_pData;

	LOG.write("<<< NMS_MAP_INFO(%u, %u, %u)\n", infos[1], infos[2], infos[3]);

	mapinfo.map_type = MapType(infos[0]);
	mapinfo.partcount = infos[1];
	mapinfo.ziplength = infos[2];
	mapinfo.length = infos[3];

	LOG.lprintf("Map has %u parts (%u/%u), i'll request them all!\n", infos[1], infos[2], infos[3]);

	temp_ui = 0;
	temp_ul = 0;

	if(mapinfo.zipdata)
		delete[] mapinfo.zipdata;
	mapinfo.zipdata = new unsigned char[mapinfo.ziplength + 1];

	for(unsigned int parts = 0; parts < mapinfo.partcount; ++parts)
	{
		send_queue.push(NMS_MAP_DATA);
		LOG.write(">>>NMS_MAP_DATA\n");
	}
}

///////////////////////////////////////////////////////////////////////////////
/// Kartendaten
/// @param message	Nachricht, welche ausgef�hrt wird
inline void GameClient::OnNMSMapData(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength == 0)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}
	LOG.write("<<< NMS_MAP_DATA(%u)\n", message->m_uiLength);

	unsigned char *data = (unsigned char*)message->m_pData;

	memcpy(&mapinfo.zipdata[temp_ul], data, message->m_uiLength);
	temp_ul += message->m_uiLength;

	++(temp_ui);
	if(temp_ui >= mapinfo.partcount)
	{
		FILE *map_f = fopen(clientconfig.mapfilepath.c_str(), "wb");

		if(!map_f)
		{
			LOG.lprintf("Fatal error: can't write map to %s: %s\n", clientconfig.mapfilepath.c_str(), strerror(errno));

			Stop();
			return;
		}

		char *map_data = new char[mapinfo.length + 1];

		unsigned int length = mapinfo.length;

		int err = BZ_OK;
		if( (err = BZ2_bzBuffToBuffDecompress(map_data, &length, (char*)mapinfo.zipdata, mapinfo.ziplength, 0, 0)) != BZ_OK)
		{
			LOG.lprintf("FATAL ERROR: BZ2_bzBuffToBuffDecompress failed with code %d\n", err);
			Stop();
			return;
		}
		fwrite(map_data, 1, mapinfo.length, map_f);

		mapinfo.checksum = CalcChecksumOfBuffer((unsigned char*)map_data, mapinfo.length);
		delete[] map_data;

		fclose(map_f);

		// Map-Typ unterscheiden
		switch(mapinfo.map_type)
		{
		case MAPTYPE_OLDMAP:
			{
				libsiedler2::ArchivInfo map;

				// Karteninformationen laden
				if(libsiedler2::loader::LoadMAP(clientconfig.mapfilepath.c_str(), &map, true) != 0)
				{
					LOG.lprintf("GameClient::OnNMSMapData: ERROR: Map \"%s\", couldn't load header!\n",clientconfig.mapfilepath.c_str());
					Stop();
					return;
				}

				const libsiedler2::ArchivItem_Map_Header *header = &(dynamic_cast<const glArchivItem_Map *>(map.get(0))->getHeader());
				assert(header);

				player_count = header->getPlayer();
				mapinfo.title = header->getName();

			} break;
		case MAPTYPE_SAVEGAME:
			{
				if(!mapinfo.savegame.Load(clientconfig.mapfilepath.c_str(),true,true))
				{
					Stop();
					return;
				}

				mapinfo.title = mapinfo.savegame.map_name;


			} break;
		case MAPTYPE_RTTRMAP:
			break;
		case MAPTYPE_RANDOMMAP:
			break;
		}

		GameMessage *msg = send_queue.push(NMS_MAP_CHECKSUM);
		msg->m_uiLength = sizeof(unsigned int);
		msg->alloc();
		unsigned int *crc = (unsigned int*)msg->m_pData;
		*crc = mapinfo.checksum;

		LOG.write(">>>NMS_MAP_CHECKSUM(%u)\n", mapinfo.checksum);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// map-checksum
/// @param message	Nachricht, welche ausgef�hrt wird
inline void GameClient::OnNMSMapChecksum(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength != 1)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}

	bool checksumok = *(bool*)message->m_pData;

	LOG.write("<<< NMS_MAP_CHECKSUM(%d)\n", checksumok);

	if(checksumok == false)
	{
		if(ci)
			ci->CI_Error(CE_WRONGMAP);

		Stop();
		return;
	}

}

///////////////////////////////////////////////////////////////////////////////
/// server typ
/// @param message	Nachricht, welche ausgef�hrt wird
inline void GameClient::OnNMSGGSChange(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength != GlobalGameSettings::GGS_BUFFER_SIZE)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}

	unsigned char *buffer = (unsigned char*)message->m_pData;

	LOG.write("<<< NMS_GGS_CHANGE\n");

	ggs.Deserialize(buffer);

	if(ci)
		ci->CI_GGSChanged(ggs);
}

///////////////////////////////////////////////////////////////////////////////
/// NFC Antwort vom Server
/// @param message	Nachricht, welche ausgef�hrt wird
inline void GameClient::OnNMSNfcAnswer(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength < 2)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}

	unsigned char player= *(unsigned char*)message->m_pData;
//	unsigned short nc_type = *(unsigned short*)&((unsigned char*)message->m_pData)[1];
//	unsigned char *nc_data = &((unsigned char*)message->m_pData)[3];

	//LOG.write("<<< NMS_NFC_ANSWER(%u, %u)\n", playerid, nc_type);

	if(player != 0xFF)
		// Nachricht in Queue einh�ngen
		players[player]->nfc_queue.push_back(new GameMessage(*message));
}

///////////////////////////////////////////////////////////////////////////////
/// NFC Done vom Server
/// @param message	Nachricht, welche ausgef�hrt wird
inline void GameClient::OnNMSNfcDone(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength != 0)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}

	//LOG.write("<<< NMS_NFC_DONE\n");
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  NFC Pause-Nachricht von Server
 *
 *  @param[in] message Nachricht, welche ausgef�hrt wird
 *
 *  @author FloSoft
 */
inline void GameClient::OnNMSNfcPause(GameMessage *message)
{
	if(!message)
		return;
	if(message->m_uiLength != 1)
	{
		LOG.lprintf("Invalid/corrupt message 0x%04X received\n", message->m_usID);

		Stop();
		return;
	}

	framesinfo.pause = (*(char*)message->m_pData == 1);
	framesinfo.lastmsgtime = VideoDriverWrapper::inst().GetTickCount();

	LOG.write("<<< NMS_NFC_PAUSE(%u)\n", framesinfo.pause);

	if(framesinfo.pause)
		ci->CI_GamePaused();
	else
		ci->CI_GameResumed();

}

///////////////////////////////////////////////////////////////////////////////
/// testet ob ein Netwerkframe abgelaufen ist und f�hrt dann ggf die Befehle aus
void GameClient::ExecuteGameFrame(const bool skipping)
{
	unsigned int currenttime = VideoDriverWrapper::inst().GetTickCount();

	if(framesinfo.pause)
	{
		if(!replay_mode && ((currenttime - framesinfo.lastmsgtime) > 10000) )
		{
			//if(ci && GLOBALVARS.ingame)
			//	ci->CI_GamePaused();

			framesinfo.lastmsgtime = currenttime;
		}

		// pause machen ;)

		return;
	}

	// Wurde der n�chsten Game-Frame zeitlich erreicht (bzw. wenn nur Frames �bersprungen werden sollen,
	// brauchen wir nicht zu warten)?
	if(skipping || (currenttime - framesinfo.lasttime) > framesinfo.gf_length)
	{
		//LOG.lprintf("%d = %d\n", framesinfo.nr / framesinfo.nwf_length, Random::inst().GetCurrentRandomValue());


		if(replay_mode)
		{
			// Diesen Zeitpunkt merken
			framesinfo.lasttime = currenttime - ( currenttime - framesinfo.lasttime - framesinfo.gf_length);
			// N�chster Game-Frame erreicht
			++framesinfo.nr;

			ExecuteGameFrame_Replay();

			// Frame-Time setzen zum Zeichnen, (immer au�er bei Lags)
			framesinfo.frame_time = currenttime - framesinfo.lasttime;

			// Diesen Zeitpunkt merken
			framesinfo.lasttime = currenttime;
		}

		// Ist jetzt auch ein NWF dran?
		else if(framesinfo.nr % framesinfo.nwf_length == 0)
		{
			// entsprechenden NC f�r diesen NWF ausf�hren
			// Beim Replay geht das etwas anderes, da werden die NFCs aus der Datei gelesen

			// Schauen wir mal ob alles angekommen ist
			bool is_lagging = false;
			for(unsigned char i = 0; i < player_count; ++i)
			{
				if(players[i]->ps == PS_OCCUPIED || players[i]->ps == PS_KI)
				{
					if(players[i]->nfc_queue.size() == 0)
					{
						players[i]->is_lagging = true;
						is_lagging = true;
					}
					else
						players[i]->is_lagging = false;
				}
			}

			// Laggt einer oder nicht?
			if(!is_lagging)
			{
				// Kein Lag, normal weitermachen

				// Diesen Zeitpunkt merken
				framesinfo.lasttime = currenttime;
				// N�chster Game-Frame erreicht
				++framesinfo.nr;

				ExecuteGameFrame_Game();

				// Frame-Time setzen zum Zeichnen, (immer au�er bei Lags)
				framesinfo.frame_time = currenttime - framesinfo.lasttime;

			} // if(!is_lagging)
			else
			{
				// LAG!
			} // else, if(!is_lagging)

		} // if(framesinfo.nr % framesinfo.nwf_length == 0)
		else
		{
			// N�hster GameFrame zwischen framesinfos

			// Diesen Zeitpunkt merken
			framesinfo.lasttime = currenttime;
			// N�chster Game-Frame erreicht
			++framesinfo.nr;

			// Frame ausf�hren
			em->NextGF();

			// Frame-Time setzen zum Zeichnen, (immer au�er bei Lags)
			framesinfo.frame_time = currenttime - framesinfo.lasttime;
		}

		// Auto-Speichern ggf.

		// Aktiviert?
		if(SETTINGS.autosave_interval && !replay_mode)
		{
			// Alle .... GF
			if(framesinfo.nr % SETTINGS.autosave_interval == 0)
			{
				std::string tmp = FILE_PATHS[85];
				tmp += _("Auto-Save");
				tmp += ".sav";

				WriteSaveHeader(tmp);
			}
		}

		// GF-Ende im Replay aktualisieren
		if(!replay_mode)
			replayinfo.replay.UpdateLastGF(framesinfo.nr);

	} // if(skipping || (currenttime - framesinfo.lasttime) > framesinfo.gf_length)
	else
	{
		// Frame-Time setzen zum Zeichnen, (immer au�er bei Lags)
		framesinfo.frame_time = currenttime - framesinfo.lasttime;
	}
}

void GameClient::ExecuteAllNCs(unsigned char * data, unsigned char player, unsigned char * player_switch_old_id,unsigned char * player_switch_new_id)
{
	// Anzahl der NCs
	unsigned char nc_count = *data;

	data+=1;

	for(unsigned char i = 0;i<nc_count;++i)
	{
		// L�nge auslesente
		unsigned short length = *((unsigned short*)data);
		// NC ausf�hren
		ExecuteNC(*((unsigned short*)(data+2)),player,data+4);

		//// Wenn ein Spieler gewechselt werden soll...
		if(*((unsigned short*)(data+2)) == NC_SWITCHPLAYER && player_switch_old_id && player_switch_new_id)
		{
			// ...m�ssen wir uns das merken
			*player_switch_old_id = player;
			*player_switch_new_id = data[4];
		}

		data+=(length+2);
	}
}

void GameClient::SendNothingNC(int checksum)
{
	if(checksum == -1)
		checksum = Random::inst().GetCurrentRandomValue();

	GameMessage nfc(NMS_NFC_COMMANDS, 5);
	*static_cast<int*>(nfc.m_pData) = checksum;
	send_queue.push(nfc);
}

// Kleine Signatur am Anfang "RTTRRP", die ein g�ltiges S25 RTTR Replay kennzeichnet
const char signature[] = {'R','T','T','R','R','P'};

void GameClient::WriteReplayHeader(const unsigned random_init)
{
	// Dateiname erzeugen
	char filename[256], time[80];
	unser_time_t temp = TIME.CurrentTime();
	TIME.FormatTime(time, "%Y-%m-%d_%H-%i-%s", &temp);

	sprintf(filename,"%s%s.rpl",FILE_PATHS[51], time);

	// Headerinfos f�llen

	// Timestamp der Aufzeichnung
	replayinfo.replay.save_time = temp;
	/// NWF-L�nge
	replayinfo.replay.nwf_length = framesinfo.nwf_length;
	// Random-Init
	replayinfo.replay.random_init = random_init;
	// Spieleranzahl
	replayinfo.replay.player_count = player_count;

	// Spielerdaten
	delete [] replayinfo.replay.players;
	replayinfo.replay.players = new SavedGameFile::Player[player_count];

	// Spielerdaten
	for(unsigned char i = 0;i<player_count;++i)
	{
		replayinfo.replay.players[i].ps = unsigned(players[i]->ps);

		if(players[i]->ps != PS_LOCKED)
		{
			replayinfo.replay.players[i].name = players[i]->name;
			replayinfo.replay.players[i].nation = players[i]->nation;
			replayinfo.replay.players[i].color = players[i]->color;
			replayinfo.replay.players[i].team = players[i]->team;
		}
	}

	// GGS-Daten
	replayinfo.replay.ggs = ggs;

	// Map

	replayinfo.replay.map_type = MapType(mapinfo.map_type);

	switch(replayinfo.replay.map_type)
	{
	default:
		break;

	case MAPTYPE_OLDMAP:
		{
			// Gr��e der entpackten Map
			replayinfo.replay.map_length  = mapinfo.length;
			// Gr��e der gepackten Map
			replayinfo.replay.map_zip_length = mapinfo.ziplength;
			// Gepackte Map
			replayinfo.replay.map_data = mapinfo.zipdata;
		} break;
	case MAPTYPE_SAVEGAME:
		{
			replayinfo.replay.savegame = &mapinfo.savegame;
		} break;
	}


	// Mapname
	replayinfo.replay.map_name = clientconfig.mapfile;

	// Datei speichern
	if(!replayinfo.replay.WriteHeader(filename))
		LOG.lprintf("GameClient::WriteReplayHeader: WARNING: File couldn't be opened. Don't use a replayinfo.replay.\n");
}

unsigned GameClient::StartReplay(const std::string &path, GameWorldViewer * &gwv)
{
	replayinfo.filename = path;
	replayinfo.replay.savegame = &mapinfo.savegame;

	if(!replayinfo.replay.LoadHeader(path,true))
		return false;


	// NWF-L�nge
	framesinfo.nwf_length = replayinfo.replay.nwf_length;
	// Spieleranzahl
	player_count = replayinfo.replay.player_count;

	players = new GameClientPlayer*[player_count];

	// Spielerdaten
	for(unsigned char i = 0;i<player_count;++i)
	{
		players[i] = new GameClientPlayer(i);

		players[i]->ps = PlayerState(replayinfo.replay.players[i].ps);

		if(players[i]->ps != PS_LOCKED)
		{
			players[i]->name = replayinfo.replay.players[i].name;
			players[i]->nation = replayinfo.replay.players[i].nation;
			players[i]->color = replayinfo.replay.players[i].color;
			players[i]->team = replayinfo.replay.players[i].team;
		}
	}

	// GGS-Daten
	ggs = replayinfo.replay.ggs;

	// Map-Type auslesen
	mapinfo.map_type = replayinfo.replay.map_type;

	switch(replayinfo.replay.map_type)
	{
	default:
		break;
	case MAPTYPE_OLDMAP:
		{
			// Mapdaten auslesen und entpacken
			unsigned char * real_data = new unsigned char[replayinfo.replay.map_length];

			int err;
			if( (err = BZ2_bzBuffToBuffDecompress((char*)real_data, &replayinfo.replay.map_length, (char*)replayinfo.replay.map_data, replayinfo.replay.map_zip_length, 0, 0)) != BZ_OK)
			{
				LOG.lprintf("FATAL ERROR: BZ2_bzBuffToBuffDecompress failed with code %d\n", err);
				Stop();
				return 5;
			}

			// Richtigen Pfad zur Map erstellen
			clientconfig.mapfile = replayinfo.replay.map_name;
			clientconfig.mapfilepath = FILE_PATHS[48] +  replayinfo.replay.map_name;

			// Und entpackte Mapdaten speichern
			BinaryFile map_f;
			if(!map_f.Open(clientconfig.mapfilepath.c_str(),OFM_WRITE))
			{
				LOG.lprintf("GameClient::StartReplay: ERROR: Couldn't open file \'%s\' for writing!\n",clientconfig.mapfilepath.c_str());
				return 6;
			}
			map_f.WriteRawData(real_data,replayinfo.replay.map_length);
			map_f.Close();

			delete[] real_data;
		} break;
	case MAPTYPE_SAVEGAME:
		{
		} break;
	}

	replay_mode = true;
	replayinfo.async = false;
	replayinfo.end = false;

	StartGame(replayinfo.replay.random_init);

	replayinfo.replay.ReadGF(&replayinfo.next_gf);

	gwv = gw;

	return 0;
}
//
//unsigned GameClient::GetGlobalAnimation(8,unsigned time_part_nominator, unsigned time_part_denominator,
//		unsigned divide_nominator, unsigned divide_denominator, unsigned offset)
//{
//	//return ((networkframe.nr * networkframe.length + networkframe.frame_time+offset) % time_part) / divide;
//	return ((networkframe.nr * networkframe.length + networkframe.frame_time+offset) % (networkframe.length*time_part_nominator/time_part_denominator)) / (networkframe.length*time_part_nominator/time_part_denominator);
//}

unsigned GameClient::GetGlobalAnimation(const unsigned max,unsigned factor_numerator, unsigned factor_denumerator, unsigned offset)
{
	unsigned factor = framesinfo.gf_length*factor_numerator/factor_denumerator;
	return ((framesinfo.nr * framesinfo.gf_length + framesinfo.frame_time+offset) % (factor*max)) / factor;
}

unsigned GameClient::Interpolate(unsigned max_val,EventManager::EventPointer ev)
{
	assert( ev != NULL );
	return min<unsigned int>(((max_val*((framesinfo.nr-ev->gf) * framesinfo.gf_length+framesinfo.frame_time)) / (ev->gf_length * framesinfo.gf_length)),max_val-1);
}

int GameClient::Interpolate(int x1,int x2,EventManager::EventPointer ev)
{
	assert( ev != NULL );
	return (x1+( (x2-x1)*((int(framesinfo.nr)-int(ev->gf)) * int(framesinfo.gf_length)+int(framesinfo.frame_time))) / int(ev->gf_length * framesinfo.gf_length));
}

void GameClient::ServerLost()
{
	if(LOBBYCLIENT.LoggedIn()) // steht die Lobbyverbindung noch?
		LOBBYCLIENT.DeleteServer();

	if(ci)
		ci->CI_Error(CE_CONNECTIONLOST);


	if(state != CS_STOPPED)
		LOG.lprintf("client forced to stop\n");

	state = CS_STOPPED;

	so.Close();
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  �berspringt eine bestimmte Anzahl von Gameframes.
 *
 *  @param[in] dest_gf Zielgameframe
 *
 *  @author OLiver
 *  @author FloSoft
 */
void GameClient::SkipGF(unsigned int gf)
{
	if(gf <= framesinfo.nr)
		return;

	// Spiel entpausieren
	SetReplayPause(false);

	// GFs �berspringen
	for(unsigned int i = framesinfo.nr; i < gf; ++i)
		ExecuteGameFrame(true);

	// Spiel pausieren
	SetReplayPause(true);
}

unsigned GameClient::WriteSaveHeader(const std::string& filename)
{
	Savegame save;

	// Timestamp der Aufzeichnung
	save.save_time = TIME.CurrentTime();
	// Mapname
	save.map_name = this->mapinfo.title;
	// Anzahl Spieler
	save.player_count = player_count;
	save.players = new SavedGameFile::Player[player_count];

	// Spielerdaten
	for(unsigned char i = 0;i<player_count;++i)
	{
		save.players[i].ps = unsigned(players[i]->ps);

		if(players[i]->ps != PS_LOCKED)
		{
			save.players[i].name = players[i]->name;
			save.players[i].nation = players[i]->nation;
			save.players[i].color = players[i]->color;
			save.players[i].team = players[i]->team;
		}
	}

	// GGS-Daten
	save.ggs = ggs;

	save.start_gf = framesinfo.nr;



	// Spiel serialisieren
	save.sgd.MakeSnapshot(gw,em);

	// Und alles speichern
	if(!save.Save(filename))
		return 1;


	return 0;

}

void GameClient::GetVisualSettings()
{
	GameClientPlayer * player = GetLocalPlayer();
	//visual_settings.transport_order[0] = player->transport[GD_COINS];
	//visual_settings.transport_order[1] = player->transport[GD_SWORD];
	//visual_settings.transport_order[2] = player->transport[GD_BEER];
	//visual_settings.transport_order[3] = player->transport[GD_IRON];
	//visual_settings.transport_order[4] = player->transport[GD_GOLD];
	//visual_settings.transport_order[5] = player->transport[GD_IRONORE];
	//visual_settings.transport_order[6] = player->transport[GD_COAL];
	//visual_settings.transport_order[7] = player->transport[GD_BOARDS];
	//visual_settings.transport_order[8] = player->transport[GD_STONES];
	//visual_settings.transport_order[9] = player->transport[GD_WOOD];
	//visual_settings.transport_order[10] = player->transport[GD_WATER];
	//visual_settings.transport_order[11] = player->transport[GD_FISH];
	//visual_settings.transport_order[12] = player->transport[GD_HAMMER];
	//visual_settings.transport_order[13] = player->transport[GD_BOAT];

	visual_settings.transport_order[player->transport[GD_COINS]] = 0;
	visual_settings.transport_order[player->transport[GD_SWORD]] = 1;
	visual_settings.transport_order[player->transport[GD_BEER]] = 2;
	visual_settings.transport_order[player->transport[GD_IRON]] = 3;
	visual_settings.transport_order[player->transport[GD_GOLD]] = 4;
	visual_settings.transport_order[player->transport[GD_IRONORE]] = 5;
	visual_settings.transport_order[player->transport[GD_COAL]] = 6;
	visual_settings.transport_order[player->transport[GD_BOARDS]] = 7;
	visual_settings.transport_order[player->transport[GD_STONES]] = 8;
	visual_settings.transport_order[player->transport[GD_WOOD]] = 9;
	visual_settings.transport_order[player->transport[GD_WATER]] = 10;
	visual_settings.transport_order[player->transport[GD_FISH]] = 11;
	visual_settings.transport_order[player->transport[GD_HAMMER]] = 12;
	visual_settings.transport_order[player->transport[GD_BOAT]] = 13;



	visual_settings.distribution[0] = player->distribution[GD_FISH].percent_buildings[BLD_GRANITEMINE];
	visual_settings.distribution[1] = player->distribution[GD_FISH].percent_buildings[BLD_COALMINE];
	visual_settings.distribution[2] = player->distribution[GD_FISH].percent_buildings[BLD_IRONMINE];
	visual_settings.distribution[3] = player->distribution[GD_FISH].percent_buildings[BLD_GOLDMINE];

	visual_settings.distribution[4] = player->distribution[GD_GRAIN].percent_buildings[BLD_MILL];
	visual_settings.distribution[5] = player->distribution[GD_GRAIN].percent_buildings[BLD_PIGFARM];
	visual_settings.distribution[6] = player->distribution[GD_GRAIN].percent_buildings[BLD_DONKEYBREEDER];
	visual_settings.distribution[7] = player->distribution[GD_GRAIN].percent_buildings[BLD_BREWERY];

	visual_settings.distribution[8] = player->distribution[GD_IRON].percent_buildings[BLD_ARMORY];
	visual_settings.distribution[9] = player->distribution[GD_IRON].percent_buildings[BLD_METALWORKS];

	visual_settings.distribution[10] = player->distribution[GD_COAL].percent_buildings[BLD_ARMORY];
	visual_settings.distribution[11] = player->distribution[GD_COAL].percent_buildings[BLD_IRONSMELTER];
	visual_settings.distribution[12] = player->distribution[GD_COAL].percent_buildings[BLD_MINT];

	visual_settings.distribution[13] = player->distribution[GD_BOARDS].percent_buildings[BLD_HEADQUARTERS];
	visual_settings.distribution[14] = player->distribution[GD_BOARDS].percent_buildings[BLD_METALWORKS];
	visual_settings.distribution[15] = player->distribution[GD_BOARDS].percent_buildings[BLD_SHIPYARD];

	visual_settings.distribution[16] = player->distribution[GD_WATER].percent_buildings[BLD_BAKERY];
	visual_settings.distribution[17] = player->distribution[GD_WATER].percent_buildings[BLD_BREWERY];
	visual_settings.distribution[18] = player->distribution[GD_WATER].percent_buildings[BLD_PIGFARM];
	visual_settings.distribution[19] = player->distribution[GD_WATER].percent_buildings[BLD_DONKEYBREEDER];


	memcpy(visual_settings.military_settings,player->military_settings,7);
	memcpy(visual_settings.tools_settings,player->tools_settings,12);

	visual_settings.order_type = player->order_type;

	// Baureihenfolge f�llen (0 ist das HQ!)
	for(unsigned char i = 0; i < 31; ++i)
		visual_settings.build_order[i] = player->build_order[i];
}


void GameClient::SetReplayPause(bool pause)
{
	if(replay_mode)
	{
		framesinfo.pause = pause;
		framesinfo.frame_time = 0;
	}
}


void GameClient::AddToGameLog(const char * const str)
{
	if(!replay_mode)
		fputs(str,game_log);
}
