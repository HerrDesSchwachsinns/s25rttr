// $Id: AIPlayerJH.h 6412 2010-05-06 22:34:54Z jh $
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
#ifndef AIPLAYERJH_H_INCLUDED
#define AIPLAYERJH_H_INCLUDED

#pragma once

#include "AIBase.h"
#include "MapConsts.h"
#include "GameConsts.h"
#include "GameClientPlayer.h"
#include "AIJHHelper.h"
#include "GameWorld.h"
#include "AIEventManager.h"
#include "AIConstruction.h"

#include <queue>
#include <deque>
#include <list>

class noFlag;
class noBaseBuilding;
class noRoadNode;
class nobBaseMilitary;
class AIPlayerJH;
class nobMilitary;
class nobBaseMilitary;


/// Klasse f�r die besser JH-KI
class AIPlayerJH : public AIBase
{
	friend class AIJH::BuildJob;
	friend class AIJH::EventJob;
	friend class AIJH::ConnectJob;
	friend class iwAIDebug;
public:
	AIPlayerJH(const unsigned char playerid, const GameWorldBase * const gwb, const GameClientPlayer * const player,
		const GameClientPlayerList * const players, const GlobalGameSettings * const ggs,
		const AI::Level level);


	int GetResMapValue(MapCoord x, MapCoord y, AIJH::Resource res);

protected:
	struct Coords
	{
		MapCoord x;
		MapCoord y;
		Coords(MapCoord x, MapCoord y) : x(x), y(y) { }
	};

	void RunGF(const unsigned gf);

	void SendAIEvent(AIEvent::Base *ev);


	/// Test whether the player should resign or not
	bool TestDefeat();

	/// resigned yes/no
	bool defeated;

	/// Executes a job form the job queue
	void ExecuteAIJob();
	void AddBuildJob(AIJH::BuildJob *job, bool front = false) { construction.AddBuildJob(job, front); }

	/// Checks the list of military buildingsites and puts the coordinates into the list of military buildings if building is finished
	void CheckNewMilitaryBuildings();

	/// Initializes the nodes on start of the game
	void InitNodes();

	/// Updates the nodes around a position 
	void UpdateNodesAround(MapCoord x, MapCoord y, unsigned radius);

	/// Returns the resource on a specific point
	AIJH::Resource CalcResource(MapCoord x, MapCoord y);

	/// Initialize the resource maps
	void InitResourceMaps();

	/// Changes a single resource map around point x,y in radius; to every point around x,y distanceFromCenter * value is added
	void ChangeResourceMap(MapCoord x, MapCoord y, unsigned radius, std::vector<int> &resMap, int value);

	/// Finds a good position for a specific resource in an area using the resource maps, 
	/// first position satisfying threshold is returned, returns false if no such position found
	bool FindGoodPosition(MapCoord &x, MapCoord &y, AIJH::Resource res, int threshold, BuildingQuality size, int radius = -1, bool inTerritory = true);

	/// Finds the best position for a specific resource in an area using the resource maps, 
	/// satisfying the minimum value, returns false if no such position is found
	bool FindBestPosition(MapCoord &x, MapCoord &y, AIJH::Resource res, BuildingQuality size, int minimum, int radius = -1, bool inTerritory = true);
	bool FindBestPosition(MapCoord &x, MapCoord &y, AIJH::Resource res, BuildingQuality size, int radius = -1, bool inTerritory = true) 
	{ return FindBestPosition(x,y,res,size,1,radius,inTerritory); }

	/// Finds a position for the desired building size
	bool SimpleFindPosition(MapCoord &x, MapCoord &y, BuildingQuality size, int radius = -1);

	double GetDensity(MapCoord x, MapCoord y, AIJH::Resource res, int radius);

	/// Recalculate the Buildingquality around a certain point
	void RecalcBQAround(const MapCoord x, const MapCoord y);

	/// Does some actions after a new military building is occupied
	void HandleNewMilitaryBuilingOccupied(const Coords& coords);

	// Handle event "no more resources"
	void HandleNoMoreResourcesReachable(const Coords& coords, BuildingType bld);

	// Handle border event
	void HandleBorderChanged(const Coords& coords);

	// Handle usual building finished
	void HandleBuildingFinished(const Coords& coords, BuildingType bld);

	void HandleExpedition(const Coords& coords);

	// Handle chopped tree, test for new space
	void HandleTreeChopped(const Coords& coords);

	/// Sends a chat messsage to all players
	void Chat(std::string message);

	/// Tries to attack the enemy
	void TryToAttack();

	/// Update BQ and farming ground around new building site + road
	void RecalcGround(MapCoord x_building, MapCoord y_building, std::vector<unsigned char> &route_road);

	void SaveResourceMapsToFile();

	void InitReachableNodes();
	void UpdateReachableNodes(MapCoord x, MapCoord y, unsigned radius);
	void IterativeReachableNodeChecker(std::queue<std::pair<MapCoord, MapCoord> >& toCheck);

	void SetFarmedNodes(MapCoord x, MapCoord y);

	void RemoveUnusedRoad(const noFlag *startFlag, unsigned char excludeDir = 0xFF);


protected:
	/// The current job the AI is working on
	AIJH::Job *currentJob;

	/// Contains the jobs the AI should try to execute, for example build jobs
	/// std::deque<AIJH::Job*> aiJobs;

	/// List of coordinates at which military buildings should be
	std::list<Coords> milBuildings;

	/// List of coordinates at which military buildingsites should be
	std::list<Coords> milBuildingSites;

	/// Nodes containing some information about every map node
	std::vector<AIJH::Node> nodes;

	/// Resource maps, containing a rating for every map point concerning a resource
	std::vector<std::vector<int> > resourceMaps;

	// Required by the AIJobs:

	std::vector<gc::GameCommand*> &GetGCS() { return gcs; }
	const GameClientPlayer * const GetPlayer() const { return player; }
	const GameWorldBase *GetGWB() { return gwb; }
	unsigned char GetPlayerID() { return playerid; }
	AIConstruction *GetConstruction() { return &construction; }
	AIJH::Job *GetCurrentJob() { return currentJob; }
public:
	inline AIJH::Node &GetAINode(MapCoord x, MapCoord y) { return nodes[x + gwb->GetWidth() * y]; }
	inline unsigned GetJobNum() const { return eventManager.GetEventNum() + construction.GetBuildJobNum(); }

// Event...
protected:
	AIEventManager eventManager;
	AIConstruction construction;


};


#endif //!AIPLAYERJH_H_INCLUDED
