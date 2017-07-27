 /*
  $Id: netzkino.cpp 2016.02.13 mohousch Exp $

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

#include <netzkino.h>

extern "C" void plugin_exec(void);
extern "C" void plugin_init(void);
extern "C" void plugin_del(void);

#define NEUTRINO_ICON_NETZKINO			PLUGINDIR "/netzkino/netzkino.png"
#define NEUTRINO_ICON_NETZKINO_SMALL		PLUGINDIR "/netzkino/netzkino_small.png"
  
//
CNKMovies::CNKMovies(int mode, int id, std::string title)
{
	dprintf(DEBUG_NORMAL, "CNKMovies: mode:%d id:%d title:%s\n", mode, id, title.c_str());

	catMode = mode;
	catID = id;
	caption = title;
}

CNKMovies::~CNKMovies()
{
	dprintf(DEBUG_NORMAL, "~CNKMovies:\n");

	m_vMovieInfo.clear();
	nkparser.Cleanup();
}

#define NK_HEAD_BUTTONS_COUNT	2
const struct button_label NKHeadButtons[NK_HEAD_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_HELP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_SETUP, NONEXISTANT_LOCALE, NULL}
};

void CNKMovies::showNKMoviesMenu()
{
	dprintf(DEBUG_NORMAL, "CNKMovies::showNKMoviesMenu: mode:%d id:%d title:%s\n", catMode, catID, caption.c_str());

	//
	CHintBox loadBox(LOCALE_NETZKINO, g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
	
	loadBox.paint();

	// loadNKTitles
	loadNKTitles(catMode, caption, catID);

	loadBox.hide();
	
	if(m_vMovieInfo.empty())
	{
		MessageBox(LOCALE_MESSAGEBOX_ERROR, g_Locale->getText(LOCALE_NK_ERROR), CMessageBox::mbrCancel, CMessageBox::mbCancel, NEUTRINO_ICON_ERROR);
		
		return;
	}

	//
	std::string title;
	
	title = "NetzKino";
	title += ": ";
	title += caption;

	moviesMenu = new CMenuFrameBox(title.c_str(), NEUTRINO_ICON_NETZKINO_SMALL);

	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		moviesMenu->addItem(new CMenuFrameBoxItem(m_vMovieInfo[i].epgTitle.c_str(), this, "play", file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg"));
	}

	//moviesMenu->setItemsPerPage(3, 2);
	moviesMenu->setItemBoxColor(COL_YELLOW);
	moviesMenu->setHeaderButtons(NKHeadButtons, NK_HEAD_BUTTONS_COUNT);

	moviesMenu->addKey(CRCInput::RC_info, this, CRCInput::getSpecialKeyName(CRCInput::RC_info));
	moviesMenu->addKey(CRCInput::RC_record, this, CRCInput::getSpecialKeyName(CRCInput::RC_record));
	moviesMenu->addKey(CRCInput::RC_setup, this, CRCInput::getSpecialKeyName(CRCInput::RC_setup));

	moviesMenu->exec(NULL, "");
	//moviesMenu->hide();
	delete moviesMenu;
	moviesMenu = NULL;
}

void CNKMovies::playMovie(void)
{
	CMoviePlayerGui tmpMoviePlayerGui;

	if (&m_vMovieInfo[moviesMenu->getSelected()].file != NULL) 
	{
		tmpMoviePlayerGui.addToPlaylist(m_vMovieInfo[moviesMenu->getSelected()]);
		tmpMoviePlayerGui.exec(NULL, "urlplayback");
	}
}

void CNKMovies::showMovieInfo(void)
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

void CNKMovies::showNKCategoriesMenu()
{
	dprintf(DEBUG_NORMAL, "CNKMovies::showNKCategoriesMenu:\n");

	// load Categories
	CHintBox loadBox(LOCALE_NETZKINO, g_Locale->getText(LOCALE_NK_SCAN_FOR_CATEGORIES));
	
	loadBox.paint();

	cats = nkparser.GetCategoryList();

	loadBox.hide();

	if(cats.empty())
	{
		//FIXME show error
		MessageBox(LOCALE_MESSAGEBOX_ERROR, g_Locale->getText(LOCALE_NK_ERROR), CMessageBox::mbrCancel, CMessageBox::mbCancel, NEUTRINO_ICON_ERROR);
		
		return;
	}

	// menu
	CMenuWidget mainMenu(LOCALE_NETZKINO, NEUTRINO_ICON_NETZKINO_SMALL);

	mainMenu.disableMenuPosition();

	// categories
	for (unsigned i = 0; i < cats.size(); i++)
	{
		mainMenu.addItem(new CMenuForwarder(cats[i].title.c_str(), true, NULL, new CNKMovies(cNKFeedParser::CATEGORY, cats[i].id, cats[i].title), "", CRCInput::RC_nokey, NEUTRINO_ICON_NETZKINO));
	}

	// search
	//mainMenu.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	int select = -1;
	CMenuSelectorTarget * selector = new CMenuSelectorTarget(&select);

	std::string search;
	CStringInputSMS * stringInput = new CStringInputSMS(LOCALE_YT_SEARCH, &search);
	mainMenu.addItem(new CMenuForwarder(LOCALE_YT_SEARCH, true, search, stringInput, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_NETZKINO));

	mainMenu.addItem(new CMenuForwarder(LOCALE_EVENTFINDER_START_SEARCH, true, NULL, selector, to_string(cNKFeedParser::SEARCH).c_str(), CRCInput::RC_nokey, NEUTRINO_ICON_NETZKINO));

	mainMenu.exec(NULL, "");
	delete selector;
	delete stringInput;

	if(select == cNKFeedParser::SEARCH)
	{
		CNKMovies * test = new CNKMovies(cNKFeedParser::SEARCH, 0, search);
		test->exec(NULL, "");
		//test->hide();
		delete test;
		test = NULL;
	}
}

int CNKMovies::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CNKMovies::exec: actionKey:%s\n", actionKey.c_str());

	int returnval = menu_return::RETURN_REPAINT;

	if(parent) 
		parent->hide();

	if(actionKey == "play")
	{
		playMovie();

		return returnval;
	}
	else if(actionKey == "RC_info")
	{
		showMovieInfo();

		return returnval;
	}
	else if(actionKey == "RC_setup")
	{
		showNKCategoriesMenu();

		return returnval;
	}
	else if(actionKey == "RC_record")
	{
		nkparser.downloadMovie(m_vMovieInfo[moviesMenu->getSelected()].file.Name, m_vMovieInfo[moviesMenu->getSelected()].Url);

		return returnval;
	}

	showNKMoviesMenu();
	
	return menu_return::RETURN_EXIT;
}

void CNKMovies::loadNKTitles(int mode, std::string search, int id)
{
	dprintf(DEBUG_NORMAL, "CNKMovies::loadNKTitles:\n");

	nkparser.Cleanup();

	//
	if (nkparser.ParseFeed((cNKFeedParser::nk_feed_mode_t)mode, search, id)) 
	{
		nkparser.DownloadThumbnails();
	} 
	else 
	{
		//FIXME show error
		MessageBox(LOCALE_MESSAGEBOX_ERROR, g_Locale->getText(LOCALE_NK_ERROR), CMessageBox::mbrCancel, CMessageBox::mbCancel, NEUTRINO_ICON_ERROR);
		
		return;
	}
	
	m_vMovieInfo.clear();
	nk_video_list_t &ylist = nkparser.GetVideoList();
	
	for (unsigned int count = 0; count < ylist.size(); count++) 
	{
		MI_MOVIE_INFO movieInfo;
		m_movieInfo.clearMovieInfo(&movieInfo); // refresh structure
		
		movieInfo.epgTitle = ylist[count].title;
		//movieInfo.epgInfo1 = "";
		movieInfo.epgInfo2 = ylist[count].description;
		movieInfo.tfile = ylist[count].tfile;
		movieInfo.ytdate = ylist[count].published;
		movieInfo.ytid = ylist[count].id;
		movieInfo.file.Name = ylist[count].title;
		movieInfo.Url = ylist[count].url;
		
		m_vMovieInfo.push_back(movieInfo);
	}
}
//

// plugin API
void plugin_init(void)
{
}

void plugin_del(void)
{
}

void plugin_exec(void)
{
	CNKMovies * NKHandler = new CNKMovies(cNKFeedParser::CATEGORY, 8, "Highlights");
	
	NKHandler->exec(NULL, "");
	
	delete NKHandler;
	NKHandler = NULL;			
}


