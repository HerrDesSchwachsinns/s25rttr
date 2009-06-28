// $Id: dskGameInterface.cpp 5139 2009-06-28 21:06:58Z OLiver $
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
#include "dskGameInterface.h"

#include "files.h"
#include "VideoDriverWrapper.h"
#include "Settings.h"
#include "GlobalVars.h"
#include "ListDir.h"
#include "GameWorld.h"
#include "WindowManager.h"
#include "SoundManager.h"
#include "EventManager.h"
#include "GameServer.h"
#include "GameClient.h"
#include "GameClientPlayer.h"
#include "VideoDriverWrapper.h"
#include "LobbyClient.h"
#include "ctrlButton.h"
#include "GameMessages.h"

#include "iwChat.h"
#include "iwHQ.h"
#include "iwStorehouse.h"
#include "iwAction.h"
#include "iwRoadWindow.h"
#include "iwBuildingSite.h"
#include "iwMainMenu.h"
#include "iwPostWindow.h"
#include "iwBuilding.h"
#include "iwMilitaryBuilding.h"
#include "iwSkipGFs.h"
#include "iwMinimap.h"
#include "iwSave.h"
#include "iwTextfile.h"
#include "iwOptionsWindow.h"
#include "iwEndgame.h"

#include "noBuilding.h"
#include "nobHQ.h"
#include "noBuildingSite.h"
#include "nobMilitary.h"
#include "nobStorehouse.h"
#include "nobUsual.h"
#include <sstream>

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/**
 *  Konstruktor von @p dskGameInterface.
 *  Startet das Spiel und l�dt alles Notwendige.
 *
 *  @author OLiver
 */
dskGameInterface::dskGameInterface()
	: Desktop(NULL), 
	gwv(GameClient::inst().QueryGameWorldViewer()), cbb(GetPalette(0)),
	actionwindow(NULL), roadwindow(NULL),
	selected_x(0), selected_y(0), minimap(*gwv)
{
	road.mode = RM_DISABLED;
	road.point_x = 0;
	road.point_y = 0;
	road.start_x = 0;
	road.start_y = 0;

	SetScale(false);;

	int barx = (VideoDriverWrapper::inst().GetScreenWidth() - GetImage(resource_dat, 29)->getWidth()) / 2 + 44;
	int bary = VideoDriverWrapper::inst().GetScreenHeight() - GetImage(resource_dat, 29)->getHeight() + 4;

	AddImageButton(0, barx,        bary, 37, 32, TC_GREEN1, GetImage(io_dat,  50), _("Map"))
		->SetBorder(false);
	AddImageButton(1, barx + 37,   bary, 37, 32, TC_GREEN1, GetImage(io_dat, 192), _("Main selection"))
		->SetBorder(false);
	AddImageButton(2, barx + 37*2, bary, 37, 32, TC_GREEN1, GetImage(io_dat,  83), _("Construction aid mode"))
		->SetBorder(false);
	AddImageButton(3, barx + 37*3, bary, 37, 32, TC_GREEN1, GetImage(io_dat,  62), _("Post office"))
		->SetBorder(false);

  AddText(4, barx + 37*3 + 18, bary + 24, "0", COLOR_YELLOW, glArchivItem_Font::DF_CENTER|glArchivItem_Font::DF_VCENTER, SmallFont);

	LOBBYCLIENT.SetInterface(this);
	GAMECLIENT.SetInterface(this);

	GAMECLIENT.RealStart();

	// Wir sind nun ingame
	GLOBALVARS.ingame = true;
	gwv->SetGameInterface(this);

	cbb.loadEdges( &Loader::inst().resource_dat );
	cbb.buildBorder(VideoDriverWrapper::inst().GetScreenWidth(),
		VideoDriverWrapper::inst().GetScreenHeight(),&borders);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Destruktor von @p dskGameInterface.
 *  Beendet das Spiel und r�umt alles auf.
 *
 *  @author OLiver
 */
dskGameInterface::~dskGameInterface()
{
	GLOBALVARS.ingame = false;
}

void dskGameInterface::Msg_ButtonClick(const unsigned int ctrl_id)
{
	switch(ctrl_id)
	{
	case 0: // Karte
		{
			WindowManager::inst().Show(new iwMinimap(&minimap,*gwv));
		} break;
	case 1: // Optionen
		{
			WindowManager::inst().Show(new iwMainMenu);
		} break;
	case 2: // Baukosten
		{
			if(WindowManager::inst().IsDesktopActive())
				gwv->ShowBQ();
		} break;
	case 3: // Post
		{
			WindowManager::inst().Show(new iwPostWindow(*gwv));
		} break;
	}
}


void dskGameInterface::Msg_PaintBefore()
{
	// Spiel ausf�hren
	Run();

	// Rahmen zeichnen
	dynamic_cast<glArchivItem_Bitmap*>(borders.get(0))->Draw(0,0); // oben (mit Ecken)
	dynamic_cast<glArchivItem_Bitmap*>(borders.get(1))->Draw(0,VideoDriverWrapper::inst().GetScreenHeight()-12); // unten (mit Ecken)
	dynamic_cast<glArchivItem_Bitmap*>(borders.get(2))->Draw(0,12); // links
	dynamic_cast<glArchivItem_Bitmap*>(borders.get(3))->Draw(VideoDriverWrapper::inst().GetScreenWidth()-12,12); // rechts

	GetImage(resource_dat, 17)->Draw(12,12, 0, 0, 0, 0, 0, 0);
	GetImage(resource_dat, 18)->Draw(VideoDriverWrapper::inst().GetScreenWidth()-12-GetImage(resource_dat, 18)->getWidth(),12, 0, 0, 0, 0, 0, 0);
	GetImage(resource_dat, 19)->Draw(12,VideoDriverWrapper::inst().GetScreenHeight()-12-GetImage(resource_dat, 19)->getHeight(), 0, 0, 0, 0, 0, 0);
	GetImage(resource_dat, 20)->Draw(VideoDriverWrapper::inst().GetScreenWidth()-12-GetImage(resource_dat, 20)->getWidth(),VideoDriverWrapper::inst().GetScreenHeight()-12-GetImage(resource_dat, 20)->getHeight(), 0, 0, 0, 0, 0, 0);
	GetImage(resource_dat, 29)->Draw(VideoDriverWrapper::inst().GetScreenWidth()/2-GetImage(resource_dat, 29)->getWidth()/2,VideoDriverWrapper::inst().GetScreenHeight()-GetImage(resource_dat, 29)->getHeight(), 0, 0, 0, 0, 0, 0);
}

void dskGameInterface::Msg_PaintAfter()
{
	/* NWF-Anzeige (vorl�ufig)*/
	char nwf_string[256];

	if(GameClient::inst().IsReplayModeOn())
		snprintf(nwf_string, 255, _("(Replay-Mode) Current GF: %u (End at: %u) / GF length: %u ms / NWF length: %u gf (%u ms)"),GAMECLIENT.GetGFNumber(),GAMECLIENT.GetLastReplayGF(), GAMECLIENT.GetGFLength(), GAMECLIENT.GetNWFLength(), GAMECLIENT.GetNWFLength() * GAMECLIENT.GetGFLength());
	else
		snprintf(nwf_string, 255, _("Current GF: %u / GF length: %u ms / NWF length: %u gf (%u ms) /  Ping: %u ms"),GAMECLIENT.GetGFNumber(),GAMECLIENT.GetGFLength(), GAMECLIENT.GetNWFLength(), GAMECLIENT.GetNWFLength() * GAMECLIENT.GetGFLength(), GAMECLIENT.GetLocalPlayer()->ping);

	NormalFont->Draw(30,1,nwf_string,0,0xFFFFFF00);

	// Replaydateianzeige in der linken unteren Ecke
	if(GameClient::inst().IsReplayModeOn())
		NormalFont->Draw(0, VideoDriverWrapper::inst().GetScreenHeight(),GameClient::inst().GetReplayFileName(),glArchivItem_Font::DF_BOTTOM,0xFFFFFF00);

	// Mauszeiger
	if(road.mode != RM_DISABLED)
	{
		if(VideoDriverWrapper::inst().IsLeftDown())
			GetImage(resource_dat, 35)->Draw(VideoDriverWrapper::inst().GetMouseX(),VideoDriverWrapper::inst().GetMouseY(), 0, 0, 0, 0, 0, 0);
		else
			GetImage(resource_dat, 34)->Draw(VideoDriverWrapper::inst().GetMouseX(),VideoDriverWrapper::inst().GetMouseY(), 0, 0, 0, 0, 0, 0);
	}
	else
	{
		if(VideoDriverWrapper::inst().IsLeftDown())
			GetImage(resource_dat, 31)->Draw(VideoDriverWrapper::inst().GetMouseX(),VideoDriverWrapper::inst().GetMouseY(), 0, 0, 0, 0, 0, 0);
		else if(VideoDriverWrapper::inst().IsRightDown())
			GetImage(resource_dat, 32)->Draw(VideoDriverWrapper::inst().GetMouseX(),VideoDriverWrapper::inst().GetMouseY(), 0, 0, 0, 0, 0, 0);
		else
			GetImage(resource_dat, 30)->Draw(VideoDriverWrapper::inst().GetMouseX(),VideoDriverWrapper::inst().GetMouseY(), 0, 0, 0, 0, 0, 0);
	}

	// Laggende Spieler anzeigen in Form von Schnecken
	for(unsigned int i = 0;i < GAMECLIENT.GetPlayerCount(); ++i)
	{
		GameClientPlayer *player = GAMECLIENT.GetPlayer(i);
		if(player->is_lagging)
			GetPlayerImage(rttr_lst, 0)->Draw(SETTINGS.width-70-i*40, 35, 30, 30, 0, 0, 0, 0, COLORS[player->color]);
	}
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
bool dskGameInterface::Msg_LeftDown(const MouseCoords& mc)
{
	if(Coll(mc.x,mc.y,SETTINGS.width/2-GetImage(resource_dat, 29)->getWidth()/2+44,
		SETTINGS.height-GetImage(resource_dat, 29)->getHeight()+4,37*4,32*4))
		return false;

	// Unterscheiden je nachdem Stra�cnbaumodus an oder aus ist
	if(road.mode)
	{
		// in "richtige" Map-Koordinaten Konvertieren, den aktuellen selektierten Punkt
		unsigned short cselx=gwv->GetSelX(),csely = gwv->GetSelY();

		if(cselx == road.point_x && csely == road.point_y)
		{
			// Selektierter Punkt ist der gleiche wie der Stra�enpunkt --> Fenster mit Wegbau abbrechen
			ShowRoadWindow(mc.x,mc.y);
		}
		else
		{
			// altes Roadwindow schlie�en
			WindowManager::inst().Close((unsigned int)CGI_ROADWINDOW);

			// Ist das ein g�ltiger neuer Wegpunkt?
			if(gwv->RoadAvailable(road.mode == RM_BOAT,cselx,csely,0xFF) && gwv->GetNode(cselx,csely).owner-1 == (signed)GAMECLIENT.GetPlayerID() &&
				gwv->IsPlayerTerritory(cselx,csely))
			{
				if(!BuildRoadPart(cselx,csely,false))
					ShowRoadWindow(mc.x,mc.y);
			}
			else if(gwv->CalcBQ(cselx,csely,GAMECLIENT.GetPlayerID(),1))
			{
				// Wurde bereits auf das gebaute St�ck geklickt?
				unsigned tbr;
				if((tbr = TestBuiltRoad(cselx,csely)))
					DemolishRoad(tbr);
				else
				{
					if(BuildRoadPart(cselx,csely,1))
						CommandBuildRoad();
					else
						ShowRoadWindow(mc.x,mc.y);
				}
			}
			// Wurde auf eine Flagge geklickt und ist diese Flagge nicht der Weganfangspunkt?
			else if(gwv->GetNO(cselx,csely)->GetType() == NOP_FLAG && !(cselx == road.start_x && csely == road.start_y))
			{
				if(BuildRoadPart(cselx,csely,1))
					CommandBuildRoad();
				else
					ShowRoadWindow(mc.x,mc.y);
			}

			else
			{
				unsigned tbr;
				// Wurde bereits auf das gebaute St�ck geklickt?
				if((tbr = TestBuiltRoad(cselx,csely)))
					DemolishRoad(tbr);
				else
					ShowRoadWindow(mc.x,mc.y);
			}
		}
	}
	else
	{
		bool enable_military_buildings = false;

		iwAction::Tabs action_tabs;

		unsigned short cselx=gwv->GetSelX(),csely = gwv->GetSelY();

		// Evtl ists nen Haus? (unser Haus)
		if(gwv->GetNO(cselx,csely)->GetType() == NOP_BUILDING	&& gwv->GetNode(cselx,csely).owner-1 == (signed)GAMECLIENT.GetPlayerID())
		{
			// HQ
			if(static_cast<noBuilding*>(gwv->GetNO(cselx,csely))->GetBuildingType() == BLD_HEADQUARTERS)
				WindowManager::inst().Show(new iwHQ(gwv,gwv->GetSpecObj<nobHQ>(cselx,csely)));
			// Lagerh�user
			else if(static_cast<noBuilding*>(gwv->GetNO(cselx,csely))->GetBuildingType() == BLD_STOREHOUSE)
				WindowManager::inst().Show(new iwStorehouse(gwv,gwv->GetSpecObj<nobStorehouse>(cselx,csely)));
			// Milit�rgeb�ude
			else if(static_cast<noBuilding*>(gwv->GetNO(cselx,csely))->GetBuildingType() <= BLD_FORTRESS)
				WindowManager::inst().Show(new iwMilitaryBuilding(gwv,gwv->GetSpecObj<nobMilitary>(cselx,csely)));
			else
				WindowManager::inst().Show(new iwBuilding(gwv,this,gwv->GetSpecObj<nobUsual>(cselx,csely)));
			return true;
		}

		// oder vielleicht eine Baustelle?
		else if(gwv->GetNO(cselx,csely)->GetType() == NOP_BUILDINGSITE && gwv->GetNode(cselx,csely).owner-1 == (signed)GAMECLIENT.GetPlayerID())
		{
			WindowManager::inst().Show(new iwBuildingSite(gwv,gwv->GetSpecObj<noBuildingSite>(cselx,csely)));
			return true;

		}

		action_tabs.watch = true;
		// Unser Land
		if(gwv->GetNode(cselx,csely).owner == GAMECLIENT.GetPlayerID()+1)
		{
			// Kann hier was gebaut werden?
			if(gwv->GetNode(gwv->GetSelX(),gwv->GetSelY()).bq >= BQ_HUT)
			{
				action_tabs.build = true;

				// Welches Geb�ude kann gebaut werden?
				switch(gwv->GetNode(gwv->GetSelX(),gwv->GetSelY()).bq)
				{
				case BQ_HUT: action_tabs.build_tabs = iwAction::Tabs::BT_HUT; break;
				case BQ_HOUSE: action_tabs.build_tabs = iwAction::Tabs::BT_HOUSE; break;
				case BQ_CASTLE: action_tabs.build_tabs = iwAction::Tabs::BT_CASTLE; break;
				case BQ_MINE: action_tabs.build_tabs = iwAction::Tabs::BT_MINE; break;
				case BQ_HARBOR: action_tabs.build_tabs = iwAction::Tabs::BT_HARBOR; break;
				default: break;
				}

				if(!gwv->FlagNear(cselx,csely))
					action_tabs.setflag = true;

				// Pr�fen, ob sich Milit�rgeb�ude in der N�he befinden, wenn nein, k�nnen auch eigene
				// Milit�rgeb�ude gebaut werden
				enable_military_buildings = !gwv->IsMilitaryBuildingNearNode(cselx,csely);
			}
			else if(gwv->GetNode(gwv->GetSelX(),gwv->GetSelY()).bq == BQ_FLAG)
				action_tabs.setflag = true;



			if(gwv->GetNO(cselx,csely)->GetType() == NOP_FLAG)
				action_tabs.flag = true;

			// Pr�fen, ob irgendwo Stra�en anliegen
			bool roads = false;
			for(unsigned i = 0;i<6;++i)
				if(gwv->GetPointRoad(cselx,csely, i, true))
					roads = true;

			if( (roads) && !(
				gwv->GetNO(cselx,csely)->GetType() == NOP_FLAG ||
				gwv->GetNO(cselx,csely)->GetType() == NOP_BUILDING) )
				action_tabs.cutroad = true;
		}
		// evtl ists ein feindliches Milit�rgeb�ude, welches NICHT im Nebel liegt?
		else if(gwv->GetNO(cselx,csely)->GetType() == NOP_BUILDING && gwv->GetVisibility(cselx,csely) == VIS_VISIBLE)
		{
			// Ist es ein gew�hnliches Milit�rgeb�ude?
			if(gwv->GetSpecObj<noBuilding>(cselx,csely)->GetBuildingType() >= BLD_BARRACKS &&
				gwv->GetSpecObj<noBuilding>(cselx,csely)->GetBuildingType() <= BLD_FORTRESS)
			{
				// Dann darf es nicht neu gebaut sein!
				if(!gwv->GetSpecObj<nobMilitary>(cselx,csely)->IsNewBuilt())
					action_tabs.attack = true;
			}
			// oder ein HQ?
			else if(gwv->GetSpecObj<noBuilding>(cselx,csely)->GetBuildingType() == BLD_HEADQUARTERS)
				action_tabs.attack = true;

		}

		// Bisheriges Actionfenster schlie�en, falls es eins gab
		// aktuelle Mausposition merken, da diese durch das Schlie�en ver�ndert werden kann
		int mx = mc.x,my = mc.y;
		WindowManager::inst().Close(actionwindow);
		VideoDriverWrapper::inst().SetMousePos(mx,my);

		ShowActionWindow(action_tabs,cselx,csely,mc.x,mc.y,enable_military_buildings);

		selected_x = static_cast<unsigned short>(cselx);
		selected_y = static_cast<unsigned short>(csely);

	}

	return true;
}



bool dskGameInterface::Msg_MouseMove(const MouseCoords& mc)
{
	gwv->MouseMove(mc);
	return false;
}

bool dskGameInterface::Msg_RightDown(const MouseCoords& mc)
{
	gwv->MouseDown(mc);
	return false;
}

bool dskGameInterface::Msg_RightUp(const MouseCoords& mc)
{
	gwv->MouseUp();
	return false;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Druck von Spezialtasten auswerten.
 *
 *  @author OLiver
 */
bool dskGameInterface::Msg_KeyDown(const KeyEvent& ke)
{
	switch(ke.kt)
	{
	default:
		break;
	case KT_RETURN: // Chatfenster �ffnen
		{
			WindowManager::inst().Show(new iwChat);
		} return true;

	case KT_SPACE: // Bauqualit�ten anzeigen
		{
			gwv->ShowBQ();
		} return true;

	case KT_LEFT: // Nach Links Scrollen
		{
			gwv->MoveToX(-30);
		} return true;

	case KT_RIGHT: // Nach Rechts Scrollen
		{
			gwv->MoveToX(30);
		} return true;

	case KT_UP: // Nach Oben Scrollen
		{
			gwv->MoveToY(-30);
		} return true;

	case KT_DOWN: // Nach Unten Scrollen
		{
			gwv->MoveToY(30);
		} return true;
	case KT_F2: // Spiel speichern
		{
			WindowManager::inst().Show(new iwSave);
		} return true;
	case KT_F3: // Koordinatenanzeige ein/aus vorl�ufig zu Debugzwecken
		{
			gwv->ShowCoordinates();
		} return true;
	case KT_F8: // Tastaturbelegung
		{
			WindowManager::inst().Show(new iwTextfile("keyboardlayout.txt",_("Keyboard layout")));
		} return true;
	case KT_F9: // Readme
		{
			WindowManager::inst().Show(new iwTextfile("readme.txt",_("Readme!")));
		} return true;
	case KT_F12: // Optionsfenster
		{
			WindowManager::inst().Show(new iwOptionsWindow);
		} return true;


	}


	switch(ke.c)
	{
	case '+': // Geschwindigkeit im Replay erh�hen
		{
			GAMECLIENT.IncreaseReplaySpeed();
		} return true;
	case '-': // Geschwindigkeit im Replay verringern
		{
			GAMECLIENT.DecreaseReplaySpeed();
		} return true;

	case '1':	case '2':	case '3': // Spieler umschalten
	case '4':	case '5':	case '6':
	case '7':
		{
			if(GameClient::inst().IsReplayModeOn())
				GameClient::inst().ChangeReplayPlayer(ke.c-'1');
			else
			{
				if(GameClient::inst().GetPlayer(ke.c-'1')->ps == PS_KI)
					GAMECLIENT.AddGC(new gc::SwitchPlayer(ke.c-'1'));
			}
		} return true;

	case 'j': // GFs �berspringen
		{
			if(GAMECLIENT.IsReplayModeOn())
				WindowManager::inst().Show(new iwSkipGFs);
		} return true;

	case 'p': // Pause
		{
			if(GAMECLIENT.IsHost())
			{
				GAMESERVER.TogglePause();
				//if(GAMESERVER.TogglePause())
				//	GAMECLIENT.Command_Chat(_("pausing game"));
				//else
				//	GAMECLIENT.Command_Chat(_("continuing game"));
			}
			else if(GAMECLIENT.IsReplayModeOn())
			{
				GAMECLIENT.ToggleReplayPause();
			}
		} return true;

	case 'h': // Zum HQ springen
		{
			GameClientPlayer *player = GAMECLIENT.GetLocalPlayer();
			// Pr�fen, ob dieses �berhaupt noch existiert
			if(player->hqx != 0xFFFF)
				gwv->MoveToMapObject(player->hqx,player->hqy);
		} return true;

	case 'c': // Geb�udenamen anzeigen
		{
			gwv->ShowNames();
		} return true;
	case 's': // Produktivit�t anzeigen
		{
			gwv->ShowProductivity();
		} return true;
	case 'd': // Replay: FoW an/ausschalten
		{
			// GameClient Bescheid sagen
			GameClient::inst().ToggleReplayFOW();
			// Sichtbarkeiten neu setzen auf der Map-Anzeige und der Minimap
			gwv->RecalcAllColors();
			minimap.UpdateAll();
		} return true;
	case 'l': // Minimap anzeigen
		{
			WindowManager::inst().Show(new iwMinimap(&minimap,*gwv));
		} return true;
	case 'm': // Hauptauswahl
		{
			WindowManager::inst().Show(new iwMainMenu);
		} return true;
	case 'q': // Spiel verlassen
		{
			if(ke.alt)
				WindowManager::inst().Show(new iwEndgame);
		} return true;
	case 'b': // Zur lezten Position zur�ckspringen
		{
			gwv->MoveToLastPosition();
		} return true;
	}

	return false;
}


///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
void dskGameInterface::Run()
{
	unsigned water_percent;
	gwv->Draw(GAMECLIENT.GetPlayerID(),&water_percent,actionwindow?true:false,selected_x, selected_y,road);

	// Evtl Meeresrauschen-Sounds abspieln
	SOUNDMANAGER.PlayOceanBrawling(water_percent);

	messenger.Draw();
}


///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
void dskGameInterface::ActivateRoadMode(const RoadMode rm)
{
	// Im Replay und in der Pause keine Stra�en bauen
	if(GameClient::inst().IsReplayModeOn() || GameClient::inst().IsPaused())
		return;

	road.mode = rm;
	if(rm != RM_DISABLED)
	{
		road.route.clear();
		road.start_x = road.point_x = selected_x;
		road.start_y = road.point_y = selected_y;
		unsigned short tmp_px,tmp_py,tmp_sx,tmp_sy;
		gwv->ConvertCoords(road.point_x,road.point_y,&tmp_px,&tmp_py);
		gwv->ConvertCoords(road.start_x,road.start_y,&tmp_sx,&tmp_sy);
		road.point_x = tmp_px;
		road.point_y = tmp_py;
		road.start_x = tmp_sx;
		road.start_y = tmp_sy;
	}
	else
	{
		gwv->RemoveVisualRoad(road.start_x,road.start_y, road.route);
		for(unsigned i = 0;i<road.route.size();++i)
		{
			gwv->SetPointVirtualRoad(road.start_x,road.start_y, road.route[i], 0);
			int tx2 = road.start_x,ty2 = road.start_y;
			road.start_x = gwv->GetXA(tx2,ty2,road.route[i]);
			road.start_y = gwv->GetYA(tx2,ty2,road.route[i]);
		}
	}

}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
bool dskGameInterface::BuildRoadPart(const int cselx, const int csely,bool end)
{
	std::vector<unsigned char> new_route;
	bool path_found = gwv->FindRoadPath(road.point_x,road.point_y,cselx,csely,new_route, road.mode == RM_BOAT);

	// Weg gefunden?
	if(!path_found)
		return false;

	// Weg (visuell) bauen
	for(unsigned i = 0;i<new_route.size();++i)
	{
		gwv->SetPointVirtualRoad(road.point_x,road.point_y, new_route[i], (road.mode==RM_BOAT)?3:1);
		gwv->GetPointA(road.point_x,road.point_y, new_route[i]);
		gwv->CalcRoad(road.point_x,road.point_y,GAMECLIENT.GetPlayerID());
	}

	road.route.insert(road.route.end(), new_route.begin(), new_route.end());

	return true;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
unsigned dskGameInterface::TestBuiltRoad(const int x, const int y)
{
	int x2 = road.start_x, y2 = road.start_y;
	for(unsigned i = 0;i<road.route.size();++i)
	{
		if(x2 == x && y2 == y)
			return i+1;

		int tx2 = x2,ty2 = y2;
		x2 = gwv->GetXA(tx2,ty2,road.route[i]);
		y2 = gwv->GetYA(tx2,ty2,road.route[i]);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
void dskGameInterface::ShowRoadWindow(int mouse_x, int mouse_y)
{
	if(gwv->CalcBQ(road.point_x,road.point_y,GAMECLIENT.GetPlayerID(),1))
		WindowManager::inst().Show(roadwindow = new iwRoadWindow(this,1,mouse_x,mouse_y),true);
	else
		WindowManager::inst().Show(roadwindow = new iwRoadWindow(this,0,mouse_x,mouse_y),true);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
void dskGameInterface::ShowActionWindow(const iwAction::Tabs& action_tabs,int cselx,int csely,int mouse_x, int mouse_y, const bool enable_military_buildings)
{
	unsigned int params = 0;

	// Sind wir am Wasser?
	if(action_tabs.setflag)
	{
		for(unsigned char x = 0; x < 6; ++x)
		{
			if(gwv->GetTerrainAround(cselx, csely, x) == 14)
				params = iwAction::AWFT_WATERFLAG;
		}
	}

	// Wenn es einen Flaggen-Tab gibt, dann den Flaggentyp herausfinden und die Art des Fensters entsprechende setzen
	if(action_tabs.flag)
	{

		if(gwv->GetNO(cselx-!(csely&1),csely-1)->GetGOT() == GOT_NOB_HQ)
			params = iwAction::AWFT_HQ;
		else if(gwv->GetNO(cselx, csely)->GetType() == NOP_FLAG)
		{
			if(gwv->GetSpecObj<noFlag>(cselx,csely)->GetFlagType() == FT_WATER)
				params = iwAction::AWFT_WATERFLAG;
		}
	}

	// Angriffstab muss wissen, wieviel Soldaten maximal entsendet werden k�nnen
	if(action_tabs.attack)
	{
		if(GAMECLIENT.GetLocalPlayer()->IsPlayerAttackable(gwv->GetSpecObj<noBuilding>(cselx, csely)->GetPlayer()))
			params = gwv->GetAvailableSoldiersForAttack(GAMECLIENT.GetPlayerID(), cselx, csely);
	}

	WindowManager::inst().Show((actionwindow = new iwAction(this, gwv, action_tabs, cselx, csely, mouse_x, mouse_y, params, enable_military_buildings)), true);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author OLiver
 */
void dskGameInterface::CommandBuildRoad()
{
	GameClient::inst().AddGC(new gc::BuildRoad(road.start_x,road.start_y,road.mode == RM_BOAT,road.route));
	road.mode = RM_DISABLED;
}

void dskGameInterface::GI_FlagDestroyed(const unsigned short x, const unsigned short y)
{
	// Im Wegbaumodus und haben wir von hier eine Flagge gebaut?
	if(road.mode != RM_DISABLED && road.start_x == x && road.start_y == y)
	{
		// Wegbau abbrechen
		ActivateRoadMode(RM_DISABLED);
	}

	// Evtl Actionfenster schlie�en, da sich das ja auch auf diese Flagge bezieht
	if(actionwindow)
	{
		if(actionwindow->GetSelectedX() == x && actionwindow->GetSelectedY() == y)
			WindowManager::inst().Close(actionwindow);
	}
}

void dskGameInterface::ActionWindowClosed()
{
	actionwindow = NULL;
	gwv->DontScroll();
}

void dskGameInterface::RoadWindowClosed()
{
	roadwindow = NULL;
	gwv->DontScroll();
}



void dskGameInterface::CI_PlayerLeft(const unsigned player_id)
{
	// Info-Meldung ausgeben
	char text[256];
	snprintf(text, 256, _("Player '%s' left the game!"), GameClient::inst().GetPlayer(player_id)->name.c_str());
	messenger.AddMessage("", 0, CD_SYSTEM, text, COLOR_RED);
	// Im Spiel anzeigen, dass die KI das Spiel betreten hat
	snprintf(text, 256, _("Player '%s' joined the game!"), "KI");
	messenger.AddMessage("", 0, CD_SYSTEM, text, COLOR_GREEN);
}

void dskGameInterface::CI_GGSChanged(const GlobalGameSettings& ggs)
{
	// TODO: print what has changed
	char text[256];
	snprintf(text, 256, _("Note: Game settings changed by the server%s"),"");
	messenger.AddMessage("", 0, CD_SYSTEM, text);
}

void dskGameInterface::CI_Chat(const unsigned player_id, const ChatDestination cd, const std::string& msg)
{
	char from[256];
	snprintf(from, 256, _("<%s> "), GameClient::inst().GetPlayer(player_id)->name.c_str());
	messenger.AddMessage(from,
		COLORS[GameClient::inst().GetPlayer(player_id)->color], cd, msg);
}

void dskGameInterface::CI_Async(const std::string& checksums_list)
{
	messenger.AddMessage("", 0, CD_SYSTEM, _("The Game is not in sync. Checksums of some players don't match."), COLOR_RED);
	messenger.AddMessage("", 0, CD_SYSTEM, checksums_list.c_str(), COLOR_YELLOW);
	messenger.AddMessage("", 0, CD_SYSTEM, _("A auto-savegame is created..."), COLOR_RED);

}

void dskGameInterface::CI_ReplayAsync(const std::string& msg)
{
	messenger.AddMessage("", 0, CD_SYSTEM, msg, COLOR_RED);
}

void dskGameInterface::CI_ReplayEndReached(const std::string& msg)
{
	messenger.AddMessage("", 0, CD_SYSTEM, msg, COLOR_BLUE);
}

void dskGameInterface::CI_GamePaused()
{
	char from[256];
	snprintf(from, 256, _("<%s> "), _("SYSTEM"));
	messenger.AddMessage(from, COLOR_GREY, CD_SYSTEM, _("Game was paused."));

	/// Stra�enbau ggf. abbrechen, wenn aktiviert
	if(road.mode != RM_DISABLED)
	{
		road.mode = RM_DISABLED;
		// Fenster schlie�en
		if(roadwindow)
		{
			//WindowManager::inst().Close(roadwindow);
			roadwindow->Close();
			roadwindow = 0;
		}
		// Weg zur�ckbauen
		this->DemolishRoad(1);

	}
}

void dskGameInterface::CI_GameResumed()
{
	char from[256];
	snprintf(from, 256, _("<%s> "), _("SYSTEM"));
	messenger.AddMessage(from, COLOR_GREY, CD_SYSTEM, _("Game was resumed."));
}



void dskGameInterface::CI_Error(const ClientError ce)
{
	switch(ce)
	{
	default:
		break;

	case CE_CONNECTIONLOST:
		{
			messenger.AddMessage("", 0, CD_SYSTEM, _("Lost connection to server!"), COLOR_RED);
		} break;
	}
}


///////////////////////////////////////////////////////////////////////////////
/** 
 *  Status: Verbindung verloren.
 *
 *  @author FloSoft
 */
void dskGameInterface::LC_Status_ConnectionLost()
{
	messenger.AddMessage("", 0, CD_SYSTEM, _("Lost connection to lobby!"), COLOR_RED);
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  (Lobby-)Status: Benutzerdefinierter Fehler
 *
 *  @author FloSoft
 */
void dskGameInterface::LC_Status_Error(const std::string &error)
{
	messenger.AddMessage("", 0, CD_SYSTEM, error, COLOR_RED);
}

void dskGameInterface::CI_PlayersSwapped(const unsigned player1, const unsigned player2)
{
	// Meldung anzeigen
	char text[256];
	snprintf(text, 256, _("Player '%s' switched to player '%s'"), GameClient::inst().GetPlayer(player1)->name.c_str()
		,GameClient::inst().GetPlayer(player2)->name.c_str());
	messenger.AddMessage("", 0, CD_SYSTEM, text, COLOR_YELLOW);


	// Sichtbarkeiten und Minimap neu berechnen, wenn wir ein von den beiden Spielern sind
	if(player1 == GameClient::inst().GetPlayerID() || player2 == GameClient::inst().GetPlayerID())
	{
		minimap.UpdateAll();
		gwv->RecalcAllColors();
	}

}

/// Wenn ein Spieler verloren hat
void dskGameInterface::GI_PlayerDefeated(const unsigned player_id)
{
	char text[256];
	snprintf(text, 256, _("Player '%s' was defeated!"), GameClient::inst().GetPlayer(player_id)->name.c_str());
	messenger.AddMessage("", 0, CD_SYSTEM, text, COLOR_ORANGE);

	/// Lokaler Spieler?
	if(player_id == GameClient::inst().GetPlayerID())
	{
		/// Sichtbarkeiten neu berechnen
		gwv->RecalcAllColors();
		// Minimap updaten
		minimap.UpdateAll();
	}
}

void dskGameInterface::GI_UpdateMinimap(const MapCoord x, const MapCoord y)
{
	// Minimap Bescheid sagen
	minimap.UpdateNode(x,y);
}

/// B�ndnisvertrag wurde abgeschlossen oder abgebrochen --> Minimap updaten
void dskGameInterface::GI_TreatyOfAllianceChanged()
{
	// Nur wenn Team-Sicht aktiviert ist, k�nnen sihc die Sichtbarkeiten auch �ndern
	if(GameClient::inst().GetGGS().team_view)
	{
		/// Sichtbarkeiten neu berechnen
		gwv->RecalcAllColors();
		// Minimap updaten
		minimap.UpdateAll();
	}
}

/// Baut Weg zur�ck von Ende bis zu start_id
void dskGameInterface::DemolishRoad(const unsigned start_id)
{
	for(unsigned i = road.route.size();i>=start_id;--i)
	{
		int tx = road.point_x,ty = road.point_y;
		road.point_x = gwv->GetXA(tx,ty,(road.route[i-1]+3)%6);
		road.point_y = gwv->GetYA(tx,ty,(road.route[i-1]+3)%6);
		gwv->SetPointVirtualRoad(road.point_x,road.point_y, road.route[i-1], 0);
		gwv->CalcRoad(tx,ty,GAMECLIENT.GetPlayerID());
	}

	road.route.resize(start_id-1);
}

/// Updatet das Post-Icon mit der Nachrichtenanzahl und der Taube
void dskGameInterface::UpdatePostIcon(const unsigned postmessages_count)
{
	// Taube setzen oder nicht (Post) 
	if (postmessages_count == 0)
		GetCtrl<ctrlImageButton>(3)->SetImage(GetImage(io_dat, 62));
	else
		GetCtrl<ctrlImageButton>(3)->SetImage(GetImage(io_dat, 59));

	// und Anzahl der Postnachrichten aktualisieren
	std::stringstream ss;
	ss << postmessages_count;
	GetCtrl<ctrlText>(4)->SetText(ss.str());
}

/// Neue Post-Nachricht eingetroffen
void dskGameInterface::CI_NewPostMessage(const unsigned postmessages_count)
{
	UpdatePostIcon(postmessages_count);

	// Tauben-Sound abspielen
	GetSound(sound_lst, 114)->Play(255,false);
}

/// Es wurde eine Postnachricht vom Spieler gel�scht
void dskGameInterface::CI_PostMessageDeleted(const unsigned postmessages_count)
{
	UpdatePostIcon(postmessages_count);
}
