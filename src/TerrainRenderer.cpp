// $Id: TerrainRenderer.cpp 5098 2009-06-24 17:09:39Z FloSoft $
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
#include "TerrainRenderer.h"

#include "VideoDriverWrapper.h"
#include "MapConsts.h"
#include "GameWorld.h"
#include "Settings.h"
#include "GameClient.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

TerrainRenderer::TerrainRenderer() :
	vertices(NULL),
	gl_vertices(NULL), gl_texcoords(NULL), gl_colors(NULL),
	vbo_vertices(0), vbo_texcoords(0), vbo_colors(0),
	borders(NULL), border_count(0)
	{}

TerrainRenderer::~TerrainRenderer()
{
	if(SETTINGS.vbo)
	{
		glDeleteBuffersARB(1, (const GLuint*)&vbo_vertices);
		glDeleteBuffersARB(1, (const GLuint*)&vbo_texcoords);
		glDeleteBuffersARB(1, (const GLuint*)&vbo_colors);
	}

	delete[] vertices;

	delete[] gl_vertices;
	delete[] gl_texcoords;
	delete[] gl_colors;

	delete[] borders;
}

void GetPointAround(int& x, int& y,const unsigned dir)
{
	switch(dir)
	{
	case 0: x = x - 1; break;
	case 1: x = x - !(y&1); break;
	case 2: x = x + (y&1); break;
	case 3: x = x + 1; break;
	case 4: x = x + (y&1); break;
	case 5: x = x - !(y&1); break;
	}

	switch(dir)
	{
	default: break;
	case 1:
	case 2: --y; break;
	case 4:
	case 5: ++y; break;
	}
}

float TerrainRenderer::GetTerrainXAround(int x,  int y, const unsigned dir)
{
	unsigned short tx,ty;

	GetPointAround(x,y,dir);

	int xo,yo;
	ConvertCoords(x,y,tx,ty,&xo,&yo);

	return GetTerrainX(tx,ty)+xo;
}

float TerrainRenderer::GetTerrainYAround(int x,  int y, const unsigned dir)
{
	unsigned short tx,ty;
	
	GetPointAround(x,y,dir);

	int xo,yo;
	ConvertCoords(x,y,tx,ty,&xo,&yo);

	return GetTerrainY(tx,ty)+yo;
}

float TerrainRenderer::GetBXAround(int x, int y, const unsigned char triangle, const unsigned char dir)
{
	unsigned short tx,ty;

	GetPointAround(x,y,dir);

	int xo,yo;
	ConvertCoords(x,y,tx,ty,&xo,&yo);

	return GetBX(tx,ty,triangle)+xo;
}

float TerrainRenderer::GetBYAround(int x, int y, const unsigned char triangle, const unsigned char dir)
{
	unsigned short tx,ty;
	
	GetPointAround(x,y,dir);

	int xo,yo;
	ConvertCoords(x,y,tx,ty,&xo,&yo);

	return GetBY(tx,ty,triangle)+yo;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  erzeugt die Terrain-Vertices.
 *
 *  @author OLiver
 */
void TerrainRenderer::GenerateVertices(const GameWorldViewer * gwv)
{
	delete[] vertices;
	vertices = new Vertex[width * height];
	memset(vertices, 0, sizeof(Vertex) * width * height);

	// Terrain generieren
	for(unsigned short y = 0; y < height; ++y)
	{
		for(unsigned short x = 0; x < width; ++x)
		{
			UpdateVertexPos(x,y,gwv);
			UpdateVertexColor(x,y,gwv);
			UpdateVertexTerrain(x,y,gwv);
		}
	}

	// R�nder generieren
	for(unsigned short y = 0; y < height; ++y)
	{
		for(unsigned short x = 0; x < width; ++x)
		{
			UpdateBorderVertex(x,y,gwv);
		}
	}
}

/// erzeugt Vertex
void TerrainRenderer::UpdateVertexPos(const MapCoord x, const MapCoord y,const GameWorldViewer * gwv)
{
	GetVertex(x,y).pos.pos.x = float(x * TR_W + ( (y&1) ? TR_W/2 : 0) );
	GetVertex(x,y).pos.pos.y = float(y * TR_H - HEIGHT_FACTOR * gwv->GetNode(x,y).altitude + HEIGHT_FACTOR * 0x0A );
}


void TerrainRenderer::UpdateVertexColor(const MapCoord x, const MapCoord y,const GameWorldViewer * gwv)
{
	switch(gwv->GetVisibility(x,y))
	{
	// Unsichtbar -> schwarz
	case VIS_INVISIBLE: GetVertex(x,y).pos.color = 0.0f; break;
	// Fog of War -> abgedunkelt
	case VIS_FOW: GetVertex(x,y).pos.color = float(gwv->GetNode(x,y).shadow + 0x40) / float(0xFF) / 2; break;
	// Normal sichtbar
	case VIS_VISIBLE: GetVertex(x,y).pos.color = float(gwv->GetNode(x,y).shadow  + 0x40) / float(0xFF); break;
	}
}

void TerrainRenderer::UpdateVertexTerrain(const MapCoord x, const MapCoord y,const GameWorldViewer * gwv)
{
	if(gwv->GetNode(x,y).t1 < 20)
		GetVertex(x,y).terrain[0] = gwv->GetNode(x,y).t1;

	if(gwv->GetNode(x,y).t2 < 20)
		GetVertex(x,y).terrain[1] = gwv->GetNode(x,y).t2;
}

/// erzeugt Rand-Vertex
void TerrainRenderer::UpdateBorderVertex(const MapCoord x, const MapCoord y,const GameWorldViewer * gwv)
{
	/// @todo GetTerrainX und Co durch GetTerrainXA ausdr�cken
	GetVertex(x,y).border[0].pos.x = ( GetTerrainXAround(x,y,5) + GetTerrainX(x, y) + GetTerrainXAround(x,y,4) ) / 3.0f;
	GetVertex(x,y).border[0].pos.y = ( GetTerrainYAround(x,y,5) + GetTerrainY(x, y) + GetTerrainYAround(x,y,4) ) / 3.0f;
	GetVertex(x,y).border[0].color = ( GetColor(gwv->GetXA(x,y,5), gwv->GetYA(x,y,5)) + GetColor(x, y) + GetColor(gwv->GetXA(x,y,4), gwv->GetYA(x,y,4)) ) / 3.0f;

	GetVertex(x,y).border[1].pos.x = ( GetTerrainXAround(x,y,3) + GetTerrainX(x, y) + GetTerrainXAround(x,y,4) ) / 3.0f;
	GetVertex(x,y).border[1].pos.y = ( GetTerrainYAround(x,y,3) + GetTerrainY(x, y) + GetTerrainYAround(x,y,4) ) / 3.0f;
	GetVertex(x,y).border[1].color = ( GetColor(gwv->GetXA(x,y,3), gwv->GetYA(x,y,3)) + GetColor(x, y) + GetColor(gwv->GetXA(x,y,4), gwv->GetYA(x,y,4)) ) / 3.0f;

}

///////////////////////////////////////////////////////////////////////////////
/**
 *  erzeugt die OpenGL-Vertices.
 *
 *  @author OLiver
 *  @author FloSoft
 */
void TerrainRenderer::GenerateOpenGL(const GameWorldViewer * gwv)
{
	width = gwv->GetWidth();
	height = gwv->GetHeight();
	LandscapeType lt = gwv->GetLandscapeType();

	GenerateVertices(gwv);

	unsigned int offset = width * height * 2;

	// R�nder z�hlen
	borders = new Borders[width * height];
	for(unsigned short y = 0; y < height; ++y)
	{
		for(unsigned short x = 0; x < width; ++x)
		{
			unsigned char t1 = gwv->GetNode(x, y).t1;
			unsigned char t2 = gwv->GetNode(x, y).t2;
			unsigned int pos = y * width + x;

			if( (borders[pos].left_right[0]	= BORDER_TABLES[lt][t1][t2][1]) )
			{
				borders[pos].left_right_offset[0] = offset + border_count;
				++border_count;
			}
			if( (borders[pos].left_right[1] = BORDER_TABLES[lt][t1][t2][0]) )
			{
				borders[pos].left_right_offset[1] = offset + border_count;
				++border_count;
			}

			t1 = gwv->GetNodeAround(x, y,3).t1;
			if( (borders[pos].right_left[0]	= BORDER_TABLES[lt][t2][t1][1]) )
			{
				borders[pos].right_left_offset[0] = offset + border_count;
				++border_count;
			}
			if( (borders[pos].right_left[1] = BORDER_TABLES[lt][t2][t1][0]) )
			{
				borders[pos].right_left_offset[1] = offset + border_count;
				++border_count;
			}

			t1 = gwv->GetNode(x, y).t1;
			t2 = gwv->GetNodeAround(x,y,5).t2;
			if( (borders[pos].top_down[0] = BORDER_TABLES[lt][t1][t2][1]) )
			{
				borders[pos].top_down_offset[0] = offset + border_count;
				++border_count;
			}
			if( (borders[pos].top_down[1] = BORDER_TABLES[lt][t1][t2][0]) )
			{
				borders[pos].top_down_offset[1] = offset + border_count;
				++border_count;
			}
		}
	}

	gl_vertices = new Triangle[offset + border_count];
	gl_texcoords = new Triangle[offset + border_count];
	gl_colors = new ColorTriangle[offset + border_count];

	// Normales Terrain erzeugen
	for(unsigned short y = 0; y < height; ++y)
	{
		for(unsigned short x = 0; x < width; ++x)
		{
			UpdateTrianglePos(x,y,gwv,false);
			UpdateTriangleColor(x,y,gwv,false);
			UpdateTriangleTerrain(x,y,gwv,false);
		}
	}

	// R�nder erzeugen
	for(unsigned short y = 0; y < height; ++y)
	{
		for(unsigned short x = 0; x < width; ++x)
		{
			UpdateBorderTrianglePos(x,y,gwv,false);
			UpdateBorderTriangleColor(x,y,gwv,false);
			UpdateBorderTriangleTerrain(x,y,gwv,false);
		}
	}
	//unsigned buffer_size = (offset ) * 2 * 3 * sizeof(float);

	if(SETTINGS.vbo)
	{
		// Generiere und Binde den Vertex Buffer
		glGenBuffersARB(1, (GLuint*)&vbo_vertices);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_vertices);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, (offset + border_count) * 3 * 2 * sizeof(float), gl_vertices, GL_STATIC_DRAW_ARB);
		glVertexPointer(2, GL_FLOAT, 0, NULL);

		// Generiere und Binde den Textur Koordinaten Buffer
		glGenBuffersARB(1, (GLuint*)&vbo_texcoords);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,vbo_texcoords);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, (offset + border_count) * 3 * 2 * sizeof(float), gl_texcoords, GL_STATIC_DRAW_ARB );
		glTexCoordPointer(2, GL_FLOAT, 0, NULL);

		// Generiere und Binde den Color Buffer
		glGenBuffersARB(1, (GLuint*)&vbo_colors);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,vbo_colors);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, (offset + border_count) * 3 * 3 * sizeof(float), gl_colors, GL_STATIC_DRAW_ARB );
		glColorPointer(3, GL_FLOAT, 0, NULL);
	}
	else
	{
		glVertexPointer(2, GL_FLOAT, 0, gl_vertices);
		glTexCoordPointer(2, GL_FLOAT, 0, gl_texcoords);
		glColorPointer(3, GL_FLOAT, 0, gl_colors);
	}
}

/// Erzeugt fertiges Dreieick f�r OpenGL

void TerrainRenderer::UpdateTrianglePos(const MapCoord x, const MapCoord y,const GameWorldViewer * gwv, const bool update)
{
	unsigned int pos = 2 * width * y + x*2;

	gl_vertices[pos].pos[0].x = GetTerrainXAround(x,y,4);
	gl_vertices[pos].pos[0].y = GetTerrainYAround(x,y,4);
	gl_vertices[pos].pos[1].x = GetTerrainX(x, y);
	gl_vertices[pos].pos[1].y = GetTerrainY(x, y);
	gl_vertices[pos].pos[2].x = GetTerrainXAround(x,y,5);
	gl_vertices[pos].pos[2].y = GetTerrainYAround(x,y,5);

	++pos;

	gl_vertices[pos].pos[0].x = GetTerrainX(x, y);
	gl_vertices[pos].pos[0].y = GetTerrainY(x, y);
	gl_vertices[pos].pos[1].x = GetTerrainXAround(x,y,4);
	gl_vertices[pos].pos[1].y = GetTerrainYAround(x,y,4);
	gl_vertices[pos].pos[2].x = GetTerrainXAround(x,y,3);
	gl_vertices[pos].pos[2].y = GetTerrainYAround(x,y,3);

	if(update && SETTINGS.vbo)
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_vertices);
		glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(pos-1) * 3 * 2 * sizeof(float), 
			2 * 3 * 2 * sizeof(float), &gl_vertices[pos-1]);
	}
}

void TerrainRenderer::UpdateTriangleColor(const MapCoord x, const MapCoord y,const GameWorldViewer * gwv, const bool update)
{
	unsigned int pos = 2 * width * y + x*2;

	gl_colors[pos].colors[0].r = gl_colors[pos].colors[0].g = gl_colors[pos].colors[0].b = GetColor(gwv->GetXA(x,y,4), gwv->GetYA(x,y,4));
	gl_colors[pos].colors[1].r = gl_colors[pos].colors[1].g = gl_colors[pos].colors[1].b = GetColor(x, y);
	gl_colors[pos].colors[2].r = gl_colors[pos].colors[2].g = gl_colors[pos].colors[2].b = GetColor(gwv->GetXA(x,y,5), gwv->GetYA(x,y,5));

	++pos;

	gl_colors[pos].colors[0].r = gl_colors[pos].colors[0].g = gl_colors[pos].colors[0].b = GetColor(x, y);
	gl_colors[pos].colors[1].r = gl_colors[pos].colors[1].g = gl_colors[pos].colors[1].b = GetColor(gwv->GetXA(x,y,4), gwv->GetYA(x,y,4));
	gl_colors[pos].colors[2].r = gl_colors[pos].colors[2].g = gl_colors[pos].colors[2].b = GetColor(gwv->GetXA(x,y,3), gwv->GetYA(x,y,3));

	
	/// Bei Vertexbuffern das die Daten aktualisieren
	if(update && SETTINGS.vbo)
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_colors);
		glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(pos-1) * 3 * 3 * sizeof(float), 
			2 * 3 * 3 * sizeof(float), &gl_colors[pos-1]);
	}
}

void TerrainRenderer::UpdateTriangleTerrain(const MapCoord x, const MapCoord y,const GameWorldViewer * gwv, const bool update)
{
	unsigned int pos = 2 * width * y + x*2;

	unsigned char t1 = gwv->GetNode(x, y).t1;
	gl_texcoords[pos].pos[0].x = (t1 == 14 || t1 == 15) ? 0.4375f   : 0.45f;
	gl_texcoords[pos].pos[0].y = (t1 == 14 || t1 == 15) ? 0.0f      : 0.45f;
	gl_texcoords[pos].pos[1].y = (t1 == 14 || t1 == 15) ? 0.445312f : 0.0f;
	gl_texcoords[pos].pos[1].x = (t1 == 14 || t1 == 15) ? 0.0f      : 0.225f;
	gl_texcoords[pos].pos[2].x = (t1 == 14 || t1 == 15) ? 0.84375f  : 0.0f;
	gl_texcoords[pos].pos[2].y = (t1 == 14 || t1 == 15) ? 0.445312f : 0.45f;

	++pos;

	unsigned char t2 = gwv->GetNode(x, y).t2;
	gl_texcoords[pos].pos[0].x = (t2 == 14 || t2 == 15) ? 0.4375f   : 0.0f;
	gl_texcoords[pos].pos[0].y = (t2 == 14 || t2 == 15) ? 0.859375f : 0.0f;
	gl_texcoords[pos].pos[1].x = (t2 == 14 || t2 == 15) ? 0.84375f  : 0.235f;
	gl_texcoords[pos].pos[1].y = (t2 == 14 || t2 == 15) ? 0.445312f : 0.45f;
	gl_texcoords[pos].pos[2].x = (t2 == 14 || t2 == 15) ? 0.0f      : 0.47f;
	gl_texcoords[pos].pos[2].y = (t2 == 14 || t2 == 15) ? 0.445312f : 0.0f;

	
	/// Bei Vertexbuffern das die Daten aktualisieren
	if(update && SETTINGS.vbo)
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_texcoords);
		glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,(pos-1) * 3 * 2 * sizeof(float), 
			2 * 3 * 2 * sizeof(float), &gl_texcoords[pos-1]);
	}
}

/// Erzeugt die Dreiecke f�r die R�nder
void TerrainRenderer::UpdateBorderTrianglePos(const MapCoord x, const MapCoord y,const GameWorldViewer * gwv, const bool update)
{
	unsigned int pos = y * width + x;
	
	// F�r VBO-Aktualisierung:
	// Erzeugte R�nder z�hlen
	unsigned count_borders = 0;
	// Erstes Offset merken
	unsigned first_offset = 0;


	// Rand	links - rechts
	for(unsigned char i = 0; i < 2; ++i)
	{
		if(borders[pos].left_right[i])
		{
			unsigned int offset = borders[pos].left_right_offset[i];

			if(!first_offset)
				first_offset = offset;

			gl_vertices[offset].pos[i ? 0 : 2].x = GetTerrainX(x, y);
			gl_vertices[offset].pos[i ? 0 : 2].y = GetTerrainY(x, y);
			gl_vertices[offset].pos[1        ].x = GetTerrainXAround(x,y,4);
			gl_vertices[offset].pos[1        ].y = GetTerrainYAround(x,y,4);
			gl_vertices[offset].pos[i ? 2 : 0].x = GetBX(x, y, i);
			gl_vertices[offset].pos[i ? 2 : 0].y = GetBY(x, y, i);

			++count_borders;
		}
	}

	// Rand rechts - links
	for(unsigned char i = 0; i < 2; ++i)
	{
		if(borders[pos].right_left[i])
		{
			unsigned int offset = borders[pos].right_left_offset[i];

			if(!first_offset)
				first_offset = offset;

			gl_vertices[offset].pos[i ? 2 : 0].x = GetTerrainXAround(x,y,4);
			gl_vertices[offset].pos[i ? 2 : 0].y = GetTerrainYAround(x,y,4);
			gl_vertices[offset].pos[1        ].x = GetTerrainXAround(x,y,3);
			gl_vertices[offset].pos[1        ].y = GetTerrainYAround(x,y,3);

			if(i == 0)
			{
				gl_vertices[offset].pos[2].x = GetBX(x, y, 1);
				gl_vertices[offset].pos[2].y = GetBY(x, y, 1);
			}
			else
			{
				gl_vertices[offset].pos[0].x = GetBXAround(x, y, 0, 3);
				gl_vertices[offset].pos[0].y = GetBYAround(x, y, 0, 3);
			}

			++count_borders;
		}
	}

	// Rand oben - unten
	for(unsigned char i = 0; i < 2; ++i)
	{
		if(borders[pos].top_down[i])
		{
			unsigned int offset = borders[pos].top_down_offset[i];

			if(!first_offset)
				first_offset = offset;

			gl_vertices[offset].pos[i ? 2 : 0].x = GetTerrainXAround(x,y,5);
			gl_vertices[offset].pos[i ? 2 : 0].y = GetTerrainYAround(x,y,5);
			gl_vertices[offset].pos[1        ].x = GetTerrainXAround(x,y,4);
			gl_vertices[offset].pos[1        ].y = GetTerrainYAround(x,y,4);

			if(i == 0)
			{
				gl_vertices[offset].pos[2].x = GetBX(x,y,i);
				gl_vertices[offset].pos[2].y = GetBY(x,y,i);
			}
			else
			{
				//x - i + i * rt, y + i, i
				gl_vertices[offset].pos[0].x = GetBXAround(x, y, i, 5);
				gl_vertices[offset].pos[0].y = GetBYAround(x, y, i, 5);
			}

			++count_borders;
		}
	}

	/// Bei Vertexbuffern das die Daten aktualisieren
	if(update && SETTINGS.vbo)
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_vertices);
		glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,first_offset * 3 * 2 * sizeof(float), 
			count_borders * 3 * 2  * sizeof(float), &gl_vertices[first_offset]);
	}
}

void TerrainRenderer::UpdateBorderTriangleColor(const MapCoord x, const MapCoord y,const GameWorldViewer * gwv, const bool update)
{
	unsigned int pos = y * width + x;
	
	// F�r VBO-Aktualisierung:
	// Erzeugte R�nder z�hlen
	unsigned count_borders = 0;
	// Erstes Offset merken
	unsigned first_offset = 0;


	// Rand	links - rechts
	for(unsigned char i = 0; i < 2; ++i)
	{
		if(borders[pos].left_right[i])
		{
			unsigned int offset = borders[pos].left_right_offset[i];

			if(!first_offset)
				first_offset = offset;

			gl_colors[offset].colors[i ? 0 : 2].r = gl_colors[offset].colors[i ? 0 : 2].g = gl_colors[offset].colors[i ? 0 : 2].b = GetColor(x, y);
			gl_colors[offset].colors[1        ].r = gl_colors[offset].colors[1        ].g = gl_colors[offset].colors[1        ].b = GetColor(gwv->GetXA(x,y,4), gwv->GetYA(x,y,4));
			gl_colors[offset].colors[i ? 2 : 0].r = gl_colors[offset].colors[i ? 2 : 0].g = gl_colors[offset].colors[i ? 2 : 0].b = GetBColor(x, y, i);

			++count_borders;
		}
	}

	// Rand rechts - links
	for(unsigned char i = 0; i < 2; ++i)
	{
		if(borders[pos].right_left[i])
		{
			unsigned int offset = borders[pos].right_left_offset[i];

			if(!first_offset)
				first_offset = offset;

			gl_colors[offset].colors[i ? 2 : 0].r = gl_colors[offset].colors[i ? 2 : 0].g = gl_colors[offset].colors[i ? 2 : 0].b = GetColor(gwv->GetXA(x,y,4), gwv->GetYA(x,y,4));
			gl_colors[offset].colors[1        ].r = gl_colors[offset].colors[1        ].g = gl_colors[offset].colors[1        ].b = GetColor(gwv->GetXA(x,y,3), gwv->GetYA(x,y,3));
			gl_colors[offset].colors[i ? 0 : 2].r = gl_colors[offset].colors[i ? 0 : 2].g = gl_colors[offset].colors[i ? 0 : 2].b = GetBColor(x + i, y, i ? 0 : 1);

			++count_borders;
		}
	}

	// Rand oben - unten
	for(unsigned char i = 0; i < 2; ++i)
	{
		if(borders[pos].top_down[i])
		{
			unsigned int offset = borders[pos].top_down_offset[i];

			if(!first_offset)
				first_offset = offset;

			gl_colors[offset].colors[i ? 2 : 0].r = gl_colors[offset].colors[i ? 2 : 0].g = gl_colors[offset].colors[i ? 2 : 0].b = GetColor(gwv->GetXA(x,y,5), gwv->GetYA(x,y,5));
			gl_colors[offset].colors[1        ].r = gl_colors[offset].colors[1        ].g = gl_colors[offset].colors[1        ].b = GetColor(gwv->GetXA(x,y,4), gwv->GetYA(x,y,4));

			if(i == 0)
				gl_colors[offset].colors[2].r = gl_colors[offset].colors[2].g = gl_colors[offset].colors[2].b = GetBColor(x, y, i);
			else
				gl_colors[offset].colors[0].r = gl_colors[offset].colors[0].g = gl_colors[offset].colors[0].b = GetBColor(gwv->GetXA(x,y,5), gwv->GetYA(x,y,5),i);

			++count_borders;
		}
	}

	/// Bei Vertexbuffern das die Daten aktualisieren
	if(update && SETTINGS.vbo)
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_colors);
		glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,first_offset * 3 * 3 * sizeof(float), 
			count_borders * 3 * 3 * sizeof(float), &gl_colors[first_offset]);
	}
}

void TerrainRenderer::UpdateBorderTriangleTerrain(const MapCoord x, const MapCoord y,const GameWorldViewer * gwv, const bool update)
{
	unsigned int pos = y * width + x;
	
	// F�r VBO-Aktualisierung:
	// Erzeugte R�nder z�hlen
	unsigned count_borders = 0;
	// Erstes Offset merken
	unsigned first_offset = 0;


	// Rand	links - rechts
	for(unsigned char i = 0; i < 2; ++i)
	{
		if(borders[pos].left_right[i])
		{
			unsigned int offset = borders[pos].left_right_offset[i];

			if(!first_offset)
				first_offset = offset;

			gl_texcoords[offset].pos[i ? 0 : 2].x = 0.0f;
			gl_texcoords[offset].pos[i ? 0 : 2].y = 0.0f;
			gl_texcoords[offset].pos[1        ].x = 1.0f;
			gl_texcoords[offset].pos[1        ].y = 0.0f;
			gl_texcoords[offset].pos[i ? 2 : 0].x = 0.5f;
			gl_texcoords[offset].pos[i ? 2 : 0].y = 1.0f;

			++count_borders;
		}
	}

	// Rand rechts - links
	for(unsigned char i = 0; i < 2; ++i)
	{
		if(borders[pos].right_left[i])
		{
			unsigned int offset = borders[pos].right_left_offset[i];

			if(!first_offset)
				first_offset = offset;

			gl_texcoords[offset].pos[i ? 2 : 0].x = 0.0f;
			gl_texcoords[offset].pos[i ? 2 : 0].y = 0.0f;
			gl_texcoords[offset].pos[1        ].x = 1.0f;
			gl_texcoords[offset].pos[1        ].y = 0.0f;
			gl_texcoords[offset].pos[i ? 0 : 2].x = 0.5f;
			gl_texcoords[offset].pos[i ? 0 : 2].y = 1.0f;

			++count_borders;
		}
	}

	// Rand oben - unten
	for(unsigned char i = 0; i < 2; ++i)
	{
		if(borders[pos].top_down[i])
		{
			unsigned int offset = borders[pos].top_down_offset[i];

			if(!first_offset)
				first_offset = offset;

			gl_texcoords[offset].pos[i ? 2 : 0].x = 0.0f;
			gl_texcoords[offset].pos[i ? 2 : 0].y = 0.0f;
			gl_texcoords[offset].pos[1        ].x = 1.0f;
			gl_texcoords[offset].pos[1        ].y = 0.0f;
			gl_texcoords[offset].pos[i ? 0 : 2].x = 0.5f;
			gl_texcoords[offset].pos[i ? 0 : 2].y = 1.0f;

			++count_borders;
		}
	}

	/// Bei Vertexbuffern das die Daten aktualisieren
	if(update && SETTINGS.vbo)
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_texcoords);
		glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,first_offset * 3 * 2 * sizeof(float), 
			count_borders * 3 * 2  * sizeof(float), &gl_texcoords[first_offset]);
	}
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  zeichnet den Kartenausschnitt.
 *
 *  @author OLiver
 *  @author FloSoft
 */
void TerrainRenderer::Draw(const GameWorldViewer * gwv,unsigned int *water)
{
	assert(gl_vertices);
	assert(borders);

	// nach Texture in Listen sortieren
	list<MapTile> sorted_textures[16];

	int lastxo = 0, lastyo = 0;
	int xo, yo;
	unsigned short tx,ty;

	// Beim zeichnen immer nur beginnen, wo man auch was sieht
	unsigned int water_count = 0;
	for(int y = gwv->GetFirstY(); y < gwv->GetLastY(); ++y)
	{
		unsigned char last = 255;

		for(int x = gwv->GetFirstX(); x < gwv->GetLastX(); ++x)
		{
			ConvertCoords(x, y, tx, ty, &xo, &yo);

			unsigned char t = gwv->GetNode(tx,ty).t1;
			if(xo != lastxo || yo != lastyo)
				last = 255;

			// Wasser?
			if(t == TT_WATER)
			{
				// Sichtbar?
				if(gwv->GetVisibility(tx,ty) == VIS_VISIBLE)
					// Dann Waser erh�hen, um Wasserrauschen abzuspielen
					++water_count;
			}

			if(t == last)
				++sorted_textures[t].end()->count;
			else
			{
				MapTile tmp = {tx * 2, ty, 1, xo, yo};
				sorted_textures[t].push_back(tmp);
			}

			last = t;
			lastxo = xo;
			lastyo = yo;

			t = gwv->GetNode(tx,ty).t2;
			if(xo != lastxo || yo != lastyo)
				last = 255;

			// Wasser?
			if(t == TT_WATER)
			{
				// Sichtbar?
				if(gwv->GetVisibility(tx,ty) == VIS_VISIBLE)
					// Dann Waser erh�hen, um Wasserrauschen abzuspielen
					++water_count;
			}

			if(t == last)
				++sorted_textures[t].end()->count;
			else
			{
				MapTile tmp = {tx * 2 + 1, ty, 1, xo, yo};
				sorted_textures[t].push_back(tmp);
			}

			last = t;
			lastxo = xo;
			lastyo = yo;
		}
	}

	assert(water);

	if( (gwv->GetLastX() - gwv->GetFirstX()) && (gwv->GetLastY() - gwv->GetFirstY()) )
		*water = 50 * water_count / ( (gwv->GetLastX() - gwv->GetFirstX()) * (gwv->GetLastY() - gwv->GetFirstY()) );
	else
		*water = 0;

	list<BorderTile> sorted_borders[5];
	unsigned int offset = width * height * 2;

	lastxo = lastyo = 0;
	for(int y = gwv->GetFirstY(); y < gwv->GetLastY(); ++y)
	{
		unsigned char last = 255;
		for(int x = gwv->GetFirstX(); x < gwv->GetLastX(); ++x)
		{
			ConvertCoords(x, y, tx, ty, &xo, &yo);

			if(xo != lastxo || yo != lastyo)
				last = 255;

			unsigned char tiles[6] =
			{
				borders[ty * width + tx].left_right[0],
				borders[ty * width + tx].left_right[1],
				borders[ty * width + tx].right_left[0],
				borders[ty * width + tx].right_left[1],
				borders[ty * width + tx].top_down[0],
				borders[ty * width + tx].top_down[1],
			};

			unsigned int offsets[6] =
			{
				borders[ty * width + tx].left_right_offset[0],
				borders[ty * width + tx].left_right_offset[1],
				borders[ty * width + tx].right_left_offset[0],
				borders[ty * width + tx].right_left_offset[1],
				borders[ty * width + tx].top_down_offset[0],
				borders[ty * width + tx].top_down_offset[1],
			};

			for(unsigned char i = 0; i < 6; ++i)
			{
				if(tiles[i])
				{
					if(tiles[i] == last)
						++sorted_borders[last-1].end()->count;
					else
					{
						last = tiles[i];
						BorderTile tmp = { offsets[i], 1, xo, yo};
						sorted_borders[last-1].push_back(tmp);
					}
					++offset;

					lastxo = xo;
					lastyo = yo;
				}
			}
		}
	}

	lastxo = 0;
	lastyo = 0;

	// Arrays aktivieren
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	// Modulate2x
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
	glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 2.0f);

	// Verschieben gem�� x und y offset
	glTranslatef( float(-gwv->GetXOffset()), float(-gwv->GetYOffset()), 0.0f);

	// Alphablending aus
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

	for(unsigned char i = 0; i < 16; ++i)
	{
		if(sorted_textures[i].size())
		{
			if(i == 14)
				glBindTexture(GL_TEXTURE_2D, GetImage(water, GAMECLIENT.GetGlobalAnimation(8,5, 5, 0))->GetTexture());
			else if(i == 15)
				glBindTexture(GL_TEXTURE_2D, GetImage(lava, GAMECLIENT.GetGlobalAnimation(8,5, 5, 0) % 4)->GetTexture());
			else
				glBindTexture(GL_TEXTURE_2D, GetImage(textures, i)->GetTexture());

			for(list<MapTile>::iterator it = sorted_textures[i].begin(); it.valid(); ++it)
			{
				if(it->xo != lastxo || it->yo != lastyo)
				{
					glTranslatef( float(it->xo - lastxo), float(it->yo - lastyo), 0.0f);
					lastxo = it->xo;
					lastyo = it->yo;
				}

				glDrawArrays(GL_TRIANGLES, it->y * width * 3 * 2 + it->x * 3, it->count * 3);
			}
		}
	}
	glEnable(GL_BLEND);

	glLoadIdentity();
	glTranslatef( float(-gwv->GetXOffset()), float(-gwv->GetYOffset()), 0.0f);

	lastxo = 0;
	lastyo = 0;
	for(unsigned short i = 0;i<5;++i)
	{
		if(sorted_borders[i].size())
		{
			glBindTexture(GL_TEXTURE_2D, GetImage(borders, i)->GetTexture());

			for(list<BorderTile>::iterator it = sorted_borders[i].begin(); it.valid(); ++it)
			{
				if(it->xo != lastxo || it->yo != lastyo)
				{
					glTranslatef( float(it->xo - lastxo), float(it->yo - lastyo), 0.0f);
					lastxo = it->xo;
					lastyo = it->yo;
				}
				glDrawArrays(GL_TRIANGLES, it->offset * 3, it->count * 3);
			}
		}
	}

	glLoadIdentity();

	DrawWays(gwv);

	// Wieder zur�ck ins normale modulate
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}


///////////////////////////////////////////////////////////////////////////////
/**
 *  Konvertiert die Koordinaten.
 *
 *  @param[in,out] x  Die X-Koordinate
 *  @param[in,out] y  Die Y-Koordinate
 *  @param[out]    xo Das X-Offset
 *  @param[out]    yo Das Y-Offset
 *
 *  @author OLiver
 */
void TerrainRenderer::ConvertCoords(int x,int y,unsigned short& x_out, unsigned short& y_out, int *xo, int *yo) const
{
	bool mx = false, my = false;

	while(x < 0)
	{
		x += width;
		mx = true;
	}

	while(y < 0)
	{
		y += height;
		my = true;
	}

	if(xo)
		*xo = (mx ? (x / width) * TR_W - width * TR_W : (x / width) * TR_W * width );
	if(yo)
		*yo = (my ? (y / height) * TR_H - height * TR_H : (y / height) * TR_H * height);

	x %= width;
	y %= height;

	x_out = static_cast<unsigned short>(x);
	y_out = static_cast<unsigned short>(y);
}

void TerrainRenderer::DrawWays(const GameWorldViewer * gwv)
{
	for(int y = gwv->GetFirstY();y<gwv->GetLastY();++y)
	{
		for(int x = gwv->GetFirstX();x<gwv->GetLastX();++x)
		{
			unsigned short tx,ty;
			int xo,yo;
			ConvertCoords(x,y,tx,ty,&xo,&yo);

			float xpos = GetTerrainX(tx,ty)-gwv->GetXOffset()+xo;
			float ypos = GetTerrainY(tx,ty)-gwv->GetYOffset()+yo;


			//////////////////////////////
			// Weg(e) zeichnen

			unsigned short coords[6] =
			{
				gwv->GetXA(tx,ty,3), gwv->GetYA(tx,ty,3), // -
				gwv->GetXA(tx,ty,4), gwv->GetYA(tx,ty,4), // !/
				gwv->GetXA(tx,ty,5), gwv->GetYA(tx,ty,5), // /
			};

			float begin_end_coords[24] =
			{
				-3.0f,-3.0f,
				-3.0f, 3.0f,
				-3.0f, 3.0f,
				-3.0f,-3.0f,

				 3.0f,-3.0f,
				-3.0f, 3.0f,
				-3.0f, 3.0f,
				 3.0f,-3.0f,

				 3.0f, 3.0f,
				-3.0f,-3.0f,
				-3.0f,-3.0f,
				 3.0f, 3.0f,
			};

			// Terrain drumherum speichern
			unsigned terrain[6];
			for(unsigned i = 0;i<6;++i)
				terrain[i] = gwv->GetTerrainAround(tx,ty,i);

			// Wegtypen f�r die drei Richtungen
			unsigned char type;
			// Ein g�ltiger Wegtyp f�r das Zeichnen der Knotenpunkte (0xFF = kein Weg)
			unsigned char first_type = 0xFF;

			Visibility visibility = gwv->GetVisibility(tx,ty);

			for(unsigned char dir = 0;dir<3;++dir)
			{
				if( (type = gwv->GetVisibleRoad(tx,ty, dir+3, visibility)) )
				{
					float xpos2 = GetTerrainX(coords[dir*2],coords[dir*2+1])-gwv->GetXOffset()+xo;
					float ypos2 = GetTerrainY(coords[dir*2],coords[dir*2+1])-gwv->GetYOffset()+yo;

					--type;

					// Wegtypen "konvertieren"
					switch(type)
					{
					case RoadSegment::RT_DONKEY:
					case RoadSegment::RT_NORMAL:
						{
							// Pr�fen, ob Bergwerge gezeichnet werden m�ssen, indem man guckt, ob der Weg einen
							// Berg "streift" oder auch eine Bergwiese
							if(( (terrain[dir+2] >= TT_MOUNTAIN1 && terrain[dir+2] <= TT_MOUNTAIN4) || terrain[dir+2] == TT_MOUNTAINMEADOW) 
								|| ( (terrain[dir+3] >= TT_MOUNTAIN1  && terrain[dir+3] <= TT_MOUNTAIN4) || terrain[dir+3] == TT_MOUNTAINMEADOW))
								type = 3;

						} break;
					case RoadSegment::RT_BOAT:
						{
							type = 2;
						} break;
					}

					first_type = type;


					glColor3f(1.0f,1.0f,1.0f);
					glBindTexture(GL_TEXTURE_2D, GetImage(roads, type)->GetTexture());
					glBegin(GL_QUADS);


					glColor3f(GetColor(tx,ty),GetColor(tx,ty),GetColor(tx,ty));
					glTexCoord2f(0.0f,0.0f);
					glVertex2f(xpos+begin_end_coords[dir*8],ypos+begin_end_coords[dir*8+1]);
					glTexCoord2f(0.0f,1.0f);
					glVertex2f(xpos+begin_end_coords[dir*8+2],ypos+begin_end_coords[dir*8+3]);

					glColor3f(GetColor(coords[dir*2],coords[dir*2+1]),GetColor(coords[dir*2],coords[dir*2+1]),GetColor(coords[dir*2],coords[dir*2+1]));
					glTexCoord2f(0.78f,1.0f);
					glVertex2f(xpos2+begin_end_coords[dir*8+4],ypos2+begin_end_coords[dir*8+5]);
					glTexCoord2f(0.78f,0.0f);
					glVertex2f(xpos2+begin_end_coords[dir*8+6],ypos2+begin_end_coords[dir*8+7]);

					glEnd();
				}
			}


			// Wegknotenpunkt zeichnen
			// irgendeine Stra�e, die man als erstes findet, von der den Knotenpunkt zeichnen
			if(first_type != 0xFF)	{
				glColor3f(GetColor(tx,ty),GetColor(tx,ty),GetColor(tx,ty));
				glBindTexture(GL_TEXTURE_2D, GetImage(roads_points, first_type)->GetTexture());
				glBegin(GL_QUADS);

				glTexCoord2f(0.0f,0.0f);
				glVertex2f(xpos-3.0f,ypos-3.0f);
				glTexCoord2f(0.0f,1.0f);
				glVertex2f(xpos-3.0f,ypos+3.0f);
				glTexCoord2f(0.875f,1.0f);
				glVertex2f(xpos+3.0f,ypos+3.0f);
				glTexCoord2f(0.875f,0.0f);
				glVertex2f(xpos+3.0f,ypos-3.0f);

				glEnd();
			}
		}
	}
}

void TerrainRenderer::AltitudeChanged(const MapCoord x, const MapCoord y, const GameWorldViewer * gwv)
{
	// den selbst sowieso die Punkte darum updaten, da sich bei letzteren die Schattierung ge�ndert haben k�nnte
	UpdateVertexPos(x,y,gwv);
	UpdateVertexColor(x,y,gwv);
	
	for(unsigned i = 0;i<6;++i)
		UpdateVertexColor(gwv->GetXA(x,y,i),gwv->GetYA(x,y,i),gwv);
	

	// und f�r die R�nder
	UpdateBorderVertex(x,y,gwv);

	for(unsigned i = 0;i<6;++i)
		UpdateBorderVertex(gwv->GetXA(x,y,i),gwv->GetYA(x,y,i),gwv);

	// den selbst sowieso die Punkte darum updaten, da sich bei letzteren die Schattierung ge�ndert haben k�nnte
	UpdateTrianglePos(x,y,gwv,true);
	UpdateTriangleColor(x,y,gwv,true);
	
	for(unsigned i = 0;i<6;++i)
	{
		UpdateTrianglePos(gwv->GetXA(x,y,i),gwv->GetYA(x,y,i),gwv,true);
		UpdateTriangleColor(gwv->GetXA(x,y,i),gwv->GetYA(x,y,i),gwv,true);
	}


	// Auch im zweiten Kreis drumherum die Dreiecke neu berechnen, da die durch die Schatten�nderung der umliegenden
	// Punkte auch ge�ndert werden k�nnten
	for(unsigned i = 0;i<12;++i)
		UpdateTriangleColor(gwv->GetXA2(x,y,i),gwv->GetYA2(x,y,i),gwv,true);
	

	// und f�r die R�nder
	UpdateBorderTrianglePos(x,y,gwv,true);
	UpdateBorderTriangleColor(x,y,gwv,true);

	for(unsigned i = 0;i<6;++i)
	{
		UpdateBorderTrianglePos(gwv->GetXA(x,y,i),gwv->GetYA(x,y,i),gwv,true);
		UpdateBorderTriangleColor(gwv->GetXA(x,y,i),gwv->GetYA(x,y,i),gwv,true);
	}

	for(unsigned i = 0;i<12;++i)
		UpdateBorderTriangleColor(gwv->GetXA2(x,y,i),gwv->GetYA2(x,y,i),gwv,true);
}

void TerrainRenderer::VisibilityChanged(const MapCoord x, const MapCoord y, const GameWorldViewer * gwv)
{
	/// Noch kein Terrain gebaut? abbrechen
	if(!vertices)
		return;

	UpdateVertexColor(x,y,gwv);
	for(unsigned i = 0;i<6;++i)
		UpdateVertexColor(gwv->GetXA(x,y,i),gwv->GetYA(x,y,i),gwv);
	
	// und f�r die R�nder
	UpdateBorderVertex(x,y,gwv);
	for(unsigned i = 0;i<6;++i)
		UpdateBorderVertex(gwv->GetXA(x,y,i),gwv->GetYA(x,y,i),gwv);

	// den selbst sowieso die Punkte darum updaten, da sich bei letzteren die Schattierung ge�ndert haben k�nnte
	UpdateTriangleColor(x,y,gwv,true);
	for(unsigned i = 0;i<6;++i)
		UpdateTriangleColor(gwv->GetXA(x,y,i),gwv->GetYA(x,y,i),gwv,true);

	// und f�r die R�nder
	UpdateBorderTriangleColor(x,y,gwv,true);
	for(unsigned i = 0;i<6;++i)
		UpdateBorderTriangleColor(gwv->GetXA(x,y,i),gwv->GetYA(x,y,i),gwv,true);
}


void TerrainRenderer::UpdateAllColors(const GameWorldViewer * gwv)
{
	for(MapCoord y = 0;y<height;++y)
	{
		for(MapCoord x = 0;x<width;++x)
		{
			UpdateVertexColor(x,y,gwv);
		}
	}

	for(MapCoord y = 0;y<height;++y)
	{
		for(MapCoord x = 0;x<width;++x)
		{
			UpdateBorderVertex(x,y,gwv);
		}
	}

	for(MapCoord y = 0;y<height;++y)
	{
		for(MapCoord x = 0;x<width;++x)
		{
			UpdateTriangleColor(x,y,gwv,false);
		}
	}

	for(MapCoord y = 0;y<height;++y)
	{
		for(MapCoord x = 0;x<width;++x)
		{
			UpdateBorderTriangleColor(x,y,gwv,false);
		}
	}

	
	if(SETTINGS.vbo)
	{
		// Generiere und Binde den Color Buffer
		glGenBuffersARB(1, (GLuint*)&vbo_colors);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,vbo_colors);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, (width * height * 2 + border_count) * 3 * 3 * sizeof(float), gl_colors, GL_STATIC_DRAW_ARB );
		glColorPointer(3, GL_FLOAT, 0, NULL);
	}
	else
		glColorPointer(3, GL_FLOAT, 0, gl_colors);
}
