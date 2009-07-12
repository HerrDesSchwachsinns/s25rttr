// $Id: nofDonkeybreeder.cpp 5254 2009-07-12 15:49:16Z FloSoft $
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
#include "nofDonkeybreeder.h"

#include "GameClient.h"
#include "SoundManager.h"

#include "nobUsual.h"
#include "nofCarrier.h"
#include "GameWorld.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/**
 *  Konstruktor von @p nofDonkeybreeder.
 *
 *  @author FloSoft
 */
nofDonkeybreeder::nofDonkeybreeder(unsigned short x, unsigned short y, unsigned char player, nobUsual *workplace)
	: nofWorkman(JOB_DONKEYBREEDER, x, y, player, workplace)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Konstruktor von @p nofDonkeybreeder.
 *
 *  @author FloSoft
 */
nofDonkeybreeder::nofDonkeybreeder(SerializedGameData *sgd, unsigned int obj_id)
	: nofWorkman(sgd, obj_id)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Zeichnet ihn beim Arbeiten.
 *
 *  @author FloSoft
 */
void nofDonkeybreeder::DrawWorking(int x, int y)
{
	/// @todo KA was da gemacht werden muss
	const Nation nation = workplace->GetNation();
	const signed char walk_start[4][2] = { {2,2},{-6,-6},{-7,-7},{-7,-7} };
	const signed char walk_length[4] = { 22,19,19,23 };
	const unsigned int color = COLORS[GAMECLIENT.GetPlayer(player)->color];

	unsigned now_id = GAMECLIENT.Interpolate(9600,current_ev);

	if(now_id < 400)
	{
		LOADER.GetNationImageN(workplace->GetNation(),250+5*BLD_DONKEYBREEDER+4)->Draw(x,y,0,0,0,0,0,0);
		LOADER.GetBobN("jobs")->Draw(24,4,false,(now_id/70)%8,x+walk_start[nation][0]+now_id/100,y+walk_start[nation][1]+now_id/100,color);
	}
	else if(now_id < 1200)
		LOADER.GetBobN("jobs")->Draw(24,3,false,((now_id-400)/70)%8,x+walk_start[nation][0]+4+walk_length[nation]*(now_id-400)/800,y+walk_start[nation][1]+4,color);
	else if(now_id < 2000)
		LOADER.GetImageN("rom_bobs", 291+(now_id-1200)/100)->Draw(x+walk_start[nation][0]+4+walk_length[nation],y+walk_start[nation][1]+4,0,0,0,0,0,0, COLOR_WHITE, color);
	else if(now_id < 2800)
		LOADER.GetBobN("jobs")->Draw(24,0,false,((now_id-2000)/70)%8,x+walk_start[nation][0]+4+walk_length[nation]*(800-(now_id-2000))/800,y+walk_start[nation][1]+4,color);
	else if(now_id < 3200)
	{
		LOADER.GetNationImageN(workplace->GetNation(),250+5*BLD_DONKEYBREEDER+4)->Draw(x,y,0,0,0,0,0,0);
		LOADER.GetBobN("jobs")->Draw(24,1,false,((now_id-2800)/70)%8,x+walk_start[nation][0]+(400-(now_id-2800))/100,y+walk_start[nation][1]+(400-(now_id-2800))/100,color);
	}



    //148
	/*const signed char offsets[4][2] = { {10,2},{10,2},{10,2},{10,2} };
	const signed char walkstart[4][2] = { {-6,-6},{-6,-6},{-6,-6},{-6,-6} };

    unsigned int max_id = 120;
	unsigned now_id = GAMECLIENT.Interpolate(max_id,current_ev);
	unsigned char wpNation = workplace->GetNation();
	unsigned int plColor = GAMECLIENT.GetPlayer(player)->color;
	int walksteps=8;

    if(now_id < 8)
	{
        if (now_id<4) LOADER.LOADER.GetNationImageN(workplace->GetNation(),250+5*BLD_PIGFARM+4)->Draw(x,y,0,0,0,0,0,0);
        int walkx=x+walkstart[wpNation][0]+(((offsets[wpNation][0]-walkstart[wpNation][0])/walksteps)*(now_id));
        int walky=y+walkstart[wpNation][1]+(((offsets[wpNation][1]-walkstart[wpNation][1])/walksteps)*(now_id));
        LOADER.GetBobN("jobs")->Draw(14,4,false,now_id,walkx,walky,COLORS[plColor]);
    }
    if(now_id>=8 && now_id<20){
        LOADER.GetImageN("rom_bobs", 148+(now_id-8))
            ->Draw(x+offsets[workplace->GetNation()][0],y+offsets[wpNation][1],0,0,0,0,0,0,COLORS[plColor]);

		// Evtl Sound abspielen
		if((now_id-8) == 5)
		{
			SoundManager::inst().PlayNOSound(65,this,0);
			was_sounding = true;
		}
    }
    if(now_id>=20 && now_id<28){
        if(now_id>23) LOADER.LOADER.GetNationImageN(workplace->GetNation(),250+5*BLD_PIGFARM+4)->Draw(x,y,0,0,0,0,0,0);
        int walkx=x+offsets[wpNation][0]+(((walkstart[wpNation][0]-offsets[wpNation][0])/walksteps)*(now_id-20));
        int walky=y+offsets[wpNation][1]+(((walkstart[wpNation][1]-offsets[wpNation][1])/walksteps)*(now_id-20));
        LOADER.GetBobN("jobs")->Draw(14,1,false,now_id-20,walkx,walky,COLORS[plColor]);
    }*/
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Der Arbeiter erzeugt eine Ware.
 *
 *  @author FloSoft
 */
GoodType nofDonkeybreeder::ProduceWare()
{
	/// @todo Wie kann ich hier eine Person erzeugen?
	return GD_NOTHING;
}

void nofDonkeybreeder::WorkFinished()
{
	// Stra�e und Zielflagge f�r Esel suchen
	noRoadNode * flag_goal;
	RoadSegment * road = GameClient::inst().GetPlayer(player)->FindRoadForDonkey(workplace,&flag_goal);

	// Esel erzeugen und zum Ziel beordern
	nofCarrier * donkey = new nofCarrier(nofCarrier::CT_DONKEY,x,y,player,road,flag_goal);
	GameClient::inst().GetPlayer(player)->IncreaseInventoryJob(JOB_PACKDONKEY,1);
	donkey->InitializeRoadWalking(gwg->GetSpecObj<noRoadNode>(x,y)->routes[4],0,true);

	// Wenn keine Stra�e gefunden wurde, muss er nach Hause gehen
	if(!road)
		donkey->GoHome();
	else
		// ansonsten Arbeitsplatz Bescheid sagen
		road->GotDonkey(donkey);

	// Esel absetzen
	gwg->AddFigure(donkey,x,y);

	// In die neue Welt laufen
	donkey->ActAtFirst();
}
