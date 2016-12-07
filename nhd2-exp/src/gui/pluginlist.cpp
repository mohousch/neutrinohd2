/*
	Neutrino-GUI  -   DBoxII-Project

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

#include <plugin.h>

#include <gui/pluginlist.h>
#include <gui/widget/messagebox.h>
#include <gui/widget/icons.h>

#include <sstream>
#include <fstream>
#include <iostream>

#include <dirent.h>
#include <dlfcn.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <global.h>
#include <neutrino.h>
#include <plugins.h>
#include <driver/encoding.h>

/*zapit includes*/
#include <client/zapittools.h>

#include <system/debug.h>


extern CPlugins * g_PluginList;    /* neutrino.cpp */

CPluginList::CPluginList(const neutrino_locale_t Name, const uint32_t listtype)
{
	frameBuffer = CFrameBuffer::getInstance();
	
	name = Name;
	pluginlisttype = listtype;
	
	selected = 0;
	cFrameBox.iWidth = MENU_WIDTH;
	if(cFrameBox.iWidth > (g_settings.screen_EndX - g_settings.screen_StartX))
		cFrameBox.iWidth = (g_settings.screen_EndX - g_settings.screen_StartX);
	
	cFrameBox.iHeight = MENU_HEIGHT - 50;
	if((cFrameBox.iHeight + 50) > (g_settings.screen_EndY - g_settings.screen_StartY))
		cFrameBox.iHeight = (g_settings.screen_EndY - g_settings.screen_StartY) - 50; // 2*25 pixel frei
	
	// title height	
	titleIcon.setIcon((pluginlisttype == CPlugins::P_TYPE_GAME)? NEUTRINO_ICON_GAMES : NEUTRINO_ICON_SHELL);
	cFrameBoxTitle.iHeight  = std::max(titleIcon.iHeight, g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight()) + 6;

	// item height
	//fheight1 = g_Font[SNeutrinoSettings::FONT_TYPE_GAMELIST_ITEMLARGE]->getHeight(); // name
	//fheight2 = g_Font[SNeutrinoSettings::FONT_TYPE_GAMELIST_ITEMSMALL]->getHeight(); // desc
	//fheight = fheight1 + fheight2 + 2; // name + desc + border
	cFrameBoxItem.iHeight = g_Font[SNeutrinoSettings::FONT_TYPE_GAMELIST_ITEMLARGE]->getHeight() + g_Font[SNeutrinoSettings::FONT_TYPE_GAMELIST_ITEMSMALL]->getHeight() + 2;
	
	// foot height
	//int icon_w, icon_h;
	//frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_RED, &icon_w, &icon_h);
	footIcon.setIcon(NEUTRINO_ICON_BUTTON_RED);
	cFrameBoxFoot.iHeight = std::max(footIcon.iHeight, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight()) + 10;
	
	listmaxshow = (cFrameBox.iHeight - cFrameBoxTitle.iHeight - cFrameBoxFoot.iHeight)/cFrameBoxItem.iHeight;

	// recalculate height
	cFrameBox.iHeight = cFrameBoxTitle.iHeight + listmaxshow*cFrameBoxItem.iHeight + cFrameBoxFoot.iHeight;
	
	// coordinations / menu position
	if(g_settings.menu_position == SNeutrinoSettings::MENU_POSITION_CENTER)
	{
		cFrameBox.iX = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - cFrameBox.iWidth ) >> 1 );
		cFrameBox.iY = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - cFrameBox.iHeight) >> 1 );
	}
	else if(g_settings.menu_position == SNeutrinoSettings::MENU_POSITION_LEFT)
	{
		cFrameBox.iX = frameBuffer->getScreenX() + BORDER_LEFT;
		cFrameBox.iY = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - cFrameBox.iHeight) >> 1 );
	}
	else if(g_settings.menu_position == SNeutrinoSettings::MENU_POSITION_RIGHT)
	{
		cFrameBox.iX = frameBuffer->getScreenX() + frameBuffer->getScreenWidth() - cFrameBox.iWidth - BORDER_RIGHT;
		cFrameBox.iY = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - cFrameBox.iHeight) >> 1 );
	}
	
	liststart = 0;
}

CPluginList::~CPluginList()
{
	for(unsigned int count = 0;count < pluginlist.size();count++)
	{
		delete pluginlist[count];
	}
	pluginlist.clear();
}

int CPluginList::exec(CMenuTarget * parent, const std::string &)
{
	dprintf(DEBUG_NORMAL, "CPluginList::exec\n");

	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	int res = menu_return::RETURN_REPAINT;

	if (parent)
		parent->hide();
	
reload:	
	// clear pluginlist
	for(unsigned int count = 0; count < pluginlist.size(); count++)
	{
		delete pluginlist[count];
	}
	pluginlist.clear();

	// menu back
	pluginitem * tmp = new pluginitem();

	// refill pluginlist items
	for(unsigned int count = 0; count < (unsigned int)g_PluginList->getNumberOfPlugins(); count++)
	{
		if ((g_PluginList->getType(count) & pluginlisttype) && !g_PluginList->isHidden(count))
		{
			tmp = new pluginitem();
			tmp->number = count;
			tmp->name = g_PluginList->getName(count);
			tmp->desc = g_PluginList->getDescription(count);
			tmp->version = g_PluginList->getVersion(count);
			tmp->icon = g_PluginList->getIcon(count);
			pluginlist.push_back(tmp);
		}
	}

	paint();
	
	frameBuffer->blit();	

	unsigned long long int timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);

	bool loop = true;
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);

		if ((msg == CRCInput::RC_timeout) || (msg == CRCInput::RC_home) || (msg == CRCInput::RC_setup))
		{
			loop = false;
		}
		else if (msg == CRCInput::RC_page_up)
		{
			if ((int(selected) - int(listmaxshow)) < 0)
				selected = 0;
			else
				selected -= listmaxshow;
			liststart = (selected/listmaxshow)*listmaxshow;
			paintItems();
		}
		else if (msg == CRCInput::RC_page_down)
		{
			selected += listmaxshow;
			if (selected > pluginlist.size() - 1)
				selected = pluginlist.size() - 1;
			liststart = (selected/listmaxshow)*listmaxshow;
			paintItems();
		}
		else if (msg == CRCInput::RC_up)
		{
			int prevselected = selected;
			
			if(pluginlist.size())
			{
				if(selected == 0)
				{
					selected = pluginlist.size() - 1;
				}
				else
					selected--;
				
				paintItem(prevselected - liststart);

				unsigned int oldliststart = liststart;
				liststart = (selected/listmaxshow)*listmaxshow;

				if(oldliststart != liststart)
				{
					paintItems();
				}
				else
				{
					paintItem(selected - liststart);
				}
			}
		}
		else if (msg == CRCInput::RC_down)
		{
			int prevselected = selected;
			
			if(pluginlist.size())
			{
				selected = (selected + 1)%pluginlist.size();
				paintItem(prevselected - liststart);

				unsigned int oldliststart = liststart;
				liststart = (selected/listmaxshow)*listmaxshow;

				if(oldliststart != liststart)
				{
					paintItems();
				}
				else
				{
					paintItem(selected - liststart);
				}
			}
		}
		else if (msg == CRCInput::RC_ok || msg == CRCInput::RC_green)
		{
			if(pluginlist.size())
			{
				//exec the plugin :))
				if (pluginSelected() == close)
				{
					loop = false;
				}
			}
		}
		else if (msg == CRCInput::RC_red)
		{
			// delete selected plugin
			if(pluginlist.size())
				g_PluginList->removePlugin(pluginlist[selected]->number);
			
			hide();
			goto reload;
		}
		else if (CNeutrinoApp::getInstance()->handleMsg(msg, data) & messages_return::cancel_all)
		{
			loop = false;
			res = menu_return::RETURN_EXIT_ALL;
		}

		frameBuffer->blit();		
	}
	
	hide();
	
	return res;
}

void CPluginList::hide()
{
	frameBuffer->paintBackgroundBoxRel(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight);
	frameBuffer->blit();	
}

void CPluginList::paintItem(int pos)
{
	// Item
	cFrameBoxItem.iX = cFrameBox.iX;
	cFrameBoxItem.iY = cFrameBox.iY + cFrameBoxTitle.iHeight + pos*cFrameBoxItem.iHeight;
	//int itemheight = fheight;
	//int itemwidth;
	cFrameBoxItem.iWidth = cFrameBox.iWidth;

	uint8_t color = COL_MENUCONTENT;
	fb_pixel_t bgcolor = COL_MENUCONTENT_PLUS_0;

	if (liststart + pos == selected)
	{
		color   = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
	}
	
	sb_width = 0;
	
	if(listmaxshow <= pluginlist.size() + 1)
	{
		nrOfPages = ((pluginlist.size() - 1) / listmaxshow) + 1; 
		currPage  = (liststart/listmaxshow) + 1;
	}
	
	if(nrOfPages > 1)
		sb_width = SCROLLBAR_WIDTH;
	
	cFrameBoxItem.iWidth = cFrameBox.iWidth - sb_width;
	
	// itemBox
	frameBuffer->paintBoxRel(cFrameBoxItem.iX, cFrameBoxItem.iY, cFrameBoxItem.iWidth, cFrameBoxItem.iHeight, bgcolor);
	
	// name + desc + version + icon???
	if(liststart + pos < pluginlist.size())
	{
		pluginitem * actplugin = pluginlist[liststart + pos];
		
		// icon
		std::string IconName;
		IconName = PLUGINDIR;
		IconName += "/";
		IconName += g_PluginList->getFileName(pluginlist[liststart + pos]->number);
		IconName += "/";
		IconName += actplugin->icon;
			
		itemIcon.setIcon(NEUTRINO_ICON_PLUGIN);
			
		if (!actplugin->icon.empty() && (!access(IconName.c_str(), F_OK)))
			frameBuffer->paintIcon(IconName.c_str(), cFrameBoxItem.iX + BORDER_LEFT, cFrameBoxItem.iY + (cFrameBoxItem.iHeight - itemIcon.iHeight)/2 );
		else
			frameBuffer->paintIcon(itemIcon.iconName.c_str(), cFrameBoxItem.iX + BORDER_LEFT, cFrameBoxItem.iY + (cFrameBoxItem.iHeight - itemIcon.iHeight)/2 );
		
		std::string Description;
		Description = actplugin->desc;
		if(!actplugin->version.empty())
		{
			Description += "(";
			Description += actplugin->version;
			Description += ")";
		}
		
		// name
		g_Font[SNeutrinoSettings::FONT_TYPE_GAMELIST_ITEMLARGE]->RenderString(cFrameBoxItem.iX + BORDER_LEFT + itemIcon.iWidth + ICON_OFFSET, cFrameBoxItem.iY + g_Font[SNeutrinoSettings::FONT_TYPE_GAMELIST_ITEMLARGE]->getHeight() + 3, cFrameBoxItem.iWidth - BORDER_LEFT - BORDER_RIGHT - itemIcon.iWidth - ICON_OFFSET, actplugin->name, color, 0, true); // UTF-8
		
		// desc
		g_Font[SNeutrinoSettings::FONT_TYPE_GAMELIST_ITEMSMALL]->RenderString(cFrameBoxItem.iX + BORDER_LEFT + itemIcon.iWidth + ICON_OFFSET, cFrameBoxItem.iY + cFrameBoxItem.iHeight, cFrameBoxItem.iWidth - BORDER_LEFT - BORDER_RIGHT - itemIcon.iWidth - ICON_OFFSET, Description, color, 0, true); // UTF-8
	}
}

#define NUM_LIST_BUTTONS 2
struct button_label CPluginListButtons[NUM_LIST_BUTTONS] =
{
	{ NEUTRINO_ICON_BUTTON_RED, LOCALE_PLUGINLIST_REMOVE_PLUGIN },
	{ NEUTRINO_ICON_BUTTON_GREEN, LOCALE_PLUGINLIST_START_PLUGIN }
};

void CPluginList::paintHead()
{
	// Title
	cFrameBoxTitle.iX = cFrameBox.iX;
	cFrameBoxTitle.iY = cFrameBox.iY;
	cFrameBoxTitle.iWidth = cFrameBox.iWidth;
	
	// head
	frameBuffer->paintBoxRel(cFrameBoxTitle.iX, cFrameBoxTitle.iY, cFrameBoxTitle.iWidth, cFrameBoxTitle.iHeight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, g_settings.menu_Head_gradient);

	// title Icon
	frameBuffer->paintIcon(titleIcon.iconName.c_str(), cFrameBoxTitle.iX + BORDER_LEFT, cFrameBoxTitle.iY + (cFrameBoxTitle.iHeight - titleIcon.iHeight)/2);

	// title Text
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(cFrameBoxTitle.iX + BORDER_LEFT + titleIcon.iWidth + ICON_OFFSET, cFrameBoxTitle.iY + cFrameBoxTitle.iHeight, cFrameBoxTitle.iWidth - BORDER_LEFT - BORDER_RIGHT - titleIcon.iWidth - ICON_OFFSET, g_Locale->getText(name), COL_MENUHEAD, 0, true); // UTF-8
	
	// Body
	cFrameBoxBody.iX = cFrameBox.iX;
	cFrameBoxBody.iY = cFrameBox.iY + cFrameBoxTitle.iHeight;
	cFrameBoxBody.iWidth = cFrameBox.iWidth;
	cFrameBoxBody.iHeight = cFrameBox.iHeight - cFrameBoxTitle.iHeight - cFrameBoxFoot.iHeight;

	frameBuffer->paintBoxRel(cFrameBoxBody.iX, cFrameBoxBody.iY, cFrameBoxBody.iWidth, cFrameBoxBody.iHeight, COL_MENUCONTENT_PLUS_0);
	
	// Foot
	cFrameBoxFoot.iX = cFrameBox.iX;
	cFrameBoxFoot.iY = cFrameBox.iY + cFrameBox.iHeight - cFrameBoxFoot.iHeight;
	cFrameBoxFoot.iWidth = cFrameBox.iWidth;

	frameBuffer->paintBoxRel(cFrameBoxFoot.iX, cFrameBoxFoot.iY, cFrameBoxFoot.iWidth, cFrameBoxFoot.iHeight, COL_MENUFOOT_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.menu_Foot_gradient);
	
	// foot bottons
	int ButtonWidth = (cFrameBoxFoot.iWidth - BORDER_LEFT - BORDER_RIGHT) / 4;
	::paintButtons(frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, cFrameBoxFoot.iX + BORDER_LEFT, cFrameBoxFoot.iY, ButtonWidth, NUM_LIST_BUTTONS, CPluginListButtons, cFrameBoxFoot.iHeight);
}

void CPluginList::paint()
{
	dprintf(DEBUG_NORMAL, "CPluginList::paint\n");

	hide();
	
	liststart = (selected/listmaxshow)*listmaxshow;

	// head
	paintHead();
	
	// items
	paintItems();
}

void CPluginList::paintItems()
{
	sb_width = 0;
	
	if(listmaxshow <= pluginlist.size() + 1)
	{
		nrOfPages = ((pluginlist.size() - 1) / listmaxshow) + 1; 
		currPage  = (liststart/listmaxshow) + 1;
	}
	
	if(nrOfPages > 1)
		sb_width = SCROLLBAR_WIDTH;
	
	// ScrollBar
	cFrameBoxScrollBar.iX = cFrameBox.iX + cFrameBox.iWidth - SCROLLBAR_WIDTH;
	cFrameBoxScrollBar.iY = cFrameBox.iY + cFrameBoxTitle.iHeight;
	cFrameBoxScrollBar.iWidth = SCROLLBAR_WIDTH;
	cFrameBoxScrollBar.iHeight = cFrameBox.iHeight - cFrameBoxTitle.iHeight - cFrameBoxFoot.iHeight;	

	if(nrOfPages > 1)
	{
		frameBuffer->paintBoxRel(cFrameBoxScrollBar.iX, cFrameBoxScrollBar.iY, cFrameBoxScrollBar.iWidth, cFrameBoxScrollBar.iHeight, COL_MENUCONTENT_PLUS_1);

		// ScrollBar Slider
		frameBuffer->paintBoxRel(cFrameBoxScrollBar.iX + 2, cFrameBoxScrollBar.iY + 2 + (currPage - 1)*(cFrameBoxScrollBar.iHeight - 4)/nrOfPages, cFrameBoxScrollBar.iWidth - 4, (cFrameBoxScrollBar.iHeight - 4)/nrOfPages, COL_MENUCONTENT_PLUS_3 );
	}
	
	// items
	for(unsigned int count = 0; count < listmaxshow; count++)
	{
		paintItem(count);
	}
}

CPluginList::result_ CPluginList::pluginSelected()
{
	g_PluginList->startPlugin(pluginlist[selected]->number);
	
	paint();
	
	frameBuffer->blit();
	
	return resume;
}

CPluginChooser::CPluginChooser(const neutrino_locale_t Name, const uint32_t listtype, char* pluginname)
	: CPluginList(Name, listtype), selected_plugin(pluginname)
{
}

CPluginList::result_ CPluginChooser::pluginSelected()
{
	strcpy(selected_plugin, g_PluginList->getFileName(pluginlist[selected]->number));
	return CPluginList::close;
}
