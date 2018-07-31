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

#include <plugin.h>
#include <ard.h>


extern "C" void plugin_exec(void);
extern "C" void plugin_init(void);
extern "C" void plugin_del(void);


class CMediaPortal : public CMenuTarget
{
	private:
		CMenuWidget* mediaPortal;
		CMenuItem* item;

		void youTube(void);
		void netzKino(void);
		void iceCast(void);
		void internetRadio(void);
		void ard(void);
		void nFilm(void);
	
	public:
		CMediaPortal();
		~CMediaPortal();
		
		int exec(CMenuTarget* parent, const std::string& actionKey);
		void showMenu(void);

};

CMediaPortal::CMediaPortal()
{
	dprintf(DEBUG_NORMAL, "$Id: CMediaPortal, 2016.02.10 mohousch Exp $\n");

	mediaPortal = NULL;
	item = NULL;
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

void CMediaPortal::internetRadio(void)
{
	g_PluginList->startPlugin("internetradio");
}

void CMediaPortal::ard(void)
{
	CARD* ard = new CARD();
	ard->exec(NULL, "");
	delete ard;
	ard = NULL;
}

void CMediaPortal::nFilm()
{
	g_PluginList->startPlugin("nfilm");
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
	else if(actionKey == "internetradio")
	{
		internetRadio();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "ard")
	{
		ard();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "nfilm")
	{
		nFilm();

		return menu_return::RETURN_REPAINT;
	}

	showMenu();
	
	return returnval;
}

void CMediaPortal::showMenu(void)
{
	mediaPortal = new CMenuWidget("Media Portal", PLUGINDIR "/mediaportal/mp.png");

	mediaPortal->enableWidgetChange();
	mediaPortal->enableFootInfo();

	// youtube
	item = new CMenuForwarder("You Tube", true, NULL, this, "youtube", CRCInput::RC_nokey, NULL, PLUGINDIR "/youtube/youtube.png");

	item->setHelpText(g_PluginList->getDescription(g_PluginList->find_plugin("youtube")));

	mediaPortal->addItem(item);

	// netzkino
	item = new CMenuForwarder("NetzKino", true, NULL, this, "netzkino", CRCInput::RC_nokey, NULL, PLUGINDIR "/netzkino/netzkino.png");
	item->setHelpText(g_PluginList->getDescription(g_PluginList->find_plugin("netzkino")));

	mediaPortal->addItem(item);

	// icecast
	item = new CMenuForwarder("Ice Cast", true, NULL, this, "icecast", CRCInput::RC_nokey, NULL, PLUGINDIR "/icecast/icecast.png");
	item->setHelpText(g_PluginList->getDescription(g_PluginList->find_plugin("icecast")));

	mediaPortal->addItem(item);

	// internetradio
	item = new CMenuForwarder("Internet Radio", true, NULL, this, "internetradio", CRCInput::RC_nokey, NULL,  PLUGINDIR "/internetradio/internetradio.png");
	item->setHelpText(g_PluginList->getDescription(g_PluginList->find_plugin("internetradio")));
	
	mediaPortal->addItem(item);

	// ard
	item = new CMenuForwarder("ARD Mediathek", true, NULL, this, "ard", CRCInput::RC_nokey, NULL, PLUGINDIR "/mediaportal/ard.png");

	mediaPortal->addItem(item);

	// nFilm
	item = new CMenuForwarder("Kino Info", true, NULL, this, "nfilm", CRCInput::RC_nokey, NULL, PLUGINDIR "/nfilm/nfilm.png");
	item->setHelpText(g_PluginList->getDescription(g_PluginList->find_plugin("nfilm")));

	mediaPortal->addItem(item);

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


