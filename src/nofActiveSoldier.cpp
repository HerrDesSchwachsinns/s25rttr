// $Id: nofActiveSoldier.cpp 6333 2010-04-19 18:42:40Z OLiver $
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
#include "nofActiveSoldier.h"
#include "nobMilitary.h"
#include "Loader.h"
#include "GameConsts.h"
#include "Random.h"
#include "GameWorld.h"
#include "noFighting.h"
#include "GameClient.h"
#include "SerializedGameData.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

nofActiveSoldier::nofActiveSoldier(const unsigned short x, const unsigned short y,const unsigned char player,
				 nobBaseMilitary * const home,const unsigned char rank, const SoldierState init_state)
				 : nofSoldier(x,y,player,home,rank), state(init_state)

{
}

nofActiveSoldier::nofActiveSoldier(const nofSoldier& other, const SoldierState init_state) :
  nofSoldier(other), state(init_state) {}


void nofActiveSoldier::Serialize_nofActiveSoldier(SerializedGameData * sgd) const
{
	Serialize_nofSoldier(sgd);

	sgd->PushUnsignedChar(static_cast<unsigned char>(state));
}


nofActiveSoldier::nofActiveSoldier(SerializedGameData * sgd, const unsigned obj_id) : nofSoldier(sgd,obj_id),
state(SoldierState(sgd->PopUnsignedChar()))
{
}



void nofActiveSoldier::GoalReached()
{
	// mich hinzuf�gen
	static_cast<nobMilitary*>(building)->AddActiveSoldier(this);

	// und wir k�nnen uns auch aus der Laufliste erstmal entfernen
	gwg->RemoveFigure(this,x,y);
}

void nofActiveSoldier::ReturnHome()
{
	state = STATE_WALKINGHOME;
	WalkingHome();
}


void nofActiveSoldier::WalkingHome()
{
	// Ist evtl. unser Heimatgeb�ude zerst�rt?
	if(!building)
	{
		// Rumirren
		state = STATE_FIGUREWORK;
		StartWandering();
		Wander();

		return;
	}


	// Wieder zur�ck nach Hause laufen

	// Sind wir schon an der Flagge?
	if(x == building->GetFlag()->GetX() && y == building->GetFlag()->GetY())
	{
		// Dann INS Geb�ude laufen
		StartWalking(1);
	}
	// oder vielleicht auch schon im Geb�ude drin?
	else if(x == building->GetX() && y == building->GetY())
	{
		// Wir sind da!
		building->AddActiveSoldier(this);
		// Mich entfernen hier
		gwg->RemoveFigure(this,x,y);
	}
	// oder finden wir gar keinen Weg mehr?
	else if((dir = gwg->FindHumanPath(x,y,building->GetFlag()->GetX(),building->GetFlag()->GetY(),100)) == 0xFF)
	{
		// Kein Weg gefunden --> Rumirren
		StartWandering();
		state = STATE_FIGUREWORK;
		Wander();

		// Dem Heimatgeb�ude Bescheid sagen
		building->SoldierLost(this);
	}
	// oder ist alles ok? :)
	else
	{
		// nach Hause laufen
		StartWalking(dir);
	}
}


void nofActiveSoldier::Draw(int x, int y)

{
	switch(state)
	{
	default:
		break;
	case STATE_WAITINGFORFIGHT:
	case STATE_ATTACKING_WAITINGAROUNDBUILDING:
	case STATE_ATTACKING_WAITINGFORDEFENDER:
	case STATE_DEFENDING_WAITING:
		{
			// wenn er wartet, steht er nur rum
			LOADER.GetBobN("jobs")->Draw(30+NATION_RTTR_TO_S2[GAMECLIENT.GetPlayer(player)->nation]*6+job-JOB_PRIVATE,
				dir,false,2,x,y,COLORS[GAMECLIENT.GetPlayer(player)->color]);
			DrawShadow(x,y,2,dir);
		} break;
	case STATE_FIGUREWORK:
	case STATE_ATTACKING_WALKINGTOGOAL:
	case STATE_AGGRESSIVEDEFENDING_WALKINGTOAGGRESSOR:
	case STATE_WALKINGHOME:
	case STATE_DEFENDING_WALKINGTO:
	case STATE_DEFENDING_WALKINGFROM:
	case STATE_ATTACKING_CAPTURINGFIRST:
	case STATE_ATTACKING_CAPTURINGNEXT:
	case STATE_ATTACKING_ATTACKINGFLAG:
	case STATE_ATTACKING_WALKINGTOFIGHTSPOT:
	case STATE_SEAATTACKING_GOTOHARBOR: // geht von seinem Heimatmilit�rgeb�ude zum Starthafen
	case STATE_SEAATTACKING_RETURNTOSHIP: // befindet sich an der Zielposition auf dem Weg zur�ck zum Schiff
		{
			DrawSoldierWalking(x,y);
		} break;
	}

	/*char str[255];
	sprintf(str,"%u",obj_id);
	LOADER.GetFontN("resource",0)->Draw(x,y,str,0,0xFFFF0000);*/
}

void nofActiveSoldier::HandleDerivedEvent(const unsigned int id)
{
	// Das d�rfte nich aufgerufen werden!
	assert(false);
}

/// Gibt den Sichtradius dieser Figur zur�ck (0, falls nicht-sp�hend)
unsigned nofActiveSoldier::GetVisualRange() const
{
	return VISUALRANGE_SCOUT;
}

/// Pr�ft feindliche Leute auf Stra�en in der Umgebung und vertreibt diese
void nofActiveSoldier::ExpelEnemies()
{
	// Figuren sammeln aus der Umgebung
	std::vector<noFigure*> figures;
	
	// Am Punkt selbst
	for(list<noBase*>::iterator it = gwg->GetFigures(x,y).begin();it.valid();++it)
	{
		if((*it)->GetType() == NOP_FIGURE)
			figures.push_back(static_cast<noFigure*>(*it));
	}
	
	// Und rund herum
	for(unsigned i = 0;i<6;++i)
	{
		// Diese m�ssen sich entweder auf dem Punkt befinden oder zu diesem laufen
		for(list<noBase*>::iterator it = gwg->GetFigures(gwg->GetXA(x,y,i),gwg->GetYA(x,y,i)).begin();it.valid();++it)
		{
			// Figur?
			// Nicht dass wir noch Hase und Igel st�ren (Naturschutz!)
			if((*it)->GetType() == NOP_FIGURE)
			{
				noFigure * fig = static_cast<noFigure*>(*it);
				if(fig->GetX() == x && fig->GetY() == y)
					figures.push_back(fig);
				else if(fig->GetDestinationForCurrentMove() == Point<MapCoord>(x,y))
					figures.push_back(fig);
			}
		}
	}
	
	// Mal gucken, was uns alles ins Netz gegangen ist, und aussieben
	// Nicht, dass Erika Steinbach noch b�se wird
	for(unsigned i = 0;i<figures.size();++i)
	{
		noFigure * fig = figures[i];
		// Feind von uns und kein Soldat?
		// Au�erdem muss er auf der Stra�e unterwegs sein (keine freiarbeitenden Berufe durcheinanderbringen..)
		if(!players->getElement(player)->IsAlly(fig->GetPlayer()) &&
		!(fig->GetJobType() >= JOB_PRIVATE && fig->GetJobType() <= JOB_GENERAL)
		&& fig->IsWalkingOnRoad())
		{
			// Dann weg mit dem!
			fig->Abrogate();
			fig->StartWandering();
			// L�uft der immer noch nicht? (Tr�ger, die auf Wegen stehen und auf Waren warten)
			if(!fig->IsMoving())
				// Dann machen wir dir aber Beine
				fig->StartWalking(Random::inst().Rand(__FILE__,__LINE__,obj_id,6));
		}
	}
	
	// Stra�e ist ges�ubert, vielleicht n�tzt dem Feind das ja sogar was..
	//gwg->RoadNodeAvailable(x, y);
}

