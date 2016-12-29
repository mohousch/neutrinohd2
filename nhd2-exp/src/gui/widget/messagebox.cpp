/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: messagebox.cpp 2013/10/12 mohousch Exp $

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

#include <gui/widget/messagebox.h>

#include <gui/widget/icons.h>
#include <driver/screen_max.h>
#include <global.h>
#include <neutrino.h>

#include <system/debug.h>


#define borderwidth 4

#define HINTBOXEXT_MAX_HEIGHT 420

CMessageBox::CMessageBox(const neutrino_locale_t Caption, const char * const Text, const int Width, const char * const Icon, const CMessageBox::result_ Default, const uint32_t ShowButtons)
{
	m_message = strdup(Text);

	char *begin   = m_message;

	begin = strtok(m_message, "\n");
	
	while (begin != NULL)
	{
		std::vector<Drawable*> oneLine;
		std::string s(begin);
		DText *d = new DText(s);
		oneLine.push_back(d);
		m_lines.push_back(oneLine);
		begin = strtok(NULL, "\n");
	}
	
	init(g_Locale->getText(Caption), Width, Icon);

	returnDefaultOnTimeout = false;

	m_height += (m_fheight << 1);

	result = Default;

	showbuttons = ShowButtons;

	int MaxButtonTextWidth = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getRenderWidth(g_Locale->getText(LOCALE_MESSAGEBOX_CANCEL), true); // UTF-8
	int ButtonWidth = 20 + 33 + MaxButtonTextWidth + 5;
	int num = 0;
	
	if (showbuttons & mbYes)
		num++;
	
	if (showbuttons & mbNo)
		num++;
	
	if (showbuttons & (mbCancel | mbBack | mbOk))
		num++;
	
	int new_width = 15 + num*ButtonWidth;
	if(new_width > m_width)
		m_width = new_width;
}

CMessageBox::CMessageBox(const neutrino_locale_t Caption, ContentLines& Lines, const int Width, const char * const Icon, const CMessageBox::result_ Default, const uint32_t ShowButtons)
{
	m_message = NULL;
	m_lines = Lines;
	init(g_Locale->getText(Caption), Width, Icon);

	returnDefaultOnTimeout = false;

	m_height += (m_fheight << 1);

	result = Default;

	showbuttons = ShowButtons;
	int MaxButtonTextWidth = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getRenderWidth(g_Locale->getText(LOCALE_MESSAGEBOX_CANCEL), true); // UTF-8
	int ButtonWidth = 20 + 33 + MaxButtonTextWidth + 5;
	int num = 0;
	
	if (showbuttons & mbYes)
		num++;
	
	if (showbuttons & mbNo)
		num++;
	
	if (showbuttons & (mbCancel | mbBack | mbOk))
		num++;
	
	int new_width = 15 + num*ButtonWidth;
	if(new_width > m_width)
		m_width = new_width;
}

CMessageBox::CMessageBox(const char* const Caption, const char * const Text, const int Width, const char * const Icon, const CMessageBox::result_ Default, const uint32_t ShowButtons)
{
	m_message = strdup(Text);

	char *begin   = m_message;

	begin = strtok(m_message, "\n");
	
	while (begin != NULL)
	{
		std::vector<Drawable*> oneLine;
		std::string s(begin);
		DText *d = new DText(s);
		oneLine.push_back(d);
		m_lines.push_back(oneLine);
		begin = strtok(NULL, "\n");
	}
	
	init(Caption, Width, Icon);

	returnDefaultOnTimeout = false;

	m_height += (m_fheight << 1);

	result = Default;

	showbuttons = ShowButtons;

	int MaxButtonTextWidth = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getRenderWidth(g_Locale->getText(LOCALE_MESSAGEBOX_CANCEL), true); // UTF-8
	int ButtonWidth = 20 + 33 + MaxButtonTextWidth + 5;
	int num = 0;
	
	if (showbuttons & mbYes)
		num++;
	
	if (showbuttons & mbNo)
		num++;
	
	if (showbuttons & (mbCancel | mbBack | mbOk))
		num++;
	
	int new_width = 15 + num*ButtonWidth;
	if(new_width > m_width)
		m_width = new_width;
}

CMessageBox::CMessageBox(const char* const Caption, ContentLines& Lines, const int Width, const char * const Icon, const CMessageBox::result_ Default, const uint32_t ShowButtons)
{
	m_message = NULL;
	m_lines = Lines;
	init(Caption, Width, Icon);

	returnDefaultOnTimeout = false;

	m_height += (m_fheight << 1);

	result = Default;

	showbuttons = ShowButtons;
	int MaxButtonTextWidth = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getRenderWidth(g_Locale->getText(LOCALE_MESSAGEBOX_CANCEL), true); // UTF-8
	int ButtonWidth = 20 + 33 + MaxButtonTextWidth + 5;
	int num = 0;
	
	if (showbuttons & mbYes)
		num++;
	
	if (showbuttons & mbNo)
		num++;
	
	if (showbuttons & (mbCancel | mbBack | mbOk))
		num++;
	
	int new_width = 15 + num*ButtonWidth;
	if(new_width > m_width)
		m_width = new_width;
}

CMessageBox::~CMessageBox(void)
{
	if (m_window != NULL)
	{
		delete m_window;
		m_window = NULL;
	}
	
	if (m_message != NULL) 
	{
		free(m_message);

		// content has been set using "m_message" so we are responsible to 
		// delete it
		for (ContentLines::iterator it = m_lines.begin(); it != m_lines.end(); it++)
		{
			for (std::vector<Drawable*>::iterator it2 = it->begin(); it2 != it->end(); it2++)
			{
				delete *it2;
			}
		}
	}
}

void CMessageBox::init(const char * const Caption, const int Width, const char * const Icon)
{
	m_width   = Width;
	int nw = 0;
	m_theight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	m_fheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	m_height  = m_theight + m_fheight;
	m_maxEntriesPerPage = 0;

	m_caption = Caption;

	int page = 0;
	int line = 0;
	int maxWidth = MENU_WIDTH - 50;
	int maxOverallHeight = 0;
	m_startEntryOfPage.push_back(0);
	
	for (ContentLines::iterator it = m_lines.begin(); it!=m_lines.end(); it++)
	{
		bool pagebreak = false;
		int maxHeight = 0;
		int lineWidth = 0;
		int count = 0;
		
		for (std::vector<Drawable*>::iterator item = it->begin(); item != it->end(); item++) 
		{
			if ((*item)->getHeight() > maxHeight)
				maxHeight = (*item)->getHeight();
			lineWidth += (*item)->getWidth();
			if ((*item)->getType() == Drawable::DTYPE_PAGEBREAK)
				pagebreak = true;
			
			count++;
		}
		
		// 10 pixels left and right of every item. determined empirically :-(
		lineWidth += count * 20;
		
                if (lineWidth > maxWidth)
			maxWidth = lineWidth;
		m_height += maxHeight;
		
		if (m_height > HINTBOXEXT_MAX_HEIGHT || pagebreak) 
		{
			if (m_height-maxHeight > maxOverallHeight)
				maxOverallHeight = m_height - maxHeight;
			
			m_height = m_theight + m_fheight + maxHeight;
			
			if (pagebreak)
				m_startEntryOfPage.push_back(line + 1);
			else 
				m_startEntryOfPage.push_back(line);
			
			page++;
			
			if (m_maxEntriesPerPage < (m_startEntryOfPage[page] - m_startEntryOfPage[page -1]))
			{
				m_maxEntriesPerPage = m_startEntryOfPage[page] - m_startEntryOfPage[page -1];
			}
		}
		line++;
	}

	//FIXME:???
	m_width = w_max(maxWidth, borderwidth); 
	// if there is only one page m_height is already correct 
	//but m_maxEntries has not been set
	if (m_startEntryOfPage.size() > 1)
	{
		m_height = maxOverallHeight;
		m_width += SCROLLBAR_WIDTH; // scroll bar
	} 
	else 
	{
		m_maxEntriesPerPage = line;
	}

	m_startEntryOfPage.push_back(line + 1); // needed to calculate amount of items on last page

	//m_width = w_max(maxWidth, borderwidth); 
	m_currentPage = 0;
	m_pages = page + 1;
	unsigned int additional_width;

	if (m_startEntryOfPage.size() > 1)
		additional_width = 20 + 15;
	else
		additional_width = 20 +  0;

	if (Icon != NULL)
	{
		m_iconfile = Icon;
		additional_width += 30;
	}
	else
		m_iconfile = "";

	nw = additional_width + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getRenderWidth(m_caption); // UTF-8

	if (nw > m_width)
		m_width = nw;

	m_window = NULL;
}

void CMessageBox::paint(void)
{
	dprintf(DEBUG_NORMAL, "CHintBoxExt::paint\n");

	if (m_window != NULL)
	{
		/*
		 * do not paint stuff twice:
		 * => thread safety needed by movieplayer.cpp:
		 *    one thread calls our paint method, the other one our hide method
		 * => no memory leaks
		 */
		return;
	}

        CFrameBuffer * frameBuffer = CFrameBuffer::getInstance();
        m_window = new CFBWindow(frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - m_width ) >> 1),
                               frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - m_height) >> 2),
                               m_width + borderwidth,
                               m_height + borderwidth);

	refresh(true);
}

void CMessageBox::refresh(bool paintBg)
{
	if (m_window == NULL)
	{
		return;
	}
	
	// paint shadow
	if(paintBg)
		m_window->paintBoxRel(borderwidth, borderwidth, m_width, m_height, COL_INFOBAR_SHADOW_PLUS_0, RADIUS_MID, CORNER_BOTH);
	
	// title
	m_window->paintBoxRel(0, 0, m_width, m_theight, (CFBWindow::color_t)COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, g_settings.Head_gradient);//round
	
	int neededWidth = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getRenderWidth(m_caption); // UTF-8

	if (!m_iconfile.empty())
	{
		m_window->paintIcon(m_iconfile.c_str(), 8, 15);
	}
	
	int stringstartposX = (m_width >> 1) - (neededWidth >> 1);
	m_window->RenderString( g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE], stringstartposX, m_theight, m_width - (stringstartposX) , m_caption.c_str(), (CFBWindow::color_t)COL_MENUHEAD, 0, true); // UTF-8

	// menu text panel
	m_window->paintBoxRel(0, m_theight, m_width, ((m_maxEntriesPerPage + 1)*m_fheight), (CFBWindow::color_t)COL_MENUCONTENT_PLUS_0);

	int yPos  = m_theight + (m_fheight >> 1);

	for (ContentLines::iterator it = m_lines.begin() + m_startEntryOfPage[m_currentPage]; it != m_lines.begin() + m_startEntryOfPage[m_currentPage + 1] && it != m_lines.end(); it++)
	{
		int xPos = 10;
		int maxHeight = 0;
		
		for (std::vector<Drawable*>::iterator d = it->begin(); d!=it->end(); d++)
		{
  			(*d)->draw(m_window, xPos, yPos, m_width - 20);
			xPos += (*d)->getWidth() + 20;
			
			if ((*d)->getHeight() > maxHeight)
				maxHeight = (*d)->getHeight();
		}
		yPos += maxHeight;
	}

	// paint scrollbar
	if (has_scrollbar()) 
	{
		yPos = m_theight;
		m_window->paintBoxRel(m_width - SCROLLBAR_WIDTH, yPos, SCROLLBAR_WIDTH, m_maxEntriesPerPage*m_fheight, COL_MENUCONTENT_PLUS_1);
		
		unsigned int marker_size = (m_maxEntriesPerPage*m_fheight) / m_pages;
		m_window->paintBoxRel(m_width - 13, yPos + m_currentPage * marker_size, 11, marker_size, COL_MENUCONTENT_PLUS_3);
	}
}

bool CMessageBox::has_scrollbar(void)
{
	return (m_startEntryOfPage.size() > 2);
}

void CMessageBox::scroll_up(void)
{
	if (m_currentPage > 0)
	{
		m_currentPage--;
		refresh();
	}
}

void CMessageBox::scroll_down(void)
{
	if (m_currentPage + 1 < m_startEntryOfPage.size() - 1)
	{
		m_currentPage++;
		refresh();
	}
}

void CMessageBox::hide(void)
{
	if (m_window != NULL)
	{
		delete m_window;
		m_window = NULL;
	}
}

void CMessageBox::returnDefaultValueOnTimeout(bool returnDefault)
{
	returnDefaultOnTimeout = returnDefault;
}

void CMessageBox::paintButtons()
{
	uint8_t    color;
	fb_pixel_t bgcolor;

	m_window->paintBoxRel(0, m_height - (m_fheight << 1), m_width, (m_fheight << 1), (CFBWindow::color_t)COL_MENUCONTENT_PLUS_0, RADIUS_MID, CORNER_BOTTOM);

	//irgendwann alle vergleichen - aber cancel ist sicher der l�ngste
	int MaxButtonTextWidth = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getRenderWidth(g_Locale->getText(LOCALE_MESSAGEBOX_CANCEL), true); // UTF-8

	int ButtonWidth = 20 + 33 + MaxButtonTextWidth;

	int ButtonSpacing = (m_width - 20 - (ButtonWidth * 3)) / 2;
	if(ButtonSpacing <= 5) 
		ButtonSpacing = 5;

	int xpos = BORDER_LEFT;
	int iw, ih;
	int fh = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight();
	const int noname = 20;
	
	// yes
	if (showbuttons & mbYes)
	{
		if (result == mbrYes)
		{
			color   = COL_MENUCONTENTSELECTED;
			bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
		}
		else
		{
			color   = COL_INFOBAR_SHADOW;
			bgcolor = COL_INFOBAR_SHADOW_PLUS_0;
		}
		

		m_window->paintBoxRel(xpos, m_height - m_fheight - noname, ButtonWidth, m_fheight, (CFBWindow::color_t)bgcolor);

		CFrameBuffer::getInstance()->getIconSize(NEUTRINO_ICON_BUTTON_RED, &iw, &ih);
		m_window->paintIcon(NEUTRINO_ICON_BUTTON_RED, xpos + 15, m_height - m_fheight - noname, m_fheight);

		m_window->RenderString(g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], xpos + 43, (m_height - noname)-(m_fheight-fh)/2, ButtonWidth- 53, g_Locale->getText(LOCALE_MESSAGEBOX_YES), (CFBWindow::color_t)color, 0, true); // UTF-8
		
		xpos += ButtonWidth + ButtonSpacing;
	}

	// no
	if (showbuttons & mbNo)
	{
		if (result == mbrNo)
		{
			color   = COL_MENUCONTENTSELECTED;
			bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
		}
		else
		{
			color   = COL_INFOBAR_SHADOW;
			bgcolor = COL_INFOBAR_SHADOW_PLUS_0;
		}

		m_window->paintBoxRel(xpos, m_height - m_fheight-noname, ButtonWidth, m_fheight, (CFBWindow::color_t)bgcolor);

		m_window->paintIcon(NEUTRINO_ICON_BUTTON_GREEN, xpos + 14, m_height - m_fheight - noname, m_fheight);

		m_window->RenderString(g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], xpos + 43, (m_height - noname)-(m_fheight-fh)/2, ButtonWidth- 53, g_Locale->getText(LOCALE_MESSAGEBOX_NO), (CFBWindow::color_t)color, 0, true); // UTF-8		
	
		xpos += ButtonWidth + ButtonSpacing;
	}


	// cancel|back|ok
	if (showbuttons & (mbCancel | mbBack | mbOk))
	{
		if (result >= mbrCancel)
		{
			color   = COL_MENUCONTENTSELECTED;
			bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
		}
		else
		{
			color   = COL_INFOBAR_SHADOW;
			bgcolor = COL_INFOBAR_SHADOW_PLUS_0;
		}

		m_window->paintBoxRel(xpos, m_height-m_fheight-noname, ButtonWidth, m_fheight, (CFBWindow::color_t)bgcolor);

		m_window->paintIcon(NEUTRINO_ICON_BUTTON_HOME, xpos + 14, m_height-m_fheight - noname, m_fheight);

		m_window->RenderString(g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], 
					xpos + 43, 
					(m_height - noname) - (m_fheight - fh)/2, 
					ButtonWidth - 53, 
					g_Locale->getText((showbuttons & mbCancel) ? LOCALE_MESSAGEBOX_CANCEL : (showbuttons & mbOk) ? LOCALE_MESSAGEBOX_OK : LOCALE_MESSAGEBOX_BACK), 
					(CFBWindow::color_t)color, 0, true); // UTF-8	
	}	
}

int CMessageBox::exec(int timeout)
{
	dprintf(DEBUG_NORMAL, "CMessageBox::exec: timeout:%d\n", timeout);

	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	// hintBoxExt paint
	paint();

	paintButtons();

	CFrameBuffer::getInstance()->blit();

	if ( timeout == -1 )
		timeout = g_settings.timing[SNeutrinoSettings::TIMING_EPG];

	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd( timeout );

	bool loop = true;
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if (msg == CRCInput::RC_timeout && returnDefaultOnTimeout)
		{
			// return default
			loop = false;
		}
		else if (((msg == CRCInput::RC_timeout) || (msg == CRCInput::RC_home)) && (showbuttons & (mbCancel | mbBack | mbOk)))
		{
			result = (showbuttons & mbCancel) ? mbrCancel : (showbuttons & mbOk) ? mbrOk: mbrBack;
			loop   = false;
		}
		else if ((msg == CRCInput::RC_green) && (showbuttons & mbNo))
		{
			result = mbrNo;
			loop   = false;
		}
		else if ((msg == CRCInput::RC_red) && (showbuttons & mbYes))
		{
			result = mbrYes;
			loop   = false;
		}
		else if(msg == CRCInput::RC_right)
		{
			bool ok = false;
			while (!ok)
			{
				result = (CMessageBox::result_)((result + 1) & 3);
				ok = showbuttons & (1 << result);
			}

			paintButtons();
		}
		else if (has_scrollbar() && ((msg == CRCInput::RC_up) || (msg == CRCInput::RC_down) || (msg == CRCInput::RC_page_up) || (msg == CRCInput::RC_page_down)))
		{
			if ( (msg == CRCInput::RC_up) || (msg == CRCInput::RC_page_up))
				scroll_up();
			else
				scroll_down();
			
			paintButtons();
		}
		else if(msg == CRCInput::RC_left)
		{
			bool ok = false;
			while (!ok)
			{
				result = (CMessageBox::result_)((result - 1) & 3);
				ok = showbuttons & (1 << result);
			}

			paintButtons();

		}
		else if(msg == CRCInput::RC_ok)
		{
			loop = false;
		}
		else if (CNeutrinoApp::getInstance()->handleMsg(msg, data) & messages_return::cancel_all)
		{
			loop = false;
		}

		CFrameBuffer::getInstance()->blit();
	}

	// hide
	hide();

	CFrameBuffer::getInstance()->blit();
	
	return result;
}

int MessageBox(const neutrino_locale_t Caption, const char * const Text, const CMessageBox::result_ Default, const uint32_t ShowButtons, const char * const Icon, const int Width, const int timeout, bool returnDefaultOnTimeout)
{
   	CMessageBox * messageBox = new CMessageBox(Caption, Text, Width, Icon, Default, ShowButtons);
	messageBox->returnDefaultValueOnTimeout(returnDefaultOnTimeout);
	messageBox->exec(timeout);
	int res = messageBox->result;
	delete messageBox;

	return res;
}

int MessageBox(const neutrino_locale_t Caption, const neutrino_locale_t Text, const CMessageBox::result_ Default, const uint32_t ShowButtons, const char * const Icon, const int Width, const int timeout, bool returnDefaultOnTimeout)
{
	return MessageBox(Caption, g_Locale->getText(Text), Default, ShowButtons, Icon, Width, timeout,returnDefaultOnTimeout);
}

int MessageBox(const neutrino_locale_t Caption, const std::string & Text, const CMessageBox::result_ Default, const uint32_t ShowButtons, const char * const Icon, const int Width, const int timeout, bool returnDefaultOnTimeout)
{
	return MessageBox(Caption, Text.c_str(), Default, ShowButtons, Icon, Width, timeout,returnDefaultOnTimeout);
}

int MessageBox(const char * const Caption, const char * const Text, const CMessageBox::result_ Default, const uint32_t ShowButtons, const char * const Icon, const int Width, const int timeout, bool returnDefaultOnTimeout)
{
   	CMessageBox * messageBox = new CMessageBox(Caption, Text, Width, Icon, Default, ShowButtons);
	messageBox->returnDefaultValueOnTimeout(returnDefaultOnTimeout);
	messageBox->exec(timeout);
	int res = messageBox->result;
	delete messageBox;

	return res;
}

int MessageBox(const char * const Caption, const neutrino_locale_t Text, const CMessageBox::result_ Default, const uint32_t ShowButtons, const char * const Icon, const int Width, const int timeout, bool returnDefaultOnTimeout)
{
	return MessageBox(Caption, g_Locale->getText(Text), Default, ShowButtons, Icon, Width, timeout,returnDefaultOnTimeout);
}

int MessageBox(const char * const Caption, const std::string & Text, const CMessageBox::result_ Default, const uint32_t ShowButtons, const char * const Icon, const int Width, const int timeout, bool returnDefaultOnTimeout)
{
	return MessageBox(Caption, Text.c_str(), Default, ShowButtons, Icon, Width, timeout,returnDefaultOnTimeout);
}


