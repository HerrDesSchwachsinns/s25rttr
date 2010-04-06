// $Id: noBaseBuilding.h 6267 2010-04-05 09:16:14Z OLiver $
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

#ifndef NO_BASEBUILDING_H_
#define NO_BASEBUILDING_H_

#include "noRoadNode.h"
#include "noFlag.h"
#include "BuildingConsts.h"


class noBaseBuilding : public noRoadNode
{
protected:

	/// Typ des Geb�udes
	BuildingType type;

	/// Volk des Geb�udes (muss extra gespeichert werden, da ja auch z.B. fremde Geb�ude erobert werden k�nnen)
	const Nation nation;
	/// Doorpoints - Punkte, wo die T�r ist, bis wohin die Tr�ger gehen d�rfen
	int door_point_x;
	int door_point_y;

protected:

	/// Ware Bescheid sagen, dass sie nicht mehr hierher kommen brauch
	void WareNotNeeded(Ware * ware);
	/// Zerst�rt Anbauten, falls es sich um ein gro�es Geb�ude handelt (wo es diese auch gibt)
	void DestroyBuildingExtensions();

public:

	noBaseBuilding(const NodalObjectType nop,const BuildingType type,const unsigned short x, const unsigned short y,const unsigned char player);
	noBaseBuilding(SerializedGameData * sgd, const unsigned obj_id);

	virtual ~noBaseBuilding();

	/// Aufr�ummethoden
protected:	void Destroy_noBaseBuilding();
public:		void Destroy() { Destroy_noBaseBuilding(); }

	/// Serialisierungsfunktionen
	protected:	void Serialize_noBaseBuilding(SerializedGameData * sgd) const;
	public:		void Serialize(SerializedGameData *sgd) const { Serialize_noBaseBuilding(sgd); }

	/// Eine bestellte Ware konnte doch nicht kommen
	virtual void WareLost(Ware * ware) = 0;

	/// Gibt diverse Sachen zur�ck
	BuildingQuality GetSize() const { return BUILDING_SIZE[type]; }
	BuildingType GetBuildingType () const { return type; }
	Nation GetNation() const { return nation; }
	virtual BlockingManner GetBM() const;

	/// Ermittelt die Flagge, die vor dem Geb�ude steht
	noFlag * GetFlag() const;

	short GetDoorPointX();
	short GetDoorPointY() const { return (door_point_y & 0xFFFF); }

	/*/// Gibt die Warenverteilungspunkte zur�ck (bei 0 wurde kein Weg gefunden)
	virtual unsigned CalcDistributionPoints(noRoadNode * start,const GoodType type) = 0;*/
	/// Wird aufgerufen, wenn eine neue Ware zum dem Geb�ude geliefert wird (nicht wenn sie bestellt wurde vom Geb�ude!)
	virtual void TakeWare(Ware * ware) = 0;
	/// Wird aufgerufen, wenn ein bestimmter Arbeiter f�r das hier gerufen wurde
	virtual void GotWorker(Job job, noFigure * worker) {};
	
};


#endif

