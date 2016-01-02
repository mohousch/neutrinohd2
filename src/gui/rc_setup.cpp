/*
	Neutrino-GUI  -   DBoxII-Project

	$id: rc_setup.cpp 2016.01.02 21:33:30 mohousch $
	
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

#include <gui/widget/hintbox.h>

#include <gui/widget/keychooser.h>

#include <gui/rc_setup.h>

#include <system/debug.h>
#include <system/setting_helpers.h>
#include <system/helpers.h>


// remote control settings
enum keynames {
	// zap
	KEY_TV_RADIO_MODE,
	KEY_PAGE_UP,
	KEY_PAGE_DOWN,
	KEY_LIST_START,
	KEY_LIST_END,
	KEY_CANCEL_ACTION,
	KEY_SORT,
	KEY_RELOAD,
	VKEY_SEARCH,
	KEY_ADD_RECORD,
	KEY_ADD_REMIND,
	KEY_BOUQUET_UP,
	KEY_BOUQUET_DOWN,
	KEY_CHANNEL_UP,
	KEY_CHANNEL_DOWN,
	KEY_SUBCHANNEL_UP,
	KEY_SUBCHANNEL_DOWN,
	KEY_ZAP_HISTORY,
	KEY_LASTCHANNEL,
	KEY_SAME_TP,
	
	// mp
        MPKEY_REWIND,
        MPKEY_FORWARD,
        MPKEY_PAUSE,
        MPKEY_STOP,
        MPKEY_PLAY,
        MPKEY_AUDIO,
        MPKEY_TIME,
        MPKEY_BOOKMARK,
	KEY_TIMESHIFT,
	
	// media
	KEY_EXTRAS_RECORDSBROWSER,
	KEY_EXTRAS_AUDIOPLAYER,
	KEY_EXTRAS_PICTUREVIEWER,
	KEY_EXTRAS_TIMERLIST,
	KEY_EXTRAS_INETRADIO,
	KEY_EXTRAS_MOVIEBROWSER,
	KEY_EXTRAS_FILEBROWSER,
	KEY_EXTRAS_WEBTV,
	KEY_EXTRAS_SCREENSHOT,
	
	// mb
	KEY_EXTRAS_MB_COPY_JUMP,
	KEY_EXTRAS_MB_CUT_JUMP,
	KEY_EXTRAS_MB_TRUNCATE
};

#define KEYBINDS_COUNT 41
const neutrino_locale_t keydescription_head[KEYBINDS_COUNT] =
{
	// zap
	LOCALE_KEYBINDINGMENU_TVRADIOMODE,
	LOCALE_KEYBINDINGMENU_PAGEUP,
	LOCALE_KEYBINDINGMENU_PAGEDOWN,
	LOCALE_EXTRA_KEY_LIST_START,
	LOCALE_EXTRA_KEY_LIST_END,
	LOCALE_KEYBINDINGMENU_CANCEL,
	LOCALE_KEYBINDINGMENU_SORT,
	LOCALE_KEYBINDINGMENU_RELOAD,
	LOCALE_KEYBINDINGMENU_SEARCH,
	LOCALE_KEYBINDINGMENU_ADDRECORD,
	LOCALE_KEYBINDINGMENU_ADDREMIND,
	LOCALE_KEYBINDINGMENU_BOUQUETUP,
	LOCALE_KEYBINDINGMENU_BOUQUETDOWN,
	LOCALE_KEYBINDINGMENU_CHANNELUP,
	LOCALE_KEYBINDINGMENU_CHANNELDOWN,
	LOCALE_KEYBINDINGMENU_SUBCHANNELUP,
	LOCALE_KEYBINDINGMENU_SUBCHANNELDOWN,
	LOCALE_KEYBINDINGMENU_ZAPHISTORY,
	LOCALE_KEYBINDINGMENU_LASTCHANNEL,
	LOCALE_KEYBINDINGMENU_PIP,
	
	// mp
        LOCALE_MPKEY_REWIND,
        LOCALE_MPKEY_FORWARD,
        LOCALE_MPKEY_PAUSE,
        LOCALE_MPKEY_STOP,
        LOCALE_MPKEY_PLAY,
        LOCALE_MPKEY_AUDIO,
        LOCALE_MPKEY_TIME,
        LOCALE_MPKEY_BOOKMARK,
	LOCALE_EXTRA_KEY_TIMESHIFT,

	// media
	LOCALE_KEYBINDINGMENU_RECORDSBROWSER,
	LOCALE_KEYBINDINGMENU_AUDIOPLAYER,
	LOCALE_KEYBINDINGMENU_PICTUREVIEWER,
	LOCALE_KEYBINDINGMENU_TIMERLIST,
	LOCALE_KEYBINDINGMENU_INETRADIO,
	LOCALE_KEYBINDINGMENU_MOVIEBROWSER,
	LOCALE_KEYBINDINGMENU_FILEBROWSER,
	LOCALE_KEYBINDINGMENU_WEBTV,
	LOCALE_KEYBINDINGMENU_SCREENSHOT,
	
	// mb
	LOCALE_KEYBINDINGMENU_MB_COPY_JUMP,
	LOCALE_KEYBINDINGMENU_MB_CUT_JUMP,
	LOCALE_KEYBINDINGMENU_MB_TRUNCATE
};

const neutrino_locale_t keydescription[KEYBINDS_COUNT] =
{
	// zap
	LOCALE_KEYBINDINGMENU_TVRADIOMODE,
	LOCALE_KEYBINDINGMENU_PAGEUP,
	LOCALE_KEYBINDINGMENU_PAGEDOWN,
	LOCALE_EXTRA_KEY_LIST_START,
	LOCALE_EXTRA_KEY_LIST_END,
	LOCALE_KEYBINDINGMENU_CANCEL,
	LOCALE_KEYBINDINGMENU_SORT,
	LOCALE_KEYBINDINGMENU_RELOAD,
	LOCALE_KEYBINDINGMENU_SEARCH,
	LOCALE_KEYBINDINGMENU_ADDRECORD,
	LOCALE_KEYBINDINGMENU_ADDREMIND,
	LOCALE_KEYBINDINGMENU_BOUQUETUP,
	LOCALE_KEYBINDINGMENU_BOUQUETDOWN,
	LOCALE_KEYBINDINGMENU_CHANNELUP,
	LOCALE_KEYBINDINGMENU_CHANNELDOWN,
	LOCALE_KEYBINDINGMENU_SUBCHANNELUP,
	LOCALE_KEYBINDINGMENU_SUBCHANNELDOWN,
	LOCALE_KEYBINDINGMENU_ZAPHISTORY,
	LOCALE_KEYBINDINGMENU_LASTCHANNEL,
	LOCALE_KEYBINDINGMENU_PIP,
	
	// mp
        LOCALE_MPKEY_REWIND,
        LOCALE_MPKEY_FORWARD,
        LOCALE_MPKEY_PAUSE,
        LOCALE_MPKEY_STOP,
        LOCALE_MPKEY_PLAY,
        LOCALE_MPKEY_AUDIO,
        LOCALE_MPKEY_TIME,
        LOCALE_MPKEY_BOOKMARK,
	LOCALE_EXTRA_KEY_TIMESHIFT,
	
	// media
	LOCALE_KEYBINDINGMENU_RECORDSBROWSER,
	LOCALE_KEYBINDINGMENU_AUDIOPLAYER,
	LOCALE_KEYBINDINGMENU_PICTUREVIEWER,
	LOCALE_KEYBINDINGMENU_TIMERLIST,
	LOCALE_KEYBINDINGMENU_INETRADIO,
	LOCALE_KEYBINDINGMENU_MOVIEBROWSER,
	LOCALE_KEYBINDINGMENU_FILEBROWSER,
	LOCALE_KEYBINDINGMENU_WEBTV,
	LOCALE_KEYBINDINGMENU_SCREENSHOT,
	
	// mb
	LOCALE_KEYBINDINGMENU_MB_COPY_JUMP,
	LOCALE_KEYBINDINGMENU_MB_CUT_JUMP,
	LOCALE_KEYBINDINGMENU_MB_TRUNCATE
};

CRemoteControlSettings::CRemoteControlSettings()
{
}

CRemoteControlSettings::~CRemoteControlSettings()
{
}

int CRemoteControlSettings::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CRemoteControlSettings::exec: actionKey: %s\n", actionKey.c_str());
	
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

void CRemoteControlSettings::showMenu()
{
	dprintf(DEBUG_NORMAL, "CRemoteControlSettings::showMenu:\n");
	
	int shortcutkeysettings = 1;
	
	CMenuWidget remoteControlSettings(LOCALE_MAINSETTINGS_KEYBINDING, NEUTRINO_ICON_KEYBINDING );
	
	// intros
	remoteControlSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	remoteControlSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	remoteControlSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));

	keySetupNotifier = new CKeySetupNotifier;
	
	// repeat generic blocker
	CStringInput * remoteControlSettings_repeat_genericblocker = new CStringInput(LOCALE_KEYBINDINGMENU_REPEATBLOCKGENERIC, g_settings.repeat_genericblocker, 3, LOCALE_REPEATBLOCKER_HINT_1, LOCALE_REPEATBLOCKER_HINT_2, "0123456789 ", keySetupNotifier);
	
	// repeat blocker
	CStringInput * remoteControlSettings_repeatBlocker = new CStringInput(LOCALE_KEYBINDINGMENU_REPEATBLOCK, g_settings.repeat_blocker, 3, LOCALE_REPEATBLOCKER_HINT_1, LOCALE_REPEATBLOCKER_HINT_2, "0123456789 ", keySetupNotifier);
	keySetupNotifier->changeNotify(NONEXISTANT_LOCALE, NULL);

	remoteControlSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_KEYBINDINGMENU_RC));
	
	// repeat blocker
	remoteControlSettings.addItem(new CMenuForwarder(LOCALE_KEYBINDINGMENU_REPEATBLOCK, true, g_settings.repeat_blocker, remoteControlSettings_repeatBlocker, NULL, CRCInput::convertDigitToKey(shortcutkeysettings++)));
	
	// repeat generic blocker
 	remoteControlSettings.addItem(new CMenuForwarder(LOCALE_KEYBINDINGMENU_REPEATBLOCKGENERIC, true, g_settings.repeat_genericblocker, remoteControlSettings_repeat_genericblocker, NULL, CRCInput::convertDigitToKey(shortcutkeysettings++)));

	// keybinding menu
	remoteControlSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_KEYBINDINGMENU_HEAD));
	
	remoteControlSettings.addItem(new CMenuForwarder(LOCALE_KEYBINDINGMENU_HEAD, true, NULL, new CKeysBindingSettings(), NULL, CRCInput::convertDigitToKey(shortcutkeysettings++)));

        // usermenu 
        remoteControlSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_USERMENU_HEAD));
	
        remoteControlSettings.addItem(new CMenuForwarder(LOCALE_USERMENU_BUTTON_RED, true, NULL, new CUserMenuMenu(LOCALE_USERMENU_BUTTON_RED, 0), NULL, CRCInput::convertDigitToKey(shortcutkeysettings++) ));
        remoteControlSettings.addItem(new CMenuForwarder(LOCALE_USERMENU_BUTTON_GREEN, true, NULL, new CUserMenuMenu(LOCALE_USERMENU_BUTTON_GREEN, 1), NULL, CRCInput::convertDigitToKey(shortcutkeysettings++) ));
        remoteControlSettings.addItem(new CMenuForwarder(LOCALE_USERMENU_BUTTON_YELLOW, true, NULL, new CUserMenuMenu(LOCALE_USERMENU_BUTTON_YELLOW, 2), NULL, CRCInput::convertDigitToKey(shortcutkeysettings++) ));
        remoteControlSettings.addItem(new CMenuForwarder(LOCALE_USERMENU_BUTTON_BLUE, true, NULL, new CUserMenuMenu(LOCALE_USERMENU_BUTTON_BLUE, 3), NULL, CRCInput::convertDigitToKey(shortcutkeysettings++) ));
#if defined (ENABLE_FUNCTIONKEYS)	
	remoteControlSettings.addItem(new CMenuForwarder(LOCALE_USERMENU_BUTTON_F1, true, NULL, new CUserMenuMenu(LOCALE_USERMENU_BUTTON_F1, 4) ));
        remoteControlSettings.addItem(new CMenuForwarder(LOCALE_USERMENU_BUTTON_F2, true, NULL, new CUserMenuMenu(LOCALE_USERMENU_BUTTON_F2, 5) ));
        remoteControlSettings.addItem(new CMenuForwarder(LOCALE_USERMENU_BUTTON_F3, true, NULL, new CUserMenuMenu(LOCALE_USERMENU_BUTTON_F3, 6) ));
        remoteControlSettings.addItem(new CMenuForwarder(LOCALE_USERMENU_BUTTON_F4, true, NULL, new CUserMenuMenu(LOCALE_USERMENU_BUTTON_BLUE, 7) ));	
#endif
	
	remoteControlSettings.exec(NULL, "");
	remoteControlSettings.hide();
}

// keys binding settings
CKeysBindingSettings::CKeysBindingSettings()
{
}

CKeysBindingSettings::~CKeysBindingSettings()
{
}

int CKeysBindingSettings::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CKeysBindingSettings::exec: actionKey: %s\n", actionKey.c_str());
	
	int ret = menu_return::RETURN_REPAINT;
	
	if(parent)
		parent->hide();
	
	if(actionKey == "savesettings")
	{
		CNeutrinoApp::getInstance()->exec(NULL, "savesettings");
		
		return ret;
	}
	else if(actionKey == "savekeymap")
	{
		CHintBox * hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_KEYBINDINGMENU_SAVEKEYMAP_HINT)); // UTF-8
		hintBox->paint();
		
		g_RCInput->configfile.setModifiedFlag(true);
		g_RCInput->saveKeyMap(NEUTRINO_KEYMAP_FILE);
		
		sleep(2);
		
		hintBox->hide();
		delete hintBox;
		hintBox = NULL;

		return menu_return::RETURN_REPAINT;	
	}
	
	showMenu();
	
	return ret;
}

void CKeysBindingSettings::showMenu()
{
	dprintf(DEBUG_NORMAL, "CKeysBindingSettings::showMenu:\n");

	int * keyvalue_p[KEYBINDS_COUNT] =
	{
		// zap
		&g_settings.key_tvradio_mode,
		&g_settings.key_channelList_pageup,
		&g_settings.key_channelList_pagedown,
		&g_settings.key_list_start,
		&g_settings.key_list_end,
		&g_settings.key_channelList_cancel,
		&g_settings.key_channelList_sort,
		&g_settings.key_channelList_reload,
		&g_settings.key_channelList_search,
		&g_settings.key_channelList_addrecord,
		&g_settings.key_channelList_addremind,
		&g_settings.key_bouquet_up,
		&g_settings.key_bouquet_down,
		&g_settings.key_quickzap_up,
		&g_settings.key_quickzap_down,
		&g_settings.key_subchannel_up,
		&g_settings.key_subchannel_down,
		&g_settings.key_zaphistory,
		&g_settings.key_lastchannel,
		&g_settings.key_pip,

		// mp
		&g_settings.mpkey_rewind,
		&g_settings.mpkey_forward,
		&g_settings.mpkey_pause,
		&g_settings.mpkey_stop,
		&g_settings.mpkey_play,
		&g_settings.mpkey_audio,
		&g_settings.mpkey_time,
		&g_settings.mpkey_bookmark,
		&g_settings.key_timeshift,
		
		// media
		&g_settings.key_recordsbrowser,
		&g_settings.key_audioplayer,
		&g_settings.key_pictureviewer,
		&g_settings.key_timerlist,
		&g_settings.key_inetradio,
		&g_settings.key_moviebrowser,
		&g_settings.key_filebrowser,
		&g_settings.key_webtv,
		
		// misc
		&g_settings.key_screenshot,
		
		// mb
		&g_settings.mb_copy_jump,
		&g_settings.mb_cut_jump,
		&g_settings.mb_truncate
	};

	CKeyChooser * keychooser[KEYBINDS_COUNT];

	for (int i = 0; i < KEYBINDS_COUNT; i++)
		keychooser[i] = new CKeyChooser(keyvalue_p[i], keydescription_head[i], NEUTRINO_ICON_SETTINGS);
	
	// keybinding menu
	CMenuWidget bindSettings(LOCALE_KEYBINDINGMENU_HEAD, NEUTRINO_ICON_KEYBINDING );
	
	// intros
	bindSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	bindSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	bindSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));

	bindSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_KEYBINDINGMENU_MODECHANGE));
	
	// tv/radio mode
	bindSettings.addItem(new CMenuForwarder(keydescription[KEY_TV_RADIO_MODE], true, NULL, keychooser[KEY_TV_RADIO_MODE]));

	// channellist
	bindSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_KEYBINDINGMENU_CHANNELLIST));

	for (int i = KEY_PAGE_UP; i <= KEY_BOUQUET_DOWN; i++)
		bindSettings.addItem(new CMenuForwarder(keydescription[i], true, NULL, keychooser[i]));

	// quick zap
	bindSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_KEYBINDINGMENU_QUICKZAP));

	for (int i = KEY_CHANNEL_UP; i <= KEY_SAME_TP; i++)
		bindSettings.addItem(new CMenuForwarder(keydescription[i], true, NULL, keychooser[i]));

	// mp keys
	bindSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_MAINMENU_MOVIEPLAYER));
	for (int i = MPKEY_REWIND; i <= KEY_TIMESHIFT; i++)
		bindSettings.addItem(new CMenuForwarder(keydescription[i], true, NULL, keychooser[i]));
	
	// media
	bindSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_MAINMENU_MEDIAPLAYER));
	for (int i = KEY_EXTRAS_RECORDSBROWSER; i <= KEY_EXTRAS_WEBTV; i++)
		bindSettings.addItem(new CMenuForwarder(keydescription[i], true, NULL, keychooser[i]));
	
	// mb
	bindSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_MOVIEBROWSER_HEAD));
	for (int i = KEY_EXTRAS_MB_COPY_JUMP; i <= KEY_EXTRAS_MB_TRUNCATE; i++)
		bindSettings.addItem(new CMenuForwarder(keydescription[i], true, NULL, keychooser[i]));

	// misc
	bindSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_MAINSETTINGS_MISC));
	
	// screenshot key
	bindSettings.addItem(new CMenuForwarder(keydescription[KEY_EXTRAS_SCREENSHOT], true, NULL, keychooser[KEY_EXTRAS_SCREENSHOT]));
	
	// save keymap
	bindSettings.addItem(new CMenuForwarder(LOCALE_KEYBINDINGMENU_SAVEKEYMAP, true, NULL, this, "savekeymap" ) );
	
	bindSettings.exec(NULL, "");
	bindSettings.hide();
}


