// $Id: GameMessageInterface.h 5853 2010-01-04 16:14:16Z FloSoft $
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
#ifndef GAMEMESSAGEINTERFACE_H_INCLUDED
#define GAMEMESSAGEINTERFACE_H_INCLUDED

#pragma once

#include "MessageInterface.h"

class GameMessage_Ping;
class GameMessage_Pong;
class GameMessage_Server_Type;
class GameMessage_Server_TypeOK;
class GameMessage_Server_Password;
class GameMessage_Server_Name;
class GameMessage_Server_Start;
class GameMessage_Server_Chat;
class GameMessage_Server_Async;
class GameMessage_Player_Id;
class GameMessage_Player_Name;
class GameMessage_Player_List;
class GameMessage_Player_Toggle_State;
class GameMessage_Player_Toggle_Nation;
class GameMessage_Player_Toggle_Team;
class GameMessage_Player_Toggle_Color;
class GameMessage_Player_Kicked;
class GameMessage_Player_Ping;
class GameMessage_Player_New;
class GameMessage_Player_Ready;
class GameMessage_Player_Swap;
class GameMessage_Map_Info;
class GameMessage_Map_Data;
class GameMessage_Map_Checksum;
class GameMessage_Map_ChecksumOK;
class GameMessage_GGSChange;
class GameMessage_Pause;
class GameMessage_Server_NWFDone;
class GameMessage_GameCommand;

class GameMessageInterface : public MessageInterface
{
protected:
	virtual ~GameMessageInterface() {}

public:
	virtual void OnNMSPing(const GameMessage_Ping& msg);
	virtual void OnNMSPong(const GameMessage_Pong& msg);

	virtual void OnNMSServerType(const GameMessage_Server_Type& msg);
	virtual void OnNMSServerTypeOK(const GameMessage_Server_TypeOK& msg);
	virtual void OnNMSServerPassword(const GameMessage_Server_Password& msg);
	virtual void OnNMSServerName(const GameMessage_Server_Name& msg);
	virtual void OnNMSServerStart(const GameMessage_Server_Start& msg);
	virtual void OnNMSServerChat(const GameMessage_Server_Chat& msg);
	virtual void OnNMSServerAsync(const GameMessage_Server_Async& msg);

	virtual void OnNMSPlayerId(const GameMessage_Player_Id& msg);
	virtual void OnNMSPlayerName(const GameMessage_Player_Name& msg);
	virtual void OnNMSPlayerList(const GameMessage_Player_List& msg);
	virtual void OnNMSPlayerToggleState(const GameMessage_Player_Toggle_State& msg);
	virtual void OnNMSPlayerToggleNation(const GameMessage_Player_Toggle_Nation& msg);
	virtual void OnNMSPlayerToggleTeam(const GameMessage_Player_Toggle_Team& msg);
	virtual void OnNMSPlayerToggleColor(const GameMessage_Player_Toggle_Color& msg);
	virtual void OnNMSPlayerKicked(const GameMessage_Player_Kicked& msg);
	virtual void OnNMSPlayerPing(const GameMessage_Player_Ping& msg);
	virtual void OnNMSPlayerNew(const GameMessage_Player_New& msg);
	virtual void OnNMSPlayerReady(const GameMessage_Player_Ready& msg);
	virtual void OnNMSPlayerSwap(const GameMessage_Player_Swap& msg);

	virtual void OnNMSMapInfo(const GameMessage_Map_Info& msg);
	virtual void OnNMSMapData(const GameMessage_Map_Data& msg);
	virtual void OnNMSMapChecksum(const GameMessage_Map_Checksum& msg);
	virtual void OnNMSMapChecksumOK(const GameMessage_Map_ChecksumOK& msg);


	virtual void OnNMSPause(const GameMessage_Pause& msg);
	virtual void OnNMSServerDone(const GameMessage_Server_NWFDone& msg);
	virtual void OnNMSGameCommand(const GameMessage_GameCommand& msg);

	virtual void OnNMSGGSChange(const GameMessage_GGSChange& msg);
};

#endif //!GAMEMESSAGEINTERFACE_H_INCLUDED
