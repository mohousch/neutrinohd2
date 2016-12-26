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

#include <xmlinterface.h>

#include <channel.h>


#define DEFAULT_WEBTV_FILE 		CONFIGDIR "/webtv/webtv.xml"

class CWebTV : public CMenuTarget
{
	private:
		struct webtv_channels {
			t_channel_id id;
			std::string title;
			std::string url;
			std::string description;
		};

		xmlDocPtr parser;
		
		// channels
		std::vector<webtv_channels *> channels;
		
		// bouquets
		std::string title;

		CMenulistBox* webTVlistMenu;

		int tuned;
		
		unsigned int position;
		unsigned int duration;
		unsigned int file_prozent;
		unsigned int speed;
		
		void hide();

		void processPlaylistUrl(const char *url, const char *name, const char * description) ;
		void addUrl2Playlist(const char * url, const char *name, const char * description, t_channel_id id = 0);
		
	public:
		enum state
		{
			STOPPED     =  0,
			PLAY        =  1,
			PAUSE       =  2
		};
		
		unsigned int playstate;
		
		CWebTV();
		~CWebTV();
		int exec(CMenuTarget* parent, const std::string& actionKey);
		
		void show(bool reload = false);
		void showUserBouquet();
		
		//
		void quickZap(int key);
		
		// playback
		bool startPlayBack(int pos);
		void stopPlayBack(void);
		void pausePlayBack(void);
		void continuePlayBack(void);
		
		//
		//void showFileInfoWebTV(int pos);
		void showInfo();

		//
		unsigned int getTunedChannel() {return tuned;};
		t_channel_id getLiveChannelID() { return channels[tuned]->id;};
		
		//
		void loadChannels(void);
		void ClearChannels(void);
		
		bool readChannellist(std::string filename);
		void addUserBouquet(void);
};

#endif
