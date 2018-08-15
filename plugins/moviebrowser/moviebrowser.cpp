/*
  $Id: moviebrowser.cpp 2018/07/10 mohousch Exp $

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

class CMBrowser : public CMenuTarget
{
	private:
		// variables
		CFrameBuffer* frameBuffer;
		int selected;

		//
		ClistBox* mlist;
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

	public:
		CMBrowser();
		~CMBrowser();
		int exec(CMenuTarget* parent, const std::string& actionKey);
		void hide();

		void showMenu();
};

CMBrowser::CMBrowser()
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

CMBrowser::~CMBrowser()
{
	m_vMovieInfo.clear();
}

void CMBrowser::hide()
{
	frameBuffer->paintBackground();
	frameBuffer->blit();
}

void CMBrowser::loadPlaylist()
{
	m_vMovieInfo.clear();

	// recordingdir
	Path = g_settings.network_nfs_recordingdir;
	
	//
	if(CFileHelpers::getInstance()->readDir(Path, &filelist, &fileFilter))
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

			std::string tmp_str = files->getFileName();

			removeExtension(tmp_str);

			// refill if empty
			if(movieInfo.epgTitle.empty())
				movieInfo.epgTitle = tmp_str;

			if(movieInfo.epgInfo1.empty())
				movieInfo.epgInfo1 = tmp_str;

			//if(movieInfo.epgInfo2.empty())
			//	movieInfo.epgInfo2 = tmp_str;

			//thumbnail
			std::string fname = "";
			fname = files->Name;
			changeFileNameExt(fname, ".jpg");
					
			if(!access(fname.c_str(), F_OK) )
				movieInfo.tfile = fname.c_str();
					
			// 
			m_vMovieInfo.push_back(movieInfo);
		}
	}
}

void CMBrowser::doTMDB(MI_MOVIE_INFO& movieFile)
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
		int pich = 246;	//FIXME
		int picw = 162; //FIXME
	
		std::string thumbnail = "";
	
		//
		std::string tname = tmdb->getThumbnailDir();
		tname += "/";
		tname += movieFile.epgTitle;
		tname += ".jpg";

		tmdb->getSmallCover(minfo_list[0].poster_path, tname);
		//
				
		if(!access(tname.c_str(), F_OK) )
			thumbnail = tname.c_str();
	
		CBox position(g_settings.screen_StartX + 50, g_settings.screen_StartY + 50, g_settings.screen_EndX - g_settings.screen_StartX - 100, g_settings.screen_EndY - g_settings.screen_StartY - 100); 
	
		CInfoBox * infoBox = new CInfoBox("", g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1], CTextBox::SCROLL, &position, "", g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE], NEUTRINO_ICON_TMDB);

		infoBox->setText(&buffer, thumbnail, picw, pich);
		infoBox->exec();
		delete infoBox;

		if(MessageBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_MOVIEBROWSER_PREFER_TMDB_INFO), CMessageBox::mbrNo, CMessageBox:: mbYes | CMessageBox::mbNo) == CMessageBox::mbrYes) 
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
		MessageBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_STREAMINFO_NOT_AVAILABLE), CMessageBox::mbrBack, CMessageBox::mbBack, NEUTRINO_ICON_INFO);
	}

	delete tmdb;
	tmdb = NULL;
}

bool CMBrowser::delFile(CFile& file)
{
	bool result = true;
	unlink(file.Name.c_str()); // fix: use full path
	dprintf(DEBUG_NORMAL, "CMBrowser::delete file: %s\r\n", file.Name.c_str());
	return(result);
}

void CMBrowser::onDeleteFile(MI_MOVIE_INFO& movieFile)
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

	if (MessageBox(LOCALE_FILEBROWSER_DELETE, msg, CMessageBox::mbrNo, CMessageBox::mbYes|CMessageBox::mbNo) == CMessageBox::mbrYes)
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

#define HEAD_BUTTONS_COUNT	2
const struct button_label HeadButtons[HEAD_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_HELP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_SETUP, NONEXISTANT_LOCALE, NULL },
};

void CMBrowser::showMenu()
{
	mlist = new ClistBox("Extended Movie Browser", NEUTRINO_ICON_MOVIE, w_max ( (CFrameBuffer::getInstance()->getScreenWidth() / 20 * 17), (CFrameBuffer::getInstance()->getScreenWidth() / 20 )), h_max ( (CFrameBuffer::getInstance()->getScreenHeight() / 20 * 17), (CFrameBuffer::getInstance()->getScreenHeight() / 20)));

	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, m_vMovieInfo[i].epgChannel.c_str(), this, "mplay", NULL, file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

		std::string tmp = m_vMovieInfo[i].epgTitle;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo1;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo2;

		item->setInfo1(tmp.c_str());

		//item->setOptionFont(g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]); //FIXME: menue.cpp

		mlist->addItem(item);
	}

	mlist->setWidgetType(WIDGET_FRAME);
	mlist->setItemsPerPage(6, 2);
	mlist->setItemBoxColor(COL_YELLOW);
	mlist->setSelected(selected);
	mlist->enablePaintDate();

	// widget
	mlist->addWidget(WIDGET_EXTENDED);
	mlist->addWidget(WIDGET_INFO);
	mlist->enableWidgetChange();

	mlist->setHeaderButtons(HeadButtons, HEAD_BUTTONS_COUNT);

	mlist->addKey(CRCInput::RC_info, this, CRCInput::getSpecialKeyName(CRCInput::RC_info));
	mlist->addKey(CRCInput::RC_red, this, CRCInput::getSpecialKeyName(CRCInput::RC_red));
	mlist->addKey(CRCInput::RC_spkr, this, CRCInput::getSpecialKeyName(CRCInput::RC_spkr));

	mlist->exec(NULL, "");
	//mlist->hide();
	delete mlist;
	mlist = NULL;
}

int CMBrowser::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "\nCMBrowser::exec: actionKey:%s\n", actionKey.c_str());
	
	if(parent)
		hide();
	
	if(actionKey == "mplay")
	{
		selected = mlist->getSelected();

		if (&m_vMovieInfo[mlist->getSelected()].file != NULL) 
		{
			tmpMoviePlayerGui.addToPlaylist(m_vMovieInfo[mlist->getSelected()]);
			tmpMoviePlayerGui.exec(NULL, "");
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "RC_info")
	{
		selected = mlist->getSelected();
		m_movieInfo.showMovieInfo(m_vMovieInfo[mlist->getSelected()]);

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "RC_red")
	{
		selected = mlist->getSelected();
		hide();
		doTMDB(m_vMovieInfo[mlist->getSelected()]);
		showMenu();
		return menu_return::RETURN_EXIT_ALL;
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

		loadPlaylist();
		showMenu();
		return menu_return::RETURN_EXIT_ALL;
	}

	loadPlaylist();
	showMenu();
	
	return menu_return::RETURN_EXIT;
}

void plugin_init(void)
{
	dprintf(DEBUG_NORMAL, "CMBrowser: plugin_init\n");
}

void plugin_del(void)
{
	dprintf(DEBUG_NORMAL, "CMBrowser: plugin_del\n");
}

void plugin_exec(void)
{
	CMBrowser* movieBrowserHandler = new CMBrowser();
	
	movieBrowserHandler->exec(NULL, "");
	
	delete movieBrowserHandler;
	movieBrowserHandler = NULL;
}


