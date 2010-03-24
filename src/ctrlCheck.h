// $Id: ctrlCheck.h 6177 2010-03-24 10:44:32Z FloSoft $
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
#ifndef CTRLCHECK_H_INCLUDED
#define CTRLCHECK_H_INCLUDED

#pragma once

#include "Window.h"

class ctrlCheck : public Window
{
public:
	ctrlCheck(Window *parent, unsigned int id, unsigned short x, unsigned short y, unsigned short width, unsigned short height, TextureColor tc, const std::string& text, glArchivItem_Font *font, bool readonly);

	void SetCheck(bool check) { this->check = check; }
	bool GetCheck() const { return check; }
	void SetReadOnly(bool readonly) { this->readonly = readonly; }
	bool GetReadOnly() const { return readonly; }

	virtual bool Msg_LeftDown(const MouseCoords& mc);

protected:
	virtual bool Draw_(void);

private:
	TextureColor tc;
	std::string text;
	glArchivItem_Font *font;
	bool check;
	bool readonly;
};

#endif // !CTRLCHECK_H_INCLUDED
