// $Id: ArchivItem_Bitmap_Shadow.h 5090 2009-06-23 13:07:47Z FloSoft $
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
#ifndef ARCHIVITEM_BITMAP_SHADOW_H_INCLUDED
#define ARCHIVITEM_BITMAP_SHADOW_H_INCLUDED

#pragma once

#include "ArchivItem_Bitmap.h"
#include "ArchivItem_Palette.h"

namespace libsiedler2
{
	/// Basisklasse f�r Shadow-Bitmaps.
	class baseArchivItem_Bitmap_Shadow : public virtual baseArchivItem_Bitmap
	{
	public:
		/// Konstruktor von @p baseArchivItem_Bitmap_Shadow.
		baseArchivItem_Bitmap_Shadow(void);

		/// Kopierkonstruktor von @p baseArchivItem_Bitmap_Shadow.
		baseArchivItem_Bitmap_Shadow(const baseArchivItem_Bitmap_Shadow *item);

		/// Konstruktor von @p baseArchivItem_Bitmap_Shadow mit Laden der Bilddaten aus einer Datei.
		baseArchivItem_Bitmap_Shadow(FILE *file, const ArchivItem_Palette *palette);

		/// Destruktor von @p baseArchivItem_Bitmap_Shadow.
		~baseArchivItem_Bitmap_Shadow(void);

		/// l�dt die Bilddaten aus einer Datei.
		int load(FILE *file, const ArchivItem_Palette *palette);

		/// schreibt die Bilddaten in eine Datei.
		int write(FILE *file, const ArchivItem_Palette *palette) const;
	};

	/// Klasse f�r Shadow-Bitmaps.
	class ArchivItem_Bitmap_Shadow : public baseArchivItem_Bitmap_Shadow, public ArchivItem_Bitmap
	{
	public:
		/// Konstruktor von @p ArchivItem_Bitmap_Shadow.
		ArchivItem_Bitmap_Shadow(void) : baseArchivItem_Bitmap(), baseArchivItem_Bitmap_Shadow() {}

		/// Kopierkonstruktor von @p ArchivItem_Bitmap_Shadow.
		ArchivItem_Bitmap_Shadow(const ArchivItem_Bitmap_Shadow *item) : baseArchivItem_Bitmap(item), baseArchivItem_Bitmap_Shadow(item) {}
	};
}

#endif // !ARCHIVITEM_BITMAP_SHADOW_H_INCLUDED
