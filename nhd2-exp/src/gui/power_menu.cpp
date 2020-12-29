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
	dprintf(DEBUG_DEBUG, "CPowerMenu::CPowerMenu:\n");
}

CPowerMenu::~CPowerMenu()
{
	dprintf(DEBUG_DEBUG, "CPowerMenu::del\n");
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
	
	showMenu();
	
	return ret;
}

// showmenu
void CPowerMenu::showMenu(void)
{
	dprintf(DEBUG_NORMAL, "CPowerMenu::showMenu:\n");

	int shortcut = 1;

	ClistBoxWidget * powerMenu = new ClistBoxWidget(LOCALE_MAINMENU_POWERMENU, NEUTRINO_ICON_BUTTON_POWER);

	powerMenu->setMode(MODE_MENU);
	powerMenu->enableShrinkMenu();
	powerMenu->enableMenuPosition();
	powerMenu->enableWidgetChange();
	powerMenu->enablePaintItemInfo();
	powerMenu->enablePaintDate();
	
	// sleep timer
	powerMenu->addItem(new CMenuForwarder(LOCALE_MAINMENU_SLEEPTIMER, true, NULL, new CSleepTimerWidget, NULL, CRCInput::convertDigitToKey(shortcut++), NULL, NEUTRINO_ICON_MENUITEM_SLEEPTIMER, LOCALE_HELPTEXT_SLEEPTIMER));

	// restart neutrino
	powerMenu->addItem(new CMenuForwarder(LOCALE_SERVICEMENU_RESTART, true, NULL, this, "restart", CRCInput::convertDigitToKey(shortcut++), NULL, NEUTRINO_ICON_MENUITEM_RESTART, LOCALE_HELPTEXT_RESTART));

	// standby
	powerMenu->addItem(new CMenuForwarder(LOCALE_MAINMENU_STANDBY, true, NULL, this, "standby", CRCInput::convertDigitToKey(shortcut++), NULL, NEUTRINO_ICON_MENUITEM_STANDBY, LOCALE_HELPTEXT_STANDBY));

	// reboot
	powerMenu->addItem(new CMenuForwarder(LOCALE_MAINMENU_REBOOT, true, NULL, this, "reboot", CRCInput::convertDigitToKey(shortcut++), NULL, NEUTRINO_ICON_MENUITEM_REBOOT, LOCALE_HELPTEXT_REBOOT));

	// shutdown
	powerMenu->addItem(new CMenuForwarder(LOCALE_MAINMENU_SHUTDOWN, true, NULL, this, "shutdown", RC_standby, NEUTRINO_ICON_BUTTON_POWER, NEUTRINO_ICON_MENUITEM_SHUTDOWN, LOCALE_HELPTEXT_SHUTDOWN));

	powerMenu->integratePlugins(CPlugins::I_TYPE_POWER, shortcut++);
	
	powerMenu->exec(NULL, "");
	powerMenu->hide();
	delete powerMenu;
	powerMenu = NULL;
}



