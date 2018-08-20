/*
	$Id: listbox.cpp 2018.08.19 mohousch Exp $


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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h> //acces
#include <cctype>

#include <global.h>
#include <neutrino.h>

#include <gui/widget/listbox.h>
#include <gui/widget/icons.h>
#include <gui/widget/items2detailsline.h>
#include <gui/widget/scrollbar.h>
#include <gui/widget/textbox.h>
#include <gui/widget/stringinput.h>

#include <gui/color.h>

#include <driver/fontrenderer.h>
#include <driver/rcinput.h>

#include <system/debug.h>


ClistBoxEntry::ClistBoxEntry(const int x, const int y, const int dx, const int dy)
{
	frameBuffer = CFrameBuffer::getInstance();

	 selected = -1;
	 current_page = 0;
	pos = 0;

	cFrameBox.iX = x;
	cFrameBox.iY = y;
	cFrameBox.iWidth = dx;
	cFrameBox.iHeight = dy;

	initFrames();
}

ClistBoxEntry::ClistBoxEntry(CBox* position)
{
	frameBuffer = CFrameBuffer::getInstance();

	 selected = -1;
	 current_page = 0;
	pos = 0;

	cFrameBox = *position;

	initFrames();
}

ClistBoxEntry::~ClistBoxEntry()
{
	for(unsigned int count = 0; count < items.size(); count++) 
	{
		CMenuItem * item = items[count];
		
		delete item;
		item = NULL;
	}

	items.clear();
}

void ClistBoxEntry::addItem(CMenuItem *menuItem, const bool defaultselected)
{
	if (defaultselected)
		selected = items.size();
	
	items.push_back(menuItem);
}

bool ClistBoxEntry::hasItem()
{
	return !items.empty();
}

void ClistBoxEntry::initFrames()
{
	// calculate some values
	int itemHeightTotal = 0;
	item_height = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight() + 3;
	int heightCurrPage = 0;
	page_start.clear();
	page_start.push_back(0);
	total_pages = 1;
	int heightFirstPage = 0;
	
	for (unsigned int i = 0; i < items.size(); i++) 
	{
		item_height = items[i]->getHeight();
		itemHeightTotal += item_height;
		heightCurrPage += item_height;

		if(heightCurrPage > cFrameBox.iHeight)
		{
			page_start.push_back(i);
			heightFirstPage = heightCurrPage - item_height;
			total_pages++;
			heightCurrPage = item_height;
		}
	}

	page_start.push_back(items.size());

	// icon offset
	iconOffset = 0;

	for (unsigned int i = 0; i < items.size(); i++) 
	{
		if ((!(items[i]->iconName.empty())) || CRCInput::isNumeric(items[i]->directKey))
		{
			iconOffset = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
			break;
		}
	}

	// recalculate height
	//listmaxshow = cFrameBox.iHeight/item_height;
	//cFrameBox.iHeight = listmaxshow*item_height;
}

void ClistBoxEntry::paint()
{
	initFrames();
	paintItems();
}

void ClistBoxEntry::paintItems()
{
	sb_width = 0;
	
	if(total_pages > 1)
		sb_width = SCROLLBAR_WIDTH;

	int iwidth = cFrameBox.iWidth - sb_width;

	// item not currently on screen
	if (selected >= 0)
	{
		while(selected < (int)page_start[current_page])
			current_page--;
		
		while(selected >= (int)page_start[current_page + 1])
			current_page++;
	}

	frameBuffer->paintBoxRel(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight, COL_MENUCONTENT_PLUS_0);
	
	// paint right scrollBar if we have more then one page
	if(total_pages > 1)
	{
		::paintScrollBar(cFrameBox.iX + cFrameBox.iWidth - SCROLLBAR_WIDTH, cFrameBox.iY, cFrameBox.iHeight, total_pages, current_page);
	}

	// paint items
	int ypos = cFrameBox.iY;
	int xpos = cFrameBox.iX;
	
	for (unsigned int count = 0; count < items.size(); count++) 
	{
		CMenuItem * item = items[count];

		if ((count >= page_start[current_page]) && (count < page_start[current_page + 1])) 
		{
			item->init(xpos, ypos, iwidth, iconOffset);
			
			if( (item->isSelectable()) && (selected == -1) ) 
			{
				selected = count;
			} 

			if (selected == count) 
			{
				paintItemInfo(count);
			}

			ypos = item->paint(selected == ((signed int) count));
		} 
		else 
		{
			// x = -1 is a marker which prevents the item from being painted on setActive changes
			item->init(-1, 0, 0, 0);
		}	
	} 
}

void ClistBoxEntry::paintItemInfo(int pos)
{
	dprintf(DEBUG_NORMAL, "ClistBoxEntry::paintItemInfo:\n");
}

void ClistBoxEntry::hideItemInfo()
{
	dprintf(DEBUG_NORMAL, "ClistBoxEntry::hideItemInfo:\n");
}

void ClistBoxEntry::hide()
{
	dprintf(DEBUG_NORMAL, "ClistBoxEntry::hide:\n");

	frameBuffer->paintBackgroundBoxRel(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight);

	hideItemInfo(); 
	
	frameBuffer->blit();

}

void ClistBoxEntry::scrollLineDown()
{
	//search next / prev selectable item
	for (unsigned int count = 1; count < items.size(); count++) 
	{
		pos = (selected + count)%items.size();

		CMenuItem * item = items[pos];

		if ( item->isSelectable() ) 
		{
			if ((pos < (int)page_start[current_page + 1]) && (pos >= (int)page_start[current_page]))
			{ 
				// Item is currently on screen
				//clear prev. selected
				items[selected]->paint(false);
				//select new
				paintItemInfo(pos);
				item->paint(true);
				selected = pos;
			} 
			else 
			{
				selected = pos;
				paintItems();
			}
			break;
		}
	}
}

void ClistBoxEntry::scrollLineUp()
{
	//search next / prev selectable item
	for (unsigned int count = 1; count < items.size(); count++) 
	{
		pos = selected - count;
		if ( pos < 0 )
			pos += items.size();

		CMenuItem * item = items[pos];

		if ( item->isSelectable() ) 
		{
			if ((pos < (int)page_start[current_page + 1]) && (pos >= (int)page_start[current_page]))
			{ 
				// Item is currently on screen
				//clear prev. selected
				items[selected]->paint(false);
				//select new
				paintItemInfo(pos);
				item->paint(true);
				selected = pos;
			} 
			else 
			{
				selected = pos;
				paintItems();
			}
			break;
		}
	}
}

void ClistBoxEntry::scrollPageDown()
{
	pos = (int) page_start[current_page + 1];

	// check pos
	if(pos >= (int) items.size()) 
		pos = items.size() - 1;

	for (unsigned int count = pos ; count < items.size(); count++) 
	{
		CMenuItem * item = items[pos];
		if (item->isSelectable()) 
		{
			if ((pos < (int)page_start[current_page + 1]) && (pos >= (int)page_start[current_page])) 
			{
				items[selected]->paint(false);

				// paint new item
				paintItemInfo(pos);
				item->paint(true);
				selected = pos;
			} 
			else 
			{
				selected = pos;
				paintItems();
			}
			break;
		}
		pos++;
	}
}

void ClistBoxEntry::scrollPageUp()
{
	if(current_page) 
	{
		pos = (int) page_start[current_page] - 1;
		for (unsigned int count = pos; count > 0; count--) 
		{
			CMenuItem * item = items[pos];
			if ( item->isSelectable() ) 
			{
				if ((pos < (int)page_start[current_page + 1]) && (pos >= (int)page_start[current_page])) 
				{
					// prev item
					items[selected]->paint(false);

					// new item
					paintItemInfo(pos);
					item->paint(true);
					selected = pos;
				} 
				else 
				{
					selected = pos;
					paintItems();
				}
				break;
			}
			pos--;
		}
	} 
	else 
	{
		pos = 0;
		for (unsigned int count = 0; count < items.size(); count++) 
		{
			CMenuItem * item = items[pos];
			if (item->isSelectable()) 
			{
				if ((pos < (int)page_start[current_page + 1]) && (pos >= (int)page_start[current_page])) 
				{
					// prev item
					items[selected]->paint(false);

					// new item
					paintItemInfo(pos);
					item->paint(true);
					selected = pos;
				} 
				else 
				{
					selected = pos;
					paintItems();
				}
				break;
			}
			pos++;
		}
	}
}

int ClistBoxEntry::resume()
{
	int retval = menu_return::RETURN_REPAINT;

	if(hasItem()) 
	{
		//exec this item...
		CMenuItem* item = items[selected];
							
		int rv = item->exec(NULL);
							
		switch ( rv ) 
		{
			case menu_return::RETURN_EXIT_ALL:
				retval = menu_return::RETURN_EXIT_ALL;
									
			case menu_return::RETURN_EXIT:
				retval = menu_return::RETURN_EXIT;
				break;
									
			case menu_return::RETURN_REPAINT:
				retval = menu_return::RETURN_REPAINT;
				break;

			case menu_return::RETURN_NONE:
				retval = menu_return::RETURN_NONE;
				break;							
		}
	} 
	
	return retval;
}

//ClistBoxEntryItem
ClistBoxEntryItem::ClistBoxEntryItem(const neutrino_locale_t Text, const bool Active, const char* const Option, CMenuTarget* Target, const char* const ActionKey, const char* const IconName, const char* const ItemIcon)
{
	text = Text;
	textString = g_Locale->getText(Text);

	option = Option;

	active = Active;
	jumpTarget = Target;
	actionKey = ActionKey ? ActionKey : "";

	iconName = IconName ? IconName : "";

	itemIcon = ItemIcon? ItemIcon : "";
	itemName = g_Locale->getText(Text);
	itemType = ITEM_TYPE_LIST_BOX;
}

ClistBoxEntryItem::ClistBoxEntryItem(const char* Text, const bool Active, const char* const Option, CMenuTarget* Target, const char* const ActionKey, const char* const IconName, const char* const ItemIcon)
{
	text = NONEXISTANT_LOCALE;
	textString = Text;

	option = Option;

	active = Active;
	jumpTarget = Target;
	actionKey = ActionKey ? ActionKey : "";

	iconName = IconName ? IconName : "";

	itemIcon = ItemIcon? ItemIcon : "";
	itemName = Text;
	itemType = ITEM_TYPE_LIST_BOX;
}

int ClistBoxEntryItem::getHeight(void) const
{
	int iw = 0;
	int ih = 0;
	int height = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight() + 6;

	if(widgetType == WIDGET_CLASSIC)
	{
		CFrameBuffer::getInstance()->getIconSize(NEUTRINO_ICON_MENUITEM_NOPREVIEW, &iw, &ih);
		height = std::max(ih, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight()) + 6;
	}
	else
	{
		CFrameBuffer::getInstance()->getIconSize(iconName.c_str(), &iw, &ih);
		height = std::max(ih, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight()) + 6;
	}

	return height;
}

int ClistBoxEntryItem::getWidth(void) const
{
	int tw = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(textString); //FIXME:

	return tw;
}

int ClistBoxEntryItem::exec(CMenuTarget* parent)
{
	dprintf(DEBUG_DEBUG, "ClistBoxEntryItem::exec:\n");

	if(jumpTarget)
		return jumpTarget->exec(parent, actionKey);
	else
		return menu_return::RETURN_EXIT;
}

const char * ClistBoxEntryItem::getName(void)
{
	const char * l_name;
	
	if(text == NONEXISTANT_LOCALE)
		l_name = textString.c_str();
	else
        	l_name = g_Locale->getText(text);
	
	return l_name;
}

int ClistBoxEntryItem::paint(bool selected, bool /*AfterPulldown*/)
{
	dprintf(DEBUG_DEBUG, "ClistBoxEntryItem::paint:\n");

	CFrameBuffer * frameBuffer = CFrameBuffer::getInstance();

	int height = getHeight();
	const char * l_text = getName();

	uint8_t color = COL_MENUCONTENT;
	fb_pixel_t bgcolor = COL_MENUCONTENT_PLUS_0;

	if (selected)
	{
		color = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
	}
	else if (!active)
	{
		color = COL_MENUCONTENTINACTIVE;
		bgcolor = COL_MENUCONTENTINACTIVE_PLUS_0;
	}
	
	// itemBox
	frameBuffer->paintBoxRel(x, y, dx, height, bgcolor); //FIXME
	
	// left icon
	int icon_w = 0;
	int icon_h = 0;

	if(widgetType == WIDGET_CLASSIC)
	{
		frameBuffer->getIconSize(NEUTRINO_ICON_MENUITEM_NOPREVIEW, &icon_w, &icon_h);

		if (!itemIcon.empty())
		{
			frameBuffer->displayImage(itemIcon.c_str(), x + BORDER_LEFT, y + ((height - icon_h)/2), icon_w, icon_h);
		}
	}
	else //standard
	{
		if (!iconName.empty())
		{
			//get icon size
			frameBuffer->getIconSize(iconName.c_str(), &icon_w, &icon_h);
		
			frameBuffer->paintIcon(iconName, x + BORDER_LEFT, y + (height - icon_h)/2 );
		}
	}

	// optionInfo
	int optionInfo_width = 0;
	
	if(!optionInfo.empty())
	{
		optionInfo_width = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth(optionInfo.c_str());

		g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(x + dx - BORDER_RIGHT - optionInfo_width, y + (height - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight(), optionInfo_width, optionInfo.c_str(), color, 0, true); // UTF-8
	}

	// right icon1
	int icon1_w = 0;
	int icon1_h = 0;
	
	if (!icon1.empty())
	{
		//get icon size
		frameBuffer->getIconSize(icon1.c_str(), &icon1_w, &icon1_h);
		
		frameBuffer->paintIcon(icon1, x + dx - BORDER_LEFT - icon1_w, y + (height - icon1_h)/2 );
	}

	// right icon2
	int icon2_w = 0;
	int icon2_h = 0;
	
	if (!icon2.empty())
	{
		//get icon size
		frameBuffer->getIconSize(icon2.c_str(), &icon2_w, &icon2_h);
		
		frameBuffer->paintIcon(icon2, x + dx - BORDER_LEFT - (icon1_w? icon1_w + ICON_OFFSET : 0) - icon2_w, y + (height - icon2_h)/2 );
	}

	// number
	int numwidth = 0;
	if(number != 0)
	{
		char tmp[10];

		sprintf((char*) tmp, "%d", number);

		numwidth = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth("0000");

		int numpos = x + BORDER_LEFT + numwidth - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth(tmp);

		g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(numpos, y + (height - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight(), numwidth + 5, tmp, color, 0, true); // UTF-8
	}

	// ProgressBar
	int pBarWidth = 0;
	if(runningPercent > -1)
	{
		pBarWidth = 35;
		int pBarHeight = height/3;

		CProgressBar timescale(pBarWidth, pBarHeight);
		
		timescale.reset();
		timescale.paint(x + BORDER_LEFT + numwidth + ICON_OFFSET, y + (height - pBarHeight)/2, runningPercent);
	}
	
	// locale|option text
	int l_text_width = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(l_text, true);

	if (widgetType == WIDGET_CLASSIC)
	{
		if(l_text_width >= dx - BORDER_LEFT - BORDER_RIGHT)
			l_text_width = dx - BORDER_LEFT - BORDER_RIGHT;

		if(nLinesItem)
		{
			// local
			if(l_text != NULL)
			{
				/*nameFont*/g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x + BORDER_LEFT + icon_w + numwidth + ICON_OFFSET + pBarWidth + ICON_OFFSET, y + 3 + /*nameFont*/g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight(), dx - BORDER_RIGHT - BORDER_LEFT - numwidth - pBarWidth - 2*ICON_OFFSET - icon_w - icon1_w - icon2_w - optionInfo_width - ICON_OFFSET, l_text, color, 0, true); // UTF-8
			}

			// option
			if(!option.empty())
			{
				/*optionFont*/g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(x + BORDER_LEFT + icon_w + numwidth + ICON_OFFSET + pBarWidth + ICON_OFFSET, y + height, dx - BORDER_LEFT - BORDER_RIGHT - numwidth - pBarWidth - 2*ICON_OFFSET - icon_w - icon1_w - icon2_w - optionInfo_width - ICON_OFFSET, option.c_str(), color, 0, true);
			}
		}
		else
		{
			// locale
			if(l_text != NULL)
			{
				/*nameFont*/g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x + BORDER_LEFT + icon_w + numwidth + ICON_OFFSET + pBarWidth + ICON_OFFSET, y + /*nameFont*/g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight() + (height - /*nameFont*/g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight())/2, dx - BORDER_RIGHT - BORDER_LEFT - numwidth - pBarWidth - 2*ICON_OFFSET - icon_w - icon1_w - icon2_w - optionInfo_width - ICON_OFFSET, l_text, color, 0, true); // UTF-8
			}

			// option
			std::string Option;
			
			if(!option.empty())
			{
				int iw, ih;
				//get icon size
				frameBuffer->getIconSize(NEUTRINO_ICON_HD, &iw, &ih);

				Option = " - ";
				Option += option.c_str();

				/*optionFont*/g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(x + BORDER_LEFT + icon_w + numwidth + pBarWidth + ICON_OFFSET + l_text_width + ICON_OFFSET, y + (height - /*optionFont*/g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight())/2 + /*optionFont*/g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight(), dx - BORDER_LEFT - BORDER_RIGHT - numwidth - ICON_OFFSET - pBarWidth - ICON_OFFSET - l_text_width - icon_w - icon1_w - ICON_OFFSET - icon2_w - ICON_OFFSET - 2*iw, Option.c_str(), COL_COLORED_EVENTS_CHANNELLIST, 0, true);
			}
		}
	}
	else if(widgetType == WIDGET_STANDARD)// standard
	{
		// locale
		if(l_text_width >= dx - BORDER_LEFT - BORDER_RIGHT)
			l_text_width = dx - BORDER_LEFT - BORDER_RIGHT;

		if(l_text != NULL)
		{
			/*nameFont*/g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x + BORDER_LEFT + icon_w + numwidth + ICON_OFFSET + pBarWidth + ICON_OFFSET, y + (height - /*nameFont*/g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight())/2 + /*nameFont*/g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight(), dx - BORDER_RIGHT - BORDER_LEFT - numwidth - pBarWidth - 2*ICON_OFFSET - icon_w - icon1_w - icon2_w - optionInfo_width - ICON_OFFSET, l_text, color, 0, true); // UTF-8
		}

		// option
		std::string Option;
			
		if(!option.empty())
		{
			int iw, ih;
			//get icon size
			frameBuffer->getIconSize(NEUTRINO_ICON_HD, &iw, &ih);

			Option = " - ";
			Option += option.c_str();

			/*optionFont*/g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString(x + BORDER_LEFT + numwidth + pBarWidth + ICON_OFFSET + l_text_width + ICON_OFFSET, y + (height - /*optionFont*/g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight())/2 + /*optionFont*/g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight(), dx - BORDER_LEFT - BORDER_RIGHT - numwidth - ICON_OFFSET - pBarWidth - ICON_OFFSET - l_text_width - icon_w - icon1_w - ICON_OFFSET - icon2_w - ICON_OFFSET - 2*iw, Option.c_str(), COL_COLORED_EVENTS_CHANNELLIST, 0, true);
		}
	}

	// vfd
	if (selected)
	{
		CVFD::getInstance()->showMenuText(0, l_text, -1, true);
	}
	
	return y + height;
}


