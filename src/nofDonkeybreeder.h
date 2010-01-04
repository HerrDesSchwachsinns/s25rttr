// $Id: nofDonkeybreeder.h 5853 2010-01-04 16:14:16Z FloSoft $
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
#ifndef NOFDONKEYBREEDER_H_INCLUDED
#define NOFDONKEYBREEDER_H_INCLUDED

#pragma once

#include "nofWorkman.h"

class nobUsualBuilding;

/// Klasse f�r den Eselz�chter
class nofDonkeybreeder : public nofWorkman
{
public:
	nofDonkeybreeder(unsigned short x, unsigned short y, unsigned char player, nobUsual *workplace);
	nofDonkeybreeder(SerializedGameData *sgd, unsigned int obj_id);

	GO_Type GetGOT() const { return GOT_NOF_DONKEYBREEDER; }

private:
	/// Zeichnet ihn beim Arbeiten.
	void DrawWorking(int x, int y);
	/// Der Arbeiter erzeugt eine Ware.
	GoodType ProduceWare();
	/// Wird aufgerufen, wenn er fertig mit arbeiten ist
	void WorkFinished();

	/// Gibt die ID in JOBS.BOB zur�ck, wenn der Beruf Waren raustr�gt (bzw rein)
	unsigned short GetCarryID() const { return 0; }
};

#endif // !NOFDONKEYBREEDER_H_INCLUDED
