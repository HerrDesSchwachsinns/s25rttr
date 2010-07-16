// $Id: nofFarmer.h 6582 2010-07-16 11:23:35Z FloSoft $
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

#ifndef NOF_FARMER_H_
#define NOF_FARMER_H_

#include "nofFarmhand.h"

class nofFarmer : public nofFarmhand
{
private:

	/// Was soll gemacht werden: Ernten oder S�hen?
	bool harvest;
private:

	/// Malt den Arbeiter beim Arbeiten
	void DrawWorking(int x,int y);
	/// Fragt die abgeleitete Klasse um die ID in JOBS.BOB, wenn der Beruf Waren raustr�gt (bzw rein)
	unsigned short GetCarryID() const;

	/// Abgeleitete Klasse informieren, wenn sie anf�ngt zu arbeiten (Vorbereitungen)
	void WorkStarted();
	/// Abgeleitete Klasse informieren, wenn fertig ist mit Arbeiten
	void WorkFinished();
	/// Abgeleitete Klasse informieren, wenn Arbeiten abgebrochen werden m�ssen
	void WorkAborted_Farmhand();

	/// Fragt abgeleitete Klasse, ob hier Platz bzw ob hier ein Baum etc steht, den z.B. der Holzf�ller braucht
	bool IsPointGood(const unsigned short x, const unsigned short y);

    bool checkSurrounding(unsigned short x, unsigned short y, int type);

public:

	nofFarmer(const unsigned short x, const unsigned short y,const unsigned char player,nobUsual * workplace);
	nofFarmer(SerializedGameData * sgd, const unsigned obj_id);


	/// Serialisierungsfunktionen
	protected:	void Serialize_nofFarmer(SerializedGameData * sgd) const;
	public:		void Serialize(SerializedGameData *sgd) const { Serialize_nofFarmer(sgd); }

	GO_Type GetGOT() const { return GOT_NOF_FARMER; }


};

#endif
