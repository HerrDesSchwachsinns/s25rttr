// $Id: Log.cpp 6460 2010-05-31 11:42:38Z FloSoft $
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
#include "Log.h"

#include "files.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/**
 *  Konstruktor von @p Log.
 *
 *  @author FloSoft
 */
Log::Log(void) : log(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Destruktor von @p Log.
 *
 *  @author FloSoft
 */
Log::~Log(void)
{
	if(log)
		fclose(log);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  �ffnet das Log (falls noch nicht offen)
 *
 *  @author FloSoft
 */
void Log::open(void)
{
	if(!log)
	{
		// Dateiname erzeugen
		char filename[256], time[80];
		TIME.FormatTime(time, "%Y-%m-%d_%H-%i-%s", NULL);

		sprintf(filename,"%s%s.log", GetFilePath(FILE_PATHS[47]).c_str(), time);

		log = fopen(filename, "w");
	}
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Schreibt gef�rbte Daten ins Log und auf stdout.
 *
 *  @author FloSoft
 */
void Log::lcprintf(const unsigned int color, const char *format, ...)
{
	// On Linux, we insert escape-codes into the string. On Windows call system functions.
	#ifndef _WIN32
		std::string tmp;

		// A switch statement doesn't work here because we compare against the array COLORS[] (although it's constant, it can't be dereferenced at compile time)
		if (color == COLOR_BLUE)
			tmp = "\033[40m\033[1;34m";
		else if (color == COLOR_RED)
			tmp = "\033[40m\033[1;31m";
		else if (color == COLOR_YELLOW)
			tmp = "\033[40m\033[1;33m";
		else if (color == COLOR_GREEN)
			tmp = "\033[40m\033[1;32m";
		else if (color == COLOR_MAGENTA)
			tmp = "\033[40m\033[1;35m";
		else if (color == COLOR_CYAN)
			tmp = "\033[40m\033[1;36m";
		else if (color == COLOR_BLACK)
			tmp = "\033[47m\033[1;30m";
		else if (color == COLOR_WHITE)
			tmp = "\033[40m\033[1;37m";
		else if (color == COLOR_ORANGE)
			tmp = "\033[43m\033[1;30m";
		else if (color == COLOR_BROWN)
			tmp = "\033[40m\033[33m";
		else 
			tmp = "\033[0m";

		lprintf("%s", tmp.c_str());
	#else
		// obtain handle
		HANDLE hStdout; 
		CONSOLE_SCREEN_BUFFER_INFO csbiInfo; 

		hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 		
		if (GetConsoleScreenBufferInfo(hStdout, &csbiInfo)) 
		{
			WORD colorAttr = 0;
			if (color == COLOR_BLUE || color == COLOR_MAGENTA || color == COLOR_CYAN || color == COLOR_WHITE)
				colorAttr |= FOREGROUND_BLUE;
			if (color == COLOR_YELLOW || color == COLOR_GREEN || color == COLOR_CYAN || color == COLOR_WHITE || color == COLOR_BROWN)
				colorAttr |= FOREGROUND_GREEN;
			if (color == COLOR_RED || color == COLOR_YELLOW || color == COLOR_MAGENTA || color == COLOR_WHITE || color == COLOR_BROWN)
				colorAttr |= FOREGROUND_RED;
			if (color == COLOR_BLACK)
				colorAttr |= BACKGROUND_BLUE;
			if (color == COLOR_BLACK || color == COLOR_ORANGE)
				colorAttr |= BACKGROUND_GREEN;
			if (color == COLOR_BLACK || color == COLOR_ORANGE)
				colorAttr |= BACKGROUND_RED;

			// if color matches any but brown
			if (colorAttr != 0 && color != COLOR_BROWN)
				colorAttr |= FOREGROUND_INTENSITY;

			// if color didn't match any, make default gray
			if (colorAttr == 0)
				colorAttr = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;

			SetConsoleTextAttribute(hStdout, colorAttr);
		}
	#endif 

	va_list list;

	va_start(list, format);
	lvprintf(format, list);
	va_end(list);

	// restore white-on-black
	#ifndef _WIN32
		tmp = "\033[0m";
		lprintf("%s", tmp.c_str());
	#else
		// Obtain handle
		if (GetConsoleScreenBufferInfo(hStdout, &csbiInfo)) 
			SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
	#endif 
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Schreibt Daten ins Log und auf stdout.
 *
 *  @author FloSoft
 */
void Log::lprintf(const char *format, ...)
{
	va_list list;

	va_start(list, format);
	lvprintf(format, list);
	va_end(list);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Schreibt Daten ins Log und auf stdout.
 *
 *  @author FloSoft
 */
void Log::lvprintf(const char *format, va_list list)
{
	va_list list2;
	va_copy(list2, list);

	::vprintf(format, list);
	vwrite(format, list2);

	va_end(list2);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Schreibt Daten nur ins Log und auf stdout.
 *
 *  @author FloSoft
 */
void Log::write(const char *format, ...)
{
	va_list list;

	va_start(list, format);
	vwrite(format, list);
	va_end(list);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Schreibt Daten nur ins Log und auf stdout.
 *
 *  @author FloSoft
 */
void Log::vwrite(const char *format, va_list list)
{
	open();

	if(log)
	{
		// timestamp erzeugen
		char timestamp[256];
		TIME.FormatTime(timestamp, "%Y.%m.%d - %H:%i:%s - ", NULL);

		// TODO workaround, buggy gettext?
#ifdef _WIN32
#	undef vfprintf
#endif

		vfprintf(log, format, list);
		
		fflush(log);
	}
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Schreibt den zuletzt aufgetretetenen Systemfehler in lesbarer Form in 
 *  stdout und Log, f�gt "$text:" davor ein.
 *
 *  @author FloSoft
 */
void Log::getlasterror(const char *text)
{
	lprintf("%s: ", text);
#ifdef _WIN32
	LPVOID lpMsgBuf;
	FormatMessageA( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		0, // Default language
		(LPSTR) &lpMsgBuf,
		0,
		NULL 
	);

	lprintf("%s\n", lpMsgBuf);

	// Free the buffer.
	LocalFree( lpMsgBuf );
#else
	lprintf("%s\n", strerror(errno));
#endif
}
