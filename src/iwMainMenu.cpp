// $Id: iwMainMenu.cpp 4996 2009-06-03 19:01:25Z OLiver $
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
#include "iwMainMenu.h"

#include "Loader.h"
#include "WindowManager.h"

#include "iwDistribution.h"
#include "iwTransport.h"
#include "iwTools.h"
#include "iwMilitary.h"
#include "iwBuildOrder.h"
#include "iwOptionsWindow.h"
#include "iwInventory.h"
#include "iwBuildings.h"
#include "iwBuildingProductivities.h"
#include "iwStatistics.h"
#include "iwDiplomacy.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/**
 *  Konstruktor von @p iwMainMenu.
 *
 *  @author OLiver
*/
iwMainMenu::iwMainMenu(void)
	: IngameWindow(CGI_MAINSELECTION, 0xFFFF, 0xFFFF, 190, 286, _("Main selection"), GetImage(io_dat, 5))
{
	// Verteilung
	AddImageButton( 0,  12,  22,  53, 44, TC_GREY, GetImage(io_dat, 134), _("Distribution of goods"));
	// Transport
	AddImageButton( 1,  68,  22,  53, 44, TC_GREY, GetImage(io_dat, 198), _("Transport"));
	// Werkzeugproduktion
	AddImageButton( 2, 124,  22,  53, 44, TC_GREY, GetImage(io_dat, 137), _("Tools"));

	// Statistiken
	AddImageButton( 3,  12,  70,  39, 44, TC_GREY, GetImage(io_dat, 166), _("General statistics"));
	AddImageButton( 4,  54,  70,  39, 44, TC_GREY, GetImage(io_dat, 135), _("Merchandise statistics"));
	AddImageButton( 5,  96,  70,  39, 44, TC_GREY, GetImage(io_dat, 132), _("Buildings"));

	// Inventur
	AddImageButton( 6, 138,  70,  39, 44, TC_GREY, GetImage(io_dat, 214), _("Stock"));

	// Gebäude
	AddImageButton( 7,  12, 118,  53, 44, TC_GREY, GetImage(io_dat, 136), _("Productivity"));
	// Militär
	AddImageButton( 8,  68, 118,  53, 44, TC_GREY, GetImage(io_dat, 133), _("Military"));
	// Schiffe
	AddImageButton( 9, 124, 118,  53, 44, TC_GREY, GetImage(io_dat, 175), _("Ship register"));

	// Gebäude
	AddImageButton( 10,  12, 166,  53, 44, TC_GREY, GetImage(io_dat, 24), _("Series sequence"));
	// Diplomatie (todo: besseres Bild suchen)
	AddImageButton( 11,  68, 166,  53, 44, TC_GREY, GetImage(io_dat, 190), _("Diplomacy"));
	//// Schiffe
	//AddImageButton( 9, 124, 118,  53, 44, TC_GREY, GetImage(io_dat, 175), _("Ship register"));

	// Optionen
	AddImageButton(30,  12, 231, 165, 32, TC_GREY, GetImage(io_dat,  37), _("Options"));
}


void iwMainMenu::Msg_ButtonClick(const unsigned int ctrl_id)
{
	switch(ctrl_id)
	{
	case 0: // Verteilung
		{
			WindowManager::inst().Show(new iwDistribution);
		} break;
	case 1: // Transport
		{
			WindowManager::inst().Show(new iwTransport);
		} break;
	case 2: // Werkzeugproduktion
		{
			WindowManager::inst().Show(new iwTools);
		} break;
	case 3: // Statistik
		{
			WindowManager::inst().Show(new iwStatistics);
		} break;
	case 5: // Gebäudestatistik
		{
			WindowManager::inst().Show(new iwBuildings);
		} break;

	case 6: // Inventur
		{
			WindowManager::inst().Show(new iwInventory);
		} break;

	case 7: // Produktivitäten
		{
			WindowManager::inst().Show(new iwBuildingProductivities);
			//WindowManager::inst().Show(new iwBuildOrder);
		} break;
	case 8: // Militär
		{
			WindowManager::inst().Show(new iwMilitary);
		} break;
	case 10: // Baureihenfolge
		{
			WindowManager::inst().Show(new iwBuildOrder);
		} break;
	case 11: // Diplomatie
		{
			WindowManager::inst().Show(new iwDiplomacy);
		} break;
	
	
	case 30: // Optionen
		{
			WindowManager::inst().Show(new iwOptionsWindow);
		} break;
	}
} 
