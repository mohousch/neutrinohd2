/*
	Neutrino-GUI  -   DBoxII-Project

	$id: satip_setup.cpp 2016.01.14 11:40:30 mohousch $
	
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

#include <stdio.h> 

#include <global.h>
#include <neutrino.h>

#include <gui/widget/icons.h>
#include <gui/widget/stringinput_ext.h>

#include <gui/satip_setup.h>

#include <system/debug.h>
#include <system/settings.h>


#define MESSAGEBOX_NO_YES_OPTION_COUNT 2
const CMenuOptionChooser::keyval MESSAGEBOX_NO_YES_OPTIONS[MESSAGEBOX_NO_YES_OPTION_COUNT] =
{
	{ 0, LOCALE_MESSAGEBOX_NO, NULL },
	{ 1, LOCALE_MESSAGEBOX_YES, NULL }
};

#define SATIP_SERVERBOX_TYPE_OPTION_COUNT 3
const CMenuOptionChooser::keyval SATIP_SERVERBOX_TYPE_OPTIONS[SATIP_SERVERBOX_TYPE_OPTION_COUNT] =
{
	{ DVB_C, NONEXISTANT_LOCALE, "Cable" },
	{ DVB_S, NONEXISTANT_LOCALE, "Sat" },
	{ DVB_T, NONEXISTANT_LOCALE, "Terrestrial" }
};

CSatIPSetup::CSatIPSetup()
{
}

CSatIPSetup::~CSatIPSetup()
{
}

int CSatIPSetup::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CSatIPSetup::exec: actionKey:%s\n", actionKey.c_str());
	
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

void CSatIPSetup::showMenu()
{
	dprintf(DEBUG_NORMAL, "CSatIPSetup::showMenu\n");
	
	CMenuWidget satIP("Sat <> IP", NEUTRINO_ICON_SETTINGS);

	int shortcut = 1;

	// intros
	satIP.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));

	satIP.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	satIP.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));

	// items
	satIP.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	// allow satip on/off
	satIP.addItem(new CMenuOptionChooser("use Sat <> IP", &g_settings.satip_allow_satip, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcut++) ));
	// server box ip
	CIPInput * satip_IP = new CIPInput(LOCALE_STREAMINGMENU_SERVER_IP, g_settings.satip_serverbox_ip);
	satIP.addItem(new CMenuForwarder("Server Box IP", true, g_settings.satip_serverbox_ip, satip_IP, NULL, CRCInput::convertDigitToKey(shortcut++)));

	// server type (sat/cable/terrestrial)
	satIP.addItem(new CMenuOptionChooser("Server Box type", &g_settings.satip_serverbox_type, SATIP_SERVERBOX_TYPE_OPTIONS, SATIP_SERVERBOX_TYPE_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcut++), "", true ));
	
	satIP.exec(NULL, "");
	satIP.hide();

	delete satip_IP;
	satip_IP = NULL;
}

