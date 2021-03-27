/*
  $Id: tsbrowser.cpp 24.12.2018 mohousch Exp $

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

class CTSBrowser : public CMenuTarget
{
	private:
		// variables
		CFrameBuffer* frameBuffer;
		int selected;

		//
		ClistBoxWidget* mlist;
		CMenuItem* item;

		//
		CMovieInfo m_movieInfo;
		std::vector<MI_MOVIE_INFO> m_vMovieInfo;
		CFileFilter fileFilter;
		CFileList filelist;
		std::string Path;
		CMoviePlayerGui tmpMoviePlayerGui;

		//
		void loadPlaylist();
		void doTMDB(MI_MOVIE_INFO& movieFile);
		bool delFile(CFile& file);
		void onDeleteFile(MI_MOVIE_INFO& movieFile);
		void openFileBrowser();

		void funArt(int i);
		void playMovie(int i);

		void showMenu();

	public:
		CTSBrowser();
		~CTSBrowser();
		int exec(CMenuTarget* parent, const std::string& actionKey);
		void hide();
};

CTSBrowser::CTSBrowser()
{
	frameBuffer = CFrameBuffer::getInstance();

	//
	mlist = NULL;
	item = NULL;

	//
	selected = 0;

	//
	fileFilter.addFilter("ts");
	fileFilter.addFilter("mpg");
	fileFilter.addFilter("mpeg");
	fileFilter.addFilter("divx");
	fileFilter.addFilter("avi");
	fileFilter.addFilter("mkv");
	fileFilter.addFilter("asf");
	fileFilter.addFilter("aiff");
	fileFilter.addFilter("m2p");
	fileFilter.addFilter("mpv");
	fileFilter.addFilter("m2ts");
	fileFilter.addFilter("vob");
	fileFilter.addFilter("mp4");
	fileFilter.addFilter("mov");	
	fileFilter.addFilter("flv");	
	fileFilter.addFilter("dat");
	fileFilter.addFilter("trp");
	fileFilter.addFilter("vdr");
	fileFilter.addFilter("mts");
	fileFilter.addFilter("wmv");
	fileFilter.addFilter("wav");
	fileFilter.addFilter("flac");
	fileFilter.addFilter("mp3");
	fileFilter.addFilter("wma");
	fileFilter.addFilter("ogg");
}

CTSBrowser::~CTSBrowser()
{
	m_vMovieInfo.clear();
}

void CTSBrowser::hide()
{
	frameBuffer->paintBackground();
	frameBuffer->blit();
}

void CTSBrowser::loadPlaylist()
{
	m_vMovieInfo.clear();

	// recordingdir
	Path = g_settings.network_nfs_recordingdir;

	CHintBox loadBox("Movie Browser", g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
	loadBox.paint();
	
	//
	//if(CFileHelpers::getInstance()->readDir(Path, &filelist, &fileFilter))
	
	CFileHelpers::getInstance()->addRecursiveDir(&filelist, Path, &fileFilter);

	if(filelist.size() > 0)
	{
		// filter them
		MI_MOVIE_INFO movieInfo;
		m_movieInfo.clearMovieInfo(&movieInfo); // refresh structure

		CFileList::iterator files = filelist.begin();
		for(; files != filelist.end() ; files++)
		{
			//
			m_movieInfo.clearMovieInfo(&movieInfo); // refresh structure
					
			movieInfo.file.Name = files->Name;
					
			// load movie infos (from xml file)
			m_movieInfo.loadMovieInfo(&movieInfo);
					
			// 
			m_vMovieInfo.push_back(movieInfo);
		}
	}

	// movie dir
	Path = g_settings.network_nfs_moviedir;
	filelist.clear();

	//if(CFileHelpers::getInstance()->readDir(Path, &filelist, &fileFilter))
	
	CFileHelpers::getInstance()->addRecursiveDir(&filelist, Path, &fileFilter);

	if(filelist.size() > 0)
	{
		// filter them
		MI_MOVIE_INFO movieInfo;
		m_movieInfo.clearMovieInfo(&movieInfo); // refresh structure

		CFileList::iterator files = filelist.begin();
		for(; files != filelist.end() ; files++)
		{
			//
			m_movieInfo.clearMovieInfo(&movieInfo); // refresh structure
					
			movieInfo.file.Name = files->Name;
					
			// load movie infos (from xml file)
			m_movieInfo.loadMovieInfo(&movieInfo);

			// skip duplicate
			for (unsigned long i = 0; i < m_vMovieInfo.size(); i++)
			{
				if(m_vMovieInfo[i].file.getFileName() == movieInfo.file.getFileName())
					m_vMovieInfo.erase(m_vMovieInfo.begin() + i); 
			}
					
			// 
			m_vMovieInfo.push_back(movieInfo);
		}
	}

	loadBox.hide();
}

void CTSBrowser::openFileBrowser()
{
	CFileBrowser filebrowser((g_settings.filebrowser_denydirectoryleave) ? g_settings.network_nfs_picturedir : "");

	filebrowser.Multi_Select = true;
	filebrowser.Dirs_Selectable = true;
	filebrowser.Filter = &fileFilter;

	if (filebrowser.exec(Path.c_str()))
	{
		Path = filebrowser.getCurrentDir();

		MI_MOVIE_INFO movieInfo;
		m_movieInfo.clearMovieInfo(&movieInfo); // refresh structure

		CHintBox loadBox("Movie Browser", g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
		loadBox.paint();

		CFileList::const_iterator files = filebrowser.getSelectedFiles().begin();
		for(; files != filebrowser.getSelectedFiles().end(); files++)
		{
			// filter them
			MI_MOVIE_INFO movieInfo;
			m_movieInfo.clearMovieInfo(&movieInfo); // refresh structure

			//
			m_movieInfo.clearMovieInfo(&movieInfo); // refresh structure
					
			movieInfo.file.Name = files->Name;
					
			// load movie infos (from xml file)
			m_movieInfo.loadMovieInfo(&movieInfo);

			// skip duplicate
			for (unsigned long i = 0; i < m_vMovieInfo.size(); i++)
			{
				if(m_vMovieInfo[i].file.getFileName() == movieInfo.file.getFileName())
					m_vMovieInfo.erase(m_vMovieInfo.begin() + i); 
			}
					
			// 
			m_vMovieInfo.push_back(movieInfo);
		}

		loadBox.hide();
	}
}

void CTSBrowser::doTMDB(MI_MOVIE_INFO& movieFile)
{
	//				
	CTmdb * tmdb = new CTmdb();

	tmdb->clearMInfo();

	if(tmdb->getMovieInfo(movieFile.epgTitle))
	{
		std::vector<tmdbinfo>& minfo_list = tmdb->getMInfos();

		std::string buffer;

		buffer = movieFile.epgTitle;
		buffer += "\n\n";
	
		// prepare print buffer  
		buffer += "Vote: " + to_string(minfo_list[0].vote_average) + "/10 Votecount: " + to_string(minfo_list[0].vote_count);
		buffer += "\n\n";
		buffer += minfo_list[0].overview;
		buffer += "\n";

		buffer += (std::string)g_Locale->getText(LOCALE_EPGVIEWER_LENGTH) + ": " + to_string(minfo_list[0].runtime);
		buffer += "\n";

		buffer += (std::string)g_Locale->getText(LOCALE_EPGVIEWER_GENRE) + ": " + minfo_list[0].genres;
		buffer += "\n";
		buffer += (std::string)g_Locale->getText(LOCALE_EPGEXTENDED_ORIGINAL_TITLE) + " : " + minfo_list[0].original_title;
		buffer += "\n";
		buffer += (std::string)g_Locale->getText(LOCALE_EPGEXTENDED_YEAR_OF_PRODUCTION) + " : " + minfo_list[0].release_date.substr(0,4);
		buffer += "\n";

		if (!minfo_list[0].cast.empty())
			buffer += (std::string)g_Locale->getText(LOCALE_EPGEXTENDED_ACTORS) + ":\n" + minfo_list[0].cast;

		// thumbnail
		std::string tname = tmdb->getThumbnailDir();
		tname += "/";
		tname += movieFile.epgTitle;
		tname += ".jpg";

		tmdb->getSmallCover(minfo_list[0].poster_path, tname);
		
		// scale pic
		int p_w = 0;
		int p_h = 0;

		CFrameBuffer::getInstance()->scaleImage(tname, &p_w, &p_h);
	
		CBox position(g_settings.screen_StartX + 50, g_settings.screen_StartY + 50, g_settings.screen_EndX - g_settings.screen_StartX - 100, g_settings.screen_EndY - g_settings.screen_StartY - 100); 
	
		CInfoBox * infoBox = new CInfoBox(g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1], SCROLL, &position, "", g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE], NEUTRINO_ICON_TMDB);

		infoBox->setText(buffer.c_str(), tname.c_str(), p_w, p_h);
		infoBox->exec();
		delete infoBox;

		if(MessageBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_MOVIEBROWSER_PREFER_TMDB_INFO), mbrNo, mbYes | mbNo) == mbrYes) 
		{
			// rewrite tfile
			std::string tname = m_vMovieInfo[mlist->getSelected()].file.Name;
			changeFileNameExt(tname, ".jpg");

			if(tmdb->getBigCover(minfo_list[0].poster_path, tname)) 
				movieFile.tfile = tname;

			if(m_vMovieInfo[mlist->getSelected()].epgInfo2.empty())
				movieFile.epgInfo2 = buffer;

			m_movieInfo.saveMovieInfo(movieFile);
		}  
	}
	else
	{
		MessageBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_STREAMINFO_NOT_AVAILABLE), mbrBack, mbBack, NEUTRINO_ICON_INFO);
	}

	delete tmdb;
	tmdb = NULL;
}

bool CTSBrowser::delFile(CFile& file)
{
	bool result = true;
	unlink(file.Name.c_str()); // fix: use full path
	dprintf(DEBUG_NORMAL, "CTSBrowser::delete file: %s\r\n", file.Name.c_str());
	return(result);
}

void CTSBrowser::onDeleteFile(MI_MOVIE_INFO& movieFile)
{
	std::string msg = g_Locale->getText(LOCALE_FILEBROWSER_DODELETE1);
	msg += "\r\n ";

	if (movieFile.file.Name.length() > 40)
	{
			msg += movieFile.file.Name.substr(0, 40);
			msg += "...";
	}
	else
		msg += movieFile.file.Name;
			
	msg += "\r\n ";
	msg += g_Locale->getText(LOCALE_FILEBROWSER_DODELETE2);

	if (MessageBox(LOCALE_FILEBROWSER_DELETE, msg.c_str(), mbrNo, mbYes | mbNo) == mbrYes)
	{
		delFile(movieFile.file);
			
                int i = 1;
                char newpath[1024];
                do {
			sprintf(newpath, "%s.%03d", movieFile.file.Name.c_str(), i);
			if(access(newpath, R_OK)) 
			{
				break;
                        } 
                        else 
			{
				unlink(newpath);
				dprintf(DEBUG_NORMAL, "  delete file: %s\r\n", newpath);
                        }
                        i++;
                } while(1);
			
                std::string fname = movieFile.file.Name;
                       
		int ext_pos = 0;
		ext_pos = fname.rfind('.');
		if( ext_pos > 0)
		{
			std::string extension;
			extension = fname.substr(ext_pos + 1, fname.length() - ext_pos);
			extension = "." + extension;
			strReplace(fname, extension.c_str(), ".jpg");
		}
			
                unlink(fname.c_str());

		CFile file_xml  = movieFile.file; 
		if(m_movieInfo.convertTs2XmlName(&file_xml.Name) == true)  
		{
			delFile(file_xml);
	    	}
	    	
		m_vMovieInfo.erase( (std::vector<MI_MOVIE_INFO>::iterator)&movieFile);
	}
}

void CTSBrowser::funArt(int i)
{
	CBox box;
	box.iX = CFrameBuffer::getInstance()->getScreenX();
	box.iY = CFrameBuffer::getInstance()->getScreenY();
	box.iWidth = CFrameBuffer::getInstance()->getScreenWidth();
	box.iHeight = CFrameBuffer::getInstance()->getScreenHeight();

	CBox textbox;
	
	textbox.iWidth = box.iWidth/2 - 20;
	textbox.iHeight = 350;
	textbox.iX = box.iX + 10;
	textbox.iY = box.iY + 40 + 10;

	// playBox
	CBox frame;
	frame.iWidth = 300;
	frame.iHeight = 60;
	frame.iX = box.iX + 10;
	frame.iY = box.iY + box.iHeight - 10 - 40 - 60;

	CWindow * window = new CWindow(&box);
	CFrameBox * testFrame = new CFrameBox(&box);
	testFrame->setMode(FRAMEBOX_MODE_RANDOM);

	CWidget * widget = new CWidget();


	// art
	CFrame * artFrame = new CFrame(FRAME_PICTURE_NOTSELECTABLE);
	artFrame->setPosition(box.iX + box.iWidth/2, box.iY, box.iWidth/2, box.iHeight);
	artFrame->setIconName(m_vMovieInfo[i].tfile.c_str());

	testFrame->addFrame(artFrame);

	// text
	CFrame *textFrame = new CFrame(FRAME_TEXT_NOTSELECTABLE);
	textFrame->setPosition(&textbox);
	std::string buffer;
	buffer = m_vMovieInfo[i].epgTitle;
	buffer += "\n\n";
	buffer += m_vMovieInfo[i].epgInfo1;
	buffer += "\n";
	buffer += m_vMovieInfo[i].epgInfo2;

	textFrame->setTitle(buffer.c_str());
	textFrame->disableShadow();
	
	testFrame->addFrame(textFrame);

	// info
	CFrame * infoFrame = new CFrame();
	infoFrame->setPosition(frame.iX + 300 + 10, frame.iY, 300, 60);
	infoFrame->setTitle("Movie Details:");
	infoFrame->setIconName(NEUTRINO_ICON_INFO);
	infoFrame->setActionKey(this, "RC_info");

	testFrame->addFrame(infoFrame);

	// play
	CFrame *playFrame = new CFrame();
	playFrame->setPosition(&frame);
	playFrame->setTitle("Movie abspielen");
	playFrame->setIconName(NEUTRINO_ICON_PLAY);
	playFrame->setActionKey(this, "playMovie");

	testFrame->addFrame(playFrame);

	widget->addItem(window);
	widget->addItem(testFrame);

	widget->exec(NULL, "");

	delete widget;
	widget = NULL;

	delete window;
	window = NULL;	
}

void CTSBrowser::playMovie(int i)
{
	if (&m_vMovieInfo[i].file != NULL) 
	{
		tmpMoviePlayerGui.addToPlaylist(m_vMovieInfo[i]);
		tmpMoviePlayerGui.exec(NULL, "");
	}
}

#define HEAD_BUTTONS_COUNT	2
const struct button_label HeadButtons[HEAD_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_HELP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_MUTE_SMALL, NONEXISTANT_LOCALE, NULL},
	//{ NEUTRINO_ICON_BUTTON_2, NONEXISTANT_LOCALE, NULL },
	//{ NEUTRINO_ICON_BUTTON_1, NONEXISTANT_LOCALE, NULL },
};

#define FOOT_BUTTONS_COUNT	4
const struct button_label FootButtons[FOOT_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_RED, LOCALE_TMDB_INFO, NULL },
	{ NEUTRINO_ICON_BUTTON_GREEN, LOCALE_AUDIOPLAYER_ADD, NULL },
	{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_AUDIOPLAYER_DELETEALL, NULL },
	{ NEUTRINO_ICON_BUTTON_BLUE, LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES, NULL },
};

void CTSBrowser::showMenu()
{
	mlist = new ClistBoxWidget("Movie Browser", NEUTRINO_ICON_MOVIE, w_max ( (CFrameBuffer::getInstance()->getScreenWidth() / 20 * 17), (CFrameBuffer::getInstance()->getScreenWidth() / 20 )), h_max ( (CFrameBuffer::getInstance()->getScreenHeight() / 20 * 17), (CFrameBuffer::getInstance()->getScreenHeight() / 20)));

	mlist->clearAll();

	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "mplay");

		item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");
	
		item->setHelpText(m_vMovieInfo[i].epgInfo1.empty() ? m_vMovieInfo[i].epgInfo2.c_str() : m_vMovieInfo[i].epgInfo1.c_str());

		mlist->addItem(item);
	}

	mlist->setMode(MODE_LISTBOX);
	mlist->setWidgetType(WIDGET_TYPE_FRAME);
	mlist->setItemsPerPage(6, 2);
	mlist->setSelected(selected);
	mlist->enablePaintDate();

	mlist->enablePaintFootInfo();

	mlist->setHeaderButtons(HeadButtons, HEAD_BUTTONS_COUNT);
	mlist->setFooterButtons(FootButtons, FOOT_BUTTONS_COUNT);

	mlist->addKey(RC_info, this, CRCInput::getSpecialKeyName(RC_info));
	mlist->addKey(RC_red, this, CRCInput::getSpecialKeyName(RC_red));
	mlist->addKey(RC_green, this, CRCInput::getSpecialKeyName(RC_green));
	mlist->addKey(RC_spkr, this, CRCInput::getSpecialKeyName(RC_spkr));
	mlist->addKey(RC_yellow, this, CRCInput::getSpecialKeyName(RC_yellow));
	mlist->addKey(RC_blue, this, CRCInput::getSpecialKeyName(RC_blue));

	mlist->exec(NULL, "");
	delete mlist;
	mlist = NULL;
}

int CTSBrowser::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "\nCTSBrowser::exec: actionKey:%s\n", actionKey.c_str());
	
	if(parent)
		hide();
	
	if(actionKey == "mplay")
	{
		selected = mlist->getSelected();

		funArt(selected);

		return RETURN_REPAINT;
	}
	if(actionKey == "playMovie")
	{
		selected = mlist->getSelected();

		playMovie(selected);

		return RETURN_REPAINT;
	}
	else if(actionKey == "RC_info")
	{
		selected = mlist->getSelected();
		m_movieInfo.showMovieInfo(m_vMovieInfo[mlist->getSelected()]);

		return RETURN_REPAINT;
	}
	else if(actionKey == "RC_red")
	{
		selected = mlist->getSelected();
		hide();
		doTMDB(m_vMovieInfo[mlist->getSelected()]);
		showMenu();

		return RETURN_EXIT_ALL;
	}
	else if(actionKey == "RC_green")
	{
		openFileBrowser();
		showMenu();

		return RETURN_EXIT_ALL;
	}
	else if (actionKey == "RC_spkr") 
	{
		if(m_vMovieInfo.size() > 0)
		{	
			if (&m_vMovieInfo[mlist->getSelected()].file != NULL) 
			{
			 	onDeleteFile(m_vMovieInfo[mlist->getSelected()]);
			}
		}

		showMenu();

		return RETURN_EXIT_ALL;
	}
	else if(actionKey == "RC_yellow")
	{
		m_vMovieInfo.clear();
		showMenu();

		return RETURN_EXIT_ALL;
	}
	else if(actionKey == "RC_blue")
	{
		loadPlaylist();
		selected = 0;
		showMenu();

		return RETURN_EXIT_ALL;
	}

	loadPlaylist();
	showMenu();
	
	return RETURN_EXIT;
}

void plugin_init(void)
{
	dprintf(DEBUG_NORMAL, "CTSBrowser: plugin_init\n");
}

void plugin_del(void)
{
	dprintf(DEBUG_NORMAL, "CTSBrowser: plugin_del\n");
}

void plugin_exec(void)
{
	CTSBrowser* movieBrowserHandler = new CTSBrowser();
	
	movieBrowserHandler->exec(NULL, "");
	
	delete movieBrowserHandler;
	movieBrowserHandler = NULL;
}


