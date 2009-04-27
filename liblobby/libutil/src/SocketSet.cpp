// $Id: SocketSet.cpp 4652 2009-03-29 10:10:02Z FloSoft $
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
#include "SocketSet.h"

#include "Socket.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/**
 *  Standardkonstruktor von @p SocketSet.
 *
 *  @author OLiver
 */
SocketSet::SocketSet(void)
{
	Clear();
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  r�umt das @p SocketSet auf.
 *
 *  @author OLiver
 */
void SocketSet::Clear(void)
{
	FD_ZERO(&set);

	highest = INVALID_SOCKET;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�gt ein @p Socket zum @p SocketSet hinzu.
 *
 *  @param[in] sock Socket welches hinzugef�gt werden soll
 *
 *  @author OLiver
 *  @author FloSoft
 */
void SocketSet::Add(Socket &sock)
{
	// Socket holen
	SOCKET s = *sock.GetSocket();
	if(s == INVALID_SOCKET)
		return;

	// hinzuf�gen
	FD_SET(s, &set);

	// neues gr��tes Socket setzen
	if(s > highest || highest == INVALID_SOCKET)
		highest = s;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�hrt einen Select auf dem @p SocketSet aus.
 *
 *  @param[in] wait  Timeout des Aufrufs
 *  @param[in] which Welcher der "Kan�le" soll abgefragt werden? (0 = read, 1 = write, 2 = error)
 *
 *  @return liefert SOCKET_ERROR bei Fehler, Null bei Timeout, gr��er Null f�r die Anzahl der bereiten Sockets im Set
 *
 *  @author FloSoft
 */
int SocketSet::Select(int timeout, int which)
{
	if(highest == INVALID_SOCKET)
		return SOCKET_ERROR;

	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = timeout;

	fd_set *read, *write, *except;
	read = write = except = NULL;

	// unser Set zuweisen
	switch(which)
	{
	case 0: { read   = &set; } break;
	case 1: { write  = &set; } break;
	default:
	case 2: { except = &set; } break;
	}

	// Select ausf�hren
	return select( (int)highest + 1, read, write, except, &tv);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  ist ein @p Socket im @p SocketSet ?
 *
 *  @param[in] sock Socket welches im Set gesucht werden soll
 *
 *  @return liefert true falls das @p Socket im Set enthalten ist, false andernfalls
 *
 *  @author OLiver
 *  @author FloSoft
 */
bool SocketSet::InSet(Socket &sock)
{
	SOCKET s = *sock.GetSocket();

	// Bei ung�ltigem Socket ists nicht drin!
	if(s == INVALID_SOCKET)
		return false;

	// Ist unser Socket im Set?
	if(FD_ISSET(s, &set))
		return true;

	// nein, nicht drin
	return false;
}
