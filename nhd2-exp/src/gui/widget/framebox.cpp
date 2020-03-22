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
	backgroundColor = COL_MENUCONTENT_PLUS_0;

	itemType = WIDGET_ITEM_FRAMEBOX;

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
	backgroundColor = COL_MENUCONTENT_PLUS_0;

	itemType = WIDGET_ITEM_FRAMEBOX;

	initFrames();
}

CFrameBox::~CFrameBox()
{
	for(unsigned int count = 0; count < (unsigned int)frames.size(); count++) 
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

bool CFrameBox::hasItem()
{
	return !frames.empty();
}

void CFrameBox::initFrames()
{
	cFrameWindow.setPosition(&cFrameBox);
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
	frame_height = cFrameBox.iHeight - 2*(cFrameBox.iHeight/4);

	int frame_x = cFrameBox.iX + BORDER_LEFT;
	int frame_y = cFrameBox.iY + (cFrameBox.iHeight - frame_height)/2;

	for (unsigned int count = 0; count < (unsigned int)frames.size(); count++) 
	{
		CFrame *frame = frames[count];

		// init frame
		// frame
		frame->window.setPosition(frame_x + count*frame_width, frame_y, frame_width - BORDER_LEFT - BORDER_RIGHT, frame_height);

		frame->window.enableShadow();
		frame->item_backgroundColor = backgroundColor;
		
		if(selected == -1) 
		{
			selected = count;
		}

		if(outFocus)
			frame->paint(false);
		else
			frame->paint( selected == ((signed int) count));
	}
}

void CFrameBox::paint()
{
	dprintf(DEBUG_NORMAL, "CFrameBox::paint:\n");

	cFrameWindow.setColor(backgroundColor);
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

		if(pos < (int)frames.size())
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

		if(pos < (int)frames.size())
		{
			frames[selected]->paint(false);
			frame->paint(true);

			selected = pos;
		}
		break;
	}
}

int CFrameBox::OKPressed(CMenuTarget *parent)
{
	if(parent)
		return frames[selected]->exec(parent);
	else
		return menu_return::RETURN_EXIT;
}

// CFrame
CFrame::CFrame(const std::string title, const char * const icon, CMenuTarget * Target, const char * const ActionKey)
{
	caption = title;
	iconName = icon ? icon : "";

	jumpTarget = Target;
	actionKey = ActionKey ? ActionKey : "";
}

int CFrame::paint(bool selected, bool /*AfterPulldown*/)
{
	dprintf(DEBUG_DEBUG, "CFrame::paint:\n");

	uint8_t color = COL_MENUCONTENT;
	fb_pixel_t bgcolor = item_backgroundColor;

	if (selected)
	{
		color = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
	}

	// frame
	window.setColor(bgcolor);
	window.paint();

	// icon
	int iw = 0;
	int ih = 0;
	int iconOffset = 0;

	if(!iconName.empty())
	{
		iconOffset = ICON_OFFSET;

		CFrameBuffer::getInstance()->getIconSize(iconName.c_str(), &iw, &ih);

		CFrameBuffer::getInstance()->paintIcon(iconName, window.getWindowsPos().iX + ICON_OFFSET, window.getWindowsPos().iY + (window.getWindowsPos().iHeight - ih)/2);
	}

	// caption
	if(!option.empty())
	{
		// caption
		if(!caption.empty())
		{
			int c_w = g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE]->getRenderWidth(caption);

			g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE]->RenderString(window.getWindowsPos().iX + BORDER_LEFT + iconOffset + iw + ((window.getWindowsPos().iWidth - BORDER_LEFT - iconOffset - iw - c_w) >> 1), window.getWindowsPos().iY + 3 + g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE]->getHeight(), window.getWindowsPos().iWidth - BORDER_LEFT - BORDER_RIGHT - iconOffset - iw, caption.c_str(), color, 0, true); //
		}

		// option
		if(!option.empty())
		{
			int o_w = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth(option);

			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(window.getWindowsPos().iX + BORDER_LEFT + iconOffset + iw + ((window.getWindowsPos().iWidth - BORDER_LEFT - iconOffset - iw - o_w) >> 1), window.getWindowsPos().iY + window.getWindowsPos().iHeight, window.getWindowsPos().iWidth - BORDER_LEFT - BORDER_RIGHT - iconOffset -iw, option.c_str(), color, 0, true);
		}
	}
	else
	{
		if(!caption.empty())
		{
			int c_w = g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE]->getRenderWidth(caption);

			g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE]->RenderString(window.getWindowsPos().iX + BORDER_LEFT + iconOffset + iw + ((window.getWindowsPos().iWidth - BORDER_LEFT - iconOffset - iw - c_w)>> 1), window.getWindowsPos().iY + (window.getWindowsPos().iHeight - g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE]->getHeight(), window.getWindowsPos().iWidth - BORDER_LEFT - BORDER_RIGHT - iconOffset - iw, caption.c_str(), color);
		}
	}

	return 0;
}

int CFrame::exec(CMenuTarget *parent)
{
	dprintf(DEBUG_NORMAL, "CFrame::exec:\n");

	if(jumpTarget)
		return jumpTarget->exec(parent, actionKey);
	else
		return menu_return::RETURN_EXIT;
}


