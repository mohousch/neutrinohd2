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


CListBox::CListBox(const char * const Caption, int _width, int _height, bool itemDetails, bool titleInfo, bool paintDate)
{
	frameBuffer = CFrameBuffer::getInstance();
	caption = Caption;

	liststart = 0;
	selected =  0;

	width =  _width;
	height = _height;
	
	ItemDetails = itemDetails;
	TitleInfo = titleInfo;
	PaintDate = paintDate;
	
	InfoHeight = 0;
	TitleHeight = 0;
	
	if(ItemDetails)
		InfoHeight = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight() + 10;
	
	if(TitleInfo)
		TitleHeight = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight() + 10;
	
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_RED, &icon_bf_w, &icon_bf_h);
	ButtonHeight = std::max(icon_bf_h, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight()) + 10;
	
	modified = false;
	
	hheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	iheight = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight();
	listmaxshow = (height - hheight - ButtonHeight)/iheight;
	height = hheight + ButtonHeight + listmaxshow*iheight; // recalc height

	x = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - (width + (ItemDetails? ConnectLineBox_Width : 0))) / 2) + (ItemDetails? ConnectLineBox_Width : 0);
	y = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - (height + InfoHeight)) / 2) + TitleHeight/2;
}

CListBox::CListBox(const neutrino_locale_t Caption, int _width, int _height, bool itemDetails, bool titleInfo, bool paintDate)
{
	frameBuffer = CFrameBuffer::getInstance();
	caption = g_Locale->getText(Caption);

	liststart = 0;
	selected =  0;

	width =  _width;
	height = _height;
	
	ItemDetails = itemDetails;
	TitleInfo = titleInfo;
	PaintDate = paintDate;
	
	InfoHeight = 0;
	TitleHeight = 0;
	
	if(ItemDetails)
		InfoHeight = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight() + 10;
	
	if(TitleInfo)
		TitleHeight = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight() + 10;
	
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_RED, &icon_bf_w, &icon_bf_h);
	ButtonHeight = std::max(icon_bf_h, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight()) + 10;
	
	modified = false;
	
	hheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	iheight = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight();
	listmaxshow = (height - hheight - ButtonHeight)/iheight;
	height = hheight + ButtonHeight + listmaxshow*iheight; // recalc height

	x = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - (width + (ItemDetails? ConnectLineBox_Width : 0))) / 2) + (ItemDetails? ConnectLineBox_Width : 0);
	y = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - (height + InfoHeight)) / 2) + TitleHeight/2;
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

	// scroll bar
	int ypos = y + hheight;
	int sb = iheight*listmaxshow;
	frameBuffer->paintBoxRel(x + width - SCROLLBAR_WIDTH, ypos, SCROLLBAR_WIDTH, sb,  COL_MENUCONTENT_PLUS_1);

	int sbc = ((getItemCount() - 1)/ listmaxshow) + 1;
	float sbh = (sb - 4)/ sbc;
	int sbs = (selected/listmaxshow);

	frameBuffer->paintBoxRel(x + width - (SCROLLBAR_WIDTH - 2), ypos + 2 + int(sbs* sbh), 11, int(sbh),  COL_MENUCONTENT_PLUS_3);
}

// head
void CListBox::paintHead()
{
	// headBox
	frameBuffer->paintBoxRel(x, y, width, hheight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, true, gradientLight2Dark);//round
	
	// paint time/date
	int timestr_len = 0;
	if(PaintDate)
	{
		std::string timestr = getNowTimeStr("%d.%m.%Y %H:%M");;
		
		timestr_len = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getRenderWidth(timestr.c_str(), true); // UTF-8
			
		g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->RenderString(x + width - (BORDER_RIGHT + timestr_len), y + (hheight - g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight(), timestr_len + 1, timestr.c_str(), COL_MENUHEAD, 0, true); // UTF-8 // 100 is pic_w refresh box
	}

	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(x + BORDER_LEFT, y + hheight, width - (BORDER_LEFT + BORDER_RIGHT + timestr_len), caption.c_str() , COL_MENUHEAD);
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
	int ButtonWidth = width / 4 - BORDER_LEFT - BORDER_RIGHT;
	
	frameBuffer->paintBoxRel(x, y + height - ButtonHeight, width, ButtonHeight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_BOTTOM, true, gradientDark2Light);//round

	::paintButtons(frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, x + BORDER_LEFT, y + height - ButtonHeight, ButtonWidth, 4, Buttons, ButtonHeight);
}

void CListBox::paintItem(int pos)
{
	paintItem(liststart + pos, pos, (liststart + pos == selected) );
}

void CListBox::hide()
{
	int ypos = y - TitleHeight;
	frameBuffer->paintBackgroundBoxRel(x, ypos, width, height + ButtonHeight + InfoHeight + TitleHeight);
	
	clearItem2DetailsLine();
	
	frameBuffer->blit();
}

unsigned int CListBox::getItemCount()
{
	return listmaxshow;
}

int CListBox::getItemHeight()
{
	return iheight;
}

void CListBox::paintItem(unsigned int itemNr, int paintNr, bool _selected)
{
	int ypos = y + hheight + paintNr*getItemHeight();

	uint8_t    color;
	fb_pixel_t bgcolor;
	
	if (_selected)
	{
		color   = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
		
		// itemlines	
		paintItem2DetailsLine(paintNr);		
		
		// details
		paintDetails(itemNr);
	}
	else
	{
		color   = COL_MENUCONTENT;
		bgcolor = COL_MENUCONTENT_PLUS_0;
	}

	// itemBox
	frameBuffer->paintBoxRel(x, ypos, width - SCROLLBAR_WIDTH, getItemHeight(), bgcolor);
	
	// item 
	g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(x + BORDER_LEFT, ypos + iheight, width - (BORDER_LEFT + BORDER_RIGHT), "demo", color);
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

	paintHead();
	paint();
	paintFoot();
	
	frameBuffer->blit();

	bool loop = true;
	modified = false;
	
	while (loop)
	{
		g_RCInput->getMsg(&msg, &data, g_settings.timing[SNeutrinoSettings::TIMING_EPG]);

		if (( msg == (neutrino_msg_t)g_settings.key_channelList_cancel) || ( msg == CRCInput::RC_home))
		{
			loop = false;
		}
		else if (msg == CRCInput::RC_up || (int) msg == g_settings.key_channelList_pageup)
		{
			if(getItemCount() != 0) 
			{
				int step = 0;
				int prev_selected = selected;

				step = ((int) msg == g_settings.key_channelList_pageup) ? listmaxshow : 1;  // browse or step 1
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
		else if (msg == CRCInput::RC_down || (int) msg == g_settings.key_channelList_pagedown)
		{
			if(getItemCount() != 0) 
			{
				unsigned int step = 0;
				int prev_selected = selected;

				step = ((int) msg == g_settings.key_channelList_pagedown) ? listmaxshow : 1;  // browse or step 1
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
			paintInfo();
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
		else
		{
			CNeutrinoApp::getInstance()->handleMsg( msg, data );
			// kein canceling...
		}

		frameBuffer->blit();	
	}

	hide();
	
	return res;
}

void CListBox::paintDetails(int index)
{
	if(ItemDetails == false)
		return;
	
	// infobox refresh
	frameBuffer->paintBoxRel(x + 2, y + height + 2, width - 4, InfoHeight - 4, COL_MENUCONTENTDARK_PLUS_0, 0, 0, true, gradientLight2Dark);
}

void CListBox::paintItem2DetailsLine(int pos)
{
	if(ItemDetails == false)
		return;
	
	::paintItem2DetailsLine(x, y, width, height, InfoHeight, hheight, iheight, pos);
}

void CListBox::clearItem2DetailsLine()
{
	if(ItemDetails == false)
		return;
	   
	::clearItem2DetailsLine(x, y, width, height, InfoHeight);  
}

void CListBox::paintInfo(int index)
{
	if(TitleInfo == false)
		return;
	
	// infobox refresh
	frameBuffer->paintBoxRel(x, y - TitleHeight, width, TitleHeight, COL_MENUCONTENT_PLUS_6);
	frameBuffer->paintBoxRel(x + 2, y - TitleHeight + 2, width - 4, TitleHeight - 4, COL_MENUCONTENT_PLUS_1, 0, 0, true, gradientLight2Dark);
}


