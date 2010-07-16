// $Id: nofFlagWorker.h 6582 2010-07-16 11:23:35Z FloSoft $
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

#ifndef NOF_FLAG_WORKER_H_
#define NOF_FLAG_WORKER_H_

#include "noFigure.h"

class noFlag;

/// Basisklasse f�r Geologen und Sp�her, also die, die an eine Flagge gebunden sind zum Arbeiten
class nofFlagWorker : public noFigure
{
protected:
	/// Flaggen-Ausgangspunkt
	noFlag * flag;

	enum State
	{
		STATE_FIGUREWORK, // Zur Flagge und zur�ckgehen, Rumirren usw
		STATE_GOTOFLAG, // geht zur�ck zur Flagge um anschlie�end nach Hause zu gehen

		STATE_GEOLOGIST_GOTONEXTNODE, // Zum n�chsten Punkt gehen, um dort zu graben
		STATE_GEOLOGIST_DIG, // graben (mit Hammer auf Berg hauen)
		STATE_GEOLOGIST_CHEER, // Jubeln, dass man etwas gefunden hat

		STATE_SCOUT_SCOUTING // l�uft umher und erkundet
	} state;

protected:

	/// K�ndigt bei der Flagge
	void AbrogateWorkplace();
	/// Geht wieder zur�ck zur Flagge und dann nach Hause
	void GoToFlag();

public:

	nofFlagWorker(const Job job,const unsigned short x, const unsigned short y,const unsigned char player,noRoadNode * goal);
	nofFlagWorker(SerializedGameData * sgd, const unsigned obj_id);

	/// Aufr�ummethoden
protected:	void Destroy_nofFlagWorker();
public:		void Destroy() { Destroy_nofFlagWorker(); }

	/// Serialisierungsfunktionen
	protected:	void Serialize_nofFlagWorker(SerializedGameData * sgd) const;
	public:		void Serialize(SerializedGameData *sgd) const { Serialize_nofFlagWorker(sgd); }


	/// Wird aufgerufen, wenn die Flagge abgerissen wurde
	virtual void LostWork() = 0;

	/// Gibt Flagge zur�ck
	noFlag * GetFlag() const { return flag; }

};


#endif
