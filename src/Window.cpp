// $Id: Window.cpp 4830 2009-05-07 18:59:21Z FloSoft $
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
#include "Window.h"

#include "Loader.h"
#include "controls.h"
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
 *  Konstruktor von @p Window.
 *
 *  @author OLiver
 */
Window::Window(void)
	: x(0), y(0), id(0), parent(NULL), active(false), visible(true), scale(false), tooltip("")
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Konstruktor von @p Window mit Parametern.
 *
 *  @param[in] x      X-Position des Fensters.
 *  @param[in] y      Y-Position des Fensters.
 *  @param[in] id     ID des Fensters
 *  @param[in] parent Handle auf das Parentfenster.
 *
 *  @author OLiver
 */
Window::Window(unsigned short x, 
			   unsigned short y, 
			   unsigned int id, 
			   Window *parent, 
			   const std::string& tooltip)
	: x(x), y(y), id(id), parent(parent), active(false), visible(true), scale(false), tooltip(tooltip)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  virtueller Destruktor von @p Window.
 *
 *  @author OLiver
 */
Window::~Window(void)
{
	// Steuerelemente aufr�umen
	for(std::map<unsigned int,Window*>::iterator it = idmap.begin();it != idmap.end();++it)
		delete it->second;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  zeichnet das Fenster.
 *
 *  @author OLiver
 */
bool Window::Draw(void)
{
	if(visible)
		return Draw_();

	return true;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  liefert die X-Koordinate.
 *
 *  @param[in] absolute Absolute Koordinate oder relative?
 *
 *  @return die X-Koordinate.
 *
 *  @author OLiver
 */
unsigned short Window::GetX(bool absolute) const
{
	if(!absolute)
		return x;

	unsigned short abs_x = x;
	const Window *temp = this;

	// Relative Koordinaten in absolute umrechnen
	// ( d.h. Koordinaten von allen Eltern zusammenaddieren )
	while(temp->parent)
	{
		temp = temp->parent;
		abs_x += temp->x;
	}

	return abs_x;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  liefert die Y-Koordinate.
 *
 *  @param[in] absolute Absolute Koordinate oder relative?
 *
 *  @return die Y-Koordinate.
 *
 *  @author OLiver
 */
unsigned short Window::GetY(bool absolute) const
{
	if(!absolute)
		return y;

	unsigned short abs_y = y;
	const Window *temp = this;

	// Relative Koordinaten in absolute umrechnen
	// ( d.h. Koordinaten von allen Eltern zusammenaddieren )
	while(temp->parent)
	{
		temp = temp->parent;
		abs_y += temp->y;
	}

	return abs_y;
}


///////////////////////////////////////////////////////////////////////////////
/**
 *  Sendet eine Fensternachricht an die Steuerelemente.
 *
 *  @param[in] msg   Die Nachricht.
 *  @param[in] id    Die ID des Quellsteuerelements.
 *  @param[in] param Ein nachrichtenspezifischer Parameter.
 *
 *  @author OLiver
 */
bool Window::RelayKeyboardMessage(bool (Window::*msg)(const KeyEvent&),const KeyEvent& ke)
{
	// Alle Controls durchgehen
	// Falls das Fenster dann pl�tzlich nich mehr aktiv ist (z.b. neues Fenster ge�ffnet, sofort abbrechen!)
	for(std::map<unsigned int,Window*>::iterator it = idmap.begin(); it != idmap.end() && active; ++it)
	{
		if(it->second->visible && it->second->active)
			if((it->second->*msg)(ke))
				return true;
	}

	return false;
}

bool Window::RelayMouseMessage(bool (Window::*msg)(const MouseCoords&),const MouseCoords& mc)
{
	bool processed = false;

	// Alle Controls durchgehen
	// Falls das Fenster dann pl�tzlich nich mehr aktiv ist (z.b. neues Fenster ge�ffnet, sofort abbrechen!)
	for(std::map<unsigned int,Window*>::iterator it = idmap.begin(); it != idmap.end() && active; ++it)
	{
		if(locked_areas.size())
			if(TestWindowInRegion(it->second, mc))
				continue;

		if(it->second->visible && it->second->active)
			// Falls von einem Steuerelement verarbeitet --> abbrechen
			if((it->second->*msg)(mc))
			{
				processed = true;
				break;
			}
	}

	// Nur vorl�ufig
	if(processed && msg == &Window::Msg_LeftDown)
	{
		for(std::map<unsigned int,Window*>::iterator it = idmap.begin(); it != idmap.end() && active; ++it)
		{
			if(locked_areas.size())
				if(TestWindowInRegion(it->second, mc))
					continue;

			if(it->second->visible && it->second->active)
				// Falls von einem Steuerelement verarbeitet --> abbrechen
				it->second->Msg_LeftDown_After(mc);

		}
	}

	return processed;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  aktiviert das Fenster.
 *
 *  @param[in] activate Fenster aktivieren?
 *
 *  @author OLiver
 */
void Window::SetActive(bool activate)
{
	this->active = activate;
	ActivateControls(activate);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  aktiviert die Steuerelemente des Fensters.
 *
 *  @param[in] activate Steuerelemente aktivieren?
 *
 *  @author OLiver
 */
void Window::ActivateControls(bool activate)
{
	for(std::map<unsigned int,Window*>::iterator it = idmap.begin(); it != idmap.end(); ++it)
		it->second->SetActive(activate);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  sperrt eine Region eines Fensters.
 *
 *  @param[in] window das Fenster, welches die Region sperrt.
 *  @param[in] rect   das Rechteck, welches die Region beschreibt.
 *
 *  @author OLiver
 */
void Window::LockRegion(Window *window, const Rect &rect)
{
	LockedRegion lg = {window, rect};
	locked_areas.push_back(lg);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Gibt eine gesperrte Region wieder frei.
 *
 *  @param[in] window das Fenster, welches die Region sperrt.
 *
 *  @author OLiver
 */
void Window::FreeRegion(Window *window)
{
	for(list<LockedRegion>::iterator it = locked_areas.begin(); it.valid(); ++it)
	{
		if(window == it->window)
		{
			locked_areas.erase(it);
			return;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�gt ein BuildingIcon hinzu.
 *
 *  @author OLiver
 */
ctrlBuildingIcon *Window::AddBuildingIcon(unsigned int id, 
										  unsigned short x, 
										  unsigned short y, 
										  BuildingType type,
										  const Nation nation,
										  unsigned short size,
										  const char *tooltip)
{
	if(scale)
	{
		x = ScaleX(x);
		y = ScaleY(y);
	}

	return AddCtrl(id, new ctrlBuildingIcon(this, id, x, y, type, nation, size, tooltip));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�gt einen ButtonCtrl hinzu.
 *
 *  @param[in] x      X-Koordinate des Steuerelements
 *  @param[in] y      Y-Koordinate des Steuerelements
 *  @param[in] width  Breite des Steuerelements
 *  @param[in] height H�he des Steuerelements
 *  @param[in] tc     Farbe des Steuerelements
 *  @param[in] type   Typ des Steuerelements (@p false f�r Text, @p true f�r Bild)
 *  @param[in] text   Text des Buttons (nur f�r @p type gleich @p false)
 *  @param[in] font   Schrift des Buttons (nur f�r @p type gleich @p false)
 *  @param[in] image  Bild des Buttons (nur f�r @p type gleich @p true)
 *  @param[in] border Soll der Button einen Rahmen haben?
 *
 *  @author OLiver
 */


/// f�gt einen Text-ButtonCtrl hinzu.
ctrlTextButton *Window::AddTextButton(unsigned int id, unsigned short x, unsigned short y,unsigned short width, unsigned short height, const TextureColor tc,const std::string& text,  glArchivItem_Font *font, const std::string& tooltip)
{
	if(scale)
	{
		x = ScaleX(x);
		y = ScaleY(y);
		width = ScaleX(width);
		height = ScaleY(height);
	}

	return AddCtrl(id, new ctrlTextButton(this, id, x, y, width, height, tc, text,font,tooltip));
}


/// f�gt einen Image-ButtonCtrl hinzu.
ctrlImageButton *Window::AddImageButton(unsigned int id, unsigned short x, unsigned short y,unsigned short width, unsigned short height, const TextureColor tc,glArchivItem_Bitmap * const image,  const std::string& tooltip)
{
	if(scale)
	{
		x = ScaleX(x);
		y = ScaleY(y);
		width = ScaleX(width);
		height = ScaleY(height);
	}

	return AddCtrl(id, new ctrlImageButton(this, id, x, y, width, height, tc, image,tooltip));
}


///////////////////////////////////////////////////////////////////////////////
/**
 *  f�gt ein ChatCtrl hinzu.
 *
 *  @author Devil
 */
ctrlChat *Window::AddChatCtrl(unsigned int id,
							  unsigned short x, 
							  unsigned short y,
							  unsigned short width,
							  unsigned short height, 
							  TextureColor tc, 
							  glArchivItem_Font *font)
{
	if(scale)
	{
		x = ScaleX(x);
		y = ScaleY(y);
		width = ScaleX(width);
		height = ScaleY(height);
	}

	return AddCtrl(id, new ctrlChat(this, id, x, y, width, height, tc, font));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�gt eine Checkbox hinzu.
 *
 *  @author OLiver
 */
ctrlCheck *Window::AddCheckBox(unsigned int id,
							   unsigned short x, 
							   unsigned short y,
							   unsigned short width, 
							   unsigned short height,
							   TextureColor tc, 
							   const std::string& text, 
							   glArchivItem_Font *font, 
							   bool readonly)
{
	if(scale)
	{
		x = ScaleX(x);
		y = ScaleY(y);
		width = ScaleX(width);
		height = ScaleY(height);
	}

	return AddCtrl(id, new ctrlCheck(this, id, x, y, width, height, tc, text, font, readonly));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�gt eine Combobox hinzu.
 *
 *  @author OLiver
 */
ctrlComboBox *Window::AddComboBox(unsigned int id,
								  unsigned short x, 
								  unsigned short y,
								  unsigned short width,
								  unsigned short height,
								  TextureColor tc, 
								  glArchivItem_Font *font,
								  unsigned short list_height,
								  bool readonly)
{
	if(scale)
	{
		x = ScaleX(x);
		y = ScaleY(y);
		width = ScaleX(width);
		height = ScaleY(height);
		list_height = ScaleY(list_height);
	}

	return AddCtrl(id, new ctrlComboBox(this, id, x, y, width, height, tc, font, list_height, readonly));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�gt ein vertieftes TextCtrl hinzu.
 *
 *  @author OLiver
 */
ctrlDeepening *Window::AddDeepening(unsigned int id,
									unsigned short x,
									unsigned short y,
									unsigned short width, 
									unsigned short height, 
									TextureColor tc, 
									const std::string& text, 
									glArchivItem_Font *font, 
									unsigned int color)
{
	if(scale)
	{
		x = ScaleX(x);
		y = ScaleY(y);
		width = ScaleX(width);
		height = ScaleY(height);
	}

	return AddCtrl(id, new ctrlDeepening(this, id, x, y, width, height, tc, text, font, color));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�gt ein EditCtrl hinzu.
 *
 *  @author OLiver
 */
ctrlEdit *Window::AddEdit(unsigned int id,
						  unsigned short x, 
						  unsigned short y, 
						  unsigned short width, 
						  unsigned short height, 
						  TextureColor tc, 
						  glArchivItem_Font *font,
						  unsigned short maxlength,
						  bool password, 
						  bool disabled,
						  bool notify)
{
	if(scale)
	{
		x = ScaleX(x);
		y = ScaleY(y);
		width = ScaleX(width);
		height = ScaleY(height);
	}

	return AddCtrl(id, new ctrlEdit(this, id, x, y, width, height, tc, font, maxlength, password, disabled, notify));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�gt eine Gruppe hinzu.
 *
 *  @author FloSoft
 */
ctrlGroup *Window::AddGroup(unsigned int id, bool scale)
{
	return AddCtrl(id, new ctrlGroup(this, id, scale));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�gt ein ImageCtrl hinzu.
 *
 *  @author OLiver
 */
ctrlImage *Window::AddImage(unsigned int id,
							unsigned short x,
							unsigned short y,
							glArchivItem_Bitmap *image, const std::string& tooltip)
{
	if(scale)
	{
		x = ScaleX(x);
		y = ScaleY(y);
	}

	return AddCtrl(id, new ctrlImage(this, id, x, y, image, tooltip));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�gt ein ListCtrl hinzu.
 *
 *  @author OLiver
 */
ctrlList *Window::AddList(unsigned int id,
						  unsigned short x,
						  unsigned short y,
						  unsigned short width,
						  unsigned short height,
						  TextureColor tc,
						  glArchivItem_Font *font)
{
	if(scale)
	{
		x = ScaleX(x);
		y = ScaleY(y);
		width = ScaleX(width);
		height = ScaleY(height);
	}

	return AddCtrl(id, new ctrlList(this, id, x, y, width, height, tc, font));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�gt ein mehrzeiliges TextCtrl hinzu.
 *
 *  @author Devil
 */
ctrlMultiline *Window::AddMultiline(unsigned int id,
									unsigned short x, 
								    unsigned short y, 
								    unsigned short width, 
								    unsigned short height, 
								    TextureColor tc, 
								    glArchivItem_Font *font, 
									unsigned int format)
{
	if(scale)
	{
		x = ScaleX(x);
		y = ScaleY(y);
		width = ScaleX(width);
		height = ScaleY(height);
	}

	return AddCtrl(id, new ctrlMultiline(this, id, x, y, width, height, tc, font, format));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�gt ein OptionenGruppe hinzu.
 *
 *  @param[in] id          ID des Steuerelements
 *  @param[in] select_type Typ der Auswahl
 *
 *  @return Instanz das Steuerelement.
 *
 *  @author OLiver
 */
ctrlOptionGroup *Window::AddOptionGroup(unsigned int id,
										int select_type,
										bool scale)
{
	return AddCtrl(id, new ctrlOptionGroup(this, id, select_type, scale));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�gt eine prozentuale ProgressBar hinzu.
 *
 *  @author OLiver
 */
ctrlPercent *Window::AddPercent(unsigned int id,
								unsigned short x,
								unsigned short y, 
								unsigned short width,
								unsigned short height,
								TextureColor tc, 
								unsigned int text_color, 
								glArchivItem_Font *font,
								const unsigned short *percentage)
{
	if(scale)
	{
		x = ScaleX(x);
		y = ScaleY(y);
		width = ScaleX(width);
		height = ScaleY(height);
	}

	return AddCtrl(id, new ctrlPercent(this, id, x, y, width, height, tc, text_color, font, percentage));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�gt eine ProgressBar hinzu.
 *
 *  @author OLiver
 */
ctrlProgress *Window::AddProgress(unsigned int id,
								  unsigned short x,
								  unsigned short y,
								  unsigned short width,
								  unsigned short height,
								  TextureColor tc,
								  unsigned short button_minus,
								  unsigned short button_plus,
								  unsigned short maximum,
								  const std::string& tooltip,
								  unsigned short x_padding,
								  unsigned short y_padding,
								  unsigned int force_color,
								  const std::string& button_minus_tooltip,
								  const std::string& button_plus_tooltip)
{
	if(scale)
	{
		x = ScaleX(x);
		y = ScaleY(y);
		width = ScaleX(width);
		height = ScaleY(height);
	}

	return AddCtrl(id, new ctrlProgress(this, id, x, y, width, height, tc, button_minus, button_plus, maximum, x_padding, y_padding, force_color, tooltip, button_minus_tooltip, button_plus_tooltip));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�gt eine Scrollbar hinzu.
 *
 *  @author OLiver
 */
ctrlScrollBar *Window::AddScrollBar(unsigned int id,
							  unsigned short x, 
							  unsigned short y,
							  unsigned short width, 
							  unsigned short height, 
							  unsigned short button_height, 
							  TextureColor tc, 
							  unsigned short page_size)
{
	if(scale)
	{
		x = ScaleX(x);
		y = ScaleY(y);
		width = ScaleX(width);
		height = ScaleY(height);
		button_height = ScaleY(button_height);
	}

	return AddCtrl(id, new ctrlScrollBar(this, id, x, y, width, height, button_height, tc, page_size));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�gt ein TabCtrl hinzu.
 *
 *  @author OLiver
 */
ctrlTab *Window::AddTabCtrl(unsigned int id,
						unsigned short x, 
						unsigned short y, 
						unsigned short width)
{
	if(scale)
	{
		x = ScaleX(x);
		y = ScaleY(y);
		width = ScaleX(width);
	}

	return AddCtrl(id, new ctrlTab(this, id, x, y, width));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�gt eine Tabelle hinzu.
 *
 *  @author OLiver
 */
ctrlTable *Window::AddTable(unsigned int id,
							unsigned short x, 
							unsigned short y, 
							unsigned short width, 
							unsigned short height, 
							TextureColor tc, 
							glArchivItem_Font *font, 
							unsigned int columns, 
							...)
{
	ctrlTable *ctrl;
	va_list liste;
	va_start(liste, columns);

	if(scale)
	{
		x = ScaleX(x);
		y = ScaleY(y);
		width = ScaleX(width);
		height = ScaleY(height);
	}

	ctrl = new ctrlTable(this, id, x, y, width, height, tc, font, columns, liste);
	
	va_end(liste);

	return AddCtrl(id, ctrl);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�gt einen Timer hinzu.
 *
 *  @author FloSoft
 */
ctrlTimer *Window::AddTimer(unsigned int id, unsigned int timeout)
{
	return AddCtrl(id, new ctrlTimer(this, id, timeout));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�gt ein TextCtrl hinzu.
 *
 *  @param[in] x      X-Koordinate des Steuerelements
 *  @param[in] y      Y-Koordinate des Steuerelements
 *  @param[in] text   Text
 *  @param[in] color  Textfarbe
 *  @param[in] format Formatierung des Textes
 *                      @p 0    - Text links ( standard )
 *                      @p glArchivItem_Font::DF_CENTER  - Text mittig
 *                      @p glArchivItem_Font::DF_RIGHT   - Text rechts
 *                      @p glArchivItem_Font::DF_TOP     - Text oben ( standard )
 *                      @p glArchivItem_Font::DF_VCENTER - Text vertikal zentriert
 *                      @p glArchivItem_Font::DF_BOTTOM  - Text unten
 *  @param[in] font   Schriftart
 *
 *  @author OLiver
 */
ctrlText *Window::AddText(unsigned int id, 
						  unsigned short x,
						  unsigned short y, 
						  const std::string& text, 
						  unsigned int color, 
						  unsigned int format, 
						  glArchivItem_Font *font)
{
	if(scale)
	{
		x = ScaleX(x);
		y = ScaleY(y);
	}

	return AddCtrl(id, new ctrlText(this, id, x, y, text, color, format, font));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�gt ein vertieftes variables TextCtrl hinzu.
 *
 *  @author FloSoft
 */
ctrlVarDeepening *Window::AddVarDeepening(unsigned int id, 
									 unsigned short x, 
									 unsigned short y, 
									 unsigned short width, 
									 unsigned short height, 
									 TextureColor tc, 
									 const std::string& formatstr,  
									 glArchivItem_Font *font,
									 unsigned int color,
									 unsigned int parameters, 
									 ...)
{
	ctrlVarDeepening *ctrl;
	va_list liste;
	va_start(liste, parameters);

	if(scale)
	{
		x = ScaleX(x);
		y = ScaleY(y);
		width = ScaleX(width);
		height = ScaleY(height);
	}

	ctrl = new ctrlVarDeepening(this, id, x, y, width, height, tc, formatstr, font, color, parameters, liste);
	
	va_end(liste);

	return AddCtrl(id, ctrl);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  f�gt ein variables TextCtrl hinzu.
 *
 *  @param[in] x          X-Koordinate des Steuerelements
 *  @param[in] y          Y-Koordinate des Steuerelements
 *  @param[in] formatstr  Der Formatstring des Steuerelements
 *  @param[in] color      Textfarbe
 *  @param[in] format     Formatierung des Textes
 *                          @p 0    - Text links ( standard )
 *                          @p glArchivItem_Font::DF_CENTER  - Text mittig
 *                          @p glArchivItem_Font::DF_RIGHT   - Text rechts
 *                          @p glArchivItem_Font::DF_TOP     - Text oben ( standard )
 *                          @p glArchivItem_Font::DF_VCENTER - Text vertikal zentriert
 *                          @p glArchivItem_Font::DF_BOTTOM  - Text unten
 *  @param[in] font       Schriftart
 *  @param[in] parameters Anzahl der nachfolgenden Parameter
 *  @param[in] ...        die variablen Parameter
 *
 *  @author OLiver
 */
ctrlVarText *Window::AddVarText(unsigned int id, 
								unsigned short x, 
								unsigned short y, 
								 const std::string& formatstr, 
								unsigned int color, 
								unsigned int format, 
								glArchivItem_Font *font, 
								unsigned int parameters, 
								...)
{
	ctrlVarText *ctrl;
	va_list liste;
	va_start(liste, parameters);

	if(scale)
	{
		x = ScaleX(x);
		y = ScaleY(y);
	}

	ctrl = new ctrlVarText(this, id, x, y, formatstr, color, format, font, parameters, liste);
	
	va_end(liste);

	return AddCtrl(id, ctrl);
}

ctrlPreviewMinimap *Window::AddPreviewMinimap(const unsigned id,
				 unsigned short x, 
				 unsigned short y, 
				  unsigned short width, 
				  unsigned short height,
				 glArchivItem_Map * const map)
{
	if(scale)
	{
		x = ScaleX(x);
		y = ScaleY(y);
		width = ScaleX(width);
		height = ScaleY(height);
	}

	return AddCtrl(id, new ctrlPreviewMinimap(this, id, x, y, width, height, map));
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  Zeichnet einen 3D-Rahmen.
 *
 *  @author OLiver
 */
void Window::Draw3D(const unsigned short x, 
			const unsigned short y, 
			const unsigned short width, 
			const unsigned short height,
			const TextureColor tc,
			const unsigned short type,
			const bool illuminated,
			const bool draw_content)
{
	if(width < 4 || height < 4)
		return;

	if(type <= 1)
	{
		// �u�erer Rahmen
		GetImage(io_dat, 12 + tc)->Draw(x, y, width, 2,      0, 0, width, 2);
		GetImage(io_dat, 12 + tc)->Draw(x, y, 2,     height, 0, 0, 2,     height);

		if(illuminated)
		{
			// Modulate2x anmachen
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
			glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 2.0f);
		}

		// Inhalt der Box
		if(draw_content)
		{
			if(type)
				GetImage(io_dat, tc*2)->Draw(x+2,y+2,width-4,height-4,0, 0, width-4,height-4);
			else
				GetImage(io_dat, tc*2+1)->Draw(x+2,y+2,width-4,height-4,0, 0, width-4,height-4);
		}

		if(illuminated)
		{
			// Modulate2x wieder ausmachen
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}

		glDisable(GL_TEXTURE_2D);

		glColor3f(0.0f,0.0f,0.0f);

		glBegin(GL_QUADS);

		glVertex2i(x+width-1,y);
		glVertex2i(x+width-1,y+height);
		glVertex2i(x+width,y+height);
		glVertex2i(x+width,y);

		glVertex2i(x+width-2,y+1);
		glVertex2i(x+width-2,y+height);
		glVertex2i(x+width-1,y+height);
		glVertex2i(x+width-1,y+1);

		glVertex2i(x,y+height-1);
		glVertex2i(x,y+height);
		glVertex2i(x+width-2,y+height);
		glVertex2i(x+width-2,y+height-1);

		glVertex2i(x+1,y+height-2);
		glVertex2i(x+1,y+height-1);
		glVertex2i(x+width-2,y+height-1);
		glVertex2i(x+width-2,y+height-2);

		glEnd();
	}
	else
	{
		GetImage(io_dat,12+tc)->Draw(x,y+height-2,width,2,0, 0, width,2);
		GetImage(io_dat,12+tc)->Draw(x+width-2,y, 2,height,0, 0,2,height);

		if(illuminated)
		{
			// Modulate2x anmachen
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
			glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 2.0f);
		}

		GetImage(io_dat,tc*2+1)->Draw(x+2,y+2,width-4,2,0, 0, width-4,2);
		GetImage(io_dat,tc*2+1)->Draw(x+2,y+2,2,height-4,0, 0, 2,height-4);

		GetImage(io_dat,tc*2+1)->Draw(x+4,y+4,width-6,height-6,0, 0, width-6,height-6);

		if(illuminated)
		{
			// Modulate2x wieder ausmachen
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}

		glDisable(GL_TEXTURE_2D);

		glColor3f(0.0f,0.0f,0.0f);

		glBegin(GL_QUADS);

		glVertex2i(x,y);
		glVertex2i(x,y+1);
		glVertex2i(x+width,y+1);
		glVertex2i(x+width,y);

		glVertex2i(x,y+1);
		glVertex2i(x,y+2);
		glVertex2i(x+width-1,y+2);
		glVertex2i(x+width-1,y+1);

		glVertex2i(x,y+2);
		glVertex2i(x,y+height);
		glVertex2i(x+1,y+height);
		glVertex2i(x+1,y+2);

		glVertex2i(x+1,y+2);
		glVertex2i(x+1,y+height-1);
		glVertex2i(x+2,y+height-1);
		glVertex2i(x+2,y+2);

		glEnd();
	}
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  zeichnet ein Rechteck.
 *
 *  @param[in] x X-Koordinate
 *
 *  @author OLiver
 */
void Window::DrawRectangle(unsigned short x, unsigned short y, unsigned short width, unsigned short height, unsigned int color)
{
	glDisable(GL_TEXTURE_2D);

	glColor4ub(GetRed(color), GetGreen(color), GetBlue(color), GetAlpha(color));

	glBegin(GL_QUADS);
	glVertex2i(x, y);
	glVertex2i(x, y+height);
	glVertex2i(x+width, y+height);
	glVertex2i(x+width, y);
	glEnd();

	glEnable(GL_TEXTURE_2D);
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  zeichnet die Steuerelemente.
 *
 *  @author OLiver
 */
void Window::DrawControls(void)
{
	for(std::map<unsigned int,Window*>::iterator it = idmap.begin(); it != idmap.end(); ++it)
	{
		Window *control = it->second;
		assert(control);

		control->Msg_PaintBefore();
		control->Draw();
	}

	for(std::map<unsigned int,Window*>::iterator it = idmap.begin(); it != idmap.end(); ++it)
	{
		Window *control = it->second;
		assert(control);

		control->Msg_PaintAfter();
	}
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  pr�ft ob Mauskoordinaten in einer gesperrten Region liegt.
 *
 *  @param[in] window das Fenster, welches die Region sperrt.
 *  @param[in] mc     Mauskoordinaten.
 *
 *  @return @p true falls Mausposition innerhalb der gesperrten Region,
 *          @p false falls au�erhalb
 *
 *  @author OLiver
 */
bool Window::TestWindowInRegion(Window *window, const MouseCoords& mc)
{
	for(list<LockedRegion>::iterator it = locked_areas.begin(); it.valid(); ++it)
	{
		if(it->window != window && Coll(mc.x + GetX(), mc.y + GetY(), it->rect))
			return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
/*
 *  skaliert einen Wert.
 *
 *  @author FloSoft
 */
unsigned short Window::ScaleX(const unsigned short val)
{
	return  val*VideoDriverWrapper::inst().GetScreenWidth()/800;
}

unsigned short Window::ScaleY(const unsigned short val)
{
	return val*VideoDriverWrapper::inst().GetScreenHeight()/600;
}


///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_PaintBefore() 
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_PaintAfter()
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
bool Window::Msg_LeftDown(const MouseCoords& mc)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
bool Window::Msg_RightDown(const MouseCoords& mc)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
bool Window::Msg_LeftUp(const MouseCoords& mc)
{
	return false;
}

bool Window::Msg_LeftDown_After(const MouseCoords& mc)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
bool Window::Msg_RightUp(const MouseCoords& mc)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author Divan
 */
bool Window::Msg_WheelUp(const MouseCoords& mc)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *
 *
 *  @author Divan
 */
bool Window::Msg_WheelDown(const MouseCoords& mc)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
bool Window::Msg_MouseMove(const MouseCoords& mc)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
bool Window::Msg_KeyDown(const KeyEvent& ke)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_ButtonClick(const unsigned int ctrl_id)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_EditEnter(const unsigned int ctrl_id)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_EditChange(const unsigned int ctrl_id)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_TabChange(const unsigned int ctrl_id, const unsigned short tab_id)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_ListSelectItem(const unsigned int ctrl_id, const unsigned short selection)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_ComboSelectItem(const unsigned int ctrl_id, const unsigned short selection)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_CheckboxChange(const unsigned int ctrl_id, const bool checked)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_ProgressChange(const unsigned int ctrl_id, const unsigned short position)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_ScrollShow(const unsigned int ctrl_id, const bool visible)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_OptionGroupChange(const unsigned int ctrl_id, const unsigned short selection)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_Timer(const unsigned int ctrl_id)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_TableSelectItem(const unsigned int ctrl_id, const unsigned short selection)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_TableRightButton(const unsigned int ctrl_id, const unsigned short selection)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_TableLeftButton(const unsigned int ctrl_id, const unsigned short selection)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_MsgBoxResult(const unsigned msgbox_id, const MsgboxResult mbr) 
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_Group_ButtonClick(const unsigned int group_id, const unsigned int ctrl_id)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_Group_EditEnter(const unsigned int group_id, const unsigned int ctrl_id)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_Group_EditChange(const unsigned int group_id, const unsigned int ctrl_id)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_Group_TabChange(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short tab_id)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_Group_ListSelectItem(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short selection)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_Group_ComboSelectItem(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short selection)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_Group_CheckboxChange(const unsigned int group_id, const unsigned int ctrl_id, const bool checked)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_Group_ProgressChange(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short position)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_Group_ScrollShow(const unsigned int group_id, const unsigned int ctrl_id, const bool visible)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_Group_OptionGroupChange(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short selection)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_Group_Timer(const unsigned int group_id, const unsigned int ctrl_id)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_Group_TableSelectItem(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short selection)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_Group_TableRightButton(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short selection)
{
}

///////////////////////////////////////////////////////////////////////////////
/**
 *  
 *
 *  @author OLiver
 */
void Window::Msg_Group_TableLeftButton(const unsigned int group_id, const unsigned int ctrl_id, const unsigned short selection)
{
}
