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

// locale
enum {
	LOCALE_YOUTUBE,
	LOCALE_YT_YOUTUBE,
	LOCALE_YT_ERROR,
	LOCALE_YT_MOST_POPULAR,
	LOCALE_YT_MOST_POPULAR_ALL_TIME,
	LOCALE_YT_NEXT_RESULTS,
	LOCALE_YT_PREV_RESULTS,
	LOCALE_YT_REGION,
	LOCALE_YT_RELATED,
	LOCALE_YT_SEARCH,
	LOCALE_YT_ORDERBY,
	LOCALE_YT_ORDERBY_PUBLISHED,
	LOCALE_YT_ORDERBY_RATING,
	LOCALE_YT_ORDERBY_RELEVANCE,
	LOCALE_YT_ORDERBY_VIEWCOUNT,
	LOCALE_YT_AUTOPLAY
};

const char * locale_real_names_yt[] = {
	"",
	"youtube.yt_youtube",
	"youtube.yt_error",
	"youtube.yt_most_popular",
	"youtube.yt_most_popular_all_time",
	"youtube.yt_next_results",
	"youtube.yt_prev_results",
	"youtube.yt_region",
	"youtube.yt_related",
	"youtube.yt_search",
	"youtube.yt_orderby",
	"youtube.yt_orderby.published",
	"youtube.yt_orderby.rating",
	"youtube.yt_orderby.relevance",
	"youtube.yt_orderby.viewcount",
	"youtube.yt_autoplay"
};

#define NEUTRINO_ICON_YT			PLUGINDIR "/youtube/youtube.png"
#define NEUTRINO_ICON_YT_SMALL			PLUGINDIR "/youtube/youtube_small.png"

YTB_SETTINGS m_settings;

#define OPTIONS_OFF0_ON1_OPTION_COUNT 2
const keyval OPTIONS_OFF0_ON1_OPTIONS[OPTIONS_OFF0_ON1_OPTION_COUNT] =
{
        { 0, LOCALE_OPTIONS_OFF, NULL },
        { 1, LOCALE_OPTIONS_ON, NULL }
};
 
CYTBrowser::CYTBrowser(int mode): configfile ('\t')
{
	moviesMenu = NULL;
	item = NULL;

	init();

	ytmode = mode;
}

CYTBrowser::~CYTBrowser()
{
	dprintf(DEBUG_NORMAL, "CYTBrowser: del\n");
	
	m_vMovieInfo.clear();

	ytparser.Cleanup();

	saveSettings(&m_settings);
	ytsearch.clear();
}

void CYTBrowser::hide()
{
	CFrameBuffer::getInstance()->paintBackground();
	CFrameBuffer::getInstance()->blit();
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
		settings->ytorderby = configfile.getInt32("ytorderby", cYTFeedParser::ORDERBY_PUBLISHED);
		settings->ytregion = configfile.getString("ytregion", "default");
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

	configfile.setInt32("ytorderby", settings->ytorderby);
	configfile.setString("ytregion", settings->ytregion);
	configfile.setInt32("ytautoplay", settings->ytautoplay);
 
 	if (configfile.getModifiedFlag())
		configfile.saveConfig(YTBROWSER_SETTINGS_FILE);
	
	return (result);
}

//
#define YT_HEAD_BUTTONS_COUNT	2
const struct button_label YTHeadButtons[YT_HEAD_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_HELP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_SETUP, NONEXISTANT_LOCALE, NULL}
};

#define YT_FOOT_BUTTONS_COUNT  4
const struct button_label YTFootButtons[YT_FOOT_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_RED, NONEXISTANT_LOCALE, /*g_Locale->getCustomText((neutrino_locale_t)LOCALE_YT_NEXT_RESULTS)*/"next results" },
	{ NEUTRINO_ICON_BUTTON_GREEN, NONEXISTANT_LOCALE, /*g_Locale->getCustomText((neutrino_locale_t)LOCALE_YT_PREV_RESULTS)*/"prev results" },
	{ NEUTRINO_ICON_BUTTON_YELLOW, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_BLUE, NONEXISTANT_LOCALE, /*g_Locale->getCustomText((neutrino_locale_t)LOCALE_YT_MOST_POPULAR)*/"most popular"}
};

void CYTBrowser::showMenu()
{
	dprintf(DEBUG_NORMAL, "CYTBrowser::showMenu:\n");

	//
	std::string title;
	title = g_Locale->getCustomText((neutrino_locale_t)LOCALE_YT_YOUTUBE);
	title += " : ";
		
	neutrino_locale_t loc = getFeedLocale();
	title += g_Locale->getCustomText(loc);
	if (loc == (neutrino_locale_t)LOCALE_YT_SEARCH)
		title += " \"" + ytsearch + "\"";

	moviesMenu = new ClistBoxWidget(title.c_str(), NEUTRINO_ICON_YT_SMALL);
	
	std::string itemTitle;

	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		itemTitle = m_vMovieInfo[i].epgTitle + " (" + to_string(m_vMovieInfo[i].length) + " Min)";

		item = new ClistBoxItem(itemTitle.c_str(), true, /*m_vMovieInfo[i].epgInfo2.c_str()*/NULL, this, "play", RC_nokey, NULL,  file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

		//item->setInfo1(m_vMovieInfo[i].epgInfo2.c_str());
		item->setHelpText(m_vMovieInfo[i].epgInfo2.c_str());
 
		moviesMenu->addItem(item);
	}

	moviesMenu->setMode(MODE_LISTBOX);
	moviesMenu->setWidgetType(WIDGET_TYPE_FRAME);
	moviesMenu->setItemsPerPage(3, 2);
	moviesMenu->enablePaintItemInfo();
	//moviesMenu->setItemBoxColor(COL_YELLOW);

	moviesMenu->setHeaderButtons(YTHeadButtons, YT_HEAD_BUTTONS_COUNT);
	moviesMenu->setFooterButtons(YTFootButtons, YT_FOOT_BUTTONS_COUNT);

	moviesMenu->addKey(RC_info, this, CRCInput::getSpecialKeyName(RC_info));
	moviesMenu->addKey(RC_setup, this, CRCInput::getSpecialKeyName(RC_setup));
	moviesMenu->addKey(RC_red, this, CRCInput::getSpecialKeyName(RC_red));
	moviesMenu->addKey(RC_green, this, CRCInput::getSpecialKeyName(RC_green));
	moviesMenu->addKey(RC_blue, this, CRCInput::getSpecialKeyName(RC_blue));

	moviesMenu->addKey(RC_record, this, CRCInput::getSpecialKeyName(RC_record));

	moviesMenu->exec(NULL, "");
	delete moviesMenu;
	moviesMenu = NULL;
}

void CYTBrowser::playMovie(void)
{
	if(m_settings.ytautoplay)
	{
		// add selected video
		tmpMoviePlayerGui.addToPlaylist(m_vMovieInfo[moviesMenu->getSelected()]);

		// get related videos
		loadYTTitles(cYTFeedParser::RELATED, "", m_vMovieInfo[moviesMenu->getSelected()].ytid, false);

		for(int i = 0; i < m_vMovieInfo.size(); i++)
		{
			if (&m_vMovieInfo[i].file != NULL) 
				tmpMoviePlayerGui.addToPlaylist(m_vMovieInfo[i]);
		}

		tmpMoviePlayerGui.exec(NULL, "");
	}
	else
	{
		if (&m_vMovieInfo[moviesMenu->getSelected()].file != NULL) 
		{
			tmpMoviePlayerGui.addToPlaylist(m_vMovieInfo[moviesMenu->getSelected()]);
			tmpMoviePlayerGui.exec(NULL, "");
		}
	}
}

void CYTBrowser::showMovieInfo(void)
{
	m_movieInfo.showMovieInfo(m_vMovieInfo[moviesMenu->getSelected()]);
}

void CYTBrowser::recordMovie(void)
{
	::start_file_recording(m_vMovieInfo[moviesMenu->getSelected()].epgTitle.c_str(), m_vMovieInfo[moviesMenu->getSelected()].epgInfo2.c_str(), m_vMovieInfo[moviesMenu->getSelected()].file.Name.c_str());
}

void CYTBrowser::loadYTTitles(int mode, std::string search, std::string id, bool show_hint)
{
	dprintf(DEBUG_NORMAL, "CYTBrowser::loadYTTitles: parsed %d old mode %d new mode %d region %s\n", ytparser.Parsed(), ytparser.GetFeedMode(), ytmode, m_settings.ytregion.c_str());

	CHintBox loadBox(g_Locale->getCustomText((neutrino_locale_t)LOCALE_YT_YOUTUBE), g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));

	if(show_hint)
		loadBox.paint();

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
			MessageBox(LOCALE_MESSAGEBOX_ERROR, g_Locale->getCustomText((neutrino_locale_t)LOCALE_YT_ERROR), mbrCancel, mbCancel, NEUTRINO_ICON_ERROR);
			
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

	if(show_hint)
		loadBox.hide();
}

const keyval YT_FEED_OPTIONS[] =
{
       	{ cYTFeedParser::MOST_POPULAR, (neutrino_locale_t)LOCALE_YT_MOST_POPULAR, NULL },
        //{ cYTFeedParser::MOST_POPULAR_ALL_TIME, (neutrino_locale_t)LOCALE_YT_MOST_POPULAR_ALL_TIME, NULL },
	//{ cYTFeedParser::NEXT, (neutrino_locale_t)LOCALE_YT_NEXT_RESULTS, NULL },
	//{ cYTFeedParser::PREV, (neutrino_locale_t)LOCALE_YT_PREV_RESULTS, NULL }
};

#define YT_FEED_OPTION_COUNT (sizeof(YT_FEED_OPTIONS)/sizeof(keyval))

const keyval YT_ORDERBY_OPTIONS[] =
{
        { cYTFeedParser::ORDERBY_PUBLISHED, (neutrino_locale_t)LOCALE_YT_ORDERBY_PUBLISHED, NULL },
        { cYTFeedParser::ORDERBY_RELEVANCE, (neutrino_locale_t)LOCALE_YT_ORDERBY_RELEVANCE, NULL },
        { cYTFeedParser::ORDERBY_VIEWCOUNT, (neutrino_locale_t)LOCALE_YT_ORDERBY_VIEWCOUNT, NULL },
        { cYTFeedParser::ORDERBY_RATING, (neutrino_locale_t)LOCALE_YT_ORDERBY_RATING, NULL },
};

#define YT_ORDERBY_OPTION_COUNT (sizeof(YT_ORDERBY_OPTIONS)/sizeof(keyval))

neutrino_locale_t CYTBrowser::getFeedLocale(void)
{
	neutrino_locale_t ret = (neutrino_locale_t)LOCALE_YT_MOST_POPULAR;

	if (ytmode == cYTFeedParser::RELATED)
		return (neutrino_locale_t)LOCALE_YT_RELATED;

	if (ytmode == cYTFeedParser::SEARCH)
		return (neutrino_locale_t)LOCALE_YT_SEARCH;

	for (unsigned i = 0; i < YT_FEED_OPTION_COUNT; i++) 
	{
		if (ytmode == YT_FEED_OPTIONS[i].key)
			return YT_FEED_OPTIONS[i].value;
	}
	
	return ret;
}

int CYTBrowser::showCategoriesMenu(void)
{
	dprintf(DEBUG_NORMAL, "CYTBrowser::showCategoriesMenu:");

	int res = -1;

	ClistBoxWidget mainMenu(g_Locale->getCustomText((neutrino_locale_t)LOCALE_YT_YOUTUBE), NEUTRINO_ICON_YT_SMALL);

	mainMenu.enableSaveScreen();
	mainMenu.setMode(MODE_MENU);
	mainMenu.enableShrinkMenu();

	mainMenu.addItem(new CMenuForwarder(g_Locale->getCustomText((neutrino_locale_t)LOCALE_YT_MOST_POPULAR), true, NULL, new CYTBrowser(cYTFeedParser::MOST_POPULAR), NULL));

	mainMenu.addItem(new CMenuSeparator(LINE));
	
	// search
	mainMenu.addItem(new CMenuForwarder(g_Locale->getCustomText((neutrino_locale_t)LOCALE_YT_SEARCH), true, ytsearch.c_str(), this, "search", RC_red, NEUTRINO_ICON_BUTTON_RED));
	
	// ytorder
	mainMenu.addItem(new CMenuOptionChooser(g_Locale->getCustomText((neutrino_locale_t)LOCALE_YT_ORDERBY), &m_settings.ytorderby, YT_ORDERBY_OPTIONS, YT_ORDERBY_OPTION_COUNT, true, NULL, RC_green, NEUTRINO_ICON_BUTTON_GREEN, true));

	mainMenu.addItem(new CMenuSeparator(LINE));

	char rstr[20];
	sprintf(rstr, "%s", m_settings.ytregion.c_str());
	CMenuOptionStringChooser * region = new CMenuOptionStringChooser(g_Locale->getCustomText((neutrino_locale_t)LOCALE_YT_REGION), rstr, true, NULL, RC_blue, NEUTRINO_ICON_BUTTON_BLUE, true);
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

	mainMenu.addItem(new CMenuSeparator(LINE));

	// autoplay
	mainMenu.addItem(new CMenuOptionChooser(g_Locale->getCustomText((neutrino_locale_t)LOCALE_YT_AUTOPLAY), &m_settings.ytautoplay, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true));

	mainMenu.exec(NULL, "");
	
	if(rstr != m_settings.ytregion) 
	{
		m_settings.ytregion = rstr;
	}

	res = mainMenu.getSelected();

	return res;
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
			showMenu();
			return menu_return::RETURN_EXIT_ALL;
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
		int res = showCategoriesMenu();

		if( res >= 0 && res <= 6)
		{
			showMenu();
			return menu_return::RETURN_EXIT_ALL;
		}
		else
			return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "RC_blue")
	{
		ytvid = m_vMovieInfo[moviesMenu->getSelected()].ytid;
		ytmode = cYTFeedParser::RELATED;

		loadYTTitles(ytmode, ytsearch, ytvid);
		showMenu();

		return menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey == "RC_red")
	{
		ytvid = m_vMovieInfo[moviesMenu->getSelected()].ytid;
		ytmode = cYTFeedParser::NEXT;

		loadYTTitles(ytmode, ytsearch, ytvid);
		showMenu();

		return menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey == "RC_green")
	{
		ytvid = m_vMovieInfo[moviesMenu->getSelected()].ytid;
		ytmode = cYTFeedParser::PREV;

		loadYTTitles(ytmode, ytsearch, ytvid);
		showMenu();

		return menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey == "RC_record")
	{
		recordMovie();
		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "search")
	{
		ytmode = cYTFeedParser::SEARCH;

		CStringInputSMS stringInput(g_Locale->getCustomText((neutrino_locale_t)LOCALE_YT_SEARCH), ytsearch.c_str());
		int ret = stringInput.exec(NULL, "");

		if(!stringInput.getExitPressed() /*&& !ytsearch.empty()*/) //FIXME:
		{
			loadYTTitles(ytmode, ytsearch, ytvid);
			showMenu();

			return menu_return::RETURN_EXIT_ALL;
		}
		else
			return menu_return::RETURN_REPAINT;
	}

	loadYTTitles(ytmode, ytsearch, ytvid);
	showMenu();
	
	return menu_return::RETURN_EXIT;
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
	// load locale
	g_Locale->loadCustomLocale(g_settings.language, locale_real_names_yt, sizeof(locale_real_names_yt)/sizeof(const char *), PLUGINDIR "/youtube/");

	CYTBrowser* YTHandler = new CYTBrowser(cYTFeedParser::MOST_POPULAR);
	
	YTHandler->exec(NULL, "");
	
	delete YTHandler;
	YTHandler = NULL;

	// unload locale
	g_Locale->unloadCustomLocale(locale_real_names_yt, sizeof(locale_real_names_yt)/sizeof(const char *));		
}


