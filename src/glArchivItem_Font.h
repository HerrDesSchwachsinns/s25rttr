// $Id: glArchivItem_Font.h 6457 2010-05-31 08:40:04Z OLiver $
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
#ifndef GLARCHIVITEM_FONT_H_INCLUDED
#define GLARCHIVITEM_FONT_H_INCLUDED

#pragma once

/// Klasse f�r GL-Fontfiles.
class glArchivItem_Font : public libsiedler2::ArchivItem_Font
{
public:

	/// Konstruktor von @p glArchivItem_Font.
	glArchivItem_Font(void) : ArchivItem_Font(), _font(NULL) {}
	/// Kopierkonstruktor von @p glArchivItem_Font.
	glArchivItem_Font(const glArchivItem_Font *item) : ArchivItem_Font(item), _font(NULL) {}

	/// Zeichnet einen Text.
	void Draw(short x, short y, const std::string& text, unsigned int format, unsigned int color = COLOR_WHITE, unsigned short length = 0, unsigned short max = 0xFFFF, const std::string& end = "...", unsigned short end_length = 0);
	/// pr�ft ob ein Buchstabe existiert.
	bool CharExist(unsigned char c) const;
	/// liefert die L�nge einer Zeichenkette.
	unsigned short getWidth(const std::string& text, unsigned length = 0, unsigned max_width = 0xffffffff, unsigned short *max = NULL) const;
	/// liefert die H�he des Textes ( entspricht @p getDy()+1 )
	unsigned short getHeight() const { return dy+1; }

	/// Gibt Infos, �ber die Unterbrechungspunkte in einem Text
	class WrapInfo 
	{ 
	public:
		/// Erzeugt ein Arrays aus eigenst�ndigen Strings aus den Unterbrechungsinfos.
		void CreateSingleStrings(const std::string& origin_text, std::string *dest_strings);

	public:
		/// Array von Positionen, wo der Text umbrochen werden soll (jeweils der Anfang vom String)
		std::vector<unsigned int> positions;
	};

	/// Gibt Infos, �ber die Unterbrechungspunkte in einem Text, versucht W�rter nicht zu trennen, tut dies aber, falls
	/// es unumg�nglich ist (Wort l�nger als die Zeile)
	void GetWrapInfo(const std::string& text, const unsigned short primary_width, const unsigned short secondary_width, WrapInfo& wi);

	enum {
		DF_LEFT   = 0,
		DF_RIGHT  = 1,
		DF_CENTER = 2
	};

	enum {
		DF_TOP     = 0,
		DF_BOTTOM  = 4,
		DF_VCENTER = 8
	};

private:
	void initFont();

	glArchivItem_Bitmap_Player *_font;
	unsigned short _charwidths[256];
};

#endif // !GLARCHIVITEM_FONT_H_INCLUDED
