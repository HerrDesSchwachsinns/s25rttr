// $Id: ctrlBuildingIcon.cpp 6458 2010-05-31 11:38:51Z FloSoft $
//
// Copyright (c) 2005 - 2010 Settlers Freaks (sf-team at siedler25.org)
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
#include "ctrlBuildingIcon.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/**
 *  Konstruktor von @p ctrlBuildingIcon.
 *
 *  @author OLiver
 */
ctrlBuildingIcon::ctrlBuildingIcon(Window * const parent,
								   const unsigned int id,
								   const unsigned short x,
								   const unsigned short y,
								   const BuildingType type,
								   const Nation nation,
								   const unsigned short size,
								   const std::string& tooltip)
	: ctrlButton(parent, id, x, y, size, size, TC_GREY,tooltip),
	type(type), nation(nation), size(size)
{}

///////////////////////////////////////////////////////////////////////////////
/**
 *  zeichnet das Fenster.
 *
 *  @author OLiver
 */
bool ctrlBuildingIcon::Draw_(void)
{
	// Pr�fen, ob bei gehighlighteten Button die Maus auch noch �ber dem Button ist
	TestMouseOver();

	if(state == BUTTON_HOVER || state == BUTTON_PRESSED)
		LOADER.GetImageN("io", 0)->Draw(GetX(), GetY(), size, size, size, size, 0, 0);

	glArchivItem_Bitmap * image = LOADER.GetImageN(NATION_ICON_IDS[nation], type);
	if(type == BLD_CHARBURNER)
		image = LOADER.GetImageN("charburner",33+nation);
	image->Draw(GetX() + size / 2, GetY() + size / 2, 0, 0, 0, 0, 0, 0, (state == BUTTON_PRESSED ? 0xFFFFFF00 : 0xFFFFFFFF));

	return true;
}


void ctrlBuildingIcon::DrawContent() const
{
}
