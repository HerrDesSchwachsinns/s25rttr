// $Id: ctrlButton.h 6582 2010-07-16 11:23:35Z FloSoft $
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
#ifndef CTRLBUTTON_H_INCLUDED
#define CTRLBUTTON_H_INCLUDED

#pragma once

#include "ctrlRectangle.h"
#include "ctrlText.h"

/// Buttonklasse
class ctrlButton : public Window
{
public:
	ctrlButton(Window *parent, unsigned int id, unsigned short x, unsigned short y, unsigned short width, unsigned short height, 
		const TextureColor tc, const std::string& tooltip);
	virtual ~ctrlButton();

	void Enable(bool enable = true) { enabled = enable; }
	void SetTexture(TextureColor tc) { this->tc = tc; }

	/// Setzt Tooltip
	void SetTooltip(const std::string& tooltip) { this->tooltip = tooltip; }
	/// Liefert Tooltip zur�ck
	std::string GetTooltip(void) const { return tooltip; }
	/// Tauscht Tooltips
	void SwapTooltip(ctrlButton *two) { Swap(tooltip, two->tooltip); }

	void SetCheck(bool check) { this->check = check; }
	bool GetCheck(void) { return check; }
	void SetIlluminated(bool illuminated) { this->illuminated = illuminated; }
	bool GetIlluminated(void) { return illuminated; }
	void SetBorder(const bool border) { this->border = border; }

	virtual bool Msg_MouseMove(const MouseCoords& mc);
	virtual bool Msg_LeftDown(const MouseCoords& mc);
	virtual bool Msg_LeftUp(const MouseCoords& mc);

protected:
	/// Zeichnet Grundstruktur des Buttons
	virtual bool Draw_(void);
	/// Abgeleitete Klassen m�ssen erweiterten Button-Inhalt zeichnen
	virtual void DrawContent() const = 0;
	// Pr�fen, ob bei gehighlighteten Button die Maus auch noch �ber dem Button ist
	void TestMouseOver();

protected:

	/// Texturfarbe des Buttons
	TextureColor tc;
	/// Status des Buttons (gedr�ckt, erhellt usw. durch Maus ausgel�st)
	ButtonState state;
	/// Hat der Button einen 3D-Rand?
	bool border;
	/// Button dauerhaft gedr�ckt?
	bool check;
	/// Button "erleuchtet"?
	bool illuminated;
	/// Button angeschalten?
	bool enabled;
};

/// Button mit Text
class ctrlTextButton : public ctrlButton, public ctrlBaseText
{
public:

	ctrlTextButton(Window *parent, unsigned int id, unsigned short x, unsigned short y,
		unsigned short width, unsigned short height, const TextureColor tc,
		const std::string& text,  glArchivItem_Font *font, const std::string& tooltip);

protected:

	/// Abgeleitete Klassen m�ssen erweiterten Button-Inhalt zeichnen (Text in dem Fall)
	void DrawContent() const;
};


/// Button mit einem Bild
class ctrlImageButton : public ctrlButton
{
public:

	ctrlImageButton(Window *parent, unsigned int id, unsigned short x, unsigned short y,
		unsigned short width, unsigned short height, const TextureColor tc,
		glArchivItem_Bitmap * const image, const std::string& tooltip);

public:

	/// Setzt Bild des Buttons
	void SetImage(glArchivItem_Bitmap *image) { this->image = image; }
	/// Tauscht Bilder
	void SwapImage(ctrlImageButton *two) { Swap(image, two->image); }
	/// Gibt Bild zur�ck
	glArchivItem_Bitmap * GetButtonImage() const { return image; }
	/// �ndert Farbfilter, mit dem dieses Bild gezeichnet werden soll
	void SetModulationColor(const unsigned modulation_color)
	{ this->modulation_color = modulation_color; }
	

protected:

	/// Abgeleitete Klassen m�ssen erweiterten Button-Inhalt zeichnen (Text in dem Fall)
	void DrawContent() const;

protected:

	/// Bild
	glArchivItem_Bitmap *image;
	/// Farbe mit der das Bild gezeichnet werden soll
	unsigned modulation_color;
};

/// Button mit Farbe
class ctrlColorButton : public ctrlButton, public ColorControlInterface
{
public:

	ctrlColorButton(Window *parent, unsigned int id, unsigned short x, unsigned short y,
		unsigned short width, unsigned short height, const TextureColor tc,
		unsigned int fillColor, const std::string& tooltip);

public:

	/// Setzt die Farbe des Controls
	virtual void SetColor(const unsigned int fill_color);

protected:

	/// Abgeleitete Klassen m�ssen erweiterten Button-Inhalt zeichnen (Text in dem Fall)
	void DrawContent() const;

protected:

	unsigned int fillColor;
};



#endif // CTRLBUTTON_H_INCLUDED
