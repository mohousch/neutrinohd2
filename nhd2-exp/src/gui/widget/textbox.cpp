/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: textbox.cpp 2013/10/12 mohousch Exp $

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

#include <unistd.h>

#include <textbox.h>

#include <gui/widget/icons.h>

#include <system/debug.h>


#define MAX_WINDOW_WIDTH  	(g_settings.screen_EndX - g_settings.screen_StartX - 40)
#define MAX_WINDOW_HEIGHT 	(g_settings.screen_EndY - g_settings.screen_StartY - 40)	

#define MIN_WINDOW_WIDTH  	((g_settings.screen_EndX - g_settings.screen_StartX)>>1)
#define MIN_WINDOW_HEIGHT 	40

#define THUMBNAIL_OFFSET	3

#define BIG_FONT_FAKTOR 1.5


CTextBox::CTextBox(const char * text, CFont * font_text, const int _mode, const CBox * position, fb_pixel_t textBackgroundColor)
{
	dprintf(DEBUG_DEBUG, "CTextBox::CTextBox:\r\n");
	
	initVar();

 	if(text != NULL)
		m_cText = text;
	
	if(font_text != NULL)	
		m_pcFontText = font_text;
	
	if(position != NULL)
	{
		m_cFrame = *position;
		m_nMaxHeight = m_cFrame.iHeight;
		m_nMaxWidth = m_cFrame.iWidth;
	}

	m_nMode	= _mode;
	m_tMode = TOP_RIGHT;

	// in case of auto line break, we do no support auto width  yet
	if( !(_mode & NO_AUTO_LINEBREAK))
	{
		m_nMode = m_nMode & ~AUTO_WIDTH; // delete any AUTO_WIDTH
	}

	dprintf(DEBUG_DEBUG, " CTextBox::CTextBox: m_cText: %d, m_nMode %d\t\r\n", m_cText.size(), m_nMode);

	m_textBackgroundColor = textBackgroundColor;
	m_nFontTextHeight = m_pcFontText->getHeight();
	
	dprintf(DEBUG_DEBUG, " CTextBox::CTextBox: m_nFontTextHeight: %d\t\r\n", m_nFontTextHeight);

	// initialise the window frames first
	initFramesRel();

	// than refresh text line array 
	refreshTextLineArray();
}

CTextBox::CTextBox(const char * text)
{
	dprintf(DEBUG_DEBUG, "CTextBox::CTextBox:\r\n");
	
	initVar();

	if(text != NULL)		
		m_cText = *text;

	// initialise the window frames first
	initFramesRel();

	// than refresh text line array 
	refreshTextLineArray();
}

CTextBox::CTextBox()
{
	dprintf(DEBUG_DEBUG, "CTextBox::CTextBox:\r\n");
	
	initVar();
	initFramesRel();
}

CTextBox::~CTextBox()
{
	dprintf(DEBUG_DEBUG, "CTextBox::~CTextBox\r\n");
	
	m_cLineArray.clear();
	hide();
}

void CTextBox::initVar(void)
{
	dprintf(DEBUG_DEBUG, "CTextBox::InitVar:\r\n");
	
	m_cText	= "";
	m_nMode = SCROLL;
	m_tMode = TOP_RIGHT;

	m_pcFontText = g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1];
	m_nFontTextHeight = m_pcFontText->getHeight();

	m_nNrOfPages = 1;
	m_nNrOfLines = 0;
	m_nLinesPerPage = 0;
	m_nCurrentLine = 0;
	m_nCurrentPage = 0;

	m_cFrame.iX = g_settings.screen_StartX + ((g_settings.screen_EndX - g_settings.screen_StartX - MIN_WINDOW_WIDTH) >>1);
	m_cFrame.iWidth	= MIN_WINDOW_WIDTH;
	m_cFrame.iY = g_settings.screen_StartY + ((g_settings.screen_EndY - g_settings.screen_StartY - MIN_WINDOW_HEIGHT) >>1);
	m_cFrame.iHeight = MIN_WINDOW_HEIGHT;

	m_nMaxHeight = MAX_WINDOW_HEIGHT;
	m_nMaxWidth = MAX_WINDOW_WIDTH;
	
	m_textBackgroundColor = COL_MENUCONTENT_PLUS_0;

	m_cLineArray.clear();
	
	lx = m_cFrame.iX + 10;
	ly = m_cFrame.iY + 10;
	tw = th = 0;
	thumbnail = "";

	bigFonts = false;

	radius = NO_RADIUS;
	type = CORNER_NONE;
	gradient = nogradient;

	painted = false;
}

void CTextBox::setCorner(int Radius, int Type)
{
	radius = Radius;
	type = Type;

	//
	if(m_nMode & SCROLL)
	{
		m_cFrameScrollRel.iX = m_cFrame.iX + m_cFrame.iWidth - SCROLLBAR_WIDTH;
		m_cFrameScrollRel.iY = m_cFrame.iY;
		m_cFrameScrollRel.iWidth = SCROLLBAR_WIDTH;
		m_cFrameScrollRel.iHeight = radius? m_cFrame.iHeight - 2*radius : m_cFrame.iHeight;
	}
	else
	{
		m_cFrameScrollRel.iX = 0;
		m_cFrameScrollRel.iY = 0;
		m_cFrameScrollRel.iHeight = 0;
		m_cFrameScrollRel.iWidth = 0;
	}
}

void CTextBox::setBigFonts(bool bigfont)
{
	dprintf(DEBUG_NORMAL, "CTextBox::setBigFonts\n");

	bigFonts = bigfont;

	if(bigFonts)
	{
		m_pcFontText->setSize((int)(m_pcFontText->getSize() * BIG_FONT_FAKTOR));
	}
	else
	{
		m_pcFontText->setSize((int)(m_pcFontText->getSize() / BIG_FONT_FAKTOR));
	}

	refreshTextLineArray();
	refresh();
}

void CTextBox::reSizeMainFrameWidth(int textWidth)
{
	dprintf(DEBUG_DEBUG, "CTextBox::ReSizeMainFrameWidth: %d, current: %d\r\n", textWidth, m_cFrameTextRel.iWidth);

	int iNewWindowWidth = textWidth + m_cFrameScrollRel.iWidth + BORDER_LEFT + BORDER_RIGHT;

	if( iNewWindowWidth > m_nMaxWidth) 
		iNewWindowWidth = m_nMaxWidth;
	if( iNewWindowWidth < MIN_WINDOW_WIDTH) 
		iNewWindowWidth = MIN_WINDOW_WIDTH;

	m_cFrame.iWidth	= iNewWindowWidth;

	// Re-Init the children frames due to new main window
	initFramesRel();
}

void CTextBox::reSizeMainFrameHeight(int textHeight)
{
	dprintf(DEBUG_DEBUG, "CTextBox::ReSizeMainFrameHeight: %d, current: %d\r\n", textHeight, m_cFrameTextRel.iHeight);

	int iNewWindowHeight = textHeight + BORDER_LEFT + BORDER_RIGHT;

	if( iNewWindowHeight > m_nMaxHeight) 
		iNewWindowHeight = m_nMaxHeight;
	if( iNewWindowHeight < MIN_WINDOW_HEIGHT) 
		iNewWindowHeight = MIN_WINDOW_HEIGHT;

	m_cFrame.iHeight = iNewWindowHeight;

	// reinit the children frames due to new main window
	initFramesRel();
}

void CTextBox::initFramesRel(void)
{
	dprintf(DEBUG_DEBUG, "CTextBox::InitFramesRel:\r\n");

	m_cFrameTextRel.iX = m_cFrame.iX + BORDER_LEFT;
	m_cFrameTextRel.iY = m_cFrame.iY + 10;
	m_cFrameTextRel.iHeight	= m_cFrame.iHeight - 20;
	
	if(m_nMode & SCROLL)
	{
		m_cFrameScrollRel.iX = m_cFrame.iX + m_cFrame.iWidth - SCROLLBAR_WIDTH;
		m_cFrameScrollRel.iY = m_cFrame.iY;
		m_cFrameScrollRel.iWidth = SCROLLBAR_WIDTH;
		m_cFrameScrollRel.iHeight = radius? m_cFrame.iHeight - 2*radius : m_cFrame.iHeight;
	}
	else
	{
		m_cFrameScrollRel.iX = 0;
		m_cFrameScrollRel.iY = 0;
		m_cFrameScrollRel.iHeight = 0;
		m_cFrameScrollRel.iWidth = 0;
	}

	m_cFrameTextRel.iWidth = m_cFrame.iWidth - BORDER_LEFT - BORDER_RIGHT - m_cFrameScrollRel.iWidth;

	m_nLinesPerPage = (m_cFrameTextRel.iHeight)/m_nFontTextHeight;
}

void CTextBox::refreshTextLineArray(void)
{      
	dprintf(DEBUG_DEBUG, "CTextBox::RefreshLineArray:\r\n");
	
	int loop = true;
	int pos_prev = 0;
	int pos = 0;
	int aktWidth = 0;
	int aktWordWidth = 0;
	int lineBreakWidth;
	int maxTextWidth = 0;

	m_nNrOfNewLine = 0;

	std::string aktLine = "";
	std::string aktWord = "";

	// clear current line vector
	m_cLineArray.clear();
	m_nNrOfLines = 0;

	if( m_nMode & AUTO_WIDTH)
	{
		// In case of autowidth, we calculate the max allowed width of the textbox
		lineBreakWidth = MAX_WINDOW_WIDTH - BORDER_LEFT - BORDER_RIGHT - m_cFrameScrollRel.iWidth;
	}
	else
	{
		// If not autowidth, we just take the actuall textframe width
		lineBreakWidth = m_cFrameTextRel.iWidth;
	}
	
	//
	if( (!access(thumbnail.c_str(), F_OK) && m_nCurrentPage == 0) && m_tMode != TOP_CENTER)
		lineBreakWidth = m_cFrameTextRel.iWidth - tw - 10;
	
	const int TextChars = m_cText.size();
	
	// do not parse, if text is empty 
	if(!m_cText.empty())
	{
		while(loop)
		{
			if(m_nMode & NO_AUTO_LINEBREAK)
			{
				pos = m_cText.find_first_of("\n", pos_prev);
			}
			else
			{
				pos = m_cText.find_first_of("\n-. ", pos_prev);
			}

			//if(pos == -1)
			if(pos > TextChars || pos < 0)
			{
				//pos = TextChars + 1;
				//loop = false; // note, this is not 100% correct. if the last characters does not fit in one line, the characters after are cut
				break;
			}

			aktWord = m_cText.substr(pos_prev, pos - pos_prev + 1);
			aktWordWidth = m_pcFontText->getRenderWidth(aktWord, true);
			pos_prev = pos + 1;
			
			// if(aktWord.find("&quot;") == )
			if(1)
			{
				if( aktWidth + aktWordWidth > lineBreakWidth && !(m_nMode & NO_AUTO_LINEBREAK))
				{
					// we need a new line before we can continue
					m_cLineArray.push_back(aktLine);
					
					m_nNrOfLines++;
					aktLine = "";
					aktWidth = 0;

					if(pos_prev >= TextChars) 
						loop = false;
				}

				aktLine  += aktWord;
				aktWidth += aktWordWidth;
				if (aktWidth > maxTextWidth) 
					maxTextWidth = aktWidth;

				if( m_cText[pos] == '\n' || loop == false)
				{
					// current line ends with an carriage return, make new line
					if (m_cText[pos] == '\n')
						aktLine.erase(aktLine.size() - 1, 1);
					m_cLineArray.push_back(aktLine);
					m_nNrOfLines++;
					aktLine = "";
					aktWidth = 0;
					m_nNrOfNewLine++;
					
					if(pos_prev >= TextChars) 
						loop = false;
				}
				
				//recalculate breaklinewidth for other pages or when pic dont exists
				if(m_nNrOfLines > (th / m_nFontTextHeight))
				{
					if( m_nMode & AUTO_WIDTH)
					{
						lineBreakWidth = MAX_WINDOW_WIDTH - m_cFrameScrollRel.iWidth - BORDER_LEFT - BORDER_RIGHT;
					}
					else
					{
						lineBreakWidth = m_cFrameTextRel.iWidth;
					}
				}

				// 2nd page and over
				if(m_nNrOfLines > ((m_cFrameTextRel.iHeight) / m_nFontTextHeight))
				{
					if( m_nMode & AUTO_WIDTH)
					{
						lineBreakWidth = MAX_WINDOW_WIDTH - m_cFrameScrollRel.iWidth - BORDER_LEFT - BORDER_RIGHT;
					}
					else
					{
						lineBreakWidth = m_cFrameTextRel.iWidth;
					}
				}
			}
		}

		// check if we have to recalculate the window frame size, due to auto width and auto height
		if( m_nMode & AUTO_WIDTH)
		{
			reSizeMainFrameWidth(maxTextWidth);
		}

		if(m_nMode & AUTO_HIGH)
		{
			reSizeMainFrameHeight(m_nNrOfLines * m_nFontTextHeight);
		}

		// linesPerPage
		m_nLinesPerPage = (m_cFrameTextRel.iHeight) / m_nFontTextHeight;

		if(m_tMode == TOP_CENTER && m_nCurrentPage == 0)
			m_nLinesPerPage = (m_cFrameTextRel.iHeight - th - 10) / m_nFontTextHeight;

		// NrOfPages
		m_nNrOfPages =	((m_nNrOfLines - 1) / m_nLinesPerPage) + 1;

		if(m_nCurrentPage >= m_nNrOfPages)
		{
			m_nCurrentPage = m_nNrOfPages - 1;
			m_nCurrentLine = m_nCurrentPage * m_nLinesPerPage;
		}
	}
	else
	{
		m_nNrOfPages = 0;
		m_nNrOfLines = 0;
		m_nCurrentPage = 0;
		m_nCurrentLine = 0;
		m_nLinesPerPage = 1;
	}
}

void CTextBox::refreshScroll(void)
{
	if(!(m_nMode & SCROLL)) 
		return;

	if (m_nNrOfPages > 1) 
	{
		scrollBar.paint(&m_cFrameScrollRel, m_nNrOfPages, m_nCurrentPage);
	}
}

void CTextBox::refreshText(void)
{
	dprintf(DEBUG_DEBUG, "CTextBox::refreshText:\r\n");

	// paint text background
	m_cBoxWindow.setDimension(&m_cFrame);
	m_cBoxWindow.setColor(m_textBackgroundColor);
	m_cBoxWindow.setCorner(radius, type);
	m_cBoxWindow.setGradient(gradient);
	m_cBoxWindow.paint();
	
	// paint thumbnail (paint picture only on first page)
	if(m_nCurrentPage == 0 && !access(thumbnail.c_str(), F_OK) )
	{
		CFrameBuffer::getInstance()->paintFrameBox(lx, ly, tw, th, COL_WHITE);
		
		// picture
		CFrameBuffer::getInstance()->displayImage(thumbnail.c_str(), lx + THUMBNAIL_OFFSET, ly + THUMBNAIL_OFFSET, tw - THUMBNAIL_OFFSET, th - THUMBNAIL_OFFSET);
	}
	
	// paint text
	int y = m_cFrameTextRel.iY + 10;
	int i;
	int x_start = 0;

	if(m_tMode == TOP_CENTER && m_nCurrentPage == 0)
	{
		y = y + th + 10;
	}

	for(i = m_nCurrentLine; i < m_nNrOfLines && i < m_nCurrentLine + m_nLinesPerPage; i++)
	{
		y += m_nFontTextHeight;

		// x_start		
		if( !access(thumbnail.c_str(), F_OK) && (m_nCurrentPage == 0))
		{
			if (m_tMode == TOP_LEFT)
			{
				if(i <= (th / m_nFontTextHeight))
					x_start = tw;
			}
		}

		m_pcFontText->RenderString(m_cFrameTextRel.iX + x_start, y, m_cFrameTextRel.iWidth, m_cLineArray[i].c_str(), COL_MENUCONTENT, 0, true); // UTF-8
	}
}

void CTextBox::scrollPageDown(const int pages)
{
	if( !(m_nMode & SCROLL)) 
		return;
	
	if( m_nNrOfLines <= 0) 
		return;
	
	dprintf(DEBUG_DEBUG, "CTextBox::ScrollPageDown:\r\n");

	if(m_nCurrentPage + pages < m_nNrOfPages)
	{
		m_nCurrentPage += pages; 
	}
	else 
	{
		m_nCurrentPage = m_nNrOfPages - 1;
	}
	
	m_nCurrentLine = m_nCurrentPage * m_nLinesPerPage; 
	refresh();
}

void CTextBox::scrollPageUp(const int pages)
{
	if( !(m_nMode & SCROLL)) 
		return;
	
	if( m_nNrOfLines <= 0) 
		return;
	
	dprintf(DEBUG_DEBUG, "CTextBox::ScrollPageUp:\r\n");

	if(m_nCurrentPage - pages > 0)
	{
		m_nCurrentPage -= pages; 
	}
	else 
	{
		m_nCurrentPage = 0;
	}
	
	m_nCurrentLine = m_nCurrentPage * m_nLinesPerPage; 
	refresh();
}

void CTextBox::refresh(void)
{
	dprintf(DEBUG_DEBUG, "CTextBox::Refresh:\r\n");

	// paint text
	refreshText();

	// paint scrollbar
	refreshScroll();	
}

bool CTextBox::setText(const std::string* newText, std::string _thumbnail, int _tw, int _th, int _tmode)
{
	dprintf(DEBUG_INFO, "CTextBox::setText:\r\n");

	m_tMode = _tmode;

	// thumbnail
	thumbnail = "";
	
	if(!_thumbnail.empty() && !access(_thumbnail.c_str(), F_OK))
	{
		thumbnail = _thumbnail;

		tw = _tw;
		th = _th;

		// check th
		if(m_cFrame.iHeight > MAX_WINDOW_HEIGHT/2)
		{
			if(th > m_cFrame.iHeight/2)
				th = m_cFrame.iHeight/2 - 20;
		}
		else /*if(m_cFrame.iHeight <= MAX_WINDOW_HEIGHT/2)*/
		{
			if(th >= (m_cFrame.iHeight - 20))
				th = m_cFrame.iHeight - 20;
		}

		if(th >= (m_cFrame.iHeight - 20))
			th = m_cFrame.iHeight - 20;
		
		// check tw
		if(m_cFrame.iWidth > MAX_WINDOW_WIDTH/2)
		{
			if(tw > m_cFrame.iWidth/2)
				tw = m_cFrame.iWidth/2 - 20;
		}
		else if(m_cFrame.iWidth <= MAX_WINDOW_WIDTH/2)
		{
			if(tw >= m_cFrame.iWidth)
				tw = m_cFrame.iWidth - 20;
		}

		// position
		if(m_tMode == TOP_RIGHT)
		{
			lx = m_cFrame.iX + m_cFrame.iWidth - (tw + SCROLLBAR_WIDTH + 10);
			ly = m_cFrame.iY + 10;
		}
		else if(m_tMode == TOP_LEFT)
		{
			lx = m_cFrame.iX + 10;
			ly = m_cFrame.iY + 10;
		}
		else if(m_tMode == TOP_CENTER)
		{
			lx = m_cFrame.iX + (m_cFrame.iWidth - tw)/2;
			ly = m_cFrame.iY + 10;
		}
	}
	
	// text
	m_nNrOfPages = 0;
	m_nNrOfLines = 0;
	m_nCurrentPage = 0;
	m_nCurrentLine = 0;
	m_nLinesPerPage = 1;
		
	bool result = false;
	
	if (newText != NULL || !thumbnail.empty())
	{
		m_cText = *newText;
		refreshTextLineArray();
		refresh();
		
		result = true;
	}
	
	return(result);
}

void CTextBox::paint(void)
{
	dprintf(DEBUG_NORMAL, "CTextBox::paint:\n");

	painted = true;
	refresh();
}

void CTextBox::hide(void)
{
	if (bigFonts) 
	{
		bigFonts = false;
		m_pcFontText->setSize((int)(m_pcFontText->getSize() / BIG_FONT_FAKTOR));
	}
	
	m_cBoxWindow.setDimension(&m_cFrame);
	m_cBoxWindow.hide();

	painted = false;
}

