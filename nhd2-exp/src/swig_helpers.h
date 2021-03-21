/*
 * $Id: swig_helpers.h 07.02.2019 mohousch Exp $
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

#ifndef __SWIG_HELPERS_H__
#define __SWIG_HELPERS_H__

#include <driver/framebuffer.h>
#include <driver/rcinput.h>


extern CPlugins* g_PluginList;

class CSwigHelpers
{
	private:

	public:
		CSwigHelpers(){};

		// frameBuffer stuff
		unsigned int getScreenWidth(bool real = false){return CFrameBuffer::getInstance()->getScreenWidth(real);};
		unsigned int getScreenHeight(bool real = false){return CFrameBuffer::getInstance()->getScreenHeight(real);}; 
		unsigned int getScreenX(){return CFrameBuffer::getInstance()->getScreenX();};
		unsigned int getScreenY(){return CFrameBuffer::getInstance()->getScreenY();};

		void paintBoxRel(const int x, const int y, const int dx, const int dy, fb_pixel_t col, int radius = 0, int type = CORNER_NONE, int mode = nogradient);

		bool paintIcon(const std::string & filename, const int x, const int y, const int h = 0, bool paint = true, int width = 0, int height = 0);

		bool displayImage(const std::string & name, int posx = 0, int posy = 0, int width = DEFAULT_XRES, int height = DEFAULT_YRES, ScalingMode scaling = COLOR, int x_pan = 0, int y_pan = 0, bool clearfb = false);

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
		void blit(/*int mode3d = THREE_NONE*/);

		int getIconHeight(const char * const filename);
		int getIconWidth(const char * const filename);

		void scaleImage(const std::string tname, int p_w, int p_h);

		// fontrenderer
		void RenderString(int font_type, int x, int y, const int width, const char * text, const uint8_t color, const int boxheight = 0, bool utf8_encoded = true, const bool useBackground = false);

		int getRenderWidth(int font_type, const char *text, bool utf8_encoded = true);
		int getHeight(int font_type);

		// CRCInput
		int getRCCode(int timeout = 10);
		void addTimer(uint64_t Interval, bool oneshot = true, bool correct_time = true );
		void killTimer(uint32_t id);
		int getRCData(int timeout = 10);
};

#endif
