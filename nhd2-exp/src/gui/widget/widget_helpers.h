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

#endif /* __gui_widget_helpers_h__ */
