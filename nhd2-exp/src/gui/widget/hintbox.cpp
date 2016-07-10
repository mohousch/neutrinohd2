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

	width   = Width;

	theight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	fheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	height = theight + fheight;

	caption = g_Locale->getText(Caption);

	begin = message;

	while (true)
	{
		height += fheight;
		if (height > HINTBOX_MAX_HEIGHT)
			height -= fheight;

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
	entries_per_page = ((height - theight) / fheight) - 1;
	current_page = 0;

	unsigned int additional_width;

	if (entries_per_page < line.size())
		additional_width = 20 + 15;
	else
		additional_width = 20 +  0;

	if (Icon != NULL)
	{
		iconfile = Icon;
		additional_width += 30;
	}
	else
		iconfile = "";

	nw = additional_width + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getRenderWidth(caption); // UTF-8

	if (nw > width)
		width = nw;

	for (std::vector<char *>::const_iterator it = line.begin(); it != line.end(); it++)
	{
		nw = additional_width + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(*it, true); // UTF-8
		if (nw > width)
			width = nw;
	}
	window = NULL;
}

CHintBox::CHintBox(const char * Caption, const char * const Text, const int Width, const char * const Icon)
{
	char * begin;
	char * pos;
	int    nw;

	message = strdup(Text);

	width   = Width;

	theight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	fheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	height = theight + fheight;

	caption = Caption;

	begin = message;

	while (true)
	{
		height += fheight;
		if (height > HINTBOX_MAX_HEIGHT)
			height -= fheight;

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
	entries_per_page = ((height - theight) / fheight) - 1;
	current_page = 0;

	unsigned int additional_width;

	if (entries_per_page < line.size())
		additional_width = 20 + 15;
	else
		additional_width = 20 +  0;

	if (Icon != NULL)
	{
		iconfile = Icon;
		additional_width += 30;
	}
	else
		iconfile = "";

	nw = additional_width + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getRenderWidth(caption); // UTF-8

	if (nw > width)
		width = nw;

	for (std::vector<char *>::const_iterator it = line.begin(); it != line.end(); it++)
	{
		nw = additional_width + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(*it, true); // UTF-8
		if (nw > width)
			width = nw;
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
	
	window = new CFBWindow(frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - width ) >> 1),
			       frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - height) >> 2),
			       width + borderwidth,
			       height + borderwidth);
	refresh();
	
	CFrameBuffer::getInstance()->blit();
}

void CHintBox::refresh(void)
{
	if (window == NULL)
	{
		return;
	}

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

	// title
	window->paintBoxRel(0, 
					0, 
					width, 
					theight, 
					(CFBWindow::color_t)COL_MENUHEAD_PLUS_0, 
					RADIUS_MID, CORNER_TOP, true);//round
	
	int neededWidth = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getRenderWidth(caption); // UTF-8

	if (!iconfile.empty())
	{
		int iw, ih;
		CFrameBuffer::getInstance()->getIconSize(iconfile.c_str(), &iw, &ih);
		window->paintIcon(iconfile.c_str(), BORDER_LEFT, theight/2);
	}
	
	int stringstartposX = (width >> 1) - (neededWidth >> 1);
	window->RenderString( g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE], stringstartposX, theight, width - (stringstartposX) , caption.c_str(), (CFBWindow::color_t)COL_MENUHEAD, 0, true); // UTF-8

	// body + foot
	window->paintBoxRel(0, 
					theight, 
					width, 
					(entries_per_page + 1) * fheight, 
					(CFBWindow::color_t)COL_MENUCONTENT_PLUS_0, 
					RADIUS_MID, CORNER_BOTTOM);//round

	int count = entries_per_page;
	int ypos  = theight + (fheight >> 1);

	for (std::vector<char *>::const_iterator it = line.begin() + (entries_per_page * current_page); ((it != line.end()) && (count > 0)); it++, count--)
		window->RenderString(g_Font[SNeutrinoSettings::FONT_TYPE_MENU], 10, (ypos += fheight), width, *it, (CFBWindow::color_t)COL_MENUCONTENT, 0, true); // UTF-8

	if (entries_per_page < line.size())
	{
		ypos = theight + (fheight >> 1);
		window->paintBoxRel(width - 15, ypos, 15, entries_per_page * fheight, COL_MENUCONTENT_PLUS_1);
		unsigned int marker_size = (entries_per_page * fheight) / ((line.size() + entries_per_page - 1) / entries_per_page);
		window->paintBoxRel(width - 13, ypos + current_page * marker_size, 11, marker_size, COL_MENUCONTENT_PLUS_3);
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

//
int HintBox(const neutrino_locale_t Caption, const char * const Text, const int Width, int timeout, const char * const Icon)
{
	neutrino_msg_t msg;
	neutrino_msg_data_t data;

 	CHintBox * hintBox = new CHintBox(Caption, Text, Width, Icon);
	hintBox->paint();
	
	CFrameBuffer::getInstance()->blit();

	if ( timeout == -1 )
		timeout = g_settings.timing[SNeutrinoSettings::TIMING_INFOBAR];

	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd( timeout );

	int res = messages_return::none;

	while ( ! ( res & ( messages_return::cancel_info | messages_return::cancel_all ) ) )
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if ((msg == CRCInput::RC_timeout) || (msg == CRCInput::RC_home) || (msg == CRCInput::RC_ok))
		{
				res = messages_return::cancel_info;
		}
		else if ((hintBox->has_scrollbar()) && ((msg == CRCInput::RC_up) || (msg == CRCInput::RC_down)))
		{
			if (msg == CRCInput::RC_up)
				hintBox->scroll_up();
			else
				hintBox->scroll_down();
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

	hintBox->hide();
		
	delete hintBox;
	return 1;
}

int HintBox(const neutrino_locale_t Caption, const neutrino_locale_t Text, const int Width, int timeout, const char * const Icon)
{
	return HintBox(Caption, g_Locale->getText(Text), Width, timeout, Icon);
}

int HintBox(const char * Caption, const char * const Text, const int Width, int timeout, const char * const Icon)
{
	neutrino_msg_t msg;
	neutrino_msg_data_t data;

 	CHintBox * hintBox = new CHintBox(Caption, Text, Width, Icon);
	hintBox->paint();
	
	CFrameBuffer::getInstance()->blit();

	if ( timeout == -1 )
		timeout = g_settings.timing[SNeutrinoSettings::TIMING_INFOBAR];

	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd( timeout );

	int res = messages_return::none;

	while ( ! ( res & ( messages_return::cancel_info | messages_return::cancel_all ) ) )
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if ((msg == CRCInput::RC_timeout) || (msg == CRCInput::RC_home) || (msg == CRCInput::RC_ok))
		{
				res = messages_return::cancel_info;
		}
		else if ((hintBox->has_scrollbar()) && ((msg == CRCInput::RC_up) || (msg == CRCInput::RC_down)))
		{
			if (msg == CRCInput::RC_up)
				hintBox->scroll_up();
			else
				hintBox->scroll_down();
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

	hintBox->hide();
		
	delete hintBox;
	return 1;
}

int HintBox(const char * Caption, const neutrino_locale_t Text, const int Width, int timeout, const char * const Icon)
{
	return HintBox(Caption, g_Locale->getText(Text), Width, timeout, Icon);
}


