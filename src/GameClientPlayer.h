// $Id: GameClientPlayer.h 5074 2009-06-20 14:31:41Z OLiver $
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
class GameWorldGame;
class GameMessage_GameCommand;


/// Informationen �ber Geb�ude-Anzahlen
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
	/// Liste der Warenh�user des Spielers
	std::list<nobBaseWarehouse*> warehouses;
	///// Liste von unbesetzten Stra�en (ohne Tr�ger) von dem Spieler
	//std::list<RoadSegment*> unoccupied_roads;
	/// Lister aller Stra�en von dem Spieler
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

	/// Liste von Baustellen/Geb�uden, die bestimmten Beruf wollen
	std::list<JobNeeded> jobs_wanted;

	/// Listen der einzelnen Geb�udetypen (nur nobUsuals!)
	std::list<nobUsual*> buildings[30];
	/// Liste von s�mtlichen Baustellen
	std::list<noBuildingSite*> building_sites;
	/// Liste von allen Milit�rgeb�uden
	std::list<nobMilitary*> military_buildings;
	/// Liste von s�mtlichen Waren, die herumgetragen werden und an Fahnen liegen
	std::list<Ware*> ware_list;
	/// Liste von Geologen und Sp�hern, die an eine Flagge gebunden sind
	std::list<nofFlagWorker*> flagworkers;

	/// Liste, welchen n�chsten 10 Angreifern Verteidiger entgegenlaufen sollen
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
		/// Will dieser Spieler (also der this-Pointer) diesen Vertrag aufl�sen?
		bool want_cancel;

		Pact() : accepted(false), duration(0), start(0), want_cancel(false) {}
		Pact(Serializer * ser);
		void Serialize(Serializer * ser);
	};
	/// B�ndnisse dieses Spielers mit anderen Spielern
	Pact pacts[MAX_PLAYERS][PACTS_COUNT];

	/*/// B�ndnisvorschl�ge, die von einem anderen Spieler an diesen Spieler gemacht wurden
	struct PactSuggestion
	{
		/// Zeitpunkt des Vorschlags durch den Spieler (damit nicht weit im Nachhinein der Vertrag durch
		/// den anderen Spieler get�tigt werden kann, obwohl ersterer vielleicht gar nicht mehr dazu bereit ist
		unsigned suggestion_time;
		/// Der andere Spieler
		unsigned char player;
		/// B�ndnisart
		PactType pt;
		/// Dauer des B�ndnisses 
		unsigned duration;

		PactSuggestion(const unsigned suggestion_time, const unsigned char player, const PactType pt, const unsigned duration) 
			: suggestion_time(suggestion_time), player(player), pt(pt), duration(duration) {}
		PactSuggestion(Serializer * ser);
		void Serialize(Serializer * ser);
	};

	std::list<PactSuggestion> pact_suggestions;*/

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
		std::list<BuildingType> client_buildings; // alle Geb�ude, die diese Ware bekommen, zusammengefasst
		std::vector<unsigned char> goals;
		unsigned selected_goal;
	} distribution[WARE_TYPES_COUNT];

	/// Art der Reihenfolge (0 = nach Auftraggebung, ansonsten nach build_order)
	unsigned char order_type;
	/// Baureihenfolge
	std::vector <unsigned char> build_order;
	/// Priorit�ten der Waren im Transport
	unsigned char transport[WARE_TYPES_COUNT];
	/// Milit�reinstellungen (die vom Milit�rmen�)
	std::vector <unsigned char> military_settings;
	/// Werkzeugeinstellungen (in der Reihenfolge wie im Fenster!)
	std::vector <unsigned char> tools_settings;

private:

	// Sucht Weg f�r Job zu entsprechenden noRoadNode
	bool FindWarehouseForJob(const Job job, noRoadNode * goal);
	/// Pr�ft, ob der Spieler besiegt wurde
	void TestDefeat();
	/// B�ndnis (real, d.h. spielentscheidend) abschlie�en
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


	/// Sucht ein n�chstgelegenes Warenhaus f�r den Punkt 'start', das die Bedingung der �bergebenen Funktion
	/// IsWarehouseGood erf�llt, als letzen Parameter erh�lt jene Funktion param
	/// - forbidden ist ein optionales Stra�enst�ck, das nicht betreten werden darf,
	/// - to_wh muss auf true gesetzt werden, wenn es zum Lagerhaus geht, ansonsten auf false, in length wird die Wegesl�nge zur�ckgegeben
	nobBaseWarehouse * FindWarehouse(const noRoadNode * const start,bool (*IsWarehouseGood)(nobBaseWarehouse*,const void*),const RoadSegment * const forbidden,const bool to_wh,const void * param,const bool use_boat_roads,unsigned * const length = 0);
	/// Gibt dem Spieler bekannt, das eine neue Stra�e gebaut wurde
	void NewRoad(RoadSegment * const rs);
	/// Neue Stra�e hinzuf�gen
	void AddRoad(RoadSegment * const rs) { roads.push_back(rs); }
	/// Gibt dem Spieler bekannt, das eine Stra�e abgerissen wurde
	void RoadDestroyed();
	/// Sucht einen Tr�ger f�r die Stra�e und ruft ggf den Tr�ger aus dem jeweiligen n�chsten Lagerhaus
	bool FindCarrierForRoad(RoadSegment * rs);
	/// Warenhaus zur Warenhausliste hinzuf�gen
	void AddWarehouse(nobBaseWarehouse * wh) { warehouses.push_back(wh); }
	/// Warenhaus aus Warenhausliste entfernen
	void RemoveWarehouse(nobBaseWarehouse * wh) { warehouses.remove(wh); TestDefeat(); }
	/// (Unbesetzte) Stra�e aus der Liste entfernen
	void DeleteRoad(RoadSegment * rs) { roads.remove(rs); }
	//bool TestRoads(RoadSegment * rs) { return roads.search(rs).valid(); }

	/// F�r alle unbesetzen Stra�en Weg neu berechnen
	void FindWarehouseForAllRoads();
	/// L�sst alle Baustellen ggf. noch vorhandenes Baumaterial bestellen
	void FindMaterialForBuildingSites();
	/// F�gt ein RoadNode hinzu, der einen bestimmten Job braucht
	void AddJobWanted(const Job job,noRoadNode * workplace);
	/// Entfernt ihn wieder aus der Liste (wenn er dann doch nich mehr gebraucht wird)
	void JobNotWanted(noRoadNode * workplace);
	/// Versucht f�r alle Arbeitspl�tze eine Arbeitskraft zu suchen
	void FindWarehouseForAllJobs(const Job job);
	/// Versucht f�r alle verlorenen Waren ohne Ziel Lagerhaus zu finden
	void FindClientForLostWares();
	/// Bestellt eine Ware und gibt sie zur�ck, falls es eine gibt, ansonsten 0
	Ware * OrderWare(const GoodType ware,noBaseBuilding * goal);
	/// Versucht einen Esel zu bestellen, gibt 0 zur�ck, falls keinen gefunden
	nofCarrier * OrderDonkey(RoadSegment * road);
	/// Versucht f�r einen Esel eine Stra�e zu finden, in goal wird die Zielflagge zur�ckgegeben,
	/// sofern eine Stra�e gefunden wurde, ansonsten ist das ein Lagerhaus oder 0, falls auch das nich gefunden wurde
	RoadSegment * FindRoadForDonkey(noRoadNode * start,noRoadNode ** goal);


	/// Sucht f�r eine (neuproduzierte) Ware einen Abnehmer (wenns keinen gibt, wird ein Lagerhaus gesucht, wenn
	/// es auch dorthin keinen Weg gibt, wird 0 zur�ckgegeben
	noBaseBuilding * FindClientForWare(Ware * ware);
	/// Sucht einen Abnehmer (sprich Milit�rgeb�ude), wenn es keinen findet, wird ein Warenhaus zur�ckgegeben bzw. 0
	nobBaseMilitary * FindClientForCoin(Ware * ware);

	/// Speichert Baustellen Geb�ude etc, erkl�rt sich von selbst
	void AddBuildingSite(noBuildingSite * building_site);
	void RemoveBuildingSite(noBuildingSite * building_site);
	/// Speichert normale Geb�ude
	void AddUsualBuilding(nobUsual * building);
	void RemoveUsualBuilding(nobUsual * building);
	/// Speichert Milit�rgeb�ude
	void AddMilitaryBuilding(nobMilitary * building);
	void RemoveMilitaryBuilding(nobMilitary * building);

	/// Gibt Liste von Geb�uden des Spieler zur�ck
	const std::list<nobUsual*>& GetBuildings(const BuildingType type);
	/// Liefert die Anzahl aller Geb�ude einzeln
	void GetBuildingCount(BuildingCount& bc) const;
	/// Berechnet die durschnittlichen Produktivit�t eines jeden Geb�udetyps
	/// (erwartet als Argument ein 40-er Array!)
	void CalcProductivities(std::vector<unsigned short>& productivities); 

	/// Berechnet die durschnittlichen Produktivit�t aller Geb�ude
	unsigned short CalcAverageProductivitiy();


	/// Gibt Priorit�t der Baustelle zur�ck (entscheidet selbst�ndig, welche Reihenfolge usw)
	/// je kleiner die R�ckgabe, destro gr��er die Priorit�t!
	unsigned GetBuidingSitePriority(const noBuildingSite * building_site);

	/// Berechnet die Verteilung der Waren auf die einzelnen Geb�ude neu
	void RecalcDistribution();
	/// Berechnet die Verteilung einer (bestimmten) Ware
	void RecalcDistributionOfWare(const GoodType ware);
	/// Konvertiert die Daten vom wp_transport in "unser" Priorit�ten-Format und setzt es
	void ConvertTransportData(const std::vector<unsigned char>& transport_data);

	/// Ware zur globalen Warenliste hinzuf�gen und entfernen
	void RegisterWare(Ware * ware) { ware_list.push_back(ware); }
	void RemoveWare(Ware * ware) { ware_list.remove(ware); }
	bool IsWareRegistred(Ware * ware) { return (std::find(ware_list.begin(),ware_list.end(),ware) != ware_list.end()); }

	/// F�gt Waren zur Inventur hinzu
  void IncreaseInventoryWare(const GoodType ware, const unsigned count) { global_inventory.goods[ConvertShields(ware)]+=count; }
	void DecreaseInventoryWare(const GoodType ware, const unsigned count) { assert(global_inventory.goods[ConvertShields(ware)]>=count); global_inventory.goods[ConvertShields(ware)]-=count; }
  void IncreaseInventoryJob(const Job job, const unsigned count) { global_inventory.people[job]+=count; }
	void DecreaseInventoryJob(const Job job, const unsigned count) { assert(global_inventory.people[job]>=count); global_inventory.people[job]-=count; }

	/// Gibt Inventory-Settings zur�ck
	const Goods * GetInventory() const { return &global_inventory; }

	/// Setzt neue Milit�reinstellungen
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
	/// Pr�ft die Besatzung von allen Milit�rgeb�uden und reguliert entsprechend (bei Ver�nderung der Milit�reinstellungen)
	void RegulateAllTroops();
	/// Sucht f�r EINEN Soldaten ein neues Milit�rgeb�ude, als Argument wird Referenz auf die 
	/// entsprechende Soldatenanzahl im Lagerhaus verlangt
	void NewSoldierAvailable(const unsigned& soldier_count);
	/// Aktualisiert die Verteidiger-Liste
	void RefreshDefenderList();
	/// Pr�ft, ob f�r einen angreifenden Soldaten ein Verteidger geschickt werden soll
	bool ShouldSendDefender();

	/// Ruft einen Geologen
	void CallFlagWorker(const unsigned short x, const unsigned short y, const Job job);
	/// Registriert einen Geologen bzw. einen Sp�her an einer bestimmten Flagge, damit diese informiert werden,
	/// wenn die Flagge abgerissen wird
	void RegisterFlagWorker(nofFlagWorker * flagworker) { flagworkers.push_back(flagworker); }
	void RemoveFlagWorker(nofFlagWorker * flagworker) { flagworkers.remove(flagworker); }
	/// Wird aufgerufen, wenn eine Flagge abgerissen wurde, damit das den Flaggen-Arbeitern gesagt werden kann
	void FlagDestroyed(noFlag * flag);

	/// Gibt erstes Lagerhaus zur�ck
	nobBaseWarehouse * GetFirstWH() { return *warehouses.begin(); }


	/// Er gibt auf
	void Surrender();

	/// Macht B�ndnisvorschlag an diesen Spieler
	void SuggestPact(const unsigned char other_player, const PactType pt, const unsigned duration);
	/// Akzeptiert ein bestimmtes B�ndnis, welches an diesen Spieler gemacht wurde
	void AcceptPact(const unsigned id, const PactType pt, const unsigned char other_player);
	/// Gibt Einverst�ndnis, dass dieser Spieler den Pakt aufl�sen will
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
		


  // Statistik-Sachen

  void SetStatisticValue(StatisticType type, unsigned int value);
  void ChangeStatisticValue(StatisticType type, int change);
 
  void StatisticStep();
  
  struct Statistic
  {
    // 30 Datens�tze pro Typ
    unsigned int data[STAT_TYPE_COUNT][STAT_STEP_COUNT];
    // Index, der gerade 'vorne' (rechts im Statistikfenster) ist
    unsigned short currentIndex;
    // Counter, bei jedem vierten Update jeweils Daten zu den l�ngerfristigen Statistiken kopieren
    unsigned short counter;
  };

  const Statistic& GetStatistic(StatisticTime time) { return statistic[time]; };

private:
  // Statistikdaten
  Statistic statistic[STAT_TIME_COUNT];

  // Die Statistikwerte die 'aktuell' gemessen werden
  unsigned int statisticCurrentData[STAT_TYPE_COUNT];

  unsigned short incrStatIndex(unsigned short i) { return (i==STAT_STEP_COUNT-1) ? 0 : ++i; }
  unsigned short decrStatIndex(unsigned short i) { return (i==0) ? STAT_STEP_COUNT-1 : --i; }

};


#endif
