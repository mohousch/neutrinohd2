/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: menue.cpp 2013/10/12 mohousch Exp $

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
#include <gui/widget/stringinput.h>
#include <gui/widget/icons.h>
#include <gui/widget/infobox.h>
#include <gui/widget/items2detailsline.h>
#include <gui/widget/scrollbar.h>

#include <gui/color.h>
#include <gui/pluginlist.h>

#include <driver/fontrenderer.h>
#include <driver/rcinput.h>

#include <system/debug.h>


#define ITEM_ICON_W	128	// min=100, max=128
#define ITEM_ICON_H	128	// min=100, max=128

extern CPlugins * g_PluginList;    /* neutrino.cpp */

// CMenuSelectorTarget
int CMenuSelectorTarget::exec(CMenuTarget*/*parent*/, const std::string& actionKey)
{
	dprintf(DEBUG_DEBUG, "CMenuSelectorTarget::exec:\n");

        if (actionKey != "")
                *m_select = atoi(actionKey.c_str());
        else
                *m_select = -1;
	
        return menu_return::RETURN_EXIT;
}

// CMenuItem
void CMenuItem::init(const int X, const int Y, const int DX, const int OFFX)
{
	x    = X;
	y    = Y;
	dx   = DX;
	offx = OFFX;
}

void CMenuItem::setActive(const bool Active)
{
	active = Active;
	
	if (x != -1)
		paint();
}

void CMenuItem::setHelpText(const neutrino_locale_t ItemHelpText)
{
	itemHelpText =  g_Locale->getText(ItemHelpText);
}

void CMenuItem::setHelpText(const char* const ItemHelpText)
{
	itemHelpText =  ItemHelpText;
}

void CMenuItem::setHelpText(const std::string& ItemHelpText)
{
	itemHelpText =  ItemHelpText;
}

/*
void CMenuItem::setMarked(const bool Marked)
{
	marked = Marked;

	if (x != -1)
		paint();
}
*/

// CMenuWidget
CMenuWidget::CMenuWidget()
{
        nameString = g_Locale->getText(NONEXISTANT_LOCALE);
	name = NONEXISTANT_LOCALE;
        iconfile = "";
        selected = -1;
        iconOffset = 0;
	offx = offy = 0;
	
	//
	savescreen	= false;
	background	= NULL;

	disableMenuPos = false;
}

CMenuWidget::CMenuWidget(const neutrino_locale_t Name, const std::string & Icon, const int mwidth, const int mheight)
{
	name = Name;
        nameString = g_Locale->getText(NONEXISTANT_LOCALE);

	Init(Icon, mwidth, mheight);
}

CMenuWidget::CMenuWidget(const char* Name, const std::string & Icon, const int mwidth, const int mheight)
{
	name = NONEXISTANT_LOCALE;
        nameString = Name;

	Init(Icon, mwidth, mheight);
}

void CMenuWidget::Init(const std::string & Icon, const int mwidth, const int mheight)
{
        frameBuffer = CFrameBuffer::getInstance();
        iconfile = Icon;
        selected = -1;
        width = mwidth;
	
        if(width > (int) frameBuffer->getScreenWidth())
		width = frameBuffer->getScreenWidth();
	
      	height = mheight;
        wanted_height = mheight;

        current_page = 0;
	offx = offy = 0;
	
	//
	savescreen	= false;
	background	= NULL;

	disableMenuPos = false;
}

void CMenuWidget::move(int xoff, int yoff)
{
	offx = xoff;
	offy = yoff;
}

CMenuWidget::~CMenuWidget()
{
	for(unsigned int count = 0; count < items.size(); count++) 
	{
		CMenuItem * item = items[count];
		
		delete item;
	}

	items.clear();
	page_start.clear();
}

void CMenuWidget::addItem(CMenuItem *menuItem, const bool defaultselected)
{
	if (defaultselected)
		selected = items.size();
	
	if( (menuItem->getItemType() != ITEM_TYPE_FORWARDER_EXTENDED) && (menuItem->getItemType() != ITEM_TYPE_FRAME_BOX) && (menuItem->getItemType() != ITEM_TYPE_LIST_BOX) )
		items.push_back(menuItem);
}

bool CMenuWidget::hasItem()
{
	return !items.empty();
}

int CMenuWidget::exec(CMenuTarget* parent, const std::string&)
{
	dprintf(DEBUG_DEBUG, "CMenuWidget::exec:\n");

	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	int pos = 0;
	exit_pressed = false;

	if (parent)
		parent->hide();

	paint();
	frameBuffer->blit();

	int retval = menu_return::RETURN_REPAINT;
	unsigned long long int timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);

	//control loop
	do {
		g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd);

		if ( msg <= CRCInput::RC_MaxRC ) 
		{
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);
		}
		
		int handled = false;

		for (unsigned int i = 0; i < items.size(); i++) 
		{
			CMenuItem * titem = items[i];
			
			if ((titem->directKey != CRCInput::RC_nokey) && (titem->directKey == msg)) 
			{
				if (titem->isSelectable()) 
				{
					items[selected]->paint(false);
					selected = i;

					if (selected > page_start[current_page + 1] || selected < page_start[current_page]) 
					{
						// different page
						paintItems();
					}

					paintFootInfo(selected);
					pos = selected;

					msg = CRCInput::RC_ok;
				} 
				else 
				{
					// swallow-key...
					handled = true;
				}
				break;
			}
		}

		if (!handled) 
		{
			switch (msg) 
			{
				case (NeutrinoMessages::EVT_TIMER):
					if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all ) 
					{
						retval = menu_return::RETURN_EXIT_ALL;
						msg = CRCInput::RC_timeout;
					}
					break;
					
				case (CRCInput::RC_page_up) :
				case (CRCInput::RC_page_down) :
					if(msg == CRCInput::RC_page_up) 
					{
						if(current_page) 
						{
							pos = (int) page_start[current_page] - 1;
							for (unsigned int count = pos ; count > 0; count--) 
							{
								CMenuItem * item = items[pos];
								if ( item->isSelectable() ) 
								{
									if ((pos < (int)page_start[current_page + 1]) && (pos >= (int)page_start[current_page])) 
									{
										items[selected]->paint( false );
										item->paint( true );
										paintFootInfo(pos);
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
								if ( item->isSelectable() ) 
								{
									if ((pos < (int)page_start[current_page + 1]) && (pos >= (int)page_start[current_page])) 
									{
										items[selected]->paint(false);
										item->paint(true);
										paintFootInfo(pos);
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
					else if(msg == CRCInput::RC_page_down) 
					{
						pos = (int) page_start[current_page + 1];// - 1;
						if(pos >= (int) items.size()) 
							pos = items.size()-1;
						for (unsigned int count = pos ; count < items.size(); count++) 
						{
							CMenuItem * item = items[pos];
							if ( item->isSelectable() ) 
							{
								if ((pos < (int)page_start[current_page + 1]) && (pos >= (int)page_start[current_page])) 
								{
									items[selected]->paint( false );
									item->paint( true );
									paintFootInfo(pos);
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
					
				case (CRCInput::RC_up) :
				case (CRCInput::RC_down) :
					{
						//search next / prev selectable item
						for (unsigned int count = 1; count < items.size(); count++) 
						{
							if ( msg == CRCInput::RC_up ) 
							{
								pos = selected - count;
								if ( pos < 0 )
									pos += items.size();
							}
							else if( msg == CRCInput::RC_down ) 
							{
								pos = (selected + count)%items.size();
							}

							CMenuItem * item = items[pos];

							if ( item->isSelectable() ) 
							{
								if ((pos < (int)page_start[current_page + 1]) && (pos >= (int)page_start[current_page]))
								{ 
									// Item is currently on screen
									//clear prev. selected
									items[selected]->paint( false );
									//select new
									item->paint( true );
									paintFootInfo(pos);
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
					
				case (CRCInput::RC_left):
					if(!(items[selected]->can_arrow)) 
					{
						msg = CRCInput::RC_timeout;
						break;
					}
					
				case (CRCInput::RC_right):
				case (CRCInput::RC_ok):
					{
						if(hasItem()) 
						{
							//exec this item...
							CMenuItem * item = items[selected];
							item->msg = msg;
							
							int rv = item->exec(this);
							
							switch ( rv ) 
							{
								case menu_return::RETURN_EXIT_ALL:
									retval = menu_return::RETURN_EXIT_ALL;
									
								case menu_return::RETURN_EXIT:
									msg = CRCInput::RC_timeout;
									break;
									
								case menu_return::RETURN_REPAINT:
									hide();
									paint();
									break;
							}
						} 
						else
							msg = CRCInput::RC_timeout;
					}
					break;

				case (CRCInput::RC_home):
					exit_pressed = true;
					msg = CRCInput::RC_timeout;
					break;
					
				case (CRCInput::RC_timeout):
					break;

				case (CRCInput::RC_sat):
				case (CRCInput::RC_favorites):
					g_RCInput->postMsg(msg, 0);
					
				//close any menue on setup-key
				case (CRCInput::RC_setup):
					{
						msg = CRCInput::RC_timeout;
						retval = menu_return::RETURN_EXIT_ALL;
					}
					break;

				default:
					if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all ) 
					{
						retval = menu_return::RETURN_EXIT_ALL;
						msg = CRCInput::RC_timeout;
					}
			}

			if ( msg <= CRCInput::RC_MaxRC )
			{
				// recalculate timeout f�r RC-Tasten
				timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);
			}
		}
		
		frameBuffer->blit();
	}
	while ( msg != CRCInput::RC_timeout );
	
	hide();	

	// vfd
	if(!parent)
	{
		if(CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_iptv)
			CVFD::getInstance()->setMode(CVFD::MODE_IPTV);
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

void CMenuWidget::hide()
{
	if( savescreen && background)
		restoreScreen();
	else
		frameBuffer->paintBackgroundBoxRel(x, y, full_width, full_height); 
	
	frameBuffer->blit();
}

void CMenuWidget::paint()
{
	dprintf(DEBUG_DEBUG, "CMenuWidget::paint\n");

	const char * l_name;
	
	if(name == NONEXISTANT_LOCALE)
		l_name = nameString.c_str();
	else
        	l_name = g_Locale->getText(name);	

	CVFD::getInstance()->setMode(CVFD::MODE_MENU_UTF8 );

	height = wanted_height;

	// recalculate height
	if(height > ((int)frameBuffer->getScreenHeight() - 10))
		height = frameBuffer->getScreenHeight() - 10;

	int neededWidth = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getRenderWidth(l_name, true); // UTF-8

	// recalculate width
	if (neededWidth > width - 48) 
	{
		width = neededWidth + 49;
		
		if(width > (int)frameBuffer->getScreenWidth())
			width = frameBuffer->getScreenWidth();
	}

	// head height
	int icon_head_w = 0;
	int icon_head_h = 0;
	frameBuffer->getIconSize(iconfile.c_str(), &icon_head_w, &icon_head_h);
	hheight = std::max(icon_head_h, g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight()) + 6;
	
	// foot height
	int icon_foot_w = 0;
	int icon_foot_h = 0;
	frameBuffer->getIconSize(NEUTRINO_ICON_INFO, &icon_foot_w, &icon_foot_h);
	fheight = std::max(icon_foot_h, g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight()) + 6;

	// calculate some values
	itemHeightTotal = 0;
	item_height = 0;
	heightCurrPage = 0;
	page_start.clear();
	page_start.push_back(0);
	total_pages = 1;
	//sp_height = 5;
	heightFirstPage = 0;
	
	for (unsigned int i = 0; i < items.size(); i++) 
	{
		item_height = items[i]->getHeight();
		itemHeightTotal += item_height;
		heightCurrPage += item_height;

		if( (heightCurrPage + hheight + fheight) > height)
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

	// shrink menu if less items
	if(hheight + itemHeightTotal + fheight < height)
		height = hheight + heightCurrPage + fheight;
	else 	
		height = hheight + heightFirstPage + fheight;
		
	// coordinations
	x = offx + frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - width ) >> 1 );
	y = offy + frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - height) >> 1 );

	// menu position
	if(g_settings.menu_position == SNeutrinoSettings::MENU_POSITION_CENTER && !disableMenuPos)
	{
		x = offx + frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - width ) >> 1 );
		y = offy + frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - height) >> 1 );
	}
	else if(g_settings.menu_position == SNeutrinoSettings::MENU_POSITION_LEFT && !disableMenuPos)
	{
		x = offx + frameBuffer->getScreenX() + BORDER_LEFT;
		y = offy + frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - height) >> 1 );
	}
	else if(g_settings.menu_position == SNeutrinoSettings::MENU_POSITION_RIGHT && !disableMenuPos)
	{
		x = offx + frameBuffer->getScreenX() + frameBuffer->getScreenWidth() - width - BORDER_RIGHT;
		y = offy + frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - height) >> 1 );
	}
	
	full_width = width;
	full_height = height;
	
	//
	if(savescreen) 
		saveScreen();

	// paint head
	frameBuffer->paintBoxRel(x, y, full_width, hheight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, g_settings.Head_gradient);
	
	//paint icon
	frameBuffer->paintIcon(iconfile, x + BORDER_LEFT, y + (hheight - icon_head_h)/2);
	
	// head title
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(x + BORDER_LEFT + icon_head_w + 2*ICON_OFFSET, y + (hheight - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), width - BORDER_RIGHT - BORDER_RIGHT - icon_head_w - 2*ICON_OFFSET, l_name, COL_MENUHEAD, 0, true); // UTF-8
	
	//paint foot
	frameBuffer->paintBoxRel(x, y + height - fheight, full_width, fheight, COL_MENUFOOT_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.Foot_gradient);
	
	//item_start_y
	item_start_y = y + hheight;
	
	// paint items
	paintItems();
}

// paint items
void CMenuWidget::paintItems()
{
	// items height
	items_height = height - (hheight + fheight);
	
	// items width
	sb_width = 0;
	
	if(total_pages > 1)
		sb_width = SCROLLBAR_WIDTH; 
	else
		sb_width = 0;
	
	items_width = full_width - sb_width;
	
	// item not currently on screen
	if (selected >= 0)
	{
		while(selected < (int)page_start[current_page])
			current_page--;
		
		while(selected >= (int)page_start[current_page + 1])
			current_page++;
	}
	
	// paint items background
	frameBuffer->paintBoxRel(x, item_start_y, full_width, items_height, COL_MENUCONTENT_PLUS_0);
	
	// paint right scroll bar if we have more then one page
	if(total_pages > 1)
	{
		int sbh = ((items_height - 4) / total_pages);

		//scrollbar
		frameBuffer->paintBoxRel(x + items_width, item_start_y, SCROLLBAR_WIDTH, items_height, COL_MENUCONTENT_PLUS_1);

		frameBuffer->paintBoxRel(x + items_width + 2, item_start_y + 2 + current_page * sbh, SCROLLBAR_WIDTH - 4, sbh, COL_MENUCONTENT_PLUS_3);
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

			if( (item->isSelectable()) && (selected == -1) ) 
			{
				selected = count;
			} 

			if (selected == count) 
			{
				paintFootInfo(count);
			}
			
			ypos = item->paint( selected == ((signed int) count) );
		} 
		else 
		{
			// x = -1 is a marker which prevents the item from being painted on setActive changes
			item->init(-1, 0, 0, 0);
		}	
	} 
}

void CMenuWidget::saveScreen()
{
	if(!savescreen)
		return;

	delete[] background;

	background = new fb_pixel_t[full_width*full_height];
	
	if(background)
		frameBuffer->SaveScreen(x, y, full_width, full_height, background);
}

void CMenuWidget::restoreScreen()
{
	if(background) 
	{
		if(savescreen)
			frameBuffer->RestoreScreen(x, y, full_width, full_height, background);
	}
}

void CMenuWidget::enableSaveScreen(bool enable)
{
	savescreen = enable;
	
	if(!enable && background) 
	{
		delete[] background;
		background = NULL;
	}
}

void CMenuWidget::paintFootInfo(int pos)
{
	CMenuItem* item = items[pos];

	item->getYPosition();

	//paint foot
	frameBuffer->paintBoxRel(x, y + full_height - fheight, full_width, fheight, COL_MENUFOOT_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.Foot_gradient);

	// info icon
	int iw, ih;
	frameBuffer->getIconSize(NEUTRINO_ICON_INFO, &iw, &ih);
	frameBuffer->paintIcon(NEUTRINO_ICON_INFO, x + BORDER_LEFT, y + full_height - fheight + (fheight - ih)/2);

	// itemHelpText
	if(!item->itemHelpText.empty())
	{
		g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->RenderString(x + BORDER_LEFT + iw + ICON_OFFSET, y + full_height - fheight + (fheight - g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getHeight(), full_width - BORDER_LEFT - BORDER_RIGHT - iw, item->itemHelpText.c_str(), COL_MENUFOOT, 0, true); // UTF-8
	}
}

void CMenuWidget::integratePlugins(CPlugins::i_type_t integration, const unsigned int shortcut, bool enabled, bool paintIcon)
{
	unsigned int number_of_plugins = (unsigned int) g_PluginList->getNumberOfPlugins();

	std::string IconName;

	for (unsigned int count = 0; count < number_of_plugins; count++)
	{
		if ((g_PluginList->getIntegration(count) == integration) && !g_PluginList->isHidden(count))
		{
			//
			if(IconName.empty())
				IconName = NEUTRINO_ICON_PLUGIN;

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
			CMenuForwarder *fw_plugin = new CMenuForwarder(g_PluginList->getName(count), enabled, NULL, CPluginsExec::getInstance(), to_string(count).c_str(), CRCInput::RC_nokey, paintIcon? IconName.c_str() : "");

			fw_plugin->setHelpText(g_PluginList->getDescription(count));
			addItem(fw_plugin);
		}
	}
}

// CMenuOptionChooser
CMenuOptionChooser::CMenuOptionChooser(const neutrino_locale_t OptionName, int *const OptionValue, const struct keyval *const Options, const unsigned Number_Of_Options, const bool Active, CChangeObserver * const Observ, const neutrino_msg_t DirectKey, const std::string & IconName, bool Pulldown, bool DisableMenuPos)
{
	int iconName_w = 0;
	int iconName_h = 0;
	CFrameBuffer::getInstance()->getIconSize(NEUTRINO_ICON_BUTTON_RED, &iconName_w, &iconName_h);
	height = std::max(iconName_h, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight()) + 3;
	
	optionNameString = g_Locale->getText(OptionName);
	optionName = OptionName;
	
	options = Options;
	active = Active;
	optionValue = OptionValue;
	number_of_options = Number_Of_Options;
	observ = Observ;
	directKey = DirectKey;
	iconName = IconName;
	can_arrow = true;
	
	pulldown = Pulldown;

	disableMenuPos = DisableMenuPos;

	itemHelpText = optionNameString;

	itemType = ITEM_TYPE_OPTION_CHOOSER;
}

CMenuOptionChooser::CMenuOptionChooser(const char *OptionName, int *const OptionValue, const struct keyval *const Options, const unsigned Number_Of_Options, const bool Active, CChangeObserver * const Observ, const neutrino_msg_t DirectKey, const std::string & IconName, bool Pulldown, bool DisableMenuPos)
{
	int iconName_w = 0;
	int iconName_h = 0;
	CFrameBuffer::getInstance()->getIconSize(NEUTRINO_ICON_BUTTON_RED, &iconName_w, &iconName_h);
	height = std::max(iconName_h, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight()) + 3;

	optionNameString = OptionName;
	optionName = NONEXISTANT_LOCALE;
	
	options = Options;
	active = Active;
	optionValue = OptionValue;
	number_of_options = Number_Of_Options;
	observ = Observ;
	directKey = DirectKey;
	iconName = IconName;
	can_arrow = true;
	pulldown = Pulldown;
	disableMenuPos = DisableMenuPos;

	itemHelpText = optionNameString;

	itemType = ITEM_TYPE_OPTION_CHOOSER;
}

void CMenuOptionChooser::setOptionValue(const int newvalue)
{
	*optionValue = newvalue;
}

int CMenuOptionChooser::getOptionValue(void) const
{
	return *optionValue;
}

int CMenuOptionChooser::exec(CMenuTarget* parent)
{
	dprintf(DEBUG_DEBUG, "CMenuOptionChooser::exec:\n");

	bool wantsRepaint = false;
	int ret = menu_return::RETURN_NONE;
	
	if (parent)
		parent->hide();

	// pulldown
	if( (!parent || msg == CRCInput::RC_ok) && pulldown ) 
	{
		int select = -1;
		char cnt[5];
		CMenuWidget *menu = new CMenuWidget(optionNameString.c_str(), NEUTRINO_ICON_SETTINGS);
		
		//if(parent)
		//	menu->move(20, 0);
		
		menu->enableSaveScreen(true);
		if(disableMenuPos)
			menu->disableMenuPosition();
		
		CMenuSelectorTarget *selector = new CMenuSelectorTarget(&select);

		for(unsigned int count = 0; count < number_of_options; count++) 
		{
			bool selected = false;
			const char *l_option;
			
			if (options[count].key == (*optionValue))
				selected = true;

			if(options[count].valname != 0)
				l_option = options[count].valname;
			else
				l_option = g_Locale->getText(options[count].value);
			
			sprintf(cnt, "%d", count);
			menu->addItem(new CMenuForwarder(l_option, true, NULL, selector, cnt), selected);
		}
		
		menu->exec(NULL, "");
		ret = menu_return::RETURN_REPAINT;
		
		if(select >= 0) 
			*optionValue = options[select].key;
		
		delete menu;
		delete selector;
	} 
	else 
	{
		for(unsigned int count = 0; count < number_of_options; count++) 
		{
			if (options[count].key == (*optionValue)) 
			{
				if( msg == CRCInput::RC_left ) 
				{
					if(count > 0)
						*optionValue = options[(count-1) % number_of_options].key;
					else
						*optionValue = options[number_of_options-1].key;
				} 
				else
					*optionValue = options[(count+1) % number_of_options].key;
				
				wantsRepaint = true;
				break;
			}
		}
	}
	
	if(parent)
		paint(true, true);
	
	if(observ)
		wantsRepaint = observ->changeNotify(optionName, optionValue);

	if ( wantsRepaint )
		ret = menu_return::RETURN_REPAINT;

	return ret;
}

int CMenuOptionChooser::paint(bool selected, bool AfterPulldown)
{
	dprintf(DEBUG_DEBUG, "CMenuOptionChooser::paint\n");

	CFrameBuffer * frameBuffer = CFrameBuffer::getInstance();

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
	
	// paint item
	frameBuffer->paintBoxRel(x, y, dx, height, bgcolor); //FIXME

	neutrino_locale_t option = NONEXISTANT_LOCALE;
	const char * l_option = NULL;

	for(unsigned int count = 0 ; count < number_of_options; count++) 
	{
		if (options[count].key == *optionValue) 
		{
			option = options[count].value;
			if(options[count].valname != 0)
				l_option = options[count].valname;
			else
				l_option = g_Locale->getText(option);
			break;
		}
	}

	if(l_option == NULL) 
	{
		*optionValue = options[0].key;
		option = options[0].value;
		if(options[0].valname != 0)
			l_option = options[0].valname;
		else
			l_option = g_Locale->getText(option);
	}

	// paint icon (left)
	int icon_w = 0;
	int icon_h = 0;
		
	if (!(iconName.empty()))
	{
		frameBuffer->getIconSize(iconName.c_str(), &icon_w, &icon_h);
		
		frameBuffer->paintIcon(iconName, x + BORDER_LEFT, y + ((height - icon_h)/2) );
	}
	else if (CRCInput::isNumeric(directKey))
	{
		// define icon name depends of numeric value
		char i_name[6]; // X +'\0'
		snprintf(i_name, 6, "%d", CRCInput::getNumericValue(directKey));
		i_name[5] = '\0'; // even if snprintf truncated the string, ensure termination
		iconName = i_name;
		
		if (!iconName.empty())
		{
			frameBuffer->getIconSize(iconName.c_str(), &icon_w, &icon_h);
			
			frameBuffer->paintIcon(iconName, x + BORDER_LEFT, y+ ((height - icon_h)/2) );
		}
		else
			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(x + BORDER_LEFT, y+ height, height, CRCInput::getKeyName(directKey), color, height);
        }

	int stringwidth = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(l_option, true); // UTF-8
	int stringstartposName = x + BORDER_LEFT + (icon_w? icon_w + ICON_OFFSET : 0);
	int stringstartposOption = x + dx - (stringwidth + BORDER_RIGHT); //+ offx

	// locale
	const char * l_name = optionNameString.c_str();
	
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(stringstartposName, y + (height - g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight(), dx - BORDER_RIGHT - (stringstartposName - x), l_name, color, 0, true); // UTF-8
	
	// option
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(stringstartposOption, y + (height - g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight(), dx - BORDER_LEFT - (stringstartposOption - x), l_option, color, 0, true); // UTF-8

	// vfd
	if (selected && !AfterPulldown)
	{ 
		char str[256];
		snprintf(str, 255, "%s %s", l_name, l_option);

		CVFD::getInstance()->showMenuText(0, str, -1, true);
	}

	return y + height;
}

//CMenuOptionNumberChooser
CMenuOptionNumberChooser::CMenuOptionNumberChooser(const neutrino_locale_t Name, int * const OptionValue, const bool Active, const int min_value, const int max_value, CChangeObserver * const Observ, const int print_offset, const int special_value, const neutrino_locale_t special_value_name, const char * non_localized_name)
{
	int iconName_w = 0;
	int iconName_h = 0;
	CFrameBuffer::getInstance()->getIconSize(NEUTRINO_ICON_BUTTON_RED, &iconName_w, &iconName_h);
	height = std::max(iconName_h, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight()) + 3;

	nameString  = g_Locale->getText(Name);
	name = Name;
	
	active = Active;
	optionValue = OptionValue;

	lower_bound = min_value;
	upper_bound = max_value;

	display_offset = print_offset;

	localized_value = special_value;
	localized_value_name = special_value_name;

	optionString = non_localized_name;
	can_arrow = true;
	observ = Observ;

	itemHelpText = nameString;
	itemType = ITEM_TYPE_OPTION_NUMBER_CHOOSER;
}

CMenuOptionNumberChooser::CMenuOptionNumberChooser(const char * const Name, int * const OptionValue, const bool Active, const int min_value, const int max_value, CChangeObserver * const Observ, const int print_offset, const int special_value, const neutrino_locale_t special_value_name, const char * non_localized_name)
{
	int iconName_w = 0;
	int iconName_h = 0;
	CFrameBuffer::getInstance()->getIconSize(NEUTRINO_ICON_BUTTON_RED, &iconName_w, &iconName_h);
	height = std::max(iconName_h, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight()) + 3;

	nameString  = Name;
	name = NONEXISTANT_LOCALE;
	
	active = Active;
	optionValue = OptionValue;

	lower_bound = min_value;
	upper_bound = max_value;

	display_offset = print_offset;

	localized_value = special_value;
	localized_value_name = special_value_name;

	optionString = non_localized_name;
	can_arrow = true;
	observ = Observ;

	itemHelpText = nameString;
	itemType = ITEM_TYPE_OPTION_NUMBER_CHOOSER;
}

int CMenuOptionNumberChooser::exec(CMenuTarget*)
{
	dprintf(DEBUG_DEBUG, "CMenuOptionNumberChooser::exec:\n");

	if( msg == CRCInput::RC_left ) 
	{
		if (((*optionValue) > upper_bound) || ((*optionValue) <= lower_bound))
			*optionValue = upper_bound;
		else
			(*optionValue)--;
	} 
	else 
	{
		if (((*optionValue) >= upper_bound) || ((*optionValue) < lower_bound))
			*optionValue = lower_bound;
		else
			(*optionValue)++;
	}
	
	paint(true);
	
	if(observ)
		observ->changeNotify(optionName, optionValue);

	return menu_return::RETURN_NONE;
}

int CMenuOptionNumberChooser::paint(bool selected, bool /*AfterPulldown*/)
{
	dprintf(DEBUG_DEBUG, "CMenuOptionNumberChooser::paint\n");

	CFrameBuffer * frameBuffer = CFrameBuffer::getInstance();

	uint8_t color = COL_MENUCONTENT;
	fb_pixel_t bgcolor = COL_MENUCONTENT_PLUS_0;

	if (selected)
	{
		color = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
	}
	
	if (!active)
	{
		color = COL_MENUCONTENTINACTIVE;
		bgcolor = COL_MENUCONTENTINACTIVE_PLUS_0;
	}
	
	// paint item
	frameBuffer->paintBoxRel(x, y, dx, height, bgcolor); //FIXME

	// option
	const char * l_option;
	char option_value[11];

	if ((localized_value_name == NONEXISTANT_LOCALE) || ((*optionValue) != localized_value))
	{
		sprintf(option_value, "%d", ((*optionValue) + display_offset));
		l_option = option_value;
	}
	else
		l_option = g_Locale->getText(localized_value_name);

	int stringwidth = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(l_option, true); // UTF-8
	int stringstartposName = x + BORDER_LEFT;
	int stringstartposOption = x + dx - stringwidth - BORDER_RIGHT; //+ offx

	const char * l_name;
	
	if(name == NONEXISTANT_LOCALE)
		l_name = nameString.c_str();
	else
        	l_name = g_Locale->getText(name);
	
	l_name = (optionString != NULL) ? optionString : l_name;

	// locale
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(stringstartposName, y + (height - g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight(), dx - BORDER_RIGHT - (stringstartposName - x), l_name, color, 0, true); // UTF-8
	
	// option value
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(stringstartposOption, y + (height - g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight(), dx - BORDER_LEFT - (stringstartposOption - x), l_option, color, 0, true); // UTF-8
	
	// vfd
	if(selected)
	{ 
		char str[256];
		snprintf(str, 255, "%s %s", l_name, option_value);

		CVFD::getInstance()->showMenuText(0, str, -1, true); 
	}

	return y + height;
}

// CMenuOptionStringChooser
CMenuOptionStringChooser::CMenuOptionStringChooser(const neutrino_locale_t Name, char * OptionValue, bool Active, CChangeObserver* Observ, const neutrino_msg_t DirectKey, const std::string & IconName, bool Pulldown, bool DisableMenuPos)
{
	int iconName_w = 0;
	int iconName_h = 0;
	CFrameBuffer::getInstance()->getIconSize(NEUTRINO_ICON_BUTTON_RED, &iconName_w, &iconName_h);
	height = std::max(iconName_h, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight()) + 3;

	nameString = g_Locale->getText(Name);
	name = Name;
	active = Active;
	optionValue = OptionValue;
	observ = Observ;

	directKey = DirectKey;
	iconName = IconName;
	can_arrow = true;
	
	pulldown = Pulldown;
	disableMenuPos = DisableMenuPos;

	itemHelpText = nameString;
	itemType = ITEM_TYPE_OPTION_STRING_CHOOSER;
}

CMenuOptionStringChooser::CMenuOptionStringChooser(const char * const Name, char * OptionValue, bool Active, CChangeObserver* Observ, const neutrino_msg_t DirectKey, const std::string & IconName, bool Pulldown, bool DisableMenuPos)
{
	int iconName_w = 0;
	int iconName_h = 0;
	CFrameBuffer::getInstance()->getIconSize(NEUTRINO_ICON_BUTTON_RED, &iconName_w, &iconName_h);
	height = std::max(iconName_h, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight()) + 3;

	nameString = Name;
	name = NONEXISTANT_LOCALE;
	active = Active;
	optionValue = OptionValue;
	observ = Observ;

	directKey = DirectKey;
	iconName = IconName;
	can_arrow = true;
	
	pulldown = Pulldown;
	disableMenuPos = DisableMenuPos;

	itemHelpText = nameString;
	itemType = ITEM_TYPE_OPTION_STRING_CHOOSER;
}

CMenuOptionStringChooser::~CMenuOptionStringChooser()
{
	options.clear();
}

void CMenuOptionStringChooser::addOption(const char * const value)
{
	options.push_back(std::string(value));
}

int CMenuOptionStringChooser::exec(CMenuTarget* parent)
{
	dprintf(DEBUG_DEBUG, "CMenuOptionStringChooser::exec:\n");

	bool wantsRepaint = false;
	int ret = menu_return::RETURN_NONE;

	if (parent)
		parent->hide();

	// pulldown
	if( (!parent || msg == CRCInput::RC_ok) && pulldown ) 
	{
		int select = -1;
		char cnt[5];
		
		CMenuWidget * menu = new CMenuWidget(nameString.c_str(), NEUTRINO_ICON_SETTINGS);
		
		//if(parent) 
		//	menu->move(20, 0);
		
		menu->enableSaveScreen(true);
		if(disableMenuPos)
			menu->disableMenuPosition();
		
		CMenuSelectorTarget * selector = new CMenuSelectorTarget(&select);
		
		for(unsigned int count = 0; count < options.size(); count++) 
		{
			bool selected = false;
			if (strcmp(options[count].c_str(), optionValue) == 0)
				selected = true;
			sprintf(cnt, "%d", count);
			menu->addItem(new CMenuForwarder(options[count].c_str(), true, NULL, selector, cnt), selected);
		}
		menu->exec(NULL, "");
		ret = menu_return::RETURN_REPAINT;
		
		if(select >= 0)
			strcpy(optionValue, options[select].c_str());
		delete menu;
		delete selector;
	} 
	else 
	{
		//select next value
		for(unsigned int count = 0; count < options.size(); count++) 
		{
			if (strcmp(options[count].c_str(), optionValue) == 0) 
			{
				if( msg == CRCInput::RC_left ) 
				{
					if(count > 0)
						strcpy(optionValue, options[(count - 1) % options.size()].c_str());
					else
						strcpy(optionValue, options[options.size() - 1].c_str());
				} 
				else
					strcpy(optionValue, options[(count + 1) % options.size()].c_str());
				
				wantsRepaint = true;
				break;
			}
		}
	}

	if(parent)
		paint(true, true);
	
	if(observ) 
		wantsRepaint = observ->changeNotify(name, optionValue);
	
	if (wantsRepaint)
		ret = menu_return::RETURN_REPAINT;

	return ret;
}

int CMenuOptionStringChooser::paint( bool selected, bool afterPulldown)
{
	dprintf(DEBUG_DEBUG, "CMenuOptionStringChooser::paint\n");

	CFrameBuffer * frameBuffer = CFrameBuffer::getInstance();

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
	
	// paint item
	frameBuffer->paintBoxRel(x, y, dx, height, bgcolor); //FIXME

	// paint icon
	int icon_w = 0;
	int icon_h = 0;
		
	if (!(iconName.empty()))
	{
		frameBuffer->getIconSize(iconName.c_str(), &icon_w, &icon_h);
		
		frameBuffer->paintIcon(iconName, x + BORDER_LEFT, y + ((height - icon_h)/2) );	
	}
	else if (CRCInput::isNumeric(directKey))
	{
		// define icon name depends of numeric value
		char i_name[6]; // X +'\0'
		snprintf(i_name, 6, "%d", CRCInput::getNumericValue(directKey));
		i_name[5] = '\0'; // even if snprintf truncated the string, ensure termination
		iconName = i_name;
		
		if (!iconName.empty())
		{
			frameBuffer->getIconSize(iconName.c_str(), &icon_w, &icon_h);
			
			frameBuffer->paintIcon(iconName, x + BORDER_LEFT, y+ ((height - icon_h)/2) );
		}
		else
			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(x + BORDER_LEFT, y + height, height, CRCInput::getKeyName(directKey), color, height);
        }
        
        // locale text
	const char * l_name;
	
	if(name == NONEXISTANT_LOCALE)
		l_name = nameString.c_str();
	else
        	l_name = g_Locale->getText(name);
	
	int stringstartposName = x + BORDER_LEFT + (icon_w? icon_w + ICON_OFFSET : 0);
	
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(stringstartposName, y + (height - g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight(), dx - BORDER_RIGHT - (stringstartposName - x),  l_name, color, 0, true); // UTF-8
	
	// option value
	int stringwidth = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(optionValue, true);
	int stringstartposOption = std::max(stringstartposName + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(l_name, true) + ICON_OFFSET, x + dx - stringwidth - BORDER_RIGHT); //+ offx
	
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(stringstartposOption, y + (height - g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight(), dx - BORDER_RIGHT - (stringstartposOption - x),  optionValue, color, 0, true);
	
	if (selected && !afterPulldown)
	{
		char str[256];
		snprintf(str, 255, "%s %s", l_name, optionValue);

		CVFD::getInstance()->showMenuText(0, str, -1, true);
	}

	return y + height;
}

// CMenuOptionLanguageChooser
CMenuOptionLanguageChooser::CMenuOptionLanguageChooser(char *Name, CChangeObserver *Observ, const char * const IconName)
{
	int iconName_w = 0;
	int iconName_h = 0;
	CFrameBuffer::getInstance()->getIconSize(NEUTRINO_ICON_BUTTON_RED, &iconName_w, &iconName_h);
	height = std::max(iconName_h, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight()) + 3;
	
	optionValue = Name;
	observ = Observ;

	directKey = CRCInput::RC_nokey;
	iconName = IconName ? IconName : "";

	itemHelpText = Name;
	itemType = ITEM_TYPE_OPTION_LANGUAGE_CHOOSER;
}

CMenuOptionLanguageChooser::~CMenuOptionLanguageChooser()
{
	options.clear();
}

void CMenuOptionLanguageChooser::addOption(const char * const value)
{
	options.push_back(std::string(value));
}

int CMenuOptionLanguageChooser::exec(CMenuTarget*)
{
	dprintf(DEBUG_DEBUG, "CMenuOptionLanguageChooser::exec:\n");
	
	bool wantsRepaint = false;

	//select value
	for(unsigned int count = 0; count < options.size(); count++)
	{
		if (strcmp(options[count].c_str(), optionValue) == 0)
		{
			strcpy(g_settings.language, options[(count + 1) % options.size()].c_str());
			break;
		}
	}

	paint(true);
	
	if(observ)
		wantsRepaint = observ->changeNotify(LOCALE_LANGUAGESETUP_SELECT, optionValue);
	
	return menu_return::RETURN_EXIT;
	
	if ( wantsRepaint )
		return menu_return::RETURN_REPAINT;
	else
		return menu_return::RETURN_NONE;
}

int CMenuOptionLanguageChooser::paint( bool selected, bool /*AfterPulldown*/)
{
	dprintf(DEBUG_DEBUG, "CMenuOptionLanguageChooser::paint\n");

	CFrameBuffer * frameBuffer = CFrameBuffer::getInstance();
	
	uint8_t color = COL_MENUCONTENT;
	fb_pixel_t bgcolor = COL_MENUCONTENT_PLUS_0;
	
	if(selected)
	{
		color = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
	}
	
	// paint item
	frameBuffer->paintBoxRel(x, y, dx, height, bgcolor); //FIXME

	// paint icon
	int icon_w = 0;
	int icon_h = 0;
	
	if (!(iconName.empty()))
	{
		frameBuffer->getIconSize(iconName.c_str(), &icon_w, &icon_h);
		
		frameBuffer->paintIcon(iconName, x + BORDER_LEFT, y+ ((height - icon_h)/2) );
	}

	// locale
	int stringstartposOption = x + BORDER_LEFT + (icon_w? icon_w + ICON_OFFSET: 0);
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(stringstartposOption, y + (height - g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight(), dx - BORDER_RIGHT - (stringstartposOption - x), optionValue, color, 0, true); //UTF-8

	// vfd
	if (selected)
	{
		CVFD::getInstance()->showMenuText(1, optionValue);
	}

	return y + height;
}

//CMenuForwarder
CMenuForwarder::CMenuForwarder(const neutrino_locale_t Text, const bool Active, const char * const Option, CMenuTarget* Target, const char * const ActionKey, neutrino_msg_t DirectKey, const char * const IconName, const neutrino_locale_t HelpText)
{
	option = Option;
	option_string = NULL;
	text = Text;
	textString = g_Locale->getText(Text);
	active = Active;
	jumpTarget = Target;
	actionKey = ActionKey ? ActionKey : "";
	directKey = DirectKey;
	iconName = IconName ? IconName : "";

	itemHelpText = g_Locale->getText(HelpText);
	itemType = ITEM_TYPE_FORWARDER;
}

CMenuForwarder::CMenuForwarder(const neutrino_locale_t Text, const bool Active, const std::string &Option, CMenuTarget* Target, const char * const ActionKey, neutrino_msg_t DirectKey, const char * const IconName, const neutrino_locale_t HelpText)
{
	option = NULL;
	option_string = &Option;
	text = Text;
	textString = g_Locale->getText(Text);
	active = Active;
	jumpTarget = Target;
	actionKey = ActionKey ? ActionKey : "";
	directKey = DirectKey;
	iconName = IconName ? IconName : "";

	itemHelpText = g_Locale->getText(HelpText);
	itemType = ITEM_TYPE_FORWARDER;
}

CMenuForwarder::CMenuForwarder(const char * const Text, const bool Active, const char * const Option, CMenuTarget* Target, const char * const ActionKey, neutrino_msg_t DirectKey, const char * const IconName, const neutrino_locale_t HelpText)
{
	option = Option;
	option_string = NULL;
	text = NONEXISTANT_LOCALE;
	textString = Text;
	active = Active;
	jumpTarget = Target;
	actionKey = ActionKey ? ActionKey : "";
	directKey = DirectKey;
	iconName = IconName ? IconName : "";

	itemHelpText = g_Locale->getText(HelpText);
	itemType = ITEM_TYPE_FORWARDER;
}

CMenuForwarder::CMenuForwarder(const char * const Text, const bool Active, const std::string &Option, CMenuTarget* Target, const char * const ActionKey, neutrino_msg_t DirectKey, const char * const IconName, const neutrino_locale_t HelpText)
{
	option = NULL;
	option_string = &Option;
	text = NONEXISTANT_LOCALE;
	textString = Text;
	active = Active;
	jumpTarget = Target;
	actionKey = ActionKey ? ActionKey : "";
	directKey = DirectKey;
	iconName = IconName ? IconName : "";

	itemHelpText = g_Locale->getText(HelpText);
	itemType = ITEM_TYPE_FORWARDER;
}

int CMenuForwarder::getHeight(void) const
{
	int iconName_w = 0;
	int iconName_h = 0;

	CFrameBuffer::getInstance()->getIconSize(iconName.c_str(), &iconName_w, &iconName_h);
	
	return std::max(iconName_h, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight()) + 3;
}

int CMenuForwarder::getWidth(void) const
{
	int tw = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(textString);
	const char * option_text = NULL;

	if (option)
		option_text = option;
	else if (option_string)
		option_text = option_string->c_str();
	
        if (option_text != NULL)
                tw += g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(option_text, true);

	return tw;
}

int CMenuForwarder::exec(CMenuTarget* parent)
{
	dprintf(DEBUG_DEBUG, "CMenuForwarder::exec:\n");

	if(jumpTarget)
	{
		return jumpTarget->exec(parent, actionKey);
	}
	else
	{
		return menu_return::RETURN_EXIT;
	}
}

const char * CMenuForwarder::getOption(void)
{
	if (option)
		return option;
	else
		if (option_string)
			return option_string->c_str();
		else
			return NULL;
}

const char * CMenuForwarder::getName(void)
{
	const char * l_name;
	
	if(text == NONEXISTANT_LOCALE)
		l_name = textString.c_str();
	else
        	l_name = g_Locale->getText(text);
	
	return l_name;
}

int CMenuForwarder::paint(bool selected, bool /*AfterPulldown*/)
{
	dprintf(DEBUG_DEBUG, "CMenuForwarder::paint\n");

	CFrameBuffer * frameBuffer = CFrameBuffer::getInstance();

	int height = getHeight();
	const char * l_text = getName();
	
	int stringstartposX = x + (offx == 0 ? 0 : offx);

	const char * option_text = getOption();

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
	
	// paint item
	frameBuffer->paintBoxRel(x, y, dx, height, bgcolor); //FIXME
	
	// paint icon/direkt-key
	int icon_w = 0;
	int icon_h = 0;
	
	if (!iconName.empty())
	{
		//get icon size
		frameBuffer->getIconSize(iconName.c_str(), &icon_w, &icon_h);
		
		frameBuffer->paintIcon(iconName, x + BORDER_LEFT, y + (height - icon_h)/2 );
	}
	else if (CRCInput::isNumeric(directKey))
	{
		//define icon name depends of numeric value
		char i_name[6]; // X +'\0'
		snprintf(i_name, 6, "%d", CRCInput::getNumericValue(directKey));
		i_name[5] = '\0'; // even if snprintf truncated the string, ensure termination
		iconName = i_name;
		
		if (!iconName.empty())
		{
			//get icon size
			frameBuffer->getIconSize(iconName.c_str(), &icon_w, &icon_h);
			
			frameBuffer->paintIcon(iconName, x + BORDER_LEFT, y + (height - icon_h)/2);
		}
		else
			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(x + BORDER_LEFT, y + height, height, CRCInput::getKeyName(directKey), color, height);
	}
	
	// locale text
	stringstartposX = x + BORDER_LEFT + (icon_w? icon_w + ICON_OFFSET : 0);
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(stringstartposX, y + (height - g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight(), dx - BORDER_LEFT - BORDER_RIGHT - (icon_w? icon_w + ICON_OFFSET : 0), l_text, color, 0, true); // UTF-8

	//option-text
	if (option_text != NULL)
	{
		int stringwidth = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(option_text, true);
		int stringstartposOption = std::max(stringstartposX + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(l_text, true), x + dx - (stringwidth + BORDER_RIGHT)); //+ offx
		
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(stringstartposOption, y + (height - g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight(), dx - BORDER_LEFT - (stringstartposOption- x),  option_text, color, 0, true);
	}

	// vfd
	if (selected)
	{
		CVFD::getInstance()->showMenuText(0, l_text, -1, true);
	}
	
	return y + height;
}

// CMenuSeparator
CMenuSeparator::CMenuSeparator(const int Type, const neutrino_locale_t Text)
{
	directKey = CRCInput::RC_nokey;
	iconName = "";
	type = Type;
	text = Text;
	textString = g_Locale->getText(Text);

	itemType = ITEM_TYPE_SEPARATOR;
}

int CMenuSeparator::getHeight(void) const
{
	if (text == NONEXISTANT_LOCALE)
		return (g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight() >> 1);
	return  g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
}

int CMenuSeparator::getWidth(void) const
{
	return 0;
}

const char * CMenuSeparator::getString(void)
{
	return textString.c_str();
}

int CMenuSeparator::paint(bool /*selected*/, bool /*AfterPulldown*/)
{
	dprintf(DEBUG_DEBUG, "CMenuSeparator::paint\n");

	CFrameBuffer * frameBuffer = CFrameBuffer::getInstance();

	int height;
	height = getHeight();

	frameBuffer->paintBoxRel(x, y, dx, height, COL_MENUCONTENT_PLUS_0);

	// line
	if ((type & LINE))
	{
		frameBuffer->paintHLineRel(x + BORDER_LEFT, dx - BORDER_LEFT - BORDER_RIGHT, y + (height >> 1), COL_MENUCONTENTDARK_PLUS_0 );
		frameBuffer->paintHLineRel(x + BORDER_LEFT, dx - BORDER_LEFT - BORDER_RIGHT, y + (height >> 1) + 1, COL_MENUCONTENTDARK_PLUS_0 );
	}

	// string
	if ((type & STRING))
	{

		if (!textString.empty())
		{
			int stringstartposX;

			const char * l_text = getString();
			int stringwidth = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(l_text, true); // UTF-8

			// if no alignment is specified, align centered
			if (type & ALIGN_LEFT)
				stringstartposX = x + BORDER_LEFT;
			else if (type & ALIGN_RIGHT)
				stringstartposX = x + dx - stringwidth - BORDER_RIGHT;
			else // ALIGN_CENTER
				stringstartposX = x + (dx >> 1) - (stringwidth >> 1);

			frameBuffer->paintBoxRel(stringstartposX - 5, y, stringwidth + 10, height, COL_MENUCONTENT_PLUS_0);

			g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(stringstartposX, y + height, dx - (stringstartposX - x) , l_text, COL_MENUCONTENTINACTIVE, 0, true); // UTF-8
		}
	}

	return y + height;
}

bool CPINProtection::check()
{
	char cPIN[5];
	neutrino_locale_t hint = NONEXISTANT_LOCALE;
	
	do
	{
		cPIN[0] = 0;
		CPINInput * PINInput = new CPINInput(LOCALE_PINPROTECTION_HEAD, cPIN, 4, hint);
		PINInput->exec(getParent(), "");
		delete PINInput;
		hint = LOCALE_PINPROTECTION_WRONGCODE;
	} while ((strncmp(cPIN, validPIN, 4) != 0) && (cPIN[0] != 0));
	
	return ( strncmp(cPIN,validPIN, 4) == 0);
}

bool CZapProtection::check()
{
	int res;
	char cPIN[5];
	neutrino_locale_t hint2 = NONEXISTANT_LOCALE;
	
	do
	{
		cPIN[0] = 0;

		CPLPINInput* PINInput = new CPLPINInput(LOCALE_PARENTALLOCK_HEAD, cPIN, 4, hint2, fsk);

		res = PINInput->exec(getParent(), "");
		delete PINInput;

		hint2 = LOCALE_PINPROTECTION_WRONGCODE;
	} while ( (strncmp(cPIN,validPIN, 4) != 0) &&
		  (cPIN[0] != 0) &&
		  ( res == menu_return::RETURN_REPAINT ) &&
		  ( fsk >= g_settings.parentallock_lockage ) );
		  
	return ( ( strncmp(cPIN, validPIN, 4) == 0 ) ||
			 ( fsk < g_settings.parentallock_lockage ) );
}

int CLockedMenuForwarder::exec(CMenuTarget* parent)
{
	dprintf(DEBUG_DEBUG, "CLockedMenuForwarder::exec:\n");

	Parent = parent;
	
	if( (g_settings.parentallock_prompt != PARENTALLOCK_PROMPT_NEVER) || AlwaysAsk )
	{
		if (!check())
		{
			Parent = NULL;
			return menu_return::RETURN_REPAINT;
		}
	}

	Parent = NULL;
	
	return CMenuForwarder::exec(parent);
}

// menuselector
CMenuSelector::CMenuSelector(const char * OptionName, const bool Active , char * OptionValue, int* ReturnInt ,int ReturnIntValue ) : CMenuItem()
{
	height = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	optionValueString = NULL;
	optionName = OptionName;
	optionValue = OptionValue;
	active = Active;
	returnIntValue = ReturnIntValue;
	returnInt = ReturnInt;

	itemType = ITEM_TYPE_SELECTOR;
};

CMenuSelector::CMenuSelector(const char * OptionName, const bool Active , std::string& OptionValue, int* ReturnInt ,int ReturnIntValue ) : CMenuItem()
{
	height = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	optionValueString = &OptionValue;
	optionName = OptionName;
	strncpy(buffer,OptionValue.c_str(), 20);
	buffer[19] = 0;// terminate string
	optionValue = buffer;
	active = Active;
	returnIntValue = ReturnIntValue;
	returnInt = ReturnInt;

	itemType = ITEM_TYPE_SELECTOR;
};

int CMenuSelector::exec(CMenuTarget*)
{ 
	dprintf(DEBUG_DEBUG, "CMenuSelector::exec:\n");

	if(returnInt != NULL)
		*returnInt= returnIntValue;
		
	if(optionValue != NULL && optionName != NULL) 
	{
		if(optionValueString == NULL)
			strcpy(optionValue, optionName); 
		else
			*optionValueString = optionName;
	}
	
	return menu_return::RETURN_EXIT;
};

int CMenuSelector::paint(bool selected, bool /*AfterPulldown*/)
{
	dprintf(DEBUG_DEBUG, "CMenuSelector::paint\n");

	CFrameBuffer * frameBuffer = CFrameBuffer::getInstance();

	uint8_t color = COL_MENUCONTENT;
	fb_pixel_t bgcolor = COL_MENUCONTENT_PLUS_0;

	if (selected)
	{
		color = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
	}
	
	if (!active)
	{
		color = COL_MENUCONTENTINACTIVE;
		bgcolor = COL_MENUCONTENTINACTIVE_PLUS_0;
	}

	// paintItem
	frameBuffer->paintBoxRel(x, y, dx, height, bgcolor);

	int stringstartposName = x + offx + BORDER_LEFT;

	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(stringstartposName, y + height, dx - (stringstartposName - x), optionName, color, 0, true); // UTF-8

	// vfd
	if (selected)
	{
		CVFD::getInstance()->showMenuText(0, optionName, -1, true); // UTF-8
	}

	return y + height;
}

/// CMenuWidgetExtended
CMenuWidgetExtended::CMenuWidgetExtended()
{
        nameString = g_Locale->getText(NONEXISTANT_LOCALE);
	name = NONEXISTANT_LOCALE;
        iconfile = "";
        selected = -1;
        iconOffset = 0;
	offx = offy = 0;
	
	//
	savescreen	= false;
	background	= NULL;

	disableMenuPos = false;
}

CMenuWidgetExtended::CMenuWidgetExtended(const neutrino_locale_t Name, const std::string & Icon, const int mwidth, const int mheight)
{
	name = Name;
        nameString = g_Locale->getText(NONEXISTANT_LOCALE);

	Init(Icon, mwidth, mheight);
}

CMenuWidgetExtended::CMenuWidgetExtended(const char* Name, const std::string & Icon, const int mwidth, const int mheight)
{
	name = NONEXISTANT_LOCALE;
        nameString = Name;

	Init(Icon, mwidth, mheight);
}

void CMenuWidgetExtended::Init(const std::string & Icon, const int mwidth, const int mheight)
{
        frameBuffer = CFrameBuffer::getInstance();
        iconfile = Icon;
        selected = -1;
        width = mwidth;
	
        if(width > (int) frameBuffer->getScreenWidth())
		width = frameBuffer->getScreenWidth();
	
      	height = mheight;
        wanted_height = mheight;

        current_page = 0;
	offx = offy = 0;
	
	//
	savescreen	= false;
	background	= NULL;

	disableMenuPos = false;
}

void CMenuWidgetExtended::move(int xoff, int yoff)
{
	offx = xoff;
	offy = yoff;
}

CMenuWidgetExtended::~CMenuWidgetExtended()
{
	for(unsigned int count = 0; count < items.size(); count++) 
	{
		CMenuItem * item = items[count];
		
		delete item;
	}

	items.clear();
	page_start.clear();
}

void CMenuWidgetExtended::addItem(CMenuItem *menuItem, const bool defaultselected)
{
	if (defaultselected)
		selected = items.size();
	
	if(menuItem->getItemType() == ITEM_TYPE_FORWARDER_EXTENDED)
		items.push_back(menuItem);
}

bool CMenuWidgetExtended::hasItem()
{
	return !items.empty();
}

int CMenuWidgetExtended::exec(CMenuTarget* parent, const std::string&)
{
	dprintf(DEBUG_DEBUG, "CMenuWidgetExtended::exec:\n");

	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	int pos = 0;
	exit_pressed = false;

	if (parent)
		parent->hide();

	paint();
	frameBuffer->blit();

	int retval = menu_return::RETURN_REPAINT;
	unsigned long long int timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);

	//control loop
	do {
		g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd);

		if ( msg <= CRCInput::RC_MaxRC ) 
		{
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);
		}
		
		int handled = false;

		dprintf(DEBUG_NORMAL, "CMenuWidgetExtended::exec:msg: %s\n", CRCInput::getSpecialKeyName(msg));

		for (unsigned int i = 0; i < items.size(); i++) 
		{
			CMenuItem * titem = items[i];
			
			if ((titem->directKey != CRCInput::RC_nokey) && (titem->directKey == msg)) 
			{
				if (titem->isSelectable()) 
				{
					items[selected]->paint(false);
					selected = i;

					if (selected > page_start[current_page + 1] || selected < page_start[current_page]) 
					{
						// different page
						paintItems();
					}

					paintFootInfo(selected);
					pos = selected;

					msg = CRCInput::RC_ok;
				} 
				else 
				{
					// swallow-key...
					handled = true;
				}
				break;
			}
		}

		if (!handled) 
		{
			switch (msg) 
			{
				case (NeutrinoMessages::EVT_TIMER):
					if ( CNeutrinoApp::getInstance()->handleMsg(msg, data) & messages_return::cancel_all ) 
					{
						retval = menu_return::RETURN_EXIT_ALL;
						msg = CRCInput::RC_timeout;
					}
					break;
					
				case (CRCInput::RC_page_up) :
				case (CRCInput::RC_page_down) :
					if(msg == CRCInput::RC_page_up) 
					{
						if(current_page) 
						{
							pos = (int) page_start[current_page] - 1;
							for (unsigned int count = pos ; count > 0; count--) 
							{
								CMenuItem * item = items[pos];
								if ( item->isSelectable() ) 
								{
									if ((pos < (int)page_start[current_page + 1]) && (pos >= (int)page_start[current_page])) 
									{
										items[selected]->paint( false );
										item->paint( true );
										paintFootInfo(pos);
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
								if ( item->isSelectable() ) 
								{
									if ((pos < (int)page_start[current_page + 1]) && (pos >= (int)page_start[current_page])) 
									{
										items[selected]->paint( false );
										item->paint( true );
										paintFootInfo(pos);
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
					else if(msg == CRCInput::RC_page_down) 
					{
						pos = (int) page_start[current_page + 1];// - 1;
						if(pos >= (int) items.size()) 
							pos = items.size()-1;
						for (unsigned int count = pos ; count < items.size(); count++) 
						{
							CMenuItem * item = items[pos];
							if ( item->isSelectable() ) 
							{
								if ((pos < (int)page_start[current_page + 1]) && (pos >= (int)page_start[current_page])) 
								{
									items[selected]->paint( false );
									item->paint( true );
									paintFootInfo(pos);
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
					
				case (CRCInput::RC_up) :
				case (CRCInput::RC_down) :
					{
						//search next / prev selectable item
						for (unsigned int count = 1; count < items.size(); count++) 
						{
							if ( msg == CRCInput::RC_up ) 
							{
								pos = selected - count;
								if ( pos < 0 )
									pos += items.size();
							}
							else if( msg == CRCInput::RC_down ) 
							{
								pos = (selected + count)%items.size();
							}

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
									paintFootInfo(pos);
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
					
				case (CRCInput::RC_left):
					if(!(items[selected]->can_arrow)) 
					{
						msg = CRCInput::RC_timeout;
						break;
					}
					
				case (CRCInput::RC_right):
				case (CRCInput::RC_ok):
					{
						if(hasItem()) 
						{
							//exec this item...
							CMenuItem * item = items[selected];
							item->msg = msg;
							
							int rv = item->exec(this);
							
							switch ( rv ) 
							{
								case menu_return::RETURN_EXIT_ALL:
									retval = menu_return::RETURN_EXIT_ALL;
									
								case menu_return::RETURN_EXIT:
									msg = CRCInput::RC_timeout;
									break;
									
								case menu_return::RETURN_REPAINT:
									hide();
									paint();
									break;
							}
						} 
						else
							msg = CRCInput::RC_timeout;
					}
					break;

				case (CRCInput::RC_home):
					exit_pressed = true;
					msg = CRCInput::RC_timeout;
					break;
					
				case (CRCInput::RC_timeout):
					break;

				case (CRCInput::RC_sat):
				case (CRCInput::RC_favorites):
					g_RCInput->postMsg (msg, 0);
					
				//close any menue on setup-key
				case (CRCInput::RC_setup):
					{
						msg = CRCInput::RC_timeout;
						retval = menu_return::RETURN_EXIT_ALL;
					}
					break;

				default:
					if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all ) 
					{
						retval = menu_return::RETURN_EXIT_ALL;
						msg = CRCInput::RC_timeout;
					}
			}

			if ( msg <= CRCInput::RC_MaxRC )
			{
				// recalculate timeout f�r RC-Tasten
				timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);
			}
		}
		
		frameBuffer->blit();
	}
	while ( msg != CRCInput::RC_timeout );
	
	hide();	

	// vfd
	if(!parent)
	{
		if(CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_iptv)
			CVFD::getInstance()->setMode(CVFD::MODE_IPTV);
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

void CMenuWidgetExtended::hide()
{
	if( savescreen && background)
		restoreScreen();
	else
		frameBuffer->paintBackgroundBoxRel(x, y, full_width, full_height); 
	
	frameBuffer->blit();
}

void CMenuWidgetExtended::paint()
{
	dprintf(DEBUG_DEBUG, "CMenuWidgetExtended::paint\n");

	const char * l_name;
	
	if(name == NONEXISTANT_LOCALE)
		l_name = nameString.c_str();
	else
        	l_name = g_Locale->getText(name);	

	CVFD::getInstance()->setMode(CVFD::MODE_MENU_UTF8 );

	height = wanted_height;

	// recalculate height
	if(height > ((int)frameBuffer->getScreenHeight() - 10))
		height = frameBuffer->getScreenHeight() - 10;

	int neededWidth = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getRenderWidth(l_name, true); // UTF-8

	// recalculate width
	if (neededWidth > width - 48) 
	{
		width = neededWidth + 49;
		
		if(width > (int)frameBuffer->getScreenWidth())
			width = frameBuffer->getScreenWidth();
	}

	// head height
	int icon_head_w = 0;
	int icon_head_h = 0;
	frameBuffer->getIconSize(iconfile.c_str(), &icon_head_w, &icon_head_h);
	hheight = std::max(icon_head_h, g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight()) + 6;
	
	// foot height
	int icon_foot_w = 0;
	int icon_foot_h = 0;
	frameBuffer->getIconSize(NEUTRINO_ICON_INFO, &icon_foot_w, &icon_foot_h);
	fheight = std::max(icon_foot_h, g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight()) + 6;

	// calculate some values
	itemHeightTotal = 0;
	item_height = 0;
	heightCurrPage = 0;
	page_start.clear();
	page_start.push_back(0);
	total_pages = 1;
	sp_height = 5;
	heightFirstPage = 0;
	
	for (unsigned int i = 0; i < items.size(); i++) 
	{
		item_height = items[i]->getHeight();
		itemHeightTotal += item_height;
		heightCurrPage += item_height;

		if( (heightCurrPage + hheight + fheight) > height)
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

	// shrink menu if less items
	if(hheight + itemHeightTotal + fheight < height)
		height = hheight + heightCurrPage + fheight;
	else 	
		height = hheight + heightFirstPage + fheight;
	
	full_width = width;
	full_height = height + 2*sp_height;

	// coordinations
	x = offx + frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - width ) >> 1 );
	y = offy + frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - height) >> 1 );

	// menu position
	if(g_settings.menu_position == SNeutrinoSettings::MENU_POSITION_CENTER && !disableMenuPos)
	{
		x = offx + frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - width ) >> 1 );
		y = offy + frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - height) >> 1 );
	}
	else if(g_settings.menu_position == SNeutrinoSettings::MENU_POSITION_LEFT && !disableMenuPos)
	{
		x = offx + frameBuffer->getScreenX() + BORDER_LEFT;
		y = offy + frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - height) >> 1 );
	}
	else if(g_settings.menu_position == SNeutrinoSettings::MENU_POSITION_RIGHT && !disableMenuPos)
	{
		x = offx + frameBuffer->getScreenX() + frameBuffer->getScreenWidth() - width - BORDER_RIGHT;
		y = offy + frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - height) >> 1 );
	}
	
	//
	if(savescreen) 
		saveScreen();

	// paint head
	frameBuffer->paintBoxRel(x, y, full_width, hheight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, g_settings.Head_gradient);
	
	//paint icon
	frameBuffer->paintIcon(iconfile, x + BORDER_LEFT, y + (hheight - icon_head_h)/2);
	
	// head title
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(x + BORDER_LEFT + icon_head_w + 2*ICON_OFFSET, y + (hheight - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), width - BORDER_RIGHT - BORDER_RIGHT - icon_head_w - 2*ICON_OFFSET, l_name, COL_MENUHEAD, 0, true); // UTF-8
	
	// paint head separator
	frameBuffer->paintBoxRel(x, y + hheight, full_width, sp_height, COL_MENUCONTENTDARK_PLUS_0);
	
	// paint foot seperator
	frameBuffer->paintBoxRel(x, y + full_height - (fheight + sp_height), full_width, sp_height, COL_MENUCONTENTDARK_PLUS_0);
	
	//paint foot
	frameBuffer->paintBoxRel(x, y + full_height - fheight, full_width, fheight, COL_MENUFOOT_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.Foot_gradient);
	
	//item_start_y
	item_start_y = y + hheight + sp_height;
	
	// paint items
	paintItems();
}

// paint items
void CMenuWidgetExtended::paintItems()
{
	// items height
	items_height = full_height - (hheight + sp_height + sp_height + fheight);
	
	// items width
	sb_width = 0;
	
	if(total_pages > 1)
		sb_width = 5 + SCROLLBAR_WIDTH; 
	else
		sb_width = 0;
	
	items_width = ((full_width - BORDER_LEFT - BORDER_RIGHT - sb_width)/3)*2;
	
	// item not currently on screen
	if (selected >= 0)
	{
		while(selected < (int)page_start[current_page])
			current_page--;
		
		while(selected >= (int)page_start[current_page + 1])
			current_page++;
	}
	
	// paint items background
	frameBuffer->paintBoxRel(x, item_start_y, full_width, items_height, COL_MENUCONTENTDARK_PLUS_0);
	
	// paint right scroll bar if we have more then one page
	if(total_pages > 1)
	{
		int sbh = ((items_height - 4) / total_pages);

		//scrollbar
		frameBuffer->paintBoxRel(x + BORDER_LEFT + items_width + 5, item_start_y, SCROLLBAR_WIDTH, items_height, COL_MENUCONTENT_PLUS_1);

		frameBuffer->paintBoxRel(x + BORDER_LEFT + items_width + 5 + 2, item_start_y + 2 + current_page * sbh, SCROLLBAR_WIDTH - 4, sbh, COL_MENUCONTENT_PLUS_3);
	}

	// paint items
	int ypos = item_start_y;
	int xpos = x + BORDER_LEFT;
	
	for (unsigned int count = 0; count < items.size(); count++) 
	{
		CMenuItem * item = items[count];

		if ((count >= page_start[current_page]) && (count < page_start[current_page + 1])) 
		{
			item->init(xpos, ypos, items_width, iconOffset);

			if( (item->isSelectable()) && (selected == -1) ) 
			{
				selected = count;
			} 

			if (selected == count) 
			{
				paintFootInfo(count);
			}
			
			ypos = item->paint( selected == ((signed int) count) );
		} 
		else 
		{
			// x = -1 is a marker which prevents the item from being painted on setActive changes
			item->init(-1, 0, 0, 0);
		}	
	} 
}

void CMenuWidgetExtended::saveScreen()
{
	if(!savescreen)
		return;

	delete[] background;

	background = new fb_pixel_t[full_width*full_height];
	
	if(background)
		frameBuffer->SaveScreen(x, y, full_width, full_height, background);
}

void CMenuWidgetExtended::restoreScreen()
{
	if(background) 
	{
		if(savescreen)
			frameBuffer->RestoreScreen(x, y, full_width, full_height, background);
	}
}

void CMenuWidgetExtended::enableSaveScreen(bool enable)
{
	savescreen = enable;
	
	if(!enable && background) 
	{
		delete[] background;
		background = NULL;
	}
}

void CMenuWidgetExtended::paintFootInfo(int pos)
{
	CMenuItem* item = items[pos];

	item->getYPosition();

	//paint foot
	frameBuffer->paintBoxRel(x, y + full_height - fheight, full_width, fheight, COL_MENUFOOT_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.Foot_gradient);

	// info icon
	int iw, ih;
	frameBuffer->getIconSize(NEUTRINO_ICON_INFO, &iw, &ih);
	frameBuffer->paintIcon(NEUTRINO_ICON_INFO, x + BORDER_LEFT, y + full_height - fheight + (fheight - ih)/2);

	// itemHelpText
	if(!item->itemHelpText.empty())
	{
		g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->RenderString(x + BORDER_LEFT + iw + ICON_OFFSET, y + full_height - fheight + (fheight - g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getHeight(), full_width - BORDER_LEFT - BORDER_RIGHT - iw, item->itemHelpText.c_str(), COL_MENUFOOT, 0, true); // UTF-8
	}

	// itemIcon

	// check for minimum hight
	if(full_height - hheight - 2*sp_height - fheight >= ITEM_ICON_H)
	{  
		frameBuffer->getIconSize(item->itemIcon.c_str(), &iw, &ih);

		// refreshbox
		frameBuffer->paintBoxRel(x + BORDER_LEFT + items_width + (full_width - BORDER_LEFT - items_width - ITEM_ICON_W)/2, y + (full_height - ITEM_ICON_H)/2, ITEM_ICON_W, ITEM_ICON_H, COL_MENUCONTENTDARK_PLUS_0);

		// icon
		frameBuffer->paintIcon(item->itemIcon.c_str(), x + BORDER_LEFT + items_width + (full_width - BORDER_LEFT - items_width - iw)/2, y + (full_height - ih)/2, 0, true, (iw > ITEM_ICON_W? ITEM_ICON_W : iw), (ih > ITEM_ICON_H? ITEM_ICON_H : ih));
	}
}

void CMenuWidgetExtended::integratePlugins(CPlugins::i_type_t integration, const unsigned int shortcut, bool enabled)
{
	unsigned int number_of_plugins = (unsigned int) g_PluginList->getNumberOfPlugins();

	std::string IconName;
	unsigned int sc = shortcut;

	for (unsigned int count = 0; count < number_of_plugins; count++)
	{
		if ((g_PluginList->getIntegration(count) == integration) && !g_PluginList->isHidden(count))
		{
			//
			IconName = NEUTRINO_ICON_PLUGIN;

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
			neutrino_msg_t dk = (shortcut != CRCInput::RC_nokey) ? CRCInput::convertDigitToKey(sc++) : CRCInput::RC_nokey;

			//
			CMenuForwarderExtended *fw_plugin = new CMenuForwarderExtended(g_PluginList->getName(count), enabled, CPluginsExec::getInstance(), to_string(count).c_str(), dk, "", IconName.c_str());

			fw_plugin->setHelpText(g_PluginList->getDescription(count));
			addItem(fw_plugin);
		}
	}
}

// CMenuForwarderExtended
CMenuForwarderExtended::CMenuForwarderExtended(const neutrino_locale_t Text, const bool Active, CMenuTarget* Target, const char * const ActionKey, neutrino_msg_t DirectKey, const char * const IconName, const char * const ItemIcon, const neutrino_locale_t HelpText )
{
	textString = g_Locale->getText(Text);
	text = Text;
	
	active = Active;
	jumpTarget = Target;
	actionKey = ActionKey ? ActionKey : "";
	directKey = DirectKey;
	
	iconName = IconName ? IconName : "";
	
	itemIcon = ItemIcon ? ItemIcon : "";
	itemHelpText = g_Locale->getText(HelpText);
	itemType = ITEM_TYPE_FORWARDER_EXTENDED;
}

CMenuForwarderExtended::CMenuForwarderExtended(const char * const Text, const bool Active, CMenuTarget* Target, const char * const ActionKey, neutrino_msg_t DirectKey, const char * const IconName, const char * const ItemIcon, const neutrino_locale_t HelpText )
{
	textString = Text;
	text = NONEXISTANT_LOCALE;
	
	active = Active;
	jumpTarget = Target;
	actionKey = ActionKey ? ActionKey : "";
	directKey = DirectKey;
	
	iconName = IconName ? IconName : "";
	
	itemIcon = ItemIcon ? ItemIcon : "";
	itemHelpText = g_Locale->getText(HelpText);
	itemType = ITEM_TYPE_FORWARDER_EXTENDED;
}

int CMenuForwarderExtended::getHeight(void) const
{
	int iconName_w = 0;
	int iconName_h = 0;
	CFrameBuffer::getInstance()->getIconSize(NEUTRINO_ICON_BUTTON_RED, &iconName_w, &iconName_h);
	
	return std::max(iconName_h, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight()) + 3;
}

int CMenuForwarderExtended::getWidth(void) const
{
	int tw = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(textString);

	return tw;
}

int CMenuForwarderExtended::exec(CMenuTarget* parent)
{
	dprintf(DEBUG_DEBUG, "CMenuForwarderExtended::exec:\n");

	if(jumpTarget)
		return jumpTarget->exec(parent, actionKey);
	else
		return menu_return::RETURN_EXIT;
}

const char * CMenuForwarderExtended::getName(void)
{
	const char * l_name;
	
	if(text == NONEXISTANT_LOCALE)
		l_name = textString.c_str();
	else
        	l_name = g_Locale->getText(text);
	
	return l_name;
}

int CMenuForwarderExtended::paint(bool selected, bool /*AfterPulldown*/)
{
	dprintf(DEBUG_DEBUG, "CMenuForwarderExtended::paint\n");

	CFrameBuffer * frameBuffer = CFrameBuffer::getInstance();

	int height = getHeight();
	const char * l_text = getName();
	int stringstartposX = x + (offx == 0? 0: offx);	
	
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
	
	// vfd
	if (selected)
	{
		CVFD::getInstance()->showMenuText(0, l_text, -1, true);
	}
	
	// paint item
	frameBuffer->paintBoxRel(x, y, dx, height, bgcolor);

	// paint icon/direkt-key
	int icon_w = 0;
	int icon_h = 0;
		
	if (!iconName.empty())
	{
		frameBuffer->getIconSize(iconName.c_str(), &icon_w, &icon_h);
		
		frameBuffer->paintIcon(iconName, x + BORDER_LEFT, y + ((height - icon_h)/2) );
	}
	else if (CRCInput::isNumeric(directKey))
	{
		// define icon name depends of numeric value
		char i_name[6]; 							// X +'\0'
		snprintf(i_name, 6, "%d", CRCInput::getNumericValue(directKey));
		i_name[5] = '\0'; 							// even if snprintf truncated the string, ensure termination
		iconName = i_name;
		
		if (!iconName.empty())
		{
			frameBuffer->getIconSize(iconName.c_str(), &icon_w, &icon_h);
			
			frameBuffer->paintIcon(iconName, x + BORDER_LEFT, y + ((height - icon_h)/2) );
		}
		else
			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(x + BORDER_LEFT, y + height, height, CRCInput::getKeyName(directKey), color, height);
	}
	
	//local-text
	stringstartposX = x + BORDER_LEFT + (icon_w? icon_w + ICON_OFFSET : 0);
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(stringstartposX, y + (height - g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight(), dx - BORDER_RIGHT - icon_w - (stringstartposX - x), l_text, color, 0, true); // UTF-8

	// vfd
	if (selected)
	{
		CVFD::getInstance()->showMenuText(0, l_text, -1, true);
	}

	return y + height;
}

// lockedMenuForwardExtended
int CLockedMenuForwarderExtended::exec(CMenuTarget* parent)
{
	dprintf(DEBUG_DEBUG, "CLockedMenuForwarderExtended::exec\n");

	Parent = parent;
	
	if( (g_settings.parentallock_prompt != PARENTALLOCK_PROMPT_NEVER) || AlwaysAsk )
	{
		if (!check())
		{
			Parent = NULL;
			return menu_return::RETURN_REPAINT;
		}
	}

	Parent = NULL;
	
	return CMenuForwarderExtended::exec(parent);
}

/// CMenuFrameBox
CMenuFrameBox::CMenuFrameBox()
{
	nameString = g_Locale->getText(NONEXISTANT_LOCALE);
	name = NONEXISTANT_LOCALE;
        iconfile = "";

	init(iconfile);
}

CMenuFrameBox::CMenuFrameBox(const neutrino_locale_t Name, const std::string & Icon)
{
	name = Name;
        nameString = g_Locale->getText(NONEXISTANT_LOCALE);

	init(Icon);
}

CMenuFrameBox::CMenuFrameBox(const char* Name, const std::string & Icon)
{
	name = NONEXISTANT_LOCALE;
        nameString = Name;

	init(Icon);
}

CMenuFrameBox::~CMenuFrameBox()
{
	for(unsigned int count = 0; count < items.size(); count++) 
	{
		CMenuItem * item = items[count];
		
		delete item;
	}

	items.clear();
}

void CMenuFrameBox::init(const std::string & Icon)
{
	dprintf(DEBUG_NORMAL, "CMenuFrameBox::init\n");
	
	iconfile = Icon;

	initFrameBox();

	frameBuffer = CFrameBuffer::getInstance();

	timeout = 0;

	initFrames();
}

void CMenuFrameBox::initFrameBox(void)
{
	dprintf(DEBUG_NORMAL, "CMenuFrameBox::initFrameBox\n");

	selected = 0;
	oldselected = 0;

	x = 0;
	y = 0;

	itemBoxColor = COL_MENUCONTENTSELECTED_PLUS_0;
	backgroundColor = COL_BACKGROUND;

	//
	hbutton_count	= 0;
	hbutton_labels	= NULL;	

	//
	itemsPerX = 6;
	itemsPerY = 3;

	maxItemsPerPage = itemsPerX*itemsPerY;
}

void CMenuFrameBox::initFrames(void)
{
	dprintf(DEBUG_NORMAL, "CMenuFrameBox::initFrames\n");

	Box.iX = g_settings.screen_StartX + 20;
	Box.iY = g_settings.screen_StartY + 20;
	Box.iWidth = g_settings.screen_EndX - g_settings.screen_StartX - 40;
	Box.iHeight = g_settings.screen_EndY - g_settings.screen_StartY - 40;

	//
	frameBox.iX = Box.iX + 10;
	frameBox.iY = Box.iY + 30 + 10;
	frameBox.iWidth = (Box.iWidth - (BORDER_LEFT + BORDER_RIGHT))/itemsPerX;
	frameBox.iHeight = (Box.iHeight - 30 - 30 - 10 - 10)/itemsPerY;
}

void CMenuFrameBox::addItem(CMenuItem *menuItem, const bool defaultselected)
{
	if (defaultselected)
		selected = items.size();
	
	if(menuItem->getItemType() == ITEM_TYPE_FRAME_BOX)
		items.push_back(menuItem);
}

bool CMenuFrameBox::hasItem()
{
	return !items.empty();
}

void CMenuFrameBox::paintHead(void)
{
	//dprintf(DEBUG_NORMAL, "CMenuFrameBox::paintHead\n");

	int icon_w = 0;
	int icon_h = 0;
	frameBuffer->getIconSize(iconfile.c_str(), &icon_w, &icon_h);
	frameBuffer->paintIcon(iconfile, Box.iX + BORDER_LEFT, Box.iY + (30 - icon_h)/2);

	const char * l_name;
	
	if(name == NONEXISTANT_LOCALE)
		l_name = nameString.c_str();
	else
        	l_name = g_Locale->getText(name);

	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(Box.iX + BORDER_LEFT + icon_w + ICON_OFFSET, Box.iY + (30 - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), Box.iWidth - (BORDER_LEFT + BORDER_RIGHT + icon_w +  ICON_OFFSET), l_name, COL_MENUHEAD);

	// Buttons
	if (hbutton_count)
	{
		::paintHeadButtons(frameBuffer, Box.iX, Box.iY, Box.iWidth, 30, hbutton_count, hbutton_labels);
	}
}

void CMenuFrameBox::paintFoot(void)
{
	//dprintf(DEBUG_NORMAL, "CMenuFrameBox::paintFoot\n");

	// paint buttons
	int iw = 0;
	int ih = 0;
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_RIGHT, &iw, &ih);
	frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_RIGHT, Box.iX + Box.iWidth - BORDER_RIGHT - iw, Box.iY + Box.iHeight - 30 + (30 - ih)/2);
	
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_LEFT, &iw, &ih);
	frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_LEFT, Box.iX + Box.iWidth - BORDER_RIGHT - 2*iw - 2, Box.iY + Box.iHeight - 30 + (30 - ih)/2);
	
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_TOP, &iw, &ih);
	frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_TOP, Box.iX + Box.iWidth - BORDER_RIGHT - 3*iw - 2*2, Box.iY + Box.iHeight - 30 + (30 - ih)/2);
	
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_DOWN, &iw, &ih);
	frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_DOWN, Box.iX + Box.iWidth - BORDER_RIGHT - 4*iw - 3*2, Box.iY + Box.iHeight - 30 + (30 - ih)/2);
}

void CMenuFrameBox::paintBody(void)
{
	//dprintf(DEBUG_NORMAL, "CMenuFrameBox::paintBody\n");

	// paint background
	frameBuffer->paintBoxRel(Box.iX, Box.iY, Box.iWidth, Box.iHeight, backgroundColor);
	
	// paint horizontal line top
	frameBuffer->paintHLineRel(Box.iX + BORDER_LEFT, Box.iWidth - (BORDER_LEFT + BORDER_RIGHT), Box.iY + 30 + 5, COL_MENUCONTENT_PLUS_5);
	
	// paint horizontal line bottom
	frameBuffer->paintHLineRel(Box.iX + BORDER_LEFT, Box.iWidth - (BORDER_LEFT + BORDER_RIGHT), Box.iY + Box.iHeight - 30 - 5, COL_MENUCONTENT_PLUS_5);
}

void CMenuFrameBox::paintItems(int pos)
{
	//dprintf(DEBUG_NORMAL, "CMenuFrameBox::paintitems: pos:%d\n", pos);

	if(!items.size())
		return;

	if(pos < 0)
		pos = 0;

	int k = pos;
	for (unsigned int _y = 0; _y < itemsPerY; _y++)
	{
		for (unsigned int _x = 0; _x < itemsPerX; _x++)
		{
			frameBuffer->DisplayImage(items[k]->iconName, frameBox.iX + _x*frameBox.iWidth + 5, frameBox.iY + _y*frameBox.iHeight + 5, frameBox.iWidth - 10, frameBox.iHeight - 10);

			k += 1;

			if( (k == pos + maxItemsPerPage) || (k == items.size()))
				break;	
		}

		if( (k == pos + maxItemsPerPage) || (k == items.size()))
			break;	
	}
}

void CMenuFrameBox::paintItemBox(int oldposx, int oldposy, int posx, int posy)
{
	//dprintf(DEBUG_NORMAL, "CMenuFrameBox::paintItemBox:selected(%d) oldselected(%d) oldx:%d oldy:%d posx:%d posy:%d\n", selected, oldselected, oldposx, oldposy, posx, posy);

	//refresh prev item
	frameBuffer->paintBoxRel(frameBox.iX + frameBox.iWidth*oldposx, frameBox.iY + frameBox.iHeight*oldposy, frameBox.iWidth, frameBox.iHeight, backgroundColor);

	//
	if(items.size())
	{
		// prev item box
		frameBuffer->DisplayImage(items[oldselected]->iconName, frameBox.iX + oldposx*frameBox.iWidth + 5, frameBox.iY + oldposy*frameBox.iHeight + 5, frameBox.iWidth - 10, frameBox.iHeight - 10);

		// new itembox
		frameBuffer->paintBoxRel(frameBox.iX + frameBox.iWidth*posx, frameBox.iY + frameBox.iHeight*posy, frameBox.iWidth, frameBox.iHeight, itemBoxColor, RADIUS_SMALL, CORNER_BOTH);

		// new item
		frameBuffer->DisplayImage(items[selected]->iconName, frameBox.iX + posx*frameBox.iWidth + 5, frameBox.iY + posy*frameBox.iHeight + 5, frameBox.iWidth - 10, frameBox.iHeight - 10);
	}

}

void CMenuFrameBox::paintFootInfo(int pos)
{
	// refresh
	frameBuffer->paintBoxRel(Box.iX, Box.iY + Box.iHeight - 30, Box.iWidth, 30, backgroundColor);

	// paint buttons
	int iw = 0;
	int ih = 0;
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_RIGHT, &iw, &ih);
	frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_RIGHT, Box.iX + Box.iWidth - BORDER_RIGHT - iw, Box.iY + Box.iHeight - 30 + (30 - ih)/2);
	
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_LEFT, &iw, &ih);
	frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_LEFT, Box.iX + Box.iWidth - BORDER_RIGHT - 2*iw - 2, Box.iY + Box.iHeight - 30 + (30 - ih)/2);
	
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_TOP, &iw, &ih);
	frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_TOP, Box.iX + Box.iWidth - BORDER_RIGHT - 3*iw - 2*2, Box.iY + Box.iHeight - 30 + (30 - ih)/2);
	
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_DOWN, &iw, &ih);
	frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_DOWN, Box.iX + Box.iWidth - BORDER_RIGHT - 4*iw - 3*2, Box.iY + Box.iHeight - 30 + (30 - ih)/2);

	// text
	if(items.size() > 0)
	{
		CMenuItem* item = items[pos];
	
		// foot text
		if(!item->itemName.empty())
		{
			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(Box.iX + BORDER_LEFT, Box.iY + Box.iHeight - 30 + (30 - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE] ->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), Box.iWidth - BORDER_LEFT - BORDER_RIGHT - 40, item->itemName.c_str(), COL_MENUFOOT_INFO);
		}
	}
}

void CMenuFrameBox::paint(int pos)
{
	dprintf(DEBUG_NORMAL, "CMenuFrameBox::paint: (pos:%d) (oldselected:%d) (selected:%d)\n", pos, oldselected, selected);

	// body
	paintBody();

	// head
	paintHead();
	
	// foot
	paintFoot();
	
	// itembox
	paintItemBox(x, y, x, y);
	
	// items
	paintItems(pos); //pages

	// info
	paintFootInfo(selected);
}

void CMenuFrameBox::hide(void)
{
	dprintf(DEBUG_NORMAL, "CMenuFrameBox::hide\n");
	
	frameBuffer->paintBackground();
	frameBuffer->blit();
}

void CMenuFrameBox::addKey(neutrino_msg_t key, CMenuTarget *menue, const std::string & action)
{
	keyActionMap[key].menue = menue;
	keyActionMap[key].action = action;
}

void CMenuFrameBox::setHeaderButtons(const struct button_label* _hbutton_labels, const int _hbutton_count)
{
	hbutton_count = _hbutton_count;
	hbutton_labels = _hbutton_labels;
}

void CMenuFrameBox::integratePlugins(CPlugins::i_type_t integration, const unsigned int shortcut, bool enabled)
{
	unsigned int number_of_plugins = (unsigned int) g_PluginList->getNumberOfPlugins();

	std::string IconName;

	for (unsigned int count = 0; count < number_of_plugins; count++)
	{
		if ((g_PluginList->getIntegration(count) == integration) && !g_PluginList->isHidden(count))
		{
			//
			IconName = DATADIR "/neutrino/icons/" NEUTRINO_ICON_PLUGIN ".png";

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
			CMenuFrameBoxItem *fw_plugin = new CMenuFrameBoxItem(g_PluginList->getName(count), CPluginsExec::getInstance(), to_string(count).c_str(), IconName.c_str());

			fw_plugin->setHelpText(g_PluginList->getDescription(count));
			addItem(fw_plugin);
		}
	}
}

int CMenuFrameBox::exec(CMenuTarget* parent, const std::string& /*actionKey*/)
{
	int res = menu_return::RETURN_REPAINT;

	if (parent)
		parent->hide();

	// init variables
	itemsPerPage = 0;
	currentPage = 1;
	totalPages = 1;
	firstItemPos = 0; // items Page counter

	if(items.size() > maxItemsPerPage)
		itemsPerPage = maxItemsPerPage;
	else
		itemsPerPage = items.size();

	// calculate totalPages
	if(items.size() > maxItemsPerPage)
	{
		totalPages = items.size() / maxItemsPerPage;

		if( (items.size() - totalPages*maxItemsPerPage) > 0)
			totalPages = totalPages + 1;
	}

	dprintf(DEBUG_NORMAL, "CMenuFrameBox::exec: (items.size():%d) (totalPages:%d) (currentPage:%d) (itemsPerPage:%d) (selected:%d) (firstItemPos:%d)\n", items.size(), totalPages, currentPage, itemsPerPage, selected, firstItemPos);
	
	// paint first page
	paint(firstItemPos);
	
	// blit all
	frameBuffer->blit();
	
	// loop
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;
	
	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd(timeout == 0 ? 0xFFFF : timeout);

	bool loop = true;
	while (loop) 
	{
		g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd);

		std::map<neutrino_msg_t, keyAction>::iterator it = keyActionMap.find(msg);
			
		if (it != keyActionMap.end()) 
		{
			int rv = it->second.menue->exec(this, it->second.action);

			switch ( rv ) 
			{
				case menu_return::RETURN_EXIT_ALL:
					res = menu_return::RETURN_EXIT_ALL;
									
				case menu_return::RETURN_EXIT:
					loop = false;
					break;
									
				case menu_return::RETURN_REPAINT:
					paint(firstItemPos);
					break;
			}
		}

		if ((msg == CRCInput::RC_timeout ) || (msg == CRCInput::RC_home))
		{
			printf("msg: RC_home(timeout)\n");
			loop = false;
		}
		else if(msg == CRCInput::RC_ok)
		{
			printf("msg: RC_ok\n");

			if(hasItem()) 
			{
				//exec this item...
				CMenuItem * item = items[selected];
				item->msg = msg;
							
				int rv = item->exec(this);
							
				switch ( rv ) 
				{
					case menu_return::RETURN_EXIT_ALL:
						res = menu_return::RETURN_EXIT_ALL;
									
					case menu_return::RETURN_EXIT:
						loop = false;
						break;
									
					case menu_return::RETURN_REPAINT:
						paint(firstItemPos);
						break;
				}
			} 
			else
				loop = false;
		}
		else if (msg == CRCInput::RC_right)
		{
			printf("msg: RC_right\n"); //FIXME: ( x > 6 and y > 3)

			oldselected = selected;
			int oldx = x;
			int oldy = y;

			selected += 1;

			// check selected
			if(selected >= (maxItemsPerPage*(currentPage - 1)) + itemsPerPage)
			{
				selected = maxItemsPerPage*(currentPage - 1);
				x = y = 0;
			}
			else
			{
				// calculate xy
				x += 1;

				if(itemsPerPage <= itemsPerX)
				{
					if(x >= itemsPerPage)
					{
						x = 0; 
					}
				}
				else if(itemsPerPage > itemsPerX && itemsPerPage <= (itemsPerY - 1)*itemsPerX)
				{
					if(y == 1 && x >= (itemsPerPage - itemsPerX)) //FIXME: y?
					{
						x = 0;
						y = 0; 
					}
					else if(x >= itemsPerX)
					{
						x = 0;
						y += 1; // increase y
				
						if(y >= itemsPerY - 1)
							y = 0;
					}
				}
				else if(itemsPerPage > (itemsPerY - 1)*itemsPerX && itemsPerPage <= maxItemsPerPage)
				{
					if(y == (itemsPerY - 1) && x >= (itemsPerPage - (itemsPerY - 1)*itemsPerX))
					{
						x = 0;
						y = 0; 
					}
					else if(x >= itemsPerX)
					{
						x = 0;
						y += 1;
				
						if(y >= itemsPerY)
							y = 0;
					}
				}
			}

			dprintf(DEBUG_NORMAL, "CMenuFrameBox::exec: (items.size():%d) (totalPages:%d) (currentPage:%d) (itemsPerPage:%d) (selected:%d) (firstItemPos:%d)\n", items.size(), totalPages, currentPage, itemsPerPage, selected, firstItemPos);
			
			paintItemBox(oldx, oldy, x, y);
			paintFootInfo(selected);

			if(items.size())
				items[selected]->paint(true);
		}
		else if (msg == CRCInput::RC_left)
		{
			printf("msg: RC_left\n");

			oldselected = selected;
			int oldx = x;
			int oldy = y;

			selected -= 1;

			// check selected
			if(selected < (maxItemsPerPage*(currentPage - 1)))
				selected = maxItemsPerPage*(currentPage - 1);

			// sanity check
			if (selected == -1)
				selected = 0;

			// calculate xy
			x -= 1;

			if(x < 0 && y > 0)
			{
				x = itemsPerX - 1;
				y--;
				
				if(y < 0)
					y = 0;
			}
			
			// stay at first framBox
			if (x < 0)
				x = 0;

			dprintf(DEBUG_NORMAL, "CMenuFrameBox::exec: (items.size():%d) (totalPages:%d) (currentPage:%d) (itemsPerPage:%d) (selected:%d) (firstItemPos:%d)\n", items.size(), totalPages, currentPage, itemsPerPage, selected, firstItemPos);
			
			paintItemBox(oldx, oldy, x, y);
			paintFootInfo(selected);

			if(items.size())
				items[selected]->paint(true);
		}
		else if (msg == CRCInput::RC_page_up) 
		{
			oldselected = selected;

			printf("msg page_up\n");

			if(currentPage < totalPages)
			{
				currentPage += 1;
				firstItemPos += maxItemsPerPage;

				// calculate itemsPerPage
				if ( (items.size() - (currentPage - 1)*maxItemsPerPage) <= maxItemsPerPage)
					itemsPerPage = items.size() - (currentPage - 1)*maxItemsPerPage;
				else
					itemsPerPage = maxItemsPerPage;

				// recalculate firstItemPos and itemsPerPage
				if(firstItemPos > items.size())
				{
					firstItemPos -= maxItemsPerPage;
					itemsPerPage = items.size() - (currentPage -1)*maxItemsPerPage;
				}

				dprintf(DEBUG_NORMAL, "CMenuFrameBox::exec: (items.size():%d) (totalPages:%d) (currentPage:%d) (itemsPerPage:%d) (selected:%d) (firstItemPos:%d)\n", items.size(), totalPages, currentPage, itemsPerPage, selected, firstItemPos);

				// reset
				selected = firstItemPos;
				x = 0;
				y = 0;

				hide();
				paint(firstItemPos);

				if(items.size())
					items[selected]->paint(true);
			}
		}
		else if (msg == CRCInput::RC_page_down) 
		{
			printf("msg page_down\n");

			if(currentPage > 1)
			{
				currentPage -= 1;
				firstItemPos -= maxItemsPerPage;

				// calculate itemsPerPage
				itemsPerPage = maxItemsPerPage;

				if(items.size() < maxItemsPerPage)
					itemsPerPage = items.size();

				// check firstItemPos
				if (firstItemPos < 0)
					firstItemPos = 0;

				dprintf(DEBUG_NORMAL, "CMenuFrameBox::exec: (items.size():%d) (totalPages:%d) (currentPage:%d) (itemsPerPage:%d) (selected:%d) (firstItemPos:%d)\n", items.size(), totalPages, currentPage, itemsPerPage, selected, firstItemPos);

				// reset
				selected = firstItemPos;
				x = 0;
				y = 0;

				hide();
				paint(firstItemPos);

				if(items.size())
					items[selected]->paint(true);
			}
		}
		else if(msg == CRCInput::RC_down)
		{
			printf("msg: RC_down\n");

			oldselected = selected;
			int oldx = x;
			int oldy = y;

			// calculate xy
			y += 1;

			// jump to first line
			if(y >= itemsPerY)
			{
				y = 0;
			}

			selected = selected = maxItemsPerPage*(currentPage - 1) + y*itemsPerX + x;

			if(selected > (items.size() - 1))
			{
				y = 0;
				selected = selected = maxItemsPerPage*(currentPage - 1) + y*itemsPerX + x;	
			}

			dprintf(DEBUG_NORMAL, "CMenuFrameBox::exec: (items.size():%d) (totalPages:%d) (currentPage:%d) (itemsPerPage:%d) (selected:%d) (firstItemPos:%d)\n", items.size(), totalPages, currentPage, itemsPerPage, selected, firstItemPos);

			paintItemBox(oldx, oldy, x, y);
			paintFootInfo(selected);

			if(items.size())
				items[selected]->paint(true);
		}
		else if(msg == CRCInput::RC_up)
		{
			printf("msg: RC_up\n");

			oldselected = selected;
			int oldx = x;
			int oldy = y;

			y -= 1;

			// dont jump to last line
			if(y < 0)
			{
				y = 0;

				selected = maxItemsPerPage*(currentPage - 1) + x + y*itemsPerX;
				//

				/*
				y = (itemsPerY - 1);
				selected = maxItemsPerPage*(currentPage - 1) + x + y*itemsPerX;
				if(selected > (items.size() - 1))
				{
					y = 1; //FIXME: y???
					selected = selected = maxItemsPerPage*(currentPage - 1) + y*itemsPerX + x;	

					if(selected > (items.size() - 1))
					{
						y = 0;
						selected = selected = maxItemsPerPage*(currentPage - 1) + y*itemsPerX + x;	
					}
				}
				*/
			}
			else
			{
				selected = maxItemsPerPage*(currentPage - 1) + x + y*itemsPerX;
			}

			dprintf(DEBUG_NORMAL, "CMenuFrameBox::exec: (items.size():%d) (totalPages:%d) (currentPage:%d) (itemsPerPage:%d) (selected:%d) (firstItemPos:%d)\n", items.size(), totalPages, currentPage, itemsPerPage, selected, firstItemPos);

			paintItemBox(oldx, oldy, x, y);
			paintFootInfo(selected);

			if(items.size())
				items[selected]->paint(true);
		}
		else if (CNeutrinoApp::getInstance()->handleMsg(msg, data) & messages_return::cancel_all)
		{
			dprintf(DEBUG_NORMAL, "CMenuFrameBox::exec: getInstance\n");

			res = menu_return::RETURN_EXIT_ALL;
				
			loop = false;
		}

		frameBuffer->blit();
	}
	
	// hide and exit
	hide();

	// vfd
	if(!parent)
	{
		if(CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_iptv)
			CVFD::getInstance()->setMode(CVFD::MODE_IPTV);
		else
			CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
	}
	
	return res;
}

// CMenuFrameBoxItem
CMenuFrameBoxItem::CMenuFrameBoxItem(const neutrino_locale_t Text, CMenuTarget* Target, const char * const ActionKey, const char * const Icon)
{
	active = true;
	jumpTarget = Target;
	actionKey = ActionKey ? ActionKey : "";

	iconName = Icon ? Icon : "";
	itemType = ITEM_TYPE_FRAME_BOX;
	itemName = g_Locale->getText(Text);
}

CMenuFrameBoxItem::CMenuFrameBoxItem(const char * const Text, CMenuTarget* Target, const char * const ActionKey, const char * const Icon)
{
	active = true;
	jumpTarget = Target;
	actionKey = ActionKey ? ActionKey : "";
	
	iconName = Icon ? Icon : "";
	itemType = ITEM_TYPE_FRAME_BOX;
	itemName = Text;
}

int CMenuFrameBoxItem::exec(CMenuTarget* parent)
{
	dprintf(DEBUG_DEBUG, "CMenuFrameBoxItem::exec:\n");

	if(jumpTarget)
		return jumpTarget->exec(parent, actionKey);
	else
		return menu_return::RETURN_EXIT;
}

/// CMenulistBox
CMenulistBox::CMenulistBox()
{
        nameString = g_Locale->getText(NONEXISTANT_LOCALE);
	name = NONEXISTANT_LOCALE;
        iconfile = "";
        selected = -1;
        iconOffset = 0;
	offx = offy = 0;
	
	//
	savescreen	= false;
	background	= NULL;

	//disableMenuPos = false;

	//
	//
	fbutton_count	= 0;
	fbutton_labels	= NULL;

	//
	PaintDate = false;
	timestr_len = 0;

	//
	hbutton_count	= 0;
	hbutton_labels	= NULL;

	//
	FootInfo = false;
}

CMenulistBox::CMenulistBox(const neutrino_locale_t Name, const std::string & Icon, const int mwidth, const int mheight)
{
	name = Name;
        nameString = g_Locale->getText(NONEXISTANT_LOCALE);

	Init(Icon, mwidth, mheight);
}

CMenulistBox::CMenulistBox(const char* Name, const std::string & Icon, const int mwidth, const int mheight)
{
	name = NONEXISTANT_LOCALE;
        nameString = Name;

	Init(Icon, mwidth, mheight);
}

void CMenulistBox::Init(const std::string & Icon, const int mwidth, const int mheight)
{
        frameBuffer = CFrameBuffer::getInstance();
        iconfile = Icon;
        selected = -1;
        width = mwidth;
	
        if(width > (int) frameBuffer->getScreenWidth())
		width = frameBuffer->getScreenWidth();
	
      	height = mheight;
        wanted_height = mheight;

        current_page = 0;
	offx = offy = 0;
	
	//
	savescreen	= false;
	background	= NULL;

	//disableMenuPos = true;

	//
	fbutton_count	= 0;
	fbutton_labels	= NULL;

	//
	PaintDate = false;
	timestr_len = 0;

	//
	hbutton_count	= 0;
	hbutton_labels	= NULL;

	//
	FootInfo = false;

	timeout = 0;
}

void CMenulistBox::move(int xoff, int yoff)
{
	offx = xoff;
	offy = yoff;
}

CMenulistBox::~CMenulistBox()
{
	for(unsigned int count = 0; count < items.size(); count++) 
	{
		CMenuItem * item = items[count];
		
		delete item;
	}

	items.clear();
	page_start.clear();
}

void CMenulistBox::addItem(CMenuItem *menuItem, const bool defaultselected)
{
	if (defaultselected)
		selected = items.size();
	
	if(menuItem->getItemType() == ITEM_TYPE_LIST_BOX)
		items.push_back(menuItem);
}

bool CMenulistBox::hasItem()
{
	return !items.empty();
}

void CMenulistBox::initFrames()
{
	if(name == NONEXISTANT_LOCALE)
		l_name = nameString.c_str();
	else
        	l_name = g_Locale->getText(name);

	// footInfo height
	cFrameFootInfo.iHeight = (FootInfo)? 70 : 0;

	height = wanted_height;

	// recalculate height
	if(height > ((int)frameBuffer->getScreenHeight() - 10))
		height = frameBuffer->getScreenHeight() - 10;

	int neededWidth = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getRenderWidth(l_name, true); // UTF-8

	// recalculate width
	if (neededWidth > width - 48) 
	{
		width = neededWidth + 49;
		
		if(width > (int)frameBuffer->getScreenWidth())
			width = frameBuffer->getScreenWidth();
	}

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
	itemHeightTotal = 0;
	item_height = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight() + 3;
	heightCurrPage = 0;
	page_start.clear();
	page_start.push_back(0);
	total_pages = 1;
	heightFirstPage = 0;
	
	for (unsigned int i = 0; i < items.size(); i++) 
	{
		item_height = items[i]->getHeight();
		itemHeightTotal += item_height;
		heightCurrPage += item_height;

		if( (heightCurrPage + hheight + fheight) > height)
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
	listmaxshow = (height - hheight - fheight)/item_height;

	height = hheight + listmaxshow*item_height + fheight;

	//
	full_width = width;
	full_height = height + cFrameFootInfo.iHeight;
		
	// coordinations
	x = offx + frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - full_width ) >> 1 );
	y = offy + frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - full_height) >> 1 );
}

void CMenulistBox::resizeFrames()
{
	// footInfo height
	cFrameFootInfo.iHeight = footInfoHeight;

	//
	full_width = width;
	full_height = height + cFrameFootInfo.iHeight;
		
	// coordinations
	x = offx + frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - full_width ) >> 1 );
	y = offy + frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - full_height) >> 1 );
}

void CMenulistBox::paintHead()
{
	// paint head
	frameBuffer->paintBoxRel(x, y, width, hheight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, g_settings.Head_gradient);
	
	//paint icon (left)
	frameBuffer->paintIcon(iconfile, x + BORDER_LEFT, y + (hheight - icon_head_h)/2);

	// Buttons
	int iw[hbutton_count], ih[hbutton_count];
	int xstartPos = x + full_width - BORDER_RIGHT;
	int buttonWidth = 0; //FIXME

	if (hbutton_count)
	{
		for (unsigned int i = 0; i < hbutton_count; i++)
		{
			frameBuffer->getIconSize(hbutton_labels[i].button, &iw[i], &ih[i]);
			xstartPos -= (iw[i] + ICON_TO_ICON_OFFSET);
			buttonWidth += iw[i];
		}

		::paintHeadButtons(frameBuffer, x, y, full_width, hheight, hbutton_count, hbutton_labels);
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

void CMenulistBox::paintFoot()
{
	//paint foot
	frameBuffer->paintBoxRel(x, y + full_height - cFrameFootInfo.iHeight - fheight, width, fheight, COL_MENUFOOT_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.Foot_gradient);

	// Foot Buttons
	if (fbutton_count)
	{
		::paintButtons(frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, x + BORDER_LEFT, y + full_height - cFrameFootInfo.iHeight - fheight, width/fbutton_count, fbutton_count, fbutton_labels, fheight);
	}
}

void CMenulistBox::paint()
{
	dprintf(DEBUG_DEBUG, "CMenulistBox::paint:\n");

	CVFD::getInstance()->setMode(CVFD::MODE_MENU_UTF8 );
	
	// paint Items
	item_start_y = y + hheight;
	
	paintItems();
}

// paint items
void CMenulistBox::paintItems()
{
	// items height
	items_height = height - (hheight + fheight);
	
	// items width
	sb_width = 0;
	
	if(total_pages > 1)
		sb_width = SCROLLBAR_WIDTH; 
	else
		sb_width = 0;
	
	items_width = full_width - sb_width;
	
	// item not currently on screen
	if (selected >= 0)
	{
		while(selected < (int)page_start[current_page])
			current_page--;
		
		while(selected >= (int)page_start[current_page + 1])
			current_page++;
	}
	
	// paint items background
	frameBuffer->paintBoxRel(x, item_start_y, full_width, items_height, COL_MENUCONTENT_PLUS_0);
	
	// paint right scrollBar if we have more then one page
	if(total_pages > 1)
	{
		::paintScrollBar(x + full_width - SCROLLBAR_WIDTH, item_start_y, items_height, total_pages, current_page);
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
			
			if( (item->isSelectable()) && (selected == -1) ) 
			{
				selected = count;
			} 

			if (selected == count) 
			{
				paintFootInfo(count);
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

void CMenulistBox::paintFootInfo(int pos)
{
	if(FootInfo == false)
		return;

	//
	cFrameFootInfo.iX = x;
	cFrameFootInfo.iY = y + full_height - cFrameFootInfo.iHeight;
	cFrameFootInfo.iWidth = full_width;

	CMenuItem* item = items[pos];

	item->getYPosition();
	
	// detailslines
	::paintItem2DetailsLineD(x, y, full_width, full_height - cFrameFootInfo.iHeight, cFrameFootInfo.iHeight, hheight, item->getHeight(), item->getYPosition());

	// option_info1
	int l_ow1 = 0;
	if(!item->option_info1.empty())
	{
		l_ow1 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getRenderWidth(item->option_info1.c_str());

		g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString(x + full_width - BORDER_RIGHT - l_ow1, y + full_height - cFrameFootInfo.iHeight + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight(), full_width - BORDER_LEFT - BORDER_RIGHT - l_ow1, item->option_info1.c_str(), COL_MENUFOOT_INFO, 0, true);
	}

	// info1
	int l_w1 = 0;
	if(!item->info1.empty())
	{
		l_w1 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getRenderWidth(item->info1.c_str());

		g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(x + BORDER_LEFT, y + full_height - cFrameFootInfo.iHeight + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), full_width - BORDER_LEFT - BORDER_RIGHT - l_ow1, item->info1.c_str(), COL_MENUFOOT_INFO, 0, true);
	}

	// option_info2
	int l_ow2 = 0;
	if(!item->option_info2.empty())
	{
		l_ow2 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth(item->option_info2.c_str());

		g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(x + full_width - BORDER_RIGHT - l_ow2, y + full_height - cFrameFootInfo.iHeight + cFrameFootInfo.iHeight/2 + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight(), full_width - BORDER_LEFT - BORDER_RIGHT - l_ow2, item->option_info2.c_str(), COL_MENUFOOT_INFO, 0, true);
	}

	// info2
	int l_w2 = 0;
	if(!item->info2.empty())
	{
		l_w2 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getRenderWidth(item->info2.c_str());

		g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString (x + BORDER_LEFT, y + full_height - cFrameFootInfo.iHeight + cFrameFootInfo.iHeight/2 + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight(), full_width - BORDER_LEFT - BORDER_RIGHT - l_ow2, item->info2.c_str(), COL_MENUFOOT_INFO, 0, true); // UTF-8
	}
}

void CMenulistBox::hideFootInfo()
{
	//
	cFrameFootInfo.iX = x;
	cFrameFootInfo.iY = y + full_height - cFrameFootInfo.iHeight;
	cFrameFootInfo.iWidth = full_width;

	::clearItem2DetailsLine(x, y, full_width + ConnectLineBox_Width, full_height - cFrameFootInfo.iHeight, cFrameFootInfo.iHeight);  
}

void CMenulistBox::saveScreen()
{
	if(!savescreen)
		return;

	delete[] background;

	background = new fb_pixel_t[full_width*full_height];
	
	if(background)
		frameBuffer->SaveScreen(x, y, full_width, full_height, background);
}

void CMenulistBox::restoreScreen()
{
	if(background) 
	{
		if(savescreen)
			frameBuffer->RestoreScreen(x, y, full_width, full_height, background);
	}
}

void CMenulistBox::enableSaveScreen(bool enable)
{
	savescreen = enable;
	
	if(!enable && background) 
	{
		delete[] background;
		background = NULL;
	}
}

void CMenulistBox::setFooterButtons(const struct button_label* _fbutton_labels, const int _fbutton_count)
{
	fbutton_count = _fbutton_count;
	fbutton_labels = _fbutton_labels;
}

void CMenulistBox::addKey(neutrino_msg_t key, CMenuTarget *menue, const std::string & action)
{
	keyActionMap[key].menue = menue;
	keyActionMap[key].action = action;
}

void CMenulistBox::setHeaderButtons(const struct button_label* _hbutton_labels, const int _hbutton_count)
{
	hbutton_count = _hbutton_count;
	hbutton_labels = _hbutton_labels;
}

void CMenulistBox::enableFootInfo(void)
{
	FootInfo = true; 
	initFrames();
}

void CMenulistBox::setFootInfoHeight(int height)
{
	footInfoHeight = height;
	resizeFrames();
}

void CMenulistBox::hide()
{
	dprintf(DEBUG_DEBUG, "CMenulistBox::hide:\n");

	if( savescreen && background)
		restoreScreen();
	else
		frameBuffer->paintBackgroundBoxRel(x, y, full_width, full_height);

	CMenulistBox::hideFootInfo(); 
	
	frameBuffer->blit();
}

int CMenulistBox::exec(CMenuTarget* parent, const std::string&)
{
	dprintf(DEBUG_DEBUG, "CMenulistBox::exec:\n");

	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	int pos = 0;
	exit_pressed = false;

	if (parent)
		parent->hide();

	//
	if(savescreen) 
		saveScreen();

	//
	initFrames();

	paintHead();
	paint();
	paintFoot();

	frameBuffer->blit();

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);

	int retval = menu_return::RETURN_REPAINT;
	unsigned long long int timeoutEnd = CRCInput::calcTimeoutEnd(timeout == 0 ? 0xFFFF : timeout);

	//control loop
	do {
		g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd);

		if ( msg <= CRCInput::RC_MaxRC ) 
		{
			timeoutEnd = CRCInput::calcTimeoutEnd(timeout == 0 ? 0xFFFF : timeout);
		}
		
		int handled = false;

		//printf("CMenulistBox::exec: msg:%s\n", CRCInput::getSpecialKeyName(msg));

		std::map<neutrino_msg_t, keyAction>::iterator it = keyActionMap.find(msg);
			
		if (it != keyActionMap.end()) 
		{
			int rv = it->second.menue->exec(this, it->second.action);
			switch ( rv ) 
			{
				case menu_return::RETURN_EXIT_ALL:
					retval = menu_return::RETURN_EXIT_ALL;
				case menu_return::RETURN_EXIT:
					msg = CRCInput::RC_timeout;
					break;
				case menu_return::RETURN_REPAINT:
					paintHead();
					paint();
					paintFoot();
					break;
			}
			continue;
		}

		for (unsigned int i = 0; i < items.size(); i++) 
		{
			CMenuItem * titem = items[i];
			
			if ((titem->directKey != CRCInput::RC_nokey) && (titem->directKey == msg)) 
			{
				if (titem->isSelectable()) 
				{
					items[selected]->paint(false);
					selected = i;

					if (selected > page_start[current_page + 1] || selected < page_start[current_page]) 
					{
						// different page
						paintItems();
					}

					paintFootInfo(selected);
					pos = selected;
					msg = CRCInput::RC_ok;
				} 
				else 
				{
					// swallow-key...
					handled = true;
				}
				break;
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
						msg = CRCInput::RC_timeout;
					}
					break;
					
				case (CRCInput::RC_page_up) :
				case (CRCInput::RC_page_down) :
					if(msg == CRCInput::RC_page_up) 
					{
						printf("CMenulistBox::exec: msg:RC_page_up\n");
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
										items[selected]->paint(false);

										// new item
										paintFootInfo(pos);
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
										items[selected]->paint(false);

										// new item
										paintFootInfo(pos);
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
					else if(msg == CRCInput::RC_page_down) 
					{
						printf("CMenulistBox::exec: RC_page_down\n");

						pos = (int) page_start[current_page + 1];// - 1;

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
									paintFootInfo(pos);
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
					
				case (CRCInput::RC_up) :
				case (CRCInput::RC_down) :
					{
						//search next / prev selectable item
						for (unsigned int count = 1; count < items.size(); count++) 
						{
							if ( msg == CRCInput::RC_up ) 
							{
								pos = selected - count;
								if ( pos < 0 )
									pos += items.size();
							}
							else if( msg == CRCInput::RC_down ) 
							{
								pos = (selected + count)%items.size();
							}

							CMenuItem * item = items[pos];

							if ( item->isSelectable() ) 
							{
								if ((pos < (int)page_start[current_page + 1]) && (pos >= (int)page_start[current_page]))
								{ 
									// Item is currently on screen
									//clear prev. selected
									items[selected]->paint(false);
									//select new
									paintFootInfo(pos);
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
					
				case (CRCInput::RC_left):
					if(!(items[selected]->can_arrow)) 
					{
						msg = CRCInput::RC_timeout;
						break;
					}
					
				case (CRCInput::RC_right):
				case (CRCInput::RC_ok):
					{
						if(hasItem()) 
						{
							//exec this item...
							CMenuItem * item = items[selected];
							item->msg = msg;
							
							int rv = item->exec(this);
							
							switch ( rv ) 
							{
								case menu_return::RETURN_EXIT_ALL:
									retval = menu_return::RETURN_EXIT_ALL;
									
								case menu_return::RETURN_EXIT:
									msg = CRCInput::RC_timeout;
									break;
									
								case menu_return::RETURN_REPAINT:
									hide();
									paintHead();
									paint();
									paintFoot();
									break;
							}
						} 
						else
							msg = CRCInput::RC_timeout;
					}
					break;

				case (CRCInput::RC_home):
					exit_pressed = true;
					msg = CRCInput::RC_timeout;
					break;
					
				case (CRCInput::RC_timeout):
					break;

				default:
					if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all ) 
					{
						retval = menu_return::RETURN_EXIT_ALL;
						msg = CRCInput::RC_timeout;
					}
			}

			if ( msg <= CRCInput::RC_MaxRC )
			{
				// recalculate timeout for RC-Tasten
				timeoutEnd = CRCInput::calcTimeoutEnd(timeout == 0 ? 0xFFFF : timeout);
			}
		}
		
		frameBuffer->blit();
	}
	while ( msg != CRCInput::RC_timeout );
	
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
		if(CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_iptv)
			CVFD::getInstance()->setMode(CVFD::MODE_IPTV);
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

//CMenulistBoxItem
CMenulistBoxItem::CMenulistBoxItem(const neutrino_locale_t Text, const bool Active, CMenuTarget* Target, const char * const ActionKey, const char * const IconName, const int Num, const int Percent, const char* const Descr, const char* const Icon1, const char* const Icon2, const char* const OptionText1, const char* const OptionText2, const char* const Info1, const char* const OptionInfo1, const char* const Info2, const char* const OptionInfo2)
{
	text = Text;
	textString = g_Locale->getText(Text);

	active = Active;
	jumpTarget = Target;
	actionKey = ActionKey ? ActionKey : "";

	iconName = IconName ? IconName : "";
	number = Num;
	runningPercent = Percent;
	description = Descr;
	icon1 = Icon1;
	icon2 = Icon2;
	optionText1 = OptionText1;
	optionText2 = OptionText2;

	//
	info1 = Info1? Info1 : "";
	option_info1 = OptionInfo1? OptionInfo1 : "";
	info2 = Info2? Info2 : "";
	option_info2 = OptionInfo2? OptionInfo2 : "";

	itemType = ITEM_TYPE_LIST_BOX;
}

CMenulistBoxItem::CMenulistBoxItem(const char * const Text, const bool Active, CMenuTarget* Target, const char * const ActionKey, const char * const IconName, const int Num, const int Percent, const char* const Descr, const char* const Icon1, const char* const Icon2, const char* const OptionText1, const char* const OptionText2, const char* const Info1, const char* const OptionInfo1, const char* const Info2, const char* const OptionInfo2)
{
	text = NONEXISTANT_LOCALE;
	textString = Text;

	active = Active;
	jumpTarget = Target;
	actionKey = ActionKey ? ActionKey : "";

	iconName = IconName ? IconName : "";
	number = Num;
	runningPercent = Percent;
	description = Descr;
	icon1 = Icon1;
	icon2 = Icon2;
	optionText1 = OptionText1;
	optionText2 = OptionText2;

	//
	info1 = Info1? Info1 : "";
	option_info1 = OptionInfo1? OptionInfo1 : "";
	info2 = Info2? Info2 : "";
	option_info2 = OptionInfo2? OptionInfo2 : "";

	itemType = ITEM_TYPE_LIST_BOX;
}

int CMenulistBoxItem::getHeight(void) const
{
	int iconName_w = 0;
	int iconName_h = 0;

	CFrameBuffer::getInstance()->getIconSize(iconName.c_str(), &iconName_w, &iconName_h);
	
	return std::max(iconName_h, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight()) + 3;
}

int CMenulistBoxItem::getWidth(void) const
{
	int tw = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(textString); //FIXME:

	return tw;
}

int CMenulistBoxItem::exec(CMenuTarget* parent)
{
	dprintf(DEBUG_DEBUG, "CMenulistBoxItem::exec:\n");

	if(jumpTarget)
	{
		return jumpTarget->exec(parent, actionKey);
	}
	else
	{
		return menu_return::RETURN_EXIT;
	}
}

const char * CMenulistBoxItem::getName(void)
{
	const char * l_name;
	
	if(text == NONEXISTANT_LOCALE)
		l_name = textString.c_str();
	else
        	l_name = g_Locale->getText(text);
	
	return l_name;
}

int CMenulistBoxItem::paint(bool selected, bool /*AfterPulldown*/)
{
	dprintf(DEBUG_DEBUG, "CMenulistBoxItem::paint:\n");

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
	/*
	else if (marked)
	{
		color   = COL_MENUCONTENTINACTIVE;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_2;
	}
	*/
	
	// itemBox
	frameBuffer->paintBoxRel(x, y, dx, height, bgcolor); //FIXME
	
	// left icon
	int icon_w = 0;
	int icon_h = 0;
	
	if (!iconName.empty())
	{
		//get icon size
		frameBuffer->getIconSize(iconName.c_str(), &icon_w, &icon_h);
		
		frameBuffer->paintIcon(iconName, x + BORDER_LEFT, y + (height - icon_h)/2 );
	}

	// optionText1
	int optionText1_width = 0;
	if(!optionText1.empty())
	{
		optionText1_width = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth(optionText1.c_str());

		g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(x + dx - BORDER_RIGHT - optionText1_width, y + (height - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight(), optionText1_width, optionText1.c_str(), color, 0, true); // UTF-8
	}

	// optionText2
	int optionText2_width = 0;
	if(!optionText2.empty())
	{
		optionText2_width = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth(optionText2.c_str());

		g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(x + dx - BORDER_RIGHT - optionText1_width - ICON_OFFSET - optionText2_width, y + (height - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight(), optionText2_width, optionText2.c_str(), color, 0, true); // UTF-8
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
	
	// locale text
	int l_text_width = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(l_text, true);
	if(l_text_width >= dx - BORDER_LEFT - BORDER_RIGHT)
		l_text_width = dx - BORDER_LEFT - BORDER_RIGHT;

	if(l_text != NULL)
	{
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x + BORDER_LEFT + icon_w + numwidth + ICON_OFFSET + pBarWidth + ICON_OFFSET, y + (height - g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight(), dx - BORDER_RIGHT - BORDER_LEFT - numwidth - pBarWidth - 2*ICON_OFFSET - icon_w - icon1_w - icon2_w - optionText1_width - ICON_OFFSET - optionText2_width, l_text, color, 0, true); // UTF-8
	}

	// description text
	int descr_width = 0;
	bool paintDescription = true;
	std::string descr_text;
	if(!description.empty())
	{
		int iw, ih;
		//get icon size
		frameBuffer->getIconSize(NEUTRINO_ICON_HD, &iw, &ih);

		descr_width = dx - BORDER_LEFT - numwidth - ICON_OFFSET - icon_w - icon1_w - icon2_w - l_text_width;
		if (descr_width < 0) 
			paintDescription = false;

		if(paintDescription)
		{
			descr_text = " - ";
			descr_text += description;

			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString(x + BORDER_LEFT + numwidth + pBarWidth + ICON_OFFSET + l_text_width + ICON_OFFSET, y + (height - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight(), dx - BORDER_LEFT - BORDER_RIGHT - numwidth - ICON_OFFSET - pBarWidth - ICON_OFFSET - l_text_width - icon_w - icon1_w - ICON_OFFSET - icon2_w - ICON_OFFSET - 2*iw, descr_text.c_str(), COL_COLORED_EVENTS_CHANNELLIST, 0, true);
		}
	}

	// vfd
	if (selected)
	{
		CVFD::getInstance()->showMenuText(0, l_text, -1, true);
	}
	
	return y + height;
}

