#ifndef __fb_window_h__
#define __fb_window_h__
/*
 * $Id: fb_window.h 2013/10/12 mohousch Exp $
 *
 * abstract fb_window class - d-box2 linux project
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

#include <driver/fontrenderer.h>
#include <driver/framebuffer.h>

class CFBWindow
{
	public:
		typedef unsigned int color_t;
		typedef void *         font_t;
		
	private:
		CFrameBuffer	* frameBuffer;
		fb_pixel_t	* Background;

	public:
		int x, y;   /* upper left corner */
		int dx, dy; /* dimension         */

		CFBWindow(const int _x, const int _y, const int _dx, const int _dy);
		~CFBWindow();

		void paintBoxRel(const int _x, const int _y, const int _dx, const int _dy, const color_t _col, int radius = 0, int type = CORNER_NONE, int mode = nogradient);
		void paintIcon(const char * const _filename, const int _x, const int _y, const int _dy = 0, bool paint = true, const int iw = 0, const int ih = 0);
		void RenderString(const font_t _font, const int _x, const int _y, const int _width, const char * const _text, const color_t _color, const int _boxheight = 0, const bool _utf8_encoded = false, const bool bg = false);
};

#endif /* __fb_window_h__ */
