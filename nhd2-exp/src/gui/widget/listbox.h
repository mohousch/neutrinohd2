/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: lisbox.h 2013/10/12 mohousch Exp $

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

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


#ifndef __listbox__
#define __listbox__

#include <string>

#include <driver/framebuffer.h>
#include <gui/widget/menue.h>
#include <gui/widget/window.h>
#include <gui/widget/scrollbar.h>


class CListBox : public CMenuWidget
{
	protected:
		CFrameBuffer*	frameBuffer;

		CBox cFrameBox;
		CBox cFrameTitle;
		CBox cFrameFoot;
		CBox cFrameItem;
		CBox cFrameScrollBar;
		CBox cFrameTitleInfo;
		CBox cFrameFootInfo;

		CIcon titleIcon;
		CIcon footIcon;

		CWindow cWindowBox;
		CWindow cWindowTitle;
		CWindow cWindowFoot;
		CWindow cWindowItem;
		CWindow cWindowTitleInfo;
		CWindow cWindowFootInfo;

		int timestr_len;

		//
		bool            modified;
		std::string	caption;

		unsigned int	selected;
		unsigned int	liststart;
		unsigned int	listmaxshow;
		
		//
		bool FootInfo;
		bool TitleInfo;
		bool PaintDate;

		//
		virtual void paintItem(int pos);
		virtual void paint();
		virtual	void paintHead();
		virtual void paintFoot();
		virtual void hide();
		virtual void paintFootInfo(int index);
		virtual void paintItem2DetailsLine(int pos);
		virtual void clearItem2DetailsLine();
		virtual void paintTitleInfo(int index = 0);
		
		//
		virtual void onRedKeyPressed(){};
		virtual void onGreenKeyPressed(){};
		virtual void onYellowKeyPressed(){};
		virtual void onBlueKeyPressed(){};
		virtual void onOkKeyPressed(){};
		virtual void onMenuKeyPressed(){};
		virtual void onInfoKeyPressed(){};
		virtual void onRightKeyPressed(){};
		virtual void onLeftKeyPressed(){};
		virtual void onMuteKeyPressed(){};
		virtual void onOtherKeyPressed( int /*key*/ ){};

		//
		virtual unsigned int getItemCount();

		//
		virtual int getItemHeight();
		virtual void paintItem(uint32_t itemNr, int paintNr, bool selected);

		//
		void setModified(void);

		//
		uint32_t sec_timer_id;
		void initFrames();

	public:
		CListBox(const char * const Caption, int _width = MENU_WIDTH, int _height = MENU_HEIGHT);
		CListBox(const neutrino_locale_t Caption, int _width = MENU_WIDTH, int _height = MENU_HEIGHT); 
		virtual int exec(CMenuTarget* parent, const std::string& actionKey);

		void enableFootInfo(void);
		void enableTitleInfo(void);
		void enablePaintDate(void){PaintDate = true;};
		//void setTitleIcon(const char* icon){titleIcon.setIcon(icon); initFrames();};
		//void setTitleIcon(const std::string& icon){titleIcon.setIcon(icon); initFrames();};
};

#endif
