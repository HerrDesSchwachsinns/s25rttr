// $Id: EventManager.h 6582 2010-07-16 11:23:35Z FloSoft $
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
#ifndef EVENTMANAGER_H_INCLUDED
#define EVENTMANAGER_H_INCLUDED

#pragma once

#include "Singleton.h"
#include "GameConsts.h"
#include "list.h"
#include "GameObject.h"

class GameObject;
class SerializedGameData;

class EventManager
{
public:
	class Event : public GameObject
	{
	public:

		GameObject * obj;
		unsigned int gf;
		unsigned int gf_length;
		unsigned int id;

	public:

		Event(GameObject * const  obj, const unsigned int gf,const unsigned int gf_length,const unsigned int id)
			: obj(obj), gf(gf), gf_length(gf_length), id(id) {}

		Event(SerializedGameData * sgd, const unsigned obj_id);

		void Destroy(void);

		/// Serialisierungsfunktionen
		protected: void Serialize_Event(SerializedGameData * sgd) const;
		public: void Serialize(SerializedGameData *sgd) const { Serialize_Event(sgd); }

		GO_Type GetGOT() const { return GOT_EVENT; }

		// Vergleichsoperatur f�r chronologisches Einf�gen nach Ziel-GF
		bool operator <= (const Event& other) const { return gf+gf_length<=other.gf+other.gf_length; }
	};
	typedef Event * EventPointer;

public:


	~EventManager();


	/// f�hrt alle Events des aktuellen GameFrames aus.
	void NextGF();
	/// f�gt ein Event der Eventliste hinzu.
	EventPointer AddEvent(GameObject *obj, const unsigned int gf_length, const unsigned int id = 0);
	/// Deserialisiert ein Event und f�gt es hinzu
	EventPointer AddEvent(SerializedGameData * sgd, const unsigned obj_id);
	/// F�gt ein schon angebrochenes Event hinzu (Events, wenn jemand beim Laufen stehengeblieben ist z.B.)
	/// Ein altes Event wird also quasi fortgef�hrt (um gf_elapsed in der Vergangenheit angelegt)
	EventPointer AddEvent(GameObject *obj, const unsigned int gf_length, const unsigned int id, const unsigned gf_elapsed);

	/// L�scht alle Listen f�r Spielende
	void Clear() { eis.clear(); kill_list.clear(); }
	/// Event entfernen
	void RemoveEvent(EventPointer ep) { eis.erase(ep); delete ep; }
	/// Objekt will gekillt werden
	void AddToKillList(GameObject *obj) { kill_list.push_back(obj); }

	/// Serialisieren
	void Serialize(SerializedGameData *sgd) const;
	/// Deserialisieren
	void Deserialize(SerializedGameData *sgd);
	
	/// Ist ein Event mit bestimmter id f�r ein bestimmtes Objekt bereits vorhanden?
	bool IsEventAcive(const GameObject * const obj, const unsigned id) const;

private:
	list<Event*> eis;     ///< Liste der Events f�r die einzelnen Objekte
	list<GameObject*> kill_list; ///< Liste mit Objekten die unmittelbar nach NextGF gekillt werden sollen
};

#define EVENTMANAGER EventManager::inst()

#endif // !EVENTMANAGER_H_INCLUDED
