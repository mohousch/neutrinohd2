/*
	* $Id: zapit_setup.cpp 2013/08/18 11:23:30 mohousch Exp $
	
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

// zapit includes
#include <bouquets.h>

#include "gui/zapit_setup.h"

#include <global.h>
#include <neutrino.h>

#include <driver/screen_max.h>
#include <gui/channel_select.h>

#include <system/debug.h>


//option off0_on1
#define OPTIONS_OFF0_ON1_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_OFF0_ON1_OPTIONS[OPTIONS_OFF0_ON1_OPTION_COUNT] =
{
        { 0, LOCALE_OPTIONS_OFF, NULL },
        { 1, LOCALE_OPTIONS_ON, NULL  }
};

/* option off1 on0*/
#define OPTIONS_OFF1_ON0_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_OFF1_ON0_OPTIONS[OPTIONS_OFF1_ON0_OPTION_COUNT] =
{
        { 1, LOCALE_OPTIONS_OFF, NULL },
        { 0, LOCALE_OPTIONS_ON, NULL  }
};

#define OPTIONS_LASTMODE_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_LASTMODE_OPTIONS[OPTIONS_LASTMODE_OPTION_COUNT] =
{
        { 0, NONEXISTANT_LOCALE, "Radio" },
        { 1, NONEXISTANT_LOCALE, "TV"  }
};

CZapitSetup::CZapitSetup()
{
}

CZapitSetup::~CZapitSetup()
{
}

int CZapitSetup::exec(CMenuTarget * parent, const std::string &actionKey)
{
	dprintf(DEBUG_NORMAL, "CZapitSetup::exec: actionKey:%s\n", actionKey.c_str());
	
	int   res = menu_return::RETURN_REPAINT;
	CSelectChannelWidget*  CSelectChannelWidgetHandler;
	
	if (parent)
		parent->hide();
	
	if(actionKey == "save") 
	{
		CNeutrinoApp::getInstance()->exec(NULL, "savesettings");
		showMenu();
		return menu_return::RETURN_EXIT;
	}
	else if(actionKey == "tv")
	{
		CSelectChannelWidgetHandler = new CSelectChannelWidget();
		CSelectChannelWidgetHandler->exec(NULL, "tv");
		
		g_settings.startchanneltv_id = CSelectChannelWidgetHandler->getChanTVID();
		g_settings.StartChannelTV = g_Zapit->getChannelName(CSelectChannelWidgetHandler->getChanTVID());
		
		delete CSelectChannelWidgetHandler;
		CSelectChannelWidgetHandler = NULL;
		
		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "radio")
	{
		CSelectChannelWidgetHandler = new CSelectChannelWidget();
		CSelectChannelWidgetHandler->exec(NULL, "radio");
		
		g_settings.startchannelradio_id = CSelectChannelWidgetHandler->getChanRadioID();
		g_settings.StartChannelRadio = g_Zapit->getChannelName(CSelectChannelWidgetHandler->getChanRadioID());
		
		delete CSelectChannelWidgetHandler;
		CSelectChannelWidgetHandler = NULL;
		
		return menu_return::RETURN_REPAINT;
	}

	showMenu();

	return res;
}

void CZapitSetup::showMenu()
{
	dprintf(DEBUG_NORMAL, "CZapitSetup::showMenu:\n");

	//menue init
	CMenuWidget * zapit = new CMenuWidget(LOCALE_MISCSETTINGS_ZAPIT, NEUTRINO_ICON_SETTINGS);
	
	int shortcut = 1;
	
	// intros
	zapit->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	zapit->addItem(new CMenuSeparator(CMenuSeparator::LINE));
	
	// save settings
	zapit->addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "save", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	zapit->addItem(new CMenuSeparator(CMenuSeparator::LINE));
	
	// last mode
	CMenuOptionChooser * m2 = new CMenuOptionChooser(LOCALE_ZAPITSETUP_LAST_MODE, &g_settings.lastChannelMode, OPTIONS_LASTMODE_OPTIONS, OPTIONS_LASTMODE_OPTION_COUNT, !g_settings.uselastchannel, NULL, CRCInput::convertDigitToKey(shortcut++));

	// last channel TV
	CMenuForwarder * m3 = new CMenuForwarder(LOCALE_ZAPITSETUP_LAST_TV, !g_settings.uselastchannel, g_settings.StartChannelTV.c_str(), this, "tv", CRCInput::convertDigitToKey(shortcut++));

	// last channel radio
	CMenuForwarder * m4 = new CMenuForwarder(LOCALE_ZAPITSETUP_LAST_RADIO, !g_settings.uselastchannel, g_settings.StartChannelRadio.c_str(), this, "radio", CRCInput::convertDigitToKey(shortcut++));
	
	CZapitSetupNotifier zapitSetupNotifier(m2, m3, m4);

	// use last channel
	CMenuOptionChooser * m1 = new CMenuOptionChooser(LOCALE_MISCSETTINGS_ZAPIT, &g_settings.uselastchannel, OPTIONS_OFF1_ON0_OPTIONS, OPTIONS_OFF1_ON0_OPTION_COUNT, true, &zapitSetupNotifier, CRCInput::convertDigitToKey(shortcut++));
	
	zapit->addItem(m1);
	zapit->addItem(m2);
	zapit->addItem(m3);
	zapit->addItem(m4);

	zapit->exec(NULL, "");
	zapit->hide();
	
	delete zapit;
}

CZapitSetupNotifier::CZapitSetupNotifier(CMenuOptionChooser* m1, CMenuForwarder* m2, CMenuForwarder* m3)
{
	zapit1 = m1;
	zapit2 = m2;
	zapit3 = m3;
}

bool CZapitSetupNotifier::changeNotify(const neutrino_locale_t OptionName, void *)
{
	if (ARE_LOCALES_EQUAL(OptionName, LOCALE_MISCSETTINGS_ZAPIT))
	{
		zapit1->setActive(!g_settings.uselastchannel);
		zapit2->setActive(!g_settings.uselastchannel);
		zapit3->setActive(!g_settings.uselastchannel);
	}

	return true;
}
