/*
	$Id: framebox.cpp 09.02.2019 mohousch Exp $


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

#include <global.h>

#include <gui/widget/framebox.h>

#include <system/settings.h>
#include <system/debug.h>

CFrameBox::CFrameBox(const int x, int const y, const int dx, const int dy)
{
	dprintf(DEBUG_NORMAL, "CFrameBox::CFrameBox:\n");

	frameBuffer = CFrameBuffer::getInstance();

	cFrameBox.iX = x;
	cFrameBox.iY = y;
	cFrameBox.iWidth = dx;
	cFrameBox.iHeight = dy;

	selected = -1;
	pos = 0;
	outFocus = false;

	initFrames();
}

CFrameBox::CFrameBox(CBox* position)
{
	dprintf(DEBUG_NORMAL, "CFrameBox::CFrameBox:\n");

	frameBuffer = CFrameBuffer::getInstance();

	cFrameBox = *position;

	selected = -1;
	pos = 0;
	outFocus = false;

	initFrames();
}

CFrameBox::~CFrameBox()
{
	for(unsigned int count = 0; count < frames.size(); count++) 
	{
		CFrame * frame = frames[count];
		
		delete frame;
		frame = NULL;
	}

	frames.clear();
}

void CFrameBox::addFrame(CFrame *frame, const bool defaultselected)
{
	if (defaultselected)
		selected = frames.size();
	
	frames.push_back(frame);
}

bool CFrameBox::hasFrame()
{
	return !frames.empty();
}

void CFrameBox::initFrames()
{
	cFrameWindow.setDimension(&cFrameBox);
}

void CFrameBox::paintFrames()
{
	dprintf(DEBUG_NORMAL, "CFrameBox::paintFrames:\n");

	// frame width
	int frame_width = 0;

	if(frames.size())
		frame_width = cFrameBox.iWidth/((int)frames.size());

	// frame hight
	int frame_height = 0;
	frame_height = cFrameBox.iHeight - 2*(cFrameBox.iHeight/3);

	int frame_x = cFrameBox.iX + BORDER_LEFT;
	int frame_y = cFrameBox.iY + (cFrameBox.iHeight - frame_height)/2;

	for (unsigned int count = 0; count < frames.size(); count++) 
	{
		CFrame * frame = frames[count];

		// init frame
		// frame
		frame->window.setDimension(frame_x + count*frame_width, cFrameBox.iY + (cFrameBox.iHeight - frame_height)/2, frame_width - BORDER_LEFT - BORDER_RIGHT, frame_height);

		frame->window.enableShadow();

		if(!outFocus && (selected == -1)) 
		{
			selected = count;
		}

		frame->paint( selected == ((signed int) count));
	}
}

void CFrameBox::paint()
{
	dprintf(DEBUG_NORMAL, "CFrameBox::paint:\n");

	cFrameWindow.setColor(COL_MENUCONTENT_PLUS_0);
	//cFrameWindow.setCorner(RADIUS_MID, CORNER_ALL);
	//cFrameWindow.enableShadow();
	//cFrameWindow.enableSaveScreen();

	cFrameWindow.paint();

	paintFrames();

	CFrameBuffer::getInstance()->blit();
}

void CFrameBox::hide()
{
	dprintf(DEBUG_NORMAL, "CFrameBox::hide:\n");

	cFrameWindow.hide();
}

void CFrameBox::swipRight()
{
	dprintf(DEBUG_NORMAL, "CFrameBox::swipRight:\n");

	for (unsigned int count = 0; count < frames.size(); count++) 
	{
		pos = selected + 1;

		if(pos >= (int)frames.size())
			pos = 0;

		CFrame * frame = frames[pos];

		if(pos < frames.size())
		{
			frames[selected]->paint(false);
			frame->paint(true);

			selected = pos;
		}
		break;
	}
}

void CFrameBox::swipLeft()
{
	dprintf(DEBUG_NORMAL, "CFrameBox::swipLeft:\n");

	for (unsigned int count = 0; count < frames.size(); count++) 
	{
		pos = selected - 1;
		if(pos < 0)
			pos = frames.size() - 1;

		CFrame * frame = frames[pos];

		if(pos < frames.size())
		{
			frames[selected]->paint(false);
			frame->paint(true);

			selected = pos;
		}
		break;
	}
}

// CFrame
CFrame::CFrame(const std::string title)
{
	caption = title;
}

int CFrame::getHeight(void) const
{
	int height = 50;

	return height;
}

int CFrame::getWidth(void) const
{
	int tw = 50;

	return tw;
}

int CFrame::paint(bool selected)
{
	dprintf(DEBUG_DEBUG, "CFrame::paint:\n");

	CFrameBuffer * frameBuffer = CFrameBuffer::getInstance();

	int height = getHeight();

	uint8_t color = COL_MENUCONTENT;
	fb_pixel_t bgcolor = COL_MENUCONTENT_PLUS_0;

	if (selected)
	{
		color = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
	}

	// frame
	window.setColor(bgcolor);
	window.paint();

	// caption
	g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE]->RenderString(window.getWindowsPos().iX + BORDER_LEFT, window.getWindowsPos().iY + (window.getWindowsPos().iHeight - g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE]->getHeight(), window.getWindowsPos().iWidth - BORDER_LEFT - 2*BORDER_RIGHT, caption.c_str(), COL_MENUFOOT_INFO);

	return height;
}




