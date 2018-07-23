/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: bouqueteditor_channels.h 2013/10/12 mohousch Exp $

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


#ifndef __bouqueteditor_channels__
#define __bouqueteditor_channels__

#include <driver/framebuffer.h>
#include <gui/widget/menue.h>

// zapit
#include <client/zapitclient.h>
#include <channel.h>
#include <bouquets.h>

#include <string>


class CBEChannelWidget : public CMenuWidget
{
	public:
		ZapitChannelList* Channels;

	private:
		CFrameBuffer* frameBuffer;
	
		enum state_
		{
			beDefault,
			beMoving
		} state;

		unsigned int		selected;
		unsigned int		origPosition;
		unsigned int		newPosition;
		bool			channelsChanged;
		std::string		caption;
		CZapitClient::channelsMode mode;
		unsigned int bouquet;

		// gui
		unsigned int		liststart;
		unsigned int		listmaxshow;
		unsigned int		numwidth;
		int			fheight;
		int			theight;
		int 			info_height;
		int 			ButtonHeight;
		int		width;
		int		height;
		int		x;
		int		y;
		
		int icon_w_hd;
		int icon_h_hd;
		int icon_w_s;
		int icon_h_s;
		
		int icon_head_w;
		int icon_head_h;
		int icon_foot_w;
		int icon_foot_h;

		uint32_t sec_timer_id;

		void paintItem(int pos);
		void paint();
		void paintHead();
		void paintFoot();
		void paintDetails(int index);
		void clearItem2DetailsLine();
		void paintItem2DetailsLine(int pos);
		void hide();

		void deleteChannel();
		void addChannel();
		void beginMoveChannel();
		void finishMoveChannel();
		void cancelMoveChannel();
		void internalMoveChannel( unsigned int fromPosition, unsigned int toPosition);

	public:
		CBEChannelWidget( const std::string & Caption, unsigned int Bouquet);
		int exec(CMenuTarget* parent, const std::string & actionKey);

		bool hasChanged();
};

#endif
