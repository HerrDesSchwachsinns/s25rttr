// $Id: dskSelectMap.h 6401 2010-05-04 11:07:04Z OLiver $
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
#ifndef dskSELECTMAP_H_INCLUDED
#define dskSELECTMAP_H_INCLUDED

#pragma once

#include "Desktop.h"

#include "LobbyInterface.h"
#include "ClientInterface.h"
#include "iwDirectIPCreate.h"

struct CreateServerInfo;

class dskSelectMap : 
	public Desktop, 
	public ClientInterface,
	public LobbyInterface
{
	/// Kartenpfad
	std::string map_path;

public:
	/// Konstruktor von @p dskSelectMap.
	dskSelectMap(const CreateServerInfo& csi);
	~dskSelectMap();

private:
	/// Callbackfunktion zum Eintragen einer Karte in der Tabelle.
	static void FillTable(const std::string& filename, void *param);

	void Msg_OptionGroupChange(const unsigned int ctrl_id, const unsigned short selection);
	void Msg_ButtonClick(const unsigned int ctrl_id);
	void Msg_MsgBoxResult(const unsigned msgbox_id, const MsgboxResult mbr);
	void Msg_TableSelectItem(const unsigned int ctrl_id, const unsigned short selection);
	void Msg_TableChooseItem(const unsigned ctrl_id, const unsigned short selection);

	void CI_NextConnectState(const ConnectState cs);
	void CI_Error(const ClientError ce);

	void LC_Created(void);
	void LC_Status_Error(const std::string &error);
	
	/// Startet das Spiel mit einer bestimmten Auswahl in der Tabelle
	void StartServer();

private:
	CreateServerInfo csi;
};

#endif //!dskSELECTMAP_H_INCLUDED

