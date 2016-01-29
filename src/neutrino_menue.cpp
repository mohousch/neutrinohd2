/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: neutrino_menu.cpp 2013/10/12 11:23:30 mohousch Exp $

	Copyright (C) 2001 Steffen Hehn 'McClean' and some other guys
	Homepage: http://dbox.cyberphoria.org/

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


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
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#include <string>

#include <global.h>
#include <neutrino.h>

#include <daemonc/remotecontrol.h>

#include <gui/widget/menue.h>
#include <gui/widget/textbox.h>
#include <gui/widget/icons.h>

#include <gui/mediaplayer.h>
#include <gui/service_setup.h>
#include <gui/main_setup.h>
#include <gui/timerlist.h>
#include <gui/sleeptimer.h>
#include <gui/power_menu.h>
#include <gui/dboxinfo.h>

#include <gui/audio_select.h>
#include <gui/epgplus.h>
#include <gui/streaminfo2.h>
#include <gui/opkg_manager.h>
#include <gui/movieplayer.h>
#include <gui/pluginlist.h>

#include <system/debug.h>

#include <driver/encoding.h>


// Init Main Menu
void CNeutrinoApp::InitMainMenu(CMenuWidget &mainMenu)
{
	int shortcut = 1;

	dprintf(DEBUG_NORMAL, "CNeutrinoApp::InitMainMenu\n");
	  
	// tv modus
	mainMenu.addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_TVMODE, true, this, "tv", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED, NEUTRINO_ICON_MENUITEM_TV, LOCALE_HELPTEXT_TVMODE ), true);

	// radio modus
	mainMenu.addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_RADIOMODE, true, this, "radio", CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN, NEUTRINO_ICON_MENUITEM_RADIO, LOCALE_HELPTEXT_RADIOMODE ));	
	
	// webtv
	mainMenu.addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_WEBTVMODE, true, this, "webtv", CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW, NEUTRINO_ICON_MENUITEM_WEBTV, LOCALE_HELPTEXT_SCART) );
	
#if defined (ENABLE_SCART)
	// webtv
	mainMenu.addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_SCARTMODE, true, this, "scart", CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE, NEUTRINO_ICON_MENUITEM_SCART, LOCALE_HELPTEXT_SCART) );
#endif
	mainMenu.addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_MEDIAPLAYER, true, new CMediaPlayerMenu(), NULL, CRCInput::convertDigitToKey(shortcut++), NULL, NEUTRINO_ICON_MENUITEM_MOVIEPLAYER, LOCALE_HELPTEXT_MEDIAPLAYER ));
	
	// main setting menu
	mainMenu.addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_SETTINGS, true, new CMainSetup(), NULL, CRCInput::convertDigitToKey(shortcut++), NULL, NEUTRINO_ICON_MENUITEM_MAINSETTINGS, LOCALE_HELPTEXT_MAINSETTINGS ));

	// service
	mainMenu.addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_SERVICE, true, new CServiceSetup(), NULL, CRCInput::convertDigitToKey(shortcut++), NULL, NEUTRINO_ICON_MENUITEM_SERVICE, LOCALE_HELPTEXT_SERVICE ));
	
	
	// timerlist
	mainMenu.addItem(new CMenuForwarderExtended(LOCALE_TIMERLIST_NAME, true, new CTimerList, NULL, CRCInput::convertDigitToKey(shortcut++), NULL, NEUTRINO_ICON_MENUITEM_TIMERLIST, LOCALE_HELPTEXT_TIMERLIST ));
	
	// features
	mainMenu.addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_FEATURES, true, this, "features", CRCInput::convertDigitToKey(shortcut++), NULL, NEUTRINO_ICON_MENUITEM_PLUGINS, LOCALE_HELPTEXT_FEATURES ));

	// power menu
	mainMenu.addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_POWERMENU, true, new CPowerMenu(), NULL, CRCInput::RC_standby, NEUTRINO_ICON_BUTTON_POWER, NEUTRINO_ICON_MENUITEM_SHUTDOWN, LOCALE_HELPTEXT_SHUTDOWN ));

	//box info
	mainMenu.addItem( new CMenuForwarderExtended(LOCALE_DBOXINFO, true, new CDBoxInfoWidget, NULL, CRCInput::RC_info, NEUTRINO_ICON_BUTTON_HELP_SMALL, NEUTRINO_ICON_MENUITEM_BOXINFO, LOCALE_HELPTEXT_BOXINFO ));
}

void CNeutrinoApp::smartMenu()
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::smartMenu\n");

	int i = 0;
	int j = 0;
REPAINT:  
	CBox Box;
	
	Box.iX = g_settings.screen_StartX + 20;
	Box.iY = g_settings.screen_StartY + 20;
	Box.iWidth = g_settings.screen_EndX - g_settings.screen_StartX - 40;
	Box.iHeight = (g_settings.screen_EndY - g_settings.screen_StartY - 40);
	
	// paintBox (background)
	CFrameBuffer::getInstance()->paintBoxRel(Box.iX, Box.iY, Box.iWidth, Box.iHeight, /*COL_MENUCONTENT_PLUS_0*/COL_BACKGROUND);
	
	// paint horizontal line top
	CFrameBuffer::getInstance()->paintHLineRel(Box.iX + BORDER_LEFT, Box.iWidth - (BORDER_LEFT + BORDER_RIGHT), Box.iY + 35, COL_MENUCONTENT_PLUS_5);
	
	// paint horizontal line bottom
	CFrameBuffer::getInstance()->paintHLineRel(Box.iX + BORDER_LEFT, Box.iWidth - (BORDER_LEFT + BORDER_RIGHT), Box.iY + Box.iHeight - 35, COL_MENUCONTENT_PLUS_5);
	
	// paint title
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(Box.iX + BORDER_LEFT + ICON_OFFSET, Box.iY + (35 - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), Box.iWidth - (BORDER_LEFT + BORDER_RIGHT + ICON_OFFSET), g_Locale->getText(LOCALE_MAINMENU_HEAD), COL_MENUHEAD);
	
	// paint foot
	if(i == 0 && j == 0)
	{
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(Box.iX + BORDER_LEFT + ICON_OFFSET, Box.iY + Box.iHeight - 35 + (35 - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), Box.iWidth - (BORDER_LEFT + BORDER_RIGHT + ICON_OFFSET), g_Locale->getText(LOCALE_MAINMENU_TVMODE), COL_MENUHEAD);
	}
	else if(i == 1 && j == 0)
	{
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(Box.iX + BORDER_LEFT + ICON_OFFSET, Box.iY + Box.iHeight - 35 + (35 - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), Box.iWidth - (BORDER_LEFT + BORDER_RIGHT + ICON_OFFSET), g_Locale->getText(LOCALE_MAINMENU_RADIOMODE), COL_MENUHEAD);
	}
	else if(i == 2 && j == 0)
	{
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(Box.iX + BORDER_LEFT + ICON_OFFSET, Box.iY + Box.iHeight - 35 + (35 - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), Box.iWidth - (BORDER_LEFT + BORDER_RIGHT + ICON_OFFSET), g_Locale->getText(LOCALE_MAINMENU_WEBTVMODE), COL_MENUHEAD);
	}
	else if(i == 3 && j == 0)
	{
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(Box.iX + BORDER_LEFT + ICON_OFFSET, Box.iY + Box.iHeight - 35 + (35 - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), Box.iWidth - (BORDER_LEFT + BORDER_RIGHT + ICON_OFFSET), g_Locale->getText(LOCALE_MAINMENU_SCARTMODE), COL_MENUHEAD);
	}
	else if(i == 4 && j == 0)
	{
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(Box.iX + BORDER_LEFT + ICON_OFFSET, Box.iY + Box.iHeight - 35 + (35 - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), Box.iWidth - (BORDER_LEFT + BORDER_RIGHT + ICON_OFFSET), g_Locale->getText(LOCALE_MAINMENU_FEATURES), COL_MENUHEAD);
	}
	else if(i == 5 && j == 0)
	{
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(Box.iX + BORDER_LEFT + ICON_OFFSET, Box.iY + Box.iHeight - 35 + (35 - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), Box.iWidth - (BORDER_LEFT + BORDER_RIGHT + ICON_OFFSET), g_Locale->getText(LOCALE_MAINMENU_MEDIAPLAYER), COL_MENUHEAD);
	}
	else if(i == 0 && j == 1)
	{
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(Box.iX + BORDER_LEFT + ICON_OFFSET, Box.iY + Box.iHeight - 35 + (35 - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), Box.iWidth - (BORDER_LEFT + BORDER_RIGHT + ICON_OFFSET), g_Locale->getText(LOCALE_TIMERLIST_NAME), COL_MENUHEAD);
	}
	else if(i == 1 && j == 1)
	{
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(Box.iX + BORDER_LEFT + ICON_OFFSET, Box.iY + Box.iHeight - 35 + (35 - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), Box.iWidth - (BORDER_LEFT + BORDER_RIGHT + ICON_OFFSET), g_Locale->getText(LOCALE_DBOXINFO), COL_MENUHEAD);
	}
	else if(i == 2 && j == 1)
	{
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(Box.iX + BORDER_LEFT + ICON_OFFSET, Box.iY + Box.iHeight - 35 + (35 - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), Box.iWidth - (BORDER_LEFT + BORDER_RIGHT + ICON_OFFSET), g_Locale->getText(LOCALE_MAINMENU_SLEEPTIMER), COL_MENUHEAD);
	}
	else if(i == 3 && j == 1)
	{
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(Box.iX + BORDER_LEFT + ICON_OFFSET, Box.iY + Box.iHeight - 35 + (35 - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), Box.iWidth - (BORDER_LEFT + BORDER_RIGHT + ICON_OFFSET), g_Locale->getText(LOCALE_MAINMENU_SERVICE), COL_MENUHEAD);
	}
	else if(i == 4 && j == 1)
	{
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(Box.iX + BORDER_LEFT + ICON_OFFSET, Box.iY + Box.iHeight - 35 + (35 - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), Box.iWidth - (BORDER_LEFT + BORDER_RIGHT + ICON_OFFSET), g_Locale->getText(LOCALE_MAINMENU_SETTINGS), COL_MENUHEAD);
	}
	else if(i == 5 && j == 1)
	{
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(Box.iX + BORDER_LEFT + ICON_OFFSET, Box.iY + Box.iHeight - 35 + (35 - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), Box.iWidth - (BORDER_LEFT + BORDER_RIGHT + ICON_OFFSET), g_Locale->getText(LOCALE_MAINMENU_POWERMENU), COL_MENUHEAD);
	}
	
	// paint help buttons (foot)
	int iw, ih;
	CFrameBuffer::getInstance()->getIconSize(NEUTRINO_ICON_BUTTON_TOP, &iw, &ih);
	CFrameBuffer::getInstance()->paintIcon(NEUTRINO_ICON_BUTTON_TOP, Box.iX + Box.iWidth - BORDER_RIGHT - iw, Box.iY + Box.iHeight - 35 + (35 - ih)/2);
	
	CFrameBuffer::getInstance()->getIconSize(NEUTRINO_ICON_BUTTON_DOWN, &iw, &ih);
	CFrameBuffer::getInstance()->paintIcon(NEUTRINO_ICON_BUTTON_DOWN, Box.iX + Box.iWidth - BORDER_RIGHT - 2*iw - 2, Box.iY + Box.iHeight - 35 + (35 - ih)/2);
	
	CFrameBuffer::getInstance()->getIconSize(NEUTRINO_ICON_BUTTON_RIGHT, &iw, &ih);
	CFrameBuffer::getInstance()->paintIcon(NEUTRINO_ICON_BUTTON_RIGHT, Box.iX + Box.iWidth - BORDER_RIGHT - 3*iw - 2*2, Box.iY + Box.iHeight - 35 + (35 - ih)/2);
	
	CFrameBuffer::getInstance()->getIconSize(NEUTRINO_ICON_BUTTON_LEFT, &iw, &ih);
	CFrameBuffer::getInstance()->paintIcon(NEUTRINO_ICON_BUTTON_LEFT, Box.iX + Box.iWidth - BORDER_RIGHT - 4*iw - 3*2, Box.iY + Box.iHeight - 35 + (35 - ih)/2);
	
	// calculte frameBoxes
	CBox frameBox;
	
	frameBox.iX = Box.iX + BORDER_LEFT;
	frameBox.iY = Box.iY + 35 + 5;
	frameBox.iWidth = (Box.iWidth - (BORDER_LEFT + BORDER_RIGHT))/6;
	frameBox.iHeight = (Box.iHeight - 80)/3;
	
	// framBox
	CFrameBuffer::getInstance()->paintBoxRel(frameBox.iX + frameBox.iWidth*i, frameBox.iY + frameBox.iHeight*j, frameBox.iWidth, frameBox.iHeight, COL_MENUCONTENT_PLUS_6, RADIUS_SMALL, CORNER_BOTH);
	
	// fixme: use array
	// paint tv mode
	std::string tv_logo = DATADIR "/neutrino/icons/tv.png";
	CFrameBuffer::getInstance()->DisplayImage(tv_logo, frameBox.iX + 0*frameBox.iWidth + 5, frameBox.iY + 0*frameBox.iHeight + 5, frameBox.iWidth - 10, frameBox.iHeight - 10);
	
	// paint radio mode
	std::string radio_logo = DATADIR "/neutrino/icons/radio.png";
	CFrameBuffer::getInstance()->DisplayImage(radio_logo, frameBox.iX + 1*frameBox.iWidth + 5, frameBox.iY + 0*frameBox.iHeight + 5, frameBox.iWidth - 10, frameBox.iHeight - 10);
	
	// paint webtv
	std::string webtv_logo = DATADIR "/neutrino/icons/webtv.png";
	CFrameBuffer::getInstance()->DisplayImage(webtv_logo, frameBox.iX + 2*frameBox.iWidth + 5, frameBox.iY + 0*frameBox.iHeight + 5, frameBox.iWidth - 10, frameBox.iHeight - 10);
	
	// paint scart
	std::string scart_logo = DATADIR "/neutrino/icons/scart.png";
	CFrameBuffer::getInstance()->DisplayImage(scart_logo, frameBox.iX + 3*frameBox.iWidth + 5, frameBox.iY + 0*frameBox.iHeight + 5, frameBox.iWidth - 10, frameBox.iHeight - 10);
	
	// paint features
	std::string features_logo = DATADIR "/neutrino/icons/plugins.png";
	CFrameBuffer::getInstance()->DisplayImage(features_logo, frameBox.iX + 4*frameBox.iWidth + 5, frameBox.iY + 0*frameBox.iHeight + 5, frameBox.iWidth - 10, frameBox.iHeight - 10);
	
	// paint mediaplayer
	std::string mediaplayer_logo = DATADIR "/neutrino/icons/movieplayer.png";
	CFrameBuffer::getInstance()->DisplayImage(mediaplayer_logo, frameBox.iX + 5*frameBox.iWidth + 5, frameBox.iY + 0*frameBox.iHeight + 5, frameBox.iWidth - 10, frameBox.iHeight - 10);
	
	// paint timerlist
	std::string timerlist_logo = DATADIR "/neutrino/icons/timerlist.png";
	CFrameBuffer::getInstance()->DisplayImage(timerlist_logo, frameBox.iX + 0*frameBox.iWidth + 5, frameBox.iY + 1*frameBox.iHeight + 5, frameBox.iWidth - 10, frameBox.iHeight - 10);
	
	// paint boxinfo
	std::string boxinfo_logo = DATADIR "/neutrino/icons/boxinfo.png";
	CFrameBuffer::getInstance()->DisplayImage(boxinfo_logo, frameBox.iX + 1*frameBox.iWidth + 5, frameBox.iY + 1*frameBox.iHeight + 5, frameBox.iWidth - 10, frameBox.iHeight - 10);
	
	// paint sleeptimer
	std::string sleeptimer_logo = DATADIR "/neutrino/icons/sleeptimer.png";
	CFrameBuffer::getInstance()->DisplayImage(sleeptimer_logo, frameBox.iX + 2*frameBox.iWidth + 5, frameBox.iY + 1*frameBox.iHeight + 5, frameBox.iWidth - 10, frameBox.iHeight - 10);
	
	// paint service
	std::string service_logo = DATADIR "/neutrino/icons/service.png";
	CFrameBuffer::getInstance()->DisplayImage(service_logo, frameBox.iX + 3*frameBox.iWidth + 5, frameBox.iY + 1*frameBox.iHeight + 5, frameBox.iWidth - 10, frameBox.iHeight - 10);
	
	// paint setup
	std::string setup_logo = DATADIR "/neutrino/icons/mainsettings.png";
	CFrameBuffer::getInstance()->DisplayImage(setup_logo, frameBox.iX + 4*frameBox.iWidth + 5, frameBox.iY + 1*frameBox.iHeight + 5, frameBox.iWidth - 10, frameBox.iHeight - 10);
	
	// paint power menu
	std::string shutdown_logo = DATADIR "/neutrino/icons/" NEUTRINO_ICON_MENUITEM_SHUTDOWN ".png";
	CFrameBuffer::getInstance()->DisplayImage(shutdown_logo, frameBox.iX + 5*frameBox.iWidth + 5, frameBox.iY + 1*frameBox.iHeight + 5, frameBox.iWidth - 10, frameBox.iHeight - 10);
	
	// blit all
	CFrameBuffer::getInstance()->blit();
	
	// loop
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;
	
	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);

	bool loop = true;
	while (loop) 
	{
		g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd);

		if ((msg == CRCInput::RC_timeout) || (msg == CRCInput::RC_home) || (msg == CRCInput::RC_setup))
		{
			loop = false;
		}
		else if(msg == CRCInput::RC_ok)
		{
			if(i == 0 && j == 0)
			{
				CFrameBuffer::getInstance()->paintBackground();
				CFrameBuffer::getInstance()->blit();
					
				CNeutrinoApp::getInstance()->exec(NULL, "tv");
			}
			else if(i == 1 && j == 0)
			{
				CFrameBuffer::getInstance()->paintBackground();
				CFrameBuffer::getInstance()->blit();
					
				CNeutrinoApp::getInstance()->exec(NULL, "radio");
			}
			else if(i == 2 && j == 0)
			{
				CFrameBuffer::getInstance()->paintBackground();
				CFrameBuffer::getInstance()->blit();
					
				CNeutrinoApp::getInstance()->exec(NULL, "webtv");
			}
			else if(i == 3 && j == 0)
			{
				CFrameBuffer::getInstance()->paintBackground();
				CFrameBuffer::getInstance()->blit();
					
				CNeutrinoApp::getInstance()->exec(NULL, "scart");
			}
			else if(i == 4 && j == 0)
			{
				CFrameBuffer::getInstance()->paintBackground();
				CFrameBuffer::getInstance()->blit();
					
				CNeutrinoApp::getInstance()->exec(NULL, "features");
			}
			else if(i == 5 && j == 0)
			{
				CFrameBuffer::getInstance()->paintBackground();
				CFrameBuffer::getInstance()->blit();
				
				CMediaPlayerMenu tmpMediaPlayerMenu;
				tmpMediaPlayerMenu.exec(NULL, "");
			}
			else if(i == 0 && j == 1)
			{
				CFrameBuffer::getInstance()->paintBackground();
				CFrameBuffer::getInstance()->blit();
				
				CTimerList tmpTimerList;
				tmpTimerList.exec(NULL, "");
			}
			else if(i == 1 && j == 1)
			{
				CFrameBuffer::getInstance()->paintBackground();
				CFrameBuffer::getInstance()->blit();
				
				CDBoxInfoWidget tmpBoxInfo;
				tmpBoxInfo.exec(NULL, "");
			}
			else if(i == 2 && j == 1)
			{
				CFrameBuffer::getInstance()->paintBackground();
				CFrameBuffer::getInstance()->blit();
					
				CSleepTimerWidget tmpSleepTimer;
				tmpSleepTimer.exec(NULL, "");
			}
			else if(i == 3 && j == 1)
			{
				CFrameBuffer::getInstance()->paintBackground();
				CFrameBuffer::getInstance()->blit();
					
				CServiceSetup tmpServiceSetup;
				tmpServiceSetup.exec(NULL, "");
			}
			else if(i == 4 && j == 1)
			{
				CFrameBuffer::getInstance()->paintBackground();
				CFrameBuffer::getInstance()->blit();
					
				CMainSetup tmpMainSetup;
				tmpMainSetup.exec(NULL, "");
			}
			else if(i == 5 && j == 1)
			{
				CFrameBuffer::getInstance()->paintBackground();
				CFrameBuffer::getInstance()->blit();
					
				//CNeutrinoApp::getInstance()->exec(NULL, "shutdown");
				CPowerMenu tmpPowerMenu;
				tmpPowerMenu.exec(NULL, "");
			}
			
			goto REPAINT;
		}
		else if (msg == CRCInput::RC_right)
		{
			i++;
			if (i >= 6)
			{
				i = 0;
				j++;
				
				if(j >= 3)
					j = 0;
			}
			
			goto REPAINT;
		}
		else if (msg == CRCInput::RC_left)
		{
			i--;
			if(i < 0 && j > 0)
			{
				i = 5;
				j--;
				
				if(j < 0)
					j = 0;
			}
			
			// stay at first frameBox
			if (i < 0)
				i = 0;
			
			goto REPAINT;
		}
		else if (msg == CRCInput::RC_down)
		{
			j++;
			if (j > 2)
				j = 2;
			
			goto REPAINT;
		}
		else if (msg == CRCInput::RC_up)
		{
			j--;
			if (j < 0)
				j = 0;
			
			goto REPAINT;
		}
		else
		{
			CNeutrinoApp::getInstance()->handleMsg( msg, data );
			// kein canceling...
		}
	}
	
	// hide and exit
	CFrameBuffer::getInstance()->paintBackground();
	CFrameBuffer::getInstance()->blit();
}

// User menu
extern CRemoteControl * g_RemoteControl;	// defined neutrino.cpp

// option off0_on1
#define OPTIONS_OFF0_ON1_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_OFF0_ON1_OPTIONS[OPTIONS_OFF0_ON1_OPTION_COUNT] =
{
        { 0, LOCALE_OPTIONS_OFF, NULL },
        { 1, LOCALE_OPTIONS_ON, NULL }
};

// leave this functions, somebody might want to use it in the future again
void CNeutrinoApp::SelectNVOD()
{
        if (!(g_RemoteControl->subChannels.empty()))
        {
                // NVOD/SubService- Kanal!
                CMenuWidget NVODSelector(g_RemoteControl->are_subchannels ? LOCALE_NVODSELECTOR_SUBSERVICE : LOCALE_NVODSELECTOR_HEAD, NEUTRINO_ICON_VIDEO);
		
                if(getNVODMenu(&NVODSelector))
                        NVODSelector.exec(NULL, "");
        }
}

bool CNeutrinoApp::getNVODMenu(CMenuWidget * menu)
{
        if(menu == NULL)
                return false;
	
        if (g_RemoteControl->subChannels.empty())
                return false;

        int count = 0;
        char nvod_id[5];

        for( CSubServiceListSorted::iterator e = g_RemoteControl->subChannels.begin(); e != g_RemoteControl->subChannels.end(); ++e)
        {
                sprintf(nvod_id, "%d", count);

                if( !g_RemoteControl->are_subchannels ) 
		{
                        char nvod_time_a[50], nvod_time_e[50], nvod_time_x[50];
                        char nvod_s[100];
                        struct  tm *tmZeit;

                        tmZeit= localtime(&e->startzeit);
                        sprintf(nvod_time_a, "%02d:%02d", tmZeit->tm_hour, tmZeit->tm_min);

                        time_t endtime = e->startzeit+ e->dauer;
                        tmZeit= localtime(&endtime);
                        sprintf(nvod_time_e, "%02d:%02d", tmZeit->tm_hour, tmZeit->tm_min);

                        time_t jetzt=time(NULL);
                        if(e->startzeit > jetzt) 
			{
                                int mins=(e->startzeit- jetzt)/ 60;
                                sprintf(nvod_time_x, g_Locale->getText(LOCALE_NVOD_STARTING), mins);
                        }
                        else if( (e->startzeit<= jetzt) && (jetzt < endtime) ) 
			{
                                int proz=(jetzt- e->startzeit)*100/ e->dauer;
                                sprintf(nvod_time_x, g_Locale->getText(LOCALE_NVOD_PERCENTAGE), proz);
                        }
                        else
                                nvod_time_x[0]= 0;

                        sprintf(nvod_s, "%s - %s %s", nvod_time_a, nvod_time_e, nvod_time_x);
                        menu->addItem(new CMenuForwarder(nvod_s, true, NULL, NVODChanger, nvod_id), (count == g_RemoteControl->selected_subchannel));
                } 
		else 
		{
			if (count == 0)
				menu->addItem(new CMenuForwarder( (Latin1_to_UTF8(e->subservice_name)).c_str(), true, NULL, NVODChanger, nvod_id, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE));
			else
				menu->addItem(new CMenuForwarder( (Latin1_to_UTF8(e->subservice_name)).c_str(), true, NULL, NVODChanger, nvod_id, CRCInput::convertDigitToKey(count)), (count == g_RemoteControl->selected_subchannel));
                }

                count++;
        }

        if( g_RemoteControl->are_subchannels ) 
	{
                menu->addItem(new CMenuSeparator(CMenuSeparator::LINE));
                CMenuOptionChooser* oj = new CMenuOptionChooser(LOCALE_NVODSELECTOR_DIRECTORMODE, &g_RemoteControl->director_mode, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, NULL, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW);
                menu->addItem(oj);
        }

        return true;
}

// This is just a quick helper for the usermenu only. I already made it a class for future use.
#if defined (ENABLE_FUNCTIONKEYS) //FIXME:???
#define BUTTONMAX 8
#else
#define BUTTONMAX 4
#endif

const neutrino_msg_t key_helper_msg_def[BUTTONMAX] = {
	CRCInput::RC_red,
	CRCInput::RC_green,
	CRCInput::RC_yellow,
	CRCInput::RC_blue,
#if defined (ENABLE_FUNCTIONKEYS)
	CRCInput::RC_f1,
	CRCInput::RC_f2,
	CRCInput::RC_f3,
	CRCInput::RC_f4
#endif
};

const char * key_helper_icon_def[BUTTONMAX]={
	NEUTRINO_ICON_BUTTON_RED, 
	NEUTRINO_ICON_BUTTON_GREEN, 
	NEUTRINO_ICON_BUTTON_YELLOW, 
	NEUTRINO_ICON_BUTTON_BLUE,
#if defined (ENABLE_FUNCTIONKEYS)	
	NEUTRINO_ICON_BUTTON_F1, 
	NEUTRINO_ICON_BUTTON_F2, 
	NEUTRINO_ICON_BUTTON_F3, 
	NEUTRINO_ICON_BUTTON_F4, 
#endif
};

class CKeyHelper
{
        private:
                int number_key;
                bool color_key_used[BUTTONMAX];
        public:
                CKeyHelper(){reset();};
                void reset(void)
                {
                        number_key = 1;
                        for(int i= 0; i < BUTTONMAX; i++ )
                                color_key_used[i] = false;
                };

                /* Returns the next available button, to be used in menu as 'direct' keys. Appropriate
                 * definitions are returnd in msp and icon
                 * A color button could be requested as prefered button (other buttons are not supported yet).
                 * If the appropriate button is already in used, the next number_key button is returned instead
                 * (first 1-9 and than 0). */
                bool get(neutrino_msg_t* msg, const char** icon, neutrino_msg_t prefered_key = CRCInput::RC_nokey)
                {
                        bool result = false;
                        int button = -1;
                        if(prefered_key == CRCInput::RC_red)
                                button = 0;
                        if(prefered_key == CRCInput::RC_green)
                                button = 1;
                        if(prefered_key == CRCInput::RC_yellow)
                                button = 2;
                        if(prefered_key == CRCInput::RC_blue)
                                button = 3;
#if defined (ENABLE_FUNCTIONKEYS) //FIXME:???
			if(prefered_key == CRCInput::RC_f1)
                                button = 4;
			if(prefered_key == CRCInput::RC_f2)
                                button = 5;
			if(prefered_key == CRCInput::RC_f3)
                                button = 6;
			if(prefered_key == CRCInput::RC_f4)
                                button = 7;
#endif

                        *msg = CRCInput::RC_nokey;
                        *icon = "";
                        if(button >= 0 && button < BUTTONMAX)
                        {
				// try to get color button
                                if( color_key_used[button] == false)
                                {
                                        color_key_used[button] = true;
                                        *msg = key_helper_msg_def[button];
                                        *icon = key_helper_icon_def[button];
                                        result = true;
                                }
                        }

                        if( result == false && number_key < 10) // no key defined yet, at least try to get a numbered key
                        {
                                // there is still a available number_key
                                *msg = CRCInput::convertDigitToKey(number_key);
                                *icon = "";
                                if(number_key == 9)
                                        number_key = 0;
                                else if(number_key == 0)
                                        number_key = 10;
                                else
                                        number_key++;
                                result = true;
                        }
                        return (result);
                };
};

bool CNeutrinoApp::showUserMenu(int button)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::showUserMenu\n");
	
        if(button < 0 || button >= SNeutrinoSettings::BUTTON_MAX)
                return false;

        CMenuItem * menu_item = NULL;
        CKeyHelper keyhelper;
        neutrino_msg_t key = CRCInput::RC_nokey;
        const char * icon = NULL;

        int menu_items = 0;
        int menu_prev = -1;
	static int selected[SNeutrinoSettings::BUTTON_MAX] = {
		-1, 
		-1, 
		-1, 
		-1,
#if defined (ENABLE_FUNCTIONKEYS) //FIXME:???
		-1,
		-1,
		-1,
		-1,
#endif		
	};

        // define classes
        CAudioSelectMenuHandler * tmpAudioSelectMenuHandler     = NULL;
        CMenuWidget * tmpNVODSelector                           = NULL;
        CStreamInfo2Handler * tmpStreamInfo2Handler          	= NULL;
        CEventListHandler * tmpEventListHandler                 = NULL;
        CEPGplusHandler * tmpEPGplusHandler                     = NULL;
        CEPGDataHandler * tmpEPGDataHandler                     = NULL;
	COPKGManager * tmpOPKGManager				= NULL;

        std::string txt = g_settings.usermenu_text[button];

        if (button == SNeutrinoSettings::BUTTON_RED) 
	{
                if( txt.empty() )
                        txt = g_Locale->getText(LOCALE_INFOVIEWER_EVENTLIST);
        }
        else if( button == SNeutrinoSettings::BUTTON_GREEN) 
	{
                if( txt.empty() )
                        txt = g_Locale->getText(LOCALE_INFOVIEWER_LANGUAGES);
        }
        else if( button == SNeutrinoSettings::BUTTON_YELLOW) 
	{
                if( txt.empty() )
                        txt = g_Locale->getText((g_RemoteControl->are_subchannels) ? LOCALE_INFOVIEWER_SUBSERVICE : LOCALE_INFOVIEWER_SELECTTIME);
        }
        else if( button == SNeutrinoSettings::BUTTON_BLUE) 
	{
                if( txt.empty() )
                        txt = g_Locale->getText(LOCALE_MAINMENU_FEATURES);
        }

        CMenuWidget * menu = new CMenuWidget(txt.c_str() , NEUTRINO_ICON_FEATURES);
        if (menu == NULL)
                return 0;
	
	// intros
	menu->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	menu->addItem( new CMenuSeparator(CMenuSeparator::LINE) );

        // go through any postition number
        for(int pos = 0; pos < SNeutrinoSettings::ITEM_MAX ; pos++) 
	{
                // now compare pos with the position of any item. Add this item if position is the same
                switch(g_settings.usermenu[button][pos]) 
		{
			case SNeutrinoSettings::ITEM_NONE:
				// do nothing
				break;

			case SNeutrinoSettings::ITEM_BAR:
				if(menu_prev == -1 || menu_prev == SNeutrinoSettings::ITEM_BAR )
					break;

				menu->addItem(new CMenuSeparator(CMenuSeparator::LINE));
				menu_prev = SNeutrinoSettings::ITEM_BAR;
				break;

                        case SNeutrinoSettings::ITEM_MOVIEPLAYER_TSMB:
                                menu_items++;
                                menu_prev = SNeutrinoSettings::ITEM_MOVIEPLAYER_TSMB;
                                keyhelper.get(&key, &icon, CRCInput::RC_green);
				menu_item = new CMenuForwarder(LOCALE_MOVIEPLAYER_RECORDS, true, NULL, new CMoviePlayerGui(), "tsmoviebrowser", key, icon);
                                menu->addItem(menu_item, false);
                                break;
				
			case SNeutrinoSettings::ITEM_MOVIEPLAYER_MB:
                                menu_items++;
                                menu_prev = SNeutrinoSettings::ITEM_MOVIEPLAYER_MB;
                                keyhelper.get(&key, &icon, CRCInput::RC_green);
				menu_item = new CMenuForwarder(LOCALE_MOVIEPLAYER_MOVIES, true, NULL, new CMoviePlayerGui(), "moviebrowser", key, icon);
                                menu->addItem(menu_item, false);
                                break;

                        case SNeutrinoSettings::ITEM_TIMERLIST:
                                menu_items++;
                                menu_prev = SNeutrinoSettings::ITEM_TIMERLIST;
                                keyhelper.get(&key, &icon, CRCInput::RC_yellow);
				menu_item = new CMenuForwarder(LOCALE_TIMERLIST_NAME, true, NULL, new CTimerList, "-1", key, icon);
                                menu->addItem(menu_item, false);
                                break;

                        case SNeutrinoSettings::ITEM_REMOTE:
                                menu_items++;
                                menu_prev = SNeutrinoSettings::ITEM_REMOTE;
                                keyhelper.get(&key, &icon);
				menu_item = new CMenuForwarder(LOCALE_RCLOCK_MENUEADD, true, NULL, this->rcLock, "-1", key, icon);
                                menu->addItem(menu_item, false);
                                break;

                        case SNeutrinoSettings::ITEM_EPG_SUPER:
				if (CNeutrinoApp::getInstance()->getMode() != NeutrinoMessages::mode_iptv)
				{
					menu_items++;
					menu_prev = SNeutrinoSettings::ITEM_EPG_SUPER;
					tmpEPGplusHandler = new CEPGplusHandler();
					keyhelper.get(&key, &icon, CRCInput::RC_green);
					menu_item = new CMenuForwarder(LOCALE_EPGMENU_EPGPLUS, true, NULL, tmpEPGplusHandler  ,  "-1", key, icon);
					menu->addItem(menu_item, false);
				}
                                break;

                        case SNeutrinoSettings::ITEM_EPG_LIST:
				if (CNeutrinoApp::getInstance()->getMode() != NeutrinoMessages::mode_iptv)
				{
					menu_items++;
					menu_prev = SNeutrinoSettings::ITEM_EPG_LIST;
					tmpEventListHandler = new CEventListHandler();
					keyhelper.get(&key, &icon, CRCInput::RC_red);
					menu_item = new CMenuForwarder(LOCALE_EPGMENU_EVENTLIST, true, NULL, tmpEventListHandler,  "-1", key, icon);
					menu->addItem(menu_item, false);
				}
                                break;

                        case SNeutrinoSettings::ITEM_EPG_INFO:
				if (CNeutrinoApp::getInstance()->getMode() != NeutrinoMessages::mode_iptv)
				{
					menu_items++;
					menu_prev = SNeutrinoSettings::ITEM_EPG_INFO;
					tmpEPGDataHandler = new CEPGDataHandler();
					keyhelper.get(&key, &icon, CRCInput::RC_yellow);
					menu_item = new CMenuForwarder(LOCALE_EPGMENU_EVENTINFO, true, NULL, tmpEPGDataHandler ,  "-1", key, icon);
					menu->addItem(menu_item, false);
				}
                                break;

                        case SNeutrinoSettings::ITEM_AUDIO_SELECT:
				if (CNeutrinoApp::getInstance()->getMode() != NeutrinoMessages::mode_iptv)
				{
					menu_items++;
					menu_prev = SNeutrinoSettings::ITEM_AUDIO_SELECT;
					tmpAudioSelectMenuHandler = new CAudioSelectMenuHandler;
					keyhelper.get(&key, &icon);
					menu_item = new CMenuForwarder(LOCALE_AUDIOSELECTMENUE_HEAD, true, NULL, tmpAudioSelectMenuHandler, "-1", key,icon);
					menu->addItem(menu_item, false);
				}
                                break;

                        case SNeutrinoSettings::ITEM_SUBCHANNEL:
				if (CNeutrinoApp::getInstance()->getMode() != NeutrinoMessages::mode_iptv)
				{
					if (!(g_RemoteControl->subChannels.empty())) 
					{
						// NVOD/SubService- Kanal!
						tmpNVODSelector = new CMenuWidget(g_RemoteControl->are_subchannels ? LOCALE_NVODSELECTOR_SUBSERVICE : LOCALE_NVODSELECTOR_HEAD, NEUTRINO_ICON_VIDEO);
						
						if(getNVODMenu(tmpNVODSelector)) 
						{
							menu_items++;
							menu_prev = SNeutrinoSettings::ITEM_SUBCHANNEL;
							keyhelper.get(&key, &icon);
							menu_item = new CMenuForwarder(g_RemoteControl->are_subchannels ? LOCALE_NVODSELECTOR_SUBSERVICE : LOCALE_NVODSELECTOR_HEAD, true, NULL, tmpNVODSelector, "-1", key,icon);
							menu->addItem(menu_item, false);
						}
					}
				}
                                break;

                        case SNeutrinoSettings::ITEM_TECHINFO:
				if (CNeutrinoApp::getInstance()->getMode() != NeutrinoMessages::mode_iptv)
				{
					menu_items++;
					menu_prev = SNeutrinoSettings::ITEM_TECHINFO;
					tmpStreamInfo2Handler = new CStreamInfo2Handler();
					keyhelper.get(&key, &icon, CRCInput::RC_blue);
					menu_item = new CMenuForwarder(LOCALE_EPGMENU_STREAMINFO, true, NULL, tmpStreamInfo2Handler, "-1", key, icon);
					menu->addItem(menu_item, false);
				}
                                break;
				
			case SNeutrinoSettings::ITEM_VTXT:
				if (CNeutrinoApp::getInstance()->getMode() != NeutrinoMessages::mode_iptv)
				{
					menu_items++;
					menu_prev = SNeutrinoSettings::ITEM_VTXT;
					keyhelper.get(&key, &icon);
					menu_item = new CMenuForwarder(LOCALE_USERMENU_ITEM_VTXT, true, NULL, /*TuxtxtChanger*/new CTuxtxtChangeExec, "-1", key, icon);
					menu->addItem(menu_item, false);
				}
                                break;	

			case SNeutrinoSettings::ITEM_OPKG:
                                menu_items++;
                                menu_prev = SNeutrinoSettings::ITEM_OPKG;
                               
				tmpOPKGManager = new COPKGManager();
				
                                keyhelper.get(&key, &icon);
                                menu_item = new CMenuForwarder(LOCALE_OPKG_MANAGER, true, NULL, tmpOPKGManager, "-1", key, icon);
                                menu->addItem(menu_item, false);
                                break;
			
			// plugins
                        case SNeutrinoSettings::ITEM_PLUGIN:
				{
					menu_item++;
					menu_prev = SNeutrinoSettings::ITEM_PLUGIN;
					keyhelper.get(&key, &icon, CRCInput::RC_blue);
					menu_item = new CMenuForwarder(LOCALE_USERMENU_ITEM_PLUGINS, true, NULL, new CPluginList( LOCALE_USERMENU_ITEM_PLUGINS, CPlugins::P_TYPE_NEUTRINO | CPlugins::P_TYPE_TOOL | CPlugins::P_TYPE_SCRIPT ), "-1", key, icon);
					menu->addItem(menu_item, false);
                                }
                                break;
				
			// games
			case SNeutrinoSettings::ITEM_GAME:
				{
					menu_item++;
					menu_prev = SNeutrinoSettings::ITEM_GAME;
					keyhelper.get(&key, &icon);
					menu_item = new CMenuForwarder(LOCALE_MAINMENU_GAMES, true, NULL, new CPluginList(LOCALE_MAINMENU_GAMES, CPlugins::P_TYPE_GAME), "-1", key, icon);
					menu->addItem(menu_item, false);
                                }
                                break;

                        default:
                                printf("[neutrino] WARNING! menu wrong item!!\n");
                                break;
                }
        }

        // show menu if there are more than 2 items only
	// otherwise, we start the item directly (must be the last one)
        if(menu_items > 1 ) 
	{
		menu->setSelected(selected[button]);
                menu->exec(NULL, "");
		selected[button] = menu->getSelected();
	}
        else if (menu_item != NULL)
                menu_item->exec( NULL );

        // clear the heap
        if(tmpAudioSelectMenuHandler)   
		delete tmpAudioSelectMenuHandler;

        if(tmpNVODSelector)
		delete tmpNVODSelector;

        if(tmpStreamInfo2Handler)
		delete tmpStreamInfo2Handler;

        if(tmpEventListHandler)
		delete tmpEventListHandler;

        if(tmpEPGplusHandler)
		delete tmpEPGplusHandler;

        if(tmpEPGDataHandler)
		delete tmpEPGDataHandler;
	
	if(tmpOPKGManager)
		delete tmpOPKGManager;

        if(menu)
		delete menu;

	return 0;
}

