// $Id: noBuildingSite.cpp 5312 2009-07-22 18:02:04Z OLiver $
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
#include "noBuildingSite.h"

#include "GameClient.h"
#include "GameWorld.h"
#include "BuildingConsts.h"
#include "DoorConsts.h"
#include "Ware.h"

#include "noFlag.h"
#include "noExtension.h"
#include "nofBuilder.h"

#include "SerializedGameData.h"

#include "nofPlaner.h"

#include "FOWObjects.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

noBuildingSite::noBuildingSite(const BuildingType type,const unsigned short x, const unsigned short y, const unsigned char player)
: noBaseBuilding(NOP_BUILDINGSITE,type,x,y,player), state(STATE_BUILDING), planer(0), builder(0), boards(0), stones(0), used_boards(0),used_stones(0), build_progress(0)
{
	// �berpr�fen, ob die Baustelle erst noch planiert werden muss (nur bei mittleren/gro�en Geb�uden)
	if(GetSize() == BQ_HOUSE || GetSize() == BQ_CASTLE || GetSize() == BQ_HARBOR)
	{
		// H�he auf dem Punkt, wo die Baustelle steht
		int altitude = gwg->GetNode(x,y).altitude;

		for(unsigned i = 0;i<6;++i)
		{
			// Richtung 4 wird nicht planiert (Flagge)
			if(i != 4)
			{
				// Gibt es da Differenzen?
				if(altitude - gwg->GetNodeAround(x,y,i).altitude != 0)
					state = STATE_PLANING;
			}
		}
	}

	// Wir h�tten gerne einen Planierer/Bauarbeiter...
	gwg->GetPlayer(player)->AddJobWanted((state == STATE_PLANING) ? JOB_PLANER : JOB_BUILDER,this);

	// Bauwaren anfordern
	OrderConstructionMaterial();

	// Baustelle in den Index eintragen, damit die Wirtschaft auch Bescheid wei�
	gwg->GetPlayer(player)->AddBuildingSite(this);
}

noBuildingSite::~noBuildingSite()
{
}

void noBuildingSite::Destroy_noBuildingSite()
{
	// Bauarbeiter/Planierer Bescheid sagen
	if(builder)
		builder->LostWork();
	else if(planer)
		planer->LostWork();
	else
		gwg->GetPlayer(player)->JobNotWanted(this);

	// Bestellte Waren Bescheid sagen
	for(list<Ware*>::iterator it = ordered_boards.begin();it.valid();++it)
		WareNotNeeded((*it));
	for(list<Ware*>::iterator it = ordered_stones.begin();it.valid();++it)
		WareNotNeeded((*it));

	// und Feld wird leer
	gwg->SetNO(0,x,y);
	// Anbauten drumrum ggf. zerst�ren
	DestroyBuildingExtensions();

	gwg->RecalcBQAroundPointBig(x,y);

	// Baustelle wieder aus der Liste entfernen
	gwg->GetPlayer(player)->RemoveBuildingSite(this);

	Destroy_noBaseBuilding();
}

void noBuildingSite::Serialize_noBuildingSite(SerializedGameData * sgd) const
{
	Serialize_noBaseBuilding(sgd);

	sgd->PushUnsignedChar(static_cast<unsigned char>(state));
	sgd->PushObject(planer,true);
	sgd->PushObject(builder,true);
	sgd->PushUnsignedChar(boards);
	sgd->PushUnsignedChar(stones);
	sgd->PushUnsignedChar(used_boards);
	sgd->PushUnsignedChar(used_stones);
	sgd->PushUnsignedChar(build_progress);
	sgd->PushObjectList(ordered_boards,true);
	sgd->PushObjectList(ordered_stones,true);
}

noBuildingSite::noBuildingSite(SerializedGameData * sgd, const unsigned obj_id) : noBaseBuilding(sgd,obj_id),
state(static_cast<State>(sgd->PopUnsignedChar())),
planer(sgd->PopObject<nofPlaner>(GOT_NOF_PLANER)),
builder(sgd->PopObject<nofBuilder>(GOT_NOF_BUILDER)),
boards(sgd->PopUnsignedChar()),
stones(sgd->PopUnsignedChar()),
used_boards(sgd->PopUnsignedChar()),
used_stones(sgd->PopUnsignedChar()),
build_progress(sgd->PopUnsignedChar())
{
	sgd->PopObjectList(ordered_boards,GOT_WARE);
	sgd->PopObjectList(ordered_stones,GOT_WARE);
}

void noBuildingSite::OrderConstructionMaterial()
{
	// Bei Planieren keine Waren bestellen
	if(state == STATE_PLANING)
		return;

	// Bretter
	Ware * w;
	for(unsigned char i = used_boards+boards+ordered_boards.size();i<BUILDING_COSTS[gwg->GetPlayer(player)->nation][type].boards;++i)
	{
		if( (w = gwg->GetPlayer(player)->OrderWare(GD_BOARDS,this)) )
			ordered_boards.push_front(w);
	}
	// Steine
	for(unsigned char i = used_stones+stones+ordered_stones.size();i<BUILDING_COSTS[gwg->GetPlayer(player)->nation][type].stones;++i)
	{
		if( (w = gwg->GetPlayer(player)->OrderWare(GD_STONES,this)) )
			ordered_stones.push_back(w);
	}
}

void noBuildingSite::Draw(int x, int y)
{
	if(state == STATE_PLANING)
	{
		// Baustellenschild mit Schatten zeichnen
		LOADER.GetNationImageN(GAMECLIENT.GetLocalPlayer()->nation, 450)->Draw(x,y);
		LOADER.GetNationImageN(GAMECLIENT.GetLocalPlayer()->nation, 451)->Draw(x,y,0,0,0,0,0,0,COLOR_SHADOW);
	}
	else
	{
		// Baustellenstein und -schatten zeichnen
		LOADER.GetNationImageN(GAMECLIENT.GetLocalPlayer()->nation, 455)->Draw(x,y);
		LOADER.GetNationImageN(GAMECLIENT.GetLocalPlayer()->nation, 456)->Draw(x,y,0,0,0,0,0,0,COLOR_SHADOW);


		// Waren auf der Baustelle

		// Bretter
		for(unsigned char i = 0;i<boards;++i)
			LOADER.GetMapImageN(2200+GD_BOARDS)->Draw(x+GetDoorPointX()-5,y+GetDoorPointY()-10-i*4,0,0,0,0,0,0);
		// Steine
		for(unsigned char i = 0;i<stones;++i)
			LOADER.GetMapImageN(2200+GD_STONES)->Draw(x+GetDoorPointX()+8,y+GetDoorPointY()-12-i*4,0,0,0,0,0,0);

		// bis dahin gebautes Haus zeichnen

		// Rohbau

		// ausrechnen, wie weit er ist
		unsigned int p1 = 0, p2 = 0;

		if(BUILDING_COSTS[nation][GetBuildingType()].stones)
		{
			// Haus besteht aus Steinen und Brettern
			p1 = min<unsigned int>(build_progress,BUILDING_COSTS[nation][GetBuildingType()].boards*8);
			p2 = BUILDING_COSTS[nation][GetBuildingType()].boards*8;
		}
		else
		{
			// Haus besteht nur aus Brettern, dann 50:50
			p1 = min<unsigned int>(build_progress,BUILDING_COSTS[nation][GetBuildingType()].boards*4);
			p2 = BUILDING_COSTS[nation][GetBuildingType()].boards*4;
		}

		glArchivItem_Bitmap *image;
		unsigned short progress, build_height;

		// Normal
		image = LOADER.GetNationImageN(nation,250 + 5*type + 2);
		if(image)
		{
			progress = p1*image->getHeight() / p2;
			build_height = image->getHeight() - progress;
			if(progress != 0)
				image->Draw(x, y+build_height, 0, 0, 0, build_height, 0, progress);

		}

		// Schatten
		image = LOADER.GetNationImageN(nation,250 + 5*type + 3);
		if(image)
		{
			progress = p1*image->getHeight() / p2;
			build_height = image->getHeight() - progress;
			if(progress != 0)
				image->Draw(x, y+build_height, 0, 0, 0, build_height, 0, progress, COLOR_SHADOW);
		}

		// Das richtige Haus
		if(BUILDING_COSTS[nation][GetBuildingType()].stones)
		{
			// Haus besteht aus Steinen und Brettern
			p1 = ((build_progress >  BUILDING_COSTS[nation][GetBuildingType()].boards*8) ? (build_progress-BUILDING_COSTS[nation][GetBuildingType()].boards*8):0);
			p2 = BUILDING_COSTS[nation][GetBuildingType()].stones*8;
		}
		else
		{
			// Haus besteht nur aus Brettern, dann 50:50
			p1 = ((build_progress >  BUILDING_COSTS[nation][GetBuildingType()].boards*4)?(build_progress-BUILDING_COSTS[nation][GetBuildingType()].boards*4):0);
			p2 = BUILDING_COSTS[nation][GetBuildingType()].boards*4;
		}

		// Normal
		image = LOADER.GetNationImageN(nation, 250 + 5*type);
		if(image)
		{
			progress = p1*image->getHeight() / p2;
			build_height = image->getHeight() - progress;
			if(progress != 0)
				image->Draw(x, y+build_height, 0, 0, 0, build_height, 0, progress);
		}


		image = LOADER.GetNationImageN(nation, 250 + 5*type + 1);
		if(image)
		{
			progress = p1*image->getHeight() / p2;
			build_height = image->getHeight() - progress;
			if(progress != 0)
				image->Draw(x, y+build_height, 0, 0, 0, build_height, 0, progress, COLOR_SHADOW);
		}
	}

	//char number[256];
	//sprintf(number,"%u",obj_id);
	//NormalFont->Draw(x,y,number,0,0xFFFF0000);
}

/// Erzeugt von ihnen selbst ein FOW Objekt als visuelle "Erinnerung" f�r den Fog of War
FOWObject * noBuildingSite::CreateFOWObject() const
{
	return new fowBuildingSite(state == STATE_PLANING,type,nation,build_progress);
}


void noBuildingSite::GotWorker(Job job, noFigure * worker)
{
	// Aha, wir haben nen Planierer/Bauarbeiter bekommen
	if(state == STATE_PLANING)
	{
		assert(worker->GetGOT() == GOT_NOF_PLANER);
		planer = static_cast<nofPlaner*>(worker);
	}
	else
	{
		assert(worker->GetGOT() == GOT_NOF_BUILDER);
		builder = static_cast<nofBuilder*>(worker);
	}
}

void noBuildingSite::Abrogate()
{
	planer = 0;
	builder = 0;

	gwg->GetPlayer(player)->AddJobWanted((state == STATE_PLANING) ? JOB_PLANER : JOB_BUILDER,this);
}

unsigned noBuildingSite::CalcDistributionPoints(noRoadNode * start,const GoodType type)
{
	// Beim Planieren brauchen wir noch gar nichts
	if(state == STATE_PLANING)
		return 0;

	// Wenn wir schon genug Baumaterial haben, brauchen wir nichts mehr
	if((BUILDING_COSTS[nation][this->type].boards == ordered_boards.size() + boards + used_boards && type == GD_BOARDS) ||
		(BUILDING_COSTS[nation][this->type].stones == ordered_stones.size() + stones + used_stones && type == GD_STONES))
		return 0;

	// 10000 als Basis w�hlen, damit man auch noch was abziehen kann
	unsigned short points = 10000;

	// Baumaterial mit einberechnen (wer noch am wenigsten braucht, soll mehr Punkte kriegen, da ja m�glichst
	// zuerst Geb�ude fertiggestellt werden sollten)
	points -= (BUILDING_COSTS[nation][type].boards-ordered_boards.size()-boards-used_boards)*20;
	points -= (BUILDING_COSTS[nation][type].stones-ordered_stones.size()-stones-used_stones)*20;


	// Baupriorit�t mit einberechnen (niedriger = h�here Priorit�t, daher - !)
	points -= gwg->GetPlayer(player)->GetBuidingSitePriority(this)*30;
	return points;
}

void noBuildingSite::AddWare(Ware * ware)
{
	assert(state == STATE_BUILDING);

	if(ware->type == GD_BOARDS)
	{
		for(list<Ware*>::iterator it = ordered_boards.begin(); it.valid(); ++it)
		{
			if(*it == ware)
			{
				ordered_boards.erase(it);
				break;
			}
		}
		++boards;
	}
	else if(ware->type == GD_STONES)
	{
		for(list<Ware*>::iterator it = ordered_stones.begin(); it.valid(); ++it)
		{
			if(*it == ware)
			{
				ordered_stones.erase(it);
				break;
			}
		}
		++stones;
	}

	// Inventur entsprechend verringern
	gwg->GetPlayer(player)->DecreaseInventoryWare(ware->type,1);
	gwg->GetPlayer(player)->RemoveWare(ware);
	delete ware;
}

void noBuildingSite::WareLost(Ware * ware)
{
	assert(state == STATE_BUILDING);

	// Aus der Bestellliste entfernen
	for(list<Ware*>::iterator it = ordered_boards.begin(); it.valid(); ++it)
	{
		if((*it) == ware)
		{
			ordered_boards.erase(it);
			break;
		}
	}
	for(list<Ware*>::iterator it = ordered_stones.begin(); it.valid(); ++it)
	{
		if((*it) == ware)
		{
			ordered_stones.erase(it);
			break;
		}
	}

	OrderConstructionMaterial();
}


void noBuildingSite::TakeWare(Ware * ware)
{
	assert(state == STATE_BUILDING);

	// Ware in die Bestellliste aufnehmen
	if(ware->type == GD_BOARDS)
		ordered_boards.push_back(ware);
	else if(ware->type == GD_STONES)
		ordered_stones.push_back(ware);
}


bool noBuildingSite::IsBuildingComplete()
{
	return (build_progress == BUILDING_COSTS[nation][type].boards*8+BUILDING_COSTS[nation][type].stones*8);
}

unsigned char noBuildingSite::GetBuildProgress(bool percent) const
{
	if(!percent)
		return build_progress;

	double costs = BUILDING_COSTS[nation][type].boards*8 + BUILDING_COSTS[nation][type].stones*8;
	double progress = (build_progress / costs) * 100.0;

	return (unsigned char)progress;
}

/// Aufgerufen, wenn Planierung beendet wurde
void noBuildingSite::PlaningFinished()
{
	/// Normale Baustelle
	state = STATE_BUILDING;
	planer = 0;

	// Wir h�tten gerne einen Bauarbeiter...
	gwg->GetPlayer(player)->AddJobWanted(JOB_BUILDER,this);

	// Bauwaren anfordern
	OrderConstructionMaterial();

}
