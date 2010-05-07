// $Id: nofWoodcutter.cpp 6412 2010-05-06 22:34:54Z jh $
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
#include "nofWoodcutter.h"

#include "GameWorld.h"
#include "noGranite.h"
#include "Loader.h"
#include "macros.h"
#include "GameClient.h"
#include "Ware.h"
#include "noTree.h"
#include "SoundManager.h"
#include "AIEventManager.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

nofWoodcutter::nofWoodcutter(const unsigned short x, const unsigned short y,const unsigned char player,nobUsual * workplace)
: nofFarmhand(JOB_WOODCUTTER,x,y,player,workplace)
{
}

nofWoodcutter::nofWoodcutter(SerializedGameData * sgd, const unsigned obj_id) : nofFarmhand(sgd,obj_id)
{
}

/// Malt den Arbeiter beim Arbeiten
void nofWoodcutter::DrawWorking(int x,int y)
{
	unsigned short i = GAMECLIENT.Interpolate(118,current_ev);


	if(i < 10)
	{
		// 1. Ein St�ck vom Baum nach links laufen
		LOADER.GetBobN("jobs")->Draw(5,0,false,i%8,x-i,y,COLORS[gwg->GetPlayer(player)->color]);
		DrawShadow(x-i,y,static_cast<unsigned char>(i%8),dir);
	}
	else if(i < 82)
	{
		// 2. Hacken
		LOADER.GetImageN("rom_bobs", 24+(i-10)%8)->Draw(x-9,y,0,0,0,0,0,0, COLOR_WHITE, COLORS[gwg->GetPlayer(player)->color]);

		if((i-10)%8 == 3)
		{
			SoundManager::inst().PlayNOSound(53,this,i);
			was_sounding = true;
		}
		
	}
	else if(i<105)
	{
		// 3. Warten bis Baum umf�llt
		LOADER.GetImageN("rom_bobs", 24)->Draw(x-9,y,0,0,0,0,0,0, COLOR_WHITE, COLORS[gwg->GetPlayer(player)->color]);

		if(i == 90)
		{
			SoundManager::inst().PlayNOSound(85,this,i);
			was_sounding = true;
		}
	}
	else if(i<115)
	{
		// 4. Wieder zur�ckgehen nach rechts
		LOADER.GetBobN("jobs")->Draw(5,3,false,(i-105)%8,x-(9-(i-105)),y,COLORS[gwg->GetPlayer(player)->color]);
		DrawShadow(x-(9-(i-95)),y,static_cast<unsigned char>((i-105)%8),dir);
	}
	else
	{
		// 5. kurz am Baum warten (quasi Baumstamm in die Hand nehmen)
		LOADER.GetBobN("jobs")->Draw(5,3,false,1,x,y,COLORS[gwg->GetPlayer(player)->color]);
		DrawShadow(x,y,1,dir);

	}


}

/// Fragt die abgeleitete Klasse um die ID in JOBS.BOB, wenn der Beruf Waren raustr�gt (bzw rein)
unsigned short nofWoodcutter::GetCarryID() const
{
	return 61;
}

/// Abgeleitete Klasse informieren, wenn sie anf�ngt zu arbeiten (Vorbereitungen)
void nofWoodcutter::WorkStarted()
{
	assert(gwg->GetSpecObj<noTree>(dest_x,dest_y)->GetType() == NOP_TREE);

	gwg->GetSpecObj<noTree>(dest_x,dest_y)->FallSoon();
}

/// Abgeleitete Klasse informieren, wenn fertig ist mit Arbeiten
void nofWoodcutter::WorkFinished()
{
	// Holz in die Hand nehmen
	ware = GD_WOOD;

	// evtl. AIEvent senden
	GameClient::inst().SendAIEvent(new AIEvent::Location(AIEvent::TreeChopped, x, y), player);
}

/// Fragt abgeleitete Klasse, ob hier Platz bzw ob hier ein Baum etc steht, den z.B. der Holzf�ller braucht
bool nofWoodcutter::IsPointGood(const unsigned short x, const unsigned short y)
{
	// Gibt es hier an dieser Position einen Baum und ist dieser ausgewachsen?
	// au�erdem keine Ananas f�llen!
	noBase * no;
	if((no = gwg->GetNO(x,y))->GetType() == NOP_TREE)
	{
		if(static_cast<noTree*>(no)->size == 3 && static_cast<noTree*>(no)->type != 5)
			return true;
	}

	return false;
}


void nofWoodcutter::WorkAborted_Farmhand()
{
	// Dem Baum Bescheid sagen
	if(state == STATE_WORK)
		gwg->GetSpecObj<noTree>(x,y)->DontFall();
}
