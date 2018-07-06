/*
  $Id: test.cpp 2014/01/22 mohousch Exp $

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


extern "C" void plugin_exec(void);
extern "C" void plugin_init(void);
extern "C" void plugin_del(void);

class CTSPlayer : public CMenuTarget
{
	private:
		CFrameBuffer* frameBuffer;
		
	public:
		CTSPlayer();
		~CTSPlayer();
		int exec(CMenuTarget* parent, const std::string& actionKey);
		void hide();
		void showMenu();
};

CTSPlayer::CTSPlayer()
{
	frameBuffer = CFrameBuffer::getInstance();
}

CTSPlayer::~CTSPlayer()
{
}

void CTSPlayer::hide()
{
	CFrameBuffer::getInstance()->paintBackground();
	CFrameBuffer::getInstance()->blit();
}

int CTSPlayer::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "\nCTSPlayer::exec: actionKey:%s\n", actionKey.c_str());
	
	if(parent)
		hide();
	
	return menu_return::RETURN_REPAINT;
}

void CTSPlayer::showMenu()
{
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
	
	CMoviePlayerGui tmpMoviePlayerGui;	
	CMovieBrowser * movieBrowser;
	MI_MOVIE_INFO * mfile;
	
	movieBrowser = new CMovieBrowser();
	
	movieBrowser->setMode(MB_SHOW_RECORDS);
	
	std::string Path_local = g_settings.network_nfs_moviedir;

BROWSER:
	if (movieBrowser->exec(Path_local.c_str()))
	{
		Path_local = movieBrowser->getCurrentDir();
		
		if (movieBrowser->getSelectedFile()!= NULL) 
		{
			mfile = movieBrowser->getCurrentMovieInfo();
					
			tmpMoviePlayerGui.addToPlaylist(*mfile);
			tmpMoviePlayerGui.exec(NULL, "urlplayback");
		}

		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg != CRCInput::RC_home) 
		{
			goto BROWSER;
		}
	}
	
	delete movieBrowser;
}

void plugin_init(void)
{
	dprintf(DEBUG_NORMAL, "test: plugin_init\n");
}

void plugin_del(void)
{
	dprintf(DEBUG_NORMAL, "test: plugin_del\n");
}

void plugin_exec(void)
{
	CTSPlayer* tsPlayerHandler = new CTSPlayer();
	
	tsPlayerHandler->showMenu();
	
	delete tsPlayerHandler;
	tsPlayerHandler = NULL;
}


