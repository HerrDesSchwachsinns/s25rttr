// $Id: dskOptions.h 6458 2010-05-31 11:38:51Z FloSoft $
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
#ifndef dskOPTIONS_H_INCLUDED
#define dskOPTIONS_H_INCLUDED

#pragma once

#include "Desktop.h"
#include "../driver/src/VideoDriver.h"

/// Klasse des Optionen Desktops.
class dskOptions: public Desktop
{
public:
	dskOptions(void);

private:
	void Msg_OptionGroupChange(const unsigned int ctrl_id, const unsigned short selection);
	void Msg_ButtonClick(const unsigned int ctrl_id);
	void Msg_MsgBoxResult(const unsigned int msgbox_id, const MsgboxResult mbr);

	void Msg_Group_ButtonClick(const unsigned int group_id, const unsigned int ctrl_id);
	void Msg_Group_ProgressChange(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short position);
	void Msg_Group_ComboSelectItem(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short selection);
	void Msg_Group_OptionGroupChange(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short selection);

private:
	std::vector<VideoDriver::VideoMode> video_modes; ///< Vector f�r die Aufl�sungen
};

#endif // !dskOPTIONS_H_INCLUDED
