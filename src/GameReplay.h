// $Id: GameReplay.h 6394 2010-05-03 19:53:33Z OLiver $
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
#ifndef GAMEREPLAY_H_INCLUDED
#define GAMEREPLAY_H_INCLUDED

#pragma once

#include "GameSavedFile.h"
#include "GameProtocol.h"
#include "MapConsts.h"

class Savegame;

/// Klasse f�r geladene bzw. zu speichernde Replays
class Replay : public SavedFile
{
public:
	/// Replay-Command-Art
	enum ReplayCommand
	{
		RC_REPLAYEND = 0,
		RC_CHAT,
		RC_GAME

	};

public:
	Replay();
	~Replay();

	/// R�umt auf, schlie�t datei
	void StopRecording();

	/// Replaydatei g�ltig?
	bool IsValid() const { return file.IsValid(); }

	/// Beginnt die Save-Datei und schreibt den Header
	bool WriteHeader(const std::string& filename);
	/// L�dt den Header
	bool LoadHeader(const std::string& filename, const bool load_extended_header);

	/// F�gt ein Chat-Kommando hinzu (schreibt)
	void AddChatCommand(const unsigned gf,const unsigned char player, const unsigned char dest, const std::string& str);
	/// F�gt ein Spiel-Kommando hinzu (schreibt)
	void AddGameCommand(const unsigned gf,const unsigned short length, const unsigned char * const data);
	/// F�gt Pathfinding-Result hinzu
	void AddPathfindingResult(const unsigned char data, const unsigned * const length, const Point<MapCoord> * const next_harbor);

	/// Liest RC-Type aus, liefert false, wenn das Replay zu Ende ist
	bool ReadGF(unsigned * gf);
	/// RC-Type aus, liefert false
	ReplayCommand ReadRCType();
	/// Liest ein Chat-Command aus
	void ReadChatCommand(unsigned char * player, unsigned char  * dest, std::string& str);
	void ReadGameCommand(unsigned short *length, unsigned char ** data);
	void ReadPathfindingResult(unsigned char * data, unsigned * length, Point<MapCoord> * next_harbor);

	/// Aktualisiert den End-GF, schreibt ihn in die Replaydatei (nur beim Spielen bzw. Schreiben verwenden!)
	void UpdateLastGF(const unsigned last_gf);

public:
	/// NWF-L�nge
	unsigned short nwf_length;
	/// Zufallsgeneratorinitialisierung
	unsigned random_init;
	/// Bestimmt, ob Pathfinding-Ergebnisse in diesem Replay gespeichert sind
	bool pathfinding_results;

	/// Gespeichertes Spiel, Zufallskarte, normale Karte...?
	MapType map_type;
	/// Gepackte Map - Daten (f�r alte Karte)
	unsigned map_length, map_zip_length;
	unsigned char *map_data;
	/// Savegame (f�r gespeichertes Spiel)
	Savegame *savegame;

	/// End-GF
	unsigned last_gf;
	/// Position des End-GF in der Datei
	unsigned last_gf_file_pos;
	/// Position des GFs f�rs n�chste Command -> muss gleich hinter
	/// bestehendes beschrieben werden
	unsigned gf_file_pos;

private:
	/// Replayformat-Version und Signaturen
	static const unsigned short REPLAY_VERSION;
	static const char REPLAY_SIGNATURE[6];

	/// Dateihandle
	BinaryFile file;
};

#endif //!GAMEREPLAY_H_INCLUDED
