/*
 * $Id: items2detailsline.cpp 2016/01/30 mohousch Exp $
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gui/widget/items2detailsline.h>
#include <gui/color.h>
#include <system/settings.h>
#include <global.h>

#if 0
void paintItem2DetailsLine(int x, int y, int width, int height, int info_height, int theight, int iheight, int pos)
{
	int xpos  = x - ConnectLineBox_Width;
	int ypos1 = y + theight + pos*iheight;
	int ypos2 = y + height;
	int ypos1a = ypos1 + (iheight/2) - 2;
	int ypos2a = ypos2 + (info_height/2) - 2;
	
	fb_pixel_t col1 = COL_MENUCONTENT_PLUS_6;
	fb_pixel_t col2 = COL_MENUCONTENT_PLUS_1;

	// clear
	CFrameBuffer::getInstance()->paintBackgroundBoxRel(xpos, y, ConnectLineBox_Width, height + info_height);

	// paint Line if detail info (and not valid list pos)
	if (pos >= 0) 
	{ 
		int fh = iheight > 10 ? iheight - 10 : 5;
		
		// vertical line connected to item	
		CFrameBuffer::getInstance()->paintBoxRel(xpos + ConnectLineBox_Width - 4, ypos1 + 5, 4, fh, col1);
		CFrameBuffer::getInstance()->paintBoxRel(xpos + ConnectLineBox_Width - 4, ypos1 + 5, 1, fh, col2);
		
		// vertical line connected to infobox	
		CFrameBuffer::getInstance()->paintBoxRel(xpos + ConnectLineBox_Width - 4, ypos2 + 7, 4, info_height - 14, col1);
		CFrameBuffer::getInstance()->paintBoxRel(xpos + ConnectLineBox_Width - 4, ypos2 + 7, 1, info_height - 14, col2);			

		// vertical line
		CFrameBuffer::getInstance()->paintBoxRel(xpos + ConnectLineBox_Width - 16, ypos1a, 4, ypos2a - ypos1a, col1);
		CFrameBuffer::getInstance()->paintBoxRel(xpos + ConnectLineBox_Width - 16, ypos1a, 1, ypos2a - ypos1a + 4, col2);		

		// Hline (item)
		CFrameBuffer::getInstance()->paintBoxRel(xpos + ConnectLineBox_Width - 15, ypos1a, 12, 4, col1);
		CFrameBuffer::getInstance()->paintBoxRel(xpos + ConnectLineBox_Width - 16, ypos1a, 12, 1, col2);
		
		// Hline (infobox)
		CFrameBuffer::getInstance()->paintBoxRel(xpos + ConnectLineBox_Width - 15, ypos2a, 12, 4, col1);
		CFrameBuffer::getInstance()->paintBoxRel(xpos + ConnectLineBox_Width - 12, ypos2a, 8, 1, col2);

		// untere info box background
		CFrameBuffer::getInstance()->paintBoxRel(x, ypos2, width, info_height, col1);
		CFrameBuffer::getInstance()->paintBoxRel(x + 2, ypos2 + 2, width - 4, info_height - 4, COL_MENUCONTENTDARK_PLUS_0, NO_RADIUS, CORNER_NONE, g_settings.menu_Head_gradient);	
	}
}
#endif

void paintItem2DetailsLine(int x, int y, int width, int height, int info_height, int theight, int iheight, int iy)
{
	int xpos  = x - ConnectLineBox_Width;
	int ypos1 = /*y + theight + pos*iheight*/iy;
	int ypos2 = y + height;
	int ypos1a = ypos1 + (iheight/2) - 2;
	int ypos2a = ypos2 + (info_height/2) - 2;
	
	fb_pixel_t col1 = COL_MENUCONTENT_PLUS_6;
	fb_pixel_t col2 = COL_MENUCONTENT_PLUS_1;

	// clear
	CFrameBuffer::getInstance()->paintBackgroundBoxRel(xpos, y, ConnectLineBox_Width, height + info_height);

	// paint Line if detail info (and not valid list pos)
	//if (pos >= 0) 
	{ 
		int fh = iheight > 10 ? iheight - 10 : 5;
		
		// vertical line connected to item	
		CFrameBuffer::getInstance()->paintBoxRel(xpos + ConnectLineBox_Width - 4, ypos1 + 5, 4, fh, col1);
		CFrameBuffer::getInstance()->paintBoxRel(xpos + ConnectLineBox_Width - 4, ypos1 + 5, 1, fh, col2);
		
		// vertical line connected to infobox	
		CFrameBuffer::getInstance()->paintBoxRel(xpos + ConnectLineBox_Width - 4, ypos2 + 7, 4, info_height - 14, col1);
		CFrameBuffer::getInstance()->paintBoxRel(xpos + ConnectLineBox_Width - 4, ypos2 + 7, 1, info_height - 14, col2);			

		// vertical line
		CFrameBuffer::getInstance()->paintBoxRel(xpos + ConnectLineBox_Width - 16, ypos1a, 4, ypos2a - ypos1a, col1);
		CFrameBuffer::getInstance()->paintBoxRel(xpos + ConnectLineBox_Width - 16, ypos1a, 1, ypos2a - ypos1a + 4, col2);		

		// Hline (item)
		CFrameBuffer::getInstance()->paintBoxRel(xpos + ConnectLineBox_Width - 15, ypos1a, 12, 4, col1);
		CFrameBuffer::getInstance()->paintBoxRel(xpos + ConnectLineBox_Width - 16, ypos1a, 12, 1, col2);
		
		// Hline (infobox)
		CFrameBuffer::getInstance()->paintBoxRel(xpos + ConnectLineBox_Width - 15, ypos2a, 12, 4, col1);
		CFrameBuffer::getInstance()->paintBoxRel(xpos + ConnectLineBox_Width - 12, ypos2a, 8, 1, col2);

		// untere info box background
		CFrameBuffer::getInstance()->paintBoxRel(x, ypos2, width, info_height, col1);
		CFrameBuffer::getInstance()->paintBoxRel(x + 2, ypos2 + 2, width - 4, info_height - 4, COL_MENUCONTENTDARK_PLUS_0, NO_RADIUS, CORNER_NONE, g_settings.menu_Head_gradient);	
	}
}


void clearItem2DetailsLine(int x, int y, int width, int height, int info_height)
{ 
	// lines
	CFrameBuffer::getInstance()->paintBackgroundBoxRel(x - ConnectLineBox_Width, y, ConnectLineBox_Width, height + info_height);

	// lines around info box
	CFrameBuffer::getInstance()->paintBackgroundBoxRel(x, y + height, width, info_height);
}
