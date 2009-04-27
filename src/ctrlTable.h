// $Id: ctrlTable.h 4652 2009-03-29 10:10:02Z FloSoft $
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
#ifndef CTRLTABLE_H_INCLUDED
#define CTRLTABLE_H_INCLUDED

#pragma once

#include "ctrlScrollBar.h"

class ctrlTable : public Window
{
public:
	ctrlTable(Window *parent, unsigned int id, unsigned short x, unsigned short y, unsigned short width,  unsigned short height, TextureColor tc, glArchivItem_Font *font, unsigned short column_count, va_list liste);
	virtual ~ctrlTable(void);

	/// l�scht alle Items.
	void DeleteAllItems(void);
	/// setzt die Auswahl.
	void SetSelection(unsigned short selection, bool left = true);
	/// f�gt eine Zeile hinzu.
	void AddRow(unsigned int alwaysnull, ...);
	/// liefert den Wert eines Feldes.
	const std::string &GetItemText(unsigned short row, unsigned short column) const;
	/// sortiert die Zeilen.
	void SortRows(unsigned short column, bool *direction = NULL);
	unsigned short GetSortColumn() { return sort_column; }
	bool GetSortDirection() { return sort_direction; }
	unsigned short GetRowCount() { return static_cast<unsigned short>(rows.size()); }
	unsigned short GetColumnCount() { return static_cast<unsigned short>(columns.size()); }
	unsigned short GetSelection(bool left = true) { return (left ? row_l_selection : row_r_selection); }
	
	virtual bool Msg_LeftDown(const MouseCoords& mc);
	virtual bool Msg_RightDown(const MouseCoords& mc);
	virtual bool Msg_LeftUp(const MouseCoords& mc);
	virtual bool Msg_MouseMove(const MouseCoords& mc);
	virtual void Msg_ButtonClick(const unsigned int ctrl_id);
	virtual void Msg_ScrollShow(const unsigned int ctrl_id, const bool visible);

protected:

	virtual bool Draw_(void);

	/// Setzt die Breite und Position der Buttons ohne Scrolleiste
	void ResetButtonWidths();

private:
	unsigned short width;
	unsigned short height;
	TextureColor tc;
	glArchivItem_Font *font;

	unsigned short header_height;
	unsigned short line_count;

	struct COLUMN {
		/// Breite der Spalten in Promille von der Tabellenl�nge
		unsigned short width;
		std::string title;
	};
	std::vector<COLUMN> columns;

	unsigned short row_l_selection;
	unsigned short row_r_selection;

	unsigned short sort_column;
	bool sort_direction;

	struct ROW {
		std::vector<std::string> columns;
	};
	std::vector<ROW> rows;
};

#endif // !CTRLTABLE_H_INCLUDED
