// $Id: prototypen.h 6460 2010-05-31 11:42:38Z FloSoft $
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
#ifndef PROTOTYP_H_INCLUDED
#define PROTOTYP_H_INCLUDED

#pragma once

namespace libsiedler2
{
	/// Die verschiedenen Lade-/Schreibfunktionen der Dateien
	namespace loader
	{
		/// l�dt eine spezifizierten Bobtype aus einer Datei in ein ArchivItem.
		int LoadType(unsigned short bobtype, FILE *lst, const ArchivItem_Palette *palette, ArchivItem **item);

		/// schreibt eine spezifizierten Bobtype aus einem ArchivItem in eine Datei.
		int WriteType(unsigned short bobtype, FILE *lst, const ArchivItem_Palette *palette, const ArchivItem *item);

		/// l�dt eine LST-File in ein ArchivInfo.
		int LoadLST(const char *file, const ArchivItem_Palette *palette, ArchivInfo *items);

		/// schreibt ein ArchivInfo eine LST-File.
		int WriteLST(const char *file, const ArchivItem_Palette *palette, const ArchivInfo *items);

		/// l�dt eine TLST-File (beschriebenes LST-File in Textform) in ein ArchivInfo.
		int LoadTLST(const char *file, ArchivInfo *items);

		/// l�dt eine BBM-File in ein ArchivInfo.
		int LoadBBM(const char *file, ArchivInfo *items);

		/// schreibt ein ArchivInfo in eine BBM-File.
		int WriteBBM(const char *file, const ArchivInfo *items);

		/// l�dt eine ACT-File in ein ArchivInfo.
		int LoadACT(const char *file, ArchivInfo *items);

		/// schreibt ein ArchivInfo in eine ACT-File.
		int WriteACT(const char *file, const ArchivInfo *items, long nr = -1);

		/// l�dt eine DAT/IDX-File in ein ArchivInfo.
		int LoadDATIDX(const char *file, const ArchivItem_Palette *palette, ArchivInfo *items);

		/// schreibt ein ArchivInfo in eine DAT/IDX-File.
		int WriteDATIDX(const char *file, const ArchivItem_Palette *palette, const ArchivInfo *items);

		/// l�dt eine BMP-File in ein ArchivInfo.
		int LoadBMP(const char *file, ArchivItem **image, ArchivItem **palette = NULL);

		/// schreibt ein ArchivInfo in eine BMP-File.
		int WriteBMP(const char *file, const ArchivItem_Palette *palette, const ArchivInfo *items, long nr = -1);

		/// l�dt eine GER/ENG-File in ein ArchivInfo.
		int LoadTXT(const char *file, ArchivInfo *items, bool conversion = true);

#		define LoadGER LoadTXT
#		define LoadENG LoadTXT

		/// schreibt eine GER/ENG-File aus einem ArchivInfo.
		int WriteTXT(const char *file, const ArchivInfo *items, bool conversion = true);

#		define WriteGER WriteTXT
#		define WriteENG WriteTXT

		/// l�dt eine LBM-File in ein ArchivInfo.
		int LoadLBM(const char *file, ArchivInfo *items);

		/// schreibt ein ArchivInfo in eine LBM-File.
		int WriteLBM(const char *file, const ArchivInfo *items);

		/// l�dt eine SWD/WSD-File in ein ArchivInfo.
		int LoadMAP(const char *file, ArchivInfo *items, bool only_header = false);

#		define LoadSWD LoadMAP
#		define LoadWSD LoadMAP

		/// schreibt ein ArchivInfo in eine SWD/WSD-File.
		int WriteMAP(const char *file, const ArchivInfo *items, long nr = -1);

#		define WriteSWD WriteMAP
#		define WriteWSD WriteMAP

		/// l�dt eine BOB-File in ein ArchivInfo.
		int LoadBOB(const char *file, const ArchivItem_Palette *palette, ArchivInfo *items);

		/// schreibt ein ArchivInfo in eine BOB-File.
		int WriteBOB(const char *file, const ArchivItem_Palette *palette, const ArchivInfo *items);

		int LoadSND(const char *file, ArchivInfo *items);

#		define LoadMID LoadSND
#		define LoadXMID LoadSND
#		define LoadWAV LoadSND

		/// l�dt eine INI-File in ein ArchivInfo.
		int LoadINI(const char *file, ArchivInfo *items);
		int WriteINI(const char *file, const ArchivInfo *items);
	}
}

#endif // !PROTOTYP_H_INCLUDED
