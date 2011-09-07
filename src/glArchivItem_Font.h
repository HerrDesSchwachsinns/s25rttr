// $Id: glArchivItem_Font.h 7504 2011-09-07 12:56:11Z FloSoft $
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
#ifndef GLARCHIVITEM_FONT_H_INCLUDED
#define GLARCHIVITEM_FONT_H_INCLUDED

#pragma once

/// Klasse f�r GL-Fontfiles.
class glArchivItem_Font : public libsiedler2::ArchivItem_Font
{
public:
		/// Konstruktor von @p glArchivItem_Font.
	glArchivItem_Font(void) : ArchivItem_Font(), _font(NULL), chars_per_line(16) {}
	/// Kopierkonstruktor von @p glArchivItem_Font.
	glArchivItem_Font(const glArchivItem_Font *item) : ArchivItem_Font(item), _font(NULL) {}

	/// Zeichnet einen Text.
	void Draw(short x, short y, const std::string& text, unsigned int format, unsigned int color = COLOR_WHITE, unsigned short length = 0, unsigned short max = 0xFFFF, const std::string& end = "...", unsigned short end_length = 0);

	/// liefert die L�nge einer Zeichenkette.
	inline unsigned short getWidth(const std::string& text, unsigned length = 0, unsigned max_width = 0xffffffff, unsigned short *max = NULL) const;
	/// liefert die H�he des Textes ( entspricht @p getDy()+1 )
	inline unsigned short getHeight() const { return dy+1; }

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

	/// pr�ft ob ein Buchstabe existiert.
	inline bool CharExist(unsigned int c) const { return (CharWidth(c) > 0); }
	/// liefert die Breite eines Zeichens
	inline unsigned int CharWidth(unsigned int c) const { return CharInfo(c).width; }

private:
	void initFont();

	struct GL_T2F_V3F_Struct
	{
		GLfloat tx, ty;
		GLfloat x, y, z;
	};

	unsigned int Utf8_to_Unicode(const std::string& text, unsigned int& i) const;
	void DrawChar(const std::string& text, unsigned int& i, GL_T2F_V3F_Struct *tmp, short& cx, short& cy, float tw, float th, unsigned int& idx);

	struct char_info
	{
		char_info() : x(0), y(0), width(0), reserved(0xFFFF) {}
		unsigned short width;
		unsigned short x;
		unsigned short y;
		unsigned short reserved; // so we have 8 byte's
	};

	/// liefert das Char-Info eines Zeichens
	inline const char_info& CharInfo(unsigned int c) const
	{
		static char_info ci;

		std::map<unsigned int, char_info>::const_iterator it = utf8_mapping.find(c);
		if(it != utf8_mapping.end())
			return it->second;

		return ci;
	}

	glArchivItem_Bitmap *_font;

	unsigned int chars_per_line;
	std::map<unsigned int, char_info> utf8_mapping;
};

#endif // !GLARCHIVITEM_FONT_H_INCLUDED
