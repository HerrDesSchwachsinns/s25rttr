// $Id: Loader.h 5923 2010-01-23 16:56:39Z FloSoft $
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
#ifndef LOADER_H_INCLUDED
#define LOADER_H_INCLUDED

#pragma once

#include "Singleton.h"
#include "libsiedler2.h"

/// Loader Klasse.
class Loader : public Singleton<Loader>
{
public:
	/// Konstruktor von @p Loader.
	Loader(void);
	/// Desktruktor von @p Loader.
	~Loader(void);

	/// L�dt alle allgemeinen Dateien.
	bool LoadFilesAtStart(void);
	/// L�dt die Spieldateien.
	bool LoadFilesAtGame(unsigned char gfxset, bool *nations);
	/// L�dt das Terrain.
	bool CreateTerrainTextures(void);

	/// L�dt die Settings.
	bool LoadSettings();
	/// Speichert die Settings.
	bool SaveSettings();

protected:
	/// L�dt alle Sounds.
	inline bool LoadSounds();

private:
	bool LoadFile(const char *pfad, const libsiedler2::ArchivItem_Palette *palette = NULL, bool load_always = true);
	bool LoadFile(const char *pfad, const libsiedler2::ArchivItem_Palette *palette, libsiedler2::ArchivInfo *archiv);
	void ExtractTexture(libsiedler2::ArchivInfo *destination, Rect &rect);
	void ExtractAnimatedTexture(libsiedler2::ArchivInfo *destination, Rect &rect, unsigned char color_count, unsigned char start_index);

	bool LoadFilesFromArray(const unsigned int files_count, const unsigned int *files, bool load_always = true);
	bool LoadLsts(unsigned int dir);

	static bool SortFilesHelper(const std::string& lhs, const std::string& rhs);

public:
	inline glArchivItem_Bitmap *GetImageN(std::string file, unsigned int nr) { return dynamic_cast<glArchivItem_Bitmap*>( files[file].get(nr) ); }
	inline glArchivItem_Font *GetFontN(std::string file, unsigned int nr) { return dynamic_cast<glArchivItem_Font*>( files[file].get(nr) ); }
	inline libsiedler2::ArchivItem_Palette *GetPaletteN(std::string file, unsigned int nr = 0) { return dynamic_cast<libsiedler2::ArchivItem_Palette*>( files[file].get(nr) ); }
	inline glArchivItem_Sound *GetSoundN(std::string file, unsigned int nr) { return dynamic_cast<glArchivItem_Sound*>( files[file].get(nr) ); }
	inline const char *GetTextN(std::string file, unsigned int nr) { return dynamic_cast<libsiedler2::ArchivItem_Text*>( files[file].get(nr) ) ? dynamic_cast<libsiedler2::ArchivItem_Text*>( files[file].get(nr) )->getText() : "text missing"; }
	inline libsiedler2::ArchivInfo *GetInfoN(std::string file) { return dynamic_cast<libsiedler2::ArchivInfo*>( &files[file] ); }
	inline glArchivItem_Bob *GetBobN(std::string file) { return dynamic_cast<glArchivItem_Bob*>( files[file].get(0) ); };
	inline glArchivItem_Bitmap *GetNationImageN(unsigned int nation, unsigned int nr) { return GetImageN(NATION_GFXSET_Z[lastgfx][nation], nr); }
	inline glArchivItem_Bitmap *GetMapImageN(unsigned int nr) { return GetImageN(MAP_GFXSET_Z[lastgfx], nr); }
	inline glArchivItem_Bitmap *GetTexImageN(unsigned int nr) { return GetImageN(TEX_GFXSET[lastgfx], nr); }
	inline libsiedler2::ArchivItem_Palette *GetTexPaletteN(unsigned int nr) { return GetPaletteN(TEX_GFXSET[lastgfx], nr); }
	inline libsiedler2::ArchivItem_Ini *GetSettingsIniN(std::string name) { return static_cast<libsiedler2::ArchivItem_Ini*>( GetInfoN(CONFIG_NAME)->find(name.c_str()) ); }

	// should not use this!
	const std::map<std::string, libsiedler2::ArchivInfo> &GetFiles(void) const { return files; }

private:
	std::map<std::string, libsiedler2::ArchivInfo> files;
	unsigned char lastgfx;

public:
	libsiedler2::ArchivInfo sng_lst;

	libsiedler2::ArchivInfo textures;
	libsiedler2::ArchivInfo water;
	libsiedler2::ArchivInfo lava;
	libsiedler2::ArchivInfo borders;
	libsiedler2::ArchivInfo roads;
	libsiedler2::ArchivInfo roads_points;
};

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#define LOADER Loader::inst()

#endif // LOADER_H_INCLUDED
