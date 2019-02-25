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
static CBox cFrameBoxText;
static CTextBox* textBox = NULL;

//CFont* CMenuItem::nameFont = NULL;
//CFont* CMenuItem::optionFont = NULL;

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
CMenuItem::CMenuItem()
{
	x = -1;
	directKey = RC_nokey;
	iconName = "";
	can_arrow = false;

	number = 0;
	runningPercent = -1;

	//nameFont = g_Font[SNeutrinoSettings::FONT_TYPE_MENU];
	//optionFont = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER];
			
	//
	nLinesItem = false;
	widgetType = WIDGET_TYPE_STANDARD;

	marked = false;
}

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

void CMenuItem::setMarked(const bool Marked)
{
	marked = Marked;
	
	if (x != -1)
		paint();
}

// CMenuOptionChooser
CMenuOptionChooser::CMenuOptionChooser(const neutrino_locale_t OptionName, int *const OptionValue, const struct keyval* const Options, const unsigned Number_Of_Options, const bool Active, CChangeObserver* const Observ, const neutrino_msg_t DirectKey, const std::string& IconName, bool Pulldown, bool EnableMenuPos)
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

	enableMenuPos = EnableMenuPos;

	itemHelpText = optionNameString;

	itemType = ITEM_TYPE_OPTION_CHOOSER;
}

CMenuOptionChooser::CMenuOptionChooser(const char* OptionName, int* const OptionValue, const struct keyval *const Options, const unsigned Number_Of_Options, const bool Active, CChangeObserver* const Observ, const neutrino_msg_t DirectKey, const std::string & IconName, bool Pulldown, bool EnableMenuPos)
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
	enableMenuPos = EnableMenuPos;

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
	if( (!parent || msg == RC_ok) && pulldown ) 
	{
		int select = -1;
		char cnt[5];
		CMenuWidget *menu = new CMenuWidget(optionNameString.c_str(), NEUTRINO_ICON_SETTINGS);
		
		//if(parent)
		//	menu->move(20, 0);
		
		menu->enableSaveScreen();
		if(enableMenuPos)
			menu->enableMenuPosition();
		
		CMenuSelectorTarget* selector = new CMenuSelectorTarget(&select);

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
				if( msg == RC_left ) 
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

	// icons 
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_RED, &icon_w, &icon_h);
		
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
	int stringstartposName = x + BORDER_LEFT + icon_w + ICON_OFFSET;
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

	if( msg == RC_left ) 
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
	// icons 
	int icon_w, icon_h;
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_RED, &icon_w, &icon_h);

	int stringstartposName = x + BORDER_LEFT + icon_w + ICON_OFFSET;
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
CMenuOptionStringChooser::CMenuOptionStringChooser(const neutrino_locale_t Name, char * OptionValue, bool Active, CChangeObserver* Observ, const neutrino_msg_t DirectKey, const std::string & IconName, bool Pulldown, bool EnableMenuPos)
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
	enableMenuPos = EnableMenuPos;

	itemHelpText = nameString;
	itemType = ITEM_TYPE_OPTION_STRING_CHOOSER;
}

CMenuOptionStringChooser::CMenuOptionStringChooser(const char * const Name, char * OptionValue, bool Active, CChangeObserver* Observ, const neutrino_msg_t DirectKey, const std::string & IconName, bool Pulldown, bool EnableMenuPos)
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
	enableMenuPos = EnableMenuPos;

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
	if( (!parent || msg == RC_ok) && pulldown ) 
	{
		int select = -1;
		char cnt[5];
		
		CMenuWidget * menu = new CMenuWidget(nameString.c_str(), NEUTRINO_ICON_SETTINGS);
		
		//if(parent) 
		//	menu->move(20, 0);
		
		menu->enableSaveScreen();
		if(enableMenuPos)
			menu->enableMenuPosition();
		
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
				if( msg == RC_left ) 
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

	// icons 
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_RED, &icon_w, &icon_h);
		
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
	
	int stringstartposName = x + BORDER_LEFT + icon_w + ICON_OFFSET;
	
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

	directKey = RC_nokey;
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

	//FIXME:
/*	
	if ( wantsRepaint )
		return menu_return::RETURN_REPAINT;
	else
		return menu_return::RETURN_NONE;
*/
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

	// icons 
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_RED, &icon_w, &icon_h);
	
	if (!(iconName.empty()))
	{
		int iw, ih;

		frameBuffer->getIconSize(iconName.c_str(), &iw, &ih);
		
		frameBuffer->paintIcon(iconName, x + BORDER_LEFT, y+ ((height - ih)/2) );
	}

	// locale
	int stringstartposOption = x + BORDER_LEFT + icon_w + ICON_OFFSET;
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(stringstartposOption, y + (height - g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight(), dx - BORDER_RIGHT - (stringstartposOption - x), optionValue, color, 0, true); //UTF-8

	// vfd
	if (selected)
	{
		CVFD::getInstance()->showMenuText(1, optionValue);
	}

	return y + height;
}

// CMenuSeparator
/*
CMenuSeparator::CMenuSeparator(const int Type, const neutrino_locale_t Text)
{
	directKey = RC_nokey;
	iconName = "";
	type = Type;
	text = Text;
	textString = g_Locale->getText(Text);

	itemType = ITEM_TYPE_SEPARATOR;
}
*/

CMenuSeparator::CMenuSeparator(const int Type, const char * const Text)
{
	directKey = RC_nokey;
	iconName = "";
	type = Type;
	//text = NONEXISTANT_LOCALE;
	textString = Text;

	itemType = ITEM_TYPE_SEPARATOR;
}

int CMenuSeparator::getHeight(void) const
{
	return (textString == NULL) ? 10 : g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();	
}

int CMenuSeparator::getWidth(void) const
{
	return 0;
}

const char * CMenuSeparator::getString(void)
{
	return textString;
}

int CMenuSeparator::paint(bool /*selected*/, bool /*AfterPulldown*/)
{
	dprintf(DEBUG_DEBUG, "CMenuSeparator::paint:\n");

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

		//if (!textString.empty())
		if(textString != NULL)
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

// CMenuSelector
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

// CMenuForwarder
CMenuForwarder::CMenuForwarder(const neutrino_locale_t Text, const bool Active, const char * const Option, CMenuTarget* Target, const char * const ActionKey, neutrino_msg_t DirectKey, const char * const IconName, const char * const ItemIcon, const neutrino_locale_t HelpText )
{
	option = Option? Option : "";
	//option_string = NULL;

	textString = g_Locale->getText(Text);
	text = Text;
	
	active = Active;
	jumpTarget = Target;
	actionKey = ActionKey ? ActionKey : "";
	directKey = DirectKey;
	
	iconName = IconName ? IconName : "";
	
	itemIcon = ItemIcon ? ItemIcon : "";
	itemHelpText = g_Locale->getText(HelpText);
	itemType = ITEM_TYPE_FORWARDER;
	itemName = g_Locale->getText(Text);
}

/*
CMenuForwarder::CMenuForwarder(const neutrino_locale_t Text, const bool Active, const std::string &Option, CMenuTarget* Target, const char * const ActionKey, neutrino_msg_t DirectKey, const char * const IconName, const char * const ItemIcon, const neutrino_locale_t HelpText )
{
	option = NULL;
	option_string = &Option;

	textString = g_Locale->getText(Text);
	text = Text;
	
	active = Active;
	jumpTarget = Target;
	actionKey = ActionKey ? ActionKey : "";
	directKey = DirectKey;
	
	iconName = IconName ? IconName : "";
	
	itemIcon = ItemIcon ? ItemIcon : "";
	itemHelpText = g_Locale->getText(HelpText);
	itemType = ITEM_TYPE_FORWARDER;
	itemName = g_Locale->getText(Text);
}
*/

CMenuForwarder::CMenuForwarder(const char * const Text, const bool Active, const char * const Option, CMenuTarget* Target, const char * const ActionKey, neutrino_msg_t DirectKey, const char * const IconName, const char * const ItemIcon, const neutrino_locale_t HelpText )
{
	//option = Option;
	//option_string = NULL;

	textString = Text;
	text = NONEXISTANT_LOCALE;

	option = Option? Option : "";
	
	active = Active;
	jumpTarget = Target;
	actionKey = ActionKey ? ActionKey : "";
	directKey = DirectKey;
	
	iconName = IconName ? IconName : "";
	
	itemIcon = ItemIcon ? ItemIcon : "";
	itemHelpText = g_Locale->getText(HelpText);
	itemType = ITEM_TYPE_FORWARDER;
	itemName = Text;
}

/*
CMenuForwarder::CMenuForwarder(const char * const Text, const bool Active, const std::string &Option, CMenuTarget* Target, const char * const ActionKey, neutrino_msg_t DirectKey, const char * const IconName, const char * const ItemIcon, const neutrino_locale_t HelpText )
{
	option = NULL;
	option_string = &Option;

	textString = Text;
	text = NONEXISTANT_LOCALE;
	
	active = Active;
	jumpTarget = Target;
	actionKey = ActionKey ? ActionKey : "";
	directKey = DirectKey;
	
	iconName = IconName ? IconName : "";
	
	itemIcon = ItemIcon ? ItemIcon : "";
	itemHelpText = g_Locale->getText(HelpText);
	itemType = ITEM_TYPE_FORWARDER;
	itemName = Text;
}
*/

int CMenuForwarder::getHeight(void) const
{
	int iw = 0;
	int ih = 0;

	if(widgetType == WIDGET_TYPE_STANDARD)
		CFrameBuffer::getInstance()->getIconSize(NEUTRINO_ICON_BUTTON_RED, &iw, &ih);
	else if(widgetType == WIDGET_TYPE_EXTENDED)
	{
		CFrameBuffer::getInstance()->getIconSize(NEUTRINO_ICON_BUTTON_RED, &iw, &ih);
		return std::max(ih, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight()) + 10;
	}
	else if(widgetType == WIDGET_TYPE_CLASSIC)
	{
		iw = ITEM_ICON_W_MINI;
		ih = ITEM_ICON_H_MINI;
	}
	else if(widgetType == WIDGET_TYPE_FRAME)
	{
		return item_height;
	}
	
	return std::max(ih, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight()) + 3;
}

int CMenuForwarder::getWidth(void) const
{
	int tw = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(textString);

	//const char * option_text = NULL;

	//if (option.empyt())
	//	option_text = option;
	//else if (option_string)
	//	option_text = option_string->c_str();
	
        //if (option_text != NULL)
	if(!option.empty())
                tw += g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(option.c_str(), true);

	return tw;
}

int CMenuForwarder::exec(CMenuTarget* parent)
{
	dprintf(DEBUG_DEBUG, "CMenuForwarderExtended::exec:\n");

	if(jumpTarget)
		return jumpTarget->exec(parent, actionKey);
	else
		return menu_return::RETURN_EXIT;
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

const char * CMenuForwarder::getOption(void)
{
	//if (option)
	//	return option;
	//else
		//if (option_string)
		//	return option_string->c_str();
		//else
		//	return NULL;
	if(!option.empty())
		return option.c_str();
	else
		return NULL;
}

int CMenuForwarder::paint(bool selected, bool /*AfterPulldown*/)
{
	dprintf(DEBUG_DEBUG, "CMenuForwarder::paint\n");

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

	if(widgetType == WIDGET_TYPE_FRAME)
	{
		//
		frameBuffer->paintBoxRel(x, y, item_width, item_height, item_backgroundColor);

		if(!itemIcon.empty())
			frameBuffer->displayImage(itemIcon, x + 4*ICON_OFFSET, y + 4*ICON_OFFSET, item_width - 8*ICON_OFFSET, item_height - 8*ICON_OFFSET);

		//
		if(selected)
		{
			frameBuffer->paintBoxRel(x, y, item_width, item_height, item_selectedColor);

			if(!itemIcon.empty())
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
		int stringstartposX = x + (offx == 0? 0: offx);

		const char * option_text = getOption();	
	
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
	
		// icon
		if(widgetType == WIDGET_TYPE_STANDARD || widgetType == WIDGET_TYPE_EXTENDED)
		{
			frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_RED, &icon_w, &icon_h);
	
			if (!iconName.empty())
			{
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
		}
		else if(widgetType == WIDGET_TYPE_CLASSIC)
		{
			//frameBuffer->getIconSize(NEUTRINO_ICON_MENUITEM_NOPREVIEW, &icon_w, &icon_h);
			icon_w = ITEM_ICON_W_MINI;
			icon_h = ITEM_ICON_H_MINI;

			if (!itemIcon.empty())
			{
				frameBuffer->displayImage(itemIcon.c_str(), x + BORDER_LEFT, y + ((height - icon_h)/2), icon_w, icon_h);
			}
		}
	
		//local-text
		stringstartposX = x + BORDER_LEFT + icon_w + ICON_OFFSET;

		g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(stringstartposX, y + (height - g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight(), dx - BORDER_RIGHT - icon_w - (stringstartposX - x), l_text, color, 0, true); // UTF-8

		//option-text
		if(widgetType == WIDGET_TYPE_STANDARD)
		{
			if (option_text != NULL)
			{
				int stringwidth = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(option_text, true);
				int stringstartposOption = std::max(stringstartposX + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(l_text, true), x + dx - (stringwidth + BORDER_RIGHT)); //+ offx
		
				g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(stringstartposOption, y + (height - g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight(), dx - BORDER_LEFT - (stringstartposOption- x),  option_text, color, 0, true);
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

// lockedMenuForward
int CLockedMenuForwarder::exec(CMenuTarget* parent)
{
	dprintf(DEBUG_DEBUG, "CLockedMenuForwarder::exec\n");

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

//ClistBoxItem
ClistBoxItem::ClistBoxItem(const neutrino_locale_t Text, const bool Active, const char* const Option, CMenuTarget* Target, const char* const ActionKey, const char* const IconName, const char* const ItemIcon)
{
	text = Text;
	textString = g_Locale->getText(Text);

	option = Option? Option : "";

	active = Active;
	jumpTarget = Target;
	actionKey = ActionKey ? ActionKey : "";

	iconName = IconName ? IconName : "";
	itemIcon = ItemIcon? ItemIcon : "";
	itemName = g_Locale->getText(Text);
	itemType = ITEM_TYPE_LIST_BOX;
}

ClistBoxItem::ClistBoxItem(const char* Text, const bool Active, const char* const Option, CMenuTarget* Target, const char* const ActionKey, const char* const IconName, const char* const ItemIcon)
{
	text = NONEXISTANT_LOCALE;
	textString = Text;

	option = Option? Option : "";

	active = Active;
	jumpTarget = Target;
	actionKey = ActionKey ? ActionKey : "";

	iconName = IconName ? IconName : "";
	itemIcon = ItemIcon? ItemIcon : "";
	itemName = Text;
	itemType = ITEM_TYPE_LIST_BOX;
}

int ClistBoxItem::getHeight(void) const
{
	int iw = 0;
	int ih = 0;

	if(widgetType == WIDGET_TYPE_FRAME)
	{
		return item_height;
	}
	else if(widgetType == WIDGET_TYPE_EXTENDED || widgetType == WIDGET_TYPE_CLASSIC)
	{
		iw = ITEM_ICON_W_MINI;
		ih = ITEM_ICON_H_MINI;
		return std::max(ih, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight()) + 6;
	}
	else
	{
		CFrameBuffer::getInstance()->getIconSize(iconName.c_str(), &iw, &ih);
	
		return std::max(ih, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight()) + 6;
	}
}

int ClistBoxItem::getWidth(void) const
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

int ClistBoxItem::exec(CMenuTarget* parent)
{
	dprintf(DEBUG_DEBUG, "ClistBoxItem::exec:\n");

	if(jumpTarget)
		return jumpTarget->exec(parent, actionKey);
	else
		return menu_return::RETURN_EXIT;
}

const char * ClistBoxItem::getName(void)
{
	const char * l_name;
	
	if(text == NONEXISTANT_LOCALE)
		l_name = textString.c_str();
	else
        	l_name = g_Locale->getText(text);
	
	return l_name;
}

int ClistBoxItem::paint(bool selected, bool /*AfterPulldown*/)
{
	dprintf(DEBUG_DEBUG, "ClistBoxItem::paint:\n");

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

	if(widgetType == WIDGET_TYPE_FRAME)
	{
		//
		frameBuffer->paintBoxRel(x, y, item_width, item_height, item_backgroundColor);

		if(!itemIcon.empty())
			frameBuffer->displayImage(itemIcon, x + 4*ICON_OFFSET, y + 4*ICON_OFFSET, item_width - 8*ICON_OFFSET, item_height - 8*ICON_OFFSET);

		//
		if(selected)
		{
			frameBuffer->paintBoxRel(x, y, item_width, item_height, item_selectedColor);

			if(!itemIcon.empty())
				frameBuffer->displayImage(itemIcon, x + ICON_OFFSET/2, y + ICON_OFFSET/2, item_width - ICON_OFFSET, item_height - ICON_OFFSET);

		}

		// vfd
		if (selected)
		{
			CVFD::getInstance()->showMenuText(0, l_text, -1, true);
		}

		return 0;
	}
	else // standard|classic|extended
	{	
		// itemBox
		frameBuffer->paintBoxRel(x, y, dx, height, bgcolor); //FIXME
	
		// left icon
		int icon_w = 0;
		int icon_h = 0;

		if(widgetType == WIDGET_TYPE_CLASSIC)
		{
			icon_w = ITEM_ICON_W_MINI;
			icon_h = ITEM_ICON_H_MINI;

			if (!itemIcon.empty())
			{
				frameBuffer->displayImage(itemIcon.c_str(), x + BORDER_LEFT, y + ((height - icon_h)/2), icon_w, icon_h);
			}
		}
		else if(widgetType == WIDGET_TYPE_EXTENDED)
		{
			icon_w = ITEM_ICON_W_MINI;
			icon_h = ITEM_ICON_H_MINI;

			if (!itemIcon.empty())
			{
				frameBuffer->displayImage(itemIcon.c_str(), x + BORDER_LEFT, y + ((height - icon_h)/2), icon_w/2, icon_h);
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

		if(widgetType == WIDGET_TYPE_EXTENDED)
		{
			// local
			if(l_text_width >= dx - BORDER_LEFT - BORDER_RIGHT)
				l_text_width = dx - BORDER_LEFT - BORDER_RIGHT;

			if(l_text != NULL)
			{
				/*nameFont*/g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x + BORDER_LEFT + icon_w/2 + numwidth + ICON_OFFSET + pBarWidth + ICON_OFFSET, y + 3 + /*nameFont*/g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight(), dx - BORDER_RIGHT - BORDER_LEFT - numwidth - pBarWidth - 2*ICON_OFFSET - icon_w - icon1_w - icon2_w - optionInfo_width - ICON_OFFSET, l_text, color, 0, true); // UTF-8
			}

			// option
			if(!option.empty())
			{
				/*optionFont*/g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(x + BORDER_LEFT + icon_w/2 + numwidth + ICON_OFFSET + pBarWidth + ICON_OFFSET, y + height, dx - BORDER_LEFT - BORDER_RIGHT - numwidth - pBarWidth - 2*ICON_OFFSET - icon_w - icon1_w - icon2_w - optionInfo_width - ICON_OFFSET, option.c_str(), color, 0, true);
			}
		}
		else if (widgetType == WIDGET_TYPE_CLASSIC)
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
		else if(widgetType == WIDGET_TYPE_STANDARD)// standard
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

//ClistBoxEntryItem
ClistBoxEntryItem::ClistBoxEntryItem(const neutrino_locale_t Text, const bool Active, const char* const Option, const char* const IconName)
{
	text = Text;
	textString = g_Locale->getText(Text);
	option = Option? Option : "";

	active = Active;

	iconName = IconName ? IconName : "";
	itemName = g_Locale->getText(Text);
	itemType = ITEM_TYPE_LIST_BOX_ENTRY;
}

ClistBoxEntryItem::ClistBoxEntryItem(const char* Text, const bool Active, const char* const Option, const char* const IconName)
{
	text = NONEXISTANT_LOCALE;
	textString = Text;
	option = Option? Option : "";

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
	fb_pixel_t bgcolor = marked? COL_MENUCONTENTSELECTED_PLUS_2 : item_backgroundColor;

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
		frameBuffer->paintBoxRel(x, y, item_width, item_height, bgcolor);

		if(!itemIcon.empty())
			frameBuffer->displayImage(itemIcon, x + 4*ICON_OFFSET, y + 4*ICON_OFFSET, item_width - 8*ICON_OFFSET, item_height - 8*ICON_OFFSET);

		//
		if(selected)
		{
			frameBuffer->paintBoxRel(x, y, item_width, item_height, item_selectedColor);

			if(!itemIcon.empty())
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

/// CMenuWidget
CMenuWidget::CMenuWidget()
{
        nameString = g_Locale->getText(NONEXISTANT_LOCALE);
	name = NONEXISTANT_LOCALE;
        iconfile = "";
        selected = -1;
        iconOffset = 0;
	offx = offy = 0;
	
	//
	savescreen = false;
	background = NULL;

	MenuPos = false;

	//
	widgetType = WIDGET_TYPE_STANDARD;
	widgetChange = false;

	// frame
	backgroundColor = COL_MENUCONTENT_PLUS_0;
	itemBoxColor = COL_MENUCONTENTSELECTED_PLUS_0;
	itemsPerX = 6;
	itemsPerY = 3;

	maxItemsPerPage = itemsPerX*itemsPerY;

	//
	FootInfo = false;
	cFrameFootInfo.iHeight = 0;
	interFrame = 0;	
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
	savescreen = false;
	background = NULL;

	MenuPos = false;

	//
	widgetType = WIDGET_TYPE_STANDARD;
	widgetChange = false;

	// frame
	backgroundColor = COL_MENUCONTENT_PLUS_0;
	itemBoxColor = COL_MENUCONTENTSELECTED_PLUS_0;
	itemsPerX = 6;
	itemsPerY = 3;

	maxItemsPerPage = itemsPerX*itemsPerY;

	//
	FootInfo = false;
	cFrameFootInfo.iHeight = 0;
	interFrame = 0;
}

void CMenuWidget::move(int xoff, int yoff)
{
	offx = xoff;
	offy = yoff;
}

CMenuWidget::~CMenuWidget()
{
	items.clear();
	page_start.clear();
}

void CMenuWidget::addItem(CMenuItem *menuItem, const bool defaultselected)
{
	if (defaultselected)
		selected = items.size();
	
	items.push_back(menuItem);
}

bool CMenuWidget::hasItem()
{
	return !items.empty();
}

void CMenuWidget::hide()
{
	dprintf(DEBUG_NORMAL, "CMenuWidget::hide:\n");

	if( savescreen && background)
		restoreScreen();
	else
		frameBuffer->paintBackgroundBoxRel(x, y, full_width, full_height);

	hideItemInfo();  
	
	frameBuffer->blit();
}

void CMenuWidget::paint()
{
	dprintf(DEBUG_NORMAL, "CMenuWidget::paint\n");

	const char * l_name;
	
	if(name == NONEXISTANT_LOCALE)
		l_name = nameString.c_str();
	else
        	l_name = g_Locale->getText(name);	

	CVFD::getInstance()->setMode(CVFD::MODE_MENU_UTF8 );

	// widget type
	if(widgetChange)
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

	for (unsigned int count = 0; count < items.size(); count++) 
	{
		CMenuItem * item = items[count];

		item->widgetType = widgetType;
		item->item_backgroundColor = backgroundColor;
		item->item_selectedColor = itemBoxColor;
	} 

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
		x = g_settings.screen_StartX + 20;
		y = g_settings.screen_StartY + 20;
		int width = g_settings.screen_EndX - g_settings.screen_StartX - 40;
		height = g_settings.screen_EndY - g_settings.screen_StartY - 40;

		full_width = width;
		full_height = height;

		//
		if(savescreen) 
			saveScreen();

		// paint background
		frameBuffer->paintBoxRel(x, y, width, height, backgroundColor);

		// head
		int iw = 0;
		int ih = 0;
		frameBuffer->getIconSize(iconfile.c_str(), &iw, &ih);
		hheight = std::max(ih, g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight()) + 6;

		//headbox
		frameBuffer->paintIcon(iconfile, x + BORDER_LEFT, y + (hheight - ih)/2);

		// title
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(x + BORDER_LEFT + iw + ICON_OFFSET, y + (hheight - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), width - (BORDER_LEFT + BORDER_RIGHT + iw +  ICON_OFFSET), l_name, COL_MENUHEAD);
	
		// Hline
		frameBuffer->paintHLineRel(x + BORDER_LEFT, width - BORDER_LEFT - BORDER_RIGHT, y + hheight, COL_MENUCONTENT_PLUS_5);
	
		// paint horizontal line bottom
		fheight = hheight;
		frameBuffer->paintHLineRel(x + BORDER_LEFT, width - BORDER_LEFT - BORDER_RIGHT, y + height - fheight, COL_MENUCONTENT_PLUS_5);


		//
		item_width = width/itemsPerX;
		item_height = (height - hheight - fheight - 20)/itemsPerY;

		for (unsigned int count = 0; count < items.size(); count++) 
		{
			CMenuItem * item = items[count];

			item->item_width = item_width;
			item->item_height = item_height;
		} 

		//item_start_y
		item_start_y = y + hheight + 2*ICON_OFFSET;
	
		// paint items
		paintItems();
	}
	else
	{
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
		int itemHeightTotal = 0;
		item_height = 0;
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
		cFrameFootInfo.iHeight = 0;
		interFrame = 0;

		if(FootInfo && widgetType == WIDGET_TYPE_STANDARD)
		{
			cFrameFootInfo.iHeight = 70;
			cFrameFootInfo.iWidth = width;

			interFrame = INTER_FRAME_SPACE;
		}

		// shrink menu if less items
		if(hheight + itemHeightTotal + fheight < height)
			height = hheight + heightCurrPage + fheight;
		else 	
			height = hheight + heightFirstPage + fheight;

		//	
		full_width = width;
		full_height = height + cFrameFootInfo.iHeight + interFrame;

		// coordinations
		x = offx + frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - full_width ) >> 1 );
		y = offy + frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - full_height) >> 1 );

		// menu position
		if(g_settings.menu_position == SNeutrinoSettings::MENU_POSITION_CENTER && MenuPos)
		{
			x = offx + frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - full_width ) >> 1 );
			y = offy + frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - full_height) >> 1 );
		}
		else if(g_settings.menu_position == SNeutrinoSettings::MENU_POSITION_LEFT && MenuPos)
		{
			x = offx + frameBuffer->getScreenX() + BORDER_LEFT;
			y = offy + frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - full_height) >> 1 );
		}
		else if(g_settings.menu_position == SNeutrinoSettings::MENU_POSITION_RIGHT && MenuPos)
		{
			x = offx + frameBuffer->getScreenX() + frameBuffer->getScreenWidth() - full_width - BORDER_RIGHT;
			y = offy + frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - full_height) >> 1 );
		}

		//
		if(FootInfo)
		{
			cFrameFootInfo.iX = x;
			cFrameFootInfo.iY = y + height + interFrame;

			interFrame = INTER_FRAME_SPACE;
		}

		//
		if(savescreen) 
			saveScreen();

		// paint head
		//headers.paintHead(x, y, width, hheight, l_name, iconfile.c_str()); //FIXME
		frameBuffer->paintBoxRel(x, y, width, hheight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, g_settings.Head_gradient);

		frameBuffer->paintIcon(iconfile.c_str(), x + BORDER_LEFT, y + (hheight - icon_head_h)/2);

		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(x + BORDER_LEFT + icon_head_w + ICON_OFFSET, y + (hheight - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), width - BORDER_LEFT - BORDER_RIGHT - icon_head_w - ICON_OFFSET, l_name, COL_MENUHEAD);
	
		//paint foot
		frameBuffer->paintBoxRel(x, y + height - fheight, width, fheight, COL_MENUFOOT_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.Foot_gradient);
	
		//item_start_y
		item_start_y = y + hheight;
	
		// paint items
		paintItems();
	}
}

// paint items
void CMenuWidget::paintItems()
{
	if(widgetType == WIDGET_TYPE_FRAME)
	{
		// paint background
		int width = g_settings.screen_EndX - g_settings.screen_StartX - 40;

		frameBuffer->paintBoxRel(x, y + hheight + 2*ICON_OFFSET, width, height - hheight - fheight - 2*ICON_OFFSET, backgroundColor);

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

					item->init(x + _x*item_width, item_start_y + _y*item_height, items_width, iconOffset);

					if( (item->isSelectable()) && (selected == -1) ) 
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
		else
			sb_width = 0;
	
		//
		items_width = width - sb_width;

		if(widgetType == WIDGET_TYPE_EXTENDED)
			items_width = 2*(width/3) - sb_width;
	
		// item not currently on screen
		if (selected >= 0)
		{
			while(selected < (int)page_start[current_page])
				current_page--;
		
			while(selected >= (int)page_start[current_page + 1])
				current_page++;
		}
	
		// paint items background
		if(widgetType == WIDGET_TYPE_EXTENDED)
			frameBuffer->paintBoxRel(x, item_start_y, width, items_height, COL_MENUCONTENTDARK_PLUS_0);
		else
			frameBuffer->paintBoxRel(x, item_start_y, width, items_height, COL_MENUCONTENT_PLUS_0);
	
		// paint right scroll bar if we have more then one page
		if(total_pages > 1)
		{
			scrollBar.paint(x + items_width, item_start_y, items_height, total_pages, current_page);
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

				if (selected == (signed int)count) 
				{
					paintItemInfo(count);
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
}

void CMenuWidget::saveScreen()
{
	if(!savescreen)
		return;

	delete[] background;

	background = new fb_pixel_t[full_width*full_height];
	
	if(background)
		frameBuffer->saveScreen(x, y, full_width, full_height, background);
}

void CMenuWidget::restoreScreen()
{
	if(background) 
	{
		if(savescreen)
			frameBuffer->restoreScreen(x, y, full_width, full_height, background);
	}
}

void CMenuWidget::enableSaveScreen()
{
	savescreen = true;
	
	if(!savescreen && background) 
	{
		delete[] background;
		background = NULL;
	}
}

void CMenuWidget::paintItemInfo(int pos)
{
	if(widgetType == WIDGET_TYPE_FRAME)
	{
		// refresh box
		frameBuffer->paintBoxRel(x, y + height - fheight + 3, width, fheight - 3, backgroundColor);

		// text
		if(items.size() > 0)
		{
			CMenuItem* item = items[pos];
	
			// itemName
			if(!item->itemName.empty())
			{
				g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(x + BORDER_LEFT, y + height - fheight + (fheight - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE] ->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), width - BORDER_LEFT - BORDER_RIGHT, item->itemName.c_str(), COL_MENUFOOT_INFO);
			}

			// itemHelpText
			if(!item->itemHelpText.empty())
			{
				g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(x + BORDER_LEFT, y + height, width - BORDER_LEFT - BORDER_RIGHT, item->itemHelpText.c_str(), COL_MENUFOOT_INFO);
			}
		}
	}
	else if(widgetType == WIDGET_TYPE_CLASSIC)
	{
		CMenuItem* item = items[pos];

		item->getYPosition();

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
	}
	else if(widgetType == WIDGET_TYPE_EXTENDED)
	{
		CMenuItem* item = items[pos];

		item->getYPosition();

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
	else if(widgetType == WIDGET_TYPE_STANDARD)
	{
		if(FootInfo)
		{
			CMenuItem* item = items[pos];

			item->getYPosition();
	
			// detailslines|box
			itemsLine.paintD(x, y, width, height + interFrame, cFrameFootInfo.iHeight, hheight, item->getHeight(), item->getYPosition());


			// info icon
			int iw, ih;

			// check for minimum hight
			if(full_height - hheight - fheight >= ITEM_ICON_H)
			{ 
				frameBuffer->getIconSize(item->itemIcon.c_str(), &iw, &ih);

				frameBuffer->displayImage(item->itemIcon.c_str(), x + ICON_OFFSET, y + height + interFrame + (cFrameFootInfo.iHeight - 40)/2, 100, 40);
			}

			// itemHelpText
			if(!item->itemHelpText.empty())
			{
				g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->RenderString(x + BORDER_LEFT + 100 + ICON_OFFSET, y + height + interFrame + (cFrameFootInfo.iHeight - g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getHeight(), width - BORDER_LEFT - BORDER_RIGHT - 100, item->itemHelpText.c_str(), COL_MENUFOOT, 0, true); // UTF-8
			}

/*
			cFrameBoxText.iX = x + 100 + ICON_OFFSET;
			cFrameBoxText.iY = y + full_height - cFrameFootInfo.iHeight + 2;
			cFrameBoxText.iWidth = full_width - 100 - ICON_OFFSET - 2;
			cFrameBoxText.iHeight = cFrameFootInfo.iHeight - 4;

			if(textBox)
			{
				delete textBox;
				textBox = NULL;
			}
	
			textBox = new CTextBox("", NULL, CTextBox::SCROLL, &cFrameBoxText);

			textBox->setBackGroundColor(COL_MENUFOOT_INFO_PLUS_0);
			textBox->setGradient(g_settings.Foot_Info_gradient);

			// itemHelpText
			if(!item->itemHelpText.empty())
			{
				textBox->setText(&item->itemHelpText);
			}
*/
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

			// itemHelpText
			if(!item->itemHelpText.empty())
			{
				g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->RenderString(x + BORDER_LEFT + iw + ICON_OFFSET, y + full_height - fheight + (fheight - g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getHeight(), width - BORDER_LEFT - BORDER_RIGHT - iw, item->itemHelpText.c_str(), COL_MENUFOOT, 0, true); // UTF-8
			}
		}
	}
}

void CMenuWidget::hideItemInfo()
{
	if(widgetType == WIDGET_TYPE_STANDARD && FootInfo)
	{
		itemsLine.clear(x, y, width + ConnectLineBox_Width, height, cFrameFootInfo.iHeight);

/*
		if(textBox)
		{
			delete textBox;
			textBox = NULL;
		}
*/
	}  
}

void CMenuWidget::integratePlugins(CPlugins::i_type_t integration, const unsigned int shortcut, bool enabled)
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
	uint64_t timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);

	//control loop
	do {
		g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd);

		if ( msg <= CRCInput::RC_MaxRC ) 
		{
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);
		}
		
		int handled = false;

		dprintf(DEBUG_DEBUG, "CMenuWidget::exec:msg: %s\n", CRCInput::getSpecialKeyName(msg));

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

		if (!handled) 
		{
			switch (msg) 
			{
				case (NeutrinoMessages::EVT_TIMER):
					if ( CNeutrinoApp::getInstance()->handleMsg(msg, data) & messages_return::cancel_all ) 
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
					else
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
								pos++;
							}
						}
					}
					break;

				case (RC_page_down) :
					if(widgetType == WIDGET_TYPE_FRAME)
					{
						if(current_page) 
						{
							pos = (int) page_start[current_page] - 1;

							selected = pos;
							paintItems();
						}
					}
					else
					{
						pos = (int) page_start[current_page + 1];
						if(pos >= (int) items.size()) 
							pos = items.size() - 1;
						for (unsigned int count = pos ; count < items.size(); count++) 
						{
							CMenuItem * item = items[pos];
							if ( item->isSelectable() ) 
							{
								if ((pos < (int)page_start[current_page + 1]) && (pos >= (int)page_start[current_page])) 
								{
									items[selected]->paint( false );
									item->paint( true );
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
					
					break;
					
				case (RC_left):
					if(widgetType == WIDGET_TYPE_FRAME)
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

						break;
					}
					else
					{
						//
						if(!(items[selected]->can_arrow)) 
						{
							msg = RC_timeout;
							break;
						}
					}
					
				case (RC_right):
					if(widgetType == WIDGET_TYPE_FRAME)
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
					else
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
									msg = RC_timeout;
									break;
									
								case menu_return::RETURN_REPAINT:
									hide();
									paint();
									break;
							}
						} 
						else
							msg = RC_timeout;
					}
					break;

				case (RC_ok):
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
									msg = RC_timeout;
									break;
									
								case menu_return::RETURN_REPAINT:
									hide();
									paint();
									break;
							}
						} 
						else
							msg = RC_timeout;
					}
					break;

				case (RC_home):
					exit_pressed = true;
					msg = RC_timeout;
					break;
					
				case (RC_timeout):
					break;

				case (RC_sat):
				case (RC_favorites):
					g_RCInput->postMsg (msg, 0);
					
				//
				case (RC_setup):
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
					break;

				default:
					if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all ) 
					{
						retval = menu_return::RETURN_EXIT_ALL;
						msg = RC_timeout;
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
	while ( msg != RC_timeout );
	
	hide();

	//test
	if (background) 
	{
		delete[] background;
		background = NULL;
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

/// ClistBoxWidget
ClistBoxWidget::ClistBoxWidget()
{
        nameString = g_Locale->getText(NONEXISTANT_LOCALE);
	name = NONEXISTANT_LOCALE;
        iconfile = "";
        selected = -1;
        iconOffset = 0;
	offx = offy = 0;
	
	//
	savescreen = false;
	background = NULL;

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
	footInfoHeight = 70;
	cFrameFootInfo.iHeight = 0;
	interFrame = 0;

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

void ClistBoxWidget::Init(const std::string & Icon, const int mwidth, const int mheight)
{
        frameBuffer = CFrameBuffer::getInstance();
        iconfile = Icon;
        selected = -1;
        width = mwidth;
	wanted_width = mwidth;
	
        if(width > (int) frameBuffer->getScreenWidth())
		width = frameBuffer->getScreenWidth();
	
      	height = mheight;
        wanted_height = mheight;
	
        if(width > (int) frameBuffer->getScreenWidth())
		width = frameBuffer->getScreenWidth();
	
      	height = mheight;
        wanted_height = mheight;

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
	footInfoHeight = 70;
	cFrameFootInfo.iHeight = 0;
	interFrame = 0;

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
}

void ClistBoxWidget::move(int xoff, int yoff)
{
	offx = xoff;
	offy = yoff;
}

ClistBoxWidget::~ClistBoxWidget()
{
	items.clear();
	page_start.clear();
}

void ClistBoxWidget::addItem(CMenuItem *menuItem, const bool defaultselected)
{
	if (defaultselected)
		selected = items.size();
	
	items.push_back(menuItem);
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

	// widgettype forwarded to item 
	for (unsigned int count = 0; count < items.size(); count++) 
	{
		CMenuItem * item = items[count];

		item->widgetType = widgetType;
		item->item_backgroundColor = backgroundColor;
		item->item_selectedColor = itemBoxColor;
	} 

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
		x = g_settings.screen_StartX + 20;
		y = g_settings.screen_StartY + 20;
		width = g_settings.screen_EndX - g_settings.screen_StartX - 40;
		height = g_settings.screen_EndY - g_settings.screen_StartY - 40;

		full_width = width;
		full_height = height;

		//
		if(savescreen) 
			saveScreen();

		//head height
		icon_head_w = 0;
		icon_head_h = 0;
		frameBuffer->getIconSize(iconfile.c_str(), &icon_head_w, &icon_head_h);
		hheight = std::max(icon_head_h, g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight()) + 6;
	
		// foot height
		fheight = hheight;

		//
		item_width = width/itemsPerX;
		item_height = (height - hheight - fheight - 20)/itemsPerY;

		for (unsigned int count = 0; count < items.size(); count++) 
		{
			CMenuItem * item = items[count];

			item->item_width = item_width;
			item->item_height = item_height;
		} 
	}
	else
	{
		// footInfo height
		cFrameFootInfo.iHeight = 0;
		interFrame = 0;

		if(FootInfo && widgetType == WIDGET_TYPE_STANDARD)
		{
			cFrameFootInfo.iHeight = footInfoHeight;
			interFrame = INTER_FRAME_SPACE;
		}

		height = wanted_height;

		// recalculate height
		if(height > ((int)frameBuffer->getScreenHeight() - 10))
			height = frameBuffer->getScreenHeight() - 10;

		width = wanted_width;

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

		// shrink menu if less items
		if(shrinkMenu)
		{
			if(hheight + itemHeightTotal + fheight < height)
				height = hheight + heightCurrPage + fheight;
			else 	
				height = hheight + heightFirstPage + fheight;
		}

		//
		full_width = width;
		full_height = height + cFrameFootInfo.iHeight + interFrame;
		
		// coordinations
		x = offx + frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - full_width ) >> 1 );
		y = offy + frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - full_height) >> 1 );

		//
		if(FootInfo && widgetType == WIDGET_TYPE_STANDARD)
		{
			cFrameFootInfo.iX = x;
			cFrameFootInfo.iY = y + height + interFrame;
			cFrameFootInfo.iWidth = width;
		}
	}
}

void ClistBoxWidget::resizeFrames()
{
	// footInfo height
	cFrameFootInfo.iHeight = footInfoHeight;

	// reinit frames
	initFrames();
}

void ClistBoxWidget::paintHead()
{
	if(widgetType == WIDGET_TYPE_FRAME)
	{
		// box
		frameBuffer->paintBoxRel(x, y, width, hheight, backgroundColor);

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
		headers.enablePaintDate();
		headers.setHeaderButtons(hbutton_labels, hbutton_count);
		headers.paintHead(x, y, width, hheight, l_name, iconfile.c_str());
	}
}

void ClistBoxWidget::paintFoot()
{
	if(widgetType != WIDGET_TYPE_FRAME)
	{
		headers.paintFoot(x, y + height - fheight, width, fheight, fbutton_count, fbutton_labels);
	}
}

void ClistBoxWidget::paint()
{
	dprintf(DEBUG_NORMAL, "ClistBoxWidget::paint:\n");

	CVFD::getInstance()->setMode(CVFD::MODE_MENU_UTF8 );

	item_start_y = y + hheight;

	if(widgetType == WIDGET_TYPE_FRAME)
		item_start_y = y + hheight + 2*ICON_OFFSET;

	// widget frame paint background hlines
	if(widgetType == WIDGET_TYPE_FRAME)
	{
		// paint background
		frameBuffer->paintBoxRel(x, y, width, height, backgroundColor);

		// paint horizontal line top
		frameBuffer->paintHLineRel(x + BORDER_LEFT, width - BORDER_LEFT - BORDER_RIGHT, y + hheight, COL_MENUCONTENT_PLUS_5);
	
		// paint horizontal line bottom
		fheight = hheight;
		frameBuffer->paintHLineRel(x + BORDER_LEFT, width - BORDER_LEFT - BORDER_RIGHT, y + height - fheight, COL_MENUCONTENT_PLUS_5);
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
		frameBuffer->paintBoxRel(x, y + hheight + 2*ICON_OFFSET, width, height - hheight - fheight - 2*ICON_OFFSET, backgroundColor);

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

					item->init(x + _x*item_width, item_start_y + _y*item_height, items_width, iconOffset);

					if( (item->isSelectable()) && (selected == -1) ) 
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

		if(widgetType == WIDGET_TYPE_EXTENDED)
		{
			items_width = 2*(width/3) - sb_width;

			// extended
			cFrameBoxText.iX = x + items_width;
			cFrameBoxText.iY = y + hheight;
			cFrameBoxText.iWidth = width - items_width;
			cFrameBoxText.iHeight = items_height;

			textBox = new CTextBox("", g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER], CTextBox::SCROLL, &cFrameBoxText, COL_MENUCONTENTDARK_PLUS_0);
		}
	
		// item not currently on screen
		if (selected >= 0)
		{
			while(selected < (int)page_start[current_page])
				current_page--;
		
			while(selected >= (int)page_start[current_page + 1])
				current_page++;
		}
	
		if(widgetType == WIDGET_TYPE_EXTENDED)
		{
			// paint items background
			frameBuffer->paintBoxRel(x, item_start_y, width, height - hheight - fheight, COL_MENUCONTENTDARK_PLUS_0);
		}
		else
			frameBuffer->paintBoxRel(x, item_start_y, width, items_height, COL_MENUCONTENT_PLUS_0);
	
		// paint right scrollBar if we have more then one page
		if(total_pages > 1)
		{
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
			
				if( (item->isSelectable()) && (selected == -1) ) 
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
	if(widgetType == WIDGET_TYPE_FRAME)
	{
		frameBuffer->paintBoxRel(x, y + height - fheight + 3, width, fheight - 3, backgroundColor);

		if(items.size() > 0)
		{
			CMenuItem* item = items[pos];
	
			// itemName
			if(!item->itemName.empty())
			{
				g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(x + BORDER_LEFT, y + height - fheight + (fheight - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE] ->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), width - BORDER_LEFT - BORDER_RIGHT, item->itemName.c_str(), COL_MENUFOOT_INFO);
			}

			// option
			if(!item->option.empty())
			{
				g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(x + BORDER_LEFT, y + height, width - BORDER_LEFT - BORDER_RIGHT, item->option.c_str(), COL_MENUFOOT_INFO);
			}
		}
	}
	else if(widgetType == WIDGET_TYPE_CLASSIC)
	{
		if(fbutton_count == 0)
		{
			CMenuItem* item = items[pos];

			item->getYPosition();

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
		}
	}
	else if(widgetType == WIDGET_TYPE_STANDARD)
	{
		if(FootInfo)
		{
			CMenuItem* item = items[pos];

			item->getYPosition();
	
			// detailslines
			itemsLine.paintD(x, y, width, height + interFrame, cFrameFootInfo.iHeight, hheight, item->getHeight(), item->getYPosition());

			// option_info1
			int l_ow1 = 0;
			if(!item->option_info1.empty())
			{
				l_ow1 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getRenderWidth(item->option_info1.c_str());

				g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString(x + width - BORDER_RIGHT - l_ow1, y + height + + interFrame + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight(), width - BORDER_LEFT - BORDER_RIGHT - l_ow1, item->option_info1.c_str(), COL_MENUFOOT_INFO, 0, true);
			}

			// info1
			int l_w1 = 0;
			if(!item->info1.empty())
			{
				l_w1 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getRenderWidth(item->info1.c_str());

				g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(x + BORDER_LEFT, y + height + + interFrame + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), width - BORDER_LEFT - BORDER_RIGHT - l_w1, item->info1.c_str(), COL_MENUFOOT_INFO, 0, true);
			}

			// option_info2
			int l_ow2 = 0;
			if(!item->option_info2.empty())
			{
				l_ow2 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth(item->option_info2.c_str());

				g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(x + width - BORDER_RIGHT - l_ow2, y + height + + interFrame + cFrameFootInfo.iHeight/2 + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight(), width - BORDER_LEFT - BORDER_RIGHT - l_ow2, item->option_info2.c_str(), COL_MENUFOOT_INFO, 0, true);
			}

			// info2
			int l_w2 = 0;
			if(!item->info2.empty())
			{
				l_w2 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getRenderWidth(item->info2.c_str());

				g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString (x + BORDER_LEFT, y + height + + interFrame + cFrameFootInfo.iHeight/2 + (cFrameFootInfo.iHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight(), width - BORDER_LEFT - BORDER_RIGHT - l_w2, item->info2.c_str(), COL_MENUFOOT_INFO, 0, true); // UTF-8
			}
		}
	}
	else if(widgetType == WIDGET_TYPE_EXTENDED)
	{
		CMenuItem* item = items[pos];

		item->getYPosition();

		// scale pic
		int p_w = 0;
		int p_h = 0;

		std::string fname = item->itemIcon;

		CFrameBuffer::getInstance()->scaleImage(fname, &p_w, &p_h);

		textBox->setText(&item->info1, item->itemIcon, p_w, p_h, CTextBox::TOP_CENTER);
	}
}

void ClistBoxWidget::hideItemInfo()
{
	if(widgetType == WIDGET_TYPE_STANDARD)
	{
		itemsLine.clear(x, y, width + ConnectLineBox_Width, height, cFrameFootInfo.iHeight);
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

	delete[] background;

	background = new fb_pixel_t[full_width*full_height];
	
	if(background)
		frameBuffer->saveScreen(x, y, full_width, full_height, background);
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

int ClistBoxWidget::exec(CMenuTarget* parent, const std::string&)
{
	dprintf(DEBUG_DEBUG, "ClistBoxWidget::exec:\n");

	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	int pos = 0;
	exit_pressed = false;
	int cnt = 0;

	if (parent)
		parent->hide();

	//
	initFrames();

	//
	if(savescreen) 
		saveScreen();

	paintHead();
	paintFoot();
	paint();

	frameBuffer->blit();

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);

	int retval = menu_return::RETURN_REPAINT;
	uint64_t timeoutEnd = CRCInput::calcTimeoutEnd(timeout == 0 ? 0xFFFF : timeout);

	//control loop
	do {
		g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd);
		
		int handled = false;

		dprintf(DEBUG_DEBUG, "ClistBoxWidget::exec: msg:%s\n", CRCInput::getSpecialKeyName(msg));

		if ( msg <= CRCInput::RC_MaxRC ) 
		{
			timeoutEnd = CRCInput::calcTimeoutEnd(timeout == 0 ? 0xFFFF : timeout);
			std::map<neutrino_msg_t, keyAction>::iterator it = keyActionMap.find(msg);
			
			if (it != keyActionMap.end()) 
			{
				int rv = it->second.menue->exec(this, it->second.action);
				switch ( rv ) 
				{
					case menu_return::RETURN_EXIT_ALL:
						retval = menu_return::RETURN_EXIT_ALL;
					case menu_return::RETURN_EXIT:
						msg = RC_timeout;
						break;
					case menu_return::RETURN_REPAINT:
						paintHead();
						paintFoot();
						paint();
						break;
				}

				frameBuffer->blit();
				continue;
			}

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
						if(current_page) 
						{
							pos = (int) page_start[current_page] - 1;

							selected = pos;
							paintItems();
						}
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
					else if (widgetType == WIDGET_TYPE_EXTENDED)
					{
						textBox->scrollPageUp(1);
					}
					
					break;
					
				case (RC_right):
					if(widgetType == WIDGET_TYPE_FRAME)
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
					else if (widgetType == WIDGET_TYPE_EXTENDED)
					{
						textBox->scrollPageDown(1);
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
									paintHead();
									paintFoot();
									paint();
									break;

								case menu_return::RETURN_NONE:
									g_RCInput->killTimer(sec_timer_id);
									sec_timer_id = 0;
									retval = menu_return::RETURN_NONE;
									msg = RC_timeout;
									break;	
							}
						} 
						else
							msg = RC_timeout;
					}
					break;
				//
				case (RC_setup):
					if(widgetChange && widget.size())
					{
						hide();

						cnt++;

						if(cnt >= (int)widget.size())
						{
							cnt = 0;
						}
					
						widgetType = widget[cnt];

						initFrames();
						paintHead();
						paintFoot();
						paint();
					}
					break;

				case (RC_home):
					exit_pressed = true;
					msg = RC_timeout;
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

			if ( msg <= CRCInput::RC_MaxRC )
			{
				// recalculate timeout for RC-Tasten
				timeoutEnd = CRCInput::calcTimeoutEnd(timeout == 0 ? 0xFFFF : timeout);
			}
		}
		
		frameBuffer->blit();
	}
	while ( msg != RC_timeout );
	
	if(retval != menu_return::RETURN_NONE)
		hide();

	//test
	if (background) 
	{
		delete[] background;
		background = NULL;
	}		

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



