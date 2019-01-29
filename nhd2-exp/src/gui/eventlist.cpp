/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: eventlist.cpp 18.01.2019 mohousch Exp $

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

#include <unistd.h>

#include <global.h>
#include <gui/eventlist.h>
#include <gui/timerlist.h>

#include <gui/widget/icons.h>
#include <gui/widget/messagebox.h>
#include <gui/widget/mountchooser.h>
#include <gui/pictureviewer.h>

#include <global.h>
#include <neutrino.h>

#include <gui/widget/hintbox.h>
#include <gui/bouquetlist.h>
#include <gui/widget/stringinput.h>

#include <gui/epgplus.h>

//
#include <client/zapitclient.h>

#include <driver/screen_max.h>

#include <algorithm>

#include <system/debug.h>
#include <system/tmdbparser.h>

#include <gui/widget/infobox.h>


extern CBouquetList * bouquetList;
extern t_channel_id live_channel_id;
extern char recDir[255];			// defined in neutrino.cpp

#define PIC_W 120

void sectionsd_getEventsServiceKey(t_channel_id serviceUniqueKey, CChannelEventList &eList, char search = 0, std::string search_text = "");
bool sectionsd_getActualEPGServiceKey(const t_channel_id uniqueServiceKey, CEPGData * epgdata);
bool sectionsd_getLinkageDescriptorsUniqueKey(const event_id_t uniqueKey, CSectionsdClient::LinkageDescriptorList& descriptors);

// sort operators
bool sortById(const CChannelEvent& a, const CChannelEvent& b)
{
	return a.eventID < b.eventID ;
}

bool sortByDescription(const CChannelEvent& a, const CChannelEvent& b)
{
	if(a.description == b.description)
		return a.eventID < b.eventID;
	else
		return a.description < b.description ;
}

static bool sortByDateTime(const CChannelEvent& a, const CChannelEvent& b)
{
	return a.startTime < b.startTime;
}

EventList::EventList()
{
	frameBuffer = CFrameBuffer::getInstance();
	selected = 0;
	current_event = 0;
	sort_mode = SORT_DESCRIPTION;

	m_search_list = SEARCH_LIST_NONE;
	m_search_epg_item = SEARCH_LIST_NONE;
	m_search_epg_item = SEARCH_EPG_TITLE;
	m_search_channel_id = 1;
	m_search_bouquet_id= 1;

	//
	listBox = NULL;
	item = NULL;

	// box	
	cFrameBox.iWidth = w_max ( (frameBuffer->getScreenWidth() / 20 * 17), (frameBuffer->getScreenWidth() / 20 ));
	cFrameBox.iHeight = h_max ( (frameBuffer->getScreenHeight() / 20 * 18), (frameBuffer->getScreenHeight() / 20));
	
	cFrameBox.iX = frameBuffer->getScreenX() + (frameBuffer->getScreenWidth() - cFrameBox.iWidth) / 2;
	cFrameBox.iY = frameBuffer->getScreenY() + (frameBuffer->getScreenHeight() - cFrameBox.iHeight) / 2;		
}

EventList::~EventList()
{
}

void EventList::readEvents(const t_channel_id channel_id)
{
	evtlist.clear();
	sectionsd_getEventsServiceKey(channel_id &0xFFFFFFFFFFFFULL, evtlist);
	time_t azeit = time(NULL);

	CChannelEventList::iterator e;
	
	if ( evtlist.size() != 0 ) 
	{
		CEPGData epgData;
		
		// todo: what if there are more than one events in the Portal
		if (sectionsd_getActualEPGServiceKey(channel_id&0xFFFFFFFFFFFFULL, &epgData))
		{
			CSectionsdClient::LinkageDescriptorList	linkedServices;

			if ( sectionsd_getLinkageDescriptorsUniqueKey( epgData.eventID, linkedServices ) )
			{
				if ( linkedServices.size() > 1 )
				{
					CChannelEventList evtlist2; // stores the temporary eventlist of the subchannel channelid
					t_channel_id channel_id2;
				
					for (unsigned int i = 0; i < linkedServices.size(); i++)
					{
						channel_id2 = CREATE_CHANNEL_ID(
								linkedServices[i].serviceId,
								linkedServices[i].originalNetworkId,
								linkedServices[i].transportStreamId);
							
						// do not add parent events
						if (channel_id != channel_id2) 
						{
							evtlist2.clear();
							sectionsd_getEventsServiceKey(channel_id2 &0xFFFFFFFFFFFFULL, evtlist2);

							for (unsigned int loop = 0 ; loop < evtlist2.size(); loop++ )
							{
								//FIXME: bad ?evtlist2[loop].sub = true;
								evtlist.push_back(evtlist2[loop]);
							}
							evtlist2.clear();
						}
					}
				}
			}
		}
		
		// Houdini added for Private Premiere EPG, start sorted by start date/time
		sort(evtlist.begin(), evtlist.end(), sortByDateTime);
		
  		// Houdini: dirty workaround for RTL double events, remove them
  		CChannelEventList::iterator e2;
  		for ( e = evtlist.begin(); e != evtlist.end(); ++e )
  		{
  			e2 = e + 1;
  			if ( e2 != evtlist.end() && (e->startTime == e2->startTime)) 
			{
  				evtlist.erase(e2);
  			}
  		}
		timerlist.clear();
		g_Timerd->getTimerList(timerlist);

	}
	
	current_event = (unsigned int) - 1;
	for ( e = evtlist.begin(); e != evtlist.end(); ++e )
	{
		if ( e->startTime > azeit ) 
		{
			break;
		}
		current_event++;
	}

	if ( evtlist.size() == 0 )
	{
		CChannelEvent evt;

		evt.description = g_Locale->getText(LOCALE_EPGLIST_NOEVENTS);
		evt.eventID = 0;
		evtlist.push_back(evt);

	}
	
	if (current_event == (unsigned int) - 1)
		current_event = 0;
	selected = current_event;

	return;
}

int EventList::exec(const t_channel_id channel_id, const std::string& channelname) // UTF-8
{
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;
	bool in_search = 0;

	int res = menu_return::RETURN_REPAINT;
	
	if(m_search_list == SEARCH_LIST_NONE) // init globals once only
	{
		m_search_epg_item = SEARCH_EPG_TITLE;
		m_search_list = SEARCH_LIST_CHANNEL;
		m_search_bouquet_id= bouquetList->getActiveBouquetNumber();
	}
	
	m_search_channel_id = channel_id;
	m_showChannel = false; // do not show the channel in normal mode, we just need it in search mode

	name = channelname;
	sort_mode = SORT_DESCRIPTION;
	
	//
	readEvents(channel_id);

	//
	listBox = new ClistBoxEntry(&cFrameBox);
	paint(channel_id);
	
	// blit
	frameBuffer->blit();

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);

	int oldselected = selected;

	uint64_t timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_CHANLIST]);

	bool loop = true;
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd);

		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_CHANLIST]);

		if (msg == CRCInput::RC_up)
		{
			listBox->scrollLineUp();
		}
		else if (msg == CRCInput::RC_down)
		{
			listBox->scrollLineDown();
		}
		else if (msg == CRCInput::RC_page_up)
		{
			listBox->scrollPageUp();
		}
		else if (msg == CRCInput::RC_page_down)
		{
			listBox->scrollPageDown();
		}
		// sort
		else if (msg == (neutrino_msg_t)g_settings.key_channelList_sort)
		{
			uint64_t selected_id = evtlist[selected].eventID;
			
			if(sort_mode == SORT_DESCRIPTION) // by description
			{
				sort_mode++;
				sort(evtlist.begin(), evtlist.end(), sortByDescription);
			}
			else// datetime
			{
				sort_mode = SORT_DESCRIPTION;
				sort(evtlist.begin(), evtlist.end(), sortByDateTime);
			}
			
			// find selected
			for ( selected = 0 ; selected < evtlist.size(); selected++ )
			{
				if ( evtlist[selected].eventID == selected_id )
					break;
			}

			hide();
			listBox->clearItems();
			paint(channel_id);
		}
		// epg reload
		else if (msg == (neutrino_msg_t)g_settings.key_channelList_reload)
		{
			sort_mode = SORT_DESCRIPTION;
			hide();

			listBox->clearItems();
			paint(channel_id);			
		}
		// add record
		else if ( msg == (neutrino_msg_t)g_settings.key_channelList_addrecord )
		{
			//oldselected = selected;
			selected = listBox->getSelected();

			if (recDir != NULL)
			{
				int tID = -1;
				CTimerd::CTimerEventTypes etype = isScheduled(channel_id, &evtlist[selected], &tID);
				if(etype == CTimerd::TIMER_RECORD) 
				{
					g_Timerd->removeTimerEvent(tID);
					timerlist.clear();
					g_Timerd->getTimerList(timerlist);

					listBox->clearItems();
					paint(channel_id);
					continue;
				}
				
				if (recDir != NULL)
				{
					if (g_Timerd->addRecordTimerEvent(channel_id,
								evtlist[selected].startTime,
								evtlist[selected].startTime + evtlist[selected].duration,
								evtlist[selected].eventID, evtlist[selected].startTime,
								evtlist[selected].startTime - (ANNOUNCETIME + 120),
								TIMERD_APIDS_CONF, true, recDir,false) == -1)
					{
						if(askUserOnTimerConflict(evtlist[selected].startTime - (ANNOUNCETIME + 120), evtlist[selected].startTime + evtlist[selected].duration))
						{
							g_Timerd->addRecordTimerEvent(channel_id,
									evtlist[selected].startTime,
									evtlist[selected].startTime + evtlist[selected].duration,
									evtlist[selected].eventID, evtlist[selected].startTime,
									evtlist[selected].startTime - (ANNOUNCETIME + 120),
									TIMERD_APIDS_CONF, true, recDir,true);
									
							MessageBox(LOCALE_TIMER_EVENTRECORD_TITLE, LOCALE_TIMER_EVENTRECORD_MSG, CMessageBox::mbrBack, CMessageBox::mbBack, NEUTRINO_ICON_INFO);
						}
					} 
					else 
					{
						MessageBox(LOCALE_TIMER_EVENTRECORD_TITLE, LOCALE_TIMER_EVENTRECORD_MSG, CMessageBox::mbrBack, CMessageBox::mbBack, NEUTRINO_ICON_INFO);
					}
				}
				timerlist.clear();
				g_Timerd->getTimerList(timerlist);
				
				//
				listBox->clearItems();
				paint(channel_id);
			}					
		}
		// add remind
		else if ( msg == (neutrino_msg_t) g_settings.key_channelList_addremind )		  
		{
			selected = listBox->getSelected();

			int tID = -1;
			CTimerd::CTimerEventTypes etype = isScheduled(channel_id, &evtlist[selected], &tID);
			
			if(etype == CTimerd::TIMER_ZAPTO) 
			{
				g_Timerd->removeTimerEvent(tID);
				timerlist.clear();
				g_Timerd->getTimerList(timerlist);

				listBox->clearItems();
				paint(channel_id);
				continue;
			}

			g_Timerd->addZaptoTimerEvent(channel_id, 
						evtlist[selected].startTime,
						evtlist[selected].startTime - ANNOUNCETIME, 0,
						evtlist[selected].eventID, evtlist[selected].startTime, 0);
					
			MessageBox(LOCALE_TIMER_EVENTTIMED_TITLE, LOCALE_TIMER_EVENTTIMED_MSG, CMessageBox::mbrBack, CMessageBox::mbBack, NEUTRINO_ICON_INFO);
			timerlist.clear();
			g_Timerd->getTimerList (timerlist);
			
			//
			listBox->clearItems();
			paint(channel_id);
		}
		else if (msg == CRCInput::RC_timeout)
		{
			selected = oldselected;
			loop = false;
		}
		else if (msg == (neutrino_msg_t)g_settings.key_channelList_cancel) 
		{
			if(in_search) 
			{
				in_search = false;
				name = channelname;

				listBox->clearItems();
				paint(channel_id);
			} 
			else 
			{
				selected = oldselected;
				loop = false;
			}
		}
		else if (msg == CRCInput::RC_epg)
		{
			hide();
			CEPGplusHandler eplus;
			eplus.exec(NULL, "");
			
			readEvents(channel_id);
			paint(channel_id);
		}
		else if ( msg == CRCInput::RC_left )		  
		{
			loop = false;
		}
		else if ( msg == CRCInput::RC_right || msg == CRCInput::RC_ok || msg == CRCInput::RC_info)
		{
			selected = listBox->getSelected();

			if ( evtlist[selected].eventID != 0 )
			{
				hide();

				res = g_EpgData->show(channel_id, evtlist[selected].eventID, &evtlist[selected].startTime);
				
				if ( res == menu_return::RETURN_EXIT_ALL )
				{
					loop = false;
				}
				else
				{
					g_RCInput->getMsg( &msg, &data, 0 );

					if ( ( msg != CRCInput::RC_red ) && ( msg != CRCInput::RC_timeout ) )
					{
						// RC_red schlucken
						g_RCInput->postMsg( msg, data );
					}
					timerlist.clear();
					g_Timerd->getTimerList (timerlist);

					listBox->clearItems();
					paint(channel_id);
				}
			}
		}
		else if(msg == CRCInput::RC_0)
		{
			selected = listBox->getSelected();

			if ( evtlist[selected].eventID != 0 )
			{
				hide();

				//
				if(!evtlist[selected].description.empty())
				{
					CTmdb * tmdb = new CTmdb();

					tmdb->getMovieInfo(evtlist[selected].description);
	
					if ((!tmdb->getDescription().empty())) 
					{
						std::string buffer;

						buffer = evtlist[selected].description;
						buffer += "\n";
	
						// prepare print buffer  
						buffer += tmdb->createInfoText();

						// thumbnail
						int pich = 246;	//FIXME
						int picw = 162; 	//FIXME

						std::string thumbnail = "";
						std::string tname = tmdb->getThumbnailDir();
						tname += "/";
						tname += evtlist[selected].description;
						tname += ".jpg";

						tmdb->getSmallCover(tmdb->getPosterPath(), tname);

						if(!access(tname.c_str(), F_OK))
							thumbnail = tname;
	
						CBox position(g_settings.screen_StartX + 50, g_settings.screen_StartY + 50, g_settings.screen_EndX - g_settings.screen_StartX - 100, g_settings.screen_EndY - g_settings.screen_StartY - 100); 
	
						CInfoBox * infoBox = new CInfoBox("", g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1], CTextBox::SCROLL, &position, "", g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE], NEUTRINO_ICON_TMDB);

						infoBox->setText(&buffer, thumbnail, picw, pich);
						infoBox->exec();
						delete infoBox;
					}
					else
						MessageBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_STREAMINFO_NOT_AVAILABLE), CMessageBox::mbrBack, CMessageBox::mbBack, NEUTRINO_ICON_INFO);

					delete tmdb;
					tmdb = NULL;	

				}

				listBox->clearItems();
				paint(channel_id);
			}	
		}
		else if ( msg == CRCInput::RC_green )
		{
			in_search = findEvents();
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_CHANLIST]);
		}
		else if (msg == CRCInput::RC_sat || msg == CRCInput::RC_favorites)
		{
			g_RCInput->postMsg (msg, 0);
			res = menu_return::RETURN_EXIT_ALL;
			loop = false;
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
				res = menu_return::RETURN_EXIT_ALL;
			}
		}

		// blit
		frameBuffer->blit();	
	}

	hide();

	//
	g_RCInput->killTimer(sec_timer_id);
	sec_timer_id = 0;

	delete listBox;
	listBox = NULL;

	return res;
}

void EventList::hide()
{
	listBox->hide();
}

CTimerd::CTimerEventTypes EventList::isScheduled(t_channel_id channel_id, CChannelEvent * event, int * tID)
{
	CTimerd::TimerList::iterator timer = timerlist.begin();
	
	for(; timer != timerlist.end(); timer++) 
	{
		if(timer->channel_id == channel_id && (timer->eventType == CTimerd::TIMER_ZAPTO || timer->eventType == CTimerd::TIMER_RECORD)) 
		{
			if(timer->epgID == event->eventID) 
			{
				if(timer->epg_starttime == event->startTime) 
				{
					if(tID)
						*tID = timer->eventID;
					return timer->eventType;
				}
			}
		}
	}

	return (CTimerd::CTimerEventTypes) 0;
}

#define NUM_LIST_BUTTONS 5
struct button_label FootButtons[NUM_LIST_BUTTONS] =
{
	{ NEUTRINO_ICON_BUTTON_RED, LOCALE_EVENTLISTBAR_RECORDEVENT, NULL},
	{ NEUTRINO_ICON_BUTTON_GREEN, LOCALE_EVENTFINDER_SEARCH, NULL},
	{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_EVENTLISTBAR_CHANNELSWITCH, NULL},
	{ NEUTRINO_ICON_BUTTON_BLUE, LOCALE_EVENTLISTBAR_EVENTSORTTIME, NULL},
	{ NEUTRINO_ICON_BUTTON_SETUP_SMALL, LOCALE_KEYBINDINGMENU_RELOAD, NULL}
};

struct button_label HeadButtons = {NEUTRINO_ICON_BUTTON_HELP_SMALL, NONEXISTANT_LOCALE, NULL};

void EventList::paint(t_channel_id channel_id)
{
	dprintf(DEBUG_NORMAL, "EventList::paint\n");

	for (unsigned int count = 0; count < evtlist.size(); count++)
	{
		item = new ClistBoxEntryItem(evtlist[count].description.c_str(), true);

		//
		std::string datetime1_str, datetime2_str, duration_str;
		std::string icontype;
		icontype.clear();

		// option
		if ( evtlist[count].eventID != 0 )
		{
			char tmpstr[256];
			struct tm *tmStartZeit = localtime(&evtlist[count].startTime);

			datetime1_str = g_Locale->getText(CLocaleManager::getWeekday(tmStartZeit));

			strftime(tmpstr, sizeof(tmpstr), ". %H:%M, ", tmStartZeit );
			datetime1_str += tmpstr;

			strftime(tmpstr, sizeof(tmpstr), " %d. ", tmStartZeit );
			datetime1_str += tmpstr;

			datetime1_str += g_Locale->getText(CLocaleManager::getMonth(tmStartZeit));

			datetime1_str += '.';

			if ( m_showChannel ) // show the channel if we made a event search only (which could be made through all channels ).
			{
				t_channel_id channel = evtlist[count].get_channel_id();
				datetime1_str += "      ";
				datetime1_str += g_Zapit->getChannelName(channel);
			}

			sprintf(tmpstr, "[%d min]", evtlist[count].duration / 60 );
			duration_str = tmpstr;
		}

		item->setOption(datetime1_str.c_str());

		int seit = ( evtlist[count].startTime - time(NULL) ) / 60;
		if ( (seit> 0) && (seit<100) && (duration_str.length()!=0) )
		{
			char beginnt[100];
			sprintf((char*) &beginnt, "in %d min ", seit);

			datetime2_str = beginnt;
		}

		datetime2_str += duration_str;

		item->setOptionInfo(datetime2_str.c_str());
		
		// icon
		CTimerd::CTimerEventTypes etype = isScheduled(channel_id, &evtlist[count]);
		icontype = etype == CTimerd::TIMER_ZAPTO ? NEUTRINO_ICON_BUTTON_YELLOW : etype == CTimerd::TIMER_RECORD ? NEUTRINO_ICON_BUTTON_RED : "";

		item->setIconName(icontype.c_str());
		item->setnLinesItem();

		if(count == current_event)
			item->setMarked(true);

		listBox->addItem(item);
	}

	// head
	std::string logo;

	logo = frameBuffer->getLogoName(channel_id);

	listBox->setTitle(name.c_str(), logo.c_str(), true);
	listBox->enablePaintHead();
	listBox->enablePaintDate();
	listBox->setHeaderButtons(&HeadButtons, 1);

	// foot
	listBox->enablePaintFoot();

	if(sort_mode == SORT_DESCRIPTION)
		FootButtons[3].locale = LOCALE_EVENTLISTBAR_EVENTSORTALPHA;
	else
		FootButtons[3].locale = LOCALE_EVENTLISTBAR_EVENTSORTTIME;

	listBox->setFooterButtons(FootButtons, NUM_LIST_BUTTONS);

	//
	listBox->setSelected(selected);
	listBox->paint(/*reinit*/);
}

int CEventListHandler::exec(CMenuTarget* parent, const std::string &/*actionKey*/)
{
	dprintf(DEBUG_NORMAL, "CEventListHandler::exec:\n");

	int res = menu_return::RETURN_REPAINT;
	EventList* e;
	CChannelList* channelList;

	if (parent)
		parent->hide();

	e = new EventList;

	channelList = CNeutrinoApp::getInstance()->channelList;

	e->exec(channelList->getActiveChannel_ChannelID(), channelList->getActiveChannelName());

	delete e;

	return res;
}

int EventList::findEvents(void)
{
	int res = 0;
	int event = 0;
	t_channel_id channel_id;  //g_Zapit->getCurrentServiceID()
	
	CEventFinderMenu menu(&event, &m_search_epg_item, &m_search_keyword, &m_search_list, &m_search_channel_id, &m_search_bouquet_id);
	hide();
	menu.exec(NULL, "");
	
	if(event == 1)
	{
		res = 1;
		m_showChannel = true;   // force the event list to paint the channel name
		
		evtlist.clear();
		
		if(m_search_list == SEARCH_LIST_CHANNEL)
		{
			sectionsd_getEventsServiceKey(m_search_channel_id & 0xFFFFFFFFFFFFULL, evtlist, m_search_epg_item,m_search_keyword);
		}
		else if(m_search_list == SEARCH_LIST_BOUQUET)
		{
			int channel_nr = bouquetList->Bouquets[m_search_bouquet_id]->channelList->getSize();
			for(int channel = 0; channel < channel_nr; channel++)
			{
				channel_id = bouquetList->Bouquets[m_search_bouquet_id]->channelList->getChannelFromIndex(channel)->channel_id;
				
				sectionsd_getEventsServiceKey(channel_id & 0xFFFFFFFFFFFFULL, evtlist, m_search_epg_item,m_search_keyword);
			}
		}
		else if(m_search_list == SEARCH_LIST_ALL)
		{
			CHintBox box(LOCALE_TIMING_EPG,g_Locale->getText(LOCALE_EVENTFINDER_SEARCHING));
			box.paint();
			int bouquet_nr = bouquetList->Bouquets.size();
			
			for(int bouquet = 0; bouquet < bouquet_nr; bouquet++)
			{
				int channel_nr = bouquetList->Bouquets[bouquet]->channelList->getSize();
				for(int channel = 0; channel < channel_nr; channel++)
				{
					channel_id = bouquetList->Bouquets[bouquet]->channelList->getChannelFromIndex(channel)->channel_id;
					
					sectionsd_getEventsServiceKey(channel_id & 0xFFFFFFFFFFFFULL,evtlist, m_search_epg_item,m_search_keyword);
				}
			}
			box.hide();
		}
		
		sort(evtlist.begin(), evtlist.end(), sortByDateTime);
		current_event = (unsigned int)-1;
		time_t azeit=time(NULL);
		
		CChannelEventList::iterator e;
		for ( e = evtlist.begin(); e != evtlist.end(); ++e )
		{
			if ( e->startTime > azeit ) 
			{
				break;
			}
			current_event++;
		}
		
		if(evtlist.empty())
		{
			if ( evtlist.size() == 0 )
			{
				CChannelEvent evt;

				evt.description = g_Locale->getText(LOCALE_EPGVIEWER_NOTFOUND);
				evt.eventID = 0;
				evtlist.push_back(evt);
			}
		}            
		if (current_event == (unsigned int)-1)
			current_event = 0;
		selected= current_event;
		
		name = g_Locale->getText(LOCALE_EVENTFINDER_SEARCH);
		name += ": '";
		name += m_search_keyword;
		name += "'";
	}
	
	listBox->clearItems();
	paint();
	
	return(res);
}
  
#define SEARCH_LIST_OPTION_COUNT 3
const CMenuOptionChooser::keyval SEARCH_LIST_OPTIONS[SEARCH_LIST_OPTION_COUNT] =
{
	{ EventList::SEARCH_LIST_CHANNEL     , LOCALE_TIMERLIST_CHANNEL, NULL },
	{ EventList::SEARCH_LIST_BOUQUET     , LOCALE_BOUQUETLIST_HEAD, NULL },
	{ EventList::SEARCH_LIST_ALL         , LOCALE_CHANNELLIST_HEAD, NULL }
};

#define SEARCH_EPG_OPTION_COUNT 3
const CMenuOptionChooser::keyval SEARCH_EPG_OPTIONS[SEARCH_EPG_OPTION_COUNT] =
{
	{ EventList::SEARCH_EPG_TITLE       , LOCALE_FONTSIZE_EPG_TITLE, NULL },
	{ EventList::SEARCH_EPG_INFO1     	, LOCALE_FONTSIZE_EPG_INFO1, NULL },
	{ EventList::SEARCH_EPG_INFO2       , LOCALE_FONTSIZE_EPG_INFO2, NULL }
};

CEventFinderMenu::CEventFinderMenu(int* event, int* search_epg_item, std::string* search_keyword, int* search_list, t_channel_id* search_channel_id, t_bouquet_id * search_bouquet_id)
{
	m_event = event;
	m_search_epg_item   = search_epg_item;
	m_search_keyword	= search_keyword;
	m_search_list       = search_list;
	m_search_channel_id = search_channel_id;
	m_search_bouquet_id = search_bouquet_id;
}

int CEventFinderMenu::exec(CMenuTarget * parent, const std::string &actionKey)
{
	int res = menu_return::RETURN_REPAINT;
	
	if(actionKey == "")
	{
		if(parent != NULL)
			parent->hide();

		showMenu();
	}
	else if(actionKey == "1")
	{
		*m_event = true;
		res = menu_return::RETURN_EXIT_ALL;
	}	
	else if(actionKey == "2")
	{
		//printf("2\n");
		
		/*
		if(*m_search_list == EventList::SEARCH_LIST_CHANNEL)
		{
			mf[1]->setActive(true);
			m_search_channelname = g_Zapit->getChannelName(*m_search_channel_id);;
		}
		else if(*m_search_list == EventList::SEARCH_LIST_BOUQUET)
		{
			mf[1]->setActive(true);
			m_search_channelname = bouquetList->Bouquets[*m_search_bouquet_id]->channelList->getName();
		}
		else if(*m_search_list == EventList::SEARCH_LIST_ALL)
		{
			mf[1]->setActive(false);
			m_search_channelname = "";
		}
		*/
	}	
	else if(actionKey == "3")
	{
		// get channel id / bouquet id
		if(*m_search_list == EventList::SEARCH_LIST_CHANNEL)
		{
			int nNewChannel;
			int nNewBouquet;
			nNewBouquet = bouquetList->show();
			
			//printf("new_bouquet_id %d\n",nNewBouquet);
			
			if (nNewBouquet > -1)
			{
				nNewChannel = bouquetList->Bouquets[nNewBouquet]->channelList->show();
				//printf("nNewChannel %d\n",nNewChannel);
				if (nNewChannel > -1)
				{
					*m_search_bouquet_id = nNewBouquet;
					*m_search_channel_id = bouquetList->Bouquets[nNewBouquet]->channelList->getActiveChannel_ChannelID();
					m_search_channelname = g_Zapit->getChannelName(*m_search_channel_id);
				}
			}
		}
		else if(*m_search_list == EventList::SEARCH_LIST_BOUQUET)
		{
			int nNewBouquet;
			nNewBouquet = bouquetList->show();
			//printf("new_bouquet_id %d\n",nNewBouquet);
			if (nNewBouquet > -1)
			{
				*m_search_bouquet_id = nNewBouquet;
				m_search_channelname = bouquetList->Bouquets[nNewBouquet]->channelList->getName();
			}
		}
	}	
	else if(actionKey =="4")
	{
		//printf("4\n");
	}	
	
	return res;
}

int CEventFinderMenu::showMenu(void)
{
	dprintf(DEBUG_NORMAL, "CEventFinderMenu::showMenu:\n");

	int res = menu_return::RETURN_REPAINT;
	*m_event = false;
	
	if(*m_search_list == EventList::SEARCH_LIST_CHANNEL)
	{
		m_search_channelname = g_Zapit->getChannelName(*m_search_channel_id);
	}
	else if(*m_search_list == EventList::SEARCH_LIST_BOUQUET)
	{
		m_search_channelname = bouquetList->Bouquets[*m_search_bouquet_id]->channelList->getName();
	}
	else if(*m_search_list == EventList::SEARCH_LIST_ALL)
	{
		m_search_channelname =="";
	}
	
	CStringInputSMS stringInput(LOCALE_EVENTFINDER_KEYWORD, m_search_keyword);
	
	CMenuForwarder * mf2 = new CMenuForwarder(LOCALE_EVENTFINDER_KEYWORD, true, *m_search_keyword, &stringInput, NULL, CRCInput::RC_1 );
	CMenuOptionChooser * mo0 = new CMenuOptionChooser(LOCALE_EVENTFINDER_SEARCH_WITHIN_LIST, m_search_list, SEARCH_LIST_OPTIONS, SEARCH_LIST_OPTION_COUNT, true, NULL, CRCInput::RC_2);
	CMenuForwarder * mf1 = new CMenuForwarder("", *m_search_list != EventList::SEARCH_LIST_ALL, m_search_channelname, this, "3", CRCInput::RC_3 );
	CMenuOptionChooser * mo1 = new CMenuOptionChooser(LOCALE_EVENTFINDER_SEARCH_WITHIN_EPG, m_search_epg_item, SEARCH_EPG_OPTIONS, SEARCH_EPG_OPTION_COUNT, true, NULL, CRCInput::RC_4);
	CMenuForwarder * mf0 = new CMenuForwarder(LOCALE_EVENTFINDER_START_SEARCH, true, NULL, this, "1", CRCInput::RC_5 );
	
	CMenuWidget searchMenu(LOCALE_EVENTFINDER_HEAD, NEUTRINO_ICON_FEATURES);

        searchMenu.addItem(mf2, false);
        searchMenu.addItem(new CMenuSeparator(CMenuSeparator::LINE));
        searchMenu.addItem(mo0, false);
        searchMenu.addItem(mf1, false);
        searchMenu.addItem(mo1, false);
        searchMenu.addItem(new CMenuSeparator(CMenuSeparator::LINE));
        searchMenu.addItem(mf0, false);
	
	res = searchMenu.exec(NULL, "");
	
	return(res);
}


