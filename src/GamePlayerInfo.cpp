// $Id: GamePlayerInfo.cpp 4933 2009-05-24 12:29:23Z OLiver $
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
#include "GamePlayerInfo.h"

#include "VideoDriverWrapper.h"
#include "GameMessage.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
// Konstruktor
GamePlayerInfo::GamePlayerInfo(const unsigned playerid) :
	playerid(playerid),
	defeated(false),
	ps(PS_FREE),
	is_host(false),
	nation(NAT_AFRICANS),
	team(TM_NOTEAM),
	color(0),
	ping(0),
	rating(0),
	ready(false)
{
}

/// Deserialisierungskonstruktor
GamePlayerInfo::GamePlayerInfo(const unsigned playerid, Serializer * ser) :
	playerid(playerid),
	ps(PlayerState(ser->PopUnsignedChar())),
	name(ser->PopString()),
	origin_name(ser->PopString()),
	is_host(ser->PopBool()),
	nation(Nation(ser->PopUnsignedChar())),
	team(Team(ser->PopUnsignedChar())),
	color(ser->PopUnsignedChar()),
	ping(ser->PopUnsignedInt()),
	rating(ser->PopUnsignedShort()),
	ready(ser->PopBool())
{
}

///////////////////////////////////////////////////////////////////////////////
// Destruktor
GamePlayerInfo::~GamePlayerInfo(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// Rausschmeisser
void GamePlayerInfo::clear(void)
{
	name = "";
	defeated = false;
	ps = PS_FREE;
	/*nation = team = color = 0;*/
	ping = rating  = 0;
	ready = false;
}

/// serialisiert die Daten.
void GamePlayerInfo::serialize(Serializer * ser) const
{
	ser->PushUnsignedChar(static_cast<unsigned char>(ps));
	ser->PushString(name);
	ser->PushString(origin_name);
	ser->PushBool(is_host);
	ser->PushUnsignedChar(static_cast<unsigned char>(nation));
	ser->PushUnsignedChar(team);
	ser->PushUnsignedChar(color);
	ser->PushUnsignedInt(ping);
	ser->PushUnsignedShort(rating);
	ser->PushBool(ready);
	
}

void GamePlayerInfo::SwapPlayer(GamePlayerInfo& two)
{
	/// Besiegt?
	Swap(ps,two.ps);
	Swap(defeated,two.defeated);
	Swap(name,two.name);
	Swap(is_host,two.is_host);
	Swap(ping,two.ping);
	Swap(rating,two.rating);
	Swap(checksum,two.checksum);
	Swap(ready,two.ready);
}

