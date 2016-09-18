/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: infobox.h 2016.01.21 16:53:30 mohousch Exp $

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
	public:
		 enum mode_
		{
			AUTO_WIDTH	= 0x01,
			AUTO_HIGH	= 0x02,
			SCROLL		= 0x04,
			TITLE		= 0x08,
			FOOT		= 0x10,
			CENTER		= 0x20,
			NO_AUTO_LINEBREAK= 0x40
		}mode;

	private:
		// functions
		void initVar(void);
		void initFramesRel(void);
		void refreshFoot(void);
		void refreshTitle(void);
		void refreshText(void);

		// variables
		std::string m_cIcon;
		std::string m_cTitle;

		CBox	m_cBoxFrame;
		CBox	m_cBoxFrameText;
		CBox	m_cBoxFrameTitleRel;
		CBox	m_cBoxFrameFootRel;

		int m_nMode;

		CFont *m_pcFontTitle;
		int m_nFontTitleHeight;

		CFont *m_pcFontFoot;
		int m_nFontFootHeight;

		CTextBox *m_pcTextBox;

		CFrameBuffer * m_pcWindow;

		bool bigFonts;

		bool hide(void);

	public:
		virtual ~CInfoBox();
		CInfoBox();
		CInfoBox(const char * text);
		CInfoBox(  const char * text, 
				   CFont *fontText,
				   const int mode, 
				   const CBox* position, 
				   const char * title,
				   CFont *fontTitle,
				   const char * icon);

		// functions
		bool    paint(void);
		int     exec(int timeout = -1);
		void    refresh(void);
		void    scrollPageDown(const int pages);
		void    scrollPageUp(const int pages);
		bool	setText(const std::string* newText, std::string _thumbnail = "", int _tw = 0, int _th = 0, int tmode = CTextBox::TOP_RIGHT);

		void setBigFonts();
};

extern void InfoBox(const neutrino_locale_t Caption, 
						const char * const Text, 
						const char * const Icon = NULL, 
						const int Width = HINTBOX_WIDTH, 
						const int timeout = -1); // UTF-8

extern void InfoBox(const char * const Title, 
						const char * const Text,  
						const char * const Icon = NULL, 
						const int Width = HINTBOX_WIDTH, 
						const int timeout = -1); // UTF-8

#endif
