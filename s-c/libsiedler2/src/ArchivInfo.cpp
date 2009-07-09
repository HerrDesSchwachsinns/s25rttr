// $Id: ArchivInfo.cpp 5238 2009-07-09 20:50:28Z FloSoft $
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
#include "ArchivInfo.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/** @class libsiedler2::ArchivInfo
 *
 *  Klasse f�r Archivdateien.
 *
 *  @author FloSoft
 */

///////////////////////////////////////////////////////////////////////////////
/** @var libsiedler2::ArchivInfo::count
 * 
 *  Anzahl der Elemente.
 *
 *  @author FloSoft
 */

///////////////////////////////////////////////////////////////////////////////
/** @var libsiedler2::ArchivInfo::data
 * 
 *  die Elemente.
 *
 *  @author FloSoft
 */

///////////////////////////////////////////////////////////////////////////////
/** 
 *  Konstruktor von @p ArchivInfo.
 *
 *  @author FloSoft
 */
libsiedler2::ArchivInfo::ArchivInfo(void)
{
	data = NULL;
	count = 0;
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  Kopierkonstruktor von @p ArchivInfo.
 *
 *  @param[in] info Quellitem
 *
 *  @author FloSoft
 */
libsiedler2::ArchivInfo::ArchivInfo(const ArchivInfo &info)	: data(NULL), count(0)
{
	set(&info);
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  Kopierkonstruktor von @p ArchivInfo.
 *
 *  @param[in] info Quellitem
 *
 *  @author FloSoft
 */
libsiedler2::ArchivInfo::ArchivInfo(const ArchivInfo *info)	: data(NULL), count(0)
{
	set(info);
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  Destruktor von @p ArchivInfo, r�umt automatisch auf.
 *
 *  @author FloSoft
 */
libsiedler2::ArchivInfo::~ArchivInfo(void)
{
	clear();
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  erstellt den Datensatz in einer bestimmten Gr��e.
 *
 *  @param[in] count Elementanzahl auf den der Datensatz angelegt werden soll
 *
 *  @author FloSoft
 */
void libsiedler2::ArchivInfo::alloc(unsigned long count)
{
	clear();

	this->count = count;
	data = new ArchivItem*[count];

	memset(data, 0, sizeof(ArchivItem*)*count);
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  vergr��ert den Datensatz um eine bestimmten Gr��e.
 *
 *  @param[in] increment Elementanzahl um den der Datensatz vergr��ert werden soll
 *
 *  @author FloSoft
 */
void libsiedler2::ArchivInfo::alloc_inc(unsigned long increment)
{
	ArchivItem **new_data = new ArchivItem*[count+increment];

	memset(new_data, 0, sizeof(ArchivItem*)*(count+increment));

	if(count > 0 && data)
	{
		for(unsigned long i = 0; i < count; ++i)
			new_data[i] = data[i];

		delete[] data;
	}

	data = new_data;
	count += increment;
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  gibt die angelegten Daten wieder frei.
 *
 *  @author FloSoft
 */
void libsiedler2::ArchivInfo::clear(void)
{
	if(data)
	{
		for(unsigned long c = 0; c < count; ++c)
		{
			delete data[c];
			data[c] = NULL;
		}
		delete[] data;
	}

	data = NULL;
	count = 0;
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  Setzt den Inhalt eines ArchivItems auf das des �bergebenen.
 *
 *  @param[in] index Index des zu setzenden Eintrags
 *  @param[in] item  Item mit dem zu setzenden Inhalt
 *
 *  @author FloSoft
 */
void libsiedler2::ArchivInfo::set(int index, ArchivItem *item)
{
	if(!data)
		return;

	if( (unsigned long)index < count && index >= 0)
		data[(unsigned long)index] = item;
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  kopiert den Inhalt eines ArchivItems auf das des �bergebenen.
 *
 *  @param[in] index Index des zu setzenden Eintrags
 *  @param[in] item  Item mit dem zu kopierenden Inhalt
 *
 *  @author FloSoft
 */
void libsiedler2::ArchivInfo::setC(int index, const ArchivItem *item)
{
	if(!data)
		return;

	if( (unsigned long)index < count && index >= 0)
	{
		// ist das Item g�ltig?
		if(item == NULL)
			data[(unsigned long)index] = NULL;
		else
		{
			// ja, dann neues item erstellen
			data[(unsigned long)index] = (ArchivItem*)(*allocator)(0, 0, item);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  f�gt ein Element hinten an.
 *
 *  @param[in] item Item mit dem anzuf�genden Inhalt
 *
 *  @author FloSoft
 */
void libsiedler2::ArchivInfo::push(ArchivItem *item)
{
	alloc_inc(1);

	data[count-1] = item;
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  f�gt ein Element hinten an und kopiert die Daten von @p item.
 *
 *  @param[in] item Item mit dem zu kopierenden Inhalt
 *
 *  @author FloSoft
 */
void libsiedler2::ArchivInfo::pushC(const ArchivItem *item)
{
	alloc_inc(1);

	if(item)
		data[count-1] = (ArchivItem*)(*allocator)(0, 0, item);
	else
		data[count-1] = NULL;
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  liefert den Inhalt eines ArchivItems am entsprechenden Index.
 *
 *  @param[in] index Index des zu setzenden Eintrags
 *
 *  @return liefert NULL bei Fehler, ansonsten das entsprechende Item
 *
 *  @author FloSoft
 */
libsiedler2::ArchivItem *libsiedler2::ArchivInfo::get(int index)
{
	if(!data)
		return NULL;

	if( (unsigned long)index < count && index >= 0)
		return data[(unsigned long)index];

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  liefert den Inhalt eines ArchivItems am entsprechenden Index.
 *
 *  @param[in] index Index des zu setzenden Eintrags
 *
 *  @return liefert NULL bei Fehler, ansonsten das entsprechende Item
 *
 *  @author FloSoft
 */
const libsiedler2::ArchivItem *libsiedler2::ArchivInfo::get(int index) const
{
	if(!data)
		return NULL;

	if( (unsigned long)index < count && index >= 0)
		return data[(unsigned long)index];

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  liefert den Pointer eines ArchivItems am entsprechenden Index.
 *
 *  @param[in] index Index des zu setzenden Eintrags
 *
 *  @return liefert NULL bei Fehler, ansonsten das entsprechende Item
 *
 *  @author FloSoft
 */
libsiedler2::ArchivItem **libsiedler2::ArchivInfo::getP(int index)
{
	if(!data)
		return NULL;

	if( (unsigned long)index < count && index >= 0)
		return &data[(unsigned long)index];

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  liefert die Gr��e des Archivs.
 *
 *  @return liefert die Gr��e des Archivs.
 *
 *  @author FloSoft
 */
unsigned long libsiedler2::ArchivInfo::getCount(void) const
{
	return count;
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  Index-Operator von @p ArchivInfo. 
 *
 *  @param[in] index Index des zu liefernden Eintrags
 *
 *  @return Bei Erfolg ArchivItem, ansonsten NULL
 *
 *  @author FloSoft
 */
libsiedler2::ArchivItem *libsiedler2::ArchivInfo::operator[](int index)
{
	return get(index);
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  Zuweisungsoperator von @p ArchivInfo.
 *
 *  @param[in] info Quellitem
 *
 *  @return Referenz auf this
 *
 *  @author FloSoft
 */
libsiedler2::ArchivInfo &libsiedler2::ArchivInfo::operator=(ArchivInfo &info)
{
	clear();

	alloc(info.count);

	for(unsigned long i = 0; i < count; ++i)
		setC(i, info.get(i));

	return *this;
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  Kopierfunktion von @p ArchivInfo.
 *
 *  @param[in] to     Zielposition
 *  @param[in] from   Quellposition
 *  @param[in] count  Anzahl
 *  @param[in] source Quelle
 *
 *  @author FloSoft
 */
void libsiedler2::ArchivInfo::copy(unsigned int to, unsigned int from, unsigned int count, const ArchivInfo *source)
{
	if(to+count > this->count)
		alloc_inc(to+count-this->count);

	for(unsigned int f = from; f < from+count; ++to, ++f)
		setC(to, source->get(f));
}
