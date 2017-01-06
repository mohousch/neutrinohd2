/*
	Neutrino-GUI  -   DBoxII-Project

	$id: main_setup.cpp 2015.12.22 21:31:30 mohousch $
	
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

#include <gui/main_setup.h>

#include <gui/audio_setup.h>
#include <gui/video_setup.h>
#include <gui/parentallock_setup.h>
#include <gui/network_setup.h>
#include <gui/movieplayer_setup.h>
#include <gui/osd_setup.h>
#include <gui/audioplayer_setup.h>
#include <gui/pictureviewer_setup.h>
#include <gui/lcd_setup.h>
#include <gui/rc_setup.h>
#include <gui/recording_setup.h>
#include <gui/misc_setup.h>
#include <gui/hdd_menu.h>

#include <system/debug.h>
#include <system/setting_helpers.h>
#include <system/helpers.h>


// main settings
CMainSetup::CMainSetup()
{
}

CMainSetup::~CMainSetup()
{
}

int CMainSetup::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CMainSetup::exec: actionKey: %s\n", actionKey.c_str());
	
	int ret = menu_return::RETURN_REPAINT;
	
	if(parent)
		parent->hide();
	
	if(g_settings.menu_design == SNeutrinoSettings::MENU_DESIGN_STANDARD)
		showMenu();
	else if(g_settings.menu_design == SNeutrinoSettings::MENU_DESIGN_CLASSIC)
		showMenuClassic();
	else if(g_settings.menu_design == SNeutrinoSettings::MENU_DESIGN_SMART)
		showMenuSmart();
	
	return ret;
}

// standard
void CMainSetup::showMenu(void)
{
	dprintf(DEBUG_NORMAL, "CMainSetup::showMenu:\n");
	
	int shortcutMainSettings = 1;

	CMenuWidgetExtended* mainSettings = new CMenuWidgetExtended(LOCALE_MAINSETTINGS_HEAD, NEUTRINO_ICON_SETTINGS);

	// video settings
	mainSettings->addItem(new CMenuForwarderExtended(LOCALE_MAINSETTINGS_VIDEO, true, new CVideoSettings(), NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NULL, NEUTRINO_ICON_MENUITEM_VIDEOSETTINGS, LOCALE_HELPTEXT_VIDEOSETTINGS ));

	// audio settings
	mainSettings->addItem(new CMenuForwarderExtended(LOCALE_MAINSETTINGS_AUDIO, true, new CAudioSettings(), NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NULL, NEUTRINO_ICON_MENUITEM_AUDIOSETTINGS, LOCALE_HELPTEXT_AUDIOSETTINGS ));

	// parentallock
	if(g_settings.parentallock_prompt)
		mainSettings->addItem(new CLockedMenuForwarderExtended(LOCALE_PARENTALLOCK_PARENTALLOCK, g_settings.parentallock_pincode, true, true, new CParentalLockSettings(), NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NULL, NEUTRINO_ICON_MENUITEM_PARENTALLOCKSETTINGS, LOCALE_HELPTEXT_PARENTALLOCK ));
	else
		mainSettings->addItem(new CMenuForwarderExtended(LOCALE_PARENTALLOCK_PARENTALLOCK, true, new CParentalLockSettings(), NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NULL, NEUTRINO_ICON_MENUITEM_PARENTALLOCKSETTINGS, LOCALE_HELPTEXT_PARENTALLOCK ));

	// network settings
	mainSettings->addItem(new CMenuForwarderExtended(LOCALE_MAINSETTINGS_NETWORK, true, CNetworkSettings::getInstance(), NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NULL, NEUTRINO_ICON_MENUITEM_NETWORKSETTINGS, LOCALE_HELPTEXT_NETWORKSETTINGS ));

	// recording settings
	mainSettings->addItem(new CMenuForwarderExtended(LOCALE_MAINSETTINGS_RECORDING, true, new CRecordingSettings(), NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NULL, NEUTRINO_ICON_MENUITEM_RECORDINGSETTINGS, LOCALE_HELPTEXT_RECORDINGSETTINGS ));

	// movieplayer settings
	mainSettings->addItem(new CMenuForwarderExtended(LOCALE_MAINSETTINGS_STREAMING, true, new CMoviePlayerSettings(), NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NULL, NEUTRINO_ICON_MENUITEM_MOVIEPLAYERSETTINGS, LOCALE_HELPTEXT_MOVIEPLAYERSETTINGS ));

	//OSD settings
	mainSettings->addItem(new CMenuForwarderExtended(LOCALE_MAINSETTINGS_OSD, true, new COSDSettings(), NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NULL, NEUTRINO_ICON_MENUITEM_OSDSETTINGS, LOCALE_HELPTEXT_OSDSETTINGS ));

	// vfd/lcd settings
	//if(CVFD::getInstance()->has_lcd)
		mainSettings->addItem(new CMenuForwarderExtended(LOCALE_MAINSETTINGS_LCD, true, new CLCDSettings(), NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NULL, NEUTRINO_ICON_MENUITEM_LCDSETTINGS, LOCALE_HELPTEXT_VFDSETTINGS ));	

	// remote control settings
	mainSettings->addItem(new CMenuForwarderExtended(LOCALE_MAINSETTINGS_KEYBINDING, true, new CRemoteControlSettings(), NULL, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED, NEUTRINO_ICON_MENUITEM_REMOTECONTROLSETTINGS, LOCALE_HELPTEXT_KEYSSETTINGS ));

	// audioplayer settings
	mainSettings->addItem(new CMenuForwarderExtended(LOCALE_AUDIOPLAYERSETTINGS_GENERAL, true, new CAudioPlayerSettings(), NULL, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN, NEUTRINO_ICON_MENUITEM_AUDIOPLAYERSETTINGS, LOCALE_HELPTEXT_AUDIOPLAYERSETTINGS ));
	
	// pictureviewer settings
	mainSettings->addItem(new CMenuForwarderExtended(LOCALE_PICTUREVIEWERSETTINGS_GENERAL, true, new CPictureViewerSettings(), NULL, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW, NEUTRINO_ICON_MENUITEM_PICTUREVIEWERSETTINGS, LOCALE_HELPTEXT_PICTUREVIEWERSETTINGS ));

	// misc settings
	mainSettings->addItem(new CMenuForwarderExtended(LOCALE_MAINSETTINGS_MISC, true, new CMiscSettings(), NULL, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE, NEUTRINO_ICON_MENUITEM_MISCSETTINGS, LOCALE_HELPTEXT_MISCSETTINGS ));

	//HDD settings
	mainSettings->addItem(new CMenuForwarderExtended(LOCALE_HDD_SETTINGS, true, new CHDDMenuHandler(), NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NULL, NEUTRINO_ICON_MENUITEM_HDDSETTINGS, LOCALE_HELPTEXT_HDDSETTINGS ));
	
	mainSettings->exec(NULL, "");
	mainSettings->hide();
	delete mainSettings;
	mainSettings = NULL;
}

// smart
void CMainSetup::showMenuSmart(void)
{
	dprintf(DEBUG_NORMAL, "CMainSetup::showMenuSmart:\n");
	
	int shortcutMainSettings = 1;

	CMenuFrameBox * mainSettings = new CMenuFrameBox(LOCALE_MAINSETTINGS_HEAD, NEUTRINO_ICON_SETTINGS);

	// video settings
	mainSettings->addItem(new CMenuFrameBoxItem(LOCALE_MAINSETTINGS_VIDEO, new CVideoSettings(), NULL, NEUTRINO_ICON_SMART_VIDEOSETTINGS));

	// audio settings
	mainSettings->addItem(new CMenuFrameBoxItem(LOCALE_MAINSETTINGS_AUDIO, new CAudioSettings(), NULL, NEUTRINO_ICON_SMART_AUDIOSETTINGS));

	// parentallock
	mainSettings->addItem(new CMenuFrameBoxItem(LOCALE_PARENTALLOCK_PARENTALLOCK, new CParentalLockSettings(), NULL, NEUTRINO_ICON_SMART_PARENTALLOCKSETTINGS));

	// network settings
	mainSettings->addItem(new CMenuFrameBoxItem(LOCALE_MAINSETTINGS_NETWORK, CNetworkSettings::getInstance(), NULL, NEUTRINO_ICON_SMART_NETWORKSETTINGS));

	// recording settings
	mainSettings->addItem(new CMenuFrameBoxItem(LOCALE_MAINSETTINGS_RECORDING, new CRecordingSettings(), NULL, NEUTRINO_ICON_SMART_RECORDINGSETTINGS));

	// movieplayer settings
	mainSettings->addItem(new CMenuFrameBoxItem(LOCALE_MAINSETTINGS_STREAMING, new CMoviePlayerSettings(), NULL, NEUTRINO_ICON_SMART_MOVIEPLAYERSETTINGS));

	//OSD settings
	mainSettings->addItem(new CMenuFrameBoxItem(LOCALE_MAINSETTINGS_OSD, new COSDSettings(), NULL, NEUTRINO_ICON_SMART_OSDSETTINGS));

	// vfd/lcd settings
	//if(CVFD::getInstance()->has_lcd)
		mainSettings->addItem(new CMenuFrameBoxItem(LOCALE_MAINSETTINGS_LCD, new CLCDSettings(), NULL, NEUTRINO_ICON_SMART_LCDSETTINGS));	

	// remote control settings
	mainSettings->addItem(new CMenuFrameBoxItem(LOCALE_MAINSETTINGS_KEYBINDING, new CRemoteControlSettings(), NULL, NEUTRINO_ICON_SMART_REMOTECONTROLSETTINGS));

	// audioplayer settings
	mainSettings->addItem(new CMenuFrameBoxItem(LOCALE_AUDIOPLAYERSETTINGS_GENERAL, new CAudioPlayerSettings(), NULL, NEUTRINO_ICON_SMART_AUDIOPLAYERSETTINGS));
	
	// pictureviewer settings
	mainSettings->addItem(new CMenuFrameBoxItem(LOCALE_PICTUREVIEWERSETTINGS_GENERAL, new CPictureViewerSettings(), NULL, NEUTRINO_ICON_SMART_PICTUREVIEWERSETTINGS));

	// misc settings
	mainSettings->addItem(new CMenuFrameBoxItem(LOCALE_MAINSETTINGS_MISC, new CMiscSettings(), NULL, NEUTRINO_ICON_SMART_MISCSETTINGS));

	//HDD settings
	mainSettings->addItem(new CMenuFrameBoxItem(LOCALE_HDD_SETTINGS, new CHDDMenuHandler(), NULL, NEUTRINO_ICON_SMART_HDDSETTINGS));
	
	mainSettings->exec(NULL, "");
	mainSettings->hide();
	delete mainSettings;
	mainSettings = NULL;
}

// classic
void CMainSetup::showMenuClassic(void)
{
	dprintf(DEBUG_NORMAL, "CMainSetup::showMenuClassic:\n");
	
	int shortcutMainSettings = 1;

	CMenuWidget * mainSettings = new CMenuWidget(LOCALE_MAINSETTINGS_HEAD, NEUTRINO_ICON_SETTINGS);

	// video settings
	mainSettings->addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_VIDEO, true, NULL, new CVideoSettings(), NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NEUTRINO_ICON_CLASSIC_VIDEOSETTINGS));

	// audio settings
	mainSettings->addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_AUDIO, true, NULL, new CAudioSettings(), NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NEUTRINO_ICON_CLASSIC_AUDIOSETTINGS));

	// parentallock
	if(g_settings.parentallock_prompt)
		mainSettings->addItem(new CLockedMenuForwarder(LOCALE_PARENTALLOCK_PARENTALLOCK, g_settings.parentallock_pincode, true, true, NULL, new CParentalLockSettings(), NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NEUTRINO_ICON_CLASSIC_PARENTALLOCKSETTINGS));
	else
		mainSettings->addItem(new CMenuForwarder(LOCALE_PARENTALLOCK_PARENTALLOCK, true, NULL, new CParentalLockSettings(), NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NEUTRINO_ICON_CLASSIC_PARENTALLOCKSETTINGS));

	// network settings
	mainSettings->addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_NETWORK, true, NULL,  CNetworkSettings::getInstance(), NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NEUTRINO_ICON_CLASSIC_NETWORKSETTINGS));

	// recording settings
	mainSettings->addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_RECORDING, true, NULL, new CRecordingSettings(), NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NEUTRINO_ICON_CLASSIC_RECORDINGSETTINGS));

	// movieplayer settings
	mainSettings->addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_STREAMING, true, NULL, new CMoviePlayerSettings(), NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NEUTRINO_ICON_CLASSIC_MOVIEPLAYERSETTINGS));

	//OSD settings
	mainSettings->addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_OSD, true, NULL, new COSDSettings(), NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NEUTRINO_ICON_CLASSIC_OSDSETTINGS));

	// vfd/lcd settings
	//if(CVFD::getInstance()->has_lcd)
		mainSettings->addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_LCD, true, NULL, new CLCDSettings(), NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NEUTRINO_ICON_CLASSIC_LCDSETTINGS));	

	// remote control settings
	mainSettings->addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_KEYBINDING, true, NULL, new CRemoteControlSettings(), NULL, CRCInput::RC_red, NEUTRINO_ICON_CLASSIC_REMOTECONTROLSETTINGS));

	// audioplayer settings
	mainSettings->addItem(new CMenuForwarder(LOCALE_AUDIOPLAYERSETTINGS_GENERAL, true, NULL, new CAudioPlayerSettings(), NULL, CRCInput::RC_green, NEUTRINO_ICON_CLASSIC_AUDIOPLAYERSETTINGS));
	
	// pictureviewer settings
	mainSettings->addItem(new CMenuForwarder(LOCALE_PICTUREVIEWERSETTINGS_GENERAL, true, NULL, new CPictureViewerSettings(), NULL, CRCInput::RC_yellow, NEUTRINO_ICON_CLASSIC_PICTUREVIEWERSETTINGS));

	// misc settings
	mainSettings->addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_MISC, true, NULL, new CMiscSettings(), NULL, CRCInput::RC_blue, NEUTRINO_ICON_CLASSIC_MISCSETTINGS));

	//HDD settings
	mainSettings->addItem(new CMenuForwarder(LOCALE_HDD_SETTINGS, true, NULL, new CHDDMenuHandler(), NULL, CRCInput::convertDigitToKey(shortcutMainSettings++), NEUTRINO_ICON_CLASSIC_HDDSETTINGS));
	
	mainSettings->exec(NULL, "");
	mainSettings->hide();
	delete mainSettings;
	mainSettings = NULL;
}

