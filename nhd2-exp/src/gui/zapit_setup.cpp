/*
	* $Id: zapit_setup.cpp 16.11.2020 mohousch Exp $
	
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


extern CBouquetManager * g_bouquetManager;	// defined in der zapit.cpp

//option off0_on1
#define OPTIONS_OFF0_ON1_OPTION_COUNT 2
const keyval OPTIONS_OFF0_ON1_OPTIONS[OPTIONS_OFF0_ON1_OPTION_COUNT] =
{
        { 0, LOCALE_OPTIONS_OFF, NULL },
        { 1, LOCALE_OPTIONS_ON, NULL  }
};

/* option off1 on0*/
#define OPTIONS_OFF1_ON0_OPTION_COUNT 2
const keyval OPTIONS_OFF1_ON0_OPTIONS[OPTIONS_OFF1_ON0_OPTION_COUNT] =
{
        { 1, LOCALE_OPTIONS_OFF, NULL },
        { 0, LOCALE_OPTIONS_ON, NULL  }
};

#define OPTIONS_LASTMODE_OPTION_COUNT 3
const keyval OPTIONS_LASTMODE_OPTIONS[OPTIONS_LASTMODE_OPTION_COUNT] =
{
        { NeutrinoMessages::mode_radio, NONEXISTANT_LOCALE, "Radio" },
        { NeutrinoMessages::mode_tv, NONEXISTANT_LOCALE, "TV" },
	{ NeutrinoMessages::mode_webtv, NONEXISTANT_LOCALE, "WEBTV" }
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
		
		g_settings.startchanneltv_id = CSelectChannelWidgetHandler->getChannelID() & 0xFFFFFFFFFFFFULL;
		g_settings.StartChannelTV = g_Zapit->getChannelName(CSelectChannelWidgetHandler->getChannelID());
		//test
		g_settings.startchanneltv_nr = g_bouquetManager->tvChannelsBegin().getLowestChannelNumberWithChannelID(CSelectChannelWidgetHandler->getChannelID());

		this->getString() = g_Zapit->getChannelName(CSelectChannelWidgetHandler->getChannelID());
		
		delete CSelectChannelWidgetHandler;
		CSelectChannelWidgetHandler = NULL;
		
		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "radio")
	{
		CSelectChannelWidgetHandler = new CSelectChannelWidget();
		CSelectChannelWidgetHandler->exec(NULL, "radio");
		
		g_settings.startchannelradio_id = CSelectChannelWidgetHandler->getChannelID() & 0xFFFFFFFFFFFFULL;
		g_settings.StartChannelRadio = g_Zapit->getChannelName(CSelectChannelWidgetHandler->getChannelID());
		//test
		g_settings.startchannelradio_nr = g_bouquetManager->radioChannelsBegin().getLowestChannelNumberWithChannelID(CSelectChannelWidgetHandler->getChannelID());

		this->getString() = g_Zapit->getChannelName(CSelectChannelWidgetHandler->getChannelID());
		
		delete CSelectChannelWidgetHandler;
		CSelectChannelWidgetHandler = NULL;
		
		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "webtv")
	{
		CSelectChannelWidgetHandler = new CSelectChannelWidget();
		CSelectChannelWidgetHandler->exec(NULL, "webtv");
		
		g_settings.startchannelwebtv_id = CSelectChannelWidgetHandler->getChannelID() & 0xFFFFFFFFFFFFULL;
		g_settings.StartChannelWEBTV = g_Zapit->getChannelName(CSelectChannelWidgetHandler->getChannelID());
		//test
		g_settings.startchannelwebtv_nr = g_bouquetManager->webtvChannelsBegin().getLowestChannelNumberWithChannelID(CSelectChannelWidgetHandler->getChannelID());

		this->getString() = g_Zapit->getChannelName(CSelectChannelWidgetHandler->getChannelID());
		
		delete CSelectChannelWidgetHandler;
		CSelectChannelWidgetHandler = NULL;

		printf("id:%llx name:%s nr:%d\n", g_settings.startchannelwebtv_id, g_settings.StartChannelWEBTV.c_str(), g_settings.startchannelwebtv_nr);
		
		return menu_return::RETURN_REPAINT;
	}

	showMenu();

	return res;
}

void CZapitSetup::showMenu()
{
	dprintf(DEBUG_NORMAL, "CZapitSetup::showMenu:\n");

	//menue init
	ClistBoxWidget * zapit = new ClistBoxWidget(LOCALE_MISCSETTINGS_ZAPIT, NEUTRINO_ICON_SETTINGS);

	zapit->setMode(MODE_SETUP);
	zapit->enableShrinkMenu();
	
	int shortcut = 1;
	
	// intros
	zapit->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	zapit->addItem(new CMenuSeparator(LINE));
	
	// save settings
	zapit->addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "save", RC_red, NEUTRINO_ICON_BUTTON_RED));
	zapit->addItem(new CMenuSeparator(LINE));

	bool activTV = false;
	bool activRadio = false;
	bool activWebTV = false;

	if( (!g_settings.uselastchannel) && (g_settings.lastChannelMode == NeutrinoMessages::mode_tv) )
		activTV = true;

	if( (!g_settings.uselastchannel) && (g_settings.lastChannelMode == NeutrinoMessages::mode_radio) )
		activRadio = true;

	if( (!g_settings.uselastchannel) && (g_settings.lastChannelMode == NeutrinoMessages::mode_webtv) )
		activWebTV = true;

	// last TV channel
	CMenuForwarder * m3 = new CMenuForwarder(LOCALE_ZAPITSETUP_LAST_TV, /*!g_settings.uselastchannel*/activTV, g_settings.StartChannelTV.c_str(), this, "tv", CRCInput::convertDigitToKey(shortcut++));

	// last radio channel
	CMenuForwarder * m4 = new CMenuForwarder(LOCALE_ZAPITSETUP_LAST_RADIO, /*!g_settings.uselastchannel*/activRadio, g_settings.StartChannelRadio.c_str(), this, "radio", CRCInput::convertDigitToKey(shortcut++));

	// last webtv channel
	CMenuForwarder * m5 = new CMenuForwarder(LOCALE_ZAPITSETUP_LAST_WEBTV, /*!g_settings.uselastchannel*/activWebTV, g_settings.StartChannelWEBTV.c_str(), this, "webtv", CRCInput::convertDigitToKey(shortcut++));

	// last mode
	CZapitSetupModeNotifier zapitSetupModeNotifier((int *)&g_settings.lastChannelMode, m3, m4, m5);

	CMenuOptionChooser * m2 = new CMenuOptionChooser(LOCALE_ZAPITSETUP_LAST_MODE, (int *)&g_settings.lastChannelMode, OPTIONS_LASTMODE_OPTIONS, OPTIONS_LASTMODE_OPTION_COUNT, !g_settings.uselastchannel, &zapitSetupModeNotifier, CRCInput::convertDigitToKey(shortcut++));
	
	// use lastchannel
	CZapitSetupNotifier zapitSetupNotifier(m2, m3, m4, m5);

	CMenuOptionChooser * m1 = new CMenuOptionChooser(LOCALE_MISCSETTINGS_ZAPIT, &g_settings.uselastchannel, OPTIONS_OFF1_ON0_OPTIONS, OPTIONS_OFF1_ON0_OPTION_COUNT, true, &zapitSetupNotifier, CRCInput::convertDigitToKey(shortcut++));
	
	zapit->addItem(m1);
	zapit->addItem(m2);
	zapit->addItem(m3);
	zapit->addItem(m4);
	zapit->addItem(m5);

	zapit->exec(NULL, "");
	zapit->hide();
	
	delete zapit;
}

//
CZapitSetupNotifier::CZapitSetupNotifier(CMenuOptionChooser* m1, CMenuForwarder* m2, CMenuForwarder* m3, CMenuForwarder* m4)
{
	zapit1 = m1;
	zapit2 = m2;
	zapit3 = m3;
	zapit4 = m4;
}

bool CZapitSetupNotifier::changeNotify(const neutrino_locale_t OptionName, void *)
{
	if (ARE_LOCALES_EQUAL(OptionName, LOCALE_MISCSETTINGS_ZAPIT))
	{
		bool activTV = false;
		bool activRadio = false;
		bool activWebTV = false;

		if( (!g_settings.uselastchannel) && (g_settings.lastChannelMode == NeutrinoMessages::mode_tv) )
			activTV = true;

		if( (!g_settings.uselastchannel) && (g_settings.lastChannelMode == NeutrinoMessages::mode_radio) )
			activRadio = true;

		if( (!g_settings.uselastchannel) && (g_settings.lastChannelMode == NeutrinoMessages::mode_webtv) )
			activWebTV = true;

		zapit1->setActive(!g_settings.uselastchannel);
		zapit2->setActive(activTV);
		zapit3->setActive(activRadio);
		zapit4->setActive(activWebTV);
	}

	return true;
}

//
CZapitSetupModeNotifier::CZapitSetupModeNotifier(int *zMode, CMenuItem *m1, CMenuItem *m2, CMenuItem *m3)
{
	mode = zMode;
	item1 = m1;
	item2 = m2;
	item3 = m3;
}

bool CZapitSetupModeNotifier::changeNotify(const neutrino_locale_t, void *)
{
	int nmode = *mode;
	
	if(nmode == NeutrinoMessages::mode_radio)
	{
		item1->setActive(false);
		item2->setActive(true);
		item3->setActive(false);
	}
	else if(nmode == NeutrinoMessages::mode_tv)
	{
		item1->setActive(true);
		item2->setActive(false);
		item3->setActive(false);
	}
	else if(nmode == NeutrinoMessages::mode_webtv)
	{
		item1->setActive(false);
		item2->setActive(false);
		item3->setActive(true);
	}

	return true;
}




