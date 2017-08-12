/* 
  $Id: mediaportal.cpp 2015/13/22 mohousch Exp $

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

#include <mediaportal.h>


extern "C" void plugin_exec(void);
extern "C" void plugin_init(void);
extern "C" void plugin_del(void);


CMediaPortal::CMediaPortal()
{
	dprintf(DEBUG_NORMAL, "$Id: CMediaPortal, 2016.02.10 mohousch Exp $\n");
}

CMediaPortal::~CMediaPortal()
{
	dprintf(DEBUG_NORMAL, "CMediaPortal: del\n");
}

void CMediaPortal::youTube(void)
{
	g_PluginList->startPlugin("youtube");
}

void CMediaPortal::netzKino(void)
{
	g_PluginList->startPlugin("netzkino");
}

void CMediaPortal::iceCast(void)
{
	g_PluginList->startPlugin("icecast");
}

void CMediaPortal::ard(void)
{
	CARD* ard = new CARD();
	ard->exec(NULL, "");
	delete ard;
	ard = NULL;
}

void CMediaPortal::internetRadio(void)
{
	g_PluginList->startPlugin("internetradio");
}

int CMediaPortal::exec(CMenuTarget * parent, const std::string & actionKey)
{
	dprintf(DEBUG_NORMAL, "CMediaPortal::exec: actionKey:%s\n", actionKey.c_str());

	int returnval = menu_return::RETURN_REPAINT;

	if(parent) 
		parent->hide();

	if(actionKey == "youtube")
	{
		youTube();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "netzkino")
	{
		netzKino();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "icecast")
	{
		iceCast();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "ard")
	{
		ard();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "internetradio")
	{
		internetRadio();

		return menu_return::RETURN_REPAINT;
	}

	showMenu();
	
	return returnval;
}

void CMediaPortal::showMenu(void)
{
	CMenuFrameBox* mediaPortal = new CMenuFrameBox("Media Portal", PLUGINDIR "/mediaportal/mp.png");

	// youtube
	mediaPortal->addItem(new CMenuFrameBoxItem("You Tube", this, "youtube", PLUGINDIR "/youtube/youtube.png"));

	// netzkino
	mediaPortal->addItem(new CMenuFrameBoxItem("NetzKino", this, "netzkino", PLUGINDIR "/netzkino/netzkino.png"));

	// icecast
	mediaPortal->addItem(new CMenuFrameBoxItem("Ice Cast", this, "icecast", PLUGINDIR "/icecast/icecast.png"));

	// ard
	mediaPortal->addItem(new CMenuFrameBoxItem("ARD Mediathek", this, "ard", PLUGINDIR "/mediaportal/ard.png"));

	// internetradio
	mediaPortal->addItem(new CMenuFrameBoxItem("Internet Radio", this, "internetradio", PLUGINDIR "/internetradio/internetradio.png"));

	mediaPortal->exec(NULL, "");
	mediaPortal->hide();
	delete mediaPortal;
	mediaPortal = NULL;
}

//plugin API
void plugin_init(void)
{
}

void plugin_del(void)
{
}

void plugin_exec(void)
{
	CMediaPortal * mpHandler = new CMediaPortal();
	
	//mpHandler->showMenu();
	mpHandler->exec(NULL, "");
	
	delete mpHandler;
	mpHandler = NULL;
}


