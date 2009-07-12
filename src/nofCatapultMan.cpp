// $Id: nofCatapultMan.cpp 5253 2009-07-12 14:42:18Z FloSoft $
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
#include "nofCatapultMan.h"
#include "GameWorld.h"
#include "Random.h"
#include "nobMilitary.h"
#include "GameClient.h"
#include "nobUsual.h"
#include "CatapultStone.h"
#include "MapGeometry.h"

const int STONE_STARTS[12] = {-4,-48, -3,-47, -13,-47, -11,-48, -13,-47, -2,-47};


nofCatapultMan::nofCatapultMan(const unsigned short x, 
                               const unsigned short y, 
                               const unsigned char player,
                               nobUsual *workplace)
	: nofBuildingWorker(JOB_HELPER, x, y, player, workplace), 
	wheel_steps(0)
{
}

nofCatapultMan::nofCatapultMan(SerializedGameData *sgd, 
			       const unsigned obj_id)
	: nofBuildingWorker(sgd, obj_id), 
	wheel_steps( sgd->PopSignedInt() ), target( sgd )
{

}


void nofCatapultMan::Serialize_nofCatapultMan(SerializedGameData * sgd) const
{
	Serialize_nofBuildingWorker(sgd);

	sgd->PushSignedInt(wheel_steps);
	target.Serialize_PossibleTarget(sgd);
}


void nofCatapultMan::WalkedDerived()
{
}


void nofCatapultMan::DrawWorking(int x, int y)
{
	switch(state)
	{
	default: return;
	case STATE_CATAPULT_TARGETBUILDING:
		{
			int step = GameClient::inst().Interpolate(abs(wheel_steps) + 1, current_ev);

			if(step <= abs(wheel_steps))
			{

				if(wheel_steps < 0)
					step = -step;

				// Katapult auf dem Dach mit Stein drehend zeichnen
				LOADER.GetImageN("rom_bobs", 1781 + (7 + step) % 6)->Draw(x - 7, y - 19, 0, 0, 0, 0, 0, 0, COLOR_WHITE, COLOR_WHITE);
			}
			//else
			//	// Katapult schie�end zeichnen
			//	LOADER.GetImageN("rom_bobs", 1787+(7+wheel_steps)%6)->Draw(x-7,y-19);

		} break;
	case STATE_CATAPULT_BACKOFF:
		{
			int step = GameClient::inst().Interpolate((abs(wheel_steps)+3)*2,current_ev);

			if(step < 2 * 3)
				// Katapult nach Schie�en zeichnen (hin und her wippen
				LOADER.GetImageN("rom_bobs", 1787+(step%2)*6+(7+wheel_steps)%6)->Draw(x - 7, y - 19, 0, 0, 0, 0, 0, 0, COLOR_WHITE, COLOR_WHITE);
			else
			{
				step = (step-6)/2;

				if(wheel_steps < 0)
					step = -(step);

				// Katapult auf dem Dach mit Stein drehend zeichnen (zur�ck in Ausgangsposition: Richtung 4)
				LOADER.GetImageN("rom_bobs", 1775+(7+wheel_steps-step)%6)->Draw(x - 7, y - 19, 0, 0, 0, 0, 0, 0, COLOR_WHITE, COLOR_WHITE);
			}

		} break;

	}
}


//void nofCatapultMan::HandleStateTargetBuilding()
//{
//}
//
//void nofCatapultMan::HandleStateBackOff()
//{
//}



void nofCatapultMan::HandleDerivedEvent(const unsigned int id)
{
	switch(state)
	{
	default:
		break;
	case STATE_WAITING1:
		{
			// Fertig mit warten --> anfangen zu arbeiten
			list<nobBaseMilitary*> buildings;
			gwg->LookForMilitaryBuildings(buildings,x,y,3);

			// Liste von potentiellen Zielen
			list<PossibleTarget> pts;


			for(list<nobBaseMilitary*>::iterator it = buildings.begin();it.valid();++it)
			{
				// Auch ein richtiges Milit�rgeb�ude (kein HQ usw.), 
				if((*it)->GetGOT() == GOT_NOB_MILITARY && GameClient::inst().GetPlayer(player)->IsPlayerAttackable((*it)->GetPlayer()))
				{
					// Was nicht im Nebel liegt und auch schon besetzt wurde (nicht neu gebaut)?
					if(gwg->GetNode((*it)->GetX(),(*it)->GetY()).fow[player].visibility == VIS_VISIBLE
						&& !static_cast<nobMilitary*>((*it))->IsNewBuilt())
					{
						// Entfernung ausrechnen
						unsigned distance = CalcDistance(x,y,(*it)->GetX(),(*it)->GetY());

						// Entfernung nicht zu hoch?
						if(distance < 14)
						{
							// Mit in die Liste aufnehmen
							PossibleTarget pt((*it)->GetX(), (*it)->GetY(), distance);
							pts.push_back(pt);
						}
					}
				}
			}

			// Gibts evtl keine Ziele?
			if(!pts.size())
			{
				// Weiter warten, vielleicht gibts ja sp�ter wieder mal was
				current_ev = em->AddEvent(this,CATAPULT_WAIT1_LENGTH,1);
				StartNotWorking();
				return;
			}
			else
			{
				char str[512];
				sprintf(str,"%u: obj_id=%u found %u targets!\n",GameClient::inst().GetGFNumber(),
					obj_id,pts.size());
				GameClient::inst().AddToGameLog(str);
				for(list<PossibleTarget>::iterator it = pts.begin();it.valid();++it)
				{
					sprintf(str," - x=%u, y=%u, distance=%u\n",unsigned(it->x),unsigned(it->y),it->distance); 
					GameClient::inst().AddToGameLog(str);
				}
			}

			// Waren verbrauchen
			workplace->ConsumeWares();

			// Eins zuf�llig ausw�hlen
			target = *pts[Random::inst().Rand(__FILE__,__LINE__,obj_id,pts.size())];

			// Richtung, in die sich der Katapult drehen soll, bestimmen
			unsigned char shooting_dir;

			// Y-Abstand nur unwesentlich klein --> Richtung 0 und 3 (direkt gegen�ber) nehmen
			if(SafeDiff(target.y,y) <= SafeDiff(target.x,x)/5)
				shooting_dir = (x < target.x) ? 3 : 0;
			else
			{
				// Ansonsten noch y mit ber�cksichtigen und je einen der 4 Quadranten nehmen
				if(y < target.y)
					shooting_dir = (x < target.x) ? 4 : 5;
				else
					shooting_dir = (x < target.x) ? 2 : 1;
			}

			// "Drehschritte" ausrechnen, da von Richtung 4 aus gedreht wird
			wheel_steps = int(shooting_dir)-4;
			if(wheel_steps < -3)
				wheel_steps = 6+wheel_steps;

			current_ev = em->AddEvent(this,15*(abs(wheel_steps)+1),1);

			state = STATE_CATAPULT_TARGETBUILDING;

			// wir arbeiten
			workplace->is_working = true;

		} break;
	case STATE_CATAPULT_TARGETBUILDING:
		{
			// Stein in Bewegung setzen

			// Soll das Geb�ude getroffen werden (70%)
			bool hit = (Random::inst().Rand(__FILE__,__LINE__,obj_id,99) < 70);

			// Radius f�rs Treffen und Nicht-Treffen,  (in Pixeln), nur visuell
			const int RADIUS_HIT = 15; // nicht nach unten hin!

			// Zielkoordinaten als (Map-Koordinaten!)
			unsigned short dest_map_x,dest_map_y;

			if(hit)
			{
				// Soll getroffen werden --> Aufschlagskoordinaten gleich dem eigentlichem Ziel
				dest_map_x = target.x;
				dest_map_y = target.y;
			}
			else
			{
				// Ansonsten zuf�lligen Punkt rundrum heraussuchen
				unsigned d = Random::inst().Rand(__FILE__,__LINE__,obj_id,6);

				dest_map_x = gwg->GetXA(target.x,target.y,d);
				dest_map_y = gwg->GetYA(target.x,target.y,d);
			}

			// (Visuellen) Aufschlagpunkt bestimmen
			int dest_x = int(gwg->GetTerrainX(dest_map_x,dest_map_y));
			int dest_y = int(gwg->GetTerrainY(dest_map_x,dest_map_y));

			// Bei getroffenen den Aufschlagspunkt am Geb�ude ein bisschen variieren
			if(hit)
			{
				dest_x += (Random::inst().Rand(__FILE__,__LINE__,obj_id,RADIUS_HIT*2)-RADIUS_HIT);
				// hier nicht nach unten gehen, da die T�r (also Nullpunkt
				// ja schon ziemlich weit unten ist!
				dest_y -= Random::inst().Rand(__FILE__,__LINE__,obj_id,RADIUS_HIT);
			}

			// Stein erzeugen
			unsigned char shooting_dir = (7+wheel_steps)%6;

			gwg->AddCatapultStone(new CatapultStone(target.x,target.y,dest_map_x,dest_map_y,
				int(gwg->GetTerrainX(x,y))+STONE_STARTS[(7+wheel_steps)%6*2], int(gwg->GetTerrainY(x,y))+STONE_STARTS[shooting_dir*2+1],
				dest_x, dest_y,
				80));


			// Katapult wieder in Ausgangslage zur�ckdrehen
			current_ev = em->AddEvent(this,15*(abs(wheel_steps)+3),1);

			state = STATE_CATAPULT_BACKOFF;
		} break;
	case STATE_CATAPULT_BACKOFF:
		{
			current_ev = 0;
			// wir arbeiten nicht mehr
			workplace->is_working = false;
			// Wieder versuchen, zu arbeiten
			TryToWork();

		} break;
	}
}

void nofCatapultMan::WorkArborted()
{
}

