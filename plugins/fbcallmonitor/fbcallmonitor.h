/*
  $Id: fbcallmonitor.h 2014/01/22 mohousch Exp $
  thx to BPanther

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

#ifndef __fbcallmonitor__
#define __fbcallmonitor__

#include <plugin.h>


#define CONFIG_FILE		PLUGINDIR "/fbcallmonitor/fb.conf"

class CFBCallMonitor : public CMenuTarget
{
	private:
		std::string FB_IP_STRG;
		std::string FB_PORT_STRG;
		std::string FB_ZIEL1_STRG;
		std::string FB_ZIEL1N_STRG;
		std::string FB_ZIEL2_STRG;
		std::string FB_ZIEL2N_STRG;
		std::string FB_ZIEL3_STRG;
		std::string FB_ZIEL3N_STRG;
		std::string FB_BOXIP_STRG;
		std::string FB_BOXUSERNAME_STRG;
		std::string FB_BOXPASSWORD_STRG;
		int FB_DEBUG;
		int FB_ALLE;
		int FB_MONRING;
		int FB_MONDISCONNECT;
		int FB_MUTERING;
		int FB_POPUP;
		int FB_INVERS;
		
		void readSettings();
		bool saveSettings();

		void showMenu();
        public:
		CFBCallMonitor();
		~CFBCallMonitor();
		
		int exec(CMenuTarget *parent,  const std::string &actionkey);
		void hide();
};

#endif
