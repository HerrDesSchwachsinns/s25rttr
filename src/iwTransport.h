// $Id: iwTransport.h 6582 2010-07-16 11:23:35Z FloSoft $
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
#ifndef iwTRANSPORT_H_INCLUDED
#define iwTRANSPORT_H_INCLUDED

#pragma once

#include "IngameWindow.h"

class iwTransport : public IngameWindow
{
private:

	glArchivItem_Bitmap *TRANSPORT_SPRITES[14]; ///< Einzelne Bilder f�r die Waren.

	/// Einstellungen nach dem letzten Netzwerk-Versenden nochmal ver�ndert?
	bool settings_changed;

public:

	iwTransport();
	~iwTransport();

private:

	/// Updatet die Steuerelemente mit den aktuellen Einstellungen aus dem Spiel
	void UpdateSettings();
	/// Sendet ver�nderte Einstellungen (an den Client), falls sie ver�ndert wurden
	void TransmitSettings();

	void Msg_ButtonClick(const unsigned int ctrl_id);
	void Msg_Timer(const unsigned int ctrl_id);
};

#endif // !iwTRANSPORT_H_INCLUDED
