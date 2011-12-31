// $Id: iwDemolishBuilding.cpp 7711 2011-12-31 00:08:36Z marcus $
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

iwDemolishBuilding::iwDemolishBuilding(GameWorldViewer * const gwv,const noBaseBuilding *building)
: IngameWindow(building->CreateGUIID(), 0xFFFE,0xFFFE,200,200,_("Demolish?"),LOADER.GetImageN("resource", 41)), gwv(gwv), building(building)
{
	
	// Ja
	AddImageButton(0,14,140,66,40,TC_RED1,LOADER.GetImageN("io", 32));
	// Nein
	AddImageButton(1,82,140,66,40,TC_GREY,LOADER.GetImageN("io", 40));
	// Gehe zum Standort
	AddImageButton(2,150,140,36,40,TC_GREY,LOADER.GetImageN("io", 107));
	// Geb�udebild
	AddImage(3, 104, 109, building->GetBuildingImage());
	// Geb�udename
	AddText(4,100,125,_(BUILDING_NAMES[building->GetBuildingType()]),0xFFFFFF00,glArchivItem_Font::DF_CENTER,NormalFont);
}

void iwDemolishBuilding::Msg_ButtonClick(const unsigned int ctrl_id)
{
	switch(ctrl_id)
	{
	case 0:
		{
			const GO_Type got = building->GetGOT();
			if(got == GOT_NOB_MILITARY || got == GOT_NOB_STOREHOUSE || got == GOT_NOB_USUAL  || got == GOT_BUILDINGSITE)
			{
				GameClient::inst().AddGC(new gc::DestroyBuilding(building->GetX(), building->GetY()));
			}
			else if(got == GOT_FLAG)
				// Flagge (mitsamt Geb�ude) wegrei�en
				GameClient::inst().AddGC(new gc::DestroyFlag(building->GetX(), building->GetY()));

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
			gwv->MoveToMapObject(building->GetX(), building->GetY());
		} break;
	}
}

void iwDemolishBuilding::Msg_PaintBefore()
{
	// Schatten des Geb�udes (muss hier gezeichnet werden wegen schwarz und halbdurchsichtig)
	glArchivItem_Bitmap *bitmap = building->GetBuildingImageShadow();

	if(bitmap)
		bitmap->Draw(GetX()+104, GetY()+109, 0, 0, 0, 0, 0, 0, COLOR_SHADOW);

}

