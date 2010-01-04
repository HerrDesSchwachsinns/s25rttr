// $Id: XMIDI_Track.h 5853 2010-01-04 16:14:16Z FloSoft $
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
#ifndef XMIDI_TRACK_H_INCLUDED
#define XMIDI_TRACK_H_INCLUDED

#pragma once

#include "MIDI_Track.h"

class XMIDI_Track
{
private:
	struct MIDI_Event {
		int            time;
		unsigned char  status;

		unsigned char  data[2];

		unsigned int   len;
		unsigned char  *buffer;
		int            duration;
		MIDI_Event     *next_note;
		unsigned int   note_time; 
		MIDI_Event     *next;
	};

	struct first_state {		// Status,   Data[0]
		MIDI_Event *patch[16];  // 0xC
		MIDI_Event *bank[16];   // 0xB,      0
		MIDI_Event *pan[16];    // 0xB,      7
		MIDI_Event *vol[16];    // 0xB,      10
	};

public:
	XMIDI_Track(MIDI_Track *track);
	~XMIDI_Track(void);

	int Convert();

private:
	int ConvertTrackToList();
	unsigned int ConvertListToMTrk();

	void ApplyFirstState(first_state &fs, int chan_mask);

	int ConvertNote(const int time, const unsigned char status, const int size);
	int ConvertEvent(const int time, const unsigned char status, const int size, first_state &fs);
	int ConvertSystemMessage(const int time, const unsigned char status);

	int GetVLQ(unsigned int &quant);
	int GetVLQ2(unsigned int &quant);
	int PutVLQ(unsigned int value, bool write, unsigned int pos = 0);

	void CreateNewEvent(int time);

private:
	MIDI_Track *track;
	MIDI_Event *events;
	unsigned int event_count;
	MIDI_Event *current;
	unsigned int position;

	enum {
		MIDI_STATUS_NOTE_OFF    = 0x8,
		MIDI_STATUS_NOTE_ON     = 0x9,
		MIDI_STATUS_AFTERTOUCH  = 0xA,
		MIDI_STATUS_CONTROLLER  = 0xB,
		MIDI_STATUS_PROG_CHANGE = 0xC,
		MIDI_STATUS_PRESSURE    = 0xD,
		MIDI_STATUS_PITCH_WHEEL = 0xE,
		MIDI_STATUS_SYSEX       = 0xF
	};

	bool bank127[16];
};

#endif // XMIDIFILE_H_
