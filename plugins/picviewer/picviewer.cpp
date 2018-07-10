/*
  $Id: test.cpp 2018/07/10 mohousch Exp $

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

class CPicViewer : public CMenuTarget
{
	private:
		CFrameBuffer* frameBuffer;

		ClistBox* plist;
		CMenuItem* item;

		CPictureViewerGui tmpPictureViewerGui;
		CFileFilter fileFilter;
		CFileList filelist;
		CPicture pic;
		CPicturePlayList playlist;
		std::string Path;
		int selected;

		void loadPlaylist(bool reload = true);
		
	public:
		CPicViewer();
		~CPicViewer();
		int exec(CMenuTarget* parent, const std::string& actionKey);
		void hide();
		void showMenu(bool reload = true);
};

CPicViewer::CPicViewer()
{
	frameBuffer = CFrameBuffer::getInstance();

	plist = NULL;
	item = NULL;

	selected = 0;

	fileFilter.addFilter("png");
	fileFilter.addFilter("bmp");
	fileFilter.addFilter("jpg");
	fileFilter.addFilter("jpeg");
}

CPicViewer::~CPicViewer()
{
	playlist.clear();
}

void CPicViewer::hide()
{
	CFrameBuffer::getInstance()->paintBackground();
	CFrameBuffer::getInstance()->blit();
}

void CPicViewer::loadPlaylist(bool reload)
{
	Path = g_settings.network_nfs_picturedir;

	if(g_settings.picviewer_read_playlist_at_start && reload)
	{
		if(CFileHelpers::getInstance()->readDir(Path, &filelist, &fileFilter))
		{
			struct stat statbuf;
				
			CFileList::iterator files = filelist.begin();
			for(; files != filelist.end() ; files++)
			{
				if (files->getType() == CFile::FILE_PICTURE)
				{
					pic.Filename = files->Name;
					std::string tmp = files->Name.substr(files->Name.rfind('/') + 1);
					pic.Name = tmp.substr(0, tmp.rfind('.'));
					pic.Type = tmp.substr(tmp.rfind('.') + 1);
			
					if(stat(pic.Filename.c_str(), &statbuf) != 0)
						printf("stat error");
					pic.Date = statbuf.st_mtime;
				
					playlist.push_back(pic);
				}
			}
		}
	}
}

int CPicViewer::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CPicViewer::exec: actionKey:%s\n", actionKey.c_str());
	
	if(parent)
		hide();

	if(actionKey == "view")
	{
		selected = plist->getSelected();

		tmpPictureViewerGui.addToPlaylist(playlist[plist->getSelected()]);
		tmpPictureViewerGui.exec(NULL, "");
	}
	else if(actionKey == "RC_setup")
	{
		CPictureViewerSettings * pictureViewerSettingsMenu = new CPictureViewerSettings();
		pictureViewerSettingsMenu->exec(NULL, "");
		delete pictureViewerSettingsMenu;
		pictureViewerSettingsMenu = NULL;					
	}
	else if(actionKey == "RC_info")
	{
		tmpPictureViewerGui.showHelp();
	}
	else if(actionKey == "RC_red")
	{
		CPicturePlayList::iterator p = playlist.begin() + plist->getSelected();
		playlist.erase(p);

		if (selected >= playlist.size())
			selected = playlist.size() - 1;

		showMenu();
		return menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey == "RC_green")
	{
		playlist.clear();

		CFileBrowser filebrowser((g_settings.filebrowser_denydirectoryleave) ? g_settings.network_nfs_picturedir : "");

		filebrowser.Multi_Select = true;
		filebrowser.Dirs_Selectable = true;
		filebrowser.Filter = &fileFilter;

		if (filebrowser.exec(Path.c_str()))
		{
			Path = filebrowser.getCurrentDir();
			CFileList::const_iterator files = filebrowser.getSelectedFiles().begin();
			for(; files != filebrowser.getSelectedFiles().end(); files++)
			{
				if(files->getType() == CFile::FILE_PICTURE)
				{
					CPicture pic;
					pic.Filename = files->Name;
					std::string tmp = files->Name.substr(files->Name.rfind('/') + 1);
					pic.Name = tmp.substr(0, tmp.rfind('.'));
					pic.Type = tmp.substr(tmp.rfind('.') + 1);
					struct stat statbuf;
					if(stat(pic.Filename.c_str(),&statbuf) != 0)
						printf("stat error");
					pic.Date = statbuf.st_mtime;
							
					playlist.push_back(pic);
				}
			}
		}

		showMenu();
		return menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey == "RC_yellow")
	{
		playlist.clear();
		showMenu(false);
		return menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey == "RC_blue")
	{
		selected = plist->getSelected();

		for (unsigned int i = 0; i < playlist.size(); i++)
		{
			tmpPictureViewerGui.addToPlaylist(playlist[i]);
		}

		tmpPictureViewerGui.setCurrent(selected);
		tmpPictureViewerGui.setState(CPictureViewerGui::SLIDESHOW);		
		tmpPictureViewerGui.exec(NULL, "");
	}
	
	return menu_return::RETURN_REPAINT;
}

#define HEAD_BUTTONS_COUNT	3
const struct button_label HeadButtons[HEAD_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_SETUP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_HELP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_5, NONEXISTANT_LOCALE, NULL }
};

#define FOOT_BUTTONS_COUNT 4
const struct button_label PictureViewerButtons[FOOT_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_RED, LOCALE_AUDIOPLAYER_DELETE, NULL },
	{ NEUTRINO_ICON_BUTTON_GREEN , LOCALE_AUDIOPLAYER_ADD, NULL },
	{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_AUDIOPLAYER_DELETEALL, NULL },
	{ NEUTRINO_ICON_BUTTON_BLUE  , LOCALE_PICTUREVIEWER_SLIDESHOW, NULL }
};

void CPicViewer::showMenu(bool reload)
{
	plist = new ClistBox(LOCALE_PICTUREVIEWER_HEAD, NEUTRINO_ICON_PICTURE, w_max ( (frameBuffer->getScreenWidth() / 20 * 17), (frameBuffer->getScreenWidth() / 20 )), h_max ( (frameBuffer->getScreenHeight() / 20 * 16), (frameBuffer->getScreenHeight() / 20)));
	
	// load playlist
	loadPlaylist(reload);

	for(unsigned int i = 0; i < playlist.size(); i++)
	{
		std::string tmp = playlist[i].Name;
		tmp += " (";
		tmp += playlist[i].Type;
		tmp += ')';

		char timestring[18];
		strftime(timestring, 18, "%d-%m-%Y %H:%M", gmtime(&playlist[i].Date));

		item = new ClistBoxItem(tmp.c_str(), true, "", this, "view");
		
		item->setNumber(i + 1);	
		item->setOptionInfo(timestring);

		plist->addItem(item);
	}

	//plist->setTimeOut(g_settings.timing[SNeutrinoSettings::TIMING_CHANLIST]);
	plist->setSelected(selected);

	plist->setHeaderButtons(HeadButtons, HEAD_BUTTONS_COUNT);
	plist->setFooterButtons(PictureViewerButtons, FOOT_BUTTONS_COUNT);
	
	plist->enablePaintDate();

	plist->addKey(CRCInput::RC_info, this, CRCInput::getSpecialKeyName(CRCInput::RC_info));
	plist->addKey(CRCInput::RC_setup, this, CRCInput::getSpecialKeyName(CRCInput::RC_setup));
	plist->addKey(CRCInput::RC_red, this, CRCInput::getSpecialKeyName(CRCInput::RC_red));
	plist->addKey(CRCInput::RC_green, this, CRCInput::getSpecialKeyName(CRCInput::RC_green));
	plist->addKey(CRCInput::RC_yellow, this, CRCInput::getSpecialKeyName(CRCInput::RC_yellow));
	plist->addKey(CRCInput::RC_blue, this, CRCInput::getSpecialKeyName(CRCInput::RC_blue));

	plist->exec(NULL, "");
	//plist->hide();
	delete plist;
	plist = NULL;
}

void plugin_init(void)
{
	dprintf(DEBUG_NORMAL, "CPicViewer: plugin_init\n");
}

void plugin_del(void)
{
	dprintf(DEBUG_NORMAL, "CPicViewer: plugin_del\n");
}

void plugin_exec(void)
{
	CPicViewer* picViewerHandler = new CPicViewer();
	
	picViewerHandler->showMenu();
	
	delete picViewerHandler;
	picViewerHandler = NULL;
}


