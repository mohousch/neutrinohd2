/*
  Neutrino-GUI  -   DBoxII-Project
  
  $Id: movieplayer.cpp 2013/10/12 mohousch Exp $

  Movieplayer (c) 2003, 2004 by gagga
  Based on code by Dirch, obi and the Metzler Bros. Thanks.

  $Id: movieplayer.cpp,v 1.97 2004/07/18 00:54:52 thegoodguy Exp $

  Homepage: http://www.giggo.de/dbox2/movieplayer.html

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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/vfs.h>
#include <sys/mount.h>

#include <gui/movieplayer.h>

#include <global.h>
#include <neutrino.h>

#include <driver/fontrenderer.h>
#include <driver/rcinput.h>
#include <driver/vcrcontrol.h>

#include <daemonc/remotecontrol.h>
#include <system/settings.h>
#include <system/helpers.h>

#include <gui/eventlist.h>
#include <gui/color.h>
#include <gui/infoviewer.h>
#include <gui/nfs.h>
#include <gui/timeosd.h>
#include <gui/webtv.h>
#include <gui/audio_video_select.h>

#include <gui/widget/buttons.h>
#include <gui/widget/icons.h>
#include <gui/widget/messagebox.h>
#include <gui/widget/hintbox.h>
#include <gui/widget/stringinput.h>
#include <gui/widget/stringinput_ext.h>
#include <gui/widget/helpbox.h>
#include <gui/widget/infobox.h>

#include <system/debug.h>

#include <libxmltree/xmlinterface.h>

#include <algorithm>
#include <fstream>
#include <sstream>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <poll.h>
#include <sys/timeb.h>

/* libdvbapi */
#include <playback_cs.h>
#include <video_cs.h>
#include <audio_cs.h>

/*zapit includes*/
#include <channel.h>


// scripts
#define MOVIEPLAYER_START_SCRIPT 	CONFIGDIR "/movieplayer.start" 
#define MOVIEPLAYER_END_SCRIPT 		CONFIGDIR "/movieplayer.end"

//cPlayback * playback = NULL;
extern cPlayback *playback;

//
extern CInfoViewer * g_InfoViewer;
extern t_channel_id live_channel_id; 			//defined in zapit.cpp

#define MOVIE_HINT_BOX_TIMER 	5				// time to show bookmark hints in seconds

#define MINUTEOFFSET 		117*262072
#define MP_TS_SIZE 		262072				// ~0.5 sec

#define TIMESHIFT_SECONDS 	3

int timeshift = CMoviePlayerGui::NO_TIMESHIFT;
extern char rec_filename[1024];				// defined in stream2file.cpp

// for timeshift epg infos
bool sectionsd_getActualEPGServiceKey(const t_channel_id uniqueServiceKey, CEPGData * epgdata);
bool sectionsd_getEPGidShort(event_id_t epgID, CShortEPGData * epgdata);

extern CWebTV * webtv;

// movieplayer
CMoviePlayerGui::CMoviePlayerGui()
{
	stopped = false;

	frameBuffer = CFrameBuffer::getInstance();

	// local path
	if (strlen(g_settings.network_nfs_moviedir) != 0)
		Path_local = g_settings.network_nfs_moviedir;
	else
		Path_local = "/";

	// tsfilefilter
	tsfilefilter.addFilter("ts");
	tsfilefilter.addFilter("mpg");
	tsfilefilter.addFilter("mpeg");
	tsfilefilter.addFilter("divx");
	tsfilefilter.addFilter("avi");
	tsfilefilter.addFilter("mkv");
	tsfilefilter.addFilter("asf");
	tsfilefilter.addFilter("aiff");
	tsfilefilter.addFilter("m2p");
	tsfilefilter.addFilter("mpv");
	tsfilefilter.addFilter("m2ts");
	tsfilefilter.addFilter("vob");
	tsfilefilter.addFilter("mp4");
	tsfilefilter.addFilter("mov");	
	tsfilefilter.addFilter("flv");	
	tsfilefilter.addFilter("dat");
	tsfilefilter.addFilter("trp");
	tsfilefilter.addFilter("vdr");
	tsfilefilter.addFilter("mts");
	tsfilefilter.addFilter("wmv");
	tsfilefilter.addFilter("wav");
	tsfilefilter.addFilter("flac");
	tsfilefilter.addFilter("mp3");
	tsfilefilter.addFilter("wma");
	tsfilefilter.addFilter("ogg");	
}

CMoviePlayerGui::~CMoviePlayerGui()
{
}

void CMoviePlayerGui::cutNeutrino()
{
	dprintf(DEBUG_NORMAL, "CMoviePlayerGui::cutNeutrino\n");
	
	if (stopped)
		return;
	
	// tell neutrino we are in ts mode
	CNeutrinoApp::getInstance()->handleMsg(NeutrinoMessages::CHANGEMODE, NeutrinoMessages::mode_ts);
	
	// save (remeber) last mode
	m_LastMode = (CNeutrinoApp::getInstance()->getLastMode() | NeutrinoMessages::norezap);
	
	CNeutrinoApp::getInstance()->StopSubtitles();
	
	if(CNeutrinoApp::getInstance()->getLastMode() == NeutrinoMessages::mode_iptv)
	{
		if(webtv)
			webtv->stopPlayBack();
	}
	else
	{
		// pause epg scanning
		g_Sectionsd->setPauseScanning(true);
			
		// lock playback
		g_Zapit->lockPlayBack();
	}
	
	// start mp start-script
	puts("[movieplayer.cpp] executing " MOVIEPLAYER_START_SCRIPT ".");
	if (system(MOVIEPLAYER_START_SCRIPT) != 0)
		perror("Datei " MOVIEPLAYER_START_SCRIPT " fehlt. Bitte erstellen, wenn gebraucht.\nFile " MOVIEPLAYER_START_SCRIPT " not found. Please create if needed.\n");

	stopped = true;
}

void CMoviePlayerGui::restoreNeutrino()
{
	dprintf(DEBUG_NORMAL, "CMoviePlayerGui::restoreNeutrino\n");
	
	if (!stopped)
		return;

	if(CNeutrinoApp::getInstance()->getLastMode() == NeutrinoMessages::mode_iptv)
	{
		if(webtv)
			webtv->startPlayBack(webtv->getTunedChannel());
	}
	else
	{
		// unlock playback
		g_Zapit->unlockPlayBack();
			
		// start epg scanning
		g_Sectionsd->setPauseScanning(false);
	}
	
	CNeutrinoApp::getInstance()->StartSubtitles();

	// tell neutrino that we are in the last mode
	CNeutrinoApp::getInstance()->handleMsg(NeutrinoMessages::CHANGEMODE, m_LastMode);
	
	//show infobar
	g_RCInput->postMsg( NeutrinoMessages::SHOW_INFOBAR, 0 );
	
	// start end script
	puts("[movieplayer.cpp] executing " MOVIEPLAYER_END_SCRIPT ".");
	if (system(MOVIEPLAYER_END_SCRIPT) != 0)
		perror("Datei " MOVIEPLAYER_END_SCRIPT " fehlt. Bitte erstellen, wenn gebraucht.\nFile " MOVIEPLAYER_END_SCRIPT " not found. Please create if needed.\n");

	stopped = false;
}

bool CMoviePlayerGui::get_movie_info_apid_name(int apid, MI_MOVIE_INFO * movie_info, std::string * apidtitle)
{
	if (movie_info == NULL || apidtitle == NULL)
		return false;

	for (int i = 0; i < (int)movie_info->audioPids.size(); i++) 
	{
		if (movie_info->audioPids[i].epgAudioPid == apid && !movie_info->audioPids[i].epgAudioPidName.empty()) 
		{
			*apidtitle = movie_info->audioPids[i].epgAudioPidName;
			return true;
		}
	}

	return false;
}

void CMoviePlayerGui::updateLcd(const std::string & lcd_filename)
{
	char tmp[20];
	std::string lcd;

	switch (playstate) 
	{
		case CMoviePlayerGui::PAUSE:
			//lcd = "|| ";
			lcd = lcd_filename;
			break;
			
		case CMoviePlayerGui::REW:
			sprintf(tmp, "%dx<< ", speed);
			lcd = tmp;
			lcd += lcd_filename;
			break;
			
		case CMoviePlayerGui::FF:
			sprintf(tmp, "%dx>> ", speed);
			lcd = tmp;
			lcd += lcd_filename;
			break;

		case CMoviePlayerGui::SLOW:
			sprintf(tmp, "%ds>> ", slow);
			lcd = tmp;
			lcd += lcd_filename;
			break;

		default:
			//lcd = "> ";
			lcd = lcd_filename;
			break;
	}
	
	CVFD::getInstance()->showMenuText(0, lcd.c_str(), -1, true);	
}

void CMoviePlayerGui::addToPlaylist(CFile& file)
{	
	dprintf(DEBUG_NORMAL, "CMoviePlayerGui::addToPlaylist: %s\n", file.Name.c_str());
	
	filelist.push_back(file);
}

int CMoviePlayerGui::exec(CMenuTarget * parent, const std::string & actionKey)
{
	dprintf(DEBUG_NORMAL, "CMoviePlayerGui::exec: actionKey = %s\n", actionKey.c_str());

	if (parent) 
		parent->hide();

	bool usedBackground = frameBuffer->getuseBackground();

	if (usedBackground) 
	{
		frameBuffer->saveBackgroundImage();
		frameBuffer->ClearFrameBuffer();
		frameBuffer->blit();
	}
	
	//
	position = 0;
	duration = 0;
	file_prozent = 0;
	startposition = 0;
	minuteoffset = MINUTEOFFSET;
	secondoffset = minuteoffset / 60;
	
	//
	speed = 1;
	slow = 0;
	
	// global flags
	update_lcd = false;
	open_filebrowser = false;
	start_play = false;
	exit = false;
	was_file = false;
	m_loop = false;
	m_multiselect = false;
	isMovieBrowser = false;
	isURL = false;
	timeshift = NO_TIMESHIFT;
	
	// for playing
	playstate = CMoviePlayerGui::STOPPED;
	is_file_player = false;
	
	// timeosd
	time_forced = false;
	
	selected = 0;
	
	// pids
	g_numpida = 0;
	g_vpid = 0;
	g_vtype = 0;
	g_currentapid = 0;
	g_currentac3 = 0;
	
	filename = NULL;
	sel_filename.clear();
	
	// cutneutrino
	cutNeutrino();

	if (actionKey == "tsmoviebrowser") 
	{
		open_filebrowser = true;
		isMovieBrowser = true;
		timeshift = NO_TIMESHIFT;
		isURL = false;
		
		// moviebrowser
		moviebrowser = new CMovieBrowser();
		moviebrowser->setMode(MB_SHOW_RECORDS);
	}
	else if (actionKey == "moviebrowser") 
	{
		open_filebrowser = true;
		isMovieBrowser = true;
		timeshift = NO_TIMESHIFT;
		isURL = false;
		
		// moviebrowser
		moviebrowser = new CMovieBrowser();
		moviebrowser->setMode(MB_SHOW_FILES);
	}
	else if (actionKey == "fileplayback") 
	{
		open_filebrowser = true;
		isMovieBrowser = false;
		timeshift = NO_TIMESHIFT;
		isURL = false;
		
		// filebrowser
		if (g_settings.filebrowser_denydirectoryleave)
			filebrowser = new CFileBrowser(Path_local.c_str());
		else
			filebrowser = new CFileBrowser();
		
		filebrowser->Dirs_Selectable = false;
		
		// filebrowser multi select
		if (g_settings.movieplayer_allow_multiselect)
			filebrowser->Multi_Select = true;
		else 
			filebrowser->Multi_Select = false;
		
		filebrowser->Filter = &tsfilefilter;

	}
	else if (actionKey == "timeshift") 
	{
		timeshift = TIMESHIFT;
	} 
	else if (actionKey == "ptimeshift") 
	{
		timeshift = P_TIMESHIFT;
	} 
	else if (actionKey == "rtimeshift") 
	{
		timeshift = R_TIMESHIFT;
	} 
	else if(actionKey == "urlplayback")
	{
		open_filebrowser = false;
		isMovieBrowser = false;
		timeshift = NO_TIMESHIFT;
		isURL = true;
	}
	
	//
	PlayFile();
	
	// quit
	// Restore previous background
	if (usedBackground) 
	{
		frameBuffer->restoreBackgroundImage();
		frameBuffer->useBackground(true);
		frameBuffer->paintBackground();
		frameBuffer->blit();
	}
	
	// clear filelist
	if(!filelist.empty())
		filelist.clear();
	
	if(filename != NULL)
		filename = NULL;
	
	if(!sel_filename.empty())
		sel_filename.clear();
	
	if(!Title.empty())
		Title.clear();
	
	if(!Info1.empty())
		Info1.clear();
	
	if(!Info2.empty())
		Info2.clear();
	
	if(!Thumbnail.empty())
		Thumbnail.clear();
	
	if (actionKey == "fileplayback") 
	{
		if (filebrowser != NULL)
		{
			delete filebrowser;
			filebrowser = NULL;
		}
	}
	
	if (actionKey == "tsmoviebrowser" || actionKey == "moviebrowser") 
	{
		if (moviebrowser != NULL)
		{
			delete moviebrowser;
			moviebrowser = NULL;
		}
	}

	// restore neutrino
	restoreNeutrino();
	
	//
	CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);

	if (timeshift) 
	{
		timeshift = NO_TIMESHIFT;
		return menu_return::RETURN_EXIT_ALL;
	}

	return menu_return::RETURN_REPAINT;
}

// moviebrowser
// filebrowser
// timeshift
// url
void CMoviePlayerGui::PlayFile(void)
{
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
	
	// timeshift
	bool timesh = timeshift;
	
	// url
	if(isURL)
	{
		//
		if(!filelist.empty())
		{
			if(filelist[0].Url.empty())
			{
				filename = filelist[0].Name.c_str();
				sel_filename = filelist[0].getFileName();
							
				Title = sel_filename;
				Info1 = sel_filename;
				Info2 = sel_filename;
				
				// thumbnail
				if(Thumbnail.empty())
				{
					std::string fname = "";
					fname = filename;
					changeFileNameExt(fname, ".jpg");
						
					if(!access(fname.c_str(), F_OK) )
						Thumbnail = fname.c_str();
				}
			}
			else
			{
				filename = filelist[0].Url.c_str();
				sel_filename = filelist[0].Name.c_str();	
				Title = filelist[0].Title;
				Info1 = filelist[0].Info1;
				Info2 = filelist[0].Info2;
				Thumbnail = filelist[0].Thumbnail;
			}

			update_lcd = true;
			start_play = true;
			was_file = true;
			is_file_player = true;
			if(filelist.size() > 1)
				m_multiselect = true;
		}
						
		CVFD::getInstance()->setMode(CVFD::MODE_MENU_UTF8);	
	}

	// bookmarks menu
	timeb current_time;
	p_movie_info = NULL;	// movie info handle which comes from the MovieBrowser, if not NULL MoviePla yer is able to save new bookmarks

	int width = 280;
	int height = 65;
        int x = frameBuffer->getScreenX() + (frameBuffer->getScreenWidth() - width) / 2;
        int y = frameBuffer->getScreenY() + frameBuffer->getScreenHeight() - height - 20;

	CBox boxposition(x, y, width, height);	// window position for the hint boxes

	CTextBox endHintBox(g_Locale->getText(LOCALE_MOVIEBROWSER_HINT_MOVIEEND), NULL, CTextBox::CENTER, &boxposition);
	CTextBox comHintBox(g_Locale->getText(LOCALE_MOVIEBROWSER_HINT_JUMPFORWARD), NULL, CTextBox::CENTER, &boxposition);
	CTextBox loopHintBox(g_Locale->getText(LOCALE_MOVIEBROWSER_HINT_JUMPBACKWARD), NULL, CTextBox::CENTER, &boxposition);
	CTextBox newLoopHintBox(g_Locale->getText(LOCALE_MOVIEBROWSER_HINT_NEWBOOK_BACKWARD), NULL, CTextBox::CENTER , &boxposition);
	CTextBox newComHintBox(g_Locale->getText(LOCALE_MOVIEBROWSER_HINT_NEWBOOK_FORWARD), NULL, CTextBox::CENTER, &boxposition);

	bool showEndHintBox = false;	// flag to check whether the box shall be painted
	bool showComHintBox = false;	// flag to check whether the box shall be painted
	bool showLoopHintBox = false;	// flag to check whether the box shall be painted
	int jump_not_until = 0;		// any jump shall be avoided until this time (in seconds from moviestart)
	MI_BOOKMARK new_bookmark;	// used for new movie info bookmarks created from the movieplayer
	new_bookmark.pos = 0;		// clear , since this is used as flag for bookmark activity
	new_bookmark.length = 0;

	// very dirty usage of the menue, but it works and I already spent to much time with it, feel free to make it better ;-)
#define BOOKMARK_START_MENU_MAX_ITEMS 5
	CSelectedMenu cSelectedMenuBookStart[BOOKMARK_START_MENU_MAX_ITEMS];

	CMenuWidget bookStartMenu(LOCALE_MOVIEBROWSER_BOOK_NEW, NEUTRINO_ICON_STREAMING);

	bookStartMenu.addItem(new CMenuForwarder(LOCALE_MOVIEBROWSER_BOOK_NEW, true, NULL, &cSelectedMenuBookStart[0]));
	bookStartMenu.addItem(new CMenuForwarder(LOCALE_MOVIEBROWSER_BOOK_TYPE_FORWARD, true, NULL, &cSelectedMenuBookStart[1]));
	bookStartMenu.addItem(new CMenuForwarder(LOCALE_MOVIEBROWSER_BOOK_TYPE_BACKWARD, true, NULL, &cSelectedMenuBookStart[2]));
	bookStartMenu.addItem(new CMenuForwarder(LOCALE_MOVIEBROWSER_BOOK_MOVIESTART, true, NULL, &cSelectedMenuBookStart[3]));
	bookStartMenu.addItem(new CMenuForwarder(LOCALE_MOVIEBROWSER_BOOK_MOVIEEND, true, NULL, &cSelectedMenuBookStart[4]));

	// play loop
 go_repeat:
	do {
		// multi select
		if (playstate == CMoviePlayerGui::STOPPED && was_file && selected > 0) 
		{
			if(selected + 1 < filelist.size()) 
			{
				selected++;
				filename = filelist[selected].Name.c_str();
				sel_filename = filelist[selected].getFileName();
				
				Title = sel_filename;
				Info1 = sel_filename;
				Info2 = sel_filename;
				//thumbnail
				Thumbnail = "";
				std::string fname = "";
				fname = filelist[selected].Name;
				changeFileNameExt(fname, ".jpg");
					
				if(!access(fname.c_str(), F_OK) )
					Thumbnail = fname.c_str();
 
				update_lcd = true;
				start_play = true;
			} 
			else if(m_loop)
			{
				filename = filename;
				sel_filename = sel_filename;
				
				Title = sel_filename;
				Info1 = sel_filename;
				Info2 = sel_filename;
				//thumbnail
				Thumbnail = "";
				std::string fname = "";
				fname = filename;
				changeFileNameExt(fname, ".jpg");
					
				if(!access(fname.c_str(), F_OK) )
					Thumbnail = fname.c_str();
 
				update_lcd = true;
				start_play = true;
			}
			else 
			{
				open_filebrowser = true;
			}
		}
		
		// exit
		if (exit) 
		{	  
			exit = false;
			
			dprintf(DEBUG_NORMAL, "CMoviePlayerGui::PlayFile: stop (3)\n");
			playstate = CMoviePlayerGui::STOPPED;
			break;
		}

		// timeshift
		if (timesh) 
		{
			char fname[255];
			int cnt = 10 * 1000000;

			while (!strlen(rec_filename)) 
			{
				usleep(1000);
				cnt -= 1000;
				if (!cnt)
					break;
			}

			if (!strlen(rec_filename))
				return;

			sprintf(fname, "%s.ts", rec_filename);
			filename = fname;
			sel_filename = std::string(rindex(filename, '/') + 1);
			
			dprintf(DEBUG_NORMAL, "CMoviePlayerGui::PlayFile: Timeshift: %s\n", sel_filename.c_str());

			update_lcd = true;
			start_play = true;
			open_filebrowser = false;
			timesh = false;
			
			// extract channel epg infos
			CEPGData epgData;
			event_id_t epgid = 0;
			
			if(sectionsd_getActualEPGServiceKey(live_channel_id&0xFFFFFFFFFFFFULL, &epgData))
				epgid = epgData.eventID;

			if(epgid != 0) 
			{
				CShortEPGData epgdata;
				
				if(sectionsd_getEPGidShort(epgid, &epgdata)) 
				{
					if (!(epgdata.title.empty())) 
						Title = epgdata.title;
					
					if(!(epgdata.info1.empty()))
						Info1 = epgdata.info1;
					
					if(!(epgdata.info2.empty()))
						Info2 = epgdata.info2;
				}
			}
			
			CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);

			// start timeosd
			FileTime.SetMode(CTimeOSD::MODE_ASC);
			FileTime.update(position/1000);
		}
		
		// movie infos (moviebrowser)
		if (isMovieBrowser == true) 
		{	  
			// do all moviebrowser stuff here ( like commercial jump etc.)
			if (playstate == CMoviePlayerGui::PLAY) 
			{				
#if defined (PLATFORM_COOLSTREAM)
				playback->GetPosition(position, duration);
#else
 				playback->GetPosition((int64_t &)position, (int64_t &)duration);
#endif				
				
				int play_sec = position / 1000;	// get current seconds from moviestart

				if (play_sec + 10 < jump_not_until || play_sec > jump_not_until + 10)
					jump_not_until = 0;	// check if !jump is stale (e.g. if user jumped forward or backward)

				if (new_bookmark.pos == 0)	// do bookmark activities only, if there is no new bookmark started
				{
					if (p_movie_info != NULL)	// process bookmarks if we have any movie info
					{
						if (p_movie_info->bookmarks.end != 0) 
						{
							// Check for stop position
							if (play_sec >= p_movie_info->bookmarks.end - MOVIE_HINT_BOX_TIMER && play_sec < p_movie_info->bookmarks.end && play_sec > jump_not_until) 
							{
								if (showEndHintBox == false) 
								{
									endHintBox.paint();	// we are 5 sec before the end postition, show warning
									showEndHintBox = true;
									dprintf(DEBUG_INFO, "CMoviePlayerGui::PlayFile: user stop in 5 sec...\r\n");
								}
							} 
							else 
							{
								if (showEndHintBox == true) 
								{
									endHintBox.hide();	// if we showed the warning before, hide the box again
									showEndHintBox = false;
								}
							}

							if (play_sec >= p_movie_info->bookmarks.end && play_sec <= p_movie_info->bookmarks.end + 2 && play_sec > jump_not_until)	// stop playing
							{
								// we ARE close behind the stop position, stop playing 
								dprintf(DEBUG_INFO, "CMoviePlayerGui::PlayFile: user stop\r\n");
								playstate = CMoviePlayerGui::STOPPED;
							}
						}
						
						// Check for bookmark jumps
						int loop = true;
						showLoopHintBox = false;
						showComHintBox = false;

						for (int book_nr = 0; book_nr < MI_MOVIE_BOOK_USER_MAX && loop == true; book_nr++) 
						{
							if (p_movie_info->bookmarks.user[book_nr].pos != 0 && p_movie_info->bookmarks.user[book_nr].length != 0) 
							{
								// valid bookmark found, now check if we are close before or after it
								if (play_sec >= p_movie_info->bookmarks.user[book_nr].pos - MOVIE_HINT_BOX_TIMER && play_sec < p_movie_info->bookmarks.user[book_nr].pos && play_sec > jump_not_until) 
								{
									if (p_movie_info->bookmarks.user[book_nr].length < 0)
										showLoopHintBox = true;	// we are 5 sec before , show warning
									else if (p_movie_info->bookmarks.user[book_nr].length > 0)
										showComHintBox = true;	// we are 5 sec before, show warning
									//else  // TODO should we show a plain bookmark infomation as well?
								}

								if (play_sec >= p_movie_info->bookmarks.user[book_nr].pos && play_sec <= p_movie_info->bookmarks.user[book_nr].pos + 2 && play_sec > jump_not_until)	//
								{
									//for plain bookmark, the following calc shall result in 0 (no jump)
									g_jumpseconds = p_movie_info->bookmarks.user[book_nr].length;

									// we are close behind the bookmark, do bookmark activity (if any)
									if (p_movie_info->bookmarks.user[book_nr].length < 0) 
									{
										// if the jump back time is to less, it does sometimes cause problems (it does probably jump only 5 sec which will cause the next jump, and so on)
										if (g_jumpseconds > -15)
											g_jumpseconds = -15;

										g_jumpseconds = g_jumpseconds + p_movie_info->bookmarks.user[book_nr].pos;

										//playstate = CMoviePlayerGui::JPOS;	// bookmark  is of type loop, jump backward
										playback->SetPosition((int64_t)g_jumpseconds * 1000);
									} 
									else if (p_movie_info->bookmarks.user[book_nr].length > 0) 
									{
										// jump at least 15 seconds
										if (g_jumpseconds < 15)
											g_jumpseconds = 15;
										g_jumpseconds = g_jumpseconds + p_movie_info->bookmarks.user[book_nr].pos;

										//playstate = CMoviePlayerGui::JPOS;	// bookmark  is of type loop, jump backward
										playback->SetPosition((int64_t)g_jumpseconds * 1000);
									}
									
									dprintf(DEBUG_INFO, "CMoviePlayerGui::PlayFile: do jump %d sec\r\n", g_jumpseconds);
									update_lcd = true;
									loop = false;	// do no further bookmark checks
								}
							}
						}
						
						// check if we shall show the commercial warning
						if (showComHintBox == true) 
						{
							comHintBox.paint();
							dprintf(DEBUG_INFO, "CMoviePlayerGui::PlayFile: com jump in 5 sec...\r\n");
						} 
						else
							comHintBox.hide();

						// check if we shall show the loop warning
						if (showLoopHintBox == true) 
						{
							loopHintBox.paint();
							dprintf(DEBUG_INFO, "CMoviePlayerGui::PlayFile: loop jump in 5 sec...\r\n");
						} 
						else
							loopHintBox.hide();
					}
				}
			}
		}	

		// moviebrowser/filebrowser
		if (open_filebrowser) 
		{
			open_filebrowser = false;
			timeshift = false;
			
			FileTime.hide();
			g_InfoViewer->killTitle();

			// clear audipopids
			for (int i = 0; i < g_numpida; i++) 
			{
				g_apids[i] = 0;
				g_ac3flags[i] = 0;
			}
			g_numpida = 0; g_currentapid = 0;

			// moviebrowser
			if (isMovieBrowser == true) //moviebrowser
			{	
				if (moviebrowser->exec(Path_local.c_str())) 
				{
					// get the current path and file name
					Path_local = moviebrowser->getCurrentDir();
					CFile * file;

					if ((file = moviebrowser->getSelectedFile()) != NULL) 
					{
						filename = file->Name.c_str();

						sel_filename = file->getFileName();

						// get the movie info handle (to be used for e.g. bookmark handling)
						p_movie_info = moviebrowser->getCurrentMovieInfo();
						bool recfile = CNeutrinoApp::getInstance()->recordingstatus && !strncmp(rec_filename, filename, strlen(rec_filename));

						if (!recfile && p_movie_info->length) 
						{
							minuteoffset = file->Size / p_movie_info->length;
							minuteoffset = (minuteoffset / MP_TS_SIZE) * MP_TS_SIZE;
							if (minuteoffset < 5000000 || minuteoffset > 190000000)
								minuteoffset = MINUTEOFFSET;
							secondoffset = minuteoffset / 60;
						}

						if(!p_movie_info->audioPids.empty()) 
						{
							g_currentapid = p_movie_info->audioPids[0].epgAudioPid;	//FIXME
							g_currentac3 = p_movie_info->audioPids[0].atype;

							//if(g_currentac3)
							//	ac3state = CInfoViewer::AC3_ACTIVE;
						}

						for (int i = 0; i < (int)p_movie_info->audioPids.size(); i++) 
						{
							g_ac3flags[i] = p_movie_info->audioPids[i].atype;
							g_numpida++;

							if (p_movie_info->audioPids[i].selected) 
							{
								g_currentapid = p_movie_info->audioPids[i].epgAudioPid;	//FIXME
								g_currentac3 = p_movie_info->audioPids[i].atype;

								//if(g_currentac3)
								//	ac3state = CInfoViewer::AC3_AVAILABLE;
							}
						}

						g_vpid = p_movie_info->epgVideoPid;
						g_vtype = p_movie_info->VideoType;
						
						Title = p_movie_info->epgTitle;
						Info1 = p_movie_info->epgInfo1;
						Info2 = p_movie_info->epgInfo2;
						Thumbnail = p_movie_info->tfile;
						
						dprintf(DEBUG_INFO, "CMoviePlayerGui::PlayFile: file %s apid 0x%X atype %d vpid 0x%X vtype %d\n", filename, g_currentapid, g_currentac3, g_vpid, g_vtype);
						
						// get the start position for the movie					
						startposition = 1000 * moviebrowser->getCurrentStartPos();						

						update_lcd = true;
						start_play = true;
						was_file = true;
					}
				} 
				else if (playstate == CMoviePlayerGui::STOPPED) 
				{
					was_file = false;
					break;
				}
				
				CVFD::getInstance()->setMode(CVFD::MODE_MENU_UTF8);	
			} 
			else  // filebrowser
			{
				if (filebrowser->exec(Path_local.c_str()) == true) 
				{
					Path_local = filebrowser->getCurrentDir();
					
					if (g_settings.movieplayer_allow_multiselect) 
					{
						if(!filelist.empty())
							filelist.clear();
						
						filelist = filebrowser->getSelectedFiles();

						m_multiselect = true;
					} 
					else 
					{
						CFile *file = filebrowser->getSelectedFile();
						filelist.clear();
						filelist.push_back(*file);
					}

					if(!filelist.empty())
					{
						filename = filelist[selected].Name.c_str();
						sel_filename = filelist[selected].getFileName();
						
						Title = sel_filename;
						Info1 = sel_filename;
						Info2 = sel_filename;
						Thumbnail = "";
						std::string fname = "";
						fname = filelist[selected].Name;
						changeFileNameExt(fname, ".jpg");
					
						if(!access(fname.c_str(), F_OK) )
							Thumbnail = fname.c_str();

						update_lcd = true;
						start_play = true;
						was_file = true;
						is_file_player = true;
						selected = 0;
					}
				}
				else if (playstate == CMoviePlayerGui::STOPPED) 
				{
					was_file = false;
					break;
				}

				CVFD::getInstance()->setMode(CVFD::MODE_MENU_UTF8);
			}
		}

		// LCD 
		if (update_lcd) 
		{
			update_lcd = false;

			if (isMovieBrowser && strlen(p_movie_info->epgTitle.c_str()) && strncmp(p_movie_info->epgTitle.c_str(), "not", 3))
				updateLcd(p_movie_info->epgTitle);
			else
				updateLcd(sel_filename);
		}

		// timeosd
		if (FileTime.IsVisible()) 
		{
			if (FileTime.GetMode() == CTimeOSD::MODE_ASC) 
			{
				FileTime.update(position / 1000);
				FileTime.show(position / 1000);
			} 
			else 
			{
				FileTime.update((duration - position) / 1000);
				FileTime.show(position / 1000);
			}
		}

		// start playing
		if (start_play) 
		{
			dprintf(DEBUG_NORMAL, "CMoviePlayerGui::PlayFile: Startplay at %d seconds\n", startposition/1000);

			start_play = false;
			
			// stop playing if already playing (multiselect)
			if(playback->playing)
				playback->Stop();

			// init player
#if defined (PLATFORM_COOLSTREAM)
			playback->Open(is_file_player ? PLAYMODE_FILE : PLAYMODE_TS);
#else			
			playback->Open();
#endif			
			
			duration = 0;
			if(p_movie_info != NULL)
				duration = p_movie_info->length * 60 * 1000;
			  
			// PlayBack Start			  
			if(!playback->Start((char *)filename, g_vpid, g_vtype, g_currentapid, g_currentac3, duration))
			{
				dprintf(DEBUG_NORMAL, "CMoviePlayerGui::PlayFile: Starting Playback failed!\n");
				playback->Close();
				restoreNeutrino();
			} 
			else 
			{
				// set PlayState
				playstate = CMoviePlayerGui::PLAY;

				CVFD::getInstance()->ShowIcon(VFD_ICON_PLAY, true);
				
				// PlayBack SetStartPosition for timeshift
				if(timeshift) 
				{
					startposition = -1;
					int i;
					int towait = (timeshift == TIMESHIFT) ? TIMESHIFT_SECONDS + 1 : TIMESHIFT_SECONDS;
					
					for(i = 0; i < 500; i++) 
					{
#if defined (PLATFORM_COOLSTREAM)					  
						playback->GetPosition(position, duration);
#else						
						playback->GetPosition((int64_t &)position, (int64_t &)duration);
#endif						
						startposition = (duration - position);

						printf("CMoviePlayerGui::PlayFile: waiting for data, position %d duration %d (%d)\n", position, duration, towait);
						
						if(startposition > towait*1000)
							break;
						
						usleep(TIMESHIFT_SECONDS*1000);
					}
					
					if(timeshift == TIMESHIFT)
					{
						startposition = 0;
						
						//wait
						usleep(TIMESHIFT_SECONDS*1000);
					}
					else if(timeshift == R_TIMESHIFT) 
					{
						startposition = duration;	
					}
					else if(timeshift == P_TIMESHIFT)
					{
						if(duration > 1000000)
							startposition = duration - TIMESHIFT_SECONDS*1000;
						else
							startposition = 0;
					}
					
					dprintf(DEBUG_NORMAL, "CMoviePlayerGui::PlayFile: Timeshift %d, position %d, seek to %d seconds\n", timeshift, position, startposition/1000);
				}
				
				// set position 
				if( !is_file_player && startposition >= 0)//FIXME no jump for file at start yet
					playback->SetPosition((int64_t)startposition);
				
				if(timeshift == R_TIMESHIFT) 
					playback->SetSpeed(-1);
				
#if defined (PLATFORM_COOLSTREAM)
				playback->GetPosition(position, duration);
#else
				playback->GetPosition((int64_t &)position, (int64_t &)duration);
#endif	
				
				// show movieinfoviewer at start
				g_InfoViewer->showMovieInfo(Title, Info1, file_prozent, duration, ac3state, speed, playstate, false, isMovieBrowser);
			}
		}

		//get position/duration/speed during playing
		if ( playstate >= CMoviePlayerGui::PLAY )
		{
#if defined (PLATFORM_COOLSTREAM)
			if( playback->GetPosition(position, duration) )
#else
			if( playback->GetPosition((int64_t &)position, (int64_t &)duration) )
#endif			
			{					
				if(duration > 100)
					file_prozent = (unsigned char) (position / (duration / 100));

				playback->GetSpeed(speed);
								
				dprintf(DEBUG_DEBUG, "CMoviePlayerGui::PlayFile: speed %d position %d duration %d percent(%d%%)\n", speed, position, duration, file_prozent);					
			}
			else
			{
				if(filelist.size() > 1 && selected + 1 < filelist.size())
					g_RCInput->postMsg(CRCInput::RC_next, 0);
				else
					g_RCInput->postMsg((neutrino_msg_t) g_settings.mpkey_stop, 0);
			}
		}
		
		// loop msg
		g_RCInput->getMsg(&msg, &data, 10);	// 1 secs
		
		if (msg == (neutrino_msg_t) g_settings.mpkey_stop) 
		{
			//exit play
			playstate = CMoviePlayerGui::STOPPED;

			if (isMovieBrowser == true && moviebrowser->getMode() != MB_SHOW_FILES) 
			{
				// if we have a movie information, try to save the stop position
				ftime(&current_time);
				p_movie_info->dateOfLastPlay = current_time.time;
				current_time.time = time(NULL);
				p_movie_info->bookmarks.lastPlayStop = position / 1000;
				
				if(p_movie_info->epgChannel.empty())
					p_movie_info->epgChannel = sel_filename;
				if(p_movie_info->epgTitle.empty())
					p_movie_info->epgTitle = sel_filename;

				cMovieInfo.saveMovieInfo(*p_movie_info);
				//p_movie_info->fileInfoStale(); //TODO: we might to tell the Moviebrowser that the movie info has changed, but this could cause long reload times  when reentering the Moviebrowser
			}
			
			if(timeshift == TIMESHIFT) //timeshift
			{
				CVCRControl::getInstance()->Stop();
				
				g_Timerd->stopTimerEvent(CNeutrinoApp::getInstance()->recording_id); // this stop immediatly timeshift
				CNeutrinoApp::getInstance()->recording_id = 0;
				CNeutrinoApp::getInstance()->recordingstatus = 0;
				CNeutrinoApp::getInstance()->timeshiftstatus = 0;
				
				CVFD::getInstance()->ShowIcon(VFD_ICON_TIMESHIFT, false );
			}
			
			if(isURL)
				was_file = false;
			
			if(m_loop)
				m_loop = false;
			
			if(m_multiselect)
				m_multiselect = false;
			
			exit = true;
		} 
		else if (msg == (neutrino_msg_t) g_settings.mpkey_play) 
		{
			if (playstate >= CMoviePlayerGui::PLAY) 
			{
				playstate = CMoviePlayerGui::PLAY;
				update_lcd = true;
				CVFD::getInstance()->ShowIcon(VFD_ICON_PLAY, true);
				CVFD::getInstance()->ShowIcon(VFD_ICON_PAUSE, false);
				
				speed = 1;
				playback->SetSpeed(speed);
			} 
			else if (!timeshift) //???
			{
				open_filebrowser = true;
			}

			if (time_forced) 
			{
				time_forced = false;
				
				FileTime.hide();
			}
			
			if (FileTime.IsVisible()) 
				FileTime.hide();
			
			if(g_InfoViewer->m_visible)
				g_InfoViewer->killTitle();

			// movie title
			if(!timeshift)
			{
				if (FileTime.IsVisible()) 
				{
					if (FileTime.GetMode() == CTimeOSD::MODE_ASC) 
					{
						FileTime.SetMode(CTimeOSD::MODE_DESC);
						FileTime.update((duration - position) / 1000);
					} 
					else 
					{
						FileTime.hide();
					}
				}
				else 
				{
					FileTime.SetMode(CTimeOSD::MODE_ASC);
					FileTime.show(position / 1000);
				}
				
				if(!g_InfoViewer->m_visible)
					g_InfoViewer->showMovieInfo(Title, Info1, file_prozent, duration, ac3state, speed, playstate, true, isMovieBrowser);
			}
		} 
		else if ( msg == (neutrino_msg_t) g_settings.mpkey_pause) 
		{
			update_lcd = true;
			
			if (playstate == CMoviePlayerGui::PAUSE) 
			{
				playstate = CMoviePlayerGui::PLAY;
				CVFD::getInstance()->ShowIcon(VFD_ICON_PAUSE, false);
				// show play icon
				CVFD::getInstance()->ShowIcon(VFD_ICON_PLAY, true);
				speed = 1;
				playback->SetSpeed(speed);
			} 
			else 
			{
				playstate = CMoviePlayerGui::PAUSE;
				CVFD::getInstance()->ShowIcon(VFD_ICON_PAUSE, true);
				CVFD::getInstance()->ShowIcon(VFD_ICON_PLAY, false);
				speed = 0;
				playback->SetSpeed(speed);
			}
			
			if (FileTime.IsVisible()) 
				FileTime.hide();

			//show MovieInfoBar
			if(!timeshift)
			{
				if (FileTime.IsVisible()) 
				{
					if (FileTime.GetMode() == CTimeOSD::MODE_ASC) 
					{
						FileTime.SetMode(CTimeOSD::MODE_DESC);
						FileTime.update((duration - position) / 1000);
					} 
					else 
					{
						FileTime.hide();
					}
				}
				else 
				{
					FileTime.SetMode(CTimeOSD::MODE_ASC);
					FileTime.show(position / 1000);
				}
				
				if(!g_InfoViewer->m_visible)
					g_InfoViewer->showMovieInfo(Title, Info1, file_prozent, duration, ac3state, speed, playstate, true, isMovieBrowser);
			}
		} 
		else if (msg == (neutrino_msg_t) g_settings.mpkey_bookmark) 
		{
			if (FileTime.IsVisible()) 
				FileTime.hide();
						
			if(isMovieBrowser == true)
			{
				int pos_sec = position / 1000;

				if (newComHintBox.isPainted() == true) 
				{
					// yes, let's get the end pos of the jump forward
					new_bookmark.length = pos_sec - new_bookmark.pos;
					dprintf(DEBUG_DEBUG, "CMoviePlayerGui::PlayFile: commercial length: %d\r\n", new_bookmark.length);
					if (cMovieInfo.addNewBookmark(p_movie_info, new_bookmark) == true) 
					{
						cMovieInfo.saveMovieInfo(*p_movie_info);	/* save immediately in xml file */
					}
					new_bookmark.pos = 0;	// clear again, since this is used as flag for bookmark activity
					newComHintBox.hide();
				} 
				else if (newLoopHintBox.isPainted() == true) 
				{
					// yes, let's get the end pos of the jump backward
					new_bookmark.length = new_bookmark.pos - pos_sec;
					new_bookmark.pos = pos_sec;
					dprintf(DEBUG_DEBUG, "CMoviePlayerGui::PlayFile: loop length: %d\r\n", new_bookmark.length);
					if (cMovieInfo.addNewBookmark(p_movie_info, new_bookmark) == true) 
					{
						cMovieInfo.saveMovieInfo(*p_movie_info);	/* save immediately in xml file */
						jump_not_until = pos_sec + 5;	// avoid jumping for this time
					}
					new_bookmark.pos = 0;	// clear again, since this is used as flag for bookmark activity
					newLoopHintBox.hide();
				} 
				else 
				{
					// no, nothing else to do, we open a new bookmark menu
					new_bookmark.name = "";	// use default name
					new_bookmark.pos = 0;
					new_bookmark.length = 0;

					// next seems return menu_return::RETURN_EXIT, if something selected
					bookStartMenu.exec(NULL, "none");
					
					if (cSelectedMenuBookStart[0].selected == true) 
					{
						/* Moviebrowser plain bookmark */
						new_bookmark.pos = pos_sec;
						new_bookmark.length = 0;
						if (cMovieInfo.addNewBookmark(p_movie_info, new_bookmark) == true)
							cMovieInfo.saveMovieInfo(*p_movie_info);	/* save immediately in xml file */
						new_bookmark.pos = 0;	// clear again, since this is used as flag for bookmark activity
						cSelectedMenuBookStart[0].selected = false;	// clear for next bookmark menu
					} 
					else if (cSelectedMenuBookStart[1].selected == true)
					{
						/* Moviebrowser jump forward bookmark */
						new_bookmark.pos = pos_sec;
						dprintf(DEBUG_DEBUG, "CMoviePlayerGui::PlayFile: new bookmark 1. pos: %d\r\n", new_bookmark.pos);
						newComHintBox.paint();

						cSelectedMenuBookStart[1].selected = false;	// clear for next bookmark menu
					} 
					else if (cSelectedMenuBookStart[2].selected == true) 
					{
						/* Moviebrowser jump backward bookmark */
						new_bookmark.pos = pos_sec;
						dprintf(DEBUG_DEBUG, "CMoviePlayerGui::PlayFile: new bookmark 1. pos: %d\r\n", new_bookmark.pos);
						newLoopHintBox.paint();
						cSelectedMenuBookStart[2].selected = false;	// clear for next bookmark menu
					} 
					else if (cSelectedMenuBookStart[3].selected == true) 
					{
						/* Moviebrowser movie start bookmark */
						p_movie_info->bookmarks.start = pos_sec;
						dprintf(DEBUG_DEBUG, "CMoviePlayerGui::PlayFile: New movie start pos: %d\r\n", p_movie_info->bookmarks.start);
						cMovieInfo.saveMovieInfo(*p_movie_info);	/* save immediately in xml file */
						cSelectedMenuBookStart[3].selected = false;	// clear for next bookmark menu
					} 
					else if (cSelectedMenuBookStart[4].selected == true) 
					{
						/* Moviebrowser movie end bookmark */
						p_movie_info->bookmarks.end = pos_sec;
						dprintf(DEBUG_DEBUG, "CMoviePlayerGui::PlayFile: New movie end pos: %d\r\n", p_movie_info->bookmarks.start);
						cMovieInfo.saveMovieInfo(*p_movie_info);	/* save immediately in xml file */
						cSelectedMenuBookStart[4].selected = false;	// clear for next bookmark menu
					}
				}
			}		
		} 
		else if ( (msg == (neutrino_msg_t) g_settings.mpkey_audio) || ( msg == CRCInput::RC_audio) ) 
		{
			if (FileTime.IsVisible()) 
				FileTime.hide();
			
			CAVPIDSelectWidget * AVSelectHandler = new CAVPIDSelectWidget();
			AVSelectHandler->exec(NULL, "");
							
			delete AVSelectHandler;
			AVSelectHandler = NULL;
		} 
		else if(msg == CRCInput::RC_yellow)
		{
			if (FileTime.IsVisible()) 
				FileTime.hide();
			
			//show help
			showHelpTS();
		}
		else if (msg == CRCInput::RC_info) 
		{
			if (FileTime.IsVisible()) 
				FileTime.hide();
				
			if( !g_InfoViewer->m_visible )
				g_InfoViewer->showMovieInfo(Title, Info1, file_prozent, duration, ac3state, speed, playstate, true, isMovieBrowser);
		}
		else if ( msg == (neutrino_msg_t) g_settings.mpkey_time )
		{
			if(!timeshift)
			{
				if (FileTime.IsVisible()) 
				{
					if (FileTime.GetMode() == CTimeOSD::MODE_ASC) 
					{
						FileTime.SetMode(CTimeOSD::MODE_DESC);
						FileTime.update((duration - position) / 1000);
					} 
					else 
					{
						FileTime.hide();
					}
				}
				else 
				{
					FileTime.SetMode(CTimeOSD::MODE_ASC);
					FileTime.show(position / 1000);
				}
			}
			else
			{
				if (FileTime.IsVisible()) 
					FileTime.hide();
				else
				{
					FileTime.SetMode(CTimeOSD::MODE_ASC);
					FileTime.show(position / 1000);
				}
			}
		} 
		else if (msg == (neutrino_msg_t) g_settings.mpkey_rewind) 
		{
			// backward
			speed = (speed >= 0) ? -1 : speed - 1;
						
			if(speed < -15)
				speed = -15;			
			
			// hide icons
			CVFD::getInstance()->ShowIcon(VFD_ICON_PLAY, false);
			CVFD::getInstance()->ShowIcon(VFD_ICON_PAUSE, false);

			playback->SetSpeed(speed);
			playstate = CMoviePlayerGui::REW;
			update_lcd = true;

			if (FileTime.IsVisible()) 
				FileTime.hide();
			
			// time
			if (!FileTime.IsVisible()) 
			{
				if( !g_InfoViewer->is_visible)
				{
					FileTime.SetMode(CTimeOSD::MODE_ASC);
					FileTime.show(position / 1000);
					
					time_forced = true;
				}
			}
			
			if(!g_InfoViewer->m_visible)
				g_InfoViewer->showMovieInfo(Title, Info1, file_prozent, duration, ac3state, speed, playstate, true, isMovieBrowser);
		}
		else if (msg == (neutrino_msg_t) g_settings.mpkey_forward) 
		{	// fast-forward
			speed = (speed <= 0) ? 2 : speed + 1;
						
			if(speed > 15)
				speed = 15;			
			
			// icons
			CVFD::getInstance()->ShowIcon(VFD_ICON_PLAY, false);
			CVFD::getInstance()->ShowIcon(VFD_ICON_PAUSE, false);

			playback->SetSpeed(speed);

			update_lcd = true;
			playstate = CMoviePlayerGui::FF;

			if (FileTime.IsVisible()) 
				FileTime.hide();

			// movie info viewer
			// time
			if (!FileTime.IsVisible()) 
			{
				if( !g_InfoViewer->is_visible)
				{
					FileTime.SetMode(CTimeOSD::MODE_ASC);
					FileTime.show(position / 1000);
					
					time_forced = true;
				}
			}
			
			if(!g_InfoViewer->m_visible)
				g_InfoViewer->showMovieInfo(Title, Info1, file_prozent, duration, ac3state, speed, playstate, true, isMovieBrowser);
		} 
		else if (msg == CRCInput::RC_1) 
		{	// Jump Backwards 1 minute
			//update_lcd = true;
			playback->SetPosition(-60 * 1000);
			
			// time
			if (!FileTime.IsVisible()) 
			{
				if( !g_InfoViewer->is_visible)
				{
					FileTime.SetMode(CTimeOSD::MODE_ASC);
					FileTime.show(position / 1000);
					
					time_forced = true;
				}
			}
		} 
		else if (msg == CRCInput::RC_3) 
		{	// Jump Forward 1 minute
			//update_lcd = true;
			playback->SetPosition(60 * 1000);
			
			// time
			if (!FileTime.IsVisible()) 
			{
				if( !g_InfoViewer->is_visible)
				{
					FileTime.SetMode(CTimeOSD::MODE_ASC);
					FileTime.show(position / 1000);
					
					time_forced = true;
				}
			}
		} 
		else if (msg == CRCInput::RC_4) 
		{	// Jump Backwards 5 minutes
			playback->SetPosition(-5 * 60 * 1000);
			
			// time
			if (!FileTime.IsVisible()) 
			{
				if( !g_InfoViewer->is_visible)
				{
					FileTime.SetMode(CTimeOSD::MODE_ASC);
					FileTime.show(position / 1000);
					
					time_forced = true;
				}
			}
		} 
		else if (msg == CRCInput::RC_6) 
		{	// Jump Forward 5 minutes
			playback->SetPosition(5 * 60 * 1000);
			
			// time
			if (!FileTime.IsVisible()) 
			{
				if( !g_InfoViewer->is_visible)
				{
					FileTime.SetMode(CTimeOSD::MODE_ASC);
					FileTime.show(position / 1000);
					
					time_forced = true;
				}
			}
		} 
		else if (msg == CRCInput::RC_7) 
		{	// Jump Backwards 10 minutes
			playback->SetPosition(-10 * 60 * 1000);
			
			// time
			if (!FileTime.IsVisible()) 
			{
				if( !g_InfoViewer->is_visible)
				{
					FileTime.SetMode(CTimeOSD::MODE_ASC);
					FileTime.show(position / 1000);
					
					time_forced = true;
				}
			}
		} 
		else if (msg == CRCInput::RC_9) 
		{	// Jump Forward 10 minutes
			playback->SetPosition(10 * 60 * 1000);
			
			// time
			if (!FileTime.IsVisible()) 
			{
				if( !g_InfoViewer->is_visible)
				{
					FileTime.SetMode(CTimeOSD::MODE_ASC);
					FileTime.show(position / 1000);
					
					time_forced = true;
				}
			}
		} 
		else if ( msg == CRCInput::RC_2 )
		{	// goto start
			playback->SetPosition((int64_t)startposition);
			
			// time
			if (!FileTime.IsVisible()) 
			{
				if( !g_InfoViewer->is_visible)
				{
					FileTime.SetMode(CTimeOSD::MODE_ASC);
					FileTime.show(position / 1000);
					
					time_forced = true;
				}
			}
		} 
		else if ( msg == CRCInput::RC_repeat )
		{
			if(m_loop)
				m_loop = false;
			else
				m_loop = true;
			
			dprintf(DEBUG_NORMAL, "CMoviePlayerGui::PlayFile: Repeat Modus: [%s]\n", m_loop? "ON" : "OFF");
		} 
		else if (msg == CRCInput::RC_5) 
		{	
			// goto middle
			playback->SetPosition((int64_t)duration/2);
			
			// time
			if (!FileTime.IsVisible()) 
			{
				if( !g_InfoViewer->is_visible)
				{
					FileTime.SetMode(CTimeOSD::MODE_ASC);
					FileTime.show(position / 1000);
					
					time_forced = true;
				}
			}
		} 
		else if (msg == CRCInput::RC_8) 
		{	
			// goto end
			playback->SetPosition((int64_t)duration - 60 * 1000);
			
			//time
			if (!FileTime.IsVisible()) 
			{
				if( !g_InfoViewer->is_visible)
				{
					FileTime.SetMode(CTimeOSD::MODE_ASC);
					FileTime.show(position / 1000);
					
					time_forced = true;
				}
			}
		} 
		else if (msg == CRCInput::RC_page_up) 
		{
			playback->SetPosition(10 * 1000);
			
			// time
			if (!FileTime.IsVisible()) 
			{
				if( !g_InfoViewer->is_visible)
				{
					FileTime.SetMode(CTimeOSD::MODE_ASC);
					FileTime.show(position / 1000);
					
					time_forced = true;
				}
			}

		} 
		else if (msg == CRCInput::RC_page_down) 
		{
			playback->SetPosition(-10 * 1000);
			
			// time
			if (!FileTime.IsVisible()) 
			{
				if( !g_InfoViewer->is_visible)
				{
					FileTime.SetMode(CTimeOSD::MODE_ASC);
					FileTime.show(position / 1000);
					
					time_forced = true;
				}
			}
		} 
		else if (msg == CRCInput::RC_0) 
		{
			// cancel bookmark jump
			if (isMovieBrowser == true) 
			{
				if (new_bookmark.pos != 0) 
				{
					new_bookmark.pos = 0;	// stop current bookmark activity, TODO:  might bemoved to another key
					newLoopHintBox.hide();	// hide hint box if any
					newComHintBox.hide();
				}
				jump_not_until = (position / 1000) + 10;	// avoid bookmark jumping for the next 10 seconds, , TODO:  might be moved to another key
			} 
			else if (playstate != CMoviePlayerGui::PAUSE)
				playstate = CMoviePlayerGui::SOFTRESET;
		} 
#if !defined (PLATFORM_COOLSTREAM)		
		else if (msg == CRCInput::RC_slow) 
		{
			if (slow > 0)
				slow = 0;
			
			slow += 2;
		
			// set slow
			playback->SetSlow(slow);
			//update_lcd = true;
			playstate = CMoviePlayerGui::SLOW;
			update_lcd = true;
		}
#endif		
		else if(msg == CRCInput::RC_red)
		{
			if (FileTime.IsVisible()) 
				FileTime.hide();
			
				//showFileInfo();
		}
		else if(msg == CRCInput::RC_home)
		{
			if (FileTime.IsVisible()) 
				FileTime.hide();
			
			if(g_InfoViewer->m_visible)
				  g_InfoViewer->killTitle();
		}
		else if(msg == CRCInput::RC_left || msg == CRCInput::RC_prev)
		{
			if(!filelist.empty() && selected > 0 && playstate == CMoviePlayerGui::PLAY) 
			{
				selected--;
				filename = filelist[selected].Name.c_str();
				sel_filename = filelist[selected].getFileName();
				
				Title = sel_filename;
				Info1 = sel_filename = sel_filename;
				Info2 = sel_filename;
				//thumbnail
				Thumbnail = "";
				std::string fname = "";
				fname = filelist[selected].Name;
				changeFileNameExt(fname, ".jpg");
						
				if(!access(fname.c_str(), F_OK) )
					Thumbnail = fname.c_str();
				
				update_lcd = true;
				start_play = true;
			}
		}
		else if(msg == CRCInput::RC_right || msg == CRCInput::RC_next)
		{
			if(!filelist.empty() && selected + 1 < filelist.size() && playstate == CMoviePlayerGui::PLAY) 
			{
				selected++;
				filename = filelist[selected].Name.c_str();
				sel_filename = filelist[selected].getFileName();
				
				Title = sel_filename;
				Info1 = sel_filename = sel_filename;
				Info2 = sel_filename;
				//thumbnail
				Thumbnail = "";
				std::string fname = "";
				fname = filelist[selected].Name;
				changeFileNameExt(fname, ".jpg");
						
				if(!access(fname.c_str(), F_OK) )
					Thumbnail = fname.c_str();
				
				update_lcd = true;
				start_play = true;
			}
		}
		else if (msg == (neutrino_msg_t)g_settings.key_screenshot && isMovieBrowser == true )
		{
         		if(MessageBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_SCREENSHOT_ANNOUNCE), CMessageBox::mbrNo, CMessageBox:: mbYes | CMessageBox::mbNo) == CMessageBox::mbrYes) 
			{
				CVCRControl::getInstance()->Screenshot(0, (char *)filename);
			}
		}
		else if ((msg == NeutrinoMessages::ANNOUNCE_RECORD) || msg == NeutrinoMessages::RECORD_START || msg == NeutrinoMessages::ZAPTO || msg == NeutrinoMessages::STANDBY_ON || msg == NeutrinoMessages::SHUTDOWN || msg == NeutrinoMessages::SLEEPTIMER) 
		{	
			// Exit for Record/Zapto Timers
			exit = true;
			g_RCInput->postMsg(msg, data);
		}
		else 
		{
			if (CNeutrinoApp::getInstance()->handleMsg(msg, data) & messages_return::cancel_all)
				exit = true;
			else if ( msg <= CRCInput::RC_MaxRC ) 
			{
				CVFD::getInstance()->setMode(CVFD::MODE_MENU_UTF8);
	
				update_lcd = true;
			}
		}

		if (exit) 
		{
			dprintf(DEBUG_NORMAL, "CMoviePlayerGui::PlayFile: stop (1)\n");	

			if (isMovieBrowser == true && moviebrowser->getMode() != MB_SHOW_FILES) 
			{
				// if we have a movie information, try to save the stop position
				ftime(&current_time);
				p_movie_info->dateOfLastPlay = current_time.time;
				current_time.time = time(NULL);
				p_movie_info->bookmarks.lastPlayStop = position / 1000;
				
				if(p_movie_info->epgChannel.empty())
					p_movie_info->epgChannel = sel_filename;
				if(p_movie_info->epgTitle.empty())
					p_movie_info->epgTitle = sel_filename;

				cMovieInfo.saveMovieInfo(*p_movie_info);
				//p_movie_info->fileInfoStale(); //TODO: we might to tell the Moviebrowser that the movie info has changed, but this could cause long reload times  when reentering the Moviebrowser
			}
		}
	} while (playstate >= CMoviePlayerGui::PLAY);
	
	dprintf(DEBUG_NORMAL, "CMoviePlayerGui::PlayFile: stop (2)\n");	

	if(FileTime.IsVisible())
		FileTime.hide();
	
	if(g_InfoViewer->m_visible)
		g_InfoViewer->killTitle();
	
	playback->Close();

	CVFD::getInstance()->ShowIcon(VFD_ICON_PLAY, false);
	CVFD::getInstance()->ShowIcon(VFD_ICON_PAUSE, false);

	if (was_file) 
	{
		usleep(3000);
		
		if(!isURL)
			open_filebrowser = true;
		
		if(m_multiselect || m_loop)
			start_play = true;
		
		goto go_repeat;
	}
}

void CMoviePlayerGui::showHelpTS()
{
	Helpbox helpbox;
	helpbox.addLine(NEUTRINO_ICON_BUTTON_RED, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP1));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_GREEN, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP2));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_YELLOW, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP3));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_BLUE, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP4));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_SETUP, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP5));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_HELP, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP5));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_1, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP6));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_2, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP12) );
	helpbox.addLine(NEUTRINO_ICON_BUTTON_3, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP7));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_4, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP8));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_5, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP13));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_6, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP9));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_7, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP10));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_8, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP14));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_9, g_Locale->getText(LOCALE_MOVIEPLAYER_TSHELP11));
	helpbox.addLine("Version: $Revision: 1.97 $");
	helpbox.addLine("Movieplayer (c) 2003, 2004 by gagga");
	hide();
	helpbox.show(LOCALE_MESSAGEBOX_INFO);
}

/*
void CMoviePlayerGui::showFileInfo()
{
	dprintf(DEBUG_NORMAL, "CMoviePlayerGui::showFileInfo:%s %s %s\n", Title.c_str(), Info1.c_str(), Info2.c_str());
	
	std::string buffer;
	
	// prepare print buffer  
	buffer = Info1;
	buffer += "\n";
	buffer += Info2;
	buffer += "\n";

	// thumbnail
	int pich = 246;	//FIXME
	int picw = 162; 	//FIXME
	int lx = g_settings.screen_StartX + 50 + g_settings.screen_EndX - g_settings.screen_StartX - 100 - (picw + 10 + SCROLLBAR_WIDTH);
	int ly = g_settings.screen_StartY + 50 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight() + 20;
		
	if(access(Thumbnail.c_str(), F_OK))
		Thumbnail = "";
	
	int mode =  CInfoBox::SCROLL | CInfoBox::TITLE | CInfoBox::FOOT | CInfoBox::BORDER;// | //CInfoBox::NO_AUTO_LINEBREAK | //CInfoBox::CENTER | //CInfoBox::AUTO_WIDTH | //CInfoBox::AUTO_HIGH;
	CBox position(g_settings.screen_StartX + 50, g_settings.screen_StartY + 50, g_settings.screen_EndX - g_settings.screen_StartX - 100, g_settings.screen_EndY - g_settings.screen_StartY - 100); 
	
	CInfoBox * infoBox = new CInfoBox(Title.c_str(), g_Font[SNeutrinoSettings::FONT_TYPE_MENU], mode, &position, Title.c_str(), g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE], NULL);
	infoBox->setText(&buffer, Thumbnail, lx, ly, picw, pich);
	infoBox->exec();
	delete infoBox;
}
*/

