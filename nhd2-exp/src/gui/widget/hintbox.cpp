/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: hintbox.cpp 2013/10/12 mohousch Exp $

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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <global.h>
#include <neutrino.h>

#include <system/debug.h>

#include <gui/widget/hintbox.h>


#define HINTBOX_MAX_HEIGHT 420


CHintBox::CHintBox(const neutrino_locale_t Caption, const char * const Text, const int Width, const char * const Icon)
{
	char * begin;
	char * pos;
	int    nw;

	message = strdup(Text);

	cFrameBox.iWidth = Width;

	cFrameBoxTitle.iHeight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	cFrameBoxItem.iHeight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	cFrameBox.iHeight = cFrameBoxTitle.iHeight + cFrameBoxItem.iHeight;

	caption = g_Locale->getText(Caption);

	begin = message;

	while (true)
	{
		cFrameBox.iHeight += cFrameBoxItem.iHeight;
		if (cFrameBox.iHeight > HINTBOX_MAX_HEIGHT)
			cFrameBox.iHeight -= cFrameBoxItem.iHeight;

		line.push_back(begin);

		pos = strchr(begin, '\n');
		if (pos != NULL)
		{
			*pos = 0;
			begin = pos + 1;
		}
		else
			break;
	}

	entries_per_page = ((cFrameBox.iHeight - cFrameBoxTitle.iHeight) / cFrameBoxItem.iHeight) - 1;
	current_page = 0;

	unsigned int additional_width;

	if (entries_per_page < line.size())
		additional_width = BORDER_LEFT + BORDER_RIGHT + SCROLLBAR_WIDTH;
	else
		additional_width = BORDER_LEFT + BORDER_RIGHT;

	if (Icon != NULL)
	{
		iconfile = Icon;
		additional_width += BORDER_LEFT + BORDER_RIGHT + 2*ICON_OFFSET;
	}
	else
		iconfile = "";

	nw = additional_width + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getRenderWidth(caption); // UTF-8

	if (nw > cFrameBox.iWidth)
		cFrameBox.iWidth = nw;

	for (std::vector<char *>::const_iterator it = line.begin(); it != line.end(); it++)
	{
		nw = additional_width + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(*it, true); // UTF-8
		if (nw > cFrameBox.iWidth)
			cFrameBox.iWidth = nw;
	}
}

CHintBox::CHintBox(const char * Caption, const char * const Text, const int Width, const char * const Icon)
{
	char * begin;
	char * pos;
	int    nw;

	message = strdup(Text);

	cFrameBox.iWidth   = Width;

	cFrameBoxTitle.iHeight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	cFrameBoxItem.iHeight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	cFrameBox.iHeight = cFrameBoxTitle.iHeight + cFrameBoxItem.iHeight;

	caption = Caption;

	begin = message;

	while (true)
	{
		cFrameBox.iHeight += cFrameBoxItem.iHeight;
		if (cFrameBox.iHeight > HINTBOX_MAX_HEIGHT)
			cFrameBox.iHeight -= cFrameBoxItem.iHeight;

		line.push_back(begin);
		pos = strchr(begin, '\n');
		if (pos != NULL)
		{
			*pos = 0;
			begin = pos + 1;
		}
		else
			break;
	}
	entries_per_page = ((cFrameBox.iHeight - cFrameBoxTitle.iHeight) / cFrameBoxItem.iHeight) - 1;
	current_page = 0;

	unsigned int additional_width;

	if (entries_per_page < line.size())
		additional_width = BORDER_LEFT + BORDER_RIGHT + SCROLLBAR_WIDTH;
	else
		additional_width = BORDER_LEFT + BORDER_RIGHT;

	if (Icon != NULL)
	{
		iconfile = Icon;
		additional_width += BORDER_LEFT + BORDER_RIGHT + 2*ICON_OFFSET;
	}
	else
		iconfile = "";

	nw = additional_width + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getRenderWidth(caption); // UTF-8

	if (nw > cFrameBox.iWidth)
		cFrameBox.iWidth = nw;

	for (std::vector<char *>::const_iterator it = line.begin(); it != line.end(); it++)
	{
		nw = additional_width + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(*it, true); // UTF-8
		if (nw > cFrameBox.iWidth)
			cFrameBox.iWidth = nw;
	}
}

CHintBox::~CHintBox(void)
{
	free(message);
}

void CHintBox::paint(void)
{
	dprintf(DEBUG_NORMAL, "CHintBox::paint\n");

	// Box
	cFrameBox.iX = CFrameBuffer::getInstance()->getScreenX() + ((CFrameBuffer::getInstance()->getScreenWidth() - cFrameBox.iWidth ) >> 1);
	cFrameBox.iY = CFrameBuffer::getInstance()->getScreenY() + ((CFrameBuffer::getInstance()->getScreenHeight() - cFrameBox.iHeight) >> 2);
	
	// Box
	m_cBoxWindow.setDimension(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight);

	m_cBoxWindow.enableSaveScreen();
	m_cBoxWindow.setColor(COL_MENUCONTENT_PLUS_0);
	//m_cBoxWindow.setCorner(RADIUS_MID, CORNER_ALL);
	m_cBoxWindow.enableShadow();
	m_cBoxWindow.paint();

	refresh();
	
	CFrameBuffer::getInstance()->blit();
}

void CHintBox::refresh(void)
{
	//body
	m_cBodyWindow.setDimension(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight);
	m_cBodyWindow.setColor(COL_MENUCONTENT_PLUS_0);
	//m_cBodyWindow.setCorner(RADIUS_MID, CORNER_ALL);
	m_cBoxWindow.enableShadow();
	m_cBodyWindow.paint();
	
	// title
	cFrameBoxTitle.iX = cFrameBox.iX;
	cFrameBoxTitle.iY = cFrameBox.iY;
	cFrameBoxTitle.iWidth = cFrameBox.iWidth;

	m_cTitleWindow.setDimension(cFrameBoxTitle.iX, cFrameBoxTitle.iY, cFrameBoxTitle.iWidth, cFrameBoxTitle.iHeight);

	m_cTitleWindow.setColor(COL_MENUHEAD_PLUS_0);
	//m_cTitleWindow.setCorner(RADIUS_MID, CORNER_TOP);
	m_cTitleWindow.setGradient(g_settings.Head_gradient);
	m_cTitleWindow.paint();
	
	int icon_w = 0;
	int icon_h = 0;

	if (!iconfile.empty())
	{
		CFrameBuffer::getInstance()->getIconSize(iconfile.c_str(), &icon_w, &icon_h);
		CFrameBuffer::getInstance()->paintIcon(iconfile.c_str(), cFrameBoxTitle.iX + BORDER_LEFT, cFrameBoxTitle.iY, cFrameBoxTitle.iHeight);
	}
	
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(cFrameBoxTitle.iX + BORDER_LEFT + icon_w + BORDER_LEFT, cFrameBoxTitle.iY + cFrameBoxTitle.iHeight, cFrameBoxTitle.iWidth - BORDER_LEFT - icon_w - BORDER_LEFT, caption.c_str(), COL_MENUHEAD, 0, true); 

	// body text
	int count = entries_per_page;
	int ypos  = cFrameBoxTitle.iY + cFrameBoxTitle.iHeight + (cFrameBoxItem.iHeight >> 1);

	for (std::vector<char *>::const_iterator it = line.begin() + (entries_per_page * current_page); ((it != line.end()) && (count > 0)); it++, count--)
	{
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(cFrameBox.iX + BORDER_LEFT, (ypos += cFrameBoxItem.iHeight), cFrameBox.iWidth, *it, COL_MENUCONTENT, 0, true); 
	}

	// scrollBar #TODO
	if (entries_per_page < line.size())
	{
		ypos = cFrameBoxTitle.iHeight + (cFrameBoxItem.iHeight >> 1);
		CFrameBuffer::getInstance()->paintBoxRel(cFrameBox.iX + cFrameBox.iWidth - SCROLLBAR_WIDTH, ypos, SCROLLBAR_WIDTH, entries_per_page*cFrameBoxItem.iHeight, COL_MENUCONTENT_PLUS_1);

		unsigned int marker_size = (entries_per_page*cFrameBoxItem.iHeight) / ((line.size() + entries_per_page - 1) / entries_per_page);

		CFrameBuffer::getInstance()->paintBoxRel(cFrameBox.iX + cFrameBox.iWidth - (SCROLLBAR_WIDTH - 2), ypos + current_page*marker_size, SCROLLBAR_WIDTH - 4, marker_size, COL_MENUCONTENT_PLUS_3);
	}	
}

bool CHintBox::has_scrollbar(void)
{
	return (entries_per_page < line.size());
}

void CHintBox::scroll_up(void)
{
	if (current_page > 0)
	{
		current_page--;
		refresh();
	}
}

void CHintBox::scroll_down(void)
{
	if ((entries_per_page * (current_page + 1)) <= line.size())
	{
		current_page++;
		refresh();
	}
}

void CHintBox::hide(void)
{
	m_cBoxWindow.hide();	
}

int CHintBox::exec(int timeout)
{
	int res = messages_return::none;

	neutrino_msg_t msg;
	neutrino_msg_data_t data;

	paint();
	
	CFrameBuffer::getInstance()->blit();

	if ( timeout == -1 )
		timeout = g_settings.timing[SNeutrinoSettings::TIMING_INFOBAR];

	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd( timeout );

	while ( ! ( res & ( messages_return::cancel_info | messages_return::cancel_all ) ) )
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if ((msg == CRCInput::RC_timeout) || (msg == CRCInput::RC_home) || (msg == CRCInput::RC_ok))
		{
				res = messages_return::cancel_info;
		}
		else if ((has_scrollbar()) && ((msg == CRCInput::RC_up) || (msg == CRCInput::RC_down)))
		{
			if (msg == CRCInput::RC_up)
				scroll_up();
			else
				scroll_down();
		}
		else if((msg == CRCInput::RC_mode) || (msg == CRCInput::RC_next) || (msg == CRCInput::RC_prev)) 
		{
				res = messages_return::cancel_info;
				g_RCInput->postMsg(msg, data);
		}
		else
		{
			res = CNeutrinoApp::getInstance()->handleMsg(msg, data);
			if (res & messages_return::unhandled)
			{
				// raus hier und dar�ber behandeln...
				g_RCInput->postMsg(msg, data);
				res = messages_return::cancel_info;
			}
		}

		CFrameBuffer::getInstance()->blit();	
	}

	hide();

	return res;
}

int HintBox(const neutrino_locale_t Caption, const char * const Text, const int Width, int timeout, const char * const Icon)
{
	int res = messages_return::none;

	neutrino_msg_t msg;
	neutrino_msg_data_t data;

 	CHintBox * hintBox = new CHintBox(Caption, Text, Width, Icon);

	res = hintBox->exec(timeout);
		
	delete hintBox;
	hintBox = NULL;

	return res;
}

int HintBox(const neutrino_locale_t Caption, const neutrino_locale_t Text, const int Width, int timeout, const char * const Icon)
{
	return HintBox(Caption, g_Locale->getText(Text), Width, timeout, Icon);
}

int HintBox(const char * Caption, const char * const Text, const int Width, int timeout, const char * const Icon)
{
	int res = messages_return::none;

	neutrino_msg_t msg;
	neutrino_msg_data_t data;

 	CHintBox * hintBox = new CHintBox(Caption, Text, Width, Icon);

	res = hintBox->exec(timeout);
		
	delete hintBox;
	hintBox = NULL;

	return res;
}

int HintBox(const char * Caption, const neutrino_locale_t Text, const int Width, int timeout, const char * const Icon)
{
	return HintBox(Caption, g_Locale->getText(Text), Width, timeout, Icon);
}


