// $Id: LobbyInterface.cpp 5853 2010-01-04 16:14:16Z FloSoft $
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

///////////////////////////////////////////////////////////////////////////////
// Header
#include "main.h"
#include "LobbyInterface.h"

void LobbyInterface::LC_LoggedIn(const std::string &email) { }
void LobbyInterface::LC_Registered(void) { }
void LobbyInterface::LC_Connected(void) { }
void LobbyInterface::LC_Created(void) { }

void LobbyInterface::LC_Status_Waiting(void) { }
void LobbyInterface::LC_Status_Error(const std::string &error) { }

void LobbyInterface::LC_Chat(const std::string &player, const std::string &text) { }

///////////////////////////////////////////////////////////////////////////////
/** 
 *  Status: Verbindung verloren.
 *
 *  @author FloSoft
 */
void LobbyInterface::LC_Status_ConnectionLost()
{
	LC_Status_Error(_("Connection to Host closed!"));
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  Status: fehlerhafte Anfrage / kaputte Daten.
 *
 *  @author FloSoft
 */
void LobbyInterface::LC_Status_IncompleteMessage()
{
	LC_Status_Error(_("Too short Message received!"));
}
