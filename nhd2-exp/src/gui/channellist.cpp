/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: channellist.cpp 11.01.2019 mohousch Exp $

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

#include <gui/channellist.h>

#include <global.h>
#include <neutrino.h>

#include <driver/fontrenderer.h>
#include <driver/screen_max.h>
#include <driver/rcinput.h>
#include <driver/color.h>

#include <gui/eventlist.h>
#include <gui/infoviewer.h>

#include <gui/widget/widget_helpers.h>
#include <gui/widget/icons.h>
#include <gui/widget/messagebox.h>

#include <system/settings.h>
#include <system/lastchannel.h>
#include "gui/filebrowser.h"

#include <gui/bouquetlist.h>
#include <daemonc/remotecontrol.h>
#include <driver/vcrcontrol.h>
#include <gui/pictureviewer.h>

//
#include <bouquets.h>
#include <satconfig.h>
#include <getservices.h>
#include <frontend_c.h>

#include <gui/epgplus.h>
#include <system/debug.h>

#include <video_cs.h>
#include <misc_setup.h>


extern CBouquetList * bouquetList;      		// neutrino.cpp
extern CRemoteControl * g_RemoteControl; 		// neutrino.cpp	
extern CBouquetList   * TVbouquetList;
extern CBouquetList   * TVsatList;
extern CBouquetList   * TVfavList;
extern CBouquetList   * TVallList;
extern CBouquetList   * RADIObouquetList;
extern CBouquetList   * RADIOsatList;
extern CBouquetList   * RADIOfavList;
extern CBouquetList   * RADIOallList;


extern t_channel_id rec_channel_id;
extern t_channel_id live_channel_id;
bool pip_selected = false;
extern bool autoshift;
int info_height = 0;
bool new_mode_active = 0;

extern int FrontendCount;				// defined in zapit.cpp
extern bool CanZap(CZapitChannel * thischannel);	// defined in zapit.cpp

extern CBouquetManager * g_bouquetManager;

extern cVideo * videoDecoder;
extern CZapitChannel * live_channel;

// channel events
void sectionsd_getChannelEvents(CChannelEventList &eList, const bool tv_mode, t_channel_id *chidlist, int clen);
void sectionsd_getEventsServiceKey(t_channel_id serviceUniqueKey, CChannelEventList &eList, char search = 0, std::string search_text = "");

//
void addChannelToBouquet(const unsigned int bouquet, const t_channel_id channel_id);	// defined in zapit.cpp

extern int old_b_id;

CChannelList::CChannelList(const char * const Name, bool _historyMode, bool _vlist)
{
	frameBuffer = CFrameBuffer::getInstance();

	name = Name;
	selected = 0;
	tuned = 0xfffffff;
	zapProtection = NULL;
	this->historyMode = _historyMode;
	vlist = _vlist;

	//
	listBox = NULL;
	item = NULL;

	// box	
	cFrameBox.iWidth = w_max ( (frameBuffer->getScreenWidth() / 20 * 17), (frameBuffer->getScreenWidth() / 20 ));
	cFrameBox.iHeight = h_max ( (frameBuffer->getScreenHeight() / 20 * 18), (frameBuffer->getScreenHeight() / 20));
	
	cFrameBox.iX = frameBuffer->getScreenX() + (frameBuffer->getScreenWidth() - cFrameBox.iWidth) / 2;
	cFrameBox.iY = frameBuffer->getScreenY() + (frameBuffer->getScreenHeight() - cFrameBox.iHeight) / 2;		
}

CChannelList::~CChannelList()
{
	chanlist.clear();
}

void CChannelList::ClearList(void)
{
	chanlist.clear();
	chanlist.resize(1);
}

void CChannelList::setSize(int newsize)
{
	chanlist.reserve(newsize);
}

void CChannelList::addChannel(CZapitChannel * channel, int num)
{
	if(num)
		channel->number = num;
	
	chanlist.push_back(channel);
}

void CChannelList::putChannel(CZapitChannel * channel)
{
	int num = channel->number - 1;
	
	if(num < 0) 
	{
		return;
	}
	
	if(num >= (int) chanlist.size()) 
	{
		chanlist.resize((unsigned) num + 1);
	}
	chanlist[num] = channel;
}

void CChannelList::updateEvents(void)
{
	CChannelEventList events;

	if (displayNext) 
	{
		if (chanlist.size()) 
		{
			time_t atime = time(NULL);
			unsigned int count;
			
			for (count = 0; count < chanlist.size(); count++)
			{		
				//
				events.clear();

				sectionsd_getEventsServiceKey(chanlist[count]->channel_id, events);
				chanlist[count]->nextEvent.startTime = (long)0x7fffffff;
				
				for ( CChannelEventList::iterator e = events.begin(); e != events.end(); ++e ) 
				{
					if (((long)(e->startTime) > atime) && ((e->startTime) < (long)(chanlist[count]->nextEvent.startTime)))
					{
						chanlist[count]->nextEvent= *e;
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

		if (chanlist.size()) 
		{
			size_requested_channels = chanlist.size()*sizeof(t_channel_id);
			p_requested_channels = (t_channel_id*)malloc(size_requested_channels);
			
			for (uint32_t count = 0; count < chanlist.size(); count++)
			{
				p_requested_channels[count] = chanlist[count]->channel_id&0xFFFFFFFFFFFFULL;
			}

			CChannelEventList pevents;
			sectionsd_getChannelEvents(pevents, (CNeutrinoApp::getInstance()->getMode()) != NeutrinoMessages::mode_radio, p_requested_channels, size_requested_channels);
			
			for (uint32_t count = 0; count < chanlist.size(); count++) 
			{
				chanlist[count]->currentEvent = CChannelEvent();
				
				for ( CChannelEventList::iterator e = pevents.begin(); e != pevents.end(); ++e )
				{
					if ((chanlist[count]->channel_id&0xFFFFFFFFFFFFULL) == e->get_channel_id())
					{
						chanlist[count]->currentEvent= *e;
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

struct CmpChannelBySat: public binary_function <const CZapitChannel * const, const CZapitChannel * const, bool>
{
        static bool comparetolower(const char a, const char b)
        {
		return tolower(a) < tolower(b);
        };

        bool operator() (const CZapitChannel * const c1, const CZapitChannel * const c2)
        {
		if(c1->getSatellitePosition() == c2->getSatellitePosition())
			return std::lexicographical_compare(c1->getName().begin(), c1->getName().end(), c2->getName().begin(), c2->getName().end(), comparetolower);
		else
			return c1->getSatellitePosition() < c2->getSatellitePosition();
;
	};
};

struct CmpChannelByFreq: public binary_function <const CZapitChannel * const, const CZapitChannel * const, bool>
{
        static bool comparetolower(const char a, const char b)
        {
                return tolower(a) < tolower(b);
        };

        bool operator() (const CZapitChannel * const c1, const CZapitChannel * const c2)
        {
		if(c1->getFreqId() == c2->getFreqId())
			return std::lexicographical_compare(c1->getName().begin(), c1->getName().end(), c2->getName().begin(), c2->getName().end(), comparetolower);
		else
			return c1->getFreqId() < c2->getFreqId();
;
	};
};

void CChannelList::SortAlpha(void)
{
	sort(chanlist.begin(), chanlist.end(), CmpChannelByChName());
}

void CChannelList::SortSat(void)
{
	sort(chanlist.begin(), chanlist.end(), CmpChannelBySat());
}

CZapitChannel * CChannelList::getChannel(int number)
{
	for (uint32_t i = 0; i< chanlist.size();i++) 
	{
		if (chanlist[i]->number == number)
			return chanlist[i];
	}
	
	return(NULL);
}

CZapitChannel * CChannelList::getChannel(t_channel_id channel_id)
{
	for (uint32_t i = 0; i< chanlist.size();i++) 
	{
		if (chanlist[i]->channel_id == channel_id)
			return chanlist[i];
	}
	
	return(NULL);
}

int CChannelList::getKey(int id)
{
	return chanlist[id]->number;
}

static const std::string empty_string;

const std::string & CChannelList::getActiveChannelName(void) const
{
	if (selected < chanlist.size())
		return chanlist[selected]->name;
	else
		return empty_string;
}

t_satellite_position CChannelList::getActiveSatellitePosition(void) const
{
	if (selected < chanlist.size())
		return chanlist[selected]->getSatellitePosition();
	else
		return 0;
}

t_channel_id CChannelList::getActiveChannel_ChannelID(void) const
{
	if (selected < chanlist.size()) 
	{
		printf("CChannelList::getActiveChannel_ChannelID:%llx\n", chanlist[selected]->channel_id);
		
		return chanlist[selected]->channel_id;
	} 
	else
		return 0;
}

int CChannelList::getActiveChannelNumber(void) const
{
	return (selected + 1);
}

int CChannelList::doChannelMenu(void)
{
	int i = 0;
	int select = -1;
	static int old_selected = 0;
	signed int bouquet_id, old_bouquet_id, new_bouquet_id;
	int result;
	char cnt[5];
	t_channel_id channel_id;
	
	if( !bouquetList )
		return 0;

	CMenuWidget * menu = new CMenuWidget(LOCALE_CHANNELLIST_EDIT, NEUTRINO_ICON_SETTINGS);
	menu->enableSaveScreen();
	
	CMenuSelectorTarget * selector = new CMenuSelectorTarget(&select);

	sprintf(cnt, "%d", i);
	menu->addItem(new CMenuForwarder(LOCALE_BOUQUETEDITOR_DELETE, true, NULL, selector, cnt, RC_red, NEUTRINO_ICON_BUTTON_RED), old_selected == i++);

	sprintf(cnt, "%d", i);
	menu->addItem(new CMenuForwarder(LOCALE_BOUQUETEDITOR_MOVE, true, NULL, selector, cnt, RC_green, NEUTRINO_ICON_BUTTON_GREEN), old_selected == i++);

	sprintf(cnt, "%d", i);
	menu->addItem(new CMenuForwarder(LOCALE_EXTRA_ADD_TO_BOUQUET, true, NULL, selector, cnt, RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW), old_selected == i++);

	sprintf(cnt, "%d", i);
	menu->addItem(new CMenuForwarder(LOCALE_FAVORITES_MENUEADD, true, NULL, selector, cnt, RC_blue, NEUTRINO_ICON_BUTTON_BLUE), old_selected == i++);

	menu->exec(NULL, "");
	delete menu;
	delete selector;

	if(select >= 0) 
	{
		hide();
		
		old_selected = select;
		channel_id = chanlist[selected]->channel_id;
		
		switch(select) 
		{
			case 0: //delete
				result = MessageBox( LOCALE_BOUQUETEDITOR_DELETE, "Delete channel from bouquet?", mbrNo, mbYes | mbNo );

				if(result == mbrYes) 
				{
					bouquet_id = bouquetList->getActiveBouquetNumber();
					bouquet_id = g_bouquetManager->existsBouquet(bouquetList->Bouquets[bouquet_id]->channelList->getName());
					if (bouquet_id == -1)
						return 0;
					
					if(g_bouquetManager->existsChannelInBouquet(bouquet_id, channel_id)) 
					{
						g_bouquetManager->Bouquets[bouquet_id]->removeService(channel_id);
						return 1;
					}
				}
				break;
				
			case 1: // move
				old_bouquet_id = bouquetList->getActiveBouquetNumber();
				old_bouquet_id = g_bouquetManager->existsBouquet(bouquetList->Bouquets[old_bouquet_id]->channelList->getName());

				do {
					new_bouquet_id = bouquetList->exec(false);
				} while(new_bouquet_id == -3);

				hide();
				if(new_bouquet_id < 0)
					return 0;
				new_bouquet_id = g_bouquetManager->existsBouquet(bouquetList->Bouquets[new_bouquet_id]->channelList->getName());
				if ((new_bouquet_id == -1) || (new_bouquet_id == old_bouquet_id))
					return 0;

				if(!g_bouquetManager->existsChannelInBouquet(new_bouquet_id, channel_id)) 
				{
					addChannelToBouquet(new_bouquet_id, channel_id);
				}
				
				if(g_bouquetManager->existsChannelInBouquet(old_bouquet_id, channel_id)) 
				{
					g_bouquetManager->Bouquets[old_bouquet_id]->removeService(channel_id);
				}
				return 1;

				break;
				
			case 2: // add to
				do {
					bouquet_id = bouquetList->exec(false);
				} while(bouquet_id == -3);
				
				hide();
				
				if(bouquet_id < 0)
					return 0;
				
				bouquet_id = g_bouquetManager->existsBouquet(bouquetList->Bouquets[bouquet_id]->channelList->getName());
				if (bouquet_id == -1)
					return 0;
				
				if(!g_bouquetManager->existsChannelInBouquet(bouquet_id, channel_id)) 
				{
					addChannelToBouquet(bouquet_id, channel_id);
					return 1;
				}
				break;
				
			case 3: // add to my favorites
				bouquet_id = g_bouquetManager->existsUBouquet(g_Locale->getText(LOCALE_FAVORITES_BOUQUETNAME), true);
				if(bouquet_id == -1) 
				{
					g_bouquetManager->addBouquet(g_Locale->getText(LOCALE_FAVORITES_BOUQUETNAME), true);
					bouquet_id = g_bouquetManager->existsUBouquet(g_Locale->getText(LOCALE_FAVORITES_BOUQUETNAME), true);
				}
				
				if(!g_bouquetManager->existsChannelInBouquet(bouquet_id, channel_id)) 
				{
					addChannelToBouquet(bouquet_id, channel_id);
					return 1;
				}
				
				break;
				
			default:
				break;
		}
	}
	
	return 0;
}

int CChannelList::exec()
{
	dprintf(DEBUG_NORMAL, "CChannelList::exec\n");

	displayNext = 0; // always start with current events
	
	int nNewChannel = show();
	
	// zapto
	if ( nNewChannel > -1 && nNewChannel < (int) chanlist.size()) 
		CNeutrinoApp::getInstance()->channelList->zapTo(getKey(nNewChannel)-1);

	return nNewChannel;
}

#define CHANNEL_SMSKEY_TIMEOUT 800
//return: >= 0 to zap, -1 on cancel, -3 on list mode change, -4 list edited, -2 zap but no restore old list/chan
int CChannelList::show()
{
	dprintf(DEBUG_NORMAL, "CChannelList::show\n");

	neutrino_msg_t      msg;
	neutrino_msg_data_t data;
	bool actzap = 0;
	int res = -1;

	new_mode_active = 0;

	// display channame in vfd	
	CVFD::getInstance()->setMode(CVFD::MODE_MENU_UTF8 );	

	displayNext = false;
	
	// update events
	updateEvents();

	//
	listBox = new ClistBox(&cFrameBox);

	paint();
		
	frameBuffer->blit();

	int oldselected = selected;
	int zapOnExit = false;
	bool bShowBouquetList = false;

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);

	// loop control
	uint64_t timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_CHANLIST]);

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);

	bool loop = true;
	while (loop) 
	{
		g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd );
		
		if ( msg <= RC_MaxRC )
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_CHANLIST]);

		if ( ( msg == RC_timeout ) || ( msg == (neutrino_msg_t)g_settings.key_channelList_cancel) ) 
		{
			if(!actzap)
				selected = oldselected;
			
			loop = false;
			res = -1;
		}
		else if ((msg == RC_red) || (msg == RC_epg)) // epg
		{
			selected = listBox->getSelected();

			hide();

			if ( g_EventList->exec(chanlist[selected]->channel_id, chanlist[selected]->name) == menu_return::RETURN_EXIT_ALL) 
			{
				res = -2;
				loop = false;
			}

			listBox->clearItems();
			paint();
		}
		else if ( msg == RC_yellow && ( bouquetList != NULL ) ) //bouquets
		{ 
			//FIXME: show bqt list
			bShowBouquetList = true;

			loop = false;
		}
		else if( msg == RC_blue ) //epgplus
		{
			selected = listBox->getSelected();

			hide();
			
			CEPGplusHandler eplus;
			eplus.exec(NULL, "");

			listBox->clearItems();
			paint();
		}
		else if ( msg == RC_sat || msg == RC_favorites)
		{
			g_RCInput->postMsg (msg, 0);
			loop = false;
			res = -1;
		}
		else if ( msg == RC_setup ) 
		{
			//
			selected = listBox->getSelected();

			// chan list setup (add/move)
			old_b_id = bouquetList->getActiveBouquetNumber();
			int ret = doChannelMenu();
		
			if(ret) 
			{
				res = -4;
				loop = false;
			} 
			else 
			{
				old_b_id = -1;

				listBox->clearItems();
				paint();
			}
		}
		else if (msg == (neutrino_msg_t) g_settings.key_list_start) 
		{
			selected = 0;

			listBox->clearItems();
			paint();
			
			if(new_mode_active) 
			{ 
				actzap = true; 
				zapTo(selected); 
			}
		}
		else if (msg == (neutrino_msg_t) g_settings.key_list_end) 
		{
			selected = chanlist.size() - 1;

			listBox->clearItems();
			paint();
			
			if(new_mode_active) 
			{ 
				actzap = true; 
				zapTo(selected); 
			}
		}
                else if (msg == RC_up)
                {
			listBox->scrollLineUp();

			if(new_mode_active) 
			{
				selected = listBox->getSelected();
 
				actzap = true; 
				zapTo(selected); 
			}
                }
		else if ( msg == RC_page_up )
                {
			listBox->scrollPageUp();

			if(new_mode_active) 
			{
				selected = listBox->getSelected();
 
				actzap = true; 
				zapTo(selected); 
			}
                }
                else if (msg == RC_down)
                {
			listBox->scrollLineDown();

			if(new_mode_active) 
			{ 
				selected = listBox->getSelected();

				actzap = true; 
				zapTo(selected); 
			}
                }
		else if (msg == RC_page_down )
                {
			listBox->scrollPageDown();

			if(new_mode_active) 
			{ 
				selected = listBox->getSelected();

				actzap = true; 
				zapTo(selected); 
			}
                }
		else if ((msg == (neutrino_msg_t)g_settings.key_bouquet_up) && (bouquetList != NULL)) 
		{
			if (bouquetList->Bouquets.size() > 0) 
			{
				bool found = true;
				uint32_t nNext = (bouquetList->getActiveBouquetNumber()+1) % bouquetList->Bouquets.size();
				if(bouquetList->Bouquets[nNext]->channelList->getSize() <= 0) 
				{
					found = false;
					nNext = nNext < bouquetList->Bouquets.size() - 1 ? nNext + 1 : 0;
					for(uint32_t i = nNext; i < bouquetList->Bouquets.size(); i++) 
					{
						if(bouquetList->Bouquets[i]->channelList->getSize() > 0) 
						{
							found = true;
							nNext = i;
							break;
						}
					}
				}
				
				if(found) 
				{
					bouquetList->activateBouquet(nNext, false);
					res = bouquetList->showChannelList();
					loop = false;
				}
			}
		}
		else if ((msg == (neutrino_msg_t)g_settings.key_bouquet_down) && (bouquetList != NULL)) 
		{
			if (bouquetList->Bouquets.size() > 0) 
			{
				bool found = true;
				int nNext = (bouquetList->getActiveBouquetNumber()+bouquetList->Bouquets.size()-1) % bouquetList->Bouquets.size();
				if(bouquetList->Bouquets[nNext]->channelList->getSize() <= 0) 
				{
					found = false;
					nNext = nNext > 0 ? nNext-1 : bouquetList->Bouquets.size()-1;
					
					for(int i = nNext; i > 0; i--) 
					{
						if(bouquetList->Bouquets[i]->channelList->getSize() > 0) 
						{
							found = true;
							nNext = i;
							break;
						}
					}
				}
				
				if(found) 
				{
					bouquetList->activateBouquet(nNext, false);
					res = bouquetList->showChannelList();
					loop = false;
				}
			}
		}
		else if ( msg == RC_ok ) 
		{
			selected = listBox->getSelected();
	  
			zapOnExit = true;
			
			loop = false;
		}
		else if ( msg == RC_spkr ) 
		{
			new_mode_active = (new_mode_active ? 0 : 1);
			
			listBox->clearItems();
			paint();
		}
		else if (CRCInput::isNumeric(msg) && this->historyMode) 
		{ 
			if (this->historyMode) 
			{ 
				selected = CRCInput::getNumericValue(msg);
				zapOnExit = true;
				loop = false;
    			}
		}
		else if(CRCInput::isNumeric(msg)) 
		{
			//pushback key if...
			selected = oldselected;
			g_RCInput->postMsg( msg, data );
			loop = false;
		}
		else if ( msg == RC_green ) //next
		{
			selected = listBox->getSelected();

			displayNext = !displayNext;

			updateEvents();

			listBox->clearItems();
			paint();
		} 
		else if ( (msg == RC_info) )
		{
			selected = listBox->getSelected();

			hide();
			g_EpgData->show(chanlist[selected]->channel_id); 

			listBox->clearItems();
			paint();
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
				res = - 2;
			}
		}
			
		frameBuffer->blit();	
	}
	
	hide();

	//
	g_RCInput->killTimer(sec_timer_id);
	sec_timer_id = 0;

	delete listBox;
	listBox = NULL;
	
	// bouquets mode
	if (bShowBouquetList)
	{
		//FIXME:
		if(CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_webtv)
		{
			webTVBouquets();
		}
 
		res = bouquetList->exec(true);
	}
	
	CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
	
	new_mode_active = 0;

	if(NeutrinoMessages::mode_ts == CNeutrinoApp::getInstance()->getMode())
		return -1;

	if(zapOnExit)
		res = selected;

	printf("CChannelList::show res %d\n", res);
			
	return(res);
}

void CChannelList::hide()
{
	listBox->hide();
}

bool CChannelList::showInfo(int pos, int epgpos)
{
	if((pos >= (signed int) chanlist.size()) || (pos < 0))
		return false;

	CZapitChannel * chan = chanlist[pos];
	
	// channel infobar
	g_InfoViewer->show(pos + 1, chan->name, chan->getSatellitePosition(), chan->channel_id, true, epgpos); // UTF-8
	
	return true;
}

int CChannelList::handleMsg(const neutrino_msg_t msg, neutrino_msg_data_t data)
{
	if ( msg == NeutrinoMessages::EVT_PROGRAMLOCKSTATUS) 
	{
		// 0x100 als FSK-Status zeigt an, dass (noch) kein EPG zu einem Kanal der NICHT angezeigt
		// werden sollte (vorgesperrt) da ist
		// oder das bouquet des Kanals ist vorgesperrt

		if ((g_settings.parentallock_prompt == PARENTALLOCK_PROMPT_ONSIGNAL) || (g_settings.parentallock_prompt == PARENTALLOCK_PROMPT_CHANGETOLOCKED))
		{
			if ( zapProtection != NULL )
				zapProtection->fsk = data;
			else 
			{
				// require password if either
				// CHANGETOLOCK mode and channel/bouquet is pre locked (0x100)
				// ONSIGNAL mode and fsk(data) is beyond configured value
				// if programm has already been unlocked, dont require pin
				if ((data >= (neutrino_msg_data_t)g_settings.parentallock_lockage) &&
					 ((chanlist[selected]->last_unlocked_EPGid != g_RemoteControl->current_EPGid) || (g_RemoteControl->current_EPGid == 0)) &&
					 ((g_settings.parentallock_prompt != PARENTALLOCK_PROMPT_CHANGETOLOCKED) || (data >= 0x100)))
				{
					g_RemoteControl->stopvideo();
					
					zapProtection = new CZapProtection( g_settings.parentallock_pincode, data );
					
					if ( zapProtection->check() )
					{
						g_RemoteControl->startvideo();
						
						// remember it for the next time
						chanlist[selected]->last_unlocked_EPGid = g_RemoteControl->current_EPGid;
					}
					delete zapProtection;
					zapProtection = NULL;
				}
				else
					g_RemoteControl->startvideo();
			}
		}
		else
			g_RemoteControl->startvideo();

		return messages_return::handled;
	}
	else
		return messages_return::unhandled;
}

// bToo default to true
bool CChannelList::adjustToChannelID(const t_channel_id channel_id, bool bToo)
{
	unsigned int i;

	dprintf(DEBUG_NORMAL, "CChannelList::adjustToChannelID me %x list size %d channel_id %llx\n", (int) this, chanlist.size(), channel_id);
	
	for (i = 0; i < chanlist.size(); i++) 
	{
		if(chanlist[i] == NULL) 
		{
			//printf("CChannelList::adjustToChannelID REPORT BUG !! %d is NULL !!\n", i);
			continue;
		}

		if (chanlist[i]->channel_id == channel_id) 
		{
			selected = i;
			lastChList.store(selected, channel_id, false);

			tuned = i;
			if (bToo && (bouquetList != NULL)) 
			{
				//FIXME
				if(CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_tv) 
				{
					TVbouquetList->adjustToChannelID(channel_id);
					TVsatList->adjustToChannelID(channel_id);
					TVfavList->adjustToChannelID(channel_id);
					TVallList->adjustToChannelID(channel_id);
				} 
				else if(CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_radio) 
				{
					RADIObouquetList->adjustToChannelID(channel_id);
					RADIOsatList->adjustToChannelID(channel_id);
					RADIOfavList->adjustToChannelID(channel_id);
					RADIOallList->adjustToChannelID(channel_id);
				}
			}
			
			return true;
		}
	}

	return false;
}

int CChannelList::hasChannel(int nChannelNr)
{
	for (uint32_t i = 0; i < chanlist.size(); i++) 
	{
		if (getKey(i) == nChannelNr)
			return(i);
	}
	
	return(-1);
}

int CChannelList::hasChannelID(t_channel_id channel_id)
{
	for (uint32_t i = 0; i < chanlist.size(); i++) 
	{
		if (chanlist[i]->channel_id == channel_id)
			return i;
	}
	
	return -1;
}

// for adjusting bouquet's channel list after numzap or quickzap
void CChannelList::setSelected( int nChannelNr)
{
	selected = nChannelNr;
}

// -- Zap to channel with channel_id
bool CChannelList::zapTo_ChannelID(const t_channel_id channel_id)
{
	dprintf(DEBUG_NORMAL, "CChannelList::zapTo_ChannelID %llx\n", channel_id);
	
	for (unsigned int i = 0; i < chanlist.size(); i++) 
	{
		if (chanlist[i]->channel_id == channel_id) 
		{
			zapTo(i);
			return true;
		}
	}
	
	return false;
}

// forceStoreToLastChannels defaults to false
void CChannelList::zapTo(int pos, bool /*forceStoreToLastChannels*/)
{
	// show emty channellist error msg
	if (chanlist.empty()) 
	{
		if (FrontendCount >= 1) 
			MessageBox(LOCALE_MESSAGEBOX_ERROR, g_Locale->getText(LOCALE_CHANNELLIST_NONEFOUND), mbrCancel, mbCancel, NEUTRINO_ICON_ERROR);
		return;
	}

	if ( (pos >= (signed int) chanlist.size()) || (pos < 0) ) 
	{
		pos = 0;
	}

	CZapitChannel * chan = chanlist[pos];
	
	dprintf(DEBUG_NORMAL, "CChannelList::zapTo me %s tuned %d new %d %s -> %llx\n", name.c_str(), tuned, pos, chan->name.c_str(), chan->channel_id);
	
	if ( pos != (int)tuned ) 
	{  
		if ((g_settings.radiotext_enable) && ((CNeutrinoApp::getInstance()->getMode()) == NeutrinoMessages::mode_radio) && (g_Radiotext))
		{
			// stop radiotext PES decoding before zapping
			g_Radiotext->radiotext_stop();
		}		
		
		tuned = pos;
		g_RemoteControl->zapTo_ChannelID(chan->channel_id, chan->name, !chan->bAlwaysLocked); // UTF-8
		
		// TODO check is it possible bouquetList is NULL ?
		if (bouquetList != NULL) 
		{
			CNeutrinoApp::getInstance()->channelList->adjustToChannelID(chan->channel_id);
		}
		
		if(new_mode_active)
			selected = pos;
	}

	if(!new_mode_active) 
	{
		selected = pos;

		// remove recordModeActive from infobar
		if(g_settings.auto_timeshift && !CNeutrinoApp::getInstance()->recordingstatus) 
		{
			g_InfoViewer->handleMsg(NeutrinoMessages::EVT_RECORDMODE, 0);
		}

		g_RCInput->postMsg( NeutrinoMessages::SHOW_INFOBAR, 0 );
	}
}

// -1: channellist not found
int CChannelList::numericZap(int key)
{
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	int res = -1;

	if (chanlist.empty()) 
	{
		if (FrontendCount >= 1) 
			MessageBox(LOCALE_MESSAGEBOX_ERROR, g_Locale->getText(LOCALE_CHANNELLIST_NONEFOUND), mbrCancel, mbCancel, NEUTRINO_ICON_ERROR);
			
		return res;
	}

	// -- quickzap "0" (recall) to last seen channel...
	if (key == g_settings.key_lastchannel) 
	{
		t_channel_id channel_id = lastChList.getlast(1);
		if(channel_id) 
		{
			lastChList.clear_storedelay(); // ignore store delay
			zapTo_ChannelID(channel_id);
		}
		
		return res;
	}

	if (key == g_settings.key_zaphistory) 
	{
		// recording status
		if(!autoshift && CNeutrinoApp::getInstance()->recordingstatus) 
		{
			CChannelList * orgList = bouquetList->orgChannelList;
			CChannelList * channelList = new CChannelList(g_Locale->getText(LOCALE_CHANNELLIST_CURRENT_TP), false, true);
			
			t_channel_id recid = rec_channel_id >> 16;
			
			// get channels from the same tp as recording channel
			for ( unsigned int i = 0 ; i < orgList->chanlist.size(); i++) 
			{
				if((orgList->chanlist[i]->channel_id >> 16) == recid) 
				{
					channelList->addChannel(orgList->chanlist[i]);
				}
			}

			if (channelList->getSize() != 0) 
			{
				channelList->adjustToChannelID(orgList->getActiveChannel_ChannelID(), false);
				
				this->frameBuffer->paintBackground();
				
				this->frameBuffer->blit();

				res = channelList->exec();
			}
			
			delete channelList;
			return res;
		}
		
		// -- zap history bouquet, similar to "0" quickzap, but shows a menue of last channels
		if (this->lastChList.size() > 1) 
		{
			CChannelList * channelList = new CChannelList(g_Locale->getText(LOCALE_CHANNELLIST_HISTORY), true, true);

			for(unsigned int i = 1 ; i < this->lastChList.size() ; ++i) 
			{
				t_channel_id channel_id = this->lastChList.getlast(i);

				if(channel_id) 
				{
					CZapitChannel * channel = getChannel(channel_id);
					if(channel) 
						channelList->addChannel(channel);
				}
			}

			if (channelList->getSize() != 0) 
			{
				this->frameBuffer->paintBackground();

				this->frameBuffer->blit();

				res = channelList->exec();
			}
			delete channelList;
		}
		return res;
	}
	
	//TEST: PiP
	if(key == g_settings.key_pip )
	{
		CChannelList * orgList = bouquetList->orgChannelList;
		CChannelList * channelList = new CChannelList(g_Locale->getText(LOCALE_CHANNELLIST_CURRENT_TP), false, true);
			
		t_channel_id pipid = live_channel_id >> 16;
			
		for ( unsigned int i = 0 ; i < orgList->chanlist.size(); i++) 
		{
			if((orgList->chanlist[i]->channel_id >> 16) == pipid) 
			{
					channelList->addChannel(orgList->chanlist[i]);
			}
		}
			
		pip_selected = true;

		if (channelList->getSize() != 0) 
		{
			channelList->adjustToChannelID(orgList->getActiveChannel_ChannelID(), false);
			this->frameBuffer->paintBackground();

			this->frameBuffer->blit();

			res = channelList->exec();
		}
		delete channelList;
		return res;
	}
	//

	int sx = 4 * g_Font[SNeutrinoSettings::FONT_TYPE_CHANNEL_NUM_ZAP]->getRenderWidth(widest_number) + 14;
	int sy = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNEL_NUM_ZAP]->getHeight() + 6;

	int ox = frameBuffer->getScreenX() + (frameBuffer->getScreenWidth() - sx)/2;
	int oy = frameBuffer->getScreenY() + (frameBuffer->getScreenHeight() - sy)/2;
	char valstr[10];
	int chn = CRCInput::getNumericValue(key);
	int pos = 1;
	int lastchan= -1;
	bool doZap = true;
	bool showEPG = false;

	while(1) 
	{
		if (lastchan != chn) 
		{
			sprintf((char*) &valstr, "%d", chn);
			
			while(strlen(valstr)<4)
				strcat(valstr,"-");   //"_"

			frameBuffer->paintBoxRel(ox, oy, sx, sy, COL_INFOBAR_PLUS_0);

			for (int i = 3; i >= 0; i--) 
			{
				valstr[i+ 1]= 0;
				g_Font[SNeutrinoSettings::FONT_TYPE_CHANNEL_NUM_ZAP]->RenderString(ox + 7 + i*((sx - 14)>>2), oy + sy - 3, sx, &valstr[i], COL_INFOBAR);
			}
			
			frameBuffer->blit();

			// show infobar
			showInfo(chn - 1);
			lastchan = chn;
		}

		g_RCInput->getMsg( &msg, &data, g_settings.timing[SNeutrinoSettings::TIMING_NUMERICZAP] * 10 );

		if ( msg == RC_timeout ) 
		{
			if ( ( chn > (int)chanlist.size() ) || (chn == 0) )
				chn = tuned + 1;
			break;
		}
		else if (CRCInput::isNumeric(msg)) 
		{
			if (pos == 4) 
			{
				chn = 0;
				pos = 1;
			} 
			else 
			{
				chn *= 10;
				pos++;
			}
			chn += CRCInput::getNumericValue(msg);
		}
		else if ( msg == RC_ok ) 
		{
			if ( ( chn > (signed int) chanlist.size() ) || ( chn == 0 ) ) 
			{
				chn = tuned + 1;
			}
			break;
		}
		else if ( msg == (neutrino_msg_t)g_settings.key_quickzap_down ) 
		{
			if ( chn == 1 )
				chn = chanlist.size();
			else {
				chn--;

				if (chn > (int)chanlist.size())
					chn = (int)chanlist.size();
			}
		}
		else if ( msg == (neutrino_msg_t)g_settings.key_quickzap_up ) 
		{
			chn++;

			if (chn > (int)chanlist.size())
				chn = 1;
		}
		else if ( ( msg == RC_home ) || ( msg == RC_left ) || ( msg == RC_right) )
		{
			doZap = false;
			break;
		}
		else if ( msg == RC_red ) 
		{
			if ( ( chn <= (signed int) chanlist.size() ) && ( chn != 0 ) ) 
			{
				doZap = false;
				showEPG = true;
				break;
			}
		}
		else if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all ) 
		{
			doZap = false;
			break;
		}
	}

	frameBuffer->paintBackgroundBoxRel(ox, oy, sx, sy);
	
	frameBuffer->blit();

	chn--;
	
	if (chn < 0)
		chn = 0;
	
	if ( doZap ) 
	{
		// kill infobar
		if(g_settings.timing[SNeutrinoSettings::TIMING_INFOBAR] == 0)
			g_InfoViewer->killTitle();
		
		// zapto selected channel
		zapTo( chn );
	} 
	else 
	{
		// show infobar
		showInfo(tuned);
		
		// kill infobar
		g_InfoViewer->killTitle();

		if ( showEPG )
			g_EventList->exec(chanlist[chn]->channel_id, chanlist[chn]->name);
	}
	
	return res;
}

void CChannelList::virtual_zap_mode(bool up)
{
	dprintf(DEBUG_NORMAL, "CChannelList::virtual_zap_mode\n");

        neutrino_msg_t      msg;
        neutrino_msg_data_t data;

        if (chanlist.empty()) 
	{
		if (FrontendCount >= 1) 
			MessageBox(LOCALE_MESSAGEBOX_ERROR, g_Locale->getText(LOCALE_CHANNELLIST_NONEFOUND), mbrCancel, mbCancel, NEUTRINO_ICON_ERROR);
			
                return;
        }

        int chn = getActiveChannelNumber() + (up ? 1 : -1);
        if (chn > (int)chanlist.size())
		chn = 1;
	
        if (chn == 0)
		chn = (int)chanlist.size();
	
        int lastchan = -1;
        bool doZap = true;
        bool showEPG = false;
	int epgpos = 0;

        while(1)
        {
                if (lastchan != chn || (epgpos != 0))
                {
                        showInfo(chn - 1, epgpos);
                        lastchan = chn;
                }

		epgpos = 0;
                g_RCInput->getMsg( &msg, &data, 15*10 ); // 15 seconds, not user changable

                if ( msg == RC_ok )
                {
                        if ( ( chn > (signed int) chanlist.size() ) || ( chn == 0 ) )
                        {
                                chn = tuned + 1;
                        }
                        break;
                }
                else if ( msg == RC_left )
                {
                        if ( chn == 1 )
                                chn = chanlist.size();
                        else
                        {
                                chn--;

                                if (chn > (int)chanlist.size())
                                        chn = (int)chanlist.size();
                        }
                }
                else if ( msg == RC_right )
                {
                        chn++;

                        if (chn > (int)chanlist.size())
                                chn = 1;
                }
                else if ( msg == RC_up )
                {
                        epgpos = -1;
                }
                else if ( msg == RC_down )
                {
                        epgpos = 1;
                }
                else if ( ( msg == RC_home ) || ( msg == RC_timeout ) )
                {
                        // Abbruch ohne Channel zu wechseln
                        doZap = false;
                        break;
                }
                else if ( msg == RC_red )
                {
                        // Rote Taste zeigt EPG fuer gewaehlten Kanal an
                        if ( ( chn <= (signed int) chanlist.size() ) && ( chn != 0 ) )
                        {
                                doZap = false;
                                showEPG = true;
                                break;
                        }
                }
                else if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
                {
                        doZap = false;
                        break;
                }
        }
        
	g_InfoViewer->clearVirtualZapMode();

        chn--;
        if (chn < 0)
                chn = 0;

        if ( doZap )
        {
		if(g_settings.timing[SNeutrinoSettings::TIMING_INFOBAR] == 0)
			g_InfoViewer->killTitle();

                zapTo(chn);
        }
        else
        {
                showInfo(tuned);
                g_InfoViewer->killTitle();

                // Rote Taste zeigt EPG fuer gewaehlten Kanal an
                if ( showEPG )
                        g_EventList->exec(chanlist[chn]->channel_id, chanlist[chn]->name);
        }
}

void CChannelList::quickZap(int key, bool cycle)
{
        if(chanlist.size() == 0)
                return;

	if ( (key == g_settings.key_quickzap_down) || (key == RC_left) )
	{
                if(selected == 0)
                        selected = chanlist.size() - 1;
                else
                        selected--;
        }
	else if ((key == g_settings.key_quickzap_up) || (key == RC_right) )
	{
                selected = (selected + 1)%chanlist.size();
        }

	dprintf(DEBUG_NORMAL, "CChannelList::quickZap: quick zap selected = %d getActiveBouquetNumber %d\n", selected, bouquetList->getActiveBouquetNumber());

	if(cycle)
		bouquetList->orgChannelList->zapTo(bouquetList->Bouquets[bouquetList->getActiveBouquetNumber()]->channelList->getKey(selected)-1);
	else
        	zapTo(selected);

	g_RCInput->clearRCMsg(); //FIXME test for n.103
}

bool CChannelList::canZap(CZapitChannel * channel)
{
	bool iscurrent = true;

	if(!autoshift && CNeutrinoApp::getInstance()->recordingstatus) 
	{
		if(channel == NULL)
			channel = chanlist[selected];
		
		iscurrent = CanZap(channel);
	}
	
	return iscurrent;
}

#define NUM_LIST_BUTTONS 4
struct button_label CChannelListButtons[NUM_LIST_BUTTONS] =
{
	{ NEUTRINO_ICON_BUTTON_RED, LOCALE_INFOVIEWER_EVENTLIST, NULL},
	{ NEUTRINO_ICON_BUTTON_GREEN, LOCALE_INFOVIEWER_NEXT, NULL},
	{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_BOUQUETLIST_HEAD, NULL},
	{ NEUTRINO_ICON_BUTTON_BLUE, LOCALE_EPGMENU_EPGPLUS, NULL},
};

#define HEAD_BUTTONS_COUNT	3
const struct button_label HeadButtons[HEAD_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_HELP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_SETUP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_MUTE_ZAP_INACTIVE, NONEXISTANT_LOCALE, NULL }
};

const struct button_label HeadNewModeButtons[HEAD_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_HELP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_SETUP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_MUTE_ZAP_ACTIVE, NONEXISTANT_LOCALE, NULL }
};

void CChannelList::paint()
{
	dprintf(DEBUG_NORMAL, "CChannelList::paint\n");

	CChannelEvent* p_event = NULL;
	time_t jetzt = time(NULL);
	int runningPercent = 0;

	if(chanlist.size())
	{
		for(unsigned int i = 0; i < chanlist.size(); i++)
		{
			std::string desc = chanlist[i]->description;
			char cSeit[50] = " ";
			char cNoch[50] = " ";

			if (displayNext) 
			{
				p_event = &chanlist[i]->nextEvent;
			} 
			else 
			{
				p_event = &chanlist[i]->currentEvent;
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

			item = new ClistBoxEntryItem(chanlist[i]->name.c_str(), true, p_event->description.c_str());

			item->setNumber(i + 1);
			item->setPercent(runningPercent);
			item->setInfo1(desc.c_str());
			item->setOptionInfo1(cSeit);
			item->setInfo2(p_event->text.c_str());
			item->setOptionInfo2(cNoch);

			item->setIcon1(chanlist[i]->isHD() ? NEUTRINO_ICON_HD : chanlist[i]->isUHD()? NEUTRINO_ICON_UHD : "");
			item->setIcon2(chanlist[i]->scrambled ? NEUTRINO_ICON_SCRAMBLED : ""); 

			listBox->addItem(item);
		}
	}

	// head
	listBox->setTitle(name.c_str());
	listBox->enablePaintHead();
	listBox->enablePaintDate();
	listBox->setHeaderButtons(new_mode_active? HeadNewModeButtons : HeadButtons, HEAD_BUTTONS_COUNT);

	// foot
	listBox->enablePaintFoot();

	if (displayNext) 
	{
		CChannelListButtons[1].locale = LOCALE_INFOVIEWER_NOW;
	} 
	else 
	{
		CChannelListButtons[1].locale = LOCALE_INFOVIEWER_NEXT;
	}

	listBox->setFooterButtons(CChannelListButtons, NUM_LIST_BUTTONS);

	// foot info
	listBox->enablePaintFootInfo();

	//
	listBox->setSelected(selected);
	listBox->paint();
}

int CChannelList::getSize() const
{
	return this->chanlist.size();
}

int CChannelList::getSelectedChannelIndex() const
{
	return this->selected;
}

//
void CChannelList::webTVBouquets(void)
{
	dprintf(DEBUG_NORMAL, "CChannelList::Bouquets\n");

	CFileFilter fileFilter;
	
	fileFilter.addFilter("xml");
	fileFilter.addFilter("tv");
	fileFilter.addFilter("m3u");

	//
	CFileList filelist;
	CMenuWidget m("WebTV", NEUTRINO_ICON_WEBTV_SMALL, MENU_WIDTH + 100);
	m.enableSaveScreen();

	int select = -1;
	CMenuSelectorTarget * selector = new CMenuSelectorTarget(&select);
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
		
		dprintf(DEBUG_NORMAL, "CChannelList::addUserBouquet: settings file %s\n", g_settings.webtv_userBouquet.c_str());
		
		// load channels
		CNeutrinoApp::getInstance()->webTVChannelsInit();
	}
}

