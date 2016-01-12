/*
	Neutrino-GUI  -   DBoxII-Project

	$id: service_setup.cpp 2015.12.22 17:19:30 mohousch $
	
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

#include <global.h>
#include <neutrino.h>

#include <configfile.h>

#include <system/debug.h>
#include <system/settings.h>
#include <system/flashtool.h>

#include <gui/widget/hintbox.h>

#include <gui/service_setup.h>
#include <gui/scan_setup.h>
#include <gui/widget/icons.h>
#include <gui/update.h>
#include <gui/scan_setup.h>
#include <gui/cam_menu.h>
#include <gui/imageinfo.h>

#include <gui/bedit/bouqueteditor_bouquets.h>


#if !defined (PLATFORM_COOLSTREAM)
extern CCAMMenuHandler * g_CamHandler;		// defined neutrino.cpp
#endif

extern int FrontendCount;			// defined in zapit.cpp

//
CServiceSetup::CServiceSetup()
{
}

CServiceSetup::~CServiceSetup()
{
}

int CServiceSetup::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CServiceSetup::exec: actionKey:%s\n", actionKey.c_str());
	
	int ret = menu_return::RETURN_REPAINT;
	
	if(parent)
		parent->hide();
	
	if(actionKey == "reloadchannels")
	{
		CHintBox * hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_SERVICEMENU_RELOAD_HINT));

		hintBox->paint();
		
		g_Zapit->reinitChannels();
		
		hintBox->hide();
		delete hintBox;
		hintBox = NULL;
		
		return ret;
	}
	else if(actionKey == "restart")
	{
		CNeutrinoApp::getInstance()->exec(NULL, "restart");
		
		return ret;
	}
	
	showMenu();
	
	return ret;
}

void CServiceSetup::showMenu()
{
	dprintf(DEBUG_NORMAL, "CServiceSetup::showMenu\n");
	
	int shortcutService = 1;
	
	CMenuWidget service(LOCALE_SERVICEMENU_HEAD, NEUTRINO_ICON_SETTINGS);
	
	// scan setup
	if(FrontendCount > 1)
	{
		// scan settings
		service.addItem(new CMenuForwarderExtended(LOCALE_SERVICEMENU_SCANTS, true, new CTunerSetup(), NULL, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED, NEUTRINO_ICON_MENUITEM_SERVICE, LOCALE_HELPTEXT_SCANSETUP ));
	}
	else
	{
		// scan settings
		service.addItem(new CMenuForwarderExtended(LOCALE_SERVICEMENU_SCANTS, true, new CScanSetup(), "", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED, NEUTRINO_ICON_MENUITEM_SERVICE, LOCALE_HELPTEXT_SCANSETUP ));
	}

	// reload Channels
	service.addItem(new CMenuForwarderExtended(LOCALE_SERVICEMENU_RELOAD, true, this, "reloadchannels", CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN, NEUTRINO_ICON_MENUITEM_SERVICE, LOCALE_HELPTEXT_RELOADCHANNELS ));

	// Bouquets Editor
	service.addItem(new CMenuForwarderExtended(LOCALE_BOUQUETEDITOR_NAME, true, new CBEBouquetWidget(), NULL, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW, NEUTRINO_ICON_MENUITEM_SERVICE, LOCALE_HELPTEXT_BOUQUETSEDITOR ));
	
	// CI Cam 	
#if defined (ENABLE_CI)
	service.addItem(new CMenuForwarderExtended(LOCALE_CAM_SETTINGS, true, g_CamHandler, NULL, CRCInput::convertDigitToKey(shortcutService++), NULL, NEUTRINO_ICON_MENUITEM_CAM, LOCALE_HELPTEXT_CAM ));
#endif
	
	// image info
	service.addItem(new CMenuForwarderExtended(LOCALE_SERVICEMENU_IMAGEINFO,  true, new CImageInfo(), NULL, CRCInput::RC_info, NEUTRINO_ICON_BUTTON_HELP_SMALL, NEUTRINO_ICON_MENUITEM_BOXINFO, LOCALE_HELPTEXT_IMAGEINFO), false);

	// restart neutrino
	service.addItem(new CMenuForwarderExtended(LOCALE_SERVICEMENU_RESTART, true, this, "restart", CRCInput::RC_standby, NEUTRINO_ICON_BUTTON_POWER, NEUTRINO_ICON_MENUITEM_SHUTDOWN, LOCALE_HELPTEXT_SOFTRESTART ));
	
	// software update
	service.addItem(new CMenuForwarderExtended(LOCALE_SERVICEMENU_UPDATE, true, new CUpdateSettings(), NULL, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE, NEUTRINO_ICON_MENUITEM_SERVICE, LOCALE_HELPTEXT_SOFTWAREUPDATE ));
	
	service.exec(NULL, "");
	service.hide();
}

