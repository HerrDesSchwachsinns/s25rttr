// $Id: languages.cpp 4652 2009-03-29 10:10:02Z FloSoft $
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
#include "languages.h"

#include "files.h"
#include "Settings.h"

#include <algorithm>

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/** 
 *  
 *
 *  @author FloSoft
 */
void Languages::loadLanguages()
{
	unsigned int count = LOADER.lang_txt.getCount();

	// abrunden
	count -= (count % 2);

	for(unsigned int i = 0; i < count; i += 2)
	{
		libsiedler2::ArchivItem_Text *n = dynamic_cast<libsiedler2::ArchivItem_Text*>(LOADER.lang_txt.get(i));
		libsiedler2::ArchivItem_Text *c = dynamic_cast<libsiedler2::ArchivItem_Text*>(LOADER.lang_txt.get(i+1));

		if(!n)
			continue;

		Language l(n->getText(), "");

		if(c)
			l.code = c->getText();

		languages.push_back(l);
	}

	// Sprachen sortieren
	std::sort(languages.begin(), languages.end(), Language::compare);

	// Systemsprache hinzuf�gen
	Language l(gettext_noop("System language"), "");
	languages.insert(languages.begin(), l);

	loaded = true;
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  
 *
 *  @author FloSoft
 */
const Languages::Language &Languages::getLanguage(unsigned int i)
{
	if(!loaded)
		loadLanguages();

	if(i < languages.size())
		return languages.at(i); 

	return languages.at(0);
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  
 *
 *  @author FloSoft
 */
unsigned int Languages::getCount(void)
{
	if(!loaded)
		loadLanguages();
	
	return unsigned(languages.size()); 
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  
 *
 *  @author FloSoft
 *  @author OLiver
 */
void Languages::setLanguage(const std::string& lang_code)
{
	Settings::inst().language = lang_code;

	std::string locale = mysetlocale(LC_ALL, lang_code.c_str());
	if(Settings::inst().language.empty())
		Settings::inst().language = locale;

	const char *domain = "rttr";
	bind_textdomain_codeset(domain, "ISO-8859-1");
	bindtextdomain(domain, FILE_PATHS[15]);
	textdomain(domain);
}

///////////////////////////////////////////////////////////////////////////////
/** 
 *  
 *
 *  @author FloSoft
 */
const std::string Languages::setLanguage(unsigned int i)
{
	const Language l = getLanguage(i);

	setLanguage(l.code);

	return l.code;
}
