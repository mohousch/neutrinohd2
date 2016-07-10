/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: infoviewer.h 2013/09/03 10:45:30 mohousch Exp $

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


#ifndef __infoview__
#define __infoview__

#include <sectionsdclient/sectionsdclient.h>

#include <driver/rcinput.h>
#include <driver/framebuffer.h>
#include <driver/fontrenderer.h>
#include <system/settings.h>
#include "widget/menue.h"
#include <gui/widget/progressbar.h>

#include <string>


class CInfoViewer
{
	private:
		void Init(void);
		CFrameBuffer *frameBuffer;
		
		bool gotTime;
		bool recordModeActive;
		bool CA_Status;
		bool showButtonBar;
		
		int BoxEndX;
		int BoxEndY;
		int BoxStartX;
		int BoxStartY;
		int BoxHeight;
		int BoxWidth;

		// channel number
		int ChanNumberX;
		int ChanNumberY;
		
		// channel name
		int ChanNameX;
		int ChanNameY;
		int ChanNameWidth;
		int ChanNameHeight;
		
		// channel info
		int ChanInfoX;
		int ChanInfoY;
		int ChanInfoHeight;
		
		// sat info
		int satNameWidth;
		int SatNameHeight;
		int freqStartX;
		int freqWidth;
		
		// buttonbar
		int buttonBarStartX;
		int buttonBarStartY;
		int buttonBarHeight;
		
		// icons
		int icon_w_subt, icon_h_subt;
		int icon_w_vtxt, icon_h_vtxt;
		int icon_w_aspect, icon_h_aspect;
		int icon_w_dd, icon_h_dd;
		int icon_w_sd, icon_h_sd;
		int icon_hd_w, icon_hd_h;
		int icon_w_reso, icon_h_reso;
		int icon_w_ca, icon_h_ca;
		int icon_w_rt, icon_h_rt;
		int icon_w_rec, icon_h_rec;
		int icon_red_w, icon_red_h;
		int icon_green_w, icon_green_h;
		int icon_yellow_w, icon_yellow_h;
		int icon_blue_w, icon_blue_h;
		
		int icn_red_posx;
		int icon_green_posx;
		int icon_yellow_posx;
		int icon_blue_posx;
		
		// tuner
		int TunerNumWidth;
		int TunerNumHeight;
		
		// channel logo
		int PIC_W;
		int PIC_H;
		int logo_w; 
		int logo_h;
		int logo_bpp;
		int PIC_X;
		int PIC_Y;
		
		// date
		int dateWidth;
		int dateHeight;
		
		// ca
		int m_CA_Status;
		
		// dimensions of radiotext window		
		int             rt_dx;
		int             rt_dy;
		int             rt_x;
		int             rt_y;
		int             rt_h;
		int             rt_w;	

		int		asize;

		CSectionsdClient::CurrentNextInfo info_CurrentNext;
		t_channel_id   channel_id;

		char           aspectRatio;
		uint32_t       sec_timer_id;
		bool           virtual_zap_mode;
		
		CChannelEventList               evtlist;
		CChannelEventList::iterator     eli;

		void show_Data( bool calledFromEvent = false );
		void paintTime( bool show_dot, bool firstPaint, int posx, int posy, CFont * timeFont );
		
		void showButton_Audio();
		void showButton_SubServices();
		
		void showIcon_16_9();		
		void showIcon_RadioText(bool rt_available) const;		
	
		void showIcon_CA_Status(int);
		void paint_ca_icons(int, char*);

		void showIcon_VTXT()      const;
		void showRecordIcon(const bool show);
		void showIcon_SubT() const;

		void showIcon_Resolution() const;
		void showIcon_Audio(const int ac3state) const;
		
		void showFailure();
		void showMotorMoving(int duration);
		void showLcdPercentOver();
		void showSNR();		

		CProgressBar *snrscale, *sigscale, *timescale;
		std::string eventname;

 public:
		bool 		chanready;
		bool		is_visible;

#if defined (ENABLE_LCD)
		uint32_t    	lcdUpdateTimer;
#endif		

		CInfoViewer();

		void	start();

		void	showTitle(const int ChanNum, const std::string & Channel, const t_satellite_position satellitePosition, const t_channel_id new_channel_id = 0, const bool calledFromNumZap = false, int epgpos = 0); // Channel must be UTF-8 encoded

		enum
		{
			NO_AC3,
			AC3_AVAILABLE,
			AC3_ACTIVE
		};

		void lookAheadEPG(const int ChanNum, const std::string & Channel, const t_channel_id new_channel_id = 0, const bool calledFromNumZap = false); //alpha: fix for nvod subchannel update
		void killTitle();
		
		void getEPG(const t_channel_id for_channel_id, CSectionsdClient::CurrentNextInfo &info);
		CSectionsdClient::CurrentNextInfo getCurrentNextInfo() { return info_CurrentNext; }
	
		void showSubchan();
		void Set_CA_Status(int Status);
	
		int handleMsg(const neutrino_msg_t msg, neutrino_msg_data_t data);
		void clearVirtualZapMode() {virtual_zap_mode = false;}
		
		void showEpgInfo();
				
		void showRadiotext();
		void killRadiotext();

		// movie infoviewer
		CProgressBar *moviescale;
		bool m_visible;
		void showMovieInfo(const std::string &Title, const std::string &Info, short Percent, const int duration, const unsigned int ac3state, const int speed, const int playstate, bool show_duration = true, bool show_bookmark = false);
};

class CInfoViewerHandler : public CMenuTarget
{
	public:
		int  exec( CMenuTarget* parent,  const std::string &actionkey);
		int  doMenu();

};

#endif
