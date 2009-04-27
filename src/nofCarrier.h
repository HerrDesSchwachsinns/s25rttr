// $Id: nofCarrier.h 4652 2009-03-29 10:10:02Z FloSoft $
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

#ifndef NOF_CARRIER_H_
#define NOF_CARRIER_H_

#include "noFigure.h"
#include "GlobalVars.h"

class RoadSegment;
class Ware;
class noRoadNode;

enum CarrierState
{
	CARRS_FIGUREWORK = 0, // Aufgaben der Figur
	CARRS_WAITFORWARE, // auf Weg auf Ware warten
	CARRS_GOTOMIDDLEOFROAD, // zur Mitte seines Weges gehen
	CARRS_FETCHWARE, // Ware holen
	CARRS_CARRYWARE, // Ware zur Flagge tragen
	CARRS_CARRYWARETOBUILDING, // Ware zum Geb�ude schaffen
	CARRS_LEAVEBUILDING, // kommt aus Geb�ude wieder raus (bzw kommt von Baustelle zur�ck) zum Weg
	CARRS_WAITFORWARESPACE, // wartet vor der Flagge auf einen freien Platz
	CARRS_GOBACKFROMFLAG // geht von der Flagge zur�ck, weil kein Platz mehr frei war


};

// Stellt einen Tr�cer da
class nofCarrier : public noFigure
{
public:

	/// Tr�ger-"Typ"
	enum CarrierType
	{
		CT_NORMAL, // Normaler Tr�ger
		CT_DONKEY, // Esel
		CT_BOAT // Tr�ger mit Boot
	};

private:

	CarrierType ct;
	/// Was der Tr�ger gerade so treibt
	CarrierState state;
	/// Ist er dick?
	bool fat;
	// Weg, auf dem er arbeitet
	RoadSegment * workplace;
	/// Ware, die er gerade tr�gt (0 = nichts)
	Ware * carried_ware;
	/// Rechne-Produktivit�t-aus-Event
	EventManager::EventPointer productivity_ev;
	// Letzte errechnete Produktivit�t
	unsigned productivity;
	/// Wieviel GF von einer bestimmten Anzahl in diesem Event-Zeitraum gearbeitet wurde
	unsigned worked_gf;
	/// Zu welchem GF das letzte Mal das Arbeiten angefangen wurde
	unsigned since_working_gf;
	/// Bestimmt GF der n�chsten Tr�geranimation
	unsigned next_animation;

private:

	void GoalReached();
	void Walked();
	void AbrogateWorkplace();
	void HandleDerivedEvent(const unsigned int id);

	/// Nach dem Tragen der Ware, guckt der Tr�ger an beiden Flagge, obs Waren gibt, holt/tr�gt diese ggf oder geht ansonsten wieder in die Mitte
	void LookForWares();
	/// Nimmt eine Ware auf an der aktuellen Flagge und dreht sich um, um sie zu tragen (fetch_dir ist die Richtung der Waren, die der Tr�ger aufnehmen will)
	void FetchWare(const bool swap_wares);

	/// Pr�ft, ob die getragene Ware dann von dem Weg zum Geb�ude will
	bool WantInBuilding();

	/// F�r Produktivit�tsmessungen: f�ngt an zu arbeiten
	void StartWorking();
	/// F�r Produktivit�tsmessungen: h�rt auf zu arbeiten
	void StopWorking();

	/// Bestimmt neuen Animationszeitpunkt
	void SetNewAnimationMoment();
	
public:

	nofCarrier(const CarrierType ct, const unsigned short x, const unsigned short y,const unsigned char player,RoadSegment * workplace,noRoadNode * const goal);
	nofCarrier(SerializedGameData * sgd, const unsigned obj_id);

	~nofCarrier();

	/// Serialisierungsfunktionen
	protected:	void Serialize_nofCarrier(SerializedGameData * sgd) const;
	public:		void Serialize(SerializedGameData *sgd) const { Serialize_nofCarrier(sgd); }

		/// Aufr�ummethoden
protected:	void Destroy_nofCarrier();
public:		void Destroy() { Destroy_nofCarrier(); }

	GO_Type GetGOT() const { return GOT_NOF_CARRIER; }

	/// Gibt Tr�ger-Typ zur�ck
	CarrierType GetCarrierType() const { return ct; }
	/// Was macht der Tr�ger gerade?
	CarrierState GetCarrierState() const { return state; }
	/// Gibt Tr�ger-Produktivit�t in % zur�ck
	unsigned GetProductivity() const { return productivity; }

	void Draw(int x, int y);

	/// Wird aufgerufen, wenn der Weg des Tr�gers abgerissen wurde
	void LostWork();

	/// Wird aufgerufen, wenn der Arbeitsplatz des Tr�gers durch eine Flagge geteilt wurde
	/// der Tr�ger sucht sich damit einen der beiden als neuen Arbeitsplatz, geht zur Mitte und ruft einen neuen Tr�ger
	/// f�r das 2. Wegst�ck
	void RoadSplitted(RoadSegment * rs1, RoadSegment * rs2);

	/// Sagt dem Tr�ger Bescheid, dass es an einer Flagge noch eine Ware zu transportieren gibt
	bool AddWareJob(const noRoadNode * rn);
	/// Das Gegnteil von AddWareJob: wenn eine Ware nicht mehr transportiert werden will, sagt sie dem Tr�ger Bescheid,
	/// damit er nicht unn�tig dort hinl�uft zur Flagge
	void RemoveWareJob();

	/// Benachrichtigt den Tr�ger, wenn an einer auf seinem Weg an einer Flagge wieder ein freier Platz ist
	/// gibt zur�ck, ob der Tr�ger diesen freien Platz auch nutzen wird
	bool SpaceAtFlag(const bool flag);

	/// Gibt erste Flagge des Arbeitsweges zur�ck, falls solch einer existiert
	noRoadNode * GetFirstFlag() const;
	noRoadNode * GetSecondFlag() const;

};

#endif
