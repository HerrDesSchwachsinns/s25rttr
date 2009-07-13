// $Id: LoadINI.cpp 5259 2009-07-13 15:53:31Z FloSoft $
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
 *  l�dt eine INI-File in ein ArchivInfo.
 *
 *  @param[in]  file    Dateiname der INI-File
 *  @param[out] items   ArchivInfo-Struktur, welche gef�llt wird
 *
 *  @return Null bei Erfolg, ein Wert ungleich Null bei Fehler
 *
 *  @author FloSoft
 */
int libsiedler2::loader::LoadINI(const char *file, ArchivInfo *items)
{
	FILE *ini;

	if(file == NULL || items == NULL)
		return 1;

	// Datei zum lesen �ffnen
	ini = fopen(file, "rb");

	// hat das geklappt?
	if(ini == NULL)
		return 2;

	while(!feof(ini))
	{
		ArchivItem_Ini item;

		if(item.load(ini) != 0)
			return 3;

		items->pushC(&item);
	}

	// Datei schliessen
	fclose(ini);

	return 0;
}
