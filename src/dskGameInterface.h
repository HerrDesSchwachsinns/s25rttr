// $Id: dskGameInterface.h 4854 2009-05-11 11:26:19Z OLiver $
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
#ifndef dskGAMEINTERFACE_H_INCLUDED
#define dskGAMEINTERFACE_H_INCLUDED

#pragma once

#include "Desktop.h"
#include "Messenger.h"
#include "iwAction.h"
#include "MapConsts.h"
#include "Minimap.h"
#include "customborderbuilder.h"

#include "ClientInterface.h"
#include "GameInterface.h"
#include "LobbyInterface.h"

class IngameWindow;
class iwAction;
class iwRoadWindow;
class GameWorldViewer;

enum RoadMode
{
	RM_DISABLED, // kein Stra�enbau
	RM_NORMAL, // Bau einer normalen Stra�e
	RM_BOAT // Bau einer Wasserstra�e
};

struct RoadsBuilding
{
	RoadMode mode;   ///< Stra�enmodus

	MapCoord point_x;
	MapCoord point_y;
	MapCoord start_x;
	MapCoord start_y;
	std::vector<unsigned char> route;  ///< Richtungen der gebauten Stra�e
};

class dskGameInterface : 
	public Desktop, 
	public ClientInterface, 
	public GameInterface,
	public LobbyInterface
{
private:

	// Interface f�r das Spiel
	GameWorldViewer *gwv;

	CustomBorderBuilder cbb;

	libsiedler2::ArchivInfo borders;

	/// Stra�enbauzeug
	RoadsBuilding road;

	// Aktuell ge�ffnetes Aktionsfenster
	iwAction *actionwindow;
	// Aktuell ge�ffnetes Stra�enbaufenster
	iwRoadWindow *roadwindow;
	// Is der lokale Spieler der Host?
	bool ishost;
	// Messenger f�r die Nachrichten
	Messenger messenger;
	// Aktuell selektierter Punkt auf der Karte
	MapCoord selected_x,selected_y;
	/// Minimap-Instanz
	IngameMinimap minimap;

public:
	/// Konstruktor von @p dskGameInterface.
	dskGameInterface();
	/// Destruktor von @p dskGameInterface.
	~dskGameInterface(void);

	void LC_Status_ConnectionLost(void);
	void LC_Status_Error(const std::string &error);

	/// L�sst das Spiel laufen (zeichnen)
	void Run();

	/// Wird aufgerufen, wenn eine Taste gedr�ckt wurde
	void KeyPressed(KeyEvent ke);
	/// Wird bei Linksmausklicks aufgerufen
	void MouseClicked(MouseCoords * mc);

	/// Aktiviert Stra�enbaumodus bzw gibt zur�ck, ob er aktiviert ist
	void ActivateRoadMode(const RoadMode rm);
	RoadMode GetRoadMode() const { return road.mode; }

	/// Baut die gew�nschte bis jetzt noch visuelle Stra�e (schickt Anfrage an Server)
	void CommandBuildRoad();

	/// Wird aufgerufen, wenn die Fenster geschlossen werden
	void ActionWindowClosed();
	void RoadWindowClosed();

	friend class GameClient;
	friend class GameWorld;
	friend class RoadSegment;

	// Sucht einen Weg von road_point_x/y zu cselx/y und baut ihn ( nur visuell )
	bool BuildRoadPart(const int cselx, const int csely,bool end);
	// Prft, ob x;y auf der bereits gebauten Strecke liegt und gibt die Position+1 zurck vom Startpunkt der Strecke aus
	// wenn der Punkt nicht draufliegt, kommt 0 zurck
	unsigned TestBuiltRoad(const int x, const int y);
	// Zeigt das Stra�cnfenster an und entscheidet selbstst�cdig, ob man eine Flagge an road_point_x/y bauen kann,
	// ansonsten gibt's nur nen Button zum Abbrechen
	void ShowRoadWindow(int mouse_x, int mouse_y);
	/// Zeigt das Actionwindow an, bei Flaggen werden z.B. noch ber�cksichtigt, obs ne besondere Flagge ist usw
	void ShowActionWindow(const iwAction::Tabs& action_tabs,int cselx,int csely,int mouse_x, int mouse_y, const bool enable_military_buildings);

private:

	/// Baut Weg zur�ck von Ende bis zu start_id
	void DemolishRoad(const unsigned start_id);

	void Msg_ButtonClick(const unsigned int ctrl_id);
	void Msg_PaintBefore();
	void Msg_PaintAfter();
	bool Msg_LeftDown(const MouseCoords& mc);
	bool Msg_MouseMove(const MouseCoords& mc);
	bool Msg_RightDown(const MouseCoords& mc);
	bool Msg_RightUp(const MouseCoords& mc);
	bool Msg_KeyDown(const KeyEvent& ke);

	void CI_PlayerLeft(const unsigned player_id);
	void CI_Chat(const unsigned player_id, const ChatDestination cd, const std::string& msg);
	void CI_Async(const std::string& checksums_list);
	void CI_ReplayAsync(const std::string& msg);
	void CI_ReplayEndReached(const std::string& msg);
	void CI_GamePaused();
	void CI_GameResumed();
	void CI_Error(const ClientError ce);

	/// Wird aufgerufen, wann immer eine Flagge zerst�rt wurde, da so evtl der Wegbau abgebrochen werden muss
	void CI_FlagDestroyed(const unsigned short x, const unsigned short y);
	/// Spielerwechsel
	void CI_PlayersSwapped(const unsigned player1, const unsigned player2);

	/// Wenn ein Spieler verloren hat
	void GI_PlayerDefeated(const unsigned player_id);
	/// Es wurde etwas Minimap entscheidendes ge�ndert --> Minimap updaten
	void GI_UpdateMinimap(const MapCoord x, const MapCoord y);
	
};

#endif // !dskGAMEINTERFACE_H_INCLUDED
