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
	width = MENU_WIDTH;
	if(width > (g_settings.screen_EndX - g_settings.screen_StartX))
		width = (g_settings.screen_EndX - g_settings.screen_StartX);
	
	height = MENU_HEIGHT - 50;
	if((height + 50) > (g_settings.screen_EndY - g_settings.screen_StartY))
		height = (g_settings.screen_EndY - g_settings.screen_StartY) - 50; // 2*25 pixel frei
		
	theight  = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	fheight1 = g_Font[SNeutrinoSettings::FONT_TYPE_GAMELIST_ITEMLARGE]->getHeight(); // name
	fheight2 = g_Font[SNeutrinoSettings::FONT_TYPE_GAMELIST_ITEMSMALL]->getHeight(); // desc
	fheight = fheight1 + fheight2 + 2; // name + desc + border
	
	int icon_w, icon_h;
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_RED, &icon_w, &icon_h);
	footheight = std::max(icon_h, theight);
	
	listmaxshow = (height - theight - footheight)/fheight;
	// recalculate height
	height = theight + listmaxshow*fheight + footheight;
	
	//x = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - width) / 2);
	//y = frameBuffer->getScreenY() + (frameBuffer->getScreenHeight() - height) / 2;
	// coordinations / menu position
	if(g_settings.menu_position == SNeutrinoSettings::MENU_POSITION_CENTER)
	{
		x = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - width ) >> 1 );
		y = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - height) >> 1 );
	}
	else if(g_settings.menu_position == SNeutrinoSettings::MENU_POSITION_LEFT)
	{
		x = frameBuffer->getScreenX() + BORDER_LEFT;
		y = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - height) >> 1 );
	}
	else if(g_settings.menu_position == SNeutrinoSettings::MENU_POSITION_RIGHT)
	{
		x = frameBuffer->getScreenX() + frameBuffer->getScreenWidth() - width - BORDER_RIGHT;
		y = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - height) >> 1 );
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

		if ( ( msg == CRCInput::RC_timeout ) || ( msg == (neutrino_msg_t)g_settings.key_channelList_cancel ) )
		{
			loop = false;
		}
		else if ( msg == (neutrino_msg_t)g_settings.key_channelList_pageup )
		{
			if ((int(selected) - int(listmaxshow)) < 0)
				selected = 0;
			else
				selected -= listmaxshow;
			liststart = (selected/listmaxshow)*listmaxshow;
			paintItems();
		}
		else if ( msg == (neutrino_msg_t)g_settings.key_channelList_pagedown )
		{
			selected += listmaxshow;
			if (selected > pluginlist.size() - 1)
				selected = pluginlist.size() - 1;
			liststart = (selected/listmaxshow)*listmaxshow;
			paintItems();
		}
		else if ( msg == CRCInput::RC_up )
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
		else if ( msg == CRCInput::RC_down )
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
		else if ( msg == CRCInput::RC_ok || msg == CRCInput::RC_green)
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
		else if ( msg == CRCInput::RC_red)
		{
			// delete selected plugin
			if(pluginlist.size())
				g_PluginList->removePlugin(pluginlist[selected]->number);
			
			hide();
			goto reload;
		}
		else if ( CNeutrinoApp::getInstance()->handleMsg(msg, data) & messages_return::cancel_all )
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
	frameBuffer->paintBackgroundBoxRel(x, y, width + SCROLLBAR_WIDTH, height);	//15:sb
	frameBuffer->blit();	
}

void CPluginList::paintItem(int pos)
{
	int ypos = y + theight + pos*fheight;
	int itemheight = fheight;
	int itemwidth;

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
	
	itemwidth = width - sb_width;
	
	// itemBox
	frameBuffer->paintBoxRel(x, ypos, itemwidth, itemheight, bgcolor);
	
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
			
		frameBuffer->getIconSize(NEUTRINO_ICON_PLUGIN, &icon_w, &icon_h);
			
		if (!actplugin->icon.empty() && (!access(IconName.c_str(), F_OK)))
			frameBuffer->paintIcon(IconName.c_str(), x + 8, ypos + (itemheight - icon_h)/2 );
		else
			frameBuffer->paintIcon( NEUTRINO_ICON_PLUGIN, x + 8, ypos + (itemheight - icon_h)/2 );
		
		std::string Description;
		Description = actplugin->desc;
		if(!actplugin->version.empty())
		{
			Description += "(";
			Description += actplugin->version;
			Description += ")";
		}
		
		// name
		g_Font[SNeutrinoSettings::FONT_TYPE_GAMELIST_ITEMLARGE]->RenderString(x + 2*BORDER_LEFT + icon_w, ypos + fheight1 + 3, width - (BORDER_LEFT + BORDER_RIGHT), actplugin->name, color, 0, true); // UTF-8
		
		// desc
		g_Font[SNeutrinoSettings::FONT_TYPE_GAMELIST_ITEMSMALL]->RenderString(x + 2*BORDER_LEFT + icon_w, ypos + fheight, width - (BORDER_LEFT + BORDER_RIGHT), Description, color, 0, true); // UTF-8
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
	int iw, ih;
	
	// head
	frameBuffer->paintBoxRel(x, y, width, theight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, g_settings.menu_Head_gradient);
	
	// body
	frameBuffer->paintBoxRel(x, y + theight, width, height - theight - footheight, COL_MENUCONTENT_PLUS_0);
	
	// foot
	frameBuffer->paintBoxRel(x, y + theight + height - theight - footheight, width, footheight, COL_MENUFOOT_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.menu_Foot_gradient);
	
	// foot bottons
	int ButtonWidth = (width - (BORDER_LEFT + BORDER_RIGHT)) / 4;
	::paintButtons(frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, x + BORDER_LEFT, y + theight + height - theight - footheight, ButtonWidth, NUM_LIST_BUTTONS, CPluginListButtons, footheight);

	// title + icon
	if(pluginlisttype == CPlugins::P_TYPE_GAME)
	{
		frameBuffer->getIconSize(NEUTRINO_ICON_GAMES, &iw, &ih);
		frameBuffer->paintIcon(NEUTRINO_ICON_GAMES, x + BORDER_LEFT, y + (theight - ih)/2);

		int neededWidth = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getRenderWidth(g_Locale->getText(name), true); // UTF-8
		int stringstartposX = x + (width >> 1) - (neededWidth >> 1);

		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(stringstartposX, y + theight, width - (stringstartposX - x) , g_Locale->getText(name), COL_MENUHEAD, 0, true); // UTF-8
	} 
	else
	{
		frameBuffer->getIconSize(NEUTRINO_ICON_SHELL, &iw, &ih);
		frameBuffer->paintIcon(NEUTRINO_ICON_SHELL, x + BORDER_LEFT, y + (theight - ih)/2);

		int neededWidth = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getRenderWidth(g_Locale->getText(name), true); // UTF-8
		int stringstartposX = x +(width >> 1) - (neededWidth >> 1);

		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(stringstartposX, y + theight, width - (stringstartposX - x) , g_Locale->getText(name), COL_MENUHEAD, 0, true); // UTF-8
	}
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
	
	//scrollbar
	if(nrOfPages > 1)
	{
		frameBuffer->paintBoxRel(x + width - sb_width, y + theight, SCROLLBAR_WIDTH, height - (theight + footheight),  COL_MENUCONTENT_PLUS_1);
		frameBuffer->paintBoxRel(x + width - sb_width + 2, y + theight + 2 + (currPage - 1)*(height - theight - footheight - 4)/nrOfPages, SCROLLBAR_WIDTH - 4, (height - 2*theight - 4)/nrOfPages, COL_MENUCONTENT_PLUS_3 );
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
