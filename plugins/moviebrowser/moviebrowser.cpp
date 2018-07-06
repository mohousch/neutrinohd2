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

class CMBrowser : public CMenuTarget
{
	private:
		// variables
		CFrameBuffer* frameBuffer;
		int selected;

		//
		CMovieInfo m_movieInfo;
		std::vector<MI_MOVIE_INFO> m_vMovieInfo;
		ClistBox* mlist;

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
	selected = 0;
}

CMBrowser::~CMBrowser()
{
	m_vMovieInfo.clear();
}

void CMBrowser::hide()
{
	CFrameBuffer::getInstance()->paintBackground();
	CFrameBuffer::getInstance()->blit();
}

int CMBrowser::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "\nCMBrowser::exec: actionKey:%s\n", actionKey.c_str());
	
	if(parent)
		hide();
	
	if(actionKey == "mplay")
	{
		selected = mlist->getSelected();
		CMoviePlayerGui tmpMoviePlayerGui;

		if (&m_vMovieInfo[mlist->getSelected()].file != NULL) 
		{
			tmpMoviePlayerGui.addToPlaylist(m_vMovieInfo[mlist->getSelected()]);
			tmpMoviePlayerGui.exec(NULL, "urlplayback");
		}
	}
	else if(actionKey == "minfo")
	{
		selected = mlist->getSelected();
		m_movieInfo.showMovieInfo(m_vMovieInfo[mlist->getSelected()]);
	}
	else if(actionKey == "msetup")
	{
		mlist->hide();

		if(mlist->getWidgetType() == WIDGET_EXTENDED)
			mlist->setWidgetType(WIDGET_FRAME);
		else if(mlist->getWidgetType() == WIDGET_FRAME)
			mlist->setWidgetType(WIDGET_EXTENDED);

		mlist->initFrames();
		mlist->paint();
		mlist->paintHead();
		mlist->paintFoot();
	}
	
	return menu_return::RETURN_REPAINT;
}

#define mHEAD_BUTTONS_COUNT	2
const struct button_label mHeadButtons[mHEAD_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_HELP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_SETUP, NONEXISTANT_LOCALE, NULL },
};

void CMBrowser::showMenu()
{
	mlist = new ClistBox("Movie Browser", NEUTRINO_ICON_MOVIE, w_max ( (CFrameBuffer::getInstance()->getScreenWidth() / 20 * 17), (CFrameBuffer::getInstance()->getScreenWidth() / 20 )), h_max ( (CFrameBuffer::getInstance()->getScreenHeight() / 20 * 17), (CFrameBuffer::getInstance()->getScreenHeight() / 20)));

	CMenuItem* mm;
	
	//
	CFileFilter fileFilter;
	
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

	CFileList filelist;
	
	// recordingdir
	std::string Path_local = g_settings.network_nfs_recordingdir;
	
	//
	if(CFileHelpers::getInstance()->readDir(Path_local, &filelist, &fileFilter))
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

	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		mm = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, m_vMovieInfo[i].epgChannel.c_str(), this, "mplay", NULL, file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

		mm->setInfo1(m_vMovieInfo[i].epgInfo2.c_str());

		mlist->addItem(mm);
	}


	mlist->setWidgetType(WIDGET_FRAME);
	mlist->setItemsPerPage(6, 2);
	mlist->setItemBoxColor(COL_YELLOW);

	mlist->setSelected(selected);

	mlist->setHeaderButtons(mHeadButtons, mHEAD_BUTTONS_COUNT);

	mlist->enablePaintDate();

	mlist->addKey(CRCInput::RC_info, this, "minfo");
	mlist->addKey(CRCInput::RC_setup, this, "msetup");

	mlist->exec(NULL, "");
	mlist->hide();
	delete mlist;
	mlist = NULL;
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
	CMBrowser* movieBrowserHandler = new CMBrowser();
	
	movieBrowserHandler->showMenu();
	
	delete movieBrowserHandler;
	movieBrowserHandler = NULL;
}


