/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: color.h 2013/10/12 mohousch Exp $

	Copyright (C) 2001 Steffen Hehn 'McClean'
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


#ifndef __color__
#define __color__

#include <stdint.h>

#include <driver/framebuffer.h>


// common
#define COL_TRANSP0			0x01
#define COL_DARK_RED0	 		0x02
#define COL_DARK_GREEN0	 		0x03
#define COL_DARK_OLIVE0	 		0x04
#define COL_DARK_BLUE0	 		0x05
#define COL_DARK_PURPLE0	 	0x06
#define COL_DARK_TURQUOISE0	 	0x07
#define COL_LIGHT_GRAY0	 		0x08
#define COL_DARK_GRAY0	 		0x09
#define COL_RED0	 		0x0A
#define COL_GREEN0	 		0x0B
#define COL_YELLOW0	 		0x0C
#define COL_BLUE0	 		0x0D
#define COL_PURPLE0	 		0x0E
#define COL_LIGHT_BLUE0	 		0x0F
#define COL_WHITE0	 		0x10
#define COL_BLACK0	 		0x11
#define COL_ORANGE0			0xBD
#define COL_DARK_ORANGE0		0xBC
#define COL_BACKGROUND0 		0xFF

#define COL_TRANSP			CFrameBuffer::getInstance()->realcolor[COL_TRANSP0]
#define COL_DARK_RED			CFrameBuffer::getInstance()->realcolor[COL_DARK_RED0]
#define COL_DARK_GREEN			CFrameBuffer::getInstance()->realcolor[COL_DARK_GREEN0]
#define COL_DARK_OLIVE			CFrameBuffer::getInstance()->realcolor[COL_DARK_OLIVE0]
#define COL_DARK_BLUE			CFrameBuffer::getInstance()->realcolor[COL_DARK_BLUE0]
#define COL_DARK_PURPLE			CFrameBuffer::getInstance()->realcolor[COL_DARK_PURPLE0]
#define COL_DARK_TURQUOISE		CFrameBuffer::getInstance()->realcolor[COL_DARK_TURQUOISE0]
#define COL_LIGHT_GRAY			CFrameBuffer::getInstance()->realcolor[COL_LIGHT_GRAY0]
#define COL_DARK_GRAY			CFrameBuffer::getInstance()->realcolor[COL_DARK_GRAY0]
#define COL_RED				CFrameBuffer::getInstance()->realcolor[COL_RED0]
#define COL_GREEN			CFrameBuffer::getInstance()->realcolor[COL_GREEN0]
#define COL_YELLOW			CFrameBuffer::getInstance()->realcolor[COL_YELLOW0]
#define COL_BLUE                        CFrameBuffer::getInstance()->realcolor[COL_BLUE0]
#define COL_PURP 			CFrameBuffer::getInstance()->realcolor[COL_PURPLE0]
#define COL_LIGHT_BLUE 			CFrameBuffer::getInstance()->realcolor[COL_LIGHT_BLUE0]
#define COL_WHITE                       CFrameBuffer::getInstance()->realcolor[COL_WHITE0]
#define COL_BLACK                       CFrameBuffer::getInstance()->realcolor[COL_BLACK0]
#define COL_ORANGE			CFrameBuffer::getInstance()->realcolor[COL_ORANGE0]
#define COL_DARK_ORANGE			CFrameBuffer::getInstance()->realcolor[COL_DARK_ORANGE0]
#define COL_BACKGROUND           	CFrameBuffer::getInstance()->realcolor[COL_BACKGROUND0]

// neutrino color
#define COL_MENUCONTENTINACTIVE		254-8*1
#define COL_MENUCONTENTSELECTED		254-8*2
#define COL_MENUCONTENTDARK		254-8*3
#define COL_MENUCONTENT			254-8*4
#define COL_MENUHEAD			254-8*5
#define COL_MENUFOOT			254-8*6
#define COL_INFOBAR			254-8*7
#define COL_INFOBAR_SHADOW		254-8*8
#define COL_COLORED_EVENTS_INFOBAR	254-8*9
#define COL_COLORED_EVENTS_CHANNELLIST	254-8*10
#define COL_MENUFOOT_INFO		254-8*11

#define COL_INFOBAR_PLUS_0              CFrameBuffer::getInstance()->realcolor[COL_INFOBAR + 0]
#define COL_INFOBAR_SHADOW_PLUS_0       CFrameBuffer::getInstance()->realcolor[COL_INFOBAR_SHADOW + 0]
#define COL_INFOBAR_SHADOW_PLUS_1       CFrameBuffer::getInstance()->realcolor[COL_INFOBAR_SHADOW + 1]

#define COL_MENUHEAD_PLUS_0             CFrameBuffer::getInstance()->realcolor[COL_MENUHEAD + 0]
#define COL_MENUCONTENT_PLUS_0          CFrameBuffer::getInstance()->realcolor[COL_MENUCONTENT + 0]
#define COL_MENUCONTENT_PLUS_1          CFrameBuffer::getInstance()->realcolor[COL_MENUCONTENT + 1] // scrollbar
#define COL_MENUCONTENT_PLUS_2          CFrameBuffer::getInstance()->realcolor[COL_MENUCONTENT + 2] // progresswindow
#define COL_MENUCONTENT_PLUS_3          CFrameBuffer::getInstance()->realcolor[COL_MENUCONTENT + 3] // scrollbar
#define COL_MENUCONTENT_PLUS_4          CFrameBuffer::getInstance()->realcolor[COL_MENUCONTENT + 4] // stringinput
#define COL_MENUCONTENT_PLUS_5          CFrameBuffer::getInstance()->realcolor[COL_MENUCONTENT + 5] // epgplus
#define COL_MENUCONTENT_PLUS_6          CFrameBuffer::getInstance()->realcolor[COL_MENUCONTENT + 6]
#define COL_MENUCONTENT_PLUS_7          CFrameBuffer::getInstance()->realcolor[COL_MENUCONTENT + 7] // progress window

// CMenuwidgetExtended
#define COL_MENUCONTENTDARK_PLUS_0      CFrameBuffer::getInstance()->realcolor[COL_MENUCONTENTDARK + 0]

// menu selected
#define COL_MENUCONTENTSELECTED_PLUS_0  CFrameBuffer::getInstance()->realcolor[COL_MENUCONTENTSELECTED + 0]
#define COL_MENUCONTENTSELECTED_PLUS_1  CFrameBuffer::getInstance()->realcolor[COL_MENUCONTENTSELECTED + 1]
#define COL_MENUCONTENTSELECTED_PLUS_2  CFrameBuffer::getInstance()->realcolor[COL_MENUCONTENTSELECTED + 2]

#define COL_MENUCONTENTINACTIVE_PLUS_0  CFrameBuffer::getInstance()->realcolor[COL_MENUCONTENTINACTIVE + 0]

#define COL_MENUFOOT_PLUS_0		CFrameBuffer::getInstance()->realcolor[COL_MENUFOOT + 0]

#define COL_MENUFOOT_INFO_PLUS_0        CFrameBuffer::getInstance()->realcolor[COL_MENUFOOT_INFO + 0]

#define COL_SCROLLBAR			COL_MENUCONTENT_PLUS_1
#define COL_SCROLLBAR_SLIDER		COL_MENUCONTENT_PLUS_3


int convertSetupColor2RGB(unsigned char r, unsigned char g, unsigned char b);
int convertSetupAlpha2Alpha(unsigned char alpha);
uint8_t limitChar(int c);

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} RgbColor;

typedef struct {
	float h;
	float s;
	float v;
} HsvColor;

uint8_t getBrightnessRGB(fb_pixel_t color);

fb_pixel_t changeBrightnessRGBRel(fb_pixel_t color, int br, bool transp=true);
fb_pixel_t changeBrightnessRGB(fb_pixel_t color, uint8_t br, bool transp=true);

fb_pixel_t Hsv2SysColor(HsvColor *hsv, uint8_t tr=0xFF);
uint8_t SysColor2Hsv(fb_pixel_t color, HsvColor *hsv);

void Hsv2Rgb(HsvColor *hsv, RgbColor *rgb);
void Rgb2Hsv(RgbColor *rgb, HsvColor *hsv);

fb_pixel_t* gradientColorToTransparent(fb_pixel_t col, fb_pixel_t *gradientBuf, int bSize, int mode, int intensity = normal);

fb_pixel_t* gradientOneColor(fb_pixel_t col, fb_pixel_t *gradientBuf, int bSize, int mode, int intensity = normal, uint8_t v_min = 0x40, uint8_t v_max = 0xE0, uint8_t s = 0xC0);

fb_pixel_t* gradientColorToColor(fb_pixel_t start_col, fb_pixel_t end_col, fb_pixel_t *gradientBuf, int bSize, int mode, int intensity = normal);

uint32_t setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char alpha = 0);

inline uint32_t make16color(__u32 rgb)
{
        return 0xFF000000 | rgb;
}

inline uint32_t make16color(uint16_t r, uint16_t g, uint16_t b, uint16_t t,
				  uint32_t  /*rl*/ = 0, uint32_t  /*ro*/ = 0,
				  uint32_t  /*gl*/ = 0, uint32_t  /*go*/ = 0,
				  uint32_t  /*bl*/ = 0, uint32_t  /*bo*/ = 0,
				  uint32_t  /*tl*/ = 0, uint32_t  /*to*/ = 0)
{
	return ((t << 24) & 0xFF000000) | ((r << 8) & 0xFF0000) | ((g << 0) & 0xFF00) | (b >> 8 & 0xFF);
}

#endif
