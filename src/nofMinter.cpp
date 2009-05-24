// $Id: nofMinter.cpp 4947 2009-05-24 20:02:16Z OLiver $
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
#include "nofMinter.h"

#include "Loader.h"
#include "macros.h"
#include "GameClient.h"
#include "nobUsual.h"
#include "SoundManager.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

nofMinter::nofMinter(const unsigned short x, const unsigned short y,const unsigned char player,nobUsual * workplace)
: nofWorkman(JOB_MINTER,x,y,player,workplace)
{
}

void nofMinter::Serialize_nofMinter(SerializedGameData * sgd) const
{
	Serialize_nofWorkman(sgd);
}

nofMinter::nofMinter(SerializedGameData * sgd, const unsigned obj_id) : nofWorkman(sgd,obj_id)
{
}

void nofMinter::DrawWorking(int x, int y)
{
//	signed char offsets[4][2] = { {22,-12},{28,1},{19,-11},{19,-20} };
	signed char offsets[4][2] = { {19,-20},{19,-11},{22,-12},{28,1} };

	unsigned now_id = GAMECLIENT.Interpolate(136,current_ev);

	if(now_id < 91)
	{
		GetRomBob(84+(now_id)%8)
			->Draw(x+offsets[workplace->GetNation()][0],y+offsets[workplace->GetNation()][1],0,0,0,0,0,0,COLORS[GAMECLIENT.GetPlayer(workplace->GetPlayer())->color]);

		// Evtl Sound abspielen
		if(now_id%8 == 3)
		{
			SoundManager::inst().PlayNOSound(58,this,now_id);
			was_sounding = true;
		}
	}

	last_id = now_id;
}

GoodType nofMinter::ProduceWare()
{
  GAMECLIENT.GetPlayer(player)->ChangeStatisticValue(STAT_GOLD, 1);
	return GD_COINS;
}
