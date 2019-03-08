/*
	Neutrino-GUI  -   DBoxII-Project

	$id: pictureviewer_setup.cpp 2016.01.02 21:09:30 mohousch $
	
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
#include <gui/widget/stringinput_ext.h>

#include <gui/filebrowser.h>
#include <gui/pictureviewer_setup.h>

#include <system/debug.h>
#include <system/setting_helpers.h>
#include <system/helpers.h>

#include <video_cs.h>
#include <audio_cs.h>


#define MESSAGEBOX_NO_YES_OPTION_COUNT 2
const CMenuOptionChooser::keyval MESSAGEBOX_NO_YES_OPTIONS[MESSAGEBOX_NO_YES_OPTION_COUNT] =
{
	{ 0, LOCALE_MESSAGEBOX_NO, NULL },
	{ 1, LOCALE_MESSAGEBOX_YES, NULL }
};

#define PICTUREVIEWER_SCALING_OPTION_COUNT 3
const CMenuOptionChooser::keyval PICTUREVIEWER_SCALING_OPTIONS[PICTUREVIEWER_SCALING_OPTION_COUNT] =
{
	{ SIMPLE, LOCALE_PICTUREVIEWER_RESIZE_SIMPLE, NULL        },
	{ COLOR , LOCALE_PICTUREVIEWER_RESIZE_COLOR_AVERAGE, NULL },
	{ NONE  , LOCALE_PICTUREVIEWER_RESIZE_NONE, NULL         }
};

CPictureViewerSettings::CPictureViewerSettings()
{
}

CPictureViewerSettings::~CPictureViewerSettings()
{
}

int CPictureViewerSettings::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CPicTureViewerSettings::exec: actionKey: %s\n", actionKey.c_str());
	
	int ret = menu_return::RETURN_REPAINT;
	
	if(parent)
		parent->hide();
	
	if(actionKey == "savesettings")
	{
		CNeutrinoApp::getInstance()->exec(NULL, "savesettings");
		
		return ret;
	}
	else if(actionKey == "picturedir")
	{
		if(parent)
			parent->hide();
		
		CFileBrowser b;
		b.Dir_Mode = true;
		
		if (b.exec(g_settings.network_nfs_picturedir))
			strncpy(g_settings.network_nfs_picturedir, b.getSelectedFile()->Name.c_str(), sizeof(g_settings.network_nfs_picturedir)-1);
		
		return ret;
	}
	
	showMenu();
	
	return ret;
}

void CPictureViewerSettings::showMenu()
{
	dprintf(DEBUG_NORMAL, "CPicTureViewerSettings::showMenu:\n");
	
	CMenuWidget PicViewerSettings(LOCALE_PICTUREVIEWERSETTINGS_GENERAL, NEUTRINO_ICON_SETTINGS );

	PicViewerSettings.enableSaveScreen();
	
	int shortcutPicViewer = 1;
	
	// intros
	PicViewerSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	PicViewerSettings.addItem( new CMenuSeparator(LINE) );
	
	// save settings
	PicViewerSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", RC_red, NEUTRINO_ICON_BUTTON_RED));
	PicViewerSettings.addItem( new CMenuSeparator(LINE) );

	// Pic Viewer Scaling
	PicViewerSettings.addItem(new CMenuOptionChooser(LOCALE_PICTUREVIEWER_SCALING, &g_settings.picviewer_scaling, PICTUREVIEWER_SCALING_OPTIONS, PICTUREVIEWER_SCALING_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutPicViewer++),"", true ));

	// slide Timeout
	//CStringInput * pic_timeout= new CStringInput(LOCALE_PICTUREVIEWER_SLIDE_TIME, g_settings.picviewer_slide_time, 2, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE, "0123456789 ");
	//PicViewerSettings.addItem(new CMenuForwarder(LOCALE_PICTUREVIEWER_SLIDE_TIME, true, g_settings.picviewer_slide_time, pic_timeout, NULL, CRCInput::convertDigitToKey(shortcutPicViewer++)));

	PicViewerSettings.addItem(new CMenuOptionNumberChooser(LOCALE_PICTUREVIEWER_SLIDE_TIME, &g_settings.picviewer_slide_time, true, 0, 999));

	// Pic Viewer Default Dir
	PicViewerSettings.addItem(new CMenuForwarder(LOCALE_PICTUREVIEWER_DEFDIR, true, g_settings.network_nfs_picturedir, this, "picturedir", CRCInput::convertDigitToKey(shortcutPicViewer++)));
	
	PicViewerSettings.exec(NULL, "");
	PicViewerSettings.hide();
}


