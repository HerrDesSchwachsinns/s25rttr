// $Id: colors.h 4956 2009-05-26 06:58:47Z Demophobie $
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
#ifndef COLORS_H_INCLUDED
#define COLORS_H_INCLUDED

#pragma once

/// Texturfarben
enum TextureColor
{
	TC_GREY = 0,
	TC_RED1,
	TC_GREEN1,
	TC_GREEN2,
	TC_RED2,
	TC_BRICKS
};


/// Farbkonstanten
const unsigned COLOR_BLUE = 0xFF0073FF;
const unsigned COLOR_RED = 0xFFFF0000;
const unsigned COLOR_YELLOW	= 0xFFFFFF00;
const unsigned COLOR_GREEN	= 0xFF00FF00;
const unsigned COLOR_MAGENTA = 0xFFFF00FF;
const unsigned COLOR_CYAN = 0xFF00FFFF;
const unsigned COLOR_BLACK = 0xFF444444;
const unsigned COLOR_WHITE = 0xFFFFFFFF;
const unsigned COLOR_ORANGE = 0xFFFF8000;
const unsigned COLOR_BROWN = 0xFF6C4337;
const unsigned COLOR_GHOST = 0x10FFFFFF;
const unsigned COLOR_SHADOW = 0x40000000;
const unsigned COLOR_GREY = 0xFFCCCCCC;

/// Spielerfarben
const unsigned int PLAYER_COLORS_COUNT = 11;

const unsigned int COLORS[PLAYER_COLORS_COUNT] =
{
	COLOR_BLUE, // Blau
	COLOR_RED, // Rot
	COLOR_YELLOW, // Gelb
	COLOR_GREEN, // Gr�n
	COLOR_MAGENTA, // Magenta
	COLOR_CYAN, // Cyan,
	COLOR_BLACK, // Schwarz
	COLOR_WHITE, // Wei�
	COLOR_ORANGE, // Orange
	COLOR_BROWN, // Braun
	COLOR_GHOST // Durchsichtig
};

/// Liefert die transparente Farbkomponente zur�ck.
inline unsigned GetAlpha(const unsigned int color) { return (( color & 0xFF000000) >> 24); }
/// Liefert die rote Farbkomponente zur�ck.
inline unsigned GetRed(  const unsigned int color) { return (( color & 0x00FF0000) >> 16); }
/// Liefert die gr�ne Farbkomponente zur�ck.
inline unsigned GetGreen(const unsigned int color) { return (( color & 0x0000FF00) >> 8 ); }
/// Liefert die blaue Farbkomponente zur�ck.
inline unsigned GetBlue( const unsigned int color) { return (( color & 0x000000FF)      ); }

/// Setzt die transparente Farbkomponente.
inline unsigned SetAlpha(const unsigned int color, const unsigned int alpha) { return (( color & 0x00FFFFFF) | ( alpha << 24) ); }
/// Setzt die rote Farbkomponente.
inline unsigned SetRed(  const unsigned int color, const unsigned int red  ) { return (( color & 0xFF00FFFF) | ( red   << 16) ); }
/// Setzt die gr�ne Farbkomponente.
inline unsigned SetGreen(const unsigned int color, const unsigned int green) { return (( color & 0xFFFF00FF) | ( green << 8 ) ); }
/// Setzt die blaue Farbkomponente.
inline unsigned SetBlue( const unsigned int color, const unsigned int blue ) { return (( color & 0xFFFFFF00) | ( blue       ) ); }

/// Erzeugt eine fertige Farbe aus einzelnen Farbkomponenten
inline unsigned MakeColor(const unsigned int alpha, const unsigned int red, const unsigned int green, const unsigned int blue) 
{ 
	return ((alpha << 24) | (red << 16) | (green << 8) |  blue);
}

#endif // COLORS_H_INCLUDED
