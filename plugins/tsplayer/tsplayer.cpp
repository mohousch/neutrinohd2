/*
  $Id: tsplayer.cpp 2018/07/10 mohousch Exp $

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

		//
		CMoviePlayerGui tmpMoviePlayerGui;	
		CMovieBrowser * movieBrowser;
		MI_MOVIE_INFO * mfile;

		std::string Path;

		void showMenu();		
		
	public:
		CTSPlayer();
		~CTSPlayer();
		int exec(CMenuTarget* parent, const std::string& actionKey);
		void hide();
};

CTSPlayer::CTSPlayer()
{
	frameBuffer = CFrameBuffer::getInstance();

	//	
	movieBrowser = NULL;
	mfile = NULL;
}

CTSPlayer::~CTSPlayer()
{
}

void CTSPlayer::hide()
{
	frameBuffer->paintBackground();
	frameBuffer->blit();
}

void CTSPlayer::showMenu()
{
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
	
	movieBrowser = new CMovieBrowser();
	
	movieBrowser->setMode(MB_SHOW_RECORDS);
	
	Path = g_settings.network_nfs_moviedir;

BROWSER:
	if (movieBrowser->exec(Path.c_str()))
	{
		Path = movieBrowser->getCurrentDir();
		
		if (movieBrowser->getSelectedFile()!= NULL) 
		{
			mfile = movieBrowser->getCurrentMovieInfo();
					
			tmpMoviePlayerGui.addToPlaylist(*mfile);
			tmpMoviePlayerGui.exec(NULL, "");
		}

		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg != CRCInput::RC_home) 
		{
			goto BROWSER;
		}
	}
	
	delete movieBrowser;
}

int CTSPlayer::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "\nCTSPlayer::exec: actionKey:%s\n", actionKey.c_str());
	
	if(parent)
		hide();

	showMenu();
	
	return menu_return::RETURN_EXIT;
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
	
	tsPlayerHandler->exec(NULL, "");
	
	delete tsPlayerHandler;
	tsPlayerHandler = NULL;
}


