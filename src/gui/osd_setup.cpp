/*
	Neutrino-GUI  -   DBoxII-Project

	$id: osd_setup.cpp 2016.01.02 20:46:30 mohousch $
	
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
 
#include <sys/stat.h>
#include <dirent.h>

#include <gui/widget/hintbox.h>

#include <gui/widget/colorchooser.h>

#include <gui/filebrowser.h>
#include <gui/osd_setup.h>

#include <gui/themes.h>
#include <gui/screensetup.h>
#include <gui/alphasetup.h>

#include <system/debug.h>
#include <system/setting_helpers.h>
#include <system/helpers.h>


// osd settings
COSDSettings::COSDSettings()
{
}

COSDSettings::~COSDSettings()
{
}

int COSDSettings::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "COSDSettings::exec: actionKey: %s\n", actionKey.c_str());
	
	int ret = menu_return::RETURN_REPAINT;
	
	if(parent)
		parent->hide();
	
	if(actionKey == "savesettings")
	{
		CNeutrinoApp::getInstance()->exec(NULL, "savesettings");
		
		return ret;
	}
	else if(actionKey == "select_font")
	{
		CFileBrowser fileBrowser;
		CFileFilter fileFilter;
		fileFilter.addFilter("ttf");
		fileBrowser.Filter = &fileFilter;
		
		if (fileBrowser.exec(FONTDIR) == true)
		{
			strcpy(g_settings.font_file, fileBrowser.getSelectedFile()->Name.c_str());
			dprintf(DEBUG_NORMAL, "COSDSettings::exec: new font file %s\n", fileBrowser.getSelectedFile()->Name.c_str());
			
			CNeutrinoApp::getInstance()->SetupFonts();
		}
		
		return ret;
	}
	else if(actionKey == "font_scaling")
	{
		if(parent)
			parent->hide();
		
		CMenuWidget fontscale(LOCALE_FONTMENU_HEAD, NEUTRINO_ICON_COLORS);
		
		fontscale.enableSaveScreen(true);

		fontscale.addItem(new CMenuOptionNumberChooser(LOCALE_FONTMENU_SCALING_X, &g_settings.screen_xres, true, 50, 200, NULL) );
		fontscale.addItem(new CMenuOptionNumberChooser(LOCALE_FONTMENU_SCALING_Y, &g_settings.screen_yres, true, 50, 200, NULL) );
		
		fontscale.exec(NULL, "");
		
		CNeutrinoApp::getInstance()->SetupFonts();
		
		return ret;
	}
	
	showMenu();
	
	return ret;
}

void COSDSettings::showMenu()
{
	dprintf(DEBUG_NORMAL, "COSDSettings::showMenu:\n");
	
	CMenuWidget osdSettings(LOCALE_MAINSETTINGS_OSD, NEUTRINO_ICON_COLORS );
	
	int shortcutOSD = 1;
	
	// intros
	osdSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	osdSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	osdSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	osdSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	// Themes
	CThemes * osdSettings_Themes = new CThemes();
	
	osdSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_THEMESELECT, true, NULL, osdSettings_Themes, NULL, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN) );

	// menu colors
	osdSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_MENUCOLORS, true, NULL, new COSDMenuColorSettings(), NULL, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW) );

	// infobar
	//CMenuWidget * osdSettings_statusbarColors = new CMenuWidget(LOCALE_COLORMENU_STATUSBAR, NEUTRINO_ICON_SETTINGS);
	//InitColorSettingsStatusBarColors(*colorSettings_statusbarColors);

	osdSettings.addItem( new CMenuForwarder(LOCALE_COLORSTATUSBAR_HEAD, true, NULL, new COSDInfoBarColorSettings(), NULL, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE) );

	// language
	osdSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	osdSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_LANGUAGE, true, NULL, new CLanguageSettings(), NULL, CRCInput::convertDigitToKey(shortcutOSD++) ));

	// font/timing
	osdSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_VIDEOMENU_OSD));
	
	// select font
	osdSettings.addItem( new CMenuForwarder(LOCALE_EPGPLUS_SELECT_FONT_NAME, true, NULL, this, "select_font", CRCInput::convertDigitToKey(shortcutOSD++) ));
	
	//font scaling
	osdSettings.addItem(new CMenuForwarder(LOCALE_FONTMENU_SCALING, true, NULL, this, "font_scaling", CRCInput::convertDigitToKey(shortcutOSD++) ));

	// osd timing
	osdSettings.addItem(new CMenuForwarder(LOCALE_TIMING_HEAD, true, NULL, new COSDTimingSettings(), NULL, CRCInput::convertDigitToKey(shortcutOSD++) ));

	// sceensetup
	osdSettings.addItem(new CMenuForwarder(LOCALE_VIDEOMENU_SCREENSETUP, true, NULL, new CScreenSetup(), NULL, CRCInput::convertDigitToKey(shortcutOSD++) ));
	
	// alpha setup
	osdSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));

	CAlphaSetup * chAlphaSetup = new CAlphaSetup(LOCALE_COLORMENU_GTX_ALPHA, &g_settings.gtx_alpha);
	osdSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_GTX_ALPHA, true, NULL, chAlphaSetup, NULL, CRCInput::convertDigitToKey(shortcutOSD++)));
	
	osdSettings.exec(NULL, "");
	osdSettings.hide();
}

// osd menucolor settings
COSDMenuColorSettings::COSDMenuColorSettings()
{
}

COSDMenuColorSettings::~COSDMenuColorSettings()
{
}

int COSDMenuColorSettings::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "COSDMenuColorSettings::exec: actionKey: %s\n", actionKey.c_str());
	
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

void COSDMenuColorSettings::showMenu()
{
	dprintf(DEBUG_NORMAL, "COSDMenuColorSettings::showMenu:\n");
	
	CMenuWidget OSDmenuColorsSettings(LOCALE_COLORMENUSETUP_HEAD, NEUTRINO_ICON_SETTINGS );
	
	// intros
	OSDmenuColorsSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));

	// head
	CColorChooser* chHeadcolor = new CColorChooser(LOCALE_COLORMENU_BACKGROUND, &g_settings.menu_Head_red, &g_settings.menu_Head_green, &g_settings.menu_Head_blue, &g_settings.menu_Head_alpha, CNeutrinoApp::getInstance()->colorSetupNotifier);
	CColorChooser* chHeadTextcolor = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR, &g_settings.menu_Head_Text_red, &g_settings.menu_Head_Text_green, &g_settings.menu_Head_Text_blue,NULL, CNeutrinoApp::getInstance()->colorSetupNotifier);

	// content
	CColorChooser* chContentcolor = new CColorChooser(LOCALE_COLORMENU_BACKGROUND, &g_settings.menu_Content_red, &g_settings.menu_Content_green, &g_settings.menu_Content_blue,&g_settings.menu_Content_alpha, CNeutrinoApp::getInstance()->colorSetupNotifier);
	CColorChooser* chContentTextcolor = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR, &g_settings.menu_Content_Text_red, &g_settings.menu_Content_Text_green, &g_settings.menu_Content_Text_blue,NULL, CNeutrinoApp::getInstance()->colorSetupNotifier);

	// inactive
	CColorChooser* chContentInactivecolor = new CColorChooser(LOCALE_COLORMENU_BACKGROUND, &g_settings.menu_Content_inactive_red, &g_settings.menu_Content_inactive_green, &g_settings.menu_Content_inactive_blue,&g_settings.menu_Content_inactive_alpha, CNeutrinoApp::getInstance()->colorSetupNotifier);
	CColorChooser* chContentInactiveTextcolor = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR, &g_settings.menu_Content_inactive_Text_red, &g_settings.menu_Content_inactive_Text_green, &g_settings.menu_Content_inactive_Text_blue, NULL, CNeutrinoApp::getInstance()->colorSetupNotifier);
	
	// selected
	CColorChooser* chContentSelectedcolor = new CColorChooser(LOCALE_COLORMENU_BACKGROUND, &g_settings.menu_Content_Selected_red, &g_settings.menu_Content_Selected_green, &g_settings.menu_Content_Selected_blue,&g_settings.menu_Content_Selected_alpha, CNeutrinoApp::getInstance()->colorSetupNotifier);
	CColorChooser* chContentSelectedTextcolor = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR, &g_settings.menu_Content_Selected_Text_red, &g_settings.menu_Content_Selected_Text_green, &g_settings.menu_Content_Selected_Text_blue,NULL, CNeutrinoApp::getInstance()->colorSetupNotifier);
	
	// foot
	CColorChooser* chFootcolor = new CColorChooser(LOCALE_COLORMENU_BACKGROUND, &g_settings.menu_Foot_red, &g_settings.menu_Foot_green, &g_settings.menu_Foot_blue, &g_settings.menu_Foot_alpha, CNeutrinoApp::getInstance()->colorSetupNotifier);
	CColorChooser * chFootTextcolor = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR, &g_settings.menu_Foot_Text_red, &g_settings.menu_Foot_Text_green, &g_settings.menu_Foot_Text_blue, NULL, CNeutrinoApp::getInstance()->colorSetupNotifier);

	// head
	OSDmenuColorsSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORMENUSETUP_MENUHEAD));
	OSDmenuColorsSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_BACKGROUND, true, NULL, chHeadcolor ));
	OSDmenuColorsSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chHeadTextcolor ));

	// menu content
	OSDmenuColorsSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORMENUSETUP_MENUCONTENT));
	OSDmenuColorsSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_BACKGROUND, true, NULL, chContentcolor ));
	OSDmenuColorsSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chContentTextcolor ));

	// inactiv
	OSDmenuColorsSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORMENUSETUP_MENUCONTENT_INACTIVE));
	OSDmenuColorsSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_BACKGROUND, true, NULL, chContentInactivecolor ));
	OSDmenuColorsSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chContentInactiveTextcolor));

	// selected
	OSDmenuColorsSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORMENUSETUP_MENUCONTENT_SELECTED));
	OSDmenuColorsSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_BACKGROUND, true, NULL, chContentSelectedcolor ));
	OSDmenuColorsSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chContentSelectedTextcolor ));
	
	// foot
	OSDmenuColorsSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORMENU_HELPBAR));
	OSDmenuColorsSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_BACKGROUND, true, NULL, chFootcolor ));
	OSDmenuColorsSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chFootTextcolor ));
	
	OSDmenuColorsSettings.exec(NULL, "");
	OSDmenuColorsSettings.hide();
}

// osd infobarcolor settings
COSDInfoBarColorSettings::COSDInfoBarColorSettings()
{
}

COSDInfoBarColorSettings::~COSDInfoBarColorSettings()
{
}

int COSDInfoBarColorSettings::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "COSDInfoBarColorSettings::exec: actionKey: %s\n", actionKey.c_str());
	
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

void COSDInfoBarColorSettings::showMenu()
{
	dprintf(DEBUG_NORMAL, "COSDInfoBarColorSettings::showMenu:\n");
	
	CMenuWidget OSDinfobarColorSettings(LOCALE_COLORMENU_STATUSBAR, NEUTRINO_ICON_SETTINGS);
	
	// intros
	OSDinfobarColorSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));

	// bg
	CColorChooser * chInfobarcolor = new CColorChooser(LOCALE_COLORMENU_BACKGROUND, &g_settings.infobar_red, &g_settings.infobar_green, &g_settings.infobar_blue,&g_settings.infobar_alpha, CNeutrinoApp::getInstance()->colorSetupNotifier);
	
	// text
	CColorChooser * chInfobarTextcolor = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR, &g_settings.infobar_Text_red, &g_settings.infobar_Text_green, &g_settings.infobar_Text_blue,NULL, CNeutrinoApp::getInstance()->colorSetupNotifier);
	
	// clolored events
	CColorChooser * chColored_Events = new CColorChooser(LOCALE_COLORMENU_TEXTCOLOR, &g_settings.infobar_colored_events_red, &g_settings.infobar_colored_events_green, &g_settings.infobar_colored_events_blue, NULL, CNeutrinoApp::getInstance()->colorSetupNotifier);

	OSDinfobarColorSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_COLORSTATUSBAR_TEXT));

	OSDinfobarColorSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_BACKGROUND, true, NULL, chInfobarcolor ));

	OSDinfobarColorSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chInfobarTextcolor ));
	
	// clored events
	OSDinfobarColorSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_MISCSETTINGS_INFOBAR_COLORED_EVENTS));
	OSDinfobarColorSettings.addItem( new CMenuForwarder(LOCALE_COLORMENU_TEXTCOLOR, true, NULL, chColored_Events ));
	
	OSDinfobarColorSettings.exec(NULL, "");
	OSDinfobarColorSettings.hide();
}

// osd language settings
CLanguageSettings::CLanguageSettings()
{
}

CLanguageSettings::~CLanguageSettings()
{
}

int CLanguageSettings::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CLanguageSettings::exec: actionKey: %s\n", actionKey.c_str());
	
	int ret = menu_return::RETURN_REPAINT;
	
	if(parent)
		parent->hide();
	
	showMenu();
	
	return ret;
}

bool CLanguageSettings::changeNotify(const neutrino_locale_t OptionName, void */*data*/)
{
	if (ARE_LOCALES_EQUAL(OptionName, LOCALE_LANGUAGESETUP_SELECT)) 
	{
		dprintf(DEBUG_NORMAL, "CLanguageSettings::changeNotify: %s\n", g_settings.language);
		
		// setup font first
		if(strstr(g_settings.language, "arabic"))
		{
			//if( !strstr(g_settings.font_file, "nmsbd.ttf") || !strstr(g_settings.font_file, "ae_AlMateen.ttf") )
			{
				//if(MessageBox(LOCALE_MESSAGEBOX_INFO, "do you want to change your font to nmsbd.ttf\nthis font support your language\n", CMessageBox::mbrYes, CMessageBox::mbYes | CMessageBox::mbNo) == CMessageBox::mbrYes)
				{
					// check for nsmbd font
					if(!access(FONTDIR "/nmsbd.ttf", F_OK))
					{
						strcpy(g_settings.font_file, FONTDIR "/nmsbd.ttf");
						printf("CLanguageSettings::changeNotify:new font file %s\n", g_settings.font_file);
						CNeutrinoApp::getInstance()->SetupFonts();
					}
					else
					{
						HintBox(LOCALE_MESSAGEBOX_INFO, "install a font supporting your language (e.g nmsbd.ttf)");
					}
				}
			}
		}
		
		g_Locale->loadLocale(g_settings.language);
		return true;
	}
	
	return false;
}

void CLanguageSettings::showMenu()
{
	dprintf(DEBUG_NORMAL, "CLanguageSettings::showMenu:\n");
	
	CMenuWidget languageSettings(LOCALE_LANGUAGESETUP_HEAD, NEUTRINO_ICON_LANGUAGE );
	
	// intros
	languageSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	languageSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));

	struct dirent **namelist;
	int n;

	//printf("scanning locale dir now....(perhaps)\n");

	char *path[] = {(char *) DATADIR "/neutrino/locale", (char *) CONFIGDIR "/locale"};

	for(int p = 0; p < 2; p++) 
	{
		n = scandir(path[p], &namelist, 0, alphasort);
		
		if(n > 0)
		{
			for(int count = 0; count < n; count++) 
			{
				char * locale = strdup(namelist[count]->d_name);
				char * pos = strstr(locale, ".locale");

				if(pos != NULL) 
				{
					*pos = '\0';
				
					CMenuOptionLanguageChooser* oj = new CMenuOptionLanguageChooser((char*)locale, this, locale);
					oj->addOption(locale);
					languageSettings.addItem( oj );
				} 
				else
					free(locale);
				free(namelist[count]);
			}
			free(namelist);
		}
	}
	
	languageSettings.exec(NULL, "");
	languageSettings.hide();
}

// osd timing settings
static CTimingSettingsNotifier timingsettingsnotifier;

COSDTimingSettings::COSDTimingSettings()
{
}

COSDTimingSettings::~COSDTimingSettings()
{
}

int COSDTimingSettings::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "COSDTimingSettings::exec: actionKey: %s\n", actionKey.c_str());
	
	int ret = menu_return::RETURN_REPAINT;
	
	if(parent)
		parent->hide();
	
	if(actionKey == "savesettings")
	{
		CNeutrinoApp::getInstance()->exec(NULL, "savesettings");
		
		return ret;
	}
	else if(actionKey == "osd.def") 
	{
		for (int i = 0; i < TIMING_SETTING_COUNT; i++)
			g_settings.timing[i] = default_timing[i];

		CNeutrinoApp::getInstance()->SetupTiming();
		
		return ret;
	}
	
	showMenu();
	
	return ret;
}

void COSDTimingSettings::showMenu()
{
	dprintf(DEBUG_NORMAL, "COSDTimingSettings::showMenu:\n");
	
	CMenuWidget osdTimingSettings(LOCALE_COLORMENU_TIMING, NEUTRINO_ICON_SETTINGS);
	
	// intros
	osdTimingSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	osdTimingSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));

	for (int i = 0; i < TIMING_SETTING_COUNT; i++)
	{
		CStringInput * colorSettings_timing_item = new CStringInput(timing_setting_name[i], g_settings.timing_string[i], 3, LOCALE_TIMING_HINT_1, LOCALE_TIMING_HINT_2, "0123456789 ", &timingsettingsnotifier);
		osdTimingSettings.addItem(new CMenuForwarder(timing_setting_name[i], true, g_settings.timing_string[i], colorSettings_timing_item));
	}

	osdTimingSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	osdTimingSettings.addItem(new CMenuForwarder(LOCALE_OPTIONS_DEFAULT, true, NULL, this, "osd.def"));
	
	osdTimingSettings.exec(NULL, "");
	osdTimingSettings.hide();
}


