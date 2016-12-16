/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: listbox.cpp 2013/10/12 mohousch Exp $

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

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
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <global.h>
#include <neutrino.h>

#include <gui/widget/listbox.h>
#include <gui/widget/icons.h>
#include <gui/widget/buttons.h>
#include <gui/widget/items2detailsline.h>

#include <system/debug.h>
#include <system/helpers.h>


CListBox::CListBox(const char * const Caption, int _width, int _height)
{
	frameBuffer = CFrameBuffer::getInstance();
	caption = Caption;

	liststart = 0;
	selected =  0;

	modified = false;

	cFrameBox.iWidth = _width;
	cFrameBox.iHeight = _height;
	
	FootInfo = false;
	TitleInfo = false;
	PaintDate = false;
	
	initFrames();
}

CListBox::CListBox(const neutrino_locale_t Caption, int _width, int _height)
{
	frameBuffer = CFrameBuffer::getInstance();
	caption = g_Locale->getText(Caption);

	liststart = 0;
	selected =  0;

	modified = false;

	cFrameBox.iWidth = _width;
	cFrameBox.iHeight = _height;
	
	FootInfo = false;
	TitleInfo = false;
	PaintDate = false;
	
	initFrames();
}

void CListBox::initFrames()
{
	// Foot Info
	cFrameFootInfo.iHeight = (FootInfo)? g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight() + 10 : 0;
	
	// Title Info
	cFrameTitleInfo.iHeight = (TitleInfo)?g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight() + 10 : 0;
	
	// Foot
	footIcon.setIcon(NEUTRINO_ICON_BUTTON_RED);
	cFrameFoot.iHeight = std::max(footIcon.iHeight, g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight()) + 10;
	
	// head
	cFrameTitle.iHeight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight() + 10;

	// Item
	cFrameItem.iHeight = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight();

	//
	listmaxshow = (cFrameBox.iHeight - cFrameTitleInfo.iHeight - cFrameTitle.iHeight - cFrameFoot.iHeight - (cFrameFootInfo.iHeight))/cFrameItem.iHeight;

	// recalculate height
	cFrameBox.iHeight = cFrameTitleInfo.iHeight + cFrameTitle.iHeight + listmaxshow*cFrameItem.iHeight + cFrameFoot.iHeight + cFrameFootInfo.iHeight;

	//
	cFrameBox.iX = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - (cFrameBox.iWidth + (FootInfo? ConnectLineBox_Width : 0))) / 2) + (FootInfo? ConnectLineBox_Width : 0);
	cFrameBox.iY = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - cFrameBox.iHeight) / 2);
}

void CListBox::enableFootInfo(void)
{
	FootInfo = true; 
	initFrames();
}

void CListBox::enableTitleInfo(void)
{
	TitleInfo = true; 
	initFrames();
}

void CListBox::setModified(void)
{
	modified = true;
}

// body
void CListBox::paint()
{
	dprintf(DEBUG_DEBUG, "CListBox::paint\n");

	liststart = (selected/listmaxshow)*listmaxshow;

	// items
	for(unsigned int count = 0; count < listmaxshow; count++)
	{
		paintItem(count);
	}

	// scrollbar
	cFrameScrollBar.iX = cFrameBox.iX + cFrameBox.iWidth - SCROLLBAR_WIDTH;
	cFrameScrollBar.iY = cFrameBox.iY + cFrameTitleInfo.iHeight + cFrameTitle.iHeight;
	cFrameScrollBar.iWidth = SCROLLBAR_WIDTH;
	cFrameScrollBar.iHeight = cFrameItem.iHeight*listmaxshow;

	::paintScrollBar(&cFrameScrollBar, ((getItemCount() - 1)/ listmaxshow) + 1, (selected/listmaxshow));
}

// head
void CListBox::paintHead()
{
	// headBox
	cFrameTitle.iX = cFrameBox.iX;
	cFrameTitle.iY = cFrameBox.iY + cFrameTitleInfo.iHeight;
	cFrameTitle.iWidth = cFrameBox.iWidth;
	
	cWindowTitle.setDimension(&cFrameTitle);
	cWindowTitle.setColor(COL_MENUHEAD_PLUS_0);
	cWindowTitle.setCorner(RADIUS_MID, CORNER_TOP);
	cWindowTitle.setGradient(g_settings.menu_Head_gradient);
	cWindowTitle.paint();
	
	// title
	int timestr_len = 0;
	std::string timestr = getNowTimeStr("%d.%m.%Y %H:%M");;
		
	timestr_len = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getRenderWidth(timestr.c_str(), true); // UTF-8
	
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(cFrameTitle.iX + BORDER_LEFT, cFrameTitle.iY + cFrameTitle.iHeight, cFrameTitle.iWidth - BORDER_LEFT - BORDER_RIGHT - timestr_len, caption.c_str() , COL_MENUHEAD);

	// paint time/date
	if(PaintDate)
	{	
		g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->RenderString(cFrameTitle.iX + cFrameTitle.iWidth - BORDER_RIGHT - timestr_len, cFrameTitle.iY + (cFrameTitle.iHeight - g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight(), timestr_len + 1, timestr.c_str(), COL_MENUHEAD, 0, true); 
	}
}

// foot
const struct button_label Buttons[4] =
{
	{ NEUTRINO_ICON_BUTTON_RED, NONEXISTANT_LOCALE, "red action" },
	{ NEUTRINO_ICON_BUTTON_GREEN, NONEXISTANT_LOCALE, "green action" },
	{ NEUTRINO_ICON_BUTTON_YELLOW, NONEXISTANT_LOCALE, "yellow action" },
	{ NEUTRINO_ICON_BUTTON_BLUE, NONEXISTANT_LOCALE, "blue action" },
	
};

void CListBox::paintFoot()
{
	cFrameFoot.iX = cFrameBox.iX;
	cFrameFoot.iY = cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight - cFrameFoot.iHeight;
	cFrameFoot.iWidth = cFrameBox.iWidth;

	cWindowFoot.setDimension(&cFrameFoot);
	cWindowFoot.setColor(COL_MENUHEAD_PLUS_0);
	cWindowFoot.setCorner(RADIUS_MID, CORNER_BOTTOM);
	cWindowFoot.setGradient(g_settings.menu_Foot_gradient);
	cWindowFoot.paint();

	int ButtonWidth = cFrameFoot.iWidth /4;

	::paintButtons(frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, cFrameFoot.iX + BORDER_LEFT, cFrameFoot.iY, ButtonWidth, 4, Buttons, cFrameFoot.iHeight);
}

void CListBox::paintItem(int pos)
{
	paintItem(liststart + pos, pos, (liststart + pos == selected) );
}

void CListBox::hide()
{
	frameBuffer->paintBackgroundBoxRel(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight);
	
	clearItem2DetailsLine();
	
	frameBuffer->blit();
}

unsigned int CListBox::getItemCount()
{
	return listmaxshow;
}

int CListBox::getItemHeight()
{
	return cFrameItem.iHeight;
}

void CListBox::paintItem(unsigned int itemNr, int paintNr, bool _selected)
{
	int ypos = cFrameBox.iY + cFrameTitleInfo.iHeight + cFrameTitle.iHeight + paintNr*getItemHeight();

	uint8_t    color;
	fb_pixel_t bgcolor;
	
	if (_selected)
	{
		color   = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
		
		// itemlines	
		paintItem2DetailsLine(paintNr);		
		
		// details
		paintFootInfo(itemNr);
	}
	else
	{
		color   = COL_MENUCONTENT;
		bgcolor = COL_MENUCONTENT_PLUS_0;
	}

	// itemBox
	frameBuffer->paintBoxRel(cFrameBox.iX, ypos, cFrameBox.iWidth - SCROLLBAR_WIDTH, getItemHeight(), bgcolor);
	
	// item 
	g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(cFrameBox.iX + BORDER_LEFT, ypos + cFrameItem.iHeight, cFrameBox.iWidth - (BORDER_LEFT + BORDER_RIGHT), "demo", color);
}

int CListBox::exec(CMenuTarget* parent, const std::string &/*actionKey*/)
{
	dprintf(DEBUG_NORMAL, "CListBox::exec\n");

	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	int res = menu_return::RETURN_REPAINT;
	selected = 0;

	if (parent)
		parent->hide();

	initFrames();

	paintHead();
	paint();
	paintFoot();
	
	frameBuffer->blit();

	bool loop = true;
	modified = false;

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);
	
	while (loop)
	{
		g_RCInput->getMsg(&msg, &data, g_settings.timing[SNeutrinoSettings::TIMING_EPG]);

		if (msg == CRCInput::RC_home)
		{
			loop = false;
		}
		else if (msg == CRCInput::RC_up || msg == CRCInput::RC_page_up)
		{
			if(getItemCount() != 0) 
			{
				int step = 0;
				int prev_selected = selected;

				step = (msg == CRCInput::RC_page_up) ? listmaxshow : 1;  // browse or step 1
				selected -= step;
				if((prev_selected - step) < 0)            // because of uint
					selected = getItemCount() - 1;

				paintItem(prev_selected - liststart);

				unsigned int oldliststart = liststart;
				liststart = (selected/listmaxshow)*listmaxshow;

				if(oldliststart != liststart)
					paint();
				else
					paintItem(selected - liststart);
			}
		}
		else if (msg == CRCInput::RC_down || msg == CRCInput::RC_page_down)
		{
			if(getItemCount() != 0) 
			{
				unsigned int step = 0;
				int prev_selected = selected;

				step = (msg == CRCInput::RC_page_down) ? listmaxshow : 1;  // browse or step 1
				selected += step;

				if(selected >= getItemCount()) 
				{
					if (((getItemCount() / listmaxshow) + 1) * listmaxshow == getItemCount() + listmaxshow) // last page has full entries
						selected = 0;
					else
						selected = ((step == listmaxshow) && (selected < (((getItemCount() / listmaxshow) + 1) * listmaxshow))) ? (getItemCount() - 1) : 0;
				}

				paintItem(prev_selected - liststart);

				unsigned int oldliststart = liststart;
				liststart = (selected/listmaxshow)*listmaxshow;

				if(oldliststart != liststart)
					paint();
				else
					paintItem(selected - liststart);
			}
		}
		else if( msg == CRCInput::RC_ok)
		{
			onOkKeyPressed();
			paintTitleInfo();
		}
		else if ( msg == CRCInput::RC_red)
		{
			onRedKeyPressed();
		}
		else if ( msg == CRCInput::RC_green)
		{
			onGreenKeyPressed();
		}
		else if ( msg == CRCInput::RC_yellow)
		{
			onYellowKeyPressed();
		}
		else if ( msg == CRCInput::RC_blue)
		{
			onBlueKeyPressed();
		}
		else if ( msg == CRCInput::RC_setup)
		{
			onMenuKeyPressed();
		}
		else if ( msg == CRCInput::RC_info)
		{
			onInfoKeyPressed();
		}
		else if ( msg == CRCInput::RC_right)
		{
			onRightKeyPressed();
		}
		else if ( msg == CRCInput::RC_left)
		{
			onLeftKeyPressed();
		}
		else if ( msg == CRCInput::RC_spkr)
		{
			onMuteKeyPressed();
		}
		else if ( (msg == NeutrinoMessages::EVT_TIMER) && (data == sec_timer_id) )
		{
			if(PaintDate)
			{
				// head
				paintHead();
	
				// Foot
				//paintFoot();
	
				// paint all
				//paint();
			}
		} 
		else
		{
			CNeutrinoApp::getInstance()->handleMsg( msg, data );
			// kein canceling...
		}

		frameBuffer->blit();	
	}

	hide();

	if(PaintDate)
	{
		//
		g_RCInput->killTimer(sec_timer_id);
		sec_timer_id = 0;
	}
	
	return res;
}

void CListBox::paintFootInfo(int index)
{
	if(FootInfo == false)
		return;

	//
	cFrameFootInfo.iX = cFrameBox.iX;
	cFrameFootInfo.iY = cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight;
	cFrameFootInfo.iWidth = cFrameBox.iWidth;
	
	// infobox refresh
	frameBuffer->paintBoxRel(cFrameFootInfo.iX + 2, cFrameFootInfo.iY + 2, cFrameFootInfo.iWidth - 4, cFrameFootInfo.iHeight - 4, COL_MENUCONTENTDARK_PLUS_0, NO_RADIUS, CORNER_NONE, g_settings.menu_Head_gradient);
}

void CListBox::paintItem2DetailsLine(int pos)
{
	if(FootInfo == false)
		return;
	
	::paintItem2DetailsLine(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight - cFrameFootInfo.iHeight, cFrameFootInfo.iHeight, cFrameTitle.iHeight + cFrameTitleInfo.iHeight, cFrameItem.iHeight, pos);
}

void CListBox::clearItem2DetailsLine()
{
	if(FootInfo == false)
		return;
	   
	::clearItem2DetailsLine(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight - cFrameFootInfo.iHeight, cFrameFootInfo.iHeight);  
}

void CListBox::paintTitleInfo(int index)
{
	if(TitleInfo == false)
		return;
	
	// infobox refresh
	cFrameTitleInfo.iX = cFrameBox.iX;
	cFrameTitleInfo.iY = cFrameBox.iY;
	cFrameTitleInfo.iWidth = cFrameBox.iWidth;

	cWindowTitleInfo.setDimension(&cFrameTitleInfo);
	cWindowTitleInfo.setColor(COL_MENUCONTENT_PLUS_6);
	cWindowTitleInfo.setGradient(g_settings.menu_Head_gradient);

	cWindowTitleInfo.paint();

	//frameBuffer->paintBoxRel(x, y - TitleHeight, width, TitleHeight, COL_MENUCONTENT_PLUS_6);
	frameBuffer->paintBoxRel(cFrameTitleInfo.iX + 2, cFrameTitleInfo.iY + 2, cFrameTitleInfo.iWidth - 4, cFrameTitleInfo.iHeight - 4, COL_MENUCONTENT_PLUS_1, NO_RADIUS, CORNER_NONE, g_settings.menu_Head_gradient);
}


