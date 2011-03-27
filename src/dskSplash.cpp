// $Id: dskSplash.cpp 7091 2011-03-27 10:57:38Z OLiver $
//
// Copyright (c) 2005 - 2010 Settlers Freaks (sf-team at siedler25.org)
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
#include <stdafx.h>
#include "main.h"
#include "dskSplash.h"

#include "WindowManager.h"
#include "Loader.h"
#include "VideoDriverWrapper.h"
#include "GameManager.h"

#include "dskMainMenu.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/** @class dskSplash
 *
 *  Klasse des Splashscreen Desktops.
 *
 *  @author OLiver
 */

///////////////////////////////////////////////////////////////////////////////
/**
 *  Konstruktor von @p dskSplash.
 *
 *  @author OLiver
 *  @author FloSoft
 */
dskSplash::dskSplash() : Desktop(LOADER.GetImageN("splash", 0))
{
	GAMEMANAGER.SetCursor(CURSOR_NONE);
	AddTimer(0, 5000);
}

dskSplash::~dskSplash()
{
	GAMEMANAGER.SetCursor();
}

void dskSplash::Msg_Timer(const unsigned int ctrl_id)
{
	// Hauptmen� zeigen
	WindowManager::inst().Switch(new dskMainMenu);
}

bool dskSplash::Msg_LeftDown(const MouseCoords& mc)
{
	// Hauptmen� zeigen
	WindowManager::inst().Switch(new dskMainMenu, NULL, true);

	return true;
} 

