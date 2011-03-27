// $Id: Desktop.cpp 7091 2011-03-27 10:57:38Z OLiver $
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

///////////////////////////////////////////////////////////////////////////////
// Header
#include <stdafx.h>
#include "main.h"
#include "Desktop.h"

#include "WindowManager.h"
#include "VideoDriverWrapper.h"

///////////////////////////////////////////////////////////////////////////////
// Makros / Defines
#if defined _WIN32 && defined _DEBUG && defined _MSC_VER
	#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
/** 
 *  Konstruktor f�r einen Spieldesktop
 *
 *  @param[in] background Hintergrund des Desktops
 *
 *  @author OLiver
 */
Desktop::Desktop(glArchivItem_Bitmap *background)
	: Window(), background(background)
{
	SetScale(true);
	Resize(VideoDriverWrapper::inst().GetScreenWidth(), VideoDriverWrapper::inst().GetScreenWidth());
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Zeichenmethode zum Zeichnen des Desktops
 *  und der ggf. enthaltenen Steuerelemente.
 *
 *  @return @p true bei Erfolg, @p false bei Fehler
 *
 *  @author OLiver
 */
bool Desktop::Draw_(void)
{
	if(background)
		background->Draw(0, 0, VideoDriverWrapper::inst().GetScreenWidth(), VideoDriverWrapper::inst().GetScreenHeight(), 0, 0, 0, 0);

	DrawControls();

	return true;
}

///////////////////////////////////////////////////////////////////////////////
/*
 *  Wechselt den aktuellen Desktop im WindowManager auf diesen Desktop.
 *
 *  @author OLiver
 */
void Desktop::Show(void)
{
	WindowManager::inst().Switch(this);
}

///////////////////////////////////////////////////////////////////////////////
/*
 *  Reagiert auf Spielfenstergr��en�nderung
 *
 *  @author Divan
 */
void Desktop::Msg_ScreenResize(const ScreenResizeEvent& sr)
{
// Keep the following block the same as in ctrlGroup class:
	// F�r skalierte Desktops ist alles einfach, die brauchen im besten Fall gar nichts selbst implementieren
	if (scale)
	{
		//Zun�chst an die Kinder weiterleiten
		for(std::map<unsigned int,Window*>::iterator it = idmap.begin(); it != idmap.end(); ++it)
		if(it->second)
		{
			Window* ctrl = it->second;
			// unskalierte Position und Gr��e bekommen
			unsigned realx = ctrl->GetX() * 800 / sr.oldWidth;
			unsigned realy = ctrl->GetY() * 600 / sr.oldHeight;
			unsigned realwidth  = ctrl->GetWidth()  * 800 / sr.oldWidth;
			unsigned realheight = ctrl->GetHeight() * 600 / sr.oldHeight;
			// Rundungsfehler?
			if (realx * sr.oldWidth  / 800 < ctrl->GetX()) ++realx;
			if (realy * sr.oldHeight / 600 < ctrl->GetY()) ++realy;
			if (realwidth  * sr.oldWidth  / 800 < ctrl->GetWidth())  ++realwidth;
			if (realheight * sr.oldHeight / 600 < ctrl->GetHeight()) ++realheight;
			// Und los
			ctrl->Move(realx * sr.newWidth  / 800, realy * sr.newHeight / 600);
			ctrl->Msg_ScreenResize(sr);
			ctrl->Resize(realwidth * sr.newWidth / 800, realheight * sr.newHeight / 600);
		}
	}

	// Individuelle Reaktion ist auch erlaubt
	Resize(sr.newWidth, sr.newHeight);
}

