// $Id: SerializedGameData.h 6458 2010-05-31 11:38:51Z FloSoft $
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
#ifndef SERIALIZED_GAME_DATA_H_
#define SERIALIZED_GAME_DATA_H_

#pragma once

#include <memory.h>
#include <list>
#include <vector>
#include "list.h"
#include "BinaryFile.h"
#include "GameObject.h"
#include "FOWObjects.h"
#include "Serializer.h"

class noBase;
class GameObject;
class GameWorld;


/// K�mmert sich um das Serialisieren der GameDaten f�rs Speichern und Resynchronisieren
class SerializedGameData : public Serializer
{
	/// Objektreferenzen
	union
	{
		const GameObject ** objects_write;
		GameObject ** objects_read;
	};
	/// Voraussichtliche Gesamtanzahl an Objekten (nur beim Laden)
	unsigned total_objects_count;
	/// Aktuelle Anzahl an Objekten
	unsigned objects_count;

	EventManager *em;

private:

	/// Objekt(referenzen) lesen
	GameObject * PopObject_(GO_Type got); 

public:

	SerializedGameData();

	/// Nimmt das gesamte Spiel auf und speichert es im Buffer
	void MakeSnapshot(GameWorld *const gw, EventManager *const em);
	/// Schreibt den Buffer in eine Datei
	void WriteToFile(BinaryFile& file);
	/// Liest den Buffer aus einer Datei
	void ReadFromFile(BinaryFile& file);

	void PrepareDeserialization(EventManager * const em) { this->em = em; }

	/// Erzeugt GameObject
	GameObject * Create_GameObject(const GO_Type got, const unsigned obj_id);
	/// Erzeugt FOWObject
	FOWObject * Create_FOWObject(const FOW_Type fowtype);

	/// Kopiermethoden

	/// Objekt(referenzen) kopieren
	void PushObject(const GameObject * go,const bool known); 

	/// Kopiert eine Liste von GameObjects
	template <typename T>
	void PushObjectList(const list<T*>& gos,const bool known)
	{
		// Anzahl
		PushUnsignedInt(gos.size());
		// einzelne Objekte
		for(typename list<T*>::const_iterator it = gos.begin(); it.valid(); ++it)
			PushObject(*it,known);
	}

	/// Kopiert eine Liste von GameObjects
	template <typename T>
	void PushObjectList(const std::list<T*>& gos,const bool known)
	{
		// Anzahl
		PushUnsignedInt(gos.size());
		// einzelne Objekte
		for(typename std::list<T*>::const_iterator it = gos.begin(); it!=gos.end(); ++it)
			PushObject(*it,known);
	}

	/// Kopiert eine Liste von GameObjects
	template <typename T>
	void PushObjectVector(const std::vector<T*>& gos,const bool known)
	{
		// Anzahl
		PushUnsignedInt(gos.size());
		// einzelne Objekte
		for(unsigned i = 0;i<gos.size();++i)
			PushObject(gos[i],known);
	}

	/// FoW-Objekt
	void PushFOWObject(const FOWObject * fowobj);


	// Lesemethoden

	/// Objekt(referenzen) lesen
	template <typename T>
	T * PopObject(GO_Type got) { return static_cast<T*>(PopObject_(got)); } 

	/// FoW-Objekt
	FOWObject * PopFOWObject();
	
	/// Liest eine Liste von GameObjects
	template <typename T>
	void PopObjectList(list<T*>& gos,GO_Type got)
	{
		// Anzahl
		unsigned size = PopUnsignedInt();
		// einzelne Objekte
		for(unsigned i = 0;i<size;++i)
			gos.push_back(PopObject<T>(got));
	}

	/// Liest eine Liste von GameObjects
	template <typename T>
	void PopObjectList(std::list<T*>& gos,GO_Type got)
	{
		// Anzahl
		unsigned size = PopUnsignedInt();
		// einzelne Objekte
		for(unsigned i = 0;i<size;++i)
			gos.push_back(PopObject<T>(got));
	}

	/// Liest einen Vektor von GameObjects
	template <typename T>
	void PopObjectVector(std::vector<T*>& gos,GO_Type got)
	{
		// Anzahl
		unsigned size = PopUnsignedInt();
		gos.resize(size);
		// einzelne Objekte
		for(unsigned i = 0;i<size;++i)
			gos[i] = PopObject<T>(got);
	}


	/// F�gt ein gelesenes Objekt zur globalen Objektliste dazu
	void AddObject(GameObject * go);


	/// Sucht ein Objekt, falls vorhanden
	const GameObject * GetConstGameObject(const unsigned obj_id) const;
	GameObject * GetGameObject(const unsigned obj_id) const;
};


#endif // !SERIALIZED_GAME_DATA_H_
