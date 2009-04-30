// $Id: GameConsts.h 4746 2009-04-30 20:10:46Z OLiver $
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

#ifndef GAMECONSTS_H_
#define GAMECONSTS_H_

/// Maximale Anzahl an Spielern 
const unsigned MAX_PLAYERS = 8;

/// V�lker
enum Nation
{
	NAT_AFRICANS = 0,
	NAT_JAPANESES,
	NAT_ROMANS,
	NAT_VIKINGS,
	NAT_INVALID = 0xFFFFFFFF
};

// Bauqualit�ten
enum BuildingQuality
{
	BQ_NOTHING =0,
	BQ_FLAG,
	BQ_HUT,
	BQ_HOUSE,
	BQ_CASTLE,
	BQ_MINE,
	BQ_HARBOUR,
	BQ_DANGER =	255
};

const unsigned char TERRAIN_BQ[] =
{
	BQ_DANGER,
	BQ_FLAG,
	BQ_NOTHING,
	BQ_CASTLE,
	BQ_MINE,
	BQ_MINE,
	BQ_MINE,
	BQ_MINE,
	BQ_CASTLE,
	BQ_CASTLE,
	BQ_CASTLE,
	BQ_CASTLE,
	BQ_CASTLE,
	BQ_CASTLE,
	BQ_NOTHING,
	BQ_DANGER
};



struct RoadWindowInfo
{
	bool flag;
	int mx,my;
};

// Warentypen
enum GoodType
{
	/*	0 */GD_BEER = 0,	// Bier
	/*	1 */GD_TONGS,		// Zange
	/*	2 */GD_HAMMER,		// Hammer
	/*	3 */GD_AXE,			// Axt
	/*	4 */GD_SAW,			// S�ge
	/*	5 */GD_PICKAXE,		// Spitzhacke
	/*	6 */GD_SHOVEL,		// Schaufel
	/*	7 */GD_CRUCIBLE,	// Schmelztiegel
	/*	8 */GD_RODANDLINE,	// Angel
	/*	9 */GD_SCYTHE,		// Sense
	/* 10 */GD_WATEREMPTY,	// Wasser
	/* 11 */GD_WATER,		// Wasser
	/* 12 */GD_CLEAVER,		// Beil
	/* 13 */GD_ROLLINGPIN,	// Nudelholz
	/* 14 */GD_BOW,			// Bogen
	/* 15 */GD_BOAT,		// Boot
	/* 16 */GD_SWORD,		// Schwert
	/* 17 */GD_IRON,		// Eisen
	/* 18 */GD_FLOUR,		// Mehl
	/* 19 */GD_FISH,		// Fisch
	/* 20 */GD_BREAD,		// Brot
	/* 21 */GD_SHIELDROMANS,	// Schild
	/* 22 */GD_WOOD,		// Holz
	/* 23 */GD_BOARDS,		// Bretter
	/* 24 */GD_STONES,		// Steine
	/* 25 */GD_SHIELDVIKINGS,	// Schild
	/* 26 */GD_SHIELDAFRICANS,	// Schild
	/* 27 */GD_GRAIN,		// Getreide
	/* 28 */GD_COINS,		// Mnzen
	/* 29 */GD_GOLD,		// Gold
	/* 30 */GD_IRONORE,		// Eisenerz
	/* 31 */GD_COAL,		// Kohle
	/* 32 */GD_MEAT,		// Fleisch
	/* 33 */GD_HAM,			// Schinken ( Schwein )
	/* 34 */GD_SHIELDJAPANESE,	// Schild
	/* 35 */GD_NOTHING		// Nichts
};
// Anzahl an unterschiedlichen Warentypen
const unsigned WARE_TYPES_COUNT = GD_NOTHING;

const std::string WARE_NAMES[36] = 
{
	/*	0 */gettext_noop("Beer"),		// Bier
	/*	1 */gettext_noop("Tongs"),		// Zange
	/*	2 */gettext_noop("Hammer"),		// Hammer
	/*	3 */gettext_noop("Axe"),			// Axt
	/*	4 */gettext_noop("Saw"),			// S�ge
	/*	5 */gettext_noop("Pick-axe"),		// Spitzhacke
	/*	6 */gettext_noop("Shovel"),		// Schaufel
	/*	7 */gettext_noop("Crucible"),	// Schmelztiegel
	/*	8 */gettext_noop("Rod and line"),	// Angel
	/*	9 */gettext_noop("Scythe"),		// Sense
	/* 10 */gettext_noop("Water"),	// Wasser
	/* 11 */gettext_noop("Water"),		// Wasser
	/* 12 */gettext_noop("Cleaver"),		// Beil
	/* 13 */gettext_noop("Rolling pin"),		// Nudelholz
	/* 14 */gettext_noop("Bow"),			// Bogen
	/* 15 */gettext_noop("Boat"),		// Boot
	/* 16 */gettext_noop("Sword"),		// Schwert
	/* 17 */gettext_noop("Iron"),		// Eisen
	/* 18 */gettext_noop("Flour"),		// Mehl
	/* 19 */gettext_noop("Fish"),		// Fisch
	/* 20 */gettext_noop("Bread"),		// Brot
	/* 21 */gettext_noop("Shield"),		// Schild
	/* 22 */gettext_noop("Wood"),		// Holz
	/* 23 */gettext_noop("Boards"),		// Bretter
	/* 24 */gettext_noop("Stones"),		// Steine
	/* 25 */"",	// Schild
	/* 26 */"",	// Schild
	/* 27 */gettext_noop("Grain"),		// Getreide
	/* 28 */gettext_noop("Coins"),		// Mnzen
	/* 29 */gettext_noop("Gold"),		// Gold
	/* 30 */gettext_noop("Iron ore"),	// Eisenerz
	/* 31 */gettext_noop("Coal"),		// Kohle
	/* 32 */gettext_noop("Meat"),		// Fleisch
	/* 33 */gettext_noop("Ham"),		// Schinken ( Schwein )
	/* 34 */"",							// Schild
	/* 35 */""
};



enum BuildingType
{
	BLD_HEADQUARTERS   =  0, // ----
	BLD_BARRACKS       =  1, // NJR
	BLD_GUARDHOUSE     =  2, // NJR
	BLD_NOTHING2       =  3, // ----
	BLD_WATCHTOWER     =  4, // NJ
	BLD_NOTHING3       =  5, // ----
	BLD_NOTHING4       =  6, // ----
	BLD_NOTHING5       =  7, // ----
	BLD_NOTHING6       =  8, // ----
	BLD_FORTRESS       =  9, // NJR
	BLD_GRANITEMINE    = 10, // NJR
	BLD_COALMINE       = 11, // NJR
	BLD_IRONMINE       = 12, // NJR
	BLD_GOLDMINE       = 13, // NJR
	BLD_LOOKOUTTOWER   = 14, //
	BLD_NOTHING7       = 15, // ----
	BLD_CATAPULT       = 16, //
	BLD_WOODCUTTER     = 17, // NJR
	BLD_FISHERY        = 18, // N
	BLD_QUARRY         = 19, // NJR
	BLD_FORESTER       = 20, // NJR
	BLD_SLAUGHTERHOUSE = 21, // NJR
	BLD_HUNTER         = 22, // NJ
	BLD_BREWERY        = 23, // NJR
	BLD_ARMORY         = 24, // NJR
	BLD_METALWORKS     = 25, // NJR
	BLD_IRONSMELTER    = 26, // NJR
	BLD_NOTHING8       = 27, // ----
	BLD_PIGFARM        = 28, // NJR
	BLD_STOREHOUSE     = 29, //
	BLD_NOTHING9       = 30, // ----
	BLD_MILL           = 31, // NJR
	BLD_BAKERY         = 32, // NJR
	BLD_SAWMILL        = 33, // NJR
	BLD_MINT           = 34, // NJR
	BLD_WELL           = 35, // NJR
	BLD_SHIPYARD       = 36, //
	BLD_FARM           = 37, // NJR
	BLD_DONKEYBREEDER  = 38, //
	BLD_HARBORBUILDING = 39,  //
	BLD_NOTHING
};

// Anzahl an unterschiedlichen Geb�udetypen
const unsigned BUILDING_TYPES_COUNT = BLD_NOTHING;

const std::string BUILDING_NAMES[40] = 
{
	gettext_noop("Headquarters"), 
	gettext_noop("Barracks"),
	gettext_noop("Guardhouse"),
	"",
	gettext_noop("Watchtower"),
	"",
	"",
	"",
	"",
	gettext_noop("Fortress"),
	gettext_noop("Granite mine"),
	gettext_noop("Coal mine"),
	gettext_noop("Iron mine"),
	gettext_noop("Gold mine"),
	gettext_noop("Lookout tower"),
	"",
	gettext_noop("Catapult"),
	gettext_noop("Woodcutter"),
	gettext_noop("Fishery"),
	gettext_noop("Quarry"),
	gettext_noop("Forester"),
	gettext_noop("Slaughterhouse"),
	gettext_noop("Hunter"),
	gettext_noop("Brewery"),
	gettext_noop("Armory"),
	gettext_noop("Metalworks"),
	gettext_noop("Iron smelter"),
	"",
	gettext_noop("Pig farm"),
	gettext_noop("Storehouse"),
	"",
	gettext_noop("Mill"),
	gettext_noop("Bakery"),
	gettext_noop("Sawmill"),
	gettext_noop("Mint"),
	gettext_noop("Well"),
	gettext_noop("Shipyard"),
	gettext_noop("Farm"),
	gettext_noop("Donkey breeding"),
	gettext_noop("Harbor building"),
};



enum Job
{
	JOB_HELPER            =  0,
	JOB_WOODCUTTER        =  1,
	JOB_FISHER            =  2,
	JOB_FORESTER          =  3,
	JOB_CARPENTER         =  4,
	JOB_STONEMASON        =  5,
	JOB_HUNTER            =  6,
	JOB_FARMER            =  7,
	JOB_MILLER            =  8,
	JOB_BAKER             =  9,
	JOB_BUTCHER           = 10,
	JOB_MINER             = 11,
	JOB_BREWER            = 12,
	JOB_PIGBREEDER        = 13,
	JOB_DONKEYBREEDER     = 14,
	JOB_IRONFOUNDER       = 15,
	JOB_MINTER            = 16,
	JOB_METALWORKER       = 17,
	JOB_ARMORER           = 18,
	JOB_BUILDER           = 19,
	JOB_PLANER            = 20,
	JOB_PRIVATE           = 21,
	JOB_PRIVATEFIRSTCLASS = 22,
	JOB_SERGEANT          = 23,
	JOB_OFFICER           = 24,
	JOB_GENERAL           = 25,
	JOB_GEOLOGIST         = 26,
	JOB_SHIPWRIGHT        = 27,
	JOB_SCOUT             = 28,
	JOB_PACKDONKEY        = 29,
	JOB_BOATCARRIER		  = 30,
	JOB_NOTHING           = 31
};

// Anzahl an unterschiedlichen Berufstypen
const unsigned JOB_TYPES_COUNT = JOB_NOTHING;

const std::string JOB_NAMES[32] = 
{
	gettext_noop("Helper"),
	gettext_noop("Woodchopper"),
	gettext_noop("Fisher"),
	gettext_noop("Ranger"),
	gettext_noop("Carpenter"),
	gettext_noop("Stonemason"),
	gettext_noop("Huntsman"),
	gettext_noop("Farmer"),
	gettext_noop("Miller"),
	gettext_noop("Baker"),
	gettext_noop("Butcher"),
	gettext_noop("Miner"),
	gettext_noop("Brewer"),
	gettext_noop("Pig breeder"),
	gettext_noop("Donkey breeder"),
	gettext_noop("Iron founder"),
	gettext_noop("Minter"),
	gettext_noop("Metalworker"),
	gettext_noop("Armorer"),
	gettext_noop("Builder"),
	gettext_noop("Planer"),
	gettext_noop("Private"),
	gettext_noop("Private first class"),
	gettext_noop("Sergeant"),
	gettext_noop("Officer"),
	gettext_noop("General"),
	gettext_noop("Geologist"),
	gettext_noop("Shipwright"),
	gettext_noop("Scout"),
	gettext_noop("Pack donkey"),
	"",
	""
};



/// Waren- und Berufsstruktur ( f�r HQs, Lagerh��ser usw )
struct Goods
{
	unsigned goods[WARE_TYPES_COUNT];
	unsigned people[JOB_TYPES_COUNT];

	Goods() { memset(goods,0,sizeof(goods)); memset(people,0,sizeof(people)); }
};


/// Konvertierungstabelle von RttR-Nation-Indizes in Original-S2-Nation-Indizes
const unsigned char NATION_RTTR_TO_S2[4] =
{
	3,
	2,
	0,
	1
};
/// Konvertierungstabelle von Rohstoff-Indizes von den Bergwerken --> Map
const unsigned char RESOUCRES_MINE_TO_MAP[4] = {3,0,1,2};

/// Geschwindigkeitsabstufungen - L�ngen der GFs in ms
const unsigned SPEED_GF_LENGTHS[5] = {80,60,50,40,30};

/// Macht ggf. aus den verschiedenen Schilden der Nationen jeweils immer das r�mische normale Schild f�r
/// die Warensysteme usw
inline GoodType ConvertShields(const GoodType good) { return (good == GD_SHIELDVIKINGS ||
											   good == GD_SHIELDAFRICANS ||
											   good == GD_SHIELDJAPANESE) ? GD_SHIELDROMANS : good; }

/// Reichweite der Bergarbeiter
const unsigned MINER_RADIUS = 2;


#endif
