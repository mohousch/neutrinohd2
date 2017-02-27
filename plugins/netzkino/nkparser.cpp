/*
        Copyright (C) 2013 CoolStream International Ltd
        Copyright (C) 2013 martii

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

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <set>
#include <map>
#include <vector>
#include <bitset>
#include <string>

#include <jsoncpp/include/json/json.h>

#include <nkparser.h>

#include <system/helpers.h>
#include <system/debug.h>

#include <global.h>


#define MAIN_URL  	"http://www.netzkino.de/capi/"
#define HLS_URL		"http://mf.netzkinomobil.c.nmdn.net/netzkino_mobil/_definst_/mp4:" //%s/playlist.m3u8
#define RTMP_URL 	"rtmp://mf.netzkino.c.nmdn.net/netzkino/_definst_/mp4:"
#define	MP4_URL 	"http://dl.netzkinotv.c.nmdn.net/netzkino_tv/"

cNKFeedParser::cNKFeedParser()
{
	thumbnail_dir = "/tmp/netzkino";
	parsed = false;
	max_results = 500;
	
	movie_dir = g_settings.network_nfs_moviedir;
}

cNKFeedParser::~cNKFeedParser()
{
}

bool cNKFeedParser::parseCategoriesJSON(std::string &answer)
{
	dprintf(DEBUG_NORMAL, "cNKFeedParser::parseCategoriesJSON:\n");
	
	Json::Value root, v;
	Json::Reader reader;

	if (!reader.parse(answer, root))
		return false;

	v = root.get("status", "");
	if (v.type() != Json::stringValue || v.asString() != "ok")
		return false;

	Json::Value cats = root.get("categories", "");
	if (cats.type() != Json::arrayValue)
		return false;

	categories.clear();

	sNKCategory c;
	c.id = 0;
	c.post_count = 0;

	for(unsigned int i = 0; i < cats.size(); ++i) 
	{
		const Json::Value cat = cats[i];
		
		v = cat.get("id", "");
		if (v.type() == Json::intValue || v.type() == Json::uintValue)
			c.id = v.asInt();
		if(c.id > 111 || c.id == 9)
			continue;
		
		v = cat.get("title", "");
		if (v.type() == Json::stringValue)
			c.title = v.asString();
		
		v = cat.get("post_count", "");
		if (v.type() == Json::intValue || v.type() == Json::uintValue)
			c.post_count = v.asInt();
		
		if (c.id > 0)
			categories.push_back(c);
	}
	
	return !categories.empty();
}

bool cNKFeedParser::parseFeedJSON(std::string &answer)
{
	dprintf(DEBUG_NORMAL, "cNKFeedParser::parseFeedJSON:\n");
	
	Json::Value root, v;
	Json::Reader reader;
	
	if (!reader.parse(answer, root))
		return false;

	v = root.get("status", "" );
	if (v.type() != Json::stringValue || v.asString() != "ok")
		return false;

	Json::Value posts = root.get("posts", "");

	if (posts.type() != Json::arrayValue)
		return false;

	for(unsigned int i = 0; i < posts.size(); ++i) 
	{
		const Json::Value flick = posts[i];
		sNKVideoInfo vinfo;
		v = flick.get("title_plain", "");
		if (v.type() == Json::stringValue) 
		{
			vinfo.title = v.asString();
			htmlEntityDecode(vinfo.title, true);
		}
		
		v = flick.get("id", "");
		if (v.type() == Json::intValue || v.type() == Json::uintValue) 
		{
			vinfo.id = to_string(v.asInt());
			vinfo.tfile = thumbnail_dir + "/" + vinfo.id + ".jpg";
		}
		
		v = flick.get("content", "");
		if (v.type() == Json::stringValue) 
		{
			vinfo.description = v.asString();
			htmlEntityDecode(vinfo.description, true);
		}
		
		v = flick.get("modified", "");
		if (v.type() == Json::stringValue) 
		{
			vinfo.published = v.asString();
		}
		
		unsigned int _i = 0;
		v = flick.get("custom_fields", "");
		if (v.type() == Json::objectValue) 
		{
			v = v.get("Streaming", "");
			if (v.type() == Json::arrayValue && v.size() > 0) 
			{
				if (v[_i].type() == Json::stringValue)
				{
					vinfo.url = "http://pmd.netzkino-and.netzkino.de/";
					vinfo.url += v[_i].asString();
					vinfo.url += ".mp4";

					//vinfo.url = MP4_URL;
					//vinfo.url += v[_i].asString();
					//vinfo.url += ".mp4";

					//vinfo.url = RTMP_URL;
					//vinfo.url += v[_i].asString();

					//vinfo.url += HLS_URL;
					//vinfo.url += v[_i].asString();
					//vinfo.url += "/playlist.m3u8";
				}
			}
		}
		
		v = flick.get("attachments", "");
		if (v.type() == Json::arrayValue && v.size() > 0 && v[_i].type() == Json::objectValue) 
		{
			v = v[_i]["url"];
			if (v.type() == Json::stringValue)
				vinfo.thumbnail = v.asString();
		}
		
		// fill list
		if (!vinfo.id.empty())
			videos.push_back(vinfo);
	}

	parsed = !videos.empty();
	
	return parsed;
}

bool cNKFeedParser::ParseFeed(std::string &url)
{
	dprintf(DEBUG_NORMAL, "cNKFeedParser::parseFeed(2)\n");
	
	// clear list
	videos.clear();

	std::string answer;
	
	if (!::getUrl(url, answer))
		return false;
	
	return parseFeedJSON(answer);
}

bool cNKFeedParser::ParseFeed(nk_feed_mode_t mode, std::string search, int category)
{
	dprintf(DEBUG_NORMAL, "cNKFeedParser::parseFeed(1) mode:%d categorie:%d (search:%s)\n", mode, category, search.c_str());

	if(parsed)
		return true;
	
	std::string url = MAIN_URL;
	
	if (mode == SEARCH) 
	{
		if (search.empty())
			return false;
		
		url += "get_search_results?search=" + encodeUrl(search);
	} 
	else if (mode == CATEGORY && category > 0) 
	{
		url += "get_category_posts";
		url += "?count=" + to_string(max_results);
		url += "&id=" + to_string(category);
	} 
	else
		return false;

	url += "&custom_fields=Streaming";

	return ParseFeed(url);
}

bool cNKFeedParser::ParseCategories(void)
{
	dprintf(DEBUG_NORMAL, "cNKFeedParser::parseCategories\n");
	
	if (categories.empty()) 
	{
		std::string url = MAIN_URL;
		url += "get_category_index";
		std::string answer;
		
		if (!::getUrl(url, answer))
			return false;
		
		return parseCategoriesJSON(answer);
	}
	
	return !categories.empty();
}

bool cNKFeedParser::DownloadThumbnails(/*unsigned start, unsigned end*/)
{
	dprintf(DEBUG_NORMAL, "DownloadThumbnails:\n");
	
	bool ret = false;
	if (safe_mkdir(thumbnail_dir.c_str()) && errno != EEXIST) 
	{
		perror(thumbnail_dir.c_str());
		//return false;
	}
	
	if(videos.size() > 0)
	{
		//for (unsigned int i = start; i < end; i++)
		for (unsigned int i = 0; i < videos.size(); i++)
		{
			bool found = false;
			
			if (!videos[i].thumbnail.empty()) 
			{
				found = !access(videos[i].tfile, F_OK);
				if (!found)
					found = ::DownloadUrl(videos[i].thumbnail, videos[i].tfile);
				
				ret |= found;
			}
		}
	}
	
	return ret;
}

void cNKFeedParser::Cleanup()
{
	dprintf(DEBUG_INFO, "cNKFeedParser::Cleanup: %d videos\n", (int)videos.size());
	
	for (unsigned i = 0; i < videos.size(); i++) 
	{
		unlink(videos[i].tfile.c_str());
	}
	
	videos.clear();
	parsed = false;
}

void cNKFeedParser::CleanupThumbnails()
{
	dprintf(DEBUG_INFO, "cNKFeedParser::CleanupThumbnails: %d videos\n", (int)videos.size());
	
	for (unsigned i = 0; i < videos.size(); i++) 
	{
		unlink(videos[i].tfile.c_str());
	}
}

void cNKFeedParser::downloadMovie(std::string &fname, std::string &url)
{
	dprintf(DEBUG_INFO, "cNKFeedParser::downloadMovie:\n");
	
	if (safe_mkdir(movie_dir.c_str()) && errno != EEXIST) 
	{
		perror(movie_dir.c_str());
		return;
	}
	
	std::string filename;
	filename += movie_dir;
	filename += "/";
	filename += fname;
			
	if (!url.empty()) 
	{
		::DownloadUrl(url, filename);
	}
}



