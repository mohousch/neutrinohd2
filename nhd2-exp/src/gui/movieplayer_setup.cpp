/*
	Neutrino-GUI  -   DBoxII-Project

	$id: movieplayer_setup.cpp 2016.01.02 20:33:30 mohousch $
	
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
#include <gui/movieplayer_setup.h>

#include <system/debug.h>
#include <system/setting_helpers.h>
#include <system/helpers.h>


CMoviePlayerSettings::CMoviePlayerSettings()
{
}

CMoviePlayerSettings::~CMoviePlayerSettings()
{
}

int CMoviePlayerSettings::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CMoviePlayerSettings::exec: actionKey: %s\n", actionKey.c_str());
	
	int ret = menu_return::RETURN_REPAINT;
	
	if(parent)
		parent->hide();
	
	if(actionKey == "savesettings")
	{
		CNeutrinoApp::getInstance()->exec(NULL, "savesettings");
		
		return ret;
	}
	else if(actionKey == "moviedir")
	{
		if(parent)
			parent->hide();
		
		CFileBrowser b;
		b.Dir_Mode = true;

		if (b.exec(g_settings.network_nfs_moviedir))
			strncpy(g_settings.network_nfs_moviedir, b.getSelectedFile()->Name.c_str(), sizeof(g_settings.network_nfs_moviedir)-1);
		
		return ret;
	}
	
	showMenu();
	
	return ret;
}

void CMoviePlayerSettings::showMenu()
{
	dprintf(DEBUG_NORMAL, "CMoviePlayerSettings::showMenu:\n");
	
	CMenuWidget moviePlayerSettings(LOCALE_STREAMINGMENU_HEAD, NEUTRINO_ICON_MOVIE);
	
	// intros
	moviePlayerSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	moviePlayerSettings.addItem( new CMenuSeparator(LINE) );
	
	// save settings
	moviePlayerSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", RC_red, NEUTRINO_ICON_BUTTON_RED));
	moviePlayerSettings.addItem( new CMenuSeparator(LINE) );

	// multiformat Dir
	moviePlayerSettings.addItem(new CMenuForwarder(LOCALE_MOVIEPLAYER_DEFDIR, true, g_settings.network_nfs_moviedir, this, "moviedir") ); 
	
	moviePlayerSettings.exec(NULL, "");
	moviePlayerSettings.hide();
}


