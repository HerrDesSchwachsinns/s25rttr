// $Id: ctrlList.h 4793 2009-05-04 15:37:10Z OLiver $
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
#ifndef CTRLLIST_H_INCLUDED
#define CTRLLIST_H_INCLUDED

#pragma once

#include "Window.h"

class ctrlList : public Window
{
public:
	/// Konstruktor von @p ctrlList.
	ctrlList(Window *parent, unsigned int id, unsigned short x, unsigned short y, unsigned short width, unsigned short height, TextureColor tc, glArchivItem_Font *font);
	/// Destruktor von @p ctrlList.
	virtual ~ctrlList(void);

	/// Neuen String zur Listbox hinzuf�gen.
	void AddString(const std::string& text);
	/// Ver�ndert einen String
	void SetString(const std::string& text, const unsigned id);
	/// Listbox leeren.
	void DeleteAllItems(void);
	/// liefert den Wert einer Zeile.
	const std::string& GetItemText(unsigned short line) const;
	/// liefert den Wert der aktuell gew�hlten Zeile.
	const std::string& GetSelItemText(void) const { return GetItemText(selection); };
	/// setzt die H�he.
	void SetHeight(unsigned short height);
	/// Vertauscht zwei Zeilen.
	void Swap(unsigned short first, unsigned short second);
	/// L�scht ein Element
	void Remove(const unsigned short index);

	unsigned short GetLineCount(void) const { return static_cast<unsigned short>(lines.size()); }
	unsigned short GetSelection(void) const { return static_cast<unsigned short>(selection); };
	void SetSelection(unsigned short selection) { if(selection < lines.size()) this->selection = selection; }
	void SetWidth(unsigned short width) { this->width = width; }
	unsigned short GetWidth() const { return width; }
	unsigned short GetHeight() const { return height; }
	
	virtual bool Msg_MouseMove(const MouseCoords& mc);
	virtual bool Msg_LeftDown(const MouseCoords& mc);
	virtual bool Msg_LeftUp(const MouseCoords& mc);
	virtual bool Msg_WheelUpUp(const MouseCoords& mc);
	virtual bool Msg_WheelDownUp(const MouseCoords& mc);
protected:
	/// Zeichenmethode.
	virtual bool Draw_(void);

private:
	unsigned short width;
	unsigned short height;
	TextureColor tc;
	glArchivItem_Font *font;

	std::vector<std::string> lines;

	unsigned short selection;
	unsigned short mouseover;
	unsigned int pagesize;

};

#endif // CTRLLIST_H_INCLUDED
