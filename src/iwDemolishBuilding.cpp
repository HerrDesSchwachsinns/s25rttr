// $Id: iwDemolishBuilding.cpp 6458 2010-05-31 11:38:51Z FloSoft $
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
#include "iwDemolishBuilding.h"

#include "Loader.h"
#include "VideoDriverWrapper.h"
#include "GameClient.h"
#include "WindowManager.h"
#include "GameWorld.h"
#include "iwMsgbox.h"
#include "GameCommands.h"


///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

/// Konstruktor von @p iwBuilding.
iwDemolishBuilding::iwDemolishBuilding(GameWorldViewer * const gwv,const GO_Type got,const unsigned short building_x, const unsigned short building_y,const BuildingType building, const Nation nation, const unsigned guiid)
: IngameWindow(guiid,0xFFFE,0xFFFE,200,200,_("Demolish?"),LOADER.GetImageN("resource", 41)), gwv(gwv),  got(got),
			   building_x(building_x),  building_y(building_y), building(building), nation(nation)
{
	
	// Ja
	AddImageButton(0,14,140,66,40,TC_RED1,LOADER.GetImageN("io", 32));
	// Nein
	AddImageButton(1,82,140,66,40,TC_GREY,LOADER.GetImageN("io", 40));
	// Gehe zum Standort
	AddImageButton(2,150,140,36,40,TC_GREY,LOADER.GetImageN("io", 107));
	// Geb�udebild
	AddImage(3, 104, 109, LOADER.GetNationImageN(nation, 250+5*building));
	// Geb�udename
	AddText(4,100,125,_(BUILDING_NAMES[building]),0xFFFFFF00,glArchivItem_Font::DF_CENTER,NormalFont);
}

void iwDemolishBuilding::Msg_ButtonClick(const unsigned int ctrl_id)
{
	switch(ctrl_id)
	{
	case 0:
		{
			if(got == GOT_NOB_MILITARY || got == GOT_NOB_STOREHOUSE || got == GOT_NOB_USUAL  || got == GOT_BUILDINGSITE)
			{
				GameClient::inst().AddGC(new gc::DestroyBuilding(building_x,building_y));
			}
			else if(got == GOT_FLAG)
				// Flagge (mitsamt Geb�ude) wegrei�en
				GameClient::inst().AddGC(new gc::DestroyFlag(building_x,building_y));

			Close();

		} break;
	case 1:
		{
			// Einfach schlie�en
			Close();
		} break;
	case 2:
		{
			// Zum Ort gehen
			gwv->MoveToMapObject(building_x, building_y);
		} break;
	}
}

void iwDemolishBuilding::Msg_PaintBefore()
{
	// Schatten des Geb�udes (muss hier gezeichnet werden wegen schwarz und halbdurchsichtig)
	glArchivItem_Bitmap *bitmap = LOADER.GetNationImageN(nation, 250+5*building+1);

	if(bitmap)
		bitmap->Draw(GetX()+104, GetY()+109, 0, 0, 0, 0, 0, 0, COLOR_SHADOW);

}

