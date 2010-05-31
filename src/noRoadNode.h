// $Id: noRoadNode.h 6458 2010-05-31 11:38:51Z FloSoft $
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

#ifndef NO_ROADNODE_H_
#define NO_ROADNODE_H_

#include "noCoordBase.h"

class RoadSegment;
class noFigure;
class Ware;
// Enumforwarddeklaration bei VC nutzen
#ifdef _MSC_VER
enum Job;
#else
#include "GameConsts.h"
#endif

// Basisklasse f�r Geb�ude und Flagge (alles, was als "Stra�enknoten" dient
class noRoadNode : public noCoordBase
{
protected:

	unsigned char player;

public:

	RoadSegment	* routes[6];

public:

	noRoadNode(const NodalObjectType nop,const unsigned short x, const unsigned short y,const unsigned char player);
	noRoadNode(SerializedGameData * sgd, const unsigned obj_id);

	virtual ~noRoadNode();
	/// Aufr�ummethoden
protected:	void Destroy_noRoadNode();
public:		void Destroy() { Destroy_noRoadNode(); }

	/// Serialisierungsfunktionen
	protected:	void Serialize_noRoadNode(SerializedGameData * sgd) const;
	public:		void Serialize(SerializedGameData *sgd) const { Serialize_noRoadNode(sgd); }

	noRoadNode * GetNeighbour(const unsigned char dir) const;
	void DestroyRoad(const unsigned char dir);
	void UpgradeRoad(const unsigned char dir);
	/// Vernichtet Alle Stra�e um diesen Knoten
	void DestroyAllRoads();

	unsigned char GetPlayer() const { return player; }

	/// Legt eine Ware am Objekt ab (an allen Stra�enknoten (Geb�ude, Baustellen und Flaggen) kann man Waren ablegen
	virtual void AddWare(Ware * ware) = 0;

	/// Nur f�r Flagge, Geb�ude k�nnen 0 zur�ckgeben, gibt Wegstrafpunkte f�r das Pathfinden f�r Waren, die in eine bestimmte Richtung noch transportiert werden m�ssen
	virtual unsigned short GetPunishmentPoints(const unsigned char dir) const { return 0; }

};

#endif
