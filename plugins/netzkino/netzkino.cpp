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

#define NEUTRINO_ICON_NETZKINO_SMALL		PLUGINDIR "/netzkino/netzkino_small.png"
 
CNetzKinoBrowser::CNetzKinoBrowser()
{
	dprintf(DEBUG_NORMAL, "$Id: CNetzKinoBrowser: 2016.02.13 11:16:30 mohousch Exp $\n");
	init();
}

CNetzKinoBrowser::~CNetzKinoBrowser()
{
	dprintf(DEBUG_NORMAL, "CNetzKinoBrowser: del\n");
	
	m_vMovieInfo.clear();
	nkparser.Cleanup();
}

void CNetzKinoBrowser::init(void)
{
	dprintf(DEBUG_NORMAL, "CNetzKinoBrowser::init\n");
	
	initGlobalSettings();

	/*
	initFrameBox();

	m_pcWindow = CFrameBuffer::getInstance();

	initFrames();
	*/

	reload_movies = true;
}

void CNetzKinoBrowser::initGlobalSettings(void)
{
	dprintf(DEBUG_NORMAL, "CNetzKinoBrowser::initGlobalSettings\n");
	
	// netzkino
	m_settings.nkmode = cNKFeedParser::CATEGORY;
	m_settings.nkcategory = 8;	//8=Highlights, 81=neu bei Netzkino
	m_settings.nkcategoryname = "Highlights";

	/*
	NKStart = 0;
	NKEnd = MAX_ITEMS_PER_PAGE;

	itemsCountPerPage = MAX_ITEMS_PER_PAGE;
	*/
}

/*
void CNetzKinoBrowser::initFrameBox(void)
{
	dprintf(DEBUG_NORMAL, "CNetzKinoBrowser::initFrameBox\n");

	selected = 0;
	oldselected = 0;

	x = 0;
	y = 0;	
}

void CNetzKinoBrowser::initFrames(void)
{
	dprintf(DEBUG_NORMAL, "CNetzKinoBrowser::initFrames\n");

	Box.iX = g_settings.screen_StartX + 20;
	Box.iY = g_settings.screen_StartY + 20;
	Box.iWidth = g_settings.screen_EndX - g_settings.screen_StartX - 40;
	Box.iHeight = (g_settings.screen_EndY - g_settings.screen_StartY - 40);

	//
	frameBox.iX = Box.iX + BORDER_LEFT;
	frameBox.iY = Box.iY + 35 + 5;
	frameBox.iWidth = (Box.iWidth - (BORDER_LEFT + BORDER_RIGHT))/6;
	frameBox.iHeight = (Box.iHeight - 80)/3;
}

void CNetzKinoBrowser::paintHead(void)
{
	dprintf(DEBUG_NORMAL, "CNetzKinoBrowser::paintHead\n");

	// title
	std::string title;
	std::string mb_icon;
	
	title = g_Locale->getText(LOCALE_NETZKINO);
	if (m_settings.nkmode == cNKFeedParser::SEARCH) 
	{
		title += ": ";
		title += g_Locale->getText(LOCALE_YT_SEARCH);
		title += " \"" + m_settings.nksearch + "\"";
	} 
	else if (m_settings.nkmode == cNKFeedParser::CATEGORY) 
	{
		title += ": ";
		title += m_settings.nkcategoryname;
	}
		
	mb_icon = NEUTRINO_ICON_NETZKINO_SMALL;
	//
	
	// netzkino icon
	int icon_w, icon_h;
	m_pcWindow->getIconSize(mb_icon.c_str(), &icon_w, &icon_h);
	m_pcWindow->paintIcon(mb_icon, Box.iX + BORDER_LEFT, Box.iY + (35 - icon_h)/2);

	// setup icon
	m_pcWindow->getIconSize(NEUTRINO_ICON_BUTTON_SETUP, &icon_w, &icon_h);
	int xpos1 = Box.iX + Box.iWidth - BORDER_RIGHT;
	int ypos = Box.iY + (35 - icon_w)/2;

	m_pcWindow->paintIcon(NEUTRINO_ICON_BUTTON_SETUP, xpos1 - icon_w, ypos);

	// help icon
	int icon_h_w, icon_h_h;
	m_pcWindow->getIconSize(NEUTRINO_ICON_BUTTON_SETUP, &icon_h_w, &icon_h_h);
	m_pcWindow->paintIcon(NEUTRINO_ICON_BUTTON_HELP, xpos1 - icon_w - 2 - icon_h_w, ypos);
	///

	// title
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(Box.iX + BORDER_LEFT + ICON_OFFSET + icon_w, Box.iY + (35 - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), Box.iWidth - (BORDER_LEFT + BORDER_RIGHT + ICON_OFFSET + 2*icon_w), title.c_str(), COL_MENUHEAD);
}

void CNetzKinoBrowser::paintFoot(void)
{
	dprintf(DEBUG_NORMAL, "CNetzKinoBrowser::paintFoot\n");

	// paint buttons
	int iw, ih;
	
	// right button
	m_pcWindow->getIconSize(NEUTRINO_ICON_BUTTON_RIGHT, &iw, &ih);
	m_pcWindow->paintIcon(NEUTRINO_ICON_BUTTON_RIGHT, Box.iX + Box.iWidth - BORDER_RIGHT - iw, Box.iY + Box.iHeight - 35 + (35 - ih)/2);
	
	// left button
	m_pcWindow->getIconSize(NEUTRINO_ICON_BUTTON_LEFT, &iw, &ih);
	m_pcWindow->paintIcon(NEUTRINO_ICON_BUTTON_LEFT, Box.iX + Box.iWidth - BORDER_RIGHT - 2*iw - 2, Box.iY + Box.iHeight - 35 + (35 - ih)/2);

	// red button
	m_pcWindow->getIconSize(NEUTRINO_ICON_BUTTON_RED, &iw, &ih);
	m_pcWindow->paintIcon(NEUTRINO_ICON_BUTTON_RED, Box.iX + Box.iWidth/2, Box.iY + Box.iHeight - 35 + (35 - ih)/2);
	g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(Box.iX + Box.iWidth/2 + iw + 2, Box.iY + Box.iHeight - 35 + (35 - g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight(), Box.iWidth/4 - iw, g_Locale->getText(LOCALE_YT_PREV_RESULTS), COL_MENUHEAD);

	// green button
	m_pcWindow->getIconSize(NEUTRINO_ICON_BUTTON_GREEN, &iw, &ih);
	m_pcWindow->paintIcon(NEUTRINO_ICON_BUTTON_GREEN, Box.iX + Box.iWidth/2 + Box.iWidth/4, Box.iY + Box.iHeight - 35 + (35 - ih)/2);
	g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(Box.iX + Box.iWidth/2 + Box.iWidth/4 + iw + 2, Box.iY + Box.iHeight - 35 + (35 - g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight(), Box.iWidth/4 - iw, g_Locale->getText(LOCALE_YT_NEXT_RESULTS), COL_MENUHEAD);
}

void CNetzKinoBrowser::paintBody(void)
{
	dprintf(DEBUG_NORMAL, "CNetzKinoBrowser::paintBody\n");

	// paint background
	m_pcWindow->paintBoxRel(Box.iX, Box.iY, Box.iWidth, Box.iHeight, COL_BACKGROUND_PLUS_0);
	
	// paint horizontal line top
	m_pcWindow->paintHLineRel(Box.iX + BORDER_LEFT, Box.iWidth - (BORDER_LEFT + BORDER_RIGHT), Box.iY + 35, COL_MENUCONTENT_PLUS_5);
	
	// paint horizontal line bottom
	m_pcWindow->paintHLineRel(Box.iX + BORDER_LEFT, Box.iWidth - (BORDER_LEFT + BORDER_RIGHT), Box.iY + Box.iHeight - 35, COL_MENUCONTENT_PLUS_5);
}

void CNetzKinoBrowser::paintItems(int itemsCount)
{
	dprintf(DEBUG_NORMAL, "CNetzKinoBrowser::paintItems\n");

	if(itemsCount > MAX_ITEMS_PER_PAGE)
		itemsCount = MAX_ITEMS_PER_PAGE;

	std::string fname;

	fname = PLUGINDIR "/netzkino/nopreview.jpg";
	int k = 0;
	for (unsigned int _y = 0; _y < 3; _y++)
	{
		for (unsigned int _x = 0; _x < 6; _x++)
		{
			m_pcWindow->DisplayImage(file_exists(m_vMovieInfo[k].tfile.c_str())? m_vMovieInfo[k].tfile : fname, frameBox.iX + _x*frameBox.iWidth + 5, frameBox.iY + _y*frameBox.iHeight + 5, frameBox.iWidth - 10, frameBox.iHeight - 10);

			k++;

			if( k == itemsCount)
				break;	
		}

		if( k == itemsCount)
			break;	
	}
}

void CNetzKinoBrowser::paintInfo(void)
{
	dprintf(DEBUG_DEBUG, "CNetzKinoBrowser::paintInfo\n");

	// refresh
	m_pcWindow->paintBoxRel(Box.iX, Box.iY + Box.iHeight - 30, Box.iWidth/2, 30, COL_BACKGROUND_PLUS_0);

	// info (foot)
	g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(Box.iX, Box.iY + Box.iHeight - 35 + (35 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), Box.iWidth/2, m_vMovieInfo[selected].epgTitle, COL_MENUHEAD);
}

void CNetzKinoBrowser::paintItemBox(int oldposx, int oldposy, int posx, int posy)
{
	dprintf(DEBUG_DEBUG, "CNetzKinoBrowser::paintItemBox\n");

	//refresh prev item
	m_pcWindow->paintBoxRel(frameBox.iX + frameBox.iWidth*oldposx, frameBox.iY + frameBox.iHeight*oldposy, frameBox.iWidth, frameBox.iHeight, COL_BACKGROUND_PLUS_0);
	std::string fname;

	fname = PLUGINDIR "/netzkino/nopreview.jpg";

	m_pcWindow->DisplayImage(file_exists(m_vMovieInfo[oldselected].tfile.c_str())? m_vMovieInfo[oldselected].tfile : fname, frameBox.iX + oldposx*frameBox.iWidth + 5, frameBox.iY + oldposy*frameBox.iHeight + 5, frameBox.iWidth - 10, frameBox.iHeight - 10);

	// itembox
	m_pcWindow->paintBoxRel(frameBox.iX + frameBox.iWidth*posx, frameBox.iY + frameBox.iHeight*posy, frameBox.iWidth, frameBox.iHeight, COL_YELLOW, RADIUS_SMALL, CORNER_BOTH);

	m_pcWindow->DisplayImage(file_exists(m_vMovieInfo[selected].tfile.c_str())? m_vMovieInfo[selected].tfile : fname, frameBox.iX + posx*frameBox.iWidth + 5, frameBox.iY + posy*frameBox.iHeight + 5, frameBox.iWidth - 10, frameBox.iHeight - 10);
}
*/

/*
void CNetzKinoBrowser::paint(int itemsCount)
{
	dprintf(DEBUG_NORMAL, "CNetzKinoBrowser::paint\n");

	// body
	paintBody();

	// head
	paintHead();
	
	// foot
	paintFoot();
	
	// itembox
	paintItemBox(x, y, x, y);
	
	// items
	paintItems(itemsCount);

	// info
	paintInfo();

	netzKino = new CSmartMenu("Netz Kino", NEUTRINO_ICON_NETZKINO_SMALL);

	std::string fname = PLUGINDIR "/netzkino/nopreview.jpg";

	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		netzKino->addItem(new CMenuFrameBox(m_vMovieInfo[i].epgTitle.c_str(), this, "play", file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : fname.c_str()));
	}

	netzKino->exec(NULL, "");
	netzKino->hide();
	delete netzKino;
	netzKino = NULL;
}
*/

int CNetzKinoBrowser::exec(CMenuTarget * parent, const std::string & actionKey)
{
	dprintf(DEBUG_NORMAL, "CNetzKinoBrowser::exec: actionKey:\n");

	int returnval = menu_return::RETURN_REPAINT;

	/*
	if(actionKey == "run")
	{
		if(parent) 
			parent->hide ();
		
		exec();
	}
	*/

	if(parent) 
		parent->hide();

	if(actionKey == "play")
	{
		CFile *file;
		CMoviePlayerGui tmpMoviePlayerGui;
		MI_MOVIE_INFO * p_movie_info;

		if ((file = /*getSelectedFile()*/&m_vMovieInfo[netzKino->getSelected()].file) != NULL) 
		{
			p_movie_info = /*getCurrentMovieInfo()*/&m_vMovieInfo[netzKino->getSelected()];
			
			file->Title = p_movie_info->epgTitle;
			file->Info1 = p_movie_info->epgInfo1;
			file->Info2 = p_movie_info->epgInfo2;
			file->Thumbnail = p_movie_info->tfile;
	
			tmpMoviePlayerGui.addToPlaylist(*file);
			tmpMoviePlayerGui.exec(NULL, "urlplayback");
		}
	}

	showNKMenu();
	
	return returnval;
}

/*
void CNetzKinoBrowser::hide(void)
{
	dprintf(DEBUG_NORMAL, "CNetzKinoBrowser::hide\n");
	
	m_pcWindow->paintBackground();
	m_pcWindow->blit();
}
*/

void CNetzKinoBrowser::loadMovies(void)
{
	dprintf(DEBUG_NORMAL, "CNetzKinoBrowser::loadMovies:\n");
	
	//first clear screen
	//m_pcWindow->paintBackground();
	//m_pcWindow->blit();	

	CHintBox loadBox(LOCALE_NETZKINO, g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
	
	loadBox.paint();

	loadNKTitles(m_settings.nkmode, m_settings.nksearch, m_settings.nkcategory);
	
	loadBox.hide();

	reload_movies = false;

	//
	std::string title;
	
	title = g_Locale->getText(LOCALE_NETZKINO);
	if (m_settings.nkmode == cNKFeedParser::SEARCH) 
	{
		title += ": ";
		title += g_Locale->getText(LOCALE_YT_SEARCH);
		title += " \"" + m_settings.nksearch + "\"";
	} 
	else if (m_settings.nkmode == cNKFeedParser::CATEGORY) 
	{
		title += ": ";
		title += m_settings.nkcategoryname;
	}

	netzKino = new CSmartMenu(title.c_str(), NEUTRINO_ICON_NETZKINO_SMALL);

	std::string fname = PLUGINDIR "/netzkino/nopreview.jpg";

	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		netzKino->addItem(new CMenuFrameBox(m_vMovieInfo[i].epgTitle.c_str(), this, "play", file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : fname.c_str()));
	}

	netzKino->exec(NULL, "");
	netzKino->hide();
	delete netzKino;
	netzKino = NULL;
	//
}

//netzkino
void CNetzKinoBrowser::loadNKTitles(int mode, std::string search, int id/*, unsigned int start, unsigned int end*/)
{
	dprintf(DEBUG_NORMAL, "CNetzKinoBrowser::loadNKTitles:\n");

	//
	if (nkparser.ParseFeed((cNKFeedParser::nk_feed_mode_t)mode, search, id)) 
	{
		nkparser.DownloadThumbnails(/*start, end*/);
	} 
	else 
	{
		//FIXME show error
		MessageBox(LOCALE_MESSAGEBOX_ERROR, g_Locale->getText(LOCALE_NK_ERROR), CMessageBox::mbrCancel, CMessageBox::mbCancel, NEUTRINO_ICON_ERROR);
		
		reload_movies = true;
		return;
	}
	
	m_vMovieInfo.clear();
	nk_video_list_t &ylist = nkparser.GetVideoList();
	
	//
	videoListsize = ylist.size();
	
	//for (unsigned int count = start; count < end && end <= ylist.size(); count++) 
	for (unsigned int count = 0; count < ylist.size(); count++) 
	{
		MI_MOVIE_INFO movieInfo;
		m_movieInfo.clearMovieInfo(&movieInfo); // refresh structure
		
		movieInfo.epgTitle = ylist[count].title;
		movieInfo.epgInfo1 = m_settings.nkcategoryname;
		movieInfo.epgInfo2 = ylist[count].description;
		movieInfo.tfile = ylist[count].tfile;
		movieInfo.ytdate = ylist[count].published;
		movieInfo.ytid = ylist[count].id;
		movieInfo.file.Name = ylist[count].title;
		movieInfo.file.Url = ylist[count].url;
		
		m_vMovieInfo.push_back(movieInfo);
	}
}

#if 0
class CNKCategoriesMenu : public CMenuTarget
{
	private:
		int *nkmode;
		int *nkcategory;
		std::string *nkcategoryname;
		cNKFeedParser *nkparser;
	public:
		CNKCategoriesMenu(int &_nkmode, int &_nkcategory, std::string &_nkcategoryname, cNKFeedParser &_nkparser);
		int exec(CMenuTarget* parent, const std::string & actionKey);
};

CNKCategoriesMenu::CNKCategoriesMenu(int &_nkmode, int &_nkcategory, std::string &_nkcategoryname, cNKFeedParser &_nkparser)
{
	nkmode = &_nkmode;
	nkcategory = &_nkcategory;
	nkcategoryname = &_nkcategoryname;
	nkparser = &_nkparser;
}

int CNKCategoriesMenu::exec(CMenuTarget *parent, const std::string &actionKey)
{
	nk_category_list_t cats = nkparser->GetCategoryList();
	
	if (!cats.size())
		return menu_return::RETURN_NONE;
	
	if (actionKey != "") 
	{
		unsigned int i = atoi(actionKey);
		if (i < cats.size()) 
		{
			*nkmode = cNKFeedParser::CATEGORY;
			*nkcategory = cats[i].id;
			*nkcategoryname = cats[i].title;
		}
		g_RCInput->postMsg(CRCInput::RC_home, 0);
		return menu_return::RETURN_EXIT;
	}

	if(parent)
		parent->hide();

	CMenuWidget m(LOCALE_NK_CATEGORIES, NEUTRINO_ICON_NETZKINO_SMALL);
	m.disableMenuPosition();

	for (unsigned i = 0; i < cats.size(); i++)
	{
		m.addItem(new CMenuForwarder(cats[i].title.c_str(), true, /*("(" + to_string(cats[i].post_count) + ")").c_str()*/NULL, this, to_string(i).c_str(), CRCInput::RC_nokey, PLUGINDIR "/netzkino/netzkino.png"), cats[i].id == *nkcategory);
	}

	m.exec(NULL, "");

	return menu_return::RETURN_REPAINT;
}
#endif
  
bool CNetzKinoBrowser::showNKMenu()
{
	dprintf(DEBUG_NORMAL, "CNetzKinoBrowser::showNKMenu:\n");

	//m_pcWindow->paintBackground();

	CMenuWidget mainMenu(LOCALE_NETZKINO, NEUTRINO_ICON_NETZKINO_SMALL);
	mainMenu.disableMenuPosition();

	//
	nk_category_list_t cats = nkparser.GetCategoryList();

	for (unsigned i = 0; i < cats.size(); i++)
	{
		mainMenu.addItem(new CMenuForwarder(cats[i].title.c_str(), true, /*("(" + to_string(cats[i].post_count) + ")").c_str()*/NULL, this, to_string(i).c_str(), CRCInput::RC_nokey, PLUGINDIR "/netzkino/netzkino.png"), cats[i].id == m_settings.nkcategory);
	}

	mainMenu.exec(NULL, "");
	//

	/*
	int select = -1;
	CMenuSelectorTarget * selector = new CMenuSelectorTarget(&select);

	CNKCategoriesMenu nkCategoriesMenu(m_settings.nkmode, m_settings.nkcategory, m_settings.nkcategoryname, nkparser);
	
	mainMenu.addItem(new CMenuForwarder(LOCALE_NK_CATEGORIES, true, m_settings.nkcategoryname, &nkCategoriesMenu));
	mainMenu.addItem(new CMenuSeparator(CMenuSeparator::LINE));

	std::string search = m_settings.nksearch;
	CStringInputSMS stringInput(LOCALE_YT_SEARCH, &search);
	mainMenu.addItem(new CMenuForwarder(LOCALE_YT_SEARCH, true, search, &stringInput, NULL, CRCInput::RC_nokey, ""));

	mainMenu.addItem(new CMenuForwarder(LOCALE_EVENTFINDER_START_SEARCH, true, NULL, selector, to_string(cNKFeedParser::SEARCH).c_str(), CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));

	int oldcat = m_settings.nkcategory;
	int oldmode = m_settings.nkmode;
	*/

	mainMenu.exec(NULL, "");

	//delete selector;

	/*
	bool reload = true;
	
	dprintf(DEBUG_NORMAL, "select:%d\n", select);
	
	if (select == cNKFeedParser::SEARCH) 
	{
		dprintf(DEBUG_NORMAL, "search for: %s\n", search.c_str());
		
		if (!search.empty()) 
		{
			//reload = true;
			m_settings.nksearch = search;
			m_settings.nkmode = cNKFeedParser::SEARCH;
		}
	}
	else if (oldmode != m_settings.nkmode || oldcat != m_settings.nkcategory) 
	{
		reload = true;
	}
	*/
	
	//if (reload) 
	{
		/*
		CHintBox loadBox(LOCALE_NETZKINO, g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
		loadBox.paint();
		nkparser.Cleanup();
		loadNKTitles(m_settings.nkmode, m_settings.nksearch, m_settings.nkcategory);
		loadBox.hide();
		*/

		loadMovies();
	}
	
	return true;
}

/*
int CNetzKinoBrowser::exec()
{
	dprintf(DEBUG_NORMAL, "CNetzKinoBrowser::exec:\n");

	bool res = false;

	loadMovies();

	if(videoListsize > MAX_ITEMS_PER_PAGE)
		itemsCountPerPage = MAX_ITEMS_PER_PAGE;
	else
		itemsCountPerPage = videoListsize;

	initFrameBox();
	
	paint(getItemsCountPerPage());
	
	// blit all
	m_pcWindow->blit();
	
	// loop
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;
	
	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);

	bool loop = true;
	while (loop) 
	{
		g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd);

		if ((msg == CRCInput::RC_timeout ) || (msg == CRCInput::RC_home))
		{
			loop = false;
		}
		else if(msg == CRCInput::RC_ok)
		{
			res = true;
			loop = false;
		}
		else if (msg == CRCInput::RC_right)
		{
			oldselected = selected;
			int oldx = x;
			int oldy = y;
			int step = 1;

			selected += step;

			if(selected >= getItemsCountPerPage())
				selected = 0;

			x += step;

			if(getItemsCountPerPage() <= MAX_ITEMS_PER_X)
			{
				if(x >= getItemsCountPerPage())
				{
					x = 0; 
				}
			}
			//FIXME:
			else if(getItemsCountPerPage() > MAX_ITEMS_PER_X && getItemsCountPerPage() <= 2*MAX_ITEMS_PER_X)
			{
				if(y == 1 && x >= (getItemsCountPerPage() - MAX_ITEMS_PER_X))
				{
					x = 0;
					y = 0; 
				}
				else if(x >= MAX_ITEMS_PER_X)
				{
					x = 0;
					y += step; // increase j
				
					if(y >= MAX_ITEMS_PER_Y - 1)
						y = 0;
				}
			}
			else if(getItemsCountPerPage() > 2*MAX_ITEMS_PER_X && getItemsCountPerPage() <= MAX_ITEMS_PER_PAGE)
			{
				if(y == 2 && x >= (getItemsCountPerPage() - 2*MAX_ITEMS_PER_X))
				{
					x = 0;
					y = 0; 
				}
				else if(x >= MAX_ITEMS_PER_X)
				{
					x = 0;
					y += step;
				
					if(y >= MAX_ITEMS_PER_Y)
						y = 0;
				}
			}
			
			paintItemBox(oldx, oldy, x, y);
			paintInfo();
		}
		else if (msg == CRCInput::RC_left)
		{
			oldselected = selected;
			int oldx = x;
			int oldy = y;
			int step = 1;

			selected -= step;
			if (selected < 0)
				selected = 0;

			x -= step;

			if(x < 0 && y > 0)
			{
				x = MAX_ITEMS_PER_X - 1;
				y--;
				
				if(y < 0)
					y = 0;
			}
			
			// stay at first framBox
			if (x < 0)
				x = 0;
			
			paintItemBox(oldx, oldy, x, y);
			paintInfo();
		}
		else if (msg == CRCInput::RC_red || msg == CRCInput::RC_page_up) 
		{	
			if(NKStart >= MAX_ITEMS_PER_PAGE)
			{
				if(NKEnd == videoListsize)
					NKEnd = videoListsize - NKStart;
				else
					NKEnd -= MAX_ITEMS_PER_PAGE;

				// check NKEnd
				if(NKEnd < MAX_ITEMS_PER_PAGE)
					NKEnd = MAX_ITEMS_PER_PAGE;

				NKStart -= MAX_ITEMS_PER_PAGE;

				// check NKStart
				if (NKStart < 0)
					NKStart = 0;
				
				m_pcWindow->paintBackground();
					
				//
				CHintBox loadBox(LOCALE_NETZKINO, g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
				loadBox.paint();
				
				nkparser.CleanupThumbnails();
				loadNKTitles(m_settings.nkmode, m_settings.nksearch, m_settings.nkcategory, NKStart, NKEnd);
				
				loadBox.hide();

				itemsCountPerPage = NKEnd - NKStart;
				
				initFrameBox();

				paint(getItemsCountPerPage());
			}
		}
		else if (msg == CRCInput::RC_green || msg == CRCInput::RC_page_down) 
		{
			if(NKEnd < videoListsize)
			{
				NKEnd += MAX_ITEMS_PER_PAGE;

				// check if NKEnd
				if(NKEnd > videoListsize)
					NKEnd = videoListsize;

				NKStart += MAX_ITEMS_PER_PAGE;
				
				m_pcWindow->paintBackground();
					
				//
				CHintBox loadBox(LOCALE_NETZKINO, g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
				loadBox.paint();
				
				nkparser.CleanupThumbnails();
				loadNKTitles(m_settings.nkmode, m_settings.nksearch, m_settings.nkcategory, NKStart, NKEnd);
				
				loadBox.hide();

				itemsCountPerPage = NKEnd - NKStart;

				initFrameBox();
					
				paint(getItemsCountPerPage());
			}	
		}
		else if (msg == CRCInput::RC_blue) 
		{
			nkparser.Cleanup();
			initGlobalSettings();
				
			loadMovies();

			if(videoListsize > MAX_ITEMS_PER_PAGE)
				itemsCountPerPage = MAX_ITEMS_PER_PAGE;
			else
				itemsCountPerPage = videoListsize;

			initFrameBox();	

			paint(getItemsCountPerPage());
		}
		else if ( msg == CRCInput::RC_info) 
		{
			m_pcWindow->paintBackground();
			m_pcWindow->blit();
		  
			m_movieInfo.showMovieInfo(m_vMovieInfo[selected]);
			
			paint(getItemsCountPerPage());
		}
		else if (msg == CRCInput::RC_setup) 
		{
			NKStart = 0;
			NKEnd = MAX_ITEMS_PER_PAGE;

			itemsCountPerPage = MAX_ITEMS_PER_PAGE;

			showNKMenu();

			initFrameBox();	
			paint(getItemsCountPerPage());
		}
		else if (CNeutrinoApp::getInstance()->handleMsg(msg, data) & messages_return::cancel_all)
		{
			dprintf(DEBUG_NORMAL, "CNetzKinoBrowser::exec: getInstance\n");
				
			loop = false;
		}

		m_pcWindow->blit();
	}
	
	// hide and exit
	hide();
	
	return (res);
}

CFile * CNetzKinoBrowser::getSelectedFile(void)
{
	dprintf(DEBUG_INFO, "CNetzKinoBrowser::getSelectedFile: %s\n", m_vMovieInfo[selected].file.Name.c_str());

	if ((!(m_vMovieInfo.empty())) && (!(m_vMovieInfo[selected].file.Name.empty())))
		return &m_vMovieInfo[selected].file;
	else
		return NULL;
}
*/

// plugin API
void plugin_init(void)
{
}

void plugin_del(void)
{
}

void plugin_exec(void)
{
	/*
	CMoviePlayerGui tmpMoviePlayerGui;
	CNetzKinoBrowser * moviebrowser;
	MI_MOVIE_INFO * p_movie_info;
	
	moviebrowser = new CNetzKinoBrowser();

BROWSER:
	if (moviebrowser->exec()) 
	{
		CFile * file;

		if ((file = moviebrowser->getSelectedFile()) != NULL) 
		{
			p_movie_info = moviebrowser->getCurrentMovieInfo();
			
			file->Title = p_movie_info->epgTitle;
			file->Info1 = p_movie_info->epgInfo1;
			file->Info2 = p_movie_info->epgInfo2;
			file->Thumbnail = p_movie_info->tfile;
	
			tmpMoviePlayerGui.addToPlaylist(*file);
			tmpMoviePlayerGui.exec(NULL, "urlplayback");
		}

		neutrino_msg_t msg;
		neutrino_msg_data_t data;

		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg != CRCInput::RC_home) 
		{
			goto BROWSER;
		}
	}
						
	delete moviebrowser;
	*/

	CNetzKinoBrowser * NKHandler = new CNetzKinoBrowser();
	
	//NKHandler->showNKMenu();
	NKHandler->loadMovies();
	
	delete NKHandler;
	NKHandler = NULL;			
}


