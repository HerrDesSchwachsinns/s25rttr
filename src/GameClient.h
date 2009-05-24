// $Id: GameClient.h 4933 2009-05-24 12:29:23Z OLiver $
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
#ifndef GAMECLIENT_H_
#define GAMECLIENT_H_

#include "Singleton.h"
#include "Socket.h"

#include "GameMessageInterface.h"

#include "GamePlayerList.h"

#include "EventManager.h"
#include "GameFiles.h"
#include "GameWorld.h"
#include "GlobalGameSettings.h"

class Window;
class GameClientPlayer;
class WorldManager;
class ClientInterface;
class GameMessage;
namespace gc { class GameCommand; };

class GameClient : public Singleton<GameClient>, public GameMessageInterface
{
public:
	enum ClientState
	{
		CS_STOPPED = 0,
		CS_CONNECT,
		CS_CONFIG,
		CS_GAME
	};

	GameClient(void);
	~GameClient(void);

	void SetInterface(ClientInterface *ci) { this->ci = ci; }
	bool IsHost() const { return clientconfig.host; }
	bool IsSavegame() const { return mapinfo.map_type == MAPTYPE_SAVEGAME; }
	std::string GetGameName() const { return clientconfig.gamename; }

	unsigned char GetPlayerID() const { return playerid; }
	unsigned GetPlayerCount() const { return players.getCount(); }
	/// Liefert einen Player zur�ck
	GameClientPlayer * GetPlayer(const unsigned int id) { return dynamic_cast<GameClientPlayer*>(players.getElement(id)); }
	GameClientPlayer * GetLocalPlayer(void) { return GetPlayer(playerid); }

	/// Gibt GGS zur�ck
	const GlobalGameSettings& GetGGS() const { return ggs; }

	bool Connect(const std::string& server, const std::string& password, unsigned char servertyp, unsigned short port, bool host);
	void Run();
	void Stop();

	// Gibt GameWorldViewer zur�ck (VORL�UFIG, soll sp�ter verschwinden!!)
	GameWorldViewer * QueryGameWorldViewer() const { return static_cast<GameWorldViewer*>(gw); }
	/// Gibt Map-Titel zur�ck
	const std::string& GetMapTitle() const { return mapinfo.title; }
	/// Gibt Pfad zu der Map zur�ck
	const std::string& GetMapPath() const  { return clientconfig.mapfilepath; }
	/// Gibt Map-Typ zur�ck
	const MapType GetMapType() const { return mapinfo.map_type; }


	// Initialisiert und startet das Spiel
	void StartGame(const unsigned random_init);
	/// Wird aufgerufen, wenn das GUI fertig mit Laden ist und es losgehen kann
	void RealStart();

	/// Beendet das Spiel, zerst�rt die Spielstrukturen
	void ExitGame();

	ClientState GetState() const { return state; }
	unsigned int GetGFNumber() const { return framesinfo.nr; }
	unsigned int GetGFLength() const { return framesinfo.gf_length; }
	unsigned int GetNWFLength() const { return framesinfo.nwf_length; }
	unsigned int GetFrameTime() const { return framesinfo.frame_time; }
	unsigned int GetGlobalAnimation(const unsigned max,unsigned factor_numerator, unsigned factor_denumerator, unsigned offset);
	unsigned int Interpolate(unsigned max_val,EventManager::EventPointer ev);
	int Interpolate(int x1,int x2,EventManager::EventPointer ev);
	/// Gibt Geschwindigkeits-Faktor zur�ck

	void AddGC(gc::GameCommand * gc);


	void Command_SetFlag2(int x, int y, unsigned char player);
	void Command_Chat(const std::string& text, const ChatDestination cd );
	void Command_ToggleNation();
	void Command_ToggleTeam();
	void Command_ToggleColor();
	void Command_ToggleReady();


	/// L�dt ein Replay und startet dementsprechend das Spiel (0 = alles OK, alles andere entsprechende Fehler-ID!)
	unsigned StartReplay(const std::string &path, GameWorldViewer * &gwv);
	/// Replay-Geschwindigkeit erh�hen/verringern
	void IncreaseReplaySpeed() { if(replay_mode && framesinfo.gf_length>10) framesinfo.gf_length-=10; }
	void DecreaseReplaySpeed() { if(replay_mode && framesinfo.gf_length<1000) framesinfo.gf_length+=10; }
	void SetReplayPause(bool pause);
	void ToggleReplayPause() { SetReplayPause(!framesinfo.pause); }
	/// Schaltet FoW im Replaymodus ein/aus
	void ToggleReplayFOW() { replayinfo.all_visible = !replayinfo.all_visible; }
	/// Pr�ft, ob FoW im Replaymodus ausgeschalten ist
	bool IsReplayFOWDisabled() const { return replayinfo.all_visible; }
	/// Gibt Replay-Ende (GF) zur�ck
	unsigned GetLastReplayGF() const { return replayinfo.replay.last_gf; }

	/// Gibt Replay-Dateiname zur�ck
	const std::string& GetReplayFileName() const { return replayinfo.filename; }
	/// Wird ein Replay abgespielt?
	bool IsReplayModeOn() const { return replay_mode; }

	void SkipGF(unsigned int gf);

	/// Wechselt den aktuellen Spieler (nur zu Debugzwecken !!)
	void ChangePlayer(const unsigned char old_id,const unsigned char new_id);

	/// Wechselt den aktuellen Spieler im Replaymodus
	void ChangeReplayPlayer(const unsigned new_id);

	/// Laggt ein bestimmter Spieler gerade?
	bool IsLagging(const unsigned int id) { return GetPlayer(id)->is_lagging; }

	/// Spiel pausiert?
	bool IsPaused() const { return framesinfo.pause; }

	/// Schreibt Header der Save-Datei
	unsigned WriteSaveHeader(const std::string& filename);

	/// Visuelle Einstellungen aus den richtigen ableiten
	void GetVisualSettings();

	/// Zum Log etwas hinzuf�gen
	void AddToGameLog(const char * const str);


private:
	/// Versucht einen neuen GameFrame auszuf�hren, falls die Zeit daf�r gekommen ist
	void ExecuteGameFrame(const bool skipping = false);
	void ExecuteGameFrame_Replay();
	void ExecuteGameFrame_Game();
	/// Filtert aus einem Network-Command-Paket alle Commands aus und f�hrt sie aus, falls ein Spielerwechsel-Command
	/// dabei ist, f�llt er die �bergebenen IDs entsprechend aus
	void ExecuteAllGCs(const GameMessage_GameCommand& gcs,  unsigned char * player_switch_old_id,unsigned char * player_switch_new_id);
	/// Sendet ein NC-Paket ohne Befehle
	void SendNothingNC(int checksum = -1);


	//  Netzwerknachrichten
	virtual void OnNMSDeadMsg(unsigned int id);

	virtual void OnNMSPing();

	virtual void OnNMSServerTypeOK(const GameMessage_Server_TypeOK& msg);
	virtual void OnNMSServerPassword(const GameMessage_Server_Password& msg);
	virtual void OnNMSServerName(const GameMessage_Server_Name& msg);
	virtual void OnNMSServerStart(const GameMessage_Server_Start& msg);
	virtual void OnNMSServerChat(const GameMessage_Server_Chat& msg);
	virtual void OnNMSServerAsync(const GameMessage_Server_Async& msg);

	virtual void OnNMSPlayerId(const GameMessage_Player_Id& msg);
	virtual void OnNMSPlayerList(const GameMessage_Player_List& msg);
	virtual void OnNMSPlayerToggleState(const GameMessage_Player_Toggle_State& msg);
	virtual void OnNMSPlayerToggleNation(const GameMessage_Player_Toggle_Nation& msg);
	virtual void OnNMSPlayerToggleTeam(const GameMessage_Player_Toggle_Team& msg);
	virtual void OnNMSPlayerToggleColor(const GameMessage_Player_Toggle_Color& msg);
	virtual void OnNMSPlayerKicked(const GameMessage_Player_Kicked& msg);
	virtual void OnNMSPlayerPing(const GameMessage_Player_Ping& msg);
	virtual void OnNMSPlayerNew(const GameMessage_Player_New& msg);
	virtual void OnNMSPlayerReady(const GameMessage_Player_Ready& msg);
	virtual void OnNMSPlayerSwap(const GameMessage_Player_Swap& msg);

	void OnNMSMapInfo(const GameMessage_Map_Info& msg);
	void OnNMSMapData(const GameMessage_Map_Data& msg);
	void OnNMSMapChecksumOK(const GameMessage_Map_ChecksumOK& msg);

	virtual void OnNMSPause(const GameMessage_Pause& msg);
	virtual void OnNMSServerDone(const GameMessage_Server_NWFDone& msg);
	virtual void OnNMSGameCommand(const GameMessage_GameCommand& msg);

	void OnNMSGGSChange(const GameMessage_GGSChange& msg);

	/// Wird aufgerufen, wenn der Server gegangen ist (Verbindung verloren, ung�ltige Nachricht etc.)
	void ServerLost();

	// Replaymethoden

	/// Schreibt den Header der Replaydatei
	void WriteReplayHeader(const unsigned random_init);

public:
	/// Virtuelle Werte der Einstellungsfenster, die aber noch nicht wirksam sind, nur um die Verz�gerungen zu
	/// verstecken
	struct VisualSettings
	{
		/// Verteilung
		std::vector<unsigned char> distribution;
		/// Art der Reihenfolge (0 = nach Auftraggebung, ansonsten nach build_order)
		unsigned char order_type;
		/// Baureihenfolge
		std::vector<unsigned char> build_order;
		/// Transport-Reihenfolge
		std::vector<unsigned char> transport_order;
		/// Milit�reinstellungen (die vom Milit�rmen�)
		std::vector<unsigned char> military_settings;
		/// Werkzeugeinstellungen (in der Reihenfolge wie im Fenster!)
		std::vector<unsigned char> tools_settings;

		VisualSettings() : distribution(20), build_order(31), transport_order(14), military_settings(7), tools_settings(12)
		{}

	} visual_settings, default_settings;

private:
	/// Spielwelt
	GameWorld * gw;
	/// EventManager
	EventManager * em;
	/// Spieler
	GameClientPlayerList players;
	/// Spieler-ID dieses Clients
	unsigned char playerid;
	/// Globale Spieleinstellungen
	GlobalGameSettings ggs;

	MessageQueue recv_queue, send_queue;
	Socket socket;
	// Was soll das sein? oO
	unsigned int temp_ul;
	unsigned int temp_ui;

	ClientState state;

	class ClientConfig
	{
	public:
		ClientConfig() { Clear(); }
		void Clear();

		std::string server;
		std::string gamename;
		std::string password;
		std::string mapfile;
		std::string mapfilepath;
		unsigned char servertyp;
		unsigned short port;
		bool host;
	} clientconfig;

	class MapInfo
	{
	public:
		MapInfo() { Clear(); }
		void Clear();

		MapType map_type;
		unsigned partcount;
		unsigned ziplength;
		unsigned length;
		unsigned checksum;
		std::string title;
		unsigned char *zipdata;
		Savegame savegame;
	} mapinfo;

	class FramesInfo
	{
	public:
		FramesInfo() { Clear(); }
		void Clear();

		/// Aktueller GameFrame (der wievielte seit Beginn)
		unsigned nr;
		/// L�nge der GameFrames in ms (= Geschwindigkeit des Spiels)
		unsigned gf_length;
		/// L�nge der Network-Frames in gf(!)
		unsigned nwf_length;

		/// Zeit in ms seit dem letzten Frame
		unsigned frame_time;

		unsigned lasttime;
		unsigned lastmsgtime;
		unsigned pausetime;

		bool pause;
	} framesinfo;

	class RandCheckInfo
	{
	public:
		RandCheckInfo() { Clear(); }
		void Clear();

		int rand;
	} randcheckinfo;


	ClientInterface *ci;

	/// GameCommands, die vom Client noch an den Server gesendet werden m�ssen
	std::vector<gc::GameCommand*> gcs;

	struct ReplayInfo
	{
	public:
		ReplayInfo() { Clear(); }
		void Clear();

		/// Replaydatei
		Replay replay;
		/// Replay asynchron (Meldung nur einmal ausgeben!)
		bool async;
		bool end;
		// N�chster Replay-Command-Zeitpunkt (in GF)
		unsigned next_gf;
		/// Replay-Dateiname
		std::string filename;
		/// Alles sichtbar (FoW deaktiviert)
		bool all_visible;
	} replayinfo;

	/// Replaymodus an oder aus?
	bool replay_mode;

	/// Spiel-Log f�r Asyncs
	FILE * game_log;
};

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#define GAMECLIENT GameClient::inst()

#endif
