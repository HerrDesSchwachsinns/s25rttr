// $Id: SDL.h 4652 2009-03-29 10:10:02Z FloSoft $
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
#ifndef SDL_H_INCLUDED
#define SDL_H_INCLUDED

#include <AudioDriver.h>

/// Klasse f�r den SDL Audiotreiber.
class AudioSDL : public AudioDriver
{
private:

	/// Welche Sounds werden in den Channels gerade gespielt?
	unsigned channels[CHANNEL_COUNT];
	/// Lautst�rke der Effekte.
	unsigned char master_effects_volume;
	/// Lautst�rke der Musik.
	unsigned char master_music_volume;

public:
	/// Konstruktor von @p AudioSDL.
	AudioSDL(AudioDriverLoaderInterface * adli);

	/// Destruktor von @p AudioSDL.
	~AudioSDL(void);

	/// Treiberinitialisierungsfunktion.
	bool Initialize(void);

	/// Treiberaufr�umfunktion.
	void CleanUp(void);

	Sound *LoadEffect(unsigned int data_type, unsigned char *data, unsigned long size);
	Sound *LoadMusic(unsigned int data_type, unsigned char *data, unsigned long size);

	/// Spielt Sound ab
	unsigned int PlayEffect(Sound *sound, const unsigned char volume, const bool loop);
	/// Spielt Midi ab
	void PlayMusic(Sound * sound, const unsigned repeats);
	/// Stoppt die Musik.
	void StopMusic(void);
	/// Wird der Sound (noch) abgespielt?
	bool IsEffectPlaying(const unsigned play_id);
	/// Stoppt einen Sound
	void StopEffect(const unsigned play_id); 
	/// Ver�ndert die Lautst�rke von einem abgespielten Sound (falls er noch abgespielt wird)
	void ChangeVolume(const unsigned play_id,const unsigned char volume);

	void SetMasterEffectVolume(unsigned char volume);
	void SetMasterMusicVolume(unsigned char volume);

private:

	/// Callback f�r Audiotreiber
	static void MusicFinished();
};

#endif // !SDL_H_INCLUDED
