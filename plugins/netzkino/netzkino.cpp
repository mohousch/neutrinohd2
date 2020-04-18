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

	//
	mainWidget = NULL;
	leftWidget = NULL;
	rightWidget = NULL;
	headersWidget = NULL;
	footersWidget = NULL;

	item = NULL;

	//
	selected = 0;
	left_selected = 0;
	right_selected = 0;

	catMode = mode;
	catID = id;
	caption = title;
}

CNKMovies::~CNKMovies()
{
	dprintf(DEBUG_NORMAL, "~CNKMovies:\n");

	m_vMovieInfo.clear();
	nkparser.Cleanup();
	nksearch.clear();
}

void CNKMovies::hide()
{
	CFrameBuffer::getInstance()->paintBackground();
	CFrameBuffer::getInstance()->blit();
}

void CNKMovies::loadNKTitles(int mode, std::string search, int id)
{
	dprintf(DEBUG_NORMAL, "CNKMovies::loadNKTitles: (mode:%d) search:%s (id:%d)\n", mode, search.c_str(), id);

	CHintBox loadBox(LOCALE_NETZKINO, g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
	loadBox.paint();

	nkparser.Cleanup();

	//
	if (nkparser.ParseFeed((cNKFeedParser::nk_feed_mode_t)mode, search, id)) 
	{
		nkparser.DownloadThumbnails();
	} 
	else 
	{
		//FIXME show error
		MessageBox(LOCALE_MESSAGEBOX_ERROR, g_Locale->getText(LOCALE_NK_MOVIES_ERROR), mbrCancel, mbCancel, NEUTRINO_ICON_ERROR);
		
		return;
	}
	
	m_vMovieInfo.clear();
	nk_video_list_t &ylist = nkparser.GetVideoList();
	
	for (unsigned int count = 0; count < ylist.size(); count++) 
	{
		MI_MOVIE_INFO movieInfo;
		m_movieInfo.clearMovieInfo(&movieInfo); // refresh structure
		
		movieInfo.epgTitle = ylist[count].title;
		movieInfo.epgInfo2 = ylist[count].description;
		movieInfo.tfile = ylist[count].tfile;
		movieInfo.ytdate = ylist[count].published;
		movieInfo.ytid = ylist[count].id;
		movieInfo.file.Name = ylist[count].url;
		
		m_vMovieInfo.push_back(movieInfo);
	}

	loadBox.hide();
}

const struct button_label NKHeadButtons = { NEUTRINO_ICON_BUTTON_HELP, NONEXISTANT_LOCALE, NULL};

void CNKMovies::showMenu()
{
	dprintf(DEBUG_NORMAL, "CNKMovies::showMenu: mode:%d id:%d title:%s\n", catMode, catID, caption.c_str());
	
	if(m_vMovieInfo.empty())
	{
		MessageBox(LOCALE_MESSAGEBOX_ERROR, g_Locale->getText(LOCALE_NK_ERROR), mbrCancel, mbCancel, NEUTRINO_ICON_ERROR);
		
		return;
	}

	//
	std::string title;
	
	title = "NetzKino";
	title += ": ";
	title += caption;

	////
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

	headersWidget = new CHeaders(headBox.iX, headBox.iY, headBox.iWidth, headBox.iHeight, title.c_str(), NEUTRINO_ICON_NETZKINO_SMALL);

	headersWidget->setButtons(&NKHeadButtons, 1);
	headersWidget->enablePaintDate();
	headersWidget->setGradient(nogradient);
	headersWidget->setCorner(NO_RADIUS);

	// foot
	footBox.iWidth = frameBuffer->getScreenWidth();
	footBox.iHeight = 40;
	footBox.iX = frameBuffer->getScreenX();
	footBox.iY = frameBuffer->getScreenY() + frameBuffer->getScreenHeight() - footBox.iHeight;

	footersWidget = new CFooters(footBox.iX, footBox.iY, footBox.iWidth, footBox.iHeight);

	footersWidget->setGradient(nogradient);
	footersWidget->setCorner(NO_RADIUS);

	// leftwidget
	leftBox.iWidth = 280;
	leftBox.iHeight = mainWidget->getWindowsPos().iHeight - headBox.iHeight - 2*INTER_FRAME_SPACE - footBox.iHeight;
	leftBox.iX = mainWidget->getWindowsPos().iX;
	leftBox.iY = mainWidget->getWindowsPos().iY + headBox.iHeight + INTER_FRAME_SPACE;

	leftWidget = new ClistBox(&leftBox);

	leftWidget->setSelected(left_selected);
	leftWidget->enableShrinkMenu();

	leftWidget->addItem(new ClistBoxItem("Suche", true, nksearch.c_str(), this, "search"));
	leftWidget->addItem(new CMenuSeparator(LINE));

	if(cats.empty())
		cats = nkparser.GetCategoryList();

	// categories
	for (unsigned i = 0; i < cats.size(); i++)
	{
		leftWidget->addItem(new ClistBoxItem(cats[i].title.c_str(), true, NULL, new CNKMovies(cNKFeedParser::CATEGORY, cats[i].id, cats[i].title)));
	}

	leftWidget->addItem(new CMenuSeparator(LINE));
	leftWidget->addItem(new CMenuSeparator());
	leftWidget->addItem(new CMenuSeparator());
	leftWidget->addItem(new CMenuSeparator());
	leftWidget->addItem(new CMenuSeparator());
	leftWidget->addItem(new CMenuSeparator());
	leftWidget->addItem(new CMenuSeparator(LINE));
	leftWidget->addItem(new ClistBoxItem("Beenden", true, NULL, this, "exit"));

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

	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "play", RC_nokey, NULL, file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

		item->setHelpText(m_vMovieInfo[i].epgInfo2.c_str());

		rightWidget->addItem(item);
	}

	mainWidget->addItem(headersWidget);
	mainWidget->addItem(leftWidget);
	mainWidget->addItem(rightWidget);
	mainWidget->addItem(footersWidget);

	mainWidget->addKey(RC_info, this, CRCInput::getSpecialKeyName(RC_info));
	mainWidget->addKey(RC_record, this, CRCInput::getSpecialKeyName(RC_record));

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

void CNKMovies::playMovie(MI_MOVIE_INFO& movie)
{
	if (&movie.file != NULL) 
	{
		tmpMoviePlayerGui.addToPlaylist(movie);
		tmpMoviePlayerGui.exec(NULL, "");
	}
}

void CNKMovies::showMovieInfo(MI_MOVIE_INFO& movie)
{
	m_movieInfo.showMovieInfo(movie);
}

void CNKMovies::recordMovie(MI_MOVIE_INFO& movie)
{
	std::string infoString;

	MI_MOVIE_INFO g_movieInfo;
	m_movieInfo.clearMovieInfo(&g_movieInfo); // refresh structure
		
	g_movieInfo.epgTitle = movie.epgTitle;
	g_movieInfo.epgInfo2 = movie.epgInfo2;
	g_movieInfo.tfile = movie.tfile;
	g_movieInfo.ytdate = movie.ytdate;
	g_movieInfo.ytid = movie.ytid;
	g_movieInfo.file.Name = movie.file.Name;

	m_movieInfo.encodeMovieInfoXml(&infoString, &g_movieInfo);

	::start_file_recording(movie.epgTitle.c_str(), infoString.c_str(), movie.file.Name.c_str());
}

int CNKMovies::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CNKMovies::exec: actionKey:%s\n", actionKey.c_str());

	if(parent) 
		parent->hide();

	if(actionKey == "play")
	{
		right_selected = rightWidget->getSelected();
		playMovie(m_vMovieInfo[right_selected]);

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "RC_info")
	{
		right_selected = rightWidget->getSelected();
		showMovieInfo(m_vMovieInfo[right_selected]);

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "RC_record")
	{
		right_selected = rightWidget->getSelected();
		recordMovie(m_vMovieInfo[right_selected]);
		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "search")
	{
		nksearch.clear();

		CStringInputSMS stringInput(LOCALE_YT_SEARCH, nksearch.c_str());
		int ret = stringInput.exec(NULL, "");

		printf("ret:%d nksearch:%s\n", ret, nksearch.c_str());

		if(!stringInput.getExitPressed()) //FIXME:
		{
			loadNKTitles(cNKFeedParser::SEARCH, nksearch, 0);

			rightWidget->clearItems();
			showMenu();

			return menu_return::RETURN_EXIT_ALL;
		}
		else
			return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "exit")
	{
		return menu_return::RETURN_EXIT_ALL;
	}

	loadNKTitles(catMode, caption, catID);
	showMenu();
	
	return menu_return::RETURN_EXIT;
}

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


