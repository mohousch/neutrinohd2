/*
	$Id: widget.h 11.03.2020 mohousch Exp $


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
#if !defined(WIDGET_H_)
#define WIDGET_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gui/widget/widget_helpers.h>


//
struct menu_return
{
	enum
	{
		RETURN_NONE	= 0,
		RETURN_REPAINT 	= 1,
		RETURN_EXIT 	= 2,
		RETURN_EXIT_ALL = 4
	};
};

class CMenuTarget
{
	protected:
		std::string *valueString;
		std::string valueStringTmp;

	public:
		CMenuTarget(){ valueStringTmp = std::string(); valueString = &valueStringTmp; };
		virtual ~CMenuTarget(){};
		virtual void hide(){};
		virtual int exec(CMenuTarget *parent, const std::string &actionKey) = 0;
		virtual std::string& getString(void) { return *valueString; };
};

class CWidget : public CMenuTarget
{
	protected:
		CFrameBuffer *frameBuffer;
		CBox mainFrameBox;

		std::vector<CWidgetItem*> items;

		bool paintMainFrame;

		//
		neutrino_msg_t      msg;
		neutrino_msg_data_t data;
		
		int selected;
		bool exit_pressed;
		
		fb_pixel_t * background;
		bool savescreen;
		void saveScreen();
		void restoreScreen();

		bool enableCenter;

		struct keyAction { std::string action; CMenuTarget *menue; };
		std::map<neutrino_msg_t, keyAction> keyActionMap;

		unsigned long long int timeout;
		uint32_t sec_timer_id;

		fb_pixel_t backgroundColor;

	public:
		CWidget();
		CWidget(const int x, const int y, const int dx = MENU_WIDTH, const int dy = MENU_HEIGHT);
		CWidget(CBox *position);
		virtual ~CWidget();

		virtual void initFrames();
		virtual void paintItems();
		virtual void paint();
		virtual void hide();
		virtual int exec(CMenuTarget *parent, const std::string &actionKey);

		virtual void addItem(CWidgetItem *widgetItem, const int x = 0, const int y = 0, const int dx = MENU_WIDTH, const int dy = MENU_HEIGHT, const bool defaultselected = false);
		bool hasItem();
		int getItemsCount()const{return items.size();};
		virtual void clearItems(void){items.clear();};

		void disableCenter(){enableCenter = false;};

		void setTimeOut(int to = 0){timeout = to;};

		void addKey(neutrino_msg_t key, CMenuTarget *menue = NULL, const std::string &action = "");
		neutrino_msg_t getKey(){return msg;};

		inline CBox getWindowsPos(void){return(mainFrameBox);};
		bool getExitPressed(){return exit_pressed;};

		void setSelected(unsigned int _new) { if(_new <= items.size()) selected = _new; if (selected < 0) selected = 0;};
		int getSelected(){return exit_pressed ? -1 : selected;};

		void enablePaintMainFrame(){paintMainFrame = true;};
		void setBackgroundColor(fb_pixel_t col) {backgroundColor = col;};

		void enableSaveScreen();
};

#endif // WIDGET_H_

