// $Id: Random.h 5853 2010-01-04 16:14:16Z FloSoft $
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
#ifndef RANDOM_H_INCLUDED
#define RANDOM_H_INCLUDED

#pragma once

#include "Singleton.h"
#include "list.h"

class Random : public Singleton<Random>
{
	unsigned counter;
	list<std::string> async_log;

public:

	/// Konstruktor von @p Random.
	Random();
	/// Initialisiert den Zufallszahlengenerator.
	void Init(const unsigned int init);
	/// Erzeugt eine Zufallszahl.
	int Rand(const char * const src_name, const unsigned src_line,const unsigned obj_id,const int max);

	template <typename T>
	void Shuffle(T *const elements, unsigned int length, unsigned int repeat = 3)
	{
		for(unsigned int i = 0; i < repeat; ++i)
		{
			for(unsigned int j = 0; j < length; j++)
			{
				unsigned int to = Rand(__FILE__,__LINE__,0,length);
	 
				T temp = elements[i];
				elements[i] = elements[to];
				elements[to] = temp;
			}
		}
	}

	/// Gibt aktuelle Zufallszahl zur�ck
	int GetCurrentRandomValue() const { return zahl; }

	/// Speichere Log
	void SaveLog(const char * const filename);

private:
	int zahl; ///< Die aktuelle Zufallszahl.
};

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#define RANDOM Random::inst()

#endif // !RANDOM_H_INCLUDED
