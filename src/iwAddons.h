// $Id: iwAddons.h 5991 2010-02-10 15:44:37Z FloSoft $
//
// Copyright (c) 2005-2010 Settlers Freaks (sf-team at siedler25.org)
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
#ifndef iwADDONS_H_INCLUDED
#define iwADDONS_H_INCLUDED

#pragma once

#include "IngameWindow.h"

class iwAddons : public IngameWindow
{
public:
	enum ChangePolicy
	{
		HOSTGAME,
		READONLY,
		SETDEFAULTS
	};

public:
	iwAddons(ChangePolicy policy = SETDEFAULTS);
	~iwAddons(void);

protected:
	void Msg_ButtonClick(const unsigned int ctrl_id);
	void Msg_OptionGroupChange(const unsigned int ctrl_id, const unsigned short selection);

private:
	ChangePolicy policy;
};

#endif // !iwENHANCEMENTS_H_INCLUDED
