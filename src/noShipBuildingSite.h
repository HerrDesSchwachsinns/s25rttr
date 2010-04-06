// $Id: noShipBuildingSite.h 6267 2010-04-05 09:16:14Z OLiver $
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
#ifndef SHIP_BUILDING_SITE_H_
#define SHIP_BUILDING_SITE_H_

#include "noCoordBase.h"

/// Menschliches Skelett (Zierobjekt, das sich automatisch umwandelt und dann verschwindet)
class noShipBuildingSite: public noCoordBase
{
public:

	noShipBuildingSite(const unsigned short x, const unsigned short y, const unsigned char player);
	noShipBuildingSite(SerializedGameData * sgd, const unsigned obj_id);
	~noShipBuildingSite();
	void Destroy();
	void Serialize(SerializedGameData *sgd) const;
	GO_Type GetGOT() const { return GOT_SHIPBUILDINGSITE; }

	/// Gibt den Eigent�mer zur�ck
	unsigned char GetPlayer() const { return player; }

	/// Das Schiff wird um eine Stufe weitergebaut
	void MakeBuildStep();

	BlockingManner GetBM() const { return BM_HUT; }

protected:	

	void Draw(int x, int y);

private:

	/// Spieler, dem dieses Schiff geh�rt
	unsigned char player;
	/// Baufortschritt des Schiffes
	unsigned char progress;

};

#endif // !NOSKELETON_H_INCLUDED
