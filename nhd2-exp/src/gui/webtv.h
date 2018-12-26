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

#include <gui/widget/menue.h>

#include <channel.h>
#include <client/zapitclient.h>


#define DEFAULT_WEBTV_FILE 		CONFIGDIR "/webtv/webtv.xml"

#include <string.h>


class CWebTV : public CMenuTarget
{
	public:
		enum result_
		{
			close  = 0,
			resume = 1,
		} result;

	public:
		std::vector<CZapitChannel*> channels;
		
	private:
		// bouquets
		std::string title;

		CMenuItem* item;

		int tuned;
		bool displayNext;
		
		unsigned int position;
		unsigned int duration;
		unsigned int file_prozent;
		unsigned int speed;
		
		void hide();

	protected:
		virtual CWebTV::result_ itemSelected();
		
	public:
		enum state
		{
			STOPPED     =  0,
			PLAY        =  1,
			PAUSE       =  2
		};

		ClistBox* webTVlistMenu;
		
		unsigned int playstate;
		
		CWebTV();
		~CWebTV();
		int exec(CMenuTarget* parent, const std::string& actionKey);
		
		void show(bool reload = false, bool reinit = false);
		void userBouquet();
		void Bouquets();
		void quickZap(int key);
		//
		void updateEvents(void);
		void getEvents(t_channel_id chid);
		void showInfo();
		
		// playback
		bool startPlayBack(int pos);
		void stopPlayBack(void);
		void pausePlayBack(void);
		void continuePlayBack(void);

		//
		unsigned int getTunedChannel() {if(tuned < 0) tuned = 0; return tuned;};
		t_channel_id getLiveChannelID() { if(tuned < 0) tuned = 0; return channels[tuned]->channel_id;};
		const std::string& getLiveChannelName(void){if(tuned < 0) tuned = 0; return channels[tuned]->name;};
		const std::string& getLiveChannelUrl(void){if(tuned < 0) tuned = 0; return channels[tuned]->url;};
		const std::string& getChannelName(t_channel_id id);
		const std::string& getChannelURL(t_channel_id id);
		
		//
		void loadChannels(void);
		void ClearChannels(void);

		//
		CZapitClient::CCurrentServiceInfo getServiceInfo();
		void getPIDS(CZapitClient::responseGetPIDs& pids);
};

class CWebTVChooser : public CWebTV
{
	private:
		char *selected_item;
		
	protected:
		CWebTV::result_ itemSelected();

	public:
		CWebTVChooser(char* channelname);
};

#endif
