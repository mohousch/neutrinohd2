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
#include <gui/widget/headers.h>

#include <driver/color.h>

#include <driver/fontrenderer.h>
#include <driver/rcinput.h>

#include <system/debug.h>


ClistBox::ClistBox(const int x, const int y, const int dx, const int dy)
{
	frameBuffer = CFrameBuffer::getInstance();

	selected = -1;
	current_page = 0;
	pos = 0;

	cFrameBox.iX = x;
	cFrameBox.iY = y;
	cFrameBox.iWidth = dx;
	cFrameBox.iHeight = dy;

	hheight = 0;
	fheight = 0;
	footInfoHeight = 0;

	hbutton_count	= 0;
	hbutton_labels	= NULL;
	fbutton_count	= 0;
	fbutton_labels	= NULL;
	fbutton_width = cFrameBox.iWidth;

	paintDate = false;
	paintTitle = false;
	paint_Foot = false;
	paintFootInfo = false;

	l_name = "";
	iconfile = "";

	logo = false;
	enableCenter = true;
	outFocus = false;
	shrinkMenu = true;

	//
	backgroundColor = COL_MENUCONTENT_PLUS_0/*COL_BACKGROUND*/;
	//itemBoxColor = COL_YELLOW /*COL_MENUCONTENTSELECTED_PLUS_0*/;
	itemsPerX = 6;
	itemsPerY = 3;
	maxItemsPerPage = itemsPerX*itemsPerY;

	//
	widgetType = WIDGET_TYPE_STANDARD;
	widgetChange = false;
}

ClistBox::ClistBox(CBox* position)
{
	frameBuffer = CFrameBuffer::getInstance();

	selected = -1;
	current_page = 0;
	pos = 0;

	cFrameBox = *position;

	hheight = 0;
	fheight = 0;
	footInfoHeight = 0;

	hbutton_count	= 0;
	hbutton_labels	= NULL;
	fbutton_count	= 0;
	fbutton_labels	= NULL;
	fbutton_width = cFrameBox.iWidth;

	paintDate = false;
	paintTitle = false;
	paint_Foot = false;
	paintFootInfo = false;

	l_name = "";
	iconfile = "";

	logo = false;
	enableCenter = true;
	outFocus = false;
	shrinkMenu = true;

	//
	backgroundColor = COL_MENUCONTENT_PLUS_0/*COL_BACKGROUND*/;
	//itemBoxColor = COL_YELLOW /*COL_MENUCONTENTSELECTED_PLUS_0*/;
	itemsPerX = 6;
	itemsPerY = 3;
	maxItemsPerPage = itemsPerX*itemsPerY;

	//
	widgetType = WIDGET_TYPE_STANDARD;
	widgetChange = false;
}

ClistBox::~ClistBox()
{
	for(unsigned int count = 0; count < items.size(); count++) 
	{
		CMenuItem * item = items[count];
		
		delete item;
		item = NULL;
	}

	items.clear();
}

void ClistBox::addItem(CMenuItem *menuItem, const bool defaultselected)
{
	if (defaultselected)
		selected = items.size();
	
	items.push_back(menuItem);
}

bool ClistBox::hasItem()
{
	return !items.empty();
}

void ClistBox::initFrames()
{
	// widgettype forwarded to item 
	for (unsigned int count = 0; count < items.size(); count++) 
	{
		CMenuItem * item = items[count];

		item->widgetType = widgetType;
	} 

	// head
	if(paintTitle)
	{
		hheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight() + 6;
	}
	
	// foot height
	if(paint_Foot)
	{
		fheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight() + 6;
	}

	////
	if(widgetType == WIDGET_TYPE_FRAME)
	{
		page_start.clear();
		page_start.push_back(0);
		total_pages = 1;

		for (unsigned int i = 0; i < items.size(); i++) 
		{
			if(i == maxItemsPerPage*total_pages)
			{
				page_start.push_back(i);
				total_pages++;
			}
		}

		page_start.push_back(items.size());

		//
		item_width = cFrameBox.iWidth/itemsPerX;
		item_height = (cFrameBox.iHeight - hheight - fheight - 20)/itemsPerY;

		for (unsigned int count = 0; count < items.size(); count++) 
		{
			CMenuItem * item = items[count];

			item->item_width = item_width;
			item->item_height = item_height;
			item->item_backgroundColor = backgroundColor;
			//item->item_selectedColor = itemBoxColor;
		} 
	}
	else 
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

			if((heightCurrPage + hheight + fheight + footInfoHeight)> cFrameBox.iHeight)
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

		//
		for (unsigned int count = 0; count < items.size(); count++) 
		{
			CMenuItem * item = items[count];

			item->item_backgroundColor = backgroundColor;
		} 

		// recalculate height
		if(shrinkMenu)
		{
			listmaxshow = (cFrameBox.iHeight - hheight - fheight - footInfoHeight)/item_height;
			cFrameBox.iHeight = hheight + listmaxshow*item_height + fheight + footInfoHeight;
		}

		if(enableCenter)
		{
			cFrameBox.iX = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - cFrameBox.iWidth ) >> 1 );
			cFrameBox.iY = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - cFrameBox.iHeight) >> 1 );
		}
	}
}

void ClistBox::paint()
{
	initFrames();

	paintItems();
	paintHead();
	paintFoot();
}

void ClistBox::paintItems()
{
	dprintf(DEBUG_NORMAL, "ClistBox::paintItems:\n");

	if(widgetType == WIDGET_TYPE_FRAME)
	{
		item_start_y = cFrameBox.iY + hheight;

		// items background
		frameBuffer->paintBoxRel(cFrameBox.iX, cFrameBox.iY + hheight, cFrameBox.iWidth, cFrameBox.iHeight - hheight - fheight - footInfoHeight, backgroundColor);

		// item not currently on screen
		if (selected >= 0)
		{
			while(selected < (int)page_start[current_page])
				current_page--;
		
			while(selected >= (int)page_start[current_page + 1])
				current_page++;
		}

		for (unsigned int i = 0; i < items.size(); i++) 
		{
			CMenuItem * item = items[i];	
			item->init(-1, 0, 0, 0);
		}

		int count = (int)page_start[current_page];

		if(items.size() > 0)
		{
			for (unsigned int _y = 0; _y < itemsPerY; _y++)
			{
				for (unsigned int _x = 0; _x < itemsPerX; _x++)
				{
					CMenuItem * item = items[count];

					item->init(cFrameBox.iX + _x*item_width, item_start_y + _y*item_height, items_width, iconOffset);

					if( !outFocus && ((item->isSelectable()) && (selected == -1)) ) 
					{
						selected = count;
					} 

					if (selected == count) 
					{
						paintItemInfo(count);
					}

					item->paint( selected == ((signed int) count));

					count++;

					if ( (count == (int)page_start[current_page + 1]) || (count == (int)items.size()))
					{
						break;
					}
				}

				if ( (count == (int)page_start[current_page + 1]) || (count == (int)items.size()))
				{
					break;
				}		
			}
		}
	}
	else
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

		frameBuffer->paintBoxRel(cFrameBox.iX, cFrameBox.iY + hheight, cFrameBox.iWidth, cFrameBox.iHeight - hheight - fheight - footInfoHeight, backgroundColor);
	
		// paint right scrollBar if we have more then one page
		if(total_pages > 1)
		{
			scrollBar.paint(cFrameBox.iX + cFrameBox.iWidth - SCROLLBAR_WIDTH, cFrameBox.iY + hheight, cFrameBox.iHeight - hheight - fheight - footInfoHeight, total_pages, current_page);
		}

		// paint items
		int ypos = cFrameBox.iY + hheight;
		int xpos = cFrameBox.iX;
	
		for (unsigned int count = 0; count < items.size(); count++) 
		{
			CMenuItem * item = items[count];

			if ((count >= page_start[current_page]) && (count < page_start[current_page + 1])) 
			{
				item->init(xpos, ypos, iwidth, iconOffset);
			
				if( !outFocus && ((item->isSelectable()) && (selected == -1)) ) 
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
}

void ClistBox::paintHead()
{
	if(paintTitle)
	{
		headers.enablePaintDate();
		headers.enableLogo();
		headers.setHeaderButtons(hbutton_labels, hbutton_count);
		headers.paintHead(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, hheight, iconfile.c_str(), l_name.c_str());
	}	
}

void ClistBox::paintFoot()
{
	if(paint_Foot)
	{
		headers.paintFoot(cFrameBox.iX, cFrameBox.iY + cFrameBox.iHeight - footInfoHeight - fheight, cFrameBox.iWidth, fheight, fbutton_count, fbutton_labels);
	}
}

void ClistBox::setHeaderButtons(const struct button_label* _hbutton_labels, const int _hbutton_count)
{
	if(paintTitle)
	{
		hbutton_count = _hbutton_count;
		hbutton_labels = _hbutton_labels;
	}
}

void ClistBox::setFooterButtons(const struct button_label* _fbutton_labels, const int _fbutton_count, const int _fbutton_width)
{
	if(paint_Foot)
	{
		fbutton_count = _fbutton_count;
		fbutton_labels = _fbutton_labels;
		fbutton_width = (_fbutton_width == 0)? cFrameBox.iWidth : _fbutton_width;
	}
}

void ClistBox::paintItemInfo(int pos)
{
	if(paintFootInfo)
	{
		CMenuItem* item = items[pos];

		item->getYPosition();
	
		// detailslines
		itemsLine.paintD(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight - footInfoHeight, footInfoHeight, hheight, item->getHeight(), item->getYPosition());

		// option_info1
		int l_ow1 = 0;
		if(!item->option_info1.empty())
		{
			l_ow1 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getRenderWidth(item->option_info1.c_str());

			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString(cFrameBox.iX + cFrameBox.iWidth - BORDER_RIGHT - l_ow1, cFrameBox.iY + cFrameBox.iHeight - footInfoHeight + (footInfoHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight(), cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT - l_ow1, item->option_info1.c_str(), COL_MENUFOOT_INFO, 0, true);
		}

		// info1
		int l_w1 = 0;
		if(!item->info1.empty())
		{
			l_w1 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getRenderWidth(item->info1.c_str());

			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBox.iHeight - footInfoHeight + (footInfoHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT - l_ow1, item->info1.c_str(), COL_MENUFOOT_INFO, 0, true);
		}

		// option_info2
		int l_ow2 = 0;
		if(!item->option_info2.empty())
		{
			l_ow2 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth(item->option_info2.c_str());

			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(cFrameBox.iX + cFrameBox.iWidth - BORDER_RIGHT - l_ow2, cFrameBox.iY + cFrameBox.iHeight - footInfoHeight + footInfoHeight/2 + (footInfoHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight(), cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT - l_ow2, item->option_info2.c_str(), COL_MENUFOOT_INFO, 0, true);
		}

		// info2
		int l_w2 = 0;
		if(!item->info2.empty())
		{
			l_w2 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getRenderWidth(item->info2.c_str());

			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString (cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBox.iHeight - footInfoHeight + footInfoHeight/2 + (footInfoHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight(), cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT - l_ow2, item->info2.c_str(), COL_MENUFOOT_INFO, 0, true); // UTF-8
		}
	}
}

void ClistBox::hideItemInfo()
{
	if(paintFootInfo)
		itemsLine.clear(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth + ConnectLineBox_Width, cFrameBox.iHeight - footInfoHeight, footInfoHeight);
}

void ClistBox::hide()
{
	dprintf(DEBUG_NORMAL, "ClistBox::hide:\n");

	frameBuffer->paintBackgroundBoxRel(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight);

	hideItemInfo(); 
	
	frameBuffer->blit();

}

void ClistBox::scrollLineDown()
{
	if(widgetType == WIDGET_TYPE_FRAME)
	{
		if(items.size())
		{
			pos = selected + itemsPerX;

			//FIXME:
			if (pos >= items.size())
				pos -= itemsPerX;

			CMenuItem * item = items[pos];

			if ( item->isSelectable() ) 
			{
				if ((pos < (int)page_start[current_page + 1]) && (pos >= (int)page_start[current_page]))
				{ 
					// Item is currently on screen
					//clear prev. selected
					items[selected]->paint(false);
					//select new
					item->paint(true);
					paintItemInfo(pos);
					selected = pos;
				} 
			}
		}
	}
	else
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
}

void ClistBox::scrollLineUp()
{
	if(widgetType == WIDGET_TYPE_FRAME)
	{
		if(items.size())
		{
			pos = selected - itemsPerX;

			if(pos < 0)
				pos = selected;

			CMenuItem * item = items[pos];

			if ( item->isSelectable() ) 
			{
				if ((pos < (int)page_start[current_page + 1]) && (pos >= (int)page_start[current_page]))
				{ 
					// Item is currently on screen
					//clear prev. selected
					items[selected]->paint(false);
					//select new
					item->paint(true);
					paintItemInfo(pos);
					selected = pos;
				}
			}
		}
	}
	else
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
}

void ClistBox::scrollPageDown()
{
	if(widgetType == WIDGET_TYPE_FRAME)
	{
		if(items.size())
		{
			if(current_page) 
			{
				pos = (int) page_start[current_page] - 1;

				selected = pos;
				paintItems();
			}
		}
	}
	else
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
}

void ClistBox::scrollPageUp()
{
	if(widgetType == WIDGET_TYPE_FRAME)
	{
		if(items.size())
		{
			pos = (int) page_start[current_page + 1];
			if(pos >= (int) items.size()) 
				pos = items.size() - 1;

			selected = pos;
			paintItems();
		}
	}
	else
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
}

void ClistBox::swipLeft()
{
	dprintf(DEBUG_NORMAL, "ClistBoxEntry::swipLeft:\n");

	if(widgetType == WIDGET_TYPE_FRAME)
	{
		if(items.size())
		{
			//search next / prev selectable item
			for (unsigned int count = (int)page_start[current_page] + 1; count < (int)page_start[current_page + 1]; count++)
			{
				//pos = selected - count;
				pos = selected - 1;

				// jump to page end
				if(pos < (int)page_start[current_page])
					pos = (int)page_start[current_page + 1] - 1;

				CMenuItem * item = items[pos];

				if ( item->isSelectable() ) 
				{
					if ((pos < (int)page_start[current_page + 1]) && (pos >= (int)page_start[current_page]))
					{ 
						// Item is currently on screen
						//clear prev. selected
						items[selected]->paint(false);
						//select new
						item->paint(true);
						paintItemInfo(pos);
						selected = pos;
					}
								
					break;
				}
			}
		}
	}
}

void ClistBox::swipRight()
{
	dprintf(DEBUG_NORMAL, "ClistBoxEntry::swipRight:\n");

	if(widgetType == WIDGET_TYPE_FRAME)
	{
		if(items.size())
		{
			//search next / prev selectable item
			for (unsigned int count = (int)page_start[current_page] + 1; count < (int)page_start[current_page + 1]; count++)
			{
				pos = selected + 1;

				// jump to page start
				if(pos == (int)page_start[current_page + 1])
					pos = (int)page_start[current_page];

				CMenuItem * item = items[pos];

				if ( item->isSelectable() ) 
				{
					if ((pos < (int)page_start[current_page + 1]) && (pos >= (int)page_start[current_page]))
					{ 
						// Item is currently on screen
						//clear prev. selected
						items[selected]->paint(false);
						//select new
						item->paint(true);
						paintItemInfo(pos);
						selected = pos;
					}
								
					break;
				}
			}
		}
	}
}

//ClistBoxEntryItem
ClistBoxEntryItem::ClistBoxEntryItem(const neutrino_locale_t Text, const bool Active, const char* const Option, const char* const IconName)
{
	text = Text;
	textString = g_Locale->getText(Text);
	option = Option;

	active = Active;

	iconName = IconName ? IconName : "";
	itemName = g_Locale->getText(Text);
	itemType = ITEM_TYPE_LIST_BOX_ENTRY;
}

ClistBoxEntryItem::ClistBoxEntryItem(const char* Text, const bool Active, const char* const Option, const char* const IconName)
{
	text = NONEXISTANT_LOCALE;
	textString = Text;
	option = Option;

	active = Active;

	iconName = IconName ? IconName : "";
	itemName = Text;
	itemType = ITEM_TYPE_LIST_BOX_ENTRY;
}

int ClistBoxEntryItem::getHeight(void) const
{
	if(widgetType == WIDGET_TYPE_FRAME)
	{
		return item_height;
	}
	else
	{
		int iw = 0;
		int ih = 0;
		int height = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight() + 6;

		if(nLinesItem)
		{
			height = 50;
		}
		else
		{
			CFrameBuffer::getInstance()->getIconSize(iconName.c_str(), &iw, &ih);
			height = std::max(ih, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight()) + 6;
		}

		return height;
	}
}

int ClistBoxEntryItem::getWidth(void) const
{
	if(widgetType == WIDGET_TYPE_FRAME)
	{
		return item_width;
	}
	else
	{
		int tw = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(textString); //FIXME:

		return tw;
	}
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
	fb_pixel_t bgcolor = marked? COL_MENUCONTENTSELECTED_PLUS_2 : /*COL_MENUCONTENT_PLUS_0*/item_backgroundColor;

	if (selected)
	{
		color = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
	}
/*
	else if (!active)
	{
		color = COL_MENUCONTENTINACTIVE;
		bgcolor = COL_MENUCONTENTINACTIVE_PLUS_0;
	}
*/
	
	if(widgetType == WIDGET_TYPE_FRAME)
	{
		//
		frameBuffer->paintBoxRel(x, y, item_width, item_height, /*item_backgroundColor*/bgcolor);

		frameBuffer->displayImage(itemIcon, x + 4*ICON_OFFSET, y + 4*ICON_OFFSET, item_width - 8*ICON_OFFSET, item_height - 8*ICON_OFFSET);

		//
		if(selected)
		{
			frameBuffer->paintBoxRel(x, y, item_width, item_height, /*item_selectedColor*/bgcolor);

			frameBuffer->displayImage(itemIcon, x + ICON_OFFSET/2, y + ICON_OFFSET/2, item_width - ICON_OFFSET, item_height - ICON_OFFSET);

		}

		// vfd
		if (selected)
		{
			CVFD::getInstance()->showMenuText(0, l_text, -1, true);
		}

		return 0;
	}
	else
	{	
		// itemBox
		frameBuffer->paintBoxRel(x, y, dx, height, bgcolor); //FIXME
	
		// left icon
		int icon_w = 0;
		int icon_h = 0;

		if (!iconName.empty())
		{
			//get icon size
			frameBuffer->getIconSize(iconName.c_str(), &icon_w, &icon_h);

			// resize icon 
			if(icon_h > height)
			{
				icon_h = height - 2;
				icon_w = icon_h*1.67;	
			}
		
			frameBuffer->paintIcon(iconName, x + BORDER_LEFT, y + (height - icon_h)/2, 0, true, icon_w, icon_h);
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

		if (nLinesItem)
		{
			if(l_text_width >= dx - BORDER_LEFT - BORDER_RIGHT)
				l_text_width = dx - BORDER_LEFT - BORDER_RIGHT;

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
}


