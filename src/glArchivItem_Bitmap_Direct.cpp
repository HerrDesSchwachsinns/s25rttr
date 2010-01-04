// $Id: glArchivItem_Bitmap_Direct.cpp 5853 2010-01-04 16:14:16Z FloSoft $
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

///////////////////////////////////////////////////////////////////////////////
// Header
#include "main.h"
#include "glArchivItem_Bitmap_Direct.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/** 
 *  Konstruktor von @p glArchivItem_Bitmap_Direct.
 *
 *  @author FloSoft
 */
glArchivItem_Bitmap_Direct::glArchivItem_Bitmap_Direct(void)
	: baseArchivItem_Bitmap(), glArchivItem_Bitmap()
{
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  Kopierkonstruktor von @p glArchivItem_Bitmap_Direct.
 *
 *  @author FloSoft
 */
glArchivItem_Bitmap_Direct::glArchivItem_Bitmap_Direct(const glArchivItem_Bitmap_Direct *item)
	: baseArchivItem_Bitmap(item), glArchivItem_Bitmap(item)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  setzt einen Pixel auf einen bestimmten Wert.
 *
 *  @param[in] x       X Koordinate des Pixels
 *  @param[in] y       Y Koordinate des Pixels
 *  @param[in] color   Farbe des Pixels
 *  @param[in] palette Grundpalette
 *
 *  @author FloSoft
 */
void glArchivItem_Bitmap_Direct::tex_setPixel(unsigned short x, unsigned short y, unsigned char color, const libsiedler2::ArchivItem_Palette *palette)
{
	// Pixel in Puffer setzen
	libsiedler2::baseArchivItem_Bitmap::tex_setPixel(x, y, color, palette);

	// Ist eine GL-Textur bereits erzeugt? Wenn ja, Pixel in Textur austauschen
	if(texture != 0)
	{
		if(x < tex_width && y < tex_height)
		{
			unsigned char buffer[4] = { 0x00, 0x00, 0x00, 0xFF };
			
			if(color == libsiedler2::TRANSPARENT_INDEX)
				buffer[3] = 0x00;
			else
				this->palette->get(color, &buffer[0], &buffer[1], &buffer[2]);

			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &buffer);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  setzt einen Pixel auf einen bestimmten Wert.
 *
 *  @param[in] x X Koordinate des Pixels
 *  @param[in] y Y Koordinate des Pixels
 *  @param[in] r Roter Wert
 *  @param[in] g Gr�ner Wert
 *  @param[in] b Blauer Wert
 *  @param[in] a Alpha Wert (bei paletted nur 0xFF/0x00 unterst�tzt)
 *
 *  @author FloSoft
 */
void glArchivItem_Bitmap_Direct::tex_setPixel(unsigned short x, unsigned short y, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	// Pixel in Puffer setzen
	libsiedler2::baseArchivItem_Bitmap::tex_setPixel(x, y, r, g, b, a);

	// Ist ein GL-Textur bereits erzeugt? Wenn ja, Pixel in Textur austauschen
	if(texture != 0)
	{
		if(x < tex_width && y < tex_height)
		{
			unsigned char buffer[4] = { r, g, b, a };
			
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &buffer);
		}
	}
}

/// liefert die Farbwerte eines Pixels als uc-Array: {r,g,b,a}
unsigned char * glArchivItem_Bitmap_Direct::tex_getPixel(const unsigned short x, const unsigned short y)
{
	return &tex_data[y*tex_width+x];
}

