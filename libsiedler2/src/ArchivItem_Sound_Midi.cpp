// $Id: ArchivItem_Sound_Midi.cpp 5853 2010-01-04 16:14:16Z FloSoft $
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

///////////////////////////////////////////////////////////////////////////////
// Header
#include "main.h"
#include "ArchivItem_Sound_Midi.h"


///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/** @class libsiedler2::baseArchivItem_Sound_Midi
 *
 *  Basisklasse f�r MIDI-Sounds.
 *
 *  @author FloSoft
 */

///////////////////////////////////////////////////////////////////////////////
/** 
 *  Konstruktor von @p baseArchivItem_Sound_Midi.
 *
 *  @author FloSoft
 */
libsiedler2::baseArchivItem_Sound_Midi::baseArchivItem_Sound_Midi(void) : baseArchivItem_Sound()
{
	setType(SOUNDTYPE_MIDI);

	tracks = 0;
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  Kopierkonstruktor von @p ArchivItem_Sound_Midi.
 *
 *  @param[in] item Quellitem
 *
 *  @author FloSoft
 */
libsiedler2::baseArchivItem_Sound_Midi::baseArchivItem_Sound_Midi(const baseArchivItem_Sound_Midi *item) : baseArchivItem_Sound( (baseArchivItem_Sound*)item )
{
	tracks = item->tracks;

	for(unsigned int i = 0; i < 256; ++i)
		tracklist[i].copy(&item->tracklist[i]);
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  Destruktor von @p ArchivItem_Sound_Midi.
 *
 *  @author FloSoft
 */
libsiedler2::baseArchivItem_Sound_Midi::~baseArchivItem_Sound_Midi(void)
{
}

int libsiedler2::baseArchivItem_Sound_Midi::load(FILE *file, unsigned int length)
{
	if(file == NULL || length == 0)
		return 1;

	unsigned int item_length = length;
	long position = ftell(file);

	char header[4];
	unsigned int chunk;
	unsigned short type = 0;
	unsigned short ppqs = 96;

	// Header einlesen
	if(libendian::le_read_c(header, 4, file) != 4)
		return 2;

	// ist es eine MIDI-File? (Header "MThd")
	if(strncmp(header, "MThd", 4) != 0)
		return 3;

	// L�nge einlesen
	if(libendian::be_read_ui(&length, file) != 0)
		return 4;

	// Typ einlesen
	if(libendian::be_read_us(&type, file) != 0)
		return 5;

	// Tracksanzahl einlesen
	if(libendian::be_read_us(&tracks, file) != 0)
		return 6;

	// PPQS einlesen
	if(libendian::be_read_us(&ppqs, file) != 0)
		return 7;

	if(tracks == 0 || tracks > 256)
		return 8;

	unsigned short track_nr = 0;
	while(track_nr < tracks)
	{
		// Chunk-Typ einlesen
		if(libendian::be_read_ui(&chunk, file) != 0)
			return 9;

		switch(chunk)
		{
		case 0x4D54726B: // "MTrk"
			{
				// L�nge einlesen
				if(libendian::be_read_ui(&length, file) != 0)
					return 10;

				fseek(file, -8, SEEK_CUR);
				length += 8;

				tracklist[track_nr].allocMid(length);
				if(tracklist[track_nr].readMid(file) != 0)
					return 11;

				++track_nr;
			} break;
		default:
			return 12;
		}
	}

	// auf jeden Fall kompletten Datensatz �berspringen
	fseek(file, position + item_length, SEEK_SET);
	return 0;
}

int libsiedler2::baseArchivItem_Sound_Midi::write(FILE *file) const
{
	if(!file)
		return 1;

	unsigned int length = 0;
	for(unsigned short i = 0; i < tracks; ++i)
		length += tracklist[i].getMidLength(false);

	// LST-L�nge schreiben
	if(libendian::le_write_ui(length + 14, file) != 0)
		return 2;

	// Header schreiben
	if(libendian::le_write_c("MThd", 4, file) != 4)
		return 3;

	// L�nge schreiben
	if(libendian::be_write_ui(length, file) != 0)
		return 4;

	// Typ schreiben
	if(libendian::be_write_us(0, file) != 0)
		return 5;

	// Tracksanzahl schreiben
	if(libendian::be_write_us(tracks, file) != 0)
		return 6;

	// PPQS schreiben
	if(libendian::be_write_us(96, file) != 0)
		return 7;

	for(unsigned short i = 0; i < tracks; ++i)
	{
		if(libendian::le_write_uc(tracklist[i].getMid(false), tracklist[i].getMidLength(false), file) != (int)tracklist[i].getMidLength(false))
			return 8;
	}

	return 0;
}
