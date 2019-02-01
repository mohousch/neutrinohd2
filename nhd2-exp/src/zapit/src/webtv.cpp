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

#include <xmlinterface.h>

#include <system/debug.h>
#include <system/helpers.h>

// libdvbapi
#include <playback_cs.h>

#include <webtv.h>


extern cPlayback *playback;

CWebTV::CWebTV()
{	
	playstate = STOPPED;
}

CWebTV::~CWebTV()
{
	clearChannels();
}

void CWebTV::loadWebTVBouquet(std::string filename)
{
	dprintf(DEBUG_NORMAL, "CWebTV::loadWebTVBouquet: parsing %s\n", filename.c_str());

	_xmlDocPtr parser = NULL;
	
	// check for extension
	bool iptv = false;
	bool webtv = false;
	bool playlist = false;
	int cnt = 1;
					
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
						 id = create_channel_id64(0, 0, 0, 0, 0, url.c_str());
					
					CZapitChannel * chan = new CZapitChannel(title, id, url, description);

					if (chan != NULL) 
					{
						chan->number = cnt;
						channels.push_back(chan);
					}

					cnt++;
				}
			}
			
			fclose(f);
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
			_xmlNodePtr l0 = NULL;
			_xmlNodePtr l1 = NULL;
			l0 = xmlDocGetRootElement(parser);
			l1 = l0->xmlChildrenNode;
			
			neutrino_msg_t      msg;
			neutrino_msg_data_t data;
			
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
							id = create_channel_id64(0, 0, 0, 0, 0, url);
						
						CZapitChannel * chan = new CZapitChannel(title, id, url,  description);


						if (chan != NULL) 
						{
							chan->number = cnt;
							channels.push_back(chan);
						}
					}

					l1 = l1->xmlNextNode;
					g_RCInput->getMsg(&msg, &data, 0);
					cnt++;
				}
			}
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
							id = create_channel_id64(0, 0, 0, 0, 0, url);
					
						CZapitChannel * chan = new CZapitChannel(name, id, url,  description);

						if (chan != NULL) 
						{
							chan->number = cnt;
							channels.push_back(chan);
						}

						cnt++;
					}
				}
			}
		}
		infile.close();
	}
}

void CWebTV::clearChannels(void)
{
	dprintf(DEBUG_INFO, "CWebTV::clearChannels\n");
	
	for (std::vector<CZapitChannel*>::iterator it = channels.begin(); it != channels.end(); it++) 
	{
               	delete (*it);
        }
	
	channels.clear();
}

void CWebTV::loadChannels(void)
{
	dprintf(DEBUG_INFO, "CWebTV::loadChannels\n");

	// load all tv channels
	channels.clear();

	loadWebTVBouquet(g_settings.webtv_userBouquet);
	
	title = std::string(rindex(g_settings.webtv_userBouquet.c_str(), '/') + 1);

	removeExtension(title);
}

CZapitClient::CCurrentServiceInfo CWebTV::getServiceInfo()
{
	CZapitClient::CCurrentServiceInfo serviceInfo;
	memset(&serviceInfo, 0, sizeof(CZapitClient::CCurrentServiceInfo));

	serviceInfo.onid = 0;
	serviceInfo.sid = 0;
	serviceInfo.tsid = 0;
	serviceInfo.vpid = 0;
	serviceInfo.apid = 0;
	serviceInfo.vtxtpid = 0;
	serviceInfo.pmtpid = 0;		
	serviceInfo.pmt_version = 0xff;			
	serviceInfo.pcrpid = 0;
	serviceInfo.tsfrequency = 0;
	serviceInfo.rate = 0;
	serviceInfo.fec = (fe_code_rate)0;					
	serviceInfo.polarisation = 0;
	serviceInfo.vtype = 0;
}

void CWebTV::getPIDS(CZapitClient::responseGetPIDs& pids)
{
	CZapitClient::responseGetOtherPIDs otherPIDs;

	otherPIDs.vpid = 0;
	otherPIDs.vtxtpid = 0;
	otherPIDs.pmtpid = 0;
	otherPIDs.pcrpid = 0;
	otherPIDs.selected_apid = 0;
	otherPIDs.privatepid = 0;
}

const std::string& CWebTV::getChannelName(t_channel_id id)
{
	for(unsigned int i = 0; i < channels.size(); i++)
	{
		if(channels[i]->channel_id == id)
			return channels[i]->name;
	}

	return std::string("");
}

const std::string& CWebTV::getChannelURL(t_channel_id id)
{
	for(unsigned int i = 0; i < channels.size(); i++)
	{
		if(channels[i]->channel_id == id)
			return channels[i]->url;
	}

	return std::string("");
}

bool CWebTV::startPlayBack(t_channel_id chid)
{
	dprintf(DEBUG_NORMAL, "CWebTV::startPlayBack\n");

	playback->Open();
	
	if (!playback->Start((char *)getChannelURL(chid).c_str()))
		return false;
	
	playstate = PLAY;

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
}

void CWebTV::continuePlayBack(void)
{
	dprintf(DEBUG_DEBUG, "CWebTV::continuePlayBack\n");

	playback->SetSpeed(1);
	playstate = PLAY;
}

#include <libeventserver/eventserver.h>
extern CEventServer *eventServer;
unsigned int CWebTV::zapTo_ChannelID_NOWAIT(t_channel_id channel_id)
{
	dprintf(DEBUG_NORMAL, "CWebTV::zapTo_ChannelID: %llx\n", channel_id);

	unsigned int result = 0;

	stopPlayBack();
	
	if(!startPlayBack(channel_id))
	{
		dprintf(DEBUG_NORMAL, "CWebTV::zapTo_ChannelID: zapit failed, chid %llx\n", channel_id);
		
		eventServer->sendEvent(CZapitClient::EVT_ZAP_FAILED, CEventServer::INITID_ZAPIT, &channel_id, sizeof(channel_id));
		
		return result;
	}

	playstate = PLAY;

	result |= CZapitClient::ZAP_OK;

	dprintf(DEBUG_NORMAL, "CWebTV::zapTo_ChannelID: zapit OK, chid %llx\n", channel_id);
	
	eventServer->sendEvent(CZapitClient::EVT_ZAP_COMPLETE, CEventServer::INITID_ZAPIT, &channel_id, sizeof(channel_id));

	return result;
}



