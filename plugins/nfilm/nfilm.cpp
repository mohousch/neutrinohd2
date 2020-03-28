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

#include <system/ytparser.h>


extern "C" void plugin_exec(void);
extern "C" void plugin_init(void);
extern "C" void plugin_del(void);

class CNFilm : public CMenuTarget
{
	public:
		enum{
			WIDGET_LEFT = 1,
			WIDGET_RIGHT
		};

	private:
		// variables
		CFrameBuffer* frameBuffer;
		
		CBox mainBox;
		CBox headBox;
		CBox footBox;
		CBox leftBox;
		CBox rightBox;

		CHeaders *headersWidget;
		CWidget *mainWidget;
		ClistBox *leftWidget;
		ClistBox *rightWidget;
		CFooters *footersWidget;

		CMenuItem* item;

		//std::string caption;

		int selected;
		int left_selected;
		int right_selected;
		int focus;

		//
		CTmdb* tmdb;
		std::string thumbnail_dir;
		CFileHelpers fileHelper;
		cYTFeedParser ytparser;

		int genre_id;
		std::string plist;
		int page;
		std::string TVShows;

		//
		CMovieInfo m_movieInfo;
		std::vector<MI_MOVIE_INFO> m_vMovieInfo;

		std::vector<tmdbinfo> genres;
		std::vector<tmdbinfo> mvlist;
		std::vector<tmdbinfo>movieInfo_list; 
		std::string tmdbsearch;
		
		CMoviePlayerGui tmpMoviePlayerGui;

		void loadGenreMoviesTitle(void);
		void loadTMDBPlaylist(const char *txt = "movie", const char *list = "popular", const int seite = 1, bool search = false);

		void createThumbnailDir();
		void removeThumbnailDir();
		void showMovieInfo(MI_MOVIE_INFO& movie);
		void getMovieVideoUrl(MI_MOVIE_INFO& movie);

		int showCategoriesMenu();

		void showMenu();

	public:
		CNFilm(std::string movielist = "popular", int id = 0);
		~CNFilm();
		int exec(CMenuTarget* parent, const std::string& actionKey);
		void hide();
};

CNFilm::CNFilm(std::string movielist, int id)
{
	frameBuffer = CFrameBuffer::getInstance();

	//
	mainWidget = NULL;
	leftWidget = NULL;
	rightWidget = NULL;
	headersWidget = NULL;
	footersWidget = NULL;

	item = NULL;

	tmdb = NULL;
	thumbnail_dir = "/tmp/nfilm";
	fileHelper.createDir(thumbnail_dir.c_str(), 0755);

	plist = "popular";
	page = 1;
	TVShows = "movies";

	//
	selected = 0;
	left_selected = 0;
	right_selected = 0;

	focus = WIDGET_LEFT;

	plist = movielist;
	genre_id = id;
	page = 1;
}

CNFilm::~CNFilm()
{
	m_vMovieInfo.clear();
	fileHelper.removeDir(thumbnail_dir.c_str());

	mvlist.clear();
	genres.clear();
	movieInfo_list.clear();
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

void CNFilm::loadGenreMoviesTitle()
{
	dprintf(DEBUG_NORMAL, "CNFilm::loadGenreMoviesTitle:\n");

	m_vMovieInfo.clear();

	removeThumbnailDir();
	createThumbnailDir();

	CHintBox loadBox("Movie Trailer", g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
	loadBox.paint();

	//
	tmdb = new CTmdb();

	tmdb->clearGenreMovieList();
	tmdb->getGenreMovieList(genre_id);

	std::vector<tmdbinfo> &mglist = tmdb->getGenreMovies();
	
	for (unsigned int i = 0; i < mglist.size(); i++) 
	{
		MI_MOVIE_INFO movieInfo;
		m_movieInfo.clearMovieInfo(&movieInfo);
		
		movieInfo.epgTitle = mglist[i].title;
		
		movieInfo.epgInfo1 = mglist[i].overview;
		movieInfo.ytdate = mglist[i].release_date;
		movieInfo.vote_average = mglist[i].vote_average;
		movieInfo.vote_count = mglist[i].vote_count;
		movieInfo.original_title = mglist[i].original_title;
		movieInfo.release_date = mglist[i].release_date;
		movieInfo.media_type = mglist[i].media_type;
		movieInfo.length = mglist[i].runtime;
		movieInfo.runtimes = mglist[i].runtimes;
		movieInfo.genres = mglist[i].genres;
		movieInfo.cast = mglist[i].cast;
		movieInfo.seasons = mglist[i].seasons;
		movieInfo.episodes = mglist[i].episodes;
			
		std::string tname = thumbnail_dir;
		tname += "/";
		tname += movieInfo.epgTitle;
		tname += ".jpg";

		tmdb->getSmallCover(mglist[i].poster_path, tname);

		if(!tname.empty())
			movieInfo.tfile = tname;

		// video url (the first one)
		tmdb->clearVideoInfo();
		tmdb->getVideoInfo("movie", mglist[i].id);

		std::vector<tmdbinfo>& videoInfo_list = tmdb->getVideoInfos();

		movieInfo.vid = videoInfo_list[0].vid;
		movieInfo.vkey = videoInfo_list[0].vkey;
		movieInfo.vname = videoInfo_list[0].vname;
		
		m_vMovieInfo.push_back(movieInfo);
	}

	delete tmdb;
	tmdb = NULL;
}

void CNFilm::loadTMDBPlaylist(const char *txt, const char *list, const int seite, bool search)
{
	dprintf(DEBUG_NORMAL, "CNFilm::loadPlaylist:\n");

	thumbnail_dir = "/tmp/nfilm";
	page = seite;
	plist = list;
	TVShows = txt;

	//
	tmdb = new CTmdb();
	mvlist.clear();

	fileHelper.removeDir(thumbnail_dir.c_str());
	fileHelper.createDir(thumbnail_dir.c_str(), 0755);

	CHintBox loadBox("Movie Trailer", g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
	loadBox.paint();

	if(search)
	{
		tmdb->clearMInfo();
		tmdb->searchMovieInfo(txt);
		mvlist = tmdb->getMInfos();
	}
	else
	{
		tmdb->clearMovieList();

		tmdb->getMovieTVList(TVShows, plist, page);

		mvlist = tmdb->getMovies();
	}

	m_vMovieInfo.clear();
	
	// fill our structure
	for(unsigned int i = 0; i < mvlist.size(); i++)
	{
		MI_MOVIE_INFO movieInfo;
		m_movieInfo.clearMovieInfo(&movieInfo);

		movieInfo.epgTitle = mvlist[i].title;

		tmdb->clearMovieInfo();

		// refill with detailed infos
		if(search)
			tmdb->getMovieTVInfo(mvlist[i].media_type, mvlist[i].id);
		else
			tmdb->getMovieTVInfo(TVShows, mvlist[i].id);

		movieInfo_list = tmdb->getMovieInfos();
		
		movieInfo.epgInfo1 = movieInfo_list[0].overview;
		movieInfo.ytdate = movieInfo_list[0].release_date;
		movieInfo.vote_average = movieInfo_list[0].vote_average;
		movieInfo.vote_count = movieInfo_list[0].vote_count;
		movieInfo.original_title = movieInfo_list[0].original_title;
		movieInfo.release_date = movieInfo_list[0].release_date;
		movieInfo.media_type = movieInfo_list[0].media_type;
		movieInfo.length = movieInfo_list[0].runtime;
		movieInfo.runtimes = movieInfo_list[0].runtimes;
		movieInfo.genres = movieInfo_list[0].genres;
		movieInfo.cast = movieInfo_list[0].cast;
		movieInfo.seasons = movieInfo_list[0].seasons;
		movieInfo.episodes = movieInfo_list[0].episodes;
			
		std::string tname = thumbnail_dir;
		tname += "/";
		tname += movieInfo.epgTitle;
		tname += ".jpg";

		tmdb->getSmallCover(mvlist[i].poster_path, tname);

		if(!tname.empty())
			movieInfo.tfile = tname;

		// video url (the first one)
		tmdb->clearVideoInfo();
		tmdb->getVideoInfo("movie", mvlist[i].id);

		std::vector<tmdbinfo>& videoInfo_list = tmdb->getVideoInfos();

		movieInfo.vid = videoInfo_list[0].vid;
		movieInfo.vkey = videoInfo_list[0].vkey;
		movieInfo.vname = videoInfo_list[0].vname;

		m_vMovieInfo.push_back(movieInfo);
	}

	loadBox.hide();
}

void CNFilm::showMovieInfo(MI_MOVIE_INFO& movie)
{
	dprintf(DEBUG_NORMAL, "CNFilm::showMovieInfo:\n");

	m_movieInfo.showMovieInfo(movie);
}

void CNFilm::getMovieVideoUrl(MI_MOVIE_INFO& movie)
{
	ytparser.Cleanup();

	// setregion
	ytparser.SetRegion("DE");

	// set max result
	ytparser.SetMaxResults(1);
			
	// parse feed
	if (ytparser.ParseFeed(cYTFeedParser::SEARCH_BY_ID, movie.vname, movie.vkey))
	{
		yt_video_list_t &ylist = ytparser.GetVideoList();
	
		for (unsigned int j = 0; j < ylist.size(); j++) 
		{
			movie.ytid = ylist[j].id;
			movie.file.Name = ylist[j].GetUrl();
		}
	} 
}

int CNFilm::showCategoriesMenu()
{
	dprintf(DEBUG_NORMAL, "CNFilm::showCategoriesMenu:\n");

	int res = -1;

	ClistBoxWidget * menu = new ClistBoxWidget("Movie Trailer");

	menu->setMode(MODE_MENU);
	menu->enableShrinkMenu();

	genres.clear();
	tmdb = new CTmdb();
	tmdb->clearGenreList();
	tmdb->getGenreList();

	std::vector<tmdbinfo> &mgenrelist = tmdb->getGenres();
	
	for (unsigned int count = 0; count < mgenrelist.size(); count++) 
	{
		// fill genres for callback
		tmdbinfo tmp;

		tmp.title = mgenrelist[count].title;
		tmp.id = mgenrelist[count].id;

		genres.push_back(tmp);

		menu->addItem(new CMenuForwarder(mgenrelist[count].title.c_str()));
	}

	delete tmdb;
	tmdb = NULL;

	menu->exec(NULL, "");
	menu->hide();
	res = menu->getSelected();

	genre_id = genres[res].id;
	delete menu;
	menu = NULL;

	return res;
}

#define HEAD_BUTTONS_COUNT	2
const struct button_label HeadButtons[HEAD_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_HELP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_SETUP, NONEXISTANT_LOCALE, NULL }
};

#define FOOT_BUTTONS_COUNT	4
const struct button_label FootButtons[FOOT_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_RED, LOCALE_YT_NEXT_RESULTS, NULL },
	{ NEUTRINO_ICON_BUTTON_GREEN, LOCALE_YT_PREV_RESULTS, NULL },
	{ NEUTRINO_ICON_BUTTON_YELLOW, NONEXISTANT_LOCALE, "Focus" },
	{ NEUTRINO_ICON_BUTTON_BLUE, NONEXISTANT_LOCALE, NULL }
};

void CNFilm::showMenu()
{
	dprintf(DEBUG_NORMAL, "CNFilm::showMenu:\n");

	mainWidget = new CWidget(frameBuffer->getScreenX(), frameBuffer->getScreenY(), frameBuffer->getScreenWidth(), frameBuffer->getScreenHeight());

	mainWidget->enableSaveScreen();
	mainWidget->setSelected(selected);
	mainWidget->setBackgroundColor(COL_DARK_TURQUOISE);
	mainWidget->enablePaintMainFrame();

	// headwidget
	headBox.iWidth = frameBuffer->getScreenWidth();
	headBox.iHeight = 40;
	headBox.iX = frameBuffer->getScreenX();
	headBox.iY = frameBuffer->getScreenY();

	headersWidget = new CHeaders(headBox.iX, headBox.iY, headBox.iWidth, headBox.iHeight, "Movie Trailer", NEUTRINO_ICON_MP3);

	headersWidget->setButtons(HeadButtons, HEAD_BUTTONS_COUNT);
	headersWidget->enablePaintDate();

	// foot
	footBox.iWidth = frameBuffer->getScreenWidth();
	footBox.iHeight = 40;
	footBox.iX = frameBuffer->getScreenX();
	footBox.iY = frameBuffer->getScreenY() + frameBuffer->getScreenHeight() - footBox.iHeight;

	footersWidget = new CFooters(footBox.iX, footBox.iY, footBox.iWidth, footBox.iHeight, FOOT_BUTTONS_COUNT, FootButtons);

	footersWidget->setCorner(RADIUS_MID, CORNER_BOTTOM);

	// leftwidget
	leftBox.iWidth = 200;
	leftBox.iHeight = mainWidget->getWindowsPos().iHeight - headBox.iHeight - 2*INTER_FRAME_SPACE - footBox.iHeight;
	leftBox.iX = mainWidget->getWindowsPos().iX;
	leftBox.iY = mainWidget->getWindowsPos().iY + headBox.iHeight + INTER_FRAME_SPACE;

	leftWidget = new ClistBox(&leftBox);

	leftWidget->disableCenter();
	leftWidget->setSelected(left_selected);
	leftWidget->disableShrinkMenu();

	ClistBoxItem *item1 = new ClistBoxItem("In den Kinos", true, NULL, this, "movie_in_cinema");
	ClistBoxItem *item2 = new ClistBoxItem("Am", true, NULL, this, "movie_popular");
	item2->setOption("populärsten");
	item2->set2lines();
	ClistBoxItem *item3 = new ClistBoxItem("Am besten", true, NULL, this, "movie_top_rated");
	item3->setOption("bewertet");
	item3->set2lines();
	ClistBoxItem *item4 = new ClistBoxItem("Neue Filme", true, NULL, this, "movie_new");
	
	CMenuSeparator *item5 = new CMenuSeparator();
	CMenuSeparator *item6 = new CMenuSeparator();
	CMenuSeparator *item7 = new CMenuSeparator();
	CMenuSeparator *item8 = new CMenuSeparator();
	ClistBoxItem *item9 = new ClistBoxItem("Beenden", true, NULL, this, "exit");
	
	leftWidget->addItem(new ClistBoxItem("Suche", true, NULL, this, "search"));
	leftWidget->addItem(new CMenuSeparator(LINE));
	leftWidget->addItem(new CMenuSeparator());
	leftWidget->addItem(new CMenuSeparator());
	leftWidget->addItem(new CMenuSeparator());
	leftWidget->addItem(new CMenuSeparator());
	leftWidget->addItem(new CMenuSeparator());
	leftWidget->addItem(new CMenuSeparator(LINE));
	leftWidget->addItem(item1);
	leftWidget->addItem(new CMenuSeparator(LINE));
	leftWidget->addItem(item2);
	leftWidget->addItem(new CMenuSeparator(LINE));
	leftWidget->addItem(item3);
	leftWidget->addItem(new CMenuSeparator(LINE));
	leftWidget->addItem(item4);
	leftWidget->addItem(new CMenuSeparator(LINE));
	leftWidget->addItem(new CMenuSeparator());
	leftWidget->addItem(new CMenuSeparator());
	leftWidget->addItem(new CMenuSeparator());
	leftWidget->addItem(new CMenuSeparator());
	leftWidget->addItem(new CMenuSeparator());
	leftWidget->addItem(new CMenuSeparator(LINE));
	leftWidget->addItem(new ClistBoxItem("Genres", true, NULL, this, "genres"));
	leftWidget->addItem(new CMenuSeparator(LINE));
	leftWidget->addItem(item5);
	leftWidget->addItem(item6);
	leftWidget->addItem(item7);
	leftWidget->addItem(item8);
	leftWidget->addItem(new CMenuSeparator());
	leftWidget->addItem(new CMenuSeparator());
	leftWidget->addItem(new CMenuSeparator(LINE));
	leftWidget->addItem(item9);
	leftWidget->addItem(new CMenuSeparator(LINE));

	// rightwidget
	rightBox.iWidth = mainWidget->getWindowsPos().iWidth - INTER_FRAME_SPACE - leftBox.iWidth;
	rightBox.iHeight = mainWidget->getWindowsPos().iHeight - headBox.iHeight - 2*INTER_FRAME_SPACE - footBox.iHeight;
	rightBox.iX = mainWidget->getWindowsPos().iX + leftBox.iWidth + INTER_FRAME_SPACE;
	rightBox.iY = mainWidget->getWindowsPos().iY + headBox.iHeight + INTER_FRAME_SPACE;

	//
	rightWidget = new ClistBox(&rightBox);
	rightWidget->setWidgetType(WIDGET_TYPE_FRAME);
	rightWidget->setItemsPerPage(5,2);
	rightWidget->setSelected(right_selected);
	rightWidget->enablePaintFootInfo();
	
	// load playlist
	loadTMDBPlaylist();

	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		std::string tmp = m_vMovieInfo[i].ytdate;
		tmp += " ";
		tmp += m_vMovieInfo[i].epgInfo1;

		item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "mplay", RC_nokey, NULL, file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

		item->setHelpText(tmp.c_str());

		rightWidget->addItem(item);
	}

	mainWidget->addItem(headersWidget);
	mainWidget->addItem(leftWidget);
	mainWidget->addItem(rightWidget);
	mainWidget->addItem(footersWidget);

	mainWidget->addKey(RC_info, this, CRCInput::getSpecialKeyName(RC_info));
	mainWidget->addKey(RC_setup, this, CRCInput::getSpecialKeyName(RC_setup));
	mainWidget->addKey(RC_red, this, "nextPage");
	mainWidget->addKey(RC_green, this, "prevPage");

	//mainWidget->setSelected(focus);

	mainWidget->exec(NULL, "");

	delete mainWidget;
	mainWidget = NULL;

	delete headersWidget;
	headersWidget = NULL;

	delete leftWidget;
	leftWidget = NULL;

	delete rightWidget;
	rightWidget = NULL;

	delete footersWidget;
	footersWidget = NULL;
}

int CNFilm::exec(CMenuTarget *parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CNFilm::exec: actionKey: %s\n", actionKey.c_str());

	if(parent)
		hide();

	if(actionKey == "RC_info")
	{
		if(mainWidget->getSelected() == WIDGET_RIGHT)
		{
			right_selected = rightWidget->getSelected();

			showMovieInfo(m_vMovieInfo[right_selected]);
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "mplay")
	{
		if(mainWidget->getSelected() == WIDGET_RIGHT)
		{
			right_selected = rightWidget->getSelected();

			// get video url
			getMovieVideoUrl(m_vMovieInfo[right_selected]);
		
			// play
			if (&m_vMovieInfo[right_selected].file != NULL) 
			{
				tmpMoviePlayerGui.addToPlaylist(m_vMovieInfo[right_selected]);
				tmpMoviePlayerGui.exec(NULL, "");
			}
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "RC_setup" || actionKey == "genres")
	{
		int res = showCategoriesMenu();

		if(res >= 0) // exit_pressed return -1
		{
			loadGenreMoviesTitle();

			rightWidget->clearItems();

			// load items
			for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
			{
				item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "mplay");

				item->setOption(m_vMovieInfo[i].epgChannel.c_str());

				item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

				item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

				item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

				item->set2lines();

				std::string tmp = m_vMovieInfo[i].epgTitle;
				tmp += "\n";
				tmp += m_vMovieInfo[i].epgInfo1;
				tmp += "\n";
				tmp += m_vMovieInfo[i].epgInfo2;

				item->setHelpText(tmp.c_str());

				rightWidget->addItem(item);
			}

			rightWidget->setSelected(0);
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "search")
	{
		tmdbsearch.clear();

		CStringInputSMS stringInput(LOCALE_YT_SEARCH, tmdbsearch.c_str());
		stringInput.exec(NULL, "");

		if(!stringInput.getExitPressed())
		{
			rightWidget->clearItems();

			loadTMDBPlaylist(tmdbsearch.c_str(), "", 1, true);

			// load items
			for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
			{
				item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "mplay");

				item->setOption(m_vMovieInfo[i].epgChannel.c_str());

				item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

				item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

				item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

				item->set2lines();

				std::string tmp = m_vMovieInfo[i].epgTitle;
				tmp += "\n";
				tmp += m_vMovieInfo[i].epgInfo1;
				tmp += "\n";
				tmp += m_vMovieInfo[i].epgInfo2;

				item->setHelpText(tmp.c_str());

				rightWidget->addItem(item);
			}

			rightWidget->setSelected(0);

			tmdbsearch.clear();
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "movie_in_cinema")
	{
		right_selected = 0;
		rightWidget->clearItems();
		TVShows = "movie";
		plist = "now_playing";
		page = 1;

		loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

		// load items
		for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
		{
			item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "mplay");

			item->setOption(m_vMovieInfo[i].epgChannel.c_str());

			item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

			item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

			item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

			item->set2lines();

			std::string tmp = m_vMovieInfo[i].epgTitle;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo1;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo2;

			item->setHelpText(tmp.c_str());

			rightWidget->addItem(item);
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "movie_popular")
	{
		right_selected = 0;
		rightWidget->clearItems();
		TVShows = "movie";
		plist = "popular";
		page = 1;

		loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

		// load items
		for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
		{
			item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "mplay");

			item->setOption(m_vMovieInfo[i].epgChannel.c_str());

			item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

			item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

			item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

			item->set2lines();

			std::string tmp = m_vMovieInfo[i].epgTitle;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo1;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo2;

			item->setHelpText(tmp.c_str());

			rightWidget->addItem(item);
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "movie_top_rated")
	{
		right_selected = 0;
		rightWidget->clearItems();
		TVShows = "movie";
		plist = "top_rated";
		page = 1;

		loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

		// load items
		for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
		{
			item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "mplay");

			item->setOption(m_vMovieInfo[i].epgChannel.c_str());

			item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

			item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

			item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

			item->set2lines();

			std::string tmp = m_vMovieInfo[i].epgTitle;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo1;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo2;

			item->setHelpText(tmp.c_str());

			rightWidget->addItem(item);
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "movie_new")
	{
		right_selected = 0;
		rightWidget->clearItems();
		TVShows = "movie";
		plist = "upcoming";
		page = 1;

		loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

		// load items
		for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
		{
			item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "mplay");

			item->setOption(m_vMovieInfo[i].epgChannel.c_str());

			item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

			item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

			item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

			item->set2lines();

			std::string tmp = m_vMovieInfo[i].epgTitle;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo1;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo2;

			item->setHelpText(tmp.c_str());

			rightWidget->addItem(item);
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "nextPage")
	{
		page++;
		right_selected = 0;
		rightWidget->clearItems();

		loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

		// load items
		for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
		{
			item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "mplay");

			item->setOption(m_vMovieInfo[i].epgChannel.c_str());

			item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

			item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

			item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

			item->set2lines();

			std::string tmp = m_vMovieInfo[i].epgTitle;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo1;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo2;

			item->setHelpText(tmp.c_str());

			rightWidget->addItem(item);
		}

		rightWidget->setSelected(0);

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "prevPage")
	{
		page--;
		if(page <= 1)
			page = 1;
		right_selected = 0;
		rightWidget->clearItems();

		loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

		// load items
		for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
		{
			item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "mplay");

			item->setOption(m_vMovieInfo[i].epgChannel.c_str());

			item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

			item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

			item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

			item->set2lines();

			std::string tmp = m_vMovieInfo[i].epgTitle;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo1;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo2;

			item->setHelpText(tmp.c_str());

			rightWidget->addItem(item);
		}

		rightWidget->setSelected(0);

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "exit")
	{
		return menu_return::RETURN_EXIT_ALL;
	}

	showMenu();

	return menu_return::RETURN_EXIT;
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


