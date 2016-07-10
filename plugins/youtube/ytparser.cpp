/*
        Copyright (C) 2013 CoolStream International Ltd

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
#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <set>
#include <map>
#include <vector>
#include <bitset>
#include <string>
#include <fstream>

#include <json/json.h>

#include <ytparser.h>

#include <system/debug.h>

#include <plugin.h>

#include <youtube.h>


#define URL_TIMEOUT 		60
extern YTB_SETTINGS m_settings;

std::string cYTVideoInfo::GetUrl(int fmt)
{
	yt_urlmap_iterator_t it;
	
	if (fmt) 
	{
		if ((it = formats.find(fmt)) != formats.end())
			return it->second.url;
		return "";
	}
	
	if ((it = formats.find(37)) != formats.end())
		return it->second.url;
	if ((it = formats.find(22)) != formats.end())
		return it->second.url;
	if ((it = formats.find(18)) != formats.end())
		return it->second.url;
	
	return "";
}

cYTFeedParser::cYTFeedParser()
{
	thumbnail_dir = "/tmp/ytparser";
	parsed = false;
	feedmode = -1;
	tquality = "mqdefault";
	max_results = 25;
}

cYTFeedParser::~cYTFeedParser()
{
}

size_t cYTFeedParser::CurlWriteToString(void *ptr, size_t size, size_t nmemb, void *data)
{
        std::string* pStr = (std::string*) data;
        pStr->append((char*) ptr, nmemb);
	
        return size*nmemb;
}

bool cYTFeedParser::getUrl(std::string &url, std::string &answer)
{
	CURL * curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, &cYTFeedParser::CurlWriteToString);
	curl_easy_setopt(curl_handle, CURLOPT_FILE, (void *)&answer);
	curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1);
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, URL_TIMEOUT);
	curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, (long)1);
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, false);
	
	if(strcmp(g_settings.softupdate_proxyserver, "")!=0)
	{
		curl_easy_setopt(curl_handle, CURLOPT_PROXY, g_settings.softupdate_proxyserver);
		
		if(strcmp(g_settings.softupdate_proxyusername, "") != 0)
		{
			char tmp[200];
			strcpy(tmp, g_settings.softupdate_proxyusername);
			strcat(tmp, ":");
			strcat(tmp, g_settings.softupdate_proxypassword);
			curl_easy_setopt(curl_handle, CURLOPT_PROXYUSERPWD, tmp);
		}
	}

	char cerror[CURL_ERROR_SIZE];
	curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, cerror);

	dprintf(DEBUG_NORMAL, "cYTFeedParser::getUrl: try to get %s\n", url.c_str());
	
	CURLcode httpres = curl_easy_perform(curl_handle);

	curl_easy_cleanup(curl_handle);

	dprintf(DEBUG_NORMAL, "cYTFeedParser::getUrl: http: res %d size %d\n", httpres, (int)answer.size());

	if (httpres != 0 || answer.empty()) 
	{
		dprintf(DEBUG_NORMAL, "cYTFeedParser::getUrl: error: %s\n", cerror);
		return false;
	}
	
	return true;
}

bool cYTFeedParser::DownloadUrl(std::string &url, std::string &file)
{
	FILE * fp = fopen(file.c_str(), "wb");
	if (fp == NULL) 
	{
		perror(file.c_str());
		return false;
	}
	
	CURL * curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_FILE, fp);
	curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1);
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, URL_TIMEOUT);
	curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, (long)1);
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, false);
	
	if(strcmp(g_settings.softupdate_proxyserver, "")!=0)
	{
		curl_easy_setopt(curl_handle, CURLOPT_PROXY, g_settings.softupdate_proxyserver);
		
		if(strcmp(g_settings.softupdate_proxyusername, "") != 0)
		{
			char tmp[200];
			strcpy(tmp, g_settings.softupdate_proxyusername);
			strcat(tmp, ":");
			strcat(tmp, g_settings.softupdate_proxypassword);
			curl_easy_setopt(curl_handle, CURLOPT_PROXYUSERPWD, tmp);
		}
	}

	char cerror[CURL_ERROR_SIZE];
	curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, cerror);

	dprintf(DEBUG_NORMAL, "cYTFeedParser::DownloadUrl: try to get %s\n", url.c_str());
	
	CURLcode httpres = curl_easy_perform(curl_handle);

	double dsize;
	curl_easy_getinfo(curl_handle, CURLINFO_SIZE_DOWNLOAD, &dsize);
	curl_easy_cleanup(curl_handle);
	fclose(fp);

	dprintf(DEBUG_NORMAL, "cYTFeedParser::DownloadUrl: http: res %d size %f.\n", httpres, dsize);

	if (httpres != 0) 
	{
		dprintf(DEBUG_NORMAL, "cYTFeedParser::DownloadUrl: curl error: %s\n", cerror);
		unlink(file.c_str());
		return false;
	}
	
	return true;
}

void cYTFeedParser::decodeUrl(std::string &url)
{
	CURL * curl_handle = curl_easy_init();
	char * str = curl_easy_unescape(curl_handle, url.c_str(), 0, NULL);
	curl_easy_cleanup(curl_handle);
	
	if(str)
		url = str;
	
	curl_free(str);
}

void cYTFeedParser::encodeUrl(std::string &txt)
{
	CURL * curl_handle = curl_easy_init();
	char * str = curl_easy_escape(curl_handle, txt.c_str(), txt.length());
	curl_easy_cleanup(curl_handle);
	
	if(str)
		txt = str;
	
	curl_free(str);
}

void cYTFeedParser::splitString(std::string &str, std::string delim, std::vector<std::string> &strlist, int start)
{
	strlist.clear();
	int end = 0;
	
	while ((end = str.find(delim, start)) != std::string::npos) 
	{
		strlist.push_back(str.substr(start, end - start));
		start = end + delim.size();
	}
	strlist.push_back(str.substr(start));
}

void cYTFeedParser::splitString(std::string &str, std::string delim, std::map<std::string,std::string> &strmap, int start)
{
	int end = 0;
	if ((end = str.find(delim, start)) != std::string::npos) 
	{
		strmap[str.substr(start, end - start)] = str.substr(end - start + delim.size());
	}
}

bool cYTFeedParser::parseFeedJSON(std::string &answer)
{
	dprintf(DEBUG_NORMAL, "cYTFeedParser::parseFeedJSON\n");
	
	Json::Value root;
	Json::Reader reader;

	std::ostringstream ss;
	std::ifstream fh(curfeedfile.c_str(), std::ifstream::in);
	ss << fh.rdbuf();
	std::string filedata = ss.str();

	bool parsedSuccess = reader.parse(filedata, root, false);

	if(!parsedSuccess)
	{
		parsedSuccess = reader.parse(answer, root, false);
	}
	
	if(!parsedSuccess)
	{
		printf("Failed to parse JSON\n");
		printf("%s\n", reader.getFormattedErrorMessages().c_str());
		return false;
	}
	
	next.clear();
	prev.clear();
	
	next = root.get("nextPageToken", "").asString();
	prev = root.get("prevPageToken", "").asString();
  
	cYTVideoInfo vinfo;
	vinfo.description.clear();
	
	Json::Value elements = root["items"];
	
	for(unsigned int i = 0; i < elements.size(); ++i)
	{
		if(elements[i]["id"].type() == Json::objectValue) 
		{
			vinfo.id = elements[i]["id"].get("videoId", "").asString();
		}
		else if(elements[i]["id"].type() == Json::stringValue) 
		{
			vinfo.id = elements[i].get("id", "").asString();
		}
		vinfo.title             = elements[i]["snippet"].get("title", "").asString();
		vinfo.description       = elements[i]["snippet"].get("description", "").asString();
		vinfo.published         = elements[i]["snippet"].get("publishedAt", "").asString().substr(0, 10);
		std::string thumbnail   = elements[i]["snippet"]["thumbnails"]["default"].get("url", "").asString();
		// save thumbnail "default", if "high" not found
		vinfo.thumbnail         = elements[i]["snippet"]["thumbnails"]["high"].get("url", thumbnail).asString();
		vinfo.author            = elements[i]["snippet"].get("channelTitle", "unkown").asString();
		vinfo.category          = "";
		
		
		// duration/url
		if (!vinfo.id.empty()) 
		{
			// duration
			parseFeedDetailsJSON(vinfo);
		
			// url/fill videos list
			if(ParseVideoInfo(vinfo))
				videos.push_back(vinfo);
		}
	}
	
	parsed = !videos.empty();
	
	return parsed;
}

bool cYTFeedParser::parseFeedDetailsJSON(cYTVideoInfo &vinfo)
{
	dprintf(DEBUG_NORMAL, "cYTFeedParser::parseFeedDetailsJSON:\n");
	
	key = m_settings.ytkey;
	
	vinfo.duration = 0;
	// See at https://developers.google.com/youtube/v3/docs/videos
	std::string url = "https://www.googleapis.com/youtube/v3/videos?id=" + vinfo.id + "&part=contentDetails&key=" + key;
	std::string answer;
	
	if (!getUrl(url, answer))
		return false;
  
	Json::Value root;
	Json::Reader reader;
	bool parsedSuccess = reader.parse(answer, root, false);
	if (!parsedSuccess) 
	{
		dprintf(DEBUG_NORMAL, "cYTFeedParser::parseFeedDetailsJSON: Failed to parse JSON\n");
		dprintf(DEBUG_NORMAL, "cYTFeedParser::parseFeedDetailsJSON: %s\n", reader.getFormattedErrorMessages().c_str());
		return false;
	}
  
	Json::Value elements = root["items"];
	std::string duration = elements[0]["contentDetails"].get("duration", "").asString();
	
	if (duration.find("PT") != std::string::npos) 
	{
		int h = 0, m = 0, s = 0;
		if (duration.find("H") != std::string::npos) 
		{
			sscanf(duration.c_str(), "PT%dH%dM%dS", &h, &m, &s);
		}
		else if (duration.find("M") != std::string::npos) 
		{
			sscanf(duration.c_str(), "PT%dM%dS", &m, &s);
		}
		else if (duration.find("S") != std::string::npos) 
		{
			sscanf(duration.c_str(), "PT%dS", &s);
		}
		//printf(">>>>> duration: %s, h: %d, m: %d, s: %d\n", duration.c_str(), h, m, s);
		vinfo.duration = h*3600 + m*60 + s;
	}
	
	return true;
}

bool cYTFeedParser::supportedFormat(int fmt)
{
	if((fmt == 37) || (fmt == 22) || (fmt == 18))
		return true;
	
	return false;
}

bool cYTFeedParser::decodeVideoInfo(std::string &answer, cYTVideoInfo &vinfo)
{
	dprintf(DEBUG_NORMAL, "cYTFeedParser::decodeVideoInfo\n");
	
	bool ret = false;
	
	decodeUrl(answer);

	if(answer.find("token=") == std::string::npos)
		return ret;
	
	vinfo.formats.clear();

	//FIXME check expire
	std::vector<std::string> ulist;
	unsigned fmt = answer.find("url_encoded_fmt_stream_map=");
	
	if (fmt != std::string::npos) 
	{
		fmt = answer.find("=", fmt);
		splitString(answer, ",", ulist, fmt + 1);
		
		for (unsigned i = 0; i < ulist.size(); i++) 
		{
			std::map<std::string,std::string> smap;
			std::vector<std::string> uparams;
			splitString(ulist[i], "&", uparams);
			
			if (uparams.size() < 3)
				continue;
			
			for (unsigned j = 0; j < uparams.size(); j++) 
			{
				decodeUrl(uparams[j]);

				dprintf(DEBUG_DEBUG, "	param: %s\n", uparams[j].c_str());

				splitString(uparams[j], "=", smap);
			}

			// url
			cYTVideoUrl yurl;
			yurl.url = smap["url"];
			
			// sig
			std::string::size_type ptr = smap["url"].find("signature=");
			if (ptr != std::string::npos)
			{
				ptr = smap["url"].find("=", ptr);
				smap["url"].erase(0, ptr + 1);

				if((ptr = smap["url"].find("&")) != std::string::npos)
					yurl.sig = smap["url"].substr(0, ptr);
			}
			
			// quality/type
			int id = atoi(smap["itag"].c_str());
			if (supportedFormat(id) && !yurl.url.empty() && !yurl.sig.empty()) 
			{
				yurl.quality = smap["quality"];
				yurl.type = smap["type"];
				
				vinfo.formats.insert(yt_urlmap_pair_t(id, yurl));
				ret = true;
			}
		}
	}
	
	return ret;
}

bool cYTFeedParser::ParseVideoInfo(cYTVideoInfo &vinfo)
{
	dprintf(DEBUG_NORMAL, "cYTFeedParser::ParseVideoInfo\n");
	
	bool ret = false;
	std::vector<std::string> estr;
	estr.push_back("&el=embedded");
	estr.push_back("&el=vevo");
	estr.push_back("&el=detailpage");

	for (unsigned i = 0; i < estr.size(); i++) 
	{
		std::string vurl = "http://www.youtube.com/get_video_info?video_id=";
		vurl += vinfo.id;
		vurl += estr[i];
		vurl += "&ps=default&eurl=&gl=US&hl=en";
		
		std::string answer;
		if (!getUrl(vurl, answer))
			continue;
		
		ret = decodeVideoInfo(answer, vinfo);
		
		if (ret)
			break;
	}
	
	return ret;
}

bool cYTFeedParser::ParseFeed(std::string &url)
{
	dprintf(DEBUG_NORMAL, "cYTFeedParser::parseFeed(2)\n");
	
	videos.clear();

	std::string answer;
	curfeedfile = thumbnail_dir;
	curfeedfile += "/";
	curfeedfile += curfeed;
	curfeedfile += ".xml";

	if (!getUrl(url, answer))
		return false;

	return parseFeedJSON(answer);
}

bool cYTFeedParser::ParseFeed(yt_feed_mode_t mode, std::string search, std::string vid, yt_feed_orderby_t orderby)
{
	dprintf(DEBUG_NORMAL, "cYTFeedParser::parseFeed(1)\n");
	
	key = m_settings.ytkey;
	std::string answer;
	std::string url = "https://www.googleapis.com/youtube/v3/search?";
	bool append_res = true;
	
	if (mode < FEED_LAST) 
	{
		switch(mode) 
		{
			case MOST_POPULAR:
			default:
				curfeed = "&chart=mostPopular";
				break;
				
			case MOST_POPULAR_ALL_TIME:
				curfeed = "&chart=mostPopular";
				break;
		}
		
		url = "https://www.googleapis.com/youtube/v3/videos?part=snippet";
		
		if (!region.empty()) 
		{
			url += "&regionCode=";
			url += region;
		}
		url += curfeed;
	}
	else if (mode == NEXT) 
	{
		if (next.empty())
			return false;
		
		url = nextprevurl;
		url += "&pageToken=";
		url += next;
		append_res = false;
	}
	else if (mode == PREV) 
	{
		if (prev.empty())
			return false;
		
		url = nextprevurl;
		url += "&pageToken=";
		url += prev;
		append_res = false;
	}
	else if (mode == RELATED) 
	{
		if (vid.empty())
			return false;

		url = "https://www.googleapis.com/youtube/v3/search?part=snippet&relatedToVideoId=";
		url += vid;
		url += "&type=video&key=";
		url += key;
		append_res = false;
	}
	else if (mode == SEARCH) 
	{
		if (search.empty())
			return false;
		
		encodeUrl(search);
	
		url = "https://www.googleapis.com/youtube/v3/search?q=";
		url += search;
		url += "&part=snippet";

		const char *orderby_values[] = { "date", "relevance", "viewCount", "rating", "title", "videoCount"};
		url += "&order=" + std::string(orderby_values[orderby & 3]);
	}

	feedmode = mode;
	if (append_res) 
	{
		url += "&maxResults=";
		char res[10];
		sprintf(res, "%d", max_results);
		url += res;
		url += "&key=" + key;
		nextprevurl = url;
	}

	return ParseFeed(url);
}

bool cYTFeedParser::DownloadThumbnails()
{
	bool ret = false;
	if (mkdir(thumbnail_dir.c_str(), 0755)) 
	{
		perror(thumbnail_dir.c_str());
		//return ret;
	}
	
	for (unsigned i = 0; i < videos.size(); i++) 
	{
		if (!videos[i].thumbnail.empty()) 
		{
			std::string fname = thumbnail_dir;
			fname += "/";
			fname += videos[i].id;
			fname += ".jpg";
			bool found = !access(fname.c_str(), F_OK);
			if (!found)
				found = DownloadUrl(videos[i].thumbnail, fname);
			if (found)
				videos[i].tfile = fname;
			ret |= found;
		}
	}
	
	return ret;
}

void cYTFeedParser::Cleanup(bool delete_thumbnails)
{
	dprintf(DEBUG_NORMAL, "cYTFeedParser::Cleanup: %d videos\n", (int)videos.size());
	
	if (delete_thumbnails) 
	{
		for (unsigned i = 0; i < videos.size(); i++) 
		{
			unlink(videos[i].tfile.c_str());
		}
	}
	unlink(curfeedfile.c_str());
	videos.clear();
	parsed = false;
	feedmode = -1;
}
