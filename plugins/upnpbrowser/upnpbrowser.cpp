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
#include <gui/audioplayer.h>

#include <upnpbrowser.h>


extern "C" void plugin_exec(void);
extern "C" void plugin_init(void);
extern "C" void plugin_del(void);


const struct button_label RescanButton = {NEUTRINO_ICON_BUTTON_BLUE  , LOCALE_UPNPBROWSER_RESCAN, NULL };
const struct button_label PlayButton   = {NEUTRINO_ICON_BUTTON_RED, LOCALE_AUDIOPLAYER_PLAY, NULL };
const struct button_label PUpButton    = {NEUTRINO_ICON_BUTTON_GREEN, LOCALE_FILEBROWSER_NEXTPAGE, NULL };
const struct button_label PDownButton  = {NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_FILEBROWSER_PREVPAGE, NULL };

CUpnpBrowserGui::CUpnpBrowserGui()
{
	dprintf(DEBUG_NORMAL, "CUpnpBrowserGui::CUpnpBrowserGui:\n");

	m_socket = new CUPnPSocket();
	m_frameBuffer = CFrameBuffer::getInstance();
	m_playing_entry_is_shown = false;

	thumbnail_dir = "/tmp/upnpbrowser";
}

CUpnpBrowserGui::~CUpnpBrowserGui()
{
	dprintf(DEBUG_NORMAL, "CUpnpBrowserGui::~CUpnpBrowserGui:\n");

	if(m_socket)
		delete m_socket;

	fileHelper.removeDir(thumbnail_dir.c_str());
}

int CUpnpBrowserGui::exec(CMenuTarget* parent, const std::string & /*actionKey*/)
{
	dprintf(DEBUG_NORMAL, "CUpnpBrowserGui::exec:\n");

	if(parent)
		parent->hide();

	m_width = (g_settings.screen_EndX - g_settings.screen_StartX) - ConnectLineBox_Width;
	m_height = (g_settings.screen_EndY - g_settings.screen_StartY);

	//
	m_sheight = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight();

	// foot height
	m_frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_OKAY, &icon_foot_w, &icon_foot_h);
	m_buttonHeight = std::max(icon_foot_h, g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight()) + 6;

	// title height
	m_frameBuffer->getIconSize(NEUTRINO_ICON_UPNP, &icon_head_w, &icon_head_h);
	m_theight = std::max(icon_head_h, g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight()) + 6; 

	//
	m_mheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();

	// item height		
	m_fheight = g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getHeight();

	// head title info height
	m_title_height = 4 + m_mheight + 2 + m_mheight + 2 + m_mheight + 4;

	// foot info height
	m_info_height = 4 + m_mheight + 2 + m_mheight + 4;

	// recalculate height						//info
	m_listmaxshow = (m_height - m_title_height - m_theight - m_buttonHeight - m_info_height)/m_fheight;
	m_height = m_title_height + m_theight + m_listmaxshow*m_fheight + m_buttonHeight + m_info_height;

	m_x = (((g_settings.screen_EndX - g_settings.screen_StartX) - (m_width + ConnectLineBox_Width)) / 2) + g_settings.screen_StartX + ConnectLineBox_Width;
	m_y = (((g_settings.screen_EndY- g_settings.screen_StartY) - m_height)/ 2) + g_settings.screen_StartY;

	m_indexdevice = 0;
	m_selecteddevice = 0;

	selectDevice();

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
			isdir = true;
			for (snode = node->GetChild(); snode; snode = snode->GetNext())
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

	fileHelper.createDir(thumbnail_dir.c_str(), 0755);	

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
			selectItem("0");
			changed = true;
		}
		else if( msg == CRCInput::RC_blue)
		{
			hide();

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

	fileHelper.removeDir(thumbnail_dir.c_str());
	
	// hide
	hide();
}

void CUpnpBrowserGui::handleFolder(void)
{
	printf("CUpnpBrowserGui::handleFolder\n");

	CAudioPlayerGui tmpAudioPlayerGui;
	CMoviePlayerGui tmpMoviePlayerGui;
	CPictureViewerGui tmpPictureViewerGui;
	
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
	attribs.push_back(UPnPAttribute("RequestedCount", "100"));
	attribs.push_back(UPnPAttribute("SortCriteria", ""));

	results = m_devices[m_selecteddevice].SendSOAP("urn:schemas-upnp-org:service:ContentDirectory:1", "Browse", attribs);

	for (i = results.begin(); i != results.end(); i++)
	{
		if (i->first == "NumberReturned")
		{
			nfound = true;
		}

		if (i->first == "TotalMatches")
		{
			tfound = true;
		}

		if (i->first == "Result")
		{
			entries = decodeResult(i->second);
			rfound = true;
		}
	}

	//
	bool audioFolder = false;
	bool videoFolder = false;
	bool picFolder = false;

	for (unsigned int i = 0; i < entries->size(); i++)
	{
		
		if (!(*entries)[i].isdir)
		{
			int preferred = (*entries)[i].preferred;
			if (preferred != -1)
			{
				std::string protocol, prot, network, mime, additional;
				protocol = (*entries)[i].resources[preferred].protocol;
				splitProtocol(protocol, prot, network, mime, additional);
				
				if (mime == "audio/mpeg")
				{
					CAudiofileExt audiofile((*entries)[i].resources[preferred].url, CFile::EXTENSION_MP3);
					tmpAudioPlayerGui.addToPlaylist(audiofile);
				
					audioFolder = true;
				}
				else if (mime.substr(0, 6) == "video/")
				{
					MI_MOVIE_INFO mfile;
					
					mfile.file.Name = (*entries)[i].resources[preferred].url.c_str();
					mfile.epgTitle = (*entries)[i].title;
					mfile.ytid = "upnp";  
					
					tmpMoviePlayerGui.addToPlaylist(mfile);
					
					videoFolder = true;
				}
				else if (mime.substr(0, 6) == "image/")
				{
					std::string fname;

					//DownloadImage
					if(!((*entries)[i].resources[preferred].url).empty())
					{
						fname = thumbnail_dir;

						fname += (*entries)[i].resources[preferred].url.substr((*entries)[i].resources[preferred].url.find_last_of("/"));

						int ext_pos = 0;
						ext_pos = fname.rfind('?');
	
						if( ext_pos > 0)
						{
							std::string extension;
							extension = fname.substr(ext_pos + 1, fname.length() - ext_pos);

							fname = fname.substr(0, fname.length() - (extension.length() + 1));
						}

						::downloadUrl((*entries)[i].resources[preferred].url, fname);
					}
					
					//
					CPicture pic;
					struct stat statbuf;
				
					pic.Filename = fname;
					std::string tmp = fname.substr(fname.rfind('/') + 1);
					pic.Name = tmp.substr(0, tmp.rfind('.'));
					pic.Type = tmp.substr(tmp.rfind('.') + 1);
			
					if(stat(pic.Filename.c_str(), &statbuf) != 0)
						printf("stat error");
					pic.Date = statbuf.st_mtime;
				
					tmpPictureViewerGui.addToPlaylist(pic);
			
					picFolder = true;
				}
			}
		}
	}

	if(audioFolder)
	{
		//tmpAudioPlayerGui.hidePlayList(true);
		tmpAudioPlayerGui.exec(this, "urlplayback");
	}
	else if(videoFolder)
	{
		tmpMoviePlayerGui.exec(this, "urlplayback");
	}
	else if(picFolder)
	{
		//tmpPictureViewerGui.setState(CPictureViewerGui::SLIDESHOW);
		tmpPictureViewerGui.exec(this, "urlplayback");
	}
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
			rchanged = false;
			changed = true;
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

		else if( (msg == CRCInput::RC_yellow || (int) msg == CRCInput::RC_page_up) && selected > 0)
		{
			if (index > 0)
			{
				index -= m_listmaxshow;
				selected -= m_listmaxshow;
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
			changed = true;
		}
		else if( (msg == CRCInput::RC_green || (int) msg == CRCInput::RC_page_down)&& selected + 1 < dirnum)
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
		else if(msg == CRCInput::RC_red || msg == CRCInput::RC_ok)
		{
			if (!(*entries)[selected - index].isdir)
			{
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
						//tmpAudioPlayerGui.hidePlayList(true);
						tmpAudioPlayerGui.exec(this, "urlplayback");
					}
					else if ((mime == "image/gif") || (mime == "image/jpeg"))
					{
						CPictureViewerGui tmpPictureViewerGui;

						std::string fname;

						//DownloadImage
						if(!((*entries)[selected - index].resources[preferred].url).empty())
						{
							fname = thumbnail_dir;

							fname += (*entries)[selected - index].resources[preferred].url.substr((*entries)[selected - index].resources[preferred].url.find_last_of("/"));

							int ext_pos = 0;
							ext_pos = fname.rfind('?');
	
							if( ext_pos > 0)
							{
								std::string extension;
								extension = fname.substr(ext_pos + 1, fname.length() - ext_pos);

								fname = fname.substr(0, fname.length() - (extension.length() + 1));
							}

							::downloadUrl((*entries)[selected - index].resources[preferred].url, fname);
						}
					
						//
						CPicture pic;
						struct stat statbuf;
				
						pic.Filename = fname;
						std::string tmp = fname.substr(fname.rfind('/') + 1);
						pic.Name = tmp.substr(0, tmp.rfind('.'));
						pic.Type = tmp.substr(tmp.rfind('.') + 1);
			
						if(stat(pic.Filename.c_str(), &statbuf) != 0)
							printf("stat error");
						pic.Date = statbuf.st_mtime;
				
						tmpPictureViewerGui.addToPlaylist(pic);
						tmpPictureViewerGui.exec(this, "urlplayback");

						changed = true;
					}

					else if (mime.substr(0,6) == "video/")
					{
						MI_MOVIE_INFO mfile;
						mfile.file.Name = (*entries)[selected - index].resources[preferred].url.c_str(); 
						mfile.epgTitle = (*entries)[selected - index].title;
						mfile.ytid = "upnp"; 
						
						CMoviePlayerGui tmpMoviePlayerGui;
						tmpMoviePlayerGui.addToPlaylist(mfile);
						tmpMoviePlayerGui.exec(this, "urlplayback");
						
						changed = true;
					}
					
					m_playing_entry = (*entries)[selected - index];
				}

			} 
			else 
			{
				m_playfolder = (*entries)[selected - index].id;
				m_playid = 0;
				handleFolder();
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
	
	if (entries)
		delete entries;
	
	hide();
	
	return endall;
}

void CUpnpBrowserGui::hide()
{
	CFrameBuffer::getInstance()->ClearFrameBuffer();
	CFrameBuffer::getInstance()->blit();
}

void CUpnpBrowserGui::paintDevicePos(unsigned int pos)
{
	int ypos = m_y + m_title_height + m_theight + pos*m_fheight;
	uint8_t    color;
	fb_pixel_t bgcolor;

	if (pos == m_selecteddevice)
	{
		color = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
	}
	else
	{
		color   = COL_MENUCONTENT;
		bgcolor = COL_MENUCONTENT_PLUS_0;
	}

	// item box
	m_frameBuffer->paintBoxRel(m_x, ypos, m_width - SCROLLBAR_WIDTH, m_fheight, bgcolor);

	if (pos + m_indexdevice >= m_devices.size())
		return;

	char sNr[20];
	sprintf(sNr, "%2d", pos + 1);
	std::string num = sNr;

	std::string name = m_devices[pos + m_indexdevice].friendlyname;

	int w = g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getRenderWidth(name) + 5;
	g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(m_x + BORDER_LEFT, ypos + m_fheight, m_width - 30 - w, num, color, m_fheight, true); // UTF-8
	g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(m_x + m_width - SCROLLBAR_WIDTH - w, ypos + m_fheight, w, name, color, m_fheight, true); // UTF-8
}

void CUpnpBrowserGui::paintItemPos(std::vector<UPnPEntry> *entry, unsigned int pos, unsigned int selected)
{
	int ypos = m_y + m_title_height + m_theight + pos*m_fheight;
	uint8_t    color;
	fb_pixel_t bgcolor;

	if (pos == selected)
	{
		color = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;

		if ((*entry)[pos].isdir)
			clearItem2DetailsLine(); // clear it
		else
			paintItem2DetailsLine(pos);

		paintDetails(entry, pos);
	}
	else
	{
		color   = COL_MENUCONTENT;
		bgcolor = COL_MENUCONTENT_PLUS_0;
	}
	
	// item box
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
	m_frameBuffer->paintBoxRel(m_x, m_y, m_width, m_title_height, COL_MENUCONTENT_PLUS_6 );
	m_frameBuffer->paintBoxRel(m_x + 2, m_y + 2, m_width - 4, m_title_height - 4, COL_MENUHEAD_INFO_PLUS_0, NO_RADIUS, CORNER_NONE, g_settings.Head_Info_gradient);

	// first line
	tmp = m_devices[m_selecteddevice].manufacturer + " " + m_devices[m_selecteddevice].manufacturerurl;
	w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp, true); // UTF-8
	xstart = (m_width - w) / 2;
	if(xstart < BORDER_LEFT)
		xstart = BORDER_LEFT;

	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + xstart, m_y + 4 + m_mheight, m_width - BORDER_LEFT - BORDER_RIGHT, tmp, COL_MENUHEAD_INFO, 0, true); // UTF-8

	// second line
	tmp = m_devices[m_selecteddevice].modelname + " " +
		m_devices[m_selecteddevice].modelnumber + " " +
		m_devices[m_selecteddevice].modeldescription;

	w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp, true); // UTF-8
	xstart = (m_width - w) / 2;
	if(xstart < BORDER_LEFT)
		xstart = BORDER_LEFT;
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + xstart, m_y + 4 + m_mheight + 2 + m_mheight, m_width - BORDER_LEFT - BORDER_RIGHT, tmp, COL_MENUHEAD_INFO, 0, true); // UTF-8

	// third line
	tmp = m_devices[m_selecteddevice].modelurl;
	w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp, true); // UTF-8
	xstart = (m_width - w) / 2;
	if(xstart < BORDER_LEFT)
		xstart = BORDER_LEFT;
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + xstart, m_y + 4 + m_mheight + 2 + m_mheight + 2 + m_mheight, m_width - BORDER_LEFT - BORDER_RIGHT, tmp, COL_MENUHEAD_INFO, 0, true); // UTF-8

	// Head
	tmp = g_Locale->getText(LOCALE_UPNPBROWSER_HEAD);
	m_frameBuffer->paintBoxRel(m_x, m_y + m_title_height, m_width, m_theight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, g_settings.Head_gradient);

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
	top = m_y + m_height - m_info_height - m_buttonHeight;

	int ButtonWidth = (m_width - BORDER_LEFT - BORDER_RIGHT) / 4;
	m_frameBuffer->paintBoxRel(m_x, top, m_width, m_buttonHeight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.Foot_gradient);

	::paintButtons(m_frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, m_x + BORDER_LEFT + 3*ButtonWidth, top, ButtonWidth, 1, &RescanButton, m_buttonHeight);
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
	m_frameBuffer->paintBoxRel(m_x, m_y, m_width, m_title_height, COL_MENUCONTENT_PLUS_6);
	m_frameBuffer->paintBoxRel(m_x + 2, m_y + 2, m_width - 4, m_title_height - 4, COL_MENUHEAD_INFO_PLUS_0, NO_RADIUS, CORNER_NONE, g_settings.Head_Info_gradient);

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
	if(xstart < BORDER_LEFT)
		xstart = BORDER_LEFT;
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + xstart, m_y + 4 + m_mheight, m_width - BORDER_LEFT - BORDER_RIGHT, tmp, COL_MENUHEAD_INFO, 0, true); // UTF-8

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
	if(xstart < BORDER_LEFT)
		xstart = BORDER_LEFT;
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + xstart, m_y + 4 + m_mheight + 2 + m_mheight, m_width - BORDER_LEFT - BORDER_RIGHT, tmp, COL_MENUHEAD_INFO, 0, true); // UTF-8

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
	if(xstart < BORDER_LEFT)
		xstart = BORDER_LEFT;
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + xstart, m_y + 4 + m_mheight + 2 + m_mheight + 2 + m_mheight, m_width - BORDER_LEFT - BORDER_RIGHT, tmp, COL_MENUHEAD_INFO, 0, true); // UTF-8


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
	m_frameBuffer->paintBoxRel(m_x, top, m_width, m_buttonHeight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.Foot_gradient);
	
	// play
	::paintButtons(m_frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, m_x + BORDER_LEFT, top, ButtonWidth, 1, &PlayButton, m_buttonHeight);

	// up
	::paintButtons(m_frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, m_x + BORDER_LEFT + ButtonWidth, top, ButtonWidth, 1, &PUpButton, m_buttonHeight);

	// down
	::paintButtons(m_frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, m_x + BORDER_LEFT + 2*ButtonWidth, top, ButtonWidth, 1, &PDownButton, m_buttonHeight);

	// play (ok)
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
				m_playing_entry_is_shown = true;

				// first line
				g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(m_x + BORDER_LEFT, top + 4 + (m_info_height/2 - g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getHeight(), m_width - BORDER_LEFT - BORDER_RIGHT, m_playing_entry.title + " - " + m_playing_entry.artist, COL_MENUFOOT_INFO, 0, true); // UTF-8

				// second line
				g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(m_x + BORDER_LEFT, top + 4 + m_info_height/2 + (m_info_height/2 - g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getHeight(), m_width - BORDER_LEFT - BORDER_RIGHT, m_playing_entry.album, COL_MENUFOOT_INFO, 0, true); // UTF-8
			}
		} 
		else 
		{
			if (entry == NULL) 
				return;
			
			m_playing_entry_is_shown = false;

			// first line
			g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(m_x + BORDER_LEFT, top + 4 + (m_info_height/2 - g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getHeight(), m_width - BORDER_LEFT - BORDER_RIGHT, (*entry)[index].title + " - " + (*entry)[index].artist, COL_MENUFOOT_INFO, 0, true); // UTF-8

			// second line
			g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(m_x + BORDER_LEFT, top + 4 + m_info_height/2 + (m_info_height/2 - g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getHeight(), m_width - BORDER_LEFT - BORDER_RIGHT, (*entry)[index].album, COL_MENUFOOT_INFO, 0, true); // UTF-8
		}
	}
}

void CUpnpBrowserGui::clearItem2DetailsLine()
{
	::clearItem2DetailsLine(m_x, m_y, m_width, m_height - m_info_height, m_info_height); 
}

void CUpnpBrowserGui::paintItem2DetailsLine(int pos)
{
	::paintItem2DetailsLine(m_x, m_y, m_width, m_height - m_info_height, m_info_height, m_title_height + m_theight, m_fheight, pos);
}

void plugin_init(void)
{
}

void plugin_del(void)
{
}

void plugin_exec(void)
{
	// class handler
	CUpnpBrowserGui * upnpHandler = new CUpnpBrowserGui();
	
	upnpHandler->exec(NULL, "");
	
	delete upnpHandler;
	upnpHandler = NULL;
}



