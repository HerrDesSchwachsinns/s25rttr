// $Id: nofArmorer.cpp 4652 2009-03-29 10:10:02Z FloSoft $
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
#include "nofArmorer.h"
#include "Loader.h"
#include "macros.h"
#include "GameClient.h"
#include "nobUsual.h"
#include "SoundManager.h"
#include "SerializedGameData.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

nofArmorer::nofArmorer(const unsigned short x, const unsigned short y,const unsigned char player,nobUsual * workplace)
: nofWorkman(JOB_ARMORER,x,y,player,workplace), sword_shield(false)
{
}

void nofArmorer::Serialize_nofArmorer(SerializedGameData * sgd) const
{
	Serialize_nofWorkman(sgd);

	sgd->PushBool(sword_shield);
}

nofArmorer::nofArmorer(SerializedGameData * sgd, const unsigned obj_id) : nofWorkman(sgd,obj_id),
sword_shield(sgd->PopBool())
{
}

void nofArmorer::DrawWorking(int x, int y)
{
	signed char offsets[4][2] = { {-10,15},{-11,9},{-14,16},{-19,1} };

    unsigned int max_id = 280;
	unsigned now_id = GAMECLIENT.Interpolate(max_id,current_ev);
	unsigned char wpNation = workplace->GetNation();
	unsigned int plColor = GAMECLIENT.GetPlayer(player)->color;

	if(now_id < 200)
	{
        GetRomBob(16+(now_id%8))
            ->Draw(x+offsets[workplace->GetNation()][0],y+offsets[wpNation][1],0,0,0,0,0,0,COLORS[plColor]);

		if((now_id%8) == 5)
		{
			SoundManager::inst().PlayNOSound(52,this,now_id/8);
			was_sounding = true;
		}
	}
}

unsigned short nofArmorer::GetCarryID() const
{
	if(sword_shield)
		return 56;
	else
	{
		// Je nach Nation einen bestimmtem Schild fertigen
		switch(GAMECLIENT.GetPlayer(player)->nation)
		{
		case 0: return 60;
		case 1: return 58;
		case 2: return 57;
		case 3: return 59;
		default: return 0;
		}
	}
}

GoodType nofArmorer::ProduceWare()
{
	sword_shield = !sword_shield;

	if(sword_shield)
		return GD_SWORD;
	else
	{
		// Je nach Nation einen bestimmtem Schild fertigen
		switch(GAMECLIENT.GetPlayer(player)->nation)
		{
		case 0: return GD_SHIELDAFRICANS;
		case 1: return GD_SHIELDJAPANESE;
		case 2: return GD_SHIELDROMANS;
		case 3: return GD_SHIELDVIKINGS;
		default: return GD_NOTHING;
		}
	}
}
