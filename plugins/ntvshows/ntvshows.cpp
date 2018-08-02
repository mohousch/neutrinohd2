/*
  $Id: ntvshows.cpp 2018/08/02 mohousch Exp $

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

class CTVShows : public CMenuTarget
{
	private:
		// variables
		CFrameBuffer* frameBuffer;
		int selected;

		//
		ClistBox* mlist;
		CMenuItem* item;

		std::string caption;

		//
		CTmdb* tmdb;
		std::string thumbnail_dir;
		CFileHelpers fileHelper;
		cYTFeedParser ytparser;

		CMovieInfo m_movieInfo;
		std::vector<MI_MOVIE_INFO> m_vMovieInfo;
		std::vector<MI_MOVIE_INFO> list;

		//
		std::vector<tmdbinfo> genres;

		std::string plist;
		unsigned int page;
		int list_id;
		
		CMoviePlayerGui tmpMoviePlayerGui;

		void loadMoviesTitle(void);
		void loadGenreMoviesTitle(void);

		void loadPlaylist();
		void createThumbnailDir();
		void removeThumbnailDir();
		void showMovieInfo(MI_MOVIE_INFO& movie);

	public:
		CTVShows();
		~CTVShows();
		int exec(CMenuTarget* parent, const std::string& actionKey);
		void hide();

		void showMovies();
		void showMenu();
};

CTVShows::CTVShows()
{
	frameBuffer = CFrameBuffer::getInstance();

	//
	mlist = NULL;
	item = NULL;

	tmdb = NULL;
	thumbnail_dir = "/tmp/ntvshows";
	fileHelper.createDir(thumbnail_dir.c_str(), 0755);

	//
	selected = 0;

	plist = "popular";
	page = 1;
	list_id = 0;

	caption = "Serien Trailer (";
}

CTVShows::~CTVShows()
{
	m_vMovieInfo.clear();
	fileHelper.removeDir(thumbnail_dir.c_str());

	list.clear();
	genres.clear();
}

void CTVShows::hide()
{
	frameBuffer->paintBackground();
	frameBuffer->blit();
}

void CTVShows::createThumbnailDir()
{
	fileHelper.createDir(thumbnail_dir.c_str(), 0755);
}

void CTVShows::removeThumbnailDir()
{
	fileHelper.removeDir(thumbnail_dir.c_str());
}

void CTVShows::loadMoviesTitle(void)
{
	list.clear();

	removeThumbnailDir();
	createThumbnailDir();

	CHintBox loadBox("Serien Trailer", g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
	loadBox.paint();

	//
	tmdb = new CTmdb();

	tmdb->clearMovieList();

	tmdb->getMovieList("tv", plist, page);

	std::vector<tmdbinfo> &mvlist = tmdb->getMovies();
	
	for (unsigned int count = 0; count < mvlist.size(); count++) 
	{
		MI_MOVIE_INFO Info;
		m_movieInfo.clearMovieInfo(&Info);
		
		Info.epgTitle = mvlist[count].title;
		
		list.push_back(Info);
	}
	
	delete tmdb;
	tmdb = NULL;
}

void CTVShows::loadGenreMoviesTitle()
{
	list.clear();

	removeThumbnailDir();
	createThumbnailDir();

	CHintBox loadBox("Serien Trailer", g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
	loadBox.paint();

	//
	tmdb = new CTmdb();

	tmdb->clearGenreMovieList();
	tmdb->getGenreMovieList(list_id);

	std::vector<tmdbinfo> &mglist = tmdb->getGenreMovies();
	
	for (unsigned int count = 0; count < mglist.size(); count++) 
	{
		MI_MOVIE_INFO Info;
		m_movieInfo.clearMovieInfo(&Info);
		
		Info.epgTitle = mglist[count].title;
		
		list.push_back(Info);
	}

	delete tmdb;
	tmdb = NULL;
}

void CTVShows::loadPlaylist()
{
	m_vMovieInfo.clear();

	//
	tmdb = new CTmdb();
	
	// fill our structure
	for (unsigned int i = 0; i < list.size(); i++)
	{
		MI_MOVIE_INFO movieInfo;
		m_movieInfo.clearMovieInfo(&movieInfo); 

		movieInfo.epgTitle = list[i].epgTitle;

		// load infos from tmdb
		tmdb->getMovieInfo(movieInfo.epgTitle, false);

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
			ytparser.Cleanup();

			// setregion
			ytparser.SetRegion("DE");

			// set max result
			ytparser.SetMaxResults(1);
			
			// parse feed
			if (ytparser.ParseFeed(cYTFeedParser::SEARCH, tmdb->getVName()))
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
}

void CTVShows::showMovieInfo(MI_MOVIE_INFO& movie)
{
	std::string buffer;
	
	// prepare print buffer  
	buffer += movie.epgInfo2;

	// thumbnail
	int pich = 246;	//FIXME
	int picw = 162; 	//FIXME

	std::string thumbnail = movie.tfile;
	if(access(thumbnail.c_str(), F_OK))
		thumbnail = "";
	
	CBox position(g_settings.screen_StartX + 50, g_settings.screen_StartY + 50, g_settings.screen_EndX - g_settings.screen_StartX - 100, g_settings.screen_EndY - g_settings.screen_StartY - 100); 
	
	CInfoBox * infoBox = new CInfoBox("", g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1], CTextBox::SCROLL, &position, movie.epgTitle.c_str(), g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE], NEUTRINO_ICON_MOVIE);

	infoBox->setText(&buffer, thumbnail, picw, pich);
	infoBox->exec();
	delete infoBox;
}

#define HEAD_BUTTONS_COUNT	4
const struct button_label HeadButtons[HEAD_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_HELP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_SETUP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_YELLOW, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_GREEN, NONEXISTANT_LOCALE, NULL }
};

void CTVShows::showMovies()
{
	mlist = new ClistBox(caption.c_str(), NEUTRINO_ICON_MOVIE, w_max ( (CFrameBuffer::getInstance()->getScreenWidth() / 20 * 17), (CFrameBuffer::getInstance()->getScreenWidth() / 20 )), h_max ( (CFrameBuffer::getInstance()->getScreenHeight() / 20 * 17), (CFrameBuffer::getInstance()->getScreenHeight() / 20)));
	
	
	// load playlist
	loadPlaylist();

	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		std::string tmp = m_vMovieInfo[i].ytdate;
		tmp += " ";
		tmp += m_vMovieInfo[i].epgInfo1;

		item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, tmp.c_str(), this, "season", NULL, file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

		item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

		mlist->addItem(item);
	}


	mlist->setWidgetType(WIDGET_FRAME);
	mlist->setItemsPerPage(6, 2);
	mlist->setItemBoxColor(COL_YELLOW);
	mlist->setSelected(selected);
	mlist->enablePaintDate();

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

void CTVShows::showMenu()
{
	CMenuWidget* menu = new CMenuWidget("Serien Trailer");

	menu->addItem(new CMenuForwarder("Airing today", true, NULL, this, "airing_today"));
	menu->addItem(new CMenuForwarder("On the air", true, NULL, this, "on_the_air"));
	menu->addItem(new CMenuForwarder("popular", true, NULL, this, "popular"));
	menu->addItem(new CMenuForwarder("top rated", true, NULL, this, "top_rated"));

	// genres
/*
	menu->addItem(new CMenuSeparator(CMenuSeparator::LINE));

	genres.clear();
	tmdb = new CTmdb();
	tmdb->clearGenreList();
	tmdb->getGenreList();

	std::vector<tmdbinfo> &mgenrelist = tmdb->getGenres();
	
	for (unsigned int count = 0; count < mgenrelist.size(); count++) 
	{
		tmdbinfo tmp;

		tmp.title = mgenrelist[count].title;
		tmp.id = mgenrelist[count].id;

		genres.push_back(tmp);

		menu->addItem(new CMenuForwarder(mgenrelist[count].title.c_str(), true, NULL, this, to_string(mgenrelist[count].id).c_str()));
	}

	delete tmdb;
	tmdb = NULL;
*/

	menu->exec(NULL, "");
	menu->hide();
	delete menu;
	menu = NULL;
}

int CTVShows::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CTVShows::exec: actionKey: %s\n", actionKey.c_str());

	caption.clear();

	caption = "Serien Trailer (";

	if(parent)
		hide();

	if(actionKey == "RC_info")
	{
		selected = mlist->getSelected();

		showMovieInfo(m_vMovieInfo[selected]);

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
	else if(actionKey == "airing_today")
	{
		mlist->clearItems();
		selected = 0;
		page = 1;
		plist = "airing_today";
		loadMoviesTitle();
		caption += "Airing today)";
		showMovies();

		return menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey == "on_the_air")
	{
		mlist->clearItems();
		selected = 0;
		page = 1;
		plist = "on_the_air";
		loadMoviesTitle();
		caption += "on the air)";
		showMovies();

		return menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey == "popular")
	{
		mlist->clearItems();
		selected = 0;
		page = 1;
		plist = "popular";
		loadMoviesTitle();
		caption += "am populärsten)";
		showMovies();

		return menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey == "top_rated")
	{
		mlist->clearItems();
		selected = 0;
		page = 1;
		plist = "top_rated";
		loadMoviesTitle();
		caption += "am meisten bewertet)";
		showMovies();

		return menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey == "RC_green")
	{
		page++;
		selected = 0;
		loadMoviesTitle();
		showMovies();

		return menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey == "RC_yellow")
	{
		page--;

		if(page <= 1)
			page = 1;

		selected = 0;
		loadMoviesTitle();
		showMovies();

		return menu_return::RETURN_EXIT_ALL;
	}
	else if(!actionKey.empty())
	{
		mlist->clearItems();
		selected = 0;
		list_id = atoi(actionKey.c_str());
		loadGenreMoviesTitle();

		for(unsigned int i = 0; i < genres.size(); i++)
		{
			if(genres[i].id == list_id)
			{
				caption += genres[i].title + ")";
			}
		}
		showMovies();

		return menu_return::RETURN_EXIT_ALL;
	}

	loadMoviesTitle();
	caption += "in den Kinos)";
	showMovies();

	return menu_return::RETURN_REPAINT;
}

void plugin_init(void)
{
	dprintf(DEBUG_NORMAL, "CTVShows: plugin_init\n");
}

void plugin_del(void)
{
	dprintf(DEBUG_NORMAL, "CTVShows: plugin_del\n");
}

void plugin_exec(void)
{
	CTVShows* nTVShowsHandler = new CTVShows();
	
	nTVShowsHandler->exec(NULL, "");
	
	delete nTVShowsHandler;
	nTVShowsHandler = NULL;
}


