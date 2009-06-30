// $Id: iwBaseWarehouse.cpp 5148 2009-06-30 21:02:09Z OLiver $
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
#include "iwBaseWarehouse.h"

#include "Loader.h"

#include "VideoDriverWrapper.h"
#include "controls.h"
#include "GameClient.h"
#include "iwDemolishBuilding.h"
#include "WindowManager.h"
#include "iwHelp.h"
#include "GameCommands.h"

#include "nobBaseWarehouse.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/**
 *  Konstruktor von @p iwHQ.
 *
 *  @author OLiver
 */
iwBaseWarehouse::iwBaseWarehouse(GameWorldViewer * const gwv,const char *const title,
								 unsigned char page_count, 
								 nobBaseWarehouse *wh)
 : iwWares(wh->CreateGUIID(),0xFFFE, 0xFFFE, 167, 416, title, page_count, true, NormalFont, wh->GetInventory()), gwv(gwv), wh(wh)
{
	// Basisinitialisierungs�nderungen
	background = GetImage(resource_dat, 41);

	// Auswahl f�r Auslagern/Einlagern Verbieten-Kn�pfe
	ctrlOptionGroup *group = AddOptionGroup(10, ctrlOptionGroup::CHECK);
	// nix tun
	group->AddImageButton(0, 16, 335, 32, 32, TC_GREY, GetImage(io_dat, 214), _("do nothing"));
	// Auslagern
	group->AddImageButton(1, 52, 335, 32, 32, TC_GREY, GetImage(io_dat, 211), _("Take out of store"));
	// Einlagern verbieten
	group->AddImageButton(2, 86, 335, 32, 32, TC_GREY, GetImage(io_dat, 212), _("Stop storage"));
	// nix tun ausw�hlen
	group->SetSelection(0);

	// Alle ausw�hlen bzw setzen!
	AddImageButton(11, 122, 335, 32, 32,TC_GREY, GetImage(io_dat, 223), _("Select all"));
	// "Gehe Zu Ort"
	AddImageButton(13, 122, 369, 32, 32,TC_GREY, GetImage(io_dat, 107), _("Go to place"));

	// Ein/Auslager Overlays entsprechend setzen
	// bei Replays die reellen Einstellungen nehmen, weils die visuellen da logischweise nich gibt!
	ctrlImage * image;
	for(unsigned char category = 0;category<2;++category)
	{
		unsigned count = (category == 0)?35:30;
		for(unsigned i = 0;i<count;++i)
		{
			// Einlagern verbieten-Bild (de)aktivieren
			image = GetCtrl<ctrlGroup>(100+category)->GetCtrl<ctrlImage>(400+i);
			if(image)
				image->SetVisible(GAMECLIENT.IsReplayModeOn()?wh->CheckRealInventorySettings(category,2,i):
				wh->CheckVisualInventorySettings(category,2,i));

			// Auslagern-Bild (de)aktivieren
			image = GetCtrl<ctrlGroup>(100+category)->GetCtrl<ctrlImage>(500+i);
			if(image)
				image->SetVisible(GAMECLIENT.IsReplayModeOn()?wh->CheckRealInventorySettings(category,4,i):
				wh->CheckVisualInventorySettings(category,4,i));
		}
	}

	// Lagerhaus oder Hafengeb�ude?
	if(wh->GetGOT() == GOT_NOB_STOREHOUSE || wh->GetGOT() == GOT_NOB_HARBORBUILDING)
	{

		// Abbrennbutton hinzuf�gen
		// "Bl�ttern" in Bretter stauchen und verschieben
		GetCtrl<ctrlButton>(0)->SetWidth(32);
		GetCtrl<ctrlButton>(0)->Move(86,369,true);

		AddImageButton(1, 52, 369, 32, 32, TC_GREY, GetImage(io_dat,  23), _("Demolish house"));
	}
}


void iwBaseWarehouse::Msg_Group_ButtonClick(const unsigned int group_id, const unsigned int ctrl_id)
{
	switch(group_id)
	{
	default: // an Basis weiterleiten
		{
			iwWares::Msg_Group_ButtonClick(group_id,ctrl_id);
		} break;
	case 100: // Waren
	case 101: // Figuren
		{
			ctrlOptionGroup *optiongroup = GetCtrl<ctrlOptionGroup>(10);

			unsigned int data = 0;
			switch(optiongroup->GetSelection())
			{
			case 0: data = 0; break;
			case 1: data = 4; break;
			case 2: data = 2; break;
			}
			if(data != 0)
			{
				// Nicht bei Replays setzen
				if(GAMECLIENT.AddGC(new gc::ChangeInventorySetting(wh->GetX(),wh->GetY(),page,data,ctrl_id - 100)))
					// optisch schonmal setzen
					ChangeOverlay(ctrl_id - 100, data);;
			}
		} break;
	}
}

void iwBaseWarehouse::Msg_ButtonClick(const unsigned int ctrl_id)
{
	switch(ctrl_id)
	{
	case 1: // Geb�ude abrei�en
		{
			// Abrei�en?
			Close();
			WindowManager::inst().Show(new iwDemolishBuilding(gwv,GOT_NOB_STOREHOUSE,wh->GetX(), wh->GetY(),wh->GetBuildingType(),wh->GetNation(),wh->CreateGUIID()));
		} break;
	case 11: // "Alle ausw�hlen"
		{
			if(this->page < 2)
			{
				ctrlOptionGroup *optiongroup = GetCtrl<ctrlOptionGroup>(10);
				unsigned int data = 0;
				switch(optiongroup->GetSelection())
				{
				case 0: data = 0; break;
				case 1: data = 4; break;
				case 2: data = 2; break;
				}
				if(data != 0)
				{
					// Nicht bei Replays setzen
					if(GAMECLIENT.AddGC(new gc::ChangeAllInventorySettings(wh->GetX(),wh->GetY(),page,data)))
					{
						// optisch setzen
						unsigned short count = ((page == 0)?WARE_TYPES_COUNT:JOB_TYPES_COUNT);
						for(unsigned char i = 0; i < count; ++i)
							ChangeOverlay(i, data);
					}
				}

			}
		} break;
	case 12: // "Hilfe"
		{
			WindowManager::inst().Show(new iwHelp(GUI_ID(CGI_HELPBUILDING+wh->GetBuildingType()),_(BUILDING_NAMES[wh->GetBuildingType()]),
				_(BUILDING_HELP_STRINGS[wh->GetBuildingType()])));
		} break;
	case 13: // "Gehe Zu Ort"
		{
			gwv->MoveToMapObject(wh->GetX(), wh->GetY());
		} break;
	default: // an Basis weiterleiten
		{
			iwWares::Msg_ButtonClick(ctrl_id);
		} break;
	}
}


///////////////////////////////////////////////////////////////////////////////
/**
 *  Overlay �ndern.
 *
 *  @param[in] i    Item-Nr (0-29)
 *  @param[in] what Welcher Status soll ge�ndert werden (2 = Einlagern verbieten, 4 = Auslagern)
 * 
 *  @author FloSoft
 */
void iwBaseWarehouse::ChangeOverlay(unsigned int i, unsigned int what)
{
	ctrlImage *image;

	// Status �ndern
	wh->ChangeVisualInventorySettings(page,what,i);

	// Einlagern verbieten-Bild (de)aktivieren
	image = GetCtrl<ctrlGroup>(100+this->page)->GetCtrl<ctrlImage>(400+i);
	if(image)
		image->SetVisible(wh->CheckVisualInventorySettings(page,2,i));

	// Auslagern-Bild (de)aktivieren
	image = GetCtrl<ctrlGroup>(100+this->page)->GetCtrl<ctrlImage>(500+i);
	if(image)
		image->SetVisible(wh->CheckVisualInventorySettings(page,4,i));
}
