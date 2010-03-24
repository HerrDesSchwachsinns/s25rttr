// $Id: WinAPI.h 6179 2010-03-24 14:52:38Z FloSoft $
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
#ifndef WINAPI_H_INCLUDED
#define WINAPI_H_INCLUDED

#pragma once

#include <VideoDriver.h>

/// Klasse f�r den WinAPI Videotreiber.
class VideoWinAPI : public VideoDriver
{
public:
	/// Konstruktor von @p VideoWinAPI.
	VideoWinAPI(VideoDriverLoaderInterface * CallBack);

	/// Destruktor von @p VideoWinAPI.
	~VideoWinAPI(void);

	/// Funktion zum Auslesen des Treibernamens.
	const char *GetName(void) const;

	/// Treiberinitialisierungsfunktion.
	bool Initialize(void);

	/// Treiberaufr�umfunktion.
	void CleanUp(void);

	/// Erstellt das Fenster mit entsprechenden Werten.
	bool CreateScreen(unsigned short width, unsigned short height, const bool fullscreen);

	/// Erstellt oder ver�ndert das Fenster mit entsprechenden Werten.
	bool ResizeScreen(unsigned short width, unsigned short height, const bool fullscreen);

	/// Schliesst das Fenster.
	void DestroyScreen(void);

	/// Wechselt die OpenGL-Puffer.
	bool SwapBuffers(void);

	/// Die Nachrichtenschleife.
	bool MessageLoop(void);

	/// Funktion zum Auslesen des TickCounts.
	unsigned long GetTickCount(void) const;

	/// Funktion zum Holen einer Subfunktion.
	void *GetFunction(const char *function) const;

	/// Listet verf�gbare Videomodi auf
	void ListVideoModes(std::vector<VideoMode>& video_modes) const;

	/// Funktion zum Setzen der Mauskoordinaten.
	void SetMousePos(int x, int y);

	/// Funktion zum Setzen der X-Koordinate der Maus.
	void SetMousePosX(int x);

	/// Funktion zum Setzen der Y-Koordinate der Maus.
	void SetMousePosY(int y);

	/// Get state of the modifier keys
	KeyEvent GetModKeyState(void) const;

	/// Gibt Pointer auf ein Fenster zur�ck (device-dependent!), HWND unter Windows
	void * GetWindowPointer() const;

private:
	/// Funktion zum Senden einer gedr�ckten Taste.
	void OnWMChar(char c, bool disablepaste = false);
	void OnWMKeyDown(unsigned char c);

	/// Funktion zum Pasten von Text aus dem Clipboard.
	void OnWMPaste();

	/// Callbackfunktion der WinAPI.
	static LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	bool mouse_l;    ///< Status der Linken Maustaste.
	bool mouse_r;    ///< Status der Rechten Maustaste.
	int mouse_z;     ///< Scrolling position for mousewheel. 
	DEVMODE dm_prev; ///< Bildschirmmodus.
	HWND  screen;    ///< Fensterhandle.
	HDC   screen_dc; ///< Zeichenkontext des Fensters.
	HGLRC screen_rc; ///< OpenGL-Kontext des Fensters.
};

#endif // !WINAPI_H_INCLUDED
