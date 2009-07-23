// $Id: GameWorld.h 5319 2009-07-23 09:59:18Z OLiver $
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

#ifndef OBJMANAGER_H_
#define OBJMANAGER_H_


#include "GameConsts.h"
#include "MapConsts.h"
#include "noNothing.h"
#include "RoadSegment.h"
#include "MilitaryConsts.h"
#include "EventManager.h"
#include "TerrainRenderer.h"
#include "main.h"
#include <vector>
#include "GamePlayerList.h"

class noEnvObject;
class noGranite;
class noTree;
class noFlag;
class noFigure;
class nobHQ;
class nobFarm;
class nobCatapult;
class ncFlag;
class noRoadNode;
class noCoordBase;
class nobBaseMilitary;
class SerializedGameData;
class CatapultStone;
class nofAttacker;
class MouseCoords;
class noBuilding;
class GameInterface;
class GameWorldBase;
class glArchivItem_Map;
class noShip;

struct RoadsBuilding;
class FOWObject;

typedef bool (*FP_Node_OK_Callback)(const GameWorldBase& gwb, const MapCoord x, const MapCoord y, const unsigned char dir, const void *param);

/// Eigenschaften von einem Punkt auf der Map
struct MapNode
{
	/// Stra�en
	unsigned char roads[3];
	bool roads_real[3];
	/// H�he
	unsigned char altitude;
	/// Schattierung
	unsigned char shadow;
	/// Terrain
	unsigned char t1,t2;
	/// Ressourcen
	unsigned char resources;
	/// Reservierungen
	bool reserved;
	/// Eigent�mer (Spieler)
	unsigned char owner;
	/// Grenzsteine (der Punkt, und dann jeweils nach rechts, unten-links und unten-rechts die Zwischensteine)
	unsigned char boundary_stones[4];
	/// Bauqualit�t
	BuildingQuality bq;
	/// Visuelle Sachen f�r alle Spieler, die in Zusammenhang mit dem FoW stehen
	struct
	{
		/// Sichtbarkeit des Punktes
		Visibility visibility;
		/// FOW-Objekt
		FOWObject * object;
		/// Stra�en im FoW
		unsigned char roads[3];
		/// Grenzsteine (der Punkt, und dann jeweils nach rechts, unten-links und unten-rechts die Zwischensteine)
		unsigned char owner;
		/// Grenzsteine (der Punkt, und dann jeweils nach rechts, unten-links und unten-rechts die Zwischensteine)
		unsigned char boundary_stones[4];
	} fow[MAX_PLAYERS];

	/// Meeres-ID, d.h. zu welchem Meer geh�rt dieser Punkt (0 = kein Meer)
	unsigned short sea_id;
	/// Hafenpunkt-ID (0 = kein Hafenpunkt)
	unsigned harbor_id;

	/// Objekt, welches sich dort befindet
	noBase * obj;
	/// Figuren, K�mpfe, die sich dort befinden
	list<noBase*> figures;
	
};

enum LandscapeType
{
	LT_GREENLAND = 0,
	LT_WASTELAND,
	LT_WINTERWORLD
};

/// Grundlegende Klasse, die die Gamewelt darstellt, enth�lt nur deren Daten
class GameWorldBase
{
protected:

	/// Interface zum GUI
	GameInterface * gi;

	/// Breite und H�he der Karte in Kontenpunkten
	unsigned short width,height;
	/// Landschafts-Typ
	LandscapeType lt;

	unsigned short * handled_nodes; /// Array von abgeklapperten Knoten und deren Punktzahlen
	list<CatapultStone*> catapult_stones; /// Liste von allen umherfliegenden Katapultsteinen.

	/// Eigenschaften von einem Punkt auf der Map
	MapNode *nodes;

	/// Rendert das Terrain
	TerrainRenderer tr;

	/// Informationen �ber die Weltmeere
	struct Sea
	{
		/// Anzahl der Knoten, welches sich in diesem Meer befinden
		unsigned nodes_count;

		Sea() : nodes_count(0) {}
		Sea(const unsigned nodes_count) : nodes_count(nodes_count) {}
	};
	std::vector<Sea> seas;

	/// Alle Hafenpositionen
	struct HarborPos
	{
		MapCoord x,y;
		struct Neighbor
		{
			unsigned id;
			unsigned distance;
			unsigned char group_id;
			
			bool operator<(const Neighbor& two) const
			{ return distance < two.distance; }
		};

		struct CoastalPoint
		{
			unsigned short sea_id;
			unsigned char cp_group;
		} cps[6];

		unsigned group_count;


		struct CP_Group
		{
			unsigned char founder_dir;
			std::vector<Neighbor> neighbors[6];
		} cp_group[6];

	};

	std::vector< HarborPos > harbor_pos;


public:
	unsigned int map_size;

	noNothing nothing; // nur Platzhalter bei der Rckgabe von GetNO
	/// Liste von Milit�rgeb�uden (auch HQ und Haufengeb�ude, daher normale Geb�ude) pro "Milit�rquadrat"
	list<nobBaseMilitary*> * military_squares;

public:

	GameWorldBase();
	virtual ~GameWorldBase();

	// Grundlegende Initialisierungen
	void Init();
	/// Aufr�umen
	void Unload();

	/// Setzt GameInterface
	void SetGameInterface(GameInterface * const gi) { this->gi = gi; }

	/// Gr��e der Map abfragen
	unsigned short GetWidth() const { return width; }
	unsigned short GetHeight() const { return height; }

	/// Landschaftstyp abfragen
	LandscapeType GetLandscapeType() const { return lt; }

	/// Gibt Punkt um diesen Punkt (X-Koordinate) direkt zur�ck in einer Richtung von 0-5
	MapCoord GetXA(const MapCoord x, const MapCoord y, unsigned dir) const;
	/// Gibt Punkt um diesen Punkt (Y-Koordinate)  direkt zur�ck in einer Richtung von 0-5
	MapCoord GetYA(const MapCoord x, const MapCoord y, unsigned dir) const;
	/// Wie GetXA, blo� 2. Au�enschale (dir zwischen 0 bis 11)
	MapCoord GetXA2(const MapCoord x, const MapCoord y, unsigned dir) const;
	/// Wie GetYA, blo� 2. Au�enschale (dir zwischen 0 bis 11)
	MapCoord GetYA2(const MapCoord x, const MapCoord y, unsigned dir) const;
	/// Wandelt einen Punkt in einen Nachbarpunkt um
	void GetPointA(MapCoord& x, MapCoord& y, unsigned dir) const;
	/// Berechnet die Differenz zweier Koordinaten von x1 zu x2, wenn man ber�cksichtigt, dass man �ber den 
	/// Rand weitergehen kann
	MapCoord CalcDistanceAroundBorderX(const MapCoord x1, const MapCoord x2) const;
	MapCoord CalcDistanceAroundBorderY(const MapCoord y1, const MapCoord y2) const;

	// Erzeugt eindeutige ID aus gegebenen X und Y-Werten
	unsigned MakeCoordID(const MapCoord x, const MapCoord y) const
	{ return y*width+x; }

	/// Gibt Map-Knotenpunkt zur�ck
	const MapNode& GetNode(const MapCoord x, const MapCoord y) const { assert(x<width && y<height);  return nodes[y*width+x]; }
	MapNode& GetNode(const MapCoord x, const MapCoord y) { assert(x<width && y<height); return nodes[y*width+x]; }
	/// Gibt MapKnotenpunkt darum zur�ck
	const MapNode& GetNodeAround(const MapCoord x, const MapCoord y, const unsigned i) const
	{ return GetNode(GetXA(x,y,i),GetYA(x,y,i));  }
	MapNode& GetNodeAround(const MapCoord x, const MapCoord y, const unsigned i)
	{ return GetNode(GetXA(x,y,i),GetYA(x,y,i));  }

	// Gibt ein NO zur�ck, falls keins existiert, wird ein "Nothing-Objekt" zur�ckgegeben
	noBase * GetNO(const MapCoord x, const MapCoord y);
	// Gibt ein NO zur�ck, falls keins existiert, wird ein "Nothing-Objekt" zur�ckgegeben
	const noBase * GetNO(const MapCoord x, const MapCoord y) const;
	/// Gibt ein FOW-Objekt zur�ck, falls keins existiert, wird ein "Nothing-Objekt" zur�ckgegeben
	const FOWObject * GetFOWObject(const MapCoord x, const MapCoord y, const unsigned spectator_player) const;
	/// Gibt den GOT des an diesem Punkt befindlichen Objekts zur�ck bzw. GOT_NOTHING, wenn keins existiert
	GO_Type GetGOT(const MapCoord x, const MapCoord y) const;

	/// Gibt Figuren, die sich auf einem bestimmten Punkt befinden, zur�ck
	/// nicht bei laufenden Figuren oder
	list<noBase*>& GetFigures(const MapCoord x, const MapCoord y) const { return nodes[y*width+x].figures; }
	/// Gibt Dynamische Objekte, die von einem bestimmten Punkt aus laufen oder dort stehen sowie andere Objekte,
	/// die sich dort befinden, zur�ck
	void GetDynamicObjectsFrom(const MapCoord x, const MapCoord y,list<noBase*>& objects) const;

	// Gibt ein spezifisches Objekt zur�ck
	template<typename T> T * GetSpecObj(MapCoord x, MapCoord y) { return dynamic_cast<T*>( GetNode(x,y).obj ); }
		// Gibt ein spezifisches Objekt zur�ck
	template<typename T> const T * GetSpecObj(MapCoord x, MapCoord y) const { return dynamic_cast<const T*>( GetNode(x,y).obj ); }

	/// Gibt ein Terrain-Dreieck um einen Punkt herum zur�ck.
	unsigned char GetTerrainAround(int x, int y, unsigned char dir) const;
	/// Gibt das Terrain zur�ck, �ber das ein Mensch/Tier laufen m�sste, von X,Y in Richtung DIR (Vorw�rts).
	unsigned char GetWalkingTerrain1(MapCoord x, MapCoord y, unsigned char dir) const;
	/// Gibt das Terrain zur�ck, �ber das ein Mensch/Tier laufen m�sste, von X,Y in Richtung DIR (R�ckw�rts).
	unsigned char GetWalkingTerrain2(MapCoord x, MapCoord y, unsigned char dir) const;
	/// Gibt zur�ck, ob ein Punkt vollst�ndig von Wasser umgeben ist
	bool IsSeaPoint(MapCoord x, MapCoord y) const;

	/// liefert den Stra�en-Wert an der Stelle X,Y
	unsigned char GetRoad(const MapCoord x, const MapCoord y, unsigned char dir, bool all = false) const;
	/// liefert den Stra�en-Wert um den Punkt X,Y.
	unsigned char GetPointRoad(const MapCoord x, const MapCoord y, unsigned char dir, bool all = false) const;
	/// liefert FOW-Stra�en-Wert um den punkt X,Y
	unsigned char GetPointFOWRoad(MapCoord x, MapCoord y, unsigned char dir, const unsigned char viewing_player) const;

	/// Kann dorthin eine Stra�e gebaut werden?
	bool RoadAvailable(const bool boat_road,const int x, const int y,unsigned char to_dir,const bool visual = true) const;
	/// Bauqualit�ten berechnen, bei flagonly gibt er nur 1 zur�ck, wenn eine Flagge m�glich ist
	BuildingQuality CalcBQ(const MapCoord x, const MapCoord y,const unsigned char player,const bool flagonly = false,const bool visual = true) const;
	/// Setzt die errechnete BQ gleich mit
	void SetBQ(const MapCoord x, const MapCoord y,const unsigned char player,const bool flagonly = false,const bool visual = true)
	{ GetNode(x,y).bq = CalcBQ(x,y,player,flagonly,visual); }

	/// Pr�ft, ob der Pkut zu dem Spieler geh�rt (wenn er der Besitzer ist und es false zur�ckliefert, ist es Grenzgebiet)
	bool IsPlayerTerritory(const MapCoord x, const MapCoord y) const;
	/// Berechnet BQ bei einer gebauten Stra�e
	void CalcRoad(const int x, const int y,const unsigned char player);
	/// Ist eine Flagge irgendwo um x,y ?
	bool FlagNear(const int x, const int y) const;
	/// Pr�ft, ob sich in unmittelbarer N�he (im Radius von 4) Milit�rgeb�ude befinden
	bool IsMilitaryBuildingNearNode(const int nx, const int ny) const;

	/// setzt den virtuellen Stra�en-Wert an der Stelle X,Y (berichtigt).
	void SetVirtualRoad(const MapCoord x, const MapCoord y, unsigned char dir, unsigned char type);
	/// setzt den virtuellen Stra�en-Wert um den Punkt X,Y.
	void SetPointVirtualRoad(const MapCoord x, const MapCoord y, unsigned char dir, unsigned char type);

	/// Test, ob auf dem besagten Punkt ein Milit�rgeb�ude steht
	bool IsMilitaryBuilding(const MapCoord x, const MapCoord y) const;

	/// Erstellt eine Liste mit allen Milit�rgeb�uden in der Umgebung, radius bestimmt wie viele K�stchen nach einer Richtung im Umkreis
	void LookForMilitaryBuildings(list<nobBaseMilitary*>& buildings,const MapCoord x, const MapCoord y, const unsigned short radius) const;

	/// Pr�ft, ob von einem bestimmten Punkt aus der Untergrund f�r Figuren zug�nglich ist (kein Wasser,Lava,Sumpf)
	bool IsNodeToNodeForFigure(const MapCoord x, const MapCoord y, const unsigned dir) const;

	/// Informationen, ob Grenzen �berquert wurden
	struct CrossBorders
	{ bool left, top, right, bottom; };


	/// Wegfindung in freiem Terrain - Basisroutine
	bool FindFreePath(const MapCoord x_start,const MapCoord y_start,
				  const MapCoord x_dest, const MapCoord y_dest, const bool random_route, const unsigned max_route, 
				  std::vector<unsigned char> * route, unsigned *length, unsigned char * first_dir, CrossBorders* cb,
				  FP_Node_OK_Callback IsNodeOK, FP_Node_OK_Callback IsNodeToDestOk, const void * param) const;
	/// Ermittelt, ob eine freie Route noch passierbar ist und gibt den Endpunkt der Route zur�ck
	bool CheckFreeRoute(const MapCoord x_start,const MapCoord y_start, const std::vector<unsigned char>& route,
		const unsigned pos, FP_Node_OK_Callback IsNodeOK, FP_Node_OK_Callback IsNodeToDestOk,
		 MapCoord* x_dest,  MapCoord* y_dest, const void * const param = NULL);
	/// Wegfindung auf Stra�en - Basisroutine
	bool FindPathOnRoads(const noRoadNode * const start, const noRoadNode * const goal,
									const bool ware_mode, std::vector<unsigned char> * route, unsigned * length,
									unsigned char * first_dir, const RoadSegment * const forbidden) const;
	/// Findet einen Weg f�r Figuren
	unsigned char FindHumanPath(const MapCoord x_start,const MapCoord y_start,
		  const MapCoord x_dest, const MapCoord y_dest, const unsigned max_route = 0xFFFFFFFF, const bool random_route = false, unsigned *length = NULL);
	/// Wegfindung f�r Schiffe auf dem Wasser
	bool FindShipPath(const MapCoord x_start,const MapCoord y_start, const MapCoord x_dest, const MapCoord y_dest, std::vector<unsigned char> * route, unsigned * length, const unsigned max_length = 200,
								 CrossBorders * cb = NULL);


	/// Baut eine (bisher noch visuell gebaute) Stra�e wieder zur�ck
	void RemoveVisualRoad(unsigned short start_x, unsigned short start_y, const std::vector<unsigned char>& route);

	/// x,y ist ein Punkt auf irgendeinem Wegstck, gibt die Flagge zur�ck
	noFlag * GetRoadFlag(int x, int y,unsigned char& dir,unsigned last_i=255);

	/// Konvertiert die Koordinaten.
	void ConvertCoords(int x, int y, unsigned short * x_out, unsigned short * y_out) const;

	/// Erzeugt eine GUI-ID f�r die Fenster von Map-Objekten
	unsigned CreateGUIID(const MapCoord x, const MapCoord y) const
	{ return 1000 + width*y+x; }
	/// Gibt Terrainkoordinaten zur�ck
	float GetTerrainX(const MapCoord x, const MapCoord y)
	{ return tr.GetTerrainX(x,y); }
	float GetTerrainY(const MapCoord x, const MapCoord y)
	{ return tr.GetTerrainY(x,y); }

	/// Ver�ndert die H�he eines Punktes und die damit verbundenen Schatten
	void ChangeAltitude(const MapCoord x, const MapCoord y, const unsigned char altitude);
	
	/// Ermittelt Sichtbarkeit eines Punktes auch unter Einbeziehung der Verb�ndeten des jeweiligen Spielers
	Visibility CalcWithAllyVisiblity(const MapCoord x, const MapCoord y, const unsigned char player) const; 

	/// Ist es an dieser Stelle f�r einen Spieler m�glich einen Hafen zu bauen
	bool IsHarborPointFree(const unsigned harbor_id, const unsigned char player, 
		const unsigned short sea_id) const;
	/// Gibt die Koordinaten eines bestimmten Hafenpunktes zur�ck
	Point<MapCoord> GetHarborPoint(const unsigned harbor_id) const;
	/// Gibt die ID eines Hafenpunktes zur�ck
	unsigned GetHarborPointID(const MapCoord x, const MapCoord y) const
	{ return GetNode(x,y).harbor_id; }
	/// Ermittelt, ob ein Punkt K�stenpunkt ist, d.h. Zugang zu einem schiffbaren Meer hat 
	/// und gibt ggf. die Meeres-ID zur�ck, ansonsten 0
	unsigned short IsCoastalPoint(const MapCoord x, const MapCoord y) const;
		/// Grenzt der Hafen an ein bestimmtes Meer an?
	bool IsAtThisSea(const unsigned harbor_id, const unsigned short sea_id) const;
	/// Gibt den Punkt eines bestimmtes Meeres um den Hafen herum an, sodass Schiffe diesen anfahren k�nnen
	void GetCoastalPoint(const unsigned harbor_id, MapCoord * px, MapCoord * py, const unsigned short sea_id) const;
	/// Sucht freie Hafenpunkte, also wo noch ein Hafen gebaut werden kann
	unsigned GetNextFreeHarborPoint(const MapCoord x, const MapCoord y, const unsigned origin_harbor_id, const unsigned char dir,
										   const unsigned char player) const;
	

	void SetPlayers(GameClientPlayerList *pls) { players = pls; }
	/// Liefert einen Player zur�ck
	inline GameClientPlayer * GetPlayer(const unsigned int id) const { return players->getElement(id); }

protected:

	/// Berechnet die Schattierung eines Punktes neu
	void RecalcShadow(const MapCoord x, const MapCoord y);

	/// F�r abgeleitete Klasse, die dann das Terrain entsprechend neu generieren kann
	virtual void AltitudeChanged(const MapCoord x, const MapCoord y) = 0;
	/// F�r abgeleitete Klasse, die dann das Terrain entsprechend neu generieren kann
	virtual void VisibilityChanged(const MapCoord x, const MapCoord y) = 0;

	/// Gibt n�chsten Hafenpunkt in einer bestimmten Richtung zur�ck, bzw. 0, wenn es keinen gibt 
	unsigned GetNextHarborPoint(const MapCoord x, const MapCoord y, const unsigned origin_harbor_id, const unsigned char dir,
										   const unsigned char player, bool (GameWorldBase::*IsPointOK)(const unsigned, const unsigned char, const unsigned short) const) const;

private:
	GameClientPlayerList *players;

};

/// "Interface-Klasse" f�r GameWorldBase, die die Daten grafisch anzeigt
class GameWorldViewer : public virtual GameWorldBase
{
	bool show_bq;    ///< Bauqualit�ten-Anzeigen ein oder aus
	bool show_names; ///< Geb�udenamen-Anzeigen ein oder aus
	bool show_productivity; ///< Produktivit�t-Anzeigen ein oder aus

	/// Scrolling-Zeug
	int xoffset,yoffset;
	/// Erster gezeichneter Map-Punkt
	int fx,fy;
	/// Letzter gezeichneter Map-Punkt
	int lx,ly;
	/// Selektierter Punkt
	unsigned short selx,sely;
	int selxo,selyo;
	int sx,sy;
	/// Wird gerade gescrollt?
	bool scroll;
	/// Letzte Scrollposition, an der man war, bevor man weggesprungen ist
	int last_xoffset, last_yoffset;
	/// Koordinaten auf der Map anzeigen (zum Debuggen)?
	bool show_coordinates;

public:

	GameWorldViewer();

	// Wege und Grenzsteine zeichnen ( nur reingenommen, da die Position sowieso hier berechnet wird, da bietet es sich an )
	void DrawWays(const int x, const int y, const float xpos, const float ypos);
	unsigned char IsBoundaryStone(const int x, const int y);
	void DrawBoundaryStone(const int x, const int y, const MapCoord tx, const MapCoord ty, const int xpos, const int ypos, Visibility vis);

	/// Bauqualit�ten anzeigen oder nicht
	void ShowBQ() { show_bq = !show_bq; }
	/// Geb�udenamen zeigen oder nicht
	void ShowNames() { show_names = !show_names; }
	/// Produktivit�t zeigen oder nicht
	void ShowProductivity() { show_productivity = !show_productivity; };
	/// Schaltet Produktivit�ten/Namen komplett aus oder an
	void ShowNamesAndProductivity();

	/// Wegfinden ( A* ) --> Wegfindung auf allgemeinen Terrain ( ohne Stra�cn ) ( fr Wegebau oder frei herumlaufende )
	bool FindRoadPath(const MapCoord x_start,const MapCoord y_start, const MapCoord x_dest, const MapCoord y_dest,std::vector<unsigned char>& route, const bool boat_road);
	/// Sucht die Anzahl der verf�gbaren Soldaten, um das Milit�rgeb�ude an diesem Punkt anzugreifen
	unsigned GetAvailableSoldiersForAttack(const unsigned char player_attacker,const MapCoord x, const MapCoord y);
	/// Zeichnet die Objekte
	void Draw(const unsigned char player, unsigned * water, const bool draw_selected, const MapCoord selected_x, const MapCoord selected_y,const RoadsBuilding& rb);

	/// Scrolling-Zeug
	void MouseMove(const MouseCoords& mc);
	void MouseDown(const MouseCoords& mc);
	/// Bewegt sich zu einer bestimmten Position in Pixeln auf der Karte
	void MoveTo(int x, int y, bool absolute = false);
	/// Zentriert den Bildschirm auf ein bestimmtes Map-Object
	void MoveToMapObject(const MapCoord x, const MapCoord y);
	/// Springt zur letzten Position, bevor man "weggesprungen" ist
	void MoveToLastPosition();

	void MoveToX(int x, bool absolute = false) { MoveTo( (absolute ? 0 : xoffset) + x, yoffset, true); }
	void MoveToY(int y, bool absolute = false) { MoveTo( xoffset, (absolute ? 0 : yoffset) + y, true); }
	void MouseUp();
	void DontScroll() { scroll = false; }
	void CalcFxLx();

	/// Koordinatenanzeige ein/aus
	void ShowCoordinates() { show_coordinates = !show_coordinates; }

	/// Gibt selektierten Punkt zur�ck
	unsigned short GetSelX() const { return selx; }
	unsigned short GetSelY() const { return sely; }

	int GetSelXo() const { return selxo; }
	int GetSelYo() const { return selyo; }

	/// Gibt Scrolling-Offset zur�ck
	int GetXOffset() const { return xoffset; }
	int GetYOffset() const { return yoffset; }
	/// Gibt ersten Punkt an, der beim Zeichnen angezeigt wird
	int GetFirstX() const { return fx; }
	int GetFirstY() const { return fy; }
	/// Gibt letzten Punkt an, der beim Zeichnen angezeigt wird
	int GetLastX() const { return lx; }
	int GetLastY() const { return ly; }

	/// Ermittelt Sichtbarkeit eines Punktes f�r den lokalen Spieler, ber�cksichtigt ggf. Teamkameraden
	Visibility GetVisibility(const MapCoord x, const MapCoord y) const; 

	/// H�he wurde ver�ndert: TerrainRenderer Bescheid sagen, damit es entsprechend ver�ndert werden kann
	void AltitudeChanged(const MapCoord x, const MapCoord y);
	/// Sichtbarkeit wurde ver�ndert: TerrainRenderer Bescheid sagen, damit es entsprechend ver�ndert werden kann
	void VisibilityChanged(const MapCoord x, const MapCoord y);

	/// Schattierungen (vor allem FoW) neu berechnen
	void RecalcAllColors();

	/// liefert sichtbare Stra�e, im Nebel entsprechend die FoW-Stra�e
	unsigned char GetVisibleRoad(const MapCoord x, const MapCoord y, unsigned char dir, const Visibility visibility) const;

	/// Gibt das erste Schiff, was gefunden wird von diesem Spieler, zur�ck, ansonsten NULL, falls es nicht
	/// existiert
	noShip * GetShip(const MapCoord x, const MapCoord y, const unsigned char player) const;
};

/// "Interface-Klasse" f�r das Spiel
class GameWorldGame : public virtual GameWorldBase
{
private:

	/// vergleicht 2 Punkte, ob sie von unterschiedlichen Spielern sind und setzt
	/// Punkt ggf. zu gar keinem Spieler, 2. Funktion wird f�r Punkte im 2er Abstand verwendet, da es dort ein bisschen anders l�uft!
	void AdjustNodes(const MapCoord x1, const MapCoord y1, const MapCoord x2, const MapCoord y2);
	void AdjustNodes2(const MapCoord x1, const MapCoord y1, const MapCoord x2, const MapCoord y2);
	/// Zerst�rt Spielerteile auf einem Punkt, wenn der Punkt dem Spieler nun nich mehr geh�rt
	void DestroyPlayerRests(const MapCoord x, const MapCoord y, const unsigned char new_player,const nobBaseMilitary * exception);

	/// Pr�ft, ob auf diesem Punkt Deko-Objekte liegen, die f�r den Wegbau entfernt werden k�nnen
	bool IsObjectionableForRoad(const MapCoord x, const MapCoord y);

	
	/// Punkt vollst�ndig sichtbar?
	bool IsPointCompletelyVisible(const MapCoord x, const MapCoord y, const unsigned char player, const noBuilding * const exception) const;
	/// Pr�ft, ob auf angegebenen Punkt sich ein Sp�her von player befindet
	bool IsScoutingFigureOnNode(const MapCoord x, const MapCoord y, const unsigned player, const unsigned distance) const;
	/// Berechnet die Sichtbarkeit eines Punktes neu f�r den angegebenen Spieler
	/// exception ist ein Geb�ude (Sp�hturm, Milit�rgeb�ude), was nicht mit in die Berechnugn einbezogen
	/// werden soll, z.b. weil es abgerissen wird
	void RecalcVisibility(const MapCoord x, const MapCoord y, const unsigned char player, const noBuilding * const exception);
	/// Setzt Punkt auf jeden Fall auf sichtbar
	void SetVisibility(const MapCoord x, const MapCoord y,  const unsigned char player);

protected:

	/// Erzeugt FOW-Objekte, -Stra�en und -Grensteine von aktuellen Punkt f�r einen bestimmten Spieler
	void SaveFOWNode(const MapCoord x, const MapCoord y, const unsigned player);
	/// Berechnet f�r alle Hafenpunkt jeweils die Richtung und Entfernung zu allen anderen Hafenpunkten
	/// �ber die Kartenr�nder hinweg
	void CalcHarborPosNeighbors();


public:

	/// Stellt anderen Spielern/Spielobjekten das Game-GUI-Interface zur Verf�ung
	GameInterface * GetGameInterface() const { return gi; }

	void SetNO(noBase * obj, const MapCoord x, const MapCoord y) { GetNode(x,y).obj = obj; }
	void AddFigure(noBase * fig,const MapCoord x, const MapCoord y);
	void RemoveFigure(const noBase * fig,const MapCoord x, const MapCoord y);

	/// Berechnet Bauqualit�ten an Punkt x;y und den ersten Kreis darum neu
	void RecalcBQAroundPoint(const MapCoord x, const MapCoord y);
	/// Berechnet Bauqualit�ten wie bei letzterer Funktion, blo� noch den 2. Kreis um x;y herum
	void RecalcBQAroundPointBig(const MapCoord x, const MapCoord y);

	/// Pr�ft, ob dieser Punkt von Menschen betreten werden kann
	bool IsNodeForFigures(const MapCoord x, const MapCoord y);
	/// Kann dieser Punkt von auf Stra�en laufenden Menschen betreten werden? (K�mpfe!)
	bool IsRoadNodeForFigures(const MapCoord x, const MapCoord y,const unsigned char dir);
	/// L�sst alle Figuren, die auf diesen Punkt  auf Wegen zulaufen, anhalten auf dem Weg (wegen einem Kampf)
	void StopOnRoads(const MapCoord x, const MapCoord y, const unsigned char dir = 0xff);

	/// Sagt Bescheid, dass der Punkt wieder freigeworden ist und l�sst ggf. Figuren drumherum wieder weiterlaufen
	void RoadNodeAvailable(const MapCoord x, const MapCoord y);

	/// Flagge an x,y setzen, dis_dir ist der aus welche Richtung der Weg kommt, wenn man einen Weg mit Flagge baut
	/// kann ansonsten auf 255 gesetzt werden
	void SetFlag(const MapCoord x, const MapCoord y, const unsigned char player,const unsigned char dis_dir = 255);
	/// Flagge soll zerstr�rt werden
	void DestroyFlag(const MapCoord x, const MapCoord y);
	/// Baustelle setzen
	void SetBuildingSite(const BuildingType type,const MapCoord x, const MapCoord y, const unsigned char player);
	/// Geb�ude bzw Baustelle abrei�en
	void DestroyBuilding(const MapCoord x, const MapCoord y, const unsigned char playe);

	/// Wegfindung f�r Menschen im Stra�ennetz
	unsigned char FindHumanPathOnRoads(const noRoadNode * const start, const noRoadNode * const goal,unsigned * length = NULL,const RoadSegment * const forbidden = NULL);
	/// Wegfindung f�r Waren im Stra�ennetz
	unsigned char FindPathForWareOnRoads(const noRoadNode * const start, const noRoadNode * const goal,unsigned * length = NULL);
	/// Pr�ft, ob eine Schiffsroute noch G�ltigkeit hat
	bool CheckShipRoute(const MapCoord x_start,const MapCoord y_start, const std::vector<unsigned char>& route, const unsigned pos, 
		 MapCoord* x_dest,  MapCoord* y_dest);

	/// setzt den Stra�en-Wert an der Stelle X,Y (berichtigt).
	void SetRoad(const MapCoord x, const MapCoord y, unsigned char dir, unsigned char type);

	/// setzt den Stra�en-Wert um den Punkt X,Y.
	void SetPointRoad(const MapCoord x, const MapCoord y, unsigned char dir, unsigned char type);

	/// Funktionen aus ehemaligen Game
	/// Baut eine Stra�e ( nicht nur visuell, sondern auch wirklich )
	void BuildRoad(const unsigned char playerid,const bool boat_road,
		unsigned short start_x,unsigned short start_y, const std::vector<unsigned char>& route);
	/// Rei�t eine Stra�e ab
	void DestroyRoad(const MapCoord x, const MapCoord y, const unsigned char dir);

	/// Berechnet das Land in einem bestimmten Bereich (um ein neues, abgerissenes oder eingenommenes
	/// Milit�rgeb�ude rum) neu, destroyed gibt an, ob building abgerissen wurde und somit nicht einberechnet werden soll
	void RecalcTerritory(const nobBaseMilitary * const building,const unsigned short radius, const bool destroyed, const bool newBuilt);
	/// Greift ein Milit�rgeb�ude auf x,y an (entsendet daf�r die Soldaten etc.)
	void Attack(const unsigned char player_attacker, const MapCoord x, const MapCoord y, const unsigned short soldiers_count, const bool strong_soldiers);

	// Liefert das entsprechende Milit�rquadrat f�r einen bestimmten Punkt auf der Karte zur�ck (normale Koordinaten)
	list<nobBaseMilitary*>& GetMilitarySquare(const MapCoord x, const MapCoord y)
	{ return military_squares[(y/MILITARY_SQUARE_SIZE)*(width/MILITARY_SQUARE_SIZE+1)+x/MILITARY_SQUARE_SIZE]; }

	/// F�gt einen Katapultstein der Welt hinzu, der gezeichnt werden will
	void AddCatapultStone(CatapultStone * cs) {catapult_stones.push_back(cs); }
	void RemoveCatapultStone(CatapultStone * cs) {catapult_stones.erase(cs); }

	/// L�sst alles spielerische abbrennen, indem es alle Flaggen der Spieler zerst�rt
	void Armageddon();

  /// L�sst alles spielerische eines Spielers abbrennen, indem es alle Flaggen eines Spieler zerst�rt
	void Armageddon(const unsigned char player);

	/// Sagt der GW Bescheid, dass ein Objekt von Bedeutung an x,y vernichtet wurde, damit dieser
	/// dass ggf. an den WindowManager weiterleiten kann, damit auch ein Fenster wieder geschlossen wird
	virtual void ImportantObjectDestroyed(const MapCoord x, const MapCoord y) = 0;
	/// Sagt, dass ein Milit�rgeb�ude eingenommen wurde und ggf. ein entsprechender "Fanfarensound" abgespielt werden sollte
	virtual void MilitaryBuildingCaptured(const MapCoord x, const MapCoord y, const unsigned char player) = 0;

	/// Ist der Punkt ein geeigneter Platz zum Warten vor dem Milit�rgeb�ude
	bool ValidWaitingAroundBuildingPoint(const MapCoord x, const MapCoord y, nofAttacker * attacker);
	/// Geeigneter Punkt f�r K�mpfe?
	bool ValidPointForFighting(const MapCoord x, const MapCoord y);

	/// Berechnet die Sichtbarkeiten neu um einen Punkt mit radius
	void RecalcVisibilitiesAroundPoint(const MapCoord x, const MapCoord y, const MapCoord radius, const unsigned char player, const noBuilding * const exception);
	/// Setzt die Sichtbarkeiten um einen Punkt auf sichtbar (aus Performancegr�nden Alternative zu oberem)
	void SetVisibilitiesAroundPoint(const MapCoord x, const MapCoord y, const MapCoord radius, const unsigned char player);
	/// Berechet die ganzen Sichtbarkeiten der Karte neu
	void RecalcAllVisibilities();

	/// Stellt fest, ob auf diesem Punkt ein Grenzstein steht (ob das Grenzgebiet ist)
	bool IsBorderNode(const MapCoord x, const MapCoord y, const unsigned char player) const;
};


class GameWorld : public GameWorldViewer, public GameWorldGame
{
public:

	/// L�dt eine Karte
	bool LoadMap(const std::string& filename);

	/// Serialisiert den gesamten GameWorld
	void Serialize(SerializedGameData *sgd) const;
	void Deserialize(SerializedGameData *sgd);

	/// Sagt der GW Bescheid, dass ein Objekt von Bedeutung an x,y vernichtet wurde, damit dieser
	/// dass ggf. an den WindowManager weiterleiten kann, damit auch ein Fenster wieder geschlossen wird
	void ImportantObjectDestroyed(const MapCoord x, const MapCoord y);
	/// Sagt, dass ein Milit�rgeb�ude eingenommen wurde und ggf. ein entsprechender "Fanfarensound" abgespielt werden sollte
	void MilitaryBuildingCaptured(const MapCoord x, const MapCoord y, const unsigned char player);

private:
	/// Vermisst ein neues Weltmeer von einem Punkt aus, indem es alle mit diesem Punkt verbundenen
	/// Wasserpunkte mit der gleichen sea_id belegt und die Anzahl zur�ckgibt
	unsigned MeasureSea(const MapCoord x, const MapCoord y, const unsigned short sea_id);

	/// Erstellt Objekte anhand der ausgelesenen S2map
	void Scan(glArchivItem_Map *map);


};



#endif
