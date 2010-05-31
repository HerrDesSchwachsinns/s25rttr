// $Id: nofCatapultMan.h 6458 2010-05-31 11:38:51Z FloSoft $
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
#ifndef NOF_CATAPULTMAN_H_
#define NOF_CATAPULTMAN_H_

#include "nofBuildingWorker.h"
#include "SerializedGameData.h"

/// Arbeiter im Katapult
class nofCatapultMan : public nofBuildingWorker
{
	/// Drehschritte f�r den Katapult auf dem Dach, bis er die Angriffsrichtung erreicht hat
	/// negativ - andere Richtung!
	int wheel_steps;

	/// Ein m�gliches Ziel f�r den Katapult
	class PossibleTarget
	{
	public:
		
		/// Geb�ude
		unsigned short x,y;
		/// Entfernung
		unsigned distance;

		PossibleTarget() : x(0), y(0), distance(0) {}
		PossibleTarget(const unsigned short x, const unsigned short y, const unsigned distance) : x(x), y(y), distance(distance) {}
		PossibleTarget(SerializedGameData *sgd) : x(sgd->PopUnsignedShort()), y(sgd->PopUnsignedShort()), distance(sgd->PopUnsignedInt()) {}

		void Serialize_PossibleTarget(SerializedGameData *sgd) const
		{	
			sgd->PushUnsignedShort(x);
			sgd->PushUnsignedShort(y);
			sgd->PushUnsignedInt(distance);
		}

	} target; /// das anvisierte Ziel

private:

	/// Funktionen, die nur von der Basisklasse (noFigure) aufgerufen werden, wenn man gelaufen ist
	void WalkedDerived();
	/// Malt den Arbeiter beim Arbeiten
	void DrawWorking(int x,int y);
	/// Fragt die abgeleitete Klasse um die ID in JOBS.BOB, wenn der Beruf Waren raustr�gt (bzw rein)
	unsigned short GetCarryID() const { return 0; }

	/// Wenn jeweils gelaufen wurde oder ein Event abgelaufen ist, je nach aktuellem Status folgende Funktionen ausf�hren
	void HandleStateTargetBuilding();
	void HandleStateBackOff();

public:

	nofCatapultMan(const unsigned short x, const unsigned short y,const unsigned char player,nobUsual * workplace);
	nofCatapultMan(SerializedGameData * sgd, const unsigned obj_id);

	/// Serialisierungsfunktionen
	protected:	void Serialize_nofCatapultMan(SerializedGameData * sgd) const;
	public:		void Serialize(SerializedGameData *sgd) const { Serialize_nofCatapultMan(sgd); }

	GO_Type GetGOT() const { return GOT_NOF_CATAPULTMAN; }

	void HandleDerivedEvent(const unsigned int id);

	/// wird aufgerufen, wenn die Arbeit abgebrochen wird (von nofBuildingWorker aufgerufen)
	void WorkArborted();
};



#endif

