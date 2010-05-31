// $Id: noDisappearingEnvObject.h 6458 2010-05-31 11:38:51Z FloSoft $
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
#ifndef NO_DISAPPEARING_ENVOBJECT
#define NO_DISAPPEARING_ENVOBJECT

#include "noCoordBase.h"
#include "EventManager.h"

class noDisappearingEnvObject : public noCoordBase
{
public:
	/// Konstruktor von @p noDisappearingEnvObject.
	noDisappearingEnvObject(const unsigned short x, const unsigned short y, const unsigned living_time,
		const unsigned add_var_living_time);
	noDisappearingEnvObject(SerializedGameData * sgd, const unsigned obj_id);

	/// Aufr�ummethoden
protected:	void Destroy_noDisappearingEnvObject();
public:		void Destroy() { Destroy_noDisappearingEnvObject(); }
	/// Serialisierungsfunktionen
	protected:	void Serialize_noDisappearingEnvObject(SerializedGameData * sgd) const;
	public:		void Serialize(SerializedGameData *sgd) const { Serialize_noDisappearingEnvObject(sgd); }

	/// Benachrichtigen, wenn neuer GF erreicht wurde.
	void HandleEvent_noDisappearingEnvObject(const unsigned int id);

protected:

	/// Gibt Farbe zur�ck, mit der das Objekt gezeichnet werden soll
	unsigned GetDrawColor() const;
	/// Gibt Farbe zur�ck, mit der der Schatten des Objekts gezeichnet werden soll
	unsigned GetDrawShadowColor() const;

private:

	/// Bin ich grad in der Sterbephase (in der das Schild immer transparenter wird, bevor es verschwindet)
	bool disappearing;
	/// Event, das bestimmt wie lange es noch lebt
	EventManager::EventPointer dead_event;
};


#endif
