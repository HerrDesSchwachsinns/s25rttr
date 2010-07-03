// $Id: GameClientPlayer.h 6535 2010-07-03 08:12:55Z FloSoft $
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

#ifndef GAMECLIENTPLAYER_H_
#define GAMECLIENTPLAYER_H_

#include "GamePlayerInfo.h"
#include "GameConsts.h"
#include <list>
#include <algorithm>
#include "MapConsts.h"
#include "PostMsg.h"


class GameWorld;
class noFlag;
class RoadSegment;
class nobBaseWarehouse;
class noRoadNode;
class noBaseBuilding;
class noBuilding;
class noBuildingSite;
class Ware;
class nobMilitary;
class nobUsual;
class nofFlagWorker;
class nobBaseMilitary;
class SerializedGameData;
class nofCarrier;
class noShip;
class nobHarborBuilding;
class GameWorldGame;
class GameMessage_GameCommand;
class nofPassiveSoldier;


/// Informationen �ber Gebäude-Anzahlen
struct BuildingCount
{
	unsigned building_counts[40];
	unsigned building_site_counts[40];
};


class GameClientPlayer : public GamePlayerInfo
{
private:

	// Zugriff der Spieler auf die Spielwelt
	GameWorldGame * gwg;
	/// Liste der Warenhäuser des Spielers
	std::list<nobBaseWarehouse*> warehouses;
	/// Liste von Häfen
	std::list<nobHarborBuilding*> harbors;
	///// Liste von unbesetzten Straßen (ohne Träger) von dem Spieler
	//std::list<RoadSegment*> unoccupied_roads;
	/// Lister aller Straßen von dem Spieler
	std::list<RoadSegment*> roads;

	struct JobNeeded
	{
		Job job;
		noRoadNode * workplace;
	};

	struct BuildingWhichWantWare
	{
		unsigned char count;
		unsigned char building;
	};

	/// Liste von Baustellen/Gebäuden, die bestimmten Beruf wollen
	std::list<JobNeeded> jobs_wanted;

	/// Listen der einzelnen Gebäudetypen (nur nobUsuals!)
	std::list<nobUsual*> buildings[30];
	/// Liste von sämtlichen Baustellen
	std::list<noBuildingSite*> building_sites;
	/// Liste von allen Militärgebäuden
	std::list<nobMilitary*> military_buildings;
	/// Liste von sämtlichen Waren, die herumgetragen werden und an Fahnen liegen
	std::list<Ware*> ware_list;
	/// Liste von Geologen und Spähern, die an eine Flagge gebunden sind
	std::list<nofFlagWorker*> flagworkers;
	/// Liste von Schiffen dieses Spielers
	std::vector<noShip*> ships;

	/// Liste mit Punkten, die schon von Schiffen entdeckt wurden
	std::vector< Point<MapCoord> > enemies_discovered_by_ships;
	


	/// Liste, welchen nächsten 10 Angreifern Verteidiger entgegenlaufen sollen
	bool defenders[5];
	unsigned short defenders_pos;

	/// Inventur
	Goods global_inventory;

	/// B�ndnisse mit anderen Spielern
	struct Pact
	{
		/// B�ndnis schon akzeptiert oder nur vorgeschlagen?
		bool accepted;
		/// Dauer (in GF), 0 = kein B�ndnise, 0xFFFFFFFF = B�ndnis auf Ewigkeit
		unsigned duration;
		/// Startzeitpunkt (in GF)
		unsigned start;
		/// Will dieser Spieler (also der this-Pointer) diesen Vertrag auflösen?
		bool want_cancel;

		Pact() : accepted(false), duration(0), start(0), want_cancel(false) {}
		Pact(SerializedGameData *ser);
		void Serialize(SerializedGameData *ser);
	};
	/// B�ndnisse dieses Spielers mit anderen Spielern
	Pact pacts[MAX_PLAYERS][PACTS_COUNT];

public:

	/// Laggt der Spieler?
	bool is_lagging;
	/// Empfangene GC f�r diesen Spieler
	std::list<GameMessage_GameCommand> gc_queue;

	/// Koordinaten des HQs des Spielers
	unsigned short hqx,hqy;

	// Informationen �ber die Verteilung
	struct
	{
		unsigned char percent_buildings[40];
		std::list<BuildingType> client_buildings; // alle Gebäude, die diese Ware bekommen, zusammengefasst
		std::vector<unsigned char> goals;
		unsigned selected_goal;
	} distribution[WARE_TYPES_COUNT];

	/// Art der Reihenfolge (0 = nach Auftraggebung, ansonsten nach build_order)
	unsigned char order_type;
	/// Baureihenfolge
	std::vector <unsigned char> build_order;
	/// Prioritäten der Waren im Transport
	unsigned char transport[WARE_TYPES_COUNT];
	/// Militäreinstellungen (die vom Militärmen�)
	std::vector <unsigned char> military_settings;
	/// Werkzeugeinstellungen (in der Reihenfolge wie im Fenster!)
	std::vector <unsigned char> tools_settings;

private:

	// Sucht Weg f�r Job zu entsprechenden noRoadNode
	bool FindWarehouseForJob(const Job job, noRoadNode * goal);
	/// Pr�ft, ob der Spieler besiegt wurde
	void TestDefeat();
	/// B�ndnis (real, d.h. spielentscheidend) abschließen
	void MakePact(const PactType pt, const unsigned char other_player, const unsigned duration);

public:
	/// Konstruktor von @p GameClientPlayer.
	GameClientPlayer(const unsigned playerid);

	/// Serialisieren
	void Serialize(SerializedGameData * sgd);
	// Deserialisieren
	void Deserialize(SerializedGameData * sgd);

	/// Tauscht Spieler
	void SwapPlayer(GameClientPlayer& two);

	/// Setzt GameWorld
	void SetGameWorldPointer(GameWorldGame * const gwg) { this->gwg = gwg; }

	/*/// liefert das aktuelle (komplette) inventar.
	void GetInventory(unsigned int *wares, unsigned int *figures);*/


	/// Sucht ein nächstgelegenes Warenhaus f�r den Punkt 'start', das die Bedingung der Übergebenen Funktion
	/// IsWarehouseGood erf�llt, als letzen Parameter erhält jene Funktion param
	/// - forbidden ist ein optionales Straßenst�ck, das nicht betreten werden darf,
	/// - to_wh muss auf true gesetzt werden, wenn es zum Lagerhaus geht, ansonsten auf false, in length wird die Wegeslänge zur�ckgegeben
	nobBaseWarehouse * FindWarehouse(const noRoadNode * const start,bool (*IsWarehouseGood)(nobBaseWarehouse*,const void*),const RoadSegment * const forbidden,const bool to_wh,const void * param,const bool use_boat_roads,unsigned * const length = 0);
	/// Gibt dem Spieler bekannt, das eine neue Straße gebaut wurde
	void NewRoad(RoadSegment * const rs);
	/// Neue Straße hinzuf�gen
	void AddRoad(RoadSegment * const rs) { roads.push_back(rs); }
	/// Gibt dem Spieler brekannt, das eine Straße abgerissen wurde
	void RoadDestroyed();
	/// Sucht einen Träger f�r die Straße und ruft ggf den Träger aus dem jeweiligen nächsten Lagerhaus
	bool FindCarrierForRoad(RoadSegment * rs);
	/// Warenhaus zur Warenhausliste hinzuf�gen
	void AddWarehouse(nobBaseWarehouse * wh) { warehouses.push_back(wh); }
	/// Warenhaus aus Warenhausliste entfernen
	void RemoveWarehouse(nobBaseWarehouse * wh) { warehouses.remove(wh); TestDefeat(); }
	/// Hafen zur Warenhausliste hinzuf�gen
	void AddHarbor(nobHarborBuilding * hb);
	/// Hafen aus Warenhausliste entfernen
	void RemoveHarbor(nobHarborBuilding * hb) { harbors.remove(hb); }
	/// (Unbesetzte) Straße aus der Liste entfernen
	void DeleteRoad(RoadSegment * rs) { roads.remove(rs); }
	//bool TestRoads(RoadSegment * rs) { return roads.search(rs).valid(); }

	/// F�r alle unbesetzen Straßen Weg neu berechnen
	void FindWarehouseForAllRoads();
	/// Lässt alle Baustellen ggf. noch vorhandenes Baumaterial bestellen
	void FindMaterialForBuildingSites();
	/// F�gt ein RoadNode hinzu, der einen bestimmten Job braucht
	void AddJobWanted(const Job job,noRoadNode * workplace);
	/// Entfernt ihn wieder aus der Liste (wenn er dann doch nich mehr gebraucht wird)
	void JobNotWanted(noRoadNode * workplace);
	/// Versucht f�r alle Arbeitsplätze eine Arbeitskraft zu suchen
	void FindWarehouseForAllJobs(const Job job);
	/// Versucht f�r alle verlorenen Waren ohne Ziel Lagerhaus zu finden
	void FindClientForLostWares();
	/// Bestellt eine Ware und gibt sie zur�ck, falls es eine gibt, ansonsten 0
	Ware * OrderWare(const GoodType ware,noBaseBuilding * goal);
	/// Versucht einen Esel zu bestellen, gibt 0 zur�ck, falls keinen gefunden
	nofCarrier * OrderDonkey(RoadSegment * road);
	/// Versucht f�r einen Esel eine Straße zu finden, in goal wird die Zielflagge zur�ckgegeben,
	/// sofern eine Straße gefunden wurde, ansonsten ist das ein Lagerhaus oder 0, falls auch das nich gefunden wurde
	RoadSegment * FindRoadForDonkey(noRoadNode * start,noRoadNode ** goal);


	/// Sucht f�r eine (neuproduzierte) Ware einen Abnehmer (wenns keinen gibt, wird ein Lagerhaus gesucht, wenn
	/// es auch dorthin keinen Weg gibt, wird 0 zur�ckgegeben
	noBaseBuilding * FindClientForWare(Ware * ware);
	/// Sucht einen Abnehmer (sprich Militärgebäude), wenn es keinen findet, wird ein Warenhaus zur�ckgegeben bzw. 0
	nobBaseMilitary * FindClientForCoin(Ware * ware);

	/// Speichert Baustellen Gebäude etc, erklärt sich von selbst
	void AddBuildingSite(noBuildingSite * building_site);
	void RemoveBuildingSite(noBuildingSite * building_site);
	/// Speichert normale Gebäude
	void AddUsualBuilding(nobUsual * building);
	void RemoveUsualBuilding(nobUsual * building);
	/// Speichert Militärgebäude
	void AddMilitaryBuilding(nobMilitary * building);
	void RemoveMilitaryBuilding(nobMilitary * building);

	const std::list<noBuildingSite*>& GetBuildingSites() const { return building_sites; }

	const std::list<nobMilitary *>& GetMilitaryBuildings() const { return military_buildings; }

	/// Gibt Liste von Gebäuden des Spieler zur�ck
	const std::list<nobUsual*>& GetBuildings(const BuildingType type);
	/// Liefert die Anzahl aller Gebäude einzeln
	void GetBuildingCount(BuildingCount& bc) const;
	/// Berechnet die durschnittlichen Produktivität eines jeden Gebäudetyps
	/// (erwartet als Argument ein 40-er Array!)
	void CalcProductivities(std::vector<unsigned short>& productivities); 

	/// Berechnet die durschnittlichen Produktivität aller Gebäude
	unsigned short CalcAverageProductivitiy();


	/// Gibt Priorität der Baustelle zur�ck (entscheidet selbständig, welche Reihenfolge usw)
	/// je kleiner die R�ckgabe, destro größer die Priorität!
	unsigned GetBuidingSitePriority(const noBuildingSite * building_site);

	/// Berechnet die Verteilung der Waren auf die einzelnen Gebäude neu
	void RecalcDistribution();
	/// Berechnet die Verteilung einer (bestimmten) Ware
	void RecalcDistributionOfWare(const GoodType ware);
	/// Konvertiert die Daten vom wp_transport in "unser" Prioritäten-Format und setzt es
	void ConvertTransportData(const std::vector<unsigned char>& transport_data);

	/// Ware zur globalen Warenliste hinzuf�gen und entfernen
	void RegisterWare(Ware * ware) { ware_list.push_back(ware); }
	void RemoveWare(Ware * ware) { ware_list.remove(ware); }
	bool IsWareRegistred(Ware * ware) { return (std::find(ware_list.begin(),ware_list.end(),ware) != ware_list.end()); }
	bool IsWareDependent(Ware * ware);

	/// F�gt Waren zur Inventur hinzu
	void IncreaseInventoryWare(const GoodType ware, const unsigned count) { global_inventory.goods[ConvertShields(ware)]+=count; }
	void DecreaseInventoryWare(const GoodType ware, const unsigned count) { assert(global_inventory.goods[ConvertShields(ware)]>=count); global_inventory.goods[ConvertShields(ware)]-=count; }
	void IncreaseInventoryJob(const Job job, const unsigned count) { global_inventory.people[job]+=count; }
	void DecreaseInventoryJob(const Job job, const unsigned count) { assert(global_inventory.people[job]>=count); global_inventory.people[job]-=count; }

	/// Gibt Inventory-Settings zur�ck
	const Goods * GetInventory() const { return &global_inventory; }

	/// Setzt neue Militäreinstellungen
	void ChangeMilitarySettings(const std::vector<unsigned char>& military_settings);
	/// Setzt neue Werkzeugeinstellungen
	void ChangeToolsSettings(const std::vector<unsigned char>& tools_settings);
	/// Setzt neue Verteilungseinstellungen
	void ChangeDistribution(const std::vector<unsigned char>& distribution_settings);
	/// Setzt neue Baureihenfolge-Einstellungen
	void ChangeBuildOrder(const unsigned char order_type, const std::vector<unsigned char>& oder_data);

	/// Darf der andere Spieler von mir angegriffen werden?
	bool IsPlayerAttackable(const unsigned char player) const;
	/// Ist ein anderer Spieler ein richtiger Verb�ndeter von uns, d.h. Teamsicht, Unterst�tzung durch aggressive Verteidiger usw.?
	bool IsAlly(const unsigned char player) const;
	/// Truppen bestellen
	void OrderTroops(nobMilitary * goal, unsigned count);
	/// Pr�ft die Besatzung von allen Militärgebäuden und reguliert entsprechend (bei Veränderung der Militäreinstellungen)
	void RegulateAllTroops();
	/// Pr�ft von allen Milit�rgeb�uden die Fahnen neu
	void RecalcMilitaryFlags();
	/// Sucht f�r EINEN Soldaten ein neues Militärgebäude, als Argument wird Referenz auf die 
	/// entsprechende Soldatenanzahl im Lagerhaus verlangt
	void NewSoldierAvailable(const unsigned& soldier_count);
	/// Aktualisiert die Verteidiger-Liste
	void RefreshDefenderList();
	/// Pr�ft, ob f�r einen angreifenden Soldaten ein Verteidger geschickt werden soll
	bool ShouldSendDefender();

	/// Ruft einen Geologen
	void CallFlagWorker(const unsigned short x, const unsigned short y, const Job job);
	/// Registriert einen Geologen bzw. einen Späher an einer bestimmten Flagge, damit diese informiert werden,
	/// wenn die Flagge abgerissen wird
	void RegisterFlagWorker(nofFlagWorker * flagworker) { flagworkers.push_back(flagworker); }
	void RemoveFlagWorker(nofFlagWorker * flagworker) { flagworkers.remove(flagworker); }
	/// Wird aufgerufen, wenn eine Flagge abgerissen wurde, damit das den Flaggen-Arbeitern gesagt werden kann
	void FlagDestroyed(noFlag * flag);

	/// Gibt erstes Lagerhaus zur�ck
	nobBaseWarehouse * GetFirstWH() { return *warehouses.begin(); }

	/// Registriert ein Schiff beim Einwohnermeldeamt
	void RegisterShip(noShip * ship);
	/// Meldet das Schiff wieder ab
	void RemoveShip(noShip * ship);
	/// Versucht, f�r ein untätiges Schiff eine Arbeit zu suchen
	void GetJobForShip(noShip * ship);
	/// Schiff f�r Hafen bestellen, zweiter Parameter gibt an, ob das Schiff in einer Jobliste
	/// vermerkt werden soll, also unbedingt eins gebraucht wird 
	void OrderShip(nobHarborBuilding * hb);
	/// Gibt die ID eines Schiffes zur�ck
	unsigned GetShipID(const noShip * const ship) const;
	/// Gibt ein Schiff anhand der ID zur�ck bzw. NULL, wenn keines mit der ID existiert
	noShip * GetShipByID(const unsigned ship_id) const;
	/// Gibt die Gesamtanzahl von Schiffen zur�ck
	unsigned GetShipCount() const { return ships.size(); }
	/// Gibt eine Liste mit allen Häfen dieses Spieler zur�ck, die an ein bestimmtes Meer angrenzen
	void GetHarborBuildings(std::vector<nobHarborBuilding*>& harbor_buildings, const unsigned short sea_id) const;
	/// Gibt die Anzahl der Schiffe, die einen bestimmten Hafen ansteuern, zur�ck
	unsigned GetShipsToHarbor(nobHarborBuilding * hb) const;
	/// Gibt der Wirtschaft Bescheid, dass ein Hafen zerstört wurde
	void HarborDestroyed(nobHarborBuilding * hb);
	/// Sucht einen Hafen in der Nähe, wo dieses Schiff seine Waren abladen kann
	/// gibt true zur�ck, falls erfolgreich
	bool FindHarborForUnloading(noShip * ship, const MapCoord start_x, const MapCoord start_y, unsigned * goal_harbor_id, std::vector<unsigned char> * route,
		nobHarborBuilding * exception);
	/// Ein Schiff hat feindliches Land entdeckt --> ggf. f�r Postnachrichten ausl�sen
	void ShipDiscoveredHostileTerritory(const Point<MapCoord> location);

	/// Gibt eine Liste der verf�gbaren Häfen zur�ck
	const std::list<nobHarborBuilding*>& GetHarbors() const { return harbors; }

	/// Er gibt auf
	void Surrender();

	/// Macht B�ndnisvorschlag an diesen Spieler
	void SuggestPact(const unsigned char other_player, const PactType pt, const unsigned duration);
	/// Akzeptiert ein bestimmtes B�ndnis, welches an diesen Spieler gemacht wurde
	void AcceptPact(const unsigned id, const PactType pt, const unsigned char other_player);
	/// Gibt Einverständnis, dass dieser Spieler den Pakt auflösen will
	/// Falls dieser Spieler einen B�ndnisvorschlag gemacht hat, wird dieser dagegen zur�ckgenommen
	void CancelPact(const PactType pt, const unsigned char other_player);
	/// Zeigt an, ob ein Pakt besteht
	enum PactState
	{
		NO_PACT = 0, /// Kein Pakt geschlossen
		IN_PROGRESS, /// Pakt angeboten, aber noch nicht akzeptiert
		ACCEPTED /// B�ndnis in Kraft
	};
	PactState GetPactState(const PactType pt, const unsigned char other_player) const;
	/// Gibt die verbleibende Dauer zur�ck, die ein B�ndnis noch laufen wird (0xFFFFFFFF = f�r immer)
	unsigned GetRemainingPactTime(const PactType pt, const unsigned char other_player) const;
	/// Setzt die initialen B�ndnisse ahand der Teams
	void MakeStartPacts();
	/// Testet die B�ndnisse, ob sie nicht schon abgelaufen sind
	void TestPacts();
		


  // Statistik-Sachen

  void SetStatisticValue(StatisticType type, unsigned int value);
  void ChangeStatisticValue(StatisticType type, int change);

	void IncreaseMerchandiseStatistic(GoodType type);
 
  void StatisticStep();
  
  struct Statistic
  {
    // 30 Datensätze pro Typ
    unsigned int data[STAT_TYPE_COUNT][STAT_STEP_COUNT];
		// und das gleiche f�r die Warenstatistik
		unsigned short merchandiseData[STAT_MERCHANDISE_TYPE_COUNT][STAT_STEP_COUNT];
    // Index, der gerade 'vorne' (rechts im Statistikfenster) ist
    unsigned short currentIndex;
    // Counter, bei jedem vierten Update jeweils Daten zu den längerfristigen Statistiken kopieren
    unsigned short counter;
  };

  const Statistic& GetStatistic(StatisticTime time) { return statistic[time]; };

	// Testet ob Notfallprogramm aktiviert werden muss und tut dies dann
	void TestForEmergencyProgramm();

private:
  // Statistikdaten
  Statistic statistic[STAT_TIME_COUNT];

  // Die Statistikwerte die 'aktuell' gemessen werden
  unsigned int statisticCurrentData[STAT_TYPE_COUNT];
	unsigned short statisticCurrentMerchandiseData[STAT_MERCHANDISE_TYPE_COUNT];

  unsigned short incrStatIndex(unsigned short i) { return (i==STAT_STEP_COUNT-1) ? 0 : ++i; }
  unsigned short decrStatIndex(unsigned short i) { return (i==0) ? STAT_STEP_COUNT-1 : --i; }
	unsigned short decrStatIndex(unsigned short i, unsigned short amount) { return (i < amount) ? STAT_STEP_COUNT - (amount - i) - 1 : i - amount; }

	// Notfall-Programm aktiviert ja/nein (Es gehen nur noch Res an Holzfäller- und Sägewerk-Baustellen raus)
	bool emergency;

public:
	bool hasEmergency() const { return emergency; }

	/// Testet ob der Spieler noch mehr Katapulte bauen darf
	bool CanBuildCatapult() const;

};

#endif


