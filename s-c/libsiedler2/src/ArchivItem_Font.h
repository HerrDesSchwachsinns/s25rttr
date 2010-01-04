// $Id: ArchivItem_Font.h 5853 2010-01-04 16:14:16Z FloSoft $
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
#ifndef ARCHIVITEM_FONT_H_INCLUDED
#define ARCHIVITEM_FONT_H_INCLUDED

#pragma once

#include "ArchivItem.h"
#include "ArchivItem_Palette.h"

namespace libsiedler2
{

/// Klasse f�r Fonts.
class ArchivItem_Font : public ArchivItem, public ArchivInfo
{
public:
	/// Konstruktor von @p ArchivItem_Font.
	ArchivItem_Font(void);

	/// Kopierkonstruktor von @p ArchivItem_Font.
	ArchivItem_Font(const ArchivItem_Font *item);

	/// l�dt die Fontdaten aus einer Datei.
	int load(FILE *file, const ArchivItem_Palette *palette);

	/// schreibt die Fontdaten in eine Datei.
	int write(FILE *file, const ArchivItem_Palette *palette) const;

	/// liefert den X-Buchstabenabstand.
	inline unsigned char getDx(void) const
	{
		return dx;
	}

	/// liefert den Y-Buchstabenabstand.
	inline unsigned char getDy(void) const
	{
		return dy;
	}

	/// setzt den X-Buchstabenabstand.
	inline void setDx(unsigned char dx)
	{
		this->dx = dx;
	}

	/// setzt den Y-Buchstabenabstand.
	inline void setDy(unsigned char dy)
	{
		this->dy = dy;
	}

protected:
	unsigned char dx; ///< X-Buchstabenabstand.
	unsigned char dy; ///< Y-Buchstabenabstand.
};

}

#endif // !ARCHIVITEM_FONT_H_INCLUDED
