// $Id: Pathfinding.cpp 6458 2010-05-31 11:38:51Z FloSoft $
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
#include "Node.h"

#include "GameWorld.h"
#include "noRoadNode.h"
#include "VideoDriverWrapper.h"
#include "Random.h"
#include "MapGeometry.h"
#include "nobHarborBuilding.h"
#include "GameClient.h"

#include <set>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

/// Konstante für einen ungültigen Vorgänerknoten
const unsigned INVALID_PREV = 0xFFFFFFFF;

/// Vergleichsoperator für die Prioritätswarteschlange bzw. std::set beim straßengebundenen Wegfinden
class RoadNodeComperator
{
public:
	bool operator()(const noRoadNode* const rn1, const noRoadNode* const rn2) const;
};

struct PathfindingPoint;

/// Klass für einen Knoten mit dazugehörigen Informationen
/// Wir speichern einfach die gesamte Map und sparen uns so das dauernde Allokieren und Freigeben von Speicher
/// Die Knoten können im Array mit einer eindeutigen ID (gebildet aus y*Kartenbreite+x) identifiziert werden
struct NewNode
{
	NewNode() : way(0), count_nodes(0), dir(0), prev(INVALID_PREV), visited(false) {}

	/// Wegkosten, die vom Startpunkt bis zu diesem Knoten bestehen
	unsigned way;
	/// Anzahl der Knoten bis zu diesem Punkt, was sich von way unterscheiden kann bei der
	/// straßengebundenen Wegfindung
	unsigned count_nodes;
	/// Die Richtung, über die dieser Knoten erreicht wurde
	unsigned char dir;
	/// ID (gebildet aus y*Kartenbreite+x) des Vorgänngerknotens
	unsigned prev;
	/// Wurde Knoten schon besucht (für A*-Algorithmus)
	bool visited;
	/// Iterator auf Position in der Prioritätswarteschlange (std::set), freies Pathfinding
	std::set<PathfindingPoint>::iterator it_p;
	/// Iterator auf Position in der Prioritätswarteschlange (std::set), weggebundenes Pathfinding
	std::set<const noRoadNode*,RoadNodeComperator>::iterator it_rn;
};

/// Die Knoten der Map gespeichert, größtmöglichste Kartengröße nehmen
NewNode pf_nodes[256*256];

/// Punkte als Verweise auf die obengenannen Knoten, damit nur die beiden Koordinaten x,y im set mit rumgeschleppt
/// werden müsen
struct PathfindingPoint {
public:
	/// Die beiden Koordinaten des Punktes
	MapCoord x,y;

public:
	/// Konstruktoren
	PathfindingPoint() {};
	PathfindingPoint(const MapCoord x, const MapCoord y) : x(x), y(y) {}

	/// Koordinaten des Ziels beim jeweils aktuellen Pathfinding, die wir zur Bewertung der Punkte benötigen
	static MapCoord dst_x, dst_y;
	/// Pointer auf GameWorld, die wir brauchen, um die IDs zu berechnen bzw. die Kartengröße zu bekommen
	static const GameWorldBase * gwb;
	/// Diese statischen Variablen zu Beginn des Pathfindings festlegen
	static void Init(const MapCoord dst_x, const MapCoord dst_y,const GameWorldBase * gwb)
	{
		PathfindingPoint::dst_x = dst_x;
		PathfindingPoint::dst_y = dst_y;
		PathfindingPoint::gwb = gwb;
	}

	/// Operator für den Vergleich 
	bool operator<(const PathfindingPoint two) const
	{
		// Weglängen schätzen für beide Punkte, indem man den bisherigen Weg mit der Luftlinie vom aktullen 
		// Punkt zum Ziel addiert und auf diese Weise den kleinsten Weg auswählt
		unsigned way1 = pf_nodes[gwb->MakeCoordID(x,y)].way + gwb->CalcDistance(x,y,dst_x,dst_y);
		unsigned way2 = pf_nodes[gwb->MakeCoordID(two.x,two.y)].way + gwb->CalcDistance(two.x,two.y,dst_x,dst_y);

		// Wenn die Wegkosten gleich sind, vergleichen wir die Koordinaten, da wir für std::set eine streng
		// monoton steigende Folge brauchen
		if(way1 == way2)
			return (gwb->MakeCoordID(x,y) < gwb->MakeCoordID(two.x,two.y) );
		else
			return (way1<way2);
	}
};

// Vergleichsoperator für das straßengebundene Pathfinding, wird genauso wie das freie Pathfinding
// gehandelt, nur dass wir noRoadNodes statt direkt Points vergleichen
bool RoadNodeComperator::operator()(const noRoadNode* const rn1, const noRoadNode* const rn2) const
{
	PathfindingPoint p1 (rn1->GetX(), rn1->GetY()), p2(rn2->GetX(), rn2->GetY());
	return p1<p2;
}

/// Definitionen siehe oben
MapCoord PathfindingPoint::dst_x = 0;
MapCoord PathfindingPoint::dst_y = 0;
const GameWorldBase * PathfindingPoint::gwb = NULL;

/// Nach jedem Pathfinding sind Knoten der pf_nodes "verunreinigt", d.h. u.a. visited steht noch auf true
/// Dazu werden in dieser Liste die Knoten gespeichert, die vor jedem Pathfinding erstmal einmal wieder zurück-
/// gesetzt werden müssen
std::vector<unsigned> clean_list;


/// Wegfinden ( A* ), O(v lg v) --> Wegfindung auf allgemeinen Terrain (ohne Straßen), für Wegbau und frei herumlaufende Berufe
bool GameWorldBase::FindFreePath(const MapCoord x_start,const MapCoord y_start,
				  const MapCoord x_dest, const MapCoord y_dest, const bool random_route, 
				  const unsigned max_route, std::vector<unsigned char> * route, unsigned *length,
				  unsigned char * first_dir,  FP_Node_OK_Callback IsNodeOK, FP_Node_OK_Callback IsNodeToDestOk, const void * param) const
{
	//puts("");
	// Erst einmal wieder aufräumen
	for(unsigned i = 0;i<clean_list.size();++i)
		pf_nodes[clean_list[i]].visited = false;
	clean_list.clear();

	std::set<PathfindingPoint> todo;
	PathfindingPoint::Init(x_dest,y_dest,this);

	// Anfangsknoten einfügen
	unsigned start_id = MakeCoordID(x_start,y_start);
	std::pair< std::set<PathfindingPoint>::iterator, bool > ret = todo.insert(PathfindingPoint(x_start,y_start));
	// Und mit entsprechenden Werten füllen
	pf_nodes[start_id].it_p = ret.first;
	pf_nodes[start_id].prev = INVALID_PREV;
	pf_nodes[start_id].visited = true;
	clean_list.push_back(start_id);
	pf_nodes[start_id].way = 0;
	pf_nodes[start_id].dir = 0;

	while(todo.size())
	{
		// Knoten mit den geringsten Wegkosten auswählen
		PathfindingPoint best = *todo.begin();
		// Knoten behandelt --> raus aus der todo Liste
		todo.erase(todo.begin());
		
		//printf("x: %u y: %u\n",best.x,best.y);

		// ID des besten Punktes ausrechnen

		unsigned best_id = MakeCoordID(best.x,best.y);

		// Dieser Knoten wurde aus dem set entfernt, daher wird der entsprechende Iterator
		// auf das Ende (also nicht definiert) gesetzt, quasi als "NULL"-Ersatz
		pf_nodes[best_id].it_p = todo.end();

		// Ziel schon erreicht? Allerdings Null-Weg, wenn Start=Ende ist, verbieten
		if(x_dest == best.x && y_dest == best.y && pf_nodes[best_id].way)
		{
			// Ziel erreicht!
			// Jeweils die einzelnen Angaben zurückgeben, falls gewünscht (Pointer übergeben)
			if(length)
				*length = pf_nodes[best_id].way;
			if(route)
				route->resize(pf_nodes[best_id].way);

			// Route rekonstruieren und ggf. die erste Richtung speichern, falls gewünscht
			for(unsigned z = pf_nodes[best_id].way-1;best_id!=start_id;--z,best_id = pf_nodes[best_id].prev)
			{
				if(route)
					route->at(z) = pf_nodes[best_id].dir;
				if(first_dir && z == 0)
					*first_dir = pf_nodes[best_id].dir;
			}
				
			// Fertig, es wurde ein Pfad gefunden
			return true;
		}
		
		// Maximaler Weg schon erreicht? In dem Fall brauchen wir keine weiteren Knoten von diesem aus bilden
		if(pf_nodes[best_id].way == max_route)
			continue;

		// Bei Zufälliger Richtung anfangen (damit man nicht immer denselben Weg geht, besonders für die Soldaten wichtig)
		unsigned start = random_route?RANDOM.Rand("pf",__LINE__,y_start*GetWidth()+x_start,6):0;
		
		// Knoten in alle 6 Richtungen bilden
		for(unsigned z = start;z<start+6;++z)
		{
			unsigned i = (z+3)%6;

			// Koordinaten des entsprechenden umliegenden Punktes bilden
			MapCoord xa = GetXA(best.x,best.y,i),
				ya = GetYA(best.x,best.y,i);

			// ID des umliegenden Knotens bilden
			unsigned xaid = MakeCoordID(xa,ya);

			// Knoten schon auf dem Feld gebildet?
			if(pf_nodes[xaid].visited)
			{
				// Dann nur ggf. Weg und Vorgänger korrigieren, falls der Weg kürzer ist
				if(pf_nodes[xaid].it_p != todo.end() && pf_nodes[best_id].way+1 < pf_nodes[xaid].way)
				{
					pf_nodes[xaid].way  = pf_nodes[best_id].way+1;
					pf_nodes[xaid].prev = best_id;
					todo.erase(pf_nodes[xaid].it_p);
					ret = todo.insert(PathfindingPoint(xa,ya));
					pf_nodes[xaid].it_p = ret.first;
					pf_nodes[xaid].dir = i;
				}
				// Wir wollen nicht denselben Knoten noch einmal einfügen, daher Abbruch
				continue;
			}

			// Das Ziel wollen wir auf jedenfall erreichen lassen, daher nur diese zusätzlichen
			// Bedingungen, wenn es nicht das Ziel ist
			if(!(xa == x_dest && ya == y_dest) && IsNodeOK)
			{
				if(!IsNodeOK(*this,xa,ya,i,param))
					continue;
			}

			// Zusätzliche Bedingungen, auch die das letzte Stück zum Ziel betreffen
			if(IsNodeToDestOk)
			{
				if(!IsNodeToDestOk(*this,xa,ya,i,param))
					continue;
			}

			// Alles in Ordnung, Knoten kann gebildet werden
			pf_nodes[xaid].visited = true;
			clean_list.push_back(xaid);
			pf_nodes[xaid].way = pf_nodes[best_id].way+1;
			pf_nodes[xaid].dir = i;
			pf_nodes[xaid].prev = best_id;

			ret = todo.insert(PathfindingPoint(xa,ya));
			pf_nodes[xaid].it_p = ret.first;
		}
	}

	// Liste leer und kein Ziel erreicht --> kein Weg
	return false;
}

/// Wegfinden ( A* ), O(v lg v) --> Wegfindung auf allgemeinen Terrain (ohne Straßen), für Wegbau und frei herumlaufende Berufe
bool GameWorldBase::FindPathOnRoads(const noRoadNode * const start, const noRoadNode * const goal,
									const bool ware_mode, unsigned * length, 
									unsigned char * first_dir,  Point<MapCoord> * next_harbor,
									const RoadSegment * const forbidden) const
{
	// Aus Replay lesen?
	if(GameClient::inst().ArePathfindingResultsAvailable())
	{
		unsigned char dir;
		GameClient::inst().ReadPathfindingResult(&dir,length,next_harbor);
		if(first_dir) *first_dir = dir;
		return (dir != 0xff);
	}
			
	// Irgendwelche Null-Anfänge oder Ziele? --> Kein Weg
	if(!start || !goal)
	{
		GameClient::inst().AddPathfindingResult(0xff,length,next_harbor);
		return false;
	}

	// Erst einmal wieder aufräumen
	for(unsigned i = 0;i<clean_list.size();++i)
		pf_nodes[clean_list[i]].visited = false;
	clean_list.clear();

	std::set<const noRoadNode*,RoadNodeComperator> todo;
	PathfindingPoint::Init(goal->GetX(),goal->GetY(),this);

	// Anfangsknoten einfügen
	std::pair< std::set<const noRoadNode*, RoadNodeComperator>::iterator, bool > ret = todo.insert(start);
	unsigned start_id = MakeCoordID(start->GetX(),start->GetY());
	pf_nodes[start_id].it_rn = ret.first;
	pf_nodes[start_id].prev = INVALID_PREV;
	pf_nodes[start_id].visited = true;
	clean_list.push_back(start_id);
	pf_nodes[start_id].way = 0;
	pf_nodes[start_id].count_nodes = 0;
	pf_nodes[start_id].dir = 0;

	while(true)
	{
		// Liste leer und kein Ziel erreicht --> kein Weg
		if(!todo.size())
		{
			GameClient::inst().AddPathfindingResult(0xff,length,next_harbor);
			return false;
		}
		
		// Knoten mit den geringsten Wegkosten auswählen
		const noRoadNode* best = *todo.begin();
		// Knoten behandelt --> raus aus der todo Liste
		todo.erase(todo.begin());

		// ID des bisher besten Knotens bilden
		unsigned best_id = MakeCoordID(best->GetX(),best->GetY());

		// Knoten wurde entfernt, daher erstmal auf das Ende des set setzen (als Alternative zu NULL)
		pf_nodes[best_id].it_rn = todo.end();
		
		unsigned char first_dir_tmp = 0xff;

		// Ziel erreicht, allerdings keine Nullwege erlauben?
		if(best == goal &&  pf_nodes[best_id].way)
		{
			// Ziel erreicht!
			// Jeweils die einzelnen Angaben zurückgeben, falls gewünscht (Pointer übergeben)
			if(length)
				*length = pf_nodes[best_id].way;

			//if(route)
			//	route->resize(pf_nodes[best_id].count_nodes);
			
			// Route rekonstruieren und ggf. die erste Richtung speichern, falls gewünscht
			for(unsigned z = pf_nodes[best_id].count_nodes-1;best_id!=start_id;--z,best_id = pf_nodes[best_id].prev)
			{
				//if(route)
				//	route->at(z) = pf_nodes[best_id].dir;
				if(z == 0)
					first_dir_tmp = pf_nodes[best_id].dir;
				if(next_harbor && z == 0)
				{
					next_harbor->x = best_id%width;
					next_harbor->y = best_id/width;
				}
				
			}
			
			if(first_dir)
				*first_dir = first_dir_tmp;
				

			// Fertig, es wurde ein Pfad gefunden
			GameClient::inst().AddPathfindingResult(first_dir_tmp,length,next_harbor);
			return true;
		}
		
		// Nachbarflagge bzw. Wege in allen 6 Richtungen verfolgen
		for(unsigned i = 0;i<6;++i)
		{
			// Gibt es auch einen solchen Weg bzw. Nachbarflagge?
			const noRoadNode * rna = best->GetNeighbour(i);
			// Wenn nicht, brauchen wir mit dieser Richtung gar nicht weiter zu machen
			if(!rna)
				continue;
				
				
			// ID des umliegenden Knotens bilden
			unsigned xaid = MakeCoordID(rna->GetX(),rna->GetY());

			// Neuer Weg für diesen neuen Knoten berechnen
			unsigned new_way = pf_nodes[best_id].way  +best->routes[i]->GetLength();
			// Im Warenmodus müssen wir Strafpunkte für überlastete Träger hinzuaddieren,
			// damit der Algorithmus auch Ausweichrouten auswählt
			if(ware_mode)
				new_way += best->GetPunishmentPoints(i);

			// evtl verboten?
			if(best->routes[i] == forbidden)
				continue;

			// evtl Wasserstraße?
			if(best->routes[i]->GetRoadType() == RoadSegment::RT_BOAT && !ware_mode)
				continue;


			// Knoten schon auf dem Feld gebildet?
			if(pf_nodes[xaid].visited)
			{
				// Dann nur ggf. Weg und Vorgänger korrigieren, falls der Weg kürzer ist
				if(pf_nodes[xaid].it_rn != todo.end() && new_way < pf_nodes[xaid].way)
				{
					pf_nodes[xaid].way  = new_way;
					pf_nodes[xaid].prev = best_id;
					todo.erase(pf_nodes[xaid].it_rn);
					ret = todo.insert(rna);
					pf_nodes[xaid].it_rn = ret.first;
					pf_nodes[xaid].dir = i;
					pf_nodes[xaid].count_nodes = pf_nodes[best_id].count_nodes + 1;
				}
				continue;
			}

			// Alles in Ordnung, Knoten kann gebildet werden
			pf_nodes[xaid].visited = true;
			clean_list.push_back(xaid);
			pf_nodes[xaid].count_nodes = pf_nodes[best_id].count_nodes + 1;
			pf_nodes[xaid].way = new_way;
			pf_nodes[xaid].dir = i;
			pf_nodes[xaid].prev = best_id;

			ret = todo.insert(rna);
			pf_nodes[xaid].it_rn = ret.first;
		}

		// Stehen wir hier auf einem Hafenplatz
		if(best->GetGOT() == GOT_NOB_HARBORBUILDING)
		{
			std::vector<nobHarborBuilding::ShipConnection> scs;
			static_cast<const nobHarborBuilding*>(best)->GetShipConnections(scs);

			for(unsigned i = 0;i<scs.size();++i)
			{
				// ID des umliegenden Knotens bilden
				unsigned xaid = MakeCoordID(scs[i].dest->GetX(),scs[i].dest->GetY());
				// Neuer Weg für diesen neuen Knoten berechnen
				unsigned new_way = pf_nodes[best_id].way  + scs[i].way_costs;

				// Knoten schon auf dem Feld gebildet?
				if(pf_nodes[xaid].visited)
				{
					// Dann nur ggf. Weg und Vorgänger korrigieren, falls der Weg kürzer ist
					if(pf_nodes[xaid].it_rn != todo.end() && new_way < pf_nodes[xaid].way)
					{
						pf_nodes[xaid].way  = new_way;
						pf_nodes[xaid].prev = best_id;
						todo.erase(pf_nodes[xaid].it_rn);
						ret = todo.insert(scs[i].dest);
						pf_nodes[xaid].it_rn = ret.first;
						pf_nodes[xaid].dir = 100;
						pf_nodes[xaid].count_nodes = pf_nodes[best_id].count_nodes + 1;
					}
					continue;
				}

				// Alles in Ordnung, Knoten kann gebildet werden
				pf_nodes[xaid].visited = true;
				clean_list.push_back(xaid);
				pf_nodes[xaid].count_nodes = pf_nodes[best_id].count_nodes + 1;
				pf_nodes[xaid].way = new_way;
				pf_nodes[xaid].dir = 100;
				pf_nodes[xaid].prev = best_id;

				ret = todo.insert(scs[i].dest);
				pf_nodes[xaid].it_rn = ret.first;

			}
		}


	}
}



/// Ermittelt, ob eine freie Route noch passierbar ist und gibt den Endpunkt der Route zurück
bool GameWorldBase::CheckFreeRoute(const MapCoord x_start,const MapCoord y_start, const std::vector<unsigned char>& route, const unsigned pos, 
	FP_Node_OK_Callback IsNodeOK, FP_Node_OK_Callback IsNodeToDestOk,  MapCoord* x_dest, MapCoord* y_dest, const void * const param)
{
	MapCoord x = x_start, y = y_start;

	for(unsigned i = pos;i<route.size();++i)
	{
		GetPointA(x,y,route[i]);
		if(!IsNodeToDestOk(*this,x,y,route[i],param))
			return false;
		if(i < route.size()-1 && !IsNodeOK(*this,x,y,route[i],param))
			return false;
	}

	if(x_dest)
		*x_dest = x;
	if(y_dest)
		*y_dest = y;

	return true;
}

/// Paremter-Struktur für Straßenbaupathfinding
struct Param_RoadPath
{
	/// Straßenbaumodus erlaubt?
	bool boat_road;
};

/// Abbruch-Bedingungen für Straßenbau-Pathfinding
bool IsPointOK_RoadPath(const GameWorldBase& gwb, const MapCoord x, const MapCoord y, const unsigned char dir, const void *param)
{
	const Param_RoadPath * prp = static_cast<const Param_RoadPath*>(param);

	// Feld bebaubar?
	if(!gwb.RoadAvailable(prp->boat_road,x,y,dir))
		return false;
	// Auch auf unserem Territorium?
	if(!gwb.IsPlayerTerritory(x,y))
		return false;

	return true;
}

/// Straßenbau-Pathfinding
bool GameWorldViewer::FindRoadPath(const MapCoord x_start,const MapCoord y_start, const MapCoord x_dest, const MapCoord y_dest,std::vector<unsigned char>& route, const bool boat_road)
{
	Param_RoadPath prp = { boat_road };
	return FindFreePath(x_start,y_start,x_dest,y_dest,false,100,&route,NULL,NULL,IsPointOK_RoadPath,NULL, &prp);
}

/// Abbruch-Bedingungen für freien Pfad für Menschen
bool IsPointOK_HumanPath(const GameWorldBase& gwb, const MapCoord x, const MapCoord y, const unsigned char dir, const void *param)
{
	// Feld passierbar?
	noBase::BlockingManner bm = gwb.GetNO(x,y)->GetBM();
	if(bm != noBase::BM_NOTBLOCKING && bm != noBase::BM_TREE && bm != noBase::BM_FLAG)
		return false;

	if(gwb.GetNode(x,y).reserved)
		return false;

	return true;
}



/// Zusätzliche Abbruch-Bedingungen für freien Pfad für Menschen, die auch bei der letzen Kante
/// zum Ziel eingehalten werden müssen
bool IsPointToDestOK_HumanPath(const GameWorldBase& gwb, const MapCoord x, const MapCoord y, const unsigned char dir, const void *param)
{
	// Feld passierbar?
	// Nicht über Wasser, Lava, Sümpfe gehen
	if(!gwb.IsNodeToNodeForFigure(x,y,(dir+3)%6))
		return false;

	return true;
}

/// Abbruch-Bedingungen für freien Pfad für Schiffe
bool IsPointOK_ShipPath(const GameWorldBase& gwb, const MapCoord x, const MapCoord y, const unsigned char dir, const void *param)
{
	// Ein Meeresfeld?
	for(unsigned i = 0;i<6;++i)
	{
		if(gwb.GetTerrainAround(x,y,i) != TT_WATER)
			return false;
	}

	return true;
}

/// Zusätzliche Abbruch-Bedingungen für freien Pfad für Schiffe, die auch bei der letzen Kante
/// zum Ziel eingehalten werden müssen
bool IsPointToDestOK_ShipPath(const GameWorldBase& gwb, const MapCoord x, const MapCoord y, const unsigned char dir, const void *param)
{
	// Der Übergang muss immer aus Wasser sein zu beiden Seiten
	if(gwb.GetWalkingTerrain1(x,y,(dir+3)%6) == TT_WATER && gwb.GetWalkingTerrain2(x,y,(dir+3)%6) == TT_WATER)
		return true;
	else
		return false;
}

/// Findet einen Weg für Figuren
unsigned char GameWorldBase::FindHumanPath(const MapCoord x_start,const MapCoord y_start,
			const MapCoord x_dest, const MapCoord y_dest, const unsigned max_route, const bool random_route, unsigned *length)
{
	// Aus Replay lesen?
	if(GameClient::inst().ArePathfindingResultsAvailable())
	{
		unsigned char dir;
		GameClient::inst().ReadPathfindingResult(&dir,length,NULL);
		return dir;
	}
	
	unsigned char first_dir = 0xFF;
	FindFreePath(x_start,y_start,x_dest,y_dest,random_route,max_route,NULL,length,&first_dir,IsPointOK_HumanPath,
		IsPointToDestOK_HumanPath,NULL);
		
	GameClient::inst().AddPathfindingResult(first_dir,length,NULL);	
	
	return first_dir;

}

/// Wegfindung für Menschen im Straßennetz
unsigned char GameWorldGame::FindHumanPathOnRoads(const noRoadNode * const start, const noRoadNode * const goal,unsigned * length, Point<MapCoord> * next_harbor, const RoadSegment * const forbidden)
{
	unsigned char first_dir = 0xFF;
	if(FindPathOnRoads(start, goal, false, length, &first_dir, next_harbor, forbidden))
		return first_dir;
	else
		return 0xFF;
}

/// Wegfindung für Waren im Straßennetz
unsigned char GameWorldGame::FindPathForWareOnRoads(const noRoadNode * const start, const noRoadNode * const goal,unsigned * length, Point<MapCoord> * next_harbor)
{
	unsigned char first_dir = 0xFF;
	if(FindPathOnRoads(start, goal, true, length, &first_dir, next_harbor, NULL))
		return first_dir;
	else
		return 0xFF;
}


/// Wegfindung für Schiffe auf dem Wasser
bool GameWorldBase::FindShipPath(const MapCoord x_start,const MapCoord y_start, const MapCoord x_dest,
								 const MapCoord y_dest, std::vector<unsigned char> * route, unsigned * length, const unsigned max_length,
								 GameWorldBase::CrossBorders * cb)
{
	return FindFreePath(x_start,y_start,x_dest,y_dest,true,400,route,length,NULL,IsPointOK_ShipPath,
		IsPointToDestOK_ShipPath,NULL);
}

/// Prüft, ob eine Schiffsroute noch Gültigkeit hat
bool GameWorldGame::CheckShipRoute(const MapCoord x_start,const MapCoord y_start, const std::vector<unsigned char>& route, const unsigned pos, 
		 MapCoord* x_dest,  MapCoord* y_dest)
{
	return CheckFreeRoute(x_start,y_start,route,pos,IsPointOK_ShipPath,
		IsPointToDestOK_ShipPath,x_dest,y_dest,NULL);
}
