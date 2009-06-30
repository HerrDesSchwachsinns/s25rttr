// $Id: LobbyProtocol.h 4936 2009-05-24 15:17:07Z FloSoft $
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
#include "GameCommands.h"
#include "GameClient.h"
#include "GameClientPlayer.h"
#include "GameWorld.h"
#include "nobShipYard.h"
#include "nobMilitary.h"
#include "nobBaseWarehouse.h"
#include "nobHarborBuilding.h"

using namespace gc;

GameCommand * GameCommand::CreateGameCommand(const Type gst, Serializer * ser)
{
	switch(gst)
	{
	default: return NULL;

	case SETFLAG: return new SetFlag(ser);
	case DESTROYFLAG: return new DestroyFlag(ser);
	case BUILDROAD: return new BuildRoad(ser);
	case DESTROYROAD: return new DestroyRoad(ser);
	case CHANGEDISTRIBUTION: return new ChangeDistribution(ser);
	case CHANGEBUILDORDER: return new ChangeBuildOrder(ser);
	case SETBUILDINGSITE: return new SetBuildingSite(ser);
	case DESTROYBUILDING: return new DestroyBuilding(ser);
	case CHANGETRANSPORT: return new ChangeTransport(ser);
	case CHANGEMILITARY: return new ChangeMilitary(ser);
	case CHANGETOOLS: return new ChangeTools(ser);
	case CALLGEOLOGIST: return new CallGeologist(ser);
	case CALLSCOUT: return new CallScout(ser);
	case ATTACK: return new Attack(ser);
	case SWITCHPLAYER: return new SwitchPlayer(ser);
	case STOPGOLD: return new StopGold(ser);
	case STOPPRODUCTION: return new StopProduction(ser);
	case CHANGEINVENTORYSETTING: return new ChangeInventorySetting(ser);
	case CHANGEALLINVENTORYSETTINGS: return new ChangeAllInventorySettings(ser);
	case CHANGERESERVE: return new ChangeReserve(ser);
	case SUGGESTPACT: return new SuggestPact(ser);
	case ACCEPTPACT: return new AcceptPact(ser);
	case CANCELPACT: return new CancelPact(ser);
	case CHANGESHIPYARDMODE: return new ChangeShipYardMode(ser);
	case STARTEXPEDITION: return new StartExpedition(ser);
	case SURRENDER: return new Surrender(ser);
	case CHEAT_ARMAGEDDON: return new CheatArmageddon(ser);
	case DESTROYALL: return new DestroyAll(ser);
	}

	return NULL;
}

void SetFlag::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	gwg.SetFlag(x,y,playerid);
}

void DestroyFlag::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	gwg.DestroyFlag(x,y);
}
void BuildRoad::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	gwg.BuildRoad(playerid,boat_road,x,y,route);
}
void DestroyRoad::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	gwg.DestroyRoad(x,y,start_dir);
}
void ChangeDistribution::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	player.ChangeDistribution(data);
}
void ChangeBuildOrder::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	player.ChangeBuildOrder(order_type,data);
}
void SetBuildingSite::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	gwg.SetBuildingSite(bt,x,y,playerid);
}
void DestroyBuilding::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	gwg.DestroyBuilding(x,y,playerid);
}
void ChangeTransport::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	player.ConvertTransportData(data);
}
void ChangeMilitary::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	player.ChangeMilitarySettings(data);
}
void ChangeTools::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	player.ChangeToolsSettings(data);
}
void CallGeologist::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	if(gwg.GetNO(x,y)->GetGOT() == GOT_FLAG)
		player.CallFlagWorker(x,y,JOB_GEOLOGIST);
}
void CallScout::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	if(gwg.GetNO(x,y)->GetGOT() == GOT_FLAG)
		player.CallFlagWorker(x,y,JOB_SCOUT);
}
void Attack::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	gwg.Attack(playerid,x,y,soldiers_count,strong_soldiers);
}
void SwitchPlayer::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
}
void StopGold::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	if(gwg.GetNO(x,y)->GetGOT() == GOT_NOB_MILITARY)
	{
		if(gwg.GetSpecObj<nobMilitary>(x,y)->GetPlayer() == playerid)
			gwg.GetSpecObj<nobMilitary>(x,y)->StopGold();
	}
}
void StopProduction::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	if(gwg.GetNO(x,y)->GetGOT() == GOT_NOB_USUAL || gwg.GetNO(x,y)->GetGOT() == GOT_NOB_SHIPYARD)
		gwg.GetSpecObj<nobUsual>(x,y)->StopProduction();
}
void ChangeInventorySetting::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	if(gwg.GetNO(x,y)->GetType() == NOP_BUILDING)
		gwg.GetSpecObj<nobBaseWarehouse>(x,y)
		->ChangeRealInventorySetting(category,state,type);
}
void ChangeAllInventorySettings::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	if(gwg.GetNO(x,y)->GetType() == NOP_BUILDING)
		gwg.GetSpecObj<nobBaseWarehouse>(x,y)
		->ChangeAllRealInventorySettings(category,state);
}
void ChangeReserve::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	if(gwg.GetNO(x,y)->GetType() == NOP_BUILDING)
		gwg.GetSpecObj<nobBaseWarehouse>(x,y)->SetRealReserve(rank,count);
	
}
void Surrender::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	player.Surrender();
}

void CheatArmageddon::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	gwg.Armageddon();
}

void DestroyAll::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	 gwg.Armageddon(playerid);
}

void SuggestPact::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	player.SuggestPact(this->player,pt,duration);
}

void AcceptPact::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	::GameClient::inst().GetPlayer(this->player)->AcceptPact(id,pt,playerid);
}

void CancelPact::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	player.CancelPact(pt,this->player);
}

void ChangeShipYardMode::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	if(gwg.GetNO(x,y)->GetGOT() == GOT_NOB_SHIPYARD)
		gwg.GetSpecObj<nobShipYard>(x,y)->ToggleMode();
}

void StartExpedition::Execute(GameWorldGame& gwg, GameClientPlayer& player, const unsigned char playerid)
{
	if(gwg.GetNO(x,y)->GetGOT() == GOT_NOB_HARBORBUILDING)
		gwg.GetSpecObj<nobHarborBuilding>(x,y)->StartExpedition();
}

