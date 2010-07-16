// $Id: noFighting.h 6582 2010-07-16 11:23:35Z FloSoft $
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

#ifndef NO_FIGHTING_H_
#define NO_FIGHTING_H_

#include "noBase.h"
#include "EventManager.h"

class nofActiveSoldier;

/// Kampf an einem Punkt zwischen 2 Soldaten, der erstgenannt ist immer der, der links steht
class noFighting : public noBase
{
	/// die k�mpfenden Soldaten
	nofActiveSoldier * soldiers[2];
	// Wer ist an der Reihe mit angreifen (2 = Beginn des Kampfes)
	unsigned char turn;
	/// Verteidigungsanimation (3 = keine Verteidigung,  Treffer)
	unsigned char defending_animation;
	/// Event
	EventManager::EventPointer current_ev;
	/// Spieler des Soldaten, der gewonnen hat
	unsigned char player_won;

private:

	/// Bestimmt, ob der Angreifer erfolgreich angreift oder ob der Verteidiger sich verteidigt usw
	/// bereitet also alles f�r eine solche Angrifsseinheit vor
	void StartAttack();

public:

	noFighting(nofActiveSoldier * soldier1,nofActiveSoldier * soldier2);
	noFighting(SerializedGameData * sgd, const unsigned obj_id);

	/// Aufr�ummethoden
protected:	void Destroy_noFighting();
public:		void Destroy() { Destroy_noFighting(); }

	/// Serialisierungsfunktionen
	protected:	void Serialize_noFighting(SerializedGameData * sgd) const;
	public:		void Serialize(SerializedGameData *sgd) const { Serialize_noFighting(sgd); }

	GO_Type GetGOT() const { return GOT_FIGHTING; }

	void Draw(int x, int y);
	void HandleEvent(const unsigned int id);

	/// D�rfen andern Figuren diesen Kampf schon durchqueren?
	bool IsActive() const;

	/// Pr�fen, ob ein Soldat von einem bestimmten Spieler in den Kampf verwickelt ist
	bool IsSoldierOfPlayer(const unsigned char player) const;

};




#endif
