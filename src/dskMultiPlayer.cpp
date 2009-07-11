// $Id: dskMultiPlayer.cpp 5247 2009-07-11 19:13:17Z FloSoft $
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
#include "dskMultiPlayer.h"

#include "WindowManager.h"
#include "Loader.h"

#include "dskMainMenu.h"
#include "dskDirectIP.h"
#include "iwLobbyConnect.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/** @class dskMultiPlayer
 *
 *  Klasse des Multispieler Desktops.
 *
 *  @author OLiver
 */

///////////////////////////////////////////////////////////////////////////////
/**
 *  Konstruktor von @p dskMultiPlayer.
 *
 *  @author OLiver
 *  @author FloSoft
 */
dskMultiPlayer::dskMultiPlayer(void) : Desktop(LOADER.GetImageN("menu", 0))
{
	// Version
	AddVarText(0, 0, 600, _("The Settlers II.5 RTTR, v%s-%s"), COLOR_YELLOW, 0 | glArchivItem_Font::DF_BOTTOM, NormalFont, 2, GetWindowVersion(), GetWindowRevision());
	// URL
	AddText(1, 400, 600, _("http://www.siedler25.org"), COLOR_GREEN, glArchivItem_Font::DF_CENTER | glArchivItem_Font::DF_BOTTOM, NormalFont);
	// Copyright
	AddText(2, 800, 600, "� 2005 - 2009 Settlers Freaks", COLOR_YELLOW, glArchivItem_Font::DF_RIGHT | glArchivItem_Font::DF_BOTTOM, NormalFont);

	// "Internet - Lobby"
	AddTextButton(3, 115, 180, 220, 22, TC_GREEN2, _("Internet Lobby"),NormalFont);
	// "Netzwerk / LAN"
	AddTextButton(4, 115, 210, 220, 22, TC_GREEN2, _("Network/LAN"),NormalFont);
	// "Direkte IP"
	AddTextButton(5, 115, 250, 220, 22, TC_GREEN2, _("Direct IP"),NormalFont);
	// "Zur�ck"
	AddTextButton(6, 115, 290, 220, 22, TC_RED1, _("Back"),NormalFont);
}



void dskMultiPlayer::Msg_ButtonClick(const unsigned int ctrl_id)
{
	switch(ctrl_id)
	{
	case 3: // Lobby
		{
			WindowManager::inst().Show(new iwLobbyConnect, true);
		} break;
	case 4: // Local Area Network
		{
		} break;
	case 5: // Direct IP
		{
			WindowManager::inst().Switch(new dskDirectIP);
		} break;
	case 6: // Zur�ck
		{
			WindowManager::inst().Switch(new dskMainMenu);
		} break;
	}
}
