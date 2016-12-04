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

 
#define MAX_WINDOW_WIDTH  		(g_settings.screen_EndX - g_settings.screen_StartX - 40)
#define MAX_WINDOW_HEIGHT 		(g_settings.screen_EndY - g_settings.screen_StartY - 40)	

#define MIN_WINDOW_WIDTH  		((g_settings.screen_EndX - g_settings.screen_StartX)>>1)
#define MIN_WINDOW_HEIGHT 		200	

#define TITLE_FONT 			g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]
#define FOOT_FONT 			g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]

#define INTER_FRAME_SPACE 		4  // space between e.g. upper and lower window

#define NEUTRINO_ICON_YT_SMALL			PLUGINDIR "/youtube/youtube_small.png"

CFont* CYTBrowser::m_pcFontFoot = NULL;
CFont* CYTBrowser::m_pcFontTitle = NULL;
YTB_SETTINGS m_settings;

const neutrino_locale_t m_localizedItemName[YTB_INFO_MAX_NUMBER + 1] =
{
	LOCALE_MOVIEBROWSER_SHORT_TITLE ,
	LOCALE_MOVIEBROWSER_SHORT_LENGTH,
	LOCALE_MOVIEBROWSER_SHORT_RECORDDATE,
	NONEXISTANT_LOCALE
};

// default row size in pixel for any element
#define	YTB_ROW_WIDTH_TITLE		MAX_WINDOW_WIDTH / 2 + MAX_WINDOW_WIDTH / 6
#define	YTB_ROW_WIDTH_LENGTH		MAX_WINDOW_WIDTH / 6
#define	YTB_ROW_WIDTH_RECORDDATE 	MAX_WINDOW_WIDTH / 6

const int m_defaultRowWidth[YTB_INFO_MAX_NUMBER + 1] = 
{
	YTB_ROW_WIDTH_TITLE,
	YTB_ROW_WIDTH_LENGTH,
	YTB_ROW_WIDTH_RECORDDATE ,
	0 //MB_ROW_WIDTH_MAX_NUMBER 
};
 
CYTBrowser::CYTBrowser(): configfile ('\t')
{
	dprintf(DEBUG_NORMAL, "$Id: youtube Browser, v 0.0.1 2014/09/15 12:00:30 mohousch Exp $\n");
	init();
}

CYTBrowser::~CYTBrowser()
{
	dprintf(DEBUG_NORMAL, "CYTBrowser: del\n");
	
	m_vMovieInfo.clear();
	m_vHandleBrowserList.clear();

	m_movieSelectionHandler = NULL;

	for(int i = 0; i < LF_MAX_ROWS; i++)
	{
		m_browserListLines.lineArray[i].clear();
	}

	ytparser.Cleanup();
}

void CYTBrowser::init(void)
{
	dprintf(DEBUG_NORMAL, "CYTBrowser::init\n");
	
	initGlobalSettings();
	
	// load settings
	loadSettings(&m_settings);
		
	m_reload_movies = true;

	m_pcWindow = CFrameBuffer::getInstance();
	
	m_pcBrowser = NULL;
	m_pcInfo = NULL;
	
	m_windowFocus = YTB_FOCUS_BROWSER;
	
	m_textTitle = g_Locale->getText(LOCALE_YOUTUBE);
	
	m_movieSelectionHandler = NULL;
	m_currentBrowserSelection = 0;
 	m_prevBrowserSelection = 0;
	
	// browser
	if(m_settings.browserFrameHeight < MIN_YTBROWSER_FRAME_HEIGHT )
       		m_settings.browserFrameHeight = MIN_YTBROWSER_FRAME_HEIGHT;
	if(m_settings.browserFrameHeight > MAX_YTBROWSER_FRAME_HEIGHT)
        	m_settings.browserFrameHeight = MAX_YTBROWSER_FRAME_HEIGHT;
	
	// Browser List 
	if(m_settings.browserRowNr == 0)
	{
		dprintf(DEBUG_NORMAL, " row error\r\n");
		
		// init browser row elements if not configured correctly by neutrino.config
		m_settings.browserRowNr = 3;
		m_settings.browserRowItem[0] = YTB_INFO_TITLE;
		m_settings.browserRowItem[1] = YTB_INFO_LENGTH;
		m_settings.browserRowItem[2] = YTB_INFO_RECORDDATE;

		m_settings.browserRowWidth[0] = m_defaultRowWidth[m_settings.browserRowItem[0]];
		m_settings.browserRowWidth[1] = m_defaultRowWidth[m_settings.browserRowItem[1]];
		m_settings.browserRowWidth[2] = m_defaultRowWidth[m_settings.browserRowItem[2]];
	}

	initFrames();
	
	refreshBrowserList();	
}

void CYTBrowser::initGlobalSettings(void)
{
	dprintf(DEBUG_NORMAL, "CYTBrowser::initGlobalSettings\n");
	
	m_settings.gui = YTB_GUI_MOVIE_INFO;

	// Browser List
	m_settings.browserFrameHeight = g_settings.screen_EndY - g_settings.screen_StartY - 20 - ((g_settings.screen_EndY - g_settings.screen_StartY - 20)>>1) - (INTER_FRAME_SPACE>>1);
	
	m_settings.browserRowNr = 3;
	m_settings.browserRowItem[0] = YTB_INFO_TITLE;
	m_settings.browserRowItem[1] = YTB_INFO_LENGTH;
	m_settings.browserRowItem[2] = YTB_INFO_RECORDDATE;

	m_settings.browserRowWidth[0] = m_defaultRowWidth[m_settings.browserRowItem[0]];
	m_settings.browserRowWidth[1] = m_defaultRowWidth[m_settings.browserRowItem[1]];
	m_settings.browserRowWidth[2] = m_defaultRowWidth[m_settings.browserRowItem[2]];
	
	// youtube
	m_settings.ytmode = cYTFeedParser::MOST_POPULAR;
	m_settings.ytorderby = cYTFeedParser::ORDERBY_PUBLISHED;
	m_settings.ytregion = "default";
	m_settings.ytsearch = configfile.getString("ytsearch", "");
	m_settings.ytkey = configfile.getString("ytkey", "");
}

void CYTBrowser::initFrames(void)
{
	dprintf(DEBUG_NORMAL, "CYTBrowser::initFrames\n");
	
	m_pcFontFoot  = FOOT_FONT;
	m_pcFontTitle = TITLE_FONT;
	
	m_cBoxFrame.iX = 			g_settings.screen_StartX + 10;
	m_cBoxFrame.iY = 			g_settings.screen_StartY + 10;
	m_cBoxFrame.iWidth = 			g_settings.screen_EndX - g_settings.screen_StartX - 20;
	m_cBoxFrame.iHeight = 			g_settings.screen_EndY - g_settings.screen_StartY - 20;

	m_cBoxFrameTitleRel.iX =		0;
	m_cBoxFrameTitleRel.iY = 		0;
	m_cBoxFrameTitleRel.iWidth = 		m_cBoxFrame.iWidth;
	m_cBoxFrameTitleRel.iHeight = 		m_pcFontTitle->getHeight();

	m_cBoxFrameBrowserList.iX = 		m_cBoxFrame.iX;
	m_cBoxFrameBrowserList.iY = 		m_cBoxFrame.iY + m_cBoxFrameTitleRel.iHeight;
	m_cBoxFrameBrowserList.iWidth = 	m_cBoxFrame.iWidth;
	m_cBoxFrameBrowserList.iHeight = 	m_settings.browserFrameHeight;

	m_cBoxFrameFootRel.iX = 		0;
	m_cBoxFrameFootRel.iY = 		m_cBoxFrame.iHeight - m_pcFontFoot->getHeight()*2; //FIXME
	m_cBoxFrameFootRel.iWidth = 		m_cBoxFrameBrowserList.iWidth;
	m_cBoxFrameFootRel.iHeight = 		m_pcFontFoot->getHeight()*2; //FIXME
	
	m_cBoxFrameInfo.iX = 			m_cBoxFrameBrowserList.iX;
	m_cBoxFrameInfo.iY = 			m_cBoxFrameBrowserList.iY + m_cBoxFrameBrowserList.iHeight + INTER_FRAME_SPACE;
	m_cBoxFrameInfo.iWidth = 		m_cBoxFrameBrowserList.iWidth;
	m_cBoxFrameInfo.iHeight = 		m_cBoxFrame.iHeight - m_cBoxFrameBrowserList.iHeight - INTER_FRAME_SPACE - m_cBoxFrameFootRel.iHeight - m_cBoxFrameTitleRel.iHeight;
}

bool CYTBrowser::loadSettings(YTB_SETTINGS *settings)
{
	bool result = true;
	
	dprintf(DEBUG_NORMAL, "CYTBrowser::loadSettings\r\n");
	
	if(configfile.loadConfig(YTBROWSER_SETTINGS_FILE))
	{
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

	configfile.setInt32("ytorderby", settings->ytorderby);
	configfile.setString("ytregion", settings->ytregion);
	configfile.setString("ytsearch", settings->ytsearch);
	configfile.setString("ytkey", settings->ytkey);
 
 	if (configfile.getModifiedFlag())
		configfile.saveConfig(YTBROWSER_SETTINGS_FILE);
	
	return (result);
}

int CYTBrowser::exec(CMenuTarget * parent, const std::string & actionKey)
{
	int returnval = menu_return::RETURN_REPAINT;

	if(actionKey == "run")
	{
		if(parent) 
			parent->hide ();
		
		exec();
	}
	
	return returnval;
}

int CYTBrowser::exec()
{
	bool res = false;

	dprintf(DEBUG_NORMAL, "CYTBrowser::exec\n");
	
	int timeout = -1;
	int returnDefaultOnTimeout = true;
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;
	
	//initGlobalSettings();

	// load settings
	loadSettings(&m_settings);
	
	// init frames
	initFrames();

	// Clear all, to avoid 'jump' in screen 
	m_vHandleBrowserList.clear();
	
	m_movieSelectionHandler = NULL;

	for(int i = 0; i < LF_MAX_ROWS; i++)
	{
		m_browserListLines.lineArray[i].clear();
	}

	// paint mb
	if(paint() == false)
		return res;// paint failed due to less memory , exit 

	if ( timeout == -1 )
		timeout = g_settings.timing[SNeutrinoSettings::TIMING_FILEBROWSER];

	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd( timeout );
	
	// reload movies
	if(m_reload_movies == true)
	{
		dprintf(DEBUG_NORMAL, "CYTBrowser::exec\n");
		loadMovies();
	}
	else
	{
		// since we cleared everything above, we have to refresh the list now.
		refreshBrowserList();	
	}

	// get old movie selection and set position in windows	
	m_currentBrowserSelection = m_prevBrowserSelection;

	m_pcBrowser->setSelectedLine(m_currentBrowserSelection);

	// update movie selection
	updateMovieSelection();

	// refresh title
	refreshTitle();
	
	// on set guiwindow
	onSetGUIWindow(m_settings.gui);
	
	// browser paint 
	m_pcBrowser->paint();
	m_pcWindow->blit();

	bool loop = true;
	bool result;
	
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		result = onButtonPress(msg);
		
		if(result == false)
		{
			if (msg == CRCInput::RC_timeout && returnDefaultOnTimeout)
			{
				dprintf(DEBUG_NORMAL, "CYTBrowser::exec: Timerevent\n");
				
				loop = false;
			}
			else if(msg == CRCInput::RC_ok)
			{
				if(m_movieSelectionHandler != NULL)
				{
					res = true;
					loop = false;
				}
			}
			else if (msg == CRCInput::RC_home)
			{
				loop = false;
			}
			else if (CNeutrinoApp::getInstance()->handleMsg(msg, data) & messages_return::cancel_all)
			{
				dprintf(DEBUG_NORMAL, "CYTBrowser::exec: getInstance\r\n");
				
				loop = false;
			}
		}
		
		m_pcWindow->blit();	

		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = CRCInput::calcTimeoutEnd(timeout); // calcualate next timeout
	}
	
	hide();
	
	//
	m_prevBrowserSelection = m_currentBrowserSelection;
	
	saveSettings(&m_settings);
	
	return (res);
}

void CYTBrowser::hide(void)
{
	dprintf(DEBUG_NORMAL, "CYTBrowser::hide\n");
	
	m_pcWindow->paintBackground();
	m_pcWindow->blit();
	
	if (m_pcBrowser != NULL)
	{
		m_currentBrowserSelection = m_pcBrowser->getSelectedLine();
		delete m_pcBrowser;
		m_pcBrowser = NULL;
	}
	
	if (m_pcInfo != NULL) 
	{
		delete m_pcInfo;
		m_pcInfo = NULL;
	}
}

int CYTBrowser::paint(void)
{
	dprintf(DEBUG_NORMAL, "CYTBrowser::paint\n");

	m_pcBrowser = new CListFrame(&m_browserListLines, NULL, CListFrame::SCROLL | CListFrame::HEADER_LINE, &m_cBoxFrameBrowserList);

	m_pcInfo = new CTextBox(" ", NULL, CTextBox::SCROLL, &m_cBoxFrameInfo);	

	if(m_pcBrowser == NULL || m_pcInfo == NULL )
	{
		if (m_pcBrowser != NULL)
		{
			delete m_pcBrowser;
			m_pcBrowser = NULL;
		}

		if (m_pcInfo != NULL)
		{ 
			delete m_pcInfo;
			m_pcInfo = NULL;
		}

		return (false);
	} 
	
	return (true);
}

void CYTBrowser::refresh(void)
{
	dprintf(DEBUG_NORMAL, "CYTBrowser::refresh\n");
	
	refreshTitle();

	if (m_pcBrowser != NULL && m_showBrowserFiles == true )
		 m_pcBrowser->refresh();
	
	if (m_pcInfo != NULL && m_showMovieInfo == true) 
		refreshMovieInfo();
		
	refreshFoot();
}

CFile * CYTBrowser::getSelectedFile(void)
{
	dprintf(DEBUG_INFO, "CYTBrowser::getSelectedFile: %s\r\n", m_movieSelectionHandler->file.Name.c_str());

	if(m_movieSelectionHandler != NULL)
		return(&m_movieSelectionHandler->file);
	else
		return(NULL);
}

void CYTBrowser::refreshMovieInfo(void)
{
	dprintf(DEBUG_INFO, "CYTBrowser::refreshMovieInfo: m_vMovieInfo.size %d\n", m_vMovieInfo.size());
	
	std::string emptytext = " ";
	
	if(m_vMovieInfo.size() <= 0) 
	{
		if(m_pcInfo != NULL)
			m_pcInfo->setText(&emptytext);
		return;
	}
	
	if (m_movieSelectionHandler == NULL)
	{
		// There is no selected element, clear LCD
		m_pcInfo->setText(&emptytext);
	}
	else
	{
		int picw = 320;
		int pich = 256;

		int p_w = 0;
		int p_h = 0;
		int nbpp = 0;	
		
		if(!access(m_movieSelectionHandler->tfile.c_str(), F_OK))
		{
			m_pcWindow->getSize(m_movieSelectionHandler->tfile, &p_w, &p_h, &nbpp);

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
		
		m_pcInfo->setText(&m_movieSelectionHandler->epgInfo2, m_movieSelectionHandler->tfile, p_w, p_h);

	}
	
	m_pcWindow->blit();
}

void CYTBrowser::refreshBrowserList(void) //P1
{
	dprintf(DEBUG_INFO, "CYTBrowser::refreshBrowserList\n");
	
	std::string string_item;

	// initialise and clear list array
	m_browserListLines.rows = m_settings.browserRowNr;
	for(int row = 0; row < m_settings.browserRowNr; row++)
	{
		m_browserListLines.lineArray[row].clear();
		m_browserListLines.rowWidth[row] = m_settings.browserRowWidth[row];
		m_browserListLines.lineHeader[row]= g_Locale->getText(m_localizedItemName[m_settings.browserRowItem[row]]);
	}

	m_vHandleBrowserList.clear();
	
	if(m_vMovieInfo.size() <= 0) 
	{
		m_currentBrowserSelection = 0;
		m_movieSelectionHandler = NULL;
		if(m_pcBrowser != NULL)
			m_pcBrowser->setLines(&m_browserListLines);//FIXME last delete test
		return; // exit here if nothing else is to do
	}
	
	MI_MOVIE_INFO* movie_handle;
	
	// prepare Browser list for sorting and filtering
	for(unsigned int file = 0; file < m_vMovieInfo.size(); file++)
	{
		movie_handle = &(m_vMovieInfo[file]);
		m_vHandleBrowserList.push_back(movie_handle);
	}

	for(unsigned int handle = 0; handle < m_vHandleBrowserList.size(); handle++)
	{
		for(int row = 0; row < m_settings.browserRowNr; row++)
		{
			if ( getMovieInfoItem(*m_vHandleBrowserList[handle], m_settings.browserRowItem[row], &string_item) == false)
			{
				string_item = "n/a";
				if(m_settings.browserRowItem[row] == YTB_INFO_TITLE)
					getMovieInfoItem(*m_vHandleBrowserList[handle], YTB_INFO_TITLE, &string_item);
			}
			m_browserListLines.lineArray[row].push_back(string_item);
		}
	}

	m_pcBrowser->setLines(&m_browserListLines);
	m_currentBrowserSelection = m_pcBrowser->getSelectedLine();
	
	// update selected movie if browser is in the focus
	if (m_windowFocus == YTB_FOCUS_BROWSER)
	{
		updateMovieSelection();	
	}
}

void CYTBrowser::refreshTitle(void) 
{
	//Paint Text Background
	dprintf(DEBUG_INFO, "CYTBrowser::refreshTitle\n");
	
	// title
	std::string title;
	std::string mb_icon;
	
	title = g_Locale->getText(LOCALE_YOUTUBE);
	title += " : ";
		
	neutrino_locale_t loc = getFeedLocale();
	title += g_Locale->getText(loc);
	if (loc == LOCALE_YT_SEARCH)
		title += " \"" + m_settings.ytsearch + "\"";
		
	mb_icon = NEUTRINO_ICON_YT_SMALL;
	//

	// head box
	m_pcWindow->paintBoxRel(m_cBoxFrame.iX + m_cBoxFrameTitleRel.iX, m_cBoxFrame.iY + m_cBoxFrameTitleRel.iY, m_cBoxFrameTitleRel.iWidth, m_cBoxFrameTitleRel.iHeight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, g_settings.menu_Head_gradient);
	
	// movie icon
	int icon_w, icon_h;
	m_pcWindow->getIconSize(mb_icon.c_str(), &icon_w, &icon_h);
	m_pcWindow->paintIcon(mb_icon, m_cBoxFrame.iX + m_cBoxFrameTitleRel.iX + BORDER_LEFT, m_cBoxFrame.iY + m_cBoxFrameTitleRel.iY + (m_cBoxFrameTitleRel.iHeight - icon_h)/2);

	// setup icon
	m_pcWindow->getIconSize(NEUTRINO_ICON_BUTTON_SETUP, &icon_w, &icon_h);
	int xpos1 = m_cBoxFrame.iX + m_cBoxFrameTitleRel.iX + m_cBoxFrameTitleRel.iWidth - BORDER_RIGHT;
	int ypos = m_cBoxFrame.iY + m_cBoxFrameTitleRel.iY + (m_cBoxFrameTitleRel.iHeight - icon_h)/2;

	m_pcWindow->paintIcon(NEUTRINO_ICON_BUTTON_SETUP, xpos1 - icon_w, ypos);

	// help icon
	int icon_h_w, icon_h_h;
	m_pcWindow->getIconSize(NEUTRINO_ICON_BUTTON_SETUP, &icon_h_w, &icon_h_h);
	ypos = m_cBoxFrame.iY + m_cBoxFrameTitleRel.iY + (m_cBoxFrameTitleRel.iHeight - icon_h_h)/2;
	m_pcWindow->paintIcon(NEUTRINO_ICON_BUTTON_HELP, xpos1 - icon_w - 2 - icon_h_w, ypos);
	
	// 0 button icon (reload movies)
	int icon_0_w, icon_0_h;
	m_pcWindow->getIconSize(NEUTRINO_ICON_BUTTON_0, &icon_0_w, &icon_0_h);
	ypos = m_cBoxFrame.iY + m_cBoxFrameTitleRel.iY + (m_cBoxFrameTitleRel.iHeight - icon_0_h)/2;
	m_pcWindow->paintIcon(NEUTRINO_ICON_BUTTON_0, xpos1 - icon_w - 2 - icon_h_w - 2 - icon_0_w, ypos);
	
	// head title
	m_pcFontTitle->RenderString(m_cBoxFrame.iX + m_cBoxFrameTitleRel.iX + icon_w + BORDER_LEFT + ICON_OFFSET, m_cBoxFrame.iY + m_cBoxFrameTitleRel.iY + m_cBoxFrameTitleRel.iHeight, m_cBoxFrameTitleRel.iWidth - (BORDER_LEFT + BORDER_RIGHT + ICON_OFFSET + 4*2) - 2*icon_w - icon_h_w - icon_0_w, title.c_str(), COL_MENUHEAD, 0, true); // UTF-8
}

const struct button_label CYTBrowserButtons[4] =
{
	{ NEUTRINO_ICON_BUTTON_RED, LOCALE_YT_PREV_RESULTS, NULL },
	{ NEUTRINO_ICON_BUTTON_GREEN, LOCALE_YT_NEXT_RESULTS, NULL },
	{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_MOVIEBROWSER_NEXT_FOCUS, NULL },
	{ NEUTRINO_ICON_BUTTON_BLUE, LOCALE_YT_RELATED, NULL }
};

void CYTBrowser::refreshFoot(void) 
{
	dprintf(DEBUG_INFO, "CYTBrowser::refreshFoot\n");
	
	// foot
	m_pcWindow->paintBoxRel(m_cBoxFrame.iX + m_cBoxFrameFootRel.iX, m_cBoxFrame.iY + m_cBoxFrameFootRel.iY, m_cBoxFrameFootRel.iWidth, m_cBoxFrameFootRel.iHeight + 6, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.menu_Foot_gradient);

	::paintButtons(m_pcWindow, m_pcFontFoot, g_Locale, m_cBoxFrame.iX + m_cBoxFrameFootRel.iX + ICON_OFFSET, m_cBoxFrame.iY + m_cBoxFrameFootRel.iY, (m_cBoxFrameFootRel.iWidth - 2*ICON_OFFSET)/4, 4, CYTBrowserButtons, m_cBoxFrameFootRel.iHeight + 6);
}

bool CYTBrowser::onButtonPress(neutrino_msg_t msg)
{
	dprintf(DEBUG_INFO, "CYTBrowser::onButtonPress %d\n", msg);
	
	bool result = false;
	
	result = onButtonPressMainFrame(msg);

	if(result == false)
	{
		// if Main Frame didnot process the button, the focused window may do
		switch(m_windowFocus)
		{
			case YTB_FOCUS_BROWSER:
			 	result = onButtonPressBrowserList(msg);		
				break;
				
			case YTB_FOCUS_MOVIE_INFO:
			 	result = onButtonPressMovieInfoList(msg);		
				break;
				
			default:
				break;
		}
	}
	
	return (result);
}

bool CYTBrowser::onButtonPressMainFrame(neutrino_msg_t msg)
{
	dprintf(DEBUG_INFO, "CYTBrowser::onButtonPressMainFrame: %d\n", msg);
	
	bool result = true;

	if (msg == CRCInput::RC_home)
	{
		result = false;
	}
	else if (msg == CRCInput::RC_green) 
	{
		if(ytparser.HaveNext())
		{
			//
			m_pcWindow->paintBackground();
				
			//
			CHintBox loadBox(LOCALE_YOUTUBE, g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
			loadBox.paint();
				
			// yt clean up
			ytparser.Cleanup();
				
			loadYTitles(cYTFeedParser::NEXT, m_settings.ytsearch, m_settings.ytvid);
				
			loadBox.hide();
				
			refreshBrowserList();
			refresh();
		}		
	}
	else if (msg == CRCInput::RC_yellow) 
	{
		onSetFocusNext();
	}
	else if (msg == CRCInput::RC_blue) 
	{
		// related videos
		if( (!m_vMovieInfo.empty()) && (m_movieSelectionHandler != NULL) )
		{
			if (m_settings.ytvid != m_movieSelectionHandler->ytid) 
			{
				m_settings.ytvid = m_movieSelectionHandler->ytid;
				m_settings.ytmode = cYTFeedParser::RELATED;
			
				m_pcWindow->paintBackground();
				CHintBox loadBox(LOCALE_YOUTUBE, g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
				loadBox.paint();
				ytparser.Cleanup();
				loadYTitles(m_settings.ytmode, m_settings.ytsearch, m_settings.ytvid);
				loadBox.hide();
				
				refreshBrowserList();
				refresh();
			}
		}
	}
	else if (msg == CRCInput::RC_red ) 
	{	
		if(ytparser.HavePrev())
		{
			//
			m_pcWindow->paintBackground();
				
			//
			CHintBox loadBox(LOCALE_YOUTUBE, g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
			loadBox.paint();
				
			// yt clean up
			ytparser.Cleanup();
		
			// yt reload
			loadYTitles(cYTFeedParser::PREV, m_settings.ytsearch, m_settings.ytvid);
				
			loadBox.hide();
				
			// refresh
			refreshBrowserList();
			refresh();
		}
	}
	else if ( msg == CRCInput::RC_info) 
	{
		if(m_movieSelectionHandler != NULL)
		{
			m_pcWindow->paintBackground();
			m_pcWindow->blit();
	  
			m_movieInfo.showMovieInfo(*m_movieSelectionHandler);
			
			refresh();
		}
	}
	else if (msg == CRCInput::RC_setup) 
	{
		showYTMenu();	
	}
	else if (msg == CRCInput::RC_0)
	{
		ytparser.Cleanup();
		loadMovies();
		refresh();
	}
	else
	{
		dprintf(DEBUG_INFO, "CYTBrowser::onButtonPressMainFrame: none\r\n");
		
		result = false;
	}

	return (result);
}

bool CYTBrowser::onButtonPressBrowserList(neutrino_msg_t msg) 
{
	dprintf(DEBUG_INFO, "CYTBrowser::onButtonPressBrowserList %d\n", msg);
	
	bool result = true;
	
	if(msg == CRCInput::RC_up)
	{
		m_pcBrowser->scrollLineUp(1);
	}
	else if (msg == CRCInput::RC_down)
	{
		m_pcBrowser->scrollLineDown(1);
	}
	else if (msg == CRCInput::RC_page_up)
	{
		m_pcBrowser->scrollPageUp(1);
	}
	else if (msg == CRCInput::RC_page_down)
	{
		m_pcBrowser->scrollPageDown(1);
	}
	else if (msg == CRCInput::RC_left)
	{
		m_pcBrowser->scrollPageUp(1);
	}
	else if (msg == CRCInput::RC_right)
	{
		m_pcBrowser->scrollPageDown(1);
	}
	else
	{
		// default
		result = false;
	}
	
	if(result == true)
		updateMovieSelection();

	return (result);
}

bool CYTBrowser::onButtonPressMovieInfoList(neutrino_msg_t msg) 
{
	dprintf(DEBUG_INFO, "CYTBrowser::onButtonPressMovieInfoList: %d\n", msg);
	
	bool result = true;
	
	if(msg == CRCInput::RC_up)
	{
		m_pcInfo->scrollPageUp(1);
	}
	else if (msg == CRCInput::RC_down)
	{
		m_pcInfo->scrollPageDown(1);
	}
	else
	{
		// default
		result = false;
	}	

	return (result);
}

void CYTBrowser::onSetGUIWindow(YTB_GUI gui)
{
	m_settings.gui = gui;
	
	if(gui == YTB_GUI_MOVIE_INFO)
	{
		dprintf(DEBUG_NORMAL, "CYTBrowser::onSetGUIWindow:\n");
		
		// Paint these frames ...
		m_showMovieInfo = true;
		m_showBrowserFiles = true;
		
		m_pcBrowser->paint();
		onSetFocus(YTB_FOCUS_BROWSER);
		m_pcInfo->paint();
		refreshMovieInfo();
	}
}

void CYTBrowser::onSetFocus(YTB_FOCUS new_focus)
{
	dprintf(DEBUG_INFO, "CYTBrowser::onSetFocus: %d\n", new_focus);
	
	m_windowFocus = new_focus;
	
	if(m_windowFocus == YTB_FOCUS_BROWSER)
	{
		m_pcBrowser->showSelection(true);
	}
	else if(m_windowFocus == YTB_FOCUS_MOVIE_INFO)
	{
		m_pcBrowser->showSelection(false);
	}
	
	updateMovieSelection();
	refreshFoot();	
}

void CYTBrowser::onSetFocusNext(void) 
{
	dprintf(DEBUG_INFO, "CYTBrowser::onSetFocusNext:\n");
	
	if(m_settings.gui == YTB_GUI_MOVIE_INFO)
	{
		if(m_windowFocus == YTB_FOCUS_BROWSER)
		{
			dprintf(DEBUG_NORMAL, "CYTBrowser::onSetFocusNext: YTB_FOCUS_MOVIE_INFO\r\n");
			
			onSetFocus(YTB_FOCUS_MOVIE_INFO);
			m_windowFocus = YTB_FOCUS_MOVIE_INFO;
		}
		else
		{
			dprintf(DEBUG_NORMAL, "CYTBrowser::onSetFocusNext: YTB_FOCUS_BROWSER\r\n");
			onSetFocus(YTB_FOCUS_BROWSER);
		}
	}
}

void CYTBrowser::updateMovieSelection(void)
{
	dprintf(DEBUG_INFO, "CYTBrowser::updateMovieSelection: %d\n", m_windowFocus);
	
	if (m_vMovieInfo.size() == 0) 
		return;
	
	bool new_selection = false;
	 
	unsigned int old_movie_selection;
	
	if(m_windowFocus == YTB_FOCUS_BROWSER)
	{
		if(m_vHandleBrowserList.size() == 0)
		{
			// There are no elements in the Filebrowser, clear all handles
			m_currentBrowserSelection = 0;
			m_movieSelectionHandler = NULL;
			new_selection = true;
		}
		else
		{
			old_movie_selection = m_currentBrowserSelection;
			m_currentBrowserSelection = m_pcBrowser->getSelectedLine();
			
			if(m_currentBrowserSelection != old_movie_selection)
				new_selection = true;
			
			if(m_currentBrowserSelection < m_vHandleBrowserList.size())
				m_movieSelectionHandler = m_vHandleBrowserList[m_currentBrowserSelection];
		}
	}
	
	if(new_selection == true)
	{
		refreshMovieInfo();
	}
}

void CYTBrowser::loadMovies(void)
{
	dprintf(DEBUG_NORMAL, "CYTBrowser::loadMovies:\n");
	
	//first clear screen
	m_pcWindow->paintBackground();
	m_pcWindow->blit();	

	CHintBox loadBox(LOCALE_YOUTUBE, g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
	loadBox.paint();

	m_settings.ytmode = cYTFeedParser::MOST_POPULAR;
	loadYTitles(m_settings.ytmode, m_settings.ytsearch, m_settings.ytvid);

	loadBox.hide();

	refreshBrowserList();	
	refreshMovieInfo();	// is done by refreshBrowserList if needed
	
	m_reload_movies = false;
}

bool CYTBrowser::getMovieInfoItem(MI_MOVIE_INFO& movie_info, YTB_INFO_ITEM item, std::string* item_string)
{
	#define MAX_STR_TMP 100
	char str_tmp[MAX_STR_TMP];
	bool result = true;
	*item_string = "";

	switch(item)
	{	
		case YTB_INFO_TITLE:
			*item_string = movie_info.epgTitle;
			if(strcmp("not available", movie_info.epgTitle.c_str()) == 0)
				result = false;
			if(movie_info.epgTitle.empty())
				result = false;
			break;
			
		case YTB_INFO_LENGTH:
			snprintf(str_tmp,MAX_STR_TMP,"%4d",movie_info.length);
			*item_string = str_tmp;
			break;
			
		case YTB_INFO_RECORDDATE:
			*item_string = movie_info.ytdate;	
			break;
			
		case YTB_INFO_MAX_NUMBER:
		default:
			*item_string = "";
			result = false;
			break;
	}
	
	return(result);
}

// youtube
void CYTBrowser::loadYTitles(int mode, std::string search, std::string id)
{
	dprintf(DEBUG_NORMAL, "CYTBrowser::loadYTitles: parsed %d old mode %d new mode %d region %s\n", ytparser.Parsed(), ytparser.GetFeedMode(), m_settings.ytmode, m_settings.ytregion.c_str());
	
	if (m_settings.ytregion == "default")
		ytparser.SetRegion("");
	else
		ytparser.SetRegion(m_settings.ytregion);

	ytparser.SetMaxResults(m_pcBrowser->getLinesPerPage());

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
			m_reload_movies = true;
			
			return;
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
		movieInfo.file.Url = ylist[i].GetUrl();
		
		m_vMovieInfo.push_back(movieInfo);
	}
	
	m_currentBrowserSelection = 0;
	m_pcBrowser->setSelectedLine(m_currentBrowserSelection);
}

const CMenuOptionChooser::keyval YT_FEED_OPTIONS[] =
{
        { cYTFeedParser::MOST_POPULAR, LOCALE_YT_MOST_POPULAR, NULL },
        { cYTFeedParser::MOST_POPULAR_ALL_TIME, LOCALE_YT_MOST_POPULAR_ALL_TIME, NULL },
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
	m_pcWindow->paintBackground();
	m_pcWindow->blit();

	CMenuWidget mainMenu(LOCALE_YOUTUBE, NEUTRINO_ICON_YT_SMALL, MENU_WIDTH + 50);
	mainMenu.disableMenuPosition();

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

	mainMenu.exec(NULL, "");
	delete selector;

	bool reload = false;
	int newmode = -1;

	if (select >= 0) 
	{
		newmode = select;
		if (newmode == cYTFeedParser::NEXT || newmode == cYTFeedParser::PREV) 
		{
			reload = true;
		}
		else if (select == cYTFeedParser::SEARCH) 
		{
			if (!search.empty()) 
			{
				reload = true;
				m_settings.ytsearch = search;
				m_settings.ytmode = newmode;
			}
		}
		else if (m_settings.ytmode != newmode) 
		{
			m_settings.ytmode = newmode;
			reload = true;
		}
	}
	
	if(rstr != m_settings.ytregion) 
	{
		m_settings.ytregion = rstr;
		if (newmode < 0)
			newmode = m_settings.ytmode;
		reload = true;
		printf("change region to %s\n", m_settings.ytregion.c_str());
	}
	
	if (reload) 
	{
		CHintBox loadBox(LOCALE_YOUTUBE, g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
		loadBox.paint();
		ytparser.Cleanup();
		loadYTitles(newmode, m_settings.ytsearch, m_settings.ytvid);
		loadBox.hide();
	}
	
	refreshBrowserList();
	refresh();
	
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
	CMoviePlayerGui tmpMoviePlayerGui;
			
	CYTBrowser * moviebrowser = new CYTBrowser();
	MI_MOVIE_INFO * p_movie_info;
	
BROWSER:	
	if (moviebrowser->exec()) 
	{
		// get the current file name
		CFile * file;

		if ((file = moviebrowser->getSelectedFile()) != NULL) 
		{
			// movieinfos
			p_movie_info = moviebrowser->getCurrentMovieInfo();
			
			file->Title = p_movie_info->epgTitle;
			file->Info1 = p_movie_info->epgInfo1; //category ist always empty
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
}


