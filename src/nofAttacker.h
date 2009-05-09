// $Id: nofAttacker.h 4842 2009-05-09 11:53:45Z OLiver $
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
#ifndef NOF_ATTACKER_H_
#define NOF_ATTACKER_H_

#include "nofActiveSoldier.h"

class nofDefender;
class nofAggressiveDefender;
class nofPassiveSoldier;

/// Angreifender Soldat
class nofAttacker : public nofActiveSoldier
{
	// Unsere Feind-Freunde ;)
	friend class nofAggressiveDefender;
	friend class nofDefender;

private:
	/// Geb�ude, welches der Soldat angreift
	nobBaseMilitary * attacked_goal;
	/// Ist er schon von einem Verteidiger ins Visier genommen wurden? Wenn ja, dann ist das hier
	/// der Pointer zu diesem, wenn nicht, dann 0
	/// kann auch der aggressive Verteidiger sein!
	nofActiveSoldier * defender;
	/// Soll er von nem Verteidiger gejagt werden? (wenn nicht wurde er schon gejagt oder er soll
	/// wegen den Milit�reinstellungen nicht gejagt werden
	bool should_haunted;
	/// In welchem Radius steht der Soldat, wenn er um eine Fahne herum wartet?
	unsigned short radius;
	/// Nach einer bestimmten Zeit, in der der Angreifer an der Flagge des Geb�udes steht, blockt er den Weg
	/// nur benutzt bei STATE_ATTACKING_WAITINGFORDEFENDER
	EventManager::EventPointer blocking_event;

private:

	/// wenn man gelaufen ist
	void Walked();
	/// Geht nach Hause f�r Attacking-Missoin
	void ReturnHomeMissionAttacking();
	/// L�uft weiter
	void MissAttackingWalk();

	/// Geht zum STATE_ATTACKING_WAITINGFORDEFENDER �ber und meldet gleichzeitig ein Block-Event an
	void SwitchStateAttackingWaitingForDefender();

	/// Sagt den verschiedenen Zielen Bescheid, dass wir doch nicht mehr kommen k�nnen
	void InformTargetsAboutCancelling();

public:


	nofAttacker(nofPassiveSoldier * other,nobBaseMilitary * const attacked_goal);
	nofAttacker(SerializedGameData * sgd, const unsigned obj_id);
	~nofAttacker();

	/// Aufr�ummethoden
protected:	void Destroy_nofAttacker();
public:		void Destroy() { Destroy_nofAttacker(); }

	/// Serialisierungsfunktionen
	protected:	void Serialize_nofAttacker(SerializedGameData * sgd) const;
	public:		void Serialize(SerializedGameData *sgd) const { Serialize_nofAttacker(sgd); }

	GO_Type GetGOT() const { return GOT_NOF_ATTACKER; }

	void HandleDerivedEvent(const unsigned int id);
	/// Blockt der Angreifer noch?
	bool IsBlockingRoads() const;

	/// Wenn ein Heimat-Milit�rgeb�ude bei Missionseins�tzen zerst�rt wurde
	void HomeDestroyed();
	/// Wenn er noch in der Warteschleife vom Ausgangsgeb�ude h�ngt und dieses zerst�rt wurde
	void HomeDestroyedAtBegin();
	/// Wenn ein Kampf gewonnen wurde
	void WonFighting();
	/// Wenn ein Kampf verloren wurde (Tod)
	void LostFighting();

	/// Sagt einem angreifenden Soldaten, dass es das Ziel nich mehr gibt
	void AttackedGoalDestroyed();
	/// aggressiv Verteidigender Soldat kann nich mehr kommen
	void AggressiveDefenderLost();
	/// Der Angreifer, der gerade um die Fahne wartet soll zur Fahne laufen, wo der Verteidiger wartet und dort k�mpfen
	bool AttackFlag(nofDefender * defender);
	/// Der Platz von einem anderen Kampf wurde frei --> angreifender Soldat soll hinlaufen (KEIN Verteidiger da!)
	void AttackFlag();
	/// Ein um-die-Flagge-wartender Angreifer soll, nachdem das Milit�rgeb�ude besetzt wurde, ebenfalls mit reingehen
	void CaptureBuilding();
	/// Siehe oben, wird nach jeder Wegeinheit aufgerufen
	void CapturingWalking();
	/// Ein um-die-Flagge-wartender Angreifer soll wieder nach Hause gehen, da das eingenommene Geb�ude bereits
	/// voll besetzt ist
	void CapturedBuildingFull();
	/// Gibt den Radius von einem um-eine-Fahne-herum-wartenden angreifenden Soldaten zur�ck
	unsigned GetRadius() const { return radius; }
	/// Ein um-die-Flagge-Swartender Angreifer soll auf einen frei gewordenen Platz nachr�cken, damit keine
	/// L�cken entstehen
	void StartSucceeding(const unsigned short x, const unsigned short y, const unsigned short new_radius, const unsigned char dir);
	/// Siehe oben, wird nach jeder Wegeinheit aufgerufen
	void SucceedingWalk();

	/// aggressiv-verteidigender Soldat fragt einen Angreifer, ob er Lust zum K�mpfen h�tte
	bool WannaFight() const { return (!defender && (state == STATE_ATTACKING_WALKINGTOGOAL
		|| state == STATE_ATTACKING_WAITINGAROUNDBUILDING )); }
	/// aggressiv-verteidigender Soldat will mit einem Angreifer k�mpfen oder umgekehrt
	void LetsFight(nofAggressiveDefender * other);

	/// Fragt, ob ein Angreifender Soldat vor dem Geb�ude wartet und k�mpfen will
	bool IsAttackerReady() const { return (state == STATE_ATTACKING_WAITINGAROUNDBUILDING); }

	/// Liefert das angegriffene Geb�ude zur�ck
	nobBaseMilitary * GetAttackedGoal() const { return attacked_goal; }

	/// Fragt ab, ob der Angreifer direkt vor einem Geb�ude wartet, dieses angreift und ob gerade
	/// der Verteidiger des Geb�udes zur T�r rauskommt und wenn ja, ob ein wartender vor der Flagge, der am
	/// potenziellen Kampf vorbei will, es noch schafft vorbeizukommen, bevor der Kampf wieder anf�ngt
	bool CanPassBeforeFight() const;


};


#endif // !NOF_ATTACKER_H_
