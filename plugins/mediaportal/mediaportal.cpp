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

int CMediaPortal::exec(CMenuTarget * parent, const std::string & actionKey)
{
	dprintf(DEBUG_NORMAL, "CMediaPortal::exec: actionKey:%s\n", actionKey.c_str());

	int returnval = menu_return::RETURN_REPAINT;

	if(parent) 
		parent->hide();

	if(actionKey == "musicdeluxe")
	{
		CFile file;
		
		file.Title = "Music deluxe";
		file.Info1 = "stream";
		file.Info2 = "Musik Sender";
		file.Thumbnail = PLUGINDIR "/mediaportal/musicdeluxe.png";
		file.Name = "musicdeluxe";
		file.Url = "rtmp://flash.cdn.deluxemusic.tv/deluxemusic.tv-live/web_850.stream";

		CMoviePlayerGui tmpMoviePlayerGui;
					
		tmpMoviePlayerGui.addToPlaylist(file);
		tmpMoviePlayerGui.exec(NULL, "urlplayback");
	}
	else if(actionKey == "youtube")
	{
		g_PluginList->startPlugin("youtube");
	}
	else if(actionKey == "netzkino")
	{
		g_PluginList->startPlugin("netzkino");
	}
	else if(actionKey == "internetradio")
	{
		CAudioPlayerGui internetRadio(true);
		internetRadio.exec(NULL, "");
	}
	
	return returnval;
}

void CMediaPortal::showMenu(void)
{
	CSmartMenu * mediaPortal = new CSmartMenu("Media Portal", PLUGINDIR "/mediaportal/mp.png");

	// musicdeluxe
	mediaPortal->addItem(new CMenuFrameBox("Music Deluxe", this, "musicdeluxe", PLUGINDIR "/mediaportal/musicdeluxe.png"));

	// youtube
	mediaPortal->addItem(new CMenuFrameBox("You Tube", this, "youtube", PLUGINDIR "/youtube/youtube.png"));

	// netzkino
	mediaPortal->addItem(new CMenuFrameBox("NetzKino", this, "netzkino", PLUGINDIR "/netzkino/netzkino.png"));

	// internetradio
	mediaPortal->addItem(new CMenuFrameBox("Internet Radio", this, "internetradio", NEUTRINO_ICON_MENUITEM_INTERNETRADIO));

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
	
	mpHandler->showMenu();
	
	delete mpHandler;
}


