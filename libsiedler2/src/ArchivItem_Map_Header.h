// $Id: ArchivItem_Map_Header.h 6581 2010-07-16 11:16:34Z FloSoft $
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
#ifndef ARCHIVITEM_MAP_HEADER_H_INCLUDED
#define ARCHIVITEM_MAP_HEADER_H_INCLUDED

#pragma once

#include "ArchivItem.h"

namespace libsiedler2
{
	/// Klasse f�r einen Mapheader.
	class ArchivItem_Map_Header : public ArchivItem
	{
	public:
		/// Konstruktor von @p ArchivItem_Map_Header.
		ArchivItem_Map_Header(void);
		/// Kopierkonstruktor von @p ArchivItem_Map_Header.
		ArchivItem_Map_Header(const ArchivItem_Map_Header *item);
		/// Destruktor von @p ArchivItem_Map_Header.
		~ArchivItem_Map_Header(void);

		/// l�dt den Mapheader aus einer Datei.
		int load(FILE *file);
		/// schreibt den Mapheader in eine Datei.
		int write(FILE *file) const;

		const char *getName(void) const;
		void setName(const char *name);

		unsigned short getWidth(void) const;
		void setWidth(unsigned short width);

		unsigned short getHeight(void) const;
		void setHeight(unsigned short height);

		unsigned char getGfxSet(void) const;
		void setGfxSet(unsigned char gfxset);

		unsigned char getPlayer(void) const;
		void setPlayer(unsigned char player);

		const char *getAuthor(void) const;
		void setAuthor(const char *author);

	private:
		char name[21];
		unsigned short width;
		unsigned short height;
		unsigned char gfxset;
		unsigned char player;
		char author[21];
	};
}

#endif // !ARCHIVITEM_MAP_HEADER_H_INCLUDED
