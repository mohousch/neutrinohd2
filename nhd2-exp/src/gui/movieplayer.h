/*
  Neutrino-GUI  -   DBoxII-Project
  
  $Id: movieplayer.h 2013/10/12 mohousch Exp $

  Copyright (C) 2003,2004 gagga
  Homepage: http://www.giggo.de/dbox

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

#ifndef __movieplayergui__
#define __movieplayergui__

#include <config.h>
#include <configfile.h>

#include <driver/framebuffer.h>
#include <gui/filebrowser.h>
#include <gui/widget/menue.h>
#include <gui/moviebrowser.h>
#include <gui/movieinfo.h>
#include <gui/timeosd.h>
          	
#include <stdio.h>
#include <string>
#include <vector>


#if !defined (_FILE_OFFSET_BITS) && !defined (__USE_FILE_OFFSET64) && !defined (_DARWIN_USE_64_BIT_INODE)
#error not using 64 bit file offsets
#endif /* __USE_FILE__OFFSET64 */

class CMoviePlayerGui : public CMenuTarget
{
	public:
		enum state
		{
			STOPPED     =  0,
			PLAY        =  1,
			PAUSE       =  2,
			FF          =  3,
			REW         =  4,
			SLOW        =  5,
			SOFTRESET   = 99
		};
		
		enum {
			NO_TIMESHIFT = 0,
			TIMESHIFT,
			P_TIMESHIFT,	// paused timeshift
			R_TIMESHIFT	// rewind timeshift
		};
		
	private:
		int playstate;

		int speed;
		int slow;

		int position;
		int duration;
		int file_prozent;
		int startposition;
		off64_t minuteoffset;
		off64_t secondoffset;
		int g_jumpseconds;

		unsigned short g_apids[10];
		unsigned short g_ac3flags[10];
		unsigned short g_numpida;
		unsigned short g_vpid;
		unsigned short g_vtype;

		unsigned int g_currentapid;
		unsigned int g_currentac3;
		
		// playlist
		CFileList filelist;
		unsigned int selected;
		
		const char *filename;
		std::string Title;
		std::string Info1;
		std::string Info2;
		std::string Thumbnail;
		
		//lcd
		std::string sel_filename;
		
		// global flags
		bool update_lcd;
		bool open_filebrowser;
		bool start_play;
		bool exit;
		bool was_file;
		bool isMovieBrowser;
		bool isURL;
		bool m_loop;
		bool m_multiselect;
		
		bool is_file_player;
		
		// timeosd
		bool time_forced;
		
		// timeosd
		CTimeOSD FileTime;

	private:
		CFrameBuffer * frameBuffer;
		int            m_LastMode;	
		bool		stopped;

		std::string Path_local;

		CFileBrowser * filebrowser;
		CMovieBrowser * moviebrowser;
		
		CMovieInfo cMovieInfo;	
		MI_MOVIE_INFO * p_movie_info;

		void PlayFile();
		void cutNeutrino();
		void restoreNeutrino();
		bool get_movie_info_apid_name(int apid, MI_MOVIE_INFO * movie_info, std::string * apidtitle);

		CFileFilter tsfilefilter;

		void showHelpTS(void);
		
		// lcd
		void updateLcd(const std::string & sel_filename);
		
	public:
		CMoviePlayerGui();
		~CMoviePlayerGui();
		int exec(CMenuTarget* parent, const std::string & actionKey);
		
		// show infos
		void showFileInfo();
		void addToPlaylist(CFile& file);
		
		int getPercent(){return file_prozent;};

		static CMoviePlayerGui* getInstance();
};

#endif
