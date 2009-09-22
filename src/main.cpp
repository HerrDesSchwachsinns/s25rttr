// $Id: main.cpp 5545 2009-09-22 11:19:53Z FloSoft $
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

#include "GlobalVars.h"
#include "signale.h"
#include "Socket.h"

#include "GameManager.h"

#ifdef __APPLE__
#	include <SDL/SDL_main.h>
#endif // __APPLE__

#if defined _WIN32 && defined _DEBUG && defined _MSC_VER && !defined NOHWETRANS
#	include <windows.h>
#	include <eh.h>
#endif

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/**
 *  Exit-Handler, wird bei @p exit ausgeführt.
 *
 *  @author FloSoft
 */
void ExitHandler(void)
{
	Socket::Shutdown();

#if defined _WIN32 && !defined __CYGWIN__
	LOG.lprintf("\n\nDr%ccken Sie eine beliebige Taste . . .\n", 129);
	getch();
#endif
}

#if defined _WIN32 && defined _DEBUG && defined _MSC_VER && !defined NOHWETRANS
///////////////////////////////////////////////////////////////////////////////
/**
 *  Exception-Handler, wird bei einer C-Exception ausgeführt, falls
 *  dies in der local.h mit deaktiviertem NOHWETRANS und
 *  im Projekt mit den Compilerflags (/EHa) aktiviert ist.
 *
 *  @param[in] exception_type    Typ der Exception (siehe GetExceptionCode)
 *  @param[in] exception_pointer Genaue Beschreibung der Exception (siehe GetExceptionInformation)
 *
 *  @author OLiver
 */
void ExceptionHandler (unsigned int exception_type, _EXCEPTION_POINTERS* exception_pointer)
{
	fatal_error("C-Exception caught\n");
}
#endif // _WIN32 && _DEBUG && !NOHWETRANS

///////////////////////////////////////////////////////////////////////////////
/**
 *  Hauptfunktion von Siedler II.5 Return to the Roots
 *
 *  @param[in] argc Anzahl übergebener Argumente
 *  @param[in] argv Array der übergebenen Argumente
 *
 *  @return Exit Status, 0 bei Erfolg, > 0 bei Fehler
 *
 *  @author FloSoft
 *  @author OLiver
 */
int main(int argc, char *argv[])
{
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER && !defined NOHWETRANS
	_set_se_translator(ExceptionHandler);
#endif // _WIN32 && _DEBUG && !NOHWETRANS

#if defined _WIN32 && defined _DEBUG && defined _MSC_VER && !defined NOCRTDBG
	// Enable Memory-Leak-Detection
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF /*| _CRTDBG_CHECK_CRT_DF*/);
#endif // _WIN32 && _DEBUG && !NOCRTDBG

	// Signal-Handler setzen
#ifdef _WIN32
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);
#else
	struct sigaction sa;
	sa.sa_handler = HandlerRoutine;
	sa.sa_flags = 0; //SA_RESTART would not allow to interrupt connect call;
	sigemptyset(&sa.sa_mask);

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGPIPE, &sa, NULL);
	sigaction(SIGALRM, &sa, NULL);
#endif // _WIN32

	// diverse dirs anlegen
	const unsigned int dir_count = 7;
	unsigned int dirs[dir_count] = { 94, 47, 48, 51, 85, 98, 99 }; // settingsdir muss zuerst angelegt werden (94)

	for(unsigned int i = 0; i < dir_count; ++i)
	{
		std::string dir = GetFilePath(FILE_PATHS[dirs[i]]);
		
		#ifdef _WIN32
			CreateDirectory(dir.c_str(), NULL);
		#else
			mkdir(dir.c_str(), 0750);
		#endif
	}

	libsiedler2::setTextureFormat(libsiedler2::FORMAT_RGBA);
	libsiedler2::setAllocator(glAllocator);

	// Zufallsgenerator initialisieren (Achtung: nur für Animationens-Offsets interessant, für alles andere (spielentscheidende) wird unser Generator verwendet)
	srand(static_cast<unsigned int>(std::time(NULL)));

	// Exit-Handler initialisieren
	atexit(&ExitHandler);
	
	// Socketzeug initialisieren
	if(!Socket::Initialize())
	{
		error("Konnte Sockets nicht initialisieren!");
		return 1;
	}

	// Spiel starten
	if(!GAMEMANAGER.Start()){

        error("Das Spiel konnte nicht gestartet werden");
		return 1;
	}
	// Hauptschleife
	while(GAMEMANAGER.Run())
	{
#ifndef _WIN32
		extern bool killme;
		killme = false;
#endif // !_WIN32
	}

	// Spiel beenden
	GAMEMANAGER.Stop();

	return 0;
}
