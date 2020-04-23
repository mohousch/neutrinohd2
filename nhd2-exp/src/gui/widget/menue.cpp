/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: menue.cpp 2018/07/25 mohousch Exp $

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h> //acces
#include <cctype>

#include <global.h>
#include <neutrino.h>

#include <gui/widget/menue.h>
#include <gui/widget/icons.h>
#include <gui/widget/textbox.h>
#include <gui/widget/stringinput.h> // locked menu

#include <driver/color.h>
#include <gui/pluginlist.h>

#include <driver/fontrenderer.h>
#include <driver/rcinput.h>

#include <system/debug.h>
#include <system/settings.h>
	

extern CPlugins * g_PluginList;    // defined in neutrino.cpp

//
static CTextBox * textBox = NULL;

/// ClistBoxWidget
ClistBoxWidget::ClistBoxWidget()
{
        nameString = g_Locale->getText(NONEXISTANT_LOCALE);
	name = NONEXISTANT_LOCALE;

	Init("", MENU_WIDTH, MENU_HEIGHT);
}

ClistBoxWidget::ClistBoxWidget(const neutrino_locale_t Name, const std::string & Icon, const int mwidth, const int mheight)
{
	name = Name;
        nameString = g_Locale->getText(NONEXISTANT_LOCALE);

	Init(Icon, mwidth, mheight);
}

ClistBoxWidget::ClistBoxWidget(const char* Name, const std::string & Icon, const int mwidth, const int mheight)
{
	name = NONEXISTANT_LOCALE;
        nameString = Name;

	Init(Icon, mwidth, mheight);
}

void ClistBoxWidget::Init(const std::string &Icon, const int mwidth, const int mheight)
{
        frameBuffer = CFrameBuffer::getInstance();
        iconfile = Icon;
        selected = -1;
        width = mwidth;
	height = mheight;
	
        if(width > (int) frameBuffer->getScreenWidth() - 20)
		width = frameBuffer->getScreenWidth() - 20;

	if(height > ((int)frameBuffer->getScreenHeight() - 20))
		height = frameBuffer->getScreenHeight() - 20;

	full_width = width;
	full_height = height;

	wanted_width = width;
	wanted_height = height;

        current_page = 0;
	offx = offy = 0;
	
	//
	savescreen = false;
	background = NULL;

	//
	fbutton_count	= 0;
	fbutton_labels	= NULL;
	fbutton_width = width;

	//
	PaintDate = false;
	timestr_len = 0;

	//
	hbutton_count	= 0;
	hbutton_labels	= NULL;

	//
	FootInfo = false;
	footInfoHeight = 0;
	cFrameFootInfo.iHeight = 0;
	connectLineWidth = 0;

	timeout = 0;

	//
	widgetType = WIDGET_TYPE_STANDARD;
	widgetChange = false;

	// frame
	backgroundColor = COL_MENUCONTENT_PLUS_0;
	itemBoxColor = COL_MENUCONTENTSELECTED_PLUS_0;
	itemsPerX = 6;
	itemsPerY = 3;
	maxItemsPerPage = itemsPerX*itemsPerY;

	shrinkMenu = false;

	widgetMode = MODE_LISTBOX;
	MenuPos = false;

	//headers = new CHeaders();
}

void ClistBoxWidget::move(int xoff, int yoff)
{
	offx = xoff;
	offy = yoff;
}

ClistBoxWidget::~ClistBoxWidget()
{
	dprintf(DEBUG_NORMAL, "ClistBoxWidget:: del\n");

	items.clear();
	page_start.clear();

/*
	if(headers)
	{
		delete headers;
		headers = NULL;
	}

	if(footers)
	{
		delete footers;
		footers = NULL;
	}
*/
}

void ClistBoxWidget::addItem(CMenuItem *menuItem, const bool defaultselected)
{
	if (defaultselected)
		selected = items.size();
	
	items.push_back(menuItem);
}

void ClistBoxWidget::removeItem(long pos)
{
	items.erase(items.begin() + pos); 
}

bool ClistBoxWidget::hasItem()
{
	return !items.empty();
}

void ClistBoxWidget::initFrames()
{
	if(name == NONEXISTANT_LOCALE)
		l_name = nameString.c_str();
	else
        	l_name = g_Locale->getText(name);

	// widget type
	if(widgetChange && widgetMode == MODE_MENU)
	{
		if(g_settings.menu_design == SNeutrinoSettings::MENU_DESIGN_STANDARD)
			widgetType = WIDGET_TYPE_STANDARD;
		else if(g_settings.menu_design == SNeutrinoSettings::MENU_DESIGN_CLASSIC)
			widgetType = WIDGET_TYPE_CLASSIC;
		else if(g_settings.menu_design == SNeutrinoSettings::MENU_DESIGN_EXTENDED)
			widgetType = WIDGET_TYPE_EXTENDED;
		else if(g_settings.menu_design == SNeutrinoSettings::MENU_DESIGN_FRAME)
			widgetType = WIDGET_TYPE_FRAME;
	}

	// reinit
	width = wanted_width;
	height = wanted_height;

	// widgettype forwarded to item 
	for (unsigned int count = 0; count < items.size(); count++) 
	{
		CMenuItem * item = items[count];

		item->widgetType = widgetType;
		item->item_backgroundColor = backgroundColor;
		item->item_selectedColor = itemBoxColor;
	} 

	// init frames
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
		x = g_settings.screen_StartX;
		y = g_settings.screen_StartY;
		width = g_settings.screen_EndX - g_settings.screen_StartX;
		height = g_settings.screen_EndY - g_settings.screen_StartY;

		full_width = width;
		full_height = height;

		//head height
		icon_head_w = 0;
		icon_head_h = 0;
		frameBuffer->getIconSize(iconfile.c_str(), &icon_head_w, &icon_head_h);
		hheight = std::max(icon_head_h, g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight()) + 6;
	
		// foot height
		fheight = hheight;

		//
		item_width = width/itemsPerX;
		item_height = (height - hheight - fheight - (fbutton_count != 0? fheight : 0) - 20)/itemsPerY; // 20 pixels for hlines

		for (unsigned int count = 0; count < items.size(); count++) 
		{
			CMenuItem * item = items[count];

			item->item_width = item_width;
			item->item_height = item_height;
		} 

		if(savescreen) 
			saveScreen();
	}
	else
	{
		// footInfo height
		cFrameFootInfo.iHeight = 0;
		connectLineWidth = 0;

		if(FootInfo && (widgetType == WIDGET_TYPE_STANDARD || (widgetType == WIDGET_TYPE_CLASSIC && widgetMode == MODE_LISTBOX))&& widgetMode != MODE_SETUP)
		{
			cFrameFootInfo.iHeight = footInfoHeight;
			connectLineWidth = CONNECTLINEBOX_WIDTH;
		}

		// width
		width -= connectLineWidth;

		// head height
		icon_head_w = 0;
		icon_head_h = 0;
		frameBuffer->getIconSize(iconfile.c_str(), &icon_head_w, &icon_head_h);
		hheight = std::max(icon_head_h, g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight()) + 6;
	
		// foot height
		int icon_foot_w = 0;
		int icon_foot_h = 0;
		frameBuffer->getIconSize(NEUTRINO_ICON_INFO, &icon_foot_w, &icon_foot_h);
		fheight = std::max(icon_foot_h, g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight()) + 6;

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

			if( (heightCurrPage + hheight + fheight) > height)
			{
				page_start.push_back(i);
				total_pages++;
				heightFirstPage = std::max(heightCurrPage - item_height, heightFirstPage);
				heightCurrPage = item_height;
			}
		}

		heightFirstPage = std::max(heightCurrPage, heightFirstPage);
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

		// shrink menu if less items
		if(shrinkMenu)
		{
			height = std::min(height, hheight + heightFirstPage + fheight);
		}

		//
		full_width = width;
		full_height = height + cFrameFootInfo.iHeight;
		
		// position
		// default centered
		x = offx + frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - full_width ) >> 1 );
		y = offy + frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - full_height) >> 1 );

		// menu position
		if(widgetMode == MODE_MENU)
		{
			if(g_settings.menu_position == SNeutrinoSettings::MENU_POSITION_CENTER && MenuPos)
			{
				x = offx + frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - full_width ) >> 1 );
				y = offy + frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - full_height) >> 1 );
			}
			else if(g_settings.menu_position == SNeutrinoSettings::MENU_POSITION_LEFT && MenuPos)
			{
				x = offx + frameBuffer->getScreenX() + connectLineWidth;
				y = offy + frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - full_height) >> 1 );
			}
			else if(g_settings.menu_position == SNeutrinoSettings::MENU_POSITION_RIGHT && MenuPos)
			{
				x = offx + frameBuffer->getScreenX() + frameBuffer->getScreenWidth() - full_width - connectLineWidth;
				y = offy + frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - full_height) >> 1 );
			}
		}

		// re-set FrameFootInfo position
		if(FootInfo && (widgetType == WIDGET_TYPE_STANDARD || (widgetType == WIDGET_TYPE_CLASSIC && widgetMode == MODE_LISTBOX)))
		{
			cFrameFootInfo.iX = x;
			cFrameFootInfo.iY = y + height;
			cFrameFootInfo.iWidth = width;
		}

		headers = new CHeaders(x, y, width, hheight, l_name, iconfile.c_str());
		footers = new CFooters(x, y + height - fheight, width, fheight, fbutton_count, fbutton_labels);

		if(savescreen) 
			saveScreen();
	}
}

void ClistBoxWidget::paintHead()
{
	if(widgetType == WIDGET_TYPE_FRAME)
	{
		// box
		frameBuffer->paintBoxRel(x, y, width, hheight, backgroundColor);

		// paint horizontal line top
		frameBuffer->paintHLineRel(x + BORDER_LEFT, width - BORDER_LEFT - BORDER_RIGHT, y + hheight - 2, COL_MENUCONTENT_PLUS_5);

		// icon
		frameBuffer->paintIcon(iconfile, x + BORDER_LEFT, y + (hheight - icon_head_h)/2);

		// Buttons
		int iw[hbutton_count], ih[hbutton_count];
		int xstartPos = x + width - BORDER_RIGHT;
		int buttonWidth = 0; //FIXME

		if (hbutton_count)
		{
			for (int i = 0; i < hbutton_count; i++)
			{
				frameBuffer->getIconSize(hbutton_labels[i].button, &iw[i], &ih[i]);
				xstartPos -= (iw[i] + ICON_TO_ICON_OFFSET);
				buttonWidth += iw[i];
			}

			buttons.paintHeadButtons(x, y, width, hheight, hbutton_count, hbutton_labels);
		}

		// paint time/date
		if(PaintDate)
		{
			std::string timestr = getNowTimeStr("%d.%m.%Y %H:%M");;
		
			timestr_len = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getRenderWidth(timestr.c_str(), true); // UTF-8
	
			g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->RenderString(xstartPos - timestr_len, y + (hheight - g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight(), timestr_len + 1, timestr.c_str(), COL_MENUHEAD, 0, true); 
		}

		// title
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(x + BORDER_LEFT + icon_head_w + ICON_OFFSET, y + (hheight - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), width - BORDER_LEFT - BORDER_RIGHT - icon_head_w - 2*ICON_OFFSET - buttonWidth - (hbutton_count - 1)*ICON_TO_ICON_OFFSET - timestr_len, l_name, COL_MENUHEAD);
	}
	else
	{
/*
		if(PaintDate)
			headers->enablePaintDate();

		headers->setHeaderButtons(hbutton_labels, hbutton_count);
		headers->paint();
*/
		// paint head
		frameBuffer->paintBoxRel(x, y, width, hheight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, g_settings.Head_gradient);
	
		//paint icon (left)
		frameBuffer->paintIcon(iconfile, x + BORDER_LEFT, y + (hheight - icon_head_h)/2);

		// Buttons
		int iw[hbutton_count], ih[hbutton_count];
		int xstartPos = x + width - BORDER_RIGHT;
		int buttonWidth = 0; //FIXME

		if (hbutton_count)
		{
			for (unsigned int i = 0; i < hbutton_count; i++)
			{
				frameBuffer->getIconSize(hbutton_labels[i].button, &iw[i], &ih[i]);
				xstartPos -= (iw[i] + ICON_TO_ICON_OFFSET);
				buttonWidth += iw[i];
			}

			buttons.paintHeadButtons(x, y, width, hheight, hbutton_count, hbutton_labels);
		}

		// paint time/date
		if(PaintDate)
		{
			std::string timestr = getNowTimeStr("%d.%m.%Y %H:%M");;
		
			timestr_len = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getRenderWidth(timestr.c_str(), true); // UTF-8
	
			g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->RenderString(xstartPos - timestr_len, y + (hheight - g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight(), timestr_len + 1, timestr.c_str(), COL_MENUHEAD, 0, true); 
		}
	
		// head title
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(x + BORDER_LEFT + icon_head_w + 2*ICON_OFFSET, y + (hheight - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), width - BORDER_RIGHT - BORDER_RIGHT - icon_head_w - 2*ICON_OFFSET - timestr_len - buttonWidth - (hbutton_count - 1)*ICON_TO_ICON_OFFSET, l_name, COL_MENUHEAD, 0, true); // UTF-8
	}
}

void ClistBoxWidget::paintFoot()
{
	if(widgetType == WIDGET_TYPE_FRAME)
	{
		//
		frameBuffer->paintBoxRel(x, y + height - fheight, width, fheight, backgroundColor);

		// paint horizontal line buttom
		frameBuffer->paintHLineRel(x + BORDER_LEFT, width - BORDER_LEFT - BORDER_RIGHT, y + height - fheight + 2, COL_MENUCONTENT_PLUS_5);

		// buttons
		buttons.paintFootButtons(x, y + height - fheight, width, fheight, fbutton_count, fbutton_labels);
	}
	else
	{
		footers->paint();
	}
}

void ClistBoxWidget::paint()
{
	dprintf(DEBUG_NORMAL, "ClistBoxWidget::paint:\n");

	CVFD::getInstance()->setMode(CVFD::MODE_MENU_UTF8 );

	item_start_y = y + hheight;

	if(widgetType == WIDGET_TYPE_FRAME)
		item_start_y = y + hheight + 10;

	// paint background
	if(widgetType == WIDGET_TYPE_FRAME)
	{
		frameBuffer->paintBoxRel(x, y + hheight, width, height - hheight - fheight, backgroundColor);
	}
	else
	{
		frameBuffer->paintBoxRel(x, item_start_y, width, height - hheight - fheight, COL_MENUCONTENT_PLUS_0);
	}

	//
	paintItems();
}

// paint items
void ClistBoxWidget::paintItems()
{
	if(widgetType == WIDGET_TYPE_FRAME)
	{
		// items background
		frameBuffer->paintBoxRel(x, y + hheight + 10, width, height - hheight - fheight - (fbutton_count != 0? fheight : 0) - 20, backgroundColor);

		// item not currently on screen
		if (selected >= 0)
		{
			while(selected < (int)page_start[current_page])
				current_page--;
		
			while(selected >= (int)page_start[current_page + 1])
				current_page++;
		}

		// reset items
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

					item->init(x + _x*item_width, item_start_y + _y*item_height, items_width, iconOffset);

					if( (item->isSelectable()) && (selected == -1)) 
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
		// items height
		items_height = height - hheight - fheight;
	
		// items width
		sb_width = 0;
	
		if(total_pages > 1)
			sb_width = SCROLLBAR_WIDTH; 
	
		items_width = width - sb_width;

		// extended
		if(widgetType == WIDGET_TYPE_EXTENDED)
		{
			items_width = 2*(width/3) - sb_width;

			// extended
			if(textBox)
			{
				delete textBox;
				textBox = NULL;
			}

			textBox = new CTextBox(x + 2*(width/3), y + hheight, width/3, items_height);
		}
	
		// item not currently on screen
		if (selected >= 0)
		{
			while(selected < (int)page_start[current_page])
				current_page--;
		
			while(selected >= (int)page_start[current_page + 1])
				current_page++;
		}
	
		// paint items background
		frameBuffer->paintBoxRel(x, item_start_y, width, items_height, COL_MENUCONTENT_PLUS_0);

		if(widgetType == WIDGET_TYPE_EXTENDED && widgetMode == MODE_MENU)
		{
			frameBuffer->paintBoxRel(x + items_width, item_start_y, width - items_width, items_height, COL_MENUCONTENTDARK_PLUS_0);
		}
	
		// paint right scrollBar if we have more then one page
		if(total_pages > 1)
		{
			if(widgetType == WIDGET_TYPE_EXTENDED)
				scrollBar.paint(x + 2*(width/3) - SCROLLBAR_WIDTH, item_start_y, items_height, total_pages, current_page);
			else
				scrollBar.paint(x + width - SCROLLBAR_WIDTH, item_start_y, items_height, total_pages, current_page);
		}

		// paint items
		int ypos = item_start_y;
		int xpos = x;
	
		for (unsigned int count = 0; count < items.size(); count++) 
		{
			CMenuItem * item = items[count];

			if ((count >= page_start[current_page]) && (count < page_start[current_page + 1])) 
			{
				item->init(xpos, ypos, items_width, iconOffset);

				if( (item->isSelectable()) && (selected == -1)) 
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

void ClistBoxWidget::paintItemInfo(int pos)
{
	if(widgetType == WIDGET_TYPE_STANDARD)
	{
		if(widgetMode == MODE_MENU)
		{
			if(FootInfo)
			{
				CMenuItem* item = items[pos];

				item->getYPosition();
	
				// detailslines|box
				itemsLine.paint(x, y, width, height, cFrameFootInfo.iHeight, item->getHeight(), item->getYPosition());


				// item icon
				if(!item->itemIcon.empty())
					frameBuffer->displayImage(item->itemIcon.c_str(), x + ICON_OFFSET, y + height + (cFrameFootInfo.iHeight - 40)/2, 100, 40);

				// HelpText
				if(textBox)
				{
					delete textBox;
					textBox = NULL;
				}
	
				textBox = new CTextBox(x + 100 + ICON_OFFSET, y + full_height - cFrameFootInfo.iHeight + 2, width - 4 - ICON_OFFSET - 100, cFrameFootInfo.iHeight - 4);
				textBox->disablePaintBackground();
				textBox->setMode(~SCROLL);

				// HelpText
				if(!item->itemHelpText.empty())
				{
					textBox->setText(item->itemHelpText.c_str());
					textBox->paint();
				}
			}
			else
			{
				CMenuItem* item = items[pos];

				item->getYPosition();

				// refresh box
				frameBuffer->paintBoxRel(x, y + full_height - fheight, width, fheight, COL_MENUFOOT_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.Foot_gradient);

				// info icon
				int iw, ih;
				frameBuffer->getIconSize(NEUTRINO_ICON_INFO, &iw, &ih);
				frameBuffer->paintIcon(NEUTRINO_ICON_INFO, x + BORDER_LEFT, y + full_height - fheight + (fheight - ih)/2);

				// HelpText
				if(!item->itemHelpText.empty())
				{
					g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->RenderString(x + BORDER_LEFT + iw + ICON_OFFSET, y + full_height - fheight + (fheight - g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getHeight(), width - BORDER_LEFT - BORDER_RIGHT - iw, item->itemHelpText.c_str(), COL_MENUFOOT, 0, true); // UTF-8
				}
			}
		}
		else if(widgetMode == MODE_LISTBOX)
		{
			if(FootInfo)
			{
				CMenuItem * item = items[pos];

				item->getYPosition();
	
				// detailslines
				itemsLine.paint(x, y, width, height, cFrameFootInfo.iHeight, item->getHeight(), item->getYPosition());

				// option_info1
				int l_ow1 = 0;
				if(!item->option_info1.empty())
				{
					l_ow1 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getRenderWidth(item->option_info1.c_str());

					g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString(x + width - BORDER_RIGHT - l_ow1, y + height + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight(), width - BORDER_LEFT - BORDER_RIGHT - l_ow1, item->option_info1.c_str(), COL_MENUFOOT_INFO, 0, true);
				}

				// info1
				int l_w1 = 0;
				if(!item->info1.empty())
				{
					l_w1 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getRenderWidth(item->info1.c_str());

					if(l_w1 >= (width - BORDER_LEFT - BORDER_RIGHT - l_ow1))
						l_w1 = 0;

					g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(x + BORDER_LEFT, y + height + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), width - BORDER_LEFT - BORDER_RIGHT - l_ow1, item->info1.c_str(), COL_MENUFOOT_INFO, 0, true);
				}

				// option_info2
				int l_ow2 = 0;
				if(!item->option_info2.empty())
				{
					l_ow2 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth(item->option_info2.c_str());

					g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(x + width - BORDER_RIGHT - l_ow2, y + height + cFrameFootInfo.iHeight/2 + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight(), width - BORDER_LEFT - BORDER_RIGHT - l_ow2, item->option_info2.c_str(), COL_MENUFOOT_INFO, 0, true);
				}

				// info2
				int l_w2 = 0;
				if(!item->info2.empty())
				{
					l_w2 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getRenderWidth(item->info2.c_str());

					if(l_w2 >= (width - BORDER_LEFT - BORDER_RIGHT - l_ow2))
						l_w2 = 0;

					g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString (x + BORDER_LEFT, y + height + cFrameFootInfo.iHeight/2 + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight(), width - BORDER_LEFT - BORDER_RIGHT - l_ow2, item->info2.c_str(), COL_MENUFOOT_INFO, 0, true); // UTF-8
				}
			}
		}
	}
	else if(widgetType == WIDGET_TYPE_CLASSIC)
	{
		if(widgetMode == MODE_MENU)
		{
			CMenuItem* item = items[pos];

			item->getYPosition();

			// refresh box
			frameBuffer->paintBoxRel(x, y + full_height - fheight, width, fheight, COL_MENUFOOT_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.Foot_gradient);

			// info icon
			int iw, ih;
			frameBuffer->getIconSize(NEUTRINO_ICON_INFO, &iw, &ih);
			frameBuffer->paintIcon(NEUTRINO_ICON_INFO, x + BORDER_LEFT, y + full_height - fheight + (fheight - ih)/2);

			// HelpText
			if(!item->itemHelpText.empty())
			{
				g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->RenderString(x + BORDER_LEFT + iw + ICON_OFFSET, y + full_height - fheight + (fheight - g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getHeight(), width - BORDER_LEFT - BORDER_RIGHT - iw, item->itemHelpText.c_str(), COL_MENUFOOT, 0, true); // UTF-8
			}
		}
		else if(widgetMode == MODE_LISTBOX)
		{
			if(FootInfo)
			{
				CMenuItem * item = items[pos];

				item->getYPosition();
	
				// detailslines
				itemsLine.paint(x, y, width, height, cFrameFootInfo.iHeight, item->getHeight(), item->getYPosition());

				// option_info1
				int l_ow1 = 0;
				if(!item->option_info1.empty())
				{
					l_ow1 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getRenderWidth(item->option_info1.c_str());

					g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString(x + width - BORDER_RIGHT - l_ow1, y + height + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight(), width - BORDER_LEFT - BORDER_RIGHT - l_ow1, item->option_info1.c_str(), COL_MENUFOOT_INFO, 0, true);
				}

				// info1
				int l_w1 = 0;
				if(!item->info1.empty())
				{
					l_w1 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getRenderWidth(item->info1.c_str());

					if(l_w1 >= (width - BORDER_LEFT - BORDER_RIGHT - l_ow1))
						l_w1 = 0;

					g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(x + BORDER_LEFT, y + height + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), width - BORDER_LEFT - BORDER_RIGHT - l_ow1, item->info1.c_str(), COL_MENUFOOT_INFO, 0, true);
				}

				// option_info2
				int l_ow2 = 0;
				if(!item->option_info2.empty())
				{
					l_ow2 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth(item->option_info2.c_str());

					g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(x + width - BORDER_RIGHT - l_ow2, y + height + cFrameFootInfo.iHeight/2 + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight(), width - BORDER_LEFT - BORDER_RIGHT - l_ow2, item->option_info2.c_str(), COL_MENUFOOT_INFO, 0, true);
				}

				// info2
				int l_w2 = 0;
				if(!item->info2.empty())
				{
					l_w2 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getRenderWidth(item->info2.c_str());

					if(l_w2 >= (width - BORDER_LEFT - BORDER_RIGHT - l_ow2))
						l_w2 = 0;

					g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString (x + BORDER_LEFT, y + height + cFrameFootInfo.iHeight/2 + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight(), width - BORDER_LEFT - BORDER_RIGHT - l_ow2, item->info2.c_str(), COL_MENUFOOT_INFO, 0, true); // UTF-8
				}
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
			frameBuffer->paintBoxRel(x, y + full_height - fheight, width, fheight, COL_MENUFOOT_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.Foot_gradient);

			// info icon
			int iw, ih;
			frameBuffer->getIconSize(NEUTRINO_ICON_INFO, &iw, &ih);
			frameBuffer->paintIcon(NEUTRINO_ICON_INFO, x + BORDER_LEFT, y + full_height - fheight + (fheight - ih)/2);

			// itemHelpText
			if(!item->itemHelpText.empty())
			{
				g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->RenderString(x + BORDER_LEFT + iw + ICON_OFFSET, y + full_height - fheight + (fheight - g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getHeight(), width - BORDER_LEFT - BORDER_RIGHT - iw, item->itemHelpText.c_str(), COL_MENUFOOT, 0, true); // UTF-8
			}

			// item icon
			// check for minimum hight
			if(full_height - hheight - fheight >= ITEM_ICON_H)
			{ 
				frameBuffer->getIconSize(item->itemIcon.c_str(), &iw, &ih);

				// refreshbox
				frameBuffer->paintBoxRel(x + items_width + (width - items_width - ITEM_ICON_W)/2, y + (full_height - ITEM_ICON_H)/2, ITEM_ICON_W, ITEM_ICON_H, COL_MENUCONTENTDARK_PLUS_0);

				frameBuffer->displayImage(item->itemIcon.c_str(), x + items_width + (width - items_width - ITEM_ICON_W)/2, y + (height - ITEM_ICON_H)/2, ITEM_ICON_W, ITEM_ICON_H);
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
		frameBuffer->paintBoxRel(x, y + height - fheight - (fbutton_count != 0? fheight : 0), width, fheight, backgroundColor);

		// refresh horizontal line buttom
		frameBuffer->paintHLineRel(x + BORDER_LEFT, width - BORDER_LEFT - BORDER_RIGHT, y + height - fheight - (fbutton_count != 0? fheight : 0) + 2, COL_MENUCONTENT_PLUS_5);

		if(items.size() > 0)
		{
			CMenuItem* item = items[pos];
	
			// itemName
			if(!item->itemName.empty())
			{
				g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(x + BORDER_LEFT, y + height - fheight - (fbutton_count != 0? fheight : 0) + (fheight - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE] ->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), width - BORDER_LEFT - BORDER_RIGHT, item->itemName.c_str(), COL_MENUFOOT_INFO);
			}

			// helpText
			if(!item->itemHelpText.empty())
			{
				g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(x + BORDER_LEFT, y + height - (fbutton_count != 0? fheight : 0), width - BORDER_LEFT - BORDER_RIGHT, item->itemHelpText.c_str(), COL_MENUFOOT_INFO);
			}
		}
	}
}

void ClistBoxWidget::hideItemInfo()
{
	if((widgetType == WIDGET_TYPE_STANDARD || widgetType == WIDGET_TYPE_CLASSIC) && FootInfo)
	{
		itemsLine.clear(x, y, width + CONNECTLINEBOX_WIDTH, height, cFrameFootInfo.iHeight);
	}  
}

void ClistBoxWidget::setFooterButtons(const struct button_label* _fbutton_labels, const int _fbutton_count, const int _fbutton_width)
{
	fbutton_count = _fbutton_count;
	fbutton_labels = _fbutton_labels;
	fbutton_width = (_fbutton_width == 0)? width : _fbutton_width;
}

void ClistBoxWidget::setHeaderButtons(const struct button_label* _hbutton_labels, const int _hbutton_count)
{
	hbutton_count = _hbutton_count;
	hbutton_labels = _hbutton_labels;
}

void ClistBoxWidget::addKey(neutrino_msg_t key, CMenuTarget *menue, const std::string & action)
{
	keyActionMap[key].menue = menue;
	keyActionMap[key].action = action;
}

void ClistBoxWidget::saveScreen()
{
	if(!savescreen)
		return;

	if(background)
	{
		delete[] background;
		background = NULL;
	}

	background = new fb_pixel_t[full_width*full_height];
	
	if(background)
	{
		frameBuffer->saveScreen(x, y, full_width, full_height, background);
	}
}

void ClistBoxWidget::restoreScreen()
{
	if(background) 
	{
		if(savescreen)
			frameBuffer->restoreScreen(x, y, full_width, full_height, background);
	}
}

void ClistBoxWidget::enableSaveScreen()
{
	savescreen = true;
	
	if(!savescreen && background) 
	{
		delete[] background;
		background = NULL;
	}
}

void ClistBoxWidget::hide()
{
	dprintf(DEBUG_NORMAL, "ClistBoxWidget::hide:\n");

	if( savescreen && background)
		restoreScreen();
	else
		frameBuffer->paintBackgroundBoxRel(x, y, full_width, full_height);

	hideItemInfo(); 
	
	frameBuffer->blit();

	if(textBox != NULL)
	{
		delete textBox;
		textBox = NULL;
	}
}

void ClistBoxWidget::integratePlugins(CPlugins::i_type_t integration, const unsigned int shortcut, bool enabled)
{
	unsigned int number_of_plugins = (unsigned int) g_PluginList->getNumberOfPlugins();

	std::string IconName;
	unsigned int sc = shortcut;

	for (unsigned int count = 0; count < number_of_plugins; count++)
	{
		if ((g_PluginList->getIntegration(count) == integration) && !g_PluginList->isHidden(count))
		{
			//
			IconName = NEUTRINO_ICON_MENUITEM_PLUGIN;

			std::string icon("");
			icon = g_PluginList->getIcon(count);

			if(!icon.empty())
			{
				IconName = PLUGINDIR;
				IconName += "/";
				IconName += g_PluginList->getFileName(count);
				IconName += "/";
				IconName += g_PluginList->getIcon(count);
			}

			//
			neutrino_msg_t dk = (shortcut != RC_nokey) ? CRCInput::convertDigitToKey(sc++) : RC_nokey;

			//FIXME: iconName
			CMenuForwarder *fw_plugin = new CMenuForwarder(g_PluginList->getName(count), enabled, NULL, CPluginsExec::getInstance(), to_string(count).c_str(), dk, NULL, IconName.c_str());

			fw_plugin->setHelpText(g_PluginList->getDescription(count).c_str());

			addItem(fw_plugin);
		}
	}
}

int ClistBoxWidget::exec(CMenuTarget* parent, const std::string&)
{
	int retval = menu_return::RETURN_REPAINT;

	int pos = 0;
	exit_pressed = false;
	int cnt = 0;
	current_page = 0;

	if (parent)
		parent->hide();

	//
	initFrames();
	paintHead();
	paintFoot();
	paint();

	dprintf(DEBUG_NORMAL, "ClistBoxWidget::exec: (%s)\n", l_name);

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);

	uint64_t timeoutEnd = CRCInput::calcTimeoutEnd(timeout == 0 ? 0xFFFF : timeout);

	//control loop
	do {
		g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd);
		
		int handled = false;

		dprintf(DEBUG_DEBUG, "ClistBoxWidget::exec: msg:%s\n", CRCInput::getSpecialKeyName(msg));

		if ( msg <= RC_MaxRC ) 
		{
			timeoutEnd = CRCInput::calcTimeoutEnd(timeout == 0 ? 0xFFFF : timeout);

			// keymap
			std::map<neutrino_msg_t, keyAction>::iterator it = keyActionMap.find(msg);
			
			if (it != keyActionMap.end()) 
			{
				if (it->second.menue != NULL)
				{
					int rv = it->second.menue->exec(this, it->second.action);

					switch ( rv ) 
					{
						case menu_return::RETURN_EXIT_ALL:
							retval = menu_return::RETURN_EXIT_ALL; //fall through
						case menu_return::RETURN_EXIT:
							msg = RC_timeout;
							break;
						case menu_return::RETURN_REPAINT:
							hide();
							initFrames();
							paintHead();
							paintFoot();
							paint();
							break;
					}
				}
				else
				{
					selected = -1;
					handled = true;

					break;
				}

				frameBuffer->blit();
				continue;
			}

			// direkKey
			for (unsigned int i = 0; i < items.size(); i++) 
			{
				CMenuItem * titem = items[i];
			
				if ((titem->directKey != RC_nokey) && (titem->directKey == msg)) 
				{
					if (titem->isSelectable()) 
					{
						items[selected]->paint(false);
						selected = i;

						if (selected > (int)page_start[current_page + 1] || selected < (int)page_start[current_page]) 
						{
							// different page
							paintItems();
						}

						paintItemInfo(selected);
						pos = selected;
						msg = RC_ok;
					} 
					else 
					{
						// swallow-key...
						handled = true;
					}
					break;
				}
			}
		}

		if (!handled) 
		{
			if ( (msg == NeutrinoMessages::EVT_TIMER) && (data == sec_timer_id) )
			{
				// head
				paintHead();
			} 

			switch (msg) 
			{
				case (NeutrinoMessages::EVT_TIMER):
					if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all ) 
					{
						retval = menu_return::RETURN_EXIT_ALL;
						msg = RC_timeout;
					}
					break;
					
				case (RC_page_up) :
					if(widgetType == WIDGET_TYPE_FRAME)
					{
						pos = (int) page_start[current_page + 1];
						if(pos >= (int) items.size()) 
							pos = items.size() - 1;

						selected = pos;
						paintItems();
					}
					else if(widgetType == WIDGET_TYPE_STANDARD || widgetType == WIDGET_TYPE_CLASSIC || widgetType == WIDGET_TYPE_EXTENDED)
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

					break;

				case (RC_page_down) :
					if(widgetType == WIDGET_TYPE_FRAME)
					{
						pos = (int) page_start[current_page] - 1;
			
						if(pos < 0)
							pos = 0;

						selected = pos;
						paintItems();
					}
					else if(widgetType == WIDGET_TYPE_STANDARD || widgetType == WIDGET_TYPE_CLASSIC || widgetType == WIDGET_TYPE_EXTENDED)
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

					break;
					
				case (RC_up) :
					if(widgetType == WIDGET_TYPE_FRAME)
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
					else if(widgetType == WIDGET_TYPE_STANDARD || widgetType == WIDGET_TYPE_CLASSIC || widgetType == WIDGET_TYPE_EXTENDED)
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
					break;
					
				case (RC_down) :
					if(widgetType == WIDGET_TYPE_FRAME)
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
					else if(widgetType == WIDGET_TYPE_STANDARD || widgetType == WIDGET_TYPE_CLASSIC || widgetType == WIDGET_TYPE_EXTENDED)
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
					break;

				case (RC_left):
					if(widgetType == WIDGET_TYPE_FRAME)
					{
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
									item->paint(true);
									paintItemInfo(pos);
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
					else if (widgetType == WIDGET_TYPE_EXTENDED)
					{
						textBox->scrollPageUp(1);
					}
					else if(widgetType == WIDGET_TYPE_STANDARD)
					{
						if(widgetMode == MODE_SETUP)
						{
							if(hasItem()) 
							{
								if((items[selected]->can_arrow)) 
								{
									//exec this item...
									CMenuItem * item = items[selected];
									item->msg = msg;
							
									int rv = item->exec(this);
							
									switch ( rv ) 
									{
										case menu_return::RETURN_EXIT_ALL:
											retval = menu_return::RETURN_EXIT_ALL; //fall through
									
										case menu_return::RETURN_EXIT:
											msg = RC_timeout;
											break;
									
										case menu_return::RETURN_REPAINT:
											hide();
											initFrames();
											paintHead();
											paintFoot();
											paint();
											break;	
									}
								}
								else
									msg = RC_timeout;
							} 
							else
								msg = RC_timeout;
						}
					}
					
					break;
					
				case (RC_right):
					if(widgetType == WIDGET_TYPE_FRAME)
					{
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
									item->paint(true);
									paintItemInfo(pos);
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
					else if (widgetType == WIDGET_TYPE_EXTENDED)
					{
						textBox->scrollPageDown(1);
					}
					else if(widgetType == WIDGET_TYPE_STANDARD)
					{
						if(widgetMode == MODE_SETUP)
						{
							if(hasItem()) 
							{
								if((items[selected]->can_arrow)) 
								{
									//exec this item...
									CMenuItem * item = items[selected];
									item->msg = msg;
							
									int rv = item->exec(this);
							
									switch ( rv ) 
									{
										case menu_return::RETURN_EXIT_ALL:
											retval = menu_return::RETURN_EXIT_ALL; //fall through
									
										case menu_return::RETURN_EXIT:
											msg = RC_timeout;
											break;
									
										case menu_return::RETURN_REPAINT:
											hide();
											initFrames();
											paintHead();
											paintFoot();
											paint();
											break;	
									}
								}
								else
									msg = RC_timeout;
							} 
							else
								msg = RC_timeout;
						}
					}

					break;

				case (RC_ok):
					{
						if(hasItem()) 
						{
							//exec this item...
							CMenuItem* item = items[selected];
							item->msg = msg;
							
							int rv = item->exec(this);
							
							switch ( rv ) 
							{
								case menu_return::RETURN_EXIT_ALL:
									retval = menu_return::RETURN_EXIT_ALL;
									
								case menu_return::RETURN_EXIT:
									msg = RC_timeout;
									break;
									
								case menu_return::RETURN_REPAINT:
									hide();
									initFrames();
									paintHead();
									paintFoot();
									paint();
									break;
							}
						} 
						else
							msg = RC_timeout;
					}
					break;
				//
				case (RC_setup):
					dprintf(DEBUG_NORMAL, "ClistBoxWidget::exec: (%s) changeWidgetType\n", l_name);

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

							initFrames();
							paintHead();
							paintFoot();
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
								cnt = WIDGET_TYPE_STANDARD;
							}
					
							widgetType = widget[cnt];

							initFrames();
							paintHead();
							paintFoot();
							paint();
						}
					}
					break;

				case (RC_home):
					exit_pressed = true;
					dprintf(DEBUG_NORMAL, "ClistBoxWidget::exec: exit_pressed\n");
					msg = RC_timeout;
					selected = -1;
					break;
					
				case (RC_timeout):
					break;

				default:
					if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all ) 
					{
						retval = menu_return::RETURN_EXIT_ALL;
						msg = RC_timeout;
					}
			}

			if ( msg <= RC_MaxRC )
			{
				// recalculate timeout for RC-Tasten
				timeoutEnd = CRCInput::calcTimeoutEnd(timeout == 0 ? 0xFFFF : timeout);
			}
		}
		
		frameBuffer->blit();
	}
	while ( msg != RC_timeout );

	dprintf(DEBUG_NORMAL, "ClistBoxWidget: retval: (%d) selected:%d\n", retval, selected);
	
	hide();	

	//
	if(PaintDate)
	{
		//
		g_RCInput->killTimer(sec_timer_id);
		sec_timer_id = 0;
	}	

	// vfd
	if(!parent)
	{
		if(CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_webtv)
			CVFD::getInstance()->setMode(CVFD::MODE_WEBTV);
		else
			CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
	}

	// init items
	for (unsigned int count = 0; count < items.size(); count++) 
	{
		CMenuItem * item = items[count];
		item->init(-1, 0, 0, 0);
	}
	
	return retval;
}	



