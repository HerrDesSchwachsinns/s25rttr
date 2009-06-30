// $Id: noBuilding.cpp 5144 2009-06-30 07:45:36Z OLiver $
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
#include "noBuilding.h"
#include "GameWorld.h"
#include "Loader.h"
#include "noFire.h"
#include "EventManager.h"
#include "SerializedGameData.h"
#include "FOWObjects.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

noBuilding::noBuilding(const BuildingType type,const unsigned short x, const unsigned short y,const unsigned char player,const Nation nation)
: noBaseBuilding(NOP_BUILDING,type,x,y,player), opendoor(0)
{
}

void noBuilding::Destroy_noBuilding()
{
	// Feuer erzeugen (bei H�tten und Bergwerken kleine Feuer, bei allen anderen gro�e!)
	// Feuer setzen
	gwg->SetNO(new noFire(x,y,(GetSize() == BQ_HUT || GetSize() == BQ_MINE)?0:1),x,y);

	Destroy_noBaseBuilding();
}

void noBuilding::Serialize_noBuilding(SerializedGameData * sgd) const
{
	Serialize_noBaseBuilding(sgd);

	sgd->PushUnsignedChar(opendoor);
}

noBuilding::noBuilding(SerializedGameData * sgd, const unsigned obj_id) : noBaseBuilding(sgd,obj_id),
opendoor(sgd->PopUnsignedChar())
{
}

void noBuilding::DrawBaseBuilding(int x,int y)
{
	GetBobImage(nation, 250+5*type)->Draw(x,y,0,0,0,0,0,0);
	// ACHTUNG nicht jedes Geb�ude hat einen Schatten !!
	if(GetBobImage(nation, 250+5*type+1))
		GetBobImage(nation, 250+5*type+1)->Draw(x,y,0,0,0,0,0,0,COLOR_SHADOW);
	// Falls T�r offen ist und es ein T�r-Bild gibt, offene T�r zeichnen
	if(opendoor && GetBobImage(nation, 250+5*type+4))
		GetBobImage(nation, 250+5*type+4)->Draw(x,y,0,0,0,0,0,0);
}

void noBuilding::GotWorker(Job job, noFigure * worker)
{
}

FOWObject * noBuilding::CreateFOWObject() const
{
	return new fowBuilding(type,nation);
}
