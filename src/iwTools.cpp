// $Id: iwTools.cpp 4784 2009-05-02 20:43:44Z OLiver $
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
#include "iwTools.h"

#include "Loader.h"
#include "GameClient.h"
#include "controls.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/**
 *  Konstruktor von @p iwTools.
 *
 *  @author OLiver
 */
iwTools::iwTools(void)
	: IngameWindow(CGI_TOOLS, 0xFFFE, 0xFFFE, 166, 432, _("Tools"), GetImage(io_dat, 5)),
	settings_changed(false)
{
	// Einzelne Balken
	AddProgress( 0, 17,  25, 132, 26, TC_GREY, 141, 140, 5, _("Tongs"), 4,4, 0, _("Less often"), _("More often"));
	AddProgress( 1, 17,  53, 132, 26, TC_GREY, 145, 144, 5, _("Axe"), 4,4, 0, _("Less often"), _("More often"));
	AddProgress( 2, 17,  81, 132, 26, TC_GREY, 147, 146, 5, _("Saw"), 4,4, 0, _("Less often"), _("More often"));
	AddProgress( 3, 17, 109, 132, 26, TC_GREY, 149, 148, 5, _("Pick-axe"), 4,4, 0, _("Less often"), _("More often"));
	AddProgress( 4, 17, 137, 132, 26, TC_GREY, 143, 142, 5, _("Hammer"), 4,4, 0, _("Less often"), _("More often"));
	AddProgress( 5, 17, 165, 132, 26, TC_GREY, 151, 150, 5, _("Shovel"), 4,4, 0, _("Less often"), _("More often"));
	AddProgress( 6, 17, 193, 132, 26, TC_GREY, 153, 152, 5, _("Crucible"), 4,4, 0, _("Less often"), _("More often"));
	AddProgress( 7, 17, 221, 132, 26, TC_GREY, 155, 154, 5, _("Rod and line"), 4,4, 0, _("Less often"), _("More often"));
	AddProgress( 8, 17, 249, 132, 26, TC_GREY, 157, 156, 5, _("Scythe"), 4,4, 0, _("Less often"), _("More often"));
	AddProgress( 9, 17, 277, 132, 26, TC_GREY, 159, 158, 5, _("Cleaver"), 4,4, 0, _("Less often"), _("More often"));
	AddProgress(10, 17, 305, 132, 26, TC_GREY, 161, 160, 5, _("Rolling pin"), 4,4, 0, _("Less often"), _("More often"));
	AddProgress(11, 17, 333, 132, 26, TC_GREY, 163, 162, 5, _("Bow"), 4,4, 0, _("Less often"), _("More often"));

	// Info
	AddImageButton(12,  18, 384, 30, 32, TC_GREY, GetImage(io_dat,  21), _("Help"));
	// Standard
	AddImageButton(13, 118, 384, 30, 32, TC_GREY, GetImage(io_dat, 191), _("Default"));

	// Einstellungen festlegen
	for(unsigned char i = 0; i < 12; ++i)
		GetCtrl<ctrlProgress>(i)->SetPosition(GAMECLIENT.visual_settings.tools_settings[i]);

	// Netzwerk-�bertragungs-Timer
	AddTimer(14,2000);
}

iwTools::~iwTools()
{
	TransmitSettings();
}

void iwTools::TransmitSettings()
{
	// Wurden Einstellungen ver�ndert?
	if(settings_changed)
	{
		// Einstellungen speichern
		for(unsigned char i = 0; i < 12; ++i)
			GAMECLIENT.visual_settings.tools_settings[i] = 
			(unsigned char)GetCtrl<ctrlProgress>(i)->GetPosition();

		GAMECLIENT.NC_ChangeTools(GAMECLIENT.visual_settings.tools_settings);

		settings_changed = false;
	}
}

void iwTools::Msg_ButtonClick(const unsigned int ctrl_id)
{
	switch(ctrl_id)
	{
	case 13: // Standard
		{
			for(unsigned char i = 0; i < 12; ++i)
				GetCtrl<ctrlProgress>(i)->SetPosition(GAMECLIENT.default_settings.tools_settings[i]);
		} break;
	}
}

void iwTools::Msg_ProgressChange(const unsigned int ctrl_id, const unsigned short position)
{
	// Einstellungen wurden ge�ndert
	settings_changed = true;
}

void iwTools::Msg_Timer(const unsigned int ctrl_id)
{
	if(GAMECLIENT.IsReplayModeOn())
		// Im Replay aktualisieren wir die Werte 
		UpdateSettings();
	else
		// Im normalen Spielmodus schicken wir den ganzen Spa� ab
		TransmitSettings();
}

void iwTools::UpdateSettings()
{
	// Einstellungen festlegen
	for(unsigned i = 0;i<12;++i)
		GetCtrl<ctrlProgress>(i)->SetPosition(GAMECLIENT.visual_settings.military_settings[i]);
}
