// $Id: AIJHHelper.cpp 5353 2009-07-31 07:55:57Z FloSoft $
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

#include "AIJHHelper.h"
#include "main.h"
#include "AIPlayerJH.h"

#include "GameClientPlayer.h"
#include "GameWorld.h"
#include "GameCommands.h"
#include "GamePlayerList.h"
#include "nobMilitary.h"
#include "nobHQ.h"
#include "noBuildingSite.h"
#include "MapGeometry.h"

#include <iostream>

void AIJH::BuildJob::ExecuteJob()
{
	if (status == AIJH::JOB_WAITING)
		status = AIJH::JOB_EXECUTING_START;

	switch (status)
	{
	case AIJH::JOB_EXECUTING_START:
		{
			TryToBuild();
		}
		break;

	case AIJH::JOB_EXECUTING_ROAD1:
		{
			BuildMainRoad();
		}
		break;

	case AIJH::JOB_EXECUTING_ROAD2:
		{
			TryToBuildSecondaryRoad();
		}
		break;
	case AIJH::JOB_EXECUTING_ROAD2_2:
		{
			// evtl noch pr�fen ob auch dieser Stra�enbau erfolgreich war?
			aijh->RecalcGround(target_x, target_y, route);
			status = AIJH::JOB_FINISHED;
		}
		break;

	default:
		assert(false);
		break;


	}

	// Evil harbour-hack
	if (type == BLD_HARBORBUILDING && status == AIJH::JOB_FINISHED && target_x != 0xFFFF)
	{
		aijh->aiJobs.push_front(new AIJH::BuildJob(aijh, BLD_SHIPYARD, target_x, target_y));
	}


	// Fertig?
	if (status == AIJH::JOB_FAILED || status == AIJH::JOB_FINISHED)
		return;

	//if (target_x == 0xFFFF && target_y == 0xFFFF)
	//{

	//}

	if (aijh->GetGWB()->IsMilitaryBuildingNearNode(target_x, target_y) && type >= BLD_BARRACKS && type <= BLD_FORTRESS)
	{
		status = AIJH::JOB_FAILED;
#ifdef DEBUG_AI
		std::cout << "Player " << (unsigned)aijh->GetPlayerID() << ", Job failed: Military building too near for " << BUILDING_NAMES[type] << " at " << target_x << "/" << target_y << "." << std::endl;
#endif
		//aijh->nodes[target_x + target_y * aijh->GetGWB()->GetWidth()].
		return;
	}
}

void AIJH::BuildJob::TryToBuild()
{
	MapCoord bx = around_x;
	MapCoord by = around_y;

	if (!aijh->Wanted(type))
	{
		status = AIJH::JOB_FINISHED;
		return;
	}

	bool foundPos = false;

	switch(type)
	{
	case BLD_WOODCUTTER:
		foundPos = aijh->FindBestPosition(bx, by, AIJH::WOOD, BQ_HUT, 20, 15);
		break;
	case BLD_FORESTER:
		foundPos = aijh->FindBestPosition(bx, by, AIJH::WOOD, BQ_HUT, 1, 15);
		break;
	case BLD_QUARRY:
		foundPos = aijh->FindBestPosition(bx, by, AIJH::STONES, BQ_HUT, 40, 15);
		break;
	case BLD_BARRACKS:
	case BLD_GUARDHOUSE:
	case BLD_WATCHTOWER:
	case BLD_FORTRESS:
		foundPos = aijh->FindBestPosition(bx, by, AIJH::BORDERLAND, BUILDING_SIZE[type], 15, true);
		break;
	case BLD_GOLDMINE:
		foundPos = aijh->FindBestPosition(bx, by, AIJH::GOLD, BQ_MINE, 15, true);
		break;
	case BLD_COALMINE:
		foundPos = aijh->FindBestPosition(bx, by, AIJH::COAL, BQ_MINE, 15, true);
		break;
	case BLD_IRONMINE:
		foundPos = aijh->FindBestPosition(bx, by, AIJH::IRONORE, BQ_MINE, 15, true);
		break;
	case BLD_GRANITEMINE:
		foundPos = aijh->FindBestPosition(bx, by, AIJH::GRANITE, BQ_MINE, 15, true);
		break;

	case BLD_FISHERY:
		foundPos = aijh->FindBestPosition(bx, by, AIJH::FISH, BQ_HUT, 15, true);
		break;
	case BLD_STOREHOUSE:
		foundPos = aijh->FindStoreHousePosition(bx, by, 15);
		break;

	case BLD_FARM:
		foundPos = aijh->FindBestPosition(bx, by, AIJH::PLANTSPACE, BQ_CASTLE, 25, 12, true);
		break;

	default:
		foundPos = aijh->SimpleFindPosition(bx, by, BUILDING_SIZE[type], 15);
		break;
	}
	
	if (!foundPos)
	{
		status = JOB_FAILED;
#ifdef DEBUG_AI
		std::cout << "Player " << (unsigned)aijh->GetPlayerID() << ", Job failed: No Position found for " << BUILDING_NAMES[type] << " around " << bx << "/" << by << "." << std::endl;
#endif
		return;
	}

#ifdef DEBUG_AI
	if (type == BLD_FARM)
		std::cout << " Player " << (unsigned)aijh->GetPlayerID() << " built farm at " << bx << "/" << by << " on value of " << aijh->resourceMaps[AIJH::PLANTSPACE][bx+by*aijh->GetGWB()->GetWidth()] << std::endl;
#endif

	aijh->GetGCS().push_back(new gc::SetBuildingSite(bx, by, type));
	target_x = bx;
	target_y = by;
	status = AIJH::JOB_EXECUTING_ROAD1;
	return;
}

void AIJH::BuildJob::BuildMainRoad()
{
	const noBuildingSite *bld;
	if (!(bld = aijh->GetGWB()->GetSpecObj<noBuildingSite>(target_x, target_y)))
	{
		// Pr�fen ob sich vielleicht die BQ ge�ndert hat und damit Bau unm�glich ist
		BuildingQuality bq = aijh->GetGWB()->CalcBQ(target_x, target_y, aijh->playerid);
		if (!(bq >= BUILDING_SIZE[type] && bq < BQ_MINE) // normales Geb�ude
							&& !(bq == BUILDING_SIZE[type]))	// auch Bergwerke
		{
			status = AIJH::JOB_FAILED;
#ifdef DEBUG_AI
			std::cout << "Player " << (unsigned)aijh->GetPlayerID() << ", Job failed: BQ changed for " << BUILDING_NAMES[type] << " at " << target_x << "/" << target_y << ". Retrying..." << std::endl;
#endif
			aijh->nodes[target_x + target_y * aijh->GetGWB()->GetWidth()].bq = bq;
			aijh->aiJobs.push_back(new AIJH::BuildJob(aijh, type, around_x, around_y));
			return;
		}
		return;
	}

	if (bld->GetBuildingType() != type)
	{
#ifdef DEBUG_AI
		std::cout << "Player " << (unsigned)aijh->GetPlayerID() << ", Job failed: Wrong Builingsite found for " << BUILDING_NAMES[type] << " at " << target_x << "/" << target_y << "." << std::endl;
#endif
		status = AIJH::JOB_FAILED;
		return;
	}
	const noFlag *houseFlag = aijh->GetGWB()->GetSpecObj<noFlag>(aijh->GetGWB()->GetXA(target_x, target_y, 4), 
		aijh->GetGWB()->GetYA(target_x, target_y, 4));
	// Gucken noch nicht ans Wegnetz angeschlossen
	if (!aijh->IsConnectedToRoadSystem(houseFlag))
	{
		// Bau unm�glich?
		if (!aijh->ConnectFlagToRoadSytem(houseFlag, route))
		{
			status = AIJH::JOB_FAILED;
#ifdef DEBUG_AI
			std::cout << "Player " << (unsigned)aijh->GetPlayerID() << ", Job failed: Cannot connect " << BUILDING_NAMES[type] << " at " << target_x << "/" << target_y << ". Retrying..." << std::endl;
#endif
			aijh->nodes[target_x + target_y * aijh->GetGWB()->GetWidth()].reachable = false;
			aijh->GetGCS().push_back(new gc::DestroyBuilding(target_x, target_y));
			aijh->GetGCS().push_back(new gc::DestroyFlag(houseFlag->GetX(), houseFlag->GetY()));

			aijh->aiJobs.push_back(new AIJH::BuildJob(aijh, type, around_x, around_y));
			return;
		}
		else
		{
			// Warten bis Weg da ist...
			return;
		}
	}
	else
	{
		// Wir sind angeschlossen, BQ f�r den eben gebauten Weg aktualisieren
		//aijh->RecalcBQAround(target_x, target_y);
		aijh->RecalcGround(target_x, target_y, route);

		switch(type)
		{
		case BLD_WOODCUTTER:
			aijh->ChangeResourceMap(target_x, target_y, 7, aijh->resourceMaps[AIJH::WOOD], -15);
			break;
		case BLD_FORESTER:
			aijh->ChangeResourceMap(target_x, target_y, 7, aijh->resourceMaps[AIJH::WOOD], 15);
			break;
		case BLD_QUARRY:
			aijh->ChangeResourceMap(target_x, target_y, 7, aijh->resourceMaps[AIJH::STONES], -15);
			break;
		case BLD_BARRACKS:
		case BLD_GUARDHOUSE:
		case BLD_WATCHTOWER:
		case BLD_FORTRESS:
			aijh->ChangeResourceMap(target_x, target_y, 8, aijh->resourceMaps[AIJH::BORDERLAND], -8);
			aijh->milBuildingSites.push_back(AIPlayerJH::Coords(target_x, target_y));
		case BLD_GOLDMINE:
			aijh->ChangeResourceMap(target_x, target_y, 4, aijh->resourceMaps[AIJH::GOLD], -30);
			break;
		case BLD_COALMINE:
			aijh->ChangeResourceMap(target_x, target_y, 4, aijh->resourceMaps[AIJH::COAL], -30);
			break;
		case BLD_IRONMINE:
			aijh->ChangeResourceMap(target_x, target_y, 4, aijh->resourceMaps[AIJH::IRONORE], -30);
			break;
		case BLD_GRANITEMINE:
			aijh->ChangeResourceMap(target_x, target_y, 4, aijh->resourceMaps[AIJH::GRANITE], -30);
			break;

		case BLD_FISHERY:
			aijh->ChangeResourceMap(target_x, target_y, 10, aijh->resourceMaps[AIJH::FISH], -30);
			break;
		case BLD_STOREHOUSE:
			aijh->AddStoreHouse(target_x, target_y);
			break;

		case BLD_FARM:
			aijh->ChangeResourceMap(target_x, target_y, 5, aijh->resourceMaps[AIJH::PLANTSPACE], -30);
			aijh->SetFarmedNodes(target_y, target_y);
			break;

		case BLD_MILL:
			aijh->aiJobs.push_back(new AIJH::BuildJob(aijh, BLD_BAKERY, target_x, target_y));
			break;

		case BLD_PIGFARM:
			aijh->aiJobs.push_back(new AIJH::BuildJob(aijh, BLD_SLAUGHTERHOUSE, target_x, target_y));
			break;

		case BLD_BAKERY:
		case BLD_SLAUGHTERHOUSE:
		case BLD_BREWERY:
			aijh->aiJobs.push_back(new AIJH::BuildJob(aijh, BLD_WELL, target_x, target_y));
			break;

		default:
			break;
		}

		// Just 4 Fun Gelehrten rufen
		if (BUILDING_SIZE[type] == BQ_MINE)
		{
			aijh->GetGCS().push_back(new gc::CallGeologist(houseFlag->GetX(), houseFlag->GetY()));
		}

		status = AIJH::JOB_EXECUTING_ROAD2;
		return;
	}
}
	
void AIJH::BuildJob::TryToBuildSecondaryRoad()
{
	const noFlag *houseFlag = aijh->GetGWB()->GetSpecObj<noFlag>(aijh->GetGWB()->GetXA(target_x, target_y, 4), 
		aijh->GetGWB()->GetYA(target_x, target_y, 4));

	if (!houseFlag)
	{
		// Baustelle wurde wohl zerst�rt, oh schreck!
		status = AIJH::JOB_FAILED;
#ifdef DEBUG_AI
			std::cout << "Player " << (unsigned)aijh->GetPlayerID() << ", Job failed: House flag is gone, " << BUILDING_NAMES[type] << " at " << target_x << "/" << target_y << ". Retrying..." << std::endl;
#endif
		aijh->aiJobs.push_back(new AIJH::BuildJob(aijh, type, around_x, around_y));
		return;
	}

	if (aijh->BuildAlternativeRoad(houseFlag, route))
		status = AIJH::JOB_EXECUTING_ROAD2_2;
	else
		status = AIJH::JOB_FINISHED;
}

void AIJH::ExpandJob::ExecuteJob()
{

}


void AIJH::EventJob::ExecuteJob()
{
	switch(ev->GetType())
	{
	case AIEvent::BuildingConquered:
		{
			AIEvent::Building *evb = dynamic_cast<AIEvent::Building *>(ev);
			aijh->HandleNewMilitaryBuilingOccupied(AIPlayerJH::Coords(evb->GetX(), evb->GetY()));
			status = AIJH::JOB_FINISHED;
		}
		break;
	case AIEvent::NoMoreResourcesReachable:
		{
			AIEvent::Building *evb = dynamic_cast<AIEvent::Building *>(ev);
			aijh->HandleNoMoreResourcesReachable(AIPlayerJH::Coords(evb->GetX(), evb->GetY()), evb->GetBuildingType());
			status = AIJH::JOB_FINISHED;
		}
		break;
	case AIEvent::BorderChanged:
		{
			AIEvent::Building *evb = dynamic_cast<AIEvent::Building *>(ev);
			aijh->HandleBorderChanged(AIPlayerJH::Coords(evb->GetX(), evb->GetY()));
			status = AIJH::JOB_FINISHED;
		}
		break;
	case AIEvent::BuildingFinished:
		{
			AIEvent::Building *evb = dynamic_cast<AIEvent::Building *>(ev);
			aijh->HandleBuildingFinished(AIPlayerJH::Coords(evb->GetX(), evb->GetY()), evb->GetBuildingType());
			status = AIJH::JOB_FINISHED;
		}
		break;
	case AIEvent::ExpeditionWaiting:
		{
			AIEvent::Location *lvb = dynamic_cast<AIEvent::Location *>(ev);
			aijh->HandleExpedition(AIPlayerJH::Coords(lvb->GetX(), lvb->GetY()));
			status = AIJH::JOB_FINISHED;
		}
	default:
		break;
	}

	//temp only:
	status = AIJH::JOB_FINISHED;
}
