// $Id: VideoInterface.h 6582 2010-07-16 11:23:35Z FloSoft $
//
// Copyright (c) 2005 - 2010 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.
#ifndef VIDEOINTERFACE_H_INCLUDED
#define VIDEOINTERFACE_H_INCLUDED

#pragma once

#include "VideoDriver.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#undef DRIVERDLLAPI
#ifdef _WIN32
#	if defined _USRDLL || defined _LIB || defined BUILD_DLL
#		define DRIVERDLLAPI extern "C" __declspec(dllexport)
#		define class        class __declspec(dllexport)
#	else
#		define DRIVERDLLAPI extern "C" __declspec(dllimport)
#	endif // !_USRDLL
#else
#	define DRIVERDLLAPI extern "C"
#endif // !_WIN32

/// Instanzierungsfunktion der Treiber.
DRIVERDLLAPI VideoDriver *CreateVideoInstance(VideoDriverLoaderInterface * CallBack);

/// 
typedef VideoDriver *(*PDRIVER_CREATEVIDEOINSTANCE)(VideoDriverLoaderInterface *);

const char *GetWindowTitle();
const char *GetWindowVersion();
const char *GetWindowRevision();

#endif // !VIDEOINTERFACE_H_INCLUDED
