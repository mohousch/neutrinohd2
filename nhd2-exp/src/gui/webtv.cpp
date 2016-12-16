/*
	$Id: webtv.cpp 2013/09/03 10:45:30 mohousch Exp $
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

#include <movieplayer.h>
#include <webtv.h>

#include <gui/widget/buttons.h>
#include <gui/widget/messagebox.h>
#include <gui/widget/helpbox.h>
#include <gui/widget/infobox.h>
#include <gui/widget/hintbox.h>
#include <gui/widget/items2detailsline.h>

#include <gui/filebrowser.h>
#include <gui/audio_video_select.h>

#include <xmlinterface.h>

#include <sectionsd/edvbstring.h>
#include <client/zapittools.h>

#include <system/debug.h>
#include <system/helpers.h>

// libdvbapi
#include <playback_cs.h>
#include <video_cs.h>
#include <audio_cs.h>

// curl
#include <curl/curl.h>
#include <curl/easy.h>


extern cPlayback *playback;

CWebTV::CWebTV()
{
	frameBuffer = CFrameBuffer::getInstance();
	
	selected = g_settings.webtv_lastselectedchannel;
	liststart = 0;
	tuned = -1;
	
	parser = NULL;
	
	position = 0;
	duration = 0;
	file_prozent = 0;
	
	zapProtection = NULL;
	
	playstate = STOPPED;
	speed = 0;
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

int CWebTV::exec(bool rezap)
{
	dprintf(DEBUG_NORMAL, "CWebTV::exec:\n");

	// load streams channels list
	if(channels.empty())
		loadChannels();
	
	int nNewChannel = -1;
	if(rezap)
	{
		nNewChannel = g_settings.webtv_lastselectedchannel;
		selected = nNewChannel;
	}
	else
		nNewChannel = Show();
	
	// zapto
	if ( nNewChannel > -1 && nNewChannel < (int) channels.size()) 
		zapTo(nNewChannel, rezap);

	return nNewChannel;
}

void CWebTV::loadChannels(void)
{
	dprintf(DEBUG_INFO, "CWebTV::loadChannels\n");

	readChannellist(g_settings.webtv_userBouquet);
	
	title = std::string(rindex(g_settings.webtv_userBouquet.c_str(), '/') + 1);
	strReplace(title, ".xml", "");
	strReplace(title, ".tv", "");
	strReplace(title, ".m3u", "");
	strReplace(title, "userbouquet.", "");
}

struct MemoryStruct {
	char *memory;
	size_t size;
};

static void *myrealloc(void *ptr, size_t size)
{
	/* 
	There might be a realloc() out there that doesn't like reallocing
	NULL pointers, so we take care of it here 
	*/
	if(ptr)
		return realloc(ptr, size);
	else
		return malloc(size);
}

static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)data;

	mem->memory = (char *)myrealloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory) 
	{
		memcpy(&(mem->memory[mem->size]), ptr, realsize);
		mem->size += realsize;
		mem->memory[mem->size] = 0;
	}
	return realsize;
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

	/*
	* Now, our chunk.memory points to a memory block that is chunk.size
	* bytes big and contains the remote file.
	*
	* Do something nice with it!
	*
	* You should be aware of the fact that at this point we might have an
	* allocated data block, and nothing has yet deallocated that data. So when
	* you're done with it, you should free() it as a nice application.
	*/

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
						
						addUrl2Playlist(ptr, name, description);
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

void CWebTV::addUrl2Playlist(const char * url, const char *name, const char * description, bool locked)
{
	dprintf(DEBUG_DEBUG, "CWebTV::addUrl2Playlist\n");
	
	webtv_channels * tmp = new webtv_channels();
						
	tmp->title = name;
	tmp->url = url;
	tmp->description = description;
	tmp->locked = locked;
						
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
	int ext_pos = 0;
	ext_pos = filename.rfind('.');
	bool iptv = false;
	bool webtv = false;
	bool playlist = false;
					
	if( ext_pos > 0)
	{
		std::string extension;
		extension = filename.substr(ext_pos + 1, filename.length() - ext_pos);
						
		if( strcasecmp("tv", extension.c_str()) == 0)
			iptv = true;
		else if( strcasecmp("m3u", extension.c_str()) == 0)
			playlist = true;
		if( strcasecmp("xml", extension.c_str()) == 0)
			webtv = true;
	}
	
	if(iptv)
	{
		FILE * f = fopen(filename.c_str(), "r");
		std::string title;
		std::string URL;
		std::string url;
		std::string description;
		
		if(f != NULL)
		{
			while(1)
			{
				char line[1024];
				if (!fgets(line, 1024, f))
					break;
				
				size_t len = strlen(line);
				if (len < 2)
					// Lines with less than one char aren't meaningful
					continue;
				
				/* strip newline */
				line[--len] = 0;
				
				// strip carriage return (when found)
				if (line[len - 1] == '\r')
					line[len - 1 ] = 0;
				
				if (strncmp(line, "#SERVICE 4097:0:1:0:0:0:0:0:0:0:", 32) == 0)
					url = line + 32;
				//else if ( (strncmp(line, "#DESCRIPTION: ", 14) == 0) || (strncmp(line, "#DESCRIPTION ", 13) == 0) )
				else if (strncmp(line, "#DESCRIPTION", 12) == 0)
				{
					int offs = line[12] == ':' ? 14 : 13;
			
					title = line + offs;
				
					description = "stream";
					
					addUrl2Playlist(urlDecode(url).c_str(), title.c_str(), description.c_str()); //urlDecode defined in edvbstring.h
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
					
					// title
					if(xmlGetNextOccurence(l1, "webtv"))
					{
						title = xmlGetAttribute(l1, (char *)"title");

						// url
						url = xmlGetAttribute(l1, (char *)"url");
						
						description = xmlGetAttribute(l1, (char *)"description");
						
						addUrl2Playlist(url, title, description);
					}	
					else if (xmlGetNextOccurence(l1, "station"))
					{
						hintBox->paint();
						
						title = xmlGetAttribute(l1, (char *)"name");
						url = xmlGetAttribute(l1, (char *)"url");
						description = "stream";
						
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
					
						addUrl2Playlist(url, name, description.c_str());
					}
				}
			}
		}
		infile.close();
	}
	
	return false;
}

void CWebTV::showUserBouquet(void)
{
	dprintf(DEBUG_INFO, "CWebTV::showUserBouquet\n");

	addUserBouquet();
}

bool CWebTV::startPlayBack(int pos)
{
	dprintf(DEBUG_DEBUG, "CWebTV::startPlayBack\n");

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
	return true;
}

void CWebTV::stopPlayBack(void)
{
	dprintf(DEBUG_DEBUG, "CWebTV::stopPlayBack\n");

	playback->Close();
	playstate = STOPPED;
}

void CWebTV::pausePlayBack(void)
{
	dprintf(DEBUG_DEBUG, "CWebTV::pausePlayBack\n");

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

//
int CWebTV::zapTo(int pos, bool rezap)
{
	// show emty channellist error msg
	if (channels.empty()) 
	{
		MessageBox(LOCALE_MESSAGEBOX_ERROR, g_Locale->getText(LOCALE_WEBTV_CHANNELLIST_NONEFOUND), CMessageBox::mbrCancel, CMessageBox::mbCancel, NEUTRINO_ICON_ERROR);
		return -1;
	}

	dprintf(DEBUG_NORMAL, "CWebTV::zapTo: Channel: %s\n", channels[pos]->title.c_str());

	// if not mached
	if ( (pos >= (signed int) channels.size()) || (pos < 0) ) 
	{
		pos = 0;
	}
	
	// check if the same channel
	if ( pos != tuned || rezap) 
	{
		tuned = pos;

		playback->Close();
	
		// parentallock
		if ( (channels[pos]->locked) && ( (g_settings.parentallock_prompt == PARENTALLOCK_PROMPT_ONSIGNAL) || (g_settings.parentallock_prompt == PARENTALLOCK_PROMPT_CHANGETOLOCKED)) )
		{
			if ( zapProtection != NULL )
				zapProtection->fsk = g_settings.parentallock_lockage;
			else
			{
				zapProtection = new CZapProtection( g_settings.parentallock_pincode, g_settings.parentallock_lockage);
							
				if ( !zapProtection->check() )
				{
					delete zapProtection;
					zapProtection = NULL;
					
					// do not thing
				}
				else
				{
					delete zapProtection;
					zapProtection = NULL;
					
					// start playback
					startPlayBack(pos);
				}
			}
		}
		else
			startPlayBack(pos);
	}
	
	// vfd
	if (CVFD::getInstance()->is4digits)
		CVFD::getInstance()->LCDshowText(pos + 1);
	else
		CVFD::getInstance()->showServicename(channels[pos]->title); // UTF-8		
	
	//infoviewer
	g_InfoViewer->showMovieInfo(channels[pos]->title, channels[pos]->description, file_prozent, duration, ac3state, speed, playstate, false, false);

	return 0;
}

void CWebTV::quickZap(int key)
{
	dprintf(DEBUG_NORMAL, "CWebTV::quickZap\n");

	if (key == g_settings.key_quickzap_down)
	{
                if(selected == 0)
                        selected = channels.size() - 1;
                else
                        selected--;
        }
	else if (key == g_settings.key_quickzap_up)
	{
                selected = (selected+1)%channels.size();
        }
	
	zapTo(selected);
}

void CWebTV::showInfo()
{
	dprintf(DEBUG_INFO, "CWebTV::showInfo\n");

	//infoviewer
	if(tuned > -1)
		g_InfoViewer->showMovieInfo(channels[tuned]->title, channels[tuned]->description, file_prozent, duration, ac3state, speed, playstate, false, false);
}

void CWebTV::getInfos()
{
	playback->GetPosition((int64_t &)position, (int64_t &)duration);
	
	if(duration > 100)
		file_prozent = (unsigned char) (position / (duration / 100));
}

int CWebTV::Show()
{
	dprintf(DEBUG_NORMAL, "CWebTV::show: selected channel: %s selected:%d g_settings.webtv_lastselectedchannel:%d\n", channels[selected]->title.c_str(), selected, g_settings.webtv_lastselectedchannel);

	int res = -1;
	
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;
	
	if(channels.empty())
		loadChannels();
	
	// display channame in vfd	
	CVFD::getInstance()->setMode(CVFD::MODE_IPTV);
	
	// windows size
	cFrameBox.iWidth = w_max ( (frameBuffer->getScreenWidth() / 20 * 17), (frameBuffer->getScreenWidth() / 20 ));
	cFrameBox.iHeight = h_max ( (frameBuffer->getScreenHeight() / 20 * 16), (frameBuffer->getScreenHeight() / 20));
	
	// head height
	titleIcon.setIcon(NEUTRINO_ICON_WEBTV_SMALL);
	cFrameBoxTitle.iHeight = std::max(titleIcon.iHeight, g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight()) + 6;
	
	// foot height
	footIcon.setIcon(NEUTRINO_ICON_BUTTON_RED);
	cFrameBoxFoot.iHeight = std::max(footIcon.iHeight, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight()) + 10;

	// info height
	cFrameBoxInfo.iHeight = 5 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight() + 5 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight() + 5;

	// item height
	cFrameBoxItem.iHeight = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight() + 2;

	// items pro page
	listmaxshow = (cFrameBox.iHeight - cFrameBoxTitle.iHeight - cFrameBoxFoot.iHeight - 2 - cFrameBoxInfo.iHeight)/cFrameBoxItem.iHeight;

	// recalculate hight
	cFrameBox.iHeight = cFrameBoxTitle.iHeight + cFrameBoxItem.iHeight*listmaxshow + cFrameBoxFoot.iHeight + 2 + cFrameBoxInfo.iHeight;
	
	// coordination
	cFrameBox.iX = frameBuffer->getScreenX() + (frameBuffer->getScreenWidth() - (cFrameBox.iWidth + ConnectLineBox_Width)) / 2 + ConnectLineBox_Width;
	cFrameBox.iY = frameBuffer->getScreenY() + (frameBuffer->getScreenHeight() - cFrameBox.iHeight) / 2;
	
showList:	
	
	// head
	paintHead();
	
	// foot
	paintFoot();
		
	// paint body
	paint();
		
	frameBuffer->blit();

	int zapOnExit = false;

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);

	// loop control
	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_CHANLIST]);
	bool loop = true;
	
	while (loop) 
	{
		g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd );
		
		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_CHANLIST]);

		if ( ( msg == CRCInput::RC_timeout ) || ( msg == (neutrino_msg_t)g_settings.key_channelList_cancel) ) 
		{
			loop = false;
			res = -1;
		}
		else if ( msg == CRCInput::RC_up || (int) msg == g_settings.key_channelList_pageup || msg == CRCInput::RC_yellow)
                {
                        int step = 0;
                        int prev_selected = selected;

			step =  ((int) msg == g_settings.key_channelList_pageup || (int) msg == CRCInput::RC_yellow) ? listmaxshow : 1;  // browse or step 1
                        selected -= step;
                        if((prev_selected - step) < 0)            // because of uint
                                selected = channels.size() - 1;

                        paintItem(prev_selected - liststart);
			
                        unsigned int oldliststart = liststart;
                        liststart = (selected/listmaxshow)*listmaxshow;
                        if(oldliststart != liststart)
                                paint();
                        else
                                paintItem(selected - liststart);
                }
                else if ( msg == CRCInput::RC_down || (int) msg == g_settings.key_channelList_pagedown || msg == CRCInput::RC_green)
                {
                        unsigned int step = 0;
                        int prev_selected = selected;

			step =  ((int) msg == g_settings.key_channelList_pagedown || (int)msg == CRCInput::RC_green) ? listmaxshow : 1;  // browse or step 1
                        selected += step;

                        if(selected >= channels.size()) 
			{
                                if (((channels.size() / listmaxshow) + 1) * listmaxshow == channels.size() + listmaxshow) 	// last page has full entries
                                        selected = 0;
                                else
                                        selected = ((step == listmaxshow) && (selected < (((channels.size() / listmaxshow)+1) * listmaxshow))) ? (channels.size() - 1) : 0;
			}

                        paintItem(prev_selected - liststart);
			
                        unsigned int oldliststart = liststart;
                        liststart = (selected/listmaxshow)*listmaxshow;
                        if(oldliststart != liststart)
                                paint();
                        else
                                paintItem(selected - liststart);
                }
                else if ( msg == CRCInput::RC_ok || msg == (neutrino_msg_t) g_settings.mpkey_play) 
		{
			zapOnExit = true;
			loop = false;
		}
		else if (msg == CRCInput::RC_info) 
		{
			showFileInfoWebTV(selected);
			res = -1;
			
			goto showList;
		}
		else if (msg == CRCInput::RC_red) 
		{
			addUserBouquet();
			res = -1;
			
			goto showList;
		}
		else if(msg == CRCInput::RC_blue || msg == CRCInput::RC_favorites)
		{
			showUserBouquet();
			res = -1;
			
			goto showList;
		}
		else if( msg == (neutrino_msg_t) g_settings.key_timeshift) // pause playing
		{
			if(playstate == PAUSE)
				continuePlayBack();
			else if(playstate == PLAY)
				pausePlayBack();
			
			res = -1;
			loop = false;
		}
		else if( msg == CRCInput::RC_stop) // pause playing
		{
			if(playstate == PLAY || playstate == PAUSE)
				stopPlayBack();
			
			res = -1;
			loop = false;
		}
		else if(msg == (neutrino_msg_t)g_settings.mpkey_play)
		{
			if(playstate == PAUSE)
				continuePlayBack();
			
			res = -1;
			loop = false;
		}
		else if ( (msg == NeutrinoMessages::EVT_TIMER) && (data == sec_timer_id) )
		{
			// head
			paintHead();
	
			// Foot
			//paintFoot();
	
			// paint all
			//paint();
		} 
		else
		{
			if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all ) 
			{
				loop = false;
				res = - 1;
			}
		}
			
		frameBuffer->blit();	
	}
	
	hide();

	//
	g_RCInput->killTimer(sec_timer_id);
	sec_timer_id = 0;
	
	//CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
	
	if(zapOnExit)
		res = selected;

	dprintf(DEBUG_NORMAL, "CWebTV::show res %d\n", res);
			
	return (res);
}

void CWebTV::hide()
{
	frameBuffer->paintBackgroundBoxRel(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight);
			
        clearItem2DetailsLine();
	frameBuffer->blit();
}

void CWebTV::paintItem(int pos)
{
	// Item
	cFrameBoxItem.iX = cFrameBox.iX;
	cFrameBoxItem.iY = cFrameBox.iY + cFrameBoxTitle.iHeight + pos*cFrameBoxItem.iHeight;
	cFrameBoxItem.iWidth = cFrameBox.iWidth - SCROLLBAR_WIDTH;

	uint8_t    color = COL_MENUCONTENT;
	fb_pixel_t bgcolor = COL_MENUCONTENT_PLUS_0;
	unsigned int curr = liststart + pos;
	
	if (curr == selected) 
	{
		color   = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
		
		// itemlines	
		paintItem2DetailsLine(pos);		
		
		// details
		paintDetails(curr);
	} 
	
	// itembox
	frameBuffer->paintBoxRel(cFrameBoxItem.iX, cFrameBoxItem.iY, cFrameBoxItem.iWidth, cFrameBoxItem.iHeight, bgcolor);

	//name and description
	if(curr < channels.size()) 
	{
		char tmp[10];
		char nameAndDescription[255];
		int l = 0;
		
		sprintf((char*) tmp, "%d", curr + 1);
		l = snprintf(nameAndDescription, sizeof(nameAndDescription), "%s", channels[curr]->title.c_str());
		
		// number
		int numpos = cFrameBoxItem.iX + BORDER_LEFT + numwidth - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth(tmp);

		g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(numpos, cFrameBoxItem.iY + (cFrameBoxItem.iHeight - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight(), numwidth + 5, tmp, color, 0, true);
		
		unsigned int ch_name_len = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getRenderWidth(nameAndDescription, true);
		
		// description
		std::string Descr = channels[curr]->description.c_str();
		if(!(Descr.empty()))
		{
			snprintf(nameAndDescription + l, sizeof(nameAndDescription) -l, "  -  ");
			
			ch_name_len = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getRenderWidth(nameAndDescription, true);
			unsigned int ch_desc_len = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getRenderWidth(channels[curr]->description, true);
			
			//channel name
			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(cFrameBoxItem.iX + BORDER_LEFT + numwidth + 10, cFrameBoxItem.iY + cFrameBoxItem.iHeight, cFrameBox.iWidth - BORDER_LEFT - SCROLLBAR_WIDTH - numwidth - 10 - ch_name_len, nameAndDescription, color, 0, true);
			
			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString(cFrameBoxItem.iX + BORDER_LEFT + numwidth + 10 + ch_name_len + 5, cFrameBoxItem.iY + cFrameBoxItem.iHeight, cFrameBoxItem.iWidth - BORDER_LEFT - SCROLLBAR_WIDTH - numwidth - ch_name_len - ch_desc_len - 15, channels[curr]->description, (curr == selected)?COL_MENUCONTENTSELECTED : COL_COLORED_EVENTS_CHANNELLIST, 0, true);
		}
		else
			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(cFrameBoxItem.iX + BORDER_LEFT + numwidth + 10, cFrameBoxItem.iY + cFrameBoxItem.iHeight, cFrameBoxItem.iWidth - BORDER_LEFT - SCROLLBAR_WIDTH - numwidth - ch_name_len - 10, nameAndDescription, color, 0, true);
	}
}

// paint head
void CWebTV::paintHead()
{
	dprintf(DEBUG_DEBUG, "CWebTV::paintHead\n");

	// head
	cFrameBoxTitle.iX = cFrameBox.iX;
	cFrameBoxTitle.iY = cFrameBox.iY;
	cFrameBoxTitle.iWidth = cFrameBox.iWidth;

	frameBuffer->paintBoxRel(cFrameBoxTitle.iX, cFrameBoxTitle.iY, cFrameBoxTitle.iWidth, cFrameBoxTitle.iHeight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, g_settings.menu_Head_gradient); //round
	
	// head icon
	titleIcon.setIcon(NEUTRINO_ICON_WEBTV_SMALL);
	frameBuffer->paintIcon(titleIcon.iconName.c_str(), cFrameBoxTitle.iX + BORDER_LEFT, cFrameBoxTitle.iY + (cFrameBoxTitle.iHeight - titleIcon.iHeight)/2 );
	
	// help icon
	titleButton.setIcon(NEUTRINO_ICON_BUTTON_HELP);
	frameBuffer->paintIcon(titleButton.iconName.c_str(), cFrameBoxTitle.iX + cFrameBox.iWidth - BORDER_RIGHT - titleButton.iWidth, cFrameBoxTitle.iY + (cFrameBoxTitle.iHeight - titleButton.iHeight)/2 );
	
	// paint time/date
	int timestr_len = 0;
	std::string timestr = getNowTimeStr("%d.%m.%Y %H:%M");
	timestr_len = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getRenderWidth(timestr, true); // UTF-8
		
	g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->RenderString(cFrameBoxTitle.iX + cFrameBoxTitle.iWidth - BORDER_LEFT - BORDER_RIGHT - titleButton.iWidth - timestr_len, cFrameBoxTitle.iY + (cFrameBoxTitle.iHeight - g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight(), timestr_len, timestr, COL_MENUHEAD, 0, true); //utf-8
	
	//head title
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(cFrameBoxTitle.iX + BORDER_LEFT + titleIcon.iWidth + 5, cFrameBoxTitle.iY + (cFrameBoxTitle.iHeight - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), cFrameBoxTitle.iWidth - BORDER_LEFT - BORDER_RIGHT - titleIcon.iWidth - timestr_len, title.c_str(), COL_MENUHEAD, 0, true); // UTF-8
}

// 
#define NUM_LIST_BUTTONS 4
struct button_label CWebTVButtons[NUM_LIST_BUTTONS] =
{
	{ NEUTRINO_ICON_BUTTON_RED, LOCALE_WEBTV_BOUQUETS, NULL},
	{ NEUTRINO_ICON_BUTTON_GREEN , LOCALE_FILEBROWSER_NEXTPAGE, NULL},
	{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_FILEBROWSER_PREVPAGE, NULL},
	{ NEUTRINO_ICON_BUTTON_BLUE, LOCALE_WEBTV_BOUQUETS, NULL}
};

void CWebTV::paintFoot()
{
	dprintf(DEBUG_DEBUG, "CWebTV::paintFoot\n");

	// foot
	cFrameBoxFoot.iX = cFrameBox.iX;
	cFrameBoxFoot.iY = cFrameBox.iY + cFrameBox.iHeight - cFrameBoxInfo.iHeight - 2 - cFrameBoxFoot.iHeight;
	cFrameBoxFoot.iWidth = cFrameBox.iWidth;

	int ButtonWidth = (cFrameBoxFoot.iWidth - BORDER_LEFT - BORDER_RIGHT) / 4;
	
	frameBuffer->paintBoxRel(cFrameBoxFoot.iX, cFrameBoxFoot.iY, cFrameBoxFoot.iWidth, cFrameBoxFoot.iHeight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.menu_Foot_gradient); //round
	
	::paintButtons(frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, cFrameBoxFoot.iX + BORDER_LEFT, cFrameBoxFoot.iY, ButtonWidth, NUM_LIST_BUTTONS, CWebTVButtons, cFrameBoxFoot.iHeight);
}

// infos
void CWebTV::paintDetails(int index)
{
	dprintf(DEBUG_DEBUG, "CWebTV::paintDetails\n");

	// Info
	cFrameBoxInfo.iX = cFrameBox.iX;
	cFrameBoxInfo.iY = cFrameBox.iY + cFrameBox.iHeight - cFrameBoxInfo.iHeight;
	cFrameBoxInfo.iWidth = cFrameBox.iWidth;

	// infobox refresh
	frameBuffer->paintBoxRel(cFrameBoxInfo.iX + 2, cFrameBoxInfo.iY + 2, cFrameBoxInfo.iWidth - 4, cFrameBoxInfo.iHeight - 4, COL_MENUCONTENTDARK_PLUS_0, NO_RADIUS, CORNER_NONE, g_settings.menu_Head_gradient);
	
	if(channels.empty() )
		return;
	
	// name/description
	g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(cFrameBoxInfo.iX + BORDER_LEFT, cFrameBoxInfo.iY + 5 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), cFrameBoxInfo.iWidth - 30, channels[index]->title.c_str(), COL_MENUCONTENTDARK, 0, true);

	g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString (cFrameBoxInfo.iX + BORDER_LEFT, cFrameBoxInfo.iY + 5 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight() + 5 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight(), cFrameBoxInfo.iWidth - 30, channels[index]->description.c_str(), COL_MENUCONTENTDARK, 0, true); // UTF-8
}

void CWebTV::clearItem2DetailsLine()
{  
	::clearItem2DetailsLine(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight - cFrameBoxInfo.iHeight, cFrameBoxInfo.iHeight); 
}

void CWebTV::paintItem2DetailsLine(int pos)
{
	::paintItem2DetailsLine(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight - cFrameBoxInfo.iHeight, cFrameBoxInfo.iHeight, cFrameBoxTitle.iHeight, cFrameBoxItem.iHeight, pos);
}

// body
void CWebTV::paint()
{
	dprintf(DEBUG_INFO, "CWebTV::paint: selected:%d\n", selected);

	liststart = (selected/listmaxshow)*listmaxshow;
	
	int lastnum = liststart + listmaxshow;
	
	if(lastnum<10)
		numwidth = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth("0");
	else if(lastnum<100)
		numwidth = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth("00");
	else if(lastnum<1000)
		numwidth = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth("000");
	else if(lastnum<10000)
		numwidth = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth("0000");
	else // if(lastnum<100000)
		numwidth = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth("00000");
	
	// Body
	cFrameBoxBody.iX = cFrameBox.iX;
	cFrameBoxBody.iY = cFrameBox.iY + cFrameBoxTitle.iHeight;
	cFrameBoxBody.iWidth = cFrameBox.iWidth;
	cFrameBoxBody.iHeight = cFrameBox.iHeight - cFrameBoxTitle.iHeight - cFrameBoxFoot.iHeight - 2 - cFrameBoxInfo.iHeight;
	
	frameBuffer->paintBoxRel(cFrameBoxBody.iX, cFrameBoxBody.iY, cFrameBoxBody.iWidth, cFrameBoxBody.iHeight, COL_MENUCONTENT_PLUS_0);
	
	// paint item
	for(unsigned int count = 0; count < listmaxshow; count++) 
	{
		paintItem(count);
	}

	// ScrollBar
	cFrameBoxScrollBar.iX = cFrameBox.iX + cFrameBox.iWidth - SCROLLBAR_WIDTH;
	cFrameBoxScrollBar.iY = cFrameBox.iY + cFrameBoxTitle.iHeight;
	cFrameBoxScrollBar.iWidth = SCROLLBAR_WIDTH;
	cFrameBoxScrollBar.iHeight = cFrameBoxItem.iHeight*listmaxshow;
	
	frameBuffer->paintBoxRel(cFrameBoxScrollBar.iX, cFrameBoxScrollBar.iY, cFrameBoxScrollBar.iWidth, cFrameBoxScrollBar.iHeight, COL_MENUCONTENT_PLUS_1);

	// scrollBar Slider
	int sbc = ((channels.size() - 1)/ listmaxshow) + 1;
	int sbs = (selected/listmaxshow);

	frameBuffer->paintBoxRel(cFrameBoxScrollBar.iX + 2, cFrameBoxScrollBar.iY + 2 + sbs*(cFrameBoxScrollBar.iHeight - 4)/sbc, cFrameBoxScrollBar.iWidth - 4, (cFrameBoxScrollBar.iHeight - 4)/sbc, COL_MENUCONTENT_PLUS_3);
}

void CWebTV::showFileInfoWebTV(int pos)
{
	if(pos > -1)
	{
		CBox position(g_settings.screen_StartX + 50, g_settings.screen_StartY + 50, g_settings.screen_EndX - g_settings.screen_StartX - 100, g_settings.screen_EndY - g_settings.screen_StartY - 100); 
	
		CInfoBox * infoBox = new CInfoBox(channels[pos]->description.c_str(), g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1], CTextBox::SCROLL, &position, channels[pos]->title.c_str(), g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE], NEUTRINO_ICON_WEBTV_SMALL);
	
		infoBox->exec();
		delete infoBox;
		infoBox = NULL;
	}
}

void CWebTV::addUserBouquet(void)
{
	dprintf(DEBUG_NORMAL, "CWebTV::addUserBouquet\n");

	CFileBrowser filebrowser;
	CFileFilter fileFilter;
	
	fileFilter.addFilter("xml");
	fileFilter.addFilter("tv");
	fileFilter.addFilter("m3u");

	//
	hide();

	//
	CFileList filelist;
	CMenuWidget m(LOCALE_WEBTV_BOUQUETS, NEUTRINO_ICON_WEBTV_SMALL, MENU_WIDTH + 100);
	m.disableMenuPosition();

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
			strReplace(bTitle, ".xml", "");
			strReplace(bTitle, ".tv", "");
			strReplace(bTitle, ".m3u", "");
			strReplace(bTitle, "userbouquet.", "");

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
		selected = 0;
	}
}



