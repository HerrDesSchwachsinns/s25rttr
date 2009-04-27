// $Id: noSkeleton.cpp 4652 2009-03-29 10:10:02Z FloSoft $
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
#include "noSkeleton.h"

#include "Loader.h"
#include "macros.h"
#include "GameWorld.h"
#include "EventManager.h"
#include "Random.h"
#include "SerializedGameData.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

noSkeleton::noSkeleton(const unsigned short x, const unsigned short y)
	: noCoordBase(NOP_ENVIRONMENT,x,y), 
	type(0), current_event(em->AddEvent(this, 15000 + RANDOM.Rand(__FILE__,__LINE__,obj_id,10000)))
{
}

noSkeleton::~noSkeleton()
{
}

void noSkeleton::Destroy_noSkeleton()
{
	gwg->SetNO(NULL, x, y);

	// ggf Event abmelden
	if(current_event)
		em->RemoveEvent(current_event);
	
	Destroy_noCoordBase();
}

void noSkeleton::Serialize_noSkeleton(SerializedGameData * sgd) const
{
	Serialize_noCoordBase(sgd);

	sgd->PushUnsignedChar(type);
	sgd->PushObject(current_event,true);
}

noSkeleton::noSkeleton(SerializedGameData * sgd, const unsigned obj_id) : noCoordBase(sgd,obj_id),
type(sgd->PopUnsignedChar()),
current_event(sgd->PopObject<EventManager::Event>(GOT_EVENT))
{
	
}

void noSkeleton::Draw(int x, int y)
{
	GetImage(map_lst, 547+type)->Draw(x,y,0,0,0,0,0,0);
}

void noSkeleton::HandleEvent(const unsigned int id)
{
	if(!type)
	{
		// weiter verwesen, dann sp�ter sterben nach ner zuf�lligen Zeit
		type = 1;
		current_event = em->AddEvent(this,10000+RANDOM.Rand(__FILE__,__LINE__,obj_id,10000));
	}
	else
	{
		// ganz weg damit
		current_event = 0;
		em->AddToKillList(this);
	}
}
