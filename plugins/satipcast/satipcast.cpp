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
#include <gui/widget/hintbox.h>

#include <system/debug.h>
#include <system/settings.h>

#include <plugin.h>
#include <satipcast.h>


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

#define SATIP_SERVERBOX_GUI_OPTION_COUNT 2
const CMenuOptionChooser::keyval SATIP_SERVERBOX_GUI_OPTIONS[SATIP_SERVERBOX_GUI_OPTION_COUNT] =
{
	{ SNeutrinoSettings::SATIP_SERVERBOX_GUI_NMP, NONEXISTANT_LOCALE, "NeutrinoMP" },
	{ SNeutrinoSettings::SATIP_SERVERBOX_GUI_ENIGMA2, NONEXISTANT_LOCALE, "enigma2" }
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

	int rec = CNeutrinoApp::getInstance()->recordingstatus;
	int allow_ip = g_settings.satip_allow_satip;
	
	CMenuWidget satIP("Sat <> IP Cast", NEUTRINO_ICON_SETTINGS);

	int shortcut = 1;

	// intros
	satIP.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));

	satIP.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	satIP.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));

	// items
	satIP.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	// allow satip on/off
	CSatIPNotifier * satIPNotifier = new CSatIPNotifier;

	satIP.addItem(new CMenuOptionChooser("use Sat <> IP", &allow_ip, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, rec ? false : true, satIPNotifier, CRCInput::convertDigitToKey(shortcut++)));

	// server box ip
	CIPInput * satip_IP = new CIPInput(LOCALE_STREAMINGMENU_SERVER_IP, g_settings.satip_serverbox_ip);
	satIP.addItem(new CMenuForwarder("Server Box IP", true, g_settings.satip_serverbox_ip, satip_IP, NULL, CRCInput::convertDigitToKey(shortcut++)));

	// server gui (neutrino/neutrinohd/enigma2)
	satIP.addItem(new CMenuOptionChooser("Server Box GUI", &g_settings.satip_serverbox_gui, SATIP_SERVERBOX_GUI_OPTIONS, SATIP_SERVERBOX_GUI_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcut++), "", true ));

	// client box type (sat/cable/terrestrial)
	satIP.addItem(new CMenuOptionChooser("Client Box type", &g_settings.satip_serverbox_type, SATIP_SERVERBOX_TYPE_OPTIONS, SATIP_SERVERBOX_TYPE_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcut++), "", true ));

	if (rec)
		HintBox(LOCALE_MESSAGEBOX_INFO, LOCALE_SATIPCAST_REC_HINT, HINTBOX_WIDTH, 6);
	
	satIP.exec(NULL, "");
	satIP.hide();

	delete satip_IP;
	satip_IP = NULL;
	delete satIPNotifier;
}

// satipcast notifier
bool CSatIPNotifier::changeNotify(const neutrino_locale_t, void * Data)
{
	dprintf(DEBUG_NORMAL, "CSatIPNotifier::changeNotify\n");

	int allowip = *((int*) Data);
	int rec = CNeutrinoApp::getInstance()->recordingstatus;
	int mode = CNeutrinoApp::getInstance()->getMode() & 0xFF;

	if ((mode == 1 || mode == 2) && !rec)		// tv or radio and not recording
	{
		if (allowip)
		{
			if (mode == 1)
				CNeutrinoApp::getInstance()->StopSubtitles();

			//pause epg scanning
			g_Sectionsd->setPauseScanning(true);
			// stop playback and close video-, audiodecoder
			g_Zapit->lockPlayBack();
			// a little wait is needed here !! 
			sleep(1);
			g_settings.satip_allow_satip = allowip;
			g_Zapit->Rezap();
		}
		else
		{
			// stop ip-playback
			g_Zapit->stopPlayBack();
			g_settings.satip_allow_satip = allowip;
			// open video-, audiodecoder and start playback
			g_Zapit->unlockPlayBack();
			// start epg scanning
			g_Sectionsd->setPauseScanning(false);
			g_Zapit->Rezap();
		}
	}

	else if (mode == 8)				// web-tv
		g_settings.satip_allow_satip = allowip;
	
        return true;
}

extern "C" void plugin_exec(void);
extern "C" void plugin_init(void);
extern "C" void plugin_del(void);

void plugin_init(void)
{
	//do notthing
}

void plugin_del(void)
{
	//do not thing
}

void plugin_exec(void)
{
	// class handler
	CSatIPSetup * SatIPSetupHandler = new CSatIPSetup();
	SatIPSetupHandler->exec(NULL, "");
	
	delete SatIPSetupHandler;
	SatIPSetupHandler = NULL;
}






