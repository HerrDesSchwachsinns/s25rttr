// $Id: AIConstruction.h 5881 2010-01-10 02:06:52Z jh $
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
#ifndef AICONSTRUCTION_H_INCLUDED
#define AICONSTRUCTION_H_INCLUDED

#pragma once

class AIConstruction
{
public:
	AIConstruction(void);
	~AIConstruction(void);

	  /// Finds flags in the area around x,y
	void FindFlags(std::vector<const noFlag*>& flags, unsigned short x, unsigned short y, unsigned short radius);

	/// Connects a specific flag to a roadsystem nearby and returns true if succesful. Also returns the route of the future road.
	bool ConnectFlagToRoadSytem(const noFlag *flag, std::vector<unsigned char>& route);

	/// Builds a street between two roadnodes and sets flags on it, if route is empty, it will be calculated
	bool BuildRoad(const noRoadNode *start, const noRoadNode *target, std::vector<unsigned char> &route);

	/// Checks whether a flag is connected to the road system or not (connected = has path to HQ)
	bool IsConnectedToRoadSystem(const noFlag *flag);

	/// Randomly chooses a military building, prefering bigger buildings if enemy nearby
	BuildingType ChooseMilitaryBuilding(MapCoord x, MapCoord y);

	/// Returns the number of buildings and buildingsites of a specific typ
	unsigned GetBuildingCount(BuildingType type);



	/// Checks whether a building type is wanted atm
	bool Wanted(BuildingType type);

		/// Initializes the wanted-buildings-vector
	void InitBuildingsWanted();

		/// Update BQ and farming ground around new building site + road
	void RecalcGround(MapCoord x_building, MapCoord y_building, std::vector<unsigned char> &route_road);

		/// Tries to build a second road to a flag, which is in any way better than the first one
	bool BuildAlternativeRoad(const noFlag *flag, std::vector<unsigned char> &route);

	bool FindStoreHousePosition(MapCoord &x, MapCoord &y, unsigned radius);
	std::list<Coords> storeHouses;
	void AddStoreHouse(MapCoord x, MapCoord y) { storeHouses.push_back(Coords(x, y)); }

	noFlag *FindTargetStoreHouseFlag(MapCoord x, MapCoord y);

private:
		/// Contains how many buildings of every type is wanted
	std::vector<unsigned> buildingsWanted;
};

#endif //! AICONSTRUCTION_H_INCLUDED
