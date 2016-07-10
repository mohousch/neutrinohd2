/*
	Neutrino-GUI  -   DBoxII-Project

	$id: mediaplayer.cpp 2015.12.22 12:07:30 mohousch $
	
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

#include <system/debug.h>

#include <gui/mediaplayer.h>
#include <gui/audioplayer.h>
#include <gui/movieplayer.h>
#include <gui/pictureviewer.h>
#include <gui/upnpbrowser.h>


CMediaPlayerMenu::CMediaPlayerMenu()
{
}

CMediaPlayerMenu::~CMediaPlayerMenu()
{
}

int CMediaPlayerMenu::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CMediaplayerMenu::exec: actionKey:%s\n", actionKey.c_str());
	
	int res = menu_return::RETURN_REPAINT;
	
	if(parent)
		parent->hide();
	
	if(g_settings.menu_design == SNeutrinoSettings::MENU_DESIGN_STANDARD)
		showMenu();
	else if(g_settings.menu_design == SNeutrinoSettings::MENU_DESIGN_CLASSIC)
		showMenuClassic();
	else if(g_settings.menu_design == SNeutrinoSettings::MENU_DESIGN_SMART)
		showMenuSmart();
	
	return res;
}

void CMediaPlayerMenu::showMenu()
{
	dprintf(DEBUG_NORMAL, "CMediaPlayerMenu::showMenu:\n");

	int shortcutMediaPlayer = 1;
	
	CMenuWidget * MediaPlayer = new CMenuWidget(LOCALE_MAINMENU_MEDIAPLAYER, NEUTRINO_ICON_MOVIE);
		
	// internet Radio
	MediaPlayer->addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_INETRADIO, true, new CAudioPlayerGui(true), NULL, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED, NEUTRINO_ICON_MENUITEM_INTERNETRADIO, LOCALE_HELPTEXT_INTERNETRADIO ));

	// audioPlayer
	MediaPlayer->addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_AUDIOPLAYER, true, new CAudioPlayerGui(), NULL, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN, NEUTRINO_ICON_MENUITEM_AUDIOPLAYER, LOCALE_HELPTEXT_AUDIOPLAYER ));

	// ts player
	MediaPlayer->addItem(new CMenuForwarderExtended(LOCALE_MOVIEPLAYER_RECORDS, true, new CMoviePlayerGui(), "tsmoviebrowser", CRCInput::convertDigitToKey(shortcutMediaPlayer++), NULL, NEUTRINO_ICON_MENUITEM_TSPLAYER, LOCALE_HELPTEXT_TSMOVIEBROWSER ));
	
	// movie player
	MediaPlayer->addItem(new CMenuForwarderExtended(LOCALE_MOVIEPLAYER_MOVIES, true, new CMoviePlayerGui(), "moviebrowser", CRCInput::convertDigitToKey(shortcutMediaPlayer++), NULL, NEUTRINO_ICON_MENUITEM_MOVIEPLAYER, LOCALE_HELPTEXT_TSMOVIEBROWSER ));
	
	// fileplayback
	MediaPlayer->addItem(new CMenuForwarderExtended(LOCALE_MOVIEPLAYER_FILEPLAYBACK, true, new CMoviePlayerGui(), "fileplayback", CRCInput::convertDigitToKey(shortcutMediaPlayer++), NULL, NEUTRINO_ICON_MENUITEM_FILEPLAYER, LOCALE_HELPTEXT_FILEPLAYBACK ));		

	// pictureViewer
	MediaPlayer->addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_PICTUREVIEWER, true, new CPictureViewerGui(), NULL, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW, NEUTRINO_ICON_MENUITEM_PICTUREVIEWER, LOCALE_HELPTEXT_PICTUREVIEWER ));
	
	// UPNP Browser
	 MediaPlayer->addItem(new CMenuForwarderExtended(LOCALE_UPNPBROWSER_HEAD, true, new CUpnpBrowserGui(), NULL, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE, NEUTRINO_ICON_MENUITEM_UPNPBROWSER, LOCALE_HELPTEXT_UPNPBROWSER ));
	
	MediaPlayer->exec(NULL, "");
	MediaPlayer->hide();
	delete MediaPlayer;
	MediaPlayer = NULL;
}

// smart
void CMediaPlayerMenu::showMenuSmart(void)
{
	dprintf(DEBUG_NORMAL, "CMediaPlayerMenu::showMenuSmart:\n");

	int shortcutMediaPlayer = 1;
	
	CSmartMenu * MediaPlayer = new CSmartMenu(LOCALE_MAINMENU_MEDIAPLAYER, NEUTRINO_ICON_MOVIE);
		
	// internet Radio
	MediaPlayer->addItem(new CMenuFrameBox(LOCALE_MAINMENU_INETRADIO, new CAudioPlayerGui(true), NULL, NEUTRINO_ICON_SMART_INTERNETRADIO));

	// audioPlayer
	MediaPlayer->addItem(new CMenuFrameBox(LOCALE_MAINMENU_AUDIOPLAYER, new CAudioPlayerGui(), NULL, NEUTRINO_ICON_SMART_AUDIOPLAYER));

	// ts player
	MediaPlayer->addItem(new CMenuFrameBox(LOCALE_MOVIEPLAYER_RECORDS, new CMoviePlayerGui(), "tsmoviebrowser", NEUTRINO_ICON_SMART_TSPLAYER));
	
	// movie player
	MediaPlayer->addItem(new CMenuFrameBox(LOCALE_MOVIEPLAYER_MOVIES, new CMoviePlayerGui(), "moviebrowser", NEUTRINO_ICON_SMART_MOVIEPLAYER));
	
	// fileplayback
	MediaPlayer->addItem(new CMenuFrameBox(LOCALE_MOVIEPLAYER_FILEPLAYBACK, new CMoviePlayerGui(), "fileplayback", NEUTRINO_ICON_SMART_FILEPLAYER));		

	// pictureViewer
	MediaPlayer->addItem(new CMenuFrameBox(LOCALE_MAINMENU_PICTUREVIEWER, new CPictureViewerGui(), NULL, NEUTRINO_ICON_SMART_PICTUREVIEWER));
	
	// UPNP Browser
	 MediaPlayer->addItem(new CMenuFrameBox(LOCALE_UPNPBROWSER_HEAD, new CUpnpBrowserGui(), NULL, NEUTRINO_ICON_SMART_UPNPBROWSER));
	
	MediaPlayer->exec(NULL, "");
	MediaPlayer->hide();
	delete MediaPlayer;
	MediaPlayer = NULL;
}

// classic
void CMediaPlayerMenu::showMenuClassic()
{
	dprintf(DEBUG_NORMAL, "CMediaPlayerMenu::showMenu:\n");

	int shortcutMediaPlayer = 1;
	
	CMenuWidget * MediaPlayer = new CMenuWidget(LOCALE_MAINMENU_MEDIAPLAYER, NEUTRINO_ICON_MOVIE);
		
	// internet Radio
	MediaPlayer->addItem(new CMenuForwarder(LOCALE_MAINMENU_INETRADIO, true, NULL, new CAudioPlayerGui(true), NULL, CRCInput::RC_red, NEUTRINO_ICON_CLASSIC_INTERNETRADIO));

	// audioPlayer
	MediaPlayer->addItem(new CMenuForwarder(LOCALE_MAINMENU_AUDIOPLAYER, true, NULL, new CAudioPlayerGui(), NULL, CRCInput::RC_green, NEUTRINO_ICON_CLASSIC_AUDIOPLAYER));

	// ts player
	MediaPlayer->addItem(new CMenuForwarder(LOCALE_MOVIEPLAYER_RECORDS, true, NULL, new CMoviePlayerGui(), "tsmoviebrowser", CRCInput::convertDigitToKey(shortcutMediaPlayer++), NEUTRINO_ICON_CLASSIC_TSPLAYER));
	
	// movie player
	MediaPlayer->addItem(new CMenuForwarder(LOCALE_MOVIEPLAYER_MOVIES, true, NULL, new CMoviePlayerGui(), "moviebrowser", CRCInput::convertDigitToKey(shortcutMediaPlayer++), NEUTRINO_ICON_CLASSIC_MOVIEPLAYER));
	
	// fileplayback
	MediaPlayer->addItem(new CMenuForwarder(LOCALE_MOVIEPLAYER_FILEPLAYBACK, true, NULL, new CMoviePlayerGui(), "fileplayback", CRCInput::convertDigitToKey(shortcutMediaPlayer++), NEUTRINO_ICON_CLASSIC_FILEPLAYER));		

	// pictureViewer
	MediaPlayer->addItem(new CMenuForwarder(LOCALE_MAINMENU_PICTUREVIEWER, true, NULL, new CPictureViewerGui(), NULL, CRCInput::RC_yellow, NEUTRINO_ICON_CLASSIC_PICTUREVIEWER));
	
	// UPNP Browser
	 MediaPlayer->addItem(new CMenuForwarder(LOCALE_UPNPBROWSER_HEAD, true, NULL, new CUpnpBrowserGui(), NULL, CRCInput::RC_blue, NEUTRINO_ICON_CLASSIC_UPNPBROWSER));
	
	MediaPlayer->exec(NULL, "");
	MediaPlayer->hide();
	delete MediaPlayer;
	MediaPlayer = NULL;
}


