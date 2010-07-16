// $Id: nofArmorer.h 6582 2010-07-16 11:23:35Z FloSoft $
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

#ifndef NOF_ARMORER_H_
#define NOF_ARMORER_H_

#include "nofWorkman.h"

class nobUsualBuilding;

/// Klasse f�r den Schmied
class nofArmorer : public nofWorkman
{
private:

	/// Bestimmt, was der Schmied als n�chstes schmieden soll (immer Schwert-Schild im Wechsel)
	bool sword_shield;

private:
	void DrawWorking(int x, int y);
	/// Gibt die ID in JOBS.BOB zur�ck, wenn der Beruf Waren raustr�gt (bzw rein)
	unsigned short GetCarryID() const;
	/// Der Arbeiter erzeugt eine Ware
	GoodType ProduceWare();

public:

	nofArmorer(const unsigned short x, const unsigned short y,const unsigned char player,nobUsual * workplace);
	nofArmorer(SerializedGameData * sgd, const unsigned obj_id);

	/// Serialisierungsfunktionen
	protected:	void Serialize_nofArmorer(SerializedGameData * sgd) const;
	public:		void Serialize(SerializedGameData *sgd) const { Serialize_nofArmorer(sgd); }

	GO_Type GetGOT() const { return GOT_NOF_ARMORER; }
};

#endif
