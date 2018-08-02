/*
	Copyright (C) 2015 TangoCash

	License: GPLv2

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation;

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __TMDB__
#define __TMDB__

#include <string>

#include <system/helpers.h>


typedef struct {
	std::string title;
	std::string poster_path;
	std::string overview;
	std::string original_title;
	std::string release_date;
	std::string vote_average;
	int         vote_count;
	int         id;
	std::string media_type;
	int         result;
	int         runtime;
	std::string runtimes;
	std::string genres;
	int         episodes;
	int         seasons;
	std::string cast;
	std::string cover;

	//
	std::string vid;
	std::string vkey;
	std::string vname;
	std::string vtype;
}tmdbinfo;

class CTmdb
{
	private:
		tmdbinfo minfo;
		std::string key; // tmdb api key
		std::string lang;
		std::string thumbnail_dir;

		CFileHelpers fileHelper;

		std::vector<tmdbinfo> listInfo;

	public:
		CTmdb();
		~CTmdb();
		bool getMovieInfo(std::string text, bool cover = true);
		bool getMovieList(const std::string list = "now_playing", const int page = 1);
		std::string createInfoText();

		std::string getTitle(){ return minfo.title;}
		std::string getOrgTitle(){ return minfo.original_title;}
		std::string getReleaseDate(){ return minfo.release_date;}
		std::string getDescription(){ return minfo.overview;}
		std::string getVote(){ return minfo.vote_average;}
		std::string getCast(){ return minfo.cast;}
		int getResults(){ return minfo.result;}
		int getStars(){ return (int) (atof(minfo.vote_average.c_str()) + 0.5);}
		int getID(){return minfo.id;};
		std::string getCover(){return minfo.cover;};
		std::string getVId(){return minfo.vid;};
		std::string getVKey(){return minfo.vkey;};
		std::string getVName(){return minfo.vname;};
		std::string getVType(){return minfo.vtype;};

		//
		bool getBigCover(std::string tname);
		bool getSmallCover(std::string tname);

		//
		void cleanUp(void){listInfo.clear();};
		std::vector<tmdbinfo>& getList(){return listInfo;};
};

#endif
