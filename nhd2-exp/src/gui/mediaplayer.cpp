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
	
	showMenu();
	
	return res;
}

// showmenu
void CMediaPlayerMenu::showMenu()
{
	dprintf(DEBUG_NORMAL, "CMediaPlayerMenu::showMenu:\n");

	int shortcutMediaPlayer = 1;
	
	ClistBoxWidget * MediaPlayer = new ClistBoxWidget(LOCALE_MAINMENU_MEDIAPLAYER, NEUTRINO_ICON_MOVIE);

	MediaPlayer->setMode(MODE_MENU);
	MediaPlayer->enableShrinkMenu();
	MediaPlayer->enableMenuPosition();
	MediaPlayer->enableWidgetChange();
	MediaPlayer->enablePaintItemInfo();
	MediaPlayer->enablePaintDate();

	MediaPlayer->integratePlugins(CPlugins::I_TYPE_MULTIMEDIA, shortcutMediaPlayer++);
	
	MediaPlayer->exec(NULL, "");
	MediaPlayer->hide();
	delete MediaPlayer;
	MediaPlayer = NULL;
}


