// $Id: nobHarborBuilding.cpp 6262 2010-04-03 22:05:03Z OLiver $
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
#include "nofAttacker.h"

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
	// ins Milit�rquadrat einf�gen
	gwg->GetMilitarySquare(x,y).push_back(this);

	// Alle Waren 0, au�er 100 Tr�ger
	memset(&goods,0,sizeof(goods));
	memset(&real_goods,0,sizeof(real_goods));

	// Der Wirtschaftsverwaltung Bescheid sagen
	gwg->GetPlayer(player)->AddWarehouse(this);
	gwg->GetPlayer(player)->AddHarbor(this);

	// Aktuellen Warenbestand zur aktuellen Inventur dazu addieren
	AddToInventory();


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

		// Und Bauarbeiter (sp�ter) rausschicken
		if(expedition.builder)
			++real_goods.people[JOB_BUILDER];
	}

	// Waiting Wares l�schen
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
	// Nach dem BaseDestroy erst, da in diesem erst das Feuer gesetzt, die Stra�e gel�scht wird usw.
	gwg->RecalcTerritory(this,HARBOR_ALONE_RADIUS,true, false);

	// Wieder aus dem Milit�rquadrat rauswerfen
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
	// ins Milit�rquadrat einf�gen
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
const Point<int> BUILDER_POS[4] = { Point<int>(0,18), Point<int>(-8,17), Point<int>(0,15), Point<int>(-18,17) };
/// Relative Position der Brettert�rme
const Point<int> BOARDS_POS[4] = { Point<int>(-70,-5), Point<int>(-55,-5), Point<int>(-50,-5), Point<int>(-60,-5) };
/// Relative Position der Steint�rme
const Point<int> STONES_POS[4] = { Point<int>(-73,10), Point<int>(-60,10), Point<int>(-50,10), Point<int>(-60,10) };
/// Relative Postion der Hafenfeuer
const Point<int> FIRE_POS[4] = { Point<int>(36,-51), Point<int>(0,0), Point<int>(0,0), Point<int>(5,-80) };

void nobHarborBuilding::Draw(int x,int y)
{
	// Geb�ude an sich zeichnen
	DrawBaseBuilding(x,y);

	// Hafenfeuer zeichnen // TODO auch f�r nicht-r�mer machen
	if (nation == NAT_ROMANS || nation == NAT_JAPANESES)
	{
		LOADER.GetNationImageN(nation, 500 + 5 * GameClient::inst().GetGlobalAnimation(8,2,1,obj_id+x+y))->Draw(x+FIRE_POS[nation].x,y+FIRE_POS[nation].y,0,0,0,0,0,0);
	}
	else if (nation == NAT_AFRICANS || nation == NAT_VIKINGS)
	{
		LOADER.GetMapImageN(740+GameClient::inst().GetGlobalAnimation(8,5,2,obj_id+x+y))->Draw(x+FIRE_POS[nation].x,y+FIRE_POS[nation].y);
	}

	// L�uft gerade eine Expedition?
	if(expedition.active)
	{
		// Waren f�r die Expedition zeichnen

		// Bretter
		for(unsigned char i = 0;i<expedition.boards;++i)
			LOADER.GetMapImageN(2200+GD_BOARDS)->Draw(x+BOARDS_POS[nation].x-5,y+BOARDS_POS[nation].y-i*4,0,0,0,0,0,0);
		// Steine
		for(unsigned char i = 0;i<expedition.stones;++i)
			LOADER.GetMapImageN(2200+GD_STONES)->Draw(x+STONES_POS[nation].x+8,y+STONES_POS[nation].y-i*4,0,0,0,0,0,0);

		// Und den Bauarbeiter, falls er schon da ist
		if(expedition.builder)
		{
			unsigned id = GameClient::inst().GetGlobalAnimation(1000,7,1,this->x+this->y);

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
			// Mal wieder schauen, ob es Waren f�r unsere Expedition gibt
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

		// Waren zur�cktransferieren
		real_goods.goods[GD_BOARDS] += expedition.boards;
		goods.goods[GD_BOARDS] += expedition.boards;
		real_goods.goods[GD_STONES] += expedition.stones;
		goods.goods[GD_STONES] += expedition.stones;

		if(expedition.builder)
		{
			++real_goods.people[JOB_BUILDER];
			++goods.people[JOB_BUILDER];
			// Evtl. Abnehmer f�r die Figur wieder finden
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


/// Bestellt die zus�tzlichen erforderlichen Waren f�r eine Expedition
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

	// Pr�fen, ob jeweils noch weitere Waren bestellt werden m�ssen
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

	// Wenn immer noch nicht alles da ist, sp�ter noch einmal bestellen
	if(orderware_ev == NULL)
		orderware_ev = em->AddEvent(this,210,10);

}


/// Eine bestellte Ware konnte doch nicht kommen
void nobHarborBuilding::WareLost(Ware * ware)
{
	// ggf. neue Waren f�r Expedition bestellen
	if(expedition.active && (ware->type == GD_BOARDS || ware->type == GD_STONES))
		OrderExpeditionWares();
	RemoveDependentWare(ware);
}



/// Schiff ist angekommen
void nobHarborBuilding::ShipArrived(noShip * ship)
{
	// Verf�gbare Aufgaben abklappern

	// Steht Expedition zum Start bereit
	if(expedition.active && expedition.builder 
		&& expedition.boards == BUILDING_COSTS[nation][BLD_HARBORBUILDING].boards
		&& expedition.stones == BUILDING_COSTS[nation][BLD_HARBORBUILDING].stones)
	{
		// Aufr�umen am Hafen
		expedition.active = false;
		// Expedition starten
		ship->StartExpedition();
	}
	// Gibt es Waren oder Figuren, die ein Schiff von hier aus nutzen wollen?
	else if(wares_for_ships.size() || figures_for_ships.size())
	{
		// Das Ziel wird nach der ersten Figur bzw. ersten Ware gew�hlt
		Point<MapCoord> dest;
		if(figures_for_ships.size())
			dest = figures_for_ships.begin()->dest;
		else
			dest = (*wares_for_ships.begin())->GetNextHarbor();


		std::list<noFigure*> figures;

		// Figuren ausw�hlen, die zu diesem Ziel wollen
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

		// Und noch die Waren ausw�hlen
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
		// Dann f�gen wir die mal bei uns hinzu
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

			// Ware nicht mehr abh�ngig
			RemoveDependentWare(ware);
			// Dann zweigen wir die einfach mal f�r die Expedition ab
			gwg->GetPlayer(player)->RemoveWare(ware);
			delete ware;

			// Ggf. ist jetzt alles ben�tigte da
			CheckExpeditionReady();
			return;
		}
	}

	nobBaseWarehouse::AddWare(ware);

}

/// Eine Figur geht ins Lagerhaus
void nobHarborBuilding::AddFigure(noFigure * figure)
{
	// Brauchen wir einen Bauarbeiter f�r die Expedition?
	if(figure->GetJobType() == JOB_BUILDER && expedition.active && !expedition.builder)
	{
		
		RemoveDependentFigure(figure);
		em->AddToKillList(figure);

		expedition.builder = true;
		// Ggf. ist jetzt alles ben�tigte da
		CheckExpeditionReady();
	}
	else
		// ansonsten weiterdelegieren
		nobBaseWarehouse::AddFigure(figure);
}

/// Gibt zur�ck, ob Expedition vollst�ndig ist
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

/// Pr�ft, ob eine Expedition von den Waren her vollst�ndig ist und ruft ggf. das Schiff
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

/// Gibt die Hafenplatz-ID zur�ck, auf der der Hafen steht
unsigned nobHarborBuilding::GetHarborPosID() const
{
	return gwg->GetHarborPointID(x,y);
}

/// Abfangen, wenn ein Mann nicht mehr kommen kann --> k�nnte ein Bauarbeiter sein und
/// wenn wir einen ben�tigen, m�ssen wir einen neuen bestellen
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

		// Keinen gefunden, also m�ssen wir noch einen bestellen
		players->getElement(player)->AddJobWanted(JOB_BUILDER,this);
	}

	
}

/// Gibt eine Liste mit m�glichen Verbindungen zur�ck
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


/// F�gt einen Mensch hinzu, der mit dem Schiff irgendwo hin fahren will
void nobHarborBuilding::AddFigureForShip(noFigure * fig, Point<MapCoord> dest)
{
	FigureForShip ffs = { fig, dest };
	figures_for_ships.push_back(ffs);
	players->getElement(player)->OrderShip(this);
	// Anzahl visuell erh�hen
	++goods.people[fig->GetJobType()];
}

/// F�gt eine Ware hinzu, die mit dem Schiff verschickt werden soll
void nobHarborBuilding::AddWareForShip(Ware * ware)
{
	wares_for_ships.push_back(ware);
	// Anzahl visuell erh�hen
	++goods.goods[ConvertShields(ware->type)];
	players->getElement(player)->OrderShip(this);
	ware->WaitForShip(this);
}

/// Gibt Anzahl der Schiffe zur�ck, die noch f�r ausstehende Aufgaben ben�tigt werden
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

/// Gibt die Wichtigkeit an, dass ein Schiff kommen muss (0 -> keine Bed�rftigkeit)
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
/// (muss in dem Fall true zur�ckgeben)
bool nobHarborBuilding::UseWareAtOnce(Ware * ware, noBaseBuilding* const goal)
{
	// Evtl. muss die Ware gleich das Schiff nehmen -> 
	// dann zum Schiffsreservoir hinzuf�gen
	Point<MapCoord> next_harbor;
	ware->RecalcRoute();
	if(ware->GetNextDir() == SHIP_DIR)
	{
		// Dann f�gen wir die mal bei uns hinzu
		AddWareForShip(ware);

		return true;
	}

	return false;
}


/// Dasselbe f�r Menschen
bool nobHarborBuilding::UseFigureAtOnce(noFigure * fig, noRoadNode* const goal)
{
	// Evtl. muss die Ware gleich das Schiff nehmen -> 
	// dann zum Schiffsreservoir hinzuf�gen
	Point<MapCoord> next_harbor;
	if(gwg->FindHumanPathOnRoads(this,goal,NULL,&next_harbor) == SHIP_DIR)
	{
		// Dann f�gen wir die mal bei uns hinzu
		AddFigureForShip(fig,next_harbor);
		return true;
	}

	return false;
}

/// Erh�lt die Waren von einem Schiff und nimmt diese in den Warenbestand auf
void nobHarborBuilding::ReceiveGoodsFromShip(const std::list<noFigure*> figures, const std::list<Ware*> wares)
{
	// Menschen zur Ausgehliste hinzuf�gen
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

	// Waren zur Warteliste hinzuf�gen
	for(std::list<Ware*>::const_iterator it = wares.begin();it!=wares.end();++it)
	{
		// Optische Warenwerte entsprechend erh�hen
		++goods.goods[ConvertShields((*it)->type)];
		if((*it)->ShipJorneyEnded(this))
		{
			
			// Ware will die weitere Reise antreten, also muss sie zur Liste der rausgetragenen Waren
			// hinzugef�gt werden
			waiting_wares.push_back(*it);
		}
		else
		{
			// Ansonsten f�gen wir die Ware einfach zu unserem Inventar dazu
			RemoveDependentWare(*it);
			++real_goods.goods[ConvertShields((*it)->type)];
			players->getElement(player)->RemoveWare(*it);
			delete *it;
		}
	}

	// Ggf. neues Rausgeh-Event anmelden, was notwendig ist, wenn z.B. nur Waren zur Liste hinzugef�gt wurden
	AddLeavingEvent();
}

/// Storniert die Bestellung f�r eine bestimmte Ware, die mit einem Schiff transportiert werden soll
void nobHarborBuilding::CancelWareForShip(Ware* ware)
{
	// Ware aus der Liste entfernen
	wares_for_ships.remove(ware);
	// Ware zur Inventur hinzuf�gen
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
		// Dann zu unserem Inventar hinzuf�gen und anschlie�end vernichten
		++real_goods.people[figure->GetJobType()];
		//em->AddToKillList(figure);
	}
	// An Basisklasse weiterdelegieren
	else
		nobBaseWarehouse::CancelFigure(figure);
	
}


/// Gibt die Angreifergeb�ude zur�ck, die dieser Hafen f�r einen Seeangriff zur Verf�gung stellen kann
void nobHarborBuilding::GetAttackerBuildingsForSeaAttack(std::vector<SeaAttackerBuilding> * buildings,
											const std::vector<unsigned>& defender_harbors)
{
	list<nobBaseMilitary*> all_buildings;
	gwg->LookForMilitaryBuildings(all_buildings,x,y,3);

	// Und z�hlen
	for(list<nobBaseMilitary*>::iterator it = all_buildings.begin();it.valid();++it)
	{
		if((*it)->GetGOT() != GOT_NOB_MILITARY)
			continue;

		// Weg vom Hafen zum Milit�rgeb�ude berechnen
		if(!gwg->FindFreePath((*it)->GetX(),(*it)->GetY(),x,y,false,BASE_ATTACKING_DISTANCE*2,NULL,NULL,NULL,NULL,NULL,NULL))
			continue;
			
		// Entfernung zwischen Hafen und m�glichen Zielhafenpunkt ausrechnen
		unsigned min_distance = 0xffffffff;
		for(unsigned i = 0;i<defender_harbors.size();++i)
		{
			min_distance = min(min_distance, gwg->CalcHarborDistance(GetHarborPosID(),defender_harbors.at(i)));
		}
		
		// Geb�ude suchen, vielleicht schon vorhanden?
		std::vector<SeaAttackerBuilding>::iterator it2 = std::find(buildings->begin(), buildings->end(), 
		static_cast<nobMilitary*>(*it));
		// Noch nicht vorhanden? 
		if(it2 == buildings->end())
		{
			// Dann neu hinzuf�gen
			SeaAttackerBuilding sab = { static_cast<nobMilitary*>(*it), this, min_distance };
			buildings->push_back(sab);
		}
		// Oder vorhanden und jetzige Distanz ist kleiner?
		else if(min_distance < it2->distance)
		{
			// Dann Distanz und betreffenden Hafen aktualisieren
			it2->distance = min_distance;
			it2->harbor = this;
		}
	}
}

/// F�gt einen Schiffs-Angreifer zum Hafen hinzu
void nobHarborBuilding::AddSeaAttacker(nofAttacker * attacker)
{
	unsigned best_distance = 0xffffffff;
	unsigned best_harbor_point;
	std::vector<unsigned> harbor_points;
	gwg->GetHarborPointsAroundMilitaryBuilding(attacker->GetAttackedGoal()->GetX(),attacker->GetAttackedGoal()->GetY(),
												&harbor_points);
	for(unsigned i = 0;i<harbor_points.size();++i)
	{
		unsigned tmp_distance = gwg->CalcHarborDistance(this->GetHarborPosID(),harbor_points[i]);
		if(tmp_distance < best_distance)
		{
			best_distance = tmp_distance;
			best_harbor_point = harbor_points[i];
		}
	}
	
	
	SoldierForShip sfs = { attacker, gwg->GetHarborPoint(best_harbor_point) };
	soldiers_for_ships.push_back(sfs);
}


