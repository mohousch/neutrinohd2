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
#include <driver/screen_max.h>

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

#include <upnp.h>

#define UPNP_ICON_SMALL PLUGINDIR "/upnp/upnp_small.png"


extern "C" void plugin_exec(void);
extern "C" void plugin_init(void);
extern "C" void plugin_del(void);


const struct button_label RescanButton = {NEUTRINO_ICON_BUTTON_BLUE  , LOCALE_UPNPBROWSER_RESCAN, NULL };
const struct button_label PlayButton   = {NEUTRINO_ICON_BUTTON_RED, LOCALE_AUDIOPLAYER_PLAY, NULL };
const struct button_label PUpButton    = {NEUTRINO_ICON_BUTTON_GREEN, LOCALE_FILEBROWSER_NEXTPAGE, NULL };
const struct button_label PDownButton  = {NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_FILEBROWSER_PREVPAGE, NULL };

/*
CUpnpEntry::CUpnpEntry()
{
	m_frameBuffer = CFrameBuffer::getInstance();

	elist = NULL;
	item = NULL;
}

CUpnpEntry::~CUpnpEntry()
{
}

void CUpnpEntry::hide()
{
	m_frameBuffer->ClearFrameBuffer();
	m_frameBuffer->blit();
}
*/

CUpnpBrowserGui::CUpnpBrowserGui(UPNP_GUI g)
{
	m_socket = new CUPnPSocket();
	m_frameBuffer = CFrameBuffer::getInstance();

	thumbnail_dir = "/tmp/upnpbrowser";

	ulist = NULL;
	elist = NULL;
	item = NULL;

	selected = 0;
	gui = g;

	fileHelper.createDir(thumbnail_dir.c_str(), 0755);
}

CUpnpBrowserGui::~CUpnpBrowserGui()
{
	if(m_socket)
		delete m_socket;

	fileHelper.removeDir(thumbnail_dir.c_str());
}

void CUpnpBrowserGui::hide()
{
	m_frameBuffer->ClearFrameBuffer();
	m_frameBuffer->blit();
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

	parser = new XMLTreeParser("UTF-8");
	parser->Parse(result.c_str(), result.size(), 1);
	root = parser->RootNode();
	
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

//
bool CUpnpBrowserGui::loadItem(std::string id)
{
	dprintf(DEBUG_NORMAL, "CUpnpBrowserGui::loadItem: %s\n", id.c_str());

	unsigned int index, _selected, dirnum;

	index = 0;
	_selected = 0;
	dirnum = 0;
	entries = NULL;

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
	scount << /*ulist->getListMaxShow()*/100;

	attribs.push_back(UPnPAttribute("ObjectID", id));
	attribs.push_back(UPnPAttribute("BrowseFlag", "BrowseDirectChildren"));
	attribs.push_back(UPnPAttribute("Filter", "*"));
	attribs.push_back(UPnPAttribute("StartingIndex", sindex.str()));
	attribs.push_back(UPnPAttribute("RequestedCount", /*scount.str()*/"100"));
	attribs.push_back(UPnPAttribute("SortCriteria", ""));

	results = m_devices[_selected].SendSOAP("urn:schemas-upnp-org:service:ContentDirectory:1", "Browse", attribs);

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
		return false;

	if (!nfound || !tfound || !rfound)
	{
		delete entries;
		return false;
	}

	if (returned != entries->size())
	{
		delete entries;
		return false;
	}

	if (returned == 0)
	{
		delete entries;
		return false;
	}
	
	return true;
}

void CUpnpBrowserGui::loadDevices()
{
	m_devices.clear();

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

	delete scanBox;
	scanBox = NULL;
}

void CUpnpBrowserGui::showMenuDevice()
{
	dprintf(DEBUG_NORMAL, "CUpnpBrowserGui::showMenuDevice:\n");

	selected = 0;

	// new
	ulist = new ClistBox(LOCALE_UPNPBROWSER_HEAD, NEUTRINO_ICON_UPNP, w_max ( (m_frameBuffer->getScreenWidth() / 20 * 17), (m_frameBuffer->getScreenWidth() / 20 )), h_max ( (m_frameBuffer->getScreenHeight() / 20 * 16), (m_frameBuffer->getScreenHeight() / 20)));

	// add items
	for(unsigned int i = 0; i < m_devices.size(); i++)
	{
		char sNr[20];
		sprintf(sNr, "%2d", i + 1);

		item = new ClistBoxItem(m_devices[i].modelname.c_str(), true, "", this, "entry");

		item->setOptionInfo(m_devices[i].friendlyname.c_str());
		item->setNumber(i + 1);

		// details Box
		item->setInfo1(m_devices[i].manufacturer.c_str());
		item->setOptionInfo1(m_devices[i].manufacturerurl.c_str());
		item->setInfo2(m_devices[i].modeldescription.c_str());
		item->setOptionInfo2(m_devices[i].modelnumber.c_str());

		ulist->addItem(item);
	}
	
	//
	ulist->setWidgetType(WIDGET_STANDARD);
	//ulist->setTimeOut(g_settings.timing[SNeutrinoSettings::TIMING_CHANLIST]);
	ulist->setSelected(selected);

	ulist->setFooterButtons(&RescanButton, 1);
	
	ulist->enableFootInfo();
	ulist->enablePaintDate();

	ulist->addKey(CRCInput::RC_red, this, CRCInput::getSpecialKeyName(CRCInput::RC_red));
	ulist->addKey(CRCInput::RC_green, this, CRCInput::getSpecialKeyName(CRCInput::RC_green));
	ulist->addKey(CRCInput::RC_yellow, this, CRCInput::getSpecialKeyName(CRCInput::RC_yellow));
	ulist->addKey(CRCInput::RC_blue, this, CRCInput::getSpecialKeyName(CRCInput::RC_blue));

	ulist->exec(NULL, "");
	//list->hide();
	delete ulist;
	ulist = NULL;
}

void CUpnpBrowserGui::showMenuEntry()
{
	dprintf(DEBUG_NORMAL, "CUpnpBrowserGui::showMenuEntry:\n");

	selected = 0;

	elist = new ClistBox(LOCALE_UPNPBROWSER_HEAD, NEUTRINO_ICON_UPNP, w_max ( (m_frameBuffer->getScreenWidth() / 20 * 17), (m_frameBuffer->getScreenWidth() / 20 )), h_max ( (m_frameBuffer->getScreenHeight() / 20 * 16), (m_frameBuffer->getScreenHeight() / 20)));

	
	for(unsigned int i = 0; i < entries->size(); i++)
	{
		//
		int preferred = (*entries)[i].preferred;
		std::string info;
		std::string fileicon;
	
		if ((*entries)[i].isdir)
		{
			info = "<DIR>";
			fileicon = NEUTRINO_ICON_FOLDER;
		}
		else
		{
			//FIXME: revise this
			if (preferred != -1)
			{
				info = (*entries)[i].resources[preferred].duration;
			
				// icon
				std::string protocol, prot, network, mime, additional;

				protocol = (*entries)[i].resources[preferred].protocol;
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
		//

		item = new ClistBoxItem((*entries)[i].title.c_str(), true, "", this, "sub_entry");

		item->setIconName(fileicon.c_str());
		item->setOptionInfo(info.c_str());

		//
		std::string tmp;
		std::stringstream ts;

		ts << "Resources: " << (*entries)[i].resources.size() << " Selected: " << preferred + 1 << " ";
		tmp = ts.str();

		if (preferred != -1)
			tmp = tmp + "Duration: " + (*entries)[i].resources[preferred].duration;
		else
			tmp = tmp + "No resource for Item";

		item->setInfo1(tmp.c_str());

		if ((*entries)[i].isdir)
			tmp = "Directory";
		else
		{
			tmp = "";
			if (preferred != -1)
			{
				std::string proto, network, mime, _info;
				splitProtocol((*entries)[i].resources[preferred].protocol, proto, network, mime, _info);
				tmp = "Protocol: " + proto + ", MIME-Type: " + mime;
			}

		}

		item->setInfo2(tmp.c_str());

		elist->addItem(item);
	}

	//
	elist->setWidgetType(WIDGET_STANDARD);
	//elist->setTimeOut(g_settings.timing[SNeutrinoSettings::TIMING_CHANLIST]);
	elist->setSelected(selected);

	elist->setFooterButtons(&RescanButton, 1);
	
	elist->enableFootInfo();
	elist->enablePaintDate();

	elist->addKey(CRCInput::RC_red, this, CRCInput::getSpecialKeyName(CRCInput::RC_red));
	elist->addKey(CRCInput::RC_green, this, CRCInput::getSpecialKeyName(CRCInput::RC_green));
	elist->addKey(CRCInput::RC_yellow, this, CRCInput::getSpecialKeyName(CRCInput::RC_yellow));
	elist->addKey(CRCInput::RC_blue, this, CRCInput::getSpecialKeyName(CRCInput::RC_blue));
	elist->addKey(CRCInput::RC_home, this, CRCInput::getSpecialKeyName(CRCInput::RC_home));

	elist->exec(NULL, "");
	//elist->hide();
	delete elist;
	elist = NULL;
}

int CUpnpBrowserGui::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CUpnpBrowserGui::exec: %s\n", actionKey.c_str());

	bool endall = false;
	int cnt = 0;

	if(parent)
		parent->hide();

	if(actionKey == "entry")
	{
		gui = UPNP_GUI_ENTRY;

		loadItem("0");
		showMenuEntry();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "sub_entry")
	{
		gui = UPNP_GUI_SUBENTRY;
		cnt++;

		if(elist)
			selected = elist->getSelected();

		if ((*entries)[selected].isdir)
		{
			loadItem((*entries)[selected].id);
			showMenuEntry();

			loadItem("0");

			return menu_return::RETURN_EXIT_ALL;
		}
		else if (!(*entries)[selected].isdir)
		{
			int preferred = (*entries)[selected].preferred;
			if (preferred != -1)
			{
				std::string protocol, prot, network, mime, additional;
				protocol=(*entries)[selected].resources[preferred].protocol;
				splitProtocol(protocol, prot, network, mime, additional);
					
				if (mime == "audio/mpeg")
				{
					CAudiofile audiofile((*entries)[selected].resources[preferred].url, CFile::EXTENSION_MP3);
					tmpAudioPlayerGui.addToPlaylist(audiofile);
					tmpAudioPlayerGui.exec(this, "");
				}
				else if ((mime == "image/gif") || (mime == "image/jpeg"))
				{
					std::string fname;

					//DownloadImage
					if(!((*entries)[selected].resources[preferred].url).empty())
					{
						fname = thumbnail_dir;

						fname += (*entries)[selected].resources[preferred].url.substr((*entries)[selected].resources[preferred].url.find_last_of("/"));

						int ext_pos = 0;
						ext_pos = fname.rfind('?');
	
						if( ext_pos > 0)
						{
							std::string extension;
							extension = fname.substr(ext_pos + 1, fname.length() - ext_pos);

							fname = fname.substr(0, fname.length() - (extension.length() + 1));
						}

						::downloadUrl((*entries)[selected].resources[preferred].url, fname);
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
					tmpPictureViewerGui.exec(this, "");
				}
				else if (mime.substr(0, 6) == "video/")
				{
					MI_MOVIE_INFO mfile;
					mfile.file.Name = (*entries)[selected].resources[preferred].url.c_str(); 
					mfile.epgTitle = (*entries)[selected].title;
					mfile.ytid = "upnp"; 
						
					tmpMoviePlayerGui.addToPlaylist(mfile);
					tmpMoviePlayerGui.exec(this, "");
				}
			}

			return menu_return::RETURN_REPAINT;
		}
	}
	else if(actionKey == "RC_blue")
	{
		gui = UPNP_GUI_DEVICE;

		loadDevices();
		showMenuDevice();

		return menu_return::RETURN_EXIT_ALL;
	}
	/*
	else if(actionKey == "RC_home")
	{
		selected = 0;
		endall = true;

		//loadItem("0");
		//showMenuEntry();

		//return menu_return::RETURN_EXIT_ALL;
	}
	*/

	printf("cnt:%d\n", cnt);

/*
	if(endall && gui == UPNP_GUI_SUBENTRY)
	{
		loadItem("0");
		showMenuEntry();

		return menu_return::RETURN_EXIT_ALL;
	}
	else
*/
	if(gui == UPNP_GUI_DEVICE)
	{
		loadDevices();
		showMenuDevice();
	}

	return menu_return::RETURN_EXIT_ALL;
}

void plugin_init(void)
{
}

void plugin_del(void)
{
}

void plugin_exec(void)
{
	CUpnpBrowserGui * upnpHandler = new CUpnpBrowserGui();
	
	upnpHandler->exec(NULL, "");
	
	delete upnpHandler;
	upnpHandler = NULL;
}



