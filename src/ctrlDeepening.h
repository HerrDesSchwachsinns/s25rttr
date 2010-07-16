// $Id: ctrlDeepening.h 6582 2010-07-16 11:23:35Z FloSoft $
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
#ifndef CTRLDEEPENING_H_INCLUDED
#define CTRLDEEPENING_H_INCLUDED

#pragma once

#include "ctrlText.h"
#include "ctrlRectangle.h"

class ctrlDeepening : public ctrlText
{
public:
	ctrlDeepening(Window *parent, unsigned int id, unsigned short x, unsigned short y, unsigned short width, unsigned short height, TextureColor tc, const std::string& text, glArchivItem_Font *font, unsigned int color);

protected:
	virtual bool Draw_(void);
	/// Abgeleitete Klassen m�ssen erweiterten Inhalt zeichnen
	virtual void DrawContent() const {}

private:
	TextureColor tc;
};

/// Colored Deepening
class ctrlColorDeepening : public ctrlDeepening, public ColorControlInterface
{
public:
	ctrlColorDeepening(Window *parent, unsigned int id, unsigned short x, unsigned short y, unsigned short width, unsigned short height, TextureColor tc, unsigned int fillColor);

	/// Setzt die Farbe des Controls
	virtual void SetColor(const unsigned int fill_color);

protected:
	void DrawContent() const;

protected:
	unsigned short width, height;
	unsigned int fillColor;
};

#endif // !CTRLDEEPENING_H_INCLUDED

