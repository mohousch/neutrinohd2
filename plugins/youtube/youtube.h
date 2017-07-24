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

// settings
typedef struct
{
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
		std::vector<MI_MOVIE_INFO> m_vMovieInfo;

		std::string m_textTitle;
		
		CConfigFile configfile;
		
		CMovieInfo m_movieInfo;
		
		//
		cYTFeedParser ytparser;

		//
		CMenuFrameBox* moviesMenu;
		
		void init(void);
		bool loadSettings(YTB_SETTINGS* settings);
		bool saveSettings(YTB_SETTINGS* settings);
		
		void loadYTTitles(int mode, std::string search = "", std::string id = "");
		bool showYTMenu(void);
		void playMovie(void);
		void showMovieInfo(void);
		
		void showYTMoviesMenu(void);

		neutrino_locale_t getFeedLocale(void);
		
	public:
		CYTBrowser();
		~CYTBrowser();
		int exec(CMenuTarget* parent, const std::string & actionKey);
}; 

#endif //__YT__
