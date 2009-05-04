// $Id: WindowManager.h 4793 2009-05-04 15:37:10Z OLiver $
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
#ifndef WINDOWMANAGER_H_INCLUDED
#define WINDOWMANAGER_H_INCLUDED

#pragma once

#include "Singleton.h"
#include "Messages.h"
#include <string>
#include "VideoDriverLoaderInterface.h"

class Window;
class Desktop;
class IngameWindow;

/// Verwaltet alle (offenen) Fenster bzw Desktops samt ihren Controls und Messages
class WindowManager : public Singleton<WindowManager>, public VideoDriverLoaderInterface
{
	typedef list<IngameWindow*> IngameWindowList;                   ///< Fensterlistentyp
	typedef list<IngameWindow*>::iterator IngameWindowListIterator; ///< Fensterlistentypiterator

public:
	/// Konstruktor von @p WindowManager.
	WindowManager(void);
	/// Destruktor von @p WindowManager.
	~WindowManager(void);
	void CleanUp();

	/// Zeichnet Desktop und alle Fenster.
	void Draw(void);
	/// liefert ob der aktuelle Desktop den Focus besitzt oder nicht.
	bool IsDesktopActive(void);

	/// schickt eine Nachricht an das aktive Fenster bzw den aktiven Desktop.
	/// Sendet eine Tastaturnachricht an die Steuerelemente.
	void RelayKeyboardMessage(bool (Window::*msg)(const KeyEvent&),const KeyEvent& ke);
	/// Sendet eine Mausnachricht weiter an alle Steuerelemente
	void RelayMouseMessage(bool (Window::*msg)(const MouseCoords&),const MouseCoords& mc);

	/// �ffnet ein IngameWindow und f�gt es zur Fensterliste hinzu.
	void Show(IngameWindow *window, bool mouse = false);
	/// schliesst ein IngameWindow und entfernt es aus der Fensterliste.
	void Close(IngameWindow *window);
	/// Sucht ein Fenster mit der entsprechenden Fenster-ID und schlie�t es (falls es so eins gibt)
	void Close(unsigned int id);
	/// merkt einen Desktop zum Wechsel vor.
	void Switch(Desktop *desktop, void *data = NULL, bool mouse = false);
	/// Verarbeitung des Dr�ckens der Linken Maustaste.
	void Msg_LeftDown(const MouseCoords& mc);
	/// Verarbeitung des Loslassens der Linken Maustaste.
	void Msg_LeftUp(const MouseCoords& mc);
	/// Verarbeitung des Dr�ckens der Rechten Maustaste.
	void Msg_RightUp(const MouseCoords& mc);
	/// Verarbeitung des Loslassens der Rechten Maustaste.
	void Msg_RightDown(const MouseCoords& mc);
	/// Verarbeitung des Dr�ckens des Rad hoch.
	void Msg_WheelUpDown(const MouseCoords& mc);
	/// Verarbeitung des Loslassens des Rad hoch.
	void Msg_WheelUpUp(const MouseCoords& mc);
	/// Verarbeitung des Dr�ckens des Rad runter.
	void Msg_WheelDownDown(const MouseCoords& mc);
	/// Verarbeitung des Loslassens des Rad runter.
	void Msg_WheelDownUp(const MouseCoords& mc);
	/// Verarbeitung des Verschiebens der Maus.
	void Msg_MouseMove(const MouseCoords& mc);
	/// Verarbeitung Keyboard-Event
	void Msg_KeyDown(const KeyEvent& ke);
	// setzt den Tooltip
	void SetToolTip(Window *ttw, const std::string& tooltip);

protected:
	void DrawToolTip();

private:
	/// schliesst ein IngameWindow und entfernt es aus der Fensterliste.
	void Close(IngameWindowListIterator &it);
	/// wechselt einen Desktop
	void Switch(void);

private:
	Desktop *desktop;        ///< aktueller Desktop
	Desktop *nextdesktop;    ///< der n�chste Desktop
	void *nextdesktop_data;  ///< Daten f�r den n�chsten Desktop, welche dann MSG_SWITCH �bergeben werden
	bool disable_mouse;      ///< Mausdeaktivator, zum beheben des "Switch-Anschlie�end-Dr�ck-Bug"s

	IngameWindowList windows; ///< Fensterliste
	const MouseCoords *mc;
	std::string tooltip;
};

#endif // !WINDOWMANAGER_H_INCLUDED
