// $Id: LoadType.cpp 4652 2009-03-29 10:10:02Z FloSoft $
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

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/**
 *  l�dt eine spezifizierten Bobtype aus einer Datei in ein ArchivItem.
 *
 *  @param[in]  bobtype Typ des Items
 *  @param[in]  file    Filehandle auf die auszulesende Datei
 *  @param[in]  palette Grundpalette
 *  @param[out] item    ArchivItem-Struktur, welche gef�llt wird
 *
 *  @return Null bei Erfolg, ein Wert ungleich Null bei Fehler
 *
 *  @author FloSoft
 *  @author OLiver
 */
int libsiedler2::loader::LoadType(unsigned short bobtype, FILE *file, const ArchivItem_Palette *palette, ArchivItem **item)
{
	if(file == NULL || item == NULL)
		return 1;

	// ist das Item schon belegt?
	delete *item;

	switch(bobtype)
	{
	case BOBTYPE_SOUND: // WAVs, MIDIs
		{
			unsigned int length;

			// L�nge einlesen
			if(libendian::le_read_ui(&length, file) != 0)
				return 4;

			baseArchivItem_Sound *nitem = baseArchivItem_Sound::findSubType(file);
			if(!nitem || nitem->load(file, length) != 0)
				return 5;
			*item = nitem;
		} break;
	case BOBTYPE_BITMAP_RLE: // RLE komprimiertes Bitmap
		{
			baseArchivItem_Bitmap_RLE *nitem = dynamic_cast<baseArchivItem_Bitmap_RLE*>((*allocator)(BOBTYPE_BITMAP_RLE, 0, NULL));
			if(nitem->load(file, palette) != 0)
				return 6;
			*item = nitem;
		} break;
	case BOBTYPE_FONT: // Font
		{
			ArchivItem_Font *nitem = (ArchivItem_Font*)(*allocator)(BOBTYPE_FONT, 0, NULL);
			if(nitem->load(file, palette) != 0)
				return 7;
			*item = nitem;
		} break;
	case BOBTYPE_BITMAP_PLAYER: // Bitmap mit spezifischer Spielerfarbe
		{
			baseArchivItem_Bitmap_Player *nitem = dynamic_cast<baseArchivItem_Bitmap_Player*>((*allocator)(BOBTYPE_BITMAP_PLAYER, 0, NULL));
			if(nitem->load(file, palette) != 0)
				return 8;
			*item = nitem;
		} break;
	case BOBTYPE_PALETTE: // Palette
		{
			ArchivItem_Palette *nitem = (ArchivItem_Palette*)(*allocator)(BOBTYPE_PALETTE, 0, NULL);
			if(nitem->load(file) != 0)
				return 9;
			*item = nitem;
		} break;
	case BOBTYPE_BOB: // Bobfile
		{
			ArchivItem_Bob *nitem = dynamic_cast<ArchivItem_Bob*>((*allocator)(BOBTYPE_BOB, 0, NULL));
			if(nitem->load(file, palette) != 0)
				return 10;
			*item = nitem;
		} break;
	case BOBTYPE_BITMAP_SHADOW: // Schatten
		{
			baseArchivItem_Bitmap_Shadow *nitem = dynamic_cast<baseArchivItem_Bitmap_Shadow*>((*allocator)(BOBTYPE_BITMAP_SHADOW, 0, NULL));
			if(nitem->load(file, palette) != 0)
				return 11;
			*item = nitem;
		} break;
	case BOBTYPE_MAP: // Mapfile
		{
			ArchivItem_Map *nitem = dynamic_cast<ArchivItem_Map*>((*allocator)(BOBTYPE_MAP, 0, NULL));
			if(nitem->load(file, false) != 0)
				return 12;
			*item = nitem;
		} break;
	case BOBTYPE_TEXT: // Textfile
		{
			ArchivItem_Text *nitem = (ArchivItem_Text*)(*allocator)(BOBTYPE_TEXT, 0, NULL);
			if(nitem->load(file) != 0)
				return 13;
			*item = nitem;
		} break;
	case BOBTYPE_BITMAP_RAW: // unkomprimiertes Bitmap
		{
			baseArchivItem_Bitmap_Raw *nitem = dynamic_cast<baseArchivItem_Bitmap_Raw*>((*allocator)(BOBTYPE_BITMAP_RAW, 0, NULL));
			if(nitem->load(file, palette) != 0)
				return 14;
			*item = nitem;
		} break;
	}

	if(*item != NULL)
		(*item)->setBobType(bobtype);

	return 0;
}
