/*
 * $Id: scrollbar.cpp 2016.12.14 mohousch Exp $
 *
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

#include <gui/widget/scrollbar.h>


void paintScrollBar(const int x, const int y, const int dy, const int NrOfPages, const int CurrentPage)
{
	// scrollBar
	CBox cFrameScrollBar;
	CWindow cScrollBarWindow;

	cFrameScrollBar.iX = x;
	cFrameScrollBar.iY = y;
	cFrameScrollBar.iWidth = SCROLLBAR_WIDTH;
	cFrameScrollBar.iHeight = dy;


	cScrollBarWindow.setDimension(&cFrameScrollBar);
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

	cSliderWindow.setDimension(&cFrameSlider);
	cSliderWindow.setColor(COL_SCROLLBAR_SLIDER);
	cSliderWindow.setCorner(NO_RADIUS, CORNER_ALL);
	cSliderWindow.paint();
}

void paintScrollBar(CBox* position, const int NrOfPages, const int CurrentPage)
{
	// scrollBar
	CBox cFrameScrollBar;
	CWindow cScrollBarWindow;

	cFrameScrollBar = *position;

	cScrollBarWindow.setDimension(&cFrameScrollBar);
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

	cSliderWindow.setDimension(&cFrameSlider);
	cSliderWindow.setColor(COL_SCROLLBAR_SLIDER);
	cSliderWindow.setCorner(NO_RADIUS, CORNER_ALL);
	cSliderWindow.paint();
}


