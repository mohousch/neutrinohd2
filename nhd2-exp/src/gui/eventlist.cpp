/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: eventlist.cpp 2013/10/12 mohousch Exp $

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

#include "widget/hintbox.h"
#include "gui/bouquetlist.h"
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
	liststart = 0;
	sort_mode = SORT_DESCRIPTION;

	m_search_list = SEARCH_LIST_NONE;
	m_search_epg_item = SEARCH_LIST_NONE;
	m_search_epg_item = SEARCH_EPG_TITLE;
	m_search_channel_id = 1;
	m_search_bouquet_id= 1;
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
						channel_id2 = CREATE_CHANNEL_ID_FROM_SERVICE_ORIGINALNETWORK_TRANSPORTSTREAM_ID(
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

	// windows size
	width  = w_max ( (frameBuffer->getScreenWidth() / 20 * 17), (frameBuffer->getScreenWidth() / 20 ));
	height = h_max ( (frameBuffer->getScreenHeight() / 20 * 16), (frameBuffer->getScreenHeight() / 20));


	int icon_w;
	int icon_h;
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_RED, &icon_w, &icon_h);
	
	iheight = std::max(icon_h, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight()) + 10;
	
	//
	theight = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_TITLE]->getHeight();

	fheight1 = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight();
	{
		int h1 = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMSMALL]->getHeight();
		int h2 = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_DATETIME]->getHeight();
		fheight2 = (h1 > h2) ? h1 : h2;
	}
	
	fheight = fheight1 + fheight2 + 2;
	fwidth1 = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_DATETIME]->getRenderWidth("DDD, 00:00,  ");
	fwidth2 = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMSMALL]->getRenderWidth("[999 min] ");

	listmaxshow = (height - theight - iheight)/fheight;
	
	// recalculate height
	height = theight + iheight + listmaxshow*fheight; // recalc height

	x = frameBuffer->getScreenX() + (frameBuffer->getScreenWidth() - width) / 2;
	y = frameBuffer->getScreenY() + (frameBuffer->getScreenHeight() - height) / 2;

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
	paintHead(channel_id);
	paint(channel_id);
	showFunctionBar(true);
	
	// blit
	frameBuffer->blit();

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);

	int oldselected = selected;

	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_CHANLIST]);

	bool loop = true;
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd);

		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_CHANLIST]);

		if (msg == CRCInput::RC_up || (int) msg == g_settings.key_channelList_pageup)
		{
			int step = 0;
			int prev_selected = selected;

			step = ((int) msg == g_settings.key_channelList_pageup) ? listmaxshow : 1;  // browse or step 1
			selected -= step;
			if((prev_selected-step) < 0)            // because of uint
				selected = evtlist.size() - 1;

			paintItem(prev_selected - liststart, channel_id);
			unsigned int oldliststart = liststart;
			liststart = (selected/listmaxshow)*listmaxshow;

			if(oldliststart!=liststart)
				paint(channel_id);
			else
				paintItem(selected - liststart, channel_id);
		}
		else if (msg == CRCInput::RC_down || (int) msg == g_settings.key_channelList_pagedown)
		{
			unsigned int step = 0;
			int prev_selected = selected;

			step = ((int) msg == g_settings.key_channelList_pagedown) ? listmaxshow : 1;  // browse or step 1
			selected += step;

			if(selected >= evtlist.size()) 
			{
				if (((evtlist.size() / listmaxshow) + 1) * listmaxshow == evtlist.size() + listmaxshow) // last page has full entries
					selected = 0;
				else
					selected = ((step == listmaxshow) && (selected < (((evtlist.size() / listmaxshow) + 1) * listmaxshow))) ? (evtlist.size() - 1) : 0;
			}

			paintItem(prev_selected - liststart, channel_id);
			unsigned int oldliststart = liststart;
			liststart = (selected/listmaxshow)*listmaxshow;
			if(oldliststart!=liststart)
				paint(channel_id);
			else
				paintItem(selected - liststart, channel_id);
		}
		// sort
		else if (msg == (neutrino_msg_t)g_settings.key_channelList_sort)
		{
			unsigned long long selected_id = evtlist[selected].eventID;
			
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
			oldselected = selected;
			if(selected <=listmaxshow)
				liststart=0;
			else
				liststart=(selected/listmaxshow)*listmaxshow;
			
			hide();
			paintHead(channel_id);
			paint(channel_id);
			showFunctionBar(true);
		}
		// epg reload
		else if (msg == (neutrino_msg_t)g_settings.key_channelList_reload)
		{
			sort_mode = SORT_DESCRIPTION;
			hide();
			paintHead(channel_id);
			readEvents(channel_id);
			paint(channel_id);
			showFunctionBar(true);
		}
		// add record
		else if ( msg == (neutrino_msg_t)g_settings.key_channelList_addrecord )
		{
			if (recDir != NULL)
			{
				int tID = -1;
				CTimerd::CTimerEventTypes etype = isScheduled(channel_id, &evtlist[selected], &tID);
				if(etype == CTimerd::TIMER_RECORD) 
				{
					g_Timerd->removeTimerEvent(tID);
					timerlist.clear();
					g_Timerd->getTimerList(timerlist);
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
				
				paint(channel_id);
			}					
		}
		// add remind
		else if ( msg == (neutrino_msg_t) g_settings.key_channelList_addremind )		  
		{
			int tID = -1;
			CTimerd::CTimerEventTypes etype = isScheduled(channel_id, &evtlist[selected], &tID);
			
			if(etype == CTimerd::TIMER_ZAPTO) 
			{
				g_Timerd->removeTimerEvent(tID);
				timerlist.clear();
				g_Timerd->getTimerList (timerlist);
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
				paintHead(channel_id);
				readEvents(channel_id);
				paint(channel_id);
				showFunctionBar(true);
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
			
			paintHead(channel_id);
			readEvents(channel_id);
			paint(channel_id);
			showFunctionBar(true);
		}
		else if ( msg == CRCInput::RC_left )		  
		{
			loop = false;
		}
		else if ( msg == CRCInput::RC_right || msg == CRCInput::RC_ok || msg == CRCInput::RC_info)
		{
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

					paintHead(channel_id);
					paint(channel_id);
					showFunctionBar(true);
				}
			}
		}
		else if(msg == CRCInput::RC_0)
		{
			if ( evtlist[selected].eventID != 0 )
			{
				hide();

				//
				if(!evtlist[selected].description.empty())
				{
					cTmdb * tmdb = new cTmdb(evtlist[selected].description);
	
					if ((tmdb->getResults() > 0) && (!tmdb->getDescription().empty())) 
					{
						std::string buffer;

						buffer = tmdb->getTitle().c_str();
						buffer += "\n";
	
						// prepare print buffer  
						buffer += tmdb->CreateEPGText();

						// thumbnail
						int pich = 246;	//FIXME
						int picw = 162; 	//FIXME
	
						std::string thumbnail = "/tmp/" + evtlist[selected].description + ".jpg";
						if(access(thumbnail.c_str(), F_OK))
							thumbnail = "";
	
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

				paintHead(channel_id);
				paint(channel_id);
				showFunctionBar(true);
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
			// head
			paintHead(channel_id);
			paint(channel_id);
			showFunctionBar(true);
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

	return res;
}

void EventList::hide()
{
	frameBuffer->paintBackgroundBoxRel(x, y, width, height);

	frameBuffer->blit();

	showFunctionBar(false);

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

void EventList::paintItem(unsigned int pos, t_channel_id channel_id)
{
	uint8_t    color;
	fb_pixel_t bgcolor;
	int ypos = y + theight + pos*fheight;
	std::string datetime1_str, datetime2_str, duration_str;
	const char * icontype = 0;

	if (liststart + pos == selected)
	{
		color   = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
	}
	else if (liststart + pos == current_event )
	{
		color   = COL_MENUCONTENT + 1;
		bgcolor = COL_MENUCONTENT_PLUS_1;
	}
	else
	{
		color   = COL_MENUCONTENT;
		bgcolor = COL_MENUCONTENT_PLUS_0;
	}

	// paint  item box
	frameBuffer->paintBoxRel(x, ypos, width - SCROLLBAR_WIDTH, fheight, bgcolor);

	if(liststart + pos < evtlist.size())
	{
		if ( evtlist[liststart+pos].eventID != 0 )
		{
			char tmpstr[256];
			struct tm *tmStartZeit = localtime(&evtlist[liststart+pos].startTime);

			datetime1_str = g_Locale->getText(CLocaleManager::getWeekday(tmStartZeit));

			strftime(tmpstr, sizeof(tmpstr), ". %H:%M, ", tmStartZeit );
			datetime1_str += tmpstr;

			strftime(tmpstr, sizeof(tmpstr), " %d. ", tmStartZeit );
			datetime2_str = tmpstr;

			datetime2_str += g_Locale->getText(CLocaleManager::getMonth(tmStartZeit));

			datetime2_str += '.';

			if ( m_showChannel ) // show the channel if we made a event search only (which could be made through all channels ).
			{
				t_channel_id channel = evtlist[liststart+pos].get_channel_id();
				datetime2_str += "      ";
				datetime2_str += g_Zapit->getChannelName(channel);
			}

			sprintf(tmpstr, "[%d min]", evtlist[liststart+pos].duration / 60 );
			duration_str = tmpstr;
		}
		CTimerd::CTimerEventTypes etype = isScheduled(channel_id, &evtlist[liststart+pos]);
		icontype = etype == CTimerd::TIMER_ZAPTO ? NEUTRINO_ICON_BUTTON_YELLOW : etype == CTimerd::TIMER_RECORD ? NEUTRINO_ICON_BUTTON_RED : 0;

		// 1st line
		g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_DATETIME]->RenderString(x + 5, ypos + fheight1 + 3, fwidth1 + 5, datetime1_str, color, 0, true); // UTF-8

		g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_DATETIME]->RenderString(x + 5 + fwidth1, ypos+ fheight1 + 3, width - fwidth1 - 10 - 20, datetime2_str, color, 0, true); // UTF-8

		int seit = ( evtlist[liststart+pos].startTime - time(NULL) ) / 60;
		if ( (seit> 0) && (seit<100) && (duration_str.length()!=0) )
		{
			char beginnt[100];
			sprintf((char*) &beginnt, "in %d min", seit);
			int w = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMSMALL]->getRenderWidth(beginnt) + 10;

			g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMSMALL]->RenderString(x+width-fwidth2-5- 20- w, ypos+ fheight1+3, fwidth2, beginnt, color);
		}
		g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMSMALL]->RenderString(x+width-fwidth2-5- 20, ypos+ fheight1+3, fwidth2, duration_str, color, 0, true); // UTF-8
		
		// paint Icon
		int icon_w = 0;
		int icon_h = 0;
		
		if(icontype != 0)
		{
			frameBuffer->getIconSize(icontype, &icon_w, &icon_h);
			frameBuffer->paintIcon(icontype, x + 2, ypos + fheight - icon_h - (fheight1 - icon_h)/2);
		}
		
		// 2nd line
		g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->RenderString(x + 2 + icon_w + 2, ypos+ fheight, width - 25 - 20, evtlist[liststart + pos].description, color, 0, true);
	}	
}

void EventList::paintHead(t_channel_id channel_id)
{
	bool logo_ok = false;

	frameBuffer->paintBoxRel(x, y, width, theight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, g_settings.Head_gradient);
	
	// help icon
	int icon_h_w, icon_h_h;
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_HELP_SMALL, &icon_h_w, &icon_h_h);
	
	// paint time/date
	int timestr_len = 0;
	char timestr[18];
	
	time_t now = time(NULL);
	struct tm *tm = localtime(&now);
	
	bool gotTime = g_Sectionsd->getIsTimeSet();

	if(gotTime)
	{
		strftime(timestr, 18, "%d.%m.%Y %H:%M", tm);
		timestr_len = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getRenderWidth(timestr, true); // UTF-8
		
		g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->RenderString(x + width - BORDER_RIGHT - 2*icon_h_w - 5 - timestr_len, y + g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight() + 5, timestr_len+1, timestr, COL_MENUHEAD, 0, true); // UTF-8
	}

	// logo
	//channel logo
	int PIC_W_1 = theight*1.67;
	int logo_w = PIC_W_1; 
	int logo_h = theight;
	int logo_bpp = 0;
	
	// check for logo
	logo_ok = frameBuffer->checkLogo(channel_id);
	
	if(logo_ok)
	{
		// get logo size	
		frameBuffer->getLogoSize(channel_id, &logo_w, &logo_h, &logo_bpp);
		
		// display logo
		frameBuffer->DisplayLogo(channel_id, x + BORDER_LEFT, y, (logo_bpp == 4 && logo_w > PIC_W)?  PIC_W: PIC_W_1, theight, (logo_h > theight)? true : false, false, true);
		
		// title
		g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_TITLE]->RenderString(x + BORDER_LEFT + ( (logo_bpp == 4)? logo_w : PIC_W_1) + 5, y + theight + 1, width - BORDER_LEFT - ( (logo_bpp == 4)? logo_w : PIC_W_1) - BORDER_RIGHT - icon_h_w - 5 - timestr_len, name.c_str(), COL_MENUHEAD, 0, true); // UTF-8
	}
	else
		// title
		g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_TITLE]->RenderString(x + BORDER_LEFT, y + theight + 1, width - BORDER_LEFT - BORDER_RIGHT - icon_h_w - 5 - timestr_len, name.c_str(), COL_MENUHEAD, 0, true); // UTF-8
}

void EventList::paint(t_channel_id channel_id)
{
	liststart = (selected/listmaxshow)*listmaxshow;

	if (evtlist[0].eventID != 0)
	{
		int icon_w = 0;
		int icon_h = 0;
		
		// icon ?
		frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_HELP_SMALL, &icon_w, &icon_h);
		frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_HELP_SMALL, x + width - BORDER_RIGHT - icon_w, y + (theight - icon_h)/2 );

		// icon 0 for tmdb
		frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_0, &icon_w, &icon_h);
		frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_0, x + width - BORDER_RIGHT - icon_w - ICON_TO_ICON_OFFSET - icon_w, y + (theight - icon_h)/2 );
	}

	frameBuffer->paintBoxRel(x, y + theight, width, height - theight - iheight, COL_MENUCONTENT_PLUS_0);
	
	for(unsigned int count = 0; count < listmaxshow; count++)
	{
		paintItem(count, channel_id);
	}

	int ypos = y + theight;
	int sb = fheight*listmaxshow;
	frameBuffer->paintBoxRel(x + width - 15, ypos, 15, sb,  COL_MENUCONTENT_PLUS_1);

	int sbc = ((evtlist.size() - 1)/ listmaxshow)+ 1;
	float sbh= (sb- 4)/ sbc;
	int sbs= (selected/listmaxshow);

	frameBuffer->paintBoxRel(x + width- 13, ypos+ 2+ int(sbs* sbh) , 11, int(sbh),  COL_MENUCONTENT_PLUS_3);	
	
	frameBuffer->blit(); //FIXME:???
}

// footer
void  EventList::showFunctionBar(bool show)
{
	int icon_w;
	int icon_h;
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_RED, &icon_w, &icon_h);
	
	int bx = x + ICON_OFFSET;
	int bw = width - 2*ICON_OFFSET;
	int bh = iheight;
	int by = y + height - iheight;
	int cellwidth = bw / 5;// 5 cells
	int pos = 0;

	// -- hide only?
	if (! show)
	{
		frameBuffer->paintBackgroundBoxRel(x, by, width, bh);
		frameBuffer->blit();
		
		return;
	}
	
	int fh = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight();

	//
	frameBuffer->paintBoxRel(x, by, width, bh, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.Foot_gradient);

	// -- Button Red: Timer Record & Channelswitch
	if ( (recDir != NULL) && ((unsigned int) g_settings.key_channelList_addrecord != CRCInput::RC_nokey))	  
	{
		pos = 0;
	
		if ( g_settings.key_channelList_addrecord == CRCInput::RC_red ) 		  
		{
			frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_RED, bx + cellwidth*pos, by + (bh - icon_h)/2);
			g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(bx + icon_w + ICON_OFFSET + cellwidth*pos, by + (bh - fh)/2 + fh, cellwidth - icon_w - ICON_OFFSET, g_Locale->getText(LOCALE_EVENTLISTBAR_RECORDEVENT), COL_INFOBAR, 0, true); // UTF-8
		}
	}
	
	// Button: Event Search
	if ((unsigned int)g_settings.key_channelList_search != CRCInput::RC_nokey)
	{
		pos = 1;
		frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_GREEN, &icon_w, &icon_h);
		frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_GREEN, bx + cellwidth*pos, by + (bh - icon_h)/2);
		g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(bx + icon_w + ICON_OFFSET + cellwidth*pos, by + (bh - fh)/2 + fh, cellwidth - icon_w - ICON_OFFSET, g_Locale->getText(LOCALE_EVENTFINDER_SEARCH), COL_INFOBAR, 0, true); // UTF-8
	}

	// Button: Timer Channelswitch	
	if ((unsigned int) g_settings.key_channelList_addremind != CRCInput::RC_nokey)  
	{
		pos = 2;
		frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_YELLOW, &icon_w, &icon_h);
		
		// FIXME : display other icons depending on g_settings.key_channelList_addremind		
		if (g_settings.key_channelList_addremind == CRCInput::RC_yellow) 		  
		{
			frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_YELLOW, bx + cellwidth*pos, by + (bh - icon_h)/2);
			g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(bx + icon_w + ICON_OFFSET + cellwidth*pos, by + (bh - fh)/2 + fh, cellwidth - icon_w - ICON_OFFSET, g_Locale->getText(LOCALE_EVENTLISTBAR_CHANNELSWITCH), COL_INFOBAR, 0, true); // UTF-8
		}
	}

	// Button: Event Re-Sort
	if ((unsigned int) g_settings.key_channelList_sort != CRCInput::RC_nokey)
	{
		pos = 3;
		frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_BLUE, &icon_w, &icon_h);
		
		//FIXME: display other icons depending on g_settings.key_channelList_sort value
		if (g_settings.key_channelList_sort == CRCInput::RC_blue) 
		{
			frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_BLUE, bx + cellwidth*pos, by + (bh - icon_h)/2);
			g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(bx + icon_w + ICON_OFFSET + cellwidth*pos, by + (bh - fh)/2 + fh, cellwidth - icon_w - ICON_OFFSET, (sort_mode == SORT_DESCRIPTION)?g_Locale->getText(LOCALE_EVENTLISTBAR_EVENTSORTALPHA) : g_Locale->getText(LOCALE_EVENTLISTBAR_EVENTSORTTIME), COL_INFOBAR, 0, true); // UTF-8
		}
	}
	
	// Button: Event Reload/Refresh
	if ((unsigned int) g_settings.key_channelList_reload != CRCInput::RC_nokey)
	{
		pos = 4;
		frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_SETUP_SMALL, &icon_w, &icon_h);
		
		if (g_settings.key_channelList_reload == CRCInput::RC_setup) 
		{
			frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_SETUP_SMALL, bx + cellwidth*pos, by + (bh - icon_h)/2);
			g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(bx + icon_w + ICON_OFFSET + cellwidth*pos, by + (bh - fh)/2 + fh, cellwidth - icon_w - ICON_OFFSET, g_Locale->getText(LOCALE_KEYBINDINGMENU_RELOAD), COL_INFOBAR, 0, true); // UTF-8
		}
	}
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

	if(CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_iptv)
		e->exec(g_Webtv->getLiveChannelID(), g_Webtv->getLiveChannelName());
	else
		e->exec(live_channel_id, channelList->getActiveChannelName());
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
	
	paintHead(0);
	paint();
	showFunctionBar(true);
	
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

