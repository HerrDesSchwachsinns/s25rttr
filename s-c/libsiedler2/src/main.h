// $Id: main.h 6460 2010-05-31 11:42:38Z FloSoft $
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
#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#pragma once

#ifdef HAVE_CONFIG_H
#	include "../config.h"
#endif // HAVE_CONFIG_H

#ifdef _WIN32
#	define _CRTDBG_MAP_ALLOC
#	include <windows.h>

#	if defined _DEBUG && defined _MSC_VER
#	include <crtdbg.h>
#	endif // _DEBUG
#else
#	include <unistd.h>
#	include "strlwr.h"
#endif // !_WIN32

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include <string>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
#	define class class __declspec(dllexport)
#ifndef __CYGWIN__
#	define snprintf _snprintf
#endif
#endif

#include "libsiedler2.h"
#include "libendian.h"

extern libsiedler2::TEXTURFORMAT texturformat;
extern libsiedler2::allocatorType allocator;

#endif // MAIN_H_INCLUDED
