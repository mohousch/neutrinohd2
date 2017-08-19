/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: infoviewer.cpp 2013/10/12 mohousch Exp $

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

#include <string>
#include <algorithm>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/vfs.h>
#include <unistd.h>

#include <map>

#include <fcntl.h>

#include <gui/infoviewer.h>

#include <gui/widget/progressbar.h>
#include <gui/widget/icons.h>
#include <gui/widget/hintbox.h>

#include <daemonc/remotecontrol.h>

#include <global.h>
#include <neutrino.h>

#include <gui/pictureviewer.h>

#include <sys/timeb.h>
#include <time.h>
#include <sys/param.h>

//
#include <satconfig.h>
#include <frontend_c.h>

#include <video_cs.h>
#include <system/debug.h>

#include <system/helpers.h>


void sectionsd_getEventsServiceKey(t_channel_id serviceUniqueKey, CChannelEventList &eList, char search = 0, std::string search_text = "");
void sectionsd_getCurrentNextServiceKey(t_channel_id uniqueServiceKey, CSectionsdClient::responseGetCurrentNextInfoChannelID& current_next );

extern CRemoteControl * g_RemoteControl;		// neutrino.cpp

extern cVideo * videoDecoder;				// libdvbapi
extern CFrontend * live_fe;				// zapit.cpp
extern fe_map_t femap;					// zapit.cpp
extern CFrontend * getFE(int index);			// zapit.cpp
extern int FrontendCount;				// defined in zapit.cpp

extern bool autoshift;
extern uint32_t shift_timer;

extern std::string ext_channel_name;	// defined in vcrcontrol.cpp
extern bool timeset;			// defined in sectionsd.cpp

#define COL_INFOBAR_BUTTONS            (COL_INFOBAR_SHADOW + 1)
#define COL_INFOBAR_BUTTONS_BACKGROUND (COL_INFOBAR_SHADOW_PLUS_1)

#define borderwidth 	5 //for subchannels Box

// in us
#define LCD_UPDATE_TIME_TV_MODE (60 * 1000 * 1000)

//
#define RED_BAR 		40
#define YELLOW_BAR 		70
#define GREEN_BAR 		100

#define BAR_WIDTH 		72
#define TIMESCALE_BAR_HEIGHT	7
#define SIGSCALE_BAR_HEIGHT	8
#define SNRSCALE_BAR_HEIGHT	8

// rec
#define REC_INFOBOX_WIDTH	300
#define REC_INFOBOX_HEIGHT	20

// sat
#define SAT_INFOBOX_HEIGHT	30

// buttonbar
#define BUTTON_BAR_HEIGHT	20

// channel
#define CHANNAME_HEIGHT		35
#define CHANNAME_WIDTH		100
#define CHANNUMBER_HEIGHT	35
#define CHANNUMBER_WIDTH	100

// logo
#define CHANNEL_LOGO_HEIGHT	40

// infoboxheight
#define BOXHEIGHT_CHANNELINFO	140
#define BOXHEIGHT_MOVIEINFO	100

#define CHANINFO_HEIGHT		24

//static event_id_t last_curr_id = 0, last_next_id = 0;

static bool sortByDateTime(const CChannelEvent& a, const CChannelEvent& b)
{
        return a.startTime < b.startTime;
}

CInfoViewer::CInfoViewer()
{
  	Init();
}

void CInfoViewer::Init()
{
	frameBuffer = CFrameBuffer::getInstance();
	
	//
	recordModeActive = false;
	is_visible = false;

	showButtonBar = false;

	gotTime = timeset;
	CA_Status = false;
	virtual_zap_mode = false;
	chanready = 1;
	
	// init dimension
	initDimension();
}

void CInfoViewer::initDimension(void)
{
	// icons dimension
	frameBuffer->getIconSize(NEUTRINO_ICON_16_9, &icon_w_aspect, &icon_h_aspect);
	frameBuffer->getIconSize(NEUTRINO_ICON_VTXT, &icon_w_vtxt, &icon_h_vtxt);
	frameBuffer->getIconSize(NEUTRINO_ICON_RESOLUTION_000, &icon_w_reso, &icon_h_reso);
	frameBuffer->getIconSize(NEUTRINO_ICON_RESOLUTION_SD2, &icon_w_sd, &icon_h_sd);
	frameBuffer->getIconSize(NEUTRINO_ICON_RESOLUTION_HD2, &icon_hd_w, &icon_hd_h);
	frameBuffer->getIconSize(NEUTRINO_ICON_SUBT, &icon_w_subt, &icon_h_subt);
	frameBuffer->getIconSize(NEUTRINO_ICON_DD, &icon_w_dd, &icon_h_dd);
	frameBuffer->getIconSize(NEUTRINO_ICON_SCRAMBLED2_GREY, &icon_w_ca, &icon_h_ca);
	frameBuffer->getIconSize(NEUTRINO_ICON_RADIOTEXTOFF, &icon_w_rt, &icon_h_rt);
	frameBuffer->getIconSize(NEUTRINO_ICON_REC, &icon_w_rec, &icon_h_rec);
	
	// colored user icons
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_RED, &icon_red_w, &icon_red_h);
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_GREEN, &icon_green_w, &icon_green_h);
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_YELLOW, &icon_yellow_w, &icon_yellow_h);
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_BLUE, &icon_blue_w, &icon_blue_h);
	
	// boxheight
	BoxHeight = BOXHEIGHT_CHANNELINFO;
	
	// buttonbarheight
	buttonBarHeight = (icon_h_vtxt? icon_h_vtxt : BUTTON_BAR_HEIGHT) + 6;
	
	BoxEndX = g_settings.screen_EndX - 10;
	BoxEndY = g_settings.screen_EndY - (10 + SHADOW_OFFSET + buttonBarHeight);
	BoxStartX = g_settings.screen_StartX + 10;
	BoxStartY = BoxEndY - BoxHeight;
	BoxWidth = BoxEndX - BoxStartX;
	
	// infobar
	buttonBarStartX = BoxStartX;
	buttonBarStartY = BoxStartY + BoxHeight;
	
	// channel logo
	PIC_X = BoxStartX + CHANNUMBER_WIDTH + BORDER_LEFT;
	PIC_Y = BoxStartY + SAT_INFOBOX_HEIGHT + TIMESCALE_BAR_HEIGHT + 5;
	
	// channel number
	ChanNumberX = BoxStartX + BORDER_LEFT;
	ChanNumberY = BoxStartY + SAT_INFOBOX_HEIGHT + TIMESCALE_BAR_HEIGHT + 5;
	
	// channel name
	ChanNameX = BoxStartX + CHANNUMBER_WIDTH + BORDER_LEFT;
	ChanNameY = BoxStartY + SAT_INFOBOX_HEIGHT + TIMESCALE_BAR_HEIGHT + 5;
	
	// channel logo
	PIC_W = CHANNEL_LOGO_HEIGHT*1.67;
	PIC_H = CHANNEL_LOGO_HEIGHT;
	
	// channel info
	ChanInfoX = BoxStartX + CHANNUMBER_WIDTH + BORDER_LEFT;
	ChanInfoY = BoxStartY + SAT_INFOBOX_HEIGHT + TIMESCALE_BAR_HEIGHT + 5 + CHANNEL_LOGO_HEIGHT + 3; //PIC_Y + CHANNEL_LOGO_HEIGHT + 5
	ChanInfoHeight = std::max(CHANINFO_HEIGHT, (g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->getHeight() > CHANINFO_HEIGHT)? CHANINFO_HEIGHT : g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->getHeight());

	// button cell width
	asize = (BoxWidth - ( BORDER_RIGHT + BORDER_LEFT + ICON_TO_ICON_OFFSET + icon_w_subt + ICON_TO_ICON_OFFSET + icon_w_vtxt + ICON_TO_ICON_OFFSET + icon_w_dd + ICON_TO_ICON_OFFSET + icon_w_aspect + ICON_TO_ICON_OFFSET + icon_w_sd + ICON_TO_ICON_OFFSET + icon_w_reso + ICON_TO_ICON_OFFSET + icon_w_ca + ICON_TO_ICON_OFFSET + icon_w_rt + ICON_TO_ICON_OFFSET + 3*TunerNumWidth + 20))/4;
	
	//
	ChanNameHeight = (CHANNEL_LOGO_HEIGHT - g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_CHANNAME]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_CHANNAME]->getHeight(); //FIXME
	
	// satname
	SatNameHeight = g_SignalFont->getHeight();
	
	//
	TunerNumWidth = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getRenderWidth("T9", true);
	TunerNumHeight = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight();

	timescale_posx = BoxStartX + BORDER_LEFT;
	timescale_posy = BoxStartY + SAT_INFOBOX_HEIGHT;
}

void CInfoViewer::start()
{
	dprintf(DEBUG_NORMAL, "CInfoViewer::start\n");
#if defined (ENABLE_LCD)	
	lcdUpdateTimer = g_RCInput->addTimer(LCD_UPDATE_TIME_TV_MODE, false, true);
#endif	
}

void CInfoViewer::paintTime(int posx, int posy, CFont* timeFont)
{
	int time_left_width = 2 * timeFont->getRenderWidth(widest_number);
	int time_dot_width = timeFont->getRenderWidth(":");
	int time_width = 2*time_left_width + time_dot_width;
	int time_height = timeFont->getHeight();

	int timestr_len = timeFont->getRenderWidth("00:00");
	
	if (gotTime && is_visible) 
	{
		char timestr[10];
		struct timeb tm;
	
		ftime (&tm);
		strftime ((char *) &timestr, 20, "%H:%M", localtime (&tm.time));

		timeFont->RenderString(posx - BORDER_RIGHT - timestr_len, posy + time_height, timestr_len, timestr, COL_INFOBAR);
	
	}
}

void CInfoViewer::showRecordIcon(const bool show)
{ 
	recordModeActive = CNeutrinoApp::getInstance()->recordingstatus || shift_timer;

	if (recordModeActive && is_visible) 
	{
		if (show) 
		{
			frameBuffer->paintIcon(autoshift ? NEUTRINO_ICON_AUTO_SHIFT : NEUTRINO_ICON_REC, BoxStartX + BORDER_LEFT, BoxStartY - 30);

			if(!autoshift && !shift_timer) 
			{
				// shadow
				frameBuffer->paintBoxRel(BoxStartX + BORDER_LEFT + icon_w_rec + ICON_OFFSET + SHADOW_OFFSET, BoxStartY - 30 + SHADOW_OFFSET, REC_INFOBOX_WIDTH, REC_INFOBOX_HEIGHT, COL_INFOBAR_SHADOW_PLUS_0);
				
				// rec info box
				frameBuffer->paintBoxRel(BoxStartX + BORDER_LEFT + icon_w_rec + ICON_OFFSET, BoxStartY - 30, REC_INFOBOX_WIDTH, REC_INFOBOX_HEIGHT, COL_INFOBAR_PLUS_0);

				g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString (BoxStartX + BORDER_LEFT + icon_w_rec + ICON_OFFSET + BORDER_LEFT, BoxStartY - 8, REC_INFOBOX_WIDTH, ext_channel_name.c_str(), COL_INFOBAR, 0, true);
			} 
			else
				frameBuffer->paintBackgroundBoxRel(BoxStartX + BORDER_LEFT, BoxStartY - 30, icon_w_rec, icon_h_rec);
		} 
		else 
			frameBuffer->paintBackgroundBoxRel(BoxStartX + BORDER_LEFT, BoxStartY - 30, icon_w_rec, icon_h_rec);
	}	
}

void CInfoViewer::showTitle(const int ChanNum, const std::string & Channel, const t_satellite_position satellitePosition)
{
	dprintf(DEBUG_NORMAL, "CInfoViewer::showTitle:\n");
	
	std::string ChannelName = Channel; //FIXME:
	
	// init Dimension
	initDimension();
	
	//
	int col_NumBoxText;
	int col_NumBox;

	if (virtual_zap_mode) 
	{
		col_NumBoxText = COL_MENUHEAD;
		col_NumBox = COL_MENUHEAD_PLUS_0;
	} 
	else 
	{
		col_NumBoxText = COL_INFOBAR;
		col_NumBox = COL_INFOBAR_PLUS_0;
	}
	
	// infobar shadow
	frameBuffer->paintBoxRel(BoxStartX + SHADOW_OFFSET, BoxStartY + SHADOW_OFFSET, BoxWidth, BoxHeight + buttonBarHeight, COL_INFOBAR_SHADOW_PLUS_0, RADIUS_MID, CORNER_BOTH);
	
	// infobarbox
	frameBuffer->paintBoxRel(BoxStartX, BoxStartY, BoxWidth, BoxHeight, COL_INFOBAR_PLUS_0, RADIUS_MID, CORNER_TOP, g_settings.infobar_gradient);

	//time
	paintTime(BoxEndX, ChanNameY, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_CHANNAME]);

	//sat name
	char strChanNum[10];
	sprintf(strChanNum, "%d", ChanNum);

	if (satellitePositions.size()) 
	{
		sat_iterator_t sit = satellitePositions.find(satellitePosition);

		if(sit != satellitePositions.end()) 
		{
			satNameWidth = g_SignalFont->getRenderWidth(sit->second.name);
			
			// Reserving here for satNameWidth max. 1/3 of infobar width
			// because some sat-names are longer than : (CHANNUMBER_WIDTH + 70)
			if (satNameWidth > ((BoxEndX - BoxStartX) / 3) ) 
				satNameWidth = (BoxEndX - BoxStartX) / 3;
				
			g_SignalFont->RenderString( BoxStartX + BORDER_LEFT, BoxStartY + (SAT_INFOBOX_HEIGHT - SatNameHeight)/2 + SatNameHeight, satNameWidth, sit->second.name, COL_INFOBAR );
		}
	}

	if(CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_iptv)
	{
		satNameWidth = g_SignalFont->getRenderWidth("WebTV");
			
		if (satNameWidth > ((BoxEndX - BoxStartX) / 3) ) 
			satNameWidth = (BoxEndX - BoxStartX) / 3;
				
		g_SignalFont->RenderString( BoxStartX + BORDER_LEFT, BoxStartY + (SAT_INFOBOX_HEIGHT - SatNameHeight)/2 + SatNameHeight, satNameWidth, "WebTV", COL_INFOBAR );
	}

	// channel number/logo/name
	if (satellitePosition != 0 && satellitePositions.size() ) 
	{
		// ChannelNumber
		g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_CHANNAME]->RenderString(ChanNumberX, ChanNameY + ChanNameHeight, CHANNUMBER_WIDTH, strChanNum, col_NumBoxText);
		
		ChanNameWidth = BoxWidth - (ICON_OFFSET + 30 + CHANNUMBER_WIDTH + g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_CHANNAME]->getRenderWidth(ChannelName, true));

		// display channel picon
		bool logo_ok = false;
		
		//
		logo_w = PIC_W; 
		logo_h = PIC_H;
		logo_bpp = 0;
		
		// check logo
		logo_ok = frameBuffer->checkLogo(channel_id);
		
		if(logo_ok)
		{
			// get logo size	
			frameBuffer->getLogoSize(channel_id, &logo_w, &logo_h, &logo_bpp);
		
			// display logo
			frameBuffer->DisplayLogo(channel_id, PIC_X, PIC_Y, (logo_bpp == 4 && !g_settings.show_channelname)? logo_w : PIC_W, PIC_H, (logo_h > PIC_H)? true : false, false, true);

			// recalculate ChanNameWidth //FIXME: timewidth
			ChanNameWidth = BoxWidth - (30 + CHANNUMBER_WIDTH + logo_w + g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_CHANNAME]->getRenderWidth(ChannelName, true));
			
			// ChannelName
			if(g_settings.show_channelname)
				g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_CHANNAME]->RenderString(PIC_X + ((logo_bpp == 4)? logo_w : PIC_W) + BORDER_LEFT, ChanNameY + ChanNameHeight, ChanNameWidth, ChannelName, COL_INFOBAR, 0, true);	// UTF-8
		}
		else
		{
			// ChannelName
			g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_CHANNAME]->RenderString( BoxStartX + CHANNUMBER_WIDTH + BORDER_LEFT, ChanNameY + ChanNameHeight, ChanNameWidth, ChannelName, COL_INFOBAR, 0, true);	// UTF-8
		}
	}
		
	// show date
	std::string datestr= getNowTimeStr("%d.%m.%Y");
			
	dateWidth = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->getRenderWidth(datestr.c_str(), true); //UTF-8
	dateHeight = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->getHeight();
			
	g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->RenderString(BoxEndX - BORDER_RIGHT - dateWidth, BoxStartY + (SAT_INFOBOX_HEIGHT - dateHeight)/2 + dateHeight, dateWidth, datestr.c_str(), COL_INFOBAR, 0, true); // UTF-8

	// activ tuner
	showAktivTuner();

	// radiotext	
	if (CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_radio)
	{
		if ((g_settings.radiotext_enable) && (!recordModeActive))
			showRadiotext();
		else
			showIcon_RadioText(false);
	}

	// record icon
	showRecordIcon(show_dot);
		
	// botton bar
	frameBuffer->paintBoxRel(buttonBarStartX, buttonBarStartY, BoxWidth, buttonBarHeight, COL_INFOBAR_BUTTONS_BACKGROUND, RADIUS_MID, CORNER_BOTTOM);

	// blue button
	// features
	frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_BLUE, BoxStartX + BORDER_LEFT + icon_red_w + ICON_TO_ICON_OFFSET + asize + icon_green_w + ICON_TO_ICON_OFFSET + asize + icon_yellow_w + ICON_TO_ICON_OFFSET + asize, buttonBarStartY + (buttonBarHeight - icon_blue_h)/2);

	g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(BoxStartX + BORDER_LEFT + icon_red_w + ICON_TO_ICON_OFFSET + asize + icon_green_w + ICON_TO_ICON_OFFSET + asize + icon_yellow_w + ICON_TO_ICON_OFFSET + asize + icon_blue_w + ICON_TO_ICON_OFFSET, buttonBarStartY + (buttonBarHeight - g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight(), asize - ICON_TO_ICON_OFFSET - icon_blue_w, g_Locale->getText(LOCALE_INFOVIEWER_FEATURES), COL_INFOBAR_BUTTONS, 0, true); // UTF-8

	// other buttonbar
	if( showButtonBar )
	{
		// green
		showButton_Audio();
			
		// yellow
		showButton_SubServices();
			
		showIcon_CA_Status(0);
		showIcon_16_9();
		showIcon_VTXT();
		showIcon_SubT();
		showIcon_Resolution();
	}
}

//
void CInfoViewer::show(const int _ChanNum, const std::string & _Channel, const t_satellite_position _satellitePosition, const t_channel_id _new_channel_id, const bool _calledFromNumZap, int _epgpos)
{
	dprintf(DEBUG_NORMAL, "CInfoViewer::show:\n");

	//
	std::string ChannelName = _Channel;
	show_dot = true;
	is_visible = true;
	new_chan = false;
	showButtonBar = !_calledFromNumZap;
	//last_curr_id = last_next_id = 0;
	runningPercent = 0;

	// init progressbar
	sigscale = new CProgressBar(BAR_WIDTH, SIGSCALE_BAR_HEIGHT, RED_BAR, GREEN_BAR, YELLOW_BAR, false);
	snrscale = new CProgressBar(BAR_WIDTH, SNRSCALE_BAR_HEIGHT, RED_BAR, GREEN_BAR, YELLOW_BAR, false);
	timescale = new CProgressBar(BoxWidth - BORDER_LEFT - BORDER_RIGHT, TIMESCALE_BAR_HEIGHT);	//5? see in code
	
	sigscale->reset(); 
	snrscale->reset(); 
	timescale->reset();

	if (!gotTime)
		gotTime = timeset;

	//
	if (virtual_zap_mode) 
	{
		if ((channel_id != _new_channel_id) || (evtlist.empty())) 
		{
			evtlist.clear();
			sectionsd_getEventsServiceKey(_new_channel_id & 0xFFFFFFFFFFFFULL, evtlist);
			
			if (!evtlist.empty())
				sort(evtlist.begin(),evtlist.end(), sortByDateTime);
			
			new_chan = true;
		}
	} 

	if (! _calledFromNumZap && !(g_RemoteControl->subChannels.empty()) && (g_RemoteControl->selected_subchannel > 0))
	{
		ChannelName = g_RemoteControl->subChannels[g_RemoteControl->selected_subchannel].subservice_name;
		channel_id = g_RemoteControl->subChannels[g_RemoteControl->selected_subchannel].getChannelID();
	} 
	else 
	{
		channel_id = _new_channel_id;
	}

	//
	showTitle(_ChanNum, ChannelName, _satellitePosition);

	//
	showSNR();

	// show current_next epg data
	getCurrentNextEPG(channel_id, new_chan, _epgpos);

	frameBuffer->blit();

#if ENABLE_LCD
	showLcdPercentOver();
#endif

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);

	// loop msg
	neutrino_msg_t msg;
	neutrino_msg_data_t data;

	CNeutrinoApp * neutrino = CNeutrinoApp::getInstance();

	bool hideIt = false;
	virtual_zap_mode = false;

	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd (g_settings.timing[SNeutrinoSettings::TIMING_INFOBAR] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_INFOBAR]);

	int res = messages_return::none;

	while (!(res & (messages_return::cancel_info | messages_return::cancel_all))) 
	{
		g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd);

		dprintf(DEBUG_NORMAL, "show: msg:%s\n", CRCInput::getSpecialKeyName(msg));

		//
		sigscale->reset(); 
		snrscale->reset(); 
		timescale->reset();

		//
		showTitle(_ChanNum, ChannelName, _satellitePosition);
		show_Data();
		showSNR();

		if ( msg == CRCInput::RC_sat || msg == CRCInput::RC_favorites)
		{
			g_RCInput->postMsg (msg, 0);
			res = messages_return::cancel_info;
		}
		else if ( msg == CRCInput::RC_info )
		{
			g_RCInput->postMsg(NeutrinoMessages::SHOW_EPG, 0);
				
			res = messages_return::cancel_info;
		} 
		else if ((msg == CRCInput::RC_ok) || (msg == CRCInput::RC_home) || (msg == CRCInput::RC_timeout)) 
		{
			res = messages_return::cancel_info;
			// add this here, now OK and EXIT/HOME has effect
			// and infobar timeout settings work
			hideIt = true;
		} 			
		else if ( (msg == NeutrinoMessages::EVT_TIMER) && (data == sec_timer_id) )
		{
			showRecordIcon(show_dot);
			show_dot = !show_dot;
				
			// radiotext		
			if ((g_settings.radiotext_enable) && (CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_radio))
				showRadiotext();			

			showIcon_16_9();
				
			if ( is_visible && showButtonBar ) 
				showIcon_Resolution();
		} 
		else if ( g_settings.virtual_zap_mode && ((msg == CRCInput::RC_right) || msg == CRCInput::RC_left) && (CNeutrinoApp::getInstance()->getMode() != NeutrinoMessages::mode_iptv)) 
		{
			virtual_zap_mode = true;
			res = messages_return::cancel_all;
			hideIt = true;
		} 
		else if ( !CNeutrinoApp::getInstance()->timeshiftstatus) 
		{
			if ((msg == (neutrino_msg_t) g_settings.key_quickzap_up) || (msg == (neutrino_msg_t) g_settings.key_quickzap_down) || (msg == CRCInput::RC_0) || (msg == NeutrinoMessages::SHOW_INFOBAR)) 
			{
				// radiotext					
				if ((g_settings.radiotext_enable) && (CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_radio))
					hideIt =  true;
				else				  
					hideIt = false;
					
				g_RCInput->postMsg(msg, data);
				res = messages_return::cancel_info;
			} 
			else if (msg == NeutrinoMessages::EVT_TIMESET) 
			{
				// Handle anyway!
				neutrino->handleMsg(msg, data);
				g_RCInput->postMsg (NeutrinoMessages::SHOW_INFOBAR, 0);
				hideIt = false;
				res = messages_return::cancel_all;
			} 
			else 
			{
				if (msg == CRCInput::RC_standby) 
				{
					g_RCInput->killTimer(sec_timer_id);
				}
					
				res = neutrino->handleMsg(msg, data);
					
				if (res & messages_return::unhandled) 
				{
					// raus hier und im Hauptfenster behandeln...
					g_RCInput->postMsg (msg, data);
					res = messages_return::cancel_info;
				}
			}
		}
				
		frameBuffer->blit();		
	}

	if (hideIt)
		killTitle();
		
	g_RCInput->killTimer(sec_timer_id);
	sec_timer_id = 0;

	if (virtual_zap_mode)
	{
		CNeutrinoApp::getInstance()->channelList->virtual_zap_mode(msg == CRCInput::RC_right);
		virtual_zap_mode = false;
	}
}

void CInfoViewer::getCurrentNextEPG(t_channel_id ChannelID, bool newChan, int EPGPos)
{
	sectionsd_getCurrentNextServiceKey(ChannelID & 0xFFFFFFFFFFFFULL, info_CurrentNext);
	
	if (!evtlist.empty()) 
	{
		if (newChan) 
		{
			for ( eli = evtlist.begin(); eli!=evtlist.end(); ++eli ) 
			{
				if ((uint)eli->startTime >= info_CurrentNext.current_zeit.startzeit + info_CurrentNext.current_zeit.dauer)
					break;
			}
			
			if (eli == evtlist.end()) // the end is not valid, so go back
				--eli;
		}

		if (EPGPos != 0) 
		{
			info_CurrentNext.flags = 0;
			if ((EPGPos > 0) && (eli != evtlist.end())) 
			{
				++eli; // next epg
				if (eli == evtlist.end()) // the end is not valid, so go back
					--eli;
			}
			else if ((EPGPos < 0) && (eli != evtlist.begin())) 
			{
				--eli; // prev epg
			}

			info_CurrentNext.flags = CSectionsdClient::epgflags::has_current;
			info_CurrentNext.current_uniqueKey      = eli->eventID;
			info_CurrentNext.current_zeit.startzeit = eli->startTime;
			info_CurrentNext.current_zeit.dauer     = eli->duration;

			if (eli->description.empty())
				info_CurrentNext.current_name   = g_Locale->getText(LOCALE_INFOVIEWER_NOEPG);
			else
				info_CurrentNext.current_name   = eli->description;

			info_CurrentNext.current_fsk            = '\0';

			if (eli != evtlist.end()) 
			{
				++eli;
				if (eli != evtlist.end()) 
				{
					info_CurrentNext.flags                  = CSectionsdClient::epgflags::has_current | CSectionsdClient::epgflags::has_next;
					info_CurrentNext.next_uniqueKey         = eli->eventID;
					info_CurrentNext.next_zeit.startzeit    = eli->startTime;
					info_CurrentNext.next_zeit.dauer        = eli->duration;

					if (eli->description.empty())
						info_CurrentNext.next_name      = g_Locale->getText(LOCALE_INFOVIEWER_NOEPG);
					else
						info_CurrentNext.next_name      = eli->description;
				}
				--eli;
			}
		}
	}

	if (!(info_CurrentNext.flags & (CSectionsdClient::epgflags::has_later | CSectionsdClient::epgflags::has_current | CSectionsdClient::epgflags::not_broadcast))) 
	{
		// nicht gefunden / noch nicht geladen
		g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->RenderString (ChanInfoX, ChanInfoY + 2*ChanInfoHeight, BoxEndX - ChanNameX, g_Locale->getText (gotTime ? (showButtonBar ? LOCALE_INFOVIEWER_EPGWAIT : LOCALE_INFOVIEWER_EPGNOTLOAD) : LOCALE_INFOVIEWER_WAITTIME), COL_INFOBAR, 0, true);	// UTF-8
	} 
	else
	{
		show_Data();
	}
}

void CInfoViewer::showSubchan()
{
  	CNeutrinoApp *neutrino = CNeutrinoApp::getInstance();

  	std::string subChannelName;	// holds the name of the subchannel/audio channel
  	int subchannel = 0;		// holds the channel index

  	if (!(g_RemoteControl->subChannels.empty ())) 
	{
		// get info for nvod/subchannel
		subchannel = g_RemoteControl->selected_subchannel;
		if (g_RemoteControl->selected_subchannel >= 0)
	  		subChannelName = g_RemoteControl->subChannels[g_RemoteControl->selected_subchannel].subservice_name;
  	} 
	else if (g_RemoteControl->current_PIDs.APIDs.size () > 1 ) 
	{
		// get info for audio channel
		subchannel = g_RemoteControl->current_PIDs.PIDs.selected_apid;
		subChannelName = g_RemoteControl->current_PIDs.APIDs[g_RemoteControl->current_PIDs.PIDs.selected_apid].desc;
  	}

  	if (!(subChannelName.empty ())) 
	{
		char text[100];
		sprintf (text, "%d - %s", subchannel, subChannelName.c_str ());

		int dx = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->getRenderWidth(text) + 20;
		int dy = 25;

		int icon_w;
		int icon_h;

		if (g_RemoteControl->director_mode) 
		{
	  		int w = 20 + g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getRenderWidth (g_Locale->getText(LOCALE_NVODSELECTOR_DIRECTORMODE), true) + 20;	// UTF-8
	  		if (w > dx)
				dx = w;

			frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_YELLOW, &icon_w, &icon_h);
			
	  		dy = 25 + icon_h + 10;
		} 
		else
	  		dy = 25 + 5;

		int x = 0, y = 0;
		
		if (g_settings.infobar_subchan_disp_pos == 0) 
		{
	  		// Rechts-Oben
	  		x = g_settings.screen_EndX - dx - 10;
	  		y = g_settings.screen_StartY + 10;
		} 
		else if (g_settings.infobar_subchan_disp_pos == 1) 
		{
	  		// Links-Oben
	  		x = g_settings.screen_StartX + 10;
	  		y = g_settings.screen_StartY + 10;
		} 
		else if (g_settings.infobar_subchan_disp_pos == 2) 
		{
	  		// Links-Unten
	  		x = g_settings.screen_StartX + 10;
	  		y = g_settings.screen_EndY - dy - 10;
		} 
		else if (g_settings.infobar_subchan_disp_pos == 3) 
		{
	  		// Rechts-Unten
	  		x = g_settings.screen_EndX - dx - 10;
	  		y = g_settings.screen_EndY - dy - 10;
		}

		fb_pixel_t pixbuf[(dx + 2 * borderwidth) * (dy + 2 * borderwidth)];
		frameBuffer->SaveScreen (x - borderwidth, y - borderwidth, dx + 2 * borderwidth, dy + 2 * borderwidth, pixbuf);		

		// clear border
		frameBuffer->paintBackgroundBoxRel(x - borderwidth, y - borderwidth, dx + 2 * borderwidth, borderwidth);
		frameBuffer->paintBackgroundBoxRel(x - borderwidth, y + dy, dx + 2 * borderwidth, borderwidth);
		frameBuffer->paintBackgroundBoxRel(x - borderwidth, y, borderwidth, dy);
		frameBuffer->paintBackgroundBoxRel(x + dx, y, borderwidth, dy);

		frameBuffer->paintBoxRel(x, y, dx, dy, COL_MENUCONTENT_PLUS_0);
		g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->RenderString (x + 10, y + 30, dx - 20, text, COL_MENUCONTENT);

		if (g_RemoteControl->director_mode) 
		{
	  		frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_YELLOW, x + 8, y + dy - 20);
	  		g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString (x + 30 - icon_w + icon_w, y + dy - 2, dx - 40, g_Locale->getText (LOCALE_NVODSELECTOR_DIRECTORMODE), COL_MENUCONTENT, 0, true);	// UTF-8
		}
		
		frameBuffer->blit();	

		unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd(2);
		int res = messages_return::none;

		neutrino_msg_t msg;
		neutrino_msg_data_t data;

		while (!(res & (messages_return::cancel_info | messages_return::cancel_all))) 
		{
	  		g_RCInput->getMsgAbsoluteTimeout (&msg, &data, &timeoutEnd);

	  		if (msg == CRCInput::RC_timeout) 
			{
				res = messages_return::cancel_info;
	  		} 
			else 
			{
				res = neutrino->handleMsg(msg, data);

				if (res & messages_return::unhandled) 
				{
		  			// raus hier und im Hauptfenster behandeln...
		  			g_RCInput->postMsg (msg, data);
		  			res = messages_return::cancel_info;
				}
	  		}
		}

		frameBuffer->RestoreScreen(x - borderwidth, y - borderwidth, dx + 2 * borderwidth, dy + 2 * borderwidth, pixbuf);
		frameBuffer->blit();	
  		
	} 
	else 
	{
		g_RCInput->postMsg(NeutrinoMessages::SHOW_INFOBAR, 0);
  	}
}

// radiotext
void CInfoViewer::showIcon_RadioText(bool rt_available) const
{
	if (showButtonBar)
	{
		int mode = CNeutrinoApp::getInstance()->getMode();
		std::string rt_icon = NEUTRINO_ICON_RADIOTEXTOFF;
		
		if ((!virtual_zap_mode) && (!recordModeActive) && (mode == NeutrinoMessages::mode_radio))
		{
			if (g_settings.radiotext_enable)
			{
				rt_icon = rt_available ? NEUTRINO_ICON_RADIOTEXTGET : NEUTRINO_ICON_RADIOTEXTWAIT;
			}
		}
		
		frameBuffer->paintIcon(rt_icon, BoxEndX - (BORDER_RIGHT + icon_w_subt + ICON_TO_ICON_OFFSET + icon_w_vtxt + ICON_TO_ICON_OFFSET + icon_w_dd + ICON_TO_ICON_OFFSET + icon_w_aspect + ICON_TO_ICON_OFFSET + icon_w_sd + ICON_TO_ICON_OFFSET + icon_w_reso + ICON_TO_ICON_OFFSET + icon_w_ca + ICON_TO_ICON_OFFSET + icon_w_rt), buttonBarStartY + (buttonBarHeight - icon_h_rt)/2);
	}
}

void CInfoViewer::showIcon_16_9()
{			
	const char * aspect_icon = NEUTRINO_ICON_16_9_GREY;
			
	if(videoDecoder->getAspectRatio() == ASPECTRATIO_169)
		aspect_icon = NEUTRINO_ICON_16_9;
	
	if (is_visible)
	{
		frameBuffer->paintIcon(aspect_icon, BoxEndX - (BORDER_RIGHT + icon_w_subt + ICON_TO_ICON_OFFSET + icon_w_vtxt + ICON_TO_ICON_OFFSET + icon_w_dd + ICON_TO_ICON_OFFSET + icon_w_aspect), buttonBarStartY + (buttonBarHeight - icon_h_aspect)/2 );
	}
}

void CInfoViewer::showIcon_VTXT() const
{
	if (is_visible)
	{
		frameBuffer->paintIcon((g_RemoteControl->current_PIDs.PIDs.vtxtpid != 0 && !g_settings.satip_allow_satip) ? NEUTRINO_ICON_VTXT : NEUTRINO_ICON_VTXT_GREY, BoxEndX - (BORDER_RIGHT + icon_w_subt + ICON_TO_ICON_OFFSET + icon_w_vtxt), buttonBarStartY + (buttonBarHeight - icon_h_vtxt)/2 );
	}
}

void CInfoViewer::showIcon_Resolution() const
{
	int xres, yres, framerate;
	const char *icon_name = NULL;
	const char *icon_name_res = NULL;
	
	if (is_visible)
	{
		videoDecoder->getPictureInfo(xres, yres, framerate);
		
		// show sd/hd icon on infobar	
		switch (yres) 
		{
			case 2160:
			case 1980:
				icon_name_res = NEUTRINO_ICON_RESOLUTION_UHD2;
				//test
				CVFD::getInstance()->ShowIcon(VFD_ICON_HD, true); //FIXME: remove this to remotecontrol
				break;

			case 1920:
			case 1440:
			case 1280:
			case 1088:
			case 1080:
			case 720:
				icon_name_res = NEUTRINO_ICON_RESOLUTION_HD2;
				//test
				CVFD::getInstance()->ShowIcon(VFD_ICON_HD, true); //FIXME: remove this to remotecontrol
				break;
				
			case 704:
			case 576:
			case 544:
			case 528:
			case 480:
			case 382:
			case 352:
			case 288:
				icon_name_res = NEUTRINO_ICON_RESOLUTION_SD2;
				//test
				CVFD::getInstance()->ShowIcon(VFD_ICON_HD, false);
				break;
				
			default:
				icon_name_res = NEUTRINO_ICON_RESOLUTION_000;
				CVFD::getInstance()->ShowIcon(VFD_ICON_HD, false);
				break;	
		}
		
		// icon
		frameBuffer->paintIcon(icon_name_res, BoxEndX - (BORDER_RIGHT + icon_w_subt + ICON_TO_ICON_OFFSET + icon_w_vtxt + ICON_TO_ICON_OFFSET + icon_w_dd + ICON_TO_ICON_OFFSET + icon_w_aspect + ICON_TO_ICON_OFFSET + icon_w_sd), buttonBarStartY + (buttonBarHeight - icon_h_sd)/2 );
		
		// resolution
		switch (yres) 
		{
			case 2160:
				icon_name = NEUTRINO_ICON_RESOLUTION_2160;
				break;

			case 1980:
				icon_name = NEUTRINO_ICON_RESOLUTION_1980;
				break;

			case 1920:
				icon_name = NEUTRINO_ICON_RESOLUTION_1920;
				break;
				
			case 1088:
			case 1080:
				icon_name = NEUTRINO_ICON_RESOLUTION_1080;
				break;
				
			case 1440:
				icon_name = NEUTRINO_ICON_RESOLUTION_1440;
				break;
				
			case 1280:
				icon_name = NEUTRINO_ICON_RESOLUTION_1280;
				break;
				
			case 720:
				icon_name = NEUTRINO_ICON_RESOLUTION_720;
				break;
				
			case 704:
				icon_name = NEUTRINO_ICON_RESOLUTION_704;
				break;
				
			case 576:
				icon_name = NEUTRINO_ICON_RESOLUTION_576;
				break;
				
			case 544:
				icon_name = NEUTRINO_ICON_RESOLUTION_544;
				break;
				
			case 528:
				icon_name = NEUTRINO_ICON_RESOLUTION_528;
				break;
				
			case 480:
				icon_name = NEUTRINO_ICON_RESOLUTION_480;
				break;
				
			case 382:
				icon_name = NEUTRINO_ICON_RESOLUTION_382;
				break;
				
			case 352:
				icon_name = NEUTRINO_ICON_RESOLUTION_352;
				break;
				
			case 288:
				icon_name = NEUTRINO_ICON_RESOLUTION_288;
				break;
				
			default:
				icon_name = NEUTRINO_ICON_RESOLUTION_000;
				break;
		}

		// icon
		frameBuffer->paintIcon(icon_name, BoxEndX - (ICON_OFFSET + icon_w_subt + 2 + icon_w_vtxt + 2 + icon_w_dd + 2 + icon_w_aspect + 2 + icon_w_sd + 2 + icon_w_reso), buttonBarStartY + (buttonBarHeight - icon_h_reso)/2 );
	}
}

// dvbsub icon
void CInfoViewer::showIcon_SubT() const
{
        bool have_sub = false;
	CZapitChannel * cc = CNeutrinoApp::getInstance()->channelList->getChannel(CNeutrinoApp::getInstance()->channelList->getActiveChannelNumber());
	if(cc && cc->getSubtitleCount())
		have_sub = true;

	if(is_visible)
		frameBuffer->paintIcon(have_sub ? NEUTRINO_ICON_SUBT : NEUTRINO_ICON_SUBT_GREY, BoxEndX - (BORDER_RIGHT + icon_w_subt), buttonBarStartY + (buttonBarHeight - icon_h_subt)/2 );
}

void CInfoViewer::showFailure()
{
  	HintBox(LOCALE_MESSAGEBOX_ERROR, g_Locale->getText (LOCALE_INFOVIEWER_NOTAVAILABLE), 430);
}

void CInfoViewer::showMotorMoving (int duration)
{
	char text[256];
	char buffer[10];
	
	sprintf (buffer, "%d", duration);
	strcpy (text, g_Locale->getText (LOCALE_INFOVIEWER_MOTOR_MOVING));
	strcat (text, " (");
	strcat (text, buffer);
	strcat (text, " s)");
	
	HintBox(LOCALE_MESSAGEBOX_INFO, text, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth (text, true) + 10, duration);	// UTF-8
}

// radiotext
void CInfoViewer::killRadiotext()
{
	frameBuffer->paintBackgroundBox(rt_x, rt_y, rt_w, rt_h);
}

void CInfoViewer::showRadiotext()
{
	char stext[3][100];
	int yoff = 8, ii = 0;
	bool RTisIsUTF = false;

	if (g_Radiotext == NULL) 
		return;
	
	showIcon_RadioText(g_Radiotext->haveRadiotext());

	if (g_Radiotext->S_RtOsd) 
	{
		// dimensions of radiotext window
		rt_dx = BoxEndX - BoxStartX;
		rt_dy = 25;
		rt_x = BoxStartX;
		rt_y = g_settings.screen_StartY + 10;
		rt_h = rt_y + 7 + rt_dy*(g_Radiotext->S_RtOsdRows+1)+SHADOW_OFFSET;
		rt_w = rt_x+rt_dx + SHADOW_OFFSET;
		
		int lines = 0;
		for (int i = 0; i < g_Radiotext->S_RtOsdRows; i++) 
		{
			if (g_Radiotext->RT_Text[i][0] != '\0') lines++;
		}
		
		if (lines == 0)
		{
			frameBuffer->paintBackgroundBox(rt_x, rt_y, rt_w, rt_h);
		}

		if (g_Radiotext->RT_MsgShow) 
		{
			if (g_Radiotext->S_RtOsdTitle == 1) 
			{
				// Title
				if ((lines) || (g_Radiotext->RT_PTY !=0)) 
				{
					sprintf(stext[0], g_Radiotext->RT_PTY == 0 ? "%s %s%s" : "%s (%s)%s", tr("Radiotext"), g_Radiotext->RT_PTY == 0 ? g_Radiotext->RDS_PTYN : g_Radiotext->ptynr2string(g_Radiotext->RT_PTY), ":");
					
					// shadow
					frameBuffer->paintBoxRel(rt_x + SHADOW_OFFSET, rt_y + SHADOW_OFFSET, rt_dx, rt_dy, COL_INFOBAR_SHADOW_PLUS_0, RADIUS_MID, CORNER_TOP);
					frameBuffer->paintBoxRel(rt_x, rt_y, rt_dx, rt_dy, COL_INFOBAR_PLUS_0, RADIUS_MID, CORNER_TOP);
					g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(rt_x + 10, rt_y + 30, rt_dx - 20, stext[0], COL_INFOBAR, 0, RTisIsUTF); // UTF-8
				}
				yoff = 17;
				ii = 1;
			}
			
			// Body
			if (lines) 
			{
				frameBuffer->paintBoxRel(rt_x + SHADOW_OFFSET, rt_y+rt_dy+SHADOW_OFFSET, rt_dx, 7+rt_dy* g_Radiotext->S_RtOsdRows, COL_INFOBAR_SHADOW_PLUS_0, RADIUS_MID, CORNER_BOTTOM);

				frameBuffer->paintBoxRel(rt_x, rt_y+rt_dy, rt_dx, 7+rt_dy* g_Radiotext->S_RtOsdRows, COL_INFOBAR_PLUS_0, RADIUS_MID, CORNER_BOTTOM);

				// RT-Text roundloop
				int ind = (g_Radiotext->RT_Index == 0) ? g_Radiotext->S_RtOsdRows - 1 : g_Radiotext->RT_Index - 1;
				int rts_x = rt_x + 10;
				int rts_y = rt_y + 30;
				int rts_dx = rt_dx - 20;
				
				if (g_Radiotext->S_RtOsdLoop == 1) 
				{ 
					// latest bottom
					for (int i = ind+1; i < g_Radiotext->S_RtOsdRows; i++)
						g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(rts_x, rts_y + (ii++)*rt_dy, rts_dx, g_Radiotext->RT_Text[i], COL_INFOBAR, 0, RTisIsUTF); // UTF-8
					for (int i = 0; i <= ind; i++)
						g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(rts_x, rts_y + (ii++)*rt_dy, rts_dx, g_Radiotext->RT_Text[i], COL_INFOBAR, 0, RTisIsUTF); // UTF-8
				}
				else 
				{ 
					// latest top
					for (int i = ind; i >= 0; i--)
						g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(rts_x, rts_y + (ii++)*rt_dy, rts_dx, g_Radiotext->RT_Text[i], COL_INFOBAR, 0, RTisIsUTF); // UTF-8
					for (int i = g_Radiotext->S_RtOsdRows-1; i > ind; i--)
						g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(rts_x, rts_y + (ii++)*rt_dy, rts_dx, g_Radiotext->RT_Text[i], COL_INFOBAR, 0, RTisIsUTF); // UTF-8
				}
			}
		}
	}
	
	g_Radiotext->RT_MsgShow = false;
}

int CInfoViewer::handleMsg(const neutrino_msg_t msg, neutrino_msg_data_t data)
{
 	if ((msg == NeutrinoMessages::EVT_CURRENTNEXT_EPG) || (msg == NeutrinoMessages::EVT_NEXTPROGRAM)) 
	{
	  	getEPG(*(t_channel_id *)data, info_CurrentNext);
	  	if ( is_visible )
			show_Data(true);
			
#if ENABLE_LCD			
	  	showLcdPercentOver();
#endif			

		return messages_return::handled;
  	} 
	else if (msg == NeutrinoMessages::EVT_TIMER) 
	{
#if defined (ENABLE_LCD)	  
		if (data == lcdUpdateTimer) 
		{
	  		showLcdPercentOver();		

	  		return messages_return::handled;
		}
#endif	
  	} 
	else if (msg == NeutrinoMessages::EVT_RECORDMODE) 
	{	  
		recordModeActive = data;
		
		if(is_visible) 
			showRecordIcon(true);		
  	} 
	else if (msg == NeutrinoMessages::EVT_ZAP_GOTAPIDS) 
	{
		if ((*(t_channel_id *) data) == channel_id) 
		{
	  		if ( is_visible && showButtonBar )
				showButton_Audio();
		}
		return messages_return::handled;
  	} 
	else if (msg == NeutrinoMessages::EVT_ZAP_GOTPIDS) 
	{
		if ((*(t_channel_id *) data) == channel_id) 
		{
	  		if ( is_visible && showButtonBar ) 
			{
				showIcon_VTXT();
				showIcon_SubT();
				showIcon_CA_Status(0);
				showIcon_Resolution();
				showIcon_16_9();
	  		}
		}
		return messages_return::handled;
  	} 
	else if (msg == NeutrinoMessages::EVT_ZAP_GOT_SUBSERVICES) 
	{
		if ((*(t_channel_id *) data) == channel_id) 
		{
	  		if ( is_visible && showButtonBar )
				showButton_SubServices();
		}
		return messages_return::handled;
  	} 
  	else if ((msg == NeutrinoMessages::EVT_ZAP_COMPLETE) || (msg == NeutrinoMessages::EVT_ZAP_ISNVOD))
	{
		chanready = 1;
		showSNR();
		showAktivTuner();
		
		if ( is_visible && showButtonBar ) 
			showIcon_Resolution();
		
		channel_id = (*(t_channel_id *)data);
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_ZAP_SUB_COMPLETE) 
	{
		chanready = 1;
		showSNR();
		showAktivTuner();
		
		if ( is_visible && showButtonBar ) 
			showIcon_Resolution();

		//if ((*(t_channel_id *)data) == channel_id)
		{
	  		if ( is_visible && showButtonBar && (!g_RemoteControl->are_subchannels))
				show_Data(true);
		}

#if ENABLE_LCD
		showLcdPercentOver();
#endif		

		return messages_return::handled;
  	} 
	else if (msg == NeutrinoMessages::EVT_ZAP_SUB_FAILED) 
	{
		chanready = 1;

		showSNR();
		showAktivTuner();
		
		if ( is_visible && showButtonBar ) 
			showIcon_Resolution();

		// show failure..!
		if (CVFD::getInstance()->is4digits)
			CVFD::getInstance()->LCDshowText(g_RemoteControl->getCurrentChannelNumber());
		else
			CVFD::getInstance()->showServicename("(" + g_RemoteControl->getCurrentChannelName() + ')');
		
		dprintf(DEBUG_NORMAL, "CInfoViewer::handleMsg: zap failed!\n");
		showFailure();

#if ENABLE_LCD		
		CVFD::getInstance()->showPercentOver(255);
#endif		

		return messages_return::handled;
  	} 
	else if (msg == NeutrinoMessages::EVT_ZAP_FAILED) 
	{
		chanready = 1;

		showSNR();
		showAktivTuner();
		
		if ( is_visible && showButtonBar ) 
			showIcon_Resolution();

		if ((*(t_channel_id *) data) == channel_id) 
		{
	  		// show failure..!
			if (CVFD::getInstance()->is4digits)
				CVFD::getInstance()->LCDshowText(g_RemoteControl->getCurrentChannelNumber());
			else
				CVFD::getInstance()->showServicename ("(" + g_RemoteControl->getCurrentChannelName () + ')');
			
	  		dprintf(DEBUG_NORMAL, "CInfoViewer::handleMsg: zap failed!\n");
	  		showFailure ();

#if ENABLE_LCD			
	  		CVFD::getInstance()->showPercentOver(255);
#endif			
		}
		return messages_return::handled;
  	} 
	else if (msg == NeutrinoMessages::EVT_ZAP_MOTOR) 
	{
		chanready = 0;
		showMotorMoving (data);
		return messages_return::handled;
  	} 
	else if (msg == NeutrinoMessages::EVT_MODECHANGED) 
	{
		if ( is_visible && showButtonBar )
	  		showIcon_16_9();

		return messages_return::handled;
  	} 
	else if (msg == NeutrinoMessages::EVT_TIMESET) 
	{
		gotTime = true;
		return messages_return::handled;
  	} 
	else if (msg == NeutrinoMessages::EVT_ZAP_CA_CLEAR) 
	{
		Set_CA_Status(false);
		return messages_return::handled;
  	} 
	else if (msg == NeutrinoMessages::EVT_ZAP_CA_LOCK) 
	{
		Set_CA_Status(true);
		return messages_return::handled;
  	} 
	else if (msg == NeutrinoMessages::EVT_ZAP_CA_FTA) 
	{
		Set_CA_Status(false);
		return messages_return::handled;
  	}
	else if (msg == NeutrinoMessages::EVT_ZAP_CA_ID) 
	{
		chanready = 1;
		Set_CA_Status(data);

		showSNR();
		showAktivTuner();

		return messages_return::handled;
  	}

  	return messages_return::unhandled;
}

void CInfoViewer::showButton_SubServices()
{
  	if (!(g_RemoteControl->subChannels.empty ())) 
	{
		int icon_w;
		int icon_h;
		
		frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_YELLOW, &icon_w, &icon_h);
	
		if(is_visible)
		{
			frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_YELLOW, BoxStartX + BORDER_LEFT + icon_w + ICON_TO_ICON_OFFSET + asize + icon_w + ICON_TO_ICON_OFFSET + asize, buttonBarStartY + (buttonBarHeight - icon_h)/2 );

			g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(BoxStartX + BORDER_LEFT + icon_w + ICON_TO_ICON_OFFSET + asize + icon_w + ICON_TO_ICON_OFFSET + asize + icon_w + ICON_TO_ICON_OFFSET, buttonBarStartY + (buttonBarHeight - g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight(), asize, g_Locale->getText((g_RemoteControl->are_subchannels) ? LOCALE_INFOVIEWER_SUBSERVICE : LOCALE_INFOVIEWER_SELECTTIME), COL_INFOBAR_BUTTONS, 0, true); // UTF-8
		}
  	}
}

void CInfoViewer::getEPG(const t_channel_id for_channel_id, CSectionsdClient::CurrentNextInfo &info)
{
	dprintf(DEBUG_NORMAL, "CInfoViewer::getEPG: channel_id:0x%llx\n", for_channel_id);

	// to clear the oldinfo for channels without epg, call getEPG() with for_channel_id = 0
	if (for_channel_id == 0)
	{
		return;
	}

	sectionsd_getCurrentNextServiceKey(for_channel_id & 0xFFFFFFFFFFFFULL, info);

	// if there is no EPG, send an event so that parental lock can work
	if (info.current_uniqueKey == 0 && info.next_uniqueKey == 0) 
	{
		char *p = new char[sizeof(t_channel_id)];
		memcpy(p, &for_channel_id, sizeof(t_channel_id));

		//
		g_RCInput->postMsg(NeutrinoMessages::EVT_NOEPG_YET, (const neutrino_msg_data_t) p, false);
		
		return;
	}

	if (info.current_uniqueKey != 0 || info.next_uniqueKey != 0)
	{
		char *p = new char[sizeof(t_channel_id)];
		memcpy(p, &for_channel_id, sizeof(t_channel_id));
		neutrino_msg_t msg;
		
		if (info.flags & (CSectionsdClient::epgflags::has_current | CSectionsdClient::epgflags::has_next))
		{
			if (info.flags & CSectionsdClient::epgflags::has_current)
				msg = NeutrinoMessages::EVT_CURRENTEPG;
			else
				msg = NeutrinoMessages::EVT_NEXTEPG;
		}
		else
			msg = NeutrinoMessages::EVT_NOEPG_YET;

		//
		g_RCInput->postMsg(msg, (const neutrino_msg_data_t)p, false); // data is pointer to allocated memory
	}
}

void CInfoViewer::showSNR()
{ 
  	char percent[10];
  	uint16_t ssig = 0;
	int ssnr = 0;
  	int sw = 0;
	int snr = 0;
	int sig = 0;
	int posx = 0;
	int posy = 0;
  	int barwidth = BAR_WIDTH;
	
  	if (g_settings.infobar_sat_display && !g_settings.satip_allow_satip && CNeutrinoApp::getInstance()->getMode() != NeutrinoMessages::mode_iptv) 
	{
		if(is_visible)
		{
			// freq
			if (chanready) 
			{
				char freq[20];
				
				// get current service info
				CZapitClient::CCurrentServiceInfo si = g_Zapit->getCurrentServiceInfo();
				
				// freq
				if(live_fe != NULL)
				{
					if( live_fe->getInfo()->type == FE_QPSK || live_fe->getInfo()->type == FE_QAM)
					{
						sprintf (freq, "FREQ:%d.%d MHz", si.tsfrequency / 1000, si.tsfrequency % 1000);
					}
					else if( live_fe->getInfo()->type == FE_OFDM)
					{
						sprintf (freq, "FREQ:%d.%d MHz", si.tsfrequency / 1000000, si.tsfrequency % 1000);
					}
				}
				else
					sprintf (freq, "FREQ:%d.%d MHz", si.tsfrequency / 1000, si.tsfrequency % 1000);

				freqWidth = g_SignalFont->getRenderWidth(freq);
				// see above comment about lenght of sat name
				freqStartX = BoxStartX + satNameWidth + 20;

				g_SignalFont->RenderString(freqStartX, BoxStartY + (SAT_INFOBOX_HEIGHT - SatNameHeight)/2 + SatNameHeight, freqWidth, freq, COL_INFOBAR );
			
				if(live_fe != NULL)
				{
					ssig = live_fe->getSignalStrength();
					ssnr = live_fe->getSignalNoiseRatio();
				}

				//sig = (ssig & 0xFFFF) * 100 / 65535;
				//snr = (ssnr & 0xFFFF) * 100 / 65535;
				sig = ((ssig * 100 + 0x8001) >> 16);
				snr = ((ssnr * 100 + 0x8001) >> 16);
				
				posy = BoxStartY + (SAT_INFOBOX_HEIGHT - SatNameHeight)/2 + SatNameHeight;

				// sig
				// I commented this out because in case : (sigscale->getPercent() == sig)
				// ( and that really happens here )
				// then posx has value 0 and snrscale would be painted left outside infobar
				// result : osd fragments
				// therefore > sig- and snrscale always painting
				//if (sigscale->getPercent() != sig) 
				{
					posx = freqStartX + freqWidth + 10;

					sigscale->paint(posx, BoxStartY + (SAT_INFOBOX_HEIGHT - SIGSCALE_BAR_HEIGHT)/2, sig);

					sprintf (percent, "SIG:%d%%S", sig);
					posx = posx + barwidth + 2;
					sw = g_SignalFont->getRenderWidth(percent);

					g_SignalFont->RenderString (posx, posy, sw, percent, COL_INFOBAR );
				}

				// snr
				// see comment at sig
				//if (snrscale->getPercent() != snr) 
				{
					int snr_posx = posx + sw + 10;

					snrscale->paint(snr_posx, BoxStartY + (SAT_INFOBOX_HEIGHT - SIGSCALE_BAR_HEIGHT)/2, snr);

					sprintf (percent, "SNR:%d%%Q", snr);
					snr_posx = snr_posx + barwidth + 2;
					sw = g_SignalFont->getRenderWidth(percent);
					
					g_SignalFont->RenderString (snr_posx, posy, sw, percent, COL_INFOBAR );
				}
			}
		}
  	} 	
}

// aktiv tuner
void CInfoViewer::showAktivTuner()
{ 
	/*
  	if (!g_settings.satip_allow_satip) 
	{
		if(is_visible)
		{
			if (chanready) 
			{
				//show aktiv tuner
				if( FrontendCount > 1 )
				{	
					char AktivTuner[255] = "T0";
					
					int Index = 0;
					
					for(int i = 0; i < FrontendCount; i++)
					{
						CFrontend * fe = getFE(i);
						
						if(live_fe != NULL)
						{
							if(fe->fenumber == live_fe->fenumber && fe->fe_adapter == live_fe->fe_adapter)
								Index = i;
						}
						else
							Index = 0;
					}
					
					if(live_fe != NULL)
						sprintf(AktivTuner, "T%d", (Index + 1));
					
					g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(BoxEndX - (BORDER_RIGHT + BORDER_LEFT + icon_w_subt + 2 + icon_w_vtxt + 2 + icon_w_dd + 2 + icon_w_aspect + 2 + icon_w_sd + 2 + icon_w_reso + 2 + icon_w_ca + 2 + icon_w_rt + 2 + TunerNumWidth), buttonBarStartY + (buttonBarHeight - TunerNumHeight)/2 + TunerNumHeight, TunerNumWidth, AktivTuner, COL_INFOBAR_BUTTONS, 0, true); // UTF-8
				}
			}
		}
  	}
	*/ 	
}

void CInfoViewer::show_Data(bool calledFromEvent)
{
  	char runningStart[10] = "";
  	char runningRest[20] = "";
  	runningPercent = 0;
  	static char oldrunningPercent = 255;

  	char nextStart[10] = "";
  	char nextDuration[10] = "";

  	int is_nvod = false;

  	if (is_visible) 
	{
		// start zeit/dauer
		if ((g_RemoteControl->current_channel_id == channel_id) && (g_RemoteControl->subChannels.size () > 0) && (!g_RemoteControl->are_subchannels)) 
		{
	  		is_nvod = true;
	  		info_CurrentNext.current_zeit.startzeit = g_RemoteControl->subChannels[g_RemoteControl->selected_subchannel].startzeit;
	  		info_CurrentNext.current_zeit.dauer = g_RemoteControl->subChannels[g_RemoteControl->selected_subchannel].dauer;
		} 
		else 
		{
	  		if ((info_CurrentNext.flags & CSectionsdClient::epgflags::has_current) && (info_CurrentNext.flags & CSectionsdClient::epgflags::has_next) && (showButtonBar) ) 
			{
				if ((uint) info_CurrentNext.next_zeit.startzeit < (info_CurrentNext.current_zeit.startzeit + info_CurrentNext.current_zeit.dauer)) {
		  			is_nvod = true;
				}
	  		}
		}

		// seit / rest /runningPercent
		time_t jetzt = time(NULL);

		if (info_CurrentNext.flags & CSectionsdClient::epgflags::has_current) 
		{
	  		int seit = (jetzt - info_CurrentNext.current_zeit.startzeit + 30) / 60;
	  		int rest = (info_CurrentNext.current_zeit.dauer / 60) - seit;
			
	  		if (seit < 0) 
			{
				runningPercent = 0;
				sprintf (runningRest, "in %d min", -seit);
	  		} 
			else 
			{
				runningPercent = (unsigned) ((float) (jetzt - info_CurrentNext.current_zeit.startzeit) / (float) info_CurrentNext.current_zeit.dauer * 100.);

				if(runningPercent > 100)
					runningPercent = 100;

				sprintf (runningRest, "%d / %d min", seit, rest);
	  		}

	  		struct tm * pStartZeit = localtime(&info_CurrentNext.current_zeit.startzeit);
	  		sprintf(runningStart, "%02d:%02d", pStartZeit->tm_hour, pStartZeit->tm_min);
		} 

		if (info_CurrentNext.flags & CSectionsdClient::epgflags::has_next) 
		{
	  		unsigned dauer = info_CurrentNext.next_zeit.dauer / 60;
	  		sprintf (nextDuration, "%d min", dauer);
	  		struct tm *pStartZeit = localtime (&info_CurrentNext.next_zeit.startzeit);
	  		sprintf(nextStart, "%02d:%02d", pStartZeit->tm_hour, pStartZeit->tm_min);
		} 
		
	  	//percent
	  	if (info_CurrentNext.flags & CSectionsdClient::epgflags::has_current) 
		{
			if(!calledFromEvent || (oldrunningPercent != runningPercent)) 
			{
				oldrunningPercent = runningPercent;
			}
	  	} 
		else 
		{
			oldrunningPercent = 255;
	  	}

		// timescale
		timescale->paint(timescale_posx, timescale_posy, runningPercent);

		int EPGTimeWidth = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->getRenderWidth("00:00:00"); //FIXME

		// paint red button
		if (showButtonBar) 
		{
	  		if (info_CurrentNext.flags & CSectionsdClient::epgflags::has_anything) 
			{
				// red button
				frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_RED, BoxStartX + BORDER_LEFT, buttonBarStartY + (buttonBarHeight - icon_red_h)/2 );

				g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(BoxStartX + BORDER_LEFT + icon_red_w + ICON_OFFSET, buttonBarStartY + (buttonBarHeight - g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight(), asize - 5, g_Locale->getText(LOCALE_INFOVIEWER_EVENTLIST), COL_INFOBAR_BUTTONS, 0, true); // UTF-8
	  		}
		}

		// paint epg infos
		if ( (info_CurrentNext.flags & CSectionsdClient::epgflags::not_broadcast) || ((calledFromEvent) && !(info_CurrentNext.flags & (CSectionsdClient::epgflags::has_next | CSectionsdClient::epgflags::has_current)))) 
		{
			// noepg/waiting for time
	  		g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->RenderString(ChanInfoX, ChanInfoY + 2*ChanInfoHeight, BoxEndX - (BoxStartX + CHANNUMBER_WIDTH + 20), g_Locale->getText (gotTime ? LOCALE_INFOVIEWER_NOEPG : LOCALE_INFOVIEWER_WAITTIME), COL_INFOBAR, 0, true);	// UTF-8
		} 
		else 
		{
	  		// found some epg
	  		int duration1Width = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->getRenderWidth(runningRest);
	  		int duration1TextPos = BoxEndX - duration1Width - BORDER_RIGHT;

	  		int duration2Width = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->getRenderWidth(nextDuration);
	  		int duration2TextPos = BoxEndX - duration2Width - BORDER_RIGHT;

	  		if ((info_CurrentNext.flags & CSectionsdClient::epgflags::has_next) && (!(info_CurrentNext.flags & CSectionsdClient::epgflags::has_current))) 
			{
				// current infos
				g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->RenderString (ChanInfoX, ChanInfoY + ChanInfoHeight, BoxEndX - ChanInfoX, g_Locale->getText(LOCALE_INFOVIEWER_NOCURRENT), COL_COLORED_EVENTS_INFOBAR, 0, true);	// UTF-8

				// next
				g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->RenderString(BoxStartX + BORDER_LEFT, ChanInfoY + 2*ChanInfoHeight, EPGTimeWidth, nextStart, COL_MENUCONTENTINACTIVE);

				g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->RenderString(ChanInfoX, ChanInfoY + 2*ChanInfoHeight, duration2TextPos - ChanInfoX - 5, info_CurrentNext.next_name, COL_MENUCONTENTINACTIVE, 0, true);
				g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->RenderString(duration2TextPos, ChanInfoY + 2*ChanInfoHeight, duration2Width, nextDuration, COL_MENUCONTENTINACTIVE);
	  		} 
			else 
			{
				// current
			  	g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->RenderString (BoxStartX + 10, ChanInfoY + ChanInfoHeight, EPGTimeWidth, runningStart, COL_COLORED_EVENTS_INFOBAR);

			  	g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->RenderString (ChanInfoX, ChanInfoY + ChanInfoHeight, duration1TextPos - ChanInfoX - 5, info_CurrentNext.current_name, COL_COLORED_EVENTS_INFOBAR, 0, true);

		  		g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->RenderString (duration1TextPos, ChanInfoY + ChanInfoHeight, duration1Width, runningRest, COL_COLORED_EVENTS_INFOBAR);

				// next 
				if ((!is_nvod) && (info_CurrentNext.flags & CSectionsdClient::epgflags::has_next)) 
				{
					g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->RenderString (BoxStartX + BORDER_LEFT, ChanInfoY + 2*ChanInfoHeight, EPGTimeWidth, nextStart, COL_MENUCONTENTINACTIVE);

					
					g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->RenderString (ChanInfoX, ChanInfoY + 2*ChanInfoHeight, duration2TextPos - ChanInfoX - 5, info_CurrentNext.next_name, COL_MENUCONTENTINACTIVE, 0, true);

					g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO]->RenderString (duration2TextPos, ChanInfoY + 2*ChanInfoHeight, duration2Width, nextDuration, COL_MENUCONTENTINACTIVE);
				} 
	  		}
		}
  	}
}

void CInfoViewer::showButton_Audio()
{
  	// green, in case of several APIDs
  	uint32_t count = g_RemoteControl->current_PIDs.APIDs.size();
	
	int icon_w;
	int icon_h;
		
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_GREEN, &icon_w, &icon_h);

	if(is_visible)
	{
		frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_GREEN, BoxStartX + BORDER_LEFT + icon_w + ICON_TO_ICON_OFFSET + asize, buttonBarStartY + (buttonBarHeight - icon_h)/2 );

	  	if (count > 0 || CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_iptv) 
		{
			int sx = BoxStartX + BORDER_LEFT + icon_w + ICON_TO_ICON_OFFSET + asize + icon_w + ICON_TO_ICON_OFFSET;

			g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(sx, buttonBarStartY + (buttonBarHeight - g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight(), asize - 5 - icon_w, g_Locale->getText(LOCALE_INFOVIEWER_LANGUAGES), COL_INFOBAR_BUTTONS, 0, true); // UTF-8
	  	}
	}

	int ac3state;
  	if ((g_RemoteControl->current_PIDs.PIDs.selected_apid < count) && (g_RemoteControl->current_PIDs.APIDs[g_RemoteControl->current_PIDs.PIDs.selected_apid].is_ac3))
	{
		ac3state = AC3_ACTIVE;
	}
  	else if (g_RemoteControl->has_ac3)
	{
		ac3state = AC3_AVAILABLE;
	}
  	else
	{
		ac3state = NO_AC3;
	}

	//
	//showIcon_Audio(ac3state);
	const char *dd_icon;
	
	switch (ac3state)
	{
		case AC3_ACTIVE:
			dd_icon = NEUTRINO_ICON_DD;
			break;
			
		case AC3_AVAILABLE:
			dd_icon = NEUTRINO_ICON_DD_AVAIL;
			break;
			
		case NO_AC3:
		default:
			dd_icon = NEUTRINO_ICON_DD_GREY;
			break;
	}

	if(is_visible)
		frameBuffer->paintIcon(dd_icon, BoxEndX - (BORDER_RIGHT + icon_w_subt + ICON_TO_ICON_OFFSET + icon_w_vtxt + ICON_TO_ICON_OFFSET + icon_w_dd), buttonBarStartY + (buttonBarHeight - icon_h_dd)/2 );
	
	if (ac3state == AC3_ACTIVE)
	{
		CVFD::getInstance()->ShowIcon(VFD_ICON_DOLBY, true);
#if defined(PLATFORM_SPARK7162)
		CVFD::getInstance()->ShowIcon(VFD_ICON_AC3, true);
		CVFD::getInstance()->ShowIcon(VFD_ICON_MP3, false);//FIXME:@dbo: why???
#endif
		//CVFD::getInstance()->ShowIcon(VFD_ICON_MP3, false);//FIXME:@dbo: why???
	}
	else
	{
		CVFD::getInstance()->ShowIcon(VFD_ICON_DOLBY, false);
#if defined(PLATFORM_SPARK7162)
		CVFD::getInstance()->ShowIcon(VFD_ICON_AC3, false);
		CVFD::getInstance()->ShowIcon(VFD_ICON_MP3, true); //FIXME:@dbo: why???
#endif
	}
}

void CInfoViewer::killTitle()
{
  	if (is_visible) 
	{
		is_visible = false;

		frameBuffer->paintBackgroundBox(BoxStartX, BoxStartY - 30, BoxEndX + SHADOW_OFFSET, buttonBarStartY + SHADOW_OFFSET + buttonBarHeight);

		// hide radiotext	
		if (g_settings.radiotext_enable && g_Radiotext) 
		{
			g_Radiotext->S_RtOsd = g_Radiotext->haveRadiotext() ? 1 : 0;
			killRadiotext();
		}

		frameBuffer->blit();

		if(sigscale)
		{
			delete sigscale;
			sigscale = NULL;
		}
		
		if(snrscale)
		{
			delete snrscale;
			snrscale = NULL;
		}
		
		if(timescale)
		{
			delete timescale;
			timescale = NULL;
		}
  	}
}

void CInfoViewer::Set_CA_Status(int Status)
{
	CA_Status = Status;
	m_CA_Status = Status;

	if ( is_visible && showButtonBar )
		showIcon_CA_Status(1);
}

#if ENABLE_LCD
void CInfoViewer::showLcdPercentOver()
{
	if (g_settings.lcd_setting[SNeutrinoSettings::LCD_SHOW_VOLUME] != 1) 
	{
		int runningPercent = -1;
		time_t jetzt = time (NULL);

		if (info_CurrentNext.flags & CSectionsdClient::epgflags::has_current) 
		{
			if (jetzt < info_CurrentNext.current_zeit.startzeit)
				runningPercent = 0;
			else
				runningPercent = MIN ((unsigned) ((float) (jetzt - info_CurrentNext.current_zeit.startzeit) / (float) info_CurrentNext.current_zeit.dauer * 100.), 100);
		}

		CVFD::getInstance()->showPercentOver(runningPercent);	
	}
}
#endif

extern int pmt_caids[11];

void CInfoViewer::showIcon_CA_Status(int notfirst)
{
	int i;
	int caids[] = { 0x0600, 0x1700, 0x0100, 0x0500, 0x1800, 0x0B00, 0x0D00, 0x0900, 0x2600, 0x4a00, 0x0E00 };
	
	if(is_visible)
	{	
		if(!notfirst) 
		{
			bool fta = true;
			
			for(i = 0; i < (int)(sizeof(caids)/sizeof(int)); i++) 
			{
				if (pmt_caids[i]) 
				{
					fta = false;
					break;
				}
			}
		
			frameBuffer->paintIcon( fta ? NEUTRINO_ICON_SCRAMBLED2_GREY : NEUTRINO_ICON_SCRAMBLED2, BoxEndX - (ICON_OFFSET + icon_w_subt + 2 + icon_w_vtxt + 2 + icon_w_dd + 2 + icon_w_aspect + 2 + icon_w_sd + 2 + icon_w_reso + 2 + icon_w_ca), buttonBarStartY + (buttonBarHeight - icon_h_ca)/2 );

	#if !defined (PLATFORM_COOLSTREAM)
			CVFD::getInstance()->ShowIcon(VFD_ICON_LOCK, !fta);
	#endif			
			return;
		}
	}
}

void CInfoViewer::showEpgInfo()   //message on event change
{
	int mode = CNeutrinoApp::getInstance()->getMode();
	
	// show epg info only if we in TV- or Radio mode and current event is not the same like before
	if ((eventname != info_CurrentNext.current_name) && (mode == NeutrinoMessages::mode_tv || mode == NeutrinoMessages::mode_radio))
	{
		eventname = info_CurrentNext.current_name;
		
		g_RCInput->postMsg(NeutrinoMessages::SHOW_INFOBAR , 0);
	}
}

int CInfoViewerHandler::exec(CMenuTarget * parent, const std::string &/*actionkey*/)
{
	dprintf(DEBUG_NORMAL, "CInfoViewerHandler::exec:\n");


	int res = menu_return::RETURN_EXIT_ALL;
	CChannelList * channelList;
	CInfoViewer * i;
	
	if (parent) 
		parent->hide();
	
	i = new CInfoViewer;
	
	channelList = CNeutrinoApp::getInstance()->channelList;
	i->start();
	i->show(channelList->getActiveChannelNumber(), channelList->getActiveChannelName(), channelList->getActiveSatellitePosition(), channelList->getActiveChannel_ChannelID());	// UTF-8

	delete i;
	
	return res;
}


