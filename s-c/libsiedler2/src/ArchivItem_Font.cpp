// $Id: ArchivItem_Font.cpp 5259 2009-07-13 15:53:31Z FloSoft $
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

///////////////////////////////////////////////////////////////////////////////
// Header
#include "main.h"
#include "ArchivItem_Font.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/** @class libsiedler2::ArchivItem_Font
 *
 *  Klasse f�r Fonts.
 *
 *  @author FloSoft
 */

///////////////////////////////////////////////////////////////////////////////
/** @var libsiedler2::ArchivItem_Font::dx
 *
 *  X-Buchstabenabstand.
 *
 *  @author FloSoft
 */

///////////////////////////////////////////////////////////////////////////////
/** @var libsiedler2::ArchivItem_Font::dy
 *
 *  Y-Buchstabenabstand.
 *
 *  @author FloSoft
 */

///////////////////////////////////////////////////////////////////////////////
/**
 *  Konstruktor von @p ArchivItem_Font.
 *
 *  @author FloSoft
 */
libsiedler2::ArchivItem_Font::ArchivItem_Font(void) : ArchivItem(), ArchivInfo(), dx(0), dy(0)
{
	setBobType(BOBTYPE_FONT);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Kopierkonstruktor von @p ArchivItem_Font.
 *
 *  @param[in] item Quellitem
 *
 *  @author FloSoft
 */
libsiedler2::ArchivItem_Font::ArchivItem_Font(const ArchivItem_Font *item) : ArchivItem( item ), ArchivInfo( item ), dx(item->dx), dy(item->dy)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  l�dt die Fontdaten aus einer Datei.
 *
 *  @param[in] file    Dateihandle der Datei
 *  @param[in] palette Grundpalette
 *
 *	@return liefert Null bei Erfolg, ungleich Null bei Fehler
 *
 *  @author FloSoft
 */
int libsiedler2::ArchivItem_Font::load(FILE *file, const ArchivItem_Palette *palette)
{
	if(file == NULL || palette == NULL)
		return 1;

	// X-Spacing einlesen
	if(libendian::le_read_uc(&dx, 1, file) != 1)
		return 2;

	// Y-Spacing einlesen
	if(libendian::le_read_uc(&dy, 1, file) != 1)
		return 3;

	// Speicher f�r Buchstaben alloziieren
	alloc(256);

	// Buchstaben einlesen
	for(unsigned long i = 32; i < 256; ++i)
	{
		short bobtype;

		// bobtype des Items einlesen
		if(libendian::le_read_s(&bobtype, file) != 0)
			return 4;

		if(bobtype == 0x0000)
			continue;

		// Daten von Item auswerten
		if(loader::LoadType(bobtype, file, palette, getP(i)) != 0)
			return 5;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  schreibt die Fontdaten in eine Datei.
 *
 *  @param[in] file    Dateihandle der Datei
 *  @param[in] palette Grundpalette
 *
 *	@return liefert Null bei Erfolg, ungleich Null bei Fehler
 *
 *  @author FloSoft
 */
int libsiedler2::ArchivItem_Font::write(FILE *file, const ArchivItem_Palette *palette) const
{
	if(file == NULL || palette == NULL)
		return 1;

	// X-Spacing schreiben
	unsigned char ddx = dx, ddy = dy;
	if(libendian::le_write_uc(&ddx, 1, file) != 1)
		return 2;

	// Y-Spacing schreiben
	if(libendian::le_write_uc(&ddy, 1, file) != 1)
		return 3;

	// Buchstaben schreiben
	for(unsigned long i = 32; i < 256; ++i)
	{
		const ArchivItem *item = get(i);
		short bobtype = 0;

		if(item)
			bobtype = item->getBobType();

		// bobtype des Items schreiben
		if(libendian::le_write_s(bobtype, file) != 0)
			return 4;

		if(item == NULL)
			continue;

		// Daten von Item auswerten
		if(loader::WriteType(bobtype, file, palette, item) != 0)
			return 5;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  liefert den X-Buchstabenabstand.
 *
 *	@return liefert den X-Buchstabenabstand.
 *
 *  @author FloSoft
 */

///////////////////////////////////////////////////////////////////////////////
/**
 *  liefert den Y-Buchstabenabstand.
 *
 *	@return liefert den Y-Buchstabenabstand.
 *
 *  @author FloSoft
 */

///////////////////////////////////////////////////////////////////////////////
/**
 *  setzt den X-Buchstabenabstand.
 *
 *  @param[in] dx X-Buchstabenabstand
 *
 *  @author FloSoft
 */

///////////////////////////////////////////////////////////////////////////////
/**
 *  setzt den Y-Buchstabenabstand.
 *
 *  @param[in] dy Y-Buchstabenabstand
 *
 *  @author FloSoft
 */
