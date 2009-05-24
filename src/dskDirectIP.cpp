// $Id: dskDirectIP.cpp 4933 2009-05-24 12:29:23Z OLiver $
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
#include "dskDirectIP.h"

#include "WindowManager.h"
#include "Loader.h"
#include "GameProtocol.h"

#include "dskMultiPlayer.h"

#include "iwDirectIPCreate.h"
#include "iwDirectIPConnect.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/**
 *  Konstruktor von @p dskDirectIP.
 *
 *  @author FloSoft
 */
dskDirectIP::dskDirectIP(void) : Desktop(GetImage(backgrounds, 0))
{
	// Version
	AddVarText(0, 0, 600, _("The Settlers II.5 RTTR, v%s-%s"), COLOR_YELLOW, 0 | glArchivItem_Font::DF_BOTTOM, NormalFont, 2, GetWindowVersion(), GetWindowRevision());

	// URL
	AddText(1, 400, 600, _("http://www.siedler25.org"), COLOR_GREEN, glArchivItem_Font::DF_CENTER | glArchivItem_Font::DF_BOTTOM, NormalFont);

	// Copyright
	AddText(2, 800, 600, "� 2005 - 2009 Settlers Freaks", COLOR_YELLOW, glArchivItem_Font::DF_RIGHT | glArchivItem_Font::DF_BOTTOM, NormalFont);

	AddTextButton(3, 115, 180, 220, 22, TC_GREEN2, _("Create Game"),NormalFont);
	AddTextButton(4, 115, 210, 220, 22, TC_GREEN2, _("Join Game"),NormalFont);

	// "Zur�ck"
	AddTextButton(5, 115, 250, 220, 22, TC_RED1, _("Back"),NormalFont);
}


void dskDirectIP::Msg_ButtonClick(const unsigned int ctrl_id)
{
	switch(ctrl_id)
	{
	case 3: // "Erstellen"
		{
			WindowManager::inst().Show(new iwDirectIPCreate(NP_DIRECT));
		} break;
	case 4: // "Verbinden"
		{
			WindowManager::inst().Show(new iwDirectIPConnect(NP_DIRECT));
		} break;
	case 5: // "Zur�ck"
		{
		  WindowManager::inst().Switch(new dskMultiPlayer);
		} break;
	}
}
