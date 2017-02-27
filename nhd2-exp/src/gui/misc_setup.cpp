/*
	Neutrino-GUI  -   DBoxII-Project

	$id: misc_setup.cpp 2016.01.02 21:55:30 mohousch $
	
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
#include <sys/vfs.h>

#include <xmlinterface.h>

#include <gui/widget/messagebox.h>

#include <gui/filebrowser.h>
#include <gui/misc_setup.h>

#include <gui/zapit_setup.h>
#include <gui/psisetup.h>

#include <system/debug.h>
#include <system/setting_helpers.h>
#include <system/helpers.h>

// configfiles
#include <gui/moviebrowser.h>
#include <timerd/timermanager.h>
#include <nhttpd/yconfig.h>

#include <audio_cs.h>
#include <video_cs.h>


extern cVideo *videoDecoder;
extern cAudio *audioDecoder;
extern CFrontend * live_fe;

#define OPTIONS_OFF0_ON1_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_OFF0_ON1_OPTIONS[OPTIONS_OFF0_ON1_OPTION_COUNT] =
{
        { 0, LOCALE_OPTIONS_OFF, NULL },
        { 1, LOCALE_OPTIONS_ON, NULL }
};

#define MESSAGEBOX_NO_YES_OPTION_COUNT 2
const CMenuOptionChooser::keyval MESSAGEBOX_NO_YES_OPTIONS[MESSAGEBOX_NO_YES_OPTION_COUNT] =
{
	{ 0, LOCALE_MESSAGEBOX_NO, NULL },
	{ 1, LOCALE_MESSAGEBOX_YES, NULL }
};

// misc settings
extern Zapit_config zapitCfg;			//defined in neutrino.cpp
void setZapitConfig(Zapit_config * Cfg);
void getZapitConfig(Zapit_config *Cfg);

// option off1 on0
#define OPTIONS_OFF1_ON0_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_OFF1_ON0_OPTIONS[OPTIONS_OFF1_ON0_OPTION_COUNT] =
{
        { 1, LOCALE_OPTIONS_OFF, NULL },
        { 0, LOCALE_OPTIONS_ON, NULL }
};

#define MISCSETTINGS_FILESYSTEM_IS_UTF8_OPTION_COUNT 2
const CMenuOptionChooser::keyval MISCSETTINGS_FILESYSTEM_IS_UTF8_OPTIONS[MISCSETTINGS_FILESYSTEM_IS_UTF8_OPTION_COUNT] =
{
	{ 0, LOCALE_FILESYSTEM_IS_UTF8_OPTION_ISO8859_1, NULL },
	{ 1, LOCALE_FILESYSTEM_IS_UTF8_OPTION_UTF8, NULL      }
};

#define INFOBAR_SUBCHAN_DISP_POS_OPTIONS_COUNT 4
const CMenuOptionChooser::keyval  INFOBAR_SUBCHAN_DISP_POS_OPTIONS[INFOBAR_SUBCHAN_DISP_POS_OPTIONS_COUNT]=
{
	{ 0 , LOCALE_SETTINGS_POS_TOP_RIGHT, NULL },
	{ 1 , LOCALE_SETTINGS_POS_TOP_LEFT, NULL },
	{ 2 , LOCALE_SETTINGS_POS_BOTTOM_LEFT, NULL },
	{ 3 , LOCALE_SETTINGS_POS_BOTTOM_RIGHT, NULL }
};

#define SECTIONSD_SCAN_OPTIONS_COUNT 2
const CMenuOptionChooser::keyval SECTIONSD_SCAN_OPTIONS[SECTIONSD_SCAN_OPTIONS_COUNT] =
{
	{ 0, LOCALE_OPTIONS_OFF, NULL },
	{ 1, LOCALE_OPTIONS_ON, NULL }
};

// volumebar position
#define VOLUMEBAR_DISP_POS_OPTIONS_COUNT 6
const CMenuOptionChooser::keyval  VOLUMEBAR_DISP_POS_OPTIONS[VOLUMEBAR_DISP_POS_OPTIONS_COUNT]=
{
	{ 0 , LOCALE_SETTINGS_POS_TOP_RIGHT, NULL },
	{ 1 , LOCALE_SETTINGS_POS_TOP_LEFT, NULL },
	{ 2 , LOCALE_SETTINGS_POS_BOTTOM_LEFT, NULL },
	{ 3 , LOCALE_SETTINGS_POS_BOTTOM_RIGHT, NULL },
	{ 4 , LOCALE_SETTINGS_POS_DEFAULT_CENTER, NULL },
	{ 5 , LOCALE_SETTINGS_POS_HIGHER_CENTER, NULL }
};

#define MENU_CORNERSETTINGS_TYPE_OPTION_COUNT 2
const CMenuOptionChooser::keyval MENU_CORNERSETTINGS_TYPE_OPTIONS[MENU_CORNERSETTINGS_TYPE_OPTION_COUNT] =
{
	{ 0, LOCALE_EXTRA_ROUNDED_CORNERS_OFF, NULL },
	{ 1, LOCALE_EXTRA_ROUNDED_CORNERS_ON, NULL }	
};

#define MENU_DESIGN_OPTION_COUNT 3
const CMenuOptionChooser::keyval MENU_DESIGN_OPTIONS[MENU_DESIGN_OPTION_COUNT] =
{
	{ SNeutrinoSettings::MENU_DESIGN_STANDARD, LOCALE_EXTRA_MENU_DESIGN_STANDARD, NULL },
	{ SNeutrinoSettings::MENU_DESIGN_CLASSIC, LOCALE_EXTRA_MENU_DESIGN_CLASSIC, NULL },
	{ SNeutrinoSettings::MENU_DESIGN_SMART, LOCALE_EXTRA_MENU_DESIGN_SMART, NULL }
};

#define MENU_POSITION_OPTION_COUNT 3
const CMenuOptionChooser::keyval MENU_POSITION_OPTIONS[MENU_POSITION_OPTION_COUNT] =
{
	{ SNeutrinoSettings::MENU_POSITION_LEFT, LOCALE_EXTRA_MENU_POSITION_LEFT, NULL },
	{ SNeutrinoSettings::MENU_POSITION_CENTER, LOCALE_EXTRA_MENU_POSITION_CENTER, NULL },
	{ SNeutrinoSettings::MENU_POSITION_RIGHT, LOCALE_EXTRA_MENU_POSITION_RIGHT, NULL }
};

CMenuOptionStringChooser * tzSelect;

CMiscSettings::CMiscSettings()
{
}

CMiscSettings::~CMiscSettings()
{
}

int CMiscSettings::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CMiscSettings::exec: actionKey: %s\n", actionKey.c_str());
	
	int ret = menu_return::RETURN_REPAINT;
	
	if(parent)
		parent->hide();
	
	if(actionKey == "savesettings")
	{
		CNeutrinoApp::getInstance()->exec(NULL, "savesettings");
		
		return ret;
	}
	else if(actionKey == "logos_dir") 
	{
		if(parent)
			parent->hide();
		
		CFileBrowser b;
		b.Dir_Mode = true;
		
		if (b.exec(g_settings.logos_dir.c_str())) 
		{
			g_settings.logos_dir = b.getSelectedFile()->Name;

			dprintf(DEBUG_NORMAL, "CMiscSettings::exec: new logos dir %s\n", b.getSelectedFile()->Name.c_str());
		}

		return ret;
	}
	else if(actionKey == "epgdir") 
	{
		if(parent)
			parent->hide();
		
		CFileBrowser b;
		b.Dir_Mode = true;
		
		if ( b.exec(g_settings.epg_dir.c_str())) 
		{
			const char * newdir = b.getSelectedFile()->Name.c_str();
			if(check_dir(newdir))
				printf("CNeutrinoApp::exec: Wrong/unsupported epg dir %s\n", newdir);
			else
			{
				g_settings.epg_dir = b.getSelectedFile()->Name;
				CNeutrinoApp::getInstance()->SendSectionsdConfig();
			}
		}

		return ret;
	}
	
	if(g_settings.menu_design == SNeutrinoSettings::MENU_DESIGN_STANDARD)
		showMenu();
	else if(g_settings.menu_design == SNeutrinoSettings::MENU_DESIGN_CLASSIC)
		showMenuClassic();
	else if(g_settings.menu_design == SNeutrinoSettings::MENU_DESIGN_SMART)
		showMenuSmart();
	
	return ret;
}

// standard
void CMiscSettings::showMenu(void)
{
	dprintf(DEBUG_NORMAL, "CMiscSettings::showMenu:\n");
	
	int shortcutMiscSettings = 1;
	
	CMenuWidgetExtended* miscSettings = new CMenuWidgetExtended(LOCALE_MISCSETTINGS_HEAD, NEUTRINO_ICON_SETTINGS);

	//miscSettings general
	miscSettings->addItem(new CMenuForwarderExtended(LOCALE_MISCSETTINGS_GENERAL, true, new CGeneralSettings(), NULL, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED, NEUTRINO_ICON_MENUITEM_GENERALSETTINGS, LOCALE_HELPTEXT_GENERALSETTINGS));
	
	//channellist settings
	miscSettings->addItem(new CMenuForwarderExtended(LOCALE_MISCSETTINGS_CHANNELLIST, true, new CChannelListSettings(), NULL, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN, NEUTRINO_ICON_MENUITEM_CHANNELLISTSETTINGS, LOCALE_HELPTEXT_MISCSETTINGSCHANNELLIST));

	// epg settings
	miscSettings->addItem(new CMenuForwarderExtended(LOCALE_MISCSETTINGS_EPG_HEAD, true, new CEPGSettings(), NULL, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW, NEUTRINO_ICON_MENUITEM_EPGSETTINGS, LOCALE_HELPTEXT_EPGSETTINGS));

	// filebrowser settings
	miscSettings->addItem(new CMenuForwarderExtended(LOCALE_MISCSETTINGS_FILEBROWSER, true, new CFileBrowserSettings(), NULL, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE, NEUTRINO_ICON_MENUITEM_FILEBROWSERSETTINGS, LOCALE_HELPTEXT_FILEBROWSERSETTINGS));
	
	// zapit setup (start channel)
	miscSettings->addItem(new CMenuForwarderExtended(LOCALE_MISCSETTINGS_ZAPIT, true, new CZapitSetup(), NULL, CRCInput::convertDigitToKey(shortcutMiscSettings++), NULL, NEUTRINO_ICON_MENUITEM_STARTCHANNELSETTINGS, LOCALE_HELPTEXT_STARTCHANNELSETTINGS));
	
	// psi setup
	//FIXME:	
	//CPSISetup * chPSISetup = new CPSISetup(LOCALE_VIDEOMENU_PSISETUP, &g_settings.contrast, &g_settings.saturation, &g_settings.brightness, &g_settings.tint);
	//miscSettings->addItem( new CMenuForwarderExtended(LOCALE_VIDEOMENU_PSISETUP, true, chPSISetup, NULL, CRCInput::convertDigitToKey(shortcutMiscSettings++), NULL, NEUTRINO_ICON_MENUITEM_PSISETTINGS, LOCALE_HELPTEXT_PSISETTINGS));
	
	miscSettings->exec(NULL, "");
	miscSettings->hide();
	delete miscSettings;
	miscSettings = NULL;
}

// smart
void CMiscSettings::showMenuSmart(void)
{
	dprintf(DEBUG_NORMAL, "CMiscSettings::showMenuSmart:\n");
	
	int shortcutMiscSettings = 1;
	
	CMenuFrameBox* miscSettings = new CMenuFrameBox(LOCALE_MISCSETTINGS_HEAD, NEUTRINO_ICON_SETTINGS);

	//miscSettings general
	miscSettings->addItem(new CMenuFrameBoxItem(LOCALE_MISCSETTINGS_GENERAL, new CGeneralSettings(), NULL, NEUTRINO_ICON_SMART_GENERALSETTINGS));
	
	//channellist settings
	miscSettings->addItem(new CMenuFrameBoxItem(LOCALE_MISCSETTINGS_CHANNELLIST, new CChannelListSettings(), NULL, NEUTRINO_ICON_SMART_CHANNELLISTSETTINGS));

	// epg settings
	miscSettings->addItem(new CMenuFrameBoxItem(LOCALE_MISCSETTINGS_EPG_HEAD, new CEPGSettings(), NULL, NEUTRINO_ICON_SMART_EPGSETTINGS));

	// filebrowser settings
	miscSettings->addItem(new CMenuFrameBoxItem(LOCALE_MISCSETTINGS_FILEBROWSER, new CFileBrowserSettings(), NULL, NEUTRINO_ICON_SMART_FILEBROWSERSETTINGS));
	
	// zapit setup (start channel)
	miscSettings->addItem(new CMenuFrameBoxItem(LOCALE_MISCSETTINGS_ZAPIT, new CZapitSetup(), NULL, NEUTRINO_ICON_SMART_STARTCHANNELSETTINGS));
	
	// psi setup
	//FIXME:	
	//CPSISetup * chPSISetup = new CPSISetup(LOCALE_VIDEOMENU_PSISETUP, &g_settings.contrast, &g_settings.saturation, &g_settings.brightness, &g_settings.tint);
	//miscSettings->addItem( new CMenuFrameBoxItem(LOCALE_VIDEOMENU_PSISETUP, chPSISetup, NULL, NEUTRINO_ICON_SMART_PSISETTINGS));
	
	miscSettings->exec(NULL, "");
	miscSettings->hide();
	delete miscSettings;
	miscSettings = NULL;
}

// classic
void CMiscSettings::showMenuClassic(void)
{
	dprintf(DEBUG_NORMAL, "CMiscSettings::showMenuClassic:\n");
	
	int shortcutMiscSettings = 1;

	CMenuWidget * miscSettings = new CMenuWidget(LOCALE_MISCSETTINGS_HEAD, NEUTRINO_ICON_SETTINGS);

	//miscSettings general
	miscSettings->addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_GENERAL, true, NULL, new CGeneralSettings(), NULL, CRCInput::RC_red, NEUTRINO_ICON_CLASSIC_GENERALSETTINGS, LOCALE_HELPTEXT_GENERALSETTINGS));
	
	//channellist settings
	miscSettings->addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_CHANNELLIST, true, NULL, new CChannelListSettings(), NULL, CRCInput::RC_green, NEUTRINO_ICON_CLASSIC_CHANNELLISTSETTINGS, LOCALE_HELPTEXT_MISCSETTINGSCHANNELSETTINGS));

	// epg settings
	miscSettings->addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_EPG_HEAD, true, NULL, new CEPGSettings(), NULL, CRCInput::RC_yellow, NEUTRINO_ICON_CLASSIC_EPGSETTINGS, LOCALE_HELPTEXT_EPGSETTINGS));

	// filebrowser settings
	miscSettings->addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_FILEBROWSER, true, NULL, new CFileBrowserSettings(), NULL, CRCInput::RC_blue, NEUTRINO_ICON_CLASSIC_FILEBROWSERSETTINGS, LOCALE_HELPTEXT_FILEBROWSERSETTINGS));
	
	// zapit setup (start channel)
	miscSettings->addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_ZAPIT, true, NULL, new CZapitSetup(), NULL, CRCInput::convertDigitToKey(shortcutMiscSettings++), NEUTRINO_ICON_CLASSIC_STARTCHANNELSETTINGS, LOCALE_HELPTEXT_STARTCHANNELSETTINGS));
	
	// psi setup
	//FIXME:	
	//CPSISetup * chPSISetup = new CPSISetup(LOCALE_VIDEOMENU_PSISETUP, &g_settings.contrast, &g_settings.saturation, &g_settings.brightness, &g_settings.tint);
	//miscSettings->addItem( new CMenuForwarder(LOCALE_VIDEOMENU_PSISETUP, true, NULL, chPSISetup, NULL, CRCInput::convertDigitToKey(shortcutMiscSettings++), NEUTRINO_ICON_CLASSIC_PSISETTINGS, LOCALE_HELPTEXT_LOCALE_HELPTEXT_PSISETTINGS));
	
	miscSettings->exec(NULL, "");
	miscSettings->hide();
	delete miscSettings;
	miscSettings = NULL;
}

// general settings
// progressbar color
#define PROGRESSBAR_COLOR_OPTION_COUNT 2
const CMenuOptionChooser::keyval PROGRESSBAR_COLOR_OPTIONS[PROGRESSBAR_COLOR_OPTION_COUNT] =
{
	{ 0, NONEXISTANT_LOCALE, "colored" },
	{ 1, NONEXISTANT_LOCALE, "Mono Chrom" }
};

extern CRemoteControl * g_RemoteControl;	// defined neutrino.cpp

CGeneralSettings::CGeneralSettings()
{
}

CGeneralSettings::~CGeneralSettings()
{
}

int CGeneralSettings::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CGeneralSettings::exec: actionKey: %s\n", actionKey.c_str());
	
	int ret = menu_return::RETURN_REPAINT;
	
	if(parent)
		parent->hide();
	
	if(actionKey == "savesettings")
	{
		CNeutrinoApp::getInstance()->exec(NULL, "savesettings");
		
		return ret;
	}
	else if(actionKey == "logos_dir") 
	{
		if(parent)
			parent->hide();
		
		CFileBrowser b;
		b.Dir_Mode = true;
		
		if (b.exec(g_settings.logos_dir.c_str())) 
		{
			g_settings.logos_dir = b.getSelectedFile()->Name;

			dprintf(DEBUG_NORMAL, "CMiscSettings::exec: new logos dir %s\n", b.getSelectedFile()->Name.c_str());
		}

		return ret;
	}
	
	showMenu();
	
	return ret;
}

bool CGeneralSettings::changeNotify(const neutrino_locale_t OptionName, void */*data*/)
{
	dprintf(DEBUG_NORMAL, "CGeneralSettings::changeNotify:\n");
	
	if(ARE_LOCALES_EQUAL(OptionName, LOCALE_MISCSETTINGS_INFOBAR_RADIOTEXT)) 
	{
		bool usedBackground = CFrameBuffer::getInstance()->getuseBackground();
		
		if (g_settings.radiotext_enable) 
		{
			//
			if (g_Radiotext == NULL)
				g_Radiotext = new CRadioText;

			if (g_Radiotext && ((CNeutrinoApp::getInstance()->getMode()) == NeutrinoMessages::mode_radio))
			{
				// hide radiomode background pic
				if (usedBackground) 
				{
					CFrameBuffer::getInstance()->saveBackgroundImage();
					CFrameBuffer::getInstance()->ClearFrameBuffer();

					CFrameBuffer::getInstance()->blit();
				}

				if(live_fe)
					g_Radiotext->setPid(g_RemoteControl->current_PIDs.APIDs[g_RemoteControl->current_PIDs.PIDs.selected_apid].pid);
			}
		} 
		else 
		{
			if (g_Radiotext && ((CNeutrinoApp::getInstance()->getMode()) == NeutrinoMessages::mode_radio))
			{
				// restore previous background
				if (usedBackground) 
				{
					CFrameBuffer::getInstance()->restoreBackgroundImage();
					CFrameBuffer::getInstance()->useBackground(true);
					CFrameBuffer::getInstance()->paintBackground();
					CFrameBuffer::getInstance()->blit();
				}
			
				CFrameBuffer::getInstance()->loadBackgroundPic("radiomode.jpg");
				CFrameBuffer::getInstance()->blit();
			}

			delete g_Radiotext;
			g_Radiotext = NULL;
		}
		
		return true;
	}

	return false;
}

void CGeneralSettings::showMenu()
{
	dprintf(DEBUG_NORMAL, "CGeneralSettings::showMenu:\n");
	
	CMenuWidget miscSettingsGeneral(LOCALE_MISCSETTINGS_GENERAL, NEUTRINO_ICON_SETTINGS);
	
	// intros
	miscSettingsGeneral.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	miscSettingsGeneral.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	miscSettingsGeneral.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	miscSettingsGeneral.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	// rc delay
	CMenuOptionChooser * m1 = new CMenuOptionChooser(LOCALE_MISCSETTINGS_SHUTDOWN_REAL_RCDELAY, &g_settings.shutdown_real_rcdelay, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, !g_settings.shutdown_real);

	CMiscNotifier * miscNotifier = new CMiscNotifier( m1 );

	// shutdown real
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_MISCSETTINGS_SHUTDOWN_REAL, &g_settings.shutdown_real, OPTIONS_OFF1_ON0_OPTIONS, OPTIONS_OFF1_ON0_OPTION_COUNT, true, miscNotifier ));

	// delayed shutdown
	miscSettingsGeneral.addItem(m1);

	// delay counter
	CStringInput * miscSettings_shutdown_count = new CStringInput(LOCALE_MISCSETTINGS_SHUTDOWN_COUNT, g_settings.shutdown_count, 3, LOCALE_MISCSETTINGS_SHUTDOWN_COUNT_HINT1, LOCALE_MISCSETTINGS_SHUTDOWN_COUNT_HINT2, "0123456789 ");
	miscSettingsGeneral.addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_SHUTDOWN_COUNT, true, g_settings.shutdown_count, miscSettings_shutdown_count));

	// start to standby
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_EXTRA_STARTSTANDBY, &g_settings.power_standby, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true));

	// timezone
	xmlDocPtr parser;

	parser = parseXmlFile("/etc/timezone.xml");
	if (parser != NULL) 
	{	
		tzSelect = new CMenuOptionStringChooser(LOCALE_MAINSETTINGS_TIMEZONE, g_settings.timezone, true, new CTZChangeNotifier(), CRCInput::RC_nokey, "", true);

		xmlNodePtr search = xmlDocGetRootElement(parser)->xmlChildrenNode;
		bool found = false;

		while (search) 
		{
			if (!strcmp(xmlGetName(search), "zone")) 
			{
				std::string name = xmlGetAttribute(search, (char *) "name");
				std::string zone = xmlGetAttribute(search, (char *) "zone");
				
				//printf("Timezone: %s -> %s\n", name.c_str(), zone.c_str());
				
				tzSelect->addOption(name.c_str());
				found = true;
			}
			search = search->xmlNextNode;
		}

		if(found)
			miscSettingsGeneral.addItem(tzSelect);
		else 
		{
			delete tzSelect;
			tzSelect = NULL;
		}	
		xmlFreeDoc(parser);
	}

	// sig/snr
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_MISCSETTINGS_INFOBAR_SAT_DISPLAY, &g_settings.infobar_sat_display, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true));
	
	// radio text	
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_MISCSETTINGS_INFOBAR_RADIOTEXT, &g_settings.radiotext_enable, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, this ));
	
	// logos dir
	miscSettingsGeneral.addItem( new CMenuForwarder(LOCALE_MISCSETTINGS_LOGOSDIR, true, g_settings.logos_dir, this, "logos_dir" ) );
	
	// epgplus logos
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_MISCSETTINGS_EPGPLUS_SHOW_LOGOS, &g_settings.epgplus_show_logo, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true ));
	
	// infobar show channelname
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_MISCSETTINGS_INFOBAR_SHOW_CHANNELNAME, &g_settings.show_channelname, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true ));
	
	// recording screenshot
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_MISCSETTINGS_RECORDING_SCREENSHOT, &g_settings.recording_screenshot, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true ));

	// subchan pos
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_INFOVIEWER_SUBCHAN_DISP_POS, &g_settings.infobar_subchan_disp_pos, INFOBAR_SUBCHAN_DISP_POS_OPTIONS, INFOBAR_SUBCHAN_DISP_POS_OPTIONS_COUNT, true, NULL, CRCInput::RC_nokey, "", true));
	
	// volumebar position
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_EXTRA_VOLUME_POS, &g_settings.volume_pos, VOLUMEBAR_DISP_POS_OPTIONS, VOLUMEBAR_DISP_POS_OPTIONS_COUNT, true, NULL, CRCInput::RC_nokey, "", true ));

	// volume bar steps
	CStringInput * audio_step = new CStringInput(LOCALE_AUDIOMENU_VOLUMEBAR_AUDIOSTEPS,g_settings.audio_step, 2, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE, "0123456789 " );
	CMenuForwarder *as = new CMenuForwarder(LOCALE_AUDIOMENU_VOLUMEBAR_AUDIOSTEPS, true, g_settings.audio_step, audio_step );
	miscSettingsGeneral.addItem(as);

	// osd
	miscSettingsGeneral.addItem( new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_VIDEOMENU_OSD));
	
	// menu design
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_EXTRA_MENU_DESIGN, &g_settings.menu_design, MENU_DESIGN_OPTIONS, MENU_DESIGN_OPTION_COUNT, true));

	// menu position
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_EXTRA_MENU_POSITION, &g_settings.menu_position, MENU_POSITION_OPTIONS, MENU_POSITION_OPTION_COUNT, true));

	// corners
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_EXTRA_ROUNDED_CORNERS, &g_settings.rounded_corners, MENU_CORNERSETTINGS_TYPE_OPTIONS, MENU_CORNERSETTINGS_TYPE_OPTION_COUNT, true));

	// progressbar color
	miscSettingsGeneral.addItem(new CMenuOptionChooser(LOCALE_PROGRESSBAR_COLOR, &g_settings.progressbar_color, PROGRESSBAR_COLOR_OPTIONS, PROGRESSBAR_COLOR_OPTION_COUNT, true));

	// reset factory setup
	miscSettingsGeneral.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	
	CDataResetNotifier * resetNotifier = new CDataResetNotifier();
	miscSettingsGeneral.addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_RESET, true, NULL, resetNotifier, "factory", CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN ));
	miscSettingsGeneral.addItem(new CMenuForwarder(LOCALE_SETTINGS_BACKUP,  true, NULL, resetNotifier, "backup", CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW ));
	miscSettingsGeneral.addItem(new CMenuForwarder(LOCALE_SETTINGS_RESTORE, true, NULL, resetNotifier, "restore", CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE ));
	
	miscSettingsGeneral.exec(NULL, "");
	miscSettingsGeneral.hide();
}

// TZ notifier
bool CTZChangeNotifier::changeNotify(const neutrino_locale_t, void * Data)
{
	bool found = false;
	std::string name, zone;
	
	dprintf(DEBUG_NORMAL, "CTZChangeNotifier::changeNotify: %s\n", (char *) Data);

        xmlDocPtr parser = parseXmlFile("/etc/timezone.xml");
	
        if (parser != NULL) 
	{
                xmlNodePtr search = xmlDocGetRootElement(parser)->xmlChildrenNode;
                while (search) 
		{
                        if (!strcmp(xmlGetName(search), "zone")) 
			{
                                name = xmlGetAttribute(search, (char *) "name");
                                zone = xmlGetAttribute(search, (char *) "zone");

				if(!strcmp(g_settings.timezone, name.c_str())) 
				{
					found = true;
					break;
				}
                        }
                        search = search->xmlNextNode;
                }
                xmlFreeDoc(parser);
        }

	if(found) 
	{
		dprintf(DEBUG_NORMAL, "CTZChangeNotifier::changeNotify: Timezone: %s -> %s\n", name.c_str(), zone.c_str());
		
		std::string cmd = "ln -sf /usr/share/zoneinfo/" + zone + " /etc/localtime";
		
		dprintf(DEBUG_NORMAL, "exec %s\n", cmd.c_str());
		
		system(cmd.c_str());
		
		tzset();
	}

	return true;
}

// data reset notifier
extern Zapit_config zapitCfg;
void loadZapitSettings();
void getZapitConfig(Zapit_config *Cfg);

int CDataResetNotifier::exec(CMenuTarget */*parent*/, const std::string& actionKey)
{
	CFileBrowser fileBrowser;
	CFileFilter fileFilter;

	if( actionKey == "factory") 
	{
		int result = MessageBox(LOCALE_RESET_SETTINGS, g_Locale->getText(LOCALE_RESET_CONFIRM), CMessageBox::mbrNo, CMessageBox::mbYes | CMessageBox::mbNo);
		if(result != CMessageBox::mbrYes) 
			return true;
		
		// neutrino settings
		unlink(NEUTRINO_SETTINGS_FILE);
		
		// moviebrowser settings
		unlink(MOVIEBROWSER_SETTINGS_FILE);
		
		// timerd settings
		unlink(TIMERD_CONFIGFILE);
		
		// nhttpd settings
		unlink(HTTPD_CONFIGFILE );
		unlink(YWEB_CONFIGFILE);
		
		// load default settings
		CNeutrinoApp::getInstance()->loadSetup(NEUTRINO_SETTINGS_FILE);
		
		// create default settings to stop wizard
		CNeutrinoApp::getInstance()->saveSetup(NEUTRINO_SETTINGS_FILE);
		
		CFrameBuffer::getInstance()->paintBackground();
#ifdef FB_BLIT
		CFrameBuffer::getInstance()->blit();
#endif		
		// video mode
		if(videoDecoder)
		{
			videoDecoder->SetVideoSystem(g_settings.video_Mode);

			//aspect-ratio
			videoDecoder->setAspectRatio(g_settings.video_Ratio, g_settings.video_Format);
#if defined (PLATFORM_COOLSTREAM)
			videoDecoder->SetVideoMode((analog_mode_t) g_settings.analog_mode);
#else			
			videoDecoder->SetAnalogMode( g_settings.analog_mode); 
#endif

#if !defined (PLATFORM_COOLSTREAM)	
			videoDecoder->SetSpaceColour(g_settings.hdmi_color_space);
#endif
		}

		// audio mode
		g_Zapit->setAudioMode(g_settings.audio_AnalogMode);

		if(audioDecoder)
			audioDecoder->SetHdmiDD(g_settings.hdmi_dd );

		CNeutrinoApp::getInstance()->SetupTiming();
	}
	else if(actionKey == "backup") 
	{
		fileBrowser.Dir_Mode = true;
		if (fileBrowser.exec("/media") == true) 
		{
			char  fname[256];
			struct statfs s;
			int ret = ::statfs(fileBrowser.getSelectedFile()->Name.c_str(), &s);

			if(ret == 0 && s.f_type != 0x72b6L/*jffs2*/ && s.f_type != 0x5941ff53L /*yaffs2*/)
			{ 
				const char backup_sh[] = "backup.sh";

				sprintf(fname, "%s %s", backup_sh, fileBrowser.getSelectedFile()->Name.c_str());
				
				dprintf(DEBUG_NORMAL, "CDataResetNotifier::exec: executing %s\n", fname);
				
				system(fname);
			} 
			else
				MessageBox(LOCALE_MESSAGEBOX_ERROR, g_Locale->getText(LOCALE_SETTINGS_BACKUP_FAILED),CMessageBox::mbrBack, CMessageBox::mbBack, NEUTRINO_ICON_ERROR);
		}
	}
	else if(actionKey == "restore") 
	{
		fileFilter.addFilter("tar");
		fileBrowser.Filter = &fileFilter;
		if (fileBrowser.exec("/media") == true) 
		{
			int result = MessageBox(LOCALE_SETTINGS_RESTORE, g_Locale->getText(LOCALE_SETTINGS_RESTORE_WARN), CMessageBox::mbrNo, CMessageBox::mbYes | CMessageBox::mbNo);
			if(result == CMessageBox::mbrYes) 
			{
				char  fname[256];
				
				const char restore_sh[] = "restore.sh";
				
				sprintf(fname, "%s %s", restore_sh, fileBrowser.getSelectedFile()->Name.c_str());
				
				dprintf(DEBUG_NORMAL, "CDataResetNotifier::exec: executing %s\n", fname);
				
				system(fname);
			}
			
			
		}
	}

	return true;
}


// channellist settings
extern t_channel_id live_channel_id;

CChannelListSettings::CChannelListSettings(bool disablePosition)
{
	disablePos = disablePosition;
}

CChannelListSettings::~CChannelListSettings()
{
}

int CChannelListSettings::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CChannelListSettings::exec: actionKey: %s\n", actionKey.c_str());
	
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

bool CChannelListSettings::changeNotify(const neutrino_locale_t OptionName, void */*data*/)
{
	dprintf(DEBUG_NORMAL, "CChannelListSettings::changeNotify:\n");
	
	if(ARE_LOCALES_EQUAL(OptionName, LOCALE_CHANNELLIST_MAKE_HDLIST)) 
	{
		CNeutrinoApp::getInstance()->channelsInit();
		CNeutrinoApp::getInstance()->channelList->adjustToChannelID(live_channel_id);//FIXME
		
		return true;
	}
	else if(ARE_LOCALES_EQUAL(OptionName, LOCALE_EXTRA_ZAPIT_MAKE_BOUQUET)) 
	{
		setZapitConfig(&zapitCfg);
		
		CNeutrinoApp::getInstance()->channelsInit();
		CNeutrinoApp::getInstance()->channelList->adjustToChannelID(live_channel_id);//FIXME
		
		return true;
	}
	
	return false;
}

void CChannelListSettings::showMenu()
{
	dprintf(DEBUG_NORMAL, "CChannelListSettings::showMenu:\n");
	
	CMenuWidget miscSettingsChannelList(LOCALE_MISCSETTINGS_CHANNELLIST, NEUTRINO_ICON_SETTINGS);
	miscSettingsChannelList.enableSaveScreen(true);
	if(disablePos)
		miscSettingsChannelList.disableMenuPosition();
	
	int shortcutMiscChannel = 1;
	
	// intros
	miscSettingsChannelList.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	miscSettingsChannelList.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	miscSettingsChannelList.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	miscSettingsChannelList.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// HD list
	miscSettingsChannelList.addItem(new CMenuOptionChooser(LOCALE_CHANNELLIST_MAKE_HDLIST, &g_settings.make_hd_list, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, this, CRCInput::convertDigitToKey(shortcutMiscChannel++) ));
	
	// virtual zap
	miscSettingsChannelList.addItem(new CMenuOptionChooser(LOCALE_MISCSETTINGS_VIRTUAL_ZAP_MODE, &g_settings.virtual_zap_mode, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutMiscChannel++) ));
	
	// zap cycle
	miscSettingsChannelList.addItem(new CMenuOptionChooser(LOCALE_EXTRA_ZAP_CYCLE, &g_settings.zap_cycle, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutMiscChannel++) ));
	
	// sms channel
	//miscSettingsChannelList.addItem(new CMenuOptionChooser(LOCALE_EXTRA_SMS_CHANNEL, &g_settings.sms_channel, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, false, NULL, CRCInput::convertDigitToKey(shortcutMiscChannel++) ));
	
	// channellist ca
	miscSettingsChannelList.addItem(new CMenuOptionChooser(LOCALE_CHANNELLIST_SHOWCA, &g_settings.channellist_ca, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutMiscChannel++) ));
	
	// extended channel list
	miscSettingsChannelList.addItem(new CMenuOptionChooser(LOCALE_CHANNELLIST_EXTENDED, &g_settings.channellist_extended, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutMiscChannel++) ));
	
	//
	getZapitConfig(&zapitCfg);
	
	miscSettingsChannelList.addItem(new CMenuOptionChooser(LOCALE_EXTRA_ZAPIT_MAKE_BOUQUET, (int *)&zapitCfg.makeRemainingChannelsBouquet, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutMiscChannel++) ));
	miscSettingsChannelList.addItem( new CMenuOptionChooser(LOCALE_ZAPIT_SCANSDT, (int *)&zapitCfg.scanSDT, SECTIONSD_SCAN_OPTIONS, SECTIONSD_SCAN_OPTIONS_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutMiscChannel++)) );
	
	miscSettingsChannelList.exec(NULL, "");
	miscSettingsChannelList.hide();
}

// epg settings
CEPGSettings::CEPGSettings()
{
}

CEPGSettings::~CEPGSettings()
{
}

int CEPGSettings::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CEPGSettings::exec: actionKey: %s\n", actionKey.c_str());
	
	int ret = menu_return::RETURN_REPAINT;
	
	if(parent)
		parent->hide();
	
	if(actionKey == "savesettings")
	{
		CNeutrinoApp::getInstance()->exec(NULL, "savesettings");
		
		return ret;
	}
	else if(actionKey == "epgdir") 
	{
		if(parent)
			parent->hide();
		
		CFileBrowser b;
		b.Dir_Mode = true;
		
		if ( b.exec(g_settings.epg_dir.c_str())) 
		{
			const char * newdir = b.getSelectedFile()->Name.c_str();
			if(check_dir(newdir))
				printf("CNeutrinoApp::exec: Wrong/unsupported epg dir %s\n", newdir);
			else
			{
				g_settings.epg_dir = b.getSelectedFile()->Name;
				CNeutrinoApp::getInstance()->SendSectionsdConfig();
			}
		}

		return ret;
	}
	
	showMenu();
	
	return ret;
}

void CEPGSettings::showMenu()
{
	dprintf(DEBUG_NORMAL, "CEPGSettings::showMenu:\n");
	
	CMenuWidget miscSettingsEPG(LOCALE_MISCSETTINGS_EPG_HEAD, NEUTRINO_ICON_SETTINGS);
	
	int shortcutMiscEpg = 1;
	
	// intros
	miscSettingsEPG.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	miscSettingsEPG.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	miscSettingsEPG.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	miscSettingsEPG.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	// read epg from xml
	CEPGConfigNotifier* epgConfigNotifier = new CEPGConfigNotifier;
	miscSettingsEPG.addItem(new CMenuOptionChooser(LOCALE_MISCSETTINGS_EPG_READ, &g_settings.epg_read, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, epgConfigNotifier, CRCInput::convertDigitToKey(shortcutMiscEpg++) ));

	// save epg
	CSectionsdConfigNotifier* sectionsdConfigNotifier = new CSectionsdConfigNotifier;
	miscSettingsEPG.addItem(new CMenuOptionChooser(LOCALE_MISCSETTINGS_EPG_SAVE, &g_settings.epg_save, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutMiscEpg++) ));

	// epg cache
        CStringInput * miscSettings_epg_cache = new CStringInput(LOCALE_MISCSETTINGS_EPG_CACHE, &g_settings.epg_cache, 2,LOCALE_MISCSETTINGS_EPG_CACHE_HINT1, LOCALE_MISCSETTINGS_EPG_CACHE_HINT2 , "0123456789 ", sectionsdConfigNotifier);
        miscSettingsEPG.addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_EPG_CACHE, true, g_settings.epg_cache, miscSettings_epg_cache, NULL, CRCInput::convertDigitToKey(shortcutMiscEpg++) ));

	// extended epg cache
        CStringInput * miscSettings_epg_cache_e = new CStringInput(LOCALE_MISCSETTINGS_EPG_EXTENDEDCACHE, &g_settings.epg_extendedcache, 3,LOCALE_MISCSETTINGS_EPG_EXTENDEDCACHE_HINT1, LOCALE_MISCSETTINGS_EPG_EXTENDEDCACHE_HINT2 , "0123456789 ", sectionsdConfigNotifier);
        miscSettingsEPG.addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_EPG_EXTENDEDCACHE, true, g_settings.epg_extendedcache, miscSettings_epg_cache_e, NULL, CRCInput::convertDigitToKey(shortcutMiscEpg++)));

	// old events
        CStringInput * miscSettings_epg_old_events = new CStringInput(LOCALE_MISCSETTINGS_EPG_OLD_EVENTS, &g_settings.epg_old_events, 2,LOCALE_MISCSETTINGS_EPG_OLD_EVENTS_HINT1, LOCALE_MISCSETTINGS_EPG_OLD_EVENTS_HINT2 , "0123456789 ", sectionsdConfigNotifier);
        miscSettingsEPG.addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_EPG_OLD_EVENTS, true, g_settings.epg_old_events, miscSettings_epg_old_events, NULL, CRCInput::convertDigitToKey(shortcutMiscEpg++) ));

	// max epg events
        CStringInput * miscSettings_epg_max_events = new CStringInput(LOCALE_MISCSETTINGS_EPG_MAX_EVENTS, &g_settings.epg_max_events, 5,LOCALE_MISCSETTINGS_EPG_MAX_EVENTS_HINT1, LOCALE_MISCSETTINGS_EPG_MAX_EVENTS_HINT2 , "0123456789 ", sectionsdConfigNotifier);
        miscSettingsEPG.addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_EPG_MAX_EVENTS, true, g_settings.epg_max_events, miscSettings_epg_max_events, NULL, CRCInput::convertDigitToKey(shortcutMiscEpg++) ));

	// epg save dir
        miscSettingsEPG.addItem(new CMenuForwarder(LOCALE_MISCSETTINGS_EPG_DIR, true, g_settings.epg_dir, this, "epgdir", CRCInput::convertDigitToKey(shortcutMiscEpg++) ));
	
	// epglang
	miscSettingsEPG.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_MISCSETTINGS_PREF_EPGS_HEAD));
	
	CMenuOptionStringChooser * epglangSelect[3];
	CEPGlangSelectNotifier * EPGlangNotifier = new CEPGlangSelectNotifier();
	
	for(int i = 0; i < 3; i++) 
	{
		epglangSelect[i] = new CMenuOptionStringChooser(LOCALE_MISCSETTINGS_PREF_EPGS, g_settings.pref_epgs[i], true, EPGlangNotifier, CRCInput::RC_nokey, "", true);
		std::map<std::string, std::string>::const_iterator it;
		
		epglangSelect[i]->addOption("");
		for(it = iso639rev.begin(); it != iso639rev.end(); it++) 
			epglangSelect[i]->addOption(it->first.c_str());
	}
	
	// epglang
	for(int i = 0; i < 3; i++) 
		miscSettingsEPG.addItem(epglangSelect[i]);
	
	miscSettingsEPG.exec(NULL, "");
	miscSettingsEPG.hide();
}

// epg language select notifier
void sectionsd_set_languages(const std::vector<std::string>& newLanguages);

bool CEPGlangSelectNotifier::changeNotify(const neutrino_locale_t, void *)
{
	std::vector<std::string> v_languages;
	bool found = false;
	std::map<std::string, std::string>::const_iterator it;

	//prefered audio languages
	for(int i = 0; i < 3; i++) 
	{
		if(strlen(g_settings.pref_epgs[i])) 
		{
			dprintf(DEBUG_NORMAL, "EPG: setLanguages: %d: %s\n", i, g_settings.pref_epgs[i]);
			
			std::string temp(g_settings.pref_epgs[i]);
			
			for(it = iso639.begin(); it != iso639.end(); it++) 
			{
				if(temp == it->second) 
				{
					v_languages.push_back(it->first);
					
					dprintf(DEBUG_NORMAL, "EPG: setLanguages: adding %s\n", it->first.c_str());
					
					found = true;
				}
			}
		}
	}
	
	if(found)
		sectionsd_set_languages(v_languages);
	
	return true;
}

// epg config notifier
bool CEPGConfigNotifier::changeNotify(const neutrino_locale_t, void *)
{
	dprintf(DEBUG_NORMAL, "CEPGConfigNotifier::changeNotify\n");

        if (g_settings.epg_read)
		CNeutrinoApp::getInstance()->readEPG();
	
        return true;
}

// filebrowser settings
CFileBrowserSettings::CFileBrowserSettings()
{
}

CFileBrowserSettings::~CFileBrowserSettings()
{
}

int CFileBrowserSettings::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CFileBrowserSettings::exec: actionKey: %s\n", actionKey.c_str());
	
	int ret = menu_return::RETURN_REPAINT;
	
	if(parent)
		parent->hide();
	
	showMenu();
	
	return ret;
}

void CFileBrowserSettings::showMenu()
{
	dprintf(DEBUG_NORMAL, "CFileBrowserSettings::showMenu:\n");
	
	CMenuWidget miscSettingsFileBrowser(LOCALE_FILEBROWSER_HEAD, NEUTRINO_ICON_SETTINGS);
	
	int shortcutMiscFileBrowser = 1;
	
	// intros
	miscSettingsFileBrowser.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	miscSettingsFileBrowser.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	miscSettingsFileBrowser.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	miscSettingsFileBrowser.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	// UTF 
	miscSettingsFileBrowser.addItem(new CMenuOptionChooser(LOCALE_FILESYSTEM_IS_UTF8, &g_settings.filesystem_is_utf8, MISCSETTINGS_FILESYSTEM_IS_UTF8_OPTIONS, MISCSETTINGS_FILESYSTEM_IS_UTF8_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutMiscFileBrowser++), "", true ));

	// show rights
	miscSettingsFileBrowser.addItem(new CMenuOptionChooser(LOCALE_FILEBROWSER_SHOWRIGHTS, &g_settings.filebrowser_showrights, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutMiscFileBrowser++) ));

	// deny dir
	miscSettingsFileBrowser.addItem(new CMenuOptionChooser(LOCALE_FILEBROWSER_DENYDIRECTORYLEAVE, &g_settings.filebrowser_denydirectoryleave, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, NULL, CRCInput::convertDigitToKey(shortcutMiscFileBrowser++) ));
	
	miscSettingsFileBrowser.exec(NULL, "");
	miscSettingsFileBrowser.hide();
}

// misc notifier
CMiscNotifier::CMiscNotifier( CMenuItem* i1)
{
   	toDisable[0] = i1;
}

bool CMiscNotifier::changeNotify(const neutrino_locale_t, void *)
{
	dprintf(DEBUG_NORMAL, "CMiscNotifier::changeNotify\n");

   	toDisable[0]->setActive(!g_settings.shutdown_real);

   	return true;
}

