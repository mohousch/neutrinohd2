/*
	$port: themes.cpp,v 1.16 2010/09/05 21:27:44 tuxbox-cvs Exp $
	
	Neutrino-GUI  -   DBoxII-Project

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

	Copyright (C) 2007, 2008, 2009 (flasher) Frank Liebelt

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>

#include <global.h>
#include <neutrino.h>

#include <system/setting_helpers.h>
#include <system/debug.h>

#include <gui/widget/menue.h>
#include <gui/widget/stringinput.h>
#include <gui/widget/stringinput_ext.h>
#include <gui/widget/messagebox.h>
#include <gui/widget/hintbox.h>

#include <driver/screen_max.h>

#include <sys/stat.h>
#include <sys/time.h>

#include <gui/themes.h>


#define THEMEDIR 		DATADIR "/neutrino/themes/"
#define USERDIR 		CONFIGDIR "/themes/"
#define FILE_PREFIX 		".theme"

CThemes::CThemes()
: themefile('\t')
{
	notifier = NULL;
}

int CThemes::exec(CMenuTarget * parent, const std::string & actionKey)
{
	dprintf(DEBUG_NORMAL, "CThemes::exec: actionKey:%s\n", actionKey.c_str());

	int res = menu_return::RETURN_REPAINT;

	if (parent)
		parent->hide();

	if( !actionKey.empty() )
	{
		if(actionKey == "savesettings")
		{
			CNeutrinoApp::getInstance()->exec(NULL, "savesettings");

			return res;
		}
		else if(actionKey == "saveCurrentTheme")
		{
			std::string file_name = "";
			CStringInputSMS nameInput(LOCALE_COLORTHEMEMENU_NAME, file_name.c_str());

			nameInput.exec(NULL, "");
			
			if (file_name.length() > 1)
			{
				HintBox(LOCALE_COLORTHEMEMENU_SAVE, g_Locale->getText(LOCALE_MAINSETTINGS_SAVESETTINGSNOW_HINT));

				saveFile((char*)((std::string)USERDIR + file_name + FILE_PREFIX).c_str());
			}

			Show();

			return menu_return::RETURN_EXIT;
		}
		else if (actionKey == "theme_default")
		{
			setupDefaultColors();
			notifier = new CColorSetupNotifier();
			notifier->changeNotify(NONEXISTANT_LOCALE, NULL);
			delete notifier;

			return res;
		}
		else
		{
			std::string themeFile = actionKey;
			if ( strstr(themeFile.c_str(), "{U}") != 0 ) 
			{
				themeFile.erase(0, 3);
				readFile((char*)((std::string)USERDIR + themeFile + FILE_PREFIX).c_str());
			} 
			else
				readFile((char*)((std::string)THEMEDIR + themeFile + FILE_PREFIX).c_str());

			return res;
		}
	}

	return Show();
}

void CThemes::readThemes(ClistBoxWidget &themes)
{
	struct dirent **themelist;
	int n;
	const char *pfade[] = {THEMEDIR, USERDIR};
	bool hasCVSThemes, hasUserThemes;
	hasCVSThemes = hasUserThemes = false;
	std::string userThemeFile = "";
	CMenuForwarder* oj;

	for(int p = 0; p < 2; p++)
	{
		n = scandir(pfade[p], &themelist, 0, alphasort);
		if(n < 0)
			perror("loading themes: scandir");
		else
		{
			for(int count = 0; count < n; count++)
			{
				char *file = themelist[count]->d_name;
				char *pos = strstr(file, ".theme");

				if(pos != NULL)
				{
					if ( p == 0 && hasCVSThemes == false ) 
					{
						themes.addItem(new CMenuSeparator(LINE | STRING, g_Locale->getText(LOCALE_COLORTHEMEMENU_SELECT2)));
						hasCVSThemes = true;
					} 
					else if ( p == 1 && hasUserThemes == false ) 
					{
						themes.addItem(new CMenuSeparator(LINE | STRING, g_Locale->getText(LOCALE_COLORTHEMEMENU_SELECT1)));
						hasUserThemes = true;
					}
					
					*pos = '\0';
					if ( p == 1 ) 
					{
						userThemeFile = "{U}" + (std::string)file;
						oj = new CMenuForwarder((char*)file, true, "", this, userThemeFile.c_str());
					} 
					else
						oj = new CMenuForwarder((char*)file, true, "", this, file);
					
					themes.addItem( oj );
				}
				free(themelist[count]);
			}
			free(themelist);
		}
	}
}

int CThemes::Show()
{
	dprintf(DEBUG_NORMAL, "CThemes::Show:\n");

	ClistBoxWidget themes(LOCALE_COLORMENU_MENUCOLORS, NEUTRINO_ICON_SETTINGS);

	themes.setMode(MODE_MENU);
	themes.enableShrinkMenu();

	// intros
	themes.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	themes.addItem( new CMenuSeparator(LINE) );
	
	// save settings
	themes.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", RC_red, NEUTRINO_ICON_BUTTON_RED));

	themes.addItem(new CMenuForwarder(LOCALE_COLORTHEMEMENU_SAVE, true , NULL, this, "saveCurrentTheme", RC_green, NEUTRINO_ICON_BUTTON_GREEN));

	themes.addItem( new CMenuSeparator(LINE) );
	
	//set default theme
	themes.addItem(new CMenuForwarder(LOCALE_COLORTHEMEMENU_DEFAULT_THEME, true, NULL, this, "theme_default" ));
	
	readThemes(themes);

	int res = themes.exec(NULL, "");
	
	return res;
}

void CThemes::readFile(char* themename)
{
	if(themefile.loadConfig(themename))
	{
		g_settings.menu_Head_alpha = themefile.getInt32( "menu_Head_alpha", 0);
		g_settings.menu_Head_red = themefile.getInt32( "menu_Head_red", 0);
		g_settings.menu_Head_green = themefile.getInt32( "menu_Head_green", 0);
		g_settings.menu_Head_blue = themefile.getInt32( "menu_Head_blue", 0);

		g_settings.menu_Head_Text_alpha = themefile.getInt32( "menu_Head_Text_alpha", 0);
		g_settings.menu_Head_Text_red = themefile.getInt32( "menu_Head_Text_red", 100 );
		g_settings.menu_Head_Text_green = themefile.getInt32( "menu_Head_Text_green", 100 );
		g_settings.menu_Head_Text_blue = themefile.getInt32( "menu_Head_Text_blue", 100 );
	
		g_settings.menu_Content_alpha = themefile.getInt32( "menu_Content_alpha", 20);
		g_settings.menu_Content_red = themefile.getInt32( "menu_Content_red", 25);
		g_settings.menu_Content_green = themefile.getInt32( "menu_Content_green", 25);
		g_settings.menu_Content_blue = themefile.getInt32( "menu_Content_blue", 25);
		g_settings.menu_Content_Text_alpha = themefile.getInt32( "menu_Content_Text_alpha", 0);
		g_settings.menu_Content_Text_red = themefile.getInt32( "menu_Content_Text_red", 85 );
		g_settings.menu_Content_Text_green = themefile.getInt32( "menu_Content_Text_green", 85 );
		g_settings.menu_Content_Text_blue = themefile.getInt32( "menu_Content_Text_blue", 85 );
	
		g_settings.menu_Content_Selected_alpha = themefile.getInt32( "menu_Content_Selected_alpha", 20);
		g_settings.menu_Content_Selected_red = themefile.getInt32( "menu_Content_Selected_red", 75);
		g_settings.menu_Content_Selected_green = themefile.getInt32( "menu_Content_Selected_green", 75);
		g_settings.menu_Content_Selected_blue = themefile.getInt32( "menu_Content_Selected_blue", 75);
		g_settings.menu_Content_Selected_Text_alpha = themefile.getInt32( "menu_Content_Selected_Text_alpha", 0);
		g_settings.menu_Content_Selected_Text_red = themefile.getInt32( "menu_Content_Selected_Text_red", 25 );
		g_settings.menu_Content_Selected_Text_green = themefile.getInt32( "menu_Content_Selected_Text_green", 25 );
		g_settings.menu_Content_Selected_Text_blue = themefile.getInt32( "menu_Content_Selected_Text_blue", 25 );
	
		g_settings.menu_Content_inactive_alpha = themefile.getInt32( "menu_Content_inactive_alpha", 20);
		g_settings.menu_Content_inactive_red = themefile.getInt32( "menu_Content_inactive_red", 25);
		g_settings.menu_Content_inactive_green = themefile.getInt32( "menu_Content_inactive_green", 25);
		g_settings.menu_Content_inactive_blue = themefile.getInt32( "menu_Content_inactive_blue", 25);
		g_settings.menu_Content_inactive_Text_alpha = themefile.getInt32( "menu_Content_inactive_Text_alpha", 0);
		g_settings.menu_Content_inactive_Text_red = themefile.getInt32( "menu_Content_inactive_Text_red", 55);
		g_settings.menu_Content_inactive_Text_green = themefile.getInt32( "menu_Content_inactive_Text_green", 55);
		g_settings.menu_Content_inactive_Text_blue = themefile.getInt32( "menu_Content_inactive_Text_blue", 55);

		g_settings.infobar_alpha = themefile.getInt32( "infobar_alpha", 0 );
		g_settings.infobar_red = themefile.getInt32( "infobar_red", 0 );
		g_settings.infobar_green = themefile.getInt32( "infobar_green", 0 );
		g_settings.infobar_blue = themefile.getInt32( "infobar_blue", 0 );
		g_settings.infobar_Text_alpha = themefile.getInt32( "infobar_Text_alpha", 0 );
		g_settings.infobar_Text_red = themefile.getInt32( "infobar_Text_red", 100 );
		g_settings.infobar_Text_green = themefile.getInt32( "infobar_Text_green", 100 );
		g_settings.infobar_Text_blue = themefile.getInt32( "infobar_Text_blue", 100 );
		
		g_settings.infobar_colored_events_alpha = themefile.getInt32( "infobar_colored_events_alpha", 0);
		g_settings.infobar_colored_events_red = themefile.getInt32( "infobar_colored_events_red", 95);
		g_settings.infobar_colored_events_green = themefile.getInt32( "infobar_colored_events_green", 70);
		g_settings.infobar_colored_events_blue = themefile.getInt32( "infobar_colored_events_blue", 0);
	
		g_settings.menu_Foot_alpha = themefile.getInt32( "menu_Foot_alpha", 0);
		g_settings.menu_Foot_red = themefile.getInt32( "menu_Foot_red", 0);
		g_settings.menu_Foot_green = themefile.getInt32( "menu_Foot_green", 0);
		g_settings.menu_Foot_blue = themefile.getInt32( "menu_Foot_blue", 0);
		
		g_settings.menu_Foot_Text_alpha = themefile.getInt32( "menu_Foot_Text_alpha", 0);
		g_settings.menu_Foot_Text_red = themefile.getInt32( "menu_Foot_Text_red", 100);
		g_settings.menu_Foot_Text_green = themefile.getInt32( "menu_Foot_Text_green", 100);
		g_settings.menu_Foot_Text_blue = themefile.getInt32( "menu_Foot_Text_blue", 100);

		g_settings.menu_FootInfo_alpha = themefile.getInt32( "menu_FootInfo_alpha", 0);
		g_settings.menu_FootInfo_red = themefile.getInt32( "menu_FootInfo_red", 0);
		g_settings.menu_FootInfo_green = themefile.getInt32( "menu_FootInfo_green", 0);
		g_settings.menu_FootInfo_blue = themefile.getInt32( "menu_FootInfo_blue", 0);
		
		g_settings.menu_FootInfo_Text_alpha = themefile.getInt32( "menu_FootInfo_Text_alpha", 0);
		g_settings.menu_FootInfo_Text_red = themefile.getInt32( "menu_FootInfo_Text_red", 50);
		g_settings.menu_FootInfo_Text_green = themefile.getInt32( "menu_FootInfo_Text_green", 50);
		g_settings.menu_FootInfo_Text_blue = themefile.getInt32( "menu_FootInfo_Text_blue", 50);

		notifier = new CColorSetupNotifier;
		notifier->changeNotify(NONEXISTANT_LOCALE, NULL);
		
		delete notifier;
	}
	else
		printf("[neutrino theme] %s not found\n", themename);
}

void CThemes::saveFile(char * themename)
{
	themefile.setInt32( "menu_Head_alpha", g_settings.menu_Head_alpha );
	themefile.setInt32( "menu_Head_red", g_settings.menu_Head_red );
	themefile.setInt32( "menu_Head_green", g_settings.menu_Head_green );
	themefile.setInt32( "menu_Head_blue", g_settings.menu_Head_blue );
	themefile.setInt32( "menu_Head_Text_alpha", g_settings.menu_Head_Text_alpha );
	themefile.setInt32( "menu_Head_Text_red", g_settings.menu_Head_Text_red );
	themefile.setInt32( "menu_Head_Text_green", g_settings.menu_Head_Text_green );
	themefile.setInt32( "menu_Head_Text_blue", g_settings.menu_Head_Text_blue );

	themefile.setInt32( "menu_Content_alpha", g_settings.menu_Content_alpha );
	themefile.setInt32( "menu_Content_red", g_settings.menu_Content_red );
	themefile.setInt32( "menu_Content_green", g_settings.menu_Content_green );
	themefile.setInt32( "menu_Content_blue", g_settings.menu_Content_blue );
	themefile.setInt32( "menu_Content_Text_alpha", g_settings.menu_Content_Text_alpha );
	themefile.setInt32( "menu_Content_Text_red", g_settings.menu_Content_Text_red );
	themefile.setInt32( "menu_Content_Text_green", g_settings.menu_Content_Text_green );
	themefile.setInt32( "menu_Content_Text_blue", g_settings.menu_Content_Text_blue );

	themefile.setInt32( "menu_Content_Selected_alpha", g_settings.menu_Content_Selected_alpha );
	themefile.setInt32( "menu_Content_Selected_red", g_settings.menu_Content_Selected_red );
	themefile.setInt32( "menu_Content_Selected_green", g_settings.menu_Content_Selected_green );
	themefile.setInt32( "menu_Content_Selected_blue", g_settings.menu_Content_Selected_blue );
	themefile.setInt32( "menu_Content_Selected_Text_alpha", g_settings.menu_Content_Selected_Text_alpha );
	themefile.setInt32( "menu_Content_Selected_Text_red", g_settings.menu_Content_Selected_Text_red );
	themefile.setInt32( "menu_Content_Selected_Text_green", g_settings.menu_Content_Selected_Text_green );
	themefile.setInt32( "menu_Content_Selected_Text_blue", g_settings.menu_Content_Selected_Text_blue );

	themefile.setInt32( "menu_Content_inactive_alpha", g_settings.menu_Content_inactive_alpha );
	themefile.setInt32( "menu_Content_inactive_red", g_settings.menu_Content_inactive_red );
	themefile.setInt32( "menu_Content_inactive_green", g_settings.menu_Content_inactive_green );
	themefile.setInt32( "menu_Content_inactive_blue", g_settings.menu_Content_inactive_blue );
	themefile.setInt32( "menu_Content_inactive_Text_alpha", g_settings.menu_Content_inactive_Text_alpha );
	themefile.setInt32( "menu_Content_inactive_Text_red", g_settings.menu_Content_inactive_Text_red );
	themefile.setInt32( "menu_Content_inactive_Text_green", g_settings.menu_Content_inactive_Text_green );
	themefile.setInt32( "menu_Content_inactive_Text_blue", g_settings.menu_Content_inactive_Text_blue );

	themefile.setInt32( "infobar_alpha", g_settings.infobar_alpha );
	themefile.setInt32( "infobar_red", g_settings.infobar_red );
	themefile.setInt32( "infobar_green", g_settings.infobar_green );
	themefile.setInt32( "infobar_blue", g_settings.infobar_blue );
	themefile.setInt32( "infobar_Text_alpha", g_settings.infobar_Text_alpha );
	themefile.setInt32( "infobar_Text_red", g_settings.infobar_Text_red );
	themefile.setInt32( "infobar_Text_green", g_settings.infobar_Text_green );
	themefile.setInt32( "infobar_Text_blue", g_settings.infobar_Text_blue );
	
	themefile.setInt32( "infobar_colored_events_alpha", g_settings.infobar_colored_events_alpha );
	themefile.setInt32( "infobar_colored_events_red", g_settings.infobar_colored_events_red );
	themefile.setInt32( "infobar_colored_events_green", g_settings.infobar_colored_events_green );
	themefile.setInt32( "infobar_colored_events_blue", g_settings.infobar_colored_events_blue );
	
	themefile.setInt32( "menu_Foot_alpha", g_settings.menu_Foot_alpha );
	themefile.setInt32( "menu_Foot_red", g_settings.menu_Foot_red );
	themefile.setInt32( "menu_Foot_green", g_settings.menu_Foot_green );
	themefile.setInt32( "menu_Foot_blue", g_settings.menu_Foot_blue );
	themefile.setInt32( "menu_Foot_Text_alpha", g_settings.menu_Foot_Text_alpha );
	themefile.setInt32( "menu_Foot_Text_red", g_settings.menu_Foot_Text_red );
	themefile.setInt32( "menu_Foot_Text_green", g_settings.menu_Foot_Text_green );
	themefile.setInt32( "menu_Foot_Text_blue", g_settings.menu_Foot_Text_blue );

	themefile.setInt32( "menu_FootInfo_alpha", g_settings.menu_FootInfo_alpha );
	themefile.setInt32( "menu_FootInfo_red", g_settings.menu_FootInfo_red );
	themefile.setInt32( "menu_FootInfo_green", g_settings.menu_FootInfo_green );
	themefile.setInt32( "menu_FootInfo_blue", g_settings.menu_FootInfo_blue );
	themefile.setInt32( "menu_FootInfo_Text_alpha", g_settings.menu_FootInfo_Text_alpha );
	themefile.setInt32( "menu_FootInfo_Text_red", g_settings.menu_FootInfo_Text_red );
	themefile.setInt32( "menu_FootInfo_Text_green", g_settings.menu_FootInfo_Text_green );
	themefile.setInt32( "menu_FootInfo_Text_blue", g_settings.menu_FootInfo_Text_blue );

	if (!themefile.saveConfig(themename))
		printf("[neutrino theme] %s write error\n", themename);
}



// setup default Colors
void CThemes::setupDefaultColors()
{
	g_settings.menu_Head_alpha = 0;
	g_settings.menu_Head_red = 0;
	g_settings.menu_Head_green = 0;
	g_settings.menu_Head_blue = 0;

	g_settings.menu_Head_Text_alpha = 0;
	g_settings.menu_Head_Text_red = 100;
	g_settings.menu_Head_Text_green = 100;
	g_settings.menu_Head_Text_blue = 100;
	
	g_settings.menu_Content_alpha = 20;
	g_settings.menu_Content_red = 25;
	g_settings.menu_Content_green = 25;
	g_settings.menu_Content_blue = 25;

	g_settings.menu_Content_Text_alpha = 0;
	g_settings.menu_Content_Text_red = 85;
	g_settings.menu_Content_Text_green = 85;
	g_settings.menu_Content_Text_blue = 85;
	
	g_settings.menu_Content_Selected_alpha = 20;
	g_settings.menu_Content_Selected_red = 75;
	g_settings.menu_Content_Selected_green = 75;
	g_settings.menu_Content_Selected_blue = 75;

	g_settings.menu_Content_Selected_Text_alpha = 0;
	g_settings.menu_Content_Selected_Text_red = 25;
	g_settings.menu_Content_Selected_Text_green = 25;
	g_settings.menu_Content_Selected_Text_blue = 25;
	
	g_settings.menu_Content_inactive_alpha = 20;
	g_settings.menu_Content_inactive_red = 25;
	g_settings.menu_Content_inactive_green = 25;
	g_settings.menu_Content_inactive_blue = 25;

	g_settings.menu_Content_inactive_Text_alpha = 0;
	g_settings.menu_Content_inactive_Text_red = 55;
	g_settings.menu_Content_inactive_Text_green = 55;
	g_settings.menu_Content_inactive_Text_blue = 55;

	g_settings.infobar_alpha = 0;
	g_settings.infobar_red = 0;
	g_settings.infobar_green = 0;
	g_settings.infobar_blue = 0;

	g_settings.infobar_Text_alpha = 0;
	g_settings.infobar_Text_red = 100;
	g_settings.infobar_Text_green = 100;
	g_settings.infobar_Text_blue = 100;
		
	g_settings.infobar_colored_events_alpha = 0;
	g_settings.infobar_colored_events_red = 95;
	g_settings.infobar_colored_events_green = 70;
	g_settings.infobar_colored_events_blue = 0;
	
	g_settings.menu_Foot_alpha = 0;
	g_settings.menu_Foot_red = 0;
	g_settings.menu_Foot_green = 0;
	g_settings.menu_Foot_blue = 0;
		
	g_settings.menu_Foot_Text_alpha = 0;
	g_settings.menu_Foot_Text_red = 100;
	g_settings.menu_Foot_Text_green = 100;
	g_settings.menu_Foot_Text_blue = 100;

	g_settings.menu_FootInfo_alpha = 0;
	g_settings.menu_FootInfo_red = 0;
	g_settings.menu_FootInfo_green = 0;
	g_settings.menu_FootInfo_blue = 0;
		
	g_settings.menu_FootInfo_Text_alpha = 0;
	g_settings.menu_FootInfo_Text_red = 50;
	g_settings.menu_FootInfo_Text_green = 50;
	g_settings.menu_FootInfo_Text_blue = 50;
}

