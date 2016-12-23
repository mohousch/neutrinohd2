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

#include <xmlinterface.h>

#include <sectionsd/edvbstring.h>

#include <system/debug.h>
#include <system/helpers.h>

// libdvbapi
#include <playback_cs.h>

// curl
#include <curl/curl.h>
#include <curl/easy.h>

#include <webtv.h>


extern cPlayback *playback;

CWebTV::CWebTV()
{
	tuned = -1;
	
	parser = NULL;
	
	position = 0;
	duration = 0;
	file_prozent = 0;
	
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
		g_InfoViewer->showMovieInfo(channels[tuned]->title, channels[tuned]->description, file_prozent, duration, ac3state, speed, playstate, false, false);
}

void CWebTV::showInfo()
{
	dprintf(DEBUG_INFO, "CWebTV::showInfo\n");

	//infoviewer
	if(tuned > -1)
		g_InfoViewer->showMovieInfo(channels[tuned]->title, channels[tuned]->description, file_prozent, duration, ac3state, speed, playstate, false, false);
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

	CFileFilter fileFilter;
	
	fileFilter.addFilter("xml");
	fileFilter.addFilter("tv");
	fileFilter.addFilter("m3u");

	//
	CFileList filelist;
	CMenuWidget m(LOCALE_WEBTV_BOUQUETS, NEUTRINO_ICON_WEBTV_SMALL, MENU_WIDTH + 100);
	m.disableMenuPosition();
	m.enableSaveScreen(true);

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
	}
}

#define HEAD_BUTTONS_COUNT	1
const struct button_label HeadButtons[HEAD_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_HELP, NONEXISTANT_LOCALE, NULL }	
};

#define FOOT_BUTTONS_COUNT 4
struct button_label FootButtons[FOOT_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_RED, LOCALE_WEBTV_BOUQUETS, NULL},
	{ NEUTRINO_ICON_BUTTON_GREEN , LOCALE_FILEBROWSER_NEXTPAGE, NULL},
	{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_FILEBROWSER_PREVPAGE, NULL},
	{ NEUTRINO_ICON_BUTTON_BLUE, LOCALE_WEBTV_BOUQUETS, NULL}
};

void CWebTV::show(bool reload)
{
	dprintf(DEBUG_NORMAL, "CWebTV::hide:\n");

	// load channesl
	if(reload)
		loadChannels();

	//
	webTVlistMenu = new CMenulistBox(title.c_str(), NEUTRINO_ICON_WEBTV_SMALL, w_max ( (CFrameBuffer::getInstance()->getScreenWidth() / 20 * 17), (CFrameBuffer::getInstance()->getScreenWidth() / 20 )), h_max ( (CFrameBuffer::getInstance()->getScreenHeight() / 20 * 16), (CFrameBuffer::getInstance()->getScreenHeight() / 20)));

	if(channels.size())
	{
		for(unsigned int i = 0; i< channels.size(); i++)
		{
			webTVlistMenu->addItem(new CMenulistBoxItem(channels[i]->title.c_str(), true, this, "zapit", NULL, (i +1), file_prozent, channels[i]->description.c_str(), "", "", channels[i]->title.c_str(), channels[i]->description.c_str(), "", ""));
		}
	}

	webTVlistMenu->setTimeOut(g_settings.timing[SNeutrinoSettings::TIMING_CHANLIST]);
	webTVlistMenu->setSelected(tuned);

	webTVlistMenu->setHeaderButtons(HeadButtons, HEAD_BUTTONS_COUNT);
	webTVlistMenu->setFooterButtons(FootButtons, FOOT_BUTTONS_COUNT);
	
	webTVlistMenu->enablePaintDate();
	webTVlistMenu->enableFootInfo();

	// head
	webTVlistMenu->addKey(CRCInput::RC_info, this, CRCInput::getSpecialKeyName(CRCInput::RC_info));
	webTVlistMenu->addKey(CRCInput::RC_setup, this, CRCInput::getSpecialKeyName(CRCInput::RC_setup));

	// footer
	webTVlistMenu->addKey(CRCInput::RC_red, this, CRCInput::getSpecialKeyName(CRCInput::RC_red));
	webTVlistMenu->addKey(CRCInput::RC_green, this, CRCInput::getSpecialKeyName(CRCInput::RC_green));
	webTVlistMenu->addKey(CRCInput::RC_yellow, this, CRCInput::getSpecialKeyName(CRCInput::RC_yellow));
	webTVlistMenu->addKey(CRCInput::RC_blue, this, CRCInput::getSpecialKeyName(CRCInput::RC_blue));

	//
	webTVlistMenu->addKey(CRCInput::RC_pause, this, CRCInput::getSpecialKeyName(CRCInput::RC_pause));
	webTVlistMenu->addKey(CRCInput::RC_stop, this, CRCInput::getSpecialKeyName(CRCInput::RC_stop));
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
		stopPlayBack();
		startPlayBack(webTVlistMenu->getSelected());
		tuned = webTVlistMenu->getSelected();

		//infoviewer
		g_InfoViewer->showMovieInfo(channels[tuned]->title, channels[tuned]->description, file_prozent, duration, ac3state, speed, playstate, false, false);

		return menu_return::RETURN_EXIT_ALL;
	}
	else if( (actionKey == "RC_blue") || (actionKey == "RC_red"))
	{
		showUserBouquet();
		show();
		return menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey == "RC_green")
	{
		g_RCInput->postMsg(CRCInput::RC_page_down, 0);
	}
	else if(actionKey == "RC_yellow")
	{
		g_RCInput->postMsg(CRCInput::RC_page_up, 0);
	}
	else if(actionKey == "RC_info")
	{
		showFileInfoWebTV(webTVlistMenu->getSelected());
	}
	else if(actionKey == "RC_pause")
	{
		pausePlayBack();

		//infoviewer
		g_InfoViewer->showMovieInfo(channels[tuned]->title, channels[tuned]->description, file_prozent, duration, ac3state, speed, playstate, false, false);
	}
	else if(actionKey == "RC_play")
	{
		continuePlayBack();

		//infoviewer
		g_InfoViewer->showMovieInfo(channels[tuned]->title, channels[tuned]->description, file_prozent, duration, ac3state, speed, playstate, false, false);
	}
	else if(actionKey == "RC_stop")
	{
		stopPlayBack();

		//infoviewer
		g_InfoViewer->showMovieInfo(channels[tuned]->title, channels[tuned]->description, file_prozent, duration, ac3state, speed, playstate, false, false);
	}

	return menu_return::RETURN_REPAINT;
}

