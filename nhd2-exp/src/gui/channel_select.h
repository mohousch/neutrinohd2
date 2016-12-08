/*
	* $Id: channel_select.h 2015/07/ 11:23:30 mohousch Exp $

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

#ifndef __CHANNEL_SELECT__
#define __CHANNEL_SELECT__

#include <gui/widget/menue.h>

#include <string>

/*zapit includes*/
#include <client/zapitclient.h>

		
class CSelectChannelWidget : public CMenuWidget
{	
	private:
		t_channel_id ChannelTVID;
		t_channel_id ChannelRadioID;

		void InitZapitChannelHelper(CZapitClient::channelsMode mode);

	public:
		CSelectChannelWidget();
		~CSelectChannelWidget();
		int exec(CMenuTarget *parent, const std::string & actionKey);
		t_channel_id getChanTVID(){return ChannelTVID;};
		t_channel_id getChanRadioID(){return ChannelRadioID;};
};

#endif
