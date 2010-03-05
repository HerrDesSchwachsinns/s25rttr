// $Id: GameSavegame.cpp 6120 2010-03-05 23:42:17Z jh $
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
#include "GameSavegame.h"

/// Kleine Signatur am Anfang "RTTRSAVE", die ein g�ltiges S25 RTTR Savegame kennzeichnet
const char Savegame::SAVE_SIGNATURE[8] = {'R','T','T','R','S','A','V','E'};
/// Version des Savegame-Formates
const unsigned short Savegame::SAVE_VERSION = 21;

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *  @author OLiver
 */
Savegame::Savegame() : SavedFile(), start_gf(0)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *  @author OLiver
 */
Savegame::~Savegame()
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *  @author OLiver
 */
bool Savegame::Save(const std::string& filename)
{
	BinaryFile file;
	
	if(!file.Open(filename.c_str(),OFM_WRITE))
		return false;

	bool ret = Save(file);

	file.Close();

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *  @author OLiver
 */
bool Savegame::Save(BinaryFile& file)
{
	// Versionszeug schreiben
	WriteVersion(file,8,SAVE_SIGNATURE,SAVE_VERSION);

	// Timestamp der Aufzeichnung (TODO: Little/Big Endian unterscheidung)
	file.WriteRawData(&save_time,8);

	// Mapname
	file.WriteShortString(map_name);

	// Anzahl Spieler
	file.WriteUnsignedChar(player_count);

	// Gr��e der Spielerdaten (sp�ter ausf�llen)
	unsigned players_size = 0;
	unsigned players_pos = file.Tell();
	file.WriteUnsignedInt(players_size);

	// Spielerdaten
	WritePlayerData(file);

	// Wieder zur�ckspringen und Gr��e des Spielerblocks eintragen
	unsigned new_pos = file.Tell();
	file.Seek(players_pos,SEEK_SET);
	file.WriteUnsignedInt(new_pos-players_pos-4);
	file.Seek(new_pos,SEEK_SET);

	// GGS
	WriteGGS(file);

	// Start-GF
	file.WriteUnsignedInt(start_gf);

	// Serialisiertes Spielzeug reinschreiben
	sgd.WriteToFile(file);

	// Addoneinstellungen
	WriteAddonSettings(file);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *  @author OLiver
 */
bool Savegame::Load(const std::string&  filename,const bool load_players,const bool load_sgd)
{
	BinaryFile file;
	
	if(!file.Open(filename.c_str(),OFM_READ))
		return false;

	bool ret = Load(file,load_players,load_sgd);

	file.Close();

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *  @author OLiver
 */
bool Savegame::Load(BinaryFile& file,const bool load_players,const bool load_sgd)
{
	// Signatur und Version einlesen
	if(!ValidateFile(file,8,SAVE_SIGNATURE,SAVE_VERSION))
	{
		LOG.lprintf("Savegame::Load: ERROR: File is not a valid RTTR savegame!\n");
		return false;
	}

	// Zeitstempel
	file.ReadRawData(&save_time,8);

	// Map-Name
	file.ReadShortString(map_name);

	// Anzahl Spieler
	player_count = file.ReadUnsignedChar();

	// Spielerzeug
	if(load_players)
	{
		// Gr��e des Spielerblocks �berspringen
		file.Seek(4,SEEK_CUR);

		ReadPlayerData(file);
	}
	else
	{
		// �berspringen
		players = 0;
		unsigned player_size = file.ReadUnsignedInt();
		file.Seek(player_size,SEEK_CUR);
	}

	// GGS
	ReadGGS(file);

	// Start-GF
	start_gf = file.ReadUnsignedInt();

	if(load_sgd)
	{
		// Serialisiertes Spielzeug lesen
		sgd.ReadFromFile(file);

		// Addoneinstellungen
		ReadAddonSettings(file);
	}

	return true;
}
