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
*/

#if !defined(INFOBOX_H)
#define INFOBOX_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>

#include <global.h>

#include <gui/widget/textbox.h>
#include <gui/widget/icons.h>
#include <gui/widget/widget_helpers.h>

#include <driver/framebuffer.h>


class CInfoBox  
{
	private:
		CFrameBuffer * frameBuffer;

		CBox	m_cBoxFrame;
		CBox	m_cBoxFrameText;
		CBox	m_cBoxFrameTitleRel;
		CBox	m_cBoxFrameFootRel;

		// variables
		std::string m_cIcon;
		std::string m_cTitle;

		int m_nMode;

		CFont *m_pcFontTitle;
		int m_nFontTitleHeight;

		CFont* m_pcFontText;

		CTextBox *m_pcTextBox;

		bool bigFonts;

		bool hide(void);

		// functions
		void initVar(void);
		void initFramesRel(void);
		void refreshTitle(void);
		void refreshText(void);
		void refreshFoot(void);

		bool paint(void);
		void refresh(void);

		void scrollPageDown(const int pages);
		void scrollPageUp(const int pages);

		void setBigFonts();

	public:
		virtual ~CInfoBox();
		CInfoBox();
		CInfoBox(CFont *fontText,
				   const int mode, 
				   const CBox* position, 
				   const char * title,
				   CFont *fontTitle = g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE],
				   const char * icon = NEUTRINO_ICON_INFO);

		// functions
		int exec(int timeout = -1);
		bool setText(const char * const newText, const char * const _thumbnail = NULL, int _tw = 0, int _th = 0, int tmode = TOP_RIGHT);
};

//
void InfoBox(const char * const text, const char * const title, const char * const icon = NEUTRINO_ICON_INFO, const char * const thumbnail = NULL, int tw = 0, int th = 0, int tmode = TOP_RIGHT);

#endif
