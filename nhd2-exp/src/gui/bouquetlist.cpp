/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: bouquetlist.cpp 11.01.2019 mohousch Exp $

	Copyright (C) 2001 Steffen Hehn 'McClean'
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

#include <string>
#include <algorithm>

#include <gui/bouquetlist.h>

#include <driver/color.h>

#include <gui/eventlist.h>
#include <gui/infoviewer.h>
#include <gui/filebrowser.h>

#include <gui/widget/widget_helpers.h>
#include <gui/widget/icons.h>

#include <driver/fontrenderer.h>
#include <driver/screen_max.h>
#include <driver/rcinput.h>
#include <daemonc/remotecontrol.h>
#include <system/settings.h>

#include <global.h>
#include <neutrino.h>

#include <system/debug.h>


extern CBouquetManager * g_bouquetManager;

CBouquetList::CBouquetList(const char* const Name)
{
	frameBuffer = CFrameBuffer::getInstance();
	selected    = 0;
	
	if(Name == NULL)
		name = g_Locale->getText(LOCALE_BOUQUETLIST_HEAD);
	else
		name = Name;

	//
	listBox = NULL;
	item = NULL;

	// box	
	cFrameBox.iWidth = w_max ( (frameBuffer->getScreenWidth() / 20 * 17), (frameBuffer->getScreenWidth() / 20 ));
	cFrameBox.iHeight = h_max ( (frameBuffer->getScreenHeight() / 20 * 18), (frameBuffer->getScreenHeight() / 20));
	
	cFrameBox.iX = frameBuffer->getScreenX() + (frameBuffer->getScreenWidth() - cFrameBox.iWidth) / 2;
	cFrameBox.iY = frameBuffer->getScreenY() + (frameBuffer->getScreenHeight() - cFrameBox.iHeight) / 2;		
}

CBouquetList::~CBouquetList()
{
        for (std::vector<CBouquet *>::iterator it = Bouquets.begin(); it != Bouquets.end(); it++) 
	{
               	delete (*it);
        }
        
	Bouquets.clear();
}

CBouquet * CBouquetList::addBouquet(CZapitBouquet* zapitBouquet)
{
	CBouquet * tmp = addBouquet(zapitBouquet->Name.c_str(), -1, zapitBouquet->bLocked);
	tmp->zapitBouquet = zapitBouquet;
	
	return tmp;
}

CBouquet * CBouquetList::addBouquet(const char * const _name, int BouquetKey, bool locked)
{
	if ( BouquetKey == -1 )
		BouquetKey = Bouquets.size();

	CBouquet * tmp = new CBouquet( BouquetKey, _name, locked );
	Bouquets.push_back(tmp);
	
	return(tmp);
}

void CBouquetList::deleteBouquet(CBouquet * bouquet)
{
	if (bouquet != NULL) 
	{
		std::vector<CBouquet *>::iterator it = find(Bouquets.begin(), Bouquets.end(), bouquet);

		if (it != Bouquets.end()) 
		{
			Bouquets.erase(it);
			delete bouquet;
		}
	}
}

int CBouquetList::getActiveBouquetNumber()
{
	return selected;
}

void CBouquetList::adjustToChannel( int nChannelNr)
{
	for (uint32_t i = 0; i<Bouquets.size(); i++) 
	{
		int nChannelPos = Bouquets[i]->channelList->hasChannel(nChannelNr);
		if (nChannelPos > -1) 
		{
			selected = i;
			Bouquets[i]->channelList->setSelected(nChannelPos);
			return;
		}
	}
}

void CBouquetList::adjustToChannelID(t_channel_id channel_id)
{
	if(selected < Bouquets.size()) 
	{
		int nChannelPos = Bouquets[selected]->channelList->hasChannelID(channel_id);
		if(nChannelPos > -1) 
		{
			//printf("CBouquetList::adjustToChannelID to %llx -> not needed\n", channel_id);
			Bouquets[selected]->channelList->setSelected(nChannelPos);
			return;
		}
	}
	
	dprintf(DEBUG_NORMAL, "CBouquetList::adjustToChannelID to %llx\n", channel_id);
	
	for (uint32_t i = 0; i < Bouquets.size(); i++) 
	{
		if(i == selected)
			continue;
		
		int nChannelPos = Bouquets[i]->channelList->hasChannelID(channel_id);
		if (nChannelPos > -1) 
		{
			selected = i;
			Bouquets[i]->channelList->setSelected(nChannelPos);
			return;
		}
	}
}

// used in channellist to switch bouquets up/down
int CBouquetList::showChannelList( int nBouquet)
{
	dprintf(DEBUG_NORMAL, "CBouquetList::showChannelList\n");

	if (nBouquet == -1)
		nBouquet = selected;

	int nNewChannel = Bouquets[nBouquet]->channelList->exec();
	
	if (nNewChannel > -1) 
	{
		selected = nBouquet;
		nNewChannel = -2;
	}
	
	return nNewChannel;
}

// bShowChannelList default to false , return seems not checked anywhere
int CBouquetList::activateBouquet( int id, bool bShowChannelList)
{
	int res = -1;

	if(id < (int) Bouquets.size())
		selected = id;

	if (bShowChannelList) 
	{
		res = Bouquets[selected]->channelList->exec();

		if(res > -1)
			res = -2;
	}
	
	return res;
}

int CBouquetList::exec( bool bShowChannelList)
{
	dprintf(DEBUG_NORMAL, "CBouquetList::exec: %d\n", bShowChannelList? 1 : 0);

	// select bouquet to show
	int res = show(bShowChannelList);

	dprintf(DEBUG_NORMAL, "CBouquetList::exec: res:%d\n", res);

	if(!bShowChannelList)
		return res;
	
	// if >= 0, call activateBouquet to show channel list
	if ( res > -1) 
	{
		return activateBouquet(selected, bShowChannelList);
	}
	
	return res;
}

int CBouquetList::doMenu()
{
	int i = 0;
	int select = -1;
	static int old_selected = 0;
	int ret = menu_return::RETURN_NONE;
	signed int bouquet_id;
	CZapitBouquet * tmp, * zapitBouquet;
	ZapitChannelList * channels;

	if( !Bouquets.size() )
		return 0;

	zapitBouquet = Bouquets[selected]->zapitBouquet;
	
	// zapitBouquet not NULL only on real bouquets, not on virtual SAT or HD 
	if(!zapitBouquet)
		return 0;

	ClistBoxWidget * menu = new ClistBoxWidget(LOCALE_CHANNELLIST_EDIT, NEUTRINO_ICON_SETTINGS);
	menu->enableSaveScreen();
	menu->setMode(MODE_MENU);
	menu->enableShrinkMenu();

	if(!zapitBouquet->bUser) 
	{
		menu->addItem(new CMenuForwarder(LOCALE_FAVORITES_COPY, true, NULL, NULL, NULL, RC_blue, NEUTRINO_ICON_BUTTON_BLUE), old_selected == i ++);
		ret = menu->exec(NULL, "");
		select = menu->getSelected();
		delete menu;
		
		dprintf(DEBUG_NORMAL, "CBouquetList::doMenu: %d selected\n", select);

		if(select >= 0) 
		{
			old_selected = select;

			switch(select) 
			{
				case 0:
					hide();
					bouquet_id = g_bouquetManager->existsUBouquet(Bouquets[selected]->channelList->getName());
					if(bouquet_id < 0) 
						tmp = g_bouquetManager->addBouquet(Bouquets[selected]->channelList->getName(), true);
					else
						tmp = g_bouquetManager->Bouquets[bouquet_id];
  
					// tv
					channels = &zapitBouquet->tvChannels;
					for(int i1 = 0; i1 < (int) channels->size(); i1++)
						tmp->addService((*channels)[i1]);
					
					// radio
					channels = &zapitBouquet->radioChannels;
					for(int li = 0; li < (int) channels->size(); li++)
						tmp->addService((*channels)[li]);
					return 1;
					break;
					
				default:
					break;
			}
		}
	} 
	else 
	{
		menu->addItem(new CMenuForwarder(LOCALE_BOUQUETEDITOR_DELETE, true, NULL, NULL, NULL, RC_blue, NEUTRINO_ICON_BUTTON_BLUE), old_selected == i ++);
		ret = menu->exec(NULL, "");
		select = menu->getSelected();
		delete menu;
		
		dprintf(DEBUG_NORMAL, "CBouquetList::doMenu: %d selected\n", select);
		
		if(select >= 0) 
		{
			old_selected = select;
			switch(select) 
			{
				case 0:
					hide();
					bouquet_id = g_bouquetManager->existsUBouquet(Bouquets[selected]->channelList->getName());
					if(bouquet_id >= 0) 
					{
						g_bouquetManager->deleteBouquet(bouquet_id);
						return 1;
					}
					break;
					
				default:
					break;
			}
		}
	}
	
	return ret;
}

// bShowChannelList default to true, returns new bouquet or -1/-2
int CBouquetList::show(bool bShowChannelList)
{
	dprintf(DEBUG_NORMAL, "CBouquetList::show\n");

	neutrino_msg_t      msg;
	neutrino_msg_data_t data;
	int res = -1;
	
	CVFD::getInstance()->setMode(CVFD::MODE_MENU_UTF8);	

	//
	if(listBox)
	{
		delete listBox;
		listBox = NULL;
	}

	listBox = new ClistBox(&cFrameBox);

	paint();
		
	frameBuffer->blit();

	int zapOnExit = false;

	uint64_t timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_CHANLIST]);

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);
	
	int mode = CNeutrinoApp::getInstance()->getMode();

	bool loop = true;
	while (loop) 
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if ( msg <= RC_MaxRC )
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_CHANLIST]);

		if ((msg == RC_timeout) || (msg == (neutrino_msg_t)g_settings.key_channelList_cancel))
		{
			loop = false;
		}
		else if(msg == RC_red || msg == RC_favorites) 
		{
			CNeutrinoApp::getInstance()->SetChannelMode(LIST_MODE_FAV, mode);

			hide();
			return -3;
		} 
		else if(msg == RC_green) 
		{
			CNeutrinoApp::getInstance()->SetChannelMode(LIST_MODE_PROV, mode);
			hide();
			return -3;
		} 
		else if(msg == RC_yellow || msg == RC_sat)
		{
			CNeutrinoApp::getInstance()->SetChannelMode(LIST_MODE_SAT, mode);

			hide();
			return -3;
		} 
		else if(msg == RC_blue) 
		{
			CNeutrinoApp::getInstance()->SetChannelMode(LIST_MODE_ALL, mode);

			hide();
			return -3;
		}
		else if(Bouquets.size() == 0)
		{
			continue; //FIXME msgs not forwarded to neutrino !!
		}
		else if ( msg == RC_setup ) 
		{
			selected = listBox->getSelected();

			if (CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_webtv)
			{
#if defined ENABLE_LOAD_ALL_WEBTV_BOUQUETS
				int ret = doMenu();
				if(ret) 
				{
					res = -4;
					loop = false;
				}
#else
				int ret = webTVBouquets();

				if(ret) 
				{
					res = -1; //FIXME: dont show channellist (worlaround)
					loop = false;
				}  
#endif
			}
			else 
			{
				int ret = doMenu();
				if(ret) 
				{
					res = -4;
					loop = false;
				} 
			}
		}
		else if ( msg == (neutrino_msg_t) g_settings.key_list_start ) 
		{
			selected = 0;
			listBox->clearItems();
			paint();
		}
		else if ( msg == (neutrino_msg_t) g_settings.key_list_end ) 
		{
			selected = Bouquets.size() - 1;

			listBox->clearItems();
			paint();
		}
		else if (msg == RC_up || (int) msg == g_settings.key_channelList_pageup )
		{
			listBox->scrollLineUp();
		}
		else if ( msg == RC_down || (int) msg == g_settings.key_channelList_pagedown )
		{
			listBox->scrollLineDown();
		}
		else if ( msg == RC_ok ) 
		{
			selected = listBox->getSelected();

			if(!bShowChannelList || Bouquets[selected]->channelList->getSize() > 0) 
			{
				zapOnExit = true;
				loop = false;
			}
		}
		else if ( (msg == NeutrinoMessages::EVT_TIMER) && (data == sec_timer_id) )
		{
			listBox->paintHead();
		} 
		else 
		{
			if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all ) 
			{
				loop = false;
				res = -2;
			}
		}
			
		frameBuffer->blit();	
	}
	
	hide();
	
	CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);

	//
	g_RCInput->killTimer(sec_timer_id);
	sec_timer_id = 0;

	delete listBox;
	listBox = NULL;
	
	if(zapOnExit) 
		return (selected);
	else 
		return (res);
}

void CBouquetList::hide()
{
	if(listBox)
		listBox->hide();
	else
		CFrameBuffer::getInstance()->clearFrameBuffer();
}

const struct button_label HButton = {NEUTRINO_ICON_BUTTON_SETUP, NONEXISTANT_LOCALE, NULL };

const struct button_label CBouquetListButtons[4] =
{
        { NEUTRINO_ICON_BUTTON_RED, LOCALE_CHANNELLIST_FAVS},
        { NEUTRINO_ICON_BUTTON_GREEN, LOCALE_CHANNELLIST_PROVS},
        { NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_CHANNELLIST_SATS},
        { NEUTRINO_ICON_BUTTON_BLUE, LOCALE_CHANNELLIST_HEAD}
};

void CBouquetList::paint()
{
	dprintf(DEBUG_NORMAL, "CBouquetList::paint\n");

	for (unsigned int count = 0; count < Bouquets.size(); count++)
	{
		item = new ClistBoxItem((Bouquets[count]->zapitBouquet && Bouquets[count]->zapitBouquet->bFav) ? g_Locale->getText(LOCALE_FAVORITES_BOUQUETNAME) : Bouquets[count]->channelList->getName());

		item->setNumber(count + 1);
		listBox->addItem(item);
	}

	listBox->setTitle(name.c_str());
	listBox->enablePaintHead();
	listBox->enablePaintDate();

	listBox->setHeaderButtons(&HButton, 1);

	// foot
	listBox->enablePaintFoot();
	listBox->setFooterButtons(CBouquetListButtons, 4);

	//
	listBox->setSelected(selected);
	listBox->paint();
}

//
int CBouquetList::webTVBouquets(void)
{
	dprintf(DEBUG_NORMAL, "CBouquetList::webTVBouquets\n");

	int ret = menu_return::RETURN_NONE;

	CFileFilter fileFilter;
	
	fileFilter.addFilter("xml");
	fileFilter.addFilter("tv");
	fileFilter.addFilter("m3u");

	//
	CFileList filelist;
	ClistBoxWidget m("WebTV", NEUTRINO_ICON_WEBTV_SMALL/*, w_max ( (frameBuffer->getScreenWidth() / 20 * 17), (frameBuffer->getScreenWidth() / 20 )), h_max ( (frameBuffer->getScreenHeight() / 20 * 18), (frameBuffer->getScreenHeight() / 20))*/);
	m.enableSaveScreen();
	m.enableShrinkMenu();

	m.setMode(MODE_LISTBOX);
	m.enablePaintDate();

	int select = -1;
	int count = 0;
	static int old_select = 0;

	std::string Path_local = CONFIGDIR "/webtv";

	// read list
	if(CFileHelpers::getInstance()->readDir(Path_local, &filelist, &fileFilter))
	{
		std::string bTitle;

		for (unsigned int i = 0; i < filelist.size(); i++)
		{
			bTitle = filelist[i].getFileName();

			removeExtension(bTitle);

			m.addItem(new CMenuForwarder(bTitle.c_str(), true, NULL, NULL, to_string(count).c_str()), old_select == count);

			count++;
		}
	}

	ret = m.exec(NULL, "");
	select = m.getSelected();

	// select
	if(select >= 0)
	{
		old_select = select;

		g_settings.webtv_userBouquet.clear();
		
		g_settings.webtv_userBouquet = filelist[select].Name.c_str();
		
		dprintf(DEBUG_NORMAL, "CBouquetList::addUserBouquet: settings file %s\n", g_settings.webtv_userBouquet.c_str());
		
		// reload channels
		g_Zapit->reinitChannels();

		return 1;
	}

	return ret;
}



