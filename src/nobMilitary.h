// $Id: nobMilitary.h 4902 2009-05-21 09:12:10Z OLiver $
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

#ifndef NOB_MILITARYBUILDING_H_
#define NOB_MILITARYBUILDING_H_

#include "nobBaseMilitary.h"

class nofPassiveSoldier;
class nofActiveSoldier;
class nofAttacker;
class nofAggressiveDefender;
class nofDefender;
class Ware;
class iwMilitaryBuilding;

/// Stellt ein Milit�rgeb�ude beliebiger Gr��e (also von Baracke bis Festung) dar
class nobMilitary : public nobBaseMilitary
{
	/// wurde das Geb�ude gerade neu gebaut (muss also die Landgrenze beim Eintreffen von einem Soldaten neu berechnet werden?)
	bool new_built;
	/// Anzahl der Goldm�nzen im Geb�ude
	unsigned char coins;
	/// Gibt an, ob Goldm�nzen gesperrt worden (letzteres nur visuell, um Netzwerk-Latenzen zu verstecken)
	bool disable_coins, disable_coins_virtual;
	/// Entfernung zur freindlichen Grenze (woraus sich dann die Besatzung ergibt) von 0-2, 0 fern, 2 nah
	unsigned char frontier_distance;
	/// Gr��e bzw Typ des Milit�rgeb�udes (0 = Baracke, 3 = Festung)
	unsigned char size;
	/// Bestellte Soldaten
	list<nofPassiveSoldier*> ordered_troops;
	/// Bestellter Goldm�nzen
	list<Ware*> ordered_coins;
	/// Gibt an, ob gerade die Eroberer in das Geb�ude gehen (und es so nicht angegegriffen werden sollte)
	bool capturing;
	/// Anzahl der Soldaten, die das Milit�rgeb�ude gerade noch einnehmen
	unsigned capturing_soldiers;
	/// Gold-Bestell-Event
	EventManager::EventPointer goldorder_event;
	/// Bef�rderung-Event
	EventManager::EventPointer upgrade_event;

public:

	/// Soldatenbesatzung
	list<nofPassiveSoldier*> troops;
	
	// Das Fenster braucht ja darauf Zugriff
	friend class iwMilitaryBuilding;

private:

	/// Bestellungen (sowohl Truppen als auch Goldm�nzen) zur�cknehmen
	void CancelOrders();
	/// W�hlt je nach Milit�reinstellungen (Verteidigerst�rke) einen passenden Soldaten aus
	list<nofPassiveSoldier*>::iterator ChooseSoldier();
	/// Stellt Verteidiger zur Verf�gung
	nofDefender * ProvideDefender(nofAttacker * const attacker);
	/// Will/kann das Geb�ude noch M�nzen bekommen?
	bool WantCoins();
	/// Pr�ft, ob Goldm�nzen und Soldaten, die bef�rdert werden k�nnen, vorhanden sind und meldet ggf. ein
	/// Bef�rderungsevent an
	void PrepareUpgrading();

public:

	nobMilitary(const BuildingType type,const unsigned short x, const unsigned short y,const unsigned char player,const Nation nation);
	nobMilitary(SerializedGameData * sgd, const unsigned obj_id);

	~nobMilitary();

	/// Aufr�ummethoden
protected:	void Destroy_nobMilitary();
public:		void Destroy() { Destroy_nobMilitary(); }

	/// Serialisierungsfunktionen
protected: void Serialize_nobMilitary(SerializedGameData * sgd) const;
public: void Serialize(SerializedGameData *sgd) const { Serialize_nobMilitary(sgd); }

	GO_Type GetGOT() const { return GOT_NOB_MILITARY; }

	void Draw(int x, int y);
	void HandleEvent(const unsigned int id);

	/// Wurde das Milit�rgeb�ude neu gebaut und noch nicht besetzt und kann somit abgerissen werden bei Land-verlust?
	bool IsNewBuilt() const { return new_built; }

	/// Liefert Milit�rradius des Geb�udes
	MapCoord GetMilitaryRadius() const;

	/// Sucht feindliche Miit�rgeb�ude im Umkreis und setzt die frontier_distance entsprechend (sowohl selber als
	/// auch von den feindlichen Geb�uden) und bestellt somit ggf. neue Soldaten
	void LookForEnemyBuildings();
	
	/// Wird von gegnerischem Geb�ude aufgerufen, wenn sie neu gebaut worden sind und es so ein neues Geb�ude im Umkreis gibt
	/// setzt frontier_distance neu falls m�glich und sendet ggf. Verst�rkung
	void NewEnemyMilitaryBuilding(const unsigned short distance);
	/// Gibt Distanz zur�ck
	unsigned char GetFrontierDistance() const { return frontier_distance; }

	/// Berechnet die gew�nschte Besatzung je nach Grenzn�he
	int CalcTroopsCount();
	/// Reguliert die Besatzung des Geb�udes je nach Grenzn�he, bestellt neue Soldaten und schickt �berfl�ssige raus
	void RegulateTroops();
	/// Gibt aktuelle Besetzung zur�ck
	unsigned GetTroopsCount() const { return troops.size(); }
	

	/// Wird aufgerufen, wenn eine neue Ware zum dem Geb�ude geliefert wird (in dem Fall nur Goldst�cke)
	void TakeWare(Ware * ware);
	/// Legt eine Ware am Objekt ab (an allen Stra�enknoten (Geb�ude, Baustellen und Flaggen) kann man Waren ablegen
	void AddWare(Ware * ware);
	/// Eine bestellte Ware konnte doch nicht kommen
	void WareLost(Ware * ware);
	/// Wird aufgerufen, wenn von der Fahne vor dem Geb�ude ein Rohstoff aufgenommen wurde
	bool FreePlaceAtFlag();

	/// Berechnet, wie dringend eine Goldm�nze gebraucht wird, in Punkten, je h�her desto dringender
	unsigned CalcCoinsPoints();

	/// Wird aufgerufen, wenn ein Soldat kommt
	void GotWorker(Job job, noFigure * worker);
	/// F�gt aktiven Soldaten (der aus von einer Mission) zum Milit�rgeb�ude hinzu
	void AddActiveSoldier(nofActiveSoldier * soldier);
	/// F�gt passiven Soldaten (der aus einem Lagerhaus kommt) zum Milit�rgeb�ude hinzu
	void AddPassiveSoldier(nofPassiveSoldier * soldier);
	/// Soldat konnte nicht kommen
	void SoldierLost(nofSoldier * soldier);
	/// Soldat ist jetzt auf Mission
	void SoldierOnMission(nofPassiveSoldier * passive_soldier,nofActiveSoldier * active_soldier);

	/// Schickt einen Verteidiger raus, der einem Angreifer in den Weg rennt
	nofAggressiveDefender * SendDefender(nofAttacker * attacker);

	/// Geb�ude wird vom Gegner eingenommen, player ist die neue Spieler-ID
	void Capture(const unsigned char new_owner);
	/// Das Geb�ude wurde bereits eingenommen, hier wird gepr�ft, ob noch weitere Soldaten f�r die Besetzung
	/// notwendig sind, wenn ja wird ein neuer Soldat gerufen, wenn nein, werden alle restlichen nach Hause
	/// geschickt
	void NeedOccupyingTroops(const unsigned char new_owner);
	/// Sagt dem Geb�ude schonmal, dass es eingenommen wird, wenn er erste Eroberer gerade in das Geb�ude reinl�uft
	/// (also noch bevor er drinnen ist!) - damit da nicht zus�tzliche Soldaten reinlaufen
	void PrepareCapturing() { capturing = true; ++capturing_soldiers; }

	/// Wird das Geb�ude gerade eingenommen?
	bool IsCaptured() const { return capturing; }
	/// Geb�ude wird nicht mehr eingenommen (falls anderer Soldat zuvor reingekommen ist beim Einnehmen)
	void StopCapturing() { capturing = false; }
	/// Sagt, dass ein erobernder Soldat das Milit�rgeb�ude erreicht hat
	void CapturingSoldierArrived() { --capturing_soldiers; } 

	/// Stoppt/Erlaubt Goldzufuhr (visuell)
	void StopGoldVirtual() { disable_coins_virtual = !disable_coins_virtual; }
	/// Stoppt/Erlaubt Goldzufuhr (real)
	void StopGold();
	/// Fragt ab, ob Goldzufuhr ausgeschaltet ist (visuell)
	bool IsGoldDisabledVirtual() { return disable_coins_virtual; }
	/// Fragt ab, ob Goldzufuhr ausgeschaltet ist (real)
	bool IsGoldDisabled() { return disable_coins; }

	/// Sucht s�mtliche Lagerh�user nach Goldm�nzen ab und bestellt ggf. eine, falls eine gebraucht wird
	void SearchCoins();

	/// Geb�ude wird von einem Katapultstein getroffen
	void HitOfCatapultStone();

	/// Sind noch Truppen drinne, die dieses Geb�ude verteidigen k�nnen
	bool DefendersAvailable() const { return (GetTroopsCount() > 0); }

	/// Darf das Milit�rgeb�ude abgerissen werden (Abriss-Verbot ber�cksichtigen)?
	bool IsDemolitionAllowed() const;
};


#endif
