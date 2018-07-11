/* 
  $Id: youtube.cpp 2018/07/11 mohousch Exp $

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

#include <youtube.h>

extern "C" void plugin_exec(void);
extern "C" void plugin_init(void);
extern "C" void plugin_del(void);

#define NEUTRINO_ICON_YT			PLUGINDIR "/youtube/youtube.png"
#define NEUTRINO_ICON_YT_SMALL			PLUGINDIR "/youtube/youtube_small.png"

YTB_SETTINGS m_settings;

#define OPTIONS_OFF0_ON1_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_OFF0_ON1_OPTIONS[OPTIONS_OFF0_ON1_OPTION_COUNT] =
{
        { 0, LOCALE_OPTIONS_OFF, NULL },
        { 1, LOCALE_OPTIONS_ON, NULL }
};
 
CYTBrowser::CYTBrowser(): configfile ('\t')
{
	moviesMenu = NULL;
	item = NULL;

	init();
}

CYTBrowser::~CYTBrowser()
{
	dprintf(DEBUG_NORMAL, "CYTBrowser: del\n");
	
	m_vMovieInfo.clear();

	ytparser.Cleanup();

	saveSettings(&m_settings);
}

void CYTBrowser::init(void)
{
	dprintf(DEBUG_NORMAL, "CYTBrowser::init\n");
	
	loadSettings(&m_settings);
}

bool CYTBrowser::loadSettings(YTB_SETTINGS *settings)
{
	bool result = true;
	
	dprintf(DEBUG_NORMAL, "CYTBrowser::loadSettings\r\n");
	
	if(configfile.loadConfig(YTBROWSER_SETTINGS_FILE))
	{
		//settings->ytmode = configfile.getInt32("ytmode", cYTFeedParser::MOST_POPULAR);
		m_settings.ytmode = cYTFeedParser::MOST_POPULAR;
		settings->ytorderby = configfile.getInt32("ytorderby", cYTFeedParser::ORDERBY_PUBLISHED);
		settings->ytregion = configfile.getString("ytregion", "default");
		settings->ytsearch = configfile.getString("ytsearch", "");
		settings->ytkey = configfile.getString("ytkey", "");

		settings->ytautoplay = configfile.getInt32("ytautoplay", 0);
	}
	else
	{
		dprintf(DEBUG_NORMAL, "CYTBrowser::loadSettings failed\r\n"); 
		configfile.clear();
		result = false;
	}
	
	return (result);
}

bool CYTBrowser::saveSettings(YTB_SETTINGS *settings)
{
	bool result = true;
	dprintf(DEBUG_NORMAL, "CYTBrowser::saveSettings\r\n");

	//configfile.setInt32("ytmode", settings->ytmode);
	configfile.setInt32("ytorderby", settings->ytorderby);
	configfile.setString("ytregion", settings->ytregion);
	configfile.setString("ytsearch", settings->ytsearch);
	configfile.setString("ytkey", settings->ytkey);

	configfile.setInt32("ytautoplay", settings->ytautoplay);
 
 	if (configfile.getModifiedFlag())
		configfile.saveConfig(YTBROWSER_SETTINGS_FILE);
	
	return (result);
}

//
#define YT_HEAD_BUTTONS_COUNT	3
const struct button_label YTHeadButtons[YT_HEAD_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_HELP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_SETUP, NONEXISTANT_LOCALE, NULL},
	{ NEUTRINO_ICON_BUTTON_RED, NONEXISTANT_LOCALE, NULL}
};

void CYTBrowser::showYTMoviesMenu(bool reload)
{
	dprintf(DEBUG_NORMAL, "CYTBrowser::loadMovies:\n");

	if(reload)
	{
		CHintBox loadBox(LOCALE_YOUTUBE, g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
		loadBox.paint();

		loadYTTitles(m_settings.ytmode, m_settings.ytsearch, m_settings.ytvid);

		loadBox.hide();
	}

	//
	std::string title;
	title = g_Locale->getText(LOCALE_YOUTUBE);
	title += " : ";
		
	neutrino_locale_t loc = getFeedLocale();
	title += g_Locale->getText(loc);
	if (loc == LOCALE_YT_SEARCH)
		title += " \"" + m_settings.ytsearch + "\"";

	moviesMenu = new ClistBox(title.c_str(), NEUTRINO_ICON_YT_SMALL);
	
	std::string itemTitle;

	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		itemTitle = m_vMovieInfo[i].epgTitle + " (" + to_string(m_vMovieInfo[i].length) + " Min)";

		item = new ClistBoxItem(itemTitle.c_str(), true, m_vMovieInfo[i].epgInfo2.c_str(), this, "play", NULL,  file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

		item->setInfo1(m_vMovieInfo[i].epgInfo2.c_str());
 
		moviesMenu->addItem(item);
	}

	moviesMenu->setWidgetType(WIDGET_FRAME);
	moviesMenu->setItemsPerPage(3, 2);
	moviesMenu->setItemBoxColor(COL_YELLOW);
	moviesMenu->setHeaderButtons(YTHeadButtons, YT_HEAD_BUTTONS_COUNT);

	moviesMenu->addKey(CRCInput::RC_info, this, CRCInput::getSpecialKeyName(CRCInput::RC_info));
	moviesMenu->addKey(CRCInput::RC_setup, this, CRCInput::getSpecialKeyName(CRCInput::RC_setup));
	moviesMenu->addKey(CRCInput::RC_red, this, CRCInput::getSpecialKeyName(CRCInput::RC_red));

	moviesMenu->exec(NULL, "");
	//moviesMenu->hide();
	delete moviesMenu;
	moviesMenu = NULL;
}

void CYTBrowser::playMovie(void)
{
	CMoviePlayerGui tmpMoviePlayerGui;

	if(m_settings.ytautoplay)
	{
		// add selected video
		tmpMoviePlayerGui.addToPlaylist(m_vMovieInfo[moviesMenu->getSelected()]);

		// get related videos
		loadYTTitles(cYTFeedParser::RELATED, "", m_vMovieInfo[moviesMenu->getSelected()].ytid);

		for(int i = 0; i < m_vMovieInfo.size(); i++)
		{
			if (&m_vMovieInfo[i].file != NULL) 
				tmpMoviePlayerGui.addToPlaylist(m_vMovieInfo[i]);
		}

		tmpMoviePlayerGui.exec(NULL, "urlplayback");
	}
	else
	{
		if (&m_vMovieInfo[moviesMenu->getSelected()].file != NULL) 
		{
			tmpMoviePlayerGui.addToPlaylist(m_vMovieInfo[moviesMenu->getSelected()]);
			tmpMoviePlayerGui.exec(NULL, "urlplayback");
		}
	}
}

void CYTBrowser::showMovieInfo(void)
{
	m_movieInfo.showMovieInfo(m_vMovieInfo[moviesMenu->getSelected()]);
}

int CYTBrowser::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CYTBrowser::exec: actionKey:%s\n", actionKey.c_str());

	if(parent) 
		parent->hide();

	if(actionKey == "play")
	{
		playMovie();

		if(m_settings.ytautoplay)
		{
			showYTMoviesMenu(false);
		}
		else
			return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "RC_info")
	{
		showMovieInfo();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "RC_setup")
	{
		int res = showYTMenu();

		if( res >= 0 && res <= 6)
			showYTMoviesMenu();
		else
			return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "RC_red")
	{
		m_settings.ytvid = m_vMovieInfo[moviesMenu->getSelected()].ytid;
		m_settings.ytmode = cYTFeedParser::RELATED;

		showYTMoviesMenu();
	}
	
	return menu_return::RETURN_EXIT;
}

void CYTBrowser::loadYTTitles(int mode, std::string search, std::string id)
{
	dprintf(DEBUG_NORMAL, "CYTBrowser::loadYTTitles: parsed %d old mode %d new mode %d region %s\n", ytparser.Parsed(), ytparser.GetFeedMode(), m_settings.ytmode, m_settings.ytregion.c_str());

	//
	ytparser.Cleanup();
	
	if (m_settings.ytregion == "default")
		ytparser.SetRegion("");
	else
		ytparser.SetRegion(m_settings.ytregion);

REPEAT:
	if (!ytparser.Parsed() || (ytparser.GetFeedMode() != mode)) 
	{
	
		if (ytparser.ParseFeed((cYTFeedParser::yt_feed_mode_t)mode, search, id, (cYTFeedParser::yt_feed_orderby_t)m_settings.ytorderby))
		{
			ytparser.DownloadThumbnails();
		} 
		else 
		{
			//FIXME show error
			MessageBox(LOCALE_MESSAGEBOX_ERROR, g_Locale->getText(LOCALE_YT_ERROR), CMessageBox::mbrCancel, CMessageBox::mbCancel, NEUTRINO_ICON_ERROR);
			
			//return;
			if(mode == cYTFeedParser::PREV)
			{
				mode = ytparser.GetFeedMode();
				goto REPEAT;
			}
		}
	}
	
	m_vMovieInfo.clear();
	yt_video_list_t &ylist = ytparser.GetVideoList();
	
	for (unsigned i = 0; i < ylist.size(); i++) 
	{
		MI_MOVIE_INFO movieInfo;
		m_movieInfo.clearMovieInfo(&movieInfo); // refresh structure
		
		movieInfo.epgChannel = ylist[i].author;
		movieInfo.epgTitle = ylist[i].title;
		movieInfo.epgInfo1 = ylist[i].category;
		movieInfo.epgInfo2 = ylist[i].description;
		movieInfo.length = ylist[i].duration/60 ;
		movieInfo.tfile = ylist[i].tfile;
		movieInfo.ytdate = ylist[i].published;
		movieInfo.ytid = ylist[i].id;
		movieInfo.file.Name = ylist[i].GetUrl();
		
		m_vMovieInfo.push_back(movieInfo);
	}
}

const CMenuOptionChooser::keyval YT_FEED_OPTIONS[] =
{
        { cYTFeedParser::MOST_POPULAR, LOCALE_YT_MOST_POPULAR, NULL },
        { cYTFeedParser::MOST_POPULAR_ALL_TIME, LOCALE_YT_MOST_POPULAR_ALL_TIME, NULL },
	{ cYTFeedParser::NEXT, LOCALE_YT_NEXT_RESULTS, NULL },
	{ cYTFeedParser::PREV, LOCALE_YT_PREV_RESULTS, NULL }
};

#define YT_FEED_OPTION_COUNT (sizeof(YT_FEED_OPTIONS)/sizeof(CMenuOptionChooser::keyval))

const CMenuOptionChooser::keyval YT_ORDERBY_OPTIONS[] =
{
        { cYTFeedParser::ORDERBY_PUBLISHED, LOCALE_YT_ORDERBY_PUBLISHED, NULL },
        { cYTFeedParser::ORDERBY_RELEVANCE, LOCALE_YT_ORDERBY_RELEVANCE, NULL },
        { cYTFeedParser::ORDERBY_VIEWCOUNT, LOCALE_YT_ORDERBY_VIEWCOUNT, NULL },
        { cYTFeedParser::ORDERBY_RATING, LOCALE_YT_ORDERBY_RATING, NULL },
};

#define YT_ORDERBY_OPTION_COUNT (sizeof(YT_ORDERBY_OPTIONS)/sizeof(CMenuOptionChooser::keyval))

neutrino_locale_t CYTBrowser::getFeedLocale(void)
{
	neutrino_locale_t ret = LOCALE_YT_MOST_POPULAR;

	if (m_settings.ytmode == cYTFeedParser::RELATED)
		return LOCALE_YT_RELATED;

	if (m_settings.ytmode == cYTFeedParser::SEARCH)
		return LOCALE_YT_SEARCH;

	for (unsigned i = 0; i < YT_FEED_OPTION_COUNT; i++) 
	{
		if (m_settings.ytmode == YT_FEED_OPTIONS[i].key)
			return YT_FEED_OPTIONS[i].value;
	}
	
	return ret;
}

int CYTBrowser::showYTMenu(void)
{
	CMenuWidget mainMenu(LOCALE_YOUTUBE, NEUTRINO_ICON_YT_SMALL, MENU_WIDTH + 100);
	mainMenu.enableSaveScreen();

	int select = -1;
	CMenuSelectorTarget * selector = new CMenuSelectorTarget(&select);

	char cnt[5];
	for (unsigned i = 0; i < YT_FEED_OPTION_COUNT; i++) 
	{
		sprintf(cnt, "%d", YT_FEED_OPTIONS[i].key);
		mainMenu.addItem(new CMenuForwarder(YT_FEED_OPTIONS[i].value, true, NULL, selector, cnt, CRCInput::convertDigitToKey(i + 1)), m_settings.ytmode == (int) YT_FEED_OPTIONS[i].key);
	}

	mainMenu.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	std::string search = m_settings.ytsearch;
	
	// search
	CStringInputSMS stringInput(LOCALE_YT_SEARCH, &search);
	mainMenu.addItem(new CMenuForwarder(LOCALE_YT_SEARCH, true, search, &stringInput, NULL, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	
	// ytorder
	mainMenu.addItem(new CMenuOptionChooser(LOCALE_YT_ORDERBY, &m_settings.ytorderby, YT_ORDERBY_OPTIONS, YT_ORDERBY_OPTION_COUNT, true, NULL, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN, true));

	// search
	sprintf(cnt, "%d", cYTFeedParser::SEARCH);
	mainMenu.addItem(new CMenuForwarder(LOCALE_EVENTFINDER_START_SEARCH, true, NULL, selector, cnt, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW));

	mainMenu.addItem(new CMenuSeparator(CMenuSeparator::LINE));

	char rstr[20];
	sprintf(rstr, "%s", m_settings.ytregion.c_str());
	CMenuOptionStringChooser * region = new CMenuOptionStringChooser(LOCALE_YT_REGION, rstr, true, NULL, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE, true, true);
	region->addOption("default");
	region->addOption("DE");
	region->addOption("PL");
	region->addOption("RU");
	region->addOption("NL");
	region->addOption("CZ");
	region->addOption("FR");
	region->addOption("HU");
	region->addOption("US");
	mainMenu.addItem(region);

	// key
	mainMenu.addItem(new CMenuSeparator(CMenuSeparator::LINE));

	std::string key = m_settings.ytkey;
	
	CStringInputSMS* keyInput = new CStringInputSMS("YT Key:", (char *)key.c_str());
	mainMenu.addItem(new CMenuForwarder("YT:", true, key, keyInput));

	// autoplay
	mainMenu.addItem(new CMenuOptionChooser(LOCALE_YT_AUTOPLAY, &m_settings.ytautoplay, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true));

	mainMenu.exec(NULL, "");
	delete selector;
	delete keyInput;
	keyInput = NULL;

	printf("select: %d\n", select);

	int newmode = -1;

	if (select >= 0) 
	{
		newmode = select;
		if (newmode == cYTFeedParser::NEXT || newmode == cYTFeedParser::PREV) 
		{
			m_settings.ytmode = newmode;
		}
		else if (select == cYTFeedParser::SEARCH) 
		{
			if (!search.empty()) 
			{
				m_settings.ytsearch = search;
				m_settings.ytmode = newmode;
			}
		}
		else if (m_settings.ytmode != newmode) 
		{
			m_settings.ytmode = newmode;
		}
	}
	
	if(rstr != m_settings.ytregion) 
	{
		m_settings.ytregion = rstr;
		if (newmode < 0)
			newmode = m_settings.ytmode;
	}

	return (select);
}

//
void plugin_init(void)
{
}

void plugin_del(void)
{
}

void plugin_exec(void)
{
	CYTBrowser * YTHandler = new CYTBrowser();
	
	YTHandler->showYTMoviesMenu();
	YTHandler->exec(NULL, "");
	
	delete YTHandler;
	YTHandler = NULL;	
}


