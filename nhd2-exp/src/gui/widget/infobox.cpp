/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: infobox.cpp 2016.12.02 12:25:30 mohousch Exp $

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

#include <infobox.h>

#include <gui/widget/icons.h>
#include <neutrino.h>

#include <system/debug.h>


#define MAX_WINDOW_WIDTH  			(g_settings.screen_EndX - g_settings.screen_StartX )
#define MAX_WINDOW_HEIGHT 			(g_settings.screen_EndY - g_settings.screen_StartY - 40)	

#define MIN_WINDOW_WIDTH  			(MAX_WINDOW_WIDTH>>1)
#define MIN_WINDOW_HEIGHT 			40	

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Function Name:	CInfoBox	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
CInfoBox::CInfoBox(  const char * text, 
				   CFont *fontText,
				   const int _mode, 
				   const CBox* position, 
				   const char * title,
				   CFont *fontTitle,
				   const char * icon)
{
	initVar();

	if(title != NULL)		
		m_cTitle = title;
	
	if(fontTitle != NULL)	
		m_pcFontTitle = fontTitle;
	
	if(icon != NULL)		
		m_cIcon = icon;
	
	if(position != NULL)	
		m_cBoxFrame = *position;
	
	m_nMode	= _mode;

	// initialise the window frames first
	initFramesRel();

	m_pcTextBox = new CTextBox(text, fontText, _mode, &m_cBoxFrameText);

	if(_mode & CTextBox::AUTO_WIDTH || _mode & CTextBox::AUTO_HIGH)
	{
		// window might changed in size
		m_cBoxFrameText = m_pcTextBox->getWindowsPos();

		m_cBoxFrame.iWidth = m_cBoxFrameText.iWidth;
		m_cBoxFrame.iHeight = m_cBoxFrameText.iHeight +  m_cBoxFrameTitleRel.iHeight;

		initFramesRel();
	}

	if(_mode & CTextBox::CENTER)
	{
		m_cBoxFrame.iX = g_settings.screen_StartX + ((g_settings.screen_EndX - g_settings.screen_StartX - m_cBoxFrame.iWidth) >>1);
		m_cBoxFrame.iY = g_settings.screen_StartY + ((g_settings.screen_EndY - g_settings.screen_StartY - m_cBoxFrame.iHeight) >>2);
	}
}

//////////////////////////////////////////////////////////////////////
// Function Name:	CInfoBox	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
CInfoBox::CInfoBox(const char * text)
{
	initVar();

	m_pcTextBox = new CTextBox(text);
	
	// initialise the window frames first
	initFramesRel();
}

//////////////////////////////////////////////////////////////////////
// Function Name:	~CInfoBox	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
CInfoBox::~CInfoBox()
{
	if (m_pcTextBox != NULL)
	{
		delete m_pcTextBox;
		m_pcTextBox = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// Function Name:	InitVar	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
void CInfoBox::initVar(void)
{
	m_cTitle = "";
	m_cIcon = "";
	m_nMode = CTextBox::SCROLL;

	// set the title variables
	m_pcFontTitle  =  g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE];
	m_nFontTitleHeight = m_pcFontTitle->getHeight();

	// set the main frame to default
	m_cBoxFrame.iX = g_settings.screen_StartX + ((g_settings.screen_EndX - g_settings.screen_StartX - MIN_WINDOW_WIDTH) >>1);
	m_cBoxFrame.iWidth = MIN_WINDOW_WIDTH;
	m_cBoxFrame.iY = g_settings.screen_StartY + ((g_settings.screen_EndY - g_settings.screen_StartY - MIN_WINDOW_HEIGHT) >>2);
	m_cBoxFrame.iHeight = MIN_WINDOW_HEIGHT;

	frameBuffer = CFrameBuffer::getInstance();

	bigFonts = false;
}

//////////////////////////////////////////////////////////////////////
// Function Name:	InitFramesRel	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
void CInfoBox::initFramesRel(void)
{
	// init the title frame
	m_cBoxFrameTitleRel.iX		= m_cBoxFrame.iX;
	m_cBoxFrameTitleRel.iY		= m_cBoxFrame.iY;
	m_cBoxFrameTitleRel.iWidth	= m_cBoxFrame.iWidth;
	m_cBoxFrameTitleRel.iHeight	= m_nFontTitleHeight + 2;

	// init the text frame
	m_cBoxFrameText.iY		= m_cBoxFrameTitleRel.iY + m_cBoxFrameTitleRel.iHeight;
	m_cBoxFrameText.iX		= m_cBoxFrameTitleRel.iX;
	m_cBoxFrameText.iHeight		= m_cBoxFrame.iHeight - m_cBoxFrameTitleRel.iHeight;
	m_cBoxFrameText.iWidth		= m_cBoxFrame.iWidth;		
}

//////////////////////////////////////////////////////////////////////
// Function Name:	RefreshTitle	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
void CInfoBox::refreshTitle(void)
{
	// titlebox
	frameBuffer->paintBoxRel(m_cBoxFrameTitleRel.iX, 
					m_cBoxFrameTitleRel.iY, 
					m_cBoxFrameTitleRel.iWidth, 
					m_cBoxFrameTitleRel.iHeight, 
					COL_MENUHEAD_PLUS_0, 
					RADIUS_MID, CORNER_TOP, g_settings.Head_gradient);

	// icon
	int iw = 0;
	int ih = 0;
	if (!m_cIcon.empty())
	{
		frameBuffer->getIconSize(m_cIcon.c_str(), &iw, &ih);
	
		frameBuffer->paintIcon(m_cIcon.c_str(), m_cBoxFrameTitleRel.iX + BORDER_LEFT, m_cBoxFrameTitleRel.iY + (m_cBoxFrameTitleRel.iHeight - ih)/2);
	}

	//
	m_pcFontTitle->RenderString(m_cBoxFrameTitleRel.iX + BORDER_LEFT + iw + 5, m_cBoxFrameTitleRel.iY + m_cBoxFrameTitleRel.iHeight + (m_cBoxFrameTitleRel.iHeight - m_pcFontTitle->getHeight())/2, m_cBoxFrameTitleRel.iWidth - (BORDER_LEFT + BORDER_RIGHT + 2*iw + 5), m_cTitle.c_str(), COL_MENUHEAD, 0, true); // UTF-8
}

//////////////////////////////////////////////////////////////////////
// global Functions
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Function Name:	Hide	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
bool CInfoBox::hide(void)
{
	if(m_pcTextBox != NULL)
	{
		m_pcTextBox->hide();
	}

	// hide title
	frameBuffer->paintBackgroundBoxRel(m_cBoxFrame.iX, m_cBoxFrame.iY, m_cBoxFrame.iWidth, m_cBoxFrame.iHeight);
	
	frameBuffer->blit();
	
	return (true);
}

//////////////////////////////////////////////////////////////////////
// Function Name:	ScrollPageDown	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
void CInfoBox::scrollPageDown(const int pages)
{
	// send scroll up event to text box if there is one
	if(m_pcTextBox != NULL)
	{
		m_pcTextBox->scrollPageDown(pages);
	}

}

//////////////////////////////////////////////////////////////////////
// Function Name:	ScrollPageUp	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
void CInfoBox::scrollPageUp(const int pages)
{
	// send scroll up event to text box if there is one
	if(m_pcTextBox != NULL)
	{
		m_pcTextBox->scrollPageUp(pages);
	}
}

//////////////////////////////////////////////////////////////////////
// Function Name:	setBigFonts	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
void CInfoBox::setBigFonts()
{
	dprintf(DEBUG_NORMAL, "CInfoBox::setBigFonts\n");

	// send setBigFonts event to textbox if there is one
	if(m_pcTextBox != NULL)
	{
		m_pcTextBox->setBigFonts(bigFonts);
	}
}

//////////////////////////////////////////////////////////////////////
// Function Name:	Paint	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
bool CInfoBox::paint(void)
{
	dprintf(DEBUG_DEBUG, "CInfoBox::paint\n");

	// title
	refreshTitle();

	// textBox
	if(m_pcTextBox != NULL)
	{
		// set corner
		m_pcTextBox->setCorner(RADIUS_MID, CORNER_BOTTOM);

		// paint
		m_pcTextBox->paint();
	}
	
	return (true);
}

//////////////////////////////////////////////////////////////////////
// Function Name:	Refresh	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
void CInfoBox::refresh(void)
{
	//refresh title
	refreshTitle();

	// rep-draw textbox if there is one
	if(m_pcTextBox != NULL)
	{
		m_pcTextBox->refresh();
	}
}

//////////////////////////////////////////////////////////////////////
// Function Name:	Exec	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
int CInfoBox::exec(int timeout)
{
	dprintf(DEBUG_NORMAL, "CInfoBox::exec: timeout:%d\n", timeout);

	neutrino_msg_t      msg;
	neutrino_msg_data_t data;
	int res = menu_return::RETURN_REPAINT;

	// show infobox
	paint();
	frameBuffer->blit();

	if ( timeout == -1 )
		timeout = g_settings.timing[SNeutrinoSettings::TIMING_EPG];

	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd( timeout );

	bool loop = true;
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if (((msg == CRCInput::RC_timeout) || (msg == CRCInput::RC_home)))
		{
			loop = false;
		}
		else if (msg == CRCInput::RC_up || msg == CRCInput::RC_page_up)
		{
			scrollPageUp(1);
		}
		else if (msg == CRCInput::RC_down || msg == CRCInput::RC_page_down)
		{
			scrollPageDown(1);
		}
		else if(msg == CRCInput::RC_ok)
		{
			loop = false;
		}
		else if(msg == CRCInput::RC_info)
		{
			bigFonts = bigFonts? false : true;

			setBigFonts();
		}
		else if (CNeutrinoApp::getInstance()->handleMsg(msg, data) & messages_return::cancel_all)
		{
			res  = menu_return::RETURN_EXIT_ALL;
			loop = false;
		}

		frameBuffer->blit();
	}

	hide();
	
	return res;
}

//////////////////////////////////////////////////////////////////////
// Function Name:	SetText	
// Description:		
// Parameters:		
// Data IN/OUT:		
// Return:		
// Notes:		
//////////////////////////////////////////////////////////////////////
bool CInfoBox::setText(const std::string* newText, std::string _thumbnail, int _tw, int _th, int tmode)
{
	bool _result = false;
	
	// update text in textbox if there is one
	if(m_pcTextBox != NULL && newText != NULL)
	{
		_result = m_pcTextBox->setText(newText, _thumbnail, _tw, _th, tmode);
	}
	
	return(_result);
}


