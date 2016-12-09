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

#include <gui/widget/hintbox.h>

#include <global.h>
#include <neutrino.h>

#include <system/debug.h>

#define borderwidth 4

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

	window = NULL;
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

	window = NULL;
}

CHintBox::~CHintBox(void)
{
	if (window != NULL)
	{
		delete window;
		window = NULL;
	}

	free(message);
}

void CHintBox::paint(void)
{
	dprintf(DEBUG_NORMAL, "CHintBox::paint\n");

	if (window != NULL)
	{
		/*
		 * do not paint stuff twice:
		 * => thread safety needed by movieplayer.cpp:
		 *    one thread calls our paint method, the other one our hide method
		 * => no memory leaks
		 */
		return;
	}

	CFrameBuffer *frameBuffer = CFrameBuffer::getInstance();

	// Box
	cFrameBox.iX = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - cFrameBox.iWidth ) >> 1);
	cFrameBox.iY = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - cFrameBox.iHeight) >> 2);
	
	window = new CFBWindow(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth + SHADOW_OFFSET, cFrameBox.iHeight + SHADOW_OFFSET);

	refresh();
	
	CFrameBuffer::getInstance()->blit();
}

void CHintBox::refresh(void)
{
	if (window == NULL)
	{
		return;
	}

	/*
	// shadow top right
	window->paintBoxRel(width - (BORDER_LEFT + BORDER_RIGHT), 
					borderwidth, 
					borderwidth + (BORDER_LEFT + BORDER_RIGHT), 
					height - borderwidth, 
					COL_INFOBAR_SHADOW_PLUS_0, 
					RADIUS_MID, CORNER_TOP); // right
	
	// shadow bottom
	window->paintBoxRel(borderwidth, 
					height - (BORDER_LEFT + BORDER_RIGHT), 
					width, 
					borderwidth + (BORDER_LEFT + BORDER_RIGHT), 
					COL_INFOBAR_SHADOW_PLUS_0, 
					RADIUS_MID, CORNER_BOTTOM); // bottom
	*/
	// Shadow
	window->paintBoxRel(SHADOW_OFFSET, SHADOW_OFFSET, cFrameBox.iWidth, cFrameBox.iHeight, COL_INFOBAR_SHADOW_PLUS_0, RADIUS_MID, CORNER_ALL);

	// title
	cFrameBoxTitle.iWidth = cFrameBox.iWidth;

	window->paintBoxRel(0, 0, cFrameBoxTitle.iWidth, cFrameBoxTitle.iHeight, (CFBWindow::color_t)COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, g_settings.menu_Head_gradient);
	
	int neededWidth = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getRenderWidth(caption); 

	if (!iconfile.empty())
	{
		int iw, ih;
		CFrameBuffer::getInstance()->getIconSize(iconfile.c_str(), &iw, &ih);
		window->paintIcon(iconfile.c_str(), BORDER_LEFT, cFrameBoxTitle.iHeight/2);
	}
	
	int stringstartposX = (cFrameBox.iWidth >> 1) - (neededWidth >> 1);
	window->RenderString( g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE], stringstartposX, cFrameBoxTitle.iHeight, cFrameBox.iWidth - (stringstartposX) , caption.c_str(), (CFBWindow::color_t)COL_MENUHEAD, 0, true); 

	// body
	window->paintBoxRel(0, cFrameBoxTitle.iHeight, cFrameBox.iWidth, (entries_per_page + 1)*cFrameBoxItem.iHeight, (CFBWindow::color_t)COL_MENUCONTENT_PLUS_0, RADIUS_MID, CORNER_BOTTOM);

	int count = entries_per_page;
	int ypos  = cFrameBoxTitle.iHeight + (cFrameBoxItem.iHeight >> 1);

	for (std::vector<char *>::const_iterator it = line.begin() + (entries_per_page * current_page); ((it != line.end()) && (count > 0)); it++, count--)
	{
		window->RenderString(g_Font[SNeutrinoSettings::FONT_TYPE_MENU], 10, (ypos += cFrameBoxItem.iHeight), cFrameBox.iWidth, *it, (CFBWindow::color_t)COL_MENUCONTENT, 0, true); 
	}

	// scrollBar
	if (entries_per_page < line.size())
	{
		ypos = cFrameBoxTitle.iHeight + (cFrameBoxItem.iHeight >> 1);
		window->paintBoxRel(cFrameBox.iWidth - SCROLLBAR_WIDTH, ypos, SCROLLBAR_WIDTH, entries_per_page*cFrameBoxItem.iHeight, COL_MENUCONTENT_PLUS_1);

		unsigned int marker_size = (entries_per_page*cFrameBoxItem.iHeight) / ((line.size() + entries_per_page - 1) / entries_per_page);

		window->paintBoxRel(cFrameBox.iWidth - (SCROLLBAR_WIDTH - 2), ypos + current_page*marker_size, SCROLLBAR_WIDTH - 4, marker_size, COL_MENUCONTENT_PLUS_3);
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
	if (window != NULL)
	{
		delete window;
		window = NULL;
	}	
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

//
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


