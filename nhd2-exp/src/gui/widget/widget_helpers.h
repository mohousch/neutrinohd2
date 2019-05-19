/*
 * $Id: buttons.h 2016/01/12 mohousch Exp $
 *
 * (C) 2003 by thegoodguy <thegoodguy@berlios.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __gui_widget_helpers_h__
#define __gui_widget_helpers_h__

#include <driver/fontrenderer.h>
#include <driver/framebuffer.h>

#include <system/localize.h>
#include <system/settings.h>


// buttons
typedef struct button_label
{
	const char * button;
	neutrino_locale_t locale;
	const char * localename;
} button_label_struct;


class CButtons
{
	private:
	public:
		CButtons();

		void paintFootButtons(const int x, const int y, const int dx, const int dy, const unsigned int count, const struct button_label* const content);

		// head buttons right
		void paintHeadButtons(const int x, const int y, const int dx, const int dy, const unsigned int count, const struct button_label * const content);
};

//scrollBar
class CScrollBar
{
	private:
	public:
		CScrollBar(){};
		virtual ~CScrollBar(){};

		void paint(const int x, const int y, const int dy, const int NrOfPages, const int CurrentPage);
		void paint(CBox* position, const int NrOfPages, const int CurrentPage);
};

// detailsLine
class CItems2DetailsLine
{
	private:
	public:
		CItems2DetailsLine(){};
		virtual ~CItems2DetailsLine(){};
		
		void paint(int x, int y, int width, int height, int info_height, int iheight, int iy);
		void clear(int x, int y, int width, int height, int info_height);
};

// headers
class CHeaders
{
	private:
		// head
		//CBox headBox;
		fb_pixel_t bgcolor;
		int radius;
		int corner;
		int gradient;
		bool paintDate;
		bool logo;
		int hbutton_count;
		const struct button_label* hbutton_labels;

		// foot
		CBox footBox;
		fb_pixel_t fbgcolor;
		int fradius;
		int fcorner;
		int fgradient;
	
	public:
		CHeaders();
		virtual ~CHeaders(){};

		// head
		void setHeadColor(fb_pixel_t col){bgcolor = col;};
		void setHeadCorner(int ra = NO_RADIUS, int co = CORNER_NONE){radius = ra; corner = co;};
		void setHeadGradient(int grad){gradient = grad;};
		void enablePaintDate(void){paintDate = true;};
		void setHeaderButtons(const struct button_label* _hbutton_label, const int _hbutton_count);
		void enableLogo(void){logo = true;};

		void paintHead(const int x, const int y, const int dx, const int dy, const char * const title, const char * const icon = NULL);
		void paintHead(CBox position, const char * const title, const char * const icon = NULL);

		// foot
		void paintFoot(int x, int y, int dx, int dy, const unsigned int count = 0, const struct button_label * const content = NULL);
		void paintFoot(CBox position, const unsigned int count = 0, const struct button_label * const content = NULL);

		void setFootColor(fb_pixel_t col){fbgcolor = col;};
		void setFootCorner(int ra = NO_RADIUS, int co = CORNER_NONE){fradius = ra; fcorner = co;};
		void setFootGradient(int grad){fgradient = grad;};
};

#endif /* __gui_widget_helpers_h__ */
