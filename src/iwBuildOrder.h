// $Id: iwBuildOrder.h 4784 2009-05-02 20:43:44Z OLiver $
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
#ifndef iwBUILDORDER_H_INCLUDED
#define iwBUILDORDER_H_INCLUDED

#pragma once

#include "IngameWindow.h"

class iwBuildOrder : public IngameWindow
{
	/// Einstellungen nach dem letzten Netzwerk-Versenden nochmal ver�ndert?
	bool settings_changed;
public:

	iwBuildOrder(void);
	~iwBuildOrder();

private:

	/// Updatet die Steuerelemente mit den aktuellen Einstellungen aus dem Spiel
	void UpdateSettings();
	/// Sendet ver�nderte Einstellungen (an den Client), falls sie ver�ndert wurden
	void TransmitSettings();

	void Msg_Timer(const unsigned int ctrl_id);
	void Msg_ListSelectItem(const unsigned int ctrl_id, const unsigned short selection);
	void Msg_ButtonClick(const unsigned int ctrl_id);
};

#endif // !iwBUILDORDER_H_INCLUDED
