// $Id: DoorConsts.h 6458 2010-05-31 11:38:51Z FloSoft $
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

#ifndef DOOR_CONSTS_H_
#define DOOR_CONSTS_H_


// Konstanten fr den Punkt, ab dem die Tr�cer "verschwinden" bei einem Geb�cde, jeweils als Y-Angabe

const signed char DOOR_CONSTS[4][40] =
{
	// Nubier
	{5,
	10,
	13,
	0,
	10,
	0,
	0,
	0,
	0,
	6,
	8,
	8,
	8,
	8,
	6,
	0,
	10,
	10,
	12,
	14,
	9,
	5,
	11,
	19,
	19,
	12,
	18,
	0,
	-6,
	19,
	0,
	12,
	11,
	6,
	10,
	0,
	0,
	-1,
	4,
	13},

	// Japaner
	{9,
	1,
	5,
	0,
	12,
	0,
	0,
	0,
	0,
	7,
	8,
	8,
	8,
	8,
	5,
	0,
	10,
	9,
	5,
	3,
	9,
	10,
	3,
	12,
	10,
	13,
	7,
	0,
	-8,
	14,
	0,
	11,
	10,
	9,
	11,
	0,
	15,
	-7,
	-5,
	16},


	// R�cer
	{11,
	6,
	8,
	0,
	12,
	0,
	0,
	0,
	0,
	-3,
	8,
	8,
	8,
	8,
	6,
	0,
	10,
	12,
	14,
	12,
	9,
	12,
	12,
	16,
	19,
	14,
	16,
	0,
	-8,
	17,
	0,
	6,
	9,
	8,
	14,
	6,
	4,
	-13,
	-8,
	2},


	// Wikinger
	{10,
	12,
	11,
	0,
	11,
	0,
	0,
	0,
	0,
	14,
	9,
	9,
	9,
	9,
	4,
	0,
	10,
	10,
	10,
	12,
	12,
	19,
	11,
	13,
	6,
	11,
	14,
	0,
	-3,
	11,
	0,
	9,
	11,
	10,
	16,
	0,
	16,
	-6,
	-2,
	10}
};

#endif
