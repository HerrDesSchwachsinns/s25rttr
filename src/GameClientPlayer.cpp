// $Id: GameClientPlayer.cpp 4746 2009-04-30 20:10:46Z OLiver $
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
#include "GameClientPlayer.h"
#include "GameWorld.h"
#include "GameClient.h"
#include "Random.h"

#include "GameConsts.h"
#include "MilitaryConsts.h"

#include "RoadSegment.h"
#include "Ware.h"

#include "noFlag.h"
#include "noBuildingSite.h"
#include "nobBaseWarehouse.h"
#include "nobUsual.h"
#include "nobMilitary.h"
#include "nofFlagWorker.h"
#include "nofCarrier.h"

#include "GameInterface.h"

#include "SerializedGameData.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

// Standardbelegung der Transportreihenfolge festlegen
const unsigned char STD_TRANSPORT[35] =
{
	2,12,12,12,12,12,12,12,12,12,10,10,12,12,12,13,1,3,11,11,11,1,9,7,8,1,1,11,0,4,5,6,11,11,1
};

///////////////////////////////////////////////////////////////////////////////
/**
 *  Konstruktor von @p GameClientPlayer.
 *
 *  @author OLiver
 */
GameClientPlayer::GameClientPlayer(const unsigned playerid) : GamePlayerInfo(playerid)
{
	// Erstmal kein HQ (leerer Spieler) wie das bei manchen Karten der Fall ist
	hqy = hqx = 0xFFFF;

	// Verteilung mit Standardwerten f�llen bei Waren mit nur einem Ziel (wie z.B. Mehl, Holz...)
	distribution[GD_FLOUR].client_buildings.push_back(BLD_BAKERY);
	distribution[GD_WOOD].client_buildings.push_back(BLD_SAWMILL);
	distribution[GD_GOLD].client_buildings.push_back(BLD_MINT);
	distribution[GD_IRONORE].client_buildings.push_back(BLD_IRONSMELTER);
	distribution[GD_HAM].client_buildings.push_back(BLD_SLAUGHTERHOUSE);
	distribution[GD_STONES].client_buildings.push_back(BLD_HEADQUARTERS); // BLD_HEADQUARTERS = Baustellen!
	distribution[GD_STONES].client_buildings.push_back(BLD_CATAPULT);


	// Waren mit mehreren m�glichen Zielen erstmal nullen, kann dann im Fenster eingestellt werden
	for(unsigned char i = 0; i < WARE_TYPES_COUNT; ++i)
	{
		memset(distribution[i].percent_buildings, 0, 40*sizeof(unsigned char));
		distribution[i].selected_goal = 0;
	}

	// Standardverteilung der Waren
	GAMECLIENT.visual_settings.distribution[0] = distribution[GD_FISH].percent_buildings[BLD_GRANITEMINE] = 3;
	GAMECLIENT.visual_settings.distribution[1] = distribution[GD_FISH].percent_buildings[BLD_COALMINE] = 5;
	GAMECLIENT.visual_settings.distribution[2] = distribution[GD_FISH].percent_buildings[BLD_IRONMINE] = 7;
	GAMECLIENT.visual_settings.distribution[3] = distribution[GD_FISH].percent_buildings[BLD_GOLDMINE] = 10;

	GAMECLIENT.visual_settings.distribution[4] = distribution[GD_GRAIN].percent_buildings[BLD_MILL] = 5;
	GAMECLIENT.visual_settings.distribution[5] = distribution[GD_GRAIN].percent_buildings[BLD_PIGFARM] = 3;
	GAMECLIENT.visual_settings.distribution[6] = distribution[GD_GRAIN].percent_buildings[BLD_DONKEYBREEDER] = 2;
	GAMECLIENT.visual_settings.distribution[7] = distribution[GD_GRAIN].percent_buildings[BLD_BREWERY] = 3;

	GAMECLIENT.visual_settings.distribution[8] = distribution[GD_IRON].percent_buildings[BLD_ARMORY] = 8;
	GAMECLIENT.visual_settings.distribution[9] = distribution[GD_IRON].percent_buildings[BLD_METALWORKS] = 4;

	GAMECLIENT.visual_settings.distribution[10] = distribution[GD_COAL].percent_buildings[BLD_ARMORY] = 8;
	GAMECLIENT.visual_settings.distribution[11] = distribution[GD_COAL].percent_buildings[BLD_IRONSMELTER] = 7;
	GAMECLIENT.visual_settings.distribution[12] = distribution[GD_COAL].percent_buildings[BLD_MINT] = 10;

	GAMECLIENT.visual_settings.distribution[13] = distribution[GD_BOARDS].percent_buildings[BLD_HEADQUARTERS] = 10;
	GAMECLIENT.visual_settings.distribution[14] = distribution[GD_BOARDS].percent_buildings[BLD_METALWORKS] = 4;
	GAMECLIENT.visual_settings.distribution[15] = distribution[GD_BOARDS].percent_buildings[BLD_SHIPYARD] = 2;

	GAMECLIENT.visual_settings.distribution[16] = distribution[GD_WATER].percent_buildings[BLD_BAKERY] = 6;
	GAMECLIENT.visual_settings.distribution[17] = distribution[GD_WATER].percent_buildings[BLD_BREWERY] = 3;
	GAMECLIENT.visual_settings.distribution[18] = distribution[GD_WATER].percent_buildings[BLD_PIGFARM] = 2;
	GAMECLIENT.visual_settings.distribution[19] = distribution[GD_WATER].percent_buildings[BLD_DONKEYBREEDER] = 3;

	RecalcDistribution();

	GAMECLIENT.visual_settings.order_type = order_type = 0;

	// Baureihenfolge f�llen (0 ist das HQ!)
	for(unsigned char i = 1, j = 0; i < 40; ++i)
	{
		// Diese Ids sind noch nicht besetzt
		if(i==3 || (i>=5 && i<=8) || i == 15 || i == 27 || i == 30)
			continue;

		GAMECLIENT.visual_settings.build_order[j] = build_order[j] = i;
		++j;
	}

	// Transportreihenfolge festlegen
	memcpy(transport,STD_TRANSPORT,35 * sizeof(unsigned char));

	GAMECLIENT.visual_settings.transport_order[0] = STD_TRANSPORT[GD_COINS];
	GAMECLIENT.visual_settings.transport_order[1] = STD_TRANSPORT[GD_SWORD];
	GAMECLIENT.visual_settings.transport_order[2] = STD_TRANSPORT[GD_BEER];
	GAMECLIENT.visual_settings.transport_order[3] = STD_TRANSPORT[GD_IRON];
	GAMECLIENT.visual_settings.transport_order[4] = STD_TRANSPORT[GD_GOLD];
	GAMECLIENT.visual_settings.transport_order[5] = STD_TRANSPORT[GD_IRONORE];
	GAMECLIENT.visual_settings.transport_order[6] = STD_TRANSPORT[GD_COAL];
	GAMECLIENT.visual_settings.transport_order[7] = STD_TRANSPORT[GD_BOARDS];
	GAMECLIENT.visual_settings.transport_order[8] = STD_TRANSPORT[GD_STONES];
	GAMECLIENT.visual_settings.transport_order[9] = STD_TRANSPORT[GD_WOOD];
	GAMECLIENT.visual_settings.transport_order[10] = STD_TRANSPORT[GD_WATER];
	GAMECLIENT.visual_settings.transport_order[11] = STD_TRANSPORT[GD_FISH];
	GAMECLIENT.visual_settings.transport_order[12] = STD_TRANSPORT[GD_HAMMER];
	GAMECLIENT.visual_settings.transport_order[13] = STD_TRANSPORT[GD_BOAT];

	// Milit�r- und Werkzeugeinstellungen
	memset(military_settings, 0, 7);
	military_settings[0] = 5;
	military_settings[1] = 3;
	military_settings[2] = 5;
	military_settings[3] = 3;
	military_settings[4] = 1;
	military_settings[5] = 2;
	military_settings[6] = 5;
	memcpy(GAMECLIENT.visual_settings.military_settings,military_settings,7);
	memset(tools_settings,0,12);
	memcpy(GAMECLIENT.visual_settings.tools_settings,tools_settings,12);

	// Standardeinstellungen kopieren
	memcpy(&GAMECLIENT.default_settings, &GAMECLIENT.visual_settings, sizeof(GameClient::VisualSettings));

	defenders_pos = 0;
	for(unsigned i = 0;i<5;++i)
		defenders[i] = true;

	is_lagging = false;

	// Inventur nullen
	memset(&global_inventory,0,sizeof(global_inventory));
}

void GameClientPlayer::Serialize(SerializedGameData * sgd)
{
	// PlayerStatus speichern, ehemalig
	sgd->PushUnsignedChar(static_cast<unsigned char>(ps));

	// Nur richtige Spieler serialisieren
	if(!(ps == PS_OCCUPIED || ps == PS_KI))
		return;

	sgd->PushObjectList(warehouses,false);

	//sgd->PushObjectList(unoccupied_roads,true);
	sgd->PushObjectList(roads,true);

	sgd->PushUnsignedInt(jobs_wanted.size());
	for(list<JobNeeded>::iterator it = jobs_wanted.begin();it.valid();++it)
	{
		sgd->PushUnsignedChar(it->job);
		sgd->PushObject(it->workplace,false);
	}

	for(unsigned i = 0;i<30;++i)
		sgd->PushObjectList(buildings[i],true);

	sgd->PushObjectList(building_sites,true);

	sgd->PushObjectList(military_buildings,true);

	sgd->PushObjectList(ware_list,true);

	sgd->PushObjectList(flagworkers,false);

	for(unsigned i = 0;i<5;++i)
		sgd->PushBool(defenders[i]);
	sgd->PushUnsignedShort(defenders_pos);

	sgd->PushUnsignedShort(hqx);
	sgd->PushUnsignedShort(hqy);

	for(unsigned i = 0;i<WARE_TYPES_COUNT;++i)
	{
		sgd->PushRawData(distribution[i].percent_buildings,40);
		sgd->PushUnsignedInt(distribution[i].client_buildings.size());
		for(list<BuildingType>::iterator it = distribution[i].client_buildings.begin();it.valid();++it)
			sgd->PushUnsignedChar(*it);
		sgd->PushUnsignedInt(unsigned(distribution[i].goals.size()));
		for(unsigned z = 0;z<distribution[i].goals.size();++z)
			sgd->PushUnsignedChar(distribution[i].goals[z]);
		sgd->PushUnsignedInt(distribution[i].selected_goal);
	}

	sgd->PushUnsignedChar(order_type);

	sgd->PushRawData(build_order,31);

	sgd->PushRawData(transport,WARE_TYPES_COUNT);

	sgd->PushRawData(military_settings,7);

	sgd->PushRawData(tools_settings,12);

	for(unsigned i = 0;i<WARE_TYPES_COUNT;++i)
		sgd->PushUnsignedInt(global_inventory.goods[i]);
	for(unsigned i = 0;i<JOB_TYPES_COUNT;++i)
		sgd->PushUnsignedInt(global_inventory.people[i]);
}

void GameClientPlayer::Deserialize(SerializedGameData * sgd)
{
	// Ehemaligen PS auslesen
	PlayerState origin_ps = PlayerState(sgd->PopUnsignedChar());
	// Nur richtige Spieler serialisieren
	if(!(origin_ps == PS_OCCUPIED || origin_ps == PS_KI))
		return;

	sgd->PopObjectList(warehouses,GOT_UNKNOWN);

	//sgd->PopObjectList(unoccupied_roads,GOT_ROADSEGMENT);
	sgd->PopObjectList(roads,GOT_ROADSEGMENT);

	unsigned list_size = sgd->PopUnsignedInt();
	for(unsigned i = 0;i<list_size;++i)
	{
		JobNeeded nj;
		nj.job = Job(sgd->PopUnsignedChar());
		nj.workplace = sgd->PopObject<noRoadNode>(
			GOT_UNKNOWN);
		jobs_wanted.push_back(nj);

	}

	for(unsigned i = 0;i<30;++i)
		sgd->PopObjectList(buildings[i],GOT_NOB_USUAL);

	sgd->PopObjectList(building_sites,GOT_BUILDINGSITE);

	sgd->PopObjectList(military_buildings,GOT_NOB_MILITARY);

	sgd->PopObjectList(ware_list,GOT_WARE);

	sgd->PopObjectList(flagworkers,GOT_UNKNOWN);

	for(unsigned i = 0;i<5;++i)
		defenders[i] = sgd->PopBool();
	defenders_pos = sgd->PopUnsignedShort();

	hqx = sgd->PopUnsignedShort();
	hqy = sgd->PopUnsignedShort();

	for(unsigned i = 0;i<WARE_TYPES_COUNT;++i)
	{
		sgd->PopRawData(distribution[i].percent_buildings,40);
		list_size = sgd->PopUnsignedInt();
		for(unsigned z = 0;z<list_size;++z)
			distribution[i].client_buildings.push_back(BuildingType(sgd->PopUnsignedChar()));
		unsigned goal_count = sgd->PopUnsignedInt();
		distribution[i].goals.resize(goal_count);
		for(unsigned z = 0;z<goal_count;++z)
			distribution[i].goals[z] = sgd->PopUnsignedChar();
		distribution[i].selected_goal = sgd->PopUnsignedInt();
	}

	order_type = sgd->PopUnsignedChar();

	sgd->PopRawData(build_order,31);

	char str[256] = "";
	for(unsigned char i = 0;i<31;++i)
	{
		char tmp[256];
		sprintf(tmp,"%u ",i);
		strcat(str,tmp);
	}
	strcat(str,"\n");
	puts(str);

	sgd->PopRawData(transport,WARE_TYPES_COUNT);

	sgd->PopRawData(military_settings,7);

	sgd->PopRawData(tools_settings,12);

	for(unsigned i = 0;i<WARE_TYPES_COUNT;++i)
		global_inventory.goods[i] = sgd->PopUnsignedInt();
	for(unsigned i = 0;i<JOB_TYPES_COUNT;++i)
		global_inventory.people[i] = sgd->PopUnsignedInt();

	// Visuelle Einstellungen festlegen
}

void GameClientPlayer::SwapPlayer(GameClientPlayer& two)
{
	GamePlayerInfo::SwapPlayer(two);

	Swap(this->is_lagging,two.is_lagging);
}

nobBaseWarehouse * GameClientPlayer::FindWarehouse(const noRoadNode * const start,bool (*IsWarehouseGood)(nobBaseWarehouse*,const void*),
		const RoadSegment * const forbidden,const bool to_wh,const void * param,const bool use_boat_roads,unsigned * const length)
{
	nobBaseWarehouse * best	= 0;

	unsigned char path = 0xFF, tpath = 0xFF;
	unsigned tlength = 0xFFFFFFFF,best_length = 0xFFFFFFFF;

	for(list<nobBaseWarehouse*>::iterator w = warehouses.begin(); w.valid(); ++w)
	{
		// Lagerhaus geeignet?
		if(IsWarehouseGood(*w,param))
		{
			// Bei der erlaubten Benutzung von Bootsstra�en Waren-Pathfinding benutzen
			if(to_wh)
				tpath = gwg->FindPath(start,*w,1,&tlength,forbidden,use_boat_roads);
			else
				tpath = gwg->FindPath(*w,start,1,&tlength,forbidden,use_boat_roads);

			if((tlength < best_length || !best)	&& tpath!=0xFF)
			{
				path = tpath;
				best_length = tlength;
				best = (*w);
			}
		}
	}

	if(length)
		*length = best_length;

	return best;
}

void GameClientPlayer::NewRoad(RoadSegment * const rs)
{
	// Zu den Stra�en hinzufgen, da's ja ne neue ist
	roads.push_back(rs);

	// Alle Stra�en m�ssen nun gucken, ob sie einen Weg zu einem Warehouse finden
	FindWarehouseForAllRoads();

	// Alle Stra�en m�ssen gucken, ob sie einen Esel bekommen k�nnen
	for(list<RoadSegment*>::iterator it = roads.begin();it!=roads.end();++it)
		(*it)->TryGetDonkey();

	// Alle Arbeitspl�tze m�ssen nun gucken, ob sie einen Weg zu einem Lagerhaus mit entsprechender Arbeitskraft finden
	FindWarehouseForAllJobs(JOB_NOTHING);

	// Alle Baustellen m�ssen nun gucken, ob sie ihr ben�tigtes Baumaterial bekommen (evtl war vorher die Stra�e zum Lagerhaus unterbrochen
	FindMaterialForBuildingSites();

	// Alle Lost-Wares m�ssen gucken, ob sie ein Lagerhaus finden
	FindClientForLostWares();

	// Alle Milit�rgeb�ude m�ssen ihre Truppen �berpr�fen und k�nnen nun ggf. neue bestellen
	// und m�ssen pr�fen, ob sie evtl Gold bekommen
	for(list<nobMilitary*>::iterator it = military_buildings.begin();it.valid();++it)
	{
		(*it)->RegulateTroops();
		(*it)->SearchCoins();
	}
}



void GameClientPlayer::FindClientForLostWares()
{
	// Alle Lost-Wares m�ssen gucken, ob sie ein Lagerhaus finden
	for(list<Ware*>::iterator it = ware_list.begin(); it.valid(); ++it)
	{
		if((*it)->IsLostWare())
		{
			(*it)->FindRouteToWarehouse();
		}

	}
}

void GameClientPlayer::RoadDestroyed()
{
	// Alle Waren, die an Flagge liegen und in Lagerh�usern, m�ssen gucken, ob sie ihr Ziel noch erreichen k�nnen, jetzt wo eine Stra�e fehlt
	for(list<Ware*>::iterator it = ware_list.begin(); it.valid(); ++it)
	{
		if((*it)->LieAtFlag())
		{
			// Liegt die Flagge an einer Flagge, muss ihr Weg neu berechnet werden
			unsigned char last_next_dir = (*it)->GetNextDir();
			(*it)->RecalcRoute();
			(*it)->RemoveWareJobForCurrentDir(last_next_dir);
			// Tr�ger Bescheid sagen
			if((*it)->GetNextDir() != 0xFF)
				(*it)->GetLocation()->routes[(*it)->GetNextDir()]->AddWareJob((*it)->GetLocation());
		}
		else if((*it)->LieInWarehouse())
		{
			if(!(*it)->FindRouteFromWarehouse())
			{
				Ware * ware = *it;

				// Ware aus der Warteliste des Lagerhauses entfernen
				static_cast<nobBaseWarehouse*>((*it)->GetLocation())->CancelWare(ware);
				// Das Ziel wird nun nich mehr beliefert
				ware->NotifyGoalAboutLostWare();
				// Ware aus der Liste raus
				ware_list.erase(&it);
			}
		}
	}
}

bool GameClientPlayer::FindCarrierForRoad(RoadSegment * rs)
{
	unsigned length[2];
	nobBaseWarehouse * best[2];

	// Braucht der ein Boot?
	if(rs->GetRoadType() == RoadSegment::RT_BOAT)
	{
		// dann braucht man Tr�ger UND Boot
		FW::Param_WareAndJob p = { {GD_BOAT,1}, {JOB_HELPER,1} };
		best[0] = FindWarehouse(rs->f1,FW::Condition_WareAndJob,rs,0,&p,false,&length[0]);
		// 2. Flagge des Weges
		best[1] = FindWarehouse(rs->f2,FW::Condition_WareAndJob,rs,0,&p,false,&length[1]);
	}
	else
	{
		// 1. Flagge des Weges
		FW::Param_Job p = { JOB_HELPER, 1 };
		best[0] = FindWarehouse(rs->f1,FW::Condition_Job,rs,0,&p,false,&length[0]);
		// 2. Flagge des Weges
		best[1] = FindWarehouse(rs->f2,FW::Condition_Job,rs,0,&p,false,&length[1]);
	}

	// �berhaupt nen Weg gefunden?
	// Welche Flagge benutzen?
	if(best[0] && (length[0]<length[1]))
		best[0]->OrderCarrier(rs->f1,rs);
	else if(best[1])
		best[1]->OrderCarrier(rs->f2,rs);
	else
		return false;

	return true;
}

void GameClientPlayer::RecalcDistribution()
{
	RecalcDistributionOfWare(GD_FISH);
	RecalcDistributionOfWare(GD_GRAIN);
	RecalcDistributionOfWare(GD_IRON);
	RecalcDistributionOfWare(GD_COAL);
	RecalcDistributionOfWare(GD_BOARDS);
	RecalcDistributionOfWare(GD_WATER);
}



void GameClientPlayer::RecalcDistributionOfWare(const GoodType ware)
{
	// Punktesystem zur Verteilung, in der Liste alle Geb�ude sammeln, die die Ware wollen
	distribution[ware].client_buildings.clear();

	// 1. Anteile der einzelnen Waren ausrechnen

	// Liste von Geb�udetypen, die die Waren wollen
	list<BuildingWhichWantWare> bwww_list;

	unsigned goal_count = 0;

	for(unsigned char i = 0;i<40;++i)
	{
		if(distribution[ware].percent_buildings[i])
		{
			distribution[ware].client_buildings.push_back(static_cast<BuildingType>(i));
			goal_count += distribution[ware].percent_buildings[i];
			BuildingWhichWantWare bwww = {distribution[ware].percent_buildings[i],i};
			bwww_list.push_back(bwww);
		}
	}

	// TODO: evtl noch die counts miteinander k�rzen (ggt berechnen)

	// Array f�r die Geb�udtypen erstellen

	distribution[ware].goals.clear();
	distribution[ware].goals.resize(goal_count);
	for(unsigned i = 0;i<goal_count;++i)
		distribution[ware].goals[i] = 0;

	// In Array schreiben
	float position;
	unsigned char pos;
	for(list<BuildingWhichWantWare>::iterator it = bwww_list.begin();it.valid();++it)
	{
		position = 0;
		// Distanz zwischen zwei gleichen Geb�uden
		float dist = float(goal_count) / float(it->count);

		// M�glichst gleichm��ige Verteilung der Geb�ude auf das Array berechnen
		for(unsigned char i = 0; i < it->count; ++i, position = fmod(position + dist, float(goal_count)) )
		{
			for(pos = (unsigned char)(position + .5); distribution[ware].goals[pos] != 0; pos = (pos + 1) % goal_count);
			distribution[ware].goals[pos] = it->building;
		}
	}

	// Und ordentlich sch�tteln ;)
	//RandomShuffle(distribution[ware].goals,distribution[ware].goal_count);


	//for(unsigned char i = 0;i<distribution[ware].goal_count;++i)
	//	LOG.lprintf("%u ",distribution[ware].goals[i]);
	//LOG.lprintf("\n");


	// Alles f�ngt wieder von vorne an...
	distribution[ware].selected_goal = 0;
}

void GameClientPlayer::FindWarehouseForAllRoads()
{
	for(list<RoadSegment*>::iterator it = roads.begin(); it.valid(); ++it)
	{
		if(!(*it)->carrier[0])
			FindCarrierForRoad(*it);
	}

}

void GameClientPlayer::FindMaterialForBuildingSites()
{
	for(list<noBuildingSite*>::iterator it = building_sites.begin(); it.valid(); ++it)
		(*it)->OrderConstructionMaterial();
}

void GameClientPlayer::AddJobWanted(const Job job,noRoadNode * workplace)
{
	// Und gleich suchen
	if(!FindWarehouseForJob(job,workplace))
	{
		JobNeeded jn = { job, workplace };
		jobs_wanted.push_back(jn);
	}
}

void GameClientPlayer::JobNotWanted(noRoadNode * workplace)
{
	for(list<JobNeeded>::iterator it = jobs_wanted.begin(); it.valid(); ++it)
	{
		if(it->workplace == workplace)
		{
			jobs_wanted.erase(it);
			return;
		}
	}
}

bool GameClientPlayer::FindWarehouseForJob(const Job job, noRoadNode * goal)
{
	FW::Param_Job p = { job, 1 };
	nobBaseWarehouse * wh = FindWarehouse(goal,FW::Condition_Job,0,false,&p,false);

	if(wh)
	{
		// Es wurde ein Lagerhaus gefunden, wo es den geforderten Beruf gibt, also den Typen zur Arbeit rufen
		wh->OrderJob(job,goal);
		return true;
	}

	return false;
}

void GameClientPlayer::FindWarehouseForAllJobs(const Job job)
{
	for(list<JobNeeded>::iterator it = jobs_wanted.begin(); it.valid(); ++it)
	{
		if(job == JOB_NOTHING || it->job == job)
		{
			if(FindWarehouseForJob(it->job,it->workplace))
				jobs_wanted.erase(&it);
		}
	}
}

Ware * GameClientPlayer::OrderWare(const GoodType ware,noBaseBuilding * goal)
{
	/// Gibt es ein Lagerhaus mit dieser Ware?
	FW::Param_Ware p = { ware, 1 };
	nobBaseWarehouse * wh = FindWarehouse(goal,FW::Condition_Ware,0,false,&p,true);

	if(wh)
		return wh->OrderWare(ware,goal);
	else
		return 0;
}

nofCarrier * GameClientPlayer::OrderDonkey(RoadSegment * road)
{
	unsigned length[2];
	nobBaseWarehouse * best[2];

	// 1. Flagge des Weges
	FW::Param_Job p = { JOB_PACKDONKEY, 1 };
	best[0] = FindWarehouse(road->f1,FW::Condition_Job,road,0,&p,false,&length[0]);
	// 2. Flagge des Weges
	best[1] = FindWarehouse(road->f2,FW::Condition_Job,road,0,&p,false,&length[1]);

	// �berhaupt nen Weg gefunden?
	// Welche Flagge benutzen?
	if(best[0] && (length[0]<length[1]))
		return best[0]->OrderDonkey(road,road->f1);
	else if(best[1])
		return best[1]->OrderDonkey(road,road->f2);
	else
		return 0;
}

RoadSegment * GameClientPlayer::FindRoadForDonkey(noRoadNode * start,noRoadNode ** goal)
{
	// Bisher h�chste Tr�gerproduktivit�t und die entsprechende Stra�e dazu
	unsigned best_productivity = 0;
	RoadSegment * best_road = 0;
	// Beste Flagge dieser Stra�e
	*goal = 0;

	for(list<RoadSegment*>::iterator it = roads.begin();it!=roads.end();++it)
	{
		// Braucht die Stra�e einen Esel?
		if((*it)->NeedDonkey())
		{
			// Beste Flagge von diesem Weg, und beste Wegstrecke
			noRoadNode * current_best_goal = 0;
			// Weg zu beiden Flaggen berechnen
			unsigned length1 = 0,length2 = 0;
			gwg->FindPath(start,(*it)->f1,false,&length1,*it);
			gwg->FindPath(start,(*it)->f2,false,&length2,*it);

			// Wenn man zu einer Flagge nich kommt, die jeweils andere nehmen
			if(!length1)
				current_best_goal = (length2)?(*it)->f2 : 0;
			else if(length2)
				current_best_goal = (length1)?(*it)->f1 : 0;
			else
			{
				// ansonsten die k�rzeste von beiden
				current_best_goal = (length1 < length2) ? (*it)->f1 : (*it)->f2;
			}

			// Kein Weg f�hrt hin, n�chste Stra�e bitte
			if(!current_best_goal)
				continue;

			// Jeweiligen Weg bestimmen
			unsigned current_best_way = ((*it)->f1 == current_best_goal) ? length1 : length2;

			// Produktivit�t ausrechnen, *10 die Produktivit�t + die Wegstrecke, damit die
			// auch noch mit einberechnet wird
			unsigned current_productivity = 10*(*it)->carrier[0]->GetProductivity()+current_best_way;

			// Besser als der bisher beste?
			if(current_productivity > best_productivity)
			{
				// Dann wird der vom Thron gesto�en
				best_productivity = current_productivity;
				best_road = (*it);
				*goal = current_best_goal;
			}

		}
	}

	return best_road;
}

noBaseBuilding * GameClientPlayer::FindClientForWare(Ware * ware)
{
	// Wenn es eine Goldm�nze ist, wird das Ziel auf eine andere Art und Weise berechnet
	if(ware->type == GD_COINS)
		return FindClientForCoin(ware);

	noBaseBuilding * bb = 0;
	unsigned best_points = 0;

	for(list<BuildingType>::iterator it = distribution[ware->type].client_buildings.begin(); it.valid(); ++it)
	{
		unsigned way_points,points;

		// BLD_HEADQUARTERS sind Baustellen!!, da HQs ja sowieso nicht gebaut werden k�nnen
		if(*it == BLD_HEADQUARTERS)
		{
			// Bei Baustellen die Extraliste abfragen
			for(list<noBuildingSite*>::iterator i = building_sites.begin(); i.valid(); ++i)
			{
				// Weg dorthin berechnen
				if(gwg->FindPath(ware->GetLocation(),*i,false,&way_points) != 0xFF)
				{
					points = (*i)->CalcDistributionPoints(ware->GetLocation(),ware->type);

					if(points)
					{
						// Die Wegpunkte noch davon abziehen, Verteilung draufaddieren
						points -= way_points;
						points += distribution[ware->type].percent_buildings[BLD_HEADQUARTERS]*30;

							/*char str[256];
							sprintf(str,"gf = %u, points = %u, way_points = %u, distribution = %u  \n", 
							GameClient::inst().GetGFNumber(), points, way_points, distribution[ware->type].percent_buildings[BLD_HEADQUARTERS]);
							GameClient::inst().AddToGameLog(str);*/


						// Besser als der bisher Beste?
						if(points > best_points)
						{
							best_points = points;
							bb = *i;
						}
					}
				}
			}

			//// Bei Baustellen die Extraliste abfragen
			//for(list<noBuildingSite*>::iterator i = building_sites.begin(); i.valid(); ++i)
			//{
			//	// Zus�tzliche Distribution-Punkte draufaddieren, welcher Geb�udetyp bekommt zuerst die Waren?
			//	if(unsigned short distri_points = (*i)->CalcDistributionPoints(ware->GetLocation(),ware->type))
			//	{
			//		points = distri_points + distribution[ware->type].percent_buildings[BLD_HEADQUARTERS]*25;
			//		if((points < best_points || !bb) && points)
			//		{
			//			bb = *i;
			//			best_points = points;
			//		}
			//	}
			//}
		}
		else
		{
			// F�r �brige Geb�ude
			for(list<nobUsual*>::iterator i = buildings[*it-10].begin(); i.valid(); ++i)
			{
				// Weg dorthin berechnen
				if(gwg->FindPath(ware->GetLocation(),*i,false,&way_points) != 0xFF)
				{
					points = (*i)->CalcDistributionPoints(ware->GetLocation(),ware->type);
					// Wenn 0, dann braucht er die Ware nicht
					if(points)
					{
						// Die Wegpunkte noch davon abziehen, Verteilung draufaddieren
						//points -= way_points;
						points -= (unsigned int) (0.5 * way_points);
						//points += distribution[ware->type].percent_buildings[*it]*30;

						//// Verteilung �berpr�fen ob Geb�udetyp an der Reihe ist
						//if(distribution[ware->type].goals.size())
						//{
						//	char str[256];
						//	sprintf(str,"gf = %u, obj_id = %u, selected_goal = %u,  rest = %u\n", 
						//	GameClient::inst().GetGFNumber(), ware->GetObjId(), distribution[ware->type].selected_goal, distribution[ware->type].goals[distribution[ware->type].selected_goal]);
						//	GameClient::inst().AddToGameLog(str);
						//}

						if(distribution[ware->type].goals.size()) {
							if ((*i)->GetBuildingType() == 
								static_cast<BuildingType>(distribution[ware->type].goals[distribution[ware->type].selected_goal])) {
								points += 300;
							} else {
								points -= 300;
							}
						}

						// Besser als der bisher Beste?
						if(points > best_points)
						{
							best_points = points;
							bb = *i;
						}
					}

				}
			}
		}
	}

	if(bb && distribution[ware->type].goals.size())
		distribution[ware->type].selected_goal = (distribution[ware->type].selected_goal + 1) % unsigned(distribution[ware->type].goals.size());

	// Wenn kein Abnehmer gefunden wurde, muss es halt in ein Lagerhaus
	if(!bb)
		bb = FindWarehouse(ware->GetLocation(),FW::Condition_StoreWare,0,true,&ware->type,true);

	// Abnehmer Bescheid sagen
	if(bb)
		bb->TakeWare(ware);

	return bb;
}

nobBaseMilitary * GameClientPlayer::FindClientForCoin(Ware * ware)
{
	nobBaseMilitary * bb = 0;
	unsigned best_points = 0,points;

	// Milit�rgeb�ude durchgehen
	for(list<nobMilitary*>::iterator it = military_buildings.begin();it.valid();++it)
	{
		unsigned way_points;
		// Weg dorthin berechnen
		if(gwg->FindPath(ware->GetLocation(),*it,false,&way_points) != 0xFF)
		{
			points = (*it)->CalcCoinsPoints();
			// Wenn 0, will er gar keine M�nzen (Goldzufuhr gestoppt)
			if(points)
			{
				// Die Wegpunkte noch davon abziehen
				points -= way_points;
				// Besser als der bisher Beste?
				if(points > best_points)
				{
					best_points = points;
					bb = *it;
				}
			}

		}
	}

	// Wenn kein Abnehmer gefunden wurde, muss es halt in ein Lagerhaus
	if(!bb)
		bb = FindWarehouse(ware->GetLocation(),FW::Condition_StoreWare,0,true,&ware->type,true);

	// Abnehmer Bescheid sagen
	if(bb)
		bb->TakeWare(ware);

	return bb;
}

void GameClientPlayer::AddBuildingSite(noBuildingSite * building_site)
{
	building_sites.push_back(building_site);
}

void GameClientPlayer::RemoveBuildingSite(noBuildingSite
* building_site)
{
	for(list<noBuildingSite*>::iterator it = building_sites.begin(); it.valid(); ++it)
	{
		if(building_site == *it)
		{
			building_sites.erase(it);
			return;
		}
	}
}

void GameClientPlayer::AddUsualBuilding(nobUsual * building)
{
	buildings[building->GetBuildingType()-10].push_back(building);
}

void GameClientPlayer::RemoveUsualBuilding(nobUsual * building)
{
	buildings[building->GetBuildingType()-10].erase(building);
}

void GameClientPlayer::AddMilitaryBuilding(nobMilitary * building)
{
	military_buildings.push_back(building);

}

void GameClientPlayer::RemoveMilitaryBuilding(nobMilitary * building)
{
	military_buildings.erase(building);
	TestDefeat();
}

/// Gibt Liste von Geb�uden des Spieler zur�ck
const list<nobUsual*>& GameClientPlayer::GetBuildings(const BuildingType type)
{
	assert(type >= 10);

	return buildings[type-10];
}

/// Liefert die Anzahl aller Geb�ude einzeln
void GameClientPlayer::GetBuildingCount(BuildingCount& bc) const
{
	memset(&bc,0,sizeof(bc));

	// Normale Geb�ude z�hlen
	for(unsigned i = 0;i<30;++i)
		bc.building_counts[i+10] = buildings[i].size();
	// Lagerh�user z�hlen
	for(list<nobBaseWarehouse*>::const_iterator it = warehouses.begin();it.valid();++it)
		++bc.building_counts[(*it)->GetBuildingType()];
	// Milit�rgeb�ude z�hlen
	for(list<nobMilitary*>::const_iterator it = military_buildings.begin();it.valid();++it)
		++bc.building_counts[(*it)->GetBuildingType()];
	// Baustellen z�hlen
	for(list<noBuildingSite*>::const_iterator it = building_sites.begin();it.valid();++it)
		++bc.building_site_counts[(*it)->GetBuildingType()];
}


/// Berechnet die durschnittlichen Produktivit�t eines jeden Geb�udetyps
/// (erwartet als Argument ein 40-er Array!)
void GameClientPlayer::CalcProductivities(std::vector<unsigned short>& productivities)
{
	assert(productivities.size() == 40);

	for(unsigned i = 0;i<30;++i)
	{
		// Durschnittliche Produktivit�t errrechnen, indem man die Produktivit�ten aller Geb�ude summiert
		// und den Mittelwert bildet
		unsigned total_productivity = 0;

		for(list<nobUsual*>::iterator it = buildings[i].begin();it.valid();++it)
			total_productivity += *(*it)->GetProduktivityPointer();

		if(buildings[i].size())
			total_productivity /= buildings[i].size();

		productivities[i+10] = static_cast<unsigned short>(total_productivity);
	}
}


unsigned GameClientPlayer::GetBuidingSitePriority(const noBuildingSite * building_site)
{
	if(order_type)
	{
		// Spezielle Reihenfolge

		// Typ in der Reihenfolge suchen und Position als Priorit�t zur�ckgeben
		for(unsigned i = 0;i<31;++i)
		{
			if(building_site->GetBuildingType() == static_cast<BuildingType>(build_order[i]))
			{
			/*	char str[256];
				sprintf(str,"gf = %u, pr = %u\n",
				GameClient::inst().GetGFNumber(), i);
				GameClient::inst().AddToGameLog(str);*/
				return i;
			}
		}
	}
	else
	{
		// Reihenfolge der Bauauftr�ge, also was zuerst in Auftrag gegeben wurde, wird zuerst gebaut
		unsigned i = 0;
		for(list<noBuildingSite*>::iterator it = building_sites.begin(); it.valid(); ++it, ++i)
		{
			if(building_site == *it)
				return i;
		}
	}

	LOG.lprintf("GameClientPlayer::GetBuidingSitePriority: ERROR: BuildingSite or type of it not found in the list!\n");
	return 0xFFFFFFFF;
}

void GameClientPlayer::ConvertTransportData(const unsigned char * const transport_data)
{
	// Mit Hilfe der Standardbelegung l�sst sich das recht einfach konvertieren:
	for(unsigned i = 0;i<35;++i)
	{
		for(unsigned z = 0;z<14;++z)
		{
			if(transport_data[z] == STD_TRANSPORT[i])
			{
				transport[i] = z;
				break;
			}
		}

	}
}

bool GameClientPlayer::IsAlly(const unsigned char player) const
{
	return (playerid == player || (team && team == GAMECLIENT.GetPlayer(player)->team));
}

void GameClientPlayer::OrderTroops(nobMilitary * goal, unsigned count)
{
	// Solange Lagerh�user nach Soldaten absuchen, bis entweder keins mehr �brig ist oder alle Soldaten bestellt sind
	nobBaseWarehouse * wh;
	do
	{
		unsigned param_count = 1;
		wh = FindWarehouse(goal,FW::Condition_Troops,0,false,&param_count,false);
		if(wh)
		{
			unsigned order_count = min(wh->GetSoldiersCount(),count);
			count -=order_count;
			wh->OrderTroops(goal,order_count);
		}
	} while(count && wh);
}


void GameClientPlayer::RegulateAllTroops()
{
	for(list<nobMilitary*>::iterator it = military_buildings.begin();it.valid();++it)
		(*it)->RegulateTroops();
}

	/// Sucht f�r EINEN Soldaten ein neues Milit�rgeb�ude, als Argument wird Referenz auf die 
	/// entsprechende Soldatenanzahl im Lagerhaus verlangt
void GameClientPlayer::NewSoldierAvailable(const unsigned& soldier_count)
{
	// solange laufen lassen, bis soldier_count = 0, d.h. der Soldat irgendwohin geschickt wurde
	// Zuerst nach unbesetzten Milit�rgeb�ude schauen
	for(list<nobMilitary*>::iterator it = military_buildings.begin();it.valid() && soldier_count;++it)
	{
		if((*it)->IsNewBuilt())
			(*it)->RegulateTroops();
	}

	if(!soldier_count)
		return;

	// Als n�chstes Geb�ude in Grenzn�he
	for(list<nobMilitary*>::iterator it = military_buildings.begin();it.valid() && soldier_count;++it)
	{
		if((*it)->GetFrontierDistance() == 2)
			(*it)->RegulateTroops();
	}

	if(!soldier_count)
		return;

	// Und den Rest ggf.
	for(list<nobMilitary*>::iterator it = military_buildings.begin();it.valid() && soldier_count;++it)
		(*it)->RegulateTroops();

}

void GameClientPlayer::CallFlagWorker(const unsigned short x, const unsigned short y, const Job job)
{
	/// Flagge rausfinden
	noFlag * flag = gwg->GetSpecObj<noFlag>(x,y);
	/// Lagerhaus mit Geologen finden
	FW::Param_Job p = { job, 1 };
	nobBaseWarehouse * wh = FindWarehouse(flag,FW::Condition_Job,0,false,&p,false);

	/// Wenns eins gibt, dann rufen
	if(wh)
		wh->OrderJob(job,flag);
}


void GameClientPlayer::FlagDestroyed(noFlag * flag)
{
	// Alle durchgehen und ggf. sagen, dass sie keine Flagge mehr haben, wenn das ihre Flagge war, die zerst�rt wurde
	for(list<nofFlagWorker*>::iterator it = flagworkers.begin();it.valid();++it)
	{
		if((*it)->GetFlag() == flag)
		{
			(*it)->LostWork();
			flagworkers.erase(&it);
		}
	}
}

void GameClientPlayer::RefreshDefenderList()
{
	/// Die Verteidigungsliste muss erneuert werden
	memset(defenders,0,5);
	for(unsigned i = 0;i<5;++i)
		defenders[i] = (i<military_settings[2]);
	// und ordentlich sch�tteln
	RANDOM.Shuffle(defenders,5);

	defenders_pos = 0;
}

void GameClientPlayer::ChangeMilitarySettings(const unsigned char* military_settings)
{
	memcpy(this->military_settings,military_settings,7);
	/// Truppen m�ssen neu kalkuliert werden
	RegulateAllTroops();
	/// Die Verteidigungsliste muss erneuert werden
	RefreshDefenderList();
}

bool GameClientPlayer::ShouldSendDefender()
{
	// Wenn wir schon am Ende sind, muss die Verteidgungsliste erneuert werden
	if(defenders_pos == 4)
		RefreshDefenderList();

	return defenders[defenders_pos++];
}

void GameClientPlayer::TestDefeat()
{
	/// Keine Milit�rgeb�ude, keine Lagerh�user (HQ,H�fen) -> kein Land --> verloren
	if(!military_buildings.size() && !warehouses.size())
	{
		defeated = true;

		// GUI Bescheid sagen
		gwg->GetGameInterface()->GI_PlayerDefeated(playerid);
	}
}

//void GameClientPlayer::GetInventory(unsigned int *wares, unsigned int *figures)
//{
//	// todo: waren in richtige reihenfolge bringen...
//	static GoodType ware_map[31] = {
//		GD_WOOD, GD_BOARDS, GD_STONES, GD_HAM,
//		GD_GRAIN, GD_FLOUR, GD_FISH, GD_MEAT, GD_BREAD,
//		GD_WATER, GD_BEER, GD_COAL, GD_IRONORE,
//		GD_GOLD, GD_IRON, GD_COINS, GD_TONGS, GD_AXE,
//		GD_SAW, GD_PICKAXE, GD_HAMMER, GD_SHOVEL,
//		GD_CRUCIBLE, GD_RODANDLINE, GD_SCYTHE, GD_CLEAVER, GD_ROLLINGPIN,
//		GD_BOW, GD_SWORD, GD_SHIELDROMANS, GD_BOAT
//	};
//
//	/*static Job figure_map[30] = {
//		JOB_HELPER, JOB_BUILDER, JOB_PLANER, JOB_WOODCUTTER,
//		JOB_FORESTER, JOB_STONEMASON, JOB_FISHER, JOB_HUNTER, JOB_CARPENTER,
//		JOB_FARMER, JOB_PIGBREEDER, JOB_DONKEYBREEDER, JOB_MILLER,
//		JOB_BAKER, JOB_BUTCHER, JOB_BREWER, JOB_MINER, JOB_IRONFOUNDER,
//		JOB_ARMORER, JOB_MINTER, JOB_METALWORKER, JOB_SHIPWRIGHT,
//		JOB_GEOLOGIST, JOB_SCOUT, JOB_PACKDONKEY, JOB_PRIVATE, JOB_PRIVATEFIRSTCLASS,
//		JOB_SERGEANT, JOB_OFFICER, JOB_GENERAL
//	};*/
//
//	// Warenlisten der Warenh�user sammeln
//	for(list<nobBaseWarehouse*>::iterator wh = warehouses.begin(); wh.valid(); ++wh)
//		(*wh)->GetInventory(wares, figures);
//
//	if(wares)
//	{
//		// einzelne Waren sammeln
//		for(list<Ware*>::iterator we = ware_list.begin(); we.valid(); ++we)
//		{
//			++(wares[ware_map[(*we)->type]]);
//		}
//	}
//
//	// Todo: einzelne Figuren sammeln
//	if(figures)
//	{
//	}
//}

void GameClientPlayer::Surrender()
{
	defeated = true;

	// GUI Bescheid sagen
	gwg->GetGameInterface()->GI_PlayerDefeated(playerid);
}
