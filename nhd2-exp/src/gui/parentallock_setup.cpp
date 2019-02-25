/*
	Neutrino-GUI  -   DBoxII-Project

	$id: parentallock_setup.cpp 2016.01.02 20:10:30 mohousch $
	
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
#include <gui/parentallock_setup.h>

#include <system/debug.h>
#include <system/setting_helpers.h>
#include <system/helpers.h>


extern bool parentallocked;			// defined neutrino.cpp

#define PARENTALLOCK_PROMPT_OPTION_COUNT 3
const CMenuOptionChooser::keyval PARENTALLOCK_PROMPT_OPTIONS[PARENTALLOCK_PROMPT_OPTION_COUNT] =
{
	{ PARENTALLOCK_PROMPT_NEVER         , LOCALE_PARENTALLOCK_NEVER, NULL         },
	{ PARENTALLOCK_PROMPT_CHANGETOLOCKED, LOCALE_PARENTALLOCK_CHANGETOLOCKED, NULL },
	{ PARENTALLOCK_PROMPT_ONSIGNAL      , LOCALE_PARENTALLOCK_ONSIGNAL, NULL       }
};

#define PARENTALLOCK_LOCKAGE_OPTION_COUNT 3
const CMenuOptionChooser::keyval PARENTALLOCK_LOCKAGE_OPTIONS[PARENTALLOCK_LOCKAGE_OPTION_COUNT] =
{
	{ 12, LOCALE_PARENTALLOCK_LOCKAGE12, NULL },
	{ 16, LOCALE_PARENTALLOCK_LOCKAGE16, NULL },
	{ 18, LOCALE_PARENTALLOCK_LOCKAGE18, NULL }
};

CParentalLockSettings::CParentalLockSettings()
{
}

CParentalLockSettings::~CParentalLockSettings()
{
}

int CParentalLockSettings::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CParentalLockSettings::exec: actionKey: %s\n", actionKey.c_str());
	
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

void CParentalLockSettings::showMenu()
{
	dprintf(DEBUG_NORMAL, "CParentalLockSettings::showMenu:\n");
	
	int shortcutLock = 1;
	
	CMenuWidget parentallockSettings(LOCALE_PARENTALLOCK_PARENTALLOCK, NEUTRINO_ICON_LOCK);
	
	// intro
	parentallockSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	parentallockSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	parentallockSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", RC_red, NEUTRINO_ICON_BUTTON_RED));
	parentallockSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	// prompt
	parentallockSettings.addItem(new CMenuOptionChooser(LOCALE_PARENTALLOCK_PROMPT, &g_settings.parentallock_prompt, PARENTALLOCK_PROMPT_OPTIONS, PARENTALLOCK_PROMPT_OPTION_COUNT, !parentallocked, NULL, CRCInput::convertDigitToKey(shortcutLock++), "", true ));

	// lockage
	parentallockSettings.addItem(new CMenuOptionChooser(LOCALE_PARENTALLOCK_LOCKAGE, &g_settings.parentallock_lockage, PARENTALLOCK_LOCKAGE_OPTIONS, PARENTALLOCK_LOCKAGE_OPTION_COUNT, !parentallocked, NULL, CRCInput::convertDigitToKey(shortcutLock++), "", true ));

	// Pin
	CPINChangeWidget * pinChangeWidget = new CPINChangeWidget(LOCALE_PARENTALLOCK_CHANGEPIN, g_settings.parentallock_pincode, 4, LOCALE_PARENTALLOCK_CHANGEPIN_HINT1);
	parentallockSettings.addItem( new CMenuForwarder(LOCALE_PARENTALLOCK_CHANGEPIN, true, g_settings.parentallock_pincode, pinChangeWidget, NULL, CRCInput::convertDigitToKey(shortcutLock++) ));
	
	parentallockSettings.exec(NULL, "");
	parentallockSettings.hide();
}


