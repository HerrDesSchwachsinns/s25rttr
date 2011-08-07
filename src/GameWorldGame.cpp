// $Id: GameWorldGame.cpp 7348 2011-08-07 13:17:05Z OLiver $
//
// Copyright (c) 2005 - 2010 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

///////////////////////////////////////////////////////////////////////////////
// Header
#include <stdafx.h>
#include "main.h"
#include "GameWorld.h"

#include "GameClient.h"
#include "GameClientPlayer.h"
#include "Random.h"
#include "SoundManager.h"
#include "SerializedGameData.h"

#include "nofCarrier.h"
#include "noEnvObject.h"
#include "noStaticObject.h"
#include "noGranite.h"
#include "noTree.h"
#include "noFlag.h"
#include "nobHQ.h"
#include "noFire.h"
#include "nobUsual.h"
#include "noBuildingSite.h"
#include "Ware.h"
#include "MilitaryConsts.h"
#include "TerritoryRegion.h"
#include "nobMilitary.h"
#include "nofAttacker.h"
#include "nofPassiveSoldier.h"
#include "noAnimal.h"
#include "noFighting.h"
#include "CatapultStone.h"
#include "MapGeometry.h"
#include "nofScout_Free.h"
#include "noShip.h"

#include "WindowManager.h"
#include "GameInterface.h"


void GameWorldGame::RecalcBQAroundPoint(const MapCoord x, const MapCoord y)
{
	// Drumherum BQ neu berechnen, da diese sich ja jetzt hätten ändern kÃ¶nnen
	GetNode(x,y).bq = CalcBQ(x,y,GAMECLIENT.GetPlayerID());
	for(unsigned char i = 0;i<6;++i)
		GetNode(GetXA(x,y,i),GetYA(x,y,i)).bq = CalcBQ(GetXA(x,y,i), GetYA(x,y,i),GAMECLIENT.GetPlayerID());
}

void GameWorldGame::RecalcBQAroundPointBig(const MapCoord x, const MapCoord y)
{
	RecalcBQAroundPoint(x,y);

	// 2. AuÃenschale
	for(unsigned i = 0;i<12;++i)
		GetNode(GetXA2(x,y,i),GetYA2(x,y,i)).bq = CalcBQ(GetXA2(x,y,i),GetYA2(x,y,i),GAMECLIENT.GetPlayerID());
}

void GameWorldGame::SetFlag(const MapCoord x, const MapCoord y, const unsigned char player,const unsigned char dis_dir)
{
	// TODO: VerzÃ¶gerungsbugabfrage, kann später ggf. weg
	if(CalcBQ(x,y,player,true,false) != BQ_FLAG)
		return;
	//
	//// Abfragen, ob schon eine Flagge in der Nähe ist (keine Mini-1-Wege)
	//for(unsigned char i = 0;i<6;++i)
	//{
	//	if(GetNO(GetXA(x,y,i), GetYA(x,y,i))->GetType() == NOP_FLAG)
	//		return;
	//}

	//// TODO: VerzÃ¶gerungsbugabfrage, kann später ggf. weg
	//// Abfragen, ob evtl ein Baum gepflanzt wurde, damit der nicht überschrieben wird
	//if(GetNO(x,y)->GetType() == NOP_TREE)
	//	return;

	// Gucken, nicht, dass schon eine Flagge dasteht
	if(GetNO(x,y)->GetType() != NOP_FLAG)
	{
		noBase *no = GetSpecObj<noBase>(x,y);
		if(no)
		{
			no->Destroy();
			delete no;
		}

		SetNO(NULL, x, y);
		SetNO(new noFlag(x, y, player, dis_dir), x, y);

		RecalcBQAroundPointBig(x,y);
	}
}

void GameWorldGame::DestroyFlag(const MapCoord x, const MapCoord y)
{
	// Let's see if there is a flag
	if(GetNO(x,y)->GetType() == NOP_FLAG)
	{
		// Get the attached building if existing
		noBase * building = GetNO(GetXA(x,y,1),GetYA(x,y,1));
		
		// Is this a military building?
		if(building->GetGOT() == GOT_NOB_MILITARY)
		{
			// Maybe demolition of the building is not allowed?
			if(!static_cast<nobMilitary*>(building)->IsDemolitionAllowed())
				// Abort the whole thing
				return;
		}
		
		
		// Demolish, also the building
		noFlag * flag = GetSpecObj<noFlag>(x,y);
		
		SetNO(NULL, x, y);
		flag->DestroyAttachedBuilding();
		flag->Destroy();
		delete flag;

		RecalcBQAroundPointBig(x,y);
	}

	gi->GI_FlagDestroyed(x,y);
}



///////////////////////////////////////////////////////////////////////////////
/**
 *  setzt den echten StraÃen-Wert an der Stelle X,Y (berichtigt).
 *
 * Bit 0-6 jeweils 2 Bit für jede Richtung jeweils der Typ, Bit 7
 *  @author OLiver
 */	
void GameWorldGame::SetRoad(const MapCoord x, const MapCoord y, unsigned char dir, unsigned char type)
{
	assert(dir < 6);

	// Virtuelle StraÃe setzen
	SetVirtualRoad(x, y, dir, type);

	unsigned pos = width * unsigned(y) + unsigned(x);


	// Flag nullen wenn nur noch das real-flag da ist oder es setzen
	if(!nodes[pos].roads[dir])
		nodes[pos].roads_real[dir] = false;
	else
		nodes[pos].roads_real[dir] = true;

	if(gi)
		gi->GI_UpdateMinimap(x,y);
}


///////////////////////////////////////////////////////////////////////////////
/**
 *  setzt den StraÃen-Wert um den Punkt X,Y.
 *
 *  @author OLiver
 */
void GameWorldGame::SetPointRoad(const MapCoord x, const MapCoord y, unsigned char dir, unsigned char type)
{
	assert(dir < 6);

	if(dir >= 3)
		SetRoad(x,y, dir - 3, type);
	else
		SetRoad(GetXA(x, y, dir),GetYA(x, y, dir), dir, type);
}



void GameWorldGame::AddFigure(noBase * fig,const MapCoord x, const MapCoord y)
{
	if(!fig)
		return;

	assert(!GetNode(x,y).figures.search(fig).valid());
	GetNode(x,y).figures.push_back(fig);



	for(unsigned char i = 0;i<6;++i)
	{
		int xa = GetXA(x,y,i);
		int ya = GetYA(x,y,i);

		if(xa <0 || ya < 0 || xa >= width || ya >= height)
			continue;

		if(GetNode(xa,ya).figures.search(fig).valid())
			assert(false);
	}

	//if(fig->GetDir() == 1 || fig->GetDir() == 2)
	//	figures[y*width+x].push_front(fig);
	//else
	//	figures[y*width+x].push_back(fig);
}

void GameWorldGame::RemoveFigure(const noBase * fig,const MapCoord x, const MapCoord y)
{
	for(list<noBase*>::iterator it = GetNode(x,y).figures.begin(); it.valid(); ++it)
	{
		if(*it == fig)
		{
			GetNode(x,y).figures.erase(it);
			return;
		}
	}
}



void GameWorldGame::SetBuildingSite(const BuildingType type,const MapCoord x, const MapCoord y, const unsigned char player)
{
	// Gucken, ob das Gebäude hier überhaupt noch gebaut wrden kann
	BuildingQuality bq = CalcBQ(x,y,player,false,false);

	switch(BUILDING_SIZE[type])
	{
	case BQ_HUT: if(!((bq >= BQ_HUT && bq <= BQ_CASTLE) || bq == BQ_HARBOR)) return; break;
	case BQ_HOUSE: if(!((bq >= BQ_HOUSE && bq <= BQ_CASTLE) || bq == BQ_HARBOR)) return; break;
	case BQ_CASTLE: if(!( bq == BQ_CASTLE || bq == BQ_HARBOR)) return; break;
	case BQ_HARBOR: if(bq != BQ_HARBOR) return; break;
	case BQ_MINE: if(bq != BQ_MINE) return; break;
	default: break;
	}

	// TODO: VerzÃ¶gerungsbugabfrage, kann später ggf. weg
	// Wenn das ein Militärgebäude ist und andere Militärgebäude bereits in der Nähe sind, darf dieses nicht gebaut werden
	if(type >= BLD_BARRACKS && type <= BLD_FORTRESS)
	{
		if(IsMilitaryBuildingNearNode(x,y))
			return;
	}
	
	// Prüfen ob Katapult und ob Katapult erlaubt ist
	if (type == BLD_CATAPULT && !GetPlayer(player)->CanBuildCatapult())
		return;

	// ggf. vorherige Objekte lÃ¶schen
	noBase *no = GetSpecObj<noBase>(x,y);
	if(no)
	{
		no->Destroy();
		delete no;
	}

	// Baustelle setzen
	SetNO(new noBuildingSite(type, x, y, player), x, y);
	gi->GI_UpdateMinimap(x,y);

	// Bauplätze drumrum neu berechnen
	RecalcBQAroundPointBig(x,y);
}

void GameWorldGame::DestroyBuilding(const MapCoord x, const MapCoord y, const unsigned char player)
{
	// Steht da auch ein Gebäude oder eine Baustelle, nicht dass wir aus VerzÃ¶gerung Feuer abreiÃen wollen, das geht schief
	if(GetNO(x,y)->GetType() == NOP_BUILDING || 
		GetNO(x,y)->GetType() == NOP_BUILDINGSITE)
	{
		
		noBaseBuilding * nbb  = GetSpecObj<noBaseBuilding>(x,y);

		// Ist das Gebäude auch von dem Spieler, der es abreiÃen will?
		if(nbb->GetPlayer() != player)
			return;

		// Militärgebäude?
		if(nbb->GetGOT() == GOT_NOB_MILITARY)
		{
			// Darf das Gebäude abgerissen werden?
			if(!static_cast<nobMilitary*>(nbb)->IsDemolitionAllowed())
				// Nein, darf nicht abgerissen werden
				return;
		}

		
		nbb->Destroy();
		delete nbb;
		// Bauplätze drumrum neu berechnen
		RecalcBQAroundPointBig(x,y);
	}
	else
		LOG.lprintf("GameWorldGame::DestroyBuilding: WARNING: BuildingType not found!\n");
}


void GameWorldGame::BuildRoad(const unsigned char playerid,const bool boat_road,
							  unsigned short start_x,unsigned short start_y, const std::vector<unsigned char>& route)
{
	// TODO: VerzÃ¶gerungsbugabfrage, kann später ggf. weg
	if(!GetSpecObj<noFlag>(start_x,start_y))
	{
		RemoveVisualRoad(start_x,start_y,route);
		return;
	}
	// Falscher Spieler?
	else if(GetSpecObj<noFlag>(start_x,start_y)->GetPlayer() != playerid)
	{
		// Dann Weg nicht bauen und ggf. das visuelle wieder zurückbauen
		RemoveVisualRoad(start_x,start_y,route);
		return;
	}

	unsigned short tmpx = start_x, tmpy = start_y;

	// TODO: VerzÃ¶gerungsbugabfrage, kann später ggf. weg
	// Gucken, ob der Weg überhaupt noch gebaut werden kann
	unsigned short testx = start_x, testy = start_y;
	assert(route.size() > 1);
	for(unsigned i = 0;i<route.size()-1;++i)
	{
		int tx = testx,ty = testy;
		testx = GetXA(tx,ty,route[i]);
		testy = GetYA(tx,ty,route[i]);

		// Feld bebaubar und auf unserem Gebiet
		if(!RoadAvailable(boat_road,testx,testy,i,false) || !IsPlayerTerritory(testx,testy))
		{
			// Nein? Dann prüfen ob genau der gewünscht Weg schon da ist und ansonsten den visuellen wieder zurückbauen
			if (RoadAlreadyBuilt(boat_road, start_x, start_y, route))
			{
				return;
			}
			else
			{
				RemoveVisualRoad(start_x,start_y,route);
				return;
			}
		}
	}

	int tx = testx,ty = testy;
	testx = GetXA(tx,ty,route[route.size()-1]);
	testy = GetYA(tx,ty,route[route.size()-1]);

	// Prüfen, ob am Ende auch eine Flagge steht oder eine gebaut werden kann
	if(GetNO(testx,testy)->GetGOT() == GOT_FLAG)
	{
		// Falscher Spieler?
		if(GetSpecObj<noFlag>(testx,testy)->GetPlayer() != playerid)
		{
			// Dann Weg nicht bauen und ggf. das visuelle wieder zurückbauen
			RemoveVisualRoad(start_x,start_y,route);
			return;
		}
	}
	else
	{
		// Es ist keine Flagge dort, dann muss getestet werden, ob da wenigstens eine gebaut werden kann

		// TODO: VerzÃ¶gerungsbugabfrage, kann später ggf. weg
		// Abfragen, ob schon eine Flagge in der Nähe ist (keine Mini-1-Wege)
		for(unsigned char i = 0;i<6;++i)
		{
			if(GetNO(GetXA(testx,testy,i), GetYA(testx,testy,i))->GetGOT() == GOT_FLAG)
			{
				// Dann Weg nicht bauen und ggf. das visuelle wieder zurückbauen
				RemoveVisualRoad(start_x,start_y,route);
				return;
			}
		}

		// TODO: VerzÃ¶gerungsbugabfrage, kann später ggf. weg
		// Abfragen, ob evtl ein Baum gepflanzt wurde, damit der nicht überschrieben wird
		if(GetNO(testx,testy)->GetType() == NOP_TREE)
		{
			// Dann Weg nicht bauen und ggf. das visuelle wieder zurückbauen
			RemoveVisualRoad(start_x,start_y,route);
			return;
		}
	}

	// Evtl Zierobjekte abreiÃen (Anfangspunkt)
	if(IsObjectionableForRoad(start_x,start_y))
	{
		noBase * obj = GetSpecObj<noBase>(start_x,start_y);
		obj->Destroy();
		delete obj;
		SetNO(0,start_x,start_y);
	}

	for(unsigned i = 0;i<route.size();++i)
	{
		SetPointRoad(start_x,start_y, route[i], boat_road?(RoadSegment::RT_BOAT+1):(RoadSegment::RT_NORMAL+1));
		int tx = start_x,ty = start_y;
		start_x = GetXA(tx,ty,route[i]);
		start_y = GetYA(tx,ty,route[i]);
		CalcRoad(tx,ty,GAMECLIENT.GetPlayerID());

		// Evtl Zierobjekte abreiÃen
		if(IsObjectionableForRoad(start_x,start_y))
		{
			noBase * obj = GetSpecObj<noBase>(start_x,start_y);
			obj->Destroy();
			delete obj;
			SetNO(0,start_x,start_y);
		}
	}

	if(GetNO(start_x,start_y)->GetType() != NOP_FLAG)
		SetFlag(start_x,start_y,playerid,(route[route.size()-1]+3)%6);

	RoadSegment * rs = new RoadSegment(boat_road?RoadSegment::RT_BOAT:RoadSegment::RT_NORMAL,
		GetSpecObj<noFlag>(tmpx,tmpy),GetSpecObj<noFlag>(start_x,start_y),route);

	GetSpecObj<noFlag>(tmpx,tmpy)->routes[route.front()] = rs;
	GetSpecObj<noFlag>(start_x,start_y)->routes[(route.back()+3)%6] = rs;

	// Der Wirtschaft mitteilen, dass eine neue StraÃe gebaut wurde, damit sie alles Näcige macht
	GetPlayer(playerid)->NewRoad(rs);

}



bool GameWorldGame::IsObjectionableForRoad(const MapCoord x, const MapCoord y)
{
	if(GetNO(x,y)->GetGOT() == GOT_ENVOBJECT)
	{
		noEnvObject *no = GetSpecObj<noEnvObject>(x,y);
		unsigned short type = no->GetItemID();
		switch(no->GetItemFile())
		{
		case 0xFFFF: // map_?_z.lst
			{
				if(type == 505 || type == 506 || type == 507 || type == 508 || type == 510 || (type >= 542 && type <= 546) ||
					type == 512 || type == 513 || // Kakteen
					type == 536 || type == 541) // abgeerntete Getreidefelder
				   return true;
			} break;
		case 0:
			{
				// todo:
			} break;
		case 1:
			{
				if(type <= 12)
					return true;
				// todo:
			} break;
		case 2:
			{
				// todo:
			} break;
		case 3:
			{
				// todo:
			} break;
		case 4:
			{
				// todo:
			} break;
		case 5:
			{
				// todo:
			} break;
		// Charburner rests
		case 6:
			{
				return true;
			} break;
		}
	}

	return false;
}

void GameWorldGame::DestroyRoad(const MapCoord x, const MapCoord y, const unsigned char dir)
{
	// TODO: VerzÃ¶gerungsbugabfrage, kann später ggf. weg
	if(!GetSpecObj<noFlag>(x,y))
		return;

	GetSpecObj<noFlag>(x,y)->DestroyRoad(dir);
}

void GameWorldGame::UpgradeRoad(const MapCoord x, const MapCoord y, const unsigned char dir)
{
	if(!GetSpecObj<noFlag>(x,y))
		return;

	GetSpecObj<noFlag>(x,y)->UpgradeRoad(dir);
}

void GameWorldGame::RecalcTerritory(const noBaseBuilding * const building,const unsigned short radius, const bool destroyed, const bool newBuilt)
{
	std::list<nobBaseMilitary*> buildings; // Liste von Militärgebäuden in der Nähe

	// alle Militärgebäude in der Nähe abgrasen
	LookForMilitaryBuildings(buildings,building->GetX(),building->GetY(),3);

	// Radius der noch draufaddiert wird auf den eigentlich ausreichenden Bereich, für das Eliminieren von
	// herausragenden Landesteilen und damit Grenzsteinen
	const int ADD_RADIUS = 2;

	// Koordinaten erzeugen für TerritoryRegion 
	int x1 = int(building->GetX())-(radius+ADD_RADIUS);
	int y1 = int(building->GetY())-(radius+ADD_RADIUS);
	int x2 = int(building->GetX())+(radius+ADD_RADIUS)+1;
	int y2 = int(building->GetY())+(radius+ADD_RADIUS)+1;


	TerritoryRegion tr(x1,y1,x2,y2,this);

	buildings.sort(nobBaseMilitary::Compare);

	// Alle Gebäude ihr Terrain in der Nähe neu berechnen
	for(std::list<nobBaseMilitary*>::iterator it = buildings.begin();it!=buildings.end();++it)
	{
		// Ist es ein richtiges Militärgebäude?
		if((*it)->GetBuildingType() >= BLD_BARRACKS && (*it)->GetBuildingType() <= BLD_FORTRESS)
		{
			// Wenn es noch nicht besetzt war(also gerade neu gebaut), darf es nicht mit einberechnet werden!
			if(static_cast<nobMilitary*>(*it)->IsNewBuilt())
				continue;
		}

		// Wenn das Gebäude abgerissen wird oder wenn es noch nicht besetzt war, natürlich nicht mit einberechnen
		if(*it != building || !destroyed)
			tr.CalcTerritoryOfBuilding(*it);
	}

	// Baustellen von Häfen mit einschlieÃen
	for(std::list<noBuildingSite*>::iterator it = harbor_building_sites_from_sea.begin();
		it != harbor_building_sites_from_sea.end();++it)
	{
		if(*it != building || !destroyed)
			tr.CalcTerritoryOfBuilding(*it);
	}
		


	// Merken, wo sich der Besitzer geändert hat
	bool * owner_changed = new bool[(x2-x1)*(y2-y1)];


	std::vector<int> sizeChanges(GAMECLIENT.GetPlayerCount());
	// Daten von der TR kopieren in die richtige Karte, dabei zus. Grenzen korrigieren und Objekte zerstÃ¶ren, falls
	// das Land davon jemanden anders nun gehÃ¶rt
 	for(int y = y1;y<y2;++y)
	{
		for(int x = x1;x<x2;++x)
		{
			unsigned char prev_player,player;
			MapCoord tx,ty;
			ConvertCoords(x,y,&tx,&ty);
			// Wenn der Punkt den Besitz geändert hat
			if((prev_player=GetNode(tx,ty).owner) != (player=tr.GetOwner(x,y)))
			{
				// Dann entsprechend neuen Besitzer setzen
				GetNode(tx,ty).owner = player;
				owner_changed[(x2-x1)*(y-y1)+(x-x1)] = true;
				if (player != 0)
					sizeChanges[player-1]++;
				if (prev_player != 0)
					sizeChanges[prev_player-1]--;
			}
			else
				owner_changed[(x2-x1)*(y-y1)+(x-x1)] = false;
		}
	}

	for (unsigned i=0; i<GAMECLIENT.GetPlayerCount(); ++i)
	{
		GetPlayer(i)->ChangeStatisticValue(STAT_COUNTRY, sizeChanges[i]);

		// Negatives Wachstum per Post dem/der jeweiligen Landesherren/dame melden, nur bei neugebauten Gebäuden
		if (newBuilt && sizeChanges[i] < 0)
		{
			if(GameClient::inst().GetPlayerID() == i)
				GameClient::inst().SendPostMessage(
					new ImagePostMsgWithLocation(_("Lost land by this building"), PMC_MILITARY, building->GetX(), building->GetY(), 
					building->GetBuildingType(), building->GetNation()));
		}
	}

	for(int y = y1;y<y2;++y)
	{
		for(int x = x1;x<x2;++x)
		{
			MapCoord tx,ty;
			ConvertCoords(x,y,&tx,&ty);
			bool isplayerterritory_near = false;
			/// Grenzsteine, die alleine "rausragen" und nicht mit einem richtigen Territorium verbunden sind, raushauen
			for(unsigned d = 0;d<6;++d)
			{
				if(IsPlayerTerritory(GetXA(tx,ty,d),GetYA(tx,ty,d)))
				{
					isplayerterritory_near = true;
					break;
				}
			}

			// Wenn kein Land angrenzt, dann nicht nehmen
			if(!isplayerterritory_near)
				GetNode(tx,ty).owner = 0;

			// Drumherum (da ja Grenzen mit einberechnet werden ins Gebiet, da darf trotzdem nichts stehen) alles vom Spieler zerstÃ¶ren
			// nicht das Militärgebäude oder dessen Flagge nochmal abreiÃen
			if(owner_changed[(x2-x1)*(y-y1)+(x-x1)])
			{
				for(unsigned char i = 0;i<6;++i)
				{
					unsigned short ttx = GetXA(tx,ty,i), tty = GetYA(tx,ty,i);

					DestroyPlayerRests(ttx,tty,GetNode(tx,ty).owner,building);

					// BQ neu berechnen
					GetNode(ttx,tty).bq = CalcBQ(ttx,tty,GAMECLIENT.GetPlayerID());
					// ggf den noch darüber, falls es eine Flagge war (kann ja ein Gebäude entstehen)
					if(GetNodeAround(ttx,tty,1).bq)
						SetBQ(GetXA(ttx,tty,1),GetYA(ttx,tty,1),GAMECLIENT.GetPlayerID());
				}

				if(gi)
					gi->GI_UpdateMinimap(tx,ty);
			}
		}
	}

	delete [] owner_changed;

	// Grenzsteine neu berechnen, noch 1 über das Areal hinausgehen, da dieses auch die Grenzsteine rundrum
	// mit beeinflusst
	
	// In diesem Array merken, wie wieviele Nachbarn ein Grenzstein hat
	//unsigned neighbors[y2-y1+7][x2-x1+7];
	std::vector<std::vector <unsigned> > neighbors(y2-y1+7, std::vector<unsigned>(x2-x1+7, 0));
	

	for(int y = y1-3;y < y2+3;++y)
	{
		//memset(neighbors[y-(y1-3)],0,x2-x1+7);
		
		for(int x = x1-3;x < x2+3;++x)
		{
			// Korrigierte X-Koordinaten
			MapCoord xc,yc;
			ConvertCoords(x,y,&xc,&yc);

			unsigned char owner = GetNode(xc,yc).owner;

			// Grenzstein direkt auf diesem Punkt?
			if(owner && IsBorderNode(xc,yc,owner))
			{
				GetNode(xc,yc).boundary_stones[0] = owner;

				// Grenzsteine prüfen auf den Zwischenstücken in die 3 Richtungen nach unten und nach rechts
				for(unsigned i = 0;i<3;++i)
				{
					MapCoord xa = GetXA(xc,yc,3+i), ya = GetYA(xc,yc,3+i);
					if(IsBorderNode(xa,ya,owner))
						GetNode(xc,yc).boundary_stones[i+1] = owner;
					else
						GetNode(xc,yc).boundary_stones[i+1] = 0;

				}
				
				// Zählen
				for(unsigned i = 0;i<6;++i)
				{
					neighbors[y-(y1-3)][x-(x1-3)] = 0;
					if(GetNodeAround(xc,yc,i).boundary_stones[0] == owner)
						++neighbors[y-(y1-3)][x-(x1-3)];
				}
			}
			else
			{
				// Kein Grenzstein --> etwaige vorherige Grenzsteine lÃ¶schen
				for(unsigned i = 0;i<4;++i)
					GetNode(xc,yc).boundary_stones[i] = 0;

				//for(unsigned i = 0;i<3;++i)
				//	GetNodeAround(x,y,3+i).boundary_stones[i+1] = 0;
			}
			
			
		}
	}
	
/*	// Nochmal durchgehen und bei Grenzsteinen mit mehr als 3 Nachbarn welche löschen
	// da sich sonst gelegentlich solche "Klötzchen" bilden können
	for(int y = y1-3;y < y2+3;++y)
	{
		//memset(neighbors[y-(y1-3)],0,x2-x1+7);
		
		for(int x = x1-3;x < x2+3;++x)
		{
			
			// Korrigierte X-Koordinaten (nicht über den Rand gehen)
			MapCoord xc,yc;
			ConvertCoords(x,y,&xc,&yc);
			
			// Steht auch hier ein Grenzstein?
			unsigned char owner = GetNode(xc,yc).boundary_stones[0];
			if(!owner)
				continue;
			
			if(neighbors[y-(y1-3)][x-(x1-3)] > 2)
			{
				for(unsigned dir = 0;dir<3 && neighbors[y-(y1-3)][x-(x1-3)] > 2;++dir)
				{
					// Da ein Grenzstein vom selben Besitzer?
					MapCoord xa = GetXA(xc,yc,dir+3);
					MapCoord ya = GetYA(xc,yc,dir+3);
					
					if(GetNode(xa,ya).boundary_stones[0] == owner)
					{
						Point<int> p(x,y);
						Point<int> pa = GetPointAround(p,dir+3);
						// Hat der auch zu viele Nachbarn?
						if(neighbors[pa.y-(y1-3)][pa.x-(x1-3)] > 2)
						{
							// Dann löschen wir hier einfach die Verbindung
							GetNode(xc,yc).boundary_stones[dir+1] = 0;
							--neighbors[y-(y1-3)][x-(x1-3)];
							--neighbors[pa.y-(y1-3)][pa.x-(x1-3)];
						}
						
					}
				}
				
			}
		}
	}*/

	// Sichtbarkeiten berechnen
	
	// Wurde es zerstÃ¶rt, müssen die Sichtbarkeiten entsprechend neu berechnet werden, ansonsten reicht es auch
	// sie einfach auf sichtbar zu setzen
	unsigned harborRadius = (building->GetBuildingType() == BLD_HARBORBUILDING) 
		? HARBOR_ALONE_RADIUS : static_cast<const nobBaseMilitary*>(building)->GetMilitaryRadius();
	if(destroyed)
		RecalcVisibilitiesAroundPoint(building->GetX(),building->GetY(),harborRadius+VISUALRANGE_MILITARY,
			building->GetPlayer(), destroyed ? building : 0);
	else
		SetVisibilitiesAroundPoint(building->GetX(),building->GetY(),harborRadius+VISUALRANGE_MILITARY,
			building->GetPlayer());
}

void GameWorldGame::DestroyPlayerRests(const MapCoord x, const MapCoord y, const unsigned char new_player,const noBaseBuilding * exception)
{
	noBase * no = GetNO(x,y);


	// Flaggen, Gebäude und Baustellen zerstÃ¶ren, aber keine übernommenen und nicht die Ausahme oder dessen Flagge!
	if((no->GetType() == NOP_FLAG || no->GetType() == NOP_BUILDING || no->GetType() == NOP_BUILDINGSITE) && exception != no)
	{
		// Wurde das Objekt auch nicht vom Gegner übernommen?
		if(static_cast<noRoadNode*>(no)->GetPlayer()+1 != new_player)
		{
			// vorher Bescheid sagen
			if(no->GetType() == NOP_FLAG && no != (exception?exception->GetFlag():0))
				static_cast<noFlag*>(no)->DestroyAttachedBuilding();

			no->Destroy();

			delete no;

			return;
		}
	}


	// ggf. Weg kappen
	unsigned char dir;
	noFlag * flag = GetRoadFlag(x,y,dir,0xFF);
	if(flag)
	{
		// Die MinistraÃe von dem Militärgebäude nich abreiÃen!
		if(flag->routes[dir]->GetLength() == 1)
		{
			if(flag->routes[dir]->GetF2() == exception)
				return;
		}

		flag->DestroyRoad(dir);
	}
}


bool GameWorldGame::IsNodeForFigures(const MapCoord x, const MapCoord y)
{
	// Nicht über die Kante gehen!
	if(x>=width||y>=height)
		return false;


	// Irgendwelche Objekte im Weg?
	noBase::BlockingManner bm =GetNO(x,y)->GetBM();
	if(bm != noBase::BM_NOTBLOCKING && bm != noBase::BM_TREE && bm != noBase::BM_FLAG)
		return false;

	unsigned char t;

	// Terrain untersuchen
	unsigned char good_terrains = 0;
	for(unsigned char i = 0;i<6;++i)
	{
		t = GetTerrainAround(x,y,i);
		if(TERRAIN_BQ[t] == BQ_CASTLE || TERRAIN_BQ[t] == BQ_MINE || TERRAIN_BQ[t] == BQ_FLAG) ++good_terrains;
		else if(TERRAIN_BQ[t] == BQ_DANGER) return false; // in die Nähe von Lava usw. dürfen die Figuren gar nich kommen!
	}

	// Darf nicht im Wasser liegen,
	if(!good_terrains)
		return false;

	return true;
}

void GameWorldGame::RoadNodeAvailable(const MapCoord x, const MapCoord y)
{
	// Figuren direkt daneben
	for(unsigned char i = 0;i<6;++i)
	{
		// Nochmal prüfen, ob er nun wirklich verfügbar ist (evtl blocken noch mehr usw.)
		if(!IsRoadNodeForFigures(x,y,(i+3)%6))
			continue;

		// Koordinaten um den Punkt herum
		MapCoord xa = GetXA(x,y,i), ya = GetYA(x,y,i);

		
		// Figuren Bescheid sagen, es kÃ¶nnen auch auf den Weg gestoppte sein, die müssen auch berücksichtigt 
		// werden, daher die *From-Methode
		list<noBase*> objects;
		GetDynamicObjectsFrom(xa,ya,objects);

		// Auch Figuren da, die rumlaufen kÃ¶nnen?
		if(objects.size())
		{
	
			for(list<noBase*>::iterator it = objects.begin();it.valid();++it)
			{
				if((*it)->GetType() == NOP_FIGURE)
					static_cast<noFigure*>(*it)->NodeFreed(x,y);
			}


			//// Achtung: Hier kÃ¶nnen Iteratoren gelÃ¶scht werden in NodeFreed, daher Sicherheitsschleife!
			//list<noBase*>::iterator next_it;
			//for(list<noBase*>::iterator it = GetFigures(xa,ya).begin();
			//	it.valid();it = next_it)
			//{
			//	next_it = it.GetNext();
			//	if((*it)->GetType() == NOP_FIGURE)
			//		static_cast<noFigure*>(*it)->NodeFreed(x,y);
			//}
		}
	}
}



/// Kleine Klasse für Angriffsfunktion für einen potentielle angreifenden Soldaten
struct PotentialAttacker
{
	nofPassiveSoldier * soldier;
	/// Weglänge zum Angriffsziel
	unsigned distance;
};

void GameWorldGame::Attack(const unsigned char player_attacker, const MapCoord x, const MapCoord y, const unsigned short soldiers_count, const bool strong_soldiers)
{
	// VerzÃ¶gerungsbug-Abfrage:
	// Existiert das angegriffenen Gebäude überhaupt noch?
	if(GetNO(x,y)->GetGOT() != GOT_NOB_MILITARY && GetNO(x,y)->GetGOT() != GOT_NOB_HQ
	&& GetNO(x,y)->GetGOT() != GOT_NOB_HARBORBUILDING)
		return;

	// Auch noch ein Gebäude von einem Feind (nicht inzwischen eingenommen)?
	if(!GetPlayer(player_attacker)->IsPlayerAttackable(GetSpecObj<noBuilding>(x,y)->GetPlayer()))
		return;

	// Prüfen, ob der angreifende Spieler das Gebäude überhaupt sieht (Cheatvorsorge)
	if(CalcWithAllyVisiblity(x,y,player_attacker) != VIS_VISIBLE)
		return;

	// Ist das angegriffenne ein normales Gebäude?
	nobBaseMilitary * attacked_building = GetSpecObj<nobBaseMilitary>(x,y);
	if(attacked_building->GetBuildingType() >= BLD_BARRACKS && attacked_building->GetBuildingType() <= BLD_FORTRESS)
	{
		// Wird es gerade eingenommen?
		if(static_cast<nobMilitary*>(attacked_building)->IsCaptured())
			// Dann darf es nicht angegriffen werden
			return;
		if (static_cast<nobMilitary*>(attacked_building)->IsNewBuilt())
			return;
	}

	// Militärgebäude in der Nähe finden
	std::list<nobBaseMilitary*> buildings;
	LookForMilitaryBuildings(buildings,x,y,3);

	// Liste von verfügbaren Soldaten, geordnet einfügen, damit man dann starke oder schwache Soldaten nehmen kann
	list<PotentialAttacker> soldiers;


	for(std::list<nobBaseMilitary*>::iterator it = buildings.begin();it!=buildings.end();++it)
	{
		// Muss ein Gebäude von uns sein und darf nur ein "normales Militärgebäude" sein (kein HQ etc.)
		if((*it)->GetPlayer() == player_attacker && (*it)->GetBuildingType() >= BLD_BARRACKS && (*it)->GetBuildingType() <= BLD_FORTRESS)
		{

			// Soldaten ausrechnen, wie viel man davon nehmen kÃ¶nnte, je nachdem wie viele in den
			// Militäreinstellungen zum Angriff eingestellt wurden
			unsigned short soldiers_count =
				(static_cast<nobMilitary*>(*it)->GetTroopsCount()>1)?
				((static_cast<nobMilitary*>(*it)->GetTroopsCount()-1)*GetPlayer(player_attacker)->military_settings[3]/MILITARY_SETTINGS_SCALE[3]):0;

      unsigned int distance = CalcDistance(x,y,(*it)->GetX(),(*it)->GetY());

      // Falls Entfernung grÃ¶Ãer als Basisreichweite, Soldaten subtrahieren
      if (distance > BASE_ATTACKING_DISTANCE)
      {
        // je einen soldaten zum entfernen vormerken für jeden EXTENDED_ATTACKING_DISTANCE groÃen Schritt
        unsigned short soldiers_to_remove = ((distance - BASE_ATTACKING_DISTANCE + EXTENDED_ATTACKING_DISTANCE - 1) / EXTENDED_ATTACKING_DISTANCE);
        if (soldiers_to_remove < soldiers_count)
          soldiers_count -= soldiers_to_remove;
        else
          continue;
      }

			if(soldiers_count)
			{
				// und auch der Weg zu FuÃ darf dann nicht so weit sein, wenn das alles bestanden ist, kÃ¶nnen wir ihn nehmen..
				// Bei dem freien Pfad noch ein bisschen Toleranz mit einberechnen
				if(FindHumanPath(x,y,(*it)->GetX(),(*it)->GetY(),MAX_ATTACKING_RUN_DISTANCE) != 0xFF) // TODO check: hier wird ne random-route berechnet? soll das so?
				{
					// Soldaten davon nehmen
					unsigned i = 0;
					for(list<nofPassiveSoldier*>::iterator it2 = strong_soldiers?
						static_cast<nobMilitary*>(*it)->troops.end():static_cast<nobMilitary*>(*it)->troops.begin()
						;it2.valid()&&i<soldiers_count;++i)
					{
						// Sortiert einfügen (aufsteigend nach Rang)

						unsigned old_size = soldiers.size();

						for(list<PotentialAttacker>::iterator it3 = soldiers.end(); it3.valid(); --it3)
						{
							// Ist das einzufügende Item grÃ¶Ãer als das aktuelle?
							// an erster Stelle nach Rang, an zweiter dann nach Entfernung gehen
							if( (it3->soldier->GetRank() < (*it2)->GetRank() && !strong_soldiers) ||
								(it3->soldier->GetRank() > (*it2)->GetRank() && strong_soldiers) ||
								(it3->soldier->GetRank() == (*it2)->GetRank() && it3->distance > distance))
							{
								// ja dann hier einfügen
								PotentialAttacker pa = { *it2,distance };
								soldiers.insert(it3, pa);
								break;
							}
						}

						// Ansonsten ganz nach vorn einfügen, wenns noch nich eingefügt wurde
						if(old_size == soldiers.size())
						{
							PotentialAttacker pa = { *it2,distance };
							soldiers.push_front(pa);
						}

						if(strong_soldiers)
							--it2;
						else
							++it2;
					}
				}
			}
		}
	}

	// Alle Soldaten zum Angriff schicken (jeweils wieder von hinten oder vorne durchgehen um schwache oder starke
	// Soldaten zu nehmen)
	unsigned short i = 0;

	for(list<PotentialAttacker>::iterator it = soldiers.begin();
		it.valid()&&i<soldiers_count;++i,++it)
	{
		// neuen Angreifer-Soldaten erzeugen
		new nofAttacker(it->soldier,attacked_building);
		// passiven Soldaten entsorgen
		it->soldier->Destroy();
		delete it->soldier;
	}

	/*if(!soldiers.size())
		LOG.lprintfS("GameWorldGame::Attack: WARNING: Attack failed. No Soldiers available!\n");*/
}

void  GameWorldGame::AttackViaSea(const unsigned char player_attacker, const MapCoord x, const MapCoord y, const unsigned short soldiers_count, const bool strong_soldiers)
{
	// VerzÃ¶gerungsbug-Abfrage:
	// Existiert das angegriffenen Gebäude überhaupt noch?
	if(GetNO(x,y)->GetGOT() != GOT_NOB_MILITARY && GetNO(x,y)->GetGOT() != GOT_NOB_HQ
	&& GetNO(x,y)->GetGOT() != GOT_NOB_HARBORBUILDING)
		return;

	// Auch noch ein Gebäude von einem Feind (nicht inzwischen eingenommen)?
	if(!GetPlayer(player_attacker)->IsPlayerAttackable(GetSpecObj<noBuilding>(x,y)->GetPlayer()))
		return;

	// Prüfen, ob der angreifende Spieler das Gebäude überhaupt sieht (Cheatvorsorge)
	if(CalcWithAllyVisiblity(x,y,player_attacker) != VIS_VISIBLE)
		return;
		
	// Verfügbare Soldaten herausfinden
	std::list<GameWorldBase::PotentialSeaAttacker> attackers;
	GetAvailableSoldiersForSeaAttack(player_attacker,x,y,&attackers);
	
	// Ist das angegriffenne ein normales Gebäude?
	nobBaseMilitary * attacked_building = GetSpecObj<nobBaseMilitary>(x,y);
	if(attacked_building->GetBuildingType() >= BLD_BARRACKS && attacked_building->GetBuildingType() <= BLD_FORTRESS)
	{
		// Wird es gerade eingenommen?
		if(static_cast<nobMilitary*>(attacked_building)->IsCaptured())
			// Dann darf es nicht angegriffen werden
			return;
		if (static_cast<nobMilitary*>(attacked_building)->IsNewBuilt())
			return;
	}
	
	unsigned counter = 0;
	if(strong_soldiers)
		for(std::list<GameWorldBase::PotentialSeaAttacker>::iterator it = attackers.begin();it!=attackers.end() &&
			counter < soldiers_count;++it,++counter)
		{
			// neuen Angreifer-Soldaten erzeugen
			new nofAttacker(it->soldier,attacked_building,it->harbor);
			// passiven Soldaten entsorgen
			it->soldier->Destroy();
			delete it->soldier;
		}
	else
		for(std::list<GameWorldBase::PotentialSeaAttacker>::reverse_iterator it = attackers.rbegin();it!=attackers.rend() &&
			counter < soldiers_count;++it,++counter)
		{
			// neuen Angreifer-Soldaten erzeugen
			new nofAttacker(it->soldier,attacked_building,it->harbor);
			// passiven Soldaten entsorgen
			it->soldier->Destroy();
			delete it->soldier;
		}
}


bool GameWorldGame::IsRoadNodeForFigures(const MapCoord x, const MapCoord y,const unsigned char dir)
{
	/// Objekte sammeln
	list<noBase*> objects;
	GetDynamicObjectsFrom(x,y,objects);

	// Figuren durchgehen, bei Kämpfen und wartenden Angreifern sowie anderen wartenden Figuren stoppen!
	for(list<noBase*>::iterator it = objects.begin();it.valid();++it)
	{
		// andere wartende Figuren
		if((*it)->GetType() == NOP_FIGURE)
		{
			noFigure * fig = static_cast<noFigure*>(*it);
			// Figuren dürfen sich nicht gegenüber stehen, sonst warten sie ja ewig aufeinander
			// AuÃerdem muss auch die Position stimmen, sonst spinnt der ggf. rum, da 
			if(fig->IsWaitingForFreeNode() && (fig->GetDir()+3)%6 != dir)
				return false;
		}

		// Kampf
		if((*it)->GetGOT() == GOT_FIGHTING)
		{
			if(static_cast<noFighting*>(*it)->IsActive())
				return false;
		}

		//// wartende Angreifer
		if((*it)->GetGOT() == GOT_NOF_ATTACKER)
		{
			if(static_cast<nofAttacker*>(*it)->IsBlockingRoads())
				return false;
		}
	}

	// alles ok
	return true;
}

/// Lässt alle Figuren, die auf diesen Punkt  auf Wegen zulaufen, anhalten auf dem Weg (wegen einem Kampf)
void GameWorldGame::StopOnRoads(const MapCoord x, const MapCoord y, const unsigned char dir)
{
	// Figuren drumherum sammeln (auch von dem Punkt hier aus)
	list<noBase*> figures;

	// Auch vom Ausgangspunkt aus, da sie im GameWorldGame wegem Zeichnen auch hier hängen kÃ¶nnen!
	for(list<noBase*>::iterator it = GetFigures(x,y).begin();it.valid();++it)
		if((*it)->GetType() == NOP_FIGURE)
			figures.push_back(*it);

	// Und natürlich in unmittelbarer Umgebung suchen
	for(unsigned d = 0;d<6;++d)
	{
		for(list<noBase*>::iterator it = GetFigures(GetXA(x,y,d),GetYA(x,y,d)).begin()
			;it.valid();++it)
			if((*it)->GetType() == NOP_FIGURE)
				figures.push_back(*it);
	}

	for(list<noBase*>::iterator it = figures.begin();it.valid();++it)
	{
		if(dir <6)
		{
			if((dir+3)%6 == static_cast<noFigure*>(*it)->GetDir())
			{
				if(GetXA(x,y,dir) == static_cast<noFigure*>(*it)->GetX() &&
					GetYA(x,y,dir) == static_cast<noFigure*>(*it)->GetY())
					continue;
			}
		}

		// Derjenige muss ggf. stoppen, wenn alles auf ihn zutrifft
		static_cast<noFigure*>(*it)->StopIfNecessary(x,y);
	}
}

void GameWorldGame::Armageddon()
{
	for(unsigned i = 0;i < map_size; ++i)
	{
		if(nodes[i].obj)
		{
			if(nodes[i].obj->GetGOT() == GOT_FLAG)
			{
				noFlag * flag = static_cast<noFlag*>(nodes[i].obj);
				nodes[i].obj = 0;
				flag->DestroyAttachedBuilding();
				flag->Destroy();
				delete flag;
			}
		}
	}
}

void GameWorldGame::Armageddon(const unsigned char player)
{
	for(unsigned i = 0;i < map_size; ++i)
	{
		if(nodes[i].obj)
		{
			if(nodes[i].obj->GetGOT() == GOT_FLAG)
			{
				noFlag * flag = static_cast<noFlag*>(nodes[i].obj);
        if (flag->GetPlayer() == player)
        {
				  nodes[i].obj = 0;
				  flag->DestroyAttachedBuilding();
				  flag->Destroy();
				  delete flag;
        }
			}
		}
	}
}



bool GameWorldGame::ValidWaitingAroundBuildingPoint(const MapCoord x, const MapCoord y, nofAttacker * attacker)
{
	// Gültiger Punkt für Figuren?
	if(!IsNodeForFigures(x,y))
		return false;

	// Objekte, die sich hier befinden durchgehen
	for(list<noBase*>::iterator it = GetFigures(x,y).begin();it.valid();++it)
	{
		// Ist hier ein anderer Soldat, der hier ebenfalls wartet?
		if((*it)->GetGOT() == GOT_NOF_ATTACKER || (*it)->GetGOT() == GOT_NOF_AGGRESSIVEDEFENDER || 
			(*it)->GetGOT() == GOT_NOF_DEFENDER)
		{
			if(static_cast<nofActiveSoldier*>(*it)->GetState() == nofActiveSoldier::STATE_WAITINGFORFIGHT ||
				static_cast<nofActiveSoldier*>(*it)->GetState() == nofActiveSoldier::STATE_ATTACKING_WAITINGAROUNDBUILDING )
				return false;
		}

		// Oder ein Kampf, der hier tobt?
		if((*it)->GetGOT() == GOT_FIGHTING)
			return false;
	}

	return true;
}

bool GameWorldGame::ValidPointForFighting(const MapCoord x, const MapCoord y, const bool avoid_military_building_flags, nofActiveSoldier *exception)
{
	// Is this a flag of a military building?
	if(avoid_military_building_flags && GetNO(x,y)->GetGOT() == GOT_FLAG)
	{
		GO_Type got = GetNO(GetXA(x,y,1),GetYA(x,y,1))->GetGOT();
		if(got == GOT_NOB_MILITARY || got == GOT_NOB_HARBORBUILDING || got == GOT_NOB_HQ)
			return false;
	}
	
	// Objekte, die sich hier befinden durchgehen
	for(list<noBase*>::iterator it = GetFigures(x,y).begin();it.valid();++it)
	{
		// Ist hier ein anderer Soldat, der hier ebenfalls wartet?
		if((*it)->GetGOT() == GOT_NOF_ATTACKER || (*it)->GetGOT() == GOT_NOF_AGGRESSIVEDEFENDER || 
			(*it)->GetGOT() == GOT_NOF_DEFENDER)
		{
			if (static_cast<nofActiveSoldier*>(*it) == exception)
				continue;
			switch(static_cast<nofActiveSoldier*>(*it)->GetState())
			{
			default: break;
			case nofActiveSoldier::STATE_WAITINGFORFIGHT:
			case nofActiveSoldier::STATE_ATTACKING_WAITINGAROUNDBUILDING:
			case nofActiveSoldier::STATE_ATTACKING_WAITINGFORDEFENDER:
			case nofActiveSoldier::STATE_DEFENDING_WAITING:
				return false;
			}
		}

		// Oder ein Kampf, der hier tobt?
		if((*it)->GetGOT() == GOT_FIGHTING)
		{
			if(static_cast<noFighting*>(*it)->IsActive())
			return false;
		}
	}
	// Liegt hier was rum auf dem man nicht kämpfen sollte?
	noBase::BlockingManner bm = GetNO(x,y)->GetBM();
	if(bm != noBase::BM_NOTBLOCKING && bm != noBase::BM_TREE && bm != noBase::BM_FLAG)
		return false;

	return true;
}

bool GameWorldGame::IsPointCompletelyVisible(const MapCoord x, const MapCoord y, const unsigned char player, const noBaseBuilding * const exception) const
{ 
	std::list<nobBaseMilitary*> buildings;
	LookForMilitaryBuildings(buildings,x,y,3);

	// Sichtbereich von Militärgebäuden
	for(std::list<nobBaseMilitary*>::iterator it = buildings.begin();it!=buildings.end();++it)
	{
		if((*it)->GetPlayer() == player && *it != exception)
		{
			// Prüfen, obs auch unbesetzt ist
			if((*it)->GetGOT() == GOT_NOB_MILITARY)
			{
				if(static_cast<nobMilitary*>(*it)->IsNewBuilt())
					continue;
			}

			if(CalcDistance(x,y,(*it)->GetX(),(*it)->GetY()) 
				<= unsigned((*it)->GetMilitaryRadius()+VISUALRANGE_MILITARY))
				return true;
		}
	}

	// Sichtbereich von Hafenbaustellen
	for(std::list<noBuildingSite*>::const_iterator it = harbor_building_sites_from_sea.begin();
		it!=harbor_building_sites_from_sea.end();++it)
	{
		if((*it)->GetPlayer() == player && *it != exception)
		{

			if(CalcDistance(x,y,(*it)->GetX(),(*it)->GetY()) 
				<= unsigned(HARBOR_ALONE_RADIUS+VISUALRANGE_MILITARY))
				return true;
		}
	}

	// Sichtbereich von Spähtürmen

	for(std::list<nobUsual*>::const_iterator it = GetPlayer(player)->GetBuildings(BLD_LOOKOUTTOWER).begin();
		it!=GetPlayer(player)->GetBuildings(BLD_LOOKOUTTOWER).end(); ++it)
	{
		// Ist Späturm überhaupt besetzt?
		if(!(*it)->HasWorker())
			continue;

		// Nicht die Ausnahme wählen
		if(*it == exception)
			continue;

		// Liegt Spähturm innerhalb des Sichtradius?
		if(CalcDistance(x,y,(*it)->GetX(),(*it)->GetY()) <= VISUALRANGE_LOOKOUTTOWER)
				return true;
	}

	

	// Erkunder prüfen

	// Zunächst auf dem Punkt selbst
	if(IsScoutingFigureOnNode(x,y,player,0))
		return true;

	// Und drumherum
	for(MapCoord tx=GetXA(x,y,0), r = 1;r<=VISUALRANGE_EXPLORATION_SHIP;tx=GetXA(tx,y,0),++r)
	{
		MapCoord tx2 = tx, ty2 = y;
		for(unsigned i = 2;i<8;++i)
		{
			for(MapCoord r2=0;r2<r;GetPointA(tx2,ty2,i%6),++r2)
			{
				if(IsScoutingFigureOnNode(tx2,ty2,player,r))
					return true;
			}
		}
	}



	return false;

	///// Auf eigenem Terrain --> sichtbar
	//if(GetNode(x,y).owner == player+1)
	//	visible = true;
}

bool GameWorldGame::IsScoutingFigureOnNode(const MapCoord x, const MapCoord y, const unsigned player, const unsigned distance) const
{
	list<noBase*> objects;
	GetDynamicObjectsFrom(x,y,objects);

	// Späher/Soldaten in der Nähe prüfen und direkt auf dem Punkt
	for(list<noBase*>::iterator it = objects.begin();it.valid();++it)
	{
		if(distance <= VISUALRANGE_SCOUT)
		{
			// Späher?
			if((*it)->GetGOT() == GOT_NOF_SCOUT_FREE)
			{
				// Prüfen, ob er auch am Erkunden ist und an der Position genau und ob es vom richtigen Spieler ist
				nofScout_Free* scout = dynamic_cast<nofScout_Free*>(*it);
				if(scout->GetX() == x && scout->GetY() == y && scout->GetPlayer() == player)
					return true;
			}
		}

		// Soldaten?
		if(distance <= VISUALRANGE_SOLDIER)
		{
			// Soldaten?
			if((*it)->GetGOT() == GOT_NOF_ATTACKER || (*it)->GetGOT() == GOT_NOF_AGGRESSIVEDEFENDER)
			{
				nofActiveSoldier * soldier = dynamic_cast<nofActiveSoldier*>(*it);
				if(soldier->GetX() == x && soldier->GetY() == y && soldier->GetPlayer() == player)
					return true;
			}
			// Kämpfe (wo auch Soldaten drin sind)
			else if((*it)->GetGOT() == GOT_FIGHTING)
			{
				// Prüfen, ob da ein Soldat vom angegebenen Spieler dabei ist
				if(dynamic_cast<noFighting*>(*it)->IsSoldierOfPlayer(player))
					return true;
			}
		}

		// Schiffe?
		
		if((*it)->GetGOT() == GOT_SHIP)
		{
			noShip * ship = dynamic_cast<noShip*>(*it);
			if(distance <= ship->GetVisualRange())
			{
				if(ship->GetX() == x && ship->GetY() == y && ship->GetPlayer() == player)
					return true;
			}
		}

	}

	return false;
}

void GameWorldGame::RecalcVisibility(const MapCoord x, const MapCoord y, const unsigned char player, const noBaseBuilding * const exception)
{
	///// Bei vÃ¶llig ausgeschalteten Nebel muss nur das erste Mal alles auf sichtbar gesetzt werden
	//if(GameClient::inst().GetGGS().exploration == GlobalGameSettings::EXP_DISABLED && !update_terrain)
	//	GetNode(x,y).fow[player].visibility = VIS_VISIBLE;
	//else if(GameClient::inst().GetGGS().exploration == GlobalGameSettings::EXP_DISABLED && update_terrain)
	//	return;

	/// Zustand davor merken
	Visibility visibility_before = GetNode(x,y).fow[player].visibility;

	/// Herausfinden, ob vollständig sichtbar
	bool visible = IsPointCompletelyVisible(x,y,player,exception);

	// Vollständig sichtbar --> vollständig sichtbar logischerweise
	if(visible)
	{
		GetNode(x,y).fow[player].visibility = VIS_VISIBLE;

		// Etwaige FOW-Objekte zerstÃ¶ren
		delete GetNode(x,y).fow[player].object;
		GetNode(x,y).fow[player].object = NULL;
	}
	else
	{
		// nicht mehr sichtbar
		// Je nach vorherigen Zustand und Einstellung entscheiden
		switch(GameClient::inst().GetGGS().exploration)
		{
		default: assert(false);
		case GlobalGameSettings::EXP_DISABLED:
		case GlobalGameSettings::EXP_CLASSIC:
			{
				// einmal sichtbare Bereiche bleiben erhalten
				// nichts zu tun
			} break;
		case GlobalGameSettings::EXP_FOGOFWAR:
		case GlobalGameSettings::EXP_FOGOFWARE_EXPLORED:
			{
				// wenn es mal sichtbar war, nun im Nebel des Krieges
				if(visibility_before == VIS_VISIBLE)
				{
					GetNode(x,y).fow[player].visibility = VIS_FOW;

					SaveFOWNode(x,y,player);
				}
			} break;
		}

	}

	// Minimap Bescheid sagen
	if(gi && visibility_before != GetNode(x,y).fow[player].visibility)
		gi->GI_UpdateMinimap(x,y);

	// Lokaler Spieler oder Verbündeter (wenn Team-Sicht an ist)? --> Terrain updaten
	if(player == GameClient::inst().GetPlayerID() || 
		(GameClient::inst().GetGGS().team_view && GameClient::inst().GetLocalPlayer()->IsAlly(player)))
		VisibilityChanged(x,y);
}

void GameWorldGame::SetVisibility(const MapCoord x, const MapCoord y,  const unsigned char player)
{
	Visibility visibility_before = GetNode(x,y).fow[player].visibility;
	GetNode(x,y).fow[player].visibility = VIS_VISIBLE;

	// Etwaige FOW-Objekte zerstÃ¶ren
	delete GetNode(x,y).fow[player].object;
	GetNode(x,y).fow[player].object = NULL;

	// Minimap Bescheid sagen
	if(gi && visibility_before != GetNode(x,y).fow[player].visibility)
		gi->GI_UpdateMinimap(x,y);

	// Lokaler Spieler oder Verbündeter (wenn Team-Sicht an ist)? --> Terrain updaten
	if(player == GameClient::inst().GetPlayerID() || 
		(GameClient::inst().GetGGS().team_view && GameClient::inst().GetLocalPlayer()->IsAlly(player)))
		VisibilityChanged(x,y);
}





void GameWorldGame::RecalcVisibilitiesAroundPoint(const MapCoord x, const MapCoord y, const MapCoord radius, const unsigned char player, const noBaseBuilding * const exception)
{
	RecalcVisibility(x,y,player,exception);

	for(MapCoord tx=GetXA(x,y,0), r=1;r<=radius;tx=GetXA(tx,y,0),++r)
	{
		MapCoord tx2 = tx, ty2 = y;
		for(unsigned i = 2;i<8;++i)
		{
			for(MapCoord r2=0;r2<r;GetPointA(tx2,ty2,i%6),++r2)
				RecalcVisibility(tx2,ty2,player,exception);
		}
	}
}

/// Setzt die Sichtbarkeiten um einen Punkt auf sichtbar (aus Performancegründen Alternative zu oberem)
void GameWorldGame::SetVisibilitiesAroundPoint(const MapCoord x, const MapCoord y, const MapCoord radius, const unsigned char player)
{
	SetVisibility(x,y,player);

	for(MapCoord tx=GetXA(x,y,0), r=1;r<=radius;tx=GetXA(tx,y,0),++r)
	{
		MapCoord tx2 = tx, ty2 = y;
		for(unsigned i = 2;i<8;++i)
		{
			for(MapCoord r2=0;r2<r;GetPointA(tx2,ty2,i%6),++r2)
				SetVisibility(tx2,ty2,player);
		}
	}
}

/// Bestimmt bei der Bewegung eines spähenden Objekts die Sichtbarkeiten an
/// den Rändern neu
void GameWorldGame::RecalcMovingVisibilities(const MapCoord x, const MapCoord y, const unsigned char player, const MapCoord radius, 
const unsigned char moving_dir, Point<MapCoord> * enemy_territory)
{
	// Neue Sichtbarkeiten zuerst setzen
	// Zum Eckpunkt der beiden neuen sichtbaren Kanten gehen
	MapCoord tx = x, ty = y;
	for(MapCoord i = 0;i<radius;++i)
		this->GetPointA(tx,ty,moving_dir);
		
	// Und zu beiden Abzweigungen weiter gehen und Punkte auf visible setzen
	SetVisibility(tx,ty,player);
	MapCoord ttx = tx, tty = ty;
	unsigned char dir = (moving_dir+2)%6;
	for(MapCoord i = 0;i<radius;++i)
	{
		this->GetPointA(ttx,tty,dir);
		// Sichtbarkeit und für FOW-Gebiet vorherigen Besitzer merken
		// (d.h. der dort  zuletzt war, als es für Spieler player sichtbar war)
		Visibility old_vis = GetNode(ttx,tty).fow[player].visibility;
		unsigned char old_owner = GetNode(ttx,tty).fow[player].owner;
		SetVisibility(ttx,tty,player);
		// Neues feindliches Gebiet entdeckt?
		// Muss vorher undaufgedeckt oder FOW gewesen sein, aber in dem Fall darf dort vorher noch kein 
		// Territorium entdeckt worden sein
		unsigned char current_owner = GetNode(ttx,tty).owner;
		if(current_owner && (old_vis == VIS_INVISIBLE ||
			(old_vis == VIS_FOW && old_owner != current_owner)))
		{
			if(GameClient::inst().GetPlayer(player)->IsPlayerAttackable(current_owner) && enemy_territory)
			{
				enemy_territory->x = ttx;
				enemy_territory->y = tty;
			}
		}
	}
	
	ttx = tx;
	tty = ty;
	dir = (moving_dir+6-2)%6;
	for(MapCoord i = 0;i<radius;++i)
	{
		this->GetPointA(ttx,tty,dir);
		// Sichtbarkeit und für FOW-Gebiet vorherigen Besitzer merken
		// (d.h. der dort  zuletzt war, als es für Spieler player sichtbar war)
		Visibility old_vis = GetNode(ttx,tty).fow[player].visibility;
		unsigned char old_owner = GetNode(ttx,tty).fow[player].owner;
		SetVisibility(ttx,tty,player);
		// Neues feindliches Gebiet entdeckt?
		// Muss vorher undaufgedeckt oder FOW gewesen sein, aber in dem Fall darf dort vorher noch kein 
		// Territorium entdeckt worden sein
		unsigned char current_owner = GetNode(ttx,tty).owner;
		if(current_owner && (old_vis == VIS_INVISIBLE ||
			(old_vis == VIS_FOW && old_owner != current_owner)))
		{
			if(GameClient::inst().GetPlayer(player)->IsPlayerAttackable(current_owner) && enemy_territory)
			{
				enemy_territory->x = ttx;
				enemy_territory->y = tty;
			}
		}
	}
	
	// Dasselbe für die zurückgebliebenen Punkte
	// Diese müssen allerdings neu berechnet werden!
	tx = x;
	ty = y;
	unsigned char anti_moving_dir = (moving_dir+3)%6;
	for(MapCoord i = 0;i<radius+1;++i)
		this->GetPointA(tx,ty,anti_moving_dir);
		
	RecalcVisibility(tx,ty,player,NULL);
	ttx = tx;
	tty = ty;
	dir = (anti_moving_dir+2)%6;
	for(MapCoord i = 0;i<radius;++i)
	{
		this->GetPointA(ttx,tty,dir);
		RecalcVisibility(ttx,tty,player,NULL);
	}
	
	ttx = tx;
	tty = ty;
	dir = (anti_moving_dir+6-2)%6;
	for(unsigned i = 0;i<radius;++i)
	{
		this->GetPointA(ttx,tty,dir);
		RecalcVisibility(ttx,tty,player,NULL);
	}
		
}


void GameWorldGame::SaveFOWNode(const MapCoord x, const MapCoord y, const unsigned player)
{
	GetNode(x,y).fow[player].last_update_time = GameClient::inst().GetGFNumber();
	
	// FOW-Objekt erzeugen
	noBase * obj = GetNO(x,y);
	delete GetNode(x,y).fow[player].object;
	GetNode(x,y).fow[player].object = obj->CreateFOWObject();


	// Wege speichern, aber nur richtige, keine, die gerade gebaut werden
	for(unsigned i = 0;i<3;++i)
	{
		if(GetNode(x,y).roads_real[i])
			GetNode(x,y).fow[player].roads[i] = GetNode(x,y).roads[i];
		else
			GetNode(x,y).fow[player].roads[i] = 0;
	}

	// Besitzverhältnisse speichern, damit auch die Grenzsteine im FoW gezeichnet werden kÃ¶nnen
	GetNode(x,y).fow[player].owner = GetNode(x,y).owner;
	// Grenzsteine merken
	for(unsigned i =0;i<4;++i)
		GetNode(x,y).fow[player].boundary_stones[i] = GetNode(x,y).boundary_stones[i];
}

/// Stellt fest, ob auf diesem Punkt ein Grenzstein steht (ob das Grenzgebiet ist)
bool GameWorldGame::IsBorderNode(const MapCoord x, const MapCoord y, const unsigned char player) const
{
	// Wenn ich Besitzer des Punktes bin, dieser mir aber nicht gehÃ¶rt
	return (GetNode(x,y).owner == player && !IsPlayerTerritory(x,y));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Konvertiert Ressourcen zwischen Typen hin und her oder lÃ¶scht sie.
 *  Für Spiele ohne Gold.
 *
 *  @author Divan
 */
void GameWorldGame::ConvertMineResourceTypes(unsigned char from, unsigned char to)
{
	// to == 0xFF heiÃt lÃ¶schen
	// in Map-Resource-Koordinaten konvertieren
	from = RESOURCES_MINE_TO_MAP[from];
	to = ((to != 0xFF) ? RESOURCES_MINE_TO_MAP[to] : 0xFF);

	// Zeiger auf zu veränderte Daten
	unsigned char *resources;
	
	//LOG.lprintf("Convert map resources from %i to %i\n", from, to);
	// Alle Punkte durchgehen
	for (unsigned short x = 0; x < width; ++x)	
	for (unsigned short y = 0; y < height; ++y)
	{
		resources = &(GetNode(x,y).resources);
		// Gibt es Ressourcen dieses Typs?
		// Wenn ja, dann umwandeln bzw lÃ¶schen
		if (*resources >= 0x40+from*8 && *resources < 0x48+from*8)
			*resources -= ((to != 0xFF) ?  from*8-to*8 : *resources);
	}
}

/// Prüfen, ob zu einem bestimmten Küsenpunkt ein Hafenpunkt gehÃ¶rt und wenn ja, wird dieser zurückgegeben
unsigned short GameWorldGame::GetHarborPosID(const MapCoord x, const MapCoord y)
{
	for(unsigned d = 0;d<6;++d)
	{
		for(unsigned i = 1;i<harbor_pos.size();++i)
		{
			if(harbor_pos[i].x == GetXA(x,y,d) && harbor_pos[i].y == GetYA(x,y,d))
			{
				return i;
			}
		}
	}
	
	return 0;
}

// Struktur für die Breitensuche zum Finden der jeweiligen Hafenpos-Nachbarn
struct DepthFirstSearchNode
{
	// Richtige Position des Punktes
	Point<int> real_pos;
	// Vorgänger
	Point<MapCoord> pre;
	// Entfernung bis hierhin
	unsigned way;
	// Von welcher Richtung wir gekommen sind
	unsigned char dir;
};

/// Bestimmt die Schifffahrtrichtung, in der ein Punkt relativ zu einem anderen liegt 
unsigned char GameWorldGame::GetShipDir(Point<int> pos1, Point<int> pos2)
{
	// Richtung bestimmen, in der dieser Punkt relativ zum Ausgangspunkt steht
	unsigned char exp_dir = 0xff;
				
	unsigned diff = SafeDiff<int>(pos1.y,pos2.y);
	if(!diff)
		diff = 1;
	// Oben?
	bool marginal_x = ((SafeDiff<int>(pos1.x,pos2.x) * 1000 / diff) < 180);
	if(pos2.y < pos1.y)
	{
		if(marginal_x)
			exp_dir = 0;
		else if(pos2.x < pos1.x)
			exp_dir = 5;
		else
			exp_dir = 1;
	}
	else
	{
		if(marginal_x)
			exp_dir = 3;
		else if(pos2.x < pos1.x)
			exp_dir = 4;
		else
			exp_dir = 2;
	}
	
	return exp_dir;
	
}

/// Berechnet für alle Hafenpunkt jeweils die Richtung und Entfernung zu allen anderen Hafenpunkten
/// über die Kartenränder hinweg
void GameWorldGame::CalcHarborPosNeighbors()
{
	std::vector<DepthFirstSearchNode> nodes(width*height);
	
	for(unsigned i = 1;i<harbor_pos.size();++i)
	{
		// Häfen, die gefunden wurden
		std::vector<bool> visited_harbor_pos(harbor_pos.size(),false);
		// Den Ausgangshafen als gefunden setzen, damit wir diesen nicht als Nachbar ansehen
		visited_harbor_pos[i] = true;
		// Breitensuche von diesem Punkt aus durchführen
		std::vector<bool> visited(width*height,false);
		std::queue< Point<MapCoord> > todo;

		Point<MapCoord> start(harbor_pos[i].x,harbor_pos[i].y);
		todo.push(start);
		nodes[start.y*width+start.x].way = 0;
		nodes[start.y*width+start.x].real_pos = Point<int>(start.x,start.y);

		// Knoten zählen (Startknoten schon mit inbegriffen)
		//unsigned count = 0; (war unused - nÃ¶tig für irgendwas? - jh)
		
		// Am Anfang müssen wir auch Küstenpunkte zulassen
		bool coastal_points = true;

		while(!todo.empty())
		{
			Point<MapCoord> p = todo.front();
			todo.pop();

			if(visited[p.y*width+p.x])
				continue;

			for(unsigned char d = 0;d<6;++d)
			{
				MapCoord xa,ya;
				xa = GetXA(p.x,p.y,d);
				ya = GetYA(p.x,p.y,d);

				// Ist das dort auch ein Meerespunkt?
				if(!IsSeaPoint(xa,ya) && !(coastal_points && IsCoastalPoint(xa,ya)))
					continue;

				if(!visited[ya*width+xa])
				{
					Point<MapCoord> add(xa,ya);
					todo.push(add);
					nodes[ya*width+xa].pre = p;
					nodes[ya*width+xa].way = nodes[p.y*width+p.x].way+1;
					nodes[ya*width+xa].real_pos = GetPointAround(nodes[p.y*width+p.x].real_pos,d);
					nodes[ya*width+xa].dir = d;
					visited[p.y*width+p.x] = true;
					
					// Umliegende Punkte prüfen, ob sie Küstenpunkte sind
					for(unsigned dd = 0;dd<6;++dd)
					{
						MapCoord x = GetXA(xa,ya,dd), y = GetYA(xa,ya,dd);
						
						if(IsCoastalPoint(x,y))
						{
							// Dann wieder prüfen, ob der Küstenpunkt auch zu einem Hafenpunkt gehÃ¶rt
							unsigned short harbor_pos_id;
							if((harbor_pos_id = GetHarborPosID(x,y)))
							{
								// Prüfen, ob es dieser Hafen nicht schon einmal besucht wurde
								if(!visited_harbor_pos[harbor_pos_id])
								{
									// Dann tragen wir ihn als neuen Nachbarn ein
									unsigned char dir = GetShipDir(Point<int>(start.x,start.y),GetPointAround(nodes[ya*width+xa].real_pos,dd));

								/*	// Weg zurückverfolgen
									Point<MapCoord> current_pos(xa,ya);
									std::vector<unsigned char> path(nodes[p.y*width+p.x].way+1);

									path[nodes[p.y*width+p.x].way] = dd;
									for(unsigned z = nodes[p.y*width+p.x].way;z>0;--z)
									{
										path[z-1] = nodes[current_pos.y*width+current_pos.x].dir;
										GetPointA(current_pos.x,current_pos.y,(path[z-1]+3)%6);
									}*/

									harbor_pos[i].neighbors[dir].push_back(HarborPos::Neighbor(harbor_pos_id,nodes[ya*width+xa].way+1));
									visited_harbor_pos[harbor_pos_id] = true;
									
								}
							}
						}
						
					}
				}
			}

			coastal_points = false;
		}
	}
}


/// Gründet vom Schiff aus eine neue Kolonie 
bool GameWorldGame::FoundColony(const unsigned harbor_point, const unsigned char player, const unsigned short sea_id)
{
	// Ist es hier überhaupt noch mÃ¶glich, eine Kolonie zu gründen?
	if(!IsHarborPointFree(harbor_point,player,sea_id))
		return false;

	Point<MapCoord> pos(GetHarborPoint(harbor_point));

	// Hafenbaustelle errichten
	noBuildingSite * bs = new noBuildingSite(pos.x,pos.y,player);
	SetNO(bs,pos.x,pos.y);
	AddHarborBuildingSiteFromSea(bs);

	RecalcTerritory(bs,HARBOR_ALONE_RADIUS,false,true);
	
	// BQ neu berechnen (evtl durch RecalcTerritory noch nicht geschehen)
	RecalcBQAroundPointBig(pos.x,pos.y);

	return true;
}

/// Gibt zurück, ob eine bestimmte Baustellen eine Baustelle ist, die vom Schiff aus errichtet wurde
bool GameWorldGame::IsHarborBuildingSiteFromSea(const noBuildingSite * building_site) const
{
	return (std::find(harbor_building_sites_from_sea.begin(),
	harbor_building_sites_from_sea.end(),building_site) != harbor_building_sites_from_sea.end());
}


/// Liefert eine Liste der Hafenpunkte, die von einem bestimmten Hafenpunkt erreichbar sind
void GameWorldGame::GetHarborPointsWithinReach(const unsigned hp,std::vector<unsigned>& hps) const
{
	for(unsigned i = 1;i<harbor_pos.size();++i)
	{
		if(i == hp)
			continue;
		unsigned dist = CalcHarborDistance(hp,i);
		if(dist == 0xffffffff)
			continue;

		hps.push_back(i);
	}
}


