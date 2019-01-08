/*
	$Id: webtv.h 2013/09/03 10:45:30 mohousch Exp $
	based on martii webtv

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

#ifndef __webtv_h__
#define __webtv_h__

#include <string>
#include <vector>

#include <channel.h>
#include <client/zapitclient.h>


#define DEFAULT_WEBTV_FILE 		CONFIGDIR "/webtv/webtv.xml"

#include <string.h>


class CWebTV
{
	private:
		// bouquets
		std::string title;

	public:
		std::vector<CZapitChannel*> channels;

		void loadWebTVBouquet(std::string filename);
		
		enum state
		{
			STOPPED     =  0,
			PLAY        =  1,
			PAUSE       =  2
		};
		
		unsigned int playstate;
		
		CWebTV();
		~CWebTV();
		
		bool startPlayBack(t_channel_id chid);
		void stopPlayBack(void);
		void pausePlayBack(void);
		void continuePlayBack(void);

		const std::string& getChannelName(t_channel_id id);
		const std::string& getChannelURL(t_channel_id id);
		const std::string& getBouquetName(){return title;};

		std::vector<CZapitChannel*> getChannels(void){return channels;};
		
		//
		void loadChannels(void);
		void clearChannels(void);

		//
		CZapitClient::CCurrentServiceInfo getServiceInfo();
		void getPIDS(CZapitClient::responseGetPIDs& pids);

		unsigned int zapTo_ChannelID_NOWAIT(const t_channel_id channel_id);
};

#endif
