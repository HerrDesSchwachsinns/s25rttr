// $Id: iwAddons.cpp 5999 2010-02-11 09:53:02Z FloSoft $
//
// Copyright (c) 2005-2010 Settlers Freaks (sf-team at siedler25.org)
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
#include "iwAddons.h"
#include "controls.h"

#include "AddonManager.h"

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
 *  @author FloSoft
 */
iwAddons::iwAddons(ChangePolicy policy)
	: IngameWindow(CGI_ADDONS, 0xFFFF, 0xFFFF, 700, 500, _("Addon Settings"), LOADER.GetImageN("resource", 41), true),
	policy(policy)
{
	AddText(0, 20, 30, _("Additional features:"), COLOR_YELLOW, 0, NormalFont);

	if(policy != READONLY)
		AddTextButton(1,  20, height-40, 200, 22, TC_GREY, _("Apply Changes"), NormalFont);
	
	AddTextButton(2, 250, height-40, 200, 22, TC_RED1, _("Close Without Saving"), NormalFont);
	
	if(policy != READONLY)
		AddTextButton(3, 480, height-40, 200, 22, TC_GREY, _("Use S2 Defaults"), NormalFont);

	// Kategorien
	ctrlOptionGroup *optiongroup = AddOptionGroup(5, ctrlOptionGroup::CHECK, scale);
	// "Alle"
	optiongroup->AddTextButton(ADDONGROUP_ALL,  20, 50, 120, 22, TC_GREEN2, _("All"), NormalFont);
	// "Milit�r"
	optiongroup->AddTextButton(ADDONGROUP_MILITARY, 150, 50, 120, 22, TC_GREEN2, _("Military"), NormalFont);
	// "Wirtschaft"
	optiongroup->AddTextButton(ADDONGROUP_ECONOMY, 290, 50, 120, 22, TC_GREEN2, _("Economy"), NormalFont);
	// "Spielverhalten"
	optiongroup->AddTextButton(ADDONGROUP_GAMEPLAY, 430, 50, 120, 22, TC_GREEN2, _("Gameplay"), NormalFont);
	// "Sonstiges"
	optiongroup->AddTextButton(ADDONGROUP_OTHER, 560, 50, 120, 22, TC_GREEN2, _("Other"), NormalFont);

	optiongroup->SetSelection(ADDONGROUP_ALL, true);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author FloSoft
 */
iwAddons::~iwAddons()
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author FloSoft
 */
void iwAddons::Msg_ButtonClick(const unsigned int ctrl_id)
{
	switch(ctrl_id)
	{
	default:
		break;

	case 1: // Apply changes
		{
			if(policy == READONLY)
				Close();

			// Einstellungen in ADDONMANAGER �bertragen
			for(unsigned int i = 0; i < ADDONMANAGER.getCount(); ++i)
			{
				unsigned int status;
				const Addon *addon = ADDONMANAGER.getAddon(i, status);

				if(!addon)
					continue;

				bool failed = false;
				status = addon->getGuiStatus(this, 10 + 20*i, failed);
				if(!failed)
					ADDONMANAGER.setSelection(addon->getId(), status);
			}

			switch(policy)
			{
			default:
				break;
			case SETDEFAULTS:
				{
					ADDONMANAGER.SaveSettings();
				} break;
			case HOSTGAME:
				{
					// send message via msgboxresult
					MsgboxResult mbr = MSR_YES;
					parent->Msg_MsgBoxResult(GetID(), mbr);
				} break;
			}
			Close();
		} break;

	case 2: // Discard changes
		{
			Close();
		} break;

	case 3: // Load S2 Defaults
		{
			// Standardeinstellungen aufs Fenster �bertragen
			for(unsigned int i = 0; i < ADDONMANAGER.getCount(); ++i)
			{
				unsigned int status;
				const Addon *addon = ADDONMANAGER.getAddon(i, status);

				if(!addon)
					continue;

				addon->setGuiStatus(this, 10 + 20*i, addon->getDefaultStatus());
			}
		} break;
	}
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author FloSoft
 */
void iwAddons::Msg_OptionGroupChange(const unsigned int ctrl_id, const unsigned short selection)
{
	switch(ctrl_id)
	{
	case 5: // richtige Kategorie anzeigen
		{
			unsigned short y = 90;
			for(unsigned int i = 0; i < ADDONMANAGER.getCount(); ++i)
			{
				unsigned int id = 10 + 20*i;
				unsigned int status;
				const Addon *addon = ADDONMANAGER.getAddon(i, status);

				if(!addon)
					continue;

				unsigned int groups = addon->getGroups();
				if( (groups & selection) != selection)
				{
					addon->hideGui(this, id);
					continue;
				}

				addon->createGui(this, id, y, (policy == READONLY), status);
			}		
		} break;
	}
}
