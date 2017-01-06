/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: infobox.h 2016.12.02 12:26:30 mohousch Exp $

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

	***********************************************************
  
	Module Name: msgbox.h .

	Description: interface of the CMsgBox class

	Date:	  Nov 2005

	Author: Günther@tuxbox.berlios.org
		based on code of Steffen Hehn 'McClean'

	Revision History:
	Date			Author		Change Description
	   Nov 2005		Günther	initial implementation

*/

#if !defined(INFOBOX_H)
#define INFOBOX_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>

#include "textbox.h"

#include <gui/widget/icons.h>
#include <driver/framebuffer.h>


class CInfoBox  
{
	private:
		CBox	m_cBoxFrame;
		CBox	m_cBoxFrameText;
		CBox	m_cBoxFrameTitleRel;

		// variables
		std::string m_cIcon;
		std::string m_cTitle;

		int m_nMode;

		CFont *m_pcFontTitle;
		int m_nFontTitleHeight;

		CTextBox *m_pcTextBox;

		CFrameBuffer * m_pcWindow;

		bool bigFonts;

		bool hide(void);

		// functions
		void initVar(void);
		void initFramesRel(void);
		void refreshTitle(void);
		void refreshText(void);

		bool    paint(void);
		void    refresh(void);

		void    scrollPageDown(const int pages);
		void    scrollPageUp(const int pages);

		void setBigFonts();

	public:
		virtual ~CInfoBox();
		CInfoBox();
		CInfoBox(const char * text);
		CInfoBox(const char * text, 
				   CFont *fontText,
				   const int mode, 
				   const CBox* position, 
				   const char * title,
				   CFont *fontTitle,
				   const char * icon);

		// functions
		int     exec(int timeout = -1);
		bool	setText(const std::string* newText, std::string _thumbnail = "", int _tw = 0, int _th = 0, int tmode = CTextBox::TOP_RIGHT);
};

#endif
