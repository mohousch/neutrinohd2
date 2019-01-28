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

#include <gui/widget/menue.h>
#include <gui/widget/items2detailsline.h>
#include <gui/widget/scrollbar.h>


class ClistBoxEntry
{
	private:
		CFrameBuffer* frameBuffer;
		std::vector<CMenuItem*>	items;

		CBox cFrameBox;

		int selected;

		//
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

		//
		CItems2DetailsLine itemsLine;
		CScrollBar scrollBar;

		virtual void paintItems();

	public:
		ClistBoxEntry(const int x, int const y, const int dx, const int dy);
		ClistBoxEntry(CBox* position);
		virtual ~ClistBoxEntry();

		virtual void addItem(CMenuItem* menuItem, const bool defaultselected = false);
		bool hasItem();
		void clearItems(void){items.clear();};
		void setSelected(unsigned int _new) { if(_new <= items.size()) selected = _new; };

		virtual void initFrames();
		virtual void paint(bool reinit = true);
		virtual void hide();
		virtual void paintHead();
		virtual void paintFoot();
		virtual void paintItemInfo(int pos);
		virtual void hideItemInfo();
		void enablePaintDate(void){paintDate = true;};
		void setTitle(const char* title = "", const char* icon = NULL, bool logo_ok = false){l_name = title; if(icon != NULL) iconfile = icon; logo = logo_ok;};
		void setHeaderButtons(const struct button_label* _hbutton_label, const int _hbutton_count);
		void setFooterButtons(const struct button_label* _fbutton_label, const int _fbutton_count, const int _fbutton_width = 0);
		void enablePaintHead(){paintTitle = true;};
		void enablePaintFoot(){paint_Foot = true;};
		void enablePaintFootInfo(int fh = 70){paintFootInfo = true; footInfoHeight = fh;};

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
};

// CMenulistBoxItem
class ClistBoxEntryItem : public CMenuItem
{
	protected:
		//
		neutrino_locale_t text;
		std::string textString;

		//
		virtual const char* getName(void);

	public:
		ClistBoxEntryItem(const neutrino_locale_t Text, const bool Active = true, const char* const Option = "", const char* const Icon = NULL);

		ClistBoxEntryItem(const char* const Text, const bool Active = true, const char* const Option = "", const char* const IconName = NULL);
		
		int paint(bool selected = false, bool AfterPulldown = false);
		int getHeight(void) const;
		int getWidth(void) const;

		bool isSelectable(void) const {return active;}
};

#endif // LISTBOX_H_
