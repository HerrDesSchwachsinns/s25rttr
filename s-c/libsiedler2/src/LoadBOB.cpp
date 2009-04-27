// $Id: LoadBOB.cpp 4652 2009-03-29 10:10:02Z FloSoft $
//
// Copyright (c) 2005-2009 Settlers Freaks (sf-team at siedler25.org)
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

///////////////////////////////////////////////////////////////////////////////
// Header
#include "main.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/**
 *  l�dt eine BOB-File in ein ArchivInfo.
 *
 *  @param[in]  file    Dateiname der BOB-File
 *  @param[out] items   ArchivInfo-Struktur, welche gef�llt wird
 *
 *  @return Null bei Erfolg, ein Wert ungleich Null bei Fehler
 *
 *  @author FloSoft
 */
int libsiedler2::loader::LoadBOB(const char *file, const ArchivItem_Palette *palette, ArchivInfo *items)
{
	FILE *bob;
	unsigned int header;

	if(file == NULL || palette == NULL || items == NULL)
		return 1;

	// Datei zum lesen �ffnen
	bob = fopen(file, "rb");

	// hat das geklappt?
	if(bob == NULL)
		return 2;

	// Header einlesen
	if(libendian::be_read_ui(&header, bob) != 0)
		return 3;

	// ist es eine BOB-File? (Header 0xF601F501)
	if(header != 0xF601F501)
		return 4;

	ArchivItem_Bob *item = dynamic_cast<ArchivItem_Bob*>((*allocator)(BOBTYPE_BOB, 0, NULL));

	const char *name = strrchr(file, '/');
	if(name)
		item->setName(name+1);

	if(item->load(bob, palette) != 0)
		return 5;

	// Item alloziieren und zuweisen
	items->alloc(1);
	items->set(0, item);

	fclose(bob);

	return 0;
}
