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
#include <driver/screen_max.h>
#include <driver/color.h>

#include <daemonc/remotecontrol.h>

#include <gui/eventlist.h>
#include <gui/infoviewer.h>
#include <gui/nfs.h>

#include <gui/widget/widget_helpers.h>
#include <gui/widget/icons.h>
#include <gui/widget/menue.h>
#include <gui/widget/messagebox.h>
#include <gui/widget/hintbox.h>
#include <gui/widget/stringinput.h>
#include <gui/widget/stringinput_ext.h>
#include <gui/widget/progresswindow.h>
#include <gui/widget/helpbox.h>

#include <gui/audioplayer_setup.h>

#include <system/settings.h>
#include <system/helpers.h>

#include <xmlinterface.h>

#include <algorithm>
#include <sys/time.h>
#include <fstream>
#include <iostream>

#include <system/debug.h>


extern int current_muted;

#define AUDIOPLAYER_START_SCRIPT 			CONFIGDIR "/audioplayer.start"
#define AUDIOPLAYER_END_SCRIPT 				CONFIGDIR "/audioplayer.end"

CAudioPlayerGui::CAudioPlayerGui()
{
	m_frameBuffer = CFrameBuffer::getInstance();

	Init();

	CFileHelpers::getInstance()->createDir("/tmp/audioplayer", 0755);
}

void CAudioPlayerGui::Init(void)
{
	m_inetmode = false;

	m_current = 0;
	m_metainfo.clear();

	infoPainted = false;

	repeatMode = REPEAT_NONE;

	// playInfo
	//
	cFrameBox.iWidth = m_frameBuffer->getScreenWidth(true) - 20; 
	if((g_settings.screen_EndX - g_settings.screen_StartX) < cFrameBox.iWidth)
		cFrameBox.iWidth = (g_settings.screen_EndX - g_settings.screen_StartX) - 20;
	cFrameBox.iHeight = 80;
	cFrameBox.iX = (((g_settings.screen_EndX - g_settings.screen_StartX) - cFrameBox.iWidth)/ 2) + g_settings.screen_StartX;
	cFrameBox.iY = g_settings.screen_EndY - 10 - cFrameBox.iHeight;

	// timeBox
	timeBox.iWidth = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth("00:00 / 00:00") + 4;
	timeBox.iHeight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight() + 4;
	timeBox.iX = g_settings.screen_EndX - 10 - timeBox.iWidth;
	timeBox.iY = g_settings.screen_StartY + 10;
}

CAudioPlayerGui::~CAudioPlayerGui()
{
	m_playlist.clear();

	CFileHelpers::getInstance()->removeDir("/tmp/audioplayer");
}

int CAudioPlayerGui::exec(CMenuTarget * parent, const std::string &actionKey)
{
	dprintf(DEBUG_NORMAL, "CAudioPlayerGui::exec: actionKey:%s\n", actionKey.c_str());

	CAudioPlayer::getInstance()->init();
	
	m_state = CAudioPlayerGui::STOP;

	// playInfo
	//
	cFrameBox.iWidth = m_frameBuffer->getScreenWidth(true) - 20; 
	if((g_settings.screen_EndX - g_settings.screen_StartX) < cFrameBox.iWidth)
		cFrameBox.iWidth = (g_settings.screen_EndX - g_settings.screen_StartX) - 20;
	cFrameBox.iHeight = 80;
	cFrameBox.iX = (((g_settings.screen_EndX - g_settings.screen_StartX) - cFrameBox.iWidth)/ 2) + g_settings.screen_StartX;
	cFrameBox.iY = g_settings.screen_EndY - 10 - cFrameBox.iHeight;

	// timeBox
	timeBox.iWidth = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth("00:00 / 00:00") + 4;
	timeBox.iHeight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight() + 4;
	timeBox.iX = g_settings.screen_EndX - 10 - timeBox.iWidth;
	timeBox.iY = g_settings.screen_StartY + 10;
	
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
	CNeutrinoApp::getInstance()->handleMsg(NeutrinoMessages::CHANGEMODE, NeutrinoMessages::mode_audio );
	
	// remember last mode
	m_LastMode = (CNeutrinoApp::getInstance()->getLastMode() | NeutrinoMessages::norezap);
	
	// stop playback
	CNeutrinoApp::getInstance()->lockPlayBack();

	//start AP start-script
	puts("[audioplayer.cpp] executing " AUDIOPLAYER_START_SCRIPT "."); 
	if (system(AUDIOPLAYER_START_SCRIPT) != 0) 
		perror("Datei " AUDIOPLAYER_START_SCRIPT " fehlt.Bitte erstellen, wenn gebraucht.\nFile " AUDIOPLAYER_START_SCRIPT " not found. Please create if needed.\n");

	//show
	playFile();

	//restore previous background
	if (usedBackground)
	{
		m_frameBuffer->restoreBackgroundImage();
		m_frameBuffer->useBackground(usedBackground);
	}

	// hide background image
	m_frameBuffer->paintBackground();
	m_frameBuffer->blit();		

	// start playback
	CNeutrinoApp::getInstance()->unlockPlayBack();

	//set last saved mode
	CNeutrinoApp::getInstance()->handleMsg( NeutrinoMessages::CHANGEMODE, m_LastMode );
	
	//show infobar
	g_RCInput->postMsg( NeutrinoMessages::SHOW_INFOBAR, 0 );

	if(!m_playlist.empty())
		m_playlist.clear();

	//always repaint
	return RETURN_EXIT;
}

void CAudioPlayerGui::playFile()
{
	dprintf(DEBUG_NORMAL, "CAudioPlayerGui::playFile\n");

	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	CVFD::getInstance()->setMode(CVFD::MODE_AUDIO, g_Locale->getText(m_inetmode? LOCALE_INETRADIO_NAME : LOCALE_AUDIOPLAYER_HEAD));
		
	paintLCD();		

	bool loop = true;
	exit_pressed = false;
	
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
			// last track
			if(m_current == ((int)m_playlist.size() - 1) && repeatMode != REPEAT_ALL)
			{
				loop = false;	
			}

			// playNext or repeat
			if(m_playlist[m_current].FileExtension != CFile::EXTENSION_URL)
				playNext();
		}

		if(!m_inetmode)
			updateTimes(true);

		if(!infoPainted)
			paintInfo(m_playlist[m_current]);
		
		g_RCInput->getMsg(&msg, &data, 10); // 1 sec timeout to update play/stop state display

		if (msg == RC_home)
		{ 
			exit_pressed = true;
			loop = false;
		}
		else if (msg == RC_left || msg == RC_prev)
		{
			playPrev();
		}
		else if (msg == RC_right || msg == RC_next)
		{
			playNext();
		}
		else if (msg == RC_stop)
		{
			stop();
		}
		else if( msg == RC_pause)
		{
			pause();
		}
		else if(msg == RC_play)
		{
			play(m_current);
		}
		else if(msg == RC_loop)
		{
			if (!m_inetmode)
			{
				if(repeatMode == CAudioPlayerGui::REPEAT_NONE)
				{
					repeatMode = CAudioPlayerGui::REPEAT_TRACK;
					printf("\nCAudioPlayerGui::PlayFile(): repeat track mode on\n");
				}
				else if(repeatMode == CAudioPlayerGui::REPEAT_TRACK)
				{
					repeatMode = CAudioPlayerGui::REPEAT_ALL;
					printf("\nCAudioPlayerGui::PlayFile(): repeat all mode on\n");
				}
				else if(repeatMode == CAudioPlayerGui::REPEAT_ALL)
				{
					repeatMode = CAudioPlayerGui::REPEAT_NONE;
					printf("\nCAudioPlayerGui::PlayFile(): repeat mode off\n");
				}

				paintInfo(m_playlist[m_current]);
			}
		}
		else if(msg == RC_forward)
		{
			ff(1);
		}
		else if(msg == RC_rewind)
		{
			rev(1);
		}
		else if(msg == RC_red)
		{
			//if(m_state == CAudioPlayerGui::STOP)
			{
				if (!m_playlist.empty()) 
				{
					savePlaylist();

					CVFD::getInstance()->setMode(CVFD::MODE_AUDIO, g_Locale->getText(m_inetmode? LOCALE_INETRADIO_NAME : LOCALE_AUDIOPLAYER_HEAD));
						
					paintLCD();
				}
			}
		} 
		else if(msg == RC_green)
		{
			if(m_state != CAudioPlayerGui::STOP)
			{
				// is no stream, so we do not have to test for this case
				int seconds = 0;
				CIntInput secondsInput(LOCALE_AUDIOPLAYER_JUMP_DIALOG_TITLE,
							seconds,
							5,
							LOCALE_AUDIOPLAYER_JUMP_DIALOG_HINT1,
							LOCALE_AUDIOPLAYER_JUMP_DIALOG_HINT2);
							
				int res = secondsInput.exec(NULL, "");
					
				if (seconds != 0 && res != RETURN_EXIT_ALL)
					rev(seconds);
			}
		}
		else if(msg == RC_yellow)
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
					
				if (seconds != 0 && res != RETURN_EXIT_ALL)
					ff(seconds);
			}
		}
		else if(msg == RC_blue)
		{
			shufflePlaylist();
		}
		else if( ((msg == RC_setup) || (msg == RC_vfdmenu)))
		{
			//hide();
			CAudioPlayerSettings * audioPlayerSettingsMenu = new CAudioPlayerSettings();
			audioPlayerSettingsMenu->exec(NULL, "");
			delete audioPlayerSettingsMenu;
			audioPlayerSettingsMenu = NULL;					
		}
		else if(msg == RC_info)
		{
			showHelp();
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
}

void CAudioPlayerGui::hide()
{
	// infos
	m_frameBuffer->paintBackgroundBoxRel(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight);

	infoPainted = false;

	// times
	m_frameBuffer->paintBackgroundBoxRel(timeBox.iX, timeBox.iY, timeBox.iWidth, timeBox.iHeight);

	m_frameBuffer->blit();
}

void CAudioPlayerGui::paintFanArt(CAudiofile& File)
{
	m_frameBuffer->loadBackgroundPic("mp3.jpg");

	if (!File.MetaData.cover.empty())
	{
		if (file_exists(File.MetaData.cover.c_str()))
		{
			//m_frameBuffer->loadBackgroundPic("/tmp/cover.jpg");
			m_frameBuffer->displayImage(File.MetaData.cover, m_frameBuffer->getScreenX(), m_frameBuffer->getScreenY(), m_frameBuffer->getScreenWidth(), m_frameBuffer->getScreenHeight());
		}
		else
		{
			m_frameBuffer->loadBackgroundPic("mp3.jpg");
		}
				
	}
	else
	{
		m_frameBuffer->loadBackgroundPic("mp3.jpg");
	}

	m_frameBuffer->blit();	
}

void CAudioPlayerGui::paintInfo(CAudiofile& File)
{
	// title info box 
	// shadow
	m_frameBuffer->paintBoxRel(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight, COL_MENUCONTENT_PLUS_6);//FIXME: gradient
	
	// box	
	m_frameBuffer->paintBoxRel(cFrameBox.iX + 1, cFrameBox.iY + 1 , cFrameBox.iWidth - 2, cFrameBox.iHeight - 2, COL_INFOBAR_PLUS_0, NO_RADIUS, CORNER_NONE, g_settings.infobar_gradient); 

	// first line (Track number)
	std::string tmp;
	if (m_inetmode) 
	{
		tmp = File.MetaData.album;
	} 
	else 
	{
		char sNr[20];
		sprintf(sNr, ": %2d", m_current + 1);
		tmp = g_Locale->getText(LOCALE_AUDIOPLAYER_PLAYING);
		tmp += sNr ;
	}

	int w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp, true); // UTF-8
	int xstart = (cFrameBox.iWidth - w) / 2;
	
	if(xstart < (BORDER_LEFT + 1*cFrameBox.iHeight + ICON_OFFSET))
		xstart = BORDER_LEFT + 1*cFrameBox.iHeight + ICON_OFFSET;

	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(cFrameBox.iX + xstart, cFrameBox.iY + 4 + cFrameBox.iHeight/3, cFrameBox.iWidth - 20, tmp, COL_INFOBAR); // UTF-8

	// second line (Artist/Title)
	if (File.MetaData.title.empty())
		tmp = File.MetaData.artist;
	else if (File.MetaData.artist.empty())
		tmp = File.MetaData.title;
	else 
	{
		tmp = File.MetaData.title;
		tmp += " / ";
		tmp += File.MetaData.artist;
	}

	w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp, true); // UTF-8
	xstart = (cFrameBox.iWidth - w)/2;
	if(xstart < (BORDER_LEFT + 1*cFrameBox.iHeight + ICON_OFFSET))
		xstart = BORDER_LEFT + 1*cFrameBox.iHeight + ICON_OFFSET;
	
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(cFrameBox.iX + xstart, cFrameBox.iY + 2 + cFrameBox.iHeight/3 + 2 + cFrameBox.iHeight/3, cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT - 1*cFrameBox.iHeight - ICON_OFFSET, tmp, COL_INFOBAR); // UTF-8		

	//playstate
	int icon_w, icon_h;
	const char* icon = NEUTRINO_ICON_PLAY;
		
	switch(m_state)
	{
		case CAudioPlayerGui::PAUSE: icon = NEUTRINO_ICON_PAUSE; break;
		case CAudioPlayerGui::PLAY: icon = NEUTRINO_ICON_PLAY; break;
		case CAudioPlayerGui::REV: icon = NEUTRINO_ICON_REW; break;
		case CAudioPlayerGui::FF: icon = NEUTRINO_ICON_FF; break;
		case CAudioPlayerGui::STOP: icon = NEUTRINO_ICON_STOP_SMALL; break;
	}

	switch(repeatMode)
	{
		case CAudioPlayerGui::REPEAT_TRACK: if(m_state == CAudioPlayerGui::PLAY) icon = NEUTRINO_ICON_REPEAT_TRACK; break;
		case CAudioPlayerGui::REPEAT_ALL: if(m_state == CAudioPlayerGui::PLAY)icon = NEUTRINO_ICON_REPEAT_ALL; break;
	}
	
	m_frameBuffer->getIconSize(icon, &icon_w, &icon_h);
	m_frameBuffer->paintIcon(icon, cFrameBox.iX /*+ cFrameBox.iHeight*/ + ICON_OFFSET, cFrameBox.iY + (cFrameBox.iHeight - icon_h)/2);
		
	//
	m_metainfo.clear();
	m_time_total = 0;
	m_time_played = 0;

	updateMetaData();

	// third line
	if(updateMeta || updateScreen)
	{
		xstart = ((cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT - g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getRenderWidth(m_metainfo))/2) + BORDER_LEFT;

		g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(cFrameBox.iX + xstart, cFrameBox.iY + cFrameBox.iHeight - g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight()/2, cFrameBox.iWidth- 2*xstart, m_metainfo, COL_INFOBAR);
	}

	infoPainted = true;
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
			play(m_playlist.size() - 1);
		else
			play(0);

		result = true;
	}

	return(result);
}

void CAudioPlayerGui::stop()
{
	m_state = CAudioPlayerGui::STOP;	

	if(CAudioPlayer::getInstance()->getState() != CBaseDec::STOP)
		CAudioPlayer::getInstance()->stop();

	paintInfo(m_playlist[m_current]);
	paintLCD();
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

	paintInfo(m_playlist[m_current]);	
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
	
	paintInfo(m_playlist[m_current]);
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

	paintInfo(m_playlist[m_current]);
	paintLCD();
}

void CAudioPlayerGui::play(unsigned int pos)
{
	dprintf(DEBUG_NORMAL, "CAudioPlayerGui::play\n");

	if(!m_playlist.size())
		return;

	m_current = pos;

	// stop if playing
	if(CAudioPlayer::getInstance()->getState() != CBaseDec::STOP)
		CAudioPlayer::getInstance()->stop();

	// play
	CAudioPlayer::getInstance()->play(&m_playlist[pos], g_settings.audioplayer_highprio == 1);

	m_state = CAudioPlayerGui::PLAY;
	
	GetMetaData(m_playlist[pos]);
	paintFanArt(m_playlist[pos]);
	paintInfo(m_playlist[pos]);
	paintLCD();
}

int CAudioPlayerGui::getNext()
{
	int ret = -1;

	if(repeatMode == CAudioPlayerGui::REPEAT_TRACK)
		ret = m_current;
	else
		ret = m_current + 1;

	if(m_playlist.empty())
		return -1;
	
	if((unsigned)ret >= m_playlist.size()) 
	{
		if(repeatMode == CAudioPlayerGui::REPEAT_ALL)
		{
			ret = 0;
		}
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

		if (!meta.artist.empty()  && meta.artist != m_playlist[m_current].MetaData.artist)
		{
			m_playlist[m_current].MetaData.artist = meta.artist;
			updateScreen = true;
			updateLcd = true;
		}
		
		if (!meta.title.empty() && meta.title != m_playlist[m_current].MetaData.title)
		{
			m_playlist[m_current].MetaData.title = meta.title;
			updateScreen = true;
			updateLcd = true;
		}
		
		if (!meta.sc_station.empty()  && meta.sc_station != m_playlist[m_current].MetaData.album)
		{
			m_playlist[m_current].MetaData.album = meta.sc_station;
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

		// shadow
		m_frameBuffer->paintBoxRel(timeBox.iX, timeBox.iY, timeBox.iWidth, timeBox.iHeight, COL_MENUCONTENT_PLUS_6);//FIXME: gradient
		
		// box
	m_frameBuffer->paintBoxRel(timeBox.iX + 1, timeBox.iY + 1, timeBox.iWidth - 2, timeBox.iHeight - 2, COL_INFOBAR_PLUS_0/*, NO_RADIUS, CORNER_NONE, g_settings.infobar_gradient*/); //FIXME:gradient

		if (m_time_total != CAudioPlayer::getInstance()->getTimeTotal())
		{
			m_time_total = CAudioPlayer::getInstance()->getTimeTotal();
			if (m_playlist[m_current].MetaData.total_time != CAudioPlayer::getInstance()->getTimeTotal())
			{
				m_playlist[m_current].MetaData.total_time = CAudioPlayer::getInstance()->getTimeTotal();
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
		
		// total time
		char tot_time[11];
		snprintf(tot_time, 10, " / %ld:%02ld", m_time_total / 60, m_time_total % 60);

		//
		char tmp_time[8];
		snprintf(tmp_time, 7, "%ld:00", m_time_total / 60);

		//
		char play_time[8];
		snprintf(play_time, 7, "%ld:%02ld", m_time_played / 60, m_time_played % 60);

		int w1 = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tot_time);
		int w2 = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp_time);

		if (updateTotal)
		{
			if(m_time_total > 0)
				g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(timeBox.iX + timeBox.iWidth - timeBox.iWidth/2, timeBox.iY + (timeBox.iHeight - g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight(), w1, tot_time, COL_INFOBAR);
		}
			
		if (updatePlayed || (m_state == CAudioPlayerGui::PAUSE))
		{
			struct timeval tv;
			gettimeofday(&tv, NULL);

			if ((m_state != CAudioPlayerGui::PAUSE) || (tv.tv_sec & 1))
			{
				g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(timeBox.iX + timeBox.iWidth/2 - w2 - 2, timeBox.iY + (timeBox.iHeight - g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight(), w2, play_time, COL_INFOBAR);
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
				CVFD::getInstance()->LCDshowText(m_current + 1);
			else
				CVFD::getInstance()->showAudioTrack(m_playlist[m_current].MetaData.artist, m_playlist[m_current].MetaData.title, m_playlist[m_current].MetaData.album);			
					
#if ENABLE_LCD
			if(m_playlist[m_current].FileExtension != CFile::EXTENSION_URL && m_time_total != 0)
				CVFD::getInstance()->showAudioProgress(100 * m_time_played / m_time_total, current_muted);
#endif

			break;
		case CAudioPlayerGui::PAUSE:
			CVFD::getInstance()->showAudioPlayMode(CVFD::AUDIO_MODE_PAUSE);
			if (CVFD::getInstance()->is4digits)
				CVFD::getInstance()->LCDshowText(m_current + 1);
			else
				CVFD::getInstance()->showAudioTrack(m_playlist[m_current].MetaData.artist, m_playlist[m_current].MetaData.title, m_playlist[m_current].MetaData.album);				
			break;
			
		case CAudioPlayerGui::FF:
			CVFD::getInstance()->showAudioPlayMode(CVFD::AUDIO_MODE_FF);
			if (CVFD::getInstance()->is4digits)
				CVFD::getInstance()->LCDshowText(m_current + 1);
			else
				CVFD::getInstance()->showAudioTrack(m_playlist[m_current].MetaData.artist, m_playlist[m_current].MetaData.title, m_playlist[m_current].MetaData.album);				
			break;
			
		case CAudioPlayerGui::REV:
			CVFD::getInstance()->showAudioPlayMode(CVFD::AUDIO_MODE_REV);
			if (CVFD::getInstance()->is4digits)
				CVFD::getInstance()->LCDshowText(m_current + 1);
			else
				CVFD::getInstance()->showAudioTrack(m_playlist[m_current].MetaData.artist, m_playlist[m_current].MetaData.title, m_playlist[m_current].MetaData.album);			
			break;
	}

	switch (repeatMode)
	{
		case CAudioPlayerGui::REPEAT_TRACK:
		case CAudioPlayerGui::REPEAT_ALL:
			break;
	}
}

void CAudioPlayerGui::GetMetaData(CAudiofile& File)
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

void CAudioPlayerGui::getFileInfoToDisplay(std::string &info, CAudiofile& file)
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
	
	info += fileInfo;
}

void CAudioPlayerGui::addToPlaylist(CAudiofile& file)
{	
	dprintf(DEBUG_DEBUG, "CAudioPlayerGui::add2Playlist: %s\n", file.Filename.c_str());

	m_playlist.push_back(file);
}

void CAudioPlayerGui::addToPlaylist(const CFile& file)
{
	dprintf(DEBUG_DEBUG, "CAudioPlayerGui::add2Playlist: %s\n", file.Name.c_str());

	CAudiofile audiofile(file.Name, file.getExtension());
				
	m_playlist.push_back(audiofile);
}

void CAudioPlayerGui::addToPlaylist(const char* fileName)
{
	dprintf(DEBUG_DEBUG, "CAudioPlayerGui::add2Playlist: %s\n", fileName);

	CFile file;
	file.Name = fileName;

	CAudiofile audiofile(file.Name, file.getExtension());
				
	m_playlist.push_back(audiofile);
}

void CAudioPlayerGui::clearPlaylist(void)
{
	dprintf(DEBUG_NORMAL, "CAudioPlayerGui::clearPlaylist:\n");

	if (!(m_playlist.empty()))
	{
		m_playlist.clear();
		m_current = 0;
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

			MessageBox(LOCALE_MESSAGEBOX_ERROR, msg, mbrCancel, mbCancel, NEUTRINO_ICON_ERROR);
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
	bool res = (MessageBox(LOCALE_AUDIOPLAYER_PLAYLIST_FILEOVERWRITE_TITLE, msg, mbrYes, mbYes | mbNo) == mbrYes);
	
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

bool CAudioPlayerGui::shufflePlaylist(void)
{
	dprintf(DEBUG_NORMAL, "CAudioPlayerGui::shufflePlaylist\n");
	
	RandomNumber rnd;
	bool result = false;
	
	if (!(m_playlist.empty()))
	{
		if (m_current > 0)
		{
			std::swap(m_playlist[0], m_playlist[m_current]);
			m_current = 0;
		}

		std::random_shuffle((m_current != 0) ? m_playlist.begin() : m_playlist.begin() + 1, m_playlist.end(), rnd);

		m_current = 0;

		result = true;
	}
	
	return(result);
}

void CAudioPlayerGui::showHelp()
{
	CHelpBox helpbox;

	helpbox.addLine(NEUTRINO_ICON_BUTTON_RED, "save playlist");
	helpbox.addLine(NEUTRINO_ICON_BUTTON_GREEN, "jump prev");
	helpbox.addLine(NEUTRINO_ICON_BUTTON_YELLOW, "jump ff");
	helpbox.addLine(NEUTRINO_ICON_BUTTON_BLUE, "shuffle playlist");
	helpbox.addLine(NEUTRINO_ICON_BUTTON_SETUP, "Audio Player Setup");

	hide();
	helpbox.show(LOCALE_MESSAGEBOX_INFO);
}




