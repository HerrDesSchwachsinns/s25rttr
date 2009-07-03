// $Id: nofFarmhand.cpp 5180 2009-07-03 14:19:17Z FloSoft $
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
#include "nofFarmhand.h"

#include "nobUsual.h"
#include "GameWorld.h"
#include "EventManager.h"
#include "Random.h"
#include "JobConsts.h"
#include "SoundManager.h"
#include "SerializedGameData.h"
#include "GameClient.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

nofFarmhand::nofFarmhand(const Job job,const unsigned short x, const unsigned short y,const unsigned char player,nobUsual * workplace)
: nofBuildingWorker(job,x,y,player,workplace), dest_x(0), dest_y(0)
{
}

void nofFarmhand::Serialize_nofFarmhand(SerializedGameData * sgd) const
{
	Serialize_nofBuildingWorker(sgd);

	sgd->PushUnsignedShort(dest_x);
	sgd->PushUnsignedShort(dest_y);
}

nofFarmhand::nofFarmhand(SerializedGameData * sgd, const unsigned obj_id) : nofBuildingWorker(sgd,obj_id),
dest_x(sgd->PopUnsignedShort()),
dest_y(sgd->PopUnsignedShort())
{}


void nofFarmhand::WalkedDerived()
{
	switch(state)
	{
	case STATE_WALKTOWORKPOINT: WalkToWorkpoint(); break;
	case STATE_WALKINGHOME: WalkHome(); break;
	default:
		break;
	}
}


void nofFarmhand::HandleDerivedEvent(const unsigned int id)
{
	switch(state)
	{
	case STATE_WORK:
		{
			// fertig mit Arbeiten --> dann m�ssen die "Folgen des Arbeitens" ausgef�hrt werden
			WorkFinished();
			// Objekt wieder freigeben
			gwg->GetNode(x,y).reserved = false;;
			// Wieder nach Hause gehen
			StartWalkingHome();

			// Evtl. Sounds l�schen
			if(was_sounding)
			{
				SoundManager::inst().WorkingFinished(this);
				was_sounding = false;
			}

		} break;
	case STATE_WAITING1:
		{
			// Fertig mit warten --> anfangen zu arbeiten
			// Die Arbeitsradien der Berufe wie in JobConst.h (ab JOB_WOODCUTTER!)
			const unsigned char RADIUS[7] =
			{ 6,7,6,0,8,2,2 };

			
			// Anzahl der Radien, wo wir g�ltige Punkte gefunden haben
			unsigned radius_count = 0;

			list< Point<MapCoord> > available_points;

			for(MapCoord tx=gwg->GetXA(x,y,0), r=1;r<=RADIUS[job-JOB_WOODCUTTER];tx=gwg->GetXA(tx,y,0),++r)
			{
				// Wurde ein Punkt in diesem Radius gefunden?
				bool found_in_radius = false;

				MapCoord tx2 = tx, ty2 = y;
				for(unsigned i = 2;i<8;++i)
				{
					for(MapCoord r2=0;r2<r;gwg->GetPointA(tx2,ty2,i%6),++r2)
					{
						if(IsPointAvailable(tx2,ty2) && !gwg->GetNode(tx2,ty2).reserved)
						{
							available_points.push_back(Point<MapCoord>(tx2, ty2));
							found_in_radius = true;
						}
					}
				}


				// Nur die zwei n�chsten Radien erst einmal nehmen
				if(found_in_radius)
				{
					if( ++radius_count == 2)
						break;
				}
			}

			// Gibt es �berhaupt ein Objekt, wo ich hingehen kann?
			if(available_points.size())
			{
				// Ein Objekt zuf�llig heraussuchen
				Point<MapCoord> p = *available_points[RANDOM.Rand(__FILE__,__LINE__,obj_id,available_points.size())];

				// Als neues Ziel nehmen
				dest_x = p.x;
				dest_y = p.y;

				state = STATE_WALKTOWORKPOINT;

				// Wir arbeiten jetzt
				workplace->is_working = true;

				// Punkt f�r uns reservieren
				gwg->GetNode(dest_x,dest_y).reserved = true;;

				// Anfangen zu laufen (erstmal aus dem Haus raus!)
				StartWalking(4);

				StopNotWorking();
			}
			else
			{

				if(GameClient::inst().GetPlayerID() == this->player)
				{
					if (!OutOfRessourcesMsgSent)
					{
						switch(job)
						{
						case JOB_STONEMASON:
							GameClient::inst().SendPostMessage(
							  new ImagePostMsgWithLocation(_("No more stones in range"), PMC_GENERAL, x, y, workplace->GetBuildingType(), workplace->GetNation()));
							break;
						case JOB_FISHER:
							GameClient::inst().SendPostMessage(
							  new ImagePostMsgWithLocation(_("No more fishes in range"), PMC_GENERAL, x, y, workplace->GetBuildingType(), workplace->GetNation()));
							break;
						default:
							break;
						}
						OutOfRessourcesMsgSent = true;
					}
				}

				// Weiter warten, vielleicht gibts ja sp�ter wieder mal was
				current_ev = em->AddEvent(this,JOB_CONSTS[job].wait1_length,1);

				StartNotWorking();
			}

		} break;
    default:
		break;
	}
}

bool nofFarmhand::IsPointAvailable(const unsigned short x, const unsigned short y)
{
	// Gibts an diesen Punkt �berhaupt die n�tigen Vorraussetzungen f�r den Beruf?
	if(IsPointGood(x,y))
	{
		// Gucken, ob ein Weg hinf�hrt
		if(gwg->FindHumanPath(this->x,this->y,x,y,20) != 0xFF)
			return 1;
		else
			return 0;
	}
	else
		return 0;
}

void nofFarmhand::WalkToWorkpoint()
{
	// Sind wir am Ziel angekommen?
	if(x == dest_x && y == dest_y)
	{
		// Anfangen zu arbeiten
		state = STATE_WORK;
		current_ev = em->AddEvent(this,JOB_CONSTS[job].work_length,1);
		WorkStarted();
	}
	// Weg suchen und gucken ob der Punkt noch in Ordnung ist
	else if((dir = gwg->FindHumanPath(x,y,dest_x,dest_y,20)) == 0xFF || !IsPointGood(dest_x,dest_y))
	{
		// Punkt freigeben
		gwg->GetNode(dest_x,dest_y).reserved = false;;
		// Kein Weg f�hrt mehr zum Ziel oder Punkt ist nich mehr in Ordnung --> wieder nach Hause gehen
		StartWalkingHome();
	}
	else
	{
		// Alles ok, wir k�nnen hinlaufen
		StartWalking(dir);
	}
}

void nofFarmhand::StartWalkingHome()
{
	state = STATE_WALKINGHOME;
	// Fahne vor dem Geb�ude anpeilen
	dest_x = gwg->GetXA(workplace->GetX(), workplace->GetY(), 4);
	dest_y = gwg->GetYA(workplace->GetX(), workplace->GetY(), 4);

	// Zu Laufen anfangen
	WalkHome();
}

void nofFarmhand::WalkHome()
{
	// Sind wir zu Hause angekommen? (genauer an der Flagge !!)
	if(x == dest_x && y == dest_y)
	{
		// Weiteres �bernimmt nofBuildingWorker
		WorkingReady();
	}
	// Weg suchen und ob wir �berhaupt noch nach Hause kommen
	else if((dir = gwg->FindHumanPath(x,y,dest_x,dest_y,40)) == 0xFF)
	{
		// Kein Weg f�hrt mehr nach Hause--> Rumirren
		StartWandering();
		Wander();
		// Haus Bescheid sagen
		workplace->WorkerLost();
		workplace = 0;
	}
	else
	{
		// Alles ok, wir k�nnen hinlaufen
		StartWalking(dir);
	}
}


void nofFarmhand::WorkAborted()
{
	// Platz freigeben, falls man gerade arbeitet
	if(state == STATE_WORK || state == STATE_WALKTOWORKPOINT)
		gwg->GetNode(dest_x,dest_y).reserved = false;

	WorkAborted_Farmhand();
}


void nofFarmhand::WorkAborted_Farmhand()
{
}


/// Zeichnen der Figur in sonstigen Arbeitslagen
void nofFarmhand::DrawOtherStates(const int x, const int y)
{
	switch(state)
	{
	case STATE_WALKTOWORKPOINT:
		{
			// Normales Laufen zeichnen
			DrawWalking(x,y);
		} break;
	default: return;
	}
}
