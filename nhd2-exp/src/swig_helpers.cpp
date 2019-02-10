/*
 * $Id: swig_helpers.cpp 07.02.2019 mohousch Exp $
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <global.h>

#include <swig_helpers.h>


void CSwigHelpers::paintBoxRel(const int x, const int y, const int dx, const int dy, fb_pixel_t col, int radius, int type, int mode)
{
	CFrameBuffer::getInstance()->paintBoxRel(x, y, dx, dy, col, radius, type, mode);
}

bool CSwigHelpers::paintIcon(const std::string & filename, const int x, const int y, const int h, bool paint, int width, int height)
{
	return CFrameBuffer::getInstance()->paintIcon(filename, x, y, h, paint, width, height);
}

bool CSwigHelpers::displayImage(const std::string & name, int posx, int posy, int width, int height, CFrameBuffer::ScalingMode scaling, int x_pan, int y_pan, bool clearfb)
{
	return CFrameBuffer::getInstance()->displayImage(name, posx, posy, width, height, scaling, x_pan, y_pan, clearfb);
}

bool CSwigHelpers::displayLogo(t_channel_id channel_id, int posx, int posy, int width, int height, bool upscale, bool center_x, bool center_y)
{
	return CFrameBuffer::getInstance()->displayLogo(channel_id, posx, posy, width, height, upscale, center_x, center_y);
}

void CSwigHelpers::paintBackground()
{
	CFrameBuffer::getInstance()->paintBackground();
}

void CSwigHelpers::paintBackgroundBoxRel(int x, int y, int dx, int dy)
{
	CFrameBuffer::getInstance()->paintBackgroundBoxRel(x, y, dx, dy);
}

bool CSwigHelpers::loadBackgroundPic(const std::string& filename, bool show)
{
	return CFrameBuffer::getInstance()->loadBackgroundPic(filename, show);
}

void CSwigHelpers::paintVLineRel(int x, int y, int dy, const fb_pixel_t col)
{
	CFrameBuffer::getInstance()->paintVLineRel(x, y, dy, col);
}

void CSwigHelpers::paintHLineRel(int x, int dx, int y, const fb_pixel_t col)
{
	CFrameBuffer::getInstance()->paintHLineRel(x, dx, y, col);
}

void CSwigHelpers::paintFrameBox(const int x, const int y, const int dx, const int dy, const fb_pixel_t col)
{
	CFrameBuffer::getInstance()->paintFrameBox(x, y, dx, dy, col);
}

void CSwigHelpers::saveBackgroundImage(void)
{
	CFrameBuffer::getInstance()->saveBackgroundImage();
}

void CSwigHelpers::restoreBackgroundImage(void)
{
	CFrameBuffer::getInstance()->restoreBackgroundImage();
}

void CSwigHelpers::saveScreen(int x, int y, int dx, int dy, fb_pixel_t * const memp)
{
	CFrameBuffer::getInstance()->saveScreen(x, y, dx, dy, memp);
}

void CSwigHelpers::restoreScreen(int x, int y, int dx, int dy, fb_pixel_t * const memp)
{
	CFrameBuffer::getInstance()->restoreScreen(x, y, dx, dy, memp);
}

void CSwigHelpers::RenderString(int font_type, int x, int y, const int width, const char * text, const uint8_t color, const int boxheight, bool utf8_encoded, const bool useBackground)
{
	g_Font[font_type]->RenderString(x, y, width, text, color, boxheight, utf8_encoded, useBackground);
}

int CSwigHelpers::getRenderWidth(int font_type, const char *text, bool utf8_encoded)
{
	return g_Font[font_type]->getRenderWidth(text, utf8_encoded);
}

int CSwigHelpers::getHeight(int font_type)
{
	return g_Font[font_type]->getHeight();
}










