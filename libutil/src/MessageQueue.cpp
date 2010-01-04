// $Id: MessageQueue.cpp 5853 2010-01-04 16:14:16Z FloSoft $
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
#include "MessageQueue.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/**
 *  Destruktor von @p MessageQueue
 *
 *  @author FloSoft
 */
MessageQueue::~MessageQueue(void)
{
	clear();
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Copy-Konstruktor von @p MessageQueue, um Messages neu zu erstellen, ansonsten werden
 *  nur die Pointer kopiert!
 *
 *  @author OLiver
 */
MessageQueue::MessageQueue(const MessageQueue& mq) : messages(mq.messages.size()), createfunction(mq.createfunction)
{
	for(unsigned i = 0;i<mq.messages.size();++i)
		messages[i] = mq.messages[i]->duplicate();
}

/// Zuweisungsoperator, da Messages kopiert werden m�ssen
MessageQueue& MessageQueue::operator=(const MessageQueue& mq)
{
	messages.resize(mq.messages.size());
	for(unsigned i = 0;i<mq.messages.size();++i)
		messages[i] = mq.messages[i]->duplicate();

	return *this;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  entfernt alle Elemente aus der Queue.
 *
 *  @author FloSoft
 */
void MessageQueue::clear(void)
{
	for(QueueIt It = messages.begin(); It < messages.end(); ++It)
		delete (*It);
	messages.clear();
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  entfernt das vorderste Element aus der Queue.
 *
 *  @author FloSoft
 */
void MessageQueue::pop(void)
{
	if(messages.size() == 0)
		return;

	QueueIt It = messages.begin();
	delete (*It);

	messages.erase(It);
}

///////////////////////////////////////////////////////////////////////////////
/// ruft eine nachricht ab und h�ngt sie in die queue
bool MessageQueue::recv(Socket *sock, bool wait)
{
	if(!sock)
		return false;

	// Nachricht abrufen
	int error = -1;
	Message *msg = Message::recv(sock, error, wait, createfunction);

	if(msg)
	{
		push(msg);
		return true;
	}

	// noch nicht alles empfangen, true liefern f�r okay (error == -1 bedeutet fehler
	return (error >= 0);
}

///////////////////////////////////////////////////////////////////////////////
/*
 *  
 *
 *  @author FloSoft
 */
bool MessageQueue::send(Socket *sock, int max, unsigned int sizelimit)
{
	if(!sock || !sock->isValid())
		return false;

	// send-queue abarbeiten
	int count = 0;
	for(QueueIt It = messages.begin(); It < messages.end(); It = messages.begin())
	{
		if(count > max)
			break;

		// maximal 1 gro�es Paket verschicken
		if(count > 0 && (*It)->GetLength() > sizelimit)
			break;

		if((*It)->getId() > 0)
		{
			if(!(*It)->send(sock))
			{
				LOG.lprintf("Sending Message to server failed\n");
				return false;
			}
		}

		pop();

		++count;
	}
	return true;
}
