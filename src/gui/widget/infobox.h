/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: msgbox.h 2013/10/12 mohousch Exp $

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


#define BIG_FONT_FAKTOR 1.5

class CInfoBox  
{
	public:
		// enum definition
		enum result_
		{
			mbrYes    = 0,
			mbrNo     = 1,
			mbrCancel = 2,
			mbrBack   = 3
		};
		
		enum buttons_
		{
			mbYes =		0x01,
			mbNo =		0x02,
			mbCancel =	0x04,
			mbAll =		0x07,
			mbBack =	0x08
		};
		
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
		// Functions
		void initVar(void);
		void initFramesRel(void);
		void refreshFoot(void);
		void refreshTitle(void);
		void refreshText(void);

		// Variables
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
		int m_nFootButtons;

		CTextBox *m_pcTextBox;

		CFrameBuffer * m_pcWindow;

		result_	m_nResult;

		bool bigFonts;

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
				   const char * icon,
				   int return_button = mbCancel, 
				   const result_ default_result = mbrCancel);

		// Functions
		bool    paint(void);
		bool    hide(void);
		int     exec(int timeout = -1, int returnDefaultOnTimeout = false);
		void    refresh(void);
		void    scrollPageDown(const int pages);
		void    scrollPageUp(const int pages);
		int	result(void);
		bool	setText(const std::string* newText, std::string _thumbnail = "", int _tw = 0, int _th = 0, int tmode = CTextBox::TOP_RIGHT);
};

extern int InfoBox(const neutrino_locale_t Caption, 
						const char * const Text, 
						const CInfoBox::result_ Default, 
						const uint32_t ShowButtons, 
						const char * const Icon = NULL, 
						const int Width = HINTBOX_WIDTH, 
						const int timeout = -1, 
						bool returnDefaultOnTimeout = false); // UTF-8

extern int InfoBox(const char * const Title, 
						const char * const Text, 
						const CInfoBox::result_ Default, 
						const uint32_t ShowButtons, 
						const char * const Icon = NULL, 
						const int Width = HINTBOX_WIDTH, 
						const int timeout = -1, 
						bool returnDefaultOnTimeout = false); // UTF-8

#endif
