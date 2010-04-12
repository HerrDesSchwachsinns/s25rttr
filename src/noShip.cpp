// $Id: noShip.cpp 6311 2010-04-11 11:35:56Z OLiver $
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
#include "noShip.h"
#include "Loader.h"
#include "macros.h"
#include "GameClient.h"
#include "Random.h"
#include "EventManager.h"
#include "GameWorld.h"
#include "SerializedGameData.h"
#include "nobHarborBuilding.h"
#include "noFigure.h"
#include "Ware.h"
#include "PostMsg.h"
#include "AIEventManager.h"
#include "nofAttacker.h"

const unsigned SHIP_SPEED = 20;
const unsigned int ship_count = 55;

/// Zeit zum Beladen des Schiffes
const unsigned LOADING_TIME = 200;
/// Zeit zum Entladen des Schiffes
const unsigned UNLOADING_TIME = 200;

/// Maximaler Weg, der zur�ckgelegt werden kann bei einem Erkundungsschiff 
const unsigned MAX_EXPLORATION_EXPEDITION_DISTANCE = 100;
/// Zeit (in GF), die das Schiff bei der Erkundungs-Expedition jeweils an einem Punkt ankert
const unsigned EXPLORATION_EXPEDITION_WAITING_TIME = 300;

const std::string ship_names[NATION_COUNT][ship_count] = {
	/* Nubier */	{ "Aica", "Aida", "Ainra", "Alayna", "Alisha", "Alma", "Amila", "Anina", "Armina", "Banu", "Baya", "Bea", "Bia", "Bisa", "Cheche", "Dafina", "Daria", "Dina", "Do", "Dofi", "Efia", "Erin", "Esi", "Esra", "Fahari", "Faraya", "Fujo", "Ghiday", "Habiaba", "Hajunza", "Ina", "Layla", "Lenia", "Lillian", "Malika", "Mona", "Naja", "Neriman", "Nyela", "Olufunmilayo", "Panyin", "Rayyan", "Rhiannon", "Safiya", "Sahra", "Selda", "Senna", "Shaira", "Shakira", "Sharina", "Sinah", "Suada", "Sulamith", "Tiada", "Yelda" },
	/* Japaner */	{ "Ai", "Aiko", "Aimi", "Akemi", "Amaya", "Aoi", "Ayaka", "Ayano", "Beniko", "Chiyo", "Chiyoko", "Emi", "Fumiko", "Haruka", "Hiroko", "Hotaru", "Kaori", "Kasumi", "Kazuko", "Kazumi", "Keiko", "Kiriko", "Kumiko", "Mai", "Mayumi", "Megumi", "Midori", "Misaki", "Miu", "Moe", "Nanami", "Naoko", "Naomi", "Natsuki", "Noriko", "Reika", "Sachiko", "Sadako", "Sakura", "Satsuki", "Sayuri", "Setsuko", "Shigeko", "Teiko", "Tomomi", "Umeko", "Yoko", "Yoshiko", "Youko", "Yukiko", "Yumi", "Yumiko", "Yuna", "Yuuka", "Yuzuki" },
	/* R�mer */		{ "Antia", "Ateia", "Aurelia", "Camilia", "Claudia", "Duccia", "Epidia", "Equitia", "Fabia", "Galeria", "Helvetia", "Iunia", "Iusta", "Iuventia", "Lafrenia", "Livia", "Longinia", "Maelia", "Maxima", "Nigilia", "Nipia", "Norbana", "Novia", "Orania", "Otacilia", "Petronia", "Pinaria", "Piscia", "Pisentia", "Placidia", "Quintia", "Quirinia", "Rusonia", "Rutilia", "Sabucia", "Sallustia", "Salonia", "Salvia", "Scribonia", "Secundia", "Secundinia", "Tadia", "Talmudia", "Tanicia", "Tertinia", "Tita", "Ulpia", "Umbrenia", "Valeria", "Varia", "Vassenia", "Vatinia", "Vedia", "Velia", "Verania" },
	/* Wikinger */	{ "Adelberga", "Adelgund", "Adelheid", "Adelinde", "Alsuna", "Alwina", "Amelinde", "Astrid", "Baltrun", "Bernhild", "Bothilde", "Dagny", "Dankrun", "Eldrid", "Erlgard", "Fehild ", "Ferun", "Frauke ", "Freya", "Gerda ", "Gesa", "Gismara", "Hella", "Henrike ", "Hilke", "Ida", "Irma", "Irmlinde", "Isantrud ", "Kunheide", "Kunigunde", "Lioba", "Lykke", "Marada", "Margard ", "Merlinde", "Minnegard", "Nanna", "Norwiga", "Oda", "Odarike", "Osrun ", "Raginhild ", "Raskild ", "Rinelda", "Runa", "Runhild ", "Salgard", "Sarhild", "Tanka", "Tyra", "Ulla", "Uta", "Walda", "Wiebke" }
};

//{"FloSoftius", "Demophobius", "Olivianus", "Spikeonius", "Nastius"};

/// Positionen der Flaggen am Schiff f�r die 6 unterschiedlichen Richtungen jeweils#
const Point<int> SHIPS_FLAG_POS[12] =
{
	// Und wenn das Schiff steht und Segel nicht gehisst hat
	Point<int>(-3,-77),
	Point<int>(-6,-71),
	Point<int>(-3,-71),
	Point<int>(-7,-77),
	Point<int>(-1,-70),
	Point<int>(-1,-70),

	// Und wenn es f�hrt
	Point<int>(3,-70),
	Point<int>(0,-64),
	Point<int>(3,-64),
	Point<int>(-1,-70),
	Point<int>(5,-63),
	Point<int>(5,-63)
};

/// Konstruktor
noShip::noShip(const unsigned short x, const unsigned short y, const unsigned char player) 
	: noMovable(NOP_SHIP, x, y), 
	player(player), state(STATE_IDLE), sea_id(0), goal_harbor_id(0), goal_dir(0), 
	name(ship_names[gwg->GetPlayer(player)->nation][Random::inst().Rand(__FILE__, __LINE__, this->obj_id, ship_count)]),
	lost(false), remaining_sea_attackers(0), home_harbor(0), covered_distance(0)
{
	// Meer ermitteln, auf dem dieses Schiff f�hrt
	for(unsigned i = 0;i<6;++i)
	{
		unsigned short sea_id = gwg->GetNodeAround(x,y,i).sea_id;
		if(sea_id)
			this->sea_id = sea_id;
	}

	// Auf irgendeinem Meer m�ssen wir ja sein
	assert(sea_id > 0);

	gwg->AddFigure(this,x,y);

	// Schiff registrieren lassen
	players->getElement(player)->RegisterShip(this);
}

void noShip::Serialize(SerializedGameData * sgd) const
{
	Serialize_noMovable(sgd);

	sgd->PushUnsignedChar(player);
	sgd->PushUnsignedChar(static_cast<unsigned char>(state));
	sgd->PushUnsignedShort(sea_id);
	sgd->PushUnsignedInt(goal_harbor_id);
	sgd->PushUnsignedChar(goal_dir);
	sgd->PushString(name);
	sgd->PushUnsignedInt(pos);
	sgd->PushUnsignedInt(route.size());
	sgd->PushBool(lost);
	sgd->PushUnsignedInt(remaining_sea_attackers);
	sgd->PushUnsignedInt(home_harbor);
	sgd->PushUnsignedInt(covered_distance);
	for(unsigned i =0;i<route.size();++i)
		sgd->PushUnsignedChar(route[i]);
	sgd->PushObjectList(figures,false);
	sgd->PushObjectList(wares,true);
	
	
}

noShip::noShip(SerializedGameData * sgd, const unsigned obj_id) : 
noMovable(sgd,obj_id),
player(sgd->PopUnsignedChar()),
state(State(sgd->PopUnsignedChar())),
sea_id(sgd->PopUnsignedShort()),
goal_harbor_id(sgd->PopUnsignedInt()),
goal_dir(sgd->PopUnsignedChar()),
name(sgd->PopString()),
pos(sgd->PopUnsignedInt()),
route(sgd->PopUnsignedInt()),
lost(sgd->PopBool()),
remaining_sea_attackers(sgd->PopUnsignedInt()),
home_harbor(sgd->PopUnsignedInt()),
covered_distance(sgd->PopUnsignedInt())
{
	for(unsigned i = 0;i<route.size();++i)
		route[i] = sgd->PopUnsignedChar();
	sgd->PopObjectList(figures,GOT_UNKNOWN);
	sgd->PopObjectList(wares,GOT_WARE);
}

void noShip::Destroy()
{
	// Schiff wieder abmelden
	players->getElement(player)->RemoveShip(this);
}

/// Zeichnet das Schiff stehend mit oder ohne Waren
void noShip::DrawFixed(const int x, const int y, const bool draw_wares)
{
	LOADER.GetImageN("boot_z",  ((dir+3)%6)*2 + 1)->Draw(x,y,0,0,0,0,0,0,COLOR_SHADOW);
	LOADER.GetImageN("boot_z",  ((dir+3)%6)*2)->Draw(x,y);
	
	if(draw_wares)
		/// Waren zeichnen 
		LOADER.GetImageN("boot_z",  30+((dir+3)%6))->Draw(x,y);
}

void noShip::Draw(int x, int y)
{
	unsigned flag_drawing_type = 1;
	
	// Sind wir verloren? Dann immer stehend zeichnen
	if(lost)
	{
		DrawFixed(x,y,true);
		return;
	}
	
 	switch(state)
	{
	default:
		break;
	case STATE_IDLE:
	case STATE_SEAATTACK_WAITING:
		{
			DrawFixed(x,y,false);
			flag_drawing_type = 0;
		} break;
	
	case STATE_GOTOHARBOR:
		{
			DrawDriving(x,y);
		} break;
	case STATE_EXPEDITION_LOADING:
	case STATE_EXPEDITION_UNLOADING:
	case STATE_TRANSPORT_LOADING:
	case STATE_TRANSPORT_UNLOADING:
	case STATE_SEAATTACK_LOADING:
	case STATE_SEAATTACK_UNLOADING:
	case STATE_EXPLORATIONEXPEDITION_LOADING:
	case STATE_EXPLORATIONEXPEDITION_UNLOADING:
		{
			DrawFixed(x,y,false);
		} break;
	case STATE_EXPLORATIONEXPEDITION_WAITING:
	case STATE_EXPEDITION_WAITING:
	
		{
			DrawFixed(x,y,true);
		} break;
	case STATE_EXPEDITION_DRIVING:
	case STATE_TRANSPORT_DRIVING:
	case STATE_SEAATTACK_DRIVINGTODESTINATION:
	case STATE_EXPLORATIONEXPEDITION_DRIVING:
	
		{
			DrawDrivingWithWares(x,y);
		} break;
	case STATE_SEAATTACK_RETURN:
		{
			if(figures.size() || wares.size())
				DrawDrivingWithWares(x,y);
			else
				DrawDriving(x,y);
		} break;
	}

	LOADER.GetImageN("boot_z", 40 + GAMECLIENT.GetGlobalAnimation(6,1,1,obj_id))->
		Draw(x+SHIPS_FLAG_POS[dir+flag_drawing_type*6].x,y+SHIPS_FLAG_POS[dir+flag_drawing_type*6].y,0,0,0,0,0,0,COLOR_WHITE, COLORS[gwg->GetPlayer(player)->color]);
	// Second, white flag, only when on expedition, always swinging in the opposite direction
	if(state >= STATE_EXPEDITION_LOADING && state <= STATE_EXPEDITION_DRIVING)
	LOADER.GetImageN("boot_z", 40 + GAMECLIENT.GetGlobalAnimation(6,1,1,obj_id+4))->
		Draw(x+SHIPS_FLAG_POS[dir+flag_drawing_type*6].x,y+4+SHIPS_FLAG_POS[dir+flag_drawing_type*6].y,0,0,0,0,0,0,COLOR_WHITE, COLOR_WHITE);

}
/// Zeichnet normales Fahren auf dem Meer ohne irgendwelche G�ter
void noShip::DrawDriving(int& x, int& y)
{
	// Interpolieren zwischen beiden Knotenpunkten
	CalcWalkingRelative(x,y);

	LOADER.GetImageN("boot_z", 13+((dir+3)%6)*2)->Draw(x,y,0,0,0,0,0,0,COLOR_SHADOW);
	LOADER.GetImageN("boot_z", 12+((dir+3)%6)*2)->Draw(x,y);
}

/// Zeichnet normales Fahren auf dem Meer mit G�tern
void noShip::DrawDrivingWithWares(int& x, int& y)
{
	// Interpolieren zwischen beiden Knotenpunkten
	CalcWalkingRelative(x,y);

	LOADER.GetImageN("boot_z", 13+((dir+3)%6)*2)->Draw(x,y,0,0,0,0,0,0,COLOR_SHADOW);
	LOADER.GetImageN("boot_z", 12+((dir+3)%6)*2)->Draw(x,y);
	/// Waren zeichnen 
	LOADER.GetImageN("boot_z",  30+((dir+3)%6))->Draw(x,y);
}


void noShip::HandleEvent(const unsigned int id)
{
	current_ev = 0;

	switch(id)
	{
	// Laufevent
	case 0:
		{
			// neue Position einnehmen
			Walk();

			// entscheiden, was als n�chstes zu tun ist
			Driven();
		} break;
	default:
		{
			switch(state)
			{
			default: break;
			case STATE_EXPEDITION_LOADING:
				{
					// Schiff ist nun bereit und Expedition kann beginnen
					state = STATE_EXPEDITION_WAITING;

					// Spieler benachrichtigen
					if(GameClient::inst().GetPlayerID() == this->player)
						GAMECLIENT.SendPostMessage(new ShipPostMsg(_("A ship is ready for an expedition."), PMC_GENERAL, GAMECLIENT.GetPlayer(player)->nation, x, y));

					// KI Event senden
					GAMECLIENT.SendAIEvent(new AIEvent::Location(AIEvent::ExpeditionWaiting, x, y), player);
				} break;
			case STATE_EXPLORATIONEXPEDITION_LOADING:
				{
					// Schiff ist nun bereit und Expedition kann beginnen
					ContinueExplorationExpedition();
				} break;
			case STATE_EXPLORATIONEXPEDITION_WAITING:
				{
					// Schiff ist nun bereit und Expedition kann beginnen
					ContinueExplorationExpedition();
				} break;
				
			case STATE_EXPEDITION_UNLOADING:
				{			
						// Hafen herausfinden 
					Point<MapCoord> goal_pos(gwg->GetHarborPoint(goal_harbor_id));
					noBase * hb = gwg->GetNO(goal_pos.x,goal_pos.y);
					
					if(hb->GetGOT() == GOT_NOB_HARBORBUILDING)
					{
						Goods goods;
						memset(&goods,0,sizeof(Goods));
						unsigned char nation = players->getElement(player)->nation;
						goods.goods[GD_BOARDS] = BUILDING_COSTS[nation][BLD_HARBORBUILDING].boards;
						goods.goods[GD_STONES] = BUILDING_COSTS[nation][BLD_HARBORBUILDING].stones;
						goods.people[JOB_BUILDER] = 1;
						static_cast<nobBaseWarehouse*>(hb)->AddGoods(goods);
						// Wieder idlen und ggf. neuen Job suchen
						StartIdling();
						players->getElement(player)->GetJobForShip(this);
					}
					else
					{
						// Erstmal verloren
						// Neuen Hafen suchen
						if(players->getElement(player)->FindHarborForUnloading
							(this,x,y,&goal_harbor_id,&route,NULL))
							HandleState_ExpeditionDriving();
						else
							// Ansonsten als verloren markieren, damit uns sp�ter Bescheid gesagt wird
							// wenn es einen neuen Hafen gibt
							lost = true;
					}
					
				} break;
			case STATE_EXPLORATIONEXPEDITION_UNLOADING:
				{
					// Hafen herausfinden 
					Point<MapCoord> goal_pos(gwg->GetHarborPoint(goal_harbor_id));
					noBase * hb = gwg->GetNO(goal_pos.x,goal_pos.y);
					
					unsigned old_visual_range = GetVisualRange();
					
					if(hb->GetGOT() == GOT_NOB_HARBORBUILDING)
					{
						// Sp�her wieder entladen
						Goods goods;
						memset(&goods,0,sizeof(Goods));
						goods.people[JOB_SCOUT] = SCOUTS_EXPLORATION_EXPEDITION;
						static_cast<nobBaseWarehouse*>(hb)->AddGoods(goods);
						// Wieder idlen und ggf. neuen Job suchen
						StartIdling();
						players->getElement(player)->GetJobForShip(this);
					}
					else
					{
						// Erstmal verloren
						// Neuen Hafen suchen
						if(players->getElement(player)->FindHarborForUnloading
							(this,x,y,&goal_harbor_id,&route,NULL))
							HandleState_ExplorationExpeditionDriving();
						else
							// Ansonsten als verloren markieren, damit uns sp�ter Bescheid gesagt wird
							// wenn es einen neuen Hafen gibt
							lost = true;
					}
					
					// Sichtbarkeiten neu berechnen
					gwg->RecalcVisibilitiesAroundPoint(x,y,old_visual_range,player,NULL);

					
				} break;
			case STATE_TRANSPORT_LOADING:
				{
					StartTransport();
				} break;
			case STATE_TRANSPORT_UNLOADING:
				{
					// Hafen herausfinden 
					Point<MapCoord> goal_pos(gwg->GetHarborPoint(goal_harbor_id));
					noBase * hb = gwg->GetNO(goal_pos.x,goal_pos.y);
					if(hb->GetGOT() == GOT_NOB_HARBORBUILDING)
					{
						static_cast<nobHarborBuilding*>(hb)->ReceiveGoodsFromShip(figures,wares);
						figures.clear();
						wares.clear();
					}
					else
					{
						// todo
						assert(false);
					}

					// Wieder idlen und ggf. neuen Job suchen
					StartIdling();
					players->getElement(player)->GetJobForShip(this);
				} break;
			case STATE_SEAATTACK_LOADING:
				{
					StartSeaAttack();
				} break;
			case STATE_SEAATTACK_WAITING:
				{
					// N�chsten Soldaten nach drau�en beordern
					if(!figures.size())
						break;

					nofAttacker * attacker = static_cast<nofAttacker*>(*figures.begin());
					// Evtl. ist ein Angreifer schon fertig und wieder an Board gegangen
					// der darf dann nat�rlich nicht noch einmal raus, sonst kann die sch�ne Reise
					// b�se enden
					if(attacker->IsSeaAttackCompleted())
						break;

					figures.pop_front();
					gwg->AddFigure(attacker,x,y);
					attacker->StartAttackOnOtherIsland(x,y,obj_id);
					current_ev = em->AddEvent(this,30,1);
				};
			case STATE_SEAATTACK_UNLOADING:
				{
				} break;
			}
		} break;

	}

}

void noShip::StartDriving(const unsigned char dir)
{
	StartMoving(dir,SHIP_SPEED);
}

void noShip::Driven()
{
	gwg->RecalcMovingVisibilities(x,y,player,GetVisualRange(),dir);
	//gwg->RecalcVisibilitiesAroundPoint(gwg->GetXA(x,y,(dir+3)%6),gwg->GetYA(x,y,(dir+3)%6),
	//	GetVisualRange()+2,player,NULL);
	//gwg->RecalcVisibilitiesAroundPoint(x,y,GetVisualRange()+2,player,NULL);


	switch(state)
	{
	case STATE_GOTOHARBOR: HandleState_GoToHarbor(); break;
	case STATE_EXPEDITION_DRIVING: HandleState_ExpeditionDriving(); break;
	case STATE_EXPLORATIONEXPEDITION_DRIVING: HandleState_ExplorationExpeditionDriving(); break;
	case STATE_TRANSPORT_DRIVING: HandleState_TransportDriving(); break;
	case STATE_SEAATTACK_DRIVINGTODESTINATION: HandleState_SeaAttackDriving(); break;
	case STATE_SEAATTACK_RETURN: HandleState_SeaAttackReturn(); break;
	default:
		break;
	}
}

/// Gibt Sichtradius dieses Schiffes zur�ck
unsigned noShip::GetVisualRange() const
{
	// Erkundungsschiffe haben einen gr��eren Sichtbereich
	if(state >= STATE_EXPLORATIONEXPEDITION_LOADING && state <= STATE_EXPLORATIONEXPEDITION_DRIVING)
		return VISUALRANGE_EXPLORATION_SHIP;
	else
		return VISUALRANGE_SHIP;
}




/// F�hrt zum Hafen, um dort eine Mission (Expedition) zu erledigen
void noShip::GoToHarbor(nobHarborBuilding * hb, const std::vector<unsigned char>& route)
{
	state = STATE_GOTOHARBOR;

	goal_harbor_id = gwg->GetNode(hb->GetX(),hb->GetY()).harbor_id;

	// Route merken
	this->route = route;
	pos = 1;

	// losfahren
	StartDriving(route[0]);
}

void noShip::HandleState_GoToHarbor()
{
	// Hafen schon zerst�rt?
	if(goal_harbor_id == 0)
	{
		StartIdling();
	}
	else
	{
		Result res = DriveToHarbour();
		switch(res)
		{
		default: return;
		case GOAL_REACHED:
			{

				Point<MapCoord> goal(gwg->GetHarborPoint(goal_harbor_id));
				// Erstmal nichts machen und idlen
				StartIdling();
				// Hafen Bescheid sagen, dass wir da sind (falls er �berhaupt noch existiert)
				noBase * nb = gwg->GetNO(goal.x,goal.y);
				if(nb->GetGOT() == GOT_NOB_HARBORBUILDING)
					static_cast<nobHarborBuilding*>(nb)->ShipArrived(this);

				
			} break;
		case NO_ROUTE_FOUND:
			{
				Point<MapCoord> goal(gwg->GetHarborPoint(goal_harbor_id));
				// Dem Hafen Bescheid sagen
				gwg->GetSpecObj<nobHarborBuilding>(goal.x,goal.y)->ShipLost(this);
				// Nichts machen und idlen
				StartIdling();
			} break;
		case HARBOR_DOESNT_EXIST:
			{
				// Nichts machen und idlen
				StartIdling();
			} break;
		}
	}
}

/// Startet eine Expedition
void noShip::StartExpedition()
{
	/// Schiff wird "beladen", also kurze Zeit am Hafen stehen, bevor wir bereit sind
	state = STATE_EXPEDITION_LOADING;
	current_ev = em->AddEvent(this,LOADING_TIME,1);
	home_harbor = goal_harbor_id;
}

/// Startet eine Erkundungs-Expedition
void noShip::StartExplorationExpedition()
{
	/// Schiff wird "beladen", also kurze Zeit am Hafen stehen, bevor wir bereit sind
	state = STATE_EXPLORATIONEXPEDITION_LOADING;
	current_ev = em->AddEvent(this,LOADING_TIME,1);
	covered_distance = 0;
	home_harbor = goal_harbor_id;
	// Sichtbarkeiten neu berechnen
	gwg->SetVisibilitiesAroundPoint(x,y,GetVisualRange(),player);
}


/// F�hrt weiter zu einem Hafen
noShip::Result noShip::DriveToHarbour()
{
	Point<MapCoord> goal(gwg->GetHarborPoint(goal_harbor_id));
	
	// Existiert der Hafen �berhaupt noch?
	if(gwg->GetGOT(goal.x,goal.y) != GOT_NOB_HARBORBUILDING)
		return HARBOR_DOESNT_EXIST;

	return DriveToHarbourPlace();

	//nobHarborBuilding * hb = gwg->GetSpecObj<nobHarborBuilding>(goal.x,goal.y);
}

/// F�hrt weiter zu Hafenbauplatz
noShip::Result noShip::DriveToHarbourPlace()
{
	// Sind wir schon da?
	if(pos == route.size())
		return GOAL_REACHED;
	else
	{
		// Punkt, zu dem uns der Hafen hinf�hren will (K�stenpunkt)
		MapCoord coastal_x, coastal_y;
		gwg->GetCoastalPoint(goal_harbor_id,&coastal_x, &coastal_y,sea_id);

		MapCoord goal_x_route, goal_y_route;

		// Route �berpr�fen
		if(!gwg->CheckShipRoute(x,y,route,pos,&goal_x_route,&goal_y_route))
		{
			// Route kann nicht mehr passiert werden --> neue Route suchen
			if(!gwg->FindShipPath(x,y,coastal_x,coastal_y,&route,NULL))
			{
				// Wieder keine gefunden -> raus
				return NO_ROUTE_FOUND;
			}

			// Wir fangen bei der neuen Route wieder von vorne an
			pos = 0;
		}

		// Kommen wir auch mit unser bisherigen Route an dem urspr�nglich anvisierten Hafenplatz an?
		if(!(coastal_x == goal_x_route && coastal_y == goal_y_route))
		{
			// Nein, d.h. wenn wir schon nah dran sind, m�ssen wir die Route neu berechnen
			if(route.size()-pos < 10)
			{
				if(!gwg->FindShipPath(x,y,coastal_x,coastal_y,&route,NULL))
					// Keiner gefunden -> raus
					return NO_ROUTE_FOUND;

				pos = 0;
			}
		}

		StartDriving(route[pos++]);
		return DRIVING;
	}

}


unsigned noShip::GetCurrentHarbor() const
{
	assert(state == STATE_EXPEDITION_WAITING);
	return goal_harbor_id;
}


/// Weist das Schiff an, in einer bestimmten Richtung die Expedition fortzusetzen
void noShip::ContinueExpedition(const unsigned char dir)
{
	if(state != STATE_EXPEDITION_WAITING)
		return;

	assert(state == STATE_EXPEDITION_WAITING);

	// N�chsten Hafenpunkt in dieser Richtung suchen
	unsigned new_goal = gwg->GetNextFreeHarborPoint(x,y,goal_harbor_id,dir,player);

	// Auch ein Ziel gefunden?
	if(!new_goal)
		return;

	MapCoord coastal_x, coastal_y;
	gwg->GetCoastalPoint(new_goal,&coastal_x, &coastal_y,sea_id);

	// Versuchen, Weg zu finden
	if(!gwg->FindShipPath(x,y,coastal_x,coastal_y,&route,NULL))
		return;

	// Dann fahren wir da mal hin
	pos = 0;
	goal_harbor_id = new_goal;
	state = STATE_EXPEDITION_DRIVING;

	StartDriving(route[pos++]);
}

/// Weist das Schiff an, eine Expedition abzubrechen (nur wenn es steht) und zum
/// Hafen zur�ckzukehren
void noShip::CancelExpedition()
{
	// Zum Heimathafen zur�ckkehren
	
	// Oder sind wir schon dort?
	if(goal_harbor_id == home_harbor)
	{
		route.clear();
		pos = 0;
		HandleState_ExpeditionDriving();
	}
	else
	{
		state = STATE_EXPEDITION_DRIVING;
		goal_harbor_id = home_harbor;
		StartDrivingToHarborPlace();
		HandleState_ExpeditionDriving();
	}
}

/// Weist das Schiff an, an der aktuellen Position einen Hafen zu gr�nden
void noShip::FoundColony()
{
	if(state != STATE_EXPEDITION_WAITING)
		return;

	// Kolonie gr�nden
	if(gwg->FoundColony(goal_harbor_id,player,sea_id))
	{
		// Dann idlen wir wieder
		StartIdling();
		// Neue Arbeit suchen
		players->getElement(player)->GetJobForShip(this);
	}

}


void noShip::HandleState_ExpeditionDriving()
{
	Result res;
	// Zum Heimathafen fahren?
	if(home_harbor == goal_harbor_id)
		res = DriveToHarbour();
	else
		res = DriveToHarbourPlace();
		
	switch(res)
	{
	default: return;
	case GOAL_REACHED:
		{
			// Haben wir unsere Expedition beendet?
			if(home_harbor == goal_harbor_id)
			{
				// Sachen wieder in den Hafen verladen
				state = STATE_EXPEDITION_UNLOADING;
				current_ev = em->AddEvent(this,UNLOADING_TIME,1);
			}
			else
			{
				// Warten auf weitere Anweisungen
				state = STATE_EXPEDITION_WAITING;

				// Spieler benachrichtigen
				if(GameClient::inst().GetPlayerID() == this->player)
					GAMECLIENT.SendPostMessage(new ShipPostMsg(_("A ship has reached the destination of its expedition."), PMC_GENERAL, GAMECLIENT.GetPlayer(player)->nation, x, y));

				// KI Event senden
				GAMECLIENT.SendAIEvent(new AIEvent::Location(AIEvent::ExpeditionWaiting, x, y), player);
			}
		} break;
	case NO_ROUTE_FOUND:
		{
			Point<MapCoord> goal(gwg->GetHarborPoint(goal_harbor_id));
			// Nichts machen und idlen
			StartIdling();
		} break;
	}
}

void noShip::HandleState_ExplorationExpeditionDriving()
{
	Result res;
	// Zum Heimathafen fahren?
	if(home_harbor == goal_harbor_id && covered_distance >= MAX_EXPLORATION_EXPEDITION_DISTANCE)
		res = DriveToHarbour();
	else
		res = DriveToHarbourPlace();
		
	switch(res)
	{
	default: return;
	case GOAL_REACHED:
		{
			// Haben wir unsere Expedition beendet?
			if(home_harbor == goal_harbor_id && covered_distance >= MAX_EXPLORATION_EXPEDITION_DISTANCE)
			{
				// Dann sind wir fertig -> wieder entladen
				state = STATE_EXPLORATIONEXPEDITION_UNLOADING;
				current_ev = em->AddEvent(this,UNLOADING_TIME,1);
			}
			else
			{
				// Strecke, die wir gefahren sind, draufaddieren
				covered_distance += route.size();
				// Erstmal kurz ausruhen an diesem Punkt und das Rohr ausfahren, um ein bisschen
				// auf der Insel zu gucken
				state = STATE_EXPLORATIONEXPEDITION_WAITING;
				current_ev = em->AddEvent(this,EXPLORATION_EXPEDITION_WAITING_TIME,1);
			}

		} break;
	case NO_ROUTE_FOUND:
		{
			Point<MapCoord> goal(gwg->GetHarborPoint(goal_harbor_id));
			unsigned old_visual_range = GetVisualRange();
			// Nichts machen und idlen
			StartIdling();
			// Sichtbarkeiten neu berechnen
			gwg->RecalcVisibilitiesAroundPoint(x,y,old_visual_range,player,NULL);
		} break;
	case HARBOR_DOESNT_EXIST:
		{
			// Neuen Hafen suchen
			if(players->getElement(player)->FindHarborForUnloading
				(this,x,y,&goal_harbor_id,&route,NULL))
				HandleState_TransportDriving();
			else
				// Ansonsten als verloren markieren, damit uns sp�ter Bescheid gesagt wird
				// wenn es einen neuen Hafen gibt
				lost = true;
			
		} break;
	}
}

void noShip::HandleState_TransportDriving()
{
	Result res = DriveToHarbour();
	switch(res)
	{
	default: return;
	case GOAL_REACHED:
		{
			// Waren abladen, daf�r wieder kurze Zeit hier ankern
			state = STATE_TRANSPORT_UNLOADING;
			current_ev = em->AddEvent(this,UNLOADING_TIME,1);
			
		} break;
	case NO_ROUTE_FOUND:
		{
			// Nichts machen und idlen
			StartIdling();
		} break;
	case HARBOR_DOESNT_EXIST:
		{
			// Kein Hafen mehr?
			// Dann m�ssen alle Leute ihren Heimatgeb�uden Bescheid geben, dass sie
			// nun nicht mehr kommen
			// Das Schiff muss einen Notlandeplatz ansteuern
			for(std::list<noFigure*>::iterator it = figures.begin();it!=figures.end();++it)
				(*it)->Abrogate();
			for(std::list<Ware*>::iterator it = wares.begin();it!=wares.end();++it)
			{
				(*it)->NotifyGoalAboutLostWare();
				(*it)->goal = NULL;
			}
			
			// Neuen Hafen suchen
			if(players->getElement(player)->FindHarborForUnloading
				(this,x,y,&goal_harbor_id,&route,NULL))
				HandleState_TransportDriving();
			else
				// Ansonsten als verloren markieren, damit uns sp�ter Bescheid gesagt wird
				// wenn es einen neuen Hafen gibt
				lost = true;
			
		} break;
	}
}

/// Gibt zur�ck, ob das Schiff jetzt in der Lage w�re, eine Kolonie zu gr�nden
bool noShip::IsAbleToFoundColony() const
{
	// Warten wir gerade?
	if(state == STATE_EXPEDITION_WAITING)
	{
		// Ist der Punkt, an dem wir gerade ankern, noch frei?
		if(gwg->IsHarborPointFree(goal_harbor_id,player,sea_id))
			return true;
	}

	return false;
}

/// Gibt zur�ck, ob das Schiff einen bestimmten Hafen ansteuert
bool noShip::IsGoingToHarbor(nobHarborBuilding * hb) const
{
	return (goal_harbor_id == hb->GetHarborPosID() && 
		(state == STATE_GOTOHARBOR || state == STATE_TRANSPORT_DRIVING
		|| state == STATE_TRANSPORT_LOADING || state == STATE_TRANSPORT_UNLOADING));
}

/// Bel�dt das Schiff mit Waren und Figuren, um eine Transportfahrt zu starten
void noShip::PrepareTransport(Point<MapCoord> goal, const std::list<noFigure*>& figures, const std::list<Ware*>& wares)
{
	// ID von Zielhafen herausfinden
	noBase * nb = gwg->GetNO(goal.x,goal.y);
	assert(nb->GetGOT() == GOT_NOB_HARBORBUILDING);
	this->goal_harbor_id = static_cast<nobHarborBuilding*>(nb)->GetHarborPosID();

	this->figures = figures;
	this->wares = wares;

	state = STATE_TRANSPORT_LOADING;
	current_ev = em->AddEvent(this,LOADING_TIME,1);
}

/// Bel�dt das Schiff mit Schiffs-Angreifern
void noShip::PrepareSeaAttack(Point<MapCoord> goal, const std::list<noFigure*>& figures)
{
	// Heimathafen merken
	home_harbor = goal_harbor_id;
	this->goal_harbor_id = gwg->GetHarborPointID(goal.x,goal.y);
	this->figures = figures;
	for(std::list<noFigure*>::iterator it = this->figures.begin();it!=this->figures.end();++it)
	{
		static_cast<nofAttacker*>(*it)->StartShipJourney(goal);
		static_cast<nofAttacker*>(*it)->SeaAttackStarted();
	}
	state = STATE_SEAATTACK_LOADING;
	current_ev = em->AddEvent(this,LOADING_TIME,1);
}

/// Startet Schiffs-Angreiff
void noShip::StartSeaAttack()
{
	state = STATE_SEAATTACK_DRIVINGTODESTINATION;
	StartDrivingToHarborPlace();
	HandleState_SeaAttackDriving();
}

void noShip::HandleState_SeaAttackDriving()
{
	Result res = DriveToHarbourPlace();
	switch(res)
	{
	default: return;
	case GOAL_REACHED:
		{
			// Ziel erreicht, dann stellen wir das Schiff hier hin und die Soldaten laufen nacheinander
			// raus zum Ziel
			state = STATE_SEAATTACK_WAITING;
			current_ev = em->AddEvent(this,15,1);
			remaining_sea_attackers = figures.size();

		} break;
	case NO_ROUTE_FOUND:
		{
			Point<MapCoord> goal(gwg->GetHarborPoint(goal_harbor_id));
			// Nichts machen und idlen
			StartIdling();
		} break;
	}
}

void noShip::HandleState_SeaAttackReturn()
{
	Result res = DriveToHarbour();
	switch(res)
	{
	default: return;
	case GOAL_REACHED:
		{
			// Entladen
			state = STATE_SEAATTACK_UNLOADING;
			this->current_ev = em->AddEvent(this,UNLOADING_TIME,1);
		} break;
	case HARBOR_DOESNT_EXIST:
		{
			// Kein Hafen mehr?
			// Dann m�ssen alle Angreifer ihren Heimatgeb�uden Bescheid geben, dass sie
			// nun nicht mehr kommen
			// Das Schiff muss einen Notlandeplatz ansteuern
			for(std::list<noFigure*>::iterator it = figures.begin();it!=figures.end();++it)
				static_cast<nofAttacker*>(*it)->CancelAtHomeMilitaryBuilding();

			state = STATE_TRANSPORT_DRIVING;
			
			// Neuen Hafen suchen
			if(players->getElement(player)->FindHarborForUnloading
				(this,x,y,&goal_harbor_id,&route,NULL))
				HandleState_TransportDriving();
			else
				lost = true;

			
		} break;
	case NO_ROUTE_FOUND:
		{
			Point<MapCoord> goal(gwg->GetHarborPoint(goal_harbor_id));
			// Nichts machen und idlen
			StartIdling();
		} break;
	}
}

/// F�ngt an zu einem Hafen zu fahren (berechnet Route usw.)
void noShip::StartDrivingToHarborPlace()
{
	MapCoord coastal_x, coastal_y;
	gwg->GetCoastalPoint(goal_harbor_id,&coastal_x, &coastal_y,sea_id);

	// Versuchen, Weg zu finden
	if(!gwg->FindShipPath(x,y,coastal_x,coastal_y,&route,NULL))
	{
		// todo
		LOG.lprintf("Achtung: Bug im Spiel: noShip::StartDrivingToHarborPlace: Schiff hat keinen Weg gefunden!\n");
		return;
	}
	pos = 0;
}

/// Startet die eigentliche Transportaktion, nachdem das Schiff beladen wurde
void noShip::StartTransport()
{
	state = STATE_TRANSPORT_DRIVING;

	StartDrivingToHarborPlace();

	// Einfach weiterfahren
	HandleState_TransportDriving(); 
}

/// Sagt dem Schiff, das ein bestimmter Hafen zerst�rt wurde
void noShip::HarborDestroyed(nobHarborBuilding * hb)
{
	// Ist unser Ziel betroffen?
	if(hb->GetHarborPosID() == goal_harbor_id && ((state >= STATE_TRANSPORT_LOADING && state <= STATE_TRANSPORT_UNLOADING)))
	{
		// Punkt von dem aus berechnet werden soll, ist normalerweise der normale Standpunkt des Schiffes,
		// allderdings der n�chste Punkt, wenn das Schiff gerade am fahren ist (der Zielpunkt der aktuellen
		// Bewegung
		MapCoord current_x = x, current_y = y;
		if(state == STATE_TRANSPORT_DRIVING)
		{
			current_x = gwg->GetXA(x,y,dir);
			current_y = gwg->GetYA(x,y,dir);
		}

		// Neues Ziel suchen, wo wir ankern k�nnen
		if(players->getElement(player)->FindHarborForUnloading(this,current_x,current_y,&goal_harbor_id,&route,hb))
		{

			switch(state)
			{
			default: return;
			case STATE_TRANSPORT_LOADING: // Schiff wird mit Waren/Figuren erst noch beladen, bleibt also f�r kurze Zeit am Hafen
				{
					state = STATE_TRANSPORT_DRIVING;
					// Hafen suchen, wo wir gerade ankern
					for(unsigned i = 0;i<6;++i)
					{
						noBase * nob = gwg->GetNO(gwg->GetXA(x,y,i),gwg->GetYA(x,y,i));
						if(nob->GetGOT() == GOT_NOB_HARBORBUILDING && nob != hb)
						{
							// Waren wieder abladen
							static_cast<nobHarborBuilding*>(nob)->ReceiveGoodsFromShip(figures,wares);
							// Neuen Job f�r mich suchen
							StartIdling();
							goal_harbor_id = 0;
							players->getElement(player)->GetJobForShip(this);
							return;
						}
					}

				} // Keinbreak;
			case STATE_TRANSPORT_DRIVING: /// Schiff transportiert Waren/Figuren von einen Ort zum anderen
				{
					// Route wird wieder von vorne abgearbeitet
					pos = 0;

				} break;
			case STATE_TRANSPORT_UNLOADING: /// Entl�dt Schiff am Zielhafen, kurze Zeit ankern, bevor Waren im Hafengeb�ude ankommen..
				{
					// Event zum Abladen abmelden
					em->RemoveEvent(current_ev);
					current_ev = NULL;
					// Route wird wieder von vorne abgearbeitet
					pos = 0;
					state = STATE_TRANSPORT_DRIVING;
				} break;

			}

			// Waren und Figure �ber ver�ndertes Ziel informieren
			for(std::list<noFigure*>::iterator it = figures.begin();it!=figures.end();++it)
				(*it)->StartShipJourney(gwg->GetHarborPoint(goal_harbor_id));
		}
		else
		{
			/*// Kein Ziel gefunden, also erstmal idlen
			StartIdling();
			em->RemoveEvent(current_ev);
			current_ev = NULL;*/
		}
	}
}

/// F�ngt an mit idlen und setzt n�tigen Sachen auf NULL
void noShip::StartIdling()
{
	//goal_harbor_id = 0;
	state = STATE_IDLE;
}


/// Sagt Bescheid, dass ein Schiffsangreifer nicht mehr mit nach Hause fahren will
void noShip::SeaAttackerWishesNoReturn()
{
	assert(remaining_sea_attackers);
	--remaining_sea_attackers;
	// Alle Soldaten an Bord 
	if(remaining_sea_attackers == 0)
	{
		if(figures.size())
		{
			// Wieder nach Hause fahren
			goal_harbor_id = home_harbor;
			StartDrivingToHarborPlace();
			state = STATE_TRANSPORT_DRIVING;
			HandleState_TransportDriving();
			for(std::list<noFigure*>::iterator it = figures.begin();it!=figures.end();++it)
				(*it)->StartShipJourney(gwg->GetHarborPoint(goal_harbor_id));
		}
		else
		{
			// Wenn keine Soldaten mehr da sind k�nnen wir auch erstmal idlen
			StartIdling();
			players->getElement(player)->GetJobForShip(this);
		}
	}

}

/// Schiffs-Angreifer sind nach dem Angriff wieder zur�ckgekehrt
void noShip::AddAttacker(nofAttacker * attacker)
{
	assert(std::find(figures.begin(), figures.end(),attacker) == figures.end());

	figures.push_back(attacker);
	// Nun brauchen wir quasi einen Angreifer weniger
	SeaAttackerWishesNoReturn();
}

/// Weist das Schiff an, seine Erkundungs-Expedition fortzusetzen
void noShip::ContinueExplorationExpedition()
{
	// Sind wir schon �ber unserem Limit, also zu weit gefahren
	if(covered_distance >= MAX_EXPLORATION_EXPEDITION_DISTANCE)
	{
		// Dann steuern wir unseren Heimathafen an!
		goal_harbor_id = home_harbor;
	}
	else
	{
		// N�chsten Zielpunkt bestimmen
		std::vector<unsigned> hps;
		gwg->GetHarborPointsWithinReach(goal_harbor_id,hps);
		
		// Keine m�glichen H�fen gefunden?
		if(hps.size() == 0)
			// Dann wieder Heimathafen ansteuern
			goal_harbor_id = home_harbor;

		else
			// Zuf�llig den n�chsten Hafen ausw�hlen
			goal_harbor_id = hps[Random::inst().Rand(__FILE__,__LINE__,obj_id,hps.size())];
	}

	StartDrivingToHarborPlace();
	state = STATE_EXPLORATIONEXPEDITION_DRIVING;
	HandleState_ExplorationExpeditionDriving();
}

/// Sagt dem Schiff, dass ein neuer Hafen erbaut wurde
void noShip::NewHarborBuilt(nobHarborBuilding * hb)
{
	if(lost)
	{
		// Liegt der Hafen auch am Meer von diesem Schiff?
		if(!gwg->IsAtThisSea(hb->GetHarborPosID(),sea_id))
			return;
		
		home_harbor = goal_harbor_id = hb->GetHarborPosID();
		lost = false;
		
		StartDrivingToHarborPlace();
		
		switch(state)
		{
		case STATE_EXPLORATIONEXPEDITION_DRIVING:
		case STATE_EXPEDITION_DRIVING:
		case STATE_TRANSPORT_DRIVING:
			{
				Driven();
			} break;

		default: assert(false); // Das darf eigentlich nicht passieren
		}
		
	}
}
