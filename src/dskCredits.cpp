// $Id: dskCredits.cpp 5839 2010-01-03 13:10:52Z Demophobie $
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
#include "dskCredits.h"

#include "WindowManager.h"
#include "Loader.h"

#include "dskMainMenu.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/** @class dskCredits
 *
 *  Klasse des Credits Desktops.
 *
 *  @author FloSoft
 */

///////////////////////////////////////////////////////////////////////////////
/**
 *  Konstruktor von @p dskCredits.
 *
 *  @author FloSoft
 */
dskCredits::dskCredits(void) : Desktop(LOADER.GetImageN("setup013", 0))
{

	// "Zur�ck"
	AddTextButton(0, 300, 560, 200, 22, TC_RED1, _("Back"),NormalFont);

	// "Die Siedler II.5 RTTR"
	AddText(1, 400, 10, _("The Settlers II.5 RTTR"), COLOR_YELLOW, glArchivItem_Font::DF_CENTER, LargeFont);

	// "Credits"
	AddText(2, 400, 33, _("Credits"), COLOR_YELLOW, glArchivItem_Font::DF_CENTER, LargeFont);
	
	/// @todo Bilder von uns + Siedler laufen unten am Bildschirmrand

	// "Programmiert von:"
	AddText(3, 400, 90, _("Programming"), COLOR_RED, glArchivItem_Font::DF_CENTER, LargeFont);
	// "OLiver"
	AddText(4, 400, 120, "Oliver Siebert (OLiver)", COLOR_YELLOW, glArchivItem_Font::DF_CENTER, NormalFont);
	// "FloSoft"
	AddText(5, 400, 140, "Florian Doersch (FloSoft)", COLOR_YELLOW, glArchivItem_Font::DF_CENTER, NormalFont);
	// "jh"
	AddText(6, 400, 160, "Jan-Henrik Kluth (jh)", COLOR_YELLOW, glArchivItem_Font::DF_CENTER, NormalFont);

	// "Qualit�tssicherung"
	AddText( 7, 400, 200, _("Quality assurance"), COLOR_RED, glArchivItem_Font::DF_CENTER, LargeFont);
	// "Demophobie"
	AddText(8, 400, 230, "Patrick Haak (Demophobie)", COLOR_YELLOW, glArchivItem_Font::DF_CENTER, NormalFont);
	// "NastX"
	AddText(9, 400, 250, "Jonas Trampe (NastX)", COLOR_YELLOW, glArchivItem_Font::DF_CENTER, NormalFont);


	// "Sonstige Unterst�tzung"
	AddText(10, 400, 290, _("Other Support"), COLOR_RED, glArchivItem_Font::DF_CENTER, LargeFont);
	// "muhahahaha"
	AddText(11, 330, 320, "muhahahaha", COLOR_YELLOW, glArchivItem_Font::DF_CENTER, NormalFont);
	// "Zwoks-Stef"
	AddText(12, 470, 320, "Zwoks-Stef", COLOR_YELLOW, glArchivItem_Font::DF_CENTER, NormalFont);
	// "Devil"
	AddText(13, 330, 340, "Devil", COLOR_YELLOW, glArchivItem_Font::DF_CENTER, NormalFont);
	// "Cat666"
	AddText(14, 470, 340, "Cat666", COLOR_YELLOW, glArchivItem_Font::DF_CENTER, NormalFont);
	// "Sotham"
	AddText(15, 330, 360, "Sotham", COLOR_YELLOW, glArchivItem_Font::DF_CENTER, NormalFont);
	// "liwo"
	AddText(16, 470, 360, "liwo", COLOR_YELLOW, glArchivItem_Font::DF_CENTER, NormalFont);
	// "Airhardt"
	AddText(17, 330, 380, "Airhardt", COLOR_YELLOW, glArchivItem_Font::DF_CENTER, NormalFont);
    	// "Divan"
    	AddText(18, 470, 380, "Divan", COLOR_YELLOW, glArchivItem_Font::DF_CENTER, NormalFont);
    	// "Fenan"
    	AddText(19, 330, 400, "Fenan", COLOR_YELLOW, glArchivItem_Font::DF_CENTER, NormalFont);

	// "Spender:"
	AddText( 20, 400, 440, _("Donators"), COLOR_RED, glArchivItem_Font::DF_CENTER, LargeFont);
	// "diverse anonyme Spenden"
	AddText(21, 400, 470, _("various anonymous donators"), COLOR_YELLOW, glArchivItem_Font::DF_CENTER, NormalFont);
	// "morlock"
	AddText(22, 400, 490, "morlock", COLOR_YELLOW, glArchivItem_Font::DF_CENTER, NormalFont);
	// "Jan Montag"
	AddText(23, 400, 510, "Jan Montag", COLOR_YELLOW, glArchivItem_Font::DF_CENTER, NormalFont);

	// "Vielen Dank an alle Spender!"
	AddText(24, 400, 530, _("Thank you for your donations!"), COLOR_RED, glArchivItem_Font::DF_CENTER, NormalFont);
}

void dskCredits::Msg_ButtonClick(const unsigned int ctrl_id)
{
	switch(ctrl_id)
	{
	case 0: // "Zur�ck"
		{
			WindowManager::inst().Switch(new dskMainMenu);
		} break;
	}
}
