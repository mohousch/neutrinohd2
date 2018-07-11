/*
  Neutrino-GUI  -   DBoxII-Project
  
  $Id: audioplayer.cpp 2018/07/10 mohousch Exp $

  AudioPlayer by Dirch,Zwen

  (C) 2002-2008 the tuxbox project contributors
  (C) 2008 Novell, Inc. Author: Stefan Seyfried

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <dirent.h>

#include <gui/audioplayer.h>

#include <global.h>
#include <neutrino.h>

#include <driver/encoding.h>
#include <driver/fontrenderer.h>
#include <driver/rcinput.h>
#include <driver/audiometadata.h>

#include <daemonc/remotecontrol.h>

#include <gui/eventlist.h>
#include <gui/color.h>
#include <gui/infoviewer.h>
#include <gui/nfs.h>

#include <gui/widget/buttons.h>
#include <gui/widget/icons.h>
#include <gui/widget/menue.h>
#include <gui/widget/messagebox.h>
#include <gui/widget/hintbox.h>
#include <gui/widget/stringinput.h>
#include <gui/widget/stringinput_ext.h>
#include <gui/widget/progresswindow.h>
#include <gui/widget/items2detailsline.h>
#include <gui/audioplayer_setup.h>

#include <system/settings.h>
#include <system/helpers.h>

#include <xmlinterface.h>

#include <algorithm>
#include <sys/time.h>
#include <fstream>
#include <iostream>

#include <system/debug.h>

#include <curl/curl.h>
#include <curl/easy.h>


extern int current_muted;

#define AUDIOPLAYER_START_SCRIPT 			CONFIGDIR "/audioplayer.start"
#define AUDIOPLAYER_END_SCRIPT 				CONFIGDIR "/audioplayer.end"


CAudiofileExt::CAudiofileExt()
: CAudiofile(), firstChar('\0')
{
}

CAudiofileExt::CAudiofileExt(std::string name, CFile::FileExtension extension)
: CAudiofile(name, extension), firstChar('\0')
{
}

CAudiofileExt::CAudiofileExt(const CAudiofileExt& src)
: CAudiofile(src), firstChar(src.firstChar)
{
}

void CAudiofileExt::operator=(const CAudiofileExt& src)
{
	if (&src == this)
		return;
	
	CAudiofile::operator=(src);
	firstChar = src.firstChar;
}

CAudioPlayerGui::CAudioPlayerGui()
{
	m_frameBuffer = CFrameBuffer::getInstance();

	Init();
}

void CAudioPlayerGui::Init(void)
{
	m_inetmode = false;

	stimer = 0;
	m_current = 0;
	m_metainfo.clear();
}

CAudioPlayerGui::~CAudioPlayerGui()
{
	m_playlist.clear();
}

int CAudioPlayerGui::exec(CMenuTarget * parent, const std::string &actionKey)
{
	dprintf(DEBUG_NORMAL, "CAudioPlayerGui::exec: actionKey:%s\n", actionKey.c_str());

	CAudioPlayer::getInstance()->init();
	
	m_state = CAudioPlayerGui::STOP;

	m_width = m_frameBuffer->getScreenWidth(true) - 20; 
	
	if((g_settings.screen_EndX - g_settings.screen_StartX) < m_width+ConnectLineBox_Width)
		m_width = (g_settings.screen_EndX - g_settings.screen_StartX) - 10;

	m_title_height = 80;

	m_x = (((g_settings.screen_EndX - g_settings.screen_StartX) - m_width)/ 2) + g_settings.screen_StartX;

	m_y = g_settings.screen_StartY + 10;
	
	m_idletime = time(NULL);

	if(parent)
		parent->hide(); 
	
	// save background
	bool usedBackground = m_frameBuffer->getuseBackground();
	if (usedBackground)
		m_frameBuffer->saveBackgroundImage();
	
	//show audio background pic	
	m_frameBuffer->loadBackgroundPic("mp3.jpg");
	m_frameBuffer->blit();	
	
	// tell neutrino we're in audio mode
	CNeutrinoApp::getInstance()->handleMsg(NeutrinoMessages::CHANGEMODE , NeutrinoMessages::mode_audio );
	
	// remember last mode
	m_LastMode = (CNeutrinoApp::getInstance()->getLastMode() | NeutrinoMessages::norezap);
	
	// stop playback
	CNeutrinoApp::getInstance()->lockPlayBack();

	//start AP start-script
	puts("[audioplayer.cpp] executing " AUDIOPLAYER_START_SCRIPT "."); 
	if (system(AUDIOPLAYER_START_SCRIPT) != 0) 
		perror("Datei " AUDIOPLAYER_START_SCRIPT " fehlt.Bitte erstellen, wenn gebraucht.\nFile " AUDIOPLAYER_START_SCRIPT " not found. Please create if needed.\n");

	//show
	show();

	//restore previous background
	if (usedBackground)
		m_frameBuffer->restoreBackgroundImage();
	
	m_frameBuffer->useBackground(usedBackground);
		
	m_frameBuffer->paintBackground();
	m_frameBuffer->blit();		

	// end-script
	puts("[audioplayer.cpp] executing " AUDIOPLAYER_END_SCRIPT "."); 
	if (system(AUDIOPLAYER_END_SCRIPT) != 0) 
		perror("Datei " AUDIOPLAYER_END_SCRIPT " fehlt. Bitte erstellen, wenn gebraucht.\nFile " AUDIOPLAYER_END_SCRIPT " not found. Please create if needed.\n");
	
	// start playback
	CNeutrinoApp::getInstance()->unlockPlayBack();

	//set last saved mode
	CNeutrinoApp::getInstance()->handleMsg( NeutrinoMessages::CHANGEMODE, m_LastMode );
	
	//show infobar
	g_RCInput->postMsg( NeutrinoMessages::SHOW_INFOBAR, 0 );
	
	// remove mp3 cover
	if(!access("/tmp/cover.jpg", F_OK))
		remove("/tmp/cover.jpg");

	//always repaint
	return menu_return::RETURN_REPAINT;
}

int CAudioPlayerGui::show()
{
	dprintf(DEBUG_NORMAL, "CAudioPlayerGui::show\n");

	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	int ret = -1;

	CVFD::getInstance()->setMode(CVFD::MODE_AUDIO, g_Locale->getText(m_inetmode? LOCALE_INETRADIO_NAME : LOCALE_AUDIOPLAYER_HEAD));
		
	paintLCD();		

	bool loop = true;
	
	//
	if(!m_playlist.empty())
		play(m_current);

	// control loop
	while(loop)
	{
		if(CNeutrinoApp::getInstance()->getMode() != NeutrinoMessages::mode_audio)
		{
			// stop if mode was changed in another thread
			loop = false;
		}
		
		if ((m_state != CAudioPlayerGui::STOP) && (CAudioPlayer::getInstance()->getState() == CBaseDec::STOP) && (!m_playlist.empty()))
		{
			if(m_current == (m_playlist.size() - 1))
			{
				loop = false;	
			}

			if(m_curr_audiofile.FileExtension != CFile::EXTENSION_URL)
				playNext();
		}
		
		g_RCInput->getMsg(&msg, &data, 10); // 1 sec timeout to update play/stop state display

		paintInfo();

		if (msg == CRCInput::RC_home)
		{ 
			loop = false;
		}
		else if (msg == CRCInput::RC_left || msg == CRCInput::RC_prev)
		{
			playPrev();
		}
		else if (msg == CRCInput::RC_right || msg == CRCInput::RC_next)
		{
			playNext();
		}
		else if (msg == CRCInput::RC_stop)
		{
			stop();
		}
		else if( msg == CRCInput::RC_pause)
		{
			pause();
		}
		else if(msg == CRCInput::RC_play)
		{
			play(m_current);
		}
		else if(msg == CRCInput::RC_green)
		{
			if(m_state == CAudioPlayerGui::STOP)
			{
				if (!m_playlist.empty()) 
				{
					savePlaylist();

					CVFD::getInstance()->setMode(CVFD::MODE_AUDIO, g_Locale->getText(m_inetmode? LOCALE_INETRADIO_NAME : LOCALE_AUDIOPLAYER_HEAD));
						
					paintLCD();
				}
			} 
			else
			{
				// is no stream, so we do not have to test for this case
				int seconds = 0;
				CIntInput secondsInput(LOCALE_AUDIOPLAYER_JUMP_DIALOG_TITLE,
							seconds,
							5,
							LOCALE_AUDIOPLAYER_JUMP_DIALOG_HINT1,
							LOCALE_AUDIOPLAYER_JUMP_DIALOG_HINT2);
							
				int res = secondsInput.exec(NULL, "");
					
				if (seconds != 0 && res != menu_return::RETURN_EXIT_ALL)
					rev(seconds);
			}
		}
		else if(msg == CRCInput::RC_yellow)
		{
			if (m_state != CAudioPlayerGui::STOP)
			{
				// is no stream, so we do not have to test for this case
				int seconds = 0;
				CIntInput secondsInput(LOCALE_AUDIOPLAYER_JUMP_DIALOG_TITLE,
							seconds,
							5,
							LOCALE_AUDIOPLAYER_JUMP_DIALOG_HINT1,
							LOCALE_AUDIOPLAYER_JUMP_DIALOG_HINT2);
							
				int res = secondsInput.exec(NULL, "");
					
				if (seconds != 0 && res != menu_return::RETURN_EXIT_ALL)
					ff(seconds);
			}
		}
		else if( ((msg == CRCInput::RC_setup) || (msg == CRCInput::RC_vfdmenu)))
		{
			loop = false;
		}
		else if(msg == NeutrinoMessages::CHANGEMODE)
		{
			if((data & NeutrinoMessages::mode_mask) != NeutrinoMessages::mode_audio)
			{
				loop = false;
				m_LastMode = data;
			}
		}
		else if(msg == NeutrinoMessages::RECORD_START ||
				msg == NeutrinoMessages::ZAPTO ||
				msg == NeutrinoMessages::STANDBY_ON ||
				msg == NeutrinoMessages::SHUTDOWN ||
				msg == NeutrinoMessages::SLEEPTIMER)
		{
			// Exit for Record/Zapto Timers
			loop = false;
			g_RCInput->postMsg(msg, data);
		}
		else if(msg == NeutrinoMessages::EVT_TIMER)
		{
			CNeutrinoApp::getInstance()->handleMsg( msg, data );
		}
		else
		{
			if( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
			{
				loop = false;
			}
		}
			
		m_frameBuffer->blit();	
	}
	
	stop();	

	return ret;
}

bool CAudioPlayerGui::playNext(bool allow_rotate)
{
	bool result = false;

	if (!(m_playlist.empty()))
	{
		int next = getNext();
		
		if(next >= 0)
			play(next);
		else if(allow_rotate)
			play(0);
		else
			stop();
		
		result = true;
	}

	return(result);
}

bool CAudioPlayerGui::playPrev(bool allow_rotate)
{
	bool result = false;

	if (!(m_playlist.empty()))
	{
		if(m_current == -1)
			stop();
		else if(m_current - 1 > 0)
			play(m_current - 1);
		else if(allow_rotate)
			play(m_playlist.size()-1);
		else
			play(0);

		result = true;
	}

	return(result);
}

void CAudioPlayerGui::hide()
{
	// infos
	m_frameBuffer->paintBackgroundBoxRel(m_x, m_y, m_width, m_title_height);

	m_frameBuffer->blit();
}

void CAudioPlayerGui::paintInfo()
{
	// title info box
	m_frameBuffer->paintBoxRel(m_x, m_y, m_width, m_title_height, COL_MENUCONTENT_PLUS_6);//FIXME: gradient
		
	m_frameBuffer->paintBoxRel(m_x + 2, m_y + 2 , m_width - 4, m_title_height - 4, COL_MENUHEAD_INFO_PLUS_0, NO_RADIUS, CORNER_NONE, g_settings.Head_Info_gradient); //FIXME:gradient

	// first line 
	// Track number
	std::string tmp;
	if (m_inetmode) 
	{
		tmp = m_curr_audiofile.MetaData.album;
	} 
	else 
	{
		char sNr[20];
		sprintf(sNr, ": %2d", m_current + 1);
		tmp = g_Locale->getText(LOCALE_AUDIOPLAYER_PLAYING);
		tmp += sNr ;
	}

	int w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp, true); // UTF-8
	int xstart = (m_width - w) / 2;
	if(xstart < BORDER_LEFT)
		xstart = BORDER_LEFT;

	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + xstart, m_y + 4 + m_title_height/3, m_width - 20, tmp, COL_MENUHEAD_INFO, 0, true); // UTF-8

	// second line 
	// Artist/Title
	GetMetaData(m_curr_audiofile);

	if (m_curr_audiofile.MetaData.title.empty())
		tmp = m_curr_audiofile.MetaData.artist;
	else if (m_curr_audiofile.MetaData.artist.empty())
		tmp = m_curr_audiofile.MetaData.title;
	else 
	{
		tmp = m_curr_audiofile.MetaData.title;
		tmp += " / ";
		tmp += m_curr_audiofile.MetaData.artist;
	}

	w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp, true); // UTF-8
	xstart = (m_width - w)/2;
	if(xstart < BORDER_LEFT)
		xstart = BORDER_LEFT + m_title_height - 4 + m_title_height + ICON_OFFSET;

		
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + xstart, m_y + 2 + m_title_height/3 + 2 + m_title_height/3, m_width - BORDER_LEFT - BORDER_RIGHT, tmp, COL_MENUHEAD_INFO, 0, true); // UTF-8		
		
	// cover
	if (!m_curr_audiofile.MetaData.cover.empty())
	{
		if(!access("/tmp/cover.jpg", F_OK))
			m_frameBuffer->DisplayImage("/tmp/cover.jpg", m_x + 2, m_y + 2, m_title_height - 4, m_title_height - 4);		
	}

	//playstate
	int icon_w, icon_h;
	const char* icon = NEUTRINO_ICON_PLAY;
		
	switch(m_state)
	{
		case CAudioPlayerGui::PAUSE: icon = NEUTRINO_ICON_PAUSE; break;
		case CAudioPlayerGui::PLAY: icon = NEUTRINO_ICON_PLAY; break;
		case CAudioPlayerGui::REV: icon = NEUTRINO_ICON_REW; break;
		case CAudioPlayerGui::FF: icon = NEUTRINO_ICON_FF; break;
		case CAudioPlayerGui::STOP: break;
	}
	
	m_frameBuffer->getIconSize(icon, &icon_w, &icon_h);
	m_frameBuffer->paintIcon(icon, m_x + m_title_height + ICON_OFFSET, m_y + (m_title_height - icon_h)/2);
		
	//
	m_metainfo.clear();
	m_time_total = 0;
	m_time_played = 0;

	updateMetaData();

	// third line
	if(updateMeta || updateScreen)
	{
		int xstart = ((m_width - 20 - g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getRenderWidth(m_metainfo))/2) + 10;

		g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(m_x + xstart, m_y + m_title_height - g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight()/2, m_width- 2*xstart, m_metainfo, COL_MENUHEAD_INFO);
	}

	// update time
	updateTimes(true);
}

void CAudioPlayerGui::stop()
{
	m_state = CAudioPlayerGui::STOP;
	//m_current = 0;
		
	//LCD
	paintLCD();	

	if(CAudioPlayer::getInstance()->getState() != CBaseDec::STOP)
		CAudioPlayer::getInstance()->stop();
}

void CAudioPlayerGui::pause()
{
	if(m_state == CAudioPlayerGui::PLAY || m_state == CAudioPlayerGui::FF || m_state == CAudioPlayerGui::REV)
	{
		m_state = CAudioPlayerGui::PAUSE;
		CAudioPlayer::getInstance()->pause();
	}
	else if(m_state == CAudioPlayerGui::PAUSE)
	{
		m_state = CAudioPlayerGui::PLAY;
		CAudioPlayer::getInstance()->pause();
	}
		
	paintLCD();	
}

void CAudioPlayerGui::ff(unsigned int seconds)
{
	if(m_state == CAudioPlayerGui::FF)
	{
		m_state = CAudioPlayerGui::PLAY;
		CAudioPlayer::getInstance()->ff(seconds);
	}
	else if(m_state == CAudioPlayerGui::PLAY || m_state == CAudioPlayerGui::PAUSE || m_state == CAudioPlayerGui::REV)
	{
		m_state = CAudioPlayerGui::FF;
		CAudioPlayer::getInstance()->ff(seconds);
	}
	
	paintLCD();	
}

void CAudioPlayerGui::rev(unsigned int seconds)
{
	if(m_state == CAudioPlayerGui::REV)
	{
		m_state = CAudioPlayerGui::PLAY;
		CAudioPlayer::getInstance()->rev(seconds);
	}
	else if(m_state == CAudioPlayerGui::PLAY 
			|| m_state == CAudioPlayerGui::PAUSE
			|| m_state == CAudioPlayerGui::FF)
	{
		m_state = CAudioPlayerGui::REV;
		CAudioPlayer::getInstance()->rev(seconds);
	}

	paintLCD();
}

void CAudioPlayerGui::play(unsigned int pos)
{
	dprintf(DEBUG_NORMAL, "CAudioPlayerGui::play\n");

	if(!m_playlist.size())
		return;
	
	unsigned int old_current = m_current;

	m_current = pos;

	// metadata
	if ( (m_playlist[pos].FileExtension != CFile::EXTENSION_M3U || m_playlist[pos].FileExtension != CFile::EXTENSION_URL || m_playlist[pos].FileExtension != CFile::EXTENSION_PLS) && !m_playlist[pos].MetaData.bitrate)
	{
		GetMetaData(m_playlist[pos]);
	}
	
	m_metainfo.clear();
	m_time_played = 0;
	m_time_total = m_playlist[m_current].MetaData.total_time;
	m_state = CAudioPlayerGui::PLAY;

	//
	m_curr_audiofile = m_playlist[m_current];

	// play
	CAudioPlayer::getInstance()->play(&m_curr_audiofile, g_settings.audioplayer_highprio == 1);

	//lcd	
	paintLCD();
	
	paintInfo();
}

int CAudioPlayerGui::getNext()
{
	int ret = m_current + 1;
	if(m_playlist.empty())
		return -1;
	
	if((unsigned)ret >= m_playlist.size()) 
	{
		if (g_settings.audioplayer_repeat_on == 1)
			ret = 0;
		else
			ret = -1;
	}

	return ret;
}

void CAudioPlayerGui::updateMetaData()
{
	updateMeta = false;
	updateLcd = false;
	updateScreen = false;

	if(m_state == CAudioPlayerGui::STOP)
		return;

	if( CAudioPlayer::getInstance()->hasMetaDataChanged() || m_metainfo.empty() )
	{
		const CAudioMetaData meta = CAudioPlayer::getInstance()->getMetaData();

		std::stringstream info;
		info.precision(3);

		if ( meta.bitrate > 0 )
		{
			info << meta.bitrate/1000 << "kbps";
		}

		if ( meta.samplerate > 0 )
		{
			info << " / " << meta.samplerate/1000 << "." << (meta.samplerate/100)%10 <<"kHz";
		}

		m_metainfo = meta.type_info + info.str();
		updateMeta = true;

		if (!meta.artist.empty()  && meta.artist != m_curr_audiofile.MetaData.artist)
		{
			m_curr_audiofile.MetaData.artist = meta.artist;
			updateScreen = true;
			updateLcd = true;
		}
		
		if (!meta.title.empty() && meta.title != m_curr_audiofile.MetaData.title)
		{
			m_curr_audiofile.MetaData.title = meta.title;
			updateScreen = true;
			updateLcd = true;
		}
		
		if (!meta.sc_station.empty()  && meta.sc_station != m_curr_audiofile.MetaData.album)
		{
			m_curr_audiofile.MetaData.album = meta.sc_station;
			updateLcd = true;
		}
	}
	
	if (CAudioPlayer::getInstance()->hasMetaDataChanged() != 0)
		updateLcd = true;
		
	if(updateLcd)
		paintLCD();
}

void CAudioPlayerGui::updateTimes(const bool force)
{
	if (m_state != CAudioPlayerGui::STOP)
	{
		bool updateTotal = force;
		bool updatePlayed = force;

		if (m_time_total != CAudioPlayer::getInstance()->getTimeTotal())
		{
			m_time_total = CAudioPlayer::getInstance()->getTimeTotal();
			if (m_curr_audiofile.MetaData.total_time != CAudioPlayer::getInstance()->getTimeTotal())
			{
				m_curr_audiofile.MetaData.total_time = CAudioPlayer::getInstance()->getTimeTotal();
				if(m_current >= 0)
					m_playlist[m_current].MetaData.total_time = CAudioPlayer::getInstance()->getTimeTotal();
			}
			updateTotal = true;
		}
		
		if ((m_time_played != CAudioPlayer::getInstance()->getTimePlayed()))
		{
			m_time_played = CAudioPlayer::getInstance()->getTimePlayed();
			updatePlayed = true;
		}
		
		//NOTE:time played
		char tot_time[11];
		snprintf(tot_time, 10, " / %ld:%02ld", m_time_total / 60, m_time_total % 60);
		char tmp_time[8];
		snprintf(tmp_time, 7, "%ld:00", m_time_total / 60);
		char play_time[8];
		snprintf(play_time, 7, "%ld:%02ld", m_time_played / 60, m_time_played % 60);

		int w1 = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tot_time);
		int w2 = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp_time);

		if (updateTotal)
		{
			if(m_time_total > 0)
				g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + m_width - w1 - 10, m_y + 4 + m_title_height/3, w1, tot_time, COL_MENUHEAD_INFO);
		}
			
		if (updatePlayed || (m_state == CAudioPlayerGui::PAUSE))
		{
			struct timeval tv;
			gettimeofday(&tv, NULL);

			if ((m_state != CAudioPlayerGui::PAUSE) || (tv.tv_sec & 1))
			{
				g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + m_width - w1 - w2 - 11, m_y + 4 + m_title_height/3, w2 + 4, play_time, COL_MENUHEAD_INFO);
			}
		}			
		
#if ENABLE_LCD	
		if((updatePlayed || updateTotal) && m_time_total != 0)
		{
			CVFD::getInstance()->showAudioProgress(100 * m_time_played / m_time_total, current_muted);
		}
#endif		
	}
}

void CAudioPlayerGui::paintLCD()
{
/*
	switch(m_state)
	{
		case CAudioPlayerGui::STOP:
			CVFD::getInstance()->showAudioPlayMode(CVFD::AUDIO_MODE_STOP);
			
#if ENABLE_LCD
			CVFD::getInstance()->showAudioProgress(0, current_muted);
#endif
			break;
		case CAudioPlayerGui::PLAY:
			CVFD::getInstance()->showAudioPlayMode(CVFD::AUDIO_MODE_PLAY);

			// audio-track	
			if (CVFD::getInstance()->is4digits)
				CVFD::getInstance()->LCDshowText(m_selected + 1);
			else
				CVFD::getInstance()->showAudioTrack(m_curr_audiofile.MetaData.artist, m_curr_audiofile.MetaData.title, m_curr_audiofile.MetaData.album);			
					
#if ENABLE_LCD
			if(m_curr_audiofile.FileExtension != CFile::EXTENSION_URL && m_time_total != 0)
				CVFD::getInstance()->showAudioProgress(100 * m_time_played / m_time_total, current_muted);
#endif

			break;
		case CAudioPlayerGui::PAUSE:
			CVFD::getInstance()->showAudioPlayMode(CVFD::AUDIO_MODE_PAUSE);
			if (CVFD::getInstance()->is4digits)
				CVFD::getInstance()->LCDshowText(m_selected + 1);
			else
				CVFD::getInstance()->showAudioTrack(m_curr_audiofile.MetaData.artist, m_curr_audiofile.MetaData.title, m_curr_audiofile.MetaData.album);				
			break;
			
		case CAudioPlayerGui::FF:
			CVFD::getInstance()->showAudioPlayMode(CVFD::AUDIO_MODE_FF);
			if (CVFD::getInstance()->is4digits)
				CVFD::getInstance()->LCDshowText(m_selected + 1);
			else
				CVFD::getInstance()->showAudioTrack(m_curr_audiofile.MetaData.artist, m_curr_audiofile.MetaData.title, m_curr_audiofile.MetaData.album);				
			break;
			
		case CAudioPlayerGui::REV:
			CVFD::getInstance()->showAudioPlayMode(CVFD::AUDIO_MODE_REV);
			if (CVFD::getInstance()->is4digits)
				CVFD::getInstance()->LCDshowText(m_selected + 1);
			else
				CVFD::getInstance()->showAudioTrack(m_curr_audiofile.MetaData.artist, m_curr_audiofile.MetaData.title, m_curr_audiofile.MetaData.album);			
			break;
	}
*/
}

void CAudioPlayerGui::GetMetaData(CAudiofileExt &File)
{
	dprintf(DEBUG_DEBUG, "CAudioPlayerGui::GetMetaData: fileExtension:%d\n", File.FileExtension);
	
	bool ret = 1;

	if (CFile::EXTENSION_URL != File.FileExtension)
		ret = CAudioPlayer::getInstance()->readMetaData(&File, m_state != CAudioPlayerGui::STOP && !g_settings.audioplayer_highprio);

	if (!ret || (File.MetaData.artist.empty() && File.MetaData.title.empty() ))
	{
		//Set from Filename
		std::string tmp = File.Filename.substr(File.Filename.rfind('/') + 1);
		tmp = tmp.substr(0, tmp.length() - 4);	//remove extension (.mp3)
		std::string::size_type i = tmp.rfind(" - ");
		
		if(i != std::string::npos)
		{ 
			// Trennzeichen " - " gefunden
			File.MetaData.artist = tmp.substr(0, i);
			File.MetaData.title = tmp.substr(i + 3);
		}
		else
		{
			i = tmp.rfind('-');
			if(i != std::string::npos)
			{ //Trennzeichen "-"
				File.MetaData.artist = tmp.substr(0, i);
				File.MetaData.title = tmp.substr(i + 1);
			}
			else
				File.MetaData.title = tmp;
		}
		
		File.MetaData.artist = FILESYSTEM_ENCODING_TO_UTF8(std::string(File.MetaData.artist).c_str());
		File.MetaData.title  = FILESYSTEM_ENCODING_TO_UTF8(std::string(File.MetaData.title).c_str());
	}
}

void CAudioPlayerGui::getFileInfoToDisplay(std::string &info, CAudiofileExt &file)
{
	std::string fileInfo;
	std::string artist;
	std::string title;

	if (!m_inetmode) 
	{
		// artist
		artist = "Artist?";

		//
		if ( file.FileExtension == CFile::EXTENSION_M3U || file.FileExtension == CFile::EXTENSION_URL || file.FileExtension == CFile::EXTENSION_PLS)
			artist = "";
		
		// title
		title = "Title?";
	}

	if (!file.MetaData.bitrate)
	{
		GetMetaData(file);
	}

	if (!file.MetaData.artist.empty())
		artist = file.MetaData.artist;

	if (!file.MetaData.title.empty())
		title = file.MetaData.title;

	fileInfo += title;
	if (!title.empty() && !artist.empty()) 
		fileInfo += ", ";
		
	fileInfo += artist;

	if (!file.MetaData.album.empty())
	{
		fileInfo += " (";
		fileInfo += file.MetaData.album;
		fileInfo += ')';
	} 
	
	if (fileInfo.empty())
	{
		fileInfo += "Unknown";
	}
	
	file.firstChar = tolower(fileInfo[0]);
	info += fileInfo;
}

void CAudioPlayerGui::addToPlaylist(CAudiofileExt &file)
{	
	dprintf(DEBUG_NORMAL, "CAudioPlayerGui::add2Playlist: %s\n", file.Filename.c_str());

	m_playlist.push_back(file);
}

void CAudioPlayerGui::clearPlaylist(void)
{
	dprintf(DEBUG_NORMAL, "CAudioPlayerGui::clearPlaylist:\n");

	if (!(m_playlist.empty()))
	{
		m_playlist.clear();
		m_current = -1;
	}
}

void CAudioPlayerGui::removeFromPlaylist(long pos)
{
	dprintf(DEBUG_NORMAL, "CAudioPlayerGui::removeFromPlayList:\n");

	m_playlist.erase(m_playlist.begin() + pos); 
}

void CAudioPlayerGui::savePlaylist()
{
	const char *path;

	// .m3u playlist
	// http://hanna.pyxidis.org/tech/m3u.html

	CFileBrowser browser;
	browser.Multi_Select = false;
	browser.Dir_Mode = true;
	CFileFilter dirFilter;
	dirFilter.addFilter("m3u");
	browser.Filter = &dirFilter;
	
	// select preferred directory if exists
	if (strlen(g_settings.network_nfs_audioplayerdir) != 0)
		path = g_settings.network_nfs_audioplayerdir;
	else
		path = "/";

	// let user select target directory
	this->hide();

	if (browser.exec(path)) 
	{
		// refresh view
		CFile *file = browser.getSelectedFile();
		std::string absPlaylistDir = file->getPath();

		// add a trailing slash if necessary
		if ((absPlaylistDir.empty()) || ((*(absPlaylistDir.rbegin()) != '/')))
		{
			absPlaylistDir += '/';
		}
		absPlaylistDir += file->getFileName();

		const int filenamesize = MAX_INPUT_CHARS + 1;
		char filename[filenamesize + 1] = "";

		if (file->getType() == CFile::FILE_PLAYLIST) 
		{
			// file is playlist so we should ask if we can overwrite it
			std::string name = file->getPath();
			name += '/';
			name += file->getFileName();
			bool overwrite = askToOverwriteFile(name);
			if (!overwrite) 
			{
				return;
			}
			snprintf(filename, name.size(), "%s", name.c_str());
		} 
		else if (file->getType() == CFile::FILE_DIR) 
		{
			// query for filename
			this->hide();
			CStringInputSMS filenameInput(LOCALE_AUDIOPLAYER_PLAYLIST_NAME,
							filename,
							filenamesize - 1,
							LOCALE_AUDIOPLAYER_PLAYLIST_NAME_HINT1,
							LOCALE_AUDIOPLAYER_PLAYLIST_NAME_HINT2,
							"abcdefghijklmnopqrstuvwxyz0123456789-.,:!?/ ");

			filenameInput.exec(NULL, "");
			// refresh view
			std::string name = absPlaylistDir;
			name += '/';
			name += filename;
			name += ".m3u";
			std::ifstream input(name.c_str());

			// test if file exists and query for overwriting it or not
			if (input.is_open()) 
			{
				bool overwrite = askToOverwriteFile(name);
				if (!overwrite) 
				{
					return;
				}
			}
			input.close();
		} 
		else 
		{
			std::cout << "CAudioPlayerGui: neither .m3u nor directory selected, abort" << std::endl;
			return;
		}
		std::string absPlaylistFilename = absPlaylistDir;
		absPlaylistFilename += '/';
		absPlaylistFilename += filename;
		absPlaylistFilename += ".m3u";		
		std::ofstream playlistFile(absPlaylistFilename.c_str());
		std::cout << "CAudioPlayerGui: writing playlist to " << absPlaylistFilename << std::endl;
		
		if (!playlistFile) 
		{
			// an error occured
			const int msgsize = 255;
			char msg[msgsize] = "";
			snprintf(msg,
				msgsize,
				"%s\n%s",
				g_Locale->getText(LOCALE_AUDIOPLAYER_PLAYLIST_FILEERROR_MSG),
				absPlaylistFilename.c_str());

			MessageBox(LOCALE_MESSAGEBOX_ERROR, msg, CMessageBox::mbrCancel, CMessageBox::mbCancel, NEUTRINO_ICON_ERROR);
			// refresh view
			std::cout << "CAudioPlayerGui: could not create play list file " 
			<< absPlaylistFilename << std::endl;
			return;
		}
		// writing .m3u file
		playlistFile << "#EXTM3U" << std::endl;

		CAudioPlayList::const_iterator it;
		for (it = m_playlist.begin();it!=m_playlist.end();it++) 
		{
			playlistFile << "#EXTINF:" << it->MetaData.total_time << ","
			<< it->MetaData.artist << " - " << it->MetaData.title << std::endl;
			if (m_inetmode)
				playlistFile << it->Filename << std::endl;
			else
				playlistFile << absPath2Rel(absPlaylistDir, it->Filename) << std::endl;
		}
		playlistFile.close();
	} 
}

bool CAudioPlayerGui::askToOverwriteFile(const std::string& filename) 
{
	char msg[filename.length() + 127];
	
	snprintf(msg, filename.length() + 126, "%s\n%s", g_Locale->getText(LOCALE_AUDIOPLAYER_PLAYLIST_FILEOVERWRITE_MSG), filename.c_str());
	bool res = (MessageBox(LOCALE_AUDIOPLAYER_PLAYLIST_FILEOVERWRITE_TITLE, msg, CMessageBox::mbrYes, CMessageBox::mbYes | CMessageBox::mbNo) == CMessageBox::mbrYes);
	//this->paint();
	return res;
}

std::string CAudioPlayerGui::absPath2Rel(const std::string& fromDir, const std::string& absFilename) 
{
	std::string res = "";

	int length = fromDir.length() < absFilename.length() ? fromDir.length() : absFilename.length();
	int lastSlash = 0;
	// find common prefix for both paths
	// fromDir:     /foo/bar/angle/1          (length: 16)
	// absFilename: /foo/bar/devil/2/fire.mp3 (length: 19)
	// -> /foo/bar/ is prefix, lastSlash will be 8
	for (int i = 0; i < length; i++) 
	{
		if (fromDir[i] == absFilename[i]) 
		{
			if (fromDir[i] == '/') 
			{
				lastSlash = i;
			}
		} 
		else 
		{
			break;
		}
	}
	// cut common prefix
	std::string relFilepath = absFilename.substr(lastSlash + 1, absFilename.length() - lastSlash + 1);
	// relFilepath is now devil/2/fire.mp3

	// First slash is not removed because we have to go up each directory.
	// Since the slashes are counted later we make sure for each directory one slash is present
	std::string relFromDir = fromDir.substr(lastSlash, fromDir.length() - lastSlash);
	// relFromDir is now /angle/1

	// go up as many directories as neccessary
	for (unsigned int i = 0; i < relFromDir.size(); i++)
	{
		if (relFromDir[i] == '/') 
		{
			res = res + "../";
		}
	}

	res = res + relFilepath;
	return res;
}


