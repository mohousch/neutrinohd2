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
	float vote_average;
	int         vote_count;
	int         id;
	std::string media_type;
	//int         result;
	int         runtime;
	std::string runtimes;
	std::string genres;
	int         episodes;
	int         seasons;
	std::string cast;
	float popularity;
	bool video;
	std::string original_language;

	//
	//std::string cover;

	//
	std::string vid;
	std::string vkey;
	std::string vname;
	std::string vtype;
}tmdbinfo;

class CTmdb
{
	private:
		std::vector<tmdbinfo> minfo;
		std::string key; // tmdb api key
		std::string lang;
		std::string thumbnail_dir;

		CFileHelpers fileHelper;

		//
		std::vector<tmdbinfo> movieInfo;
		std::vector<tmdbinfo> movieList;
		std::vector<tmdbinfo> genreList;
		std::vector<tmdbinfo> genreMovieList;
		std::vector<tmdbinfo> seasonList;
		std::vector<tmdbinfo> episodeList;
		std::vector<tmdbinfo> videoInfo;

	public:
		CTmdb();
		~CTmdb();

		bool getMovieInfo(std::string text); //search

		//
		std::string createInfoText();

		//
		std::string getTitle(){ return minfo[0].title;}
		std::string getOriginaleTitle(){ return minfo[0].original_title;}
		std::string getReleaseDate(){ return minfo[0].release_date;}
		std::string getDescription(){ return minfo[0].overview;}
		std::string getCast(){ return minfo[0].cast;}
		int getID(){return minfo[0].id;};
		float getVoteAverage(){return minfo[0].vote_average;};
		int getVoteCount(){return minfo[0].vote_count;};
		float getPopularity(){return minfo[0].popularity;};
		bool getVideo(){return minfo[0].video;};
		std::string getPosterPath(){return minfo[0].poster_path;};
		std::string getOriginalLanguage(){return minfo[0].original_language;};

		//
		std::string getThumbnailDir(void){return thumbnail_dir;};

		//
		std::string getVID(){return minfo[0].vid;};
		std::string getVKey(){return minfo[0].vkey;};
		std::string getVName(){return minfo[0].vname;};
		std::string getVType(){return minfo[0].vtype;};

		//
		bool getBigCover(std::string cover, std::string tname);
		bool getSmallCover(std::string cover, std::string tname);


		//
		bool getMovieTVInfo(std::string mtype = "movie", int id = 0); //FIXME
		bool getMovieTVList(std::string mtype = "movie", std::string list = "now_playing", int page = 1);
		bool getGenreList(std::string mtype = "movie");
		bool getGenreMovieList(int id);
		bool getSeasonsList(int id);
		bool getEpisodesList(int id, int nr = 0);
		bool getVideoInfo(std::string mtype = "movie", int id = 0, int s_nr = 0); //FIXME
		bool searchMovieInfo(std::string text);

		// minfo
		void clearMInfo(void){return minfo.clear();};
		std::vector<tmdbinfo>& getMInfos(){return minfo;};

		// movie list
		void clearMovieList(void){movieList.clear();};
		std::vector<tmdbinfo>& getMovies(){return movieList;};

		// genre list
		void clearGenreList(void) {genreList.clear();};
		std::vector<tmdbinfo>& getGenres(){return genreList;};

		// genre movie list
		void clearGenreMovieList(void){genreMovieList.clear();};
		std::vector<tmdbinfo>& getGenreMovies(){return genreMovieList;};

		// season list
		void clearSeasonList(void){seasonList.clear();};
		std::vector<tmdbinfo>& getSeasons(){return seasonList;};

		// episode list
		void clearEpisodeList(void){episodeList.clear();};
		std::vector<tmdbinfo>& getEpisodes(){return episodeList;};

		// movie info
		void clearMovieInfo(void){movieInfo.clear();};
		std::vector<tmdbinfo>& getMovieInfos(){return movieInfo;};

		// video info
		void clearVideoInfo(void){videoInfo.clear();};
		std::vector<tmdbinfo>& getVideoInfos(){return videoInfo;};
};

#endif
