/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: bouqueteditor_chanselect.h 2013/10/12 mohousch Exp $

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

#ifndef __bouqueteditor_chanselect__
#define __bouqueteditor_chanselect__

#include <string>

#include <driver/framebuffer.h>
#include <gui/widget/window.h>
#include <gui/widget/scrollbar.h>

/*zapit includes*/
#include <client/zapitclient.h>
#include <channel.h>
#include <bouquets.h>


class CBEChannelSelectWidget
{
	private:
		//
		CFrameBuffer* frameBuffer;

		//
		CBox cFrameBox;
		CBox cFrameTitle;
		CBox cFrameFoot;
		CBox cFrameScrollBar;
		CBox cFrameItem;
		CBox cFrameFootInfo;

		CWindow cWindowTitle;
		CWindow cWindowFoot;
		CWindow cWindowFootInfo;

		CIcon footIcon;
		CIcon itemIcon1, itemIcon2, itemIcon3;

		uint32_t sec_timer_id;

		//
		unsigned int bouquet;
		CZapitClient::channelsMode mode;
		bool isChannelInBouquet(int index);

		bool modified;
		std::string caption;

		//
		unsigned int	selected;
		unsigned int	liststart;
		unsigned int	listmaxshow;

		void setModified(){modified = true;};

		uint getItemCount();
		void paintItem(int pos);

		//
		void paintHead();
		void paint();
		void paintFoot();
		void hide();
		
		//		
		void paintDetails(int index);
		void paintItem2DetailsLine(int pos);
		void clearItem2DetailsLine();

	public:
		ZapitChannelList Channels;
		ZapitChannelList * bouquetChannels;

		CBEChannelSelectWidget(const std::string & Caption, unsigned int Bouquet, CZapitClient::channelsMode Mode);
		int exec(CMenuTarget* parent, const std::string & actionKey);
		bool hasChanged();
};

#endif

