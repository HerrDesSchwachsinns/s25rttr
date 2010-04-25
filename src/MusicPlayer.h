// $Id: MusicPlayer.h 6352 2010-04-25 12:59:33Z OLiver $
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
#ifndef MUSICPLAYER_H_INCLUDED
#define MUSICPLAYER_H_INCLUDED

#include "Singleton.h"

#pragma once

class iwMusicPlayer;

/// Speichert die Daten �ber eine Playlist und verwaltet diese
class Playlist
{
public:
	Playlist();

	/// bereitet die Playlist aufs abspielen vor.
	void Prepare();

	/// liefert den Dateinamen des akteullen Songs
	const std::string getCurrentSong() const	{	return (songs.size() && order.size() ? songs[order[0]] : "");	}
	
	/// schaltet einen Song weiter und liefert den Dateinamen des aktuellen Songs
	const std::string getNextSong();

	/// Playlist in Datei speichern
	bool SaveAs(const std::string filename, const bool overwrite);
	/// Playlist laden
	bool Load(const std::string filename);

	/// F�llt das iwMusicPlayer-Fenster mit den entsprechenden Werten
	void FillMusicPlayer(iwMusicPlayer *window) const;
	/// Liest die Werte aus dem iwMusicPlayer-Fenster
	void ReadMusicPlayer(const iwMusicPlayer *const window);
	
	/// W�hlt den Start-Song aus
	void SetStartSong(const unsigned id);

protected:
	unsigned int repeats;				///< Anzahl der Wiederholungen
	bool random;						///< Zufallswiedergabe?
	std::vector<std::string> songs;		///< Dateinamen der abzuspielenden Titel
	std::vector<unsigned int> order;	///< Reihenfolge der Titel
};

/// Globaler Musikplayer bzw. eine abspielbare Playlist
class MusicPlayer : public Singleton<MusicPlayer>
{
public:
	MusicPlayer();

	/// Startet Abspielvorgang
	void Play();
	/// Stoppt Abspielvorgang
	void Stop(); 

	/// Playlist laden
	bool Load(const std::string filename) { return list.Load(filename); }
	/// Musik wurde fertiggespielt (Callback)
	void MusicFinished()	{	PlayNext();	}
	/// liefert die Playlist.
	Playlist& GetPlaylist() { return list;}

protected:
	/// Spielt n�chstes St�ck ab
	void PlayNext();

private:
	bool playing;					///< L�uft die Musik gerade?
	Playlist list;					///< Unsere aktuell aktive Playlist
	libsiedler2::ArchivInfo sng;	///< externes benutzerdefiniertes Musikst�ck (z.B. andere mp3)
};

#endif // !MUSICPLAYER_H_INCLUDED
