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


#define RC0        	CRCInput::RC_0
#define RC1        	CRCInput::RC_1
#define RC2        	CRCInput::RC_2
#define RC3        	CRCInput::RC_3
#define RC4        	CRCInput::RC_4
#define RC5        	CRCInput::RC_5
#define RC6        	CRCInput::RC_6
#define RC7        	CRCInput::RC_7
#define RC8        	CRCInput::RC_8
#define RC9        	CRCInput::RC_9
#define RCRIGHT    	CRCInput::RC_right
#define RCLEFT     	CRCInput::RC_left
#define RCUP       	CRCInput::RC_up
#define RCDOWN     	CRCInput::RC_down
#define RCOK       	CRCInput::RC_ok
#define RCMUTE     	CRCInput::RC_spkr
#define RCSTANDBY  	CRCInput::RC_standby
#define RCGREEN    	CRCInput::RC_green
#define RCYELLOW   	CRCInput::RC_yellow
#define RCRED      	CRCInput::RC_red
#define RCBLUE     	CRCInput::RC_blue
#define RCPLUS     	CRCInput::RC_plus
#define RCMINUS    	CRCInput::RC_minus
#define RCHELP     	CRCInput::RC_info
#define RCDBOX     	CRCInput::RC_setup
#define RCTEXT     	CRCInput::RC_text
#define RCHOME     	CRCInput::RC_home
#define RCPAGEUP	CRCInput::RC_page_up
#define RCPAGEDOWN	CRCInput::RC_page_down

class CSwigHelpers
{
	private:

	public:
		CSwigHelpers(){};

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

		// fontrenderer
		void RenderString(int font_type, int x, int y, const int width, const char * text, const uint8_t color, const int boxheight = 0, bool utf8_encoded = true, const bool useBackground = false);

		int getRenderWidth(int font_type, const char *text, bool utf8_encoded = true);
		int getHeight(int font_type);

		// CRCInput
		int getRCcode(int ms = 10);
};

#endif
