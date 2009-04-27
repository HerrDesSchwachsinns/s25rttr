// $Id: glArchivItem_Sound_Wave.cpp 4652 2009-03-29 10:10:02Z FloSoft $
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
#include "glArchivItem_Sound_Wave.h"

#include "AudioDriverWrapper.h"
#include "Settings.h"
#include "../driver/src/AudioDriver.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/**
 *  Spielt den Sound ab.
 *
 *  @param[in] volume Lautstärke des Sounds.
 *  @param[in] loop   Endlosschleife ja/nein
 *
 *  @author FloSoft
 */
unsigned int glArchivItem_Sound_Wave::Play(unsigned char volume, bool loop)
{
	if(SETTINGS.effekte == false/* || !VideoDriverWrapper::inst().audiodriver*/)
		return 0xFFFFFFFF;

	if(sound == NULL)
		sound = AudioDriverWrapper::inst().LoadEffect(AudioDriver::AD_WAVE, data, length);

	return AudioDriverWrapper::inst().PlayEffect(sound, volume, loop);
}
