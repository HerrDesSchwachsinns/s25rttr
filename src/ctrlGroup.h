// $Id: ctrlGroup.h 4793 2009-05-04 15:37:10Z OLiver $
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
#ifndef CTRLGROUP_H_INCLUDED
#define CTRLGROUP_H_INCLUDED

#pragma once

#include "Window.h"

class ctrlGroup : public Window
{
public:
	ctrlGroup(Window *parent, unsigned int id, bool scale = false);

	virtual void Msg_ButtonClick(const unsigned int ctrl_id);
	virtual void Msg_EditEnter(const unsigned int ctrl_id);
	virtual void Msg_EditChange(const unsigned int ctrl_id);
	virtual void Msg_TabChange(const unsigned int ctrl_id, const unsigned short tab_id);
	virtual void Msg_ListSelectItem(const unsigned int ctrl_id, const unsigned short selection);
	virtual void Msg_ComboSelectItem(const unsigned int ctrl_id, const unsigned short selection);
	virtual void Msg_CheckboxChange(const unsigned int ctrl_id, const bool checked);
	virtual void Msg_ProgressChange(const unsigned int ctrl_id, const unsigned short position);
	virtual void Msg_ScrollShow(const unsigned int ctrl_id, const bool visible);
	virtual void Msg_OptionGroupChange(const unsigned int ctrl_id, const unsigned short selection);
	virtual void Msg_Timer(const unsigned int ctrl_id);
	virtual void Msg_TableSelectItem(const unsigned int ctrl_id, const unsigned short selection);
	virtual void Msg_TableRightButton(const unsigned int ctrl_id, const unsigned short selection);
	virtual void Msg_TableLeftButton(const unsigned int ctrl_id, const unsigned short selection);

	virtual void Msg_Group_ButtonClick(const unsigned int group_id, const unsigned int ctrl_id);
	virtual void Msg_Group_EditEnter(const unsigned int group_id, const unsigned int ctrl_id);
	virtual void Msg_Group_EditChange(const unsigned int group_id, const unsigned int ctrl_id);
	virtual void Msg_Group_TabChange(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short tab_id);
	virtual void Msg_Group_ListSelectItem(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short selection);
	virtual void Msg_Group_ComboSelectItem(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short selection);
	virtual void Msg_Group_CheckboxChange(const unsigned int group_id, const unsigned int ctrl_id, const bool checked);
	virtual void Msg_Group_ProgressChange(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short position);
	virtual void Msg_Group_ScrollShow(const unsigned int group_id, const unsigned int ctrl_id, const bool visible);
	virtual void Msg_Group_OptionGroupChange(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short selection);
	virtual void Msg_Group_Timer(const unsigned int group_id, const unsigned int ctrl_id);
	virtual void Msg_Group_TableSelectItem(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short selection);
	virtual void Msg_Group_TableRightButton(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short selection);
	virtual void Msg_Group_TableLeftButton(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short selection);

	virtual bool Msg_LeftDown(const MouseCoords& mc);
	virtual bool Msg_RightDown(const MouseCoords& mc);
	virtual bool Msg_LeftUp(const MouseCoords& mc);
	virtual bool Msg_RightUp(const MouseCoords& mc);
	virtual bool Msg_WheelUpDown(const MouseCoords& mc);
	virtual bool Msg_WheelUpUp(const MouseCoords& mc);
	virtual bool Msg_WheelDownDown(const MouseCoords& mc);
	virtual bool Msg_WheelDownUp(const MouseCoords& mc);
	virtual bool Msg_MouseMove(const MouseCoords& mc);
	virtual bool Msg_KeyDown(const KeyEvent& ke);

protected:
	virtual bool Draw_(void);

public:
	struct MsgParam
	{
		Msg msg;
		unsigned int id;
		void *param;
	};
};

#endif // !CTRLGROUP_H_INCLUDED
