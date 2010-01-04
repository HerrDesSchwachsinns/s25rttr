// $Id: Desktop.h 5853 2010-01-04 16:14:16Z FloSoft $
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
#ifndef DESKTOP_H_INCLUDED
#define DESKTOP_H_INCLUDED

#pragma once

#include "Window.h"

/// Desktopklasse f�r Spielmen�-Haupthintergrundfl�chen.
class Desktop : public Window
{
public:
	Desktop(glArchivItem_Bitmap *background);
	void Show(void);

protected:
	bool Draw_(void);

protected:
	glArchivItem_Bitmap *background;
};

#endif // !DESKTOP_H_INCLUDED
