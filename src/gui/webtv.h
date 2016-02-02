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

#include <sys/types.h>

#include <string>
#include <vector>

#include <sys/stat.h>

#include <driver/file.h>

#include <xmlinterface.h>


#define DEFAULT_WEBTV_FILE 		CONFIGDIR "/webtv/webtv.xml"

class CWebTV
{
	private:
		struct webtv_channels {
			std::string title;
			std::string url;
			std::string description;
			bool locked;		// for parentallock
		};

		xmlDocPtr parser;
		
		// channels
		std::vector<webtv_channels *> channels;
		CZapProtection * 	zapProtection;
		
		// bouquets
		std::string title;
		
		// gui
		CFrameBuffer * frameBuffer;
		
		int            	width;
		int            	height;
		int            	x;
		int            	y;
		
		int            	hheight; 	// title font height
		int            	iheight; 	// item font height (buttons???)
		
		int icon_bf_w;
		int icon_bf_h;
		int icon_hd_w;
		int icon_hd_h;
		
		unsigned int   	selected;
		unsigned int oldselected;
		int tuned;
		
		unsigned int   	liststart;
		int		buttonHeight;
		unsigned int	listmaxshow;
		unsigned int	numwidth;
		int 		info_height;
		
		unsigned int position;
		unsigned int duration;
		unsigned int file_prozent;
		unsigned int speed;
		
		void paintDetails(int index);
		void clearItem2DetailsLine ();
		void paintItem2DetailsLine (int pos);
		void paintItem(int pos);
		void paint();
		void paintHead();
		void paintFoot();
		void hide();

		void processPlaylistUrl(const char *url, const char *name, const char * description) ;
		void addUrl2Playlist(const char * url, const char *name, const char * description, bool locked = false);
		
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
		int exec(bool rezap = false);
		
		int Show();
		void showUserBouquet();
		
		//
		void zapTo(int pos = 0, bool rezap = false);
		void quickZap(int key);
		
		// playback
		bool startPlayBack(int pos);
		void stopPlayBack(void);
		void pausePlayBack(void);
		void continuePlayBack(void);
		
		void showFileInfoWebTV(int pos);
		void showInfo();
		void getInfos();
		
		void showAudioDialog();
		
		unsigned int getTunedChannel() {return tuned;};
		unsigned int getlastSelectedChannel() { return selected;};
		
		void loadChannels(void);
		void ClearChannels(void);
		
		bool readChannellist(std::string filename);
		void addUserBouquet(void);
		
		unsigned int hasChannels() { return channels.size();};
};

#endif
