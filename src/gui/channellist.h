/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: channellist.h 2013/10/12 mohousch Exp $

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

#ifndef __channellist__
#define __channellist__

#include <driver/framebuffer.h>
#include <gui/widget/menue.h>
#include <system/lastchannel.h>

#include <sectionsdclient/sectionsdclient.h>

/*zapit includes*/
#include <client/zapitclient.h>
#include <channel.h>

#include <string>
#include <vector>


enum {
	LIST_MODE_FAV,
	LIST_MODE_PROV,
	LIST_MODE_SAT,
	LIST_MODE_ALL
};

class CChannelList
{
	private:
		CFrameBuffer		*frameBuffer;
		unsigned int		selected;
		t_channel_id		selected_chid;
		CLastChannel		lastChList;
		unsigned int		liststart;
		unsigned int		listmaxshow;
		unsigned int		numwidth;
		int			iheight; 	// Fonthoehe Channellist-Inhalt
		int			theight; 	// Fonthoehe Channellist-Titel
		int			buttonHeight;

		std::string             name;
		std::vector<CZapitChannel *>	chanlist;
		CZapProtection * 	zapProtection;

		int 			width;
		int 			height;
		int 			x;
		int 			y;
		
		int icon_head_w;
		int icon_head_h;
		int icon_footer_w;
		int icon_footer_h;
		int icon_ca_w;
		int icon_ca_h;
		int icon_hd_w;
		int icon_hd_h;
		int icon_help_w;
		int icon_help_h;
		int icon_setup_w;
		int icon_setup_h;

		bool historyMode;
		bool vlist; 				// "virtual" list, not bouquet
		bool displayNext;

		SMSKeyInput c_SMSKeyInput;

		void paintDetails(int index);
		void clearItem2DetailsLine();
		void paintItem2DetailsLine(int pos);
		void paintItem(int pos);
		void paint();
		void paintHead();
		void hide();

	public:
		CChannelList(const char * const Name, bool _historyMode = false, bool _vlist = false );
		~CChannelList();
		
		void addChannel(CZapitChannel* chan, int num = 0);
		void putChannel(CZapitChannel* chan);
		
		CZapitChannel * getChannel(int number);
		CZapitChannel * getChannel(t_channel_id channel_id);
		CZapitChannel * getChannelFromIndex( uint32_t index) { if (chanlist.size() > index) return chanlist[index]; else return NULL;};
		CZapitChannel * operator[]( uint32_t index) { if (chanlist.size() > index) return chanlist[index]; else return NULL;};
		int getKey(int);

		const char * getName                   (void) const { return name.c_str(); };
		const std::string &  getActiveChannelName      (void) const; // UTF-8
		t_satellite_position getActiveSatellitePosition(void) const;
		int                  getActiveChannelNumber    (void) const;
		t_channel_id         getActiveChannel_ChannelID(void) const;

		void zapTo(int pos, bool forceStoreToLastChannels = false);
		void virtual_zap_mode(bool up);
		bool zapTo_ChannelID(const t_channel_id channel_id);
		bool adjustToChannelID(const t_channel_id channel_id, bool bToo = true);
		bool showInfo(int pos, int epgpos = 0);
		void updateEvents(void);
		int numericZap(int key);
		int show();
		int exec();
		void quickZap(int key, bool cycle = false);
		int  hasChannel(int nChannelNr);
		int  hasChannelID(t_channel_id channel_id);
		void setSelected( int nChannelNr); 		// for adjusting bouquet's channel list after numzap or quickzap

		int handleMsg(const neutrino_msg_t msg, neutrino_msg_data_t data);

		int getSize() const;
		int getSelectedChannelIndex() const;
		void setSize(int newsize);
		unsigned int		tuned;
		int doChannelMenu(void);
		void SortAlpha(void);
		void SortSat(void);
		void ClearList(void);
		
		bool canZap(CZapitChannel * channel = NULL);
};

#endif
