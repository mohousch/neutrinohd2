/*
  $Id: nfilm.cpp 2018/07/31 mohousch Exp $

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

#include <fstream>
#include <jsoncpp/include/json/json.h>
#include <system/ytparser.h>


extern "C" void plugin_exec(void);
extern "C" void plugin_init(void);
extern "C" void plugin_del(void);

class CNFilm : public CMenuTarget
{
	private:
		// variables
		CFrameBuffer* frameBuffer;
		int selected;

		//
		ClistBox* mlist;
		CMenuItem* item;

		//
		CTmdb* tmdb;
		std::string thumbnail_dir;
		CFileHelpers fileHelper;
		cYTFeedParser ytparser;

		CMovieInfo m_movieInfo;
		std::vector<MI_MOVIE_INFO> m_vMovieInfo;
		std::vector<MI_MOVIE_INFO> listMovie;

		std::string plist;
		unsigned int page;
		
		CMoviePlayerGui tmpMoviePlayerGui;

		void loadPlaylist();
		void createThumbnailDir();
		void removeThumbnailDir();

	public:
		CNFilm();
		~CNFilm();
		int exec(CMenuTarget* parent, const std::string& actionKey);
		void hide();

		void showMovies();
		void showMenu();
};

CNFilm::CNFilm()
{
	frameBuffer = CFrameBuffer::getInstance();

	//
	mlist = NULL;
	item = NULL;

	tmdb = NULL;
	thumbnail_dir = "/tmp/nfilm";
	fileHelper.createDir(thumbnail_dir.c_str(), 0755);

	//
	selected = 0;

	plist = "now_playing";
	page = 1;
}

CNFilm::~CNFilm()
{
	m_vMovieInfo.clear();
	fileHelper.removeDir(thumbnail_dir.c_str());
}

void CNFilm::hide()
{
	frameBuffer->paintBackground();
	frameBuffer->blit();
}

void CNFilm::createThumbnailDir()
{
	fileHelper.createDir(thumbnail_dir.c_str(), 0755);
}

void CNFilm::removeThumbnailDir()
{
	fileHelper.removeDir(thumbnail_dir.c_str());
}

void CNFilm::loadPlaylist()
{
	m_vMovieInfo.clear();
	listMovie.clear();

	removeThumbnailDir();
	createThumbnailDir();

	CHintBox loadBox("Kino Trailer", g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
	loadBox.paint();

	//
	tmdb = new CTmdb();

	tmdb->cleanUp();
	tmdb->getMovieList(plist, page);

	std::vector<tmdbinfo> &mvlist = tmdb->getList();
	
	for (unsigned int count = 0; count < mvlist.size(); count++) 
	{
		MI_MOVIE_INFO Info;
		m_movieInfo.clearMovieInfo(&Info); // refresh structure
		
		Info.epgTitle = mvlist[count].title;
		
		listMovie.push_back(Info);
	}

	for (unsigned int i = 0; i < listMovie.size(); i++)
	{
		MI_MOVIE_INFO movieInfo;
		m_movieInfo.clearMovieInfo(&movieInfo); // refresh structure

		movieInfo.epgTitle = listMovie[i].epgTitle;

		// load infos from tmdb
		tmdb->getMovieInfo(movieInfo.epgTitle, false, "search");

		if ((tmdb->getResults() > 0) && (!tmdb->getDescription().empty())) 
		{
			movieInfo.epgInfo1 = tmdb->getDescription();
			movieInfo.epgInfo2 = tmdb->createInfoText();
			movieInfo.ytdate = tmdb->getReleaseDate();
			
			std::string tname = thumbnail_dir;
			tname += "/";
			tname += tmdb->getTitle();
			tname += ".jpg";

			tmdb->getSmallCover(tname);

			if(!tname.empty())
				movieInfo.tfile = tname;

			//file.name extract from youtube
			std::string search_string;
			ytparser.Cleanup();

			// setregion
			ytparser.SetRegion("DE");

			// set max result
			ytparser.SetMaxResults(1);

			search_string = movieInfo.epgTitle;
			
			// parse feed
			if (ytparser.ParseFeed(cYTFeedParser::SEARCH, search_string))
			{
				yt_video_list_t &ylist = ytparser.GetVideoList();
	
				for (unsigned int j = 0; j < ylist.size(); j++) 
				{
					movieInfo.ytid = ylist[j].id;
					movieInfo.file.Name = ylist[j].GetUrl();
				}
			} 
		}
					
		// 
		m_vMovieInfo.push_back(movieInfo);
	}

	delete tmdb;
	tmdb = NULL;

	loadBox.hide();
}

#define HEAD_BUTTONS_COUNT	4
const struct button_label HeadButtons[HEAD_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_HELP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_SETUP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_YELLOW, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_GREEN, NONEXISTANT_LOCALE, NULL }
};

void CNFilm::showMovies()
{
	std::string caption = "Kino Trailer (" + plist + ")";
	mlist = new ClistBox(caption.c_str(), NEUTRINO_ICON_MOVIE, w_max ( (CFrameBuffer::getInstance()->getScreenWidth() / 20 * 17), (CFrameBuffer::getInstance()->getScreenWidth() / 20 )), h_max ( (CFrameBuffer::getInstance()->getScreenHeight() / 20 * 17), (CFrameBuffer::getInstance()->getScreenHeight() / 20)));
	
	
	// load playlist
	loadPlaylist();

	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		std::string tmp = m_vMovieInfo[i].ytdate;
		tmp += " ";
		tmp += m_vMovieInfo[i].epgInfo1;

		item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, tmp.c_str(), this, "mplay", NULL, file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

		item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

		mlist->addItem(item);
	}


	mlist->setWidgetType(WIDGET_FRAME);
	mlist->setItemsPerPage(6, 2);
	mlist->setItemBoxColor(COL_YELLOW);
	mlist->setSelected(selected);
	mlist->enablePaintDate();

	// widget
	mlist->addWidget(WIDGET_EXTENDED);
	mlist->enableWidgetChange();

	mlist->setHeaderButtons(HeadButtons, HEAD_BUTTONS_COUNT);

	mlist->addKey(CRCInput::RC_info, this, CRCInput::getSpecialKeyName(CRCInput::RC_info));
	mlist->addKey(CRCInput::RC_setup, this, CRCInput::getSpecialKeyName(CRCInput::RC_setup));
	mlist->addKey(CRCInput::RC_green, this, CRCInput::getSpecialKeyName(CRCInput::RC_green));
	mlist->addKey(CRCInput::RC_yellow, this, CRCInput::getSpecialKeyName(CRCInput::RC_yellow));

	mlist->exec(NULL, "");
	//mlist->hide();
	delete mlist;
	mlist = NULL;
}

void CNFilm::showMenu()
{
	CMenuWidget* menu = new CMenuWidget("Kino Trailer");

	menu->addItem(new CMenuForwarder("Now Playing", true, NULL, this, "now_playing"));
	menu->addItem(new CMenuForwarder("Most Popular", true, NULL, this, "popular"));
	menu->addItem(new CMenuForwarder("Top rated", true, NULL, this, "top_rated"));
	menu->addItem(new CMenuForwarder("Up coming", true, NULL, this, "upcoming"));

	menu->exec(NULL, "");
	menu->hide();
	delete menu;
	menu = NULL;
}

int CNFilm::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CNFilm::exec: actionKey: %s\n", actionKey.c_str());

	if(parent)
		hide();

	if(actionKey == "RC_info")
	{
		selected = mlist->getSelected();
		m_movieInfo.showMovieInfo(m_vMovieInfo[mlist->getSelected()]);

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "mplay")
	{
		selected = mlist->getSelected();
		
		if (&m_vMovieInfo[selected].file != NULL) 
		{
			tmpMoviePlayerGui.addToPlaylist(m_vMovieInfo[selected]);
			tmpMoviePlayerGui.exec(NULL, "");
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "RC_setup")
	{
		showMenu();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "now_playing")
	{
		mlist->clearItems();
		selected = 0;
		page = 1;
		plist = "now_playing";
		showMovies();

		return menu_return::RETURN_EXIT;
	}
	else if(actionKey == "popular")
	{
		mlist->clearItems();
		selected = 0;
		page = 1;
		plist = "popular";
		showMovies();

		return menu_return::RETURN_EXIT;
	}
	else if(actionKey == "top_rated")
	{
		mlist->clearItems();
		selected = 0;
		page = 1;
		plist = "top_rated";
		showMovies();

		return menu_return::RETURN_EXIT;
	}
	else if(actionKey == "upcoming")
	{
		mlist->clearItems();
		selected = 0;
		page = 1;
		plist = "upcoming";
		showMovies();

		return menu_return::RETURN_EXIT;
	}
	else if(actionKey == "RC_green")
	{
		page++;
		selected = 0;
		showMovies();

		return menu_return::RETURN_EXIT;
	}
	else if(actionKey == "RC_yellow")
	{
		page--;

		if(page <= 1)
			page = 1;

		selected = 0;
		showMovies();

		return menu_return::RETURN_EXIT;
	}

	showMovies();

	return menu_return::RETURN_REPAINT;
}

void plugin_init(void)
{
	dprintf(DEBUG_NORMAL, "CNFilm: plugin_init\n");
}

void plugin_del(void)
{
	dprintf(DEBUG_NORMAL, "CNFilm: plugin_del\n");
}

void plugin_exec(void)
{
	CNFilm* nFilmHandler = new CNFilm();
	
	nFilmHandler->exec(NULL, "");
	
	delete nFilmHandler;
	nFilmHandler = NULL;
}


