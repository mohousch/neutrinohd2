/*
 * $Id: widget_helpers.cpp 27.02.2019 mohousch Exp $
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

#include <global.h>

#include <driver/color.h>
#include <system/settings.h>

#include <gui/widget/widget_helpers.h>


CButtons::CButtons()
{
}

void CButtons::paintFootButtons(const int x, const int y, const int dx, const int dy, const unsigned int count, const struct button_label * const content)
{
	int iw, ih;

	int buttonWidth = 0;
	
	if(count)
	{
		buttonWidth = (dx - BORDER_LEFT - BORDER_RIGHT)/count;

		for (unsigned int i = 0; i < count; i++)
		{
			if(content[i].button != NULL)
			{
				//const char * l_option = NULL;
				std::string l_option("");

				l_option.clear();

				CFrameBuffer::getInstance()->getIconSize(content[i].button, &iw, &ih);
				int f_h = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight();

				if(content[i].localename != NULL)
					l_option = content[i].localename;
				else
					l_option = g_Locale->getText(content[i].locale);
		
				CFrameBuffer::getInstance()->paintIcon(content[i].button, x + BORDER_LEFT + i*buttonWidth, y + (dy - ih)/2);

				g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(x + BORDER_LEFT + iw + ICON_OFFSET + i*buttonWidth, y + f_h + (dy - f_h)/2, buttonWidth - iw - ICON_OFFSET, l_option, COL_MENUFOOT, 0, true); // UTF-8
			}
		}
	}
}

// head buttons (right)
void CButtons::paintHeadButtons(const int x, const int y, const int dx, const int dy, const unsigned int count, const struct button_label * const content)
{
	int iw[count], ih[count];
	int startx = x + dx - BORDER_RIGHT;
	
	for (unsigned int i = 0; i < count; i++)
	{
		if(content[i].button != NULL)
		{
			CFrameBuffer::getInstance()->getIconSize(content[i].button, &iw[i], &ih[i]);
		
			startx -= (iw[i] + ICON_TO_ICON_OFFSET);

			CFrameBuffer::getInstance()->paintIcon(content[i].button, startx, y + (dy - ih[i])/2);
		}
	}
}

// scrollBar
void CScrollBar::paint(const int x, const int y, const int dy, const int NrOfPages, const int CurrentPage)
{
	// scrollBar
	CBox cFrameScrollBar;
	CWindow cScrollBarWindow;

	cFrameScrollBar.iX = x;
	cFrameScrollBar.iY = y;
	cFrameScrollBar.iWidth = SCROLLBAR_WIDTH;
	cFrameScrollBar.iHeight = dy;


	cScrollBarWindow.setPosition(&cFrameScrollBar);
	cScrollBarWindow.setColor(COL_SCROLLBAR);
	cScrollBarWindow.setCorner(NO_RADIUS, CORNER_ALL);
	cScrollBarWindow.paint();
		
	// scrollBar slider
	CBox cFrameSlider;
	CWindow cSliderWindow;	

	cFrameSlider.iX = cFrameScrollBar.iX + 2;
	cFrameSlider.iY = cFrameScrollBar.iY + CurrentPage*(cFrameScrollBar.iHeight/NrOfPages);
	cFrameSlider.iWidth = cFrameScrollBar.iWidth - 4;
	cFrameSlider.iHeight = cFrameScrollBar.iHeight/NrOfPages;

	cSliderWindow.setPosition(&cFrameSlider);
	cSliderWindow.setColor(COL_SCROLLBAR_SLIDER);
	cSliderWindow.setCorner(NO_RADIUS, CORNER_ALL);
	cSliderWindow.paint();
}

void CScrollBar::paint(CBox* position, const int NrOfPages, const int CurrentPage)
{
	// scrollBar
	CBox cFrameScrollBar;
	CWindow cScrollBarWindow;

	cFrameScrollBar = *position;

	cScrollBarWindow.setPosition(&cFrameScrollBar);
	cScrollBarWindow.setColor(COL_SCROLLBAR);
	cScrollBarWindow.setCorner(NO_RADIUS, CORNER_ALL);
	cScrollBarWindow.paint();
		
	// scrollBar slider
	CBox cFrameSlider;
	CWindow cSliderWindow;	

	cFrameSlider.iX = cFrameScrollBar.iX + 2;
	cFrameSlider.iY = cFrameScrollBar.iY + CurrentPage*(cFrameScrollBar.iHeight/NrOfPages);
	cFrameSlider.iWidth = cFrameScrollBar.iWidth - 4;
	cFrameSlider.iHeight = cFrameScrollBar.iHeight/NrOfPages;

	cSliderWindow.setPosition(&cFrameSlider);
	cSliderWindow.setColor(COL_SCROLLBAR_SLIDER);
	cSliderWindow.setCorner(NO_RADIUS, CORNER_ALL);
	cSliderWindow.paint();
}

// detailsLine
void CItems2DetailsLine::paint(int x, int y, int width, int height, int info_height/*, int theight*/, int iheight, int iy)
{
	int xpos  = x - ConnectLineBox_Width;
	int ypos1 = iy;
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
		CFrameBuffer::getInstance()->paintBoxRel(x + 2, ypos2 + 2, width - 4, info_height - 4, COL_MENUFOOT_INFO_PLUS_0, NO_RADIUS, CORNER_NONE, g_settings.Foot_Info_gradient);	
	}
}


void CItems2DetailsLine::clear(int x, int y, int width, int height, int info_height)
{ 
	// lines
	CFrameBuffer::getInstance()->paintBackgroundBoxRel(x - ConnectLineBox_Width, y, ConnectLineBox_Width, height + info_height);

	// lines around info box
	CFrameBuffer::getInstance()->paintBackgroundBoxRel(x, y + height, width, info_height);
}

