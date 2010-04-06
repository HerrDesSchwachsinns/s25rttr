// $Id: noStaticObject.h 6268 2010-04-05 09:59:22Z OLiver $
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
#ifndef NOSTATICOBJECT_H_INCLUDED
#define NOSTATICOBJECT_H_INCLUDED

#pragma once

#include "noCoordBase.h"

class noStaticObject : public noCoordBase
{
public:
	noStaticObject(unsigned short x, unsigned short y, unsigned short id, unsigned short file = 0xFFFF, unsigned char size = 0, NodalObjectType type = NOP_OBJECT);
	noStaticObject(SerializedGameData * sgd, const unsigned obj_id);

	void Destroy() { Destroy_noStaticObject(); }

	/// gibt die Item-ID zur�ck (nr in der jeweiligen File)
	unsigned short GetItemID() const { return id; }
	/// gibt die Nr der File zur�ck)
	unsigned short GetItemFile() const { return file; }
	/// gibt die Gr��e des Objekts zur�ck.
	unsigned char GetSize(void) const { return size; }

	virtual BlockingManner GetBM() const { return BlockingManner(unsigned(BM_HUT) + size); }

	/// zeichnet das Objekt.
	void Draw(int x, int y);

	/// Serialisierungsfunktionen
	protected:	void Serialize_noStaticObject(SerializedGameData * sgd) const;
	public:		void Serialize(SerializedGameData *sgd) const { Serialize_noStaticObject(sgd); }

	GO_Type GetGOT() const { return GOT_STATICOBJECT; }

protected:
	void Destroy_noStaticObject();

protected:

	unsigned short id;
	unsigned short file;
	unsigned char size;
};

#endif // !NOSTATICOBJECT_H_INCLUDED
