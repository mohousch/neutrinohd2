/*
	Neutrino-GUI  -   DBoxII-Project

	$id: audioplayer_setup.cpp 2016.01.02 20:57:30 mohousch $
	
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

#include <gui/filebrowser.h>
#include <gui/audioplayer.h>
#include <gui/audioplayer_setup.h>

#include <system/debug.h>
#include <system/setting_helpers.h>
#include <system/helpers.h>


#define MESSAGEBOX_NO_YES_OPTION_COUNT 2
const CMenuOptionChooser::keyval MESSAGEBOX_NO_YES_OPTIONS[MESSAGEBOX_NO_YES_OPTION_COUNT] =
{
	{ 0, LOCALE_MESSAGEBOX_NO, NULL },
	{ 1, LOCALE_MESSAGEBOX_YES, NULL }
};

/*
#define AUDIOPLAYER_DISPLAY_ORDER_OPTION_COUNT 2
const CMenuOptionChooser::keyval AUDIOPLAYER_DISPLAY_ORDER_OPTIONS[AUDIOPLAYER_DISPLAY_ORDER_OPTION_COUNT] =
{
	{ CAudioPlayerGui::ARTIST_TITLE, LOCALE_AUDIOPLAYER_ARTIST_TITLE, NULL },
	{ CAudioPlayerGui::TITLE_ARTIST, LOCALE_AUDIOPLAYER_TITLE_ARTIST, NULL }
};
*/

CAudioPlayerSettings::CAudioPlayerSettings()
{
}

CAudioPlayerSettings::~CAudioPlayerSettings()
{
}

int CAudioPlayerSettings::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CAudioPlayerSettings::exec: actionKey: %s\n", actionKey.c_str());
	
	int ret = menu_return::RETURN_REPAINT;
	
	if(parent)
		parent->hide();
	
	if(actionKey == "savesettings")
	{
		CNeutrinoApp::getInstance()->exec(NULL, "savesettings");
		
		return ret;
	}
	else if(actionKey == "audioplayerdir")
	{
		if(parent)
			parent->hide();
		
		CFileBrowser b;
		b.Dir_Mode = true;
		
		if (b.exec(g_settings.network_nfs_audioplayerdir))
			strncpy(g_settings.network_nfs_audioplayerdir, b.getSelectedFile()->Name.c_str(), sizeof(g_settings.network_nfs_audioplayerdir)-1);
		
		return ret;
	}
	
	showMenu();
	
	return ret;
}

void CAudioPlayerSettings::showMenu()
{
	dprintf(DEBUG_NORMAL, "CAudioPlayerSettings::showMenu:\n");
	
	CMenuWidget audioPlayerSettings(LOCALE_AUDIOPLAYERSETTINGS_GENERAL, NEUTRINO_ICON_SETTINGS);
	
	int shortcutAudioPlayer = 1;
	
	// intros
	audioPlayerSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	audioPlayerSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	audioPlayerSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	audioPlayerSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	// auto select
	//audioPlayerSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOPLAYER_FOLLOW, &g_settings.audioplayer_follow, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutAudioPlayer++) ));

	// repeat
	audioPlayerSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOPLAYER_REPEAT_ON, &g_settings.audioplayer_repeat_on, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutAudioPlayer++) ));

	// high prio
	audioPlayerSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOPLAYER_HIGHPRIO, &g_settings.audioplayer_highprio, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutAudioPlayer++) ));

	// start dir
	audioPlayerSettings.addItem(new CMenuForwarder(LOCALE_AUDIOPLAYER_DEFDIR, true, g_settings.network_nfs_audioplayerdir, this, "audioplayerdir", CRCInput::convertDigitToKey(shortcutAudioPlayer++)));
	
	audioPlayerSettings.exec(NULL, "");
	audioPlayerSettings.hide();
}


