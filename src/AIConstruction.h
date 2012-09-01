// $Id: AIConstruction.h 8119 2012-09-01 19:12:36Z jh $
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
#ifndef AICONSTRUCTION_H_INCLUDED
#define AICONSTRUCTION_H_INCLUDED

#pragma once

#include <limits>
#include <vector>
#include "list.h"
#include "AIJHHelper.h"
#include "GameWorld.h"
#include "noFlag.h"
#include "AIInterface.h"

class AIPlayerJH;

namespace gc { class GameCommand; }

class AIConstruction
{
public:
	AIConstruction(AIInterface *aii, AIPlayerJH *aijh);
	~AIConstruction(void);

	/// Adds a build job to the queue
	void AddBuildJob(AIJH::BuildJob *job, bool front);
	void AddJob(AIJH::Job *job, bool front);

	AIJH::Job *GetBuildJob();
	unsigned GetBuildJobNum() const { return buildJobs.size(); }

	void AddConnectFlagJob(const noFlag *flag); // todo aijh als param suckt

	bool BuildJobAvailable() const { return buildJobs.size() > 0;}

	/// Finds flags in the area around x,y
	void FindFlags(std::vector<const noFlag*>& flags, unsigned short x, unsigned short y, unsigned short radius, bool clear = true);

	void FindFlags(std::vector<const noFlag*>& flags, unsigned short x, unsigned short y, unsigned short radius, 
															 unsigned short real_x, unsigned short real_y, unsigned short real_radius, bool clear = true);

	/// Connects a specific flag to a roadsystem nearby and returns true if succesful. Also returns the route of the future road.
	bool ConnectFlagToRoadSytem(const noFlag *flag, std::vector<unsigned char>& route, unsigned int maxSearchRadius = 10);

	/// Builds a street between two roadnodes and sets flags on it, if route is empty, it will be calculated
	bool BuildRoad(const noRoadNode *start, const noRoadNode *target, std::vector<unsigned char> &route);

	/// Checks whether a flag is connected to the road system or not (connected = has path to HQ)
	bool IsConnectedToRoadSystem(const noFlag *flag);

	/// Randomly chooses a military building, prefering bigger buildings if enemy nearby
	BuildingType ChooseMilitaryBuilding(MapCoord x, MapCoord y);

	/// Returns the number of buildings and buildingsites of a specific type (refresh with RefreshBuildingCount())
	unsigned GetBuildingCount(BuildingType type);

	/// Returns the number of buildingsites of a specific type (refresh with RefreshBuildingCount())
	unsigned GetBuildingSitesCount(BuildingType type);

	/// Refreshes the number of buildings by asking the GameClientPlayer and recalcs some wanted buildings
	void RefreshBuildingCount();


	/// Checks whether a building type is wanted atm
	bool Wanted(BuildingType type);

	/// Initializes the wanted-buildings-vector
	void InitBuildingsWanted();

	/// Update BQ and farming ground around new building site + road
	/// HIer oder in AIPlayerJH?
	//void RecalcGround(MapCoord x_building, MapCoord y_building, std::vector<unsigned char> &route_road);

	/// Tries to build a second road to a flag, which is in any way better than the first one
	bool BuildAlternativeRoad(const noFlag *flag, std::vector<unsigned char> &route);

	bool FindStoreHousePosition(MapCoord &x, MapCoord &y, unsigned radius);
	
	void AddStoreHouse(MapCoord x, MapCoord y) { storeHouses.push_back(AIJH::Coords(x, y)); }
	void AddStoreHouseFront(MapCoord x,MapCoord y) {storeHouses.push_front(AIJH::Coords(x,y));}

	noFlag *FindTargetStoreHouseFlag(MapCoord x, MapCoord y);

	std::list<AIJH::Coords> &GetStoreHousePositions() { return storeHouses; }

private:
	/// Contains how many buildings of every type is wanted
	std::vector<unsigned> buildingsWanted;

	/// The current job the AI is working on
	AIJH::Job *currentJob;

	/// Contains the jobs the AI should try to execute, for example build jobs
	std::deque<AIJH::Job*> buildJobs;

	AIInterface *aii;
	AIPlayerJH *aijh;

	/// Number of buildings and building sites of this player (refreshed by RefreshBuildingCount())
	BuildingCount buildingCounts;

	std::list<AIJH::Coords> storeHouses;

	unsigned char playerID;
};

#endif //! AICONSTRUCTION_H_INCLUDED
