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


class ClistBoxEntry
{
	private:
		CFrameBuffer* frameBuffer;
		std::vector<CMenuItem*>	items;

		CBox cFrameBox;

		virtual void paintItems();
		int selected;

		//
		std::vector<unsigned int> page_start;
		unsigned int current_page;
		unsigned int total_pages;
		int item_height;
		int item_width;
		int sb_width;
		//int items_height;
		//int items_width;
		int listmaxshow;
		int iconOffset;
		int pos;

		virtual void initFrames();
		virtual void paintItemInfo(int pos);
		virtual void hideItemInfo();

	public:
		ClistBoxEntry(const int x, int const y, const int dx, const int dy);
		ClistBoxEntry(CBox* position);
		virtual ~ClistBoxEntry();

		virtual void addItem(CMenuItem* menuItem, const bool defaultselected = false);
		bool hasItem();
		int getItemsCount()const{return items.size();};
		void clearItems(void){items.clear();};

		void setSelected(unsigned int _new) { if(_new <= items.size()) selected = _new; };
		int getSelected() {return selected;};
		inline	CBox getWindowsPos(void){return(cFrameBox);};

		//
		virtual void paint();
		virtual void hide();

		virtual void scrollLineDown();
		virtual void scrollLineUp();
		virtual void scrollPageDown();
		virtual void scrollPageUp();
		virtual int resume();
};

// CMenulistBoxItem
class ClistBoxEntryItem : public CMenuItem
{
	CMenuTarget* jumpTarget;
	std::string actionKey;

	protected:
		//
		neutrino_locale_t text;
		std::string textString;

		//
		virtual const char* getName(void);

	public:
		ClistBoxEntryItem(const neutrino_locale_t Text, const bool Active = true, const char* const Option = "", CMenuTarget* Target = NULL, const char* const ActionKey = NULL, const char* const Icon = NULL, const char* const ItemIcon = NULL);

		ClistBoxEntryItem(const char* const Text, const bool Active = true, const char* const Option = "", CMenuTarget* Target = NULL, const char* const ActionKey = NULL, const char* const IconName = NULL, const char* const ItemIcon = NULL);
		
		int paint(bool selected = false, bool AfterPulldown = false);
		int getHeight(void) const;
		int getWidth(void) const;

		int exec(CMenuTarget* parent);
		bool isSelectable(void) const {return active;}
};

#endif // LISTBOX_H_
