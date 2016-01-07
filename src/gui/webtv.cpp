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
#include "movieplayer.h"
#include "webtv.h"
#include <gui/widget/buttons.h>
#include <gui/widget/messagebox.h>
#include <gui/widget/helpbox.h>
#include <gui/widget/infobox.h>
#include <gui/widget/hintbox.h>

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
	
	selected = 0;
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
	if (parser)
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
	
	int nNewChannel = Show();
	
	// zapto
	if ( nNewChannel > -1 && nNewChannel < (int) channels.size()) 
		zapTo(nNewChannel, rezap);

	return nNewChannel;
}

void CWebTV::loadChannels(void)
{
	readChannellist(g_settings.userBouquet);
	
	title = std::string(rindex(g_settings.userBouquet.c_str(), '/') + 1);
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
	addUserBouquet();
}

bool CWebTV::startPlayBack(int pos)
{
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
	playback->Close();
	playstate = STOPPED;
}

void CWebTV::pausePlayBack(void)
{
	playback->SetSpeed(0);
	playstate = PAUSE;
	speed = 0;
}

void CWebTV::continuePlayBack(void)
{
	playback->SetSpeed(1);
	playstate = PLAY;
	speed = 1;
}

//
void CWebTV::zapTo(int pos, bool rezap)
{
	// show emty channellist error msg
	if (channels.empty()) 
	{
		MessageBox(LOCALE_MESSAGEBOX_ERROR, g_Locale->getText(LOCALE_WEBTV_CHANNELLIST_NONEFOUND), CMessageBox::mbrCancel, CMessageBox::mbCancel, NEUTRINO_ICON_ERROR);
		return;
	}

	// if not mached
	if ( (pos >= (signed int) channels.size()) || (pos < 0) ) 
	{
		pos = 0;
	}
	
	// check if the same channel
	if ( pos != tuned || rezap) 
	{
		tuned = pos;
		
		// 
		if(playback->playing)
			playback->Stop();
	
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
}

void CWebTV::quickZap(int key)
{
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
	int res = -1;
	
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;
	
	if(channels.empty())
		loadChannels();
	
	// display channame in vfd	
	CVFD::getInstance()->setMode(CVFD::MODE_IPTV);
	
	// windows size
	width  = w_max ( (frameBuffer->getScreenWidth() / 20 * 17), (frameBuffer->getScreenWidth() / 20 ));
	height = h_max ( (frameBuffer->getScreenHeight() / 20 * 16), (frameBuffer->getScreenHeight() / 20));
	
	// head height
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_HELP, &icon_hd_w, &icon_hd_h);
	theight = std::max(icon_hd_h, g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight()) + 6;
	
	// buttonheight
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_RED, &icon_bf_w, &icon_bf_h);
	buttonHeight = std::max(icon_bf_h, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight()) + 6;

	// listbox/items
	iheight = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight() + 2;
	listmaxshow = (height - theight - buttonHeight)/iheight;
	height = theight + buttonHeight + listmaxshow * iheight;
	
	// info height
	info_height = 5 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight() + 5 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight() + 5;
	
	// x/y
	x = frameBuffer->getScreenX() + (frameBuffer->getScreenWidth() - (width + ConnectLineBox_Width)) / 2 + ConnectLineBox_Width;
	y = frameBuffer->getScreenY() + (frameBuffer->getScreenHeight() - (height + info_height)) / 2;
	
showList:	
	
	// head
	paintHead();
	
	// foot
	paintFoot();
		
	// paint all
	paint();
		
	frameBuffer->blit();

	oldselected = selected;
	int zapOnExit = false;

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
			selected = oldselected;
			
			loop = false;
			res = -1;
		}
		else if ( msg == CRCInput::RC_up || (int) msg == g_settings.key_channelList_pageup || msg == CRCInput::RC_yellow)
                {
                        int step = 0;
                        int prev_selected = selected;

			step =  ((int) msg == g_settings.key_channelList_pageup || (int) msg == CRCInput::RC_yellow) ? listmaxshow : 1;  // browse or step 1
                        selected -= step;
                        if((prev_selected-step) < 0)            // because of uint
                                selected = channels.size() - 1;

                        paintItem(prev_selected - liststart);
			
                        unsigned int oldliststart = liststart;
                        liststart = (selected/listmaxshow)*listmaxshow;
                        if(oldliststart!=liststart)
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
	
	//CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
	
	if(zapOnExit)
		res = selected;

	printf("CWebTV::show res %d\n", res);
			
	return (res);
}

void CWebTV::hide()
{
	frameBuffer->paintBackgroundBoxRel(x, y, width + 5, height + info_height + 5);
			
        clearItem2DetailsLine();
	frameBuffer->blit();
}

void CWebTV::paintItem(int pos)
{
	int ypos = y + theight + pos*iheight;
	uint8_t    color;
	fb_pixel_t bgcolor;
	unsigned int curr = liststart + pos;
	
	if (curr == selected) 
	{
		color   = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
		
		// itemlines	
		paintItem2DetailsLine(pos, curr);		
		
		// details
		paintDetails(curr);
	} 
	else 
	{
		color = COL_MENUCONTENT;
		bgcolor = COL_MENUCONTENT_PLUS_0;
	}
	
	// itembox
	frameBuffer->paintBoxRel(x, ypos, width - SCROLLBAR_WIDTH, iheight, bgcolor);

	//name and description
	if(curr < channels.size()) 
	{
		char tmp[10];
		char nameAndDescription[255];
		int l = 0;
		
		sprintf((char*) tmp, "%d", curr + 1);
		l = snprintf(nameAndDescription, sizeof(nameAndDescription), "%s", channels[curr]->title.c_str());
		
		// number
		int numpos = x + BORDER_LEFT + numwidth - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth(tmp);
		g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(numpos, ypos + (iheight - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight(), numwidth + 5, tmp, color, 0, true);
		
		unsigned int ch_name_len = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getRenderWidth(nameAndDescription, true);
		
		// description
		std::string Descr = channels[curr]->description.c_str();
		if(!(Descr.empty()))
		{
			snprintf(nameAndDescription + l, sizeof(nameAndDescription) -l, "  -  ");
			
			ch_name_len = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getRenderWidth(nameAndDescription, true);
			unsigned int ch_desc_len = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getRenderWidth(channels[curr]->description, true);
			
			//channel name
			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(x + BORDER_LEFT + numwidth + 10, ypos + iheight, width - BORDER_LEFT - SCROLLBAR_WIDTH - numwidth - 10 - ch_name_len, nameAndDescription, color, 0, true);
			
			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString(x + BORDER_LEFT + numwidth + 10 + ch_name_len + 5, ypos + iheight, width - BORDER_LEFT - SCROLLBAR_WIDTH - numwidth - ch_name_len - ch_desc_len - 15, channels[curr]->description, (curr == selected)?COL_MENUCONTENTSELECTED : COL_COLORED_EVENTS_CHANNELLIST, 0, true);
		}
		else
			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(x + BORDER_LEFT + numwidth + 10, ypos + iheight, width - BORDER_LEFT - SCROLLBAR_WIDTH - numwidth - ch_name_len - 10, nameAndDescription, color, 0, true);
	}
}

#define NUM_LIST_BUTTONS 4
struct button_label CWebTVButtons[NUM_LIST_BUTTONS] =
{
	{ NEUTRINO_ICON_BUTTON_RED, LOCALE_WEBTV_ADD_BOUQUETS},
	{ NEUTRINO_ICON_BUTTON_GREEN , LOCALE_FILEBROWSER_NEXTPAGE},
	{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_FILEBROWSER_PREVPAGE},
	{ NEUTRINO_ICON_BUTTON_BLUE, LOCALE_WEBTV_BOUQUETS}
};

// paint head
void CWebTV::paintHead()
{
	// head
	frameBuffer->paintBoxRel(x, y, width, theight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, true, gradientLight2Dark); //round
	
	// head icon
	int icon_webtv_w, icon_webtv_h;
	frameBuffer->getIconSize(NEUTRINO_ICON_WEBTV_SMALL, &icon_webtv_w, &icon_webtv_h);
	frameBuffer->paintIcon(NEUTRINO_ICON_WEBTV_SMALL, x + BORDER_LEFT, y + ( theight - icon_webtv_h)/2 );
	
	// help icon
	int icon_help_w, icon_help_h;
	
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_HELP, &icon_help_w, &icon_help_h);
	frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_HELP, x + width - (BORDER_RIGHT + icon_help_w) , y + (theight - icon_help_h)/2 );
	
	// paint time/date
	int timestr_len = 0;
	std::string timestr = getNowTimeStr("%d.%m.%Y %H:%M");
	timestr_len = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getRenderWidth(timestr, true); // UTF-8
		
	g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->RenderString(x + width - BORDER_LEFT - BORDER_RIGHT - icon_help_w - timestr_len, y + (theight - g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight(), timestr_len, timestr, COL_MENUHEAD, 0, true); //utf-8
	
	//head title
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(x + BORDER_LEFT + icon_webtv_w + 5, y + (theight - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), width - 20 - icon_webtv_w - timestr_len, title.c_str(), COL_MENUHEAD, 0, true); // UTF-8
}

void CWebTV::paintFoot()
{
	// foot
	int ButtonWidth = (width - 20) / 4;
	int f_x = x;
	int f_y = y + (height - buttonHeight);
	
	frameBuffer->paintBoxRel(f_x, f_y, width, buttonHeight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_BOTTOM, true, gradientDark2Light); //round
	
	::paintButtons(frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, f_x + BORDER_LEFT, f_y, ButtonWidth, NUM_LIST_BUTTONS, CWebTVButtons, buttonHeight);
}

// infos
void CWebTV::paintDetails(int index)
{
	// infobox refresh
	frameBuffer->paintBoxRel(x + 2, y + height + 2, width - 4, info_height - 4, COL_MENUCONTENTDARK_PLUS_0, 0, 0, true, gradientLight2Dark);
	
	if(channels.empty() )
		return;
	
	// name/description
	g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(x + BORDER_LEFT, y + height + 5 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), width - 30, channels[index]->title, COL_MENUCONTENTDARK, 0, true);
	g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString (x + BORDER_LEFT, y + height + 5 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight() + 5 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight(), width - 30, channels[index]->description, COL_MENUCONTENTDARK, 0, true); // UTF-8
}

void CWebTV::clearItem2DetailsLine()
{  
	  paintItem2DetailsLine(-1, 0);  
}

void CWebTV::paintItem2DetailsLine(int pos, int /*ch_index*/)
{
	int xpos  = x - ConnectLineBox_Width;
	int ypos1 = y + theight + pos*iheight;
	int ypos2 = y + height;
	int ypos1a = ypos1 + (iheight/2) - 2;
	int ypos2a = ypos2 + (info_height/2) - 2;
	
	fb_pixel_t col1 = COL_MENUCONTENT_PLUS_6;
	fb_pixel_t col2 = COL_MENUCONTENT_PLUS_1;

	// Clear
	frameBuffer->paintBackgroundBoxRel(xpos, y, ConnectLineBox_Width, height + info_height);

	// blit
	frameBuffer->blit();

	// paint Line if detail info (and not valid list pos)
	if (pos >= 0) 
	{ 
		int fh = iheight > 10 ? iheight - 10 : 5;
			
		frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 4, ypos1 + 5, 4, fh, col1);
		frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 4, ypos1 + 5, 1, fh, col2);			
		frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 4, ypos2 + 7, 4, info_height - 14, col1);
		frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 4, ypos2 + 7, 1, info_height - 14, col2);			

		// vertical line
		frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 16, ypos1a, 4, ypos2a - ypos1a, col1);
		frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 16, ypos1a, 1, ypos2a - ypos1a + 4, col2);		

		// Hline Oben
		frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 15, ypos1a, 12, 4, col1);
		frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 16, ypos1a, 12, 1, col2);
		
		// Hline Unten
		frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 15, ypos2a, 12, 4, col1);
		frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 12, ypos2a, 8, 1, col2);

		// untere info box lines
		frameBuffer->paintBoxRel(x, ypos2, width, info_height, col1);
	}
}

// paint
void CWebTV::paint()
{
	liststart = (selected/listmaxshow)*listmaxshow;
	
	int lastnum =  liststart + listmaxshow;
	
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
	
	// channelslist body
	frameBuffer->paintBoxRel(x, y + theight, width, height - buttonHeight - theight, COL_MENUCONTENT_PLUS_0);
	
	// paint item
	for(unsigned int count = 0; count < listmaxshow; count++) 
	{
		paintItem(count);
	}

	// sb
	int ypos = y + theight;
	int sb = iheight*listmaxshow;
	
	frameBuffer->paintBoxRel(x + width - SCROLLBAR_WIDTH, ypos, SCROLLBAR_WIDTH, sb,  COL_MENUCONTENT_PLUS_1);

	int sbc = ((channels.size() - 1)/ listmaxshow) + 1;
	int sbs = (selected/listmaxshow);

	frameBuffer->paintBoxRel(x + width - 13, ypos + 2 + sbs*(sb - 4)/sbc, 11, (sb - 4)/sbc, COL_MENUCONTENT_PLUS_3);
}

void CWebTV::showFileInfoWebTV(int pos)
{
	if(pos > -1)
	{
		InfoBox(channels[pos]->title.c_str(), channels[pos]->description.c_str(), CInfoBox::mbrBack, CInfoBox::mbBack);
	}
}

void CWebTV::addUserBouquet(void)
{
	CFileBrowser filebrowser;
	CFileFilter fileFilter;
	
	fileFilter.addFilter("xml");
	fileFilter.addFilter("tv");
	fileFilter.addFilter("m3u");

	filebrowser.Filter = &fileFilter;

	if (filebrowser.exec(CONFIGDIR "/webtv"))
	{
		g_settings.userBouquet.clear();
		
		g_settings.userBouquet = filebrowser.getSelectedFile()->Name.c_str();
		
		printf("[webtv] webtv settings file %s\n", g_settings.userBouquet.c_str());
		
		// load channels
		loadChannels();
		selected = 0;
	}
}



