// $Id: iwAction.h 4652 2009-03-29 10:10:02Z FloSoft $
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
#ifndef iwACTION_H_INCLUDED
#define iwACTION_H_INCLUDED

#pragma once

#include "IngameWindow.h"
#include "ctrlGroup.h"

class dskGameInterface;
class GameWorldViewer;

class iwAction : public IngameWindow
{
public:

	/// Konstanten f�r ActionWindow-Flag-Tab - Typen
	enum
	{
		AWFT_NORMAL = 0,
		AWFT_HQ,         ///< von der HQ-Flagge kann nur eine Stra�e gebaut werden
		AWFT_STOREHOUSE, ///< von einer Lagerhaus-Flagge kann nur eine Stra�e gebaut werden oder die Flagge abgerissen werden
		AWFT_WATERFLAG   ///< Flagge mit Anker drauf (Wasserstra�e kann gebaut werden)
	};

	/// Struktur mit den Tabs, die angestellt werden sollen
	class Tabs
	{
	public:

		/// Haupttabs
		bool build, setflag, watch, flag, cutroad, attack;
		/// Geb�ude-Bau-Tabs
		enum BuildTab { BT_HUT = 0, BT_HOUSE, BT_CASTLE, BT_MINE} build_tabs;

		Tabs() : build(false), setflag(false), watch(false), flag(false), cutroad(false), attack(false),
			build_tabs(BT_HUT) {}
	};

private:

	dskGameInterface *const gi;
	GameWorldViewer * const gwv;

	unsigned short selected_x;
	unsigned short selected_y;
	unsigned short last_x;
	unsigned short last_y;

	unsigned int selected_soldiers_count;
	unsigned int available_soldiers_count;

public:
	iwAction(dskGameInterface *const gi, GameWorldViewer * const gwv, const Tabs& tabs, unsigned short selected_x, unsigned short selected_y, int mouse_x, int mouse_y, unsigned int params, bool military_buildings);
	~iwAction();

	/// Gibt zur�ck, auf welchen Punkt es sich bezieht
	unsigned short GetSelectedX() const { return selected_x; }
	unsigned short GetSelectedY() const { return selected_y; }

private:

	void Msg_Group_ButtonClick(const unsigned int group_id, const unsigned int ctrl_id);
	void Msg_TabChange(const unsigned int ctrl_id, const unsigned short tab_id);
	void Msg_Group_TabChange(const unsigned group_id,const unsigned int ctrl_id, const unsigned short tab_id);
	void Msg_PaintAfter();

	inline void OnMsgGroup(unsigned int id, ctrlGroup *group, ctrlGroup::MsgParam param);
	inline void Msg_ButtonClick_TabBuild(const unsigned int ctrl_id);
	inline void Msg_ButtonClick_TabCutRoad(const unsigned int ctrl_id);
	inline void Msg_ButtonClick_TabFlag(const unsigned int ctrl_id);
	inline void Msg_ButtonClick_TabAttack(const unsigned int ctrl_id);
	inline void Msg_ButtonClick_TabSetFlag(const unsigned int ctrl_id);
	inline void Msg_ButtonClick_TabWatch(const unsigned int ctrl_id);

};

#endif // !iwACTION_H_INCLUDED

	
