/*
 * $Id: headers.h 2018/08/19 mohousch Exp $
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

#ifndef __HEADERS_H__
#define __HEADERS_H__

#include <driver/fontrenderer.h>
#include <driver/framebuffer.h>

#include <gui/widget/buttons.h>

#include <system/settings.h>


class CHeaders
{
	private:
		// head
		fb_pixel_t bgcolor;
		int radius;
		int corner;
		int gradient;
		bool paintDate;
		bool logo;
		int hbutton_count;
		const struct button_label* hbutton_labels;

		// foot
		fb_pixel_t fbgcolor;
		int fradius;
		int fcorner;
		int fgradient;
	
	public:
		CHeaders();

		// head
		void paintHead(int x, int y, int dx, int dy, const char* icon, const neutrino_locale_t caption);
		void paintHead(int x, int y, int dx, int dy, const char* icon, const std::string caption);

		void paintHead(CBox position, const char* icon, const neutrino_locale_t caption);
		void paintHead(CBox position, const char* icon, const std::string caption);

		void setColor(fb_pixel_t col){bgcolor = col;};
		void setCorner(int ra = NO_RADIUS, int co = CORNER_NONE){radius = ra; corner = co;};
		void setGradient(int grad){gradient = grad;};
		void enablePaintDate(void){paintDate = true;};
		void setHeaderButtons(const struct button_label* _hbutton_label, const int _hbutton_count);
		void enableLogo(void){logo = true;};

		// foot
		void paintFoot(int x, int y, int dx, int dy, const unsigned int count = 0, const struct button_label * const content = NULL);
		void paintFoot(CBox position, const unsigned int count = 0, const struct button_label * const content = NULL);

		void setFootColor(fb_pixel_t col){fbgcolor = col;};
		void setFootCorner(int ra = NO_RADIUS, int co = CORNER_NONE){fradius = ra; fcorner = co;};
		void setFootGradient(int grad){fgradient = grad;};
};

#endif
