/*
  $Id: netzkino.h 2014/10/03 mohousch Exp $

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

#ifndef __NK__
#define __NK__

#include <plugin.h>
#include <nkparser.h>


#define MAX_ITEMS_PER_PAGE	18
#define MAX_ITEMS_PER_X		6
#define MAX_ITEMS_PER_Y		3

typedef struct
{
	// netzkino	
	int nkmode;
	int nkcategory;
	std::string nkcategoryname;
	std::string nksearch;
}NKB_SETTINGS;

class CNetzKinoBrowser : public CMenuTarget
{
	private:
		CFrameBuffer * m_pcWindow;
		CBox Box;
		CBox frameBox;

		int selected;
		int oldselected;

		int x;
		int y;

		int itemsCountPerPage;
		
		std::vector<MI_MOVIE_INFO> m_vMovieInfo;
		
		NKB_SETTINGS m_settings;
		
		CMovieInfo m_movieInfo;

		bool reload_movies;

		void init(void); 
		void initGlobalSettings(void); 
		void initFrames(void);
		void initFrameBox(void);
		void paintHead(void);
		void paintFoot(void);
		void paintBody(void);
		void paintItems(int itemsCount = MAX_ITEMS_PER_PAGE);
		void paintInfo(void);
		void paintItemBox(int oldposx = 0, int oldposy = 0, int posx = 0, int posy = 0);
		void paint(int itemsCount = MAX_ITEMS_PER_PAGE);
		
        	void hide(void); 
		
		void loadMovies();
		
		// netzkino		
		cNKFeedParser nkparser;
		std::string nkcategory_name;
		
		void loadNKTitles(int mode, std::string search, int id, unsigned int start, unsigned int end);
		bool showNKMenu();
		int videoListsize;
		
		int NKStart, NKEnd;
		
	public:
		CNetzKinoBrowser();
		~CNetzKinoBrowser();
		
		int exec(CMenuTarget* parent, const std::string& actionKey);

		CFile * getSelectedFile(void); 
		int exec();
		int getSelected(){ return selected;};
		int getItemsCountPerPage(){ return itemsCountPerPage;};

		MI_MOVIE_INFO* getCurrentMovieInfo(void){return(&m_vMovieInfo[selected]);};
};

#endif //__NK__

