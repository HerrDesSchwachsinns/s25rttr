// $Id: nobBaseWarehouse.h 4652 2009-03-29 10:10:02Z FloSoft $
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
#ifndef NOBBASEWAREHOUSE_H_INCLUDED
#define NOBBASEWAREHOUSE_H_INCLUDED

#pragma once

#include "nobBaseMilitary.h"
#include "GameConsts.h"
#include "EventManager.h"

class nofCarrier;
class nofWarehouseWorker;
class noFigure;
class Ware;
class nobMilitary;

/// Ein/Auslagereinstellungsstruktur
struct InventorySettings
{
	unsigned char wares[34];
	unsigned char figures[30];
};

class nobBaseWarehouse : public nobBaseMilitary
{
protected:
	// Liste von Waren, die noch rausgebracht werden m�ssen, was im Moment aber nicht m�glich ist,
	// weil die Flagge voll ist vor dem Lagerhaus
	list<Ware*> waiting_wares;
	// verhindert doppeltes Holen von Waren
	bool fetch_double_protection;
	/// Liste von Figuren, die auf dem Weg zu dem Lagerhaus sind bzw. Soldaten die von ihm kommen
	list<noFigure*> dependent_figures;
	/// Liste von Waren, die auf dem Weg zum Lagerhaus sind
	list<Ware*> dependent_wares;
	/// Produzier-Tr�ger-Event
	EventManager::EventPointer producinghelpers_event;
	/// Rekrutierungsevent f�r Soldaten
	EventManager::EventPointer recruiting_event;
	/// Auslagerevent f�r Waren und Figuren
	EventManager::EventPointer empty_event;

protected:

	/// Soldaten-Reserve-Einstellung
	unsigned reserve_soldiers_available[5]; /// einkassierte Soldaten zur Reserve
	unsigned reserve_soldiers_claimed_visual[5]; /// geforderte Soldaten zur Reserve - visuell
	unsigned reserve_soldiers_claimed_real[5]; /// geforderte Soldaten zur Reserve - real

	/// Waren bzw. Menschenanzahl im Geb�ude, real_goods ist die tats�chliche Anzahl und wird zum berechnen verwendet, goods ist nur die, die auch angezeigt wird
	Goods goods,real_goods;
	InventorySettings inventory_settings_visual; ///< die Inventar-Einstellungen, visuell
	InventorySettings inventory_settings_real; ///< die Inventar-Einstellungen, real

private:

	/// Stellt Verteidiger zur Verf�gung
	nofDefender * ProvideDefender(nofAttacker * const attacker);
	/// Pr�ft, ob alle Bedingungen zum Rekrutieren erf�llt sind
	bool AreRecruitingConditionsComply();

protected:

	void HandleBaseEvent(const unsigned int id);
	/// Versucht ein Rekrutierungsevent anzumelden, falls ausreichend Waffen und Bier sowie gen�gend Gehilfen
	/// vorhanden sind (je nach Milit�reinstellungen)
	void TryRecruiting();
	/// Versucht Rekrutierungsevent abzumeldne, falls die Bedingungen nicht mehr erf�llt sind (z.B. wenn Ware
	/// rausgetragen wurde o.�.)
	void TryStopRecruiting();
	/// Aktuellen Warenbestand zur aktuellen Inventur dazu addieren
	void AddToInventory();

public:


	nobBaseWarehouse(const BuildingType type,const unsigned short x, const unsigned short y,const unsigned char player,const Nation nation);
	nobBaseWarehouse(SerializedGameData * sgd, const unsigned obj_id);

	virtual ~nobBaseWarehouse();

	/// Aufr�ummethoden
protected:	
	void Destroy_nobBaseWarehouse();
public:		
	void Destroy() { Destroy_nobBaseWarehouse(); }

		/// Serialisierungsfunktionen
protected: void Serialize_nobBaseWarehouse(SerializedGameData * sgd) const;
public: void Serialize(SerializedGameData *sgd) const { Serialize_nobBaseWarehouse(sgd); }

	const Goods *GetInventory() const;


	/// Gibt Anzahl der Waren bzw. Figuren zur�ck
	unsigned GetRealWaresCount(GoodType type) const { return real_goods.goods[type]; }
	unsigned GetRealFiguresCount(Job type) const { return real_goods.people[type]; }
	unsigned GetVisualWaresCount(GoodType type) const { return goods.goods[type]; }
	unsigned GetVisualFiguresCount(Job type) const { return goods.people[type]; }

	/// Ver�ndert Ein/Auslagerungseinstellungen (visuell)
	void ChangeVisualInventorySettings(unsigned char category,unsigned char state,unsigned char type)
	{ ((category == 0)?inventory_settings_visual.wares[type]:inventory_settings_visual.figures[type]) ^= state; }
	/// Gibt Ein/Auslagerungseinstellungen zur�ck (visuell)
	bool CheckVisualInventorySettings(unsigned char category,unsigned char state,unsigned char type) const
	{ return ((((category == 0)?inventory_settings_visual.wares[type]:inventory_settings_visual.figures[type]) & state) == state); }
	///// Generiert einen NC-Befehl f�r eine Inventory Settings �nderung und f�hrt noch entsprechend eigene �nderungen aus
	//void SubmitInventorySettings();

	/// Ver�ndert Ein/Auslagerungseinstellungen (real)
	void ChangeRealInventorySetting(unsigned char category,unsigned char state,unsigned char type);
	/// Ver�ndert alle Ein/Auslagerungseinstellungen einer Kategorie (also Waren oder Figuren)(real)
	void ChangeAllRealInventorySettings(unsigned char category,unsigned char state);
	/// Gibt Ein/Auslagerungseinstellungen zur�ck (real)
	bool CheckRealInventorySettings(unsigned char category,unsigned char state,unsigned char type) const
	{ return ((((category == 0)?inventory_settings_real.wares[type]:inventory_settings_real.figures[type]) & state) == state); }

	/// L�sst einen bestimmten Waren/Job-Typ ggf auslagern
	void CheckOuthousing(unsigned char category, unsigned job_ware_id);


	/// Bestellt einen Tr�ger
	void OrderCarrier(noRoadNode* const goal, RoadSegment * workplace);
	/// Bestellt irgendeinen Beruf
	void OrderJob(const Job job, noRoadNode* const goal);
	/// Bestellt einen Esel
	nofCarrier * OrderDonkey(RoadSegment * road,noRoadNode * const goal_flag);
	
	/// Reiht einen Beruf sofort in die Warteschlange zum Rausgehen rein, wenn er da ist und gibt den Pointer auf
	/// ihn zur�ck, wenn keiner da ist, wird 0 zur�ckgegeben
	noFigure * OrderDefender();
	/// "Bestellt" eine Ware --> gibt den Pointer auf die Ware zur�ck
	Ware * OrderWare(const GoodType good, noBaseBuilding * const goal);
	/// Wird von den Lagerhaus-Arbeitern aufgerufen, wenn sie ein Ware wieder zur�ckbringen, die sie vorne nicht ablegen konnten
	void AddWaitingWare(Ware * ware);
	/// Wird aufgerufen, wenn von der Fahne vor dem Geb�ude ein Rohstoff aufgenommen wurde
	bool FreePlaceAtFlag();
	// Eine Ware liegt vor der Flagge des Warenhauses und will rein --> ein Warenhausmitarbeiter muss kommen und sie holen
	void FetchWare();
	// Soll die n�chste Ware nicht holen
	void DontFetchNextWare() {fetch_double_protection = true;}

	/// Legt eine Ware im Lagerhaus ab
	void AddWare(Ware * ware);
	/// Eine Figur geht ins Lagerhaus
	void AddFigure(noFigure * figure);

	void CarryOutWare();
	/// Eine bestellte Ware konnte doch nicht kommen
	void WareLost(Ware * ware);
	/// Bestellte Ware, die sich noch hier drin befindet, storniert ihre Auslieferung
	void CancelWare(Ware * ware);
	/// Bestellte Figur, die sich noch inder Warteschlange befindet, kommt nicht mehr und will rausgehauen werden
	void CancelFigure(noFigure * figure);

	/// Sowas ist bei Warenh�usern nicht n�tig
	unsigned CalcDistributionPoints(noRoadNode * start,const GoodType type) { return 0; }
	/// Wird aufgerufen, wenn eine neue Ware zum dem Geb�ude geliefert wird (nicht wenn sie bestellt wurde vom Geb�ude!)
	void TakeWare(Ware * ware);

	/// F�gt eine Figur hinzu, die auf dem Weg zum Lagerhaus ist
	void AddDependentFigure(noFigure * figure) { dependent_figures.push_back(figure); }
	//// Entfernt eine abh�ngige Figur wieder aus der Liste
	void RemoveDependentFigure(noFigure * figure) { dependent_figures.erase(figure); }

	//// Entfernt eine abh�ngige Ware wieder aus der Liste (wird mit TakeWare hinzugef�gt)
	void RemoveDependentWare(Ware * ware) { dependent_wares.erase(ware); }

	/// Pr�ft, ob es Waren zum Auslagern gibt
	bool AreWaresToEmpty() const;

	/// F�gt aktiven Soldaten (der aus von einer Mission) zum Milit�rgeb�ude hinzu
	void AddActiveSoldier(nofActiveSoldier * soldier);
	/// Gibt Gesamtanzahl aller im Lager befindlichen Soldaten zur�ck
	unsigned GetSoldiersCount() const { return real_goods.people[JOB_PRIVATE]+real_goods.people[JOB_PRIVATEFIRSTCLASS]+
		real_goods.people[JOB_SERGEANT]+real_goods.people[JOB_OFFICER]+real_goods.people[JOB_GENERAL]; }
	/// Bestellt Soldaten
	void OrderTroops(nobMilitary * goal, unsigned count);

	/// Schickt einen Verteidiger raus, der einem Angreifer in den Weg rennt
	nofAggressiveDefender * SendDefender(nofAttacker * attacker);
	/// Wird aufgerufen, wenn ein Soldat nicht mehr kommen kann
	void SoldierLost(nofSoldier * soldier);

	/// Sind noch Truppen drinne, die dieses Geb�ude verteidigen k�nnten?
	bool DefendersAvailable() const;

	/// Ver�ndert Reserveeinstellung - visuell (nur das geforderte nat�rlich)
	void IncreaseReserveVisual(unsigned rank);
	void DecreaseReserveVisual(unsigned rank);
	void IncreaseReserveReal(unsigned rank);
	void DecreaseReserveReal(unsigned rank);

	/// Versucht, die geforderten Reserve-Soldaten bereitzustellen
	void RefreshReserve(unsigned rank);

	/// Gibt Zeiger auf dir Reserve zur�ck f�r das GUI
	const unsigned * GetReservePointerAvailable(unsigned rank) const { return &reserve_soldiers_available[rank]; }
	const unsigned * GetReservePointerClaimed(unsigned rank) const { return &reserve_soldiers_claimed_visual[rank]; }


};	



/// Vorgefertigte Bedingungsfunktionen f�r FindWarehouse, param jeweils Pointer auf die einzelnen Strukturen
namespace FW
{
	struct Param_Ware { GoodType type; unsigned count; };
	bool Condition_Ware(nobBaseWarehouse * wh, const void * param);
	struct Param_Job { Job type; unsigned count; };
	bool Condition_Job(nobBaseWarehouse * wh, const void * param);
	struct Param_WareAndJob { Param_Ware ware; Param_Job job; };
	bool Condition_WareAndJob(nobBaseWarehouse * wh, const void * param);

	bool Condition_Troops(nobBaseWarehouse * wh, const void * param); // param = &unsigned --> count
	bool Condition_StoreWare(nobBaseWarehouse * wh, const void * param); // param = &GoodType -> Warentyp
	bool Condition_StoreFigure(nobBaseWarehouse * wh, const void * param); // param = &Job -> Jobtyp

	bool NoCondition(nobBaseWarehouse * wh, const void * param);
}

#endif
