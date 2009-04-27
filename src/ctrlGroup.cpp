// $Id: ctrlGroup.cpp 4652 2009-03-29 10:10:02Z FloSoft $
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
#include "ctrlGroup.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/**
 *  Konstruktor von @p ctrlGroup.
 *
 *  @author FloSoft
 */
ctrlGroup::ctrlGroup(Window *parent,
					 unsigned int id,
					 bool scale)
	: Window(0, 0, id, parent)
{
	SetScale(scale);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Zeichenmethode
 *
 *  @author FloSoft
 */
bool ctrlGroup::Draw_(void)
{
	// Steuerelemente zeichnen
	DrawControls();

	return true;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_ButtonClick(const unsigned int ctrl_id)
{
	parent->Msg_Group_ButtonClick(this->id, ctrl_id);  
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_EditEnter(const unsigned int ctrl_id)
{
	parent->Msg_Group_EditEnter(this->id, ctrl_id);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_EditChange(const unsigned int ctrl_id)
{
	parent->Msg_Group_EditChange(this->id, ctrl_id);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_TabChange(const unsigned int ctrl_id, const unsigned short tab_id)
{
	parent->Msg_Group_TabChange(this->id, ctrl_id, tab_id);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_ListSelectItem(const unsigned int ctrl_id, const unsigned short selection)
{
	parent->Msg_Group_ListSelectItem(this->id, ctrl_id, selection);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_ComboSelectItem(const unsigned int ctrl_id, const unsigned short selection)
{
	parent->Msg_Group_ComboSelectItem(this->id, ctrl_id, selection);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_CheckboxChange(const unsigned int ctrl_id, const bool checked)
{
	parent->Msg_Group_CheckboxChange(this->id, ctrl_id, checked);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_ProgressChange(const unsigned int ctrl_id, const unsigned short position)
{
	parent->Msg_Group_ProgressChange(this->id, ctrl_id, position);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_ScrollShow(const unsigned int ctrl_id, const bool visible)
{
	parent->Msg_Group_ScrollShow(this->id, ctrl_id, visible);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_OptionGroupChange(const unsigned int ctrl_id, const unsigned short selection)
{
	parent->Msg_Group_OptionGroupChange(this->id, ctrl_id, selection);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_Timer(const unsigned int ctrl_id)
{
	parent->Msg_Group_Timer(this->id, ctrl_id);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_TableSelectItem(const unsigned int ctrl_id, const unsigned short selection)
{
	parent->Msg_Group_TableSelectItem(this->id, ctrl_id, selection);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_TableRightButton(const unsigned int ctrl_id, const unsigned short selection)
{
	parent->Msg_Group_TableRightButton(this->id, ctrl_id, selection);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_TableLeftButton(const unsigned int ctrl_id, const unsigned short selection)
{
	parent->Msg_Group_TableLeftButton(this->id, ctrl_id, selection);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
bool ctrlGroup::Msg_LeftDown(const MouseCoords& mc)
{
	return RelayMouseMessage(&Window::Msg_LeftDown, mc);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
bool ctrlGroup::Msg_RightDown(const MouseCoords& mc)
{
	return RelayMouseMessage(&Window::Msg_RightDown, mc);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
bool ctrlGroup::Msg_LeftUp(const MouseCoords& mc)
{
	return RelayMouseMessage(&Window::Msg_LeftUp, mc);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
bool ctrlGroup::Msg_RightUp(const MouseCoords& mc)
{
	return RelayMouseMessage(&Window::Msg_RightUp,mc);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
bool ctrlGroup::Msg_MouseMove(const MouseCoords& mc)
{
	return RelayMouseMessage(&Window::Msg_MouseMove, mc);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
bool ctrlGroup::Msg_KeyDown(const KeyEvent& ke)
{
	return RelayKeyboardMessage(&Window::Msg_KeyDown, ke);
}


///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_Group_ButtonClick(const unsigned int group_id, const unsigned int ctrl_id)
{
	parent->Msg_Group_ButtonClick(this->id, ctrl_id);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_Group_EditEnter(const unsigned int group_id, const unsigned int ctrl_id)
{
	parent->Msg_Group_EditEnter(this->id, ctrl_id);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_Group_EditChange(const unsigned int group_id, const unsigned int ctrl_id)
{
	parent->Msg_Group_EditChange(this->id, ctrl_id);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_Group_TabChange(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short tab_id)
{
	parent->Msg_Group_TabChange(this->id, ctrl_id, tab_id);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_Group_ListSelectItem(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short selection)
{
	parent->Msg_Group_ListSelectItem(this->id, ctrl_id, selection);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_Group_ComboSelectItem(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short selection)
{
	parent->Msg_Group_ComboSelectItem(this->id, ctrl_id, selection);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_Group_CheckboxChange(const unsigned int group_id, const unsigned int ctrl_id, const bool checked)
{
	parent->Msg_Group_CheckboxChange(this->id, ctrl_id, checked);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_Group_ProgressChange(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short position)
{
	parent->Msg_Group_ProgressChange(this->id, ctrl_id, position);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_Group_ScrollShow(const unsigned int group_id, const unsigned int ctrl_id, const bool visible)
{
	parent->Msg_Group_ScrollShow(this->id, ctrl_id, visible);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_Group_OptionGroupChange(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short selection)
{
	parent->Msg_Group_OptionGroupChange(this->id, ctrl_id, selection);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_Group_Timer(const unsigned int group_id, const unsigned int ctrl_id)
{
	parent->Msg_Group_Timer(this->id, ctrl_id);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_Group_TableSelectItem(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short selection)
{
	parent->Msg_Group_TableSelectItem(this->id, ctrl_id, selection);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_Group_TableRightButton(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short selection)
{
	parent->Msg_Group_TableRightButton(this->id, ctrl_id, selection);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void ctrlGroup::Msg_Group_TableLeftButton(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short selection)
{
	parent->Msg_Group_TableLeftButton(this->id, ctrl_id, selection);
}
