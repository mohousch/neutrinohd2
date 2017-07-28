/***************************************************************************
	Neutrino-GUI  -   DBoxII-Project
 
 	Homepage: http://dbox.cyberphoria.org/

	$Id: moviebrowser.h,v 1.5 2006/09/11 21:11:35 guenther Exp $

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


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

	***********************************************************

	Module Name: moviebrowser.h .

	Description: implementation of the CMovieBrowser class

	Date:	   Nov 2005

	Author: Günther@tuxbox.berlios.org
		based on code of Steffen Hehn 'McClean'

	$Log: moviebrowser.h,v $
	Revision 1.5  2006/09/11 21:11:35  guenther
	General menu clean up
	Dir menu updated
	Add options menu
	In movie info menu  "update all" added
	Serie option added (hide serie, auto serie)
	Update movie info on delete movie
	Delete Background when menu is entered
	Timeout updated (MB does not exit after options menu is left)
	
	Revision 1.4  2006/02/20 01:10:34  guenther
	- temporary parental lock updated - remove 1s debug prints in movieplayer- Delete file without rescan of movies- Crash if try to scroll in list with 2 movies only- UTF8XML to UTF8 conversion in preview- Last file selection recovered- use of standard folders adjustable in config- reload and remount option in config
	
	Revision 1.3  2005/12/18 09:23:53  metallica
	fix compil warnings
	
	Revision 1.2  2005/12/12 07:58:02  guenther
	- fix bug on deleting CMovieBrowser - speed up parse time (20 ms per .ts file now)- update stale function- refresh directories on reload- print scan time in debug console
	

****************************************************************************/
#ifndef MOVIEBROWSER_H_
#define MOVIEBROWSER_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <configfile.h>

#include <string>
#include <vector>
#include <list>

#include <gui/widget/listframe.h>
#include <gui/widget/menue.h>
#include <gui/widget/textbox.h>
#include <gui/movieinfo.h>
#include <driver/file.h>
//#include <driver/fb_window.h>
//#include <driver/pictureviewer/pictureviewer.h>


#define MAX_NUMBER_OF_BOOKMARK_ITEMS MI_MOVIE_BOOK_USER_MAX // we just use the same size as used in Movie info (MAX_NUMBER_OF_BOOKMARK_ITEMS is used for the number of menu items)
#define MOVIEBROWSER_SETTINGS_FILE          CONFIGDIR "/moviebrowser.conf"

#define MIN_BROWSER_FRAME_HEIGHT 100
#define MAX_BROWSER_FRAME_HEIGHT 400

/* !!!! Do NOT change the order of the enum, just add items at the end !!!! */
typedef enum
{
	MB_INFO_FILENAME 		= 0,
	MB_INFO_FILEPATH 		= 1,
	MB_INFO_TITLE 			= 2,
	MB_INFO_SERIE 			= 3,
	MB_INFO_INFO1 			= 4,
	MB_INFO_MAJOR_GENRE 		= 5,
	MB_INFO_MINOR_GENRE 		= 6,
	MB_INFO_INFO2 			= 7,
	MB_INFO_PARENTAL_LOCKAGE	= 8,
	MB_INFO_CHANNEL 		= 9,
	MB_INFO_BOOKMARK		= 10,
	MB_INFO_QUALITY 		= 11,
	MB_INFO_PREVPLAYDATE 		= 12,
	MB_INFO_RECORDDATE 		= 13,
	MB_INFO_PRODDATE 		= 14,
	MB_INFO_COUNTRY 		= 15,
	MB_INFO_GEOMETRIE 		= 16,
	MB_INFO_AUDIO 			= 17,
	MB_INFO_LENGTH 			= 18,
	MB_INFO_SIZE 			= 19, 
	MB_INFO_MAX_NUMBER		= 20 	// MUST be allways the last item in the list
}MB_INFO_ITEM;

typedef enum
{	
	MB_DIRECTION_AUTO = 0,
	MB_DIRECTION_UP = 1,
	MB_DIRECTION_DOWN = 2,
	MB_DIRECTION_MAX_NUMBER = 3	// MUST be allways the last item in the list
}MB_DIRECTION;

typedef struct
{
	MB_INFO_ITEM item;
	MB_DIRECTION direction;	
}MB_SORTING;

typedef enum
{
	MB_STORAGE_TYPE_UNDEFINED = 0,
	MB_STORAGE_TYPE_NFS = 1,
	MB_STORAGE_TYPE_VLC = 2,
	MB_STORAGE_MAX_NUMBER = 3	// MUST be allways the last item in the list
}MB_STORAGE_TYPE;

typedef struct
{
	MB_INFO_ITEM item;
	std::string optionString;
	int optionVar;
}MB_FILTER;

typedef enum
{
	MB_FOCUS_BROWSER = 0,
	MB_FOCUS_LAST_PLAY = 1,
	MB_FOCUS_LAST_RECORD = 2,
	MB_FOCUS_MOVIE_INFO = 3,
	MB_FOCUS_FILTER = 4,
	MB_FOCUS_MAX_NUMBER = 5	// MUST be allways the last item in the list
}MB_FOCUS;

typedef enum
{
	MB_GUI_BROWSER_ONLY = 0,
	MB_GUI_MOVIE_INFO = 1,
	MB_GUI_LAST_PLAY = 2,
	MB_GUI_LAST_RECORD = 3,
	MB_GUI_FILTER = 4,
	MB_GUI_MAX_NUMBER = 5	// MUST be allways the last item in the list
}MB_GUI;


typedef enum
{
	MB_PARENTAL_LOCK_OFF = 0,
	MB_PARENTAL_LOCK_ACTIVE = 1,
	MB_PARENTAL_LOCK_OFF_TMP = 2, // use this to activate the lock temporarily until next dbox start up
	MB_PARENTAL_LOCK_MAX_NUMBER = 3  // MUST be allways the last item in the list
}MB_PARENTAL_LOCK;

typedef struct
{
    std::string name;
    int* used;
}MB_DIR;

// youtube
typedef enum
{
	MB_SHOW_RECORDS,
	MB_SHOW_FILES,
	MB_SHOW_OTHERS
} MB_SHOW_MODE;

#define MB_MAX_ROWS 6
#define MB_MAX_DIRS 10
/* MB_SETTINGS to be stored in g_settings anytime ....*/
typedef struct
{
	// show mode
	int show_mode;
	
	// moviebrowser
	MB_GUI gui; 				//MB_GUI
	MB_SORTING sorting; 			//MB_SORTING
	MB_FILTER filter;			//MB_FILTER
	MI_PARENTAL_LOCKAGE parentalLockAge ;	//MI_PARENTAL_LOCKAGE
	MB_PARENTAL_LOCK parentalLock;		//MB_PARENTAL_LOCK
	
	std::string storageDir[MB_MAX_DIRS];
	int storageDirUsed[MB_MAX_DIRS];
	int storageDirRecUsed;
	int storageDirMovieUsed;
	
	int reload;
	int remount;
	
	int browser_serie_mode;
	int serie_auto_create;
	int prefer_tmdb_info;
	
	// these variables are used for the listframes
	int browserFrameHeight;
	int browserRowNr;
	MB_INFO_ITEM browserRowItem[MB_MAX_ROWS];//MB_INFO_ITEM
	int browserRowWidth[MB_MAX_ROWS];

	// to be added to config later 
    	int lastPlayMaxItems;
	int lastPlayRowNr;
	MB_INFO_ITEM lastPlayRow[MB_MAX_ROWS];
	int lastPlayRowWidth[MB_MAX_ROWS];

    	int lastRecordMaxItems;
	int lastRecordRowNr;
	MB_INFO_ITEM lastRecordRow[MB_MAX_ROWS];
	int lastRecordRowWidth[MB_MAX_ROWS];
}MB_SETTINGS;

// Priorities for Developmemt: P1: critical feature, P2: important feature, P3: for next release, P4: looks nice, lets see
class CMovieBrowser : public CMenuTarget
{
	public: // Variables /////////////////////////////////////////////////
		int Multi_Select;    // for FileBrowser compatibility, not used in MovieBrowser
		int Dirs_Selectable; // for FileBrowser compatibility, not used in MovieBrowser
		
	private: // Variables
		CFrameBuffer * m_pcWindow;

		CListFrame * m_pcBrowser;
		CListFrame * m_pcLastPlay;
		CListFrame * m_pcLastRecord;
		CTextBox * m_pcInfo;
		CListFrame * m_pcFilter;  
	
		CBox m_cBoxFrame;
		CBox m_cBoxFrameLastPlayList;
		CBox m_cBoxFrameLastRecordList;
		CBox m_cBoxFrameBrowserList;
		CBox m_cBoxFrameInfo;
		CBox m_cBoxFrameBookmarkList;
		CBox m_cBoxFrameFilter;
		CBox m_cBoxFrameFootRel;
		CBox m_cBoxFrameTitleRel;
		
		LF_LINES m_browserListLines;
		LF_LINES m_recordListLines;
		LF_LINES m_playListLines;
		LF_LINES m_FilterLines;

		std::vector<MI_MOVIE_INFO> m_vMovieInfo;
		std::vector<MI_MOVIE_INFO*> m_vHandleBrowserList;
		std::vector<MI_MOVIE_INFO*> m_vHandleRecordList;
		std::vector<MI_MOVIE_INFO*> m_vHandlePlayList;
		std::vector<std::string> m_dirNames;
        	std::vector<MI_MOVIE_INFO*> m_vHandleSerienames;

		unsigned int m_currentBrowserSelection;
		unsigned int m_currentRecordSelection;
		unsigned int m_currentPlaySelection;
		unsigned int m_currentFilterSelection;
 		unsigned int m_prevBrowserSelection;
		unsigned int m_prevRecordSelection;
		unsigned int m_prevPlaySelection;

		bool m_showBrowserFiles;
		bool m_showLastRecordFiles;
		bool m_showLastPlayFiles;
		bool m_showMovieInfo;
		bool m_showFilter;

		MI_MOVIE_INFO * m_movieSelectionHandler;
		int m_currentStartPos;
		std::string m_selectedDir;
		MB_FOCUS m_windowFocus;

		bool m_file_info_stale; // if this bit is set, MovieBrowser shall reload all movie infos from HD
		bool m_seriename_stale;

		static CFont * m_pcFontFoot;
		static CFont * m_pcFontTitle;
		
		std::string m_textTitle;
		
		MB_PARENTAL_LOCK m_parentalLock;
		MB_STORAGE_TYPE m_storageType;
	
		CConfigFile configfile;
		CMovieInfo m_movieInfo;
		MB_SETTINGS m_settings;
		std::vector<MB_DIR> m_dir;
			
		int movieInfoUpdateAll[MB_INFO_MAX_NUMBER];
		int movieInfoUpdateAllIfDestEmptyOnly;

		int show_mode;
		
	public:  // Functions //////////////////////////////////////////////////////////7
		CMovieBrowser(const char * path); //P1 
		CMovieBrowser(); //P1 
		~CMovieBrowser(); //P1 
		int exec(const char* path); //P1 
        	int exec(CMenuTarget* parent, const std::string & actionKey);
		std::string getCurrentDir(void); //P1 for FileBrowser compatibility
		CFile * getSelectedFile(void); //P1 for FileBrowser compatibility
		MI_MOVIE_BOOKMARKS* getCurrentMovieBookmark(void){if(m_movieSelectionHandler == NULL) return NULL; return(&(m_movieSelectionHandler->bookmarks));};
		int getCurrentStartPos(void){return(m_currentStartPos);}; //P1 return start position in [s]
		MI_MOVIE_INFO* getCurrentMovieInfo(void){return(m_movieSelectionHandler);}; //P1 return start position in [s]
		void fileInfoStale(void); // call this function to force the Moviebrowser to reload all movie information from HD

		bool readDir(const std::string & dirname, CFileList* flist);

		bool delFile(CFile& file);
		
		int  getMode() { return show_mode; }
		void  setMode(int mode) { show_mode = mode; }
	
	private: //Functions
		///// MovieBrowser init /////////////// 
		void init(void); //P1
		void initGlobalSettings(void); //P1
		void initFrames(void);
		void initDevelopment(void); //P1 for development testing only
		void initRows(void);
		void reinit(void); //P1

		///// MovieBrowser Main Window////////// 
		int paint(void); //P1
		void refresh(void); //P1
        	void hide(void); //P1
		void refreshLastPlayList(void); //P2
		void refreshLastRecordList(void); //P2
		void refreshBrowserList(void); //P1
		void refreshFilterList(void); //P1
		void refreshMovieInfo(void); //P1
		void refreshBookmarkList(void); // P3
		void refreshFoot(void); //P2
		void refreshTitle(void); //P2
		void refreshInfo(void); // P2
		void refreshLCD(void); // P2
	
		///// Events ///////////////////////////
		bool onButtonPress(neutrino_msg_t msg); // P1
		bool onButtonPressMainFrame(neutrino_msg_t msg); // P1
		bool onButtonPressBrowserList(neutrino_msg_t msg); // P1
		bool onButtonPressLastPlayList(neutrino_msg_t msg); // P2
		bool onButtonPressLastRecordList(neutrino_msg_t msg); // P2
		bool onButtonPressFilterList(neutrino_msg_t msg); // P2
		bool onButtonPressBookmarkList(neutrino_msg_t msg); // P3
		bool onButtonPressMovieInfoList(neutrino_msg_t msg); // P2
		void onSetFocus(MB_FOCUS new_focus); // P2
		void onSetFocusNext(void); // P2
		//void onSetFocusPrev(void); // P2
		void onSetGUIWindow(MB_GUI gui);
		void onSetGUIWindowNext(void);
		void onSetGUIWindowPrev(void);
		void onDeleteFile(MI_MOVIE_INFO& movieSelectionHandler);  // P4
		bool onSortMovieInfoHandleList(std::vector<MI_MOVIE_INFO*>& pv_handle_list, MB_INFO_ITEM sort_type, MB_DIRECTION direction);
		
		///// parse Storage Directories ///////////// 
        	bool addDir(std::string& dirname, int* used);
        	void updateDir(void);
		void loadAllTsFileNamesFromStorage(void); // P1
		bool loadTsFileNamesFromDir(const std::string & dirname); // P1
		void getStorageInfo(void); // P3
		
		///// Menu //////////////////////////////////// 
		bool showMenu(MI_MOVIE_INFO* movie_info); // P2
        	void showMovieInfoMenu(MI_MOVIE_INFO* movie_info); // P2
		int  showStartPosSelectionMenu(void); // P2
		
		///// settings /////////////////////////////////// 
		bool loadSettings(MB_SETTINGS* settings); // P2
		bool saveSettings(MB_SETTINGS* settings); // P2
        	void defaultSettings(MB_SETTINGS* settings);
		
		///// EPG_DATA /XML /////////////////////////////// 
		void loadMovies();
		void loadAllMovieInfo(void); // P1
		void saveMovieInfo(std::string* filename, MI_MOVIE_INFO* movie_info); // P2
	
		// misc
		void showHelp(void);
		bool isFiltered(MI_MOVIE_INFO& movie_info);
		bool isParentalLock(MI_MOVIE_INFO& movie_info);
		bool getMovieInfoItem(MI_MOVIE_INFO& movie_info, MB_INFO_ITEM item, std::string* item_string);
		void updateMovieSelection(void);
		void updateFilterSelection(void);
		void updateSerienames(void);
        	void autoFindSerie(void);
};

// Class to show Moviebrowser Information, to be used by menu
class CMovieHelp : public CMenuTarget
{
	public:
		CMovieHelp(){};
		~CMovieHelp(){};
		int exec( CMenuTarget* parent, const std::string & actionKey );
};

//
class CFileChooser : public CMenuWidget
{
	private:
		std::string *dirPath;

	public:
		CFileChooser(std::string *path){dirPath = path;};
		int exec(CMenuTarget *parent, const std::string & actionKey);
 };

typedef enum
{
	DIR_STATE_UNKNOWN = 0,
	DIR_STATE_SERVER_DOWN = 1,
	DIR_STATE_NOT_MOUNTED = 2,
	DIR_STATE_MOUNTED = 3,
	DIR_STATE_DISABLED = 4
}DIR_STATE;

#define MAX_DIR 10
class CDirMenu : public CMenuWidget
{
	private:
		std::vector<MB_DIR>* dirList;
		DIR_STATE dirState[MAX_DIR];
		std::string dirOptionText[MAX_DIR];
		int dirNfsMountNr[MAX_DIR];
		bool changed;

		void updateDirState(void);

	public:
		CDirMenu(std::vector<MB_DIR>* dir_list);
		int exec(CMenuTarget* parent, const std::string & actionKey);
		void show(void);
		bool isChanged(){return changed;};
 };


// EPG Genre , taken from epgview, TODO: migth be splitted in major/minor to increase handling, might be moved to CMovieInfo
#define GENRE_ALL_COUNT 76
const CMenuOptionChooser::keyval GENRE_ALL[GENRE_ALL_COUNT] =
{
	{ 0x00, LOCALE_GENRE_UNKNOWN, NULL },
	{ 0x10, LOCALE_GENRE_MOVIE_0, NULL },
	{ 0x11, LOCALE_GENRE_MOVIE_1, NULL },
	{ 0x12, LOCALE_GENRE_MOVIE_2, NULL },
	{ 0x13, LOCALE_GENRE_MOVIE_3, NULL },
	{ 0x14, LOCALE_GENRE_MOVIE_4, NULL },
	{ 0x15, LOCALE_GENRE_MOVIE_5, NULL },
	{ 0x16, LOCALE_GENRE_MOVIE_6, NULL },
	{ 0x17, LOCALE_GENRE_MOVIE_7, NULL },
	{ 0x18, LOCALE_GENRE_MOVIE_8, NULL },
	{ 0x20, LOCALE_GENRE_NEWS_0, NULL },
	{ 0x21, LOCALE_GENRE_NEWS_1, NULL },
	{ 0x22, LOCALE_GENRE_NEWS_2, NULL },
	{ 0x23, LOCALE_GENRE_NEWS_3, NULL },
	{ 0x24, LOCALE_GENRE_NEWS_4, NULL },
	{ 0x30, LOCALE_GENRE_SHOW_0, NULL },
	{ 0x31, LOCALE_GENRE_SHOW_1, NULL },
	{ 0x32, LOCALE_GENRE_SHOW_2, NULL },
	{ 0x33, LOCALE_GENRE_SHOW_3, NULL },
	{ 0x40, LOCALE_GENRE_SPORTS_0, NULL },
	{ 0x41, LOCALE_GENRE_SPORTS_1, NULL },
	{ 0x42, LOCALE_GENRE_SPORTS_2, NULL },
	{ 0x43, LOCALE_GENRE_SPORTS_3, NULL },
	{ 0x44, LOCALE_GENRE_SPORTS_4, NULL },
	{ 0x45, LOCALE_GENRE_SPORTS_5, NULL },
	{ 0x46, LOCALE_GENRE_SPORTS_6, NULL },
	{ 0x47, LOCALE_GENRE_SPORTS_7, NULL },
	{ 0x48, LOCALE_GENRE_SPORTS_8, NULL },
	{ 0x49, LOCALE_GENRE_SPORTS_9, NULL },
	{ 0x4A, LOCALE_GENRE_SPORTS_10, NULL },
	{ 0x4B, LOCALE_GENRE_SPORTS_11, NULL },
	{ 0x50, LOCALE_GENRE_CHILDRENS_PROGRAMMES_0, NULL },
	{ 0x51, LOCALE_GENRE_CHILDRENS_PROGRAMMES_1, NULL },
	{ 0x52, LOCALE_GENRE_CHILDRENS_PROGRAMMES_2, NULL },
	{ 0x53, LOCALE_GENRE_CHILDRENS_PROGRAMMES_3, NULL },
	{ 0x54, LOCALE_GENRE_CHILDRENS_PROGRAMMES_4, NULL },
	{ 0x55, LOCALE_GENRE_CHILDRENS_PROGRAMMES_5, NULL },
	{ 0x60, LOCALE_GENRE_MUSIC_DANCE_0, NULL },
	{ 0x61, LOCALE_GENRE_MUSIC_DANCE_1, NULL },
	{ 0x62, LOCALE_GENRE_MUSIC_DANCE_2, NULL },
	{ 0x63, LOCALE_GENRE_MUSIC_DANCE_3, NULL },
	{ 0x64, LOCALE_GENRE_MUSIC_DANCE_4, NULL },
	{ 0x65, LOCALE_GENRE_MUSIC_DANCE_5, NULL },
	{ 0x66, LOCALE_GENRE_MUSIC_DANCE_6, NULL },
	{ 0x70, LOCALE_GENRE_ARTS_0, NULL },
	{ 0x71, LOCALE_GENRE_ARTS_1, NULL },
	{ 0x72, LOCALE_GENRE_ARTS_2, NULL },
	{ 0x73, LOCALE_GENRE_ARTS_3, NULL },
	{ 0x74, LOCALE_GENRE_ARTS_4, NULL },
	{ 0x75, LOCALE_GENRE_ARTS_5, NULL },
	{ 0x76, LOCALE_GENRE_ARTS_6, NULL },
	{ 0x77, LOCALE_GENRE_ARTS_7, NULL },
	{ 0x78, LOCALE_GENRE_ARTS_8, NULL },
	{ 0x79, LOCALE_GENRE_ARTS_9, NULL },
	{ 0x7A, LOCALE_GENRE_ARTS_10, NULL },
	{ 0x7B, LOCALE_GENRE_ARTS_11, NULL },
	{ 0x80, LOCALE_GENRE_SOCIAL_POLITICAL_0, NULL },
	{ 0x81, LOCALE_GENRE_SOCIAL_POLITICAL_1, NULL },
	{ 0x82, LOCALE_GENRE_SOCIAL_POLITICAL_2, NULL },
	{ 0x83, LOCALE_GENRE_SOCIAL_POLITICAL_3, NULL },
	{ 0x90, LOCALE_GENRE_DOCUS_MAGAZINES_0, NULL },
	{ 0x91, LOCALE_GENRE_DOCUS_MAGAZINES_1, NULL },
	{ 0x92, LOCALE_GENRE_DOCUS_MAGAZINES_2, NULL },
	{ 0x93, LOCALE_GENRE_DOCUS_MAGAZINES_3, NULL },
	{ 0x94, LOCALE_GENRE_DOCUS_MAGAZINES_4, NULL },
	{ 0x95, LOCALE_GENRE_DOCUS_MAGAZINES_5, NULL },
	{ 0x96, LOCALE_GENRE_DOCUS_MAGAZINES_6, NULL },
	{ 0x97, LOCALE_GENRE_DOCUS_MAGAZINES_7, NULL },
	{ 0xA0, LOCALE_GENRE_TRAVEL_HOBBIES_0, NULL },
	{ 0xA1, LOCALE_GENRE_TRAVEL_HOBBIES_1, NULL },
	{ 0xA2, LOCALE_GENRE_TRAVEL_HOBBIES_2, NULL },
	{ 0xA3, LOCALE_GENRE_TRAVEL_HOBBIES_3, NULL },
	{ 0xA4, LOCALE_GENRE_TRAVEL_HOBBIES_4, NULL },
	{ 0xA5, LOCALE_GENRE_TRAVEL_HOBBIES_5, NULL },
	{ 0xA6, LOCALE_GENRE_TRAVEL_HOBBIES_6, NULL },
	{ 0xA7, LOCALE_GENRE_TRAVEL_HOBBIES_7, NULL }
};

#define GENRE_MOVIE_COUNT 9
const CMenuOptionChooser::keyval genre_movie[GENRE_MOVIE_COUNT] =
{
	{ 0, LOCALE_GENRE_MOVIE_0, NULL },
	{ 1, LOCALE_GENRE_MOVIE_1, NULL },
	{ 2, LOCALE_GENRE_MOVIE_2, NULL },
	{ 3, LOCALE_GENRE_MOVIE_3, NULL },
	{ 4, LOCALE_GENRE_MOVIE_4, NULL },
	{ 5, LOCALE_GENRE_MOVIE_5, NULL },
	{ 6, LOCALE_GENRE_MOVIE_6, NULL },
	{ 7, LOCALE_GENRE_MOVIE_7, NULL },
	{ 8, LOCALE_GENRE_MOVIE_8, NULL }
};

#define GENRE_NEWS_COUNT 5
const CMenuOptionChooser::keyval genre_news[GENRE_NEWS_COUNT] =
{
	{ 0, LOCALE_GENRE_NEWS_0, NULL },
	{ 1, LOCALE_GENRE_NEWS_1, NULL },
	{ 2, LOCALE_GENRE_NEWS_2, NULL },
	{ 3, LOCALE_GENRE_NEWS_3, NULL },
	{ 4, LOCALE_GENRE_NEWS_4, NULL }
};

#define GENRE_SHOW_COUNT 4
const CMenuOptionChooser::keyval genre_show[GENRE_SHOW_COUNT] =
{
	{ 0, LOCALE_GENRE_SHOW_0, NULL },
	{ 1, LOCALE_GENRE_SHOW_1, NULL },
	{ 2, LOCALE_GENRE_SHOW_2, NULL },
	{ 3, LOCALE_GENRE_SHOW_3, NULL }
};

#define GENRE_SPORTS_COUNT 12
const CMenuOptionChooser::keyval genre_sports[GENRE_SPORTS_COUNT] =
{
	{ 0, LOCALE_GENRE_SPORTS_0, NULL },
	{ 1, LOCALE_GENRE_SPORTS_1, NULL },
	{ 2, LOCALE_GENRE_SPORTS_2, NULL },
	{ 3, LOCALE_GENRE_SPORTS_3, NULL },
	{ 4, LOCALE_GENRE_SPORTS_4, NULL },
	{ 5, LOCALE_GENRE_SPORTS_5, NULL },
	{ 6, LOCALE_GENRE_SPORTS_6, NULL },
	{ 7, LOCALE_GENRE_SPORTS_7, NULL },
	{ 8, LOCALE_GENRE_SPORTS_8, NULL },
	{ 9, LOCALE_GENRE_SPORTS_9, NULL },
	{10, LOCALE_GENRE_SPORTS_10, NULL },
	{11, LOCALE_GENRE_SPORTS_11, NULL }
};

#define GENRE_CHILDRENS_PROGRAMMES_COUNT 6
const CMenuOptionChooser::keyval genre_childrens_programmes[GENRE_CHILDRENS_PROGRAMMES_COUNT] =
{
	{ 0, LOCALE_GENRE_CHILDRENS_PROGRAMMES_0, NULL },
	{ 1, LOCALE_GENRE_CHILDRENS_PROGRAMMES_1, NULL },
	{ 2, LOCALE_GENRE_CHILDRENS_PROGRAMMES_2, NULL },
	{ 3, LOCALE_GENRE_CHILDRENS_PROGRAMMES_3, NULL },
	{ 4, LOCALE_GENRE_CHILDRENS_PROGRAMMES_4, NULL },
	{ 5, LOCALE_GENRE_CHILDRENS_PROGRAMMES_5, NULL }
};

#define GENRE_MUSIC_DANCE_COUNT 7
const CMenuOptionChooser::keyval genre_music_dance[GENRE_MUSIC_DANCE_COUNT] =
{
	{ 0, LOCALE_GENRE_MUSIC_DANCE_0, NULL },
	{ 1, LOCALE_GENRE_MUSIC_DANCE_1, NULL },
	{ 2, LOCALE_GENRE_MUSIC_DANCE_2, NULL },
	{ 3, LOCALE_GENRE_MUSIC_DANCE_3, NULL },
	{ 4, LOCALE_GENRE_MUSIC_DANCE_4, NULL },
	{ 5, LOCALE_GENRE_MUSIC_DANCE_5, NULL },
	{ 6, LOCALE_GENRE_MUSIC_DANCE_6, NULL }
};

#define GENRE_ARTS_COUNT 12
const CMenuOptionChooser::keyval genre_arts_dance[GENRE_ARTS_COUNT] =
{
	{ 0, LOCALE_GENRE_ARTS_0, NULL },
	{ 1, LOCALE_GENRE_ARTS_1, NULL },
	{ 2, LOCALE_GENRE_ARTS_2, NULL },
	{ 3, LOCALE_GENRE_ARTS_3, NULL },
	{ 4, LOCALE_GENRE_ARTS_4, NULL },
	{ 5, LOCALE_GENRE_ARTS_5, NULL },
	{ 6, LOCALE_GENRE_ARTS_6, NULL },
	{ 7, LOCALE_GENRE_ARTS_7, NULL },
	{ 8, LOCALE_GENRE_ARTS_8, NULL },
	{ 9, LOCALE_GENRE_ARTS_9, NULL },
	{10, LOCALE_GENRE_ARTS_10, NULL },
	{11, LOCALE_GENRE_ARTS_11, NULL }
};

#define GENRE_SOCIAL_POLITICAL_COUNT 4
const CMenuOptionChooser::keyval genre_social_political[GENRE_SOCIAL_POLITICAL_COUNT] =
{
	{ 0, LOCALE_GENRE_SOCIAL_POLITICAL_0, NULL },
	{ 1, LOCALE_GENRE_SOCIAL_POLITICAL_1, NULL },
	{ 2, LOCALE_GENRE_SOCIAL_POLITICAL_2, NULL },
	{ 3, LOCALE_GENRE_SOCIAL_POLITICAL_3, NULL }
};

#define GENRE_DOCUS_MAGAZINES_COUNT 8
const CMenuOptionChooser::keyval genre_docus_magazines[GENRE_DOCUS_MAGAZINES_COUNT] =
{
	{ 0, LOCALE_GENRE_DOCUS_MAGAZINES_0, NULL },
	{ 1, LOCALE_GENRE_DOCUS_MAGAZINES_1, NULL },
	{ 2, LOCALE_GENRE_DOCUS_MAGAZINES_2, NULL },
	{ 3, LOCALE_GENRE_DOCUS_MAGAZINES_3, NULL },
	{ 4, LOCALE_GENRE_DOCUS_MAGAZINES_4, NULL },
	{ 5, LOCALE_GENRE_DOCUS_MAGAZINES_5, NULL },
	{ 6, LOCALE_GENRE_DOCUS_MAGAZINES_6, NULL },
	{ 7, LOCALE_GENRE_DOCUS_MAGAZINES_7, NULL }
};

#define GENRE_TRAVEL_HOBBIES_COUNT 8
const CMenuOptionChooser::keyval genre_travel_hobbies[GENRE_TRAVEL_HOBBIES_COUNT] =
{
	{ 0, LOCALE_GENRE_TRAVEL_HOBBIES_0, NULL },
	{ 1, LOCALE_GENRE_TRAVEL_HOBBIES_1, NULL },
	{ 2, LOCALE_GENRE_TRAVEL_HOBBIES_2, NULL },
	{ 3, LOCALE_GENRE_TRAVEL_HOBBIES_3, NULL },
	{ 4, LOCALE_GENRE_TRAVEL_HOBBIES_4, NULL },
	{ 5, LOCALE_GENRE_TRAVEL_HOBBIES_5, NULL },
	{ 6, LOCALE_GENRE_TRAVEL_HOBBIES_6, NULL },
	{ 7, LOCALE_GENRE_TRAVEL_HOBBIES_7, NULL }
};

#define GENRE_MAJOR_COUNT 10
const CMenuOptionChooser::keyval genre_major[GENRE_MAJOR_COUNT] =
{
	{ 1, LOCALE_GENRE_MOVIE_0, NULL },
	{ 2, LOCALE_GENRE_NEWS_0, NULL },
	{ 3, LOCALE_GENRE_SHOW_0, NULL },
	{ 4, LOCALE_GENRE_SPORTS_0, NULL },
	{ 5, LOCALE_GENRE_CHILDRENS_PROGRAMMES_0, NULL },
	{ 6, LOCALE_GENRE_MUSIC_DANCE_0, NULL },
	{ 7, LOCALE_GENRE_ARTS_0, NULL },
	{ 8, LOCALE_GENRE_SOCIAL_POLITICAL_0, NULL },
	{ 9, LOCALE_GENRE_DOCUS_MAGAZINES_0, NULL },
	{ 10, LOCALE_GENRE_TRAVEL_HOBBIES_0, NULL }
};

// onoff notifier //needed by moviebrowser
class COnOffNotifier : public CChangeObserver
{
        private:
                int number;
                CMenuItem * toDisable[15];
        public:
                COnOffNotifier(CMenuItem* a1, CMenuItem* a2 = NULL, CMenuItem* a3 = NULL, CMenuItem* a4 = NULL, CMenuItem* a5 = NULL);
		COnOffNotifier(int OffValue = 0);
                bool changeNotify(const neutrino_locale_t, void *Data);
		void addItem(CMenuItem* menuItem);
};

#endif /*MOVIEBROWSER_H_*/




