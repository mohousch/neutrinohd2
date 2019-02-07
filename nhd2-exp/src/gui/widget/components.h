/*
 * $Id: compenents.h 07.02.2019 mohousch Exp $
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

#ifndef __COMPONENTS_H__
#define __COMPONENTS_H__

#include <driver/framebuffer.h>


class CComponents
{
	private:
	public:
		CComponents(){};

		// frameBuffer stuff
		void paintBoxRel(const int x, const int y, const int dx, const int dy, fb_pixel_t col, int radius = 0, int type = CORNER_NONE, int mode = nogradient);

		bool paintIcon(const std::string & filename, const int x, const int y, const int h = 0, bool paint = true, int width = 0, int height = 0);

		bool displayImage(const std::string & name, int posx = 0, int posy = 0, int width = DEFAULT_XRES, int height = DEFAULT_YRES, CFrameBuffer::ScalingMode scaling = CFrameBuffer::COLOR, int x_pan = 0, int y_pan = 0, bool clearfb = false);

		bool displayLogo(t_channel_id channel_id, int posx, int posy, int width, int height, bool upscale = false, bool center_x = true, bool center_y = true);

		void paintBackground();
		void paintBackgroundBoxRel(int x, int y, int dx, int dy);
		bool loadBackgroundPic(const std::string& filename, bool show = true);
		void saveBackgroundImage(void); 
		void restoreBackgroundImage(void);
		void saveScreen(int x, int y, int dx, int dy, fb_pixel_t * const memp);
		void restoreScreen(int x, int y, int dx, int dy, fb_pixel_t * const memp);

		void paintVLineRel(int x, int y, int dy, const fb_pixel_t col);
		void paintHLineRel(int x, int dx, int y, const fb_pixel_t col);

		void paintFrameBox(const int x, const int y, const int dx, const int dy, const fb_pixel_t col);
};

#endif
