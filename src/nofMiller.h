// $Id: nofMiller.h 5853 2010-01-04 16:14:16Z FloSoft $
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

#ifndef NOF_MILLER_H_
#define NOF_MILLER_H_

#include "nofWorkman.h"

class nobUsualBuilding;

/// Klasse f�r den Schreiner
class nofMiller : public nofWorkman
{
	/// Letzter M�hlensound-Zeitpunkt
	unsigned last_sound;
	/// Intervall zum n�chsten M�hlensound
	unsigned next_interval;

private:

	/// Zeichnet ihn beim Arbeiten
	void DrawWorking(int x, int y);
	/// Gibt die ID in JOBS.BOB zur�ck, wenn der Beruf Waren raustr�gt (bzw rein)
	unsigned short GetCarryID() const { return 75; }
	/// Der Arbeiter erzeugt eine Ware
	GoodType ProduceWare();

public:

	nofMiller(const unsigned short x, const unsigned short y,const unsigned char player,nobUsual * workplace);
	nofMiller(SerializedGameData * sgd, const unsigned obj_id);

	/// Serialisierungsfunktionen
	protected:	void Serialize_nofMiller(SerializedGameData * sgd) const;
	public:		void Serialize(SerializedGameData *sgd) const { Serialize_nofMiller(sgd); }

	GO_Type GetGOT() const { return GOT_NOF_MILLER; }
};

#endif
