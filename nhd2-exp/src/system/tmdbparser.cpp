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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fstream>

#include <set>
#include <string>

#include <system/helpers.h>
#include <system/debug.h>

#include <jsoncpp/include/json/json.h>

#include <global.h>

#include <system/settings.h>
#include <system/helpers.h>
#include <system/tmdbparser.h>


CTmdb::CTmdb()
{
	thumbnail_dir = "/tmp/tmdbparser";

	key = "507930c8d6d400c85eae3a7e7b3f6c78";

	std::string language = g_settings.language;
	lang = Lang2ISO639_1(language);

	fileHelper.createDir(thumbnail_dir.c_str(), 0755);
}

CTmdb::~CTmdb()
{
	fileHelper.removeDir(thumbnail_dir.c_str());
}

bool CTmdb::getMovieDetails(std::string text, const std::string& request)
{
	dprintf(DEBUG_NORMAL, "cTmdb::GetMovieDetails:\n");

	std::string url	= "http://api.themoviedb.org/3/";

	if(request == "search")
	{
		url += request + "/multi?api_key=" + key + "&language=" + lang + "&query=" + encodeUrl(text);
	}

	minfo.title = text;

	std::string answer;
	if (!::getUrl(url, answer))
		return false;

	Json::Value root;
	Json::Reader reader;
	bool parsedSuccess = reader.parse(answer, root);

	if (!parsedSuccess) 
	{
		dprintf(DEBUG_NORMAL, "cTmdb::GetMovieDetails: Failed to parse JSON\n");
		dprintf(DEBUG_NORMAL, "cTmdb::GetMovieDetails: %s\n", reader.getFormattedErrorMessages().c_str());
		return false;
	}

	minfo.result = root.get("total_results", 0).asInt();

	dprintf(DEBUG_NORMAL, "cTmdb::GetMovieDetails: results: %d\n", minfo.result);

	if (minfo.result > 0) 
	{
		Json::Value elements = root["results"];

		minfo.id = elements[0].get("id", -1).asInt();
		minfo.media_type = elements[0].get("media_type", "").asString();

		if (minfo.id > -1) 
		{
			url = "http://api.themoviedb.org/3/" + minfo.media_type + "/" + to_string(minfo.id) + "?api_key=" + key + "&language=" + lang + "&append_to_response=credits";

			answer.clear();
			if (!::getUrl(url, answer))
				return false;

			parsedSuccess = reader.parse(answer, root);
			if (!parsedSuccess) 
			{
				dprintf(DEBUG_NORMAL, "cTmdb::GetMovieDetails: Failed to parse JSON\n");
				dprintf(DEBUG_NORMAL, "cTmdb::GetMovieDetails: %s\n", reader.getFormattedErrorMessages().c_str());

				return false;
			}

			minfo.overview = root.get("overview", "").asString();
			minfo.poster_path = root.get("poster_path", "").asString();
			minfo.original_title = root.get("original_title", "").asString();
			minfo.release_date = root.get("release_date", "").asString();
			minfo.vote_average = root.get("vote_average", "").asString();
			minfo.vote_count = root.get("vote_count", 0).asInt();
			minfo.runtime = root.get("runtime", 0).asInt();

			if (minfo.media_type == "tv") 
			{
				minfo.original_title = root.get("original_name", "").asString();
				minfo.episodes = root.get("number_of_episodes", 0).asInt();
				minfo.seasons = root.get("number_of_seasons", 0).asInt();
				minfo.release_date = root.get("first_air_date", "").asString();

				elements = root["episode_run_time"];
				minfo.runtimes = elements[0].asString();
				for (unsigned int i = 1; i < elements.size(); i++) 
				{
					minfo.runtimes +=  + ", " + elements[i].asString();
				}
			}

			elements = root["genres"];
			minfo.genres = elements[0].get("name", "").asString();
			for (unsigned int i = 1; i < elements.size(); i++) 
			{
				minfo.genres += ", " + elements[i].get("name", "").asString();
			}

			elements = root["credits"]["cast"];
			for (unsigned int i = 0; i < elements.size() && i < 10; i++) 
			{
				minfo.cast +=  "  " + elements[i].get("character", "").asString() + " (" + elements[i].get("name", "").asString() + ")\n";
			}

			//
			if (!minfo.poster_path.empty())
			{
				std::string tname = thumbnail_dir;
				tname += "/";
				tname += minfo.title;
				tname += ".jpg";

				getBigCover(tname);
			}

			return true;
		}
	} 
	else
		return false;

	return false;
}

bool CTmdb::getBigCover(std::string fname)
{ 
	dprintf(DEBUG_NORMAL, "CTmdb::getBigCover: %s\n", fname.c_str());

	bool ret = false;

	if (!minfo.poster_path.empty())
	{
		bool found = false;
		found = ::downloadUrl("http://image.tmdb.org/t/p/w342" + minfo.poster_path, fname);

		if (found)
		{
			minfo.cover = fname;
			cover = fname;
		}

		ret |= found;
	}

	return ret;
}

bool CTmdb::getSmallCover(std::string fname)
{ 
	bool ret = false;

	if (!minfo.poster_path.empty())
	{
		bool found = false;
		found = ::downloadUrl("http://image.tmdb.org/t/p/w185" + minfo.poster_path, fname);
		if (found)
		{
			minfo.cover = fname;
			cover = fname;
		}

		ret |= found;
	}

	return ret;
}

std::string CTmdb::CreateEPGText()
{
	std::string epgtext;
	epgtext += "\n";
	epgtext += "Vote: " + minfo.vote_average.substr(0,3) + "/10 Votecount: " + to_string(minfo.vote_count) + "\n";
	epgtext += "\n";
	epgtext += minfo.overview + "\n";
	epgtext += "\n";

	if (minfo.media_type == "tv")
		epgtext += (std::string)g_Locale->getText(LOCALE_EPGVIEWER_LENGTH) + ": " + minfo.runtimes+"\n";
	else
		epgtext += (std::string)g_Locale->getText(LOCALE_EPGVIEWER_LENGTH) + ": " + to_string(minfo.runtime)+"\n";
	epgtext += (std::string)g_Locale->getText(LOCALE_EPGVIEWER_GENRE) + ": " + minfo.genres + "\n";
	epgtext += (std::string)g_Locale->getText(LOCALE_EPGEXTENDED_ORIGINAL_TITLE) + " : " + minfo.original_title + "\n";
	epgtext += (std::string)g_Locale->getText(LOCALE_EPGEXTENDED_YEAR_OF_PRODUCTION) + " : " + minfo.release_date.substr(0,4) + "\n";
	if (minfo.media_type == "tv")
		epgtext += "Seasons/Episodes: " + to_string(minfo.seasons) + "/" + to_string(minfo.episodes)+"\n";
	if (!minfo.cast.empty())
		epgtext += (std::string)g_Locale->getText(LOCALE_EPGEXTENDED_ACTORS) + ":\n" + minfo.cast + "\n";
	return epgtext;
}

