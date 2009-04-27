// $Id: WriteBMP.cpp 4652 2009-03-29 10:10:02Z FloSoft $
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
 *  schreibt ein ArchivInfo in eine BMP-File.
 *
 *  @param[in] file    Dateiname der BMP-File
 *  @param[in] items   ArchivInfo-Struktur, von welcher gelesen wird
 *  @param[in] palette Palette die f�r das Bitmap verwendet werden soll
 *  @param[in] nr      Nummer des Bitmaps das geschrieben werden soll 
                       (@p -1 erste Bitmap das gefunden wird)
 *
 *  @todo RGB Bitmaps (Farben > 8Bit) ebenfalls schreiben.
 *
 *  @return Null bei Erfolg, ein Wert ungleich Null bei Fehler
 *
 *  @author FloSoft
 */
int libsiedler2::loader::WriteBMP(const char *file, const ArchivItem_Palette *palette, const ArchivInfo *items, long nr)
{
	struct BMHD {
		unsigned short header; // 2
		unsigned int size; // 6
		unsigned int reserved; // 10
		unsigned int offset; // 14
	} bmhd = { 0x4D42, 40, 0, 54 };

	struct BMIH {
		unsigned int length; // 4
		int width; // 8
		int height; // 12
		short planes; // 14
		short bbp; // 16
		unsigned int compression; // 20
		unsigned int size; // 24
		int xppm; // 28
		int yppm; // 32
		int clrused; // 36
		int clrimp; // 40
	} bmih = { 40, 0, 0, 1, 24, 0, 40, 0, 0, 0, 0 };

	FILE *bmp;

	if(file == NULL || items == NULL)
		return 1;

	if(nr == -1)
	{
		// Bitmap in ArchivInfo suchen, erstes Bitmap wird geschrieben
		for(unsigned long i = 0; i < items->getCount(); ++i)
		{
			if(!items->get(i))
				continue;

			switch(items->get(i)->getBobType())
			{
			case BOBTYPE_BITMAP_RLE:
			case BOBTYPE_BITMAP_SHADOW:
			case BOBTYPE_BITMAP_RAW:
				{
					nr = i;
				} break;
			}
			if(nr != -1)
				break;
		}
	}

	// Haben wir eine gefunden?
	if(nr == -1)
		return 2;

	const ArchivItem_Bitmap *bitmap = dynamic_cast<const ArchivItem_Bitmap*>(items->get(nr));

	// Datei zum schreiben �ffnen
	bmp = fopen(file, "wb");
	
	// hat das geklappt?
	if(bmp == NULL)
		return 3;

	bmih.height = bitmap->getHeight();
	bmih.width = bitmap->getWidth();
	bmih.size = 0;

	bmhd.size = 0;

	// Bitmap-Header schreiben
	//if(libendian::le_write_uc((unsigned char*)&bmhd, 14, bmp) != 14)
	//	return 4;
	if(libendian::le_write_us(bmhd.header, bmp) != 0)
		return 4;
	if(libendian::le_write_ui(bmhd.size, bmp) != 0)
		return 4;
	if(libendian::le_write_ui(bmhd.reserved, bmp) != 0)
		return 4;
	if(libendian::le_write_ui(bmhd.offset, bmp) != 0)
		return 4;

	// Bitmap-Info-Header schreiben
	//if(libendian::le_write_uc((unsigned char*)&bmih, 40, bmp) != 40)
	//	return 5;
	if(libendian::le_write_ui(bmih.length, bmp) != 0)
		return 5;
	if(libendian::le_write_i(bmih.width, bmp) != 0)
		return 5;
	if(libendian::le_write_i(bmih.height, bmp) != 0)
		return 5;
	if(libendian::le_write_s(bmih.planes, bmp) != 0)
		return 5;
	if(libendian::le_write_s(bmih.bbp, bmp) != 0)
		return 5;
	if(libendian::le_write_ui(bmih.compression, bmp) != 0)
		return 5;
	if(libendian::le_write_ui(bmih.size, bmp) != 0)
		return 5;
	if(libendian::le_write_i(bmih.xppm, bmp) != 0)
		return 5;
	if(libendian::le_write_i(bmih.yppm, bmp) != 0)
		return 5;
	if(libendian::le_write_i(bmih.clrused, bmp) != 0)
		return 5;
	if(libendian::le_write_i(bmih.clrimp, bmp) != 0)
		return 5;

	fflush(bmp);

	// Farbpalette lesen
	unsigned char colors[256][4];

	// Farbpalette zuweisen
	for(int i = 0; i < bmih.clrused; ++i)
	{
		colors[i][3] = 0;
		palette->get(i, &colors[i][2], &colors[i][1], &colors[i][0]);
	}

	// Farbpalette schreiben
	if(libendian::le_write_uc(colors[0], bmih.clrused*4, bmp) != bmih.clrused*4)
		return 6;

	unsigned char *buffer = new unsigned char[bmih.width*bmih.height * 4 +1];
	memset(buffer, 0x00, bmih.width*bmih.height * 4 + 1);

	/// @todo: bug im print?!?
	if(bitmap->print(buffer, bmih.width, bmih.height, FORMAT_RGBA, palette) != 0)
		return 7;

	unsigned char placeholder[80];
	memset(placeholder, 0, 80);

	// Bottom-Up, "von unten nach oben"
	for(int y = bmih.height-1; y >= 0; --y)
	{
		for(int x = 0; x < bmih.width; ++x)
		{
			switch(bmih.bbp)
			{
			case 8:
				{
					unsigned char color = buffer[x + bmih.width*y];
					libendian::le_write_uc(&color, 1, bmp);
				} break;
			case 24:
				{
					unsigned char r = buffer[4*(x + bmih.width*y)];
					unsigned char g = buffer[4*(x + bmih.width*y) + 1];
					unsigned char b = buffer[4*(x + bmih.width*y) + 2];
					if(buffer[4*(x + bmih.width*y) + 3] == 0x00)
					{
						r = 0xff;
						g = 0x00;
						b = 0x8f;
					}
					if(libendian::le_write_uc(&b, 1, bmp) != 1)
						return 8;
					if(libendian::le_write_uc(&g, 1, bmp) != 1)
						return 8;
					if(libendian::le_write_uc(&r, 1, bmp) != 1)
						return 8;
				} break;
			}
		}
		if((bmih.width * bmih.bbp/8) % 4 > 0)
			libendian::le_write_uc(placeholder, 4 - (bmih.width * bmih.bbp/8) % 4, bmp);
	}
	if(ftell(bmp) % 4 > 0)
		libendian::le_write_uc(placeholder, 4 - (ftell(bmp) % 4), bmp);

	delete[] buffer;

	// Datei schliessen
	fclose(bmp);

	// alles ok
	return 0;
}
