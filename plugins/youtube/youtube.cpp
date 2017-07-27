/* 
  $Id: youtube.cpp 2014/10/03 mohousch Exp $

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
 
CYTBrowser::CYTBrowser(): configfile ('\t')
{
	dprintf(DEBUG_NORMAL, "$Id: youtube Browser, v 0.0.1 2014/09/15 12:00:30 mohousch Exp $\n");

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
	{ NEUTRINO_ICON_BUTTON_0, NONEXISTANT_LOCALE, NULL}
};

void CYTBrowser::showYTMoviesMenu(void)
{
	dprintf(DEBUG_NORMAL, "CYTBrowser::loadMovies:\n");

	CHintBox loadBox(LOCALE_YOUTUBE, g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
	loadBox.paint();

	loadYTTitles(m_settings.ytmode, m_settings.ytsearch, m_settings.ytvid);

	loadBox.hide();

	//
	std::string title;
	title = g_Locale->getText(LOCALE_YOUTUBE);
	title += " : ";
		
	neutrino_locale_t loc = getFeedLocale();
	title += g_Locale->getText(loc);
	if (loc == LOCALE_YT_SEARCH)
		title += " \"" + m_settings.ytsearch + "\"";

	moviesMenu = new CMenuFrameBox(title.c_str(), NEUTRINO_ICON_YT_SMALL);
	
	std::string itemTitle;

	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		itemTitle = m_vMovieInfo[i].epgTitle + " (" + to_string(m_vMovieInfo[i].length) + " Min)";
 
		moviesMenu->addItem(new CMenuFrameBoxItem(/*m_vMovieInfo[i].epgTitle*/itemTitle.c_str(), this, "play", file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg"));
	}

	moviesMenu->setItemsPerPage(3, 2);
	moviesMenu->setItemBoxColor(COL_YELLOW);
	moviesMenu->setHeaderButtons(YTHeadButtons, YT_HEAD_BUTTONS_COUNT);

	moviesMenu->addKey(CRCInput::RC_info, this, CRCInput::getSpecialKeyName(CRCInput::RC_info));
	moviesMenu->addKey(CRCInput::RC_setup, this, CRCInput::getSpecialKeyName(CRCInput::RC_setup));
	moviesMenu->addKey(CRCInput::RC_0, this, CRCInput::getSpecialKeyName(CRCInput::RC_0));

	moviesMenu->exec(NULL, "");
	//moviesMenu->hide();
	delete moviesMenu;
	moviesMenu = NULL;
	//
}

void CYTBrowser::playMovie(void)
{
	CMoviePlayerGui tmpMoviePlayerGui;

	if (&m_vMovieInfo[moviesMenu->getSelected()].file != NULL) 
	{
		tmpMoviePlayerGui.addToPlaylist(m_vMovieInfo[moviesMenu->getSelected()]);
		tmpMoviePlayerGui.exec(NULL, "urlplayback");
	}
}

void CYTBrowser::showMovieInfo(void)
{
	CBox position(g_settings.screen_StartX + 50, g_settings.screen_StartY + 50, g_settings.screen_EndX - g_settings.screen_StartX - 100, g_settings.screen_EndY - g_settings.screen_StartY - 100); 
	
	CInfoBox * infoBox = new CInfoBox(m_vMovieInfo[moviesMenu->getSelected()].epgInfo2.c_str(), g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1], CTextBox::SCROLL, &position, m_vMovieInfo[moviesMenu->getSelected()].epgTitle.c_str(), g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE], NEUTRINO_ICON_MOVIE);

	// icon
	int picw = 320; //FIXME
	int pich = 256;	//FIXME

	int p_w = 0;
	int p_h = 0;
	int nbpp = 0;

	if(!access(m_vMovieInfo[moviesMenu->getSelected()].tfile.c_str(), F_OK))
	{
		CFrameBuffer::getInstance()->getSize(m_vMovieInfo[moviesMenu->getSelected()].tfile, &p_w, &p_h, &nbpp);

		// scale
		if(p_w <= picw && p_h <= pich)
		{
			picw = p_w;
			pich = p_h;
		}
		else
		{
			float aspect = (float)(p_w) / (float)(p_h);
					
			if (((float)(p_w) / (float)picw) > ((float)(p_h) / (float)pich)) 
			{
				p_w = picw;
				p_h = (int)(picw / aspect);
			}
			else
			{
				p_h = pich;
				p_w = (int)(pich * aspect);
			}
		}
	}

	infoBox->setText(&m_vMovieInfo[moviesMenu->getSelected()].epgInfo2, m_vMovieInfo[moviesMenu->getSelected()].tfile, p_w, p_h);
	infoBox->exec();
	delete infoBox;
}

int CYTBrowser::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CYTBrowser::exec: actionKey:%s\n", actionKey.c_str());

	if(parent) 
		parent->hide();

	if(actionKey == "play")
	{
		playMovie();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "RC_info")
	{
		showMovieInfo();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "RC_setup")
	{
		showYTMenu();
	}
	else if(actionKey == "RC_0")
	{
		m_settings.ytvid = m_vMovieInfo[moviesMenu->getSelected()].ytid;
		m_settings.ytmode = cYTFeedParser::RELATED;
	}

	showYTMoviesMenu();
	
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
		movieInfo.file.Name = ylist[i].title;
		movieInfo.Url = ylist[i].GetUrl();
		
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

bool CYTBrowser::showYTMenu()
{
	CMenuWidget mainMenu(LOCALE_YOUTUBE, NEUTRINO_ICON_YT_SMALL, MENU_WIDTH + 100);
	mainMenu.disableMenuPosition();
	mainMenu.enableSaveScreen(true);

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
	
	CStringInputSMS stringInput(LOCALE_YT_SEARCH, &search);
	mainMenu.addItem(new CMenuForwarder(LOCALE_YT_SEARCH, true, search, &stringInput, NULL, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	
	mainMenu.addItem(new CMenuOptionChooser(LOCALE_YT_ORDERBY, &m_settings.ytorderby, YT_ORDERBY_OPTIONS, YT_ORDERBY_OPTION_COUNT, true, NULL, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN, true));

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

	mainMenu.exec(NULL, "");
	delete selector;
	delete keyInput;
	keyInput = NULL;

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
		
		printf("change region to %s\n", m_settings.ytregion.c_str());
	}
	
	return true;
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
	
	YTHandler->exec(NULL, "");
	
	delete YTHandler;
	YTHandler = NULL;	
}


