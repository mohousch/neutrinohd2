/*
	Neutrino-GUI  -   DBoxII-Project

	$id: power_menu.cpp 2016.01.29 17:22:30 mohousch $
	
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

#include <gui/power_menu.h>
#include <gui/sleeptimer.h>

#include <system/debug.h>


CPowerMenu::CPowerMenu()
{
}

CPowerMenu::~CPowerMenu()
{
}

int CPowerMenu::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CPowerMenu::exec: actionKey: %s\n", actionKey.c_str());
	
	int ret = menu_return::RETURN_REPAINT;
	
	if(parent)
		parent->hide();
	
	if(actionKey == "restart")
	{
		CNeutrinoApp::getInstance()->exec(NULL, "restart");
		
		return ret;
	}
	else if(actionKey == "standby")
	{
		CNeutrinoApp::getInstance()->exec(NULL, "standby");
		
		return ret;
	}
	else if(actionKey == "reboot")
	{
		CNeutrinoApp::getInstance()->exec(NULL, "reboot");
		
		return ret;
	}
	else if(actionKey == "shutdown")
	{
		CNeutrinoApp::getInstance()->exec(NULL, "shutdown");
		
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
void CPowerMenu::showMenu(void)
{
	dprintf(DEBUG_NORMAL, "CPowerMenu::showMenu:\n");

	int shortcut = 1;

	CMenuWidgetExtended* powerMenu = new CMenuWidgetExtended(LOCALE_MAINMENU_POWERMENU, NEUTRINO_ICON_BUTTON_POWER);
	
	// sleep timer
	powerMenu->addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_SLEEPTIMER, true, new CSleepTimerWidget, NULL, CRCInput::convertDigitToKey(shortcut++), NULL, NEUTRINO_ICON_MENUITEM_SLEEPTIMER));

	// restart neutrino
	powerMenu->addItem(new CMenuForwarderExtended(LOCALE_SERVICEMENU_RESTART, true, this, "restart", CRCInput::convertDigitToKey(shortcut++), NULL, NEUTRINO_ICON_MENUITEM_RESTART));

	// standby
	powerMenu->addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_STANDBY, true, this, "standby", CRCInput::convertDigitToKey(shortcut++), NULL, NEUTRINO_ICON_MENUITEM_STANDBY));

	// reboot
	powerMenu->addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_REBOOT, true, this, "reboot", CRCInput::convertDigitToKey(shortcut++), NULL, NEUTRINO_ICON_MENUITEM_REBOOT));

	// shutdown
	powerMenu->addItem(new CMenuForwarderExtended(LOCALE_MAINMENU_SHUTDOWN, true, this, "shutdown", CRCInput::RC_standby, NEUTRINO_ICON_BUTTON_POWER, NEUTRINO_ICON_MENUITEM_SHUTDOWN));
	
	powerMenu->exec(NULL, "");
	powerMenu->hide();
	delete powerMenu;
	powerMenu = NULL;
}

// smart
void CPowerMenu::showMenuSmart(void)
{
	dprintf(DEBUG_NORMAL, "CPowerMenu::showMenuSmart:\n");

	int shortcut = 1;

	CMenuFrameBox * powerMenu = new CMenuFrameBox(LOCALE_MAINMENU_POWERMENU, NEUTRINO_ICON_BUTTON_POWER);
	
	// sleep timer
	powerMenu->addItem(new CMenuFrameBoxItem(LOCALE_MAINMENU_SLEEPTIMER, new CSleepTimerWidget, NULL, NEUTRINO_ICON_SMART_SLEEPTIMER));

	// restart neutrino
	powerMenu->addItem(new CMenuFrameBoxItem(LOCALE_SERVICEMENU_RESTART, this, "restart", NEUTRINO_ICON_SMART_RESTART));

	// standby
	powerMenu->addItem(new CMenuFrameBoxItem(LOCALE_MAINMENU_STANDBY, this, "standby", NEUTRINO_ICON_SMART_STANDBY));

	// reboot
	powerMenu->addItem(new CMenuFrameBoxItem(LOCALE_MAINMENU_REBOOT, this, "reboot", NEUTRINO_ICON_SMART_REBOOT));

	// shutdown
	powerMenu->addItem(new CMenuFrameBoxItem(LOCALE_MAINMENU_SHUTDOWN, this, "shutdown", NEUTRINO_ICON_SMART_SHUTDOWN));
	
	powerMenu->exec(NULL, "");
	powerMenu->hide();
	delete powerMenu;
	powerMenu = NULL;
}

// classic
void CPowerMenu::showMenuClassic(void)
{
	dprintf(DEBUG_NORMAL, "CPowerMenu::showMenuClassic:\n");

	int shortcut = 1;

	CMenuWidget * powerMenu = new CMenuWidget(LOCALE_MAINMENU_POWERMENU, NEUTRINO_ICON_BUTTON_POWER);
	
	// sleep timer
	powerMenu->addItem(new CMenuForwarder(LOCALE_MAINMENU_SLEEPTIMER, true, NULL, new CSleepTimerWidget, NULL, CRCInput::convertDigitToKey(shortcut++), NEUTRINO_ICON_CLASSIC_SLEEPTIMER));

	// restart neutrino
	powerMenu->addItem(new CMenuForwarder(LOCALE_SERVICEMENU_RESTART, true, NULL, this, "restart", CRCInput::convertDigitToKey(shortcut++), NEUTRINO_ICON_CLASSIC_RESTART));

	// standby
	powerMenu->addItem(new CMenuForwarder(LOCALE_MAINMENU_STANDBY, true, NULL, this, "standby", CRCInput::convertDigitToKey(shortcut++), NEUTRINO_ICON_CLASSIC_STANDBY));

	// reboot
	powerMenu->addItem(new CMenuForwarder(LOCALE_MAINMENU_REBOOT, true, NULL, this, "reboot", CRCInput::convertDigitToKey(shortcut++), NEUTRINO_ICON_CLASSIC_REBOOT));

	// shutdown
	powerMenu->addItem(new CMenuForwarder(LOCALE_MAINMENU_SHUTDOWN, true, NULL, this, "shutdown", CRCInput::RC_standby, NEUTRINO_ICON_CLASSIC_SHUTDOWN));
	
	powerMenu->exec(NULL, "");
	powerMenu->hide();
	delete powerMenu;
	powerMenu = NULL;
}



