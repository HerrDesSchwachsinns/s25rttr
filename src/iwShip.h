// $Id: iwShip.h 6582 2010-07-16 11:23:35Z FloSoft $
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
#ifndef iwSHIP_H_INCLUDED
#define iwSHIP_H_INCLUDED

#pragma once

#include "IngameWindow.h"

class dskGameInterface;
class noShip;
class GameWorldViewer;

class iwShip : public IngameWindow
{
	GameWorldViewer * const gwv;
	dskGameInterface *const gi; ///< Das GameInterface
	unsigned ship_id; /// ID des Schiffes, welches gerade angezeigt wird
	unsigned char player; /// Besitzer des Schiffes, den wir f�r die Umwandlung ID->richtiges Schiff brauchen

public:
	/// Konstruktor von @p iwBuilding.
	iwShip(GameWorldViewer * const gwv,dskGameInterface *const gi,noShip *const ship);

private:

	void Msg_PaintBefore();
	void Msg_PaintAfter();
	void Msg_ButtonClick(const unsigned int ctrl_id);

	void DrawCargo();
};

#endif // !iwSHIP_H_INCLUDED
