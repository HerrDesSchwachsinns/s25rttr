// $Id: AIEventManager.h 5644 2009-10-17 12:22:53Z jh $
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
#ifndef AIEVENTMANAGER_H_INCLUDED
#define AIEVENTMANAGER_H_INCLUDED

#pragma once

#include "main.h"
#include "MapConsts.h"
#include "GameConsts.h"
#include <queue>

namespace AIEvent
{
	enum AIEventType
	{
		BuildingDestroyed,
		BuildingConquered,
		BuildingLost,
		BuildingOccupied,
		BorderChanged,
		TerritoryLost,
		NoMoreResourcesReachable
	};


	class Base
	{
	public:
		Base(AIEventType type) : type(type) { }
		virtual ~Base() { }
		AIEventType GetType() const { return type; }

	protected:
		AIEventType type;
	};


	class Location : public Base
	{
	public:
		Location(AIEventType type, MapCoord x, MapCoord y) : Base(type), x(x), y(y) { }
		~Location() { }
		MapCoord GetX() const { return x; }
		MapCoord GetY() const { return y; }

	protected:
		MapCoord x, y;
	};


	class Building : public Location
	{
	public:
		Building(AIEventType type, MapCoord x, MapCoord y, BuildingType building) : Location(type, x, y), building(building) { }
		~Building() { }
		BuildingType GetBuildingType() const { return building; }

	protected:
		BuildingType building;
	};


}

class AIEventManager
{
public:
	AIEventManager(void);
	~AIEventManager(void);
	void AddAIEvent(AIEvent::Base *ev) { events.push(ev); }
	AIEvent::Base *GetEvent();

protected:
	std::queue<AIEvent::Base *> events;
};


#endif // !AIEVENTMANAGER_H_INCLUDED