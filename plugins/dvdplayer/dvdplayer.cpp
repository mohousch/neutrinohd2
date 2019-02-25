/*
  $Id: dvdplayer.cpp 2018/07/22 mohousch Exp $

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

class CDVDPlayer : public CMenuTarget
{
	private:
		CFileBrowser fileBrowser;
		CFileFilter fileFilter;
	
		CMoviePlayerGui tmpMoviePlayerGui;
		std::string Path_dvd ;

		neutrino_msg_t msg;
		neutrino_msg_data_t data;

		void showMenu(void);

	public:
		CDVDPlayer();
		~CDVDPlayer();
		int exec(CMenuTarget* parent, const std::string& actionKey);
};

CDVDPlayer::CDVDPlayer()
{
}

CDVDPlayer::~CDVDPlayer()
{
}

void CDVDPlayer::showMenu()
{
	fileFilter.addFilter("vob");

	fileBrowser.Filter = &fileFilter;
	fileBrowser.Multi_Select    = true;
	fileBrowser.Dirs_Selectable = false;
	
	Path_dvd = "/mnt/dvd";
				
	// create mount path
	safe_mkdir((char *)Path_dvd.c_str());
						
	// mount selected iso image
	char cmd[128];
	sprintf(cmd, "mount -o loop /media/hdd/dvd.iso %s", (char *)Path_dvd.c_str());
	system(cmd);
	
DVD_BROWSER:
	if(fileBrowser.exec(Path_dvd.c_str()))
	{
		Path_dvd = fileBrowser.getCurrentDir();

		CFileList::const_iterator files = fileBrowser.getSelectedFiles().begin();
		for(; files != fileBrowser.getSelectedFiles().end(); files++)
		{
			MI_MOVIE_INFO mfile;
			
			mfile.file.Name = files->Name;
			
			tmpMoviePlayerGui.addToPlaylist(mfile);
		}
		
		tmpMoviePlayerGui.exec(NULL, "urlplayback");

		g_RCInput->getMsg_ms(&msg, &data, 10);
		
		if (msg != RC_home) 
		{
			goto DVD_BROWSER;
		}
	}
}

int CDVDPlayer::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CDVDPlayer::exec:%s\n", actionKey.c_str());

	if(parent)
		hide();

	showMenu();

	return menu_return::RETURN_REPAINT;
}

void plugin_init(void)
{
}

void plugin_del(void)
{
}

void plugin_exec(void)
{
	CDVDPlayer* dvdPlayerHandler = new CDVDPlayer();

	dvdPlayerHandler->exec(NULL, "");

	delete dvdPlayerHandler;
	dvdPlayerHandler = NULL;
}
