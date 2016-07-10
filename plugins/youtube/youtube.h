/*
  $Id: youtube.h 2014/10/03 mohousch Exp $

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

#ifndef __YT__
#define __YT__

#include <plugin.h>
#include <ytparser.h>


#define YTBROWSER_SETTINGS_FILE          PLUGINDIR "/youtube/yt.conf"

//
#define MIN_YTBROWSER_FRAME_HEIGHT 	100
#define MAX_YTBROWSER_FRAME_HEIGHT 	400
#define YTB_MAX_ROWS 			3

typedef enum
{
	YTB_INFO_TITLE 			= 0,
	YTB_INFO_LENGTH 		= 1,
	YTB_INFO_RECORDDATE 		= 2,
	YTB_INFO_MAX_NUMBER		= 3
}YTB_INFO_ITEM;

typedef enum
{
	YTB_FOCUS_BROWSER = 0,
	YTB_FOCUS_MOVIE_INFO = 1,
	YTB_FOCUS_MAX_NUMBER = 2
}YTB_FOCUS;

typedef enum
{
	YTB_GUI_BROWSER_ONLY = 0,
	YTB_GUI_MOVIE_INFO = 1,
	YTB_GUI_MAX_NUMBER = 2
}YTB_GUI;

// settings
typedef struct
{
	YTB_GUI gui;
	
	// these variables are used for the listframes
	int browserFrameHeight;
	int browserRowNr;
	YTB_INFO_ITEM browserRowItem[YTB_MAX_ROWS];
	int browserRowWidth[YTB_MAX_ROWS];
	
	// youtube
	int ytmode;
	int ytorderby;
	std::string ytregion;
	std::string ytvid;
	std::string ytsearch;
	std::string ytkey;
}YTB_SETTINGS;

class CYTBrowser : public CMenuTarget
{
	private:
		CFrameBuffer * m_pcWindow;
		
		CListFrame * m_pcBrowser;
		CTextBox * m_pcInfo;
		
		CBox m_cBoxFrame;
		CBox m_cBoxFrameBrowserList;
		CBox m_cBoxFrameFootRel;
		CBox m_cBoxFrameTitleRel;
		CBox m_cBoxFrameInfo;
		
		LF_LINES m_browserListLines;
		
		std::vector<MI_MOVIE_INFO> m_vMovieInfo;
		std::vector<MI_MOVIE_INFO*> m_vHandleBrowserList;
		
		unsigned int m_currentBrowserSelection;
 		unsigned int m_prevBrowserSelection;
		
		bool m_showBrowserFiles;
		bool m_showMovieInfo;
		
		MI_MOVIE_INFO * m_movieSelectionHandler;
		
		YTB_FOCUS m_windowFocus;
		
		bool m_reload_movies;
		
		static CFont * m_pcFontFoot;
		static CFont * m_pcFontTitle;
		
		std::string m_textTitle;
		
		CConfigFile configfile;
		
		CMovieInfo m_movieInfo;
		
		// youtube
		cYTFeedParser ytparser;
		
		bool loadSettings(YTB_SETTINGS* settings); // P2
		bool saveSettings(YTB_SETTINGS* settings); // P2
		
		void loadYTitles(int mode, std::string search = "", std::string id = "");
		bool showYTMenu(void);
		
	public:
		CYTBrowser();
		~CYTBrowser();
		
		int exec();
		int exec(CMenuTarget* parent, const std::string & actionKey);
		
		CFile * getSelectedFile(void); 
		MI_MOVIE_INFO* getCurrentMovieInfo(void){return(m_movieSelectionHandler);};
		
	private:
		// browser init
		void init(void); 
		void initGlobalSettings(void); 
		void initFrames(void);
		
		// browser main window
		int paint(void); 
		void refresh(void);
        	void hide(void); 
		void refreshBrowserList(void);
		void refreshMovieInfo(void);
		void refreshFoot(void);
		void refreshTitle(void);
		void refreshInfo(void);
		
		// event
		bool onButtonPress(neutrino_msg_t msg); 
		bool onButtonPressMainFrame(neutrino_msg_t msg);
		bool onButtonPressBrowserList(neutrino_msg_t msg);
		bool onButtonPressMovieInfoList(neutrino_msg_t msg);
		
		void onSetFocus(YTB_FOCUS new_focus);
		void onSetFocusNext(void);
		
		void onSetGUIWindow(YTB_GUI gui);
		
		void loadMovies();
		
		// misc
		void updateMovieSelection(void);
		bool getMovieInfoItem(MI_MOVIE_INFO& movie_info, YTB_INFO_ITEM item, std::string* item_string);
		
		// yt
		neutrino_locale_t getFeedLocale(void);
}; 

#endif //__YT__
