// $Id: IngameWindow.h 4836 2009-05-08 20:31:23Z OLiver $
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
#ifndef INGAMEWINDOW_H_INCLUDED
#define INGAMEWINDOW_H_INCLUDED

#pragma once

#include "Window.h"
#include "const_gui_ids.h"

class IngameWindow : public Window
{
public:
	/// Konstruktor von @p IngameWindow.
	IngameWindow(unsigned int id, unsigned short x, unsigned short y, unsigned short width, unsigned short height, const std::string& title, glArchivItem_Bitmap *background, bool modal = false);
	/// Destruktor von @p IngameWindow.
	~IngameWindow(void);

	/// setzt die Fenster-ID.
	void SetID(unsigned int id) { this->id = id; }
	/// liefert die Fenster-ID.
	unsigned int GetID(void) { return id; }

	/// setzt den Hintergrund.
	void SetBackground(glArchivItem_Bitmap *background) { this->background = background; }
	/// liefert den Hintergrund.
	glArchivItem_Bitmap *GetBackground(void) { return background; }

	/// setzt den Fenstertitel.
	void SetTitle(const std::string& title) { this->title = title; }
	/// liefert den Fenstertitel.
	const std::string& GetTitle(void) { return title; }

	/// setzt die Breite des Fensters.
	void SetWidth(unsigned short width) { this->width = width; }
	/// liefert die Breite des Fensters.
	unsigned short GetWidth(void) const { return width; }

	/// setzt die H�he des Fensters.
	void SetHeight(unsigned short height) { this->height = height; }
	/// liefert die H�he des Fensters.
	unsigned short GetHeight(void) const { return (minimized?31:height); }

	/// merkt das Fenster zum Schlie�en vor.
	void Close(bool closeme = true) { this->closeme = closeme; }
	/// soll das Fenster geschlossen werden.
	bool ShouldBeClosed() { return closeme; }

	/// minimiert das Fenster.
	void SetMinimized(bool minimized = true) { this->minimized = minimized; }
	/// ist das Fenster minimiert?
	bool GetMinimized() { return minimized; }

	/// "modalisiert" das Fenster.
	void SetModal(bool modal = true) { this->modal = modal; }
	/// ist das Fenster ein modales Fenster?
	bool GetModal(void) { return modal; }

	void MouseLeftDown(const MouseCoords& mc);
	void MouseLeftUp(const MouseCoords& mc);
	void MouseMove(const MouseCoords& mc);

protected:
	virtual bool Draw_(void);

	/// Verschiebt Fenster in die Bildschirmmitte
	void MoveToCenter();
	/// Verschiebt Fenster neben die Maus
	void MoveNextToMouse();

	/// Weiterleitung von Nachrichten erlaubt oder nicht?
	bool IsMessageRelayAllowed() const;

protected:
	unsigned short width;
	unsigned short height;
	std::string title;
	glArchivItem_Bitmap *background;
	unsigned short last_x;
	unsigned short last_y;
	bool last_down;
	bool last_down2;
	ButtonState button_state[2];

private:
	bool modal;
	bool closeme;
	bool minimized;
	bool move;
};

#endif // !INGAMEWINDOW_H_INCLUDED
