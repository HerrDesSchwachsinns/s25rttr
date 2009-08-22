#ifndef LOCAL_H_INCLUDED
#define LOCAL_H_INCLUDED

#ifdef _WIN32
///////////////////////////////////////////////////////////////////////////////
//	Windows Dependant-Configuration

//	disable Memory Leak Detection (comment it out to enable it)
#	define NOCRTDBG

//	disable catching of exceptions (comment it out to enable it)
#	define NOHWETRANS

#	ifdef _DEBUG
//		You can define paths here, but you should only use
//		those definitions in DEBUG-configuration.
#	endif // _DEBUG
//	if you don't define anything here, the current working
//	directory will be used to search the files and paths
//	(the s25rttr-files must be in the SETTLERS II installation folder)

//
///////////////////////////////////////////////////////////////////////////////
#else
///////////////////////////////////////////////////////////////////////////////
//	Other-OS Dependant-Configuration

//	set global prefix (normal /usr/local)
//#define PREFIX "/usr/local"
#cmakedefine PREFIX "${PREFIX}"

//	set binary directory (normal $(prefix)/bin)
//#define BINDIR "/usr/local/bin"
#cmakedefine BINDIR "${BINDIR}"

//	set data directory (normal $(datadir)/s25rttr)
//#define DATADIR "/usr/local/share/s25rttr"
#cmakedefine DATADIR "${DATADIR}"

//	set game directory (normal $(datadir)/s25rttr/S2)
//#define GAMEDIR DATADIR"/S2"

//	set driver directory (normal $(datadir)/s25rttr/driver)
//#define DRIVERDIR DATADIR"/driver"

//	set settings directory (normal $HOME/.s25rttr)
#define SETTINGSDIR "~/.s25rttr"

//
///////////////////////////////////////////////////////////////////////////////
#endif // !_WIN32

// name of settings file (normal $HOME/.s25rttr/settings.bin)
//#define SETTINGSFILE SETTINGSDIR"/settings.bin"

#endif // !LOCAL_H_INCLUDED
