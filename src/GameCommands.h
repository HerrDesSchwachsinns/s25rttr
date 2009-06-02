#ifndef GAME_COMMANDS_H_
#define GAME_COMMANDS_H_

#include "Serializer.h"
#include "GameConsts.h"
#include "MapConsts.h"
#include <vector>
#include <cassert>

class GameClient;
class GameServer;
class GameWorldGame;
class GameClientPlayer;

namespace gc
{

enum Type
{
	NOTSEND = 0,
	NOTHING,
	SETFLAG,
	DESTROYFLAG,
	BUILDROAD,
	DESTROYROAD,
	CHANGEDISTRIBUTION,
	CHANGEBUILDORDER,
	SETBUILDINGSITE,
	DESTROYBUILDING,
	CHANGETRANSPORT,
	CHANGEMILITARY,
	CHANGETOOLS,
	CALLGEOLOGIST,
	CALLSCOUT,
	ATTACK,
	SWITCHPLAYER,
	STOPGOLD,
	STOPPRODUCTION,
	CHANGEINVENTORYSETTING,
	CHANGEALLINVENTORYSETTINGS,
	CHANGERESERVE,
	SURRENDER,
	CHEAT_ARMAGEDDON,
	DESTROYALL
};


class GameCommand
{
	/// Typ dieses Command
	const Type gst;
public:

	/// Erzeugt GameCommand anhand von Typen
	static GameCommand * CreateGameCommand(const Type gst, Serializer * ser);

	GameCommand(const Type gst) : gst(gst) {}
	
	/// Gibt den entsprechenden Typen zur�ck
	Type GetType() const { return gst; }
	/// Serialisiert dieses GameCommand
	virtual void Serialize(Serializer * ser) const = 0;

	/// F�hrt das GameCommand aus
	virtual void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid) = 0;
};



/// Basisklasse f�r s�mtliche GameCommands mit Koordinaten
class Coords : public GameCommand
{
protected:
	/// Koordinaten auf der Map, die dieses Command betreffen
	const MapCoord x,y;
public:
	Coords(const Type gst, const MapCoord x, const MapCoord y) 
		: GameCommand(gst), x(x), y(y) {}
	Coords(const Type gst, Serializer * ser)
		: GameCommand(gst), x(ser->PopUnsignedShort()), y(ser->PopUnsignedShort()) {}

	virtual void Serialize(Serializer *ser) const
	{
		ser->PushUnsignedShort(x);
		ser->PushUnsignedShort(y);
	}

};

/// Flagge setzen
class SetFlag : public Coords
{
	friend class GameClient;
public:
	SetFlag(const MapCoord x, const MapCoord y)
		: Coords(SETFLAG,x,y) {}
	SetFlag(Serializer * ser)
		: Coords(SETFLAG,ser) {}

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};

/// Flagge zerst�ren
class DestroyFlag : public Coords
{
	friend class GameClient;
public:
	DestroyFlag(const MapCoord x, const MapCoord y)
		: Coords(DESTROYFLAG,x,y) {}
	DestroyFlag(Serializer * ser)
		: Coords(DESTROYFLAG,ser) {}

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};

/// Stra�e bauen
class BuildRoad : public Coords
{
	friend class GameClient;
	/// Boot-Stra�e oder nicht?
	const bool boat_road;
	/// Beschreibung der Stra�e mittels einem Array aus Richtungen
	std::vector<unsigned char> route;
public:
	BuildRoad(const MapCoord x, const MapCoord y, const bool boat_road, const std::vector<unsigned char>& route)
		: Coords(BUILDROAD,x,y), boat_road(boat_road), route(route) {}
	BuildRoad(Serializer * ser)
		: Coords(BUILDROAD,ser),
		boat_road(ser->PopBool()),
		route(ser->PopUnsignedInt())
	{
		for(unsigned i = 0;i<route.size();++i)
			route[i] = ser->PopUnsignedChar();
	}

	virtual void Serialize(Serializer *ser) const
	{
		Coords::Serialize(ser);

		ser->PushBool(boat_road);
		ser->PushUnsignedInt(route.size());
		for(unsigned i = 0;i<route.size();++i)
			ser->PushUnsignedChar(route[i]);
	}

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};

/// Stra�e zerst�ren
class DestroyRoad : public Coords
{
	friend class GameClient;
	/// Richtung in der von der Flagge an x;y aus gesehen die Stra�e zerst�rt werden soll
	const unsigned char start_dir;
public:
	DestroyRoad(const MapCoord x, const MapCoord y, const unsigned char start_dir)
		: Coords(DESTROYROAD,x,y), start_dir(start_dir) {}
	DestroyRoad(Serializer * ser)
		: Coords(DESTROYROAD,ser),
		start_dir(ser->PopUnsignedChar()) {}

	virtual void Serialize(Serializer *ser) const
	{
		Coords::Serialize(ser);

		ser->PushUnsignedChar(start_dir);
	}

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};

/// Warenverteilung �ndern
class ChangeDistribution : public GameCommand
{
	friend class GameClient;
	/// Gr��e der Distributionsdaten
	static const unsigned DATA_SIZE = 20;
	/// Daten der Distribution (einzelne Prozente der Waren in Geb�uden)
	std::vector<unsigned char> data;
public:
	ChangeDistribution(const std::vector<unsigned char>& data) 
		: GameCommand(CHANGEDISTRIBUTION), data(data) { assert(data.size() == DATA_SIZE); }
	ChangeDistribution(Serializer * ser) 
		: GameCommand(CHANGEDISTRIBUTION), data(DATA_SIZE)
	{
		for(unsigned i = 0;i<DATA_SIZE;++i)
			data[i] = ser->PopUnsignedChar();
	}

	virtual void Serialize(Serializer *ser) const
	{
		for(unsigned i = 0;i<DATA_SIZE;++i)
			ser->PushUnsignedChar(data[i]);
	}

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};

/// Baureihenfolge �ndern
class ChangeBuildOrder : public GameCommand
{
	friend class GameClient;
	/// Gr��e der BuildOrder-Daten
	static const unsigned DATA_SIZE = 31;
	/// Ordnungs-Typ
	const unsigned char order_type;
	/// Daten der BuildOrder
	std::vector<unsigned char> data;
public:
	ChangeBuildOrder(const unsigned char order_type, const std::vector<unsigned char>& data) 
		: GameCommand(CHANGEBUILDORDER), order_type(order_type), data(data) { assert(data.size() == DATA_SIZE); }
	ChangeBuildOrder(Serializer * ser) 
		: GameCommand(CHANGEBUILDORDER), order_type(ser->PopUnsignedChar()), data(DATA_SIZE)
	{
		for(unsigned i = 0;i<DATA_SIZE;++i)
			data[i] = ser->PopUnsignedChar();
	}

	virtual void Serialize(Serializer *ser) const
	{
		ser->PushUnsignedChar(order_type);
		for(unsigned i = 0;i<DATA_SIZE;++i)
			ser->PushUnsignedChar(data[i]);
	}

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};


/// Baustelle setzen
class SetBuildingSite : public Coords
{
	friend class GameClient;
	/// Art des Geb�udes, was gebaut werden soll
	const BuildingType bt;
public:
	SetBuildingSite(const MapCoord x, const MapCoord y, const BuildingType bt)
		: Coords(SETBUILDINGSITE,x,y), bt(bt) {}
	SetBuildingSite(Serializer * ser)
		: Coords(SETBUILDINGSITE,ser),
		bt(BuildingType(ser->PopUnsignedChar())) {}

	virtual void Serialize(Serializer *ser) const
	{
		Coords::Serialize(ser);

		ser->PushUnsignedChar(static_cast<unsigned char>(bt));
	}

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};

/// Geb�ude zerst�ren
class DestroyBuilding : public Coords
{
	friend class GameClient;
public:
	DestroyBuilding(const MapCoord x, const MapCoord y)
		: Coords(DESTROYBUILDING,x,y) {}
	DestroyBuilding(Serializer * ser)
		: Coords(DESTROYBUILDING,ser) {}

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};


/// Transportreihenfolge �ndern
class ChangeTransport : public GameCommand
{
	friend class GameClient;
	/// Gr��e der Distributionsdaten
	static const unsigned DATA_SIZE = 14;
	/// Daten der Distribution (einzelne Prozente der Waren in Geb�uden)
	std::vector<unsigned char> data;
public:
	ChangeTransport(const std::vector<unsigned char>& data) 
		: GameCommand(CHANGETRANSPORT), data(data) { assert(data.size() == DATA_SIZE); }
	ChangeTransport(Serializer * ser) 
		: GameCommand(CHANGETRANSPORT), data(DATA_SIZE)
	{
		for(unsigned i = 0;i<DATA_SIZE;++i)
			data[i] = ser->PopUnsignedChar();
	}

	virtual void Serialize(Serializer *ser) const
	{
		for(unsigned i = 0;i<DATA_SIZE;++i)
			ser->PushUnsignedChar(data[i]);
	}

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};

/// Transportreihenfolge �ndern
class ChangeMilitary : public GameCommand
{
	friend class GameClient;
	/// Gr��e der Distributionsdaten
	static const unsigned DATA_SIZE = 7;
	/// Daten der Distribution (einzelne Prozente der Waren in Geb�uden)
	std::vector<unsigned char> data;
public:
	ChangeMilitary(const std::vector<unsigned char>& data) 
		: GameCommand(CHANGEMILITARY), data(data) { assert(data.size() == DATA_SIZE); }
	ChangeMilitary(Serializer * ser) 
		: GameCommand(CHANGEMILITARY), data(DATA_SIZE)
	{
		for(unsigned i = 0;i<DATA_SIZE;++i)
			data[i] = ser->PopUnsignedChar();
	}

	virtual void Serialize(Serializer *ser) const
	{
		for(unsigned i = 0;i<DATA_SIZE;++i)
			ser->PushUnsignedChar(data[i]);
	}

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};

/// Werkzeugeinstellungen �ndern
class ChangeTools : public GameCommand
{
	friend class GameClient;
	/// Gr��e der Distributionsdaten
	static const unsigned DATA_SIZE = 12;
	/// Daten der Distribution (einzelne Prozente der Waren in Geb�uden)
	std::vector<unsigned char> data;
public:
	ChangeTools(const std::vector<unsigned char>& data) 
		: GameCommand(CHANGETOOLS), data(data) { assert(data.size() == DATA_SIZE); }
	ChangeTools(Serializer * ser) 
		: GameCommand(CHANGETOOLS), data(DATA_SIZE)
	{
		for(unsigned i = 0;i<DATA_SIZE;++i)
			data[i] = ser->PopUnsignedChar();
	}

	virtual void Serialize(Serializer *ser) const
	{
		for(unsigned i = 0;i<DATA_SIZE;++i)
			ser->PushUnsignedChar(data[i]);
	}

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};

/// Geologen rufen
class CallGeologist : public Coords
{
	friend class GameClient;
public:
	CallGeologist(const MapCoord x, const MapCoord y)
		: Coords(CALLGEOLOGIST,x,y) {}
	CallGeologist(Serializer * ser)
		: Coords(CALLGEOLOGIST,ser) {}

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};

/// Sp�her rufen
class CallScout : public Coords
{
public:
	CallScout(const MapCoord x, const MapCoord y)
		: Coords(CALLSCOUT,x,y) {}
	CallScout(Serializer * ser)
		: Coords(CALLSCOUT,ser) {}

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};

/// Angriff starten
class Attack : public Coords
{
	friend class GameClient;
	/// Anzahl der Soldaten
	const unsigned soldiers_count;
	/// Starke Soldaten oder schwache Soldaten?
	const bool strong_soldiers;
	
public:
	Attack(const MapCoord x, const MapCoord y, const unsigned soldiers_count, const bool strong_soldiers)
		: Coords(ATTACK,x,y), soldiers_count(soldiers_count), strong_soldiers(strong_soldiers) {}
	Attack(Serializer * ser)
		: Coords(ATTACK,ser),
		soldiers_count(ser->PopUnsignedInt()), strong_soldiers(ser->PopBool()) {}

	virtual void Serialize(Serializer *ser) const
	{
		Coords::Serialize(ser);

		ser->PushUnsignedInt(soldiers_count);
		ser->PushBool(strong_soldiers);
	}

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};

/// Spielerwechsel
class SwitchPlayer : public GameCommand
{
	friend class GameClient;
	friend class GameServer;
	/// ID des Spielers, zu den hingewechselt werden soll
	const unsigned new_player_id;

public:
	SwitchPlayer(const unsigned char new_player_id) 
		: GameCommand(SWITCHPLAYER), new_player_id(new_player_id) {}
	SwitchPlayer(Serializer * ser) 
		: GameCommand(SWITCHPLAYER), new_player_id(ser->PopUnsignedChar()) {}

	virtual void Serialize(Serializer * ser) const
	{
		ser->PushUnsignedChar(new_player_id);
	}

	const unsigned int GetNewPlayerId() const { return new_player_id; }

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};

/// Goldzufuhr in einem Geb�ude stoppen/erlauben
class StopGold : public Coords
{
	friend class GameClient;
public:
	StopGold(const MapCoord x, const MapCoord y)
		: Coords(STOPGOLD,x,y) {}
	StopGold(Serializer * ser)
		: Coords(STOPGOLD,ser) {}

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};

/// Produktivit�t in einem Geb�ude deaktivieren/aktivieren
class StopProduction : public Coords
{
	friend class GameClient;
public:
	StopProduction(const MapCoord x, const MapCoord y)
		: Coords(STOPPRODUCTION,x,y) {}
	StopProduction(Serializer * ser)
		: Coords(STOPPRODUCTION,ser) {}

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};

/// Einlagerungseinstellungen von einem Lagerhaus ver�ndern
class ChangeInventorySetting : public Coords
{
	/// Kategorie (Waren, Menschen), Status (Einlagern/Auslagern), type (welche Ware, welcher Mensch)
	const unsigned char category, state, type;
public:
	ChangeInventorySetting(const MapCoord x, const MapCoord y, const unsigned char category,
		const unsigned char state, const unsigned char type)
		: Coords(CHANGEINVENTORYSETTING,x,y), category(category), state(state), type(type) {}
	ChangeInventorySetting(Serializer * ser)
		: Coords(CHANGEINVENTORYSETTING,ser),
		category(ser->PopUnsignedChar()),
		state(ser->PopUnsignedChar()),
		type(ser->PopUnsignedChar())
	{}

	virtual void Serialize(Serializer *ser) const
	{
		Coords::Serialize(ser);

		ser->PushUnsignedChar(category);
		ser->PushUnsignedChar(state);
		ser->PushUnsignedChar(type);
	}

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};

/// Alle Einlagerungseinstellungen (f�r alle Menschen oder Waren) von einem Lagerhaus ver�ndern
class ChangeAllInventorySettings : public Coords
{
	friend class GameClient;
	/// Kategorie (Waren, Menschen), Status (Einlagern/Auslagern), type (welche Ware, welcher Mensch)
	const unsigned char category, state;
public:
	ChangeAllInventorySettings(const MapCoord x, const MapCoord y, const unsigned char category,
		const unsigned char state)
		: Coords(CHANGEALLINVENTORYSETTINGS,x,y), category(category), state(state) {}
	ChangeAllInventorySettings(Serializer * ser)
		: Coords(CHANGEALLINVENTORYSETTINGS,ser),
		category(ser->PopUnsignedChar()),
		state(ser->PopUnsignedChar())
	{}

	virtual void Serialize(Serializer *ser) const
	{
		Coords::Serialize(ser);

		ser->PushUnsignedChar(category);
		ser->PushUnsignedChar(state);
	}

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};

/// Ver�ndert die Reserve im HQ auf einen bestimmten Wert
class ChangeReserve : public Coords
{
	friend class GameClient;
	/// Rang des Soldaten, der ver�ndert werden soll
	const unsigned char rank;
	/// Anzahl der Reserve f�r diesen Rang
	const unsigned char count;
public:
	ChangeReserve(const MapCoord x, const MapCoord y, const unsigned char rank, const unsigned char count)
		: Coords(CHANGERESERVE,x,y), rank(rank), count(count) {}
	ChangeReserve(Serializer * ser)
		: Coords(CHANGERESERVE,ser),
		rank(ser->PopUnsignedChar()),
		count(ser->PopUnsignedChar())
	{}

	virtual void Serialize(Serializer *ser) const
	{
		Coords::Serialize(ser);

		ser->PushUnsignedChar(rank);
		ser->PushUnsignedChar(count);
	}

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};

/// Alle Fahnen zerst�ren
class CheatArmageddon : public GameCommand
{
	friend class GameClient;
public:
	CheatArmageddon()
		: GameCommand(CHEAT_ARMAGEDDON) {}
	CheatArmageddon(Serializer * ser)
		: GameCommand(CHEAT_ARMAGEDDON) {}

	virtual void Serialize(Serializer *ser) const
	{}

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};

/// Aufgeben
class Surrender : public GameCommand
{
	friend class GameClient;
public:
	Surrender()
		: GameCommand(SURRENDER) {}
	Surrender(Serializer * ser)
		: GameCommand(SURRENDER) {}

	virtual void Serialize(Serializer *ser) const
	{}

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};

/// Alle eigenen Fahnen zerst�ren
class DestroyAll : public GameCommand
{
	friend class GameClient;
public:
	DestroyAll()
		: GameCommand(DESTROYALL) {}
	DestroyAll(Serializer * ser)
		: GameCommand(DESTROYALL) {}

	virtual void Serialize(Serializer *ser) const
	{}

	/// F�hrt das GameCommand aus
	void Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid);
};

}



#endif



