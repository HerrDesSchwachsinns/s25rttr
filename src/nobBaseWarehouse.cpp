// $Id: nobBaseWarehouse.cpp 5137 2009-06-28 19:28:27Z OLiver $
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
#include "nobBaseWarehouse.h"
#include "GameWorld.h"
#include "EventManager.h"
#include "nofCarrier.h"
#include "GameClient.h"
#include "GameClientPlayer.h"
#include "Ware.h"
#include "nofWarehouseWorker.h"
#include "nofPassiveSoldier.h"
#include "nofAggressiveDefender.h"
#include "nofDefender.h"
#include "nobMilitary.h"
#include "Random.h"
#include "JobConsts.h"
#include "BurnedWarehouse.h"
#include "SerializedGameData.h"
#include "nofPassiveWorker.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

/// Intervall für Ausleerung (in gf)
const unsigned EMPTY_INTERVAL = 100;
/// Dauer für das Erstellen von Trägern
const unsigned short PRODUCE_HELPERS_GF = 300;
const unsigned short PRODUCE_HELPERS_RANDOM_GF = 20;
/// Dauer für das Rekrutierung von Soldaten
const unsigned short RECRUITE_GF = 400;
const unsigned short RECRUITE_RANDOM_GF = 400;

nobBaseWarehouse::nobBaseWarehouse(const BuildingType type,const unsigned short x, const unsigned short y,const unsigned char player,const Nation nation)
: nobBaseMilitary(type,x,y,player,nation), fetch_double_protection(false), producinghelpers_event(em->AddEvent(this,PRODUCE_HELPERS_GF+RANDOM.Rand(__FILE__,__LINE__,obj_id,PRODUCE_HELPERS_RANDOM_GF),1)), recruiting_event(0),
empty_event(0)
{
	// Evtl gabs verlorene Waren, die jetzt in das HQ wieder reinkönnen
	GAMECLIENT.GetPlayer(player)->FindClientForLostWares();

	// Reserve nullen
	for(unsigned i = 0;i<5;++i)
		reserve_soldiers_available[i] = 
		reserve_soldiers_claimed_visual[i] = 
		reserve_soldiers_claimed_real[i] = 0;
}

nobBaseWarehouse::~nobBaseWarehouse()
{
	// Waiting Wares löschen
	for(list<Ware*>::iterator it = waiting_wares.begin();it.valid();++it)
		delete (*it);
}

void nobBaseWarehouse::Destroy_nobBaseWarehouse()
{
	// Aus der Warenhausliste entfernen
	GAMECLIENT.GetPlayer(player)->RemoveWarehouse(this);
	// Den Waren und Figuren Bescheid sagen, die zu uns auf den Weg sind, dass wir nun nicht mehr existieren
	for(list<noFigure*>::iterator it = dependent_figures.begin();it.valid();++it)
		(*it)->GoHome();
	for(list<Ware*>::iterator it = dependent_wares.begin();it.valid();++it)
		(*it)->GoalDestroyed();

	// ggf. Events abmelden
	em->RemoveEvent(recruiting_event);
	em->RemoveEvent(producinghelpers_event);
	em->RemoveEvent(empty_event);

	// Waiting Wares löschen
	for(list<Ware*>::iterator it = waiting_wares.begin();it.valid();++it)
	{
		(*it)->WareLost(player);
		delete (*it);
	}

	waiting_wares.clear();

	// restliche Warenbestände von der Inventur wieder abziehen
	for(unsigned int i = 0; i < WARE_TYPES_COUNT; ++i)
		GAMECLIENT.GetPlayer(player)->DecreaseInventoryWare(GoodType(i),real_goods.goods[i]);

	//for(unsigned int i = 0; i < 30; ++i)
	//	GAMECLIENT.GetPlayer(player)->DecreaseInventoryJob(Job(i),real_goods.people[i]);

	// Objekt, das die flüchtenden Leute nach und nach ausspuckt, erzeugen
	new BurnedWarehouse(x,y,player,real_goods.people);

	Destroy_nobBaseMilitary();
}


void nobBaseWarehouse::Serialize_nobBaseWarehouse(SerializedGameData * sgd) const
{
	Serialize_nobBaseMilitary(sgd);

	sgd->PushObjectList(waiting_wares,true);
	sgd->PushBool(fetch_double_protection);
	sgd->PushObjectList(dependent_figures,false);
	sgd->PushObjectList(dependent_wares,true);
	sgd->PushObject(producinghelpers_event,true);
	sgd->PushObject(recruiting_event,true);
	sgd->PushObject(empty_event,true);

	for(unsigned i = 0;i<5;++i)
	{
		// Nur das Reale, nicht das visuelle speichern, das wäre sinnlos!, beim Laden ist das visuelle = realem
		sgd->PushUnsignedInt(reserve_soldiers_available[i]);
		sgd->PushUnsignedInt(reserve_soldiers_claimed_real[i]);
	}

	for(unsigned i = 0;i<WARE_TYPES_COUNT;++i)
	{
		sgd->PushUnsignedInt(goods.goods[i]);
		sgd->PushUnsignedInt(real_goods.goods[i]);
		sgd->PushUnsignedChar(inventory_settings_real.wares[i]);
	}
	for(unsigned i = 0;i<JOB_TYPES_COUNT;++i)
	{
		sgd->PushUnsignedInt(goods.people[i]);
		sgd->PushUnsignedInt(real_goods.people[i]);
		sgd->PushUnsignedChar(inventory_settings_real.figures[i]);
	}
}

nobBaseWarehouse::nobBaseWarehouse(SerializedGameData * sgd, const unsigned obj_id) : nobBaseMilitary(sgd,obj_id)
{
	sgd->PopObjectList(waiting_wares,GOT_WARE);
	fetch_double_protection = sgd->PopBool();
	sgd->PopObjectList(dependent_figures,GOT_UNKNOWN);
	sgd->PopObjectList(dependent_wares,GOT_WARE);
	producinghelpers_event = sgd->PopObject<EventManager::Event>(GOT_EVENT);
	recruiting_event = sgd->PopObject<EventManager::Event>(GOT_EVENT);
	empty_event = sgd->PopObject<EventManager::Event>(GOT_EVENT);

	for(unsigned i = 0;i<5;++i)
	{
		reserve_soldiers_available[i] = sgd->PopUnsignedInt();
		reserve_soldiers_claimed_visual[i] = reserve_soldiers_claimed_real[i] = sgd->PopUnsignedInt();
	}

	for(unsigned i = 0;i<WARE_TYPES_COUNT;++i)
	{
		goods.goods[i] = sgd->PopUnsignedInt();
		real_goods.goods[i] = sgd->PopUnsignedInt();
		inventory_settings_real.wares[i] = inventory_settings_visual.wares[i] = sgd->PopUnsignedChar();
	}
	for(unsigned i = 0;i<JOB_TYPES_COUNT;++i)
	{
		goods.people[i] = sgd->PopUnsignedInt();
		real_goods.people[i] = sgd->PopUnsignedInt();
		inventory_settings_real.figures[i] = inventory_settings_visual.figures[i] = sgd->PopUnsignedChar();
	}
}


void nobBaseWarehouse::OrderCarrier(noRoadNode* const goal, RoadSegment * workplace)
{
	AddLeavingFigure(workplace->carrier[0] = new nofCarrier(
		(workplace->GetRoadType() == RoadSegment::RT_BOAT) ? nofCarrier::CT_BOAT : nofCarrier::CT_NORMAL,x,y,player,workplace,goal));
	--real_goods.people[JOB_HELPER];
	if((workplace->GetRoadType() == RoadSegment::RT_BOAT))
		--real_goods.goods[GD_BOAT];

	// Evtl. kein Gehilfe mehr, sodass das Rekrutieren gestoppt werden muss
	TryStopRecruiting();
}

void nobBaseWarehouse::OrderJob(const Job job, noRoadNode* const goal)
{
	// Job überhaupt hier vorhanden
	if(!real_goods.people[job])
	{
		// Evtl das Werkzeug der Person vorhanden sowie ein Träger?
		bool tool_available = (JOB_CONSTS[job].tool == GD_NOTHING) ? true : (real_goods.goods[JOB_CONSTS[job].tool]!=0);
		if(!(real_goods.people[JOB_HELPER] && tool_available))
		{
			// nein --> dann tschüss
			LOG.lprintf("nobBaseWarehouse::OrderJob: WARNING: No JobType %u (or his tool) in warehouse!\n",static_cast<unsigned>(job));
			return;
		}
	}

	noFigure * fig = CreateJob(job,x,y,player,goal);
	AddLeavingFigure(fig);

		
	// Ziel Bescheid sagen, dass dortin ein neuer Arbeiter kommt (bei Flaggen als das anders machen)
	if(goal->GetType() == NOP_FLAG)
	{
	}
	else
	{
		static_cast<noBaseBuilding*>(goal)->GotWorker(job,fig);
	}

	if(real_goods.people[job])
		--real_goods.people[job];
	else
	{
		// ansonsten muss er erst noch "rekrutiert" werden
		if(JOB_CONSTS[job].tool != GD_NOTHING)
		{
			--real_goods.goods[JOB_CONSTS[job].tool];
			--goods.goods[JOB_CONSTS[job].tool];
			GAMECLIENT.GetPlayer(player)->DecreaseInventoryWare(JOB_CONSTS[job].tool,1);
		}

		--real_goods.people[JOB_HELPER];
		--goods.people[JOB_HELPER];
		GAMECLIENT.GetPlayer(player)->DecreaseInventoryJob(JOB_HELPER,1);

		// erhöhen, da er ja dann rauskommt und es bei den visuellen wieder abgezogen wird!
		++goods.people[job];
		GAMECLIENT.GetPlayer(player)->IncreaseInventoryJob(job,1);
	}


	// Evtl. kein Gehilfe mehr da, sodass das Rekrutieren gestoppt werden muss
	TryStopRecruiting();
}

nofCarrier * nobBaseWarehouse::OrderDonkey(RoadSegment * road,noRoadNode * const goal_flag)
{
	// Überhaupt ein Esel vorhanden?
	if(!real_goods.people[JOB_PACKDONKEY])
		return 0;

	nofCarrier * donkey;
	AddLeavingFigure(donkey = new nofCarrier(nofCarrier::CT_DONKEY,x,y,player,road,goal_flag));
	--real_goods.people[JOB_PACKDONKEY];

	return donkey;
}


void nobBaseWarehouse::HandleBaseEvent(const unsigned int id)
{
	switch(id)
	{
	// Rausgeh-Event
	case 0:
		{
			leaving_event = 0;

			// Falls eine Bestellung storniert wurde
			if(!leave_house.size() && !waiting_wares.size())
			{
				go_out = false;
				return;
			}

			// Figuren kommen zuerst raus
			if(leave_house.size())
			{
				noFigure * fig = *leave_house.begin();
				gwg->AddFigure(*leave_house.begin(),x,y);

				/// Aktive Soldaten laufen nicht im Wegenetz, die das Haus verteidigen!
				if(fig->GetGOT() != GOT_NOF_AGGRESSIVEDEFENDER && 
					fig->GetGOT() != GOT_NOF_DEFENDER)
					// ansonsten alle anderen müssen aber wissen, auf welcher Straße sie zu Beginn laufen
					(*leave_house.begin())->InitializeRoadWalking(routes[4],0,true);

				(*leave_house.begin())->ActAtFirst();
				// Bei Lagerhausarbeitern das nicht abziehen!
				if(!(*leave_house.begin())->MemberOfWarehouse())
				{
					// War das ein Boot-Träger?
					if((*leave_house.begin())->GetJobType() == JOB_BOATCARRIER)
					{
						// Träger abziehen einzeln
						--goods.people[JOB_HELPER];
						// Boot abziehen einzeln
						--goods.goods[GD_BOAT];
					}
					else
						--goods.people[(*leave_house.begin())->GetJobType()];
				}

				leave_house.pop_front();
			}
			else
			{
				// Ist noch Platz an der Flagge?
				if(GetFlag()->GetWareCount() < 8)
				{
					// Dann Ware raustragen lassen
					Ware * ware = *waiting_wares.begin();
					nofWarehouseWorker * worker = new nofWarehouseWorker(x,y,player,ware,0);
					gwg->AddFigure(worker,x,y);
					assert(goods.goods[ware->type]>0);
					--goods.goods[ware->type];
					worker->WalkToGoal();
					ware->Carry(GetFlag());
					waiting_wares.pop_front();
				}
				else
				{
					// Kein Platz mehr für Waren --> keiner brauch mehr rauszukommen, und Figuren gibts ja auch keine mehr
					go_out = false;
				}
			}

			// Wenn keine Figuren und Waren mehr da sind (bzw die Flagge vorm Haus voll ist), brauch auch keiner mehr rauszukommen
			if(!leave_house.size() && !waiting_wares.size())
				go_out = false;


			if(go_out)
				leaving_event = em->AddEvent(this,20+RANDOM.Rand(__FILE__,__LINE__,obj_id,10));
		} break;
	// Träger-Produzier-Event
	case 1:
		{
			// Nur bei unter 100 Träcern, weitere "produzieren"
			if(real_goods.people[JOB_HELPER] < 100)
			{
				++real_goods.people[JOB_HELPER];
				++goods.people[JOB_HELPER];

				GAMECLIENT.GetPlayer(player)->IncreaseInventoryJob(JOB_HELPER,1);

				if(real_goods.people[JOB_HELPER] == 1)
				{
					
					// Wenn vorher keine Träger da waren, müssen alle unbesetzen Wege gucken, ob sie nen Weg hierher finden, könnte ja sein, dass vorher nich genug Träger da waren
					GAMECLIENT.GetPlayer(player)->FindWarehouseForAllRoads();
					// evtl Träger mit Werkzeug kombiniert -> neuer Beruf
					GAMECLIENT.GetPlayer(player)->FindWarehouseForAllJobs(JOB_NOTHING);
				}
			}
			else if(real_goods.people[JOB_HELPER] > 100)
			{
				// Bei Überbevölkerung Träger vernichten
				--real_goods.people[JOB_HELPER];
				--goods.people[JOB_HELPER];

				GAMECLIENT.GetPlayer(player)->DecreaseInventoryJob(JOB_HELPER,1);
			}

			producinghelpers_event = em->AddEvent(this,PRODUCE_HELPERS_GF+RANDOM.Rand(__FILE__,__LINE__,obj_id,PRODUCE_HELPERS_RANDOM_GF),1);


			// Evtl. genau der Gehilfe, der zum Rekrutieren notwendig ist
			TryRecruiting();

			// Evtl die Typen gleich wieder auslagern, falls erforderlich
			CheckOuthousing(1,JOB_HELPER);
		} break;
	// Soldaten-Rekrutierungsevent
	case 2:
		{
			recruiting_event = 0;

			// Ein Gefreiter mehr, dafür Träger, Waffen und Bier weniger
			++real_goods.people[JOB_PRIVATE];
			++goods.people[JOB_PRIVATE];
			GAMECLIENT.GetPlayer(player)->IncreaseInventoryJob(JOB_PRIVATE,1);

			--real_goods.people[JOB_HELPER];
			--goods.people[JOB_HELPER];
			GAMECLIENT.GetPlayer(player)->DecreaseInventoryJob(JOB_HELPER,1);
			
			--real_goods.goods[GD_SWORD];
			--goods.goods[GD_SWORD];
			GAMECLIENT.GetPlayer(player)->DecreaseInventoryWare(GD_SWORD,1);

			--real_goods.goods[GD_SHIELDROMANS];
			--goods.goods[GD_SHIELDROMANS];
			GAMECLIENT.GetPlayer(player)->DecreaseInventoryWare(GD_SHIELDROMANS,1);

			--real_goods.goods[GD_BEER];
			--goods.goods[GD_BEER];
			GAMECLIENT.GetPlayer(player)->DecreaseInventoryWare(GD_BEER,1);


			// Evtl. versuchen nächsten zu rekrutieren
			TryRecruiting();

			// Wenn vorher keine Soldaten hier waren, Militärgebäude prüfen (evtl kann der Soldat ja wieder in eins gehen)
			if(real_goods.people[JOB_PRIVATE] == 1)
				GAMECLIENT.GetPlayer(player)->NewSoldierAvailable(real_goods.people[JOB_PRIVATE]);

		} break;
	// Auslagerevent
	case 3:
		{
			empty_event = 0;

			list<unsigned> type_list;
			// Waren und Figuren zum Auslagern zusammensuchen (id >= 34 --> Figur!)
			// Wenn keine Platz an Flagge, dann keine Waren raus
			if(GetFlag()->IsSpaceForWare())
			{
				for(unsigned i = 0;i<WARE_TYPES_COUNT;++i)
				{
					if(CheckRealInventorySettings(0,4,i) && real_goods.goods[i])
						type_list.push_back(i);
				}
			}

			for(unsigned i = 0;i<JOB_TYPES_COUNT;++i)
			{
				// Figuren, die noch nicht implementiert sind, nicht nehmen!
				if(CheckRealInventorySettings(1,4,i) && real_goods.people[i])
					type_list.push_back(WARE_TYPES_COUNT+i);
			}

			// Gibts überhaupt welche?
			if(!type_list.size())
				// ansonsten gleich tschüss
				return;

			// Eine ID zufällig auswählen
			unsigned type = *type_list[RANDOM.Rand(__FILE__,__LINE__,obj_id,type_list.size())];

			if(type < WARE_TYPES_COUNT)
			{
				// Ware

				Ware * ware = new Ware(GoodType(type),0,this);
				ware->goal = GAMECLIENT.GetPlayer(player)->FindClientForWare(ware);

				// Ware zur Liste hinzufügen, damit sie dann rausgetragen wird
				waiting_wares.push_back(ware);

				AddLeavingEvent();

				// Ware aus Inventar entfernen
				--(real_goods.goods[type]);

				// Evtl. kein Schwert/Schild/Bier mehr da, sodass das Rekrutieren gestoppt werden muss
				TryStopRecruiting();
			}
			else
			{
				// Figur
				type-=WARE_TYPES_COUNT;

				nobBaseWarehouse * wh = GAMECLIENT.GetPlayer(player)->FindWarehouse(this,FW::Condition_StoreFigure,0,true,&type,false);
				nofPassiveWorker * fig = new nofPassiveWorker(Job(type),x,y,player,NULL);
				
				if(wh)
					fig->GoHome(wh);
				else
					fig->StartWandering();

				// Kein Ziel gefunden, dann später gleich rumirren!
				/*if(!wh)
					fig->StartWandering();*/

				AddLeavingFigure(fig);

				// Person aus Inventar entfernen
				--(real_goods.people[type]);

				// Evtl. kein Gehilfe mehr da, sodass das Rekrutieren gestoppt werden muss
				TryStopRecruiting();
			}

			// Weitere Waren/Figuren zum Auslagern? 
			if(AreWaresToEmpty())
				// --> Nächstes Event
				empty_event = em->AddEvent(this,EMPTY_INTERVAL,3);

		} break;
	}
}

Ware * nobBaseWarehouse::OrderWare(const GoodType good, noBaseBuilding* const goal)
{
	// Ware überhaupt hier vorhanden (Abfrage eigentlich nicht nötig, aber erstmal zur Sicherheit)
	if(!real_goods.goods[good])
	{
		LOG.lprintf("nobBaseWarehouse::OrderWare: WARNING: No ware type %u in warehouse!\n",static_cast<unsigned>(good));
		return 0;
	}

	Ware * ware = new Ware(good,goal,this);

	// Ware zur Liste hinzufügen, damit sie dann rausgetragen wird
	waiting_wares.push_back(ware);

	--real_goods.goods[good];

	// Wenn gerade keiner rausgeht, muss neues Event angemeldet werden
	AddLeavingEvent();

	// Evtl. keine Waffen/Bier mehr da, sodass das Rekrutieren gestoppt werden muss
	TryStopRecruiting();

	return ware;
}

void nobBaseWarehouse::AddWaitingWare(Ware * ware)
{
	waiting_wares.push_back(ware);
	// Wenn gerade keiner rausgeht, muss neues Event angemeldet werden
	AddLeavingEvent();
	// Die visuelle Warenanzahl wieder erhöhen
	++goods.goods[ware->type];
}

bool nobBaseWarehouse::FreePlaceAtFlag()
{
	if(waiting_wares.size())
	{
		AddLeavingEvent();
		return true;
	}
	else
	{
		// Evtl. war die Flagge voll und das Auslagern musste gestoppt werden
		// Weitere Waren/Figuren zum Auslagern und kein Event angemeldet?
		if(AreWaresToEmpty() && !empty_event)
			// --> Nächstes Event
			empty_event = em->AddEvent(this,EMPTY_INTERVAL,3);

		return false;
	}
}

void nobBaseWarehouse::AddWare(Ware * ware)
{
	// Ware nicht mehr abhängig
	RemoveDependentWare(ware);

	// Die Schilde der verschiedenen Nation in eine "Schild-Sorte" (den der Römer) umwandeln!
	GoodType type;
	if(ware->type == GD_SHIELDAFRICANS || ware->type == GD_SHIELDJAPANESE || ware->type == GD_SHIELDVIKINGS)
		type = GD_SHIELDROMANS;
	else
		type = ware->type;

	GAMECLIENT.GetPlayer(player)->RemoveWare(ware);
	delete ware;

	++real_goods.goods[type];
	++goods.goods[type];
	

	// Wenn es ein Werkzeug war, evtl neuen Job suchen, der jetzt erzeugt werden könnte..
	if(type >= GD_TONGS && type <= GD_BOAT)
	{
		for(unsigned i = 0;i<30;++i)
		{
			if(JOB_CONSTS[i].tool == type)
				GAMECLIENT.GetPlayer(player)->FindWarehouseForAllJobs(Job(i));
		}
	}

	

	// Wars Baumaterial? Dann den Baustellen Bescheid sagen
	if(type == GD_BOARDS || type == GD_STONES)
		GAMECLIENT.GetPlayer(player)->FindMaterialForBuildingSites();

	// Evtl wurden Bier oder Waffen reingetragen --> versuchen zu rekrutieren
	TryRecruiting();

	// Evtl die Ware gleich wieder auslagern, falls erforderlich
	CheckOuthousing(0,type);
}

void nobBaseWarehouse::AddFigure(noFigure * figure)
{
	// Warenhausarbeiter werden nicht gezählt!
	if(!figure->MemberOfWarehouse())
	{
		// War das ein Boot-Träger?
		if(figure->GetJobType() == JOB_BOATCARRIER)
		{
			// Träger hinzufügen einzeln
			++goods.people[JOB_HELPER];
			++real_goods.people[JOB_HELPER];
			// Boot hinzufügen einzeln
			++goods.goods[GD_BOAT];
			++real_goods.goods[GD_BOAT];
		}
		else
		{
			++goods.people[figure->GetJobType()];
			++real_goods.people[figure->GetJobType()];
		}
	}
	RemoveDependentFigure(figure);
	em->AddToKillList(figure);

	// Evtl ging ein Gehilfe rein --> versuchen zu rekrutieren
	if(figure->GetJobType() == JOB_HELPER)
		TryRecruiting();

	if(figure->GetJobType() >= JOB_PRIVATE && figure->GetJobType() <= JOB_GENERAL)
	{
		// Truppen prüfen in allen Häusern
		GAMECLIENT.GetPlayer(player)->NewSoldierAvailable(real_goods.people[figure->GetJobType()]);
		// Reserve prüfen
		RefreshReserve(figure->GetJobType()-JOB_PRIVATE);
	}
	else
	{
		if(figure->GetJobType() == JOB_PACKDONKEY)
		{
			// Straße für Esel suchen
			noRoadNode * goal;
			if(RoadSegment * road = GameClient::inst().GetPlayer(player)->FindRoadForDonkey(this,&goal))
			{
				// gefunden --> Esel an die Straße bestellen
				road->GotDonkey(OrderDonkey(road,goal));
			}

		}
		else
		{
			// Evtl. Abnehmer für die Figur wieder finden
			GAMECLIENT.GetPlayer(player)->FindWarehouseForAllJobs(figure->GetJobType());
			// Wenns ein Träger war, auch Wege prüfen
			if(figure->GetJobType() == JOB_HELPER && real_goods.people[JOB_HELPER]==1)
			{
				// evtl als Träger auf Straßen schicken
				GAMECLIENT.GetPlayer(player)->FindWarehouseForAllRoads();
				// evtl Träger mit Werkzeug kombiniert -> neuer Beruf
				GAMECLIENT.GetPlayer(player)->FindWarehouseForAllJobs(JOB_NOTHING);
			}
				
		}
	}

	// Evtl den Typen gleich wieder auslagern, falls erforderlich
	CheckOuthousing(1,figure->GetJobType());
}

void nobBaseWarehouse::FetchWare()
{
	if(!fetch_double_protection)
	{
		AddLeavingFigure(new nofWarehouseWorker(x,y,player,0,1));
		/*gwg->AddFigure(worker,x,y);
		worker->ActAtFirst();*/
	}

	fetch_double_protection = false;
}

void nobBaseWarehouse::WareLost(Ware * ware)
{
	RemoveDependentWare(ware);
}

void nobBaseWarehouse::CancelWare(Ware * ware)
{
	// Ware aus den Waiting-Wares entfernen
	waiting_wares.erase(ware);
	// Anzahl davon wieder hochsetzen
	++real_goods.goods[ConvertShields(ware->type)];
}

/// Bestellte Figur, die sich noch inder Warteschlange befindet, kommt nicht mehr und will rausgehauen werden
void nobBaseWarehouse::CancelFigure(noFigure * figure)
{
	// Figure aus den Waiting-Wares entfernen
	leave_house.erase(figure);
	// Anzahl davon wieder hochsetzen
	if(figure->GetJobType() == JOB_BOATCARRIER)
	{
		// Träger hinzufügen einzeln
		++real_goods.people[JOB_HELPER];
		// Boot hinzufügen einzeln
		++real_goods.goods[GD_BOAT];
	}
	else
		++real_goods.people[figure->GetJobType()];
}

void nobBaseWarehouse::TakeWare(Ware * ware)
{
	assert(ware->GetObjId() != 536528);
	// Ware zur Abhängigkeitsliste hinzufügen, damit sie benachrichtigt wird, wenn dieses Lagerhaus zerstört wird
	dependent_wares.push_back(ware);
}

void nobBaseWarehouse::OrderTroops(nobMilitary * goal, unsigned count)
{
	// Soldaten durchgehen und count rausschicken

	// Ränge durchgehen, absteigend, starke zuerst
	if (GameClient::inst().GetPlayer(player)->military_settings[1] > 2)
	{
		for(unsigned i = 5;i && count;--i)
		{
			// Vertreter der Ränge ggf rausschicken
			while(real_goods.people[JOB_PRIVATE-1+i] && count)
			{
				nofSoldier * soldier = new nofPassiveSoldier(x,y,player,goal,goal,i-1);
				AddLeavingFigure(soldier);
				goal->GotWorker(JOB_NOTHING,soldier);

				--real_goods.people[JOB_PRIVATE-1+i];

				--count;
			}
		}
	}
	// Ränge durchgehen, aufsteigend, schwache zuerst
	else
	{
		for(unsigned i = 1;i<=5 && count;++i)
		{
			// Vertreter der Ränge ggf rausschicken
			while(real_goods.people[JOB_PRIVATE-1+i] && count)
			{
				nofSoldier * soldier = new nofPassiveSoldier(x,y,player,goal,goal,i-1);
				AddLeavingFigure(soldier);
				goal->GotWorker(JOB_NOTHING,soldier);

				--real_goods.people[JOB_PRIVATE-1+i];

				--count;
			}
		}
	}
	
}

nofAggressiveDefender * nobBaseWarehouse::SendDefender(nofAttacker * attacker)
{
	// Sind noch Soldaten da?
	unsigned char rank;
	for(rank = 5;rank;--rank)
	{
		if(real_goods.people[JOB_PRIVATE+rank-1])
			break;
	}

	// Wenn kein Soldat mehr da ist --> 0 zurückgeben
	if(!rank)
		return 0;

	// Dann den Stärksten rausschicken
	nofAggressiveDefender * soldier = new nofAggressiveDefender(x,y,player,this,rank-1,attacker);
	--real_goods.people[JOB_PRIVATE+rank-1];
	AddLeavingFigure(soldier);

	troops_on_mission.push_back(soldier);

	return soldier;
}

void nobBaseWarehouse::SoldierLost(nofSoldier * soldier)
{
	// Soldat konnte nicht (mehr) kommen --> rauswerfen
	troops_on_mission.erase(static_cast<nofActiveSoldier*>(soldier));
}

void nobBaseWarehouse::AddActiveSoldier(nofActiveSoldier * soldier)
{
	// Soldat hinzufügen
	++real_goods.people[JOB_PRIVATE+soldier->GetRank()];
	++goods.people[JOB_PRIVATE+soldier->GetRank()];

	// Truppen prüfen in allen Häusern
	GAMECLIENT.GetPlayer(player)->RegulateAllTroops();

	// und Soldat vernichten
	em->AddToKillList(soldier);
}

nofDefender * nobBaseWarehouse::ProvideDefender(nofAttacker * const attacker)
{
	// Ränge zählen
	unsigned rank_count = 0;

	for(unsigned i = 0;i<5;++i)
	{
		if(real_goods.people[JOB_PRIVATE+i] || reserve_soldiers_available[i])
			++rank_count;
	}


	if(rank_count)
	{
		// Gewünschten Rang an Hand der Militäreinstellungen ausrechnen, je nachdem wie stark verteidigt werden soll
		unsigned rank = (rank_count-1)*GAMECLIENT.GetPlayer(player)->military_settings[1]/5;

		// Gewünschten Rang suchen
		unsigned r = 0;
		for(unsigned i = 0;i<5;++i)
		{
			
			// anderere Soldaten bevorzugen
			if(real_goods.people[JOB_PRIVATE+i])
			{
				if(r == rank)
				{
					// diesen Soldaten wollen wir
					--real_goods.people[JOB_PRIVATE+i];
					nofDefender * soldier = new nofDefender(x,y,player,this,i,attacker);
					return soldier;
				}

				++r;
			}
			// Reserve
			else if(reserve_soldiers_available[i])
			{
				if(r == rank)
				{
					// diesen Soldaten wollen wir
					--reserve_soldiers_available[i];
					// bei der visuellen Warenanzahl wieder hinzufügen, da er dann wiederrum von der abgezogen wird, wenn
					// er rausgeht und es so ins minus rutschen würde
					++goods.people[JOB_PRIVATE+i];
					nofDefender * soldier = new nofDefender(x,y,player,this,i,attacker);
					return soldier;

				}

				++r;
			}
			
		}
	}

	// Kein Soldat gefunden, als letzten Hoffnung die Soldaten nehmen, die ggf in der Warteschlange noch hängen
	for(list<noFigure*>::iterator it = leave_house.begin();it.valid();++it)
	{
		// Soldat?
		if((*it)->GetGOT() == GOT_NOF_AGGRESSIVEDEFENDER)
		{
			static_cast<nofAggressiveDefender*>(*it)->NeedForHomeDefence();
			// Aus Missionsliste raushauen
			troops_on_mission.erase(static_cast<nofAggressiveDefender*>(*it));

			nofDefender * soldier = new nofDefender(x,y,player,this,static_cast<nofAggressiveDefender*>(*it)->GetRank(),attacker);
			(*it)->Destroy();
			delete *it;
			leave_house.erase(&it);
			return soldier;
		}
		else if((*it)->GetGOT() == GOT_NOF_PASSIVESOLDIER)
		{
			(*it)->Abrogate();
			nofDefender * soldier = new nofDefender(x,y,player,this,static_cast<nofPassiveSoldier*>(*it)->GetRank(),attacker);
			(*it)->Destroy();
			delete *it;
			leave_house.erase(&it);
			return soldier;
		}
	}

	return 0;

}


bool nobBaseWarehouse::AreRecruitingConditionsComply()
{
	// Mindestanzahl der Gehilfen die vorhanden sein müssen anhand der 1. Militäreinstellung ausrechnen
	unsigned needed_helpers = 100-20*GAMECLIENT.GetPlayer(player)->military_settings[0];

	// einer muss natürlich mindestens vorhanden sein!
	if(!needed_helpers) needed_helpers = 1;

	// Wenn alle Bedingungen erfüllt sind, Event anmelden
	return (real_goods.people[JOB_HELPER] >= needed_helpers && real_goods.goods[GD_SWORD]
	&& real_goods.goods[GD_SHIELDROMANS] && real_goods.goods[GD_BEER]);
}


void nobBaseWarehouse::TryRecruiting()
{
	// Wenn noch kein Event angemeldet wurde und alle Bedingungen erfüllt sind, kann ein neues angemeldet werden
	if(!recruiting_event)
	{
		if(AreRecruitingConditionsComply())
			recruiting_event = em->AddEvent(this,RECRUITE_GF+RANDOM.Rand(__FILE__,__LINE__,obj_id,RECRUITE_RANDOM_GF),2);
	}
}

void nobBaseWarehouse::TryStopRecruiting()
{
	// Wenn ein Event angemeldet wurde und die Bedingungen nicht mehr erfüllt sind, muss es wieder vernichtet werden
	if(recruiting_event)
	{
		if(!AreRecruitingConditionsComply())
		{
			em->RemoveEvent(recruiting_event);
			recruiting_event = 0;
		}
	}
}


bool FW::Condition_Ware(nobBaseWarehouse * wh, const void * param)
{
	return (wh->GetRealWaresCount(static_cast<const Param_Ware*>(param)->type) >= static_cast<const Param_Ware*>(param)->count);
}

bool FW::Condition_Job(nobBaseWarehouse * wh, const void * param)
{
	if(wh->GetRealFiguresCount(static_cast<const Param_Job*>(param)->type) >= static_cast<const Param_Job*>(param)->count)
		return true;
	else
	{
		// die entsprechende Figur ist nicht vorhanden, wenn das Werkzeug der Figur und ein Mann (Träger) zum Rekrutieren
		// da ist, geht das auch, nur bei Eseln nicht !!
		bool tool_available = (JOB_CONSTS[static_cast<const Param_Job*>(param)->type].tool != GD_NOTHING) ? 
			(wh->GetRealWaresCount(JOB_CONSTS[static_cast<const Param_Job*>(param)->type].tool)>0) : true;

		if(static_cast<const Param_Job*>(param)->type == JOB_PACKDONKEY)
			tool_available = false;

		if( wh->GetRealFiguresCount(JOB_HELPER) && tool_available)
			return true;

		return false;
	}
}



bool FW::Condition_WareAndJob(nobBaseWarehouse * wh, const void * param)
{
	return (Condition_Ware(wh,&static_cast<const Param_WareAndJob*>(param)->ware) &&
		Condition_Job(wh,&static_cast<const Param_WareAndJob*>(param)->job));
}

bool FW::Condition_Troops(nobBaseWarehouse * wh, const void * param)
{
	return (wh->GetSoldiersCount() >= *static_cast<const unsigned*>(param));
}

bool FW::NoCondition(nobBaseWarehouse * wh, const void * param)
{
	return true;
}

bool FW::Condition_StoreWare(nobBaseWarehouse * wh, const void * param)
{
	// Einlagern darf nicht verboten sein
	// Schilder beachten!
	if(*static_cast<const GoodType*>(param) == GD_SHIELDVIKINGS || *static_cast<const GoodType*>(param) == GD_SHIELDAFRICANS || 
		*static_cast<const GoodType*>(param) == GD_SHIELDJAPANESE)
		return (!wh->CheckRealInventorySettings(0,2,GD_SHIELDROMANS));
	else
		return (!wh->CheckRealInventorySettings(0,2,*static_cast<const GoodType*>(param)));
}

bool FW::Condition_StoreFigure(nobBaseWarehouse * wh, const void * param)
{
	// Einlagern darf nicht verboten sein, Bootstypen zu normalen Trägern machen
	if(*static_cast<const Job*>(param) == JOB_BOATCARRIER)
		return (!wh->CheckRealInventorySettings(1,2,JOB_HELPER));
	else
		return (!wh->CheckRealInventorySettings(1,2,*static_cast<const Job*>(param)));
}

const Goods *nobBaseWarehouse::GetInventory() const
{
	return &goods;
}

void nobBaseWarehouse::AddToInventory()
{
	for(unsigned int i = 0; i < WARE_TYPES_COUNT; ++i)
		GAMECLIENT.GetPlayer(player)->IncreaseInventoryWare(GoodType(i),real_goods.goods[i]);

	for(unsigned int i = 0; i < JOB_TYPES_COUNT; ++i)
		GAMECLIENT.GetPlayer(player)->IncreaseInventoryJob(Job(i),real_goods.people[i]);

}

//void nobBaseWarehouse::ChangeRealInventorySetting(const unsigned char * const wares,const unsigned char * const figures)
//{
//	memcpy(inventory_settings_real.wares,wares,36);
//	memcpy(inventory_settings_real.figures,figures,31);
//
//	// Evtl gabs verlorene Waren, die jetzt in das HQ wieder reinkönnen
//	GAMECLIENT.GetPlayer(player)->FindClientForLostWares();
//
//	// Sind Waren vorhanden, die ausgelagert werden müssen und ist noch kein Auslagerungsevent vorhanden --> neues anmelden
//	if(AreWaresToEmpty() && !empty_event.valid())
//		empty_event = em->AddEvent(this,EMPTY_INTERVAL,3);
//
//}

/// Verändert Ein/Auslagerungseinstellungen (real)
void nobBaseWarehouse::ChangeRealInventorySetting(unsigned char category,unsigned char state,unsigned char type)
{
	/// Einstellung ändern
	((category == 0)?inventory_settings_real.wares[type]:inventory_settings_real.figures[type]) ^= state; 

	/// Bei anderen Spielern als dem lokalen, der das in Auftrag gegeben hat, müssen die visuellen ebenfalls
	/// geändert werden oder auch bei Replays
	if(GameClient::inst().IsReplayModeOn() || GameClient::inst().GetPlayerID() != player)
		ChangeVisualInventorySettings(category,state,type);

	// Evtl gabs verlorene Waren, die jetzt in das HQ wieder reinkönnen
	if(state == 2)
		GAMECLIENT.GetPlayer(player)->FindClientForLostWares();

	// Sind Waren vorhanden, die ausgelagert werden müssen und ist noch kein Auslagerungsevent vorhanden --> neues anmelden
	if(state == 4 && ((category == 0)?real_goods.goods[type]:real_goods.people[type]) && !empty_event)
		empty_event = em->AddEvent(this,EMPTY_INTERVAL,3);
}

/// Verändert alle Ein/Auslagerungseinstellungen einer Kategorie (also Waren oder Figuren)(real)
void nobBaseWarehouse::ChangeAllRealInventorySettings(unsigned char category,unsigned char state)
{
	if(category == 0)
	{
		// Waren ändern
		for(unsigned i = 0;i<WARE_TYPES_COUNT;++i)
			inventory_settings_real.wares[i] ^= state;
	}
	else
	{
		// Figuren ändern
		for(unsigned i = 0;i<JOB_TYPES_COUNT;++i)
			inventory_settings_real.figures[i] ^= state;
	}

	// Evtl gabs verlorene Waren, die jetzt in das HQ wieder reinkönnen
	if(state == 2)
		GAMECLIENT.GetPlayer(player)->FindClientForLostWares();

	// Sind Waren vorhanden, die ausgelagert werden müssen und ist noch kein Auslagerungsevent vorhanden --> neues anmelden
	if(state == 4 && AreWaresToEmpty() && !empty_event)
		empty_event = em->AddEvent(this,EMPTY_INTERVAL,3);
}


bool nobBaseWarehouse::AreWaresToEmpty() const
{
	// Prüfen, ob Warentyp ausgelagert werden soll und ob noch Waren davon vorhanden sind
	// Waren überprüfen
	for(unsigned i = 0;i<WARE_TYPES_COUNT;++i)
	{
		if(CheckRealInventorySettings(0,4,i) && real_goods.goods[i])
			return true;
	}

	// Figuren überprüfen
	for(unsigned i = 0;i<JOB_TYPES_COUNT;++i)
	{
		if(CheckRealInventorySettings(1,4,i) && real_goods.people[i])
			return true;
	}

	return false;
}

bool nobBaseWarehouse::DefendersAvailable() const
{
	// Warenbestand und Reserve prüfen
	for(unsigned i = 0;i<5;++i)
	{
		// Reserve
		if(reserve_soldiers_available[i])
			return true;
		// Warenbestand
		if(real_goods.people[JOB_PRIVATE+i])
			return true;
	}

	return false;
}

unsigned nobBaseWarehouse::IncreaseReserveVisual(unsigned rank)
{
	return ++reserve_soldiers_claimed_visual[rank];
}

unsigned nobBaseWarehouse::DecreaseReserveVisual(unsigned rank)
{
	if(reserve_soldiers_claimed_visual[rank])
		--reserve_soldiers_claimed_visual[rank];

	return reserve_soldiers_claimed_visual[rank];
}

void nobBaseWarehouse::SetRealReserve(const unsigned rank, const unsigned count)
{
	reserve_soldiers_claimed_real[rank] = count;

	// Replay oder anderer Spieler? Dann die visuellen auch erhöhen
	if(GameClient::inst().IsReplayModeOn() || GameClient::inst().GetPlayerID() != player)
		reserve_soldiers_claimed_visual[rank] = count;

	// Geforderte Soldaten ggf. einbeziehen
	RefreshReserve(rank);
}

void nobBaseWarehouse::RefreshReserve(unsigned rank)
{
	// Zuviele oder zuwenig Soldaten einkassiert?
	if(reserve_soldiers_available[rank] < reserve_soldiers_claimed_real[rank])
	{
		// Zuwenig --> gucken,ob wir noch mehr einkassieren können
		if(real_goods.people[JOB_PRIVATE+rank])
		{
			// ja, dann nehmen wir mal noch soviele wie nötig und möglich
			unsigned add = min(real_goods.people[JOB_PRIVATE+rank], // möglich
				reserve_soldiers_claimed_real[rank]-reserve_soldiers_available[rank]); // nötig

			// Bei der Reserve hinzufügen
			reserve_soldiers_available[rank] += add;
			// vom Warenbestand abziehen
			goods.people[JOB_PRIVATE+rank] -= add;
			real_goods.people[JOB_PRIVATE+rank] -= add;
		}
	}
	else if(reserve_soldiers_available[rank] > reserve_soldiers_claimed_real[rank])
	{
		// Zuviele, dann wieder welche freigeben
		unsigned subtract = reserve_soldiers_available[rank] - reserve_soldiers_claimed_real[rank];

		// Bei der Reserve abziehen
		reserve_soldiers_available[rank] -= subtract;
		// beim Warenbestand hinzufügen
		goods.people[JOB_PRIVATE+rank] += subtract;
		real_goods.people[JOB_PRIVATE+rank] += subtract;

		// Ggf. Truppen in die Militärgebäude schicken
		GameClient::inst().GetPlayer(player)->RegulateAllTroops();
	}
	// ansonsten ists gleich und alles ist in Ordnung!
}

void nobBaseWarehouse::CheckOuthousing(unsigned char category, unsigned job_ware_id)
{
	// Bootsträger in Träger umwandeln, der evtl dann raus soll
	if(category == 1 && job_ware_id == JOB_BOATCARRIER)
		job_ware_id = JOB_HELPER;

	if(CheckRealInventorySettings(category,4,job_ware_id) && !empty_event)
		empty_event = em->AddEvent(this,EMPTY_INTERVAL,3);
}
