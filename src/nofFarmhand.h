// $Id: nofFarmhand.h 6582 2010-07-16 11:23:35Z FloSoft $
//
// Copyright (c) 2005 - 2010 Settlers Freaks (sf-team at siedler25.org)
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

#ifndef NOF_FARMHAND_H_
#define NOF_FARMHAND_H_

#include "nofBuildingWorker.h"


/// Ein Landarbeiter geht raus aus seiner H�tte und arbeitet in "freier Natur"
class nofFarmhand : public nofBuildingWorker
{
protected:

	/// Arbeitsziel, das der Arbeiter ansteuert
	unsigned short dest_x,dest_y;

private:

	/// Funktionen, die nur von der Basisklasse (noFigure) aufgerufen werden, wenn...
	void WalkedDerived();

	/// Arbeit musste wegen Arbeitsplatzverlust abgebrochen werden
	void WorkAborted();
	/// Arbeit musste wegen Arbeitsplatzverlust abgebrochen werden (an abgeleitete Klassen)
	virtual void WorkAborted_Farmhand();


	///// F�ngt das "Warten-vor-dem-Arbeiten" an, falls er arbeiten kann (m�ssen ja bestimmte "Naturobjekte" gegeben sein)
	//void TryToWork();
	/// Findet heraus, ob der Beruf an diesem Punkt arbeiten kann
	bool IsPointAvailable(const unsigned short x, const unsigned short y);
	/// Fragt abgeleitete Klasse, ob hier Platz bzw ob hier ein Baum etc steht, den z.B. der Holzf�ller braucht
	virtual bool IsPointGood(const unsigned short x, const unsigned short y) = 0;

	/// L�uft zum Arbeitspunkt
	void WalkToWorkpoint();
	/// Trifft Vorbereitungen f�rs nach Hause - Laufen
	void StartWalkingHome();
	/// L�uft wieder zu seiner H�tte zur�ck
	void WalkHome();

	///// Abgeleitete Klasse informieren, wenn man anf�ngt zum Arbeitspunkt zu laufen (Reservierungen)
	//virtual void WalkingStarted() = 0;
	/// Abgeleitete Klasse informieren, wenn sie anf�ngt zu arbeiten (Vorbereitungen)
	virtual void WorkStarted() = 0;
	/// Abgeleitete Klasse informieren, wenn fertig ist mit Arbeiten
	virtual void WorkFinished() = 0;
	/// Wird aufgerufen, wenn der Arbeiter seine Arbeit pl�tzlich abrechen

	/// Zeichnen der Figur in sonstigen Arbeitslagen
	void DrawOtherStates(const int x, const int y);

public:

	nofFarmhand(const Job job,const unsigned short x, const unsigned short y,const unsigned char player,nobUsual * workplace);
	nofFarmhand(SerializedGameData * sgd, const unsigned obj_id);

	/// Aufr�ummethoden
protected:	void Destroy_nofFarmhand() { Destroy_nofBuildingWorker(); }
public:		void Destroy() { Destroy_nofFarmhand(); }

	/// Serialisierungsfunktionen
	protected:	void Serialize_nofFarmhand(SerializedGameData * sgd) const;
	public:		void Serialize(SerializedGameData *sgd) const { Serialize_nofFarmhand(sgd); }



	void HandleDerivedEvent(const unsigned int id);

	
};


#endif
