/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: timerlist.cpp 2013/09/03 10:45:30 mohousch Exp $

	Timerliste by Zwen

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

#include <global.h>
#include <neutrino.h>

#include <gui/timerlist.h>
#include <gui/pluginlist.h>
#include <gui/plugins.h>

#include <daemonc/remotecontrol.h>

#include <driver/encoding.h>
#include <driver/fontrenderer.h>
#include <driver/rcinput.h>
#include <driver/screen_max.h>
#include <driver/color.h>

#include <gui/eventlist.h>
#include <gui/infoviewer.h>
#include <gui/channellist.h>

#include <gui/widget/widget_helpers.h>
#include <gui/widget/hintbox.h>
#include <gui/widget/icons.h>
#include <gui/widget/menue.h>
#include <gui/widget/messagebox.h>
#include <gui/widget/stringinput.h>
#include <gui/widget/stringinput_ext.h>
#include <gui/widget/mountchooser.h>

#include <system/settings.h>
#include <system/fsmounter.h>
#include <system/debug.h>

//
#include <client/zapitclient.h>
#include <channel.h>
#include <bouquets.h>

#include <gui/channel_select.h>


extern char recDir[255];			// defined in neutrino.cpp

#include <string.h>


class CTimerListNewNotifier : public CChangeObserver
{
	private:
		CMenuItem* m1;
		CMenuItem* m2;
		CMenuItem* m3;
		CMenuItem* m4;
		CMenuItem* m5;
		CMenuItem* m6;
		char* display;
		int* iType;
		time_t* stopTime;
	public:
		CTimerListNewNotifier( int *Type, time_t *time, CMenuItem *a1, CMenuItem *a2, CMenuItem *a3, CMenuItem *a4, CMenuItem *a5, CMenuItem *a6, char *d)
		{
			m1 = a1;
			m2 = a2;
			m3 = a3;
			m4 = a4;
			m5 = a5;
			m6 = a6;
			display = d;
			iType = Type;
			stopTime = time;
		}
		
		bool changeNotify(const neutrino_locale_t /*OptionName*/, void *)
		{
			CTimerd::CTimerEventTypes type = (CTimerd::CTimerEventTypes) *iType;
			
			if(type == CTimerd::TIMER_RECORD)
			{
				*stopTime = (time(NULL)/60)*60;
				struct tm *tmTime2 = localtime(stopTime);
				sprintf( display, "%02d.%02d.%04d %02d:%02d", tmTime2->tm_mday, tmTime2->tm_mon + 1,
							tmTime2->tm_year + 1900,
							tmTime2->tm_hour, tmTime2->tm_min);
				m1->setActive(true);
				m6->setActive((recDir != NULL)? true: false);
			}
			else
			{
				*stopTime = 0;
				strcpy(display,"                ");
				m1->setActive (false);
				m6->setActive(false);
			}
			
			if(type == CTimerd::TIMER_RECORD ||
				type == CTimerd::TIMER_ZAPTO ||
				type == CTimerd::TIMER_NEXTPROGRAM)
			{
				m2->setActive(true);
			}
			else
			{
				m2->setActive(false);
			}
			
			if(type == CTimerd::TIMER_STANDBY)
				m3->setActive(true);
			else
				m3->setActive(false);
			
			if(type == CTimerd::TIMER_REMIND)
				m4->setActive(true);
			else
				m4->setActive(false);
			
			if(type == CTimerd::TIMER_EXEC_PLUGIN)
				m5->setActive(true);
			else
				m5->setActive(false);
			
			return true;
		}
};

class CTimerListRepeatNotifier : public CChangeObserver
{
	private:
		CMenuForwarder *m1;
		CMenuForwarder *m2;

		int* iRepeat;
	public:
		CTimerListRepeatNotifier( int *repeat, CMenuForwarder *a1, CMenuForwarder *a2)
		{
			m1 = a1;
			m2 = a2;
			iRepeat = repeat;
		}

		bool changeNotify(const neutrino_locale_t /*OptionName*/, void *)
		{
			if(*iRepeat >= (int)CTimerd::TIMERREPEAT_WEEKDAYS)
				m1->setActive (true);
			else
				m1->setActive (false);
			
			if (*iRepeat != (int)CTimerd::TIMERREPEAT_ONCE)
				m2->setActive(true);
			else
				m2->setActive(false);
			
			return true;
		}
};

class CTimerListApidNotifier : public CChangeObserver
{
	private:
		int *o_dflt;
		int *o_std;
		int *o_alt;
		int *o_ac3;
		CMenuItem *m_dflt;
		CMenuItem *m_std;
		CMenuItem *m_alt;
		CMenuItem *m_ac3;
		
	public:
		CTimerListApidNotifier( int *o1, int *o2, int *o3, int *o4)
		{
			o_dflt = o1;
			o_std = o2;
			o_alt = o3;
			o_ac3 = o4;
		}

		void setItems(CMenuItem *m1, CMenuItem *m2, CMenuItem *m3, CMenuItem *m4)
		{
			m_dflt = m1;
			m_std = m2;
			m_alt = m3;
			m_ac3 = m4;
		}

		bool changeNotify(const neutrino_locale_t OptionName, void *)
		{
			if(OptionName == LOCALE_TIMERLIST_APIDS_DFLT)
			{
				if(*o_dflt == 0)
				{
					m_std->setActive(true);
					m_alt->setActive(true);
					m_ac3->setActive(true);
				}
				else
				{
					m_std->setActive(false);
					m_alt->setActive(false);
					m_ac3->setActive(false);
					*o_std = 0;
					*o_alt = 0;
					*o_ac3 = 0;
				}
			}
			else
			{
				if(*o_std || *o_alt || *o_ac3)
					*o_dflt = 0;
			}
			return true;
		}
};

CTimerList::CTimerList()
{
	frameBuffer = CFrameBuffer::getInstance();

	visible = false;
	selected = 0;
	skipEventID = 0;

	//
	listBox = NULL;
	item = NULL;

	// box	
	cFrameBox.iWidth = w_max ( (frameBuffer->getScreenWidth() / 20 * 17), (frameBuffer->getScreenWidth() / 20 ));
	cFrameBox.iHeight = h_max ( (frameBuffer->getScreenHeight() / 20 * 18), (frameBuffer->getScreenHeight() / 20));
	
	cFrameBox.iX = frameBuffer->getScreenX() + (frameBuffer->getScreenWidth() - cFrameBox.iWidth) / 2;
	cFrameBox.iY = frameBuffer->getScreenY() + (frameBuffer->getScreenHeight() - cFrameBox.iHeight) / 2;		
}

CTimerList::~CTimerList()
{
	timerlist.clear();
}

int CTimerList::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CTimerList::exec: actionKey:%s\n", actionKey.c_str());

	if(parent)
		parent->hide();

	const char * key = actionKey.c_str();
	
	CSelectChannelWidget*  CSelectChannelWidgetHandler;
	
	if(actionKey == "tv")
	{
		CSelectChannelWidgetHandler = new CSelectChannelWidget();
		CSelectChannelWidgetHandler->exec(NULL, "tv");
		
		timerNew_chan_id = CSelectChannelWidgetHandler->getChanTVID();
		timerNew_channel_name = g_Zapit->getChannelName(CSelectChannelWidgetHandler->getChanTVID());
		
		delete CSelectChannelWidgetHandler;
		CSelectChannelWidgetHandler = NULL;
		
		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "radio")
	{
		CSelectChannelWidgetHandler = new CSelectChannelWidget();
		CSelectChannelWidgetHandler->exec(NULL, "radio");
		
		timerNew_chan_id = CSelectChannelWidgetHandler->getChanRadioID();
		timerNew_channel_name = g_Zapit->getChannelName(CSelectChannelWidgetHandler->getChanRadioID());
		
		delete CSelectChannelWidgetHandler;
		CSelectChannelWidgetHandler = NULL;
		
		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "webtv")
	{
		CSelectChannelWidgetHandler = new CSelectChannelWidget();
		CSelectChannelWidgetHandler->exec(NULL, "webtv");
		
		timerNew_chan_id = CSelectChannelWidgetHandler->getChanWebTVID();
		timerNew_channel_name = g_WebTV->getChannelName(CSelectChannelWidgetHandler->getChanWebTVID());
		
		delete CSelectChannelWidgetHandler;
		CSelectChannelWidgetHandler = NULL;
		
		return menu_return::RETURN_REPAINT;
	}
	else if (strcmp(key, "modifytimer") == 0)
	{
		selected = listBox->getSelected();

		timerlist[selected].announceTime = timerlist[selected].alarmTime -60;
		if(timerlist[selected].eventRepeat >= CTimerd::TIMERREPEAT_WEEKDAYS)
			g_Timerd->getWeekdaysFromStr(&timerlist[selected].eventRepeat, m_weekdaysStr.c_str());
		
		if(timerlist[selected].eventType == CTimerd::TIMER_RECORD)
		{
			timerlist[selected].announceTime -= 120; // 2 more mins for rec timer
			if (timer_apids_dflt)
				timerlist[selected].apids = TIMERD_APIDS_CONF;
			else
				timerlist[selected].apids = (timer_apids_std * TIMERD_APIDS_STD) | (timer_apids_ac3 * TIMERD_APIDS_AC3) |
					(timer_apids_alt * TIMERD_APIDS_ALT);
			g_Timerd->modifyTimerAPid(timerlist[selected].eventID,timerlist[selected].apids);
			g_Timerd->modifyRecordTimerEvent(timerlist[selected].eventID, timerlist[selected].announceTime,
						      timerlist[selected].alarmTime,
						      timerlist[selected].stopTime, timerlist[selected].eventRepeat,
						      timerlist[selected].repeatCount,timerlist[selected].recordingDir);
		} 
		else
		{
			g_Timerd->modifyTimerEvent(timerlist[selected].eventID, timerlist[selected].announceTime,
						timerlist[selected].alarmTime,
						timerlist[selected].stopTime, timerlist[selected].eventRepeat,
						timerlist[selected].repeatCount);
		}
		
		return menu_return::RETURN_EXIT;
	}
	else if (strcmp(key, "newtimer") == 0)
	{
		timerNew.announceTime = timerNew.alarmTime - 60;
		CTimerd::EventInfo eventinfo;
		CTimerd::RecordingInfo recinfo;
		
		eventinfo.epgID = 0;
		eventinfo.epg_starttime = 0;
		eventinfo.channel_id = timerNew.channel_id;
		eventinfo.apids = TIMERD_APIDS_CONF;
		eventinfo.recordingSafety = false;
		timerNew.standby_on = (timerNew_standby_on == 1);
		void *data = NULL;
		
		if(timerNew.eventType == CTimerd::TIMER_STANDBY)
			data = &(timerNew.standby_on);
		else if(timerNew.eventType == CTimerd::TIMER_NEXTPROGRAM ||
			timerNew.eventType == CTimerd::TIMER_ZAPTO ||
			timerNew.eventType == CTimerd::TIMER_RECORD)
		{
			if(timerNew_channel_name.empty())
				return menu_return::RETURN_REPAINT;
			else
				timerNew.channel_id = timerNew_chan_id;
			
			if (timerNew.eventType == CTimerd::TIMER_RECORD)
			{
				recinfo.epgID = 0;
				recinfo.epg_starttime = 0;
				recinfo.channel_id = timerNew.channel_id;
				recinfo.apids = TIMERD_APIDS_CONF;
				recinfo.recordingSafety = false;

				timerNew.announceTime -= 120; // 2 more mins for rec timer
				strncpy(recinfo.recordingDir, timerNew.recordingDir, sizeof(recinfo.recordingDir));
				data = &recinfo;
			} 
			else
			{
				eventinfo.epgID = 0;
				eventinfo.epg_starttime = 0;
				eventinfo.channel_id = timerNew.channel_id;
				//eventinfo.apids = TIMERD_APIDS_CONF;
				//eventinfo.recordingSafety = false;
		
				data = &eventinfo;
			}
		}
		else if(timerNew.eventType == CTimerd::TIMER_REMIND)
			data = timerNew.message;
		else if (timerNew.eventType == CTimerd::TIMER_EXEC_PLUGIN)
		{
			if (strcmp(timerNew.pluginName, "") == 0)
				return menu_return::RETURN_REPAINT;
			
			data = timerNew.pluginName;
		}
		
		if(timerNew.eventRepeat >= CTimerd::TIMERREPEAT_WEEKDAYS)
			g_Timerd->getWeekdaysFromStr(&timerNew.eventRepeat, m_weekdaysStr.c_str());

		if (g_Timerd->addTimerEvent(timerNew.eventType,data,timerNew.announceTime,timerNew.alarmTime, timerNew.stopTime,timerNew.eventRepeat,timerNew.repeatCount,false) == -1)
		{
			bool forceAdd = askUserOnTimerConflict(timerNew.announceTime,timerNew.stopTime);

			if (forceAdd)
			{
				g_Timerd->addTimerEvent(timerNew.eventType,data,timerNew.announceTime,timerNew.alarmTime, timerNew.stopTime, timerNew.eventRepeat,timerNew.repeatCount,true);
			}
		}
		
		return menu_return::RETURN_EXIT;
	}

	int ret = show();

	return ret;
}

void CTimerList::updateEvents(void)
{
	timerlist.clear();
	g_Timerd->getTimerList(timerlist);
	
	//Remove last deleted event from List
	CTimerd::TimerList::iterator timer = timerlist.begin();
	for(; timer != timerlist.end(); timer++)
	{
		if(timer->eventID == skipEventID)
		{
			timerlist.erase(timer);
			break;
		}
	}
	sort(timerlist.begin(), timerlist.end());
}

int CTimerList::show()
{
	dprintf(DEBUG_NORMAL, "CTimerList::show\n");

	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	int res = menu_return::RETURN_REPAINT;

	//
	listBox = new ClistBox(&cFrameBox);

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);

	uint64_t timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);

	bool loop = true;
	bool update = true;
	
	while(loop)
	{
		if(update)
		{
			hide();
			updateEvents();
			update = false;
			listBox->clearItems();
			paint();
		}

		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if( msg <= RC_MaxRC )
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);

		if( ( msg == RC_timeout ) || ( msg == RC_home) )
		{ 
			//Exit after timeout or cancel key
			loop = false;
		}
		else if ((msg == RC_up) && !(timerlist.empty()))
		{
			listBox->scrollLineUp();
		}
		else if ((msg == RC_down) && !(timerlist.empty()))
		{
			listBox->scrollLineDown();
		}
		else if ((msg == RC_ok) && !(timerlist.empty()))
		{
			if (modifyTimer() == menu_return::RETURN_EXIT_ALL)
			{
				res = menu_return::RETURN_EXIT_ALL;
				loop = false;
			}
			else
				update = true;
		}
		else if((msg == RC_red) && !(timerlist.empty()))
		{
			selected = listBox->getSelected();

			g_Timerd->removeTimerEvent(timerlist[selected].eventID);
			skipEventID = timerlist[selected].eventID;
			update = true;
		}
		else if(msg == RC_green)
		{
			if (newTimer() == menu_return::RETURN_EXIT_ALL)
			{
				res = menu_return::RETURN_EXIT_ALL;
				loop = false;
			}
			else
				update = true;
		}
		else if(msg == RC_yellow)
		{
			update = true;
		}
		else if((msg == RC_blue)|| (CRCInput::isNumeric(msg)) )
		{
			//pushback key if...
			g_RCInput->postMsg( msg, data );
			loop = false;
		}
		else if(msg == RC_setup)
		{
			res = menu_return::RETURN_EXIT_ALL;
			loop = false;
		}
		else if(msg == RC_info)
		{
			selected = listBox->getSelected();

			CTimerd::responseGetTimer *timer = &timerlist[selected];
			if(timer != NULL)
			{
				if(timer->eventType == CTimerd::TIMER_RECORD || timer->eventType == CTimerd::TIMER_ZAPTO)
				{
					hide();
					res = g_EpgData->show(timer->channel_id, timer->epgID, &timer->epg_starttime);
					if(res == menu_return::RETURN_EXIT_ALL)
						loop = false;
					else
					{
						listBox->clearItems();
						paint();
					}
				}
			}
		}
		else if (msg == RC_sat || msg == RC_favorites)
		{
			g_RCInput->postMsg (msg, 0);
			loop = false;
			res = menu_return::RETURN_EXIT_ALL;
		}
		else if ( (msg == NeutrinoMessages::EVT_TIMER) && (data == sec_timer_id) )
		{
			listBox->paintHead();
		} 
		else
		{
			if( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
			{
				loop = false;
				res = menu_return::RETURN_EXIT_ALL;
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
	
	CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);

	return(res);
}

void CTimerList::hide()
{
	if(visible)
	{
		listBox->hide();
		
		visible = false;
	}
}

bool sectionsd_getEPGid(const event_id_t epgID, const time_t startzeit, CEPGData * epgdata);

const struct button_label TimerListButtons[4] =
{
	{ NEUTRINO_ICON_BUTTON_RED   , LOCALE_TIMERLIST_DELETE },
	{ NEUTRINO_ICON_BUTTON_GREEN , LOCALE_TIMERLIST_NEW    },
	{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_TIMERLIST_RELOAD },
	{ NEUTRINO_ICON_BUTTON_OKAY, LOCALE_TIMERLIST_MODIFY }
};

struct button_label CTimerListHeadButtons = {NEUTRINO_ICON_BUTTON_HELP_SMALL, NONEXISTANT_LOCALE, NULL};

void CTimerList::paint()
{
	dprintf(DEBUG_NORMAL, "CTimerList::paint\n");

	for (unsigned int count = 0; count < timerlist.size(); count++)
	{
		std::string alarm("");

		CTimerd::responseGetTimer &timer = timerlist[count];
		char zAlarmTime[25] = {0};
		struct tm *alarmTime = localtime(&(timer.alarmTime));
		strftime(zAlarmTime, 20, "%d.%m. %H:%M", alarmTime);
		char zStopTime[25] = {0};
		struct tm *stopTime = localtime(&(timer.stopTime));
		strftime(zStopTime,20, "%d.%m. %H:%M", stopTime);

		alarm = zAlarmTime;

		if(timer.stopTime != 0)
		{
			alarm += " ";
			alarm += zStopTime;
		}

		// event repeat
		alarm += " ";
		alarm += convertTimerRepeat2String(timer.eventRepeat);

		if (timer.eventRepeat != CTimerd::TIMERREPEAT_ONCE)
		{
			char srepeatcount[25] = {0};
	
			if (timer.repeatCount != 0)
			// Unicode 8734 (hex: 221E) not available in all fonts
			// sprintf(srepeatcount,"∞");
				sprintf(srepeatcount, "00");
			else
				sprintf(srepeatcount, "%ux", timer.repeatCount);

			//alarm += " ";
			//alarm += srepeatcount;
			//FIXME: dont need to show repeatscount
		}

		//
		std::string zAddData("");
		switch(timer.eventType)
		{
			case CTimerd::TIMER_NEXTPROGRAM :
			case CTimerd::TIMER_ZAPTO :
			case CTimerd::TIMER_RECORD :
				{
					zAddData = convertChannelId2String(timer.channel_id); // UTF-8
					if(timer.apids != TIMERD_APIDS_CONF)
					{
						std::string sep = "";
						zAddData += " (";
						if(timer.apids & TIMERD_APIDS_STD)
						{
							zAddData += "STD";
							sep = "/";
						}
						if(timer.apids & TIMERD_APIDS_ALT)
						{
							zAddData += sep;
							zAddData += "ALT";
							sep = "/";
						}
						if(timer.apids & TIMERD_APIDS_AC3)
						{
							zAddData += sep;
							zAddData += "AC3";
							sep = "/";
						}
						zAddData += ')';
					}

					if(timer.epgID != 0)
					{
						CEPGData epgdata;
						
						if (sectionsd_getEPGid(timer.epgID, timer.epg_starttime, &epgdata))
						{
							zAddData += " : ";
							zAddData += epgdata.title;
						}
						else if(strlen(timer.epgTitle)!=0)
						{
							zAddData += " : ";
							zAddData += timer.epgTitle;
						}
					}
					else if(strlen(timer.epgTitle)!=0)
					{
						zAddData += " : ";
						zAddData += timer.epgTitle;
					}
				}
				break;
			case CTimerd::TIMER_STANDBY:
				{
					zAddData = g_Locale->getText(timer.standby_on ? LOCALE_TIMERLIST_STANDBY_ON : LOCALE_TIMERLIST_STANDBY_OFF);
					break;
				}
			case CTimerd::TIMER_REMIND :
				{
					zAddData = timer.message; // must be UTF-8 encoded !
				}
				break;
			case CTimerd::TIMER_EXEC_PLUGIN :
			{
				zAddData = timer.pluginName;
			}
			break;
			default:{}
		}

		item = new ClistBoxEntryItem(alarm.c_str());
		item->setnLinesItem();
		item->setOption(zAddData.c_str());
		item->setOptionInfo(convertTimerType2String(timer.eventType));

		listBox->addItem(item);
	}

	// head
	listBox->setTitle(g_Locale->getText(LOCALE_TIMERLIST_NAME), NEUTRINO_ICON_TIMER);
	listBox->enablePaintHead();
	listBox->enablePaintDate();
	listBox->setHeaderButtons(&CTimerListHeadButtons, 1);

	// foot
	listBox->enablePaintFoot();
	listBox->setFooterButtons(TimerListButtons, 4);

	//
	listBox->setSelected(selected);
	listBox->paint(/*reinit*/);

	visible = true;
}

const char * CTimerList::convertTimerType2String(const CTimerd::CTimerEventTypes type) // UTF-8
{
	switch(type)
	{
		case CTimerd::TIMER_SHUTDOWN    : return g_Locale->getText(LOCALE_TIMERLIST_TYPE_SHUTDOWN   );
		case CTimerd::TIMER_NEXTPROGRAM : return g_Locale->getText(LOCALE_TIMERLIST_TYPE_NEXTPROGRAM);
		case CTimerd::TIMER_ZAPTO       : return g_Locale->getText(LOCALE_TIMERLIST_TYPE_ZAPTO      );
		case CTimerd::TIMER_STANDBY     : return g_Locale->getText(LOCALE_TIMERLIST_TYPE_STANDBY    );
		case CTimerd::TIMER_RECORD      : return g_Locale->getText(LOCALE_TIMERLIST_TYPE_RECORD     );
		case CTimerd::TIMER_REMIND      : return g_Locale->getText(LOCALE_TIMERLIST_TYPE_REMIND     );
		case CTimerd::TIMER_SLEEPTIMER  : return g_Locale->getText(LOCALE_TIMERLIST_TYPE_SLEEPTIMER );
		case CTimerd::TIMER_EXEC_PLUGIN : return g_Locale->getText(LOCALE_TIMERLIST_TYPE_EXECPLUGIN );
		default                         : return g_Locale->getText(LOCALE_TIMERLIST_TYPE_UNKNOWN    );
	}
}

std::string CTimerList::convertTimerRepeat2String(const CTimerd::CTimerEventRepeat rep) // UTF-8
{
	switch(rep)
	{
		case CTimerd::TIMERREPEAT_ONCE               : return g_Locale->getText(LOCALE_TIMERLIST_REPEAT_ONCE              );
		case CTimerd::TIMERREPEAT_DAILY              : return g_Locale->getText(LOCALE_TIMERLIST_REPEAT_DAILY             );
		case CTimerd::TIMERREPEAT_WEEKLY             : return g_Locale->getText(LOCALE_TIMERLIST_REPEAT_WEEKLY            );
		case CTimerd::TIMERREPEAT_BIWEEKLY           : return g_Locale->getText(LOCALE_TIMERLIST_REPEAT_BIWEEKLY          );
		case CTimerd::TIMERREPEAT_FOURWEEKLY         : return g_Locale->getText(LOCALE_TIMERLIST_REPEAT_FOURWEEKLY        );
		case CTimerd::TIMERREPEAT_MONTHLY            : return g_Locale->getText(LOCALE_TIMERLIST_REPEAT_MONTHLY           );
		case CTimerd::TIMERREPEAT_BYEVENTDESCRIPTION : return g_Locale->getText(LOCALE_TIMERLIST_REPEAT_BYEVENTDESCRIPTION);
		default:
			if(rep >=CTimerd::TIMERREPEAT_WEEKDAYS)
			{
				int weekdays = (((int)rep) >> 9);
				std::string weekdayStr = "";
				if(weekdays & 1)
					weekdayStr+= g_Locale->getText(LOCALE_TIMERLIST_REPEAT_MONDAY);
				weekdays >>= 1;
				if(weekdays & 1)
					weekdayStr+= g_Locale->getText(LOCALE_TIMERLIST_REPEAT_TUESDAY);
				weekdays >>= 1;
				if(weekdays & 1)
					weekdayStr+= g_Locale->getText(LOCALE_TIMERLIST_REPEAT_WEDNESDAY);
				weekdays >>= 1;
				if(weekdays & 1)
					weekdayStr+= g_Locale->getText(LOCALE_TIMERLIST_REPEAT_THURSDAY);
				weekdays >>= 1;
				if(weekdays & 1)
					weekdayStr+= g_Locale->getText(LOCALE_TIMERLIST_REPEAT_FRIDAY);
				weekdays >>= 1;
				if(weekdays & 1)
					weekdayStr+= g_Locale->getText(LOCALE_TIMERLIST_REPEAT_SATURDAY);
				weekdays >>= 1;
				if(weekdays & 1)
					weekdayStr+= g_Locale->getText(LOCALE_TIMERLIST_REPEAT_SUNDAY);
				return weekdayStr;
			}
			else
				return g_Locale->getText(LOCALE_TIMERLIST_REPEAT_UNKNOWN);
	}
}

std::string CTimerList::convertChannelId2String(const t_channel_id id) // UTF-8
{
	
	std::string name;

	if(CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_webtv)
		name = g_WebTV->getChannelName(id);
	else
		name = g_Zapit->getChannelName(id); // UTF-8

	if (name.empty())
		name = g_Locale->getText(LOCALE_TIMERLIST_PROGRAM_UNKNOWN);

	return name;
}

#define TIMERLIST_REPEAT_OPTION_COUNT 7
const CMenuOptionChooser::keyval TIMERLIST_REPEAT_OPTIONS[TIMERLIST_REPEAT_OPTION_COUNT] =
{
	{ CTimerd::TIMERREPEAT_ONCE       , LOCALE_TIMERLIST_REPEAT_ONCE, NULL       },
	{ CTimerd::TIMERREPEAT_DAILY      , LOCALE_TIMERLIST_REPEAT_DAILY, NULL      },
	{ CTimerd::TIMERREPEAT_WEEKLY     , LOCALE_TIMERLIST_REPEAT_WEEKLY, NULL     },
	{ CTimerd::TIMERREPEAT_BIWEEKLY   , LOCALE_TIMERLIST_REPEAT_BIWEEKLY, NULL   },
	{ CTimerd::TIMERREPEAT_FOURWEEKLY , LOCALE_TIMERLIST_REPEAT_FOURWEEKLY, NULL },
	{ CTimerd::TIMERREPEAT_MONTHLY    , LOCALE_TIMERLIST_REPEAT_MONTHLY, NULL    },
	{ CTimerd::TIMERREPEAT_WEEKDAYS   , LOCALE_TIMERLIST_REPEAT_WEEKDAYS, NULL   }
};

#define TIMERLIST_STANDBY_OPTION_COUNT 2
const CMenuOptionChooser::keyval TIMERLIST_STANDBY_OPTIONS[TIMERLIST_STANDBY_OPTION_COUNT] =
{
	{ 0 , LOCALE_TIMERLIST_STANDBY_OFF, NULL },
	{ 1 , LOCALE_TIMERLIST_STANDBY_ON, NULL  }
};

#if 1
#define TIMERLIST_TYPE_OPTION_COUNT 7
#else
#define TIMERLIST_TYPE_OPTION_COUNT 8
#endif
const CMenuOptionChooser::keyval TIMERLIST_TYPE_OPTIONS[TIMERLIST_TYPE_OPTION_COUNT] =
{
	{ CTimerd::TIMER_SHUTDOWN   , LOCALE_TIMERLIST_TYPE_SHUTDOWN, NULL    },
#if 0
	{ CTimerd::TIMER_NEXTPROGRAM, LOCALE_TIMERLIST_TYPE_NEXTPROGRAM, NULL },
#endif
	{ CTimerd::TIMER_ZAPTO      , LOCALE_TIMERLIST_TYPE_ZAPTO, NULL       },
	{ CTimerd::TIMER_STANDBY    , LOCALE_TIMERLIST_TYPE_STANDBY, NULL     },
	{ CTimerd::TIMER_RECORD     , LOCALE_TIMERLIST_TYPE_RECORD, NULL      },
	{ CTimerd::TIMER_SLEEPTIMER , LOCALE_TIMERLIST_TYPE_SLEEPTIMER, NULL  },
	{ CTimerd::TIMER_REMIND     , LOCALE_TIMERLIST_TYPE_REMIND, NULL      },
	{ CTimerd::TIMER_EXEC_PLUGIN, LOCALE_TIMERLIST_TYPE_EXECPLUGIN, NULL  }
};

#define MESSAGEBOX_NO_YES_OPTION_COUNT 2
const CMenuOptionChooser::keyval MESSAGEBOX_NO_YES_OPTIONS[MESSAGEBOX_NO_YES_OPTION_COUNT] =
{
	{ 0, LOCALE_MESSAGEBOX_NO, NULL  },
	{ 1, LOCALE_MESSAGEBOX_YES, NULL }
};

int CTimerList::modifyTimer()
{
	selected = listBox->getSelected();

	CTimerd::responseGetTimer* timer = &timerlist[selected];

	ClistBoxWidget timerSettings(LOCALE_TIMERLIST_MENUMODIFY, NEUTRINO_ICON_SETTINGS);
	timerSettings.enablePaintDate();
	timerSettings.enableShrinkMenu();
	
	// intros
	timerSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	timerSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	
	//
	timerSettings.addItem(new CMenuForwarder(LOCALE_TIMERLIST_SAVE, true, NULL, this, "modifytimer", RC_red, NEUTRINO_ICON_BUTTON_RED));
	timerSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));

	char type[80];
	strcpy(type, convertTimerType2String(timer->eventType)); // UTF
	CMenuForwarder *m0 = new CMenuForwarder(LOCALE_TIMERLIST_TYPE, false, type);
	timerSettings.addItem( m0);

	CDateInput timerSettings_alarmTime(LOCALE_TIMERLIST_ALARMTIME, &timer->alarmTime , LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2);
	CMenuForwarder *m1 = new CMenuForwarder(LOCALE_TIMERLIST_ALARMTIME, true, timerSettings_alarmTime.getValue(), &timerSettings_alarmTime );
	timerSettings.addItem( m1);

	CDateInput timerSettings_stopTime(LOCALE_TIMERLIST_STOPTIME, &timer->stopTime , LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2);
	if(timer->stopTime != 0)
	{
		CMenuForwarder *m2 = new CMenuForwarder(LOCALE_TIMERLIST_STOPTIME, true, timerSettings_stopTime.getValue(), &timerSettings_stopTime );
		timerSettings.addItem( m2);
	}

	g_Timerd->setWeekdaysToStr(timer->eventRepeat, (char *)m_weekdaysStr.c_str());
	timer->eventRepeat = (CTimerd::CTimerEventRepeat)(((int)timer->eventRepeat) & 0x1FF);
	CStringInput timerSettings_weekdays(LOCALE_TIMERLIST_WEEKDAYS, (char *)m_weekdaysStr.c_str(), 7, LOCALE_TIMERLIST_WEEKDAYS_HINT_1, LOCALE_TIMERLIST_WEEKDAYS_HINT_2, "-X");
	CMenuForwarder *m4 = new CMenuForwarder(LOCALE_TIMERLIST_WEEKDAYS, ((int)timer->eventRepeat) >= (int)CTimerd::TIMERREPEAT_WEEKDAYS, m_weekdaysStr.c_str(), &timerSettings_weekdays );
	CIntInput timerSettings_repeatCount(LOCALE_TIMERLIST_REPEATCOUNT, (int&)timer->repeatCount,3, LOCALE_TIMERLIST_REPEATCOUNT_HELP1, LOCALE_TIMERLIST_REPEATCOUNT_HELP2);

	CMenuForwarder *m5 = new CMenuForwarder(LOCALE_TIMERLIST_REPEATCOUNT, timer->eventRepeat != (int)CTimerd::TIMERREPEAT_ONCE, timerSettings_repeatCount.getValue(), &timerSettings_repeatCount);

	CTimerListRepeatNotifier notifier((int *)&timer->eventRepeat, m4, m5);
	CMenuOptionChooser * m3 = new CMenuOptionChooser(LOCALE_TIMERLIST_REPEAT, (int *)&timer->eventRepeat, TIMERLIST_REPEAT_OPTIONS, TIMERLIST_REPEAT_OPTION_COUNT, true, &notifier);

	//printf("TIMER: rec dir %s len %s\n", timer->recordingDir, strlen(timer->recordingDir));

	if(!strlen(timer->recordingDir))
		strncpy(timer->recordingDir, g_settings.network_nfs_recordingdir, sizeof(timer->recordingDir));

	CMountChooser recDirs(LOCALE_TIMERLIST_RECORDING_DIR, NEUTRINO_ICON_SETTINGS, NULL, timer->recordingDir, g_settings.network_nfs_recordingdir);

	if (!recDirs.hasItem())
	{
		dprintf(DEBUG_NORMAL, "CTimerList::modifyTimer: warning: no network devices available\n");
	}
	
	bool recDirEnabled = recDirs.hasItem() && (timer->eventType == CTimerd::TIMER_RECORD) && (recDir != NULL);
	CMenuForwarder *m6 = new CMenuForwarder(LOCALE_TIMERLIST_RECORDING_DIR, recDirEnabled, timer->recordingDir, &recDirs);

	timerSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	timerSettings.addItem(m3);
	timerSettings.addItem(m4);
	timerSettings.addItem(m5);
	timerSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	timerSettings.addItem(m6);

	CMenuWidget timerSettings_apids(LOCALE_TIMERLIST_APIDS, NEUTRINO_ICON_SETTINGS);
	CTimerListApidNotifier apid_notifier(&timer_apids_dflt, &timer_apids_std, &timer_apids_ac3, &timer_apids_alt);
	timer_apids_dflt = (timer->apids == 0) ? 1 : 0 ;
	timer_apids_std = (timer->apids & TIMERD_APIDS_STD) ? 1 : 0 ;
	timer_apids_ac3 = (timer->apids & TIMERD_APIDS_AC3) ? 1 : 0 ;
	timer_apids_alt = (timer->apids & TIMERD_APIDS_ALT) ? 1 : 0 ;

	timerSettings_apids.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	timerSettings_apids.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	CMenuOptionChooser* ma1 = new CMenuOptionChooser(LOCALE_TIMERLIST_APIDS_DFLT, &timer_apids_dflt, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, &apid_notifier);
	timerSettings_apids.addItem(ma1);
	CMenuOptionChooser* ma2 = new CMenuOptionChooser(LOCALE_RECORDINGMENU_APIDS_STD, &timer_apids_std, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, &apid_notifier);
	timerSettings_apids.addItem(ma2);
	CMenuOptionChooser* ma3 = new CMenuOptionChooser(LOCALE_RECORDINGMENU_APIDS_ALT, &timer_apids_alt, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, &apid_notifier);
	timerSettings_apids.addItem(ma3);
	CMenuOptionChooser* ma4 = new CMenuOptionChooser(LOCALE_RECORDINGMENU_APIDS_AC3, &timer_apids_ac3, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, &apid_notifier);
	timerSettings_apids.addItem(ma4);
	apid_notifier.setItems(ma1,ma2,ma3,ma4);
	if(timer->eventType ==  CTimerd::TIMER_RECORD)
	{  
		timerSettings.addItem( new CMenuForwarder(LOCALE_TIMERLIST_APIDS, true, NULL, &timerSettings_apids ));
	}

	return timerSettings.exec(this, "");
}

int CTimerList::newTimer()
{
	// Defaults
	timerNew.eventType = CTimerd::TIMER_RECORD ;
	timerNew.eventRepeat = CTimerd::TIMERREPEAT_ONCE ;
	timerNew.repeatCount = 0;
	timerNew.alarmTime = (time(NULL)/60)*60;
	timerNew.stopTime = (time(NULL)/60)*60;
	timerNew.channel_id = 0;
	strcpy(timerNew.message, "");
	timerNew_standby_on = false;
	strncpy(timerNew.recordingDir, g_settings.network_nfs_recordingdir, sizeof(timerNew.recordingDir));

	ClistBoxWidget timerSettings(LOCALE_TIMERLIST_MENUNEW, NEUTRINO_ICON_SETTINGS);
	timerSettings.enablePaintDate();
	timerSettings.enableShrinkMenu();
	
	// intros
	timerSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	timerSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	
	//
	timerSettings.addItem(new CMenuForwarder(LOCALE_TIMERLIST_SAVE, true, NULL, this, "newtimer", RC_red, NEUTRINO_ICON_BUTTON_RED));
	timerSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));

	CDateInput timerSettings_alarmTime(LOCALE_TIMERLIST_ALARMTIME, &(timerNew.alarmTime) , LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2);
	CMenuForwarder *m1 = new CMenuForwarder(LOCALE_TIMERLIST_ALARMTIME, true, timerSettings_alarmTime.getValue(), &timerSettings_alarmTime );

	CDateInput timerSettings_stopTime(LOCALE_TIMERLIST_STOPTIME, &(timerNew.stopTime) , LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2);
	CMenuForwarder *m2 = new CMenuForwarder(LOCALE_TIMERLIST_STOPTIME, true, timerSettings_stopTime.getValue(), &timerSettings_stopTime );

	CStringInput timerSettings_weekdays(LOCALE_TIMERLIST_WEEKDAYS, (char *)m_weekdaysStr.c_str(), 7, LOCALE_TIMERLIST_WEEKDAYS_HINT_1, LOCALE_TIMERLIST_WEEKDAYS_HINT_2, "-X");
	CMenuForwarder *m4 = new CMenuForwarder(LOCALE_TIMERLIST_WEEKDAYS, false, m_weekdaysStr.c_str(), &timerSettings_weekdays);

	CIntInput timerSettings_repeatCount(LOCALE_TIMERLIST_REPEATCOUNT, (int&)timerNew.repeatCount, 3, LOCALE_TIMERLIST_REPEATCOUNT_HELP1, LOCALE_TIMERLIST_REPEATCOUNT_HELP2);
	CMenuForwarder *m5 = new CMenuForwarder(LOCALE_TIMERLIST_REPEATCOUNT, false, timerSettings_repeatCount.getValue(), &timerSettings_repeatCount);

	CTimerListRepeatNotifier notifier((int *)&timerNew.eventRepeat, m4, m5);
	
	CMenuOptionChooser* m3 = new CMenuOptionChooser(LOCALE_TIMERLIST_REPEAT, (int *)&timerNew.eventRepeat, TIMERLIST_REPEAT_OPTIONS, TIMERLIST_REPEAT_OPTION_COUNT, true, &notifier);
	
	CMenuForwarder *m6 = new CMenuForwarder(LOCALE_TIMERLIST_CHANNEL, true, timerNew_channel_name.c_str(), this, CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_tv? "tv" : CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_radio? "radio" : "webtv");

	CMountChooser recDirs(LOCALE_TIMERLIST_RECORDING_DIR,NEUTRINO_ICON_SETTINGS, NULL, timerNew.recordingDir, g_settings.network_nfs_recordingdir);
	if (!recDirs.hasItem())
	{
		dprintf(DEBUG_NORMAL, "CTimerList::modifyTimer: warning: no network devices available\n");
	}
	CMenuForwarder* m7 = new CMenuForwarder(LOCALE_TIMERLIST_RECORDING_DIR, recDirs.hasItem(), timerNew.recordingDir, &recDirs);

	CMenuOptionChooser* m8 = new CMenuOptionChooser(LOCALE_TIMERLIST_STANDBY, &timerNew_standby_on, TIMERLIST_STANDBY_OPTIONS, TIMERLIST_STANDBY_OPTION_COUNT, false);

	CStringInputSMS timerSettings_msg(LOCALE_TIMERLIST_MESSAGE, timerNew.message);
	CMenuForwarder *m9 = new CMenuForwarder(LOCALE_TIMERLIST_MESSAGE, false, timerNew.message, &timerSettings_msg );

	CPluginChooser plugin_chooser(timerNew.pluginName);
	CMenuForwarder *m10 = new CMenuForwarder(LOCALE_TIMERLIST_PLUGIN, false, timerNew.pluginName, &plugin_chooser);

	CTimerListNewNotifier notifier2((int *)&timerNew.eventType,
					&timerNew.stopTime, m2, m6, m8, m9, m10, m7,
					timerSettings_stopTime.getValue());
	CMenuOptionChooser *m0 = new CMenuOptionChooser(LOCALE_TIMERLIST_TYPE, (int *)&timerNew.eventType, TIMERLIST_TYPE_OPTIONS, TIMERLIST_TYPE_OPTION_COUNT, true, &notifier2);

	timerSettings.addItem( m0);
	timerSettings.addItem( m1);
	timerSettings.addItem( m2);
	timerSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	timerSettings.addItem( m3);
	timerSettings.addItem( m4);
	timerSettings.addItem( m5);
	timerSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	timerSettings.addItem( m6);
	timerSettings.addItem( m7);
	timerSettings.addItem( m8);
	timerSettings.addItem( m9);
	timerSettings.addItem( m10);

	int ret = timerSettings.exec(this, "");

	return ret;
}

bool askUserOnTimerConflict(time_t announceTime, time_t stopTime)
{
	CTimerdClient Timer;
	CTimerd::TimerList overlappingTimers = Timer.getOverlappingTimers(announceTime,stopTime);
	
	//printf("[CTimerdClient] attention\n%d\t%d\t%d conflicts with:\n",timerNew.announceTime,timerNew.alarmTime,timerNew.stopTime);

	std::string timerbuf = g_Locale->getText(LOCALE_TIMERLIST_OVERLAPPING_TIMER);
	timerbuf += "\n";
	for (CTimerd::TimerList::iterator it = overlappingTimers.begin(); it != overlappingTimers.end(); it++)
	{
		timerbuf += CTimerList::convertTimerType2String(it->eventType);
		timerbuf += " (";
		timerbuf += CTimerList::convertChannelId2String(it->channel_id); // UTF-8
		if(it->epgID != 0)
		{
			CEPGData epgdata;

			if (sectionsd_getEPGid(it->epgID, it->epg_starttime, &epgdata))
			{
				timerbuf += ":";
				timerbuf += epgdata.title;
			}
			else if(strlen(it->epgTitle)!=0)
			{
				timerbuf += ":";
				timerbuf += it->epgTitle;
			}
		}
		timerbuf += ")";

		timerbuf += ":\n";
		char at[25] = {0};
		struct tm *annTime = localtime(&(it->announceTime));
		strftime(at,20,"%d.%m. %H:%M",annTime);
		timerbuf += at;
		timerbuf += " - ";

		char st[25] = {0};
		struct tm *sTime = localtime(&(it->stopTime));
		strftime(st,20,"%d.%m. %H:%M",sTime);
		timerbuf += st;
		timerbuf += "\n";
	}

	return (MessageBox(LOCALE_MESSAGEBOX_INFO, timerbuf.c_str(), mbrNo, mbNo | mbYes) == mbrYes);
}
