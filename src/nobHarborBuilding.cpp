// $Id: nobHarborBuilding.cpp 5018 2009-06-08 18:24:25Z OLiver $
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

///////////////////////////////////////////////////////////////////////////////
// Header

#include "main.h"
#include "nobHarborBuilding.h"
#include "GameWorld.h"
#include "Loader.h"
#include "noExtension.h"
#include "MilitaryConsts.h"
#include "GameClient.h"
#include "GameClientPlayer.h"
#include "Ware.h"
#include "EventManager.h"
#include "noShip.h"
#include "noFigure.h"
#include "Random.h"
#include "nobMilitary.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif


nobHarborBuilding::ExpeditionInfo::ExpeditionInfo(SerializedGameData *sgd) :
active(sgd->PopBool()),
boards(sgd->PopUnsignedInt()),
stones(sgd->PopUnsignedInt()),
builder(sgd->PopBool())
{
}

void nobHarborBuilding::ExpeditionInfo::Serialize(SerializedGameData *sgd) const
{
	sgd->PushBool(active);
	sgd->PushUnsignedInt(boards);
	sgd->PushUnsignedInt(stones);
	sgd->PushBool(builder);
}


nobHarborBuilding::nobHarborBuilding(const unsigned short x, const unsigned short y,const unsigned char player,const Nation nation) 
: nobBaseWarehouse(BLD_HARBORBUILDING,x,y,player,nation), orderware_ev(0)
{
	// ins Militärquadrat einfügen
	gwg->GetMilitarySquare(x,y).push_back(this);

	// Alle Waren 0, außer 100 Träger
	memset(&goods,0,sizeof(goods));
	memset(&real_goods,0,sizeof(real_goods));

	// Der Wirtschaftsverwaltung Bescheid sagen
	gwg->GetPlayer(player)->AddWarehouse(this);
	gwg->GetPlayer(player)->AddHarbor(this);

	// Aktuellen Warenbestand zur aktuellen Inventur dazu addieren
	AddToInventory();

	// Evtl gabs verlorene Waren, die jetzt in den Hafen wieder reinkönnen
	gwg->GetPlayer(player)->FindClientForLostWares();

	/// Die Meere herausfinden, an die dieser Hafen grenzt
	for(unsigned i = 0;i<6;++i)
		sea_ids[i] = gwg->IsCoastalPoint(gwg->GetXA(x,y,i), gwg->GetYA(x,y,i));

	// Post versenden
	if(GameClient::inst().GetPlayerID() == this->player)
		GameClient::inst().SendPostMessage(new ImagePostMsgWithLocation(
		_("New storehouse finished"), PMC_GENERAL, x, y, BLD_HARBORBUILDING, nation));
}



void nobHarborBuilding::Destroy()
{
	em->RemoveEvent(orderware_ev);
	players->getElement(player)->HarborDestroyed(this);

	// Der Wirtschaftsverwaltung Bescheid sagen
	gwg->GetPlayer(player)->RemoveWarehouse(this);
	gwg->GetPlayer(player)->RemoveHarbor(this);

	// Baumaterialien in der Inventur verbuchen
	if(expedition.active)
	{
		gwg->GetPlayer(player)->DecreaseInventoryWare(GD_BOARDS,expedition.boards);
		gwg->GetPlayer(player)->DecreaseInventoryWare(GD_STONES,expedition.stones);

		// Und Bauarbeiter (später) rausschicken
		if(expedition.builder)
			++real_goods.people[JOB_BUILDER];
	}

	// Waiting Wares löschen
	for(std::list<Ware*>::iterator it = wares_for_ships.begin();it!=wares_for_ships.end();++it)
	{
		(*it)->WareLost(player);
		delete (*it);
	}
	wares_for_ships.clear();

	// Leute, die noch aufs Schiff warten, rausschicken
	for(std::list<FigureForShip>::iterator it = figures_for_ships.begin();it!=figures_for_ships.end();++it)
	{
		gwg->AddFigure(it->fig,x,y);

		it->fig->Abrogate();
		it->fig->StartWandering();
		it->fig->StartWalking(RANDOM.Rand(__FILE__,__LINE__,obj_id,6));
	}
	figures_for_ships.clear();

	Destroy_nobBaseWarehouse();

	// Land drumherum neu berechnen (nur wenn es schon besetzt wurde!)
	// Nach dem BaseDestroy erst, da in diesem erst das Feuer gesetzt, die Straße gelöscht wird usw.
	gwg->RecalcTerritory(this,HARBOR_ALONE_RADIUS,true, false);

	// Wieder aus dem Militärquadrat rauswerfen
	gwg->GetMilitarySquare(x,y).erase(this);


}

void nobHarborBuilding::Serialize(SerializedGameData * sgd) const
{
	Serialize_nobBaseWarehouse(sgd);
	expedition.Serialize(sgd);
	sgd->PushObject(orderware_ev,true);
	for(unsigned i = 0;i<6;++i)
		sgd->PushUnsignedShort(sea_ids[i]);
	sgd->PushObjectList(wares_for_ships,true);
	sgd->PushUnsignedInt(figures_for_ships.size());
	for(std::list<FigureForShip>::const_iterator it = figures_for_ships.begin();it!=figures_for_ships.end();++it)
	{
		sgd->PushUnsignedShort(it->dest.x);
		sgd->PushUnsignedShort(it->dest.y);
		sgd->PushObject(it->fig,false);
	}

}

nobHarborBuilding::nobHarborBuilding(SerializedGameData * sgd, const unsigned obj_id) 
: nobBaseWarehouse(sgd,obj_id),
	expedition(sgd),
	orderware_ev(sgd->PopObject<EventManager::Event>(GOT_EVENT))
{
	// ins Militärquadrat einfügen
	gwg->GetMilitarySquare(x,y).push_back(this);

	for(unsigned i = 0;i<6;++i)
		sea_ids[i] = sgd->PopUnsignedShort();

	sgd->PopObjectList<Ware>(wares_for_ships,GOT_WARE);

	unsigned count = sgd->PopUnsignedInt();
	for(unsigned i = 0;i<count;++i)
	{
		FigureForShip ffs;
		ffs.dest.x = sgd->PopUnsignedShort();
		ffs.dest.y = sgd->PopUnsignedShort();
		ffs.fig = sgd->PopObject<noFigure>(GOT_UNKNOWN);
		figures_for_ships.push_back(ffs);
	}

}

// Relative Position des Bauarbeiters 
const Point<int> BUILDER_POS[4] = { Point<int>(0,0), Point<int>(0,0), Point<int>(0,15), Point<int>(0,0) };
/// Relative Position der Brettertürme
const Point<int> BOARDS_POS[4] = { Point<int>(0,0), Point<int>(0,0), Point<int>(-50,-5), Point<int>(0,0) };
/// Relative Position der Steintürme
const Point<int> STONES_POS[4] = { Point<int>(0,0), Point<int>(0,0), Point<int>(-50,10), Point<int>(0,0) };

void nobHarborBuilding::Draw(int x,int y)
{
	// Gebäude an sich zeichnen
	DrawBaseBuilding(x,y);

	// Hafenfeuer zeichnen // TODO auch für nicht-römer machen
	if (nation == NAT_ROMANS)
		LOADER.GetNationImageN(nation, 500 + 5 * GameClient::inst().GetGlobalAnimation(8,10,2,obj_id+x+y))->Draw(x,y,0,0,0,0,0,0);

	// Läuft gerade eine Expedition?
	if(expedition.active)
	{
		// Waren für die Expedition zeichnen

		// Bretter
		for(unsigned char i = 0;i<expedition.boards;++i)
			LOADER.GetMapImageN(2200+GD_BOARDS)->Draw(x+BOARDS_POS[nation].x-5,y+BOARDS_POS[nation].y-i*4,0,0,0,0,0,0);
		// Steine
		for(unsigned char i = 0;i<expedition.stones;++i)
			LOADER.GetMapImageN(2200+GD_STONES)->Draw(x+STONES_POS[nation].x+8,y+STONES_POS[nation].y-i*4,0,0,0,0,0,0);

		// Und den Bauarbeiter, falls er schon da ist
		if(expedition.builder)
		{
			unsigned id = GameClient::inst().GetGlobalAnimation(1000,1,10,this->x+this->y);

			const int WALKING_DISTANCE = 30;

			// Wegstrecke, die er von einem Punkt vom anderen schon gelaufen ist
			int walking_distance = (id%500)*WALKING_DISTANCE/500;
			// Id vom laufen
			unsigned walking_id = (id/32)%8;

			int right_point = x - 20 + BUILDER_POS[nation].x;

			if(id < 500)
			{
				LOADER.GetBobN("jobs")->Draw(23,0,false,walking_id,right_point-walking_distance,
					y+BUILDER_POS[nation].y,COLORS[gwg->GetPlayer(player)->color]);
				//DrawShadow(right_point-walking_distance,y,walking_id,0);
			}
			else
			{
				LOADER.GetBobN("jobs")->Draw(23,3,false,walking_id,
					right_point-WALKING_DISTANCE+walking_distance,y+BUILDER_POS[nation].y,
					COLORS[gwg->GetPlayer(player)->color]);
				//DrawShadow(right_point-WALKING_DISTANCE+walking_distance,y,walking_id,0);
			}		
		}
			
	}
}


void nobHarborBuilding::HandleEvent(const unsigned int id)
{
	switch(id)
	{
	// Waren-Bestell-Event
	case 10:
		{
			this->orderware_ev = NULL;
			// Mal wieder schauen, ob es Waren für unsere Expedition gibt
			OrderExpeditionWares();
		} break;
	default: HandleBaseEvent(id);
	}
}

/// Startet eine Expedition oder stoppt sie, wenn bereits eine stattfindet
void nobHarborBuilding::StartExpedition()
{
	// Schon eine Expedition gestartet?
	if(expedition.active)
	{
		// Dann diese stoppen
		expedition.active = false;

		// Waren zurücktransferieren
		real_goods.goods[GD_BOARDS] += expedition.boards;
		goods.goods[GD_BOARDS] += expedition.boards;
		real_goods.goods[GD_STONES] += expedition.stones;
		goods.goods[GD_STONES] += expedition.stones;

		if(expedition.builder)
		{
			++real_goods.people[JOB_BUILDER];
			++goods.people[JOB_BUILDER];
			// Evtl. Abnehmer für die Figur wieder finden
			gwg->GetPlayer(player)->FindWarehouseForAllJobs(JOB_BUILDER);
		}

		return;
	}

	// Initialisierung
	expedition.active = true;
	
	// In unseren Warenbestand gucken und die erforderlichen Bretter und Steine sowie den 
	// Bauarbeiter holen, falls vorhanden
	expedition.boards = min(unsigned(BUILDING_COSTS[nation][BLD_HARBORBUILDING].boards), real_goods.goods[GD_BOARDS]);
	expedition.stones = min(unsigned(BUILDING_COSTS[nation][BLD_HARBORBUILDING].stones), real_goods.goods[GD_STONES]);
	real_goods.goods[GD_BOARDS] -= expedition.boards;
	goods.goods[GD_BOARDS] -= expedition.boards;
	real_goods.goods[GD_STONES] -= expedition.stones;
	goods.goods[GD_STONES] -= expedition.stones;

	if(real_goods.people[JOB_BUILDER])
	{
		expedition.builder = true;
		--real_goods.people[JOB_BUILDER];
		--goods.people[JOB_BUILDER];
	}
	else
	{
		expedition.builder = false;
		// Bauarbeiter bestellen
		gwg->GetPlayer(player)->AddJobWanted(JOB_BUILDER,this);
	}

	// Ggf. Waren bestellen, die noch fehlen
	OrderExpeditionWares();

}


/// Bestellt die zusätzlichen erforderlichen Waren für eine Expedition
void nobHarborBuilding::OrderExpeditionWares()
{
	// Waren in der Bestellungsliste mit beachten
	unsigned boards = 0, stones = 0;
	for(list<Ware*>::iterator it = dependent_wares.begin();it.valid();++it)
	{
		if((*it)->type == GD_BOARDS)
			++boards;
		if((*it)->type == GD_STONES)
			++stones;
	}

	// Prüfen, ob jeweils noch weitere Waren bestellt werden müssen
	unsigned todo_boards = 0;
	if(boards + expedition.boards < BUILDING_COSTS[nation][BLD_HARBORBUILDING].boards)
	{
		todo_boards = BUILDING_COSTS[nation][BLD_HARBORBUILDING].boards - (boards + expedition.boards);
		Ware * ware;
		do
		{
			ware = gwg->GetPlayer(player)->OrderWare(GD_BOARDS,this);
			if(ware)
			{
				dependent_wares.push_back(ware);
				--todo_boards;
			}
		} while(ware && todo_boards);
	}

	unsigned todo_stones = 0;
	if(stones + expedition.stones < BUILDING_COSTS[nation][BLD_HARBORBUILDING].stones)
	{
		todo_stones = BUILDING_COSTS[nation][BLD_HARBORBUILDING].stones - (stones + expedition.stones);
		Ware * ware;
		do
		{
			ware = gwg->GetPlayer(player)->OrderWare(GD_STONES,this);
			if(ware)
			{
				dependent_wares.push_back(ware);
				--todo_stones;
			}
		} while(ware && todo_stones);
	}

	// Wenn immer noch nicht alles da ist, später noch einmal bestellen
	if(orderware_ev == NULL)
		orderware_ev = em->AddEvent(this,210,10);

}


/// Eine bestellte Ware konnte doch nicht kommen
void nobHarborBuilding::WareLost(Ware * ware)
{
	// ggf. neue Waren für Expedition bestellen
	if(expedition.active && (ware->type == GD_BOARDS || ware->type == GD_STONES))
		OrderExpeditionWares();
	RemoveDependentWare(ware);
}



/// Schiff ist angekommen
void nobHarborBuilding::ShipArrived(noShip * ship)
{
	// Verfügbare Aufgaben abklappern

	// Steht Expedition zum Start bereit
	if(expedition.active && expedition.builder 
		&& expedition.boards == BUILDING_COSTS[nation][BLD_HARBORBUILDING].boards
		&& expedition.stones == BUILDING_COSTS[nation][BLD_HARBORBUILDING].stones)
	{
		// Aufräumen am Hafen
		expedition.active = false;
		// Expedition starten
		ship->StartExpedition();
	}
	// Gibt es Waren oder Figuren, die ein Schiff von hier aus nutzen wollen?
	else if(wares_for_ships.size() || figures_for_ships.size())
	{
		// Das Ziel wird nach der ersten Figur bzw. ersten Ware gewählt
		Point<MapCoord> dest;
		if(figures_for_ships.size())
			dest = figures_for_ships.begin()->dest;
		else
			dest = (*wares_for_ships.begin())->GetNextHarbor();


		std::list<noFigure*> figures;

		// Figuren auswählen, die zu diesem Ziel wollen
		for(std::list<FigureForShip>::iterator it = figures_for_ships.begin();
			it!=figures_for_ships.end() && figures.size() < SHIP_CAPACITY[players->getElement(player)->nation];)
		{
			if(it->dest == dest)
			{
				figures.push_back(it->fig);
				it->fig->StartShipJourney(dest);
				--goods.people[it->fig->GetJobType()];
				it = figures_for_ships.erase(it);
				
			}
			else
				++it;
		}

		// Und noch die Waren auswählen
		std::list<Ware*> wares;
		for(std::list<Ware*>::iterator it = wares_for_ships.begin();
			it!=wares_for_ships.end() && figures.size()+wares.size() < SHIP_CAPACITY[players->getElement(player)->nation];)
		{
			if((*it)->GetNextHarbor() == dest)
			{
				wares.push_back(*it);
				(*it)->StartShipJourney();
				--goods.goods[(*it)->type];
				it = wares_for_ships.erase(it);
				
			}
			else
				++it;
		}

		// Und das Schiff starten lassen
		ship->PrepareTransport(dest,figures,wares);
		
	}
}

/// Legt eine Ware im Lagerhaus ab
void nobHarborBuilding::AddWare(Ware * ware)
{
	if(ware->goal != this)
		ware->RecalcRoute();

	// Will diese Ware mit dem Schiff irgendwo hin fahren?
	if(ware->GetNextDir() == SHIP_DIR)
	{
		// Dann fügen wir die mal bei uns hinzu
		AddWareForShip(ware);
		return;
	}

	// Brauchen wir die Ware?
	if(expedition.active)
	{
		if((ware->type == GD_BOARDS && expedition.boards < BUILDING_COSTS[nation][BLD_HARBORBUILDING].boards)
			|| (ware->type == GD_STONES && expedition.stones < BUILDING_COSTS[nation][BLD_HARBORBUILDING].stones))
		{
			if(ware->type == GD_BOARDS) ++expedition.boards;
			else if(ware->type == GD_STONES) ++expedition.stones;

			// Ware nicht mehr abhängig
			RemoveDependentWare(ware);
			// Dann zweigen wir die einfach mal für die Expedition ab
			gwg->GetPlayer(player)->RemoveWare(ware);
			delete ware;

			// Ggf. ist jetzt alles benötigte da
			CheckExpeditionReady();
			return;
		}
	}

	nobBaseWarehouse::AddWare(ware);

}

/// Eine Figur geht ins Lagerhaus
void nobHarborBuilding::AddFigure(noFigure * figure)
{
	// Brauchen wir einen Bauarbeiter für die Expedition?
	if(figure->GetJobType() == JOB_BUILDER && expedition.active && !expedition.builder)
	{
		
		RemoveDependentFigure(figure);
		em->AddToKillList(figure);

		expedition.builder = true;
		// Ggf. ist jetzt alles benötigte da
		CheckExpeditionReady();
	}
	else
		// ansonsten weiterdelegieren
		nobBaseWarehouse::AddFigure(figure);
}

/// Gibt zurück, ob Expedition vollständig ist
bool nobHarborBuilding::IsExpeditionReady() const
{
	if(!expedition.active)
		return false;
	// Alles da?
	if(expedition.boards < BUILDING_COSTS[nation][BLD_HARBORBUILDING].boards)
		return false;
	if(expedition.stones < BUILDING_COSTS[nation][BLD_HARBORBUILDING].stones)
		return false;
	if(!expedition.builder)
		return false;

	return true;
}

/// Prüft, ob eine Expedition von den Waren her vollständig ist und ruft ggf. das Schiff
void nobHarborBuilding::CheckExpeditionReady()
{
	// Alles da?
	// Dann bestellen wir mal das Schiff
	if(IsExpeditionReady())
		players->getElement(player)->OrderShip(this);
}



/// Schiff konnte nicht mehr kommen
void nobHarborBuilding::ShipLost(noShip * ship)
{
	// Neues Schiff bestellen
	players->getElement(player)->OrderShip(this);
}

/// Gibt die Hafenplatz-ID zurück, auf der der Hafen steht
unsigned nobHarborBuilding::GetHarborPosID() const
{
	return gwg->GetHarborPointID(x,y);
}

/// Abfangen, wenn ein Mann nicht mehr kommen kann --> könnte ein Bauarbeiter sein und
/// wenn wir einen benötigen, müssen wir einen neuen bestellen
void nobHarborBuilding::RemoveDependentFigure(noFigure * figure)
{
	nobBaseWarehouse::RemoveDependentFigure(figure);
	// Ist das ein Bauarbeiter und brauchen wir noch einen
	if(figure->GetJobType() == JOB_BUILDER && expedition.active && !expedition.builder)
	{
		// Alle Figuren durchkommen, die noch hierher kommen wollen und gucken, ob ein 
		// Bauarbeiter dabei ist
		for(list<noFigure*>::iterator it = dependent_figures.begin();it.valid();++it)
		{
			if((*it)->GetJobType() == JOB_BUILDER)
				// Brauchen keinen bestellen, also raus
				return;
		}

		// Keinen gefunden, also müssen wir noch einen bestellen
		players->getElement(player)->AddJobWanted(JOB_BUILDER,this);
	}

	
}

/// Gibt eine Liste mit möglichen Verbindungen zurück
void nobHarborBuilding::GetShipConnections(std::vector<ShipConnection>& connections) const
{
	std::vector<nobHarborBuilding*> harbor_buildings;
	for(unsigned short sea_id = 0;sea_id<6;++sea_id)
	{
		if(sea_ids[sea_id] != 0)
			players->getElement(player)->GetHarborBuildings(harbor_buildings,sea_ids[sea_id]);
	}

	for(unsigned i = 0;i<harbor_buildings.size();++i)
	{
		ShipConnection sc;
		sc.dest = harbor_buildings[i];
		sc.way_costs = 200; // todo
		connections.push_back(sc);
	}
}


/// Fügt einen Mensch hinzu, der mit dem Schiff irgendwo hin fahren will
void nobHarborBuilding::AddFigureForShip(noFigure * fig, Point<MapCoord> dest)
{
	FigureForShip ffs = { fig, dest };
	figures_for_ships.push_back(ffs);
	players->getElement(player)->OrderShip(this);
	// Anzahl visuell erhöhen
	++goods.people[fig->GetJobType()];
}

/// Fügt eine Ware hinzu, die mit dem Schiff verschickt werden soll
void nobHarborBuilding::AddWareForShip(Ware * ware)
{
	wares_for_ships.push_back(ware);
	// Anzahl visuell erhöhen
	++goods.goods[ConvertShields(ware->type)];
	players->getElement(player)->OrderShip(this);
	ware->WaitForShip(this);
}

/// Gibt Anzahl der Schiffe zurück, die noch für ausstehende Aufgaben benötigt werden
unsigned nobHarborBuilding::GetNeededShipsCount() const
{
	unsigned count = 0;

	// Expedition -> 1 Schiff
	if(IsExpeditionReady())
		++count;
	// Evtl. Waren und Figuren -> noch ein Schiff
	if(figures_for_ships.size() > 0 || wares_for_ships.size() > 0)
		++count;

	return count;
}

/// Gibt die Wichtigkeit an, dass ein Schiff kommen muss (0 -> keine Bedürftigkeit)
int nobHarborBuilding::GetNeedForShip(unsigned ships_coming) const
{
	int points = 0;

	// Expedition -> 1 Schiff
	if(IsExpeditionReady())
	{
		if(ships_coming == 0)
			points += 100;
		else
			--ships_coming;
	}
	if((figures_for_ships.size() > 0 || wares_for_ships.size() > 0) && ships_coming == 0)
		points += (figures_for_ships.size()+wares_for_ships.size())*5;

	return points;
}


/// Abgeleitete kann eine gerade erzeugte Ware ggf. sofort verwenden 
/// (muss in dem Fall true zurückgeben)
bool nobHarborBuilding::UseWareAtOnce(Ware * ware, noBaseBuilding* const goal)
{
	// Evtl. muss die Ware gleich das Schiff nehmen -> 
	// dann zum Schiffsreservoir hinzufügen
	Point<MapCoord> next_harbor;
	ware->RecalcRoute();
	if(ware->GetNextDir() == SHIP_DIR)
	{
		// Dann fügen wir die mal bei uns hinzu
		AddWareForShip(ware);

		return true;
	}

	return false;
}


/// Dasselbe für Menschen
bool nobHarborBuilding::UseFigureAtOnce(noFigure * fig, noRoadNode* const goal)
{
	// Evtl. muss die Ware gleich das Schiff nehmen -> 
	// dann zum Schiffsreservoir hinzufügen
	Point<MapCoord> next_harbor;
	if(gwg->FindHumanPathOnRoads(this,goal,NULL,&next_harbor) == SHIP_DIR)
	{
		// Dann fügen wir die mal bei uns hinzu
		AddFigureForShip(fig,next_harbor);
		return true;
	}

	return false;
}

/// Erhält die Waren von einem Schiff und nimmt diese in den Warenbestand auf
void nobHarborBuilding::ReceiveGoodsFromShip(const std::list<noFigure*> figures, const std::list<Ware*> wares)
{
	// Menschen zur Ausgehliste hinzufügen
	for(std::list<noFigure*>::const_iterator it = figures.begin();it!=figures.end();++it)
	{
		++goods.people[(*it)->GetJobType()];

		// Wenn es kein Ziel mehr hat, sprich keinen weiteren Weg, kann es direkt hier gelagert
		// werden
		if((*it)->HasNoGoal())
		{
			RemoveDependentFigure(*it);
			++real_goods.people[(*it)->GetJobType()];
			em->AddToKillList(*it);
		}
		else
		{
			AddLeavingFigure(*it);
			(*it)->ShipJourneyEnded();
		}
		
	}

	// Waren zur Warteliste hinzufügen
	for(std::list<Ware*>::const_iterator it = wares.begin();it!=wares.end();++it)
	{
		// Optische Warenwerte entsprechend erhöhen
		++goods.goods[ConvertShields((*it)->type)];
		if((*it)->ShipJorneyEnded(this))
		{
			
			// Ware will die weitere Reise antreten, also muss sie zur Liste der rausgetragenen Waren
			// hinzugefügt werden
			waiting_wares.push_back(*it);
		}
		else
		{
			// Ansonsten fügen wir die Ware einfach zu unserem Inventar dazu
			RemoveDependentWare(*it);
			++real_goods.goods[ConvertShields((*it)->type)];
			players->getElement(player)->RemoveWare(*it);
			delete *it;
		}
	}

	// Ggf. neues Rausgeh-Event anmelden, was notwendig ist, wenn z.B. nur Waren zur Liste hinzugefügt wurden
	AddLeavingEvent();
}

/// Storniert die Bestellung für eine bestimmte Ware, die mit einem Schiff transportiert werden soll
void nobHarborBuilding::CancelWareForShip(Ware* ware)
{
	// Ware aus der Liste entfernen
	wares_for_ships.remove(ware);
	// Ware zur Inventur hinzufügen
	// Anzahl davon wieder hochsetzen
	++real_goods.goods[ConvertShields(ware->type)];
}

/// Bestellte Figur, die sich noch inder Warteschlange befindet, kommt nicht mehr und will rausgehauen werden
void nobHarborBuilding::CancelFigure(noFigure * figure)
{
	// Merken, ob sie entfernt wurde
	bool removed = false;
	// Figur ggf. aus der List entfernen
	for(std::list<FigureForShip>::iterator it = figures_for_ships.begin();it!=figures_for_ships.end();++it)
	{
		if(it->fig == figure)
		{
			figures_for_ships.erase(it);
			removed = true;
			break;
		}
	}

	// Wurde sie entfernt?
	if(removed)
	{
		// Dann zu unserem Inventar hinzufügen und anschließend vernichten
		++real_goods.people[figure->GetJobType()];
		//em->AddToKillList(figure);
	}
	// An Basisklasse weiterdelegieren
	else
		nobBaseWarehouse::CancelFigure(figure);
	
}


/// Gibt die Anzahl der Angreifer zurück, die dieser Hafen für einen Seeangriff zur Verfügung stellen kann
unsigned nobHarborBuilding::GetAttackersForSeaAttack() const
{
	list<nobBaseMilitary*> buildings;
	gwg->LookForMilitaryBuildings(buildings,x,y,3);

	// Und zählen
	unsigned soldiers_count = 0;
	for(list<nobBaseMilitary*>::iterator it = buildings.begin();it.valid();++it)
	{
		if((*it)->GetGOT() == GOT_NOB_MILITARY)
			soldiers_count += static_cast<nobMilitary*>(*it)->GetSoldiersForAttack(x,y,player);
	}

	return soldiers_count;
}


