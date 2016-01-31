/*
  Neutrino-GUI  -   DBoxII-Project

  UPnP Browser (c) 2007 by Jochen Friedrich

  Homepage: http://dbox.cyberphoria.org/

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

#include <sstream>
#include <stdexcept>

#include <gui/upnpbrowser.h>

#include <global.h>
#include <neutrino.h>
#include <xmltree.h>
#include <upnpclient.h>

#include <driver/encoding.h>
#include <driver/fontrenderer.h>
#include <driver/rcinput.h>
#include <driver/audioplay.h>
#include <driver/audiofile.h>
#include <driver/audiometadata.h>

#include <daemonc/remotecontrol.h>

#include <gui/eventlist.h>
#include <gui/color.h>
#include <gui/infoviewer.h>

#include <gui/widget/buttons.h>
#include <gui/widget/icons.h>
#include <gui/widget/menue.h>
#include <gui/widget/messagebox.h>
#include <gui/widget/hintbox.h>
#include <gui/widget/stringinput.h>
#include <gui/widget/stringinput_ext.h>
#include <gui/widget/items2detailsline.h>

#include <system/settings.h>
#include <system/debug.h>
#include <gui/pictureviewer.h>
#include <gui/movieplayer.h>
#include <gui/webtv.h>
#include <gui/audioplayer.h>


extern CWebTV * webtv;

const struct button_label RescanButton = {NEUTRINO_ICON_BUTTON_BLUE  , LOCALE_UPNPBROWSER_RESCAN, NULL };
const struct button_label StopButton   = {NEUTRINO_ICON_BUTTON_RED, LOCALE_AUDIOPLAYER_STOP, NULL };
const struct button_label PUpButton    = {NEUTRINO_ICON_BUTTON_GREEN, LOCALE_FILEBROWSER_NEXTPAGE, NULL };
const struct button_label PDownButton  = {NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_FILEBROWSER_PREVPAGE, NULL };

CUpnpBrowserGui::CUpnpBrowserGui()
{
	m_socket = new CUPnPSocket();
	m_frameBuffer = CFrameBuffer::getInstance();
	m_playing_entry_is_shown = false;
}

CUpnpBrowserGui::~CUpnpBrowserGui()
{
	delete m_socket;
}

int CUpnpBrowserGui::exec(CMenuTarget* parent, const std::string & /*actionKey*/)
{
	dprintf(DEBUG_NORMAL, "CUpnpBrowserGui::exec:\n");

	CAudioPlayer::getInstance()->init();

	if(parent)
		parent->hide();

	// tell neutrino we're in audio mode
	CNeutrinoApp::getInstance()->handleMsg( NeutrinoMessages::CHANGEMODE , NeutrinoMessages::mode_audio);
	
	// remember last mode
	m_LastMode = (CNeutrinoApp::getInstance()->getLastMode());

	m_width = (g_settings.screen_EndX - g_settings.screen_StartX) - ConnectLineBox_Width;
	m_height = (g_settings.screen_EndY - g_settings.screen_StartY);
	
	m_frameBuffer->getIconSize(NEUTRINO_ICON_UPNP, &icon_head_w, &icon_head_h);

	m_sheight = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight();
	m_frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_OKAY, &icon_foot_w, &icon_foot_h);
	m_buttonHeight = std::max(icon_foot_h, m_sheight) + 10;				//foot
	m_theight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(); 	//title
	m_mheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();		
	m_fheight = g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getHeight();
	m_title_height = m_mheight*2 + 20 + m_sheight + 4;				// title info 
	m_info_height = m_mheight*2;							//info
	m_listmaxshow = (m_height - (m_title_height + m_theight + m_info_height + 2*m_buttonHeight)) / (m_fheight);
	m_height = m_title_height + m_theight + m_listmaxshow*m_fheight + m_buttonHeight + m_info_height; // recalc height

	m_x = (((g_settings.screen_EndX - g_settings.screen_StartX) - (m_width + ConnectLineBox_Width)) / 2) + g_settings.screen_StartX + ConnectLineBox_Width;
	m_y = (((g_settings.screen_EndY- g_settings.screen_StartY) - m_height)/ 2) + g_settings.screen_StartY;

	//
	if(CNeutrinoApp::getInstance()->getLastMode() == NeutrinoMessages::mode_iptv)
	{
		if(webtv)
			webtv->stopPlayBack();
	}
	else
	{
		// stop playback
		g_Zapit->lockPlayBack();
		
		// Stop sectionsd
		g_Sectionsd->setPauseScanning(true);
	}

	m_indexdevice = 0;
	m_selecteddevice = 0;

	selectDevice();

	//
	if(CAudioPlayer::getInstance()->getState() != CBaseDec::STOP)
		CAudioPlayer::getInstance()->stop();
	
	//
	if(CNeutrinoApp::getInstance()->getLastMode() == NeutrinoMessages::mode_iptv)
	{
		if(webtv)
			webtv->startPlayBack(webtv->getTunedChannel());
	}
	else
	{
		// start playback
		g_Zapit->unlockPlayBack();

		// Start Sectionsd
		g_Sectionsd->setPauseScanning(false);
	}

	CNeutrinoApp::getInstance()->handleMsg( NeutrinoMessages::CHANGEMODE , m_LastMode );
	g_RCInput->postMsg( NeutrinoMessages::SHOW_INFOBAR, 0 );

	// always repaint
	return menu_return::RETURN_REPAINT;
}

void CUpnpBrowserGui::splitProtocol(std::string &protocol, std::string &prot, std::string &network, std::string &mime, std::string &additional)
{
	std::string::size_type pos;
	std::string::size_type startpos = 0;

	pos = protocol.find(":", startpos);
	if (pos != std::string::npos)
	{
		prot = protocol.substr(startpos, pos-startpos);
		startpos = pos + 1;

		pos = protocol.find(":", startpos);
		if (pos != std::string::npos)
		{
			network = protocol.substr(startpos, pos-startpos);
			startpos = pos + 1;

			pos = protocol.find(":", startpos);
			if (pos != std::string::npos)
			{
				mime = protocol.substr(startpos, pos-startpos);
				startpos = pos + 1;

				pos = protocol.find(":", startpos);
				if (pos != std::string::npos)
				{
					additional = protocol.substr(startpos, pos-startpos);
				}
			}
		}
	}
	
	dprintf(DEBUG_DEBUG, "%s -> %s - %s - %s - %s\n", protocol.c_str(), prot.c_str(), network.c_str(), mime.c_str(), additional.c_str());
}

std::vector<UPnPEntry> *CUpnpBrowserGui::decodeResult(std::string result)
{
	XMLTreeParser * parser;
	XMLTreeNode   * root, * node, * snode;
	std::vector<UPnPEntry> * entries;

	parser = new XMLTreeParser("UTF-8");
	parser->Parse(result.c_str(), result.size(), 1);
	root=parser->RootNode();
	
	if (!root)
	{
		delete parser;
		return NULL;
	}
	entries = new std::vector<UPnPEntry>;

	for (node = root->GetChild(); node; node = node->GetNext())
	{
		bool isdir;
		std::string title, artist = "", album = "", id, children;
		char *type, *p;

		if (!strcmp(node->GetType(), "container"))
		{
			std::vector<UPnPResource> resources;
			isdir=true;
			for (snode=node->GetChild(); snode; snode=snode->GetNext())
			{
				type=snode->GetType();
				p = strchr(type,':');
				if (p)
					type = p + 1;
				
				if (!strcmp(type,"title"))
				{
					p = snode->GetData();
					if (!p)
						p = (char *) "";
					title = std::string(p);
				}
			}
			p = node->GetAttributeValue((char *) "id");
			if (!p)
				p = (char *) "";
			id = std::string(p);

			p = node->GetAttributeValue((char *) "childCount");
			if (!p)
				p = (char *) "";
			children = std::string(p);

			UPnPEntry entry = {id, isdir, title, artist, album, children, resources, -1};
			entries->push_back(entry);
		}
		
		if (!strcmp(node->GetType(), "item"))
		{
			std::vector<UPnPResource> resources;
			int preferred = -1;
			std::string protocol, prot, network, mime, additional;
			isdir = false;
			for (snode = node->GetChild(); snode; snode = snode->GetNext())
			{
				std::string duration, url, size;
				unsigned int i;
				type = snode->GetType();
				p = strchr(type,':');
				if (p)
					type = p + 1;

				if (!strcmp(type,"title"))
				{
					p = snode->GetData();
					if (!p)
						p = (char *) "";
					title = std::string(p);
				}
				else if (!strcmp(type,"artist"))
				{
					p = snode->GetData();
					if (!p)
						p = (char *) "";
					artist = std::string(p);
				}
				else if (!strcmp(type,"album"))
				{
					p = snode->GetData();
					if (!p)
						p = (char *) "";
					album = std::string(p);
				}
				else if (!strcmp(type,"res"))
				{
					p = snode->GetData();
					if (!p)
						p = (char *) "";
					url = std::string(p);
					p = snode->GetAttributeValue((char *) "size");
					if (!p)
						p = (char *) "0";
					size = std::string(p);
					p = snode->GetAttributeValue((char *) "duration");
					if (!p)
						p = (char *) "";
					duration = std::string(p);
					p = snode->GetAttributeValue((char *) "protocolInfo");
					if (!p)
						p = (char *) "";
					protocol = std::string(p);
					UPnPResource resource = {url, protocol, size, duration};
					resources.push_back(resource);
				}
				int pref = 0;
				preferred = -1;
				
				for (i = 0; i < resources.size(); i++)
				{
					protocol = resources[i].protocol;
					splitProtocol(protocol, prot, network, mime, additional);
					if (prot != "http-get")
						continue;
					
					if (mime == "image/jpeg" && pref < 1)
					{
						preferred = i;
						pref = 1;
					}
					if (mime == "image/gif" && pref < 2)
					{
						preferred = i;
						pref = 2;
					}

					if (mime == "audio/mpeg" && pref < 3)
					{
						preferred = i;
						pref = 3;
					}
					
					if (mime == "audio/x-vorbis+ogg" && pref < 4)
					{
						preferred=i;
						pref = 4;
					}
					
					//
					if (mime.substr(0, 6) == "video/" && pref < 5)
					{
						preferred = i;
						pref = 5;
					}
					
					if (mime == "video/x-flv" && pref < 6)
					{
						preferred = i;
						pref = 6;
					}
					
					if (mime == "video/mp4" && pref < 7)
					{
						preferred = i;
						pref = 7;
					}
					//
				}
			}
			p = node->GetAttributeValue((char *) "id");
			if (!p)
				p = (char *) "";
			id = std::string(p);

			p = node->GetAttributeValue((char *) "childCount");
			if (!p)
				p=(char *) "";
			children = std::string(p);

			UPnPEntry entry = {id, isdir, title, artist, album, children, resources, preferred};

			entries->push_back(entry);
		}
	}
	delete parser;
	return entries;
}

void CUpnpBrowserGui::selectDevice()
{
	dprintf(DEBUG_NORMAL, "CUpnpBrowserGui::selectDevice:\n");

	bool loop = true;
	bool changed = true;
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	CHintBox *scanBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_UPNPBROWSER_SCANNING)); // UTF-8
	scanBox->paint();

	m_devices = m_socket->Discover("urn:schemas-upnp-org:service:ContentDirectory:1");
	scanBox->hide();

	if (!m_devices.size())
	{
		MessageBox(LOCALE_MESSAGEBOX_INFO, LOCALE_UPNPBROWSER_NOSERVERS, CMessageBox::mbrBack, CMessageBox::mbBack, NEUTRINO_ICON_UPDATE);
		delete scanBox;
		return;
	}	

	// control loop
	while (loop)
	{
		if (changed)
		{
			paintDevice();
			changed = false;
		}
		
		m_frameBuffer->blit();

		g_RCInput->getMsg(&msg, &data, 10); // 1 sec timeout to update play/stop state display
		neutrino_msg_t msg_repeatok = msg & ~CRCInput::RC_Repeat;

		if( msg == CRCInput::RC_timeout)
		{
			// nothing
		}
		else if( msg == CRCInput::RC_home)
		{
			loop = false;
		}
		else if (msg_repeatok == CRCInput::RC_up && m_selecteddevice > 0)
		{
			m_selecteddevice--;
			if (m_selecteddevice < m_indexdevice)
				m_indexdevice -= m_listmaxshow;
			changed = true;
		}
		else if (msg_repeatok == CRCInput::RC_down && m_selecteddevice + 1 < m_devices.size())
		{
			m_selecteddevice++;
			if (m_selecteddevice + 1 > m_indexdevice + m_listmaxshow)
				m_indexdevice += m_listmaxshow;
			changed = true;
		}
		else if( msg == CRCInput::RC_right || msg == CRCInput::RC_ok)
		{
			m_folderplay = false;
			selectItem("0");
			changed = true;
		}
		else if( msg == CRCInput::RC_blue)
		{
			scanBox->paint();

			m_devices = m_socket->Discover("urn:schemas-upnp-org:service:ContentDirectory:1");
			scanBox->hide();
			
			if (!m_devices.size())
			{
				MessageBox(LOCALE_MESSAGEBOX_INFO, LOCALE_UPNPBROWSER_NOSERVERS, CMessageBox::mbrBack, CMessageBox::mbBack, NEUTRINO_ICON_UPDATE);
				delete scanBox;
				return;
			}
			changed = true;
		}
		else if(msg == NeutrinoMessages::RECORD_START ||
			msg == NeutrinoMessages::ZAPTO ||
			msg == NeutrinoMessages::STANDBY_ON ||
			msg == NeutrinoMessages::SHUTDOWN ||
			msg == NeutrinoMessages::SLEEPTIMER)
		{
			loop = false;
			g_RCInput->postMsg(msg, data);
		}
		else if(msg == NeutrinoMessages::EVT_TIMER)
		{
			CNeutrinoApp::getInstance()->handleMsg( msg, data );
		}
		else
		{
			if( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
				loop = false;
			changed = true;
		}
	
		m_frameBuffer->blit();	
	}
	
	delete scanBox;
	
	m_frameBuffer->ClearFrameBuffer();
	m_frameBuffer->blit();
}

void CUpnpBrowserGui::playnext(void)
{
	while (true)
	{
		std::list<UPnPAttribute>attribs;
		std::list<UPnPAttribute>results;
		std::list<UPnPAttribute>::iterator i;
		std::stringstream sindex;
		std::vector<UPnPEntry> *entries = NULL;
		bool rfound = false;
		bool nfound = false;
		bool tfound = false;

		sindex << m_playid;
		attribs.push_back(UPnPAttribute("ObjectID", m_playfolder));
		attribs.push_back(UPnPAttribute("BrowseFlag", "BrowseDirectChildren"));
		attribs.push_back(UPnPAttribute("Filter", "*"));
		attribs.push_back(UPnPAttribute("StartingIndex", sindex.str()));
		attribs.push_back(UPnPAttribute("RequestedCount", "1"));
		attribs.push_back(UPnPAttribute("SortCriteria", ""));

		results = m_devices[m_selecteddevice].SendSOAP("urn:schemas-upnp-org:service:ContentDirectory:1", "Browse", attribs);
		for (i = results.begin(); i!=results.end(); i++)
		{
			if (i->first=="NumberReturned")
			{
				if (atoi(i->second.c_str()) != 1)
				{
					m_folderplay = false;
					return;
				}
				nfound=true;
			}
			
			if (i->first=="TotalMatches")
			{
				tfound=true;
			}
			
			if (i->first=="Result")
			{
				entries=decodeResult(i->second);
				rfound=true;
			}
		}
		m_playid++;
		
		if ((entries != NULL) && (!(*entries)[0].isdir))
		{
			int preferred=(*entries)[0].preferred;
			if (preferred != -1)
			{
				std::string protocol, prot, network, mime, additional;
				protocol=(*entries)[0].resources[preferred].protocol;
				splitProtocol(protocol, prot, network, mime, additional);
				
				if (mime == "audio/mpeg")
				{
					m_playing_entry = (*entries)[0];
					m_playing_entry_is_shown = false;
					
					CAudioPlayerGui tmpAudioPlayerGui;
			
					CAudiofileExt audiofile((*entries)[0].resources[preferred].url, CFile::EXTENSION_MP3);
					tmpAudioPlayerGui.addToPlaylist(audiofile);
					tmpAudioPlayerGui.exec(NULL, "urlplayback");
					
					return;
				}
				else if (mime.substr(0,6) == "video/")
				{
					m_frameBuffer->ClearFrameBuffer();
					m_frameBuffer->blit();	
					
					CFile mfile;
					
					mfile.Name = (*entries)[0].resources[preferred].url.c_str(); //FIXME
					
					CMoviePlayerGui tmpMoviePlayerGui;
					
					tmpMoviePlayerGui.addToPlaylist(mfile);
					tmpMoviePlayerGui.exec(NULL, "urlplayback");
					
					return;
				}
				else if (mime.substr(0,6) == "image/")
				{
					CPictureViewerGui tmpPictureViewerGui;
					CPicture pic;
					struct stat statbuf;
						
					pic.Filename = (*entries)[0].resources[preferred].url;
					std::string tmp = (*entries)[0].resources[preferred].url.substr((*entries)[0].resources[preferred].url.rfind('/') + 1);
					pic.Name = tmp.substr(0, tmp.rfind('.'));
					pic.Type = tmp.substr(tmp.rfind('.') + 1);
						
					if(stat(pic.Filename.c_str(), &statbuf) != 0)
						printf("stat error");
					pic.Date = statbuf.st_mtime;
										
					tmpPictureViewerGui.addToPlaylist(pic);
					tmpPictureViewerGui.exec(NULL, "urlplayback");
					
					return;
				}
			}
		} 
		else 
		{
			neutrino_msg_t      msg;
			neutrino_msg_data_t data;
			g_RCInput->getMsg(&msg, &data, 10); // 1 sec timeout to update play/stop state display

			if( msg == CRCInput::RC_home)
			{
				m_folderplay = false;
				break;
			}
		}
	}
	
	//
	m_frameBuffer->ClearFrameBuffer();
	m_frameBuffer->blit();
}

bool CUpnpBrowserGui::selectItem(std::string id)
{
	bool loop = true;
	bool endall = false;
	bool changed = true;
	bool rchanged = true;
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;
	std::vector<UPnPEntry> *entries;
	unsigned int index, selected, dirnum;

	index = 0;
	selected = 0;
	dirnum = 0;
	entries = NULL;

	while (loop)
	{
		updateTimes();		
		
		if (rchanged)
		{
			if (entries)
				delete entries;
			
			entries = NULL;

			std::list<UPnPAttribute>attribs;
			std::list<UPnPAttribute>results;
			std::list<UPnPAttribute>::iterator i;
			std::stringstream sindex;
			std::stringstream scount;
			unsigned int returned = 0;

			bool rfound = false;
			bool nfound = false;
			bool tfound = false;

			sindex << index;
			scount << m_listmaxshow;

			attribs.push_back(UPnPAttribute("ObjectID", id));
			attribs.push_back(UPnPAttribute("BrowseFlag", "BrowseDirectChildren"));
			attribs.push_back(UPnPAttribute("Filter", "*"));
			attribs.push_back(UPnPAttribute("StartingIndex", sindex.str()));
			attribs.push_back(UPnPAttribute("RequestedCount", scount.str()));
			attribs.push_back(UPnPAttribute("SortCriteria", ""));

			results = m_devices[m_selecteddevice].SendSOAP("urn:schemas-upnp-org:service:ContentDirectory:1", "Browse", attribs);
			for (i = results.begin(); i != results.end(); i++)
			{
				if (i->first == "NumberReturned")
				{
					returned = atoi(i->second.c_str());
					nfound = true;
				}
				if (i->first == "TotalMatches")
				{
					dirnum = atoi(i->second.c_str());
					tfound = true;
				}
				if (i->first == "Result")
				{
					entries = decodeResult(i->second);
					rfound = true;
				}
			}
			if (!entries)
				return endall;
			if (!nfound || !tfound || !rfound)
			{
				delete entries;
				return endall;
			}
			if (returned != entries->size())
			{
				delete entries;
				return endall;
			}

			if (returned == 0)
			{
				delete entries;
				return endall;
			}
			rchanged=false;
			changed=true;
		}

		if (changed)
		{
			paintItem(entries, selected - index, dirnum - index, index);
			changed = false;
		}
		
		m_frameBuffer->blit();

		g_RCInput->getMsg(&msg, &data, 10); // 1 sec timeout to update play/stop state display
		neutrino_msg_t msg_repeatok = msg & ~CRCInput::RC_Repeat;

		if( msg == CRCInput::RC_timeout)
		{
			// nothing
		}
		else if(msg == CRCInput::RC_home)
		{
			loop = false;
			endall = true;
		}
		else if(msg == CRCInput::RC_left)
		{
			loop = false;
		}

		else if (msg_repeatok == CRCInput::RC_up && selected > 0)
		{
			selected--;
			if (selected < index)
			{
				index -= m_listmaxshow;
				rchanged = true;
			}
			changed = true;
		}

		else if( (msg == CRCInput::RC_yellow || (int) msg == g_settings.key_channelList_pageup) && selected > 0)
		{
			if (index > 0)
			{
				index-=m_listmaxshow;
				selected-=m_listmaxshow;
				rchanged=true;
			}
			else
				selected=0;
			changed = true;
		}
		else if (msg_repeatok == CRCInput::RC_down && selected + 1 < dirnum)
		{
			selected++;
			if (selected + 1 > index + m_listmaxshow)
			{
				index += m_listmaxshow;
				rchanged = true;
			}
			changed=true;
		}
		else if( (msg == CRCInput::RC_green || (int) msg == g_settings.key_channelList_pagedown)&& selected + 1 < dirnum)
		{
			if (index < ((dirnum - 1) / m_listmaxshow) * m_listmaxshow)
			{
				index += m_listmaxshow;
				selected += m_listmaxshow;
				if (selected + 1 >= dirnum)
					selected = dirnum - 1;
				rchanged = true;
			}
			else
				selected = dirnum - 1;
			changed = true;
		}
		else if(msg == CRCInput::RC_right)
		{
			if ((*entries)[selected - index].isdir)
			{
				endall = selectItem((*entries)[selected - index].id);
				if (endall)
					loop = false;
			}
			changed = true;
		}
		else if(msg == CRCInput::RC_ok)
		{
			if (!(*entries)[selected - index].isdir)
			{
				m_folderplay = false;
				
				int preferred = (*entries)[selected - index].preferred;
				if (preferred != -1)
				{
					std::string protocol, prot, network, mime, additional;
					protocol=(*entries)[selected - index].resources[preferred].protocol;
					splitProtocol(protocol, prot, network, mime, additional);
					
					if (mime == "audio/mpeg")
					{
						CAudioPlayerGui tmpAudioPlayerGui;
			
						CAudiofileExt audiofile((*entries)[selected - index].resources[preferred].url, CFile::EXTENSION_MP3);
						tmpAudioPlayerGui.addToPlaylist(audiofile);
						tmpAudioPlayerGui.exec(NULL, "urlplayback");
					}
					else if ((mime == "image/gif") || (mime == "image/jpeg"))
					{
						CPictureViewerGui tmpPictureViewerGui;
						CPicture pic;
						struct stat statbuf;
						
						pic.Filename = (*entries)[selected - index].resources[preferred].url;
						std::string tmp = (*entries)[selected - index].resources[preferred].url.substr((*entries)[selected - index].resources[preferred].url.rfind('/') + 1);
						pic.Name = tmp.substr(0, tmp.rfind('.'));
						pic.Type = tmp.substr(tmp.rfind('.') + 1);
						
						if(stat(pic.Filename.c_str(), &statbuf) != 0)
							printf("stat error");
						pic.Date = statbuf.st_mtime;
										
						tmpPictureViewerGui.addToPlaylist(pic);
						tmpPictureViewerGui.exec(NULL, "urlplayback");

						changed = true;
					}

					else if (mime.substr(0,6) == "video/")
					{
						m_frameBuffer->ClearFrameBuffer();
						m_frameBuffer->blit();	
						
						CFile file;
						file.Name = (*entries)[selected - index].resources[preferred].url.c_str(); //FIXME
						
						CMoviePlayerGui tmpMoviePlayerGui;
						tmpMoviePlayerGui.addToPlaylist(file);
						tmpMoviePlayerGui.exec(NULL, "urlplayback");
						
						changed = true;
					}
					
					m_playing_entry = (*entries)[selected - index];
				}

			} 
			else 
			{
				m_folderplay = true;
				m_playfolder = (*entries)[selected - index].id;
				m_playid = 0;
				playnext();
			}
			
			changed = true;
		}
		else if( msg == CRCInput::RC_red)
		{
			if(CAudioPlayer::getInstance()->getState() != CBaseDec::STOP)
				CAudioPlayer::getInstance()->stop();
			m_folderplay = false;
		}
		else if(msg == NeutrinoMessages::RECORD_START ||
			msg == NeutrinoMessages::ZAPTO ||
			msg == NeutrinoMessages::STANDBY_ON ||
			msg == NeutrinoMessages::SHUTDOWN ||
			msg == NeutrinoMessages::SLEEPTIMER)
		{
			loop = false;
			g_RCInput->postMsg(msg, data);
		}

		else if(msg == NeutrinoMessages::EVT_TIMER)
		{
			CNeutrinoApp::getInstance()->handleMsg( msg, data );
		}
		else
		{
			if( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
				loop = false;
			changed = true;
		}

		if (m_folderplay && (CAudioPlayer::getInstance()->getState() == CBaseDec::STOP))
			playnext();
								
		m_frameBuffer->blit();	
	}
	
	if (entries)
		delete entries;
	
	m_frameBuffer->ClearFrameBuffer();
	m_frameBuffer->blit();
	
	return endall;
}

void CUpnpBrowserGui::paintDevicePos(unsigned int pos)
{
	int ypos = m_y + m_title_height + m_theight + pos*m_fheight;
	uint8_t    color;
	fb_pixel_t bgcolor;

	if (pos == m_selecteddevice)
	{
		color   = COL_MENUCONTENT + 2;
		bgcolor = COL_MENUCONTENT_PLUS_2;
	}
	else
	{
		color   = COL_MENUCONTENT;
		bgcolor = COL_MENUCONTENT_PLUS_0;
	}
	m_frameBuffer->paintBoxRel(m_x, ypos, m_width - 15, m_fheight, bgcolor);

	if (pos + m_indexdevice >= m_devices.size())
		return;

	char sNr[20];
	sprintf(sNr, "%2d", pos + 1);
	std::string num = sNr;

	std::string name = m_devices[pos + m_indexdevice].friendlyname;

	int w = g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getRenderWidth(name) + 5;
	g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(m_x + 10, ypos + m_fheight, m_width - 30 - w, num, color, m_fheight, true); // UTF-8
	g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(m_x + m_width - 15 - w, ypos + m_fheight, w, name, color, m_fheight, true); // UTF-8
}

void CUpnpBrowserGui::paintItemPos(std::vector<UPnPEntry> *entry, unsigned int pos, unsigned int selected)
{
	int ypos = m_y + m_title_height + m_theight + pos*m_fheight;
	uint8_t    color;
	fb_pixel_t bgcolor;

	if (pos == selected)
	{
		color   = COL_MENUCONTENT + 2;
		bgcolor = COL_MENUCONTENT_PLUS_2;

		if ((*entry)[pos].isdir)
			paintItem2DetailsLine(-1); // clear it
		else
			paintItem2DetailsLine(pos);

		paintDetails(entry, pos);
	}
	else
	{
		color   = COL_MENUCONTENT;
		bgcolor = COL_MENUCONTENT_PLUS_0;
	}
	
	m_frameBuffer->paintBoxRel(m_x, ypos, m_width - SCROLLBAR_WIDTH, m_fheight, bgcolor);

	if (pos >= entry->size())
		return;

	int preferred = (*entry)[pos].preferred;
	std::string info;
	std::string fileicon;
	
	if ((*entry)[pos].isdir)
	{
		info = "<DIR>";
		fileicon = NEUTRINO_ICON_FOLDER;
	}
	else
	{
		//FIXME: revise this
		if (preferred != -1)
		{
			info = (*entry)[pos].resources[preferred].duration;
			
			// icon
			std::string protocol, prot, network, mime, additional;
			protocol = (*entry)[pos].resources[preferred].protocol;
			splitProtocol(protocol, prot, network, mime, additional);
					
			if (mime.substr(0, 6)  == "audio/")
			{
				fileicon = NEUTRINO_ICON_MP3;
			}
			else if (mime.substr(0, 6) == "image/")
			{
				fileicon = NEUTRINO_ICON_PICTURE;
			}
			else if (mime.substr(0, 6) == "video/")
			{
				fileicon = NEUTRINO_ICON_MOVIE;
			}
		}
		else
		{
			info = "(none)";
			fileicon = NEUTRINO_ICON_FILE;
		}
	}

	std::string name = (*entry)[pos].title;
	char tmp_time[] = "00:00:00.0";
	int w = g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getRenderWidth(tmp_time);

	m_frameBuffer->paintIcon(fileicon, m_x + 5 , ypos + (m_fheight - 16) / 2);
	g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(m_x + m_width - 15 - w, ypos + m_fheight, w, info, color, m_fheight);

	g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(m_x + 30, ypos + m_fheight, m_width - 50 - w, name, color, m_fheight, true); // UTF-8
}

void CUpnpBrowserGui::paintDevice()
{
	std::string tmp;
	int w, xstart, ypos, top;

	// LCD
	CVFD::getInstance()->setMode(CVFD::MODE_MENU_UTF8, "Select UPnP Device");
	CVFD::getInstance()->showMenuText(0, m_devices[m_selecteddevice].friendlyname.c_str(), -1, true);

	// Info
	m_frameBuffer->paintBoxRel(m_x, m_y, m_width, m_title_height - 10, COL_MENUCONTENT_PLUS_6 );
	m_frameBuffer->paintBoxRel(m_x + 2, m_y + 2, m_width - 4, m_title_height - 14, COL_MENUCONTENTSELECTED_PLUS_0 );

	// first line
	tmp = m_devices[m_selecteddevice].manufacturer + " " + m_devices[m_selecteddevice].manufacturerurl;
	w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp, true); // UTF-8
	xstart = (m_width - w) / 2;
	if(xstart < 10)
		xstart = 10;
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + xstart, m_y + 4 + 1*m_mheight, m_width - 20, tmp, COL_MENUCONTENTSELECTED, 0, true); // UTF-8

	// second line
	tmp = m_devices[m_selecteddevice].modelname + " " +
		m_devices[m_selecteddevice].modelnumber + " " +
		m_devices[m_selecteddevice].modeldescription;
	w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp, true); // UTF-8
	xstart = (m_width - w) / 2;
	if(xstart < 10)
		xstart = 10;
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + xstart, m_y + 4 + 2*m_mheight, m_width - 20, tmp, COL_MENUCONTENTSELECTED, 0, true); // UTF-8
	// third line
	tmp = m_devices[m_selecteddevice].modelurl;
	w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp, true); // UTF-8
	xstart = (m_width - w) / 2;
	if(xstart < 10)
		xstart = 10;
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + xstart, m_y + 4 + 3*m_mheight, m_width - 20, tmp, COL_MENUCONTENTSELECTED, 0, true); // UTF-8

	// Head
	tmp = g_Locale->getText(LOCALE_UPNPBROWSER_HEAD);
	m_frameBuffer->paintBoxRel(m_x, m_y + m_title_height, m_width, m_theight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, true, gradientLight2Dark);

	m_frameBuffer->paintIcon(NEUTRINO_ICON_UPNP, m_x + BORDER_LEFT, m_y + m_title_height + (m_theight -icon_head_h)/2);

	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(m_x + BORDER_LEFT + icon_head_w + 5, m_y + m_theight + m_title_height + 0, m_width - 45, tmp, COL_MENUHEAD, 0, true); // UTF-8
	ypos = m_y + m_title_height;
	if(m_theight > icon_head_h)
		ypos = (m_theight - icon_head_h) / 2 + m_y + m_title_height;

	// Items
	for (unsigned int count = 0; count < m_listmaxshow; count++)
		paintDevicePos(count);

	ypos = m_y + m_title_height + m_theight;
	int sb = m_fheight * m_listmaxshow;
	m_frameBuffer->paintBoxRel(m_x + m_width - SCROLLBAR_WIDTH, ypos, SCROLLBAR_WIDTH, sb, COL_MENUCONTENT_PLUS_1);

	int sbc = ((m_devices.size() - 1) / m_listmaxshow) + 1;
	int sbs = ((m_selecteddevice) / m_listmaxshow);

	m_frameBuffer->paintBoxRel(m_x + m_width - 13, ypos + 2 + sbs*(sb-4)/sbc, 11, (sb-4)/sbc, COL_MENUCONTENT_PLUS_3);

	// Foot
	top = m_y + m_height - (m_info_height + m_buttonHeight);

	int ButtonWidth = (m_width - BORDER_LEFT - BORDER_RIGHT) / 4;
	m_frameBuffer->paintBoxRel(m_x, top, m_width, m_buttonHeight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_BOTTOM, true, gradientDark2Light);

	//m_frameBuffer->paintHLine(m_x, m_x + m_width, top, COL_INFOBAR_SHADOW_PLUS_0);
	::paintButtons(m_frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, m_x + BORDER_LEFT + 3*ButtonWidth, top, ButtonWidth, 1, &RescanButton, m_buttonHeight);

	//clearItem2DetailsLine(); // clear it
}

void CUpnpBrowserGui::paintItem(std::vector<UPnPEntry> *entry, unsigned int selected, unsigned int max, unsigned int offset)
{
	std::string tmp;
	std::stringstream ts;
	int w, xstart, ypos, top;
	int preferred=(*entry)[selected].preferred;

	// LCD
	CVFD::getInstance()->setMode(CVFD::MODE_MENU_UTF8, "Select UPnP Entry");
	CVFD::getInstance()->showMenuText(0, (*entry)[selected].title.c_str(), -1, true);

	// Info
	m_frameBuffer->paintBoxRel(m_x, m_y, m_width, m_title_height - 10, COL_MENUCONTENT_PLUS_6);
	m_frameBuffer->paintBoxRel(m_x + 2, m_y + 2, m_width - 4, m_title_height - 14, COL_MENUCONTENTSELECTED_PLUS_0);

	// first line
	ts << "Resources: " << (*entry)[selected].resources.size() << " Selected: " << preferred+1 << " ";
	tmp = ts.str();

	if (preferred != -1)
		tmp = tmp + "Duration: " + (*entry)[selected].resources[preferred].duration;
	else
		tmp = tmp + "No resource for Item";
	w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp, true); // UTF-8
	if (w > m_width - 20)
		w = m_width - 20;
	xstart = (m_width - w) / 2;
	if(xstart < 10)
		xstart = 10;
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + xstart, m_y + 4 + 1*m_mheight, m_width - 20, tmp, COL_MENUCONTENTSELECTED, 0, true); // UTF-8

	// second line
	if ((*entry)[selected].isdir)
		tmp = "Directory";
	else
	{
		tmp = "";
		if (preferred != -1)
		{
			std::string proto, network, mime, info;
			splitProtocol((*entry)[selected].resources[preferred].protocol, proto, network, mime, info);
			tmp = "Protocol: " + proto + ", MIME-Type: " + mime;
		}

	}
	w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp, true); // UTF-8
	if (w > m_width - 20)
		w = m_width - 20;
	xstart = (m_width - w) / 2;
	if(xstart < 10)
		xstart = 10;
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + xstart, m_y + 4 + 2*m_mheight, m_width - 20, tmp, COL_MENUCONTENTSELECTED, 0, true); // UTF-8

	//third line
	tmp = "";
	if (!(*entry)[selected].isdir)
	{
		if (preferred != -1)
		{
			tmp = "URL: " + (*entry)[selected].resources[preferred].url;
		}

	}
	w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp, true); // UTF-8
	if (w > m_width - 20)
		w = m_width - 20;
	xstart = (m_width - w) / 2;
	if(xstart < 10)
		xstart = 10;
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + xstart, m_y + 4 + 3*m_mheight, m_width - 20, tmp, COL_MENUCONTENTSELECTED, 0, true); // UTF-8


	// Head
	tmp = g_Locale->getText(LOCALE_UPNPBROWSER_HEAD);
	m_frameBuffer->paintBoxRel(m_x, m_y + m_title_height, m_width, m_theight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, true);
	m_frameBuffer->paintIcon(NEUTRINO_ICON_UPNP, m_x + BORDER_LEFT, m_y + m_title_height + (m_theight - icon_head_h)/2);
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(m_x + BORDER_LEFT + icon_head_w + 5, m_y + m_theight + m_title_height, m_width - 45, tmp, COL_MENUHEAD, 0, true); // UTF-8
	
	//
	ypos = m_y + m_title_height;
	if(m_theight > icon_head_h)
		ypos = (m_theight - icon_head_h) / 2 + m_y + m_title_height;

	// Items
	for (unsigned int count = 0; count < m_listmaxshow; count++)
		paintItemPos(entry, count, selected);

	ypos = m_y + m_title_height + m_theight;
	int sb = m_fheight * m_listmaxshow;
	m_frameBuffer->paintBoxRel(m_x + m_width - SCROLLBAR_WIDTH, ypos, SCROLLBAR_WIDTH, sb, COL_MENUCONTENT_PLUS_1);

	int sbc = ((max + offset - 1) / m_listmaxshow) + 1;
	int sbs = ((selected + offset) / m_listmaxshow);

	int sbh = 0;
	if ((sbc > 0) && (sbc > sb - 4))
		sbh = 2;
	m_frameBuffer->paintBoxRel(m_x + m_width - 13, ypos + 2 + sbs*((sb - 4)/sbc + sbh), 11, (sb - 4)/sbc + sbh, COL_MENUCONTENT_PLUS_3);

	// Foot buttons
	top = m_y + m_height - (m_info_height + m_buttonHeight);
	int ButtonWidth = (m_width - 20) / 4;
	m_frameBuffer->paintBoxRel(m_x, top, m_width, m_buttonHeight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_BOTTOM, true, gradientDark2Light);
	
	::paintButtons(m_frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, m_x + BORDER_LEFT, top, ButtonWidth, 1, &StopButton, m_buttonHeight);
	::paintButtons(m_frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, m_x + BORDER_LEFT + ButtonWidth, top, ButtonWidth, 1, &PUpButton, m_buttonHeight);
	::paintButtons(m_frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, m_x + BORDER_LEFT + 2*ButtonWidth, top, ButtonWidth, 1, &PDownButton, m_buttonHeight);

	m_frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_OKAY, m_x + BORDER_LEFT + 3*ButtonWidth, top + (m_buttonHeight - icon_foot_h)/2);
	g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(m_x + BORDER_LEFT + 3*ButtonWidth + icon_foot_w + 5, top + (m_buttonHeight - g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight(), ButtonWidth - 40, g_Locale->getText(LOCALE_AUDIOPLAYER_PLAY), COL_INFOBAR, 0, true); // UTF-8
}

void CUpnpBrowserGui::paintDetails(std::vector<UPnPEntry> *entry, unsigned int index, bool use_playing)
{
	// Foot info
	int top = m_y + m_height - m_info_height;

	if ((!use_playing) && ((*entry)[index].isdir))
	{
		m_frameBuffer->paintBackgroundBoxRel(m_x + 2, top, m_width - 4, m_info_height - 4);
	}
	else
	{
		if (use_playing) 
		{
			if (!m_playing_entry_is_shown) 
			{
				// refreshbox
				m_frameBuffer->paintBoxRel(m_x + 2, top + 2, m_width - 4, m_info_height - 4, COL_MENUCONTENTDARK_PLUS_0, 0, 0, true, gradientLight2Dark);

				m_playing_entry_is_shown = true;

				g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(m_x + 5, top + 2 + + (m_info_height/2 - g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getHeight(), m_x + m_width - 10, m_playing_entry.title + " - " + m_playing_entry.artist, COL_INFOBAR, 0, true); // UTF-8

				g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(m_x + 5, top + 2 + m_info_height/2 + (m_info_height/2 - g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getHeight(), m_x + m_width - 10, m_playing_entry.album, COL_INFOBAR, 0, true); // UTF-8
			}
		} 
		else 
		{
			if (entry == NULL) 
				return;
			
			// refreshbox
			m_frameBuffer->paintBoxRel(m_x + 2, top + 2, m_width - 4, m_info_height - 4, COL_MENUCONTENTDARK_PLUS_0, true, 0, 0, gradientLight2Dark);
			m_playing_entry_is_shown = false;
			g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(m_x + 5, top + 2 + (m_info_height/2 - g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getHeight(), m_x + m_width - 10, (*entry)[index].title + " - " + (*entry)[index].artist, COL_INFOBAR, 0, true); // UTF-8
			g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(m_x + 5, top + 2 + m_info_height/2 + (m_info_height/2 - g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getHeight(), m_x + m_width - 10, (*entry)[index].album, COL_INFOBAR, 0, true); // UTF-8
		}
	}
}

void CUpnpBrowserGui::clearItem2DetailsLine()
{
	::clearItem2DetailsLine(m_x, m_y, m_height - m_info_height, m_info_height); 
}

void CUpnpBrowserGui::paintItem2DetailsLine(int pos)
{
	::paintItem2DetailsLine(m_x, m_y, m_width, m_height - m_info_height, m_info_height, m_title_height + m_theight, m_fheight, pos);
}

void CUpnpBrowserGui::updateTimes(const bool force)
{
	int top;
	
	if(CAudioPlayer::getInstance()->getState() != CBaseDec::STOP)
	{
		bool updatePlayed = force;

		if ((m_time_played != CAudioPlayer::getInstance()->getTimePlayed()))
		{
			m_time_played = CAudioPlayer::getInstance()->getTimePlayed();
			updatePlayed = true;
		}
		
		char play_time[8];
		snprintf(play_time, 7, "%ld:%02ld", m_time_played / 60, m_time_played % 60);
		char tmp_time[] = "000:00";
		int w = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getRenderWidth(tmp_time);

		if (updatePlayed)
		{
			paintDetails(NULL, 0, true);
			top = m_y + m_height - m_info_height;
			m_frameBuffer->paintBoxRel(m_x + m_width - w - 15, top + 2, w + 4, m_info_height - 4, COL_MENUCONTENTDARK_PLUS_0, true, 0, 0, gradientLight2Dark);

			g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(m_x + m_width - w - 11, top + (m_info_height - g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight(), w, play_time, COL_MENUHEAD);			
		}
	}
}

