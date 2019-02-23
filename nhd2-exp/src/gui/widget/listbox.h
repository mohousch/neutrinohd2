/*
	$Id: listbox.h 2018.08.19 mohousch Exp $


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

#if !defined(LISTBOX_H_)
#define LISTBOX_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>
#include <vector>

#include <driver/framebuffer.h>

#include <gui/widget/menue.h>
#include <gui/widget/items2detailsline.h>
#include <gui/widget/scrollbar.h>
#include <gui/widget/headers.h>


class ClistBox
{
	private:
		CFrameBuffer* frameBuffer;
		std::vector<CMenuItem*>	items;

		CBox cFrameBox;

		int selected;

		// title
		std::string iconfile;
		std::string l_name;

		//
		std::vector<unsigned int> page_start;
		unsigned int current_page;
		unsigned int total_pages;
		int item_height;
		int item_width;
		int sb_width;
		int listmaxshow;
		int iconOffset;
		int pos;

		//
		int hheight;
		int fheight;
		int footInfoHeight;
		int interFrame;

		//
		int hbutton_count;
		const struct button_label* hbutton_labels;
		//
		int fbutton_count;
		int fbutton_width;
		const struct button_label* fbutton_labels;

		bool paintDate;
		bool paintTitle;
		bool paint_Foot;
		bool paintFootInfo;
		bool logo;
		bool enableCenter;
		bool outFocus;
		bool shrinkMenu;

		//
		CItems2DetailsLine itemsLine;
		CScrollBar scrollBar;
		CHeaders headers;

		// frame
		fb_pixel_t backgroundColor;
		fb_pixel_t itemBoxColor;
		int itemsPerX;
		int itemsPerY;
		int maxItemsPerPage;

		unsigned int item_start_y;
		int items_height;
		int items_width;

		//
		int widgetType;
		bool widgetChange;
		//std::vector<int> widget;

		// head
		fb_pixel_t headColor;
		int headRadius;
		int headCorner;
		int headGradient;

		// foot
		fb_pixel_t footColor;
		int footRadius;
		int footCorner;
		int footGradient;

		// methods
		virtual void paintItems();

	public:
		ClistBox(const int x, int const y, const int dx, const int dy);
		ClistBox(CBox* position);
		virtual ~ClistBox();

		virtual void addItem(CMenuItem * menuItem, const bool defaultselected = false);
		bool hasItem();
		void clearItems(void){items.clear(); current_page = 0;};
		void setSelected(unsigned int _new) { /*if(_new <= items.size())*/ selected = _new; };

		virtual void initFrames();
		virtual void paint();
		virtual void hide();
		virtual void paintHead();
		virtual void paintFoot();
		virtual void paintItemInfo(int pos);
		virtual void hideItemInfo();

		// head
		void enablePaintHead(){paintTitle = true;};
		void enablePaintDate(void){paintDate = true;};
		void setTitle(const char* title = "", const char* icon = NULL, bool logo_ok = false){l_name = title; if(icon != NULL) iconfile = icon; logo = logo_ok;};
		void setHeaderButtons(const struct button_label* _hbutton_label, const int _hbutton_count);
		void setHeadColor(fb_pixel_t col) {headColor = col;};
		void setHeadCorner(int ra, int co){headRadius = ra; headCorner = co;};
		void setHeadGradient(int grad){headGradient = grad;};
		
		// foot
		void enablePaintFoot(){paint_Foot = true;};
		void setFooterButtons(const struct button_label* _fbutton_label, const int _fbutton_count, const int _fbutton_width = 0);
		void setFootColor(fb_pixel_t col) {footColor = col;};
		void setFootCorner(int ra, int co){footRadius = ra; footCorner = co;};
		void setFootGradient(int grad){footGradient = grad;};

		// itemInfo
		void enablePaintFootInfo(int fh = 70){paintFootInfo = true; footInfoHeight = fh; interFrame = 5;};

		virtual void scrollLineDown();
		virtual void scrollLineUp();
		virtual void scrollPageDown();
		virtual void scrollPageUp();

		int getItemsCount()const{return items.size();};
		int getSelected(){return selected;};
		inline CBox getWindowsPos(void){return(cFrameBox);};
		int getTitleHeight(){return hheight;};
		int getFootHeight(){return fheight;};
		int getItemHeight(){return item_height;};
		int getFootInfoHeight(){return footInfoHeight;};
		int getListMaxShow(void) const {return listmaxshow;};

		void disableCenter(){enableCenter = false;};
		void setOutFocus(bool focus){outFocus = focus;};

		void disableShrinkMenu(){shrinkMenu = false;};

		// Frame
		void setBackgroundColor(fb_pixel_t col) {backgroundColor = col;};
		void setItemBoxColor(fb_pixel_t col) {itemBoxColor = col;};
		void setItemsPerPage(int itemsX = 6, int itemsY = 3){itemsPerX = itemsX; itemsPerY = itemsY; maxItemsPerPage = itemsPerX*itemsPerY;};

		virtual void swipLeft();
		virtual void swipRight();

		//
		void setWidgetType(int type){widgetType = type; /*widget.push_back(widgetType);*/};
		int getWidgetType(){return widgetType;};
		void enableWidgetChange(){widgetChange = true;};
		//void addWidget(int wtype){widget.push_back(wtype);};
};

#endif // LISTBOX_H_
