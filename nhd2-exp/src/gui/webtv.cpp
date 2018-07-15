/*
	$Id: webtv.cpp 2016/12/18 10:45:30 mohousch Exp $
	based on martii webtv

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
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#include <algorithm>    // std::sort
#include <fstream>
#include <iostream>

#include <global.h>
#include <neutrino.h>

#include <driver/screen_max.h>

#include <gui/widget/buttons.h>
#include <gui/widget/infobox.h>
#include <gui/widget/hintbox.h>

#include <gui/audio_video_select.h> 	//ac3state
#include <gui/filebrowser.h>

#include <xmlinterface.h>

#include <system/debug.h>
#include <system/helpers.h>

// libdvbapi
#include <playback_cs.h>

// curl
#include <curl/curl.h>
#include <curl/easy.h>

#include <webtv.h>
#include <bouquets.h>


extern tallchans allchans;
extern cPlayback *playback;
xmlDocPtr parser;

CWebTV::CWebTV()
{
	tuned = -1;
	
	parser = NULL;
	
	position = 0;
	duration = 0;
	file_prozent = 0;
	
	playstate = STOPPED;
	speed = 0;

	webTVlistMenu = NULL;
	item = NULL;
}

CWebTV::~CWebTV()
{
	ClearChannels();
}

void CWebTV::ClearChannels(void)
{
	dprintf(DEBUG_INFO, "CWebTV::ClearChannels\n");

	if (parser != NULL)
	{
		xmlFreeDoc(parser);
		parser = NULL;
	}
	
	for(unsigned int j = 0; j < channels.size(); j++)
	{
		delete channels[j];
	}
	
	channels.clear();
}

void CWebTV::loadChannels(void)
{
	dprintf(DEBUG_INFO, "CWebTV::loadChannels\n");

	readChannellist(g_settings.webtv_userBouquet);
	
	title = std::string(rindex(g_settings.webtv_userBouquet.c_str(), '/') + 1);

	removeExtension(title);
}

void CWebTV::processPlaylistUrl(const char *url, const char *name, const char * description) 
{
	dprintf(DEBUG_DEBUG, "CWebTV::processPlaylistUrl\n");
	
	CURL *curl_handle;
	struct MemoryStruct chunk;
	
	chunk.memory = NULL; 	/* we expect realloc(NULL, size) to work */
	chunk.size = 0;    	/* no data at this point */

	curl_global_init(CURL_GLOBAL_ALL);

	/* init the curl session */
	curl_handle = curl_easy_init();

	/* specify URL to get */
	curl_easy_setopt(curl_handle, CURLOPT_URL, url);

	/* send all data to this function  */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

	/* some servers don't like requests that are made without a user-agent field, so we provide one */
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

	/* don't use signal for timeout */
	curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, (long)1);

	/* set timeout to 10 seconds */
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 10);
	
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

	/* get it! */
	curl_easy_perform(curl_handle);

	/* cleanup curl stuff */
	curl_easy_cleanup(curl_handle);

	long res_code;
	if (curl_easy_getinfo(curl_handle, CURLINFO_HTTP_CODE, &res_code ) ==  CURLE_OK) 
	{
		if (200 == res_code) 
		{
			//printf("\nchunk = %s\n", chunk.memory);
			std::istringstream iss;
			iss.str (std::string(chunk.memory, chunk.size));
			char line[512];
			char *ptr;
			t_channel_id id = 0;
			
			while (iss.rdstate() == std::ifstream::goodbit) 
			{
				iss.getline(line, 512);
				if (line[0] != '#') 
				{
					//printf("chunk: line = %s\n", line);
					ptr = strstr(line, "http://");
					if (ptr != NULL) 
					{
						char *tmp;
						// strip \n and \r characters from url
						tmp = strchr(line, '\r');
						if (tmp != NULL)
							*tmp = '\0';
						tmp = strchr(line, '\n');
						if (tmp != NULL)
							*tmp = '\0';

						// grab channel id from channellist
						id = 0;

						for (tallchans_iterator it = allchans.begin(); it != allchans.end(); it++)
						{
								if(strcasecmp(it->second.getName().c_str(), name) == 0)
									id = it->second.getChannelID();
						}
						
						addUrl2Playlist(ptr, name, description, id);
					}
				}
			}
		}
	}

	if(chunk.memory)
		free(chunk.memory);
 
	/* we're done with libcurl, so clean it up */
	curl_global_cleanup();
}

void CWebTV::addUrl2Playlist(const char * url, const char *name, const char * description, t_channel_id id)
{
	dprintf(DEBUG_DEBUG, "CWebTV::addUrl2Playlist\n");
	
	webtv_channels * tmp = new webtv_channels();
	
	tmp->id = id&0xFFFFFFFFFFFFULL;				
	tmp->title = name;
	tmp->url = url;
	tmp->description = description;
						
	// fill channelslist
	channels.push_back(tmp);
}

// readxml file
bool CWebTV::readChannellist(std::string filename)
{
	dprintf(DEBUG_NORMAL, "CWebTV::readChannellist parsing %s\n", filename.c_str());
	
	// clear channellist
	ClearChannels();
	
	// check for extension
	bool iptv = false;
	bool webtv = false;
	bool playlist = false;
					
	std::string extension = getFileExt(filename);
						
	if( strcasecmp("tv", extension.c_str()) == 0)
		iptv = true;
	else if( strcasecmp("m3u", extension.c_str()) == 0)
			playlist = true;
	if( strcasecmp("xml", extension.c_str()) == 0)
		webtv = true;
	
	if(iptv)
	{
		FILE * f = fopen(filename.c_str(), "r");
		std::string title;
		std::string URL;
		std::string url;
		std::string description;
		t_channel_id id = 0;
		
		if(f != NULL)
		{
			while(true)
			{
				char line[1024];
				if (!fgets(line, 1024, f))
					break;
				
				size_t len = strlen(line);
				if (len < 2)
					// Lines with less than one char aren't meaningful
					continue;
				
				// strip newline
				line[--len] = 0;
				
				// strip carriage return (when found)
				if (line[len - 1] == '\r')
					line[len - 1 ] = 0;
				
				if (strncmp(line, "#SERVICE 4097:0:1:0:0:0:0:0:0:0:", 32) == 0)
					url = line + 32;
				else if (strncmp(line, "#DESCRIPTION", 12) == 0)
				{
					int offs = line[12] == ':' ? 14 : 13;
			
					title = line + offs;
				
					description = "stream";

					if(id == 0)
						id = create_channel_id(url.c_str());
					
					addUrl2Playlist(::decodeUrl(url).c_str(), title.c_str(), description.c_str(), id);
				}
			}
			
			fclose(f);
			
			return true;
		}
	}
	else if(webtv)
	{
		if (parser != NULL)
		{
			xmlFreeDoc(parser);
			parser = NULL;
		}

		parser = parseXmlFile(filename.c_str());
		
		if (parser) 
		{
			xmlNodePtr l0 = NULL;
			xmlNodePtr l1 = NULL;
			l0 = xmlDocGetRootElement(parser);
			l1 = l0->xmlChildrenNode;
			
			neutrino_msg_t      msg;
			neutrino_msg_data_t data;
			
			CHintBox* hintBox = NULL;
			hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_SERVICEMENU_RELOAD_HINT));
			
			g_RCInput->getMsg(&msg, &data, 0);
			
			if (l1) 
			{
				while ( ((xmlGetNextOccurence(l1, "webtv")) || (xmlGetNextOccurence(l1, "station"))) && msg != CRCInput::RC_home) 
				{
					char * title;
					char * url;
					char * description;
					t_channel_id id = 0;
					
					// title
					if(xmlGetNextOccurence(l1, "webtv"))
					{
						title = xmlGetAttribute(l1, (char *)"title");
						url = xmlGetAttribute(l1, (char *)"url");
						description = xmlGetAttribute(l1, (char *)"description");
						const char *epgid = xmlGetAttribute(l1, "epgid");

						if (epgid)
							id = strtoull(epgid, NULL, 16);

						if(id == 0)
							id = create_channel_id(url);
						
						addUrl2Playlist(url, title, description, id);
					}	
					else if (xmlGetNextOccurence(l1, "station"))
					{
						hintBox->paint();
						
						title = xmlGetAttribute(l1, (char *)"name");
						url = xmlGetAttribute(l1, (char *)"url");
						description = "stream";

						if(id == 0)
							id = create_channel_id(url);
						
						processPlaylistUrl(url, title, description) ;
					}

					l1 = l1->xmlNextNode;
					g_RCInput->getMsg(&msg, &data, 0);
				}
			}
			hintBox->hide();
			delete hintBox;
			hintBox = NULL;
			
			return true;
		}
		
		xmlFreeDoc(parser);
		parser = NULL;
	}
	else if(playlist)
	{
		std::ifstream infile;
		char cLine[1024];
		char name[1024] = { 0 };
		int duration;
		std::string description;
		t_channel_id id = 0;
				
		infile.open(filename.c_str(), std::ifstream::in);

		while (infile.good())
		{
			infile.getline(cLine, sizeof(cLine));
					
			// remove CR
			if(cLine[strlen(cLine) - 1] == '\r')
				cLine[strlen(cLine) - 1] = 0;
					
			sscanf(cLine, "#EXTINF:%d,%[^\n]\n", &duration, name);
					
			if(strlen(cLine) > 0 && cLine[0] != '#')
			{
				char *url = NULL;
				if ((url = strstr(cLine, "http://")) || (url = strstr(cLine, "rtmp://")) || (url = strstr(cLine, "rtsp://")) || (url = strstr(cLine, "mmsh://")) ) 
				{
					if (url != NULL) 
					{
						description = "stream";

						if(id == 0)
							id = create_channel_id(url);
					
						addUrl2Playlist(url, name, description.c_str(), id);
					}
				}
			}
		}
		infile.close();
	}
	
	return false;
}

//
void insertEventsfromHttp(std::string& url, t_original_network_id _onid, t_transport_stream_id _tsid, t_service_id _sid);

// get events
void CWebTV::getEvents(t_channel_id chid)
{
	std::string evUrl = "http://";
	evUrl += g_settings.epg_serverbox_ip;

	if(g_settings.epg_serverbox_gui == SNeutrinoSettings::SATIP_SERVERBOX_GUI_ENIGMA2)
	{
		evUrl += "/web/epgservice?sRef=1:0:"; 

		evUrl += to_hexstring(1);
		evUrl += ":";
		evUrl += to_hexstring(GET_SERVICE_ID_FROM_CHANNEL_ID(chid)); //sid
		evUrl += ":";
		evUrl += to_hexstring(GET_TRANSPORT_STREAM_ID_FROM_CHANNEL_ID(chid)); //tsid
		evUrl += ":";
		evUrl += to_hexstring(GET_ORIGINAL_NETWORK_ID_FROM_CHANNEL_ID(chid)); //onid
		evUrl += ":";

		if(g_settings.epg_serverbox_type == DVB_C)
		{
			evUrl += "FFFF"; // namenspace for cable
		}
		else if (g_settings.epg_serverbox_type == DVB_T)
		{
			evUrl += "EEEE"; // namenspace for terrestrial
		}
		else if (g_settings.epg_serverbox_type == DVB_S)
		{
			// namenspace for sat
			evUrl += to_hexstring(GET_SATELLITEPOSITION_FROM_CHANNEL_ID(chid)); //satpos
		}

		evUrl += "0000";
		evUrl += ":";
		evUrl += "0:0:0:";
	}
	else if(g_settings.epg_serverbox_gui == SNeutrinoSettings::SATIP_SERVERBOX_GUI_NMP)
	{
		evUrl += "/control/epg?channelid=";

         	evUrl += to_hexstring(chid);

		evUrl += "&xml=true&details=true";
	}

	insertEventsfromHttp(evUrl, GET_ORIGINAL_NETWORK_ID_FROM_CHANNEL_ID(chid), GET_TRANSPORT_STREAM_ID_FROM_CHANNEL_ID(chid), GET_SERVICE_ID_FROM_CHANNEL_ID(chid));
}
//

bool CWebTV::startPlayBack(int pos)
{
	dprintf(DEBUG_NORMAL, "CWebTV::startPlayBack\n");

	playback->Open();
	
	// if not mached
	if ( (pos >= (signed int) channels.size()) || (pos < 0) ) 
	{
		pos = 0;
	}
	
	if (!playback->Start((char *)channels[pos]->url.c_str()))
		return false;
	
	playstate = PLAY;
	speed = 1;

	getEvents(channels[pos]->id&0xFFFFFFFFFFFFULL);

	g_Sectionsd->setServiceChanged(channels[pos]->id&0xFFFFFFFFFFFFULL, false);

	if (CVFD::getInstance()->is4digits)					
		CVFD::getInstance()->LCDshowText(pos + 1);
	else
		CVFD::getInstance()->showServicename(channels[pos]->title); 

	return true;
}

void CWebTV::stopPlayBack(void)
{
	dprintf(DEBUG_NORMAL, "CWebTV::stopPlayBack\n");

	playback->Close();
	playstate = STOPPED;
}

void CWebTV::pausePlayBack(void)
{
	dprintf(DEBUG_NORMAL, "CWebTV::pausePlayBack\n");

	playback->SetSpeed(0);
	playstate = PAUSE;
	speed = 0;
}

void CWebTV::continuePlayBack(void)
{
	dprintf(DEBUG_DEBUG, "CWebTV::continuePlayBack\n");

	playback->SetSpeed(1);
	playstate = PLAY;
	speed = 1;
}

void CWebTV::quickZap(int key)
{
	dprintf(DEBUG_NORMAL, "CWebTV::quickZap\n");

	if (key == g_settings.key_quickzap_down)
	{
                if(tuned <= 0)
                        tuned = channels.size() - 1;
                else
                        tuned--;
        }
	else if (key == g_settings.key_quickzap_up)
	{
                tuned = (tuned + 1)%channels.size();
        }
	
	stopPlayBack();
	startPlayBack(tuned);

	//infoviewer
	g_InfoViewer->show(tuned + 1, getLiveChannelName(), -1, getLiveChannelID());
}

void CWebTV::showInfo()
{
	dprintf(DEBUG_INFO, "CWebTV::showInfo\n");

	//infoviewer
	if(tuned > -1)
	{
		g_InfoViewer->show(tuned + 1, getLiveChannelName(), -1, getLiveChannelID());
	}
}

void CWebTV::Bouquets(void)
{
	dprintf(DEBUG_NORMAL, "CWebTV::Bouquets\n");

	CFileFilter fileFilter;
	
	fileFilter.addFilter("xml");
	fileFilter.addFilter("tv");
	fileFilter.addFilter("m3u");

	//
	CFileList filelist;
	CMenuWidget m(LOCALE_WEBTV_BOUQUETS, NEUTRINO_ICON_WEBTV_SMALL, MENU_WIDTH + 100);
	m.enableSaveScreen();

	int select = -1;
	CMenuSelectorTarget * selector = new CMenuSelectorTarget(&select);
	int count = 0;
	static int old_select = 0;

	std::string Path_local = CONFIGDIR "/webtv/";

	// read list
	if(CFileHelpers::getInstance()->readDir(Path_local, &filelist, &fileFilter))
	{
		std::string bTitle;

		for (unsigned int i = 0; i < filelist.size(); i++)
		{
			bTitle = filelist[i].getFileName();

			removeExtension(bTitle);

			m.addItem(new CMenuForwarder(bTitle.c_str(), true, NULL, selector, to_string(count).c_str()), old_select == count);

			count++;
		}
	}

	m.exec(NULL, "");
	delete selector;

	// select
	if(select >= 0)
	{
		old_select = select;

		g_settings.webtv_userBouquet.clear();
		
		g_settings.webtv_userBouquet = filelist[select].Name.c_str();
		
		dprintf(DEBUG_NORMAL, "CWebTV::addUserBouquet: settings file %s\n", g_settings.webtv_userBouquet.c_str());
		
		// load channels
		loadChannels();
	}
}

void CWebTV::userBouquet(void)
{
	dprintf(DEBUG_NORMAL, "CWebTV::addUserBouquet\n");

	CFileBrowser filebrowser;
	CFileFilter fileFilter;
	
	fileFilter.addFilter("xml");
	fileFilter.addFilter("tv");
	fileFilter.addFilter("m3u");

	filebrowser.Filter = &fileFilter;

	if (filebrowser.exec(CONFIGDIR "/webtv"))
	{
		g_settings.webtv_userBouquet.clear();
		
		g_settings.webtv_userBouquet = filebrowser.getSelectedFile()->Name.c_str();
		
		printf("[webtv] webtv settings file %s\n", g_settings.webtv_userBouquet.c_str());
		
		// load channels
		loadChannels();
	}
}

// channel events
void sectionsd_getChannelEvents(CChannelEventList &eList, const bool tv_mode, t_channel_id *chidlist, int clen);
void sectionsd_getEventsServiceKey(t_channel_id serviceUniqueKey, CChannelEventList &eList, char search = 0, std::string search_text = "");

void CWebTV::updateEvents(void)
{
	CChannelEventList events;

	if (displayNext) 
	{
		if (channels.size()) 
		{
			time_t atime = time(NULL);
			unsigned int count;
			
			for (count = 0; count < channels.size(); count++)
			{		
				//
				events.clear();

				sectionsd_getEventsServiceKey(channels[count]->id, events);
				channels[count]->nextEvent.startTime = (long)0x7fffffff;
				
				for ( CChannelEventList::iterator e = events.begin(); e != events.end(); ++e ) 
				{
					if (((long)(e->startTime) > atime) && ((e->startTime) < (long)(channels[count]->nextEvent.startTime)))
					{
						channels[count]->nextEvent= *e;
						break;
					}
				}
			}
		}
	} 
	else 
	{
		t_channel_id * p_requested_channels = NULL;
		int size_requested_channels = 0;

		if (channels.size()) 
		{
			size_requested_channels = channels.size()*sizeof(t_channel_id);
			p_requested_channels = (t_channel_id*)malloc(size_requested_channels);
			
			for (uint32_t count = 0; count < channels.size(); count++)
			{
				p_requested_channels[count] = channels[count]->id&0xFFFFFFFFFFFFULL;
			}

			CChannelEventList pevents;
			sectionsd_getChannelEvents(pevents, (CNeutrinoApp::getInstance()->getMode()) != NeutrinoMessages::mode_radio, p_requested_channels, size_requested_channels);
			
			for (uint32_t count = 0; count < channels.size(); count++) 
			{
				channels[count]->currentEvent = CChannelEvent();
				
				for ( CChannelEventList::iterator e = pevents.begin(); e != pevents.end(); ++e )
				{
					if ((channels[count]->id&0xFFFFFFFFFFFFULL) == e->get_channel_id())
					{
						channels[count]->currentEvent= *e;
						break;
					}
				}
			}
			if (p_requested_channels != NULL) 
				free(p_requested_channels);
		}
	}
	
	events.clear();
}

#define FOOT_BUTTONS_COUNT 4
struct button_label FootButtons[FOOT_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_RED, LOCALE_INFOVIEWER_EVENTLIST, NULL},
	{ NEUTRINO_ICON_BUTTON_GREEN, LOCALE_INFOVIEWER_NEXT, NULL},
	{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_FILEBROWSER_NEXTPAGE, NULL},
	{ NEUTRINO_ICON_BUTTON_BLUE, LOCALE_WEBTV_BOUQUETS, NULL},
};

#define HEAD_BUTTONS_COUNT 1
struct button_label HeadButtons[HEAD_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_HELP, NONEXISTANT_LOCALE, NULL}
};

void CWebTV::show(bool reload, bool reinit)
{
	dprintf(DEBUG_NORMAL, "CWebTV::show:\n");

	// load channesl
	if(reload)
		loadChannels();

	//
	CChannelEvent* p_event = NULL;
	time_t jetzt = time(NULL);
	int runningPercent = 0;

	updateEvents();

	if (displayNext) 
	{
		FootButtons[1].locale = LOCALE_INFOVIEWER_NOW;
	} 
	else 
	{
		FootButtons[1].locale = LOCALE_INFOVIEWER_NEXT;
	}

	//
	webTVlistMenu = new ClistBox(title.c_str(), NEUTRINO_ICON_WEBTV_SMALL, w_max ( (CFrameBuffer::getInstance()->getScreenWidth() / 20 * 17), (CFrameBuffer::getInstance()->getScreenWidth() / 20 )), h_max ( (CFrameBuffer::getInstance()->getScreenHeight() / 20 * 16), (CFrameBuffer::getInstance()->getScreenHeight() / 20)));

	if(channels.size())
	{
		for(unsigned int i = 0; i< channels.size(); i++)
		{
			std::string desc = channels[i]->description;
			char cSeit[50] = " ";
			char cNoch[50] = " ";

			if (displayNext) 
			{
				p_event = &channels[i]->nextEvent;
			} 
			else 
			{
				p_event = &channels[i]->currentEvent;
			}

			// runningPercent
			runningPercent = 0;
			
			if (((jetzt - p_event->startTime + 30) / 60) < 0 )
			{
				runningPercent = 0;
			}
			else
			{
				//printf("(jetzt:%d) (p_event->startTime:%d) (p_event->duration:%d)\n", jetzt, p_event->startTime, p_event->duration);

				if(p_event->duration > 0)
					runningPercent = (jetzt - p_event->startTime) * 30 / p_event->duration;
			}

			// description
			if (p_event != NULL && !(p_event->description.empty())) 
			{
				desc = p_event->description;

				struct tm * pStartZeit = localtime(&p_event->startTime);
				unsigned seit = ( time(NULL) - p_event->startTime ) / 60;

				if (displayNext) 
				{
					sprintf(cNoch, "(%d min)", p_event->duration / 60);
					sprintf(cSeit, g_Locale->getText(LOCALE_CHANNELLIST_START), pStartZeit->tm_hour, pStartZeit->tm_min);
				} 
				else 
				{
					sprintf(cSeit, g_Locale->getText(LOCALE_CHANNELLIST_SINCE), pStartZeit->tm_hour, pStartZeit->tm_min);
					int noch = (p_event->startTime + p_event->duration - time(NULL)) / 60;
					if ((noch < 0) || (noch >= 10000))
						noch = 0;
					sprintf(cNoch, "(%d / %d min)", seit, noch);
				}
			}
			//

			item = new ClistBoxItem(channels[i]->title.c_str(), true, desc.c_str(), this, "zapit");

			item->setNumber(i + 1);
			item->setPercent(runningPercent);
			item->setInfo1(desc.c_str());
			item->setOptionInfo1(cSeit);
			item->setInfo2(p_event->text.c_str());
			item->setOptionInfo2(cNoch);

			webTVlistMenu->addItem(item);
		}
	}

	webTVlistMenu->setTimeOut(g_settings.timing[SNeutrinoSettings::TIMING_CHANLIST]);
	webTVlistMenu->setSelected(reinit? 0 : tuned);

	webTVlistMenu->setFooterButtons(FootButtons, FOOT_BUTTONS_COUNT);
	webTVlistMenu->setHeaderButtons(HeadButtons, HEAD_BUTTONS_COUNT);
	
	webTVlistMenu->enablePaintDate();
	webTVlistMenu->enableFootInfo();

	//
	webTVlistMenu->addKey(CRCInput::RC_info, this, CRCInput::getSpecialKeyName(CRCInput::RC_info));

	// footer
	webTVlistMenu->addKey(CRCInput::RC_red, this, CRCInput::getSpecialKeyName(CRCInput::RC_red));
	webTVlistMenu->addKey(CRCInput::RC_green, this, CRCInput::getSpecialKeyName(CRCInput::RC_green));
	webTVlistMenu->addKey(CRCInput::RC_yellow, this, CRCInput::getSpecialKeyName(CRCInput::RC_yellow));
	webTVlistMenu->addKey(CRCInput::RC_blue, this, CRCInput::getSpecialKeyName(CRCInput::RC_blue));

	//
	webTVlistMenu->addKey(CRCInput::RC_pause, this, CRCInput::getSpecialKeyName(CRCInput::RC_pause));
	webTVlistMenu->addKey(CRCInput::RC_play, this, CRCInput::getSpecialKeyName(CRCInput::RC_play));

	webTVlistMenu->exec(NULL, "");
	//webTVlistMenu->hide();
	delete webTVlistMenu;
	webTVlistMenu = NULL;
}

void CWebTV::hide()
{
	dprintf(DEBUG_NORMAL, "CWebTV::hide:\n");

	CFrameBuffer::getInstance()->paintBackground();
	CFrameBuffer::getInstance()->blit();
}

int CWebTV::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CWebTV::exec: actionKey: %s\n", actionKey.c_str());

	if(parent)
		hide();

	if(actionKey == "zapit")
	{
		if(webTVlistMenu->getSelected() != tuned)
		{
			stopPlayBack();
			startPlayBack(webTVlistMenu->getSelected());
			tuned = webTVlistMenu->getSelected();

			//infoviewer
			g_InfoViewer->show(tuned + 1, getLiveChannelName(), -1, getLiveChannelID());

			// kill infobar
			g_InfoViewer->killTitle();
		}

		return menu_return::RETURN_EXIT;
	}
	else if(actionKey == "RC_red")
	{
		g_EventList->exec(channels[webTVlistMenu->getSelected()]->id, channels[webTVlistMenu->getSelected()]->title);
		/*
		userBouquet(); 
		show(false, true);
		*/

		return menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey == "RC_green")
	{
		//g_RCInput->postMsg(CRCInput::RC_page_down, 0);
		//selected = webTVlistMenu->getSelected();
		displayNext = !displayNext;
		show(false, true);
		return menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey == "RC_yellow")
	{
		g_RCInput->postMsg(CRCInput::RC_page_up, 0);
	}
	else if(actionKey == "RC_blue")
	{
		Bouquets();
		show(false, true);
		return menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey == "RC_pause")
	{
		pausePlayBack();

		//infoviewer
		g_InfoViewer->show(tuned + 1, getLiveChannelName(), -1, getLiveChannelID());
	}
	else if(actionKey == "RC_play")
	{
		continuePlayBack();

		//infoviewer
		g_InfoViewer->show(tuned + 1, getLiveChannelName(), -1, getLiveChannelID());

		// kill infobar
		g_InfoViewer->killTitle();
	}
	else if(actionKey == "RC_info")
	{
		g_EpgData->show(channels[webTVlistMenu->getSelected()]->id);
	}

	return menu_return::RETURN_REPAINT;
}

