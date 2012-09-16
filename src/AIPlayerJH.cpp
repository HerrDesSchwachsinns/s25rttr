// $Id: AIPlayerJH.cpp 8277 2012-09-16 17:59:43Z marcus $
//
// Copyright (c) 2005 - 2011 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.


#include "main.h"
#include "AIPlayerJH.h"

#include "GameClientPlayer.h"
#include "GameWorld.h"
#include "GameCommands.h"
#include "GamePlayerList.h"

#include "nobMilitary.h"
#include "nobHQ.h"
#include "noBuildingSite.h"
#include "noShip.h"
#include "noFlag.h"
#include "noTree.h"
#include "noAnimal.h"

#include "MapGeometry.h"

#include <iostream>
#include <list>

#include "GameMessages.h"
#include "GameServer.h"
#include <set>
#include <algorithm>

// from Pathfinding.cpp
bool IsPointOK_RoadPath(const GameWorldBase& gwb, const MapCoord x, const MapCoord y, const unsigned char dir, const void *param);

AIPlayerJH::AIPlayerJH(const unsigned char playerid, const GameWorldBase * const gwb, const GameClientPlayer * const player,
		const GameClientPlayerList * const players, const GlobalGameSettings * const ggs,
		const AI::Level level) : AIBase(playerid, gwb, player, players, ggs, level), defeated(false), 
		construction(AIConstruction(aii, this))
{
	currentJob = 0;
	InitNodes();
	InitResourceMaps();
	SaveResourceMapsToFile();
}

/// Wird jeden GF aufgerufen und die KI kann hier entsprechende Handlungen vollziehen
void AIPlayerJH::RunGF(const unsigned gf)
{
	if (defeated)
		return;

	if (TestDefeat())
		return;	
	if (gf == 40)
	{
		InitStoreAndMilitarylists();
	}
	if (gf == 100)
	{
		if(aii->GetMilitaryBuildings().size()<1&&aii->GetStorehouses().size()<2)
		{
			Chat(_("Hi, I'm an artifical player and I'm not very good yet!"));
			Chat(_("And I may crash your game sometimes..."));
		}

		// Set military settings to some nicer default values
		std::vector<unsigned char> milSettings;
		milSettings.resize(8);
		milSettings[0] = 10;	//recruits 0-10
		milSettings[1] = 5;		//defense 0-5
		milSettings[2] = 4;		//defenders 0-5
		milSettings[3] = 5;		//attackers 0-5
		milSettings[4] = 0;		//inland buildings (0 bar) 0-8
		milSettings[5] = 8;		//center buildings (1 bar) 0-8
		milSettings[6] = 8;		//harbor buildings (? bar) 0-8
		milSettings[7] = 8;		//border buildings (2 bar) 0-8
		aii->SetMilitarySettings(milSettings);
		//set good distribution settings 
		std::vector<unsigned char> goodSettings;
		goodSettings.resize(23);
		goodSettings[0] = 10; //food granite
		goodSettings[1] = 10; //food coal
		goodSettings[2] = 10; //food iron
		goodSettings[3] = 10; //food gold

		goodSettings[4] = 10; //grain mill
		goodSettings[5] = 10; //grain pigfarm
		goodSettings[6] = 10; //grain donkeybreeder
		goodSettings[7] = 10; //grain brewery
		goodSettings[8] = 10; //grain charburner

		goodSettings[9] = 10; //iron armory
		goodSettings[10] = 10; //iron metalworks

		goodSettings[11] = 10; //coal armory
		goodSettings[12] = 10; //coal ironsmelter
		goodSettings[13] = 10; //coal mint

		goodSettings[14] = 10; //wood sawmill
		goodSettings[15] = 10; //wood charburner

		goodSettings[16] = 10; //boards new buildings
		goodSettings[17] = 4; //boards metalworks
		goodSettings[18] = 2; //boards shipyard

		goodSettings[19] = 10; //water bakery
		goodSettings[20] = 10; //water brewery
		goodSettings[21] = 10; //water pigfarm
		goodSettings[22] = 10; //water donkeybreeder
		aii->SetDistribution(goodSettings);

	}

	if ((gf + (playerid * 11)) % 3 == 0) //try to complete a job on the list
	{
		construction.RefreshBuildingCount();
		ExecuteAIJob();
	}

	if ((gf + playerid * 17) % 100 == 0)
	{
		//CheckExistingMilitaryBuildings();
		TryToAttack();
	}

	//if ((gf + playerid * 13) % 100 == 0)
	//{
	//	CheckNewMilitaryBuildings();
	//}
	if((gf+playerid*11)%150==0) 
	{	//update tool creation settings
		std::vector<unsigned char> toolsettings;
		toolsettings.resize(12);
		toolsettings[2] = (aii->GetInventory()->goods[GD_SAW] + aii->GetInventory()->people[JOB_CARPENTER]<2)?4:aii->GetInventory()->goods[GD_SAW]<1?1:0;																					//saw
		toolsettings[3] = (aii->GetInventory()->goods[GD_PICKAXE]<1)?1:0;																															//pickaxe
		toolsettings[4] = (aii->GetInventory()->goods[GD_HAMMER]<1)?1:0;																															//hammer
		toolsettings[6] = (aii->GetInventory()->goods[GD_CRUCIBLE]+aii->GetInventory()->people[JOB_IRONFOUNDER]<construction.GetBuildingCount(BLD_IRONSMELTER)+1)?1:0;;								//crucible
		toolsettings[8]=(toolsettings[4]<1&&toolsettings[3]<1&&toolsettings[6]<1&&toolsettings[2]<1&&(aii->GetInventory()->goods[GD_SCYTHE] <1))?1:0;												//scythe
		toolsettings[9] = (aii->GetInventory()->goods[GD_CLEAVER]+aii->GetInventory()->people[JOB_BUTCHER]<construction.GetBuildingCount(BLD_SLAUGHTERHOUSE)+1)?1:0;								//cleaver
		toolsettings[10] = (aii->GetInventory()->goods[GD_ROLLINGPIN]+aii->GetInventory()->people[JOB_BAKER]<construction.GetBuildingCount(BLD_BAKERY)+1)?1:0;										//rollingpin
		toolsettings[5] =(toolsettings[4]<1&&toolsettings[3]<1&&toolsettings[6]<1&&toolsettings[2]<1&&(aii->GetInventory()->goods[GD_SHOVEL]<1))?1:0 ;												//shovel
		toolsettings[1] =(toolsettings[4]<1&&toolsettings[3]<1&&toolsettings[6]<1&&toolsettings[2]<1&&(aii->GetInventory()->goods[GD_AXE]+aii->GetInventory()->people[JOB_WOODCUTTER]<12)&&aii->GetInventory()->goods[GD_AXE]<1)?1:0;		//axe
		toolsettings[0] =0;//(toolsettings[4]<1&&toolsettings[3]<1&&toolsettings[6]<1&&toolsettings[2]<1&&(aii->GetInventory()->goods[GD_TONGS]<1))?1:0;												//Tongs(metalworks)
		toolsettings[7] = 0;																																										//rod & line 
		toolsettings[11] = 0;																																										//bow
		aii->SetToolSettings(toolsettings);	
		// Set military settings to some currently required values
		std::vector<unsigned char> milSettings;
		milSettings.resize(8);
		milSettings[0] = 10;
		milSettings[1] = 5;
		milSettings[2] = 4;
		milSettings[3] = 5;
		milSettings[4] = 0;													//interior 1 soldier each
		milSettings[5] = (unsigned char)max<int>(min<int>((SoldierAvailable()/10),8),4);	//inland min 50% max 100% depending on how many soldiers are available
		milSettings[6] = NoEnemyHarbor()?((SoldierAvailable()>10)?4:0):8;		//harbor points: enemy harbors exist? 100% if not 50% or 0% depending on our available recruits
		milSettings[7] = 8;														//front: 100%
		if(player->military_settings[5]!=milSettings[5]||player->military_settings[6]!=milSettings[6]) //only send the command if we want to change something
			aii->SetMilitarySettings(milSettings);
		//check for useless sawmills
		if(aii->GetBuildings(BLD_SAWMILL).size()>2)
		{
			int burns=0;
			for(std::list<nobUsual*>::const_iterator it=aii->GetBuildings(BLD_SAWMILL).begin();it!=aii->GetBuildings(BLD_SAWMILL).end();it++)
			{
				if(*(*it)->GetProduktivityPointer()<1&&(*it)->HasWorker()&&(*it)->GetWares(0)<1&&(aii->GetBuildings(BLD_SAWMILL).size()-burns)>2&&!(*it)->AreThereAnyOrderedWares())
				{
					aii->DestroyBuilding((*it));					
					RemoveUnusedRoad(aii->GetSpecObj<noFlag>(aii->GetXA((*it)->GetX(),(*it)->GetY(),4),aii->GetYA((*it)->GetX(),(*it)->GetY(),4)), 1,true);
					burns++;
				}
			}
		}
	}
	if((gf+playerid*7)%200==0) // plan new buildings
	{
		construction.RefreshBuildingCount();		
		//pick a random storehouse and try to build one of these buildings around it (checks if we actually want more of the building type)
		BuildingType bldToTest[] = {
		BLD_HARBORBUILDING,
		BLD_SHIPYARD,		
		BLD_SAWMILL,
		BLD_FORESTER,
		BLD_IRONSMELTER,
		BLD_MINT,
		BLD_ARMORY,
		BLD_METALWORKS,
		BLD_BREWERY,
		BLD_MILL,
		BLD_PIGFARM,
		BLD_SLAUGHTERHOUSE,
		BLD_BAKERY,
		BLD_DONKEYBREEDER,
		BLD_FARM,
		BLD_FISHERY,
		BLD_WOODCUTTER,
		BLD_QUARRY,
		BLD_GOLDMINE,
		BLD_IRONMINE,
		BLD_COALMINE,
		BLD_GRANITEMINE,
		BLD_HUNTER
		};
		unsigned numBldToTest = 23;
		unsigned char randomstore;
		randomstore=rand()%(aii->GetStorehouses().size());
		if(aii->GetStorehouses().size()<1)
			return;
		//collect swords,shields and beer in first storehouse!
		nobBaseWarehouse* wh=(*aii->GetStorehouses().begin());
		if(!wh->CheckRealInventorySettings(0,8,0))
		{
			aii->ChangeInventorySetting(wh->GetX(),wh->GetY(),0,8,0);
			aii->ChangeInventorySetting(wh->GetX(),wh->GetY(),0,8,16);
			aii->ChangeInventorySetting(wh->GetX(),wh->GetY(),0,8,21);
		}
		//go to the picked random warehouse and try to build around it
		std::list<nobBaseWarehouse*>::const_iterator it=aii->GetStorehouses().begin();
		std::advance(it,randomstore);
		UpdateNodesAroundNoBorder((*it)->GetX(),(*it)->GetY(),15); //update the area we want to build in first 
		for (unsigned int i = 0; i < numBldToTest; i++)
		{
			//only 1 shipyard for each harbor
			if(bldToTest[i]==BLD_SHIPYARD)
			{
				if(IsInvalidShipyardPosition((*it)->GetX(),(*it)->GetY()))
					continue;
			}
			if (construction.Wanted(bldToTest[i]))
			{
				AddBuildJob(bldToTest[i],(*it)->GetX(),(*it)->GetY());
			}
		}
		if(gf>1500||aii->GetInventory()->goods[GD_BOARDS]>11)
			AddBuildJob(construction.ChooseMilitaryBuilding((*it)->GetX(), (*it)->GetY()),(*it)->GetX(), (*it)->GetY());


		//now pick a random military building and try to build around that as well
		if(aii->GetMilitaryBuildings().size()<1)return;
		randomstore=rand()%(aii->GetMilitaryBuildings().size());
		std::list<nobMilitary*>::const_iterator it2 = aii->GetMilitaryBuildings().begin();
		std::advance(it2,randomstore);
		MapCoord tx=(*it2)->GetX(),ty=(*it2)->GetY();
		UpdateReachableNodes(tx, ty, 15);
		for (unsigned int i = 0; i < numBldToTest; i++) 
		{
			if(bldToTest[i]==BLD_SHIPYARD)
			{
				//only 1 shipyard for each harbor
				if(IsInvalidShipyardPosition(tx,ty))
					continue;
			}
			if (construction.Wanted(bldToTest[i]))
			{
				AddBuildJob(bldToTest[i],tx,ty);
			}
		}
		AddBuildJob(construction.ChooseMilitaryBuilding(tx, ty),tx, ty);
		if((*it2)->IsUseless()&&(*it2)->IsDemolitionAllowed())
		{
			aii->DestroyBuilding(tx, ty);
		}		
	}
}

bool AIPlayerJH::TestDefeat()
{
	if (!aii->GetStorehouses().size())
	{
		defeated = true;
		aii->Surrender();
		Chat(_("You win"));
		return true;
	}
	return false;
}

void AIPlayerJH::AddBuildJob(BuildingType type, MapCoord x, MapCoord y, bool front)
{
	construction.AddBuildJob(new AIJH::BuildJob(this, type, x, y), front);
}

void AIPlayerJH::AddJob(AIJH::Job *job, bool front)
{
	construction.AddJob(job, front);
}


void AIPlayerJH::AddBuildJob(BuildingType type)
{
	construction.AddBuildJob(new AIJH::BuildJob(this, type), false);
}

AIJH::Resource AIPlayerJH::CalcResource(MapCoord x, MapCoord y)
{
	AIJH::Resource res = aii->GetSubsurfaceResource(x, y);

	// resources on surface
	if (res == AIJH::NOTHING)
	{
		res = aii->GetSurfaceResource(x, y);

		if(res == AIJH::NOTHING)
		{
			// check terrain
			unsigned char t;
			bool good = !aii->IsRoadPoint(x, y);
			
			for(unsigned char i = 0;i<6;++i)
			{
				t = aii->GetTerrainAround(x, y, i);

				// check against valid terrains for planting
				if(t != 3 && (t < 8 || t > 12))
				{
					good = false;
				}
			}
			if (good)
			{
				res = AIJH::PLANTSPACE;
			}
		}
		else
		{
			if (res==AIJH::WOOD)
			{
				if((gwb->GetSpecObj<noTree>(x,y))->type==5) //exclude pineapple (because they are more of a "blocker" than a tree and only count as tree for animation&sound)
					res=AIJH::NOTHING;
			}
		}
	}
	else
	{
		if ((aii->GetSurfaceResource(x,y) == AIJH::STONES) || (aii->GetSurfaceResource(x,y) == AIJH::WOOD))
		{
			if (aii->GetSubsurfaceResource(x,y) == AIJH::WOOD)
			{
				if ((gwb->GetSpecObj<noTree>(x,y))->type != 5)
					res = AIJH::MULTIPLE;
			} else
			{
				res = AIJH::MULTIPLE;
			}
		}
	}
	if (res == AIJH::BLOCKED)
	{
		res = AIJH::NOTHING; // nicht so ganz logisch... aber Blocked als res is doof TODO
	}
	return res;
}

void AIPlayerJH::InitReachableNodes()
{
	unsigned short width = aii->GetMapWidth();
	unsigned short height = aii->GetMapHeight();

	std::queue<std::pair<MapCoord, MapCoord> > toCheck;

	// Alle auf not reachable setzen
	for (unsigned short y = 0; y < height; ++y)
	{
		for (unsigned short x = 0; x < width; ++x)
		{
			unsigned i = x + y * width;
			nodes[i].reachable = false;
			const noFlag *myFlag = 0;
			if ( (myFlag = aii->GetSpecObj<noFlag>(x, y)) )
			{
				if (myFlag->GetPlayer() == playerid)
				{
					nodes[i].reachable = true;
					toCheck.push(std::make_pair(x,y));
				}
			}
		}
	}

	IterativeReachableNodeChecker(toCheck);
}

void AIPlayerJH::IterativeReachableNodeChecker(std::queue<std::pair<MapCoord, MapCoord> >& toCheck)
{
	unsigned short width = aii->GetMapWidth();

	// TODO auch mal bootswege bauen können
	//Param_RoadPath prp = { false };

	while(toCheck.size() > 0)
	{
		// Reachable coordinate
		MapCoord rx = toCheck.front().first;
		MapCoord ry = toCheck.front().second;

		// Coordinates to test around this reachable coordinate
		for (unsigned dir = 0; dir < 6; ++dir)
		{
			MapCoord nx = aii->GetXA(rx, ry, dir);
			MapCoord ny = aii->GetYA(rx, ry, dir);
			unsigned ni = nx + ny * width;

			// already reached, don't test again
			if (nodes[ni].reachable)
				continue;

			bool boat = false;
			// Test whether point is reachable; yes->add to check list
			if (IsPointOK_RoadPath(*gwb, nx, ny, (dir+3)%6, (void *) &boat))
			{
				nodes[ni].reachable = true;
				toCheck.push(std::make_pair(nx, ny));
			}
		}
		toCheck.pop();
	}
}


void AIPlayerJH::UpdateReachableNodes(MapCoord x, MapCoord y, unsigned radius)
{
	unsigned short width = aii->GetMapWidth();

	std::queue<std::pair<MapCoord, MapCoord> > toCheck;

	for(MapCoord tx = aii->GetXA(x, y, 0), r = 1; r <= radius; tx = aii->GetXA(tx, y, 0), ++r)
	{
		MapCoord tx2 = tx, ty2 = y;
		for(unsigned i = 2; i < 8; ++i)
		{
			for(MapCoord r2 = 0; r2 < r; aii->GetPointA(tx2, ty2, i%6), ++r2)
			{
				unsigned i = tx2 + ty2 * width;
				nodes[i].reachable = false;
				const noFlag *myFlag = 0;
				if (( myFlag = aii->GetSpecObj<noFlag>(tx2, ty2)))
				{
					if (myFlag->GetPlayer() == playerid)
					{
						nodes[i].reachable = true;
						toCheck.push(std::make_pair(tx2, ty2));
					}
				}
			}
		}
	}
	IterativeReachableNodeChecker(toCheck);
}

void AIPlayerJH::InitNodes()
{
	unsigned short width = aii->GetMapWidth();
	unsigned short height = aii->GetMapHeight();

	nodes.resize(width * height);

	InitReachableNodes();

	for (unsigned short y = 0; y < height; ++y)
	{
		for (unsigned short x = 0; x < width; ++x)
		{
			unsigned i = x + y * width;

			// if reachable, we'll calc bq
			if (nodes[i].reachable)
			{
				nodes[i].owned = true;
				nodes[i].bq = aii->GetBuildingQuality(x, y);
			}
			else
			{
				nodes[i].owned = false;
				nodes[i].bq = BQ_NOTHING;
			}

			nodes[i].res = CalcResource(x, y);
			nodes[i].border = aii->IsBorder(x, y);
			nodes[i].farmed = false;
		}
	}
}

void AIPlayerJH::UpdateNodes()
{
	
}

void AIPlayerJH::InitResourceMaps()
{
	unsigned short width = aii->GetMapWidth();
	unsigned short height = aii->GetMapHeight();

	resourceMaps.resize(AIJH::RES_TYPE_COUNT);
	for (unsigned res = 0; res < AIJH::RES_TYPE_COUNT; ++res)
	{
		resourceMaps[res].resize(width * height);
		for (unsigned short y=0; y<height; ++y)
		{
			for (unsigned short x=0; x<width; ++x)
			{
				unsigned i = y * width + x;
				//resourceMaps[res][i] = 0;
				if (nodes[i].res == (AIJH::Resource)res && (AIJH::Resource)res != AIJH::BORDERLAND)
				{
					ChangeResourceMap(x, y, AIJH::RES_RADIUS[res], resourceMaps[res], 1);
				}

				// Grenzgebiet"ressource"
				else if (nodes[i].border && (AIJH::Resource)res == AIJH::BORDERLAND)
				{
					ChangeResourceMap(x, y, AIJH::RES_RADIUS[AIJH::BORDERLAND], resourceMaps[AIJH::BORDERLAND], 1);
				}
				if(nodes[i].res==AIJH::MULTIPLE)
				{
					if(aii->GetSubsurfaceResource(x,y)==(AIJH::Resource)res||aii->GetSurfaceResource(x,y)==(AIJH::Resource)res)
						ChangeResourceMap(x, y, AIJH::RES_RADIUS[res], resourceMaps[res], 1);
				}
			}
		}
	}
}

void AIPlayerJH::RecalcResource(AIJH::Resource restype)
{
	unsigned short width = aii->GetMapWidth();
	unsigned short height = aii->GetMapHeight();
	unsigned res=restype;
	std::vector<int> &resmap=resourceMaps[res];
	for (unsigned y=0; y<resmap.size(); ++y)
	{
		resmap[y]=0;
	}
	for (unsigned short y=0; y<height; ++y)
	{
		for (unsigned short x=0; x<width; ++x)
		{
			unsigned i = y * width + x;
			//resourceMaps[res][i] = 0;
			if (nodes[i].res == (AIJH::Resource)res && (AIJH::Resource)res != AIJH::BORDERLAND && gwb->GetNode(x,y).t1!=TT_WATER && gwb->GetNode(x,y).t1!=TT_LAVA && gwb->GetNode(x,y).t1!=TT_SWAMPLAND&& gwb->GetNode(x,y).t1!=TT_SNOW )
			{
				ChangeResourceMap(x, y, AIJH::RES_RADIUS[res], resourceMaps[res], 1);
			}
			// Grenzgebiet"ressource"
			else if (aii->IsBorder(x, y) && (AIJH::Resource)res == AIJH::BORDERLAND)
			{	//only count border area that is actually passable terrain
				if(gwb->GetNode(x,y).t1!=TT_WATER && gwb->GetNode(x,y).t1!=TT_LAVA && gwb->GetNode(x,y).t1!=TT_SWAMPLAND&& gwb->GetNode(x,y).t1!=TT_SNOW)
					ChangeResourceMap(x, y, AIJH::RES_RADIUS[AIJH::BORDERLAND], resourceMaps[AIJH::BORDERLAND], 1);
			}
			if (nodes[i].res == AIJH::MULTIPLE && gwb->GetNode(x,y).t1!=TT_WATER && gwb->GetNode(x,y).t1!=TT_LAVA && gwb->GetNode(x,y).t1!=TT_SWAMPLAND )
			{
				if(aii->GetSubsurfaceResource(x,y)==(AIJH::Resource)res||aii->GetSurfaceResource(x,y)==(AIJH::Resource)res)
						ChangeResourceMap(x, y, AIJH::RES_RADIUS[res], resourceMaps[res], 1);
			}
			if(res==AIJH::WOOD&&aii->IsBuildingOnNode(x,y,BLD_WOODCUTTER)) //existing woodcutters reduce wood rating
				ChangeResourceMap(x, y, 7, resourceMaps[res], -10);
			if(res==AIJH::PLANTSPACE&&aii->IsBuildingOnNode(x,y,BLD_FARM)) //existing farm reduce plantspace rating
				ChangeResourceMap(x, y, 3, resourceMaps[res], -25);
			if(res==AIJH::PLANTSPACE&&aii->IsBuildingOnNode(x,y,BLD_FORESTER)) //existing forester reduce plantspace rating
				ChangeResourceMap(x, y, 6, resourceMaps[res], -25);
		}
	}
}

void AIPlayerJH::SetFarmedNodes(MapCoord x, MapCoord y,bool set)
{
	// Radius in dem Bausplatz für Felder blockiert wird
	const unsigned radius = 3;

	unsigned short width = aii->GetMapWidth();
	
	nodes[x + y * width].farmed = set;

	for(MapCoord tx=aii->GetXA(x,y,0), r=1;r<=radius;tx=aii->GetXA(tx,y,0),++r)
	{
		MapCoord tx2 = tx, ty2 = y;
		for(unsigned i = 2;i<8;++i)
		{
			for(MapCoord r2=0;r2<r;aii->GetPointA(tx2,ty2,i%6),++r2)
			{
				unsigned i = tx2 + ty2 * width;
				nodes[i].farmed = set;
			}
		}
	}
}

void AIPlayerJH::ChangeResourceMap(MapCoord x, MapCoord y, unsigned radius, std::vector<int> &resMap, int value)
{
	unsigned short width = aii->GetMapWidth();

	resMap[x + y * width] += value * radius;

	for(MapCoord tx=aii->GetXA(x,y,0), r=1;r<=radius;tx=aii->GetXA(tx,y,0),++r)
	{
		MapCoord tx2 = tx, ty2 = y;
		for(unsigned i = 2;i<8;++i)
		{
			for(MapCoord r2=0;r2<r;aii->GetPointA(tx2,ty2,i%6),++r2)
			{
				unsigned i = tx2 + ty2 * width;
				resMap[i] += value * (radius-r);
			}
		}
	}


}

bool AIPlayerJH::FindGoodPosition(MapCoord &x, MapCoord &y, AIJH::Resource res, int threshold, BuildingQuality size, int radius, bool inTerritory)
{
	unsigned short width = aii->GetMapWidth();
	unsigned short height = aii->GetMapHeight();

	if (x >= width || y >= height)
	{
		x = aii->GetHeadquarter()->GetX();
		y = aii->GetHeadquarter()->GetY();
	}

	// TODO was besseres wär schön ;)
	if (radius == -1)
		radius = 30;

	for(MapCoord tx=aii->GetXA(x,y,0), r=1;r<=radius;tx=aii->GetXA(tx,y,0),++r)
	{
		MapCoord tx2 = tx, ty2 = y;
		for(unsigned i = 2;i<8;++i)
		{
			for(MapCoord r2=0;r2<r;aii->GetPointA(tx2,ty2,i%6),++r2)
			{
				unsigned i = tx2 + ty2 * width;
				if (resourceMaps[res][i] >= threshold)
				{
					if ((inTerritory && !aii->IsOwnTerritory(tx2,ty2)) || nodes[i].farmed)
						continue;
					if ( (aii->GetBuildingQuality(tx2,ty2) >= size && aii->GetBuildingQuality(tx2,ty2) < BQ_MINE) // normales Gebäude
						|| (aii->GetBuildingQuality(tx2,ty2) == size))	// auch Bergwerke
					{
						x = tx2;
						y = ty2;
						return true;
					}
				}
			}
		}
	}
	return false;
}

PositionSearch *AIPlayerJH::CreatePositionSearch(MapCoord &x, MapCoord &y, AIJH::Resource res, BuildingQuality size, int minimum, BuildingType bld, bool best)
{
	// set some basic parameters
	PositionSearch *p = new PositionSearch(x, y, res, minimum, size, BLD_WOODCUTTER, best);
	p->nodesPerStep = 25; // TODO make it dependent on something...
	p->resultValue = 0;

	// allocate memory for the nodes
	unsigned numNodes = aii->GetMapWidth() * aii->GetMapHeight();
	p->tested = new std::vector<bool>(numNodes, false);
	p->toTest = new std::queue<unsigned>;


	// if no useful startpos is given, use headquarter
	if (x >= aii->GetMapWidth() || y >= aii->GetMapHeight())
	{
		x = aii->GetHeadquarter()->GetX();
		y = aii->GetHeadquarter()->GetY();
	}

	// insert start position as first node to test
	p->toTest->push(x + y * aii->GetMapWidth());
	(*p->tested)[x + y * aii->GetMapWidth()] = true;

	return p;
}

PositionSearchState AIPlayerJH::FindGoodPosition(PositionSearch *search, bool best)
{
	// make nodesPerStep tests
	for (int i = 0; i < search->nodesPerStep; i++)
	{
		// no more nodes to test? end this!
		if (search->toTest->empty())
			break;

		// get the node
		unsigned nodeIndex = search->toTest->front();
		search->toTest->pop();
		AIJH::Node *node = &nodes[nodeIndex];
		unsigned short width = aii->GetMapWidth();
		MapCoord x = nodeIndex % width;
		MapCoord y = nodeIndex / width;
		
		// and test it... TODO exception at res::borderland?
		if (resourceMaps[search->res][nodeIndex] > search->resultValue // value better
			&& node->owned && node->reachable && !node->farmed // available node
			&& ((node->bq >= search->size && node->bq < BQ_MINE) || (node->bq == search->size)) // matching size
			)
		{
			// store location & value
			search->resultValue = resourceMaps[search->res][nodeIndex];
			search->resultX = x;
			search->resultY = y;
		}

		// now insert neighbouring nodes...
		for (unsigned char dir = 0; dir < 6; ++dir)
		{
			MapCoord nx = aii->GetXA(x, y, dir);
			MapCoord ny = aii->GetYA(x, y, dir);
			unsigned ni = nx + ny * width;

			// test if already tested or not in territory
			if (!(*search->tested)[ni] && nodes[ni].owned)
			{
				search->toTest->push(ni);
				(*search->tested)[ni] = true;
			}
		}
	}

	// decide the state of the search

	// no more nodes to test, not reached minimum
	if (search->toTest->empty() && search->resultValue < search->minimum)
	{
		return SEARCH_FAILED;
	}

	// reached minimal satifiying value or best value, if needed
	else if ( (search->resultValue >= search->minimum && !best) 
		|| (search->resultValue >= search->minimum && search->toTest->empty()))
	{
		return SEARCH_SUCCESSFUL;
	}

	// more to search...
	else
	{
		return SEARCH_IN_PROGRESS;
	}
}


bool AIPlayerJH::FindBestPositionDiminishingResource(MapCoord &x, MapCoord &y, AIJH::Resource res, BuildingQuality size, int minimum, int radius, bool inTerritory)
{
	bool fixed=ggs->isEnabled(ADDON_INEXHAUSTIBLE_MINES)&&(res==AIJH::IRONORE||res==AIJH::COAL||res==AIJH::GOLD||res==AIJH::GRANITE);
	unsigned short width = aii->GetMapWidth();
	unsigned short height = aii->GetMapHeight();
	int temp=0;
	bool lastcirclevaluecalculated=false;
	bool lastvaluecalculated=false;
	//to avoid having to calculate a value twice and still move left on the same level without any problems we use this variable to remember the first calculation we did in the circle.
	int circlestartvalue=0;
	//outside of map bounds? -> search around our main storehouse!
	if (x >= width || y >= height)
	{
		x = aii->GetStorehouses().front()->GetX();
		y = aii->GetStorehouses().front()->GetY();
	}

	// TODO was besseres wär schön ;)
	if (radius == -1)
		radius = 11;

	int best_x = 0, best_y = 0, best_value;
	best_value = -1;

	for(MapCoord tx=aii->GetXA(x,y,0), r=1;r<=radius;tx=aii->GetXA(tx,y,0),++r)
	{
		MapCoord tx2 = tx, ty2 = y;
		for(unsigned i = 2;i<8;++i)
		{
			for(MapCoord r2=0;r2<r;++r2)
			{
				unsigned n = tx2 + ty2 * width;
				if(fixed)
					temp=resourceMaps[res][n];
				else
				{
					//only do a complete calculation for the first point or when moving outward and the last value is unknown
					if((r<2||!lastcirclevaluecalculated)&&r2<1&&i<3&&resourceMaps[res][n])
					{
						temp=aii->CalcResourceValue(tx2,ty2,res);	
						circlestartvalue=temp;
						lastcirclevaluecalculated=true;
						lastvaluecalculated=true;
					}					
					else
					{
						//was there ever anything? if not skip it!
						if(!resourceMaps[res][n])
						{
							if(r2<1&&i<3)
								lastcirclevaluecalculated=false;
							lastvaluecalculated=false;
							temp=resourceMaps[res][n];
						}
						else
						{
							//temp=aii->CalcResourceValue(tx2,ty2,res);
							//circle not yet started? -> last direction was outward (left=0)
							if(r2<1&&i<3)
							{
								temp=aii->CalcResourceValue(tx2,ty2,res,0,circlestartvalue);
								circlestartvalue=temp;
							}
							else 
							{
								if(lastvaluecalculated)
								{
									if(r2>0)//we moved direction i%6
										temp=aii->CalcResourceValue(tx2,ty2,res,i%6,temp);
									else //last step was the previous direction
										temp=aii->CalcResourceValue(tx2,ty2,res,(i-1)%6,temp);
								}
								else
								{
									temp=aii->CalcResourceValue(tx2,ty2,res);
									lastvaluecalculated=true;
								}								
							}
						}						
					}
					//copy the value to the resource map
					resourceMaps[res][n]=temp;
				}				
				if (temp > best_value)					
				{				
					if (!nodes[n].reachable || (inTerritory && !aii->IsOwnTerritory(tx2,ty2)) || nodes[n].farmed)
					{
						aii->GetPointA(tx2,ty2,i%6);
						continue;
					}
					//special case fish -> check for other fishery buildings
					if(res==AIJH::FISH)
					{
						if(BuildingNearby(tx2,ty2,BLD_FISHERY,6))
						{
							aii->GetPointA(tx2,ty2,i%6);
							continue;
						}
					}
					//dont build next to harborspots
					if(HarborPosClose(tx2,ty2,3,true))
					{
						aii->GetPointA(tx2,ty2,i%6);
						continue;
					}					
					BuildingQuality bq=aii->GetBuildingQuality(tx2,ty2);
					if ( (bq >= size && bq < BQ_MINE) // normales Gebäude
						|| (bq == size))	// auch Bergwerke					
					{
						best_x = tx2;
						best_y = ty2;
						best_value = temp;
						//TODO: calculate "perfect" rating and instantly return if we got that already
					}					
				}
				aii->GetPointA(tx2,ty2,i%6);
			}
		}
	}

	if (best_value >= minimum)
	{
		x = best_x;
		y = best_y;
		return true;
	}
	return false;
}

bool AIPlayerJH::FindBestPosition(MapCoord &x, MapCoord &y, AIJH::Resource res, BuildingQuality size, int minimum, int radius, bool inTerritory)
{
	if(res==AIJH::IRONORE||res==AIJH::COAL||res==AIJH::GOLD||res==AIJH::GRANITE||res==AIJH::STONES||res==AIJH::FISH)
		return FindBestPositionDiminishingResource(x,y,res,size,minimum,radius,inTerritory);
	unsigned short width = aii->GetMapWidth();
	unsigned short height = aii->GetMapHeight();
	int temp=0;
	//to avoid having to calculate a value twice and still move left on the same level without any problems we use this variable to remember the first calculation we did in the circle.
	int circlestartvalue=0;
	//outside of map bounds? -> search around our main storehouse!
	if (x >= width || y >= height)
	{
		x = aii->GetStorehouses().front()->GetX();
		y = aii->GetStorehouses().front()->GetY();
	}

	// TODO was besseres wär schön ;)
	if (radius == -1)
		radius = 11;

	int best_x = 0, best_y = 0, best_value;
	best_value = -1;

	for(MapCoord tx=aii->GetXA(x,y,0), r=1;r<=radius;tx=aii->GetXA(tx,y,0),++r)
	{
		MapCoord tx2 = tx, ty2 = y;
		for(unsigned i = 2;i<8;++i)
		{
			for(MapCoord r2=0;r2<r;++r2)
			{
				unsigned n = tx2 + ty2 * width;
				//only do a complete calculation for the first point!
				if(r<2&&r2<1&&i<3)
				{
					temp=aii->CalcResourceValue(tx2,ty2,res);	
					circlestartvalue=temp;
				}					
				else
				{
					//temp=aii->CalcResourceValue(tx2,ty2,res);
					//circle not yet started? -> last direction was outward (left=0)
					if(r2<1&&i<3)
					{
						temp=aii->CalcResourceValue(tx2,ty2,res,0,circlestartvalue);
						circlestartvalue=temp;
					}
					else 
					{
						if(r2>0)//we moved direction i%6
							temp=aii->CalcResourceValue(tx2,ty2,res,i%6,temp);
						else //last step was the previous direction
							temp=aii->CalcResourceValue(tx2,ty2,res,(i-1)%6,temp);
					}
				}
				//copy the value to the resource map (map is only used in the ai debug mode)
				resourceMaps[res][n]=temp;
				if (temp > best_value)					
				{				
					if (!nodes[n].reachable || (inTerritory && !aii->IsOwnTerritory(tx2,ty2)) || nodes[n].farmed)
					{
						aii->GetPointA(tx2,ty2,i%6);
						continue;
					}
					if(HarborPosClose(tx2,ty2,3,true))
					{
						aii->GetPointA(tx2,ty2,i%6);
						continue;
					}
					BuildingQuality bq=aii->GetBuildingQuality(tx2,ty2);
					if ( (bq >= size && bq < BQ_MINE) // normales Gebäude
						|| (bq == size))	// auch Bergwerke					
					{
						best_x = tx2;
						best_y = ty2;
						best_value = temp;
						//TODO: calculate "perfect" rating and instantly return if we got that already
					}
				}
				aii->GetPointA(tx2,ty2,i%6);
			}
		}
	}

	if (best_value >= minimum)
	{
		x = best_x;
		y = best_y;
		return true;
	}
	return false;
}

void AIPlayerJH::UpdateNodesAround(MapCoord x, MapCoord y, unsigned radius)
{
	UpdateReachableNodes(x, y, radius);
}

void AIPlayerJH::UpdateNodesAroundNoBorder(MapCoord x, MapCoord y, unsigned radius)
{
	UpdateReachableNodes(x, y, radius);
}

void AIPlayerJH::ExecuteAIJob()
{
	// Check whether current job is finished...
	if (currentJob)
	{
		if (currentJob->GetStatus() == AIJH::JOB_FINISHED)
		{
			delete currentJob;
			currentJob = 0;
		}
	}

	// ... or it failed
	if (currentJob)
	{
		if (currentJob->GetStatus() == AIJH::JOB_FAILED)
		{
			// TODO fehlerbehandlung?
			//std::cout << "Job failed." << std::endl;
			delete currentJob;
			currentJob = 0;
		}
	}

	// if no current job available, take next one! events first, then constructions
	if (!currentJob)
	{
		if (eventManager.EventAvailable())
		{
			currentJob = new AIJH::EventJob(this, eventManager.GetEvent());
		}
		else if (construction.BuildJobAvailable())
		{
			currentJob = construction.GetBuildJob();
		}
	}

	// Something to do? Do it!
	if (currentJob)
		currentJob->ExecuteJob();
}

void AIPlayerJH::RecalcBQAround(const MapCoord x, const MapCoord y)
{
}

void AIPlayerJH::CheckNewMilitaryBuildings()
{
}

bool AIPlayerJH::SimpleFindPosition(MapCoord &x, MapCoord &y, BuildingQuality size, int radius)
{
	unsigned short width = aii->GetMapWidth();
	unsigned short height = aii->GetMapHeight();
	//if(size==BQ_HARBOR)
	//	Chat(_("looking for harbor"));

	if (x >= width || y >= height)
	{
		x = aii->GetStorehouses().front()->GetX();
		y = aii->GetStorehouses().front()->GetY();
	}

	// TODO was besseres wär schön ;)
	if (radius == -1)
		radius = 30;

	for(MapCoord tx=aii->GetXA(x,y,0), r=1;r<=radius;tx=aii->GetXA(tx,y,0),++r)
	{
		MapCoord tx2 = tx, ty2 = y;
		for(unsigned i = 2;i<8;++i)
		{
			for(MapCoord r2=0;r2<r;aii->GetPointA(tx2,ty2,i%6),++r2)
			{
				unsigned i = tx2 + ty2 * width;

				if (!nodes[i].reachable || !aii->IsOwnTerritory(tx2,ty2) || nodes[i].farmed)
					continue;
				if(HarborPosClose(tx2,ty2,3,true))
				{
					if(size!=BQ_HARBOR)
						continue;
				}
				BuildingQuality bq=aii->GetBuildingQuality(tx2,ty2);
				if ( (bq>= size && bq < BQ_MINE) // normales Gebäude
					|| (bq == size))	// auch Bergwerke
				{
					x = tx2;
					y = ty2;
					return true;
				}
			}
		}
	}

	return false;
}

double AIPlayerJH::GetDensity(MapCoord x, MapCoord y, AIJH::Resource res, int radius)
{
		unsigned short width = aii->GetMapWidth();
		unsigned short height = aii->GetMapHeight();
	

	// TODO: check warum das so ist, und ob das sinn macht! ist so weil der punkt dann außerhalb der karte liegen würde ... könnte trotzdem crashen wenn wir kein hq mehr haben ... mehr checks!
	if (x >= width || y >= height)
	{
		x = aii->GetStorehouses().front()->GetX();
		y = aii->GetStorehouses().front()->GetY();
	}



	unsigned good = 0;
	unsigned all = 0;

	for(MapCoord tx=aii->GetXA(x,y,0), r=1;r<=radius;tx=aii->GetXA(tx,y,0),++r)
	{
		MapCoord tx2 = tx, ty2 = y;
		for(unsigned i = 2;i<8;++i)
		{
			for(MapCoord r2=0;r2<r;aii->GetPointA(tx2,ty2,i%6),++r2)
			{
				unsigned i = tx2 + ty2 * width;

				if (nodes[i].res == res)
					good++;

				all++;
			}
		}
	}

	return (all != 0) ? good/(double)all : 0.0;
}

void AIPlayerJH::HandleNewMilitaryBuilingOccupied(const Coords& coords)
{	
	MapCoord x = coords.x;
	MapCoord y = coords.y;
	//kill bad flags we find
	RemoveAllUnusedRoads(x,y);
	construction.RefreshBuildingCount();
	const nobMilitary *mil = aii->GetSpecObj<nobMilitary>(x, y);
	if (mil)
	{
		if ((mil->GetBuildingType() == BLD_BARRACKS || mil->GetBuildingType() == BLD_GUARDHOUSE) && mil->GetFrontierDistance() == 0 && !mil->IsGoldDisabled())
		{
			aii->ToggleCoins(x, y);
		}

		// if near border and gold disabled (by addon): enable it
		if (mil->GetFrontierDistance() && mil->IsGoldDisabled())
		{
			aii->ToggleCoins(x, y);
		}
	}

	AddBuildJob(BLD_HARBORBUILDING, x, y);
	if(!IsInvalidShipyardPosition(x,y))
		AddBuildJob(BLD_SHIPYARD, x, y);
	if (SoldierAvailable())
	{
		AddBuildJob(construction.ChooseMilitaryBuilding(x, y), x, y);
		AddBuildJob(construction.ChooseMilitaryBuilding(x, y), x, y);
		AddBuildJob(construction.ChooseMilitaryBuilding(x, y), x, y);
	}

	// try to build one the following buildings around the new military building
	
	BuildingType bldToTest[] = {
		BLD_STOREHOUSE,
		BLD_WOODCUTTER,
		BLD_QUARRY,
		BLD_GOLDMINE,
		BLD_COALMINE,
		BLD_IRONMINE,
		BLD_GRANITEMINE,
		BLD_FISHERY,
		BLD_FARM,
		BLD_HUNTER	
	};
	unsigned numBldToTest = 0;
	//remove the storehouse from the building test list if we are close to another storehouse already
	for (std::list<nobBaseWarehouse*>::const_iterator it = aii->GetStorehouses().begin(); it != aii->GetStorehouses().end(); it++)
	{
		if (gwb->CalcDistance((*it)->GetX(), (*it)->GetY(), x, y) < 20)
		{
			numBldToTest = 1;
			break;
		}		
	}
	//same is true for warehouses which are still in production
	for(std::list<noBuildingSite*>::const_iterator it=aii->GetBuildingSites().begin();it!=aii->GetBuildingSites().end();it++)
	{
		if((*it)->GetBuildingType()==BLD_STOREHOUSE||(*it)->GetBuildingType()==BLD_HARBORBUILDING)
		{
			if (gwb->CalcDistance((*it)->GetX(), (*it)->GetY(), x, y) < 20)
			{
				numBldToTest = 1;
				break;
			}
		}
	}	

	for (unsigned int i = numBldToTest; i < 10; ++i)
	{
		if (construction.Wanted(bldToTest[i]))
		{
			AddBuildJob(bldToTest[i], x, y);
		}
	}
}

void AIPlayerJH::HandleBuilingDestroyed(const Coords& coords, BuildingType bld)
{
	MapCoord x = coords.x;
	MapCoord y = coords.y;
	if(bld==BLD_FARM)
		SetFarmedNodes(x,y,false);
}

void AIPlayerJH::HandleRoadConstructionComplete(const Coords& coords, unsigned char dir)
{
	//todo: detect "bad" roads and handle them
	MapCoord x = coords.x;
	MapCoord y = coords.y;
	const noFlag *flag;
	//does the flag still exist?
	if(!(flag = aii->GetSpecObj<noFlag>(x, y)))
		return;
	//does the roadsegment still exist?
	if(!flag->routes[dir])
		return;
	//set flags on our new road
	for(unsigned i=0; i<flag->routes[dir]->GetLength(); ++i)
		{
			aii->GetPointA(x, y, flag->routes[dir]->GetDir(false,i));
			{
				aii->SetFlag(x, y);
			}
		}
}

void AIPlayerJH::HandleRoadConstructionFailed(const Coords& coords, unsigned char dir)
{
	MapCoord x = coords.x;
	MapCoord y = coords.y;
	const noFlag *flag;
	//does the flag still exist?
	if(!(flag = aii->GetSpecObj<noFlag>(x, y)))
		return;
	//is it our flag?
	if(flag->GetPlayer() != playerid)
	{
		return;
	}
	//if it isnt a useless flag AND it has no current road connection then retry to build a road.
	if(RemoveUnusedRoad(flag, 255,true,false))
		construction.AddConnectFlagJob(flag);
}

void AIPlayerJH::HandleMilitaryBuilingLost(const Coords& coords)
{
	MapCoord x = coords.x;
	MapCoord y = coords.y;
	if(!aii->GetStorehouses().size()) //check if we have a storehouse left - if we dont have one trying to find a path to one will crash
	{		
		return;
	}
	RemoveAllUnusedRoads(x,y);

}

void AIPlayerJH::HandleBuildingFinished(const Coords& coords, BuildingType bld)
{
	switch(bld)
	{
	case BLD_HARBORBUILDING:
		UpdateNodesAround(coords.x, coords.y, 8); // todo: fix radius
		//if(!BuildingNearby(coords.x,coords.y,BLD_SHIPYARD,8))
		//{
		//	AddBuildJob(BLD_SHIPYARD, coords.x, coords.y);
		//}
		//AddBuildJob(BLD_HARBORBUILDING, coords.x, coords.y);
		//AddBuildJob(BLD_BARRACKS, coords.x, coords.y);
		//AddBuildJob(BLD_WOODCUTTER, coords.x, coords.y);
		//AddBuildJob(BLD_SAWMILL, coords.x, coords.y);
		//AddBuildJob(BLD_QUARRY, coords.x, coords.y);

		// stop beer, swords and shields -> hq only (todo: hq destroyed -> use another storehouse)
		// can't do that on harbors... maybe production is on an island which is not the hq's
		//aii->ChangeInventorySetting(coords.x, coords.y, 0, 2, 0);
		//aii->ChangeInventorySetting(coords.x, coords.y, 0, 2, 16);
		//aii->ChangeInventorySetting(coords.x, coords.y, 0, 2, 21);

		aii->StartExpedition(coords.x, coords.y);
		break;

	case BLD_SHIPYARD:
		aii->ToggleShipyardMode(coords.x, coords.y);
		break;

	case BLD_STOREHOUSE:
		break;
	case BLD_WOODCUTTER:		
		AddBuildJob(BLD_SAWMILL, coords.x, coords.y);
		break;
	default:
		break;
	}

}

void AIPlayerJH::HandleNewColonyFounded(const Coords& coords)
{
	MapCoord x=aii->GetXA(coords.x,coords.y,4),y=aii->GetYA(coords.x,coords.y,4);
	construction.AddConnectFlagJob(aii->GetSpecObj<noFlag>(x,y));
}

void AIPlayerJH::HandleExpedition(const Coords& coords)
{
	list<noBase*> objs;
	aii->GetDynamicObjects(coords.x, coords.y, objs);
	const noShip *ship = NULL;

	for(list<noBase*>::iterator it = objs.begin();it.valid();++it)
	{
		if((*it)->GetGOT() == GOT_SHIP)
		{
			if(static_cast<noShip*>(*it)->GetPlayer() == playerid)
			{
				if (static_cast<noShip*>(*it)->IsOnExpedition())
					ship = static_cast<noShip*>(*it);
			}
		}
	}


	//const noShip *ship = gwb->GetSpecObj<noShip>(coords.x, coords.y);
	if (ship)
	{
		if (ship->IsAbleToFoundColony())
			aii->FoundColony(ship);
		else
		{
			unsigned char start = rand() % 6;

			for(unsigned char i = start; i < start + 6; ++i)
			{
				if (aii->IsExplorationDirectionPossible(coords.x, coords.y, ship->GetCurrentHarbor(), i%6))
				{
					aii->TravelToNextSpot(i%6, ship);
					return;
				}
			}

			// no direction possible, sad, stop it
			aii->CancelExpedition(ship);
		}
	}

}

void AIPlayerJH::HandleTreeChopped(const Coords& coords)
{
	MapCoord x = coords.x;
	MapCoord y = coords.y;

	//std::cout << "Tree chopped." << std::endl;

	nodes[y * aii->GetMapWidth() + x].reachable = true;

	UpdateNodesAround(x, y, 3);

	int random = rand();

	

	if (random % 2 == 0)
		AddBuildJob(construction.ChooseMilitaryBuilding(x, y), x, y);
	else //if (random % 12 == 0)
		AddBuildJob(BLD_WOODCUTTER, x, y);

}

void AIPlayerJH::HandleNoMoreResourcesReachable(const Coords& coords, BuildingType bld)
{
	MapCoord x = coords.x;
	MapCoord y = coords.y;	

	// Destroy old building (once)
	
	if (aii->IsObjectTypeOnNode(x, y, NOP_BUILDING))
	{
		//keep 2 woodcutters for each forester even if they sometimes run out of trees
		if(bld==BLD_WOODCUTTER)
		{
			for (std::list<nobUsual *>::const_iterator it = aii->GetBuildings(BLD_FORESTER).begin(); it != aii->GetBuildings(BLD_FORESTER).end(); it++)
			{
				//is the forester somewhat close?
				if(gwb->CalcDistance(x,y,(*it)->GetX(),(*it)->GetY())<6)
					//then find it's 2 woodcutters
				{
					unsigned maxdist=gwb->CalcDistance(x,y,(*it)->GetX(),(*it)->GetY());
					char betterwoodcutters=0;
					for (std::list<nobUsual *>::const_iterator it2 = aii->GetBuildings(BLD_WOODCUTTER).begin(); it2 != aii->GetBuildings(BLD_WOODCUTTER).end()&&betterwoodcutters<2; it2++)
					{
						//dont count the woodcutter in question
						if(x==(*it2)->GetX()&&y==(*it2)->GetY())
							continue;
						//closer or equally close to forester than woodcutter in question?
						if(gwb->CalcDistance((*it2)->GetX(),(*it2)->GetY(),(*it)->GetX(),(*it)->GetY())<=maxdist)
							betterwoodcutters++;
					}
					//couldnt find 2 closer woodcutter -> keep it alive
					if(betterwoodcutters<2)
						return;
				}
			}
		}
		aii->DestroyBuilding(x, y);
	}
	else
		return;
	UpdateNodesAround(x, y, 11); // todo: fix radius
	RemoveUnusedRoad(aii->GetSpecObj<noFlag>(aii->GetXA(x,y,4),aii->GetYA(x,y,4)), 1,true);

	// try to expand, maybe res blocked a passage
	AddBuildJob(construction.ChooseMilitaryBuilding(x, y), x, y);
	AddBuildJob(construction.ChooseMilitaryBuilding(x, y), x, y);

	// and try to rebuild the same building
	if(bld!=BLD_HUNTER)
		AddBuildJob(bld);

	// farm is always good!
	AddBuildJob(BLD_FARM, x, y);
}

void AIPlayerJH::HandleShipBuilt(const Coords& coords)
{
	// Stop building ships if reached a maximum (TODO: make variable)
	if (aii->GetShipCount() > 5)
	{
		for (std::list<nobUsual *>::const_iterator it = aii->GetBuildings(BLD_SHIPYARD).begin(); it != aii->GetBuildings(BLD_SHIPYARD).end(); it++)
		{
			aii->StopProduction((*it)->GetX(), (*it)->GetY());
		}
	}
}

void AIPlayerJH::HandleBorderChanged(const Coords& coords)
{
	MapCoord x = coords.x;
	MapCoord y = coords.y;
	UpdateNodesAround(x, y, 11); // todo: fix radius

	const nobMilitary *mil = aii->GetSpecObj<nobMilitary>(x, y);
	if (mil)
	{
		if (mil->GetFrontierDistance() != 0 && mil->IsGoldDisabled())
		{
			aii->ToggleCoins(x, y);
		}
		if (mil->GetBuildingType() == BLD_BARRACKS || mil->GetBuildingType() == BLD_GUARDHOUSE)
		{
			AddBuildJob(construction.ChooseMilitaryBuilding(x, y), x, y);
		}
	}	
}


void AIPlayerJH::Chat(std::string message)
{
	GameMessage_Server_Chat chat = GameMessage_Server_Chat(playerid,CD_ALL,message);
	GameServer::inst().AIChat(chat);
}

void AIPlayerJH::TryToAttack() 
{
	unsigned hq_or_harbor_without_soldiers = 0;
	std::deque<const nobBaseMilitary *> potentialTargets;

	// use own military buildings (except inland buildings) to search for enemy military buildings
	for (std::list<nobMilitary*>::const_iterator it = aii->GetMilitaryBuildings().begin(); it != aii->GetMilitaryBuildings().end(); it++)
	{
		const nobMilitary* mil=(*it);
		if (mil->GetFrontierDistance() == 0)  //inland building? -> deactivate gold & skip it
		{
			if(!mil->IsGoldDisabled())
			{
				aii->ToggleCoins(mil->GetX(), mil->GetY());
			}
			continue;
		}
		else
		{
			if(mil->IsGoldDisabled())		//combat building? -> activate gold
			{
				aii->ToggleCoins(mil->GetX(), mil->GetY());
			}
		}

		// get nearby enemy buildings and store in set of potential attacking targets
		std::list<nobBaseMilitary *> buildings;

		MapCoord src_x = (*it)->GetX();
		MapCoord src_y = (*it)->GetY();

		aii->GetMilitaryBuildings(src_x, src_y, 2, buildings);
		for(std::list<nobBaseMilitary*>::iterator target = buildings.begin(); target != buildings.end(); ++target)
		{
			if ((*target)->GetGOT() == GOT_NOB_MILITARY)
			{
				const nobMilitary *enemyTarget = dynamic_cast<const nobMilitary *>((*target));

				if (enemyTarget && enemyTarget->IsNewBuilt())
					continue;
			}

			MapCoord dest_x = (*target)->GetX();
			MapCoord dest_y = (*target)->GetY();

			if (gwb->CalcDistance(src_x, src_y, dest_x, dest_y) < BASE_ATTACKING_DISTANCE 
				&& aii->IsPlayerAttackable((*target)->GetPlayer()) && aii->IsVisible(dest_x, dest_y))
			{
				if (((*target)->GetGOT() != GOT_NOB_MILITARY) && (!(*target)->DefendersAvailable()))
				{
					// headquarter or harbor without any troops :)
					hq_or_harbor_without_soldiers++;
					potentialTargets.push_front(*target);
				} else
				{
					potentialTargets.push_back(*target);
				}
			}
		}
	}

	// shuffle everything but headquarters and harbors without any troops in them
	std::random_shuffle(potentialTargets.begin() + hq_or_harbor_without_soldiers, potentialTargets.end());

	// check for each potential attacking target the number of available attacking soldiers
	for (std::deque<const nobBaseMilitary *>::iterator target = potentialTargets.begin(); target != potentialTargets.end(); target++)
	{
		const MapCoord dest_x = (*target)->GetX();
		const MapCoord dest_y = (*target)->GetY();

		unsigned attackersCount = 0;
		unsigned attackersStrength = 0;

		// ask each of nearby own military buildings for soldiers to contribute to the potential attack
		std::list<nobBaseMilitary *> myBuildings;
		aii->GetMilitaryBuildings(dest_x, dest_y, 2, myBuildings);
		for(std::list<nobBaseMilitary*>::iterator it3 = myBuildings.begin(); it3!=myBuildings.end(); ++it3)
		{
			if ((*it3)->GetPlayer() == playerid)
			{
				const nobMilitary *myMil;
				myMil = dynamic_cast<const nobMilitary *>(*it3);
				if (!myMil || myMil->IsUnderAttack())
					continue;

				unsigned newAttackers;
				attackersStrength += myMil->GetSoldiersStrengthForAttack(dest_x, dest_y, playerid, newAttackers);
				attackersCount += newAttackers;
			}
		}

		if (attackersCount == 0)
			continue;

		if ((*target)->GetGOT() == GOT_NOB_MILITARY)
		{
			const nobMilitary *enemyTarget = dynamic_cast<const nobMilitary *>((*target));

			if (enemyTarget && ((attackersStrength <= enemyTarget->GetSoldiersStrength()) || (enemyTarget->GetTroopsCount() == 0)))
			{
				continue;
			}
		}

		if ((*target)->DefendersAvailable() && (attackersCount < 3))
		{
		}

		aii->Attack(dest_x, dest_y, attackersCount, true);

		return;
	}
}


void AIPlayerJH::RecalcGround(MapCoord x_building, MapCoord y_building, std::vector<unsigned char> &route_road)
{
	MapCoord x = x_building;
	MapCoord y = y_building;

	// building itself
	RecalcBQAround(x, y);
	if (GetAINode(x, y).res == AIJH::PLANTSPACE)
	{
		ChangeResourceMap(x, y, AIJH::RES_RADIUS[AIJH::PLANTSPACE], resourceMaps[AIJH::PLANTSPACE], -1);
		GetAINode(x, y).res = AIJH::NOTHING;
	}

	// flag of building
	aii->GetPointA(x, y, 4);
	RecalcBQAround(x, y);
	if (GetAINode(x, y).res == AIJH::PLANTSPACE)
	{
		ChangeResourceMap(x, y, AIJH::RES_RADIUS[AIJH::PLANTSPACE], resourceMaps[AIJH::PLANTSPACE], -1);
		GetAINode(x, y).res = AIJH::NOTHING;
	}

	// along the road
	for (unsigned i=0; i<route_road.size(); ++i)
	{
		aii->GetPointA(x, y, route_road[i]);
		RecalcBQAround(x, y);
		// Auch Plantspace entsprechend anpassen:
		if (GetAINode(x, y).res == AIJH::PLANTSPACE)
		{
			ChangeResourceMap(x, y, AIJH::RES_RADIUS[AIJH::PLANTSPACE], resourceMaps[AIJH::PLANTSPACE], -1);
			GetAINode(x, y).res = AIJH::NOTHING;
		}
	}
}

void AIPlayerJH::SaveResourceMapsToFile()
{
#ifdef DEBUG_AI
	for(unsigned i=0; i<AIJH::RES_TYPE_COUNT; ++i)
	{
		std::stringstream ss;
		ss << "resmap-" << i << ".log";
		FILE * file = fopen(ss.str().c_str(),"w");
		for (unsigned y=0; y<aii->GetMapHeight(); ++y)
		{
			if (y % 2 == 1)
				fprintf(file,"  ");
			for (unsigned x=0; x<aii->GetMapWidth(); ++x)
			{
				fprintf(file,"%i   ",resourceMaps[i][x + y * aii->GetMapWidth()]);		
			}
			fprintf(file,"\n");
		}
		fclose(file);
	}
#endif
}

int AIPlayerJH::GetResMapValue(MapCoord x, MapCoord y, AIJH::Resource res)
{
	return resourceMaps[res][x + y * aii->GetMapWidth()];
}

void AIPlayerJH::SendAIEvent(AIEvent::Base *ev) 
{
	eventManager.AddAIEvent(ev);
}

bool AIPlayerJH::IsFlagPartofCircle(const noFlag *startFlag,unsigned maxlen,const noFlag *curFlag,unsigned char excludeDir,bool init,std::vector<int> oldflagsx,std::vector<int> oldflagsy)
{
	if(!init&&startFlag==curFlag)
		return true;
	if(maxlen<1)
		return false;
	bool partofcircle=false;
	unsigned testdir=0;
	while(testdir<6&&!partofcircle)
	{
		if (testdir == excludeDir)
		{
			testdir++;
			continue;
		}
		if(testdir==1&&(aii->IsObjectTypeOnNode(aii->GetXA(curFlag->GetX(),curFlag->GetY(),1),aii->GetYA(curFlag->GetX(),curFlag->GetY(),1),NOP_BUILDING)||aii->IsObjectTypeOnNode(aii->GetXA(curFlag->GetX(),curFlag->GetY(),1),aii->GetYA(curFlag->GetX(),curFlag->GetY(),1),NOP_BUILDINGSITE)))
		{
			testdir++;
			continue;
		}
		if(curFlag->routes[testdir])
		{
			const noFlag *flag=curFlag->routes[testdir]->GetOtherFlag(curFlag);
			bool alreadyinlist=false;
			for(unsigned i=0;i<oldflagsx.size();i++)
			{
				if (flag->GetX()==oldflagsx[i]&&flag->GetY()==oldflagsy[i])
				{
					alreadyinlist=true;
					break;
				}
			}
			if(!alreadyinlist)
			{
				oldflagsx.push_back(flag->GetX());
				oldflagsy.push_back(flag->GetY());						
				partofcircle=IsFlagPartofCircle(startFlag,maxlen-1,flag,(curFlag->routes[testdir]->GetOtherFlagDir(curFlag)+3)%6,false,oldflagsx,oldflagsy);
			}
		}
		testdir++;		
	}
	return partofcircle;
}

void AIPlayerJH::RemoveAllUnusedRoads(MapCoord x,MapCoord y)
{
	std::vector<const noFlag*> flags;
	construction.FindFlags(flags, x, y, 25);	
	// Jede Flagge testen...
	std::list<const noFlag*>reconnectflags;
	for(unsigned i=0; i<flags.size(); ++i)
	{
		if(RemoveUnusedRoad(flags[i],255,true,false))
			reconnectflags.push_back(flags[i]);
	}	
	UpdateNodesAroundNoBorder(x,y,25);
	while(reconnectflags.size()>0)
	{
		construction.AddConnectFlagJob(reconnectflags.front());
		reconnectflags.pop_front();
	}
}

bool AIPlayerJH::RemoveUnusedRoad(const noFlag *startFlag, unsigned char excludeDir,bool firstflag, bool allowcircle)
{
	unsigned char foundDir = 0xFF;
	unsigned char foundDir2= 0xFF;
	unsigned char finds = 0;
	// Count roads from this flag...
	for (unsigned char dir=0; dir < 6; ++dir)
	{
		if (dir == excludeDir)
			continue;
		if(dir==1&&(aii->IsObjectTypeOnNode(aii->GetXA(startFlag->GetX(),startFlag->GetY(),1),aii->GetYA(startFlag->GetX(),startFlag->GetY(),1),NOP_BUILDING)||aii->IsObjectTypeOnNode(aii->GetXA(startFlag->GetX(),startFlag->GetY(),1),aii->GetYA(startFlag->GetX(),startFlag->GetY(),1),NOP_BUILDINGSITE)))
		{
			//the flag belongs to a building - update the pathing map around us and try to reconnect it (if we cant reconnect it -> burn it(burning takes place at the pathfinding job))
			finds+=3;
			return true;
		}
		if(startFlag->routes[dir])
		{
			finds++;
			if(finds==1)
				foundDir = dir;
			else
				if(finds==2)
					foundDir2=dir;
		}		
	}
	// if we found more than 1 road -> the flag is still in use.	
	if (finds>2)
	{	
		return false;
	}
	else
	{
		if(finds==2)
		{
			if(allowcircle)
			{
				std::vector<int> flagcheck;
				if(!IsFlagPartofCircle(startFlag,10,startFlag,7,true,flagcheck,flagcheck))
					return false;
				if(!firstflag)
					return false;
			}
			else
				return false;
		}
	}

	// kill the flag
	aii->DestroyFlag(startFlag);

	// nothing found?
	if (foundDir > 6)
	{
		return false;
	}
	// at least 1 road exists
	RemoveUnusedRoad(startFlag->routes[foundDir]->GetOtherFlag(startFlag),(startFlag->routes[foundDir]->GetOtherFlagDir(startFlag)+3)%6,false);
	// 2 roads exist
	if(foundDir2!=0xFF)
		RemoveUnusedRoad(startFlag->routes[foundDir2]->GetOtherFlag(startFlag),(startFlag->routes[foundDir2]->GetOtherFlagDir(startFlag)+3)%6,false);
	return false;
}

unsigned AIPlayerJH::SoldierAvailable()
{
	std::list<AIJH::Coords> storeHousePoses = construction.GetStoreHousePositions();
	unsigned freeSoldiers = 0;
	for (std::list<nobBaseWarehouse*>::const_iterator it = aii->GetStorehouses().begin(); it != aii->GetStorehouses().end(); it++)
	{
		freeSoldiers += ((*it)->GetInventory()->people[JOB_PRIVATE] + (*it)->GetInventory()->people[JOB_PRIVATEFIRSTCLASS] + (*it)->GetInventory()->people[JOB_SERGEANT] + (*it)->GetInventory()->people[JOB_OFFICER] + (*it)->GetInventory()->people[JOB_GENERAL]);
	}
	return freeSoldiers;
}

bool AIPlayerJH::HuntablesinRange(unsigned x,unsigned y,unsigned min)
{
	//check first if no other hunter(or hunter buildingsite) is nearby
	if(BuildingNearby(x,y,BLD_HUNTER,15))
		return false;
	unsigned maxrange=25;
	unsigned short fx,fy,lx,ly;
	const unsigned short SQUARE_SIZE = 19;
	unsigned huntablecount=0;
	if(x > SQUARE_SIZE) fx = x-SQUARE_SIZE; else fx = 0;
	if(y > SQUARE_SIZE) fy = y-SQUARE_SIZE; else fy = 0;
	if(x+SQUARE_SIZE < aii->GetMapWidth()) lx = x+SQUARE_SIZE; else lx = aii->GetMapWidth()-1;
	if(y+SQUARE_SIZE < aii->GetMapHeight()) ly = y+SQUARE_SIZE; else ly = aii->GetMapHeight()-1;
	// Durchgehen und nach Tieren suchen
	for(unsigned short py = fy;py<=ly;++py)
	{
		for(unsigned short px = fx;px<=lx;++px)
		{
			// Gibts hier was bewegliches?
			if(gwb->GetFigures(px,py).size())
			{
				// Dann nach Tieren suchen
				for(list<noBase*>::iterator it = gwb->GetFigures(px,py).begin();it.valid();++it)
				{
					if((*it)->GetType() == NOP_ANIMAL)
					{
						// Ist das Tier überhaupt zum Jagen geeignet?
						if(!static_cast<noAnimal*>(*it)->CanHunted())
							continue;
						// Und komme ich hin?
						if(gwb->FindHumanPath(x,y,static_cast<noAnimal*>(*it)->GetX(),static_cast<noAnimal*>(*it)->GetY(),maxrange) != 0xFF)
							// Dann nehmen wir es
						{
							if(++huntablecount>=min)
								return true;
						}
						
					}	
				}
			}
		}
	}
	return false;
}

void AIPlayerJH::InitStoreAndMilitarylists()
{
	for(std::list<nobUsual*>::const_iterator it=aii->GetBuildings(BLD_FARM).begin();it!=aii->GetBuildings(BLD_FARM).end();it++)
	{
		SetFarmedNodes((*it)->GetX(),(*it)->GetY(),true);
	}
	if(aii->GetMilitaryBuildings().size()>0||aii->GetStorehouses().size()>1)
			Chat(_("AI'm back"));
}

bool AIPlayerJH::ValidTreeinRange(MapCoord x,MapCoord y)
{
	unsigned max_radius = 6;
	for(MapCoord tx=gwb->GetXA(x,y,0), r=1;r<=max_radius;tx=gwb->GetXA(tx,y,0),++r)
	{
		MapCoord tx2 = tx, ty2 = y;
		for(unsigned i = 2;i<8;++i)
		{
			for(MapCoord r2=0;r2<r;gwb->GetPointA(tx2,ty2,i%6),++r2)
			{
				//point has tree & path is available?
				if(gwb->GetNO(tx2,ty2)->GetType()==NOP_TREE)
				{
					//not already getting cut down or a freaking pineapple thingy?
					if (!gwb->GetNode(tx2,ty2).reserved && (gwb->GetSpecObj<noTree>(tx2,ty2))->type!=5)
					{
						if(gwb->FindHumanPath(x,y,tx2,ty2,20) != 0xFF)
							return true;;
					} 
				}
			}
		}
	}
	return false;
}

bool AIPlayerJH::ValidStoneinRange(MapCoord x,MapCoord y)
{
	unsigned max_radius = 8;
	for(MapCoord tx=gwb->GetXA(x,y,0), r=1;r<=max_radius;tx=gwb->GetXA(tx,y,0),++r)
	{
		MapCoord tx2 = tx, ty2 = y;
		for(unsigned i = 2;i<8;++i)
		{
			for(MapCoord r2=0;r2<r;gwb->GetPointA(tx2,ty2,i%6),++r2)
			{
				//point has tree & path is available?
				if(gwb->GetNO(tx2,ty2)->GetType()==NOP_GRANITE)
				{
					if(gwb->FindHumanPath(x,y,tx2,ty2,20) != 0xFF)
						return true;
				}
			}
		}
	}
	return false;
}

bool AIPlayerJH::BuildingNearby(MapCoord x,MapCoord y,BuildingType bld,unsigned min)
{
	//assert not a military building
	assert(bld>=10);
	for(std::list<nobUsual*>::const_iterator it=aii->GetBuildings(bld).begin();it!=aii->GetBuildings(bld).end();it++)
	{
		if(gwb->CalcDistance(x,y,(*it)->GetX(),(*it)->GetY())<min)
			return true;
	}
	for(std::list<noBuildingSite*>::const_iterator it=aii->GetBuildingSites().begin();it!=aii->GetBuildingSites().end();it++)
	{
		if((*it)->GetBuildingType()==bld)
		{
			if(gwb->CalcDistance(x,y,(*it)->GetX(),(*it)->GetY())<min)
				return true;
		}
	}
	return false;
}

bool AIPlayerJH::HarborPosClose(MapCoord x,MapCoord y,unsigned range,bool onlyempty)
{
	//skip harbordummy ... ask oliver why there has to be a dummy
	for (unsigned i=1;i<=gwb->GetHarborPointCount();i++)
	{
		if(gwb->CalcDistance(x,y,gwb->GetHarborPoint(i).x,gwb->GetHarborPoint(i).y)<range)
		{
			if(!onlyempty||!aii->IsBuildingOnNode(gwb->GetHarborPoint(i).x,gwb->GetHarborPoint(i).y,BLD_HARBORBUILDING))
				return true;
		}
	}
	return false;
}

bool AIPlayerJH::NoEnemyHarbor()
{
	for(unsigned i=1;i<=gwb->GetHarborPointCount();i++)
	{
		if(aii->IsBuildingOnNode(gwb->GetHarborPoint(i).x,gwb->GetHarborPoint(i).y,BLD_HARBORBUILDING)&&!aii->IsOwnTerritory(gwb->GetHarborPoint(i).x,gwb->GetHarborPoint(i).y))
		{
			//LOG.lprintf("found a harbor at spot %i ",i);
			return false;
		}
	}
	return true;
}

bool AIPlayerJH::IsInvalidShipyardPosition(MapCoord x,MapCoord y)
{
	if (BuildingNearby(x,y,BLD_SHIPYARD,20)||!HarborPosClose(x, y,8))
		return true;
	return false;

}
