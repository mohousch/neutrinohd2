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
#include <gui/widget/textbox.h>

#include <driver/color.h>

#include <driver/fontrenderer.h>
#include <driver/rcinput.h>

#include <system/debug.h>

static CBox cFrameBoxText;
static CTextBox * textBox = NULL;

/// ClistBox
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
	full_height = dy;
	full_width = dx;
	start_x = x;
	start_y = y;

	hheight = 0;
	fheight = 0;
	footInfoHeight = 0;
	interFrame = 0;
	cFrameFootInfo.iHeight = 0;

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
	backgroundColor = COL_MENUCONTENT_PLUS_0;

	//
	itemBoxColor = COL_MENUCONTENTSELECTED_PLUS_0;
	itemsPerX = 6;
	itemsPerY = 3;
	maxItemsPerPage = itemsPerX*itemsPerY;

	// head
	headColor = COL_MENUHEAD_PLUS_0;
	headRadius = RADIUS_MID;
	headCorner = CORNER_TOP;
	headGradient = g_settings.Head_gradient;

	// foot
	footColor = COL_MENUFOOT_PLUS_0;
	footRadius = RADIUS_MID;
	footCorner = CORNER_BOTTOM;
	footGradient = g_settings.Foot_gradient;

	//
	widgetType = WIDGET_TYPE_STANDARD;
	widgetMode = MODE_LISTBOX;
	widgetChange = false;
}

ClistBox::ClistBox(CBox* position)
{
	frameBuffer = CFrameBuffer::getInstance();

	selected = -1;
	current_page = 0;
	pos = 0;

	cFrameBox = *position;
	full_height = position->iHeight;
	full_width = position->iWidth;
	start_x = position->iX;
	start_y = position->iY;

	hheight = 0;
	fheight = 0;
	footInfoHeight = 0;
	interFrame = 0;
	cFrameFootInfo.iHeight = 0;

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
	backgroundColor = COL_MENUCONTENT_PLUS_0;

	//
	itemBoxColor = COL_MENUCONTENTSELECTED_PLUS_0;
	itemsPerX = 6;
	itemsPerY = 3;
	maxItemsPerPage = itemsPerX*itemsPerY;

	// head
	headColor = COL_MENUHEAD_PLUS_0;
	headRadius = RADIUS_MID;
	headCorner = CORNER_TOP;
	headGradient = g_settings.Head_gradient;

	// foot
	footColor = COL_MENUFOOT_PLUS_0;
	footRadius = RADIUS_MID;
	footCorner = CORNER_BOTTOM;
	footGradient = g_settings.Foot_gradient;

	//
	widgetType = WIDGET_TYPE_STANDARD;
	widgetMode = MODE_LISTBOX;
	widgetChange = false;
}

ClistBox::~ClistBox()
{
	items.clear();
}

void ClistBox::addItem(CMenuItem * menuItem, const bool defaultselected)
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
	// reinit position
	cFrameBox.iHeight = full_height;
	cFrameBox.iWidth = full_width;
	cFrameBox.iX = start_x;
	cFrameBox.iY = start_y;

	// widgettype forwarded to item 
	for (unsigned int count = 0; count < items.size(); count++) 
	{
		CMenuItem * item = items[count];

		item->widgetType = widgetType;
		item->item_backgroundColor = backgroundColor;
		item->item_selectedColor = itemBoxColor;
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

	// footInfoHeight
	if(paintFootInfo)
	{
		cFrameFootInfo.iHeight = footInfoHeight;
		interFrame = 5;
	}

	// init frames
	if(widgetType == WIDGET_TYPE_FRAME)
	{
		//
		cFrameFootInfo.iHeight = 0;
		interFrame = 0;

		//
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
		}
	}
	else 
	{
		// sanity check
		if(paintFootInfo)
		{
			if(widgetType == WIDGET_TYPE_EXTENDED || widgetType == WIDGET_TYPE_FRAME || (widgetType == WIDGET_TYPE_CLASSIC && widgetMode == MODE_MENU) || widgetMode == MODE_SETUP)
			{
				cFrameFootInfo.iHeight = 0;
				interFrame = 0;
			}
		}

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

			if((heightCurrPage + hheight + fheight + cFrameFootInfo.iHeight + interFrame)> cFrameBox.iHeight)
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
		if(shrinkMenu)
		{
			listmaxshow = (cFrameBox.iHeight - hheight - fheight - cFrameFootInfo.iHeight - interFrame)/item_height;
			cFrameBox.iHeight = hheight + listmaxshow*item_height + fheight + cFrameFootInfo.iHeight + interFrame;
		}

		// sanity check
		if(cFrameBox.iHeight > ((int)frameBuffer->getScreenHeight() - 20))
			cFrameBox.iHeight = frameBuffer->getScreenHeight() - 20;

		// sanity check
		if(cFrameBox.iWidth > (int)frameBuffer->getScreenWidth() - 20)
			cFrameBox.iWidth = frameBuffer->getScreenWidth() - 20;

		if(paintFootInfo)
		{
			cFrameBox.iWidth -= ConnectLineBox_Width;
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

	paintHead();
	paintFoot();
	paintItems();
}

void ClistBox::paintItems()
{
	dprintf(DEBUG_NORMAL, "ClistBox::paintItems:\n");

	if(widgetType == WIDGET_TYPE_FRAME)
	{
		item_start_y = cFrameBox.iY + hheight + 10;

		// items background
		frameBuffer->paintBoxRel(cFrameBox.iX, cFrameBox.iY + hheight, cFrameBox.iWidth, cFrameBox.iHeight - hheight - fheight, backgroundColor);

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
			for (int _y = 0; _y < itemsPerY; _y++)
			{
				for (int _x = 0; _x < itemsPerX; _x++)
				{
					CMenuItem * item = items[count];

					item->init(cFrameBox.iX + _x*item_width, item_start_y + _y*item_height, items_width, iconOffset);

					if( !outFocus && ((item->isSelectable()) && (selected == -1)) ) 
					{
						selected = count;
					} 

					if (selected == (signed int)count) 
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
		items_height = cFrameBox.iHeight - hheight - fheight - cFrameFootInfo.iHeight - interFrame; 

		sb_width = 0;
	
		if(total_pages > 1)
			sb_width = SCROLLBAR_WIDTH;

		//int iwidth = cFrameBox.iWidth - sb_width;
		items_width = cFrameBox.iWidth - sb_width;

		// extended
		if(widgetType == WIDGET_TYPE_EXTENDED)
		{
			items_width = 2*(cFrameBox.iWidth/3) - sb_width;

			// extended
			cFrameBoxText.iX = cFrameBox.iX + items_width;
			cFrameBoxText.iY = cFrameBox.iY + hheight;
			cFrameBoxText.iWidth = cFrameBox.iWidth - items_width;
			cFrameBoxText.iHeight = items_height;

			textBox = new CTextBox();

			textBox->setPosition(&cFrameBoxText);
		}

		// item not currently on screen
		if (selected >= 0)
		{
			while(selected < (int)page_start[current_page])
				current_page--;
		
			while(selected >= (int)page_start[current_page + 1])
				current_page++;
		}

		frameBuffer->paintBoxRel(cFrameBox.iX, cFrameBox.iY + hheight, cFrameBox.iWidth, cFrameBox.iHeight - hheight - fheight - cFrameFootInfo.iHeight - interFrame, backgroundColor);

		if(widgetType == WIDGET_TYPE_EXTENDED && widgetMode == MODE_MENU)
		{
			frameBuffer->paintBoxRel(cFrameBox.iX + items_width, cFrameBox.iY + hheight, cFrameBox.iWidth - items_width, items_height, COL_MENUCONTENTDARK_PLUS_0);
		}
	
		// paint right scrollBar if we have more then one page
		if(total_pages > 1)
		{
			scrollBar.paint(cFrameBox.iX + cFrameBox.iWidth - SCROLLBAR_WIDTH, cFrameBox.iY + hheight, cFrameBox.iHeight - hheight - fheight - cFrameFootInfo.iHeight - interFrame, total_pages, current_page);
		}

		// paint items
		int ypos = cFrameBox.iY + hheight;
		int xpos = cFrameBox.iX;
	
		for (unsigned int count = 0; count < items.size(); count++) 
		{
			CMenuItem * item = items[count];

			if ((count >= page_start[current_page]) && (count < page_start[current_page + 1])) 
			{
				item->init(xpos, ypos, items_width, iconOffset);
			
				if( !outFocus && ((item->isSelectable()) && (selected == -1)) ) 
				{
					selected = count;
				} 

				if (selected == (signed int)count) 
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
		if(widgetType == WIDGET_TYPE_FRAME)
		{
			// box
			frameBuffer->paintBoxRel(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, hheight, backgroundColor);

			// paint horizontal line top
			frameBuffer->paintHLineRel(cFrameBox.iX + BORDER_LEFT, cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT, cFrameBox.iY + hheight - 2, COL_MENUCONTENT_PLUS_5);

			// icon
			int icon_head_w = 0;
			int icon_head_h = 0;

			frameBuffer->getIconSize(iconfile.c_str(), &icon_head_w, &icon_head_h);
			frameBuffer->paintIcon(iconfile, cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + (hheight - icon_head_h)/2);

			// Buttons
			int iw[hbutton_count], ih[hbutton_count];
			int xstartPos = cFrameBox.iX + cFrameBox.iWidth - BORDER_RIGHT;
			int buttonWidth = 0; //FIXME

			if (hbutton_count)
			{
				for (int i = 0; i < hbutton_count; i++)
				{
					frameBuffer->getIconSize(hbutton_labels[i].button, &iw[i], &ih[i]);
					xstartPos -= (iw[i] + ICON_TO_ICON_OFFSET);
					buttonWidth += iw[i];
				}

				buttons.paintHeadButtons(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, hheight, hbutton_count, hbutton_labels);
			}

			// paint time/date
			int timestr_len = 0;

			if(paintDate)
			{
				std::string timestr = getNowTimeStr("%d.%m.%Y %H:%M");;
		
				timestr_len = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getRenderWidth(timestr.c_str(), true); // UTF-8
	
				g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->RenderString(xstartPos - timestr_len, cFrameBox.iY + (hheight - g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight(), timestr_len + 1, timestr.c_str(), COL_MENUHEAD, 0, true); 
			}

			// title
			g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(cFrameBox.iX + BORDER_LEFT + icon_head_w + ICON_OFFSET, cFrameBox.iY + (hheight - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT - icon_head_w - 2*ICON_OFFSET - buttonWidth - (hbutton_count - 1)*ICON_TO_ICON_OFFSET - timestr_len, l_name, COL_MENUHEAD);
		}
		else
		{
			headers.setHeadColor(headColor);
			headers.setHeadCorner(headRadius, headCorner);
			headers.setHeadGradient(headGradient);
		
			if(paintDate)
				headers.enablePaintDate();

			if(logo)
				headers.enableLogo();

			headers.setHeaderButtons(hbutton_labels, hbutton_count);

			headers.paintHead(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, hheight, l_name.c_str(), iconfile.c_str());
		}
	}	
}

void ClistBox::paintFoot()
{
	if(paint_Foot)
	{
		if(widgetType == WIDGET_TYPE_FRAME)
		{
			frameBuffer->paintBoxRel(cFrameBox.iX, cFrameBox.iY + cFrameBox.iHeight - fheight, cFrameBox.iWidth, fheight, backgroundColor);

			// paint horizontal line buttom
			frameBuffer->paintHLineRel(cFrameBox.iX + BORDER_LEFT, cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT, cFrameBox.iY + cFrameBox.iHeight - fheight + 2, COL_MENUCONTENT_PLUS_5);
		}
		else
		{
			headers.setFootColor(footColor);
			headers.setFootCorner(footRadius, footCorner);
			headers.setFootGradient(footGradient);

			headers.paintFoot(cFrameBox.iX, cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight - fheight - interFrame, cFrameBox.iWidth, fheight, fbutton_count, fbutton_labels);
		}
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
	if(paint_Foot && (widgetMode != MODE_MENU))
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
		if(widgetType == WIDGET_TYPE_STANDARD)
		{
			if(widgetMode == MODE_LISTBOX)
			{
				CMenuItem* item = items[pos];

				item->getYPosition();
	
				// detailslines
				itemsLine.paint(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight - cFrameFootInfo.iHeight, cFrameFootInfo.iHeight, item->getHeight(), item->getYPosition());

				// option_info1
				int l_ow1 = 0;
				if(!item->option_info1.empty())
				{
					l_ow1 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getRenderWidth(item->option_info1.c_str());

					g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString(cFrameBox.iX + cFrameBox.iWidth - BORDER_RIGHT - l_ow1, cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight(), cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT - l_ow1, item->option_info1.c_str(), COL_MENUFOOT_INFO, 0, true);
				}

				// info1
				int l_w1 = 0;
				if(!item->info1.empty())
				{
					l_w1 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getRenderWidth(item->info1.c_str());

					g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT - l_ow1, item->info1.c_str(), COL_MENUFOOT_INFO, 0, true);
				}

				// option_info2
				int l_ow2 = 0;
				if(!item->option_info2.empty())
				{
					l_ow2 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth(item->option_info2.c_str());

					g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(cFrameBox.iX + cFrameBox.iWidth - BORDER_RIGHT - l_ow2, cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight + cFrameFootInfo.iHeight/2 + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight(), cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT - l_ow2, item->option_info2.c_str(), COL_MENUFOOT_INFO, 0, true);
				}

				// info2
				int l_w2 = 0;
				if(!item->info2.empty())
				{
					l_w2 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getRenderWidth(item->info2.c_str());

					g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString (cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight + cFrameFootInfo.iHeight/2 + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight(), cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT - l_ow2, item->info2.c_str(), COL_MENUFOOT_INFO, 0, true); // UTF-8
				}
			}
			else if(widgetMode == MODE_MENU)
			{
				CMenuItem* item = items[pos];
	
				// detailslines box
				itemsLine.paint(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight - cFrameFootInfo.iHeight, cFrameFootInfo.iHeight, item->getHeight(), item->getYPosition());

				
				// itemIcon
				if(!item->itemIcon.empty())
					frameBuffer->displayImage(item->itemIcon.c_str(), cFrameBox.iX + ICON_OFFSET, cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight + (cFrameFootInfo.iHeight - 40)/2, 100, 40);

				// HelpText
				cFrameBoxText.iX = cFrameBox.iX + 100 + ICON_OFFSET;
				cFrameBoxText.iY = cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight + 2;
				cFrameBoxText.iWidth = cFrameBox.iWidth - 100 - ICON_OFFSET - 2;
				cFrameBoxText.iHeight = cFrameFootInfo.iHeight - 4;

				if(textBox)
				{
					delete textBox;
					textBox = NULL;
				}
	
				textBox = new CTextBox();
				textBox->setPosition(&cFrameBoxText);
				textBox->disablePaintBackground();
				textBox->setMode(~SCROLL);

				// HelpText
				if(!item->itemHelpText.empty())
				{
					textBox->setText(item->itemHelpText.c_str());
					textBox->paint();
				}
			}
		}
		else if(widgetType == WIDGET_TYPE_CLASSIC)
		{
			if(widgetMode == MODE_LISTBOX)
			{
				CMenuItem* item = items[pos];

				item->getYPosition();
	
				// detailslines
				itemsLine.paint(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight - cFrameFootInfo.iHeight, cFrameFootInfo.iHeight, item->getHeight(), item->getYPosition());

				// option_info1
				int l_ow1 = 0;
				if(!item->option_info1.empty())
				{
					l_ow1 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getRenderWidth(item->option_info1.c_str());

					g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString(cFrameBox.iX + cFrameBox.iWidth - BORDER_RIGHT - l_ow1, cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight(), cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT - l_ow1, item->option_info1.c_str(), COL_MENUFOOT_INFO, 0, true);
				}

				// info1
				int l_w1 = 0;
				if(!item->info1.empty())
				{
					l_w1 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getRenderWidth(item->info1.c_str());

					g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT - l_ow1, item->info1.c_str(), COL_MENUFOOT_INFO, 0, true);
				}

				// option_info2
				int l_ow2 = 0;
				if(!item->option_info2.empty())
				{
					l_ow2 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth(item->option_info2.c_str());

					g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(cFrameBox.iX + cFrameBox.iWidth - BORDER_RIGHT - l_ow2, cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight + cFrameFootInfo.iHeight/2 + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight(), cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT - l_ow2, item->option_info2.c_str(), COL_MENUFOOT_INFO, 0, true);
				}

				// info2
				int l_w2 = 0;
				if(!item->info2.empty())
				{
					l_w2 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getRenderWidth(item->info2.c_str());

					g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString (cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight + cFrameFootInfo.iHeight/2 + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight(), cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT - l_ow2, item->info2.c_str(), COL_MENUFOOT_INFO, 0, true); // UTF-8
				}
			}
			else if(widgetMode == MODE_MENU)
			{
				CMenuItem* item = items[pos];

				item->getYPosition();

				// refresh box
				frameBuffer->paintBoxRel(cFrameBox.iX, cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight - fheight, cFrameBox.iWidth, fheight, COL_MENUFOOT_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.Foot_gradient);

				// info icon
				int iw, ih;
				frameBuffer->getIconSize(NEUTRINO_ICON_INFO, &iw, &ih);
				frameBuffer->paintIcon(NEUTRINO_ICON_INFO, cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight - fheight + (fheight - ih)/2);

				// HelpText
				if(!item->itemHelpText.empty())
				{
					g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->RenderString(cFrameBox.iX + BORDER_LEFT + iw + ICON_OFFSET, cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight - fheight + (fheight - g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getHeight(), cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT - iw, item->itemHelpText.c_str(), COL_MENUFOOT, 0, true); // UTF-8
				}
			}
		}
		else if(widgetType == WIDGET_TYPE_EXTENDED)
		{
			CMenuItem* item = items[pos];

			item->getYPosition();

			if(widgetMode == MODE_MENU)
			{
				// item info
				// refresh box
				frameBuffer->paintBoxRel(cFrameBox.iX, cFrameBox.iY + cFrameBox.iHeight - fheight, cFrameBox.iWidth, fheight, COL_MENUFOOT_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.Foot_gradient);

				// info icon
				int iw, ih;
				frameBuffer->getIconSize(NEUTRINO_ICON_INFO, &iw, &ih);
				frameBuffer->paintIcon(NEUTRINO_ICON_INFO, cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBox.iHeight - fheight + (fheight - ih)/2);

				// itemHelpText
				if(!item->itemHelpText.empty())
				{
					g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->RenderString(cFrameBox.iX + BORDER_LEFT + iw + ICON_OFFSET, cFrameBox.iY + cFrameBox.iHeight - fheight + (fheight - g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getHeight(), cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT - iw, item->itemHelpText.c_str(), COL_MENUFOOT, 0, true); // UTF-8
				}

				// item icon
				// check for minimum hight
				if(cFrameBox.iHeight - hheight - fheight >= ITEM_ICON_H)
				{ 
					frameBuffer->getIconSize(item->itemIcon.c_str(), &iw, &ih);

					// refreshbox
					frameBuffer->paintBoxRel(cFrameBox.iX + items_width + (cFrameBox.iWidth - items_width - ITEM_ICON_W)/2, cFrameBox.iY + (cFrameBox.iHeight - ITEM_ICON_H)/2, ITEM_ICON_W, ITEM_ICON_H, COL_MENUCONTENTDARK_PLUS_0);

					frameBuffer->displayImage(item->itemIcon.c_str(), cFrameBox.iX + items_width + (cFrameBox.iWidth - items_width - ITEM_ICON_W)/2, cFrameBox.iY + (cFrameBox.iHeight - ITEM_ICON_H)/2, ITEM_ICON_W, ITEM_ICON_H);
				}
			}
			else if(widgetMode == MODE_LISTBOX)
			{
				// scale pic
				int p_w = 0;
				int p_h = 0;

				std::string fname = item->itemIcon;

				CFrameBuffer::getInstance()->scaleImage(fname, &p_w, &p_h);

				textBox->setBackgroundColor(COL_MENUCONTENTDARK_PLUS_0);

				// helpText
				textBox->setText(item->itemHelpText.c_str(), item->itemIcon.c_str(), p_w, p_h, TOP_CENTER);
				textBox->paint();
			}
		}
		else if(widgetType == WIDGET_TYPE_FRAME)
		{
			if(paint_Foot)
			{
				// refresh
				frameBuffer->paintBoxRel(cFrameBox.iX, cFrameBox.iY + cFrameBox.iHeight - fheight, cFrameBox.iWidth, fheight, backgroundColor);

				// refresh horizontal line buttom
				frameBuffer->paintHLineRel(cFrameBox.iX + BORDER_LEFT, cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT, cFrameBox.iY + cFrameBox.iHeight - fheight + 2, COL_MENUCONTENT_PLUS_5);

				if(items.size() > 0)
				{
					CMenuItem* item = items[pos];
	
					// itemName
					if(!item->itemName.empty())
					{
						g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBox.iHeight - fheight + (fheight - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE] ->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT, item->itemName.c_str(), COL_MENUFOOT_INFO);
					}

					// helpText
					if(!item->itemHelpText.empty())
					{
						g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBox.iHeight, cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT, item->itemHelpText.c_str(), COL_MENUFOOT_INFO);
					}
				}
			}
		}
	}
	else
	{
		if(widgetType == WIDGET_TYPE_STANDARD)
		{
			if(widgetMode == MODE_MENU)
			{
				CMenuItem* item = items[pos];

				// refresh box
				frameBuffer->paintBoxRel(cFrameBox.iX, cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight - fheight, cFrameBox.iWidth, fheight, COL_MENUFOOT_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.Foot_gradient);

				// info icon
				int iw, ih;
				frameBuffer->getIconSize(NEUTRINO_ICON_INFO, &iw, &ih);
				frameBuffer->paintIcon(NEUTRINO_ICON_INFO, cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight - fheight + (fheight - ih)/2);

				// HelpText
				if(!item->itemHelpText.empty())
				{
					g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->RenderString(cFrameBox.iX + BORDER_LEFT + iw + ICON_OFFSET, cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight - fheight + (fheight - g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getHeight(), cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT - iw, item->itemHelpText.c_str(), COL_MENUFOOT, 0, true); // UTF-8
				}
			}
		}
	}
}

void ClistBox::hideItemInfo()
{
	if(paintFootInfo)
		itemsLine.clear(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth + ConnectLineBox_Width, cFrameBox.iHeight - cFrameFootInfo.iHeight, cFrameFootInfo.iHeight);
}

void ClistBox::hide()
{
	dprintf(DEBUG_NORMAL, "ClistBox::hide:\n");

	frameBuffer->paintBackgroundBoxRel(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight);

	hideItemInfo(); 
	
	frameBuffer->blit();

	if(textBox != NULL)
	{
		delete textBox;
		textBox = NULL;
	}
}

void ClistBox::scrollLineDown()
{
	if(widgetType == WIDGET_TYPE_FRAME)
	{
		if(items.size())
		{
			pos = selected + itemsPerX;

			//FIXME:
			if (pos >= (int)items.size())
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
	dprintf(DEBUG_NORMAL, "ClistBox::swipLeft:\n");

	if(widgetType == WIDGET_TYPE_FRAME)
	{
		if(items.size())
		{
			//search next / prev selectable item
			for (int count = (int)page_start[current_page] + 1; count < (int)page_start[current_page + 1]; count++)
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
	else if (widgetType == WIDGET_TYPE_EXTENDED)
	{
		if(textBox)
			textBox->scrollPageUp(1);
	}
}

void ClistBox::swipRight()
{
	dprintf(DEBUG_NORMAL, "ClistBox::swipRight:\n");

	if(widgetType == WIDGET_TYPE_FRAME)
	{
		if(items.size())
		{
			//search next / prev selectable item
			for (int count = (int)page_start[current_page] + 1; count < (int)page_start[current_page + 1]; count++)
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
	else if (widgetType == WIDGET_TYPE_EXTENDED)
	{
		if(textBox)
			textBox->scrollPageDown(1);
	}
}

void ClistBox::changeWidgetType()
{
	int cnt = widgetType;

	if(widgetMode == MODE_MENU)
	{
		if(widgetChange)
		{
			hide();

			if(widgetType == WIDGET_TYPE_STANDARD)
				widgetType = WIDGET_TYPE_CLASSIC;
			else if(widgetType == WIDGET_TYPE_CLASSIC)
				widgetType = WIDGET_TYPE_EXTENDED;
			else if(widgetType == WIDGET_TYPE_EXTENDED)
				widgetType = WIDGET_TYPE_FRAME;
			else if(widgetType == WIDGET_TYPE_FRAME)
				widgetType = WIDGET_TYPE_STANDARD;

			g_settings.menu_design = widgetType;
			
			paint();
		}
	}
	else if(widgetMode == MODE_LISTBOX)
	{
		if(widgetChange && widget.size())
		{
			hide();

			cnt++;

			if(cnt >= (int)widget.size())
			{
				cnt = 0;
			}
					
			widgetType = widget[cnt];

			paint();
		}
	}
}




