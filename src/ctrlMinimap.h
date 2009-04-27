// $Id: ctrlMinimap.h 4652 2009-03-29 10:10:02Z FloSoft $
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
#ifndef CTRL_MINIMAP_H_
#define CTRL_MINIMAP_H_

#include "ctrlRectangle.h"
#include "MapConsts.h"
#include "glArchivItem_Bitmap_Direct.h"

class Minimap;

/// �bersichtskarte (MapPreview)
class ctrlMinimap : public ctrlRectangle
{
protected:
	/// Gr��e der Anzeige der Minimap
	unsigned short width_show, height_show;
	/// Abstand der Minimap vom Rand des Controls
	unsigned short padding_x, padding_y;
public:

	ctrlMinimap( Window *parent, 
				 const unsigned int id, 
				 const unsigned short x, 
				 const unsigned short y, 
				 const unsigned short width, 
				 const unsigned short height,
				 const unsigned short padding_x,
				 const unsigned short padding_y,
				 const unsigned short map_width,
				 const unsigned short map_height);

	/// Gibt width_show und height_show zur�ck
	unsigned short GetWidthShow() const { return width_show; }
	unsigned short GetHeightShow() const { return height_show; }

	/// Gibt die entsprechenden Kanten relativ zur oberen linken Ecke der Bounding-Box
	unsigned short GetLeft() const { return padding_x+(width-width_show-2*padding_x)/2; }
	unsigned short GetTop() const { return padding_y+(height-height_show-2*padding_y)/2; }
	unsigned short GetRight() const { return GetLeft() + width_show + padding_x; }
	unsigned short GetBottom() const { return GetTop() + height_show + padding_y; }

	void SetDisplaySize(const unsigned short width, const unsigned short height, const unsigned short map_width,
		const unsigned short map_height);

	/// Liefert f�r einen gegebenen Map-Punkt die Pixel-Koordinaten relativ zur Bounding-Box
	unsigned short CalcMapCoordX(const unsigned short x, const unsigned short map_width) const 
	{ return GetLeft()+width_show*x/map_width; }
	unsigned short CalcMapCoordY(const unsigned short y, const unsigned short map_height) const 
	{ return GetTop()+height_show*y/map_height; }

	/// Verkleinert Minimap soweit es geht (entfernt Bounding-Box) in Y-Richtung und gibt neue H�he zur�ck
	void RemoveBoundingBox(const unsigned short width_min, const unsigned short height_min);

protected:

	/// Zeichnet die Minimap an sich
	void DrawMap(Minimap& map);

	///// Berechnet X-Koordinaten der rechten Seiten
	//unsigned short GetLeft() const { return x+minimap.GetLeft(); }
	///// Berechnet Y-Koordinate der unteren Kante
	//unsigned short GetBottom() const { return y+minimap.GetBottom(); }
};


#endif // !MapPreview_H_

