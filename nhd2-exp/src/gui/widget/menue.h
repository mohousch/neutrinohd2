/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: menue.h 2013/10/12 mohousch Exp $

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


#ifndef __MENU__
#define __MENU__

#include <string>
#include <vector>

#include <driver/framebuffer.h>
#include <driver/rcinput.h>

#include <system/localize.h>
#include <system/helpers.h>

#include <gui/widget/widget_helpers.h>

#include <driver/color.h>
#include <gui/plugins.h>

#include <system/settings.h>
#include <gui/widget/listbox.h>


/// CMenulistBox
class ClistBoxWidget : public CMenuTarget
{
	protected:
		//
		CFrameBuffer *frameBuffer;

		//
		std::string nameString;
		neutrino_locale_t name;
		std::vector<CMenuItem*>	items;
		std::vector<unsigned int> page_start;
		std::string iconfile;

		int width;
		int height;
		int wanted_height;
		int wanted_width;
		int x;
		int y;
		int offx, offy;
		int iconOffset;
		unsigned int item_start_y;
		unsigned int current_page;
		unsigned int total_pages;

		//
		neutrino_msg_t      msg;
		neutrino_msg_data_t data;
		
		int selected;
		bool exit_pressed;
		
		fb_pixel_t * background;
		int full_width;
		int full_height;
		bool savescreen;
		
		void Init(const std::string & Icon, const int mwidth, const int mheight);
		virtual void paintItems();
		
		void saveScreen();
		void restoreScreen();
		
		int hheight;
		int fheight;
		int item_height;
		int item_width;
		int sb_width;
		int items_height;
		int items_width;

		// foot buttons
		int fbutton_count;
		int fbutton_width;
		button_label_list_t fbutton_labels;

		// head buttons
		int hbutton_count;
		button_label_list_t hbutton_labels;

		//
		struct keyAction { 
			std::string action; 
			CMenuTarget *menue; 
		};
		std::map<neutrino_msg_t, keyAction> keyActionMap;

		//
		bool PaintDate;
		int timestr_len;
		uint32_t sec_timer_id;

		// head
		int icon_head_w;
		int icon_head_h;
		const char * l_name;

		// itemInfo
		bool paintFootInfo;
		CBox cFrameFootInfo;
		int footInfoHeight;
		int connectLineWidth;
		int footInfoMode;

		//
		unsigned long long int timeout;

		//
		int widgetType;
		bool widgetChange;
		std::vector<int> widget;

		// frame
		//fb_pixel_t backgroundColor;
		//fb_pixel_t itemBoxColor;
		int itemsPerX;
		int itemsPerY;
		int maxItemsPerPage;

		bool shrinkMenu;

		//
		//CHeaders * headers;
		CFooters *footers;
		CItems2DetailsLine itemsLine;
		CScrollBar scrollBar;
		CButtons buttons;

		int widgetMode;
		bool MenuPos;

		std::string actionKey;
		
	public:
		ClistBoxWidget();
		ClistBoxWidget(const char * const Name, const std::string& Icon = "", const int mwidth = MENU_WIDTH, const int mheight = MENU_HEIGHT);
		ClistBoxWidget(const neutrino_locale_t Name, const std::string& Icon = "", const int mwidth = MENU_WIDTH, const int mheight = MENU_HEIGHT);
		
		~ClistBoxWidget();

		virtual void addItem(CMenuItem * menuItem, const bool defaultselected = false);
		virtual void removeItem(long pos);
		bool hasItem();
		int getItemsCount()const{return items.size();};
		void clearItems(void){items.clear();};
		void clearAll(void){clearItems(); hbutton_labels.clear(); fbutton_labels.clear(); widget.clear();};

		//
		virtual void initFrames();
		virtual void paintHead();
		virtual void paintFoot();
		virtual void paint();
		virtual void paintItemInfo(int pos);
		virtual void hideItemInfo();
		virtual void hide();

		//
		virtual int exec(CMenuTarget * parent, const std::string &actionKey);

		void setTitle(const char* title = "", const char* icon = NULL){nameString = title; if(icon != NULL) iconfile = icon;};

		void setSelected(unsigned int _new) { if(_new <= items.size()) selected = _new; if (selected < 0) selected = 0;};
		int getSelected(){return selected;};

		void move(int xoff, int yoff);
		int getHeight(void) const {return height;}
		int getWidth(void) const {return width;};
		int getX(void) const {return x;};
		int getY(void) const {return y;};
		bool getExitPressed(){return exit_pressed;};
		
		//
		void enableSaveScreen();

		//
		void addKey(neutrino_msg_t key, CMenuTarget *menue = NULL, const std::string &action = "");
		neutrino_msg_t getKey(){return msg;};

		//
		void setFooterButtons(const struct button_label *_fbutton_label, const int _fbutton_count = 1, const int _fbutton_width = 0);

		//
		void setHeaderButtons(const struct button_label* _hbutton_label, const int _hbutton_count = 1);

		//
		void enablePaintDate(void){PaintDate = true;};

		//
		void enablePaintFootInfo(int fh = 70){paintFootInfo = true; footInfoHeight = fh; initFrames();};
		void setFootInfoMode(int mode = FOOT_INFO_MODE){footInfoMode = mode;};

		void setTimeOut(int to = 0){timeout = to;};

		//
		void setWidgetType(int type){widgetType = type; widget.push_back(widgetType);};
		int getWidgetType(){return widgetType;};
		void enableWidgetChange(){widgetChange = true;};
		void addWidget(int wtype){widget.push_back(wtype);};

		//
		//void setBackgroundColor(fb_pixel_t col) {backgroundColor = col;};
		//void setItemBoxColor(fb_pixel_t col) {itemBoxColor = col;};

		//
		void setItemsPerPage(int itemsX = 6, int itemsY = 3){itemsPerX = itemsX; itemsPerY = itemsY; maxItemsPerPage = itemsPerX*itemsPerY;};

		void enableShrinkMenu(){shrinkMenu = true;};
		void enableCenterPos(){};

		virtual void integratePlugins(CPlugins::i_type_t integration = CPlugins::I_TYPE_DISABLED, const unsigned int shortcut = RC_nokey, bool enabled = true);

		void setMode(int mode){widgetMode = mode;};
		void enableMenuPosition(){MenuPos = true;};

		virtual std::string& getString(void) { if (hasItem())return items[selected]->itemName; };
		virtual CMenuItem *getSelectedItem(void){if (hasItem()) return items[selected];};

		//
		std::string getName(void){ return l_name;};
		std::string getActionKey(){return actionKey;};
};

#endif
