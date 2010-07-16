// $Id: iwBaseWarehouse.h 6582 2010-07-16 11:23:35Z FloSoft $
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
#ifndef iwBASEWAREHOUSE_H_INCLUDED
#define iwBASEWAREHOUSE_H_INCLUDED

#pragma once

#include "iwWares.h"

class nobBaseWarehouse;
class GameWorldViewer;

/// Basisklasse f�r die HQ- und Lagerh�userfenster
class iwBaseWarehouse : public iwWares
{
	GameWorldViewer * const gwv;

protected:
	nobBaseWarehouse *wh; ///< Pointer zum entsprechenden Lagerhaus

public:
	/// Konstruktor von @p iwBaseWarehouse.
	iwBaseWarehouse(GameWorldViewer * const gwv,const char *const title, unsigned char page_count, nobBaseWarehouse *wh);

protected:

	/// Overlay �ndern
	void ChangeOverlay(unsigned int i, unsigned int what);

	void Msg_Group_ButtonClick(const unsigned int group_id, const unsigned int ctrl_id);
	void Msg_ButtonClick(const unsigned int ctrl_id);


	
};

#endif // !iwHQ_H_INCLUDED
