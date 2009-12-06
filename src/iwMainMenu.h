// $Id: iwMainMenu.h 5763 2009-12-06 17:37:44Z OLiver $
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
#ifndef iwMAINMENU_H_INCLUDED
#define iwMAINMENU_H_INCLUDED

#pragma once

#include "IngameWindow.h"

class GameWorldViewer;
class dskGameInterface;

class iwMainMenu : public IngameWindow
{
	// Interface f�r das Spiel (brauchen einige Fenster)
	GameWorldViewer * gwv;
	/// Spielinterface-Fenster (brauchen einige Fenster)
	dskGameInterface * gi;

public:
	/// Konstruktor von @p iwMainMenu.
	iwMainMenu(GameWorldViewer * const gwv, dskGameInterface * const gi);
private:

	void Msg_ButtonClick(const unsigned int ctrl_id);
};

#endif // !iwMAINMENU_H_INCLUDED
