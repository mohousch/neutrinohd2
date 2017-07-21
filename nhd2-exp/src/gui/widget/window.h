/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: window.h 2016.12.12 11:43:30 mohousch Exp $

 	Homepage: http://dbox.cyberphoria.org/

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#if !defined(WINDOW_H)
#define WINDOW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <global.h>

#include <driver/framebuffer.h>
#include <system/settings.h>
#include <gui/color.h>


class CWindow  
{
	private:
		CBox cFrameBox;

		CFrameBuffer* frameBuffer;

		fb_pixel_t * background;
		int full_width;
		int full_height;

		int radius;
		int corner;
		fb_pixel_t bgcolor;
		int gradient;

		bool enableshadow;
		bool savescreen;

		void saveScreen();
		void restoreScreen();

	public:
		CWindow();
		CWindow(const int x, const int y, const int dx, const int dy);
		CWindow(CBox* position);
		~CWindow();

		void init(void);
		void setDimension(const int x, const int y, const int dx, const int dy);
		void setDimension(CBox* position);
		void setColor(fb_pixel_t col){bgcolor = col;};
		void setCorner(int ra, int co){radius = ra; corner = co;};
		void setGradient(int grad){gradient = grad;};
		void enableShadow(void){enableshadow = true;};
		void enableSaveScreen(void){savescreen = true;};

		void paint(void);
		void hide(void);
};

#endif
