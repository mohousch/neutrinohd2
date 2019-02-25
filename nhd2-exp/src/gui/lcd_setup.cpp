/*
	Neutrino-GUI  -   DBoxII-Project

	$id: lcd_setup.cpp 2016.01.02 21:24:30 mohousch $
	
	Copyright (C) 2001 Steffen Hehn 'McClean'
	and some other guys
	Homepage: http://dbox.cyberphoria.org/

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

#include <global.h>
#include <neutrino.h>

#include <stdio.h> 

#include <gui/widget/stringinput.h>

#include <gui/vfdcontroler.h>
#include <gui/lcd_setup.h>

#include <system/debug.h>
#include <system/setting_helpers.h>
#include <system/helpers.h>


#define OPTIONS_OFF0_ON1_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_OFF0_ON1_OPTIONS[OPTIONS_OFF0_ON1_OPTION_COUNT] =
{
        { 0, LOCALE_OPTIONS_OFF, NULL },
        { 1, LOCALE_OPTIONS_ON, NULL }
};

#if defined (ENABLE_LCD)
#define LCDMENU_STATUSLINE_OPTION_COUNT 4
const CMenuOptionChooser::keyval LCDMENU_STATUSLINE_OPTIONS[LCDMENU_STATUSLINE_OPTION_COUNT] =
{
	{ 0, LOCALE_LCDMENU_STATUSLINE_PLAYTIME, NULL   },
	{ 1, LOCALE_LCDMENU_STATUSLINE_VOLUME, NULL     },
	{ 2, LOCALE_LCDMENU_STATUSLINE_BOTH, NULL       },
	{ 3, LOCALE_LCDMENU_STATUSLINE_BOTH_AUDIO, NULL }
};

#define LCDMENU_EPG_OPTION_COUNT 6
const CMenuOptionChooser::keyval LCDMENU_EPG_OPTIONS[LCDMENU_EPG_OPTION_COUNT] =
{
	{ 1, LOCALE_LCDMENU_EPG_NAME, NULL		},
	{ 2, LOCALE_LCDMENU_EPG_TITLE, NULL		},
	{ 3, LOCALE_LCDMENU_EPG_NAME_TITLE, NULL	},
	{ 7, LOCALE_LCDMENU_EPG_NAME_SEPLINE_TITLE, NULL },
	{ 11, LOCALE_LCDMENU_EPG_NAMESHORT_TITLE, NULL },
	{ 15, LOCALE_LCDMENU_EPG_NAMESHORT_SEPLINE_TITLE, NULL }
};

#define LCDMENU_EPGALIGN_OPTION_COUNT 2
const CMenuOptionChooser::keyval LCDMENU_EPGALIGN_OPTIONS[LCDMENU_EPGALIGN_OPTION_COUNT] =
{
	{ 0, LOCALE_LCDMENU_EPGALIGN_LEFT, NULL   },
	{ 1, LOCALE_LCDMENU_EPGALIGN_CENTER, NULL }
};
#endif

#if defined (PLATFORM_GIGABLUE) 
#if !defined (ENABLE_LCD)
#define LCDMENU_LEDCOLOR_OPTION_COUNT 4
const CMenuOptionChooser::keyval LCDMENU_LEDCOLOR_OPTIONS[LCDMENU_LEDCOLOR_OPTION_COUNT] =
{
	{ CVFD::LED_OFF, LOCALE_OPTIONS_OFF, NULL },
	{ CVFD::LED_BLUE, LOCALE_LCDMENU_LEDCOLOR_BLUE, NULL },
	{ CVFD::LED_RED, LOCALE_LCDMENU_LEDCOLOR_RED, NULL },
	{ CVFD::LED_PURPLE, LOCALE_LCDMENU_LEDCOLOR_PURPLE, NULL },
};
#endif
#endif

CLCDSettings::CLCDSettings()
{
}

CLCDSettings::~CLCDSettings()
{
}

int CLCDSettings::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CLCDSettings::exec: actionKey: %s\n", actionKey.c_str());
	
	int ret = menu_return::RETURN_REPAINT;
	
	if(parent)
		parent->hide();
	
	if(actionKey == "savesettings")
	{
		CNeutrinoApp::getInstance()->exec(NULL, "savesettings");
		
		return ret;
	}
	
	showMenu();
	
	return ret;
}

void CLCDSettings::showMenu()
{
	dprintf(DEBUG_NORMAL, "CLCDSettings::showMenu:\n");
	
	CMenuWidget lcdSettings(LOCALE_LCDMENU_HEAD, NEUTRINO_ICON_LCD );
	
	int shortcutVFD = 1;
	
	// intros
	lcdSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	lcdSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	lcdSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", RC_red, NEUTRINO_ICON_BUTTON_RED));
	lcdSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	
	CLcdNotifier * lcdnotifier = new CLcdNotifier();
	
	CVfdControler * lcdsliders = new CVfdControler(LOCALE_LCDMENU_HEAD, NULL);
	
	// LCD
#if defined (ENABLE_LCD)
	//option invert
	CMenuOptionChooser* oj_inverse = new CMenuOptionChooser(LOCALE_LCDMENU_INVERSE, &g_settings.lcd_setting[SNeutrinoSettings::LCD_INVERSE], OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, lcdnotifier, CRCInput::convertDigitToKey(shortcutVFD++) );
	lcdSettings.addItem(oj_inverse);

	//status display
	CMenuOptionChooser* oj_status = new CMenuOptionChooser(LOCALE_LCDMENU_STATUSLINE, &g_settings.lcd_setting[SNeutrinoSettings::LCD_SHOW_VOLUME], LCDMENU_STATUSLINE_OPTIONS, LCDMENU_STATUSLINE_OPTION_COUNT, true);
	lcdSettings.addItem(oj_status);
	
	//lcd_epg
	CMenuOptionChooser* oj_epg = new CMenuOptionChooser(LOCALE_LCDMENU_EPG, &g_settings.lcd_setting[SNeutrinoSettings::LCD_EPGMODE], LCDMENU_EPG_OPTIONS, LCDMENU_EPG_OPTION_COUNT, true);
	lcdSettings.addItem(oj_epg);

	//align
	CMenuOptionChooser* oj_align = new CMenuOptionChooser(LOCALE_LCDMENU_EPGALIGN, &g_settings.lcd_setting[SNeutrinoSettings::LCD_EPGALIGN], LCDMENU_EPGALIGN_OPTIONS, LCDMENU_EPGALIGN_OPTION_COUNT, true);
	lcdSettings.addItem(oj_align);

	//dump to png
	CMenuOptionChooser* oj_dumppng = new CMenuOptionChooser(LOCALE_LCDMENU_DUMP_PNG, &g_settings.lcd_setting[SNeutrinoSettings::LCD_DUMP_PNG], OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true);
	lcdSettings.addItem(oj_dumppng);
	
	// lcd controller
	lcdSettings.addItem(new CMenuForwarder(LOCALE_LCDMENU_LCDCONTROLER, true, NULL, lcdsliders, NULL, CRCInput::convertDigitToKey(shortcutVFD++) ));
#else	
#if defined (PLATFORM_GIGABLUE)	
	// led color
	lcdSettings.addItem(new CMenuOptionChooser(LOCALE_LCDMENU_LEDCOLOR, &g_settings.lcd_ledcolor, LCDMENU_LEDCOLOR_OPTIONS, LCDMENU_LEDCOLOR_OPTION_COUNT, true, lcdnotifier, CRCInput::convertDigitToKey(shortcutVFD++) ));	
#elif !defined (PLATFORM_CUBEREVO_250HD) && !defined (PLATFORM_SPARK)
	// vfd power
	CMenuOptionChooser * oj2 = new CMenuOptionChooser(LOCALE_LCDMENU_POWER, &g_settings.lcd_setting[SNeutrinoSettings::LCD_POWER], OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, lcdnotifier, CRCInput::convertDigitToKey(shortcutVFD++) );
	lcdSettings.addItem(oj2);
	
	// dimm-time
	CStringInput * dim_time = new CStringInput(LOCALE_LCDMENU_DIM_TIME, g_settings.lcd_setting_dim_time, 3, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE,"0123456789 ");
	lcdSettings.addItem(new CMenuForwarder(LOCALE_LCDMENU_DIM_TIME,true, g_settings.lcd_setting_dim_time, dim_time, NULL, CRCInput::convertDigitToKey(shortcutVFD++)));

	// dimm brightness
	//CStringInput * dim_brightness = new CStringInput(LOCALE_LCDMENU_DIM_BRIGHTNESS, g_settings.lcd_setting_dim_brightness, 3,NONEXISTANT_LOCALE, NONEXISTANT_LOCALE,"0123456789 ");
	//lcdSettings.addItem(new CMenuForwarder(LOCALE_LCDMENU_DIM_BRIGHTNESS,true, g_settings.lcd_setting_dim_brightness, dim_brightness, NULL, CRCInput::convertDigitToKey(shortcutVFD++) ));

	// vfd controller
	lcdSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	lcdSettings.addItem(new CMenuForwarder(LOCALE_LCDMENU_LCDCONTROLER, true, NULL, lcdsliders, NULL, CRCInput::convertDigitToKey(shortcutVFD++) ));	
#endif	
#endif	
	
	lcdSettings.exec(NULL, "");
	lcdSettings.hide();
}

// lcd notifier
bool CLcdNotifier::changeNotify(const neutrino_locale_t, void * Data)
{
	int state = *(int *)Data;

	dprintf(DEBUG_NORMAL, "ClcdNotifier: state: %d\n", state);
	
#if defined (PLATFORM_GIGABLUE) && !defined (ENABLE_LCD)
	CVFD::getInstance()->vfd_led(state);
#else	
	CVFD::getInstance()->setPower(state);
	CVFD::getInstance()->setlcdparameter();
#endif	

	return true;
}


