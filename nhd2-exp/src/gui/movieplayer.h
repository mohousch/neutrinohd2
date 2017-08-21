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

#include <stdio.h>
#include <string>
#include <vector>

#include <configfile.h>

#include <driver/framebuffer.h>

#include <gui/widget/menue.h>
#include <gui/widget/progressbar.h>

#include <gui/moviebrowser.h>
#include <gui/movieinfo.h>
#include <gui/filebrowser.h>


#if !defined (_FILE_OFFSET_BITS) && !defined (__USE_FILE_OFFSET64) && !defined (_DARWIN_USE_64_BIT_INODE)
#error not using 64 bit file offsets
#endif /* __USE_FILE__OFFSET64 */

class CMovieInfoViewer
{
	public:
		enum mode
		{
			MODE_ASC,
			MODE_DESC
		};
	
	private:
		CFrameBuffer * frameBuffer;

		//
		time_t m_time_dis;
		time_t m_time_show;

		//
		bool visible;

		//
		int m_xstart, m_xend, m_y, m_height, m_width, twidth;
		int BoxStartX, BoxStartY, BoxEndY, BoxEndX;
		int BoxWidth, BoxHeight;

		mode m_mode;

		//
		void GetDimensions();

		// movie infoviewer
		CBox cFrameBoxInfo;
		CBox cFrameBoxButton;
		int icon_w_aspect, icon_h_aspect;
		int icon_w_dd, icon_h_dd;
		int icon_red_w, icon_red_h;
		int icon_green_w, icon_green_h;
		int icon_yellow_w, icon_yellow_h;
		int icon_blue_w, icon_blue_h;

	public:
		CMovieInfoViewer();
		~CMovieInfoViewer();
		void show(const std::string _Title, const std::string _Info, short _Percent, const unsigned int _ac3state, const int _speed, const int _playstate, bool _show_bookmark = false);
		void update(time_t time_show = 0);
		void hide();
		bool IsVisible() {return visible;}
		//bool IsmVisible() {return m_visible;}
		void SetMode(mode m) { m_mode = m;}
		mode GetMode() { return m_mode;}

		//
		void showMovieInfo(std::string Title, std::string Info, short Percent, const unsigned int ac3state, const int speed, const int playstate, bool show_bookmark = false);
};

//
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
		
	private:
		int playstate;

		int speed;
		int slow;

		int position;
		int duration;
		int file_prozent;
		int startposition;
		int g_jumpseconds;

		//
		unsigned short g_numpida;
		unsigned short g_vpid;
		unsigned short g_vtype;
		unsigned int g_currentapid;
		unsigned int g_currentac3;
		
		// playlist
		CMoviePlayList filelist;
		unsigned int selected;
		
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
		CMovieInfoViewer FileTime;

		time_t timeStartShowingInfo;

		//
		CFrameBuffer * frameBuffer;
		int m_LastMode;	
		bool stopped;

		std::string Path_local;
		CMovieBrowser * moviebrowser;
		
		CMovieInfo cMovieInfo;	

		void cutNeutrino();
		void restoreNeutrino();
		void PlayFile();
		void showHelpTS(void);
		void updateLcd(const std::string & lcd_filename);
		int showStartPosSelectionMenu(void);
		void showFileInfo();
		
	public:
		CMoviePlayerGui();
		~CMoviePlayerGui();
		int exec(CMenuTarget* parent, const std::string & actionKey);
		void hide();

		//
		void addToPlaylist(MI_MOVIE_INFO& mfile);
		void clearPlaylist(void);
		void removeFromPlaylist(long pos);
};

#endif
