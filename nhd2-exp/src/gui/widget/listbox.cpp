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
#include <gui/widget/stringinput.h> // locked menu

#include <driver/color.h>

#include <driver/fontrenderer.h>
#include <driver/rcinput.h>

#include <system/debug.h>


static CTextBox * textBox = NULL;

///
// CMenuItem
CMenuItem::CMenuItem()
{
	x = -1;
	directKey = RC_nokey;
	iconName = "";
	can_arrow = false;
	itemIcon = "";
	itemName = "";
	option = "";
	optionInfo = "";
	itemHelpText = "";
	itemIcon = "";
	info1 = "";
	option_info1 = "";
	info2 = "";
	option_info2 = "";

	icon1 = "";
	icon2 = "";

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
CMenuOptionChooser::CMenuOptionChooser(const neutrino_locale_t OptionName, int *const OptionValue, const struct keyval* const Options, const unsigned Number_Of_Options, const bool Active, CChangeObserver* const Observ, const neutrino_msg_t DirectKey, const std::string& IconName, bool Pulldown)
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

	itemHelpText = optionNameString;

	itemType = ITEM_TYPE_OPTION_CHOOSER;
}

CMenuOptionChooser::CMenuOptionChooser(const char * const OptionName, int* const OptionValue, const struct keyval *const Options, const unsigned Number_Of_Options, const bool Active, CChangeObserver* const Observ, const neutrino_msg_t DirectKey, const std::string & IconName, bool Pulldown)
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

int CMenuOptionChooser::exec(CMenuTarget *parent)
{
	dprintf(DEBUG_DEBUG, "CMenuOptionChooser::exec:\n");

	bool wantsRepaint = false;
	int ret = menu_return::RETURN_REPAINT;
	
	if (parent)
		parent->hide();

	// pulldown
	if( (!parent || msg == RC_ok) && pulldown ) 
	{
		int select = -1;

		ClistBoxWidget *menu = new ClistBoxWidget(optionNameString.c_str(), NEUTRINO_ICON_SETTINGS);

		menu->setMode(MODE_MENU);
		menu->enableShrinkMenu();
		menu->enableSaveScreen();
		
		//if(parent)
		//	menu->move(20, 0);

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
			
			menu->addItem(new CMenuForwarder(l_option), selected);
		}
		
		menu->exec(NULL, "");
		ret = menu_return::RETURN_REPAINT;

		select = menu->getSelected();
		
		if(select >= 0) 
			*optionValue = options[select].key;
		
		delete menu;
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

	return menu_return::RETURN_REPAINT;
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
CMenuOptionStringChooser::CMenuOptionStringChooser(const neutrino_locale_t Name, char * OptionValue, bool Active, CChangeObserver* Observ, const neutrino_msg_t DirectKey, const std::string & IconName, bool Pulldown)
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

	itemHelpText = nameString;
	itemType = ITEM_TYPE_OPTION_STRING_CHOOSER;
}

CMenuOptionStringChooser::CMenuOptionStringChooser(const char * const Name, char * OptionValue, bool Active, CChangeObserver* Observ, const neutrino_msg_t DirectKey, const std::string & IconName, bool Pulldown)
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

int CMenuOptionStringChooser::exec(CMenuTarget *parent)
{
	dprintf(DEBUG_DEBUG, "CMenuOptionStringChooser::exec:\n");

	bool wantsRepaint = false;
	int ret = menu_return::RETURN_REPAINT;

	if (parent)
		parent->hide();

	// pulldown
	if( (!parent || msg == RC_ok) && pulldown ) 
	{
		int select = -1;
		
		ClistBoxWidget * menu = new ClistBoxWidget(nameString.c_str(), NEUTRINO_ICON_SETTINGS);
		
		//if(parent) 
		//	menu->move(20, 0);
		
		menu->setMode(MODE_MENU);
		menu->enableSaveScreen();
		
		for(unsigned int count = 0; count < options.size(); count++) 
		{
			bool selected = false;
			if (strcmp(options[count].c_str(), optionValue) == 0)
				selected = true;

			menu->addItem(new CMenuForwarder(options[count].c_str()), selected);
		}
		menu->exec(NULL, "");
		ret = menu_return::RETURN_REPAINT;

		select = menu->getSelected();
		
		if(select >= 0)
			strcpy(optionValue, options[select].c_str());
		delete menu;
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

	//FIXME:	
	if ( wantsRepaint )
		return menu_return::RETURN_REPAINT;
	else
		return menu_return::RETURN_EXIT;
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

	if(widgetType != WIDGET_TYPE_FRAME)
	{
		frameBuffer->paintBoxRel(x, y, dx, height, /*item_backgroundColor*/COL_MENUCONTENT_PLUS_0);

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

// CMenuForwarder
CMenuForwarder::CMenuForwarder(const neutrino_locale_t Text, const bool Active, const char * const Option, CMenuTarget* Target, const char * const ActionKey, neutrino_msg_t DirectKey, const char * const IconName, const char * const ItemIcon, const neutrino_locale_t HelpText )
{
	option = Option? Option : "";

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

	optionValueString = "";
}

CMenuForwarder::CMenuForwarder(const char * const Text, const bool Active, const char * const Option, CMenuTarget* Target, const char * const ActionKey, neutrino_msg_t DirectKey, const char * const IconName, const char * const ItemIcon, const neutrino_locale_t HelpText )
{
	textString = Text? Text : "";
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
	itemName = Text? Text : "";

	optionValueString = "";
}

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

	if(!option.empty())
                tw += g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(option.c_str(), true);

	return tw;
}

int CMenuForwarder::exec(CMenuTarget *parent)
{
	dprintf(DEBUG_DEBUG, "CMenuForwarder::exec: actionKey:%s\n", actionKey.c_str());

	if(jumpTarget)
	{
		int ret = jumpTarget->exec(parent, actionKey);

		if(ret && !option.empty()) 
		{
			optionValueString = jumpTarget->getString().c_str();
		}

		return ret;
	}
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
	if(!optionValueString.empty())
		return optionValueString.c_str();
	else if(!option.empty())
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
	const char * option_text = getOption();	

	uint8_t color = COL_MENUCONTENT;
	fb_pixel_t bgcolor = marked? COL_MENUCONTENTSELECTED_PLUS_2 : COL_MENUCONTENT_PLUS_0;

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
		frameBuffer->paintBoxRel(x, y, item_width, item_height, /*item_backgroundColor*/COL_MENUCONTENT_PLUS_0);

		if(!itemIcon.empty())
			frameBuffer->displayImage(itemIcon, x + 4*ICON_OFFSET, y + 4*ICON_OFFSET, item_width - 8*ICON_OFFSET, item_height - 8*ICON_OFFSET);

		//
		if(selected)
		{
			frameBuffer->paintBoxRel(x, y, item_width, item_height, /*item_selectedColor*/COL_MENUCONTENTSELECTED_PLUS_0);

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
		int stringstartposX = x + (offx == 0? 0 : offx);

		//const char * option_text = getOption();	
	
		// paint item
		frameBuffer->paintBoxRel(x, y, dx, height, bgcolor);

		// iconName
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
			icon_w = ITEM_ICON_W_MINI;
			icon_h = ITEM_ICON_H_MINI;

			if (!itemIcon.empty())
			{
				frameBuffer->displayImage(itemIcon.c_str(), x + BORDER_LEFT, y + ((height - icon_h)/2), icon_w, icon_h);
			}
		}
	
		//local-text
		stringstartposX = x + BORDER_LEFT + icon_w + ICON_OFFSET;

		g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(stringstartposX, y + (height - g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight(), dx - BORDER_RIGHT - BORDER_LEFT - icon_w - ICON_OFFSET, l_text, color, 0, true); // UTF-8

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
int CLockedMenuForwarder::exec(CMenuTarget * parent)
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
ClistBoxItem::ClistBoxItem(const neutrino_locale_t Text, const bool Active, const char * const Option, CMenuTarget* Target, const char * const ActionKey, const neutrino_msg_t DirectKey, const char * const IconName, const char * const ItemIcon)
{
	text = Text;
	textString = g_Locale->getText(Text);

	option = Option? Option : "";

	active = Active;
	jumpTarget = Target;
	actionKey = ActionKey ? ActionKey : "";

	directKey = DirectKey;

	iconName = IconName ? IconName : "";
	itemIcon = ItemIcon? ItemIcon : "";
	itemName = g_Locale->getText(Text);
	itemType = ITEM_TYPE_LISTBOX;
}

ClistBoxItem::ClistBoxItem(const char * const Text, const bool Active, const char * const Option, CMenuTarget* Target, const char * const ActionKey, const neutrino_msg_t DirectKey, const char * const IconName, const char* const ItemIcon)
{
	text = NONEXISTANT_LOCALE;
	textString = Text? Text : "";

	option = Option? Option : "";

	active = Active;
	jumpTarget = Target;
	actionKey = ActionKey ? ActionKey : "";

	directKey = DirectKey;

	iconName = IconName ? IconName : "";
	itemIcon = ItemIcon? ItemIcon : "";
	itemName = Text? Text : "";
	itemType = ITEM_TYPE_LISTBOX;
}

int ClistBoxItem::getHeight(void) const
{
	int iw = 0;
	int ih = 0;

	if(widgetType == WIDGET_TYPE_FRAME)
	{
		return item_height;
	}
	else if(widgetType == WIDGET_TYPE_CLASSIC)
	{
		return std::max(ITEM_ICON_H_MINI, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight()) + 6;
	}
	else
	{
		if(nLinesItem)
			return (ITEM_ICON_H_MINI + 6);
		else
		{
			CFrameBuffer::getInstance()->getIconSize(iconName.c_str(), &iw, &ih);
	
			return std::max(ih, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight()) + 6;
		}
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
	{
		int ret = jumpTarget->exec(parent, actionKey);

		if(ret) 
		{
			optionValueString = jumpTarget->getString().c_str();
		}

		return ret;
	}
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

const char * ClistBoxItem::getOption(void)
{
	if(!optionValueString.empty())
		return optionValueString.c_str();
	else if(!option.empty())
		return option.c_str();
	else
		return NULL;
}

int ClistBoxItem::paint(bool selected, bool /*AfterPulldown*/)
{
	dprintf(DEBUG_DEBUG, "ClistBoxItem::paint:\n");

	CFrameBuffer * frameBuffer = CFrameBuffer::getInstance();

	int height = getHeight();
	const char * l_text = getName();
	const char * option_text = getOption();	

	uint8_t color = COL_MENUCONTENT;
	fb_pixel_t bgcolor = marked? COL_MENUCONTENTSELECTED_PLUS_2 : /*item_backgroundColor*/COL_MENUCONTENT_PLUS_0;

	if (selected)
	{
		color = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
	}

	if(widgetType == WIDGET_TYPE_FRAME)
	{
		//
		frameBuffer->paintBoxRel(x, y, item_width, item_height, /*item_backgroundColor*/COL_MENUCONTENT_PLUS_0);

		if(!itemIcon.empty())
			frameBuffer->displayImage(itemIcon, x + 4*ICON_OFFSET, y + 4*ICON_OFFSET, item_width - 8*ICON_OFFSET, item_height - 8*ICON_OFFSET);

		//
		if(selected)
		{
			frameBuffer->paintBoxRel(x, y, item_width, item_height, /*item_selectedColor*/COL_MENUCONTENTSELECTED_PLUS_0);

			if(!itemIcon.empty())
				frameBuffer->displayImage(itemIcon, x + ICON_OFFSET/2, y + ICON_OFFSET/2, item_width - ICON_OFFSET, item_height - ICON_OFFSET);

		}

		// locale ???

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
		else if(widgetType == WIDGET_TYPE_STANDARD)
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

			if(nLinesItem)
			{
				// local
				if(l_text != NULL)
				{
					/*nameFont*/g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x + BORDER_LEFT + icon_w + numwidth + ICON_OFFSET + pBarWidth + ICON_OFFSET, y + 3 + /*nameFont*/g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight(), dx - BORDER_RIGHT - BORDER_LEFT - numwidth - pBarWidth - 2*ICON_OFFSET - icon_w - icon1_w - icon2_w - optionInfo_width - ICON_OFFSET, l_text, color, 0, true); // UTF-8
				}

				// option
				if(option_text != NULL)
				{
					/*optionFont*/g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(x + BORDER_LEFT + icon_w + numwidth + ICON_OFFSET + pBarWidth + ICON_OFFSET, y + height, dx - BORDER_LEFT - BORDER_RIGHT - numwidth - pBarWidth - 2*ICON_OFFSET - icon_w - icon1_w - icon2_w - optionInfo_width - ICON_OFFSET, option_text, color, 0, true);
				}
			}
			else
			{
				// local
				if(l_text != NULL)
				{
					/*nameFont*/g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x + BORDER_LEFT + icon_w/2 + numwidth + ICON_OFFSET + pBarWidth + ICON_OFFSET, y + 3 + /*nameFont*/g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight(), dx - BORDER_RIGHT - BORDER_LEFT - numwidth - pBarWidth - 2*ICON_OFFSET - icon_w - icon1_w - icon2_w - optionInfo_width - ICON_OFFSET, l_text, color, 0, true); // UTF-8
				}

				// option
				std::string Option;
			
				if(option_text != NULL)
				{
					int iw, ih;
					//get icon size
					frameBuffer->getIconSize(NEUTRINO_ICON_HD, &iw, &ih);

					Option = " - ";
					Option += option_text;

					/*optionFont*/g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString(x + BORDER_LEFT + numwidth + pBarWidth + ICON_OFFSET + l_text_width + ICON_OFFSET, y + (height - /*optionFont*/g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight())/2 + /*optionFont*/g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight(), dx - BORDER_LEFT - BORDER_RIGHT - numwidth - ICON_OFFSET - pBarWidth - ICON_OFFSET - l_text_width - icon_w - icon1_w - ICON_OFFSET - icon2_w - ICON_OFFSET - 2*iw, Option.c_str(), COL_COLORED_EVENTS_CHANNELLIST, 0, true);
				}
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
				if(option_text != NULL)
				{
					/*optionFont*/g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(x + BORDER_LEFT + icon_w + numwidth + ICON_OFFSET + pBarWidth + ICON_OFFSET, y + height, dx - BORDER_LEFT - BORDER_RIGHT - numwidth - pBarWidth - 2*ICON_OFFSET - icon_w - icon1_w - icon2_w - optionInfo_width - ICON_OFFSET, option_text, color, 0, true);
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
			
				if(option_text != NULL)
				{
					int iw, ih;
					//get icon size
					frameBuffer->getIconSize(NEUTRINO_ICON_HD, &iw, &ih);

					Option = " - ";
					Option += option_text;

					/*optionFont*/g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(x + BORDER_LEFT + icon_w + numwidth + pBarWidth + ICON_OFFSET + l_text_width + ICON_OFFSET, y + (height - /*optionFont*/g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight())/2 + /*optionFont*/g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight(), dx - BORDER_LEFT - BORDER_RIGHT - numwidth - ICON_OFFSET - pBarWidth - ICON_OFFSET - l_text_width - icon_w - icon1_w - ICON_OFFSET - icon2_w - ICON_OFFSET - 2*iw, Option.c_str(), COL_COLORED_EVENTS_CHANNELLIST, 0, true);
				}
			}
		}
		else if(widgetType == WIDGET_TYPE_STANDARD)// standard
		{
			//
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
				if(option_text != NULL)
				{
					/*optionFont*/g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(x + BORDER_LEFT + icon_w + numwidth + ICON_OFFSET + pBarWidth + ICON_OFFSET, y + height, dx - BORDER_LEFT - BORDER_RIGHT - numwidth - pBarWidth - 2*ICON_OFFSET - icon_w - icon1_w - icon2_w - optionInfo_width - ICON_OFFSET, option_text, color, 0, true);
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
			
				if(option_text != NULL)
				{
					int iw, ih;
					//get icon size
					frameBuffer->getIconSize(NEUTRINO_ICON_HD, &iw, &ih);

					Option = " - ";
					Option += option_text;

					/*optionFont*/g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString(x + BORDER_LEFT + numwidth + pBarWidth + ICON_OFFSET + l_text_width + ICON_OFFSET, y + (height - /*optionFont*/g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight())/2 + /*optionFont*/g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight(), dx - BORDER_LEFT - BORDER_RIGHT - numwidth - ICON_OFFSET - pBarWidth - ICON_OFFSET - l_text_width - icon_w - icon1_w - ICON_OFFSET - icon2_w - ICON_OFFSET - 2*iw, Option.c_str(), COL_COLORED_EVENTS_CHANNELLIST, 0, true);
				}
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

//// ClistBox
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
	cFrameFootInfo.iHeight = 0;
	connectLineWidth = 0;

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
	enableCenter = false;
	outFocus = false;
	shrinkMenu = false;

	//
	//backgroundColor = COL_MENUCONTENT_PLUS_0;

	//
	//itemBoxColor = COL_MENUCONTENTSELECTED_PLUS_0;
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
	cnt = 0;

	itemType = WIDGET_ITEM_LISTBOX;

	savescreen = false;
	background = NULL;
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
	cFrameFootInfo.iHeight = 0;
	connectLineWidth = 0;

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
	enableCenter = false;
	outFocus = false;
	shrinkMenu = false;

	//
	//backgroundColor = COL_MENUCONTENT_PLUS_0;

	//
	//itemBoxColor = COL_MENUCONTENTSELECTED_PLUS_0;
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
	cnt = 0;

	itemType = WIDGET_ITEM_LISTBOX;

	savescreen = false;
	background = NULL;
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
		//item->item_backgroundColor = backgroundColor;
		//item->item_selectedColor = itemBoxColor;
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
		if(widgetType == WIDGET_TYPE_FRAME)
		{
			cFrameFootInfo.iHeight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight() + 6;
		}
		else
		{
			cFrameFootInfo.iHeight = footInfoHeight;
			connectLineWidth = CONNECTLINEBOX_WIDTH;
		}
	}

	// init frames
	if(widgetType == WIDGET_TYPE_FRAME)
	{
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
		item_height = (cFrameBox.iHeight - hheight - fheight - cFrameFootInfo.iHeight - 20)/itemsPerY; // 20 pixels for hlines

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
			if(widgetType == WIDGET_TYPE_EXTENDED || (widgetType == WIDGET_TYPE_CLASSIC && widgetMode == MODE_MENU) || widgetMode == MODE_SETUP)
			{
				cFrameFootInfo.iHeight = 0;
				connectLineWidth = 0;
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

			if((heightCurrPage + hheight + fheight + cFrameFootInfo.iHeight) > cFrameBox.iHeight)
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
			listmaxshow = (cFrameBox.iHeight - hheight - fheight - cFrameFootInfo.iHeight)/item_height;
			cFrameBox.iHeight = hheight + listmaxshow*item_height + fheight + cFrameFootInfo.iHeight;
		}

		// sanity check
		if(cFrameBox.iHeight > ((int)frameBuffer->getScreenHeight()))
			cFrameBox.iHeight = frameBuffer->getScreenHeight();

		// sanity check
		if(cFrameBox.iWidth > (int)frameBuffer->getScreenWidth())
			cFrameBox.iWidth = frameBuffer->getScreenWidth();

		if(paintFootInfo)
		{
			cFrameBox.iWidth -= connectLineWidth;
		}

		// position xy
		if(enableCenter)
		{
			cFrameBox.iX = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - cFrameBox.iWidth ) >> 1 );
			cFrameBox.iY = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - cFrameBox.iHeight) >> 1 );
		}
	}

	if(savescreen) 
		saveScreen();
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
		frameBuffer->paintBoxRel(cFrameBox.iX, cFrameBox.iY + hheight, cFrameBox.iWidth, cFrameBox.iHeight - hheight - fheight, /*backgroundColor*/COL_MENUCONTENT_PLUS_0);

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

					if((item->isSelectable()) && (selected == -1)) 
					{
						selected = count;
					} 

					if (selected == (signed int)count) 
					{
						paintItemInfo(count);
					}

					if(outFocus)
						item->paint(false);
					else
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
		items_height = cFrameBox.iHeight - hheight - fheight - cFrameFootInfo.iHeight; 

		sb_width = 0;
	
		if(total_pages > 1)
			sb_width = SCROLLBAR_WIDTH;

		items_width = cFrameBox.iWidth - sb_width;

		// extended
		if(widgetType == WIDGET_TYPE_EXTENDED)
		{
			items_width = 2*(cFrameBox.iWidth/3) - sb_width;

			// extended
			if(textBox)
			{
				delete textBox;
				textBox = NULL;
			}

			textBox = new CTextBox(cFrameBox.iX + 2*(cFrameBox.iWidth/3), cFrameBox.iY + hheight, (cFrameBox.iWidth/3), items_height);
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
		frameBuffer->paintBoxRel(cFrameBox.iX, cFrameBox.iY + hheight, cFrameBox.iWidth, cFrameBox.iHeight - hheight - fheight - cFrameFootInfo.iHeight, /*backgroundColor*/COL_MENUCONTENT_PLUS_0);

		if(widgetType == WIDGET_TYPE_EXTENDED && widgetMode == MODE_MENU)
		{
			frameBuffer->paintBoxRel(cFrameBox.iX + items_width, cFrameBox.iY + hheight, cFrameBox.iWidth - items_width, items_height, COL_MENUCONTENTDARK_PLUS_0);
		}
	
		// paint right scrollBar if we have more then one page
		if(total_pages > 1)
		{
			if(widgetType == WIDGET_TYPE_EXTENDED)
				scrollBar.paint(cFrameBox.iX + 2*(cFrameBox.iWidth/3) - SCROLLBAR_WIDTH, cFrameBox.iY + hheight, cFrameBox.iHeight - hheight - fheight - cFrameFootInfo.iHeight, total_pages, current_page);
			else
				scrollBar.paint(cFrameBox.iX + cFrameBox.iWidth - SCROLLBAR_WIDTH, cFrameBox.iY + hheight, cFrameBox.iHeight - hheight - fheight - cFrameFootInfo.iHeight, total_pages, current_page);
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
			
				if((item->isSelectable()) && (selected == -1)) 
				{
					selected = count;
				} 

				if (selected == (signed int)count) 
				{
					paintItemInfo(count);
				}

				if(outFocus)
					ypos = item->paint(false);
				else
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
			frameBuffer->paintBoxRel(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, hheight, /*backgroundColor*/COL_MENUCONTENT_PLUS_0);

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
			CHeaders headers(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, hheight, l_name.c_str(), iconfile.c_str());

			headers.setColor(headColor);
			headers.setCorner(headRadius, headCorner);
			headers.setGradient(headGradient);
		
			if(paintDate)
				headers.enablePaintDate();

			if(logo)
				headers.enableLogo();

			headers.setButtons(hbutton_labels, hbutton_count);

			headers.paint();
		}
	}	
}

void ClistBox::paintFoot()
{
	if(paint_Foot)
	{
		if(widgetType == WIDGET_TYPE_FRAME)
		{
			frameBuffer->paintBoxRel(cFrameBox.iX, cFrameBox.iY + cFrameBox.iHeight - fheight, cFrameBox.iWidth, fheight, /*backgroundColor*/COL_MENUCONTENT_PLUS_0);

			// paint horizontal line buttom
			frameBuffer->paintHLineRel(cFrameBox.iX + BORDER_LEFT, cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT, cFrameBox.iY + cFrameBox.iHeight - fheight + 2, COL_MENUCONTENT_PLUS_5);

			// buttons
			buttons.paintFootButtons(cFrameBox.iX, cFrameBox.iY + cFrameBox.iHeight - fheight, cFrameBox.iWidth, fheight, fbutton_count, fbutton_labels);
		}
		else
		{
			CFooters footers(cFrameBox.iX, cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight - fheight, cFrameBox.iWidth, fheight, fbutton_count, fbutton_labels);

			footers.setColor(footColor);
			footers.setCorner(footRadius, footCorner);
			footers.setGradient(footGradient);

			footers.paint();
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
				if(textBox)
				{
					delete textBox;
					textBox = NULL;
				}
	
				textBox = new CTextBox(cFrameBox.iX + 100 + ICON_OFFSET, cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight + 2, cFrameBox.iWidth - 100 - ICON_OFFSET - 2, cFrameFootInfo.iHeight - 4);
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
			// refresh
			frameBuffer->paintBoxRel(cFrameBox.iX, cFrameBox.iY + cFrameBox.iHeight - fheight - cFrameFootInfo.iHeight, cFrameBox.iWidth, cFrameFootInfo.iHeight, /*backgroundColor*/COL_MENUCONTENT_PLUS_0);

			// refresh horizontal line buttom
			frameBuffer->paintHLineRel(cFrameBox.iX + BORDER_LEFT, cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT, cFrameBox.iY + cFrameBox.iHeight - fheight - cFrameFootInfo.iHeight + 2, COL_MENUCONTENT_PLUS_5);

			if(items.size() > 0)
			{
				CMenuItem* item = items[pos];
	
				// itemName
				if(!item->itemName.empty())
				{
					g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBox.iHeight - fheight - cFrameFootInfo.iHeight + (cFrameFootInfo.iHeight - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE] ->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT, item->itemName.c_str(), COL_MENUFOOT_INFO);
				}

				// helpText
				if(!item->itemHelpText.empty())
				{
					g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBox.iHeight - fheight, cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT, item->itemHelpText.c_str(), COL_MENUFOOT_INFO);
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
		itemsLine.clear(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth + CONNECTLINEBOX_WIDTH, cFrameBox.iHeight - cFrameFootInfo.iHeight, cFrameFootInfo.iHeight);
}

void ClistBox::saveScreen()
{
	if(!savescreen)
		return;

	if(background)
	{
		delete[] background;
		background = NULL;
	}

	background = new fb_pixel_t[cFrameBox.iWidth*cFrameBox.iHeight];
	
	if(background)
	{
		frameBuffer->saveScreen(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight, background);
	}
}

void ClistBox::restoreScreen()
{
	if(background) 
	{
		if(savescreen)
			frameBuffer->restoreScreen(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight, background);
	}
}

void ClistBox::enableSaveScreen()
{
	savescreen = true;
	
	if(!savescreen && background) 
	{
		delete[] background;
		background = NULL;
	}
}

void ClistBox::hide()
{
	dprintf(DEBUG_NORMAL, "ClistBox::hide:\n");

	if( savescreen && background)
		restoreScreen();
	else
		frameBuffer->paintBackgroundBoxRel(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight);

	hideItemInfo(); 
	
	frameBuffer->blit();

	if(textBox != NULL)
	{
		delete textBox;
		textBox = NULL;
	}
}

void ClistBox::scrollLineDown(const int)
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

void ClistBox::scrollLineUp(const int)
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

void ClistBox::scrollPageDown(const int)
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

void ClistBox::scrollPageUp(const int)
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
	}
	else if (widgetType == WIDGET_TYPE_EXTENDED)
	{
		if(textBox)
			textBox->scrollPageDown(1);
	}
}

void ClistBox::changeWidgetType(int)
{
	dprintf(DEBUG_NORMAL, "ClistBox::changeWidgetType:\n");

	printf("cnt:%d\n", cnt);

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

//
int ClistBox::oKKeyPressed(CMenuTarget* parent)
{
	dprintf(DEBUG_NORMAL, "ClistBox::okKeyPressed:\n");

	if(parent)
		if(hasItem())
			return items[selected]->exec(parent);
		else
			menu_return::RETURN_EXIT;
	else
		return menu_return::RETURN_EXIT;
}

void ClistBox::onUpKeyPressed()
{
	dprintf(DEBUG_DEBUG, "ClistBox::UpKeyPressed:\n");

	scrollLineUp();
}

void ClistBox::onDownKeyPressed()
{
	dprintf(DEBUG_DEBUG, "ClistBox::DownKeyPressed:\n");

	scrollLineDown();
}

void ClistBox::onRightKeyPressed()
{
	dprintf(DEBUG_DEBUG, "ClistBox::RightKeyPressed:\n");

	swipRight();
}

void ClistBox::onLeftKeyPressed()
{
	dprintf(DEBUG_DEBUG, "ClistBox::LeftKeyPressed:\n");

	swipLeft();
}

void ClistBox::onPageUpKeyPressed()
{
	dprintf(DEBUG_DEBUG, "ClistBox::PageUpKeyPressed:\n");

	scrollPageUp();
}

void ClistBox::onPageDownKeyPressed()
{
	dprintf(DEBUG_DEBUG, "ClistBox::PageDownKeyPressed:\n");

	scrollPageDown();
}


