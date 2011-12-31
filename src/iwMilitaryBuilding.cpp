// $Id: iwMilitaryBuilding.cpp 7711 2011-12-31 00:08:36Z marcus $
//
// Copyright (c) 2005 - 2011 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

///////////////////////////////////////////////////////////////////////////////
// Header
#include "main.h"
#include "iwMilitaryBuilding.h"

#include "VideoDriverWrapper.h"

#include "Loader.h"
#include "GameClient.h"
#include "MilitaryConsts.h"
#include "WindowManager.h"
#include "controls.h"
#include "iwDemolishBuilding.h"
#include "iwMsgbox.h"
#include "iwHelp.h"
#include "nobMilitary.h"
#include "nofPassiveSoldier.h"
#include "nofActiveSoldier.h"
#include "GameCommands.h"


///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/**
 *  Konstruktor von @p iwMilitaryBuilding.
 *
 *  @author OLiver
 */
iwMilitaryBuilding::iwMilitaryBuilding(GameWorldViewer * const gwv,nobMilitary *const building)
: IngameWindow(building->CreateGUIID(), (unsigned short)-2, (unsigned short)-2, 226, 194, _(BUILDING_NAMES[building->GetBuildingType()]), LOADER.GetImageN("resource", 41)),
	building(building), gwv(gwv) 
{
	// Schwert
	AddImage(0, 28, 39, LOADER.GetMapImageN(2298));
	AddImage(1, 28, 39, LOADER.GetMapImageN(2250 + GD_SWORD));

	// Schild
	AddImage(2, 196, 39, LOADER.GetMapImageN(2298));
	AddImage(3, 196, 39, LOADER.GetMapImageN(2250 + GD_SHIELDROMANS));

	// Hilfe
	AddImageButton(4,  16, 147, 30, 32, TC_GREY, LOADER.GetImageN("io",  21));
	// Abrei�en
	AddImageButton(5,  50, 147, 34, 32, TC_GREY, LOADER.GetImageN("io",  23));
	// Gold an/aus (227,226)
	AddImageButton(6,  90, 147, 32, 32, TC_GREY, LOADER.GetImageN("io", ((building->IsGoldDisabledVirtual())?226:227)));
	// "Gehe Zu Ort"
	AddImageButton(7, 179, 147, 30, 32, TC_GREY, LOADER.GetImageN("io", 107), _("Go to place"));

	// Geb�udebild
	AddImage(8, 117, 114, LOADER.GetNationImageN(building->GetNation(), 250 + 5*building->GetBuildingType()));
}

void iwMilitaryBuilding::Msg_PaintAfter()
{
	// Schatten des Geb�udes (muss hier gezeichnet werden wegen schwarz und halbdurchsichtig)
	LOADER.GetNationImageN(building->GetNation(), 250 + 5 * building->GetBuildingType()+1)->Draw(GetX()+117, GetY()+114, 0, 0, 0, 0, 0, 0, COLOR_SHADOW);

	// Schwarzer Untergrund f�r Goldanzeige
	DrawRectangle(GetX()+width/2-22*GOLD_COUNT[building->nation][building->size]/2,GetY()+60,22*GOLD_COUNT[building->nation][building->size],24,0x96000000);
	// Gold
	for(unsigned short i = 0; i < GOLD_COUNT[building->nation][building->size]; ++i)
		LOADER.GetMapImageN(2278)->Draw(GetX() + width / 2 - 22 * GOLD_COUNT[building->nation][building->size] / 2 + 12 + i*22, GetY() + 72, 0, 0, 0, 0, 0, 0, ( i >= building->coins ? 0xFFA0A0A0 : 0xFFFFFFFF) );

	// Schwarzer Untergrund f�r Soldatenanzeige
	DrawRectangle(GetX() + width / 2 - 22 * TROOPS_COUNT[building->nation][building->size] / 2, GetY() + 98 , 22 * TROOPS_COUNT[building->nation][building->size], 24, 0x96000000);
	
	// Sammeln aus der Rausgeh-Liste und denen, die wirklich noch drinne sind
	list<unsigned> soldiers;
	for(list<nofPassiveSoldier*>::iterator it = building->troops.begin();it.valid();++it)
		soldiers.push_back((*it)->GetRank());

	for(list<noFigure*>::iterator it = building->leave_house.begin();it.valid();++it)
	{
		if((*it)->GetGOT() == GOT_NOF_ATTACKER ||
			(*it)->GetGOT() == GOT_NOF_AGGRESSIVEDEFENDER ||
			(*it)->GetGOT() == GOT_NOF_DEFENDER ||
			(*it)->GetGOT() == GOT_NOF_PASSIVESOLDIER)
		{
			soldiers.insert_ordered(static_cast<nofSoldier*>(*it)->GetRank());
		}
	}

	// Soldaten zeichnen
	unsigned short i = 0;
	for(list<unsigned>::iterator it = soldiers.begin();it.valid();++it,++i)
		LOADER.GetMapImageN(2321 + *it)->Draw(GetX() + width / 2 - 22 * TROOPS_COUNT[building->nation][building->size] / 2 + 12 + i * 22, GetY() + 110, 0, 0, 0, 0, 0, 0);
} 


void iwMilitaryBuilding::Msg_ButtonClick(const unsigned int ctrl_id)
{
	switch(ctrl_id)
	{
	case 4: // Hilfe
		{
			WindowManager::inst().Show(new iwHelp(GUI_ID(CGI_HELPBUILDING+building->GetBuildingType()),_(BUILDING_NAMES[building->GetBuildingType()]),
				_(BUILDING_HELP_STRINGS[building->GetBuildingType()])));
		} break;
	case 5: // Geb�ude abbrennen
		{
			// Darf das Geb�ude abgerissen werden?
			if(!building->IsDemolitionAllowed())
			{
				// Messagebox anzeigen
				DemolitionNotAllowed();
			}
			else
			{
				// Abrei�en?
				Close();
				WindowManager::inst().Show(new iwDemolishBuilding(gwv, building));
			}
		} break;
	case 6: // Gold einstellen/erlauben
		{
			if(!GameClient::inst().IsReplayModeOn() && !GameClient::inst().IsPaused())
			{
				// visuell anzeigen
				building->StopGoldVirtual();
				// NC senden
				GAMECLIENT.AddGC(new gc::StopGold(building->GetX(), building->GetY()));
				// anderes Bild auf dem Button
				if(building->IsGoldDisabledVirtual())
					GetCtrl<ctrlImageButton>(6)->SetImage(LOADER.GetImageN("io", 226));
				else
					GetCtrl<ctrlImageButton>(6)->SetImage(LOADER.GetImageN("io", 227));
			}
		} break;
	case 7: // "Gehe Zu Ort"
		{
			gwv->MoveToMapObject(building->GetX(), building->GetY());
		} break;
	}
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void iwMilitaryBuilding::DemolitionNotAllowed()
{
	// Meldung ausw�hlen, je nach Einstellung
	std::string msg;
	switch(GameClient::inst().GetGGS().getSelection(ADDON_DEMOLITION_PROHIBITION))
	{
	default: assert(false); break;
	case 1: msg = _("Demolition ist not allowed because the building is under attack!"); break;
	case 2: msg = _("Demolition ist not allowed because the building is located in border area!"); break;
	}

	WindowManager::inst().Show(new iwMsgbox(_("Demolition not possible"), msg, NULL, MSB_OK, MSB_EXCLAMATIONRED));
}

