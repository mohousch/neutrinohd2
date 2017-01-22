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


class CNetzKinoBrowser : public CMenuTarget
{
	private:
		// 		
		cNKFeedParser nkparser;
		nk_category_list_t cats;

		//
		void showNKCategoriesMenu();

	public:
		CNetzKinoBrowser();
		~CNetzKinoBrowser();
		
		int exec(CMenuTarget* parent, const std::string& actionKey);
};

class CNKMovies : public CMenuTarget
{
	private:
		//
		std::vector<MI_MOVIE_INFO> m_vMovieInfo;
		CMovieInfo m_movieInfo;

		//
		cNKFeedParser nkparser;

		//
		int catID;
		int catMode;
		std::string caption;

		CMenuFrameBox* moviesMenu;

		void loadNKTitles(int mode, std::string search, int id);

		void showNKMoviesMenu();
		void playMovie(void);
		void showMovieInfo(void);
	public:
		CNKMovies(int mode, int id, std::string title);
		~CNKMovies();
		int exec(CMenuTarget* parent, const std::string& actionKey);
};

#endif //__NK__

