/*
  Neutrino-GUI  -   DBoxII-Project
  
  $Id: audioplayer.cpp 2015/07/18 mohousch Exp $

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

#include <system/settings.h>
#include <system/helpers.h>

#include <xmlinterface.h>

#include <algorithm>
#include <sys/time.h>
#include <fstream>
#include <iostream>

#include <system/debug.h>

#include <gui/webtv.h>

#include <curl/curl.h>
#include <curl/easy.h>


extern int current_muted;
extern CWebTV * webtv;

#define AUDIOPLAYERGUI_SMSKEY_TIMEOUT 1000
#define SHOW_FILE_LOAD_LIMIT 50

// check if files to be added are already in the playlist
#define AUDIOPLAYER_CHECK_FOR_DUPLICATES
#define AUDIOPLAYER_START_SCRIPT 			CONFIGDIR "/audioplayer.start"
#define AUDIOPLAYER_END_SCRIPT 				CONFIGDIR "/audioplayer.end"
#define DEFAULT_RADIOSTATIONS_XMLFILE 			CONFIGDIR "/radio-stations.xml"

const long int GET_PLAYLIST_TIMEOUT = 10;
const char RADIO_STATION_XML_FILE[] = {DEFAULT_RADIOSTATIONS_XMLFILE};

const std::string icecasturl = "http://dir.xiph.org/yp.xml";
const long int GET_ICECAST_TIMEOUT = 90; 		// list is about 500kB!


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

CAudioPlayerGui::CAudioPlayerGui(bool inetmode)
{
	m_frameBuffer = CFrameBuffer::getInstance();
	m_visible = false;
	m_inetmode = inetmode;
	
	info_visible = false;

	Init();
}

void CAudioPlayerGui::Init(void)
{
	stimer = 0;
	m_selected = 0;
	m_metainfo.clear();
	isURL = false;

	m_select_title_by_name = g_settings.audioplayer_select_title_by_name == 1;

	if(m_inetmode)
		m_Path = CONFIGDIR "/";
	else
	{
		if(strlen(g_settings.network_nfs_audioplayerdir) != 0)
			m_Path = g_settings.network_nfs_audioplayerdir;
		else
			m_Path = "/";
	}

	audiofilefilter.Clear();
	
	if (m_inetmode) 
	{
		audiofilefilter.addFilter("url");
		audiofilefilter.addFilter("xml");
		audiofilefilter.addFilter("m3u");
		audiofilefilter.addFilter("pls");
	} 
	else 
	{
		audiofilefilter.addFilter("cdr");
		audiofilefilter.addFilter("mp3");
		audiofilefilter.addFilter("m2a");
		audiofilefilter.addFilter("mpa");
		audiofilefilter.addFilter("mp2");
		audiofilefilter.addFilter("ogg");
		audiofilefilter.addFilter("wav");
		audiofilefilter.addFilter("flac");
		audiofilefilter.addFilter("aac");
		audiofilefilter.addFilter("dts");
		audiofilefilter.addFilter("m4a");
	}
	
	m_SMSKeyInput.setTimeout(AUDIOPLAYERGUI_SMSKEY_TIMEOUT);
}

CAudioPlayerGui::~CAudioPlayerGui()
{
	m_playlist.clear();
	m_radiolist.clear();
	m_filelist.clear();
	m_title2Pos.clear();
}

int CAudioPlayerGui::exec(CMenuTarget * parent, const std::string &actionKey)
{
	dprintf(DEBUG_NORMAL, "CAudioPlayerGui::exec: actionKey:%s\n", actionKey.c_str());

	CAudioPlayer::getInstance()->init();
	
	m_state = CAudioPlayerGui::STOP;

	if (m_select_title_by_name != (g_settings.audioplayer_select_title_by_name == 1))
	{
		if ((g_settings.audioplayer_select_title_by_name == 1) && m_playlistHasChanged)
		{
			buildSearchTree();
		}
		m_select_title_by_name = g_settings.audioplayer_select_title_by_name;
	}

	if (m_playlist.empty())
		m_current = -1;
	else
		m_current = 0;

	m_selected = 0;

	m_width = m_frameBuffer->getScreenWidth(true) - 10; 
	
	if((g_settings.screen_EndX - g_settings.screen_StartX) < m_width+ConnectLineBox_Width)
		m_width = (g_settings.screen_EndX - g_settings.screen_StartX) - ConnectLineBox_Width - 5;
	
	m_height = m_frameBuffer->getScreenHeight(true) - 10;
	
	if((g_settings.screen_EndY - g_settings.screen_StartY) < m_height)
		m_height = (g_settings.screen_EndY - g_settings.screen_StartY - 5);
	
	m_sheight = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight();
	// 
	m_frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_OKAY, &icon_foot_w, &icon_foot_h);
	m_buttonHeight = 2*(std::max(g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight(), icon_foot_h)) + 10;
	m_theight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	m_fheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	m_title_height = m_fheight*2 + 20 + m_sheight + 4;
	m_info_height = m_fheight*2;
	m_listmaxshow = (m_height - m_title_height - m_theight - m_buttonHeight - m_info_height) / (m_fheight);
	m_height = m_title_height + m_theight + m_listmaxshow*m_fheight + m_buttonHeight + m_info_height; // recalc height

	m_x = (((g_settings.screen_EndX - g_settings.screen_StartX) - (m_width + ConnectLineBox_Width)) / 2) + g_settings.screen_StartX + ConnectLineBox_Width;
	m_y = (((g_settings.screen_EndY - g_settings.screen_StartY) - m_height)/ 2) + g_settings.screen_StartY;
	
	m_idletime = time(NULL);
	
	if(actionKey == "urlplayback")
	{
		hide_playlist = true;
		isURL = true;
	}
	else
		hide_playlist = g_settings.audioplayer_hide_playlist;

	if(parent)
		parent->hide(); 
	
	bool usedBackground = m_frameBuffer->getuseBackground();
	if (usedBackground)
		m_frameBuffer->saveBackgroundImage();
	
	//show audio background pic	
	m_frameBuffer->loadBackgroundPic("mp3.jpg");
	m_frameBuffer->blit();	
	
	// tell neutrino we're in audio mode
	CNeutrinoApp::getInstance()->handleMsg(NeutrinoMessages::CHANGEMODE , NeutrinoMessages::mode_audio );
	
	// remember last mode
	m_LastMode = (CNeutrinoApp::getInstance()->getLastMode());
	
	CNeutrinoApp::getInstance()->StopSubtitles();
	
	// stop playback
	if(CNeutrinoApp::getInstance()->getLastMode() == NeutrinoMessages::mode_iptv)
	{
		if(webtv)
			webtv->stopPlayBack();
	}
	else
	{
		// stop/lock live playback	
		g_Zapit->lockPlayBack();
		
		//pause epg scanning
		g_Sectionsd->setPauseScanning(true);
	}	

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
	if(CNeutrinoApp::getInstance()->getLastMode() == NeutrinoMessages::mode_iptv)
	{
		if(webtv)
			webtv->startPlayBack(webtv->getTunedChannel());
	}
	else
	{
		// unlock playback	
		g_Zapit->unlockPlayBack();	
		
		//start epg scanning
		g_Sectionsd->setPauseScanning(false);
	}
	
	CNeutrinoApp::getInstance()->StartSubtitles();

	//set last saved mode
	CNeutrinoApp::getInstance()->handleMsg( NeutrinoMessages::CHANGEMODE, m_LastMode );
	
	//show infobar
	g_RCInput->postMsg( NeutrinoMessages::SHOW_INFOBAR, 0 );
	
	// remove mp3 cover
	if(!access("/tmp/cover.jpg", F_OK))
		remove("/tmp/cover.jpg");

	//always repaint
	return menu_return::RETURN_EXIT_ALL;
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
	bool update = true;
	bool clear_before_update = false;
	m_key_level = 0;
	
	if(isURL)
		play(m_selected);

	// control loop
	while(loop)
	{
		if(!hide_playlist)
			updateMetaData();
		
		updateTimes();

		if(CNeutrinoApp::getInstance()->getMode() != NeutrinoMessages::mode_audio)
		{
			// stop if mode was changed in another thread
			loop = false;
		}
		
		if ((m_state != CAudioPlayerGui::STOP) && (CAudioPlayer::getInstance()->getState() == CBaseDec::STOP) && (!m_playlist.empty()))
		{
			if(isURL && m_current == m_playlist.size())
			{
				loop = false;	
			}

			if(m_curr_audiofile.FileExtension != CFile::EXTENSION_URL)
				playNext();
		}

		if (update)
		{
			if(clear_before_update)
			{
				hide();
				clear_before_update = false;
			}
			
			update = false;
			paint();
		}
		
		g_RCInput->getMsg(&msg, &data, 10); // 1 sec timeout to update play/stop state display

		if(hide_playlist && m_state == CAudioPlayerGui::PLAY)
		{
			hide();
					
			// paint infos
			paintInfo();
		}
		else
			paint();

		if( msg == CRCInput::RC_mode )
		{
			if(m_inetmode) 
			{
				m_inetmode = false;
				m_radiolist = m_playlist;
				m_playlist = m_filelist;
			} 
			else 
			{
				m_inetmode = true;
				m_filelist = m_playlist;
				m_playlist = m_radiolist;
			}

			Init();
			clear_before_update = true;
			update = true;
		}
		else if (msg == CRCInput::RC_home)
		{ 
			if (m_state != CAudioPlayerGui::STOP)
			{
				if(isURL)
					loop = false;
				else
					stop();
			}
			else
				loop = false;
		}
		else if (msg == CRCInput::RC_left)
		{
			if (m_key_level == 1)
			{
				playPrev();
			}
			else
			{
				if (!m_playlist.empty())
				{
					if (m_selected < m_listmaxshow)
					{
						m_selected = m_playlist.size() - 1;
					}					
					else
						m_selected -= m_listmaxshow;
					
					m_liststart = (m_selected / m_listmaxshow) * m_listmaxshow;
					update = true;
				}
			}

		}
		else if (msg == CRCInput::RC_right)
		{
			if (m_key_level == 1)
			{
				playNext();
			}
			else
			{
				if (!m_playlist.empty())
				{
					m_selected += m_listmaxshow;
					if (m_selected >= m_playlist.size()) 
					{
						if (((m_playlist.size() / m_listmaxshow) + 1) * m_listmaxshow == m_playlist.size() + m_listmaxshow)
							m_selected = 0;
						else
							m_selected = m_selected < (((m_playlist.size() / m_listmaxshow) + 1) * m_listmaxshow) ? (m_playlist.size()-1) : 0;
					}
					m_liststart = (m_selected / m_listmaxshow) * m_listmaxshow;
					update = true;
				}
			}
		}
		else if( ((msg &~ CRCInput::RC_Repeat) == CRCInput::RC_up || (msg &~ CRCInput::RC_Repeat) == CRCInput::RC_page_up) && !isURL)
		{
			if(hide_playlist)
			{
				paintInfo();
				paint();
			}
			
			if(!m_playlist.empty() )
			{
				int step = 0;
				int prevselected = m_selected;

				step =  msg == CRCInput::RC_page_up ? m_listmaxshow : 1;
				m_selected -= step;
				if((prevselected-step) < 0)
					m_selected = m_playlist.size()-1;

				paintItem(prevselected - m_liststart);
				unsigned int oldliststart = m_liststart;
				m_liststart = (m_selected/m_listmaxshow)*m_listmaxshow;
				
				if(oldliststart != m_liststart)
				{
					update = true;
				}
				else
				{
					paintItem(m_selected - m_liststart);
				}
			}
		}
		else if(((msg &~ CRCInput::RC_Repeat) == CRCInput::RC_down || (msg &~ CRCInput::RC_Repeat) == CRCInput::RC_page_down) && !isURL)
		{
			if(hide_playlist)
			{
				paintInfo();
				paint();
			}
			
			if(!m_playlist.empty() )
			{
				int prevselected = m_selected;
				unsigned int step =  msg == CRCInput::RC_page_down ? m_listmaxshow : 1;
				m_selected += step;

				if(m_selected >= m_playlist.size()) 
				{
					if (((m_playlist.size() / m_listmaxshow) + 1) * m_listmaxshow == m_playlist.size() + m_listmaxshow) // last page has full entries
						m_selected = 0;
					else
						m_selected = ((step == m_listmaxshow) && (m_selected < (((m_playlist.size() / m_listmaxshow)+1) * m_listmaxshow))) ? (m_playlist.size() - 1) : 0;
				}

				paintItem(prevselected - m_liststart);
				unsigned int oldliststart = m_liststart;
				m_liststart = (m_selected/m_listmaxshow)*m_listmaxshow;
				
				if(oldliststart != m_liststart)
				{
					update = true;
				}
				else
				{
					paintItem(m_selected - m_liststart);
				}
			}
		}
		else if (msg == CRCInput::RC_ok || msg == CRCInput::RC_play)
		{
			if (!m_playlist.empty()) 
			{
				play(m_selected);
			}
		}
		else if (msg == CRCInput::RC_red)
		{
			if(m_key_level == 0 && !isURL)
			{
				if (!m_playlist.empty())
				{
					removeFromPlaylist(m_selected);
					if((int)m_selected == m_current)
					{
						m_current--;
					}
					
					if(m_selected >= m_playlist.size())
						m_selected = m_playlist.size() == 0 ? m_playlist.size() : m_playlist.size() - 1;
					update = true;
				}
			}
			else if(m_key_level == 1)
			{
				stop();
			} 
		}
		else if (msg == CRCInput::RC_stop)
		{
			if(isURL)
				loop = false;
			else
				stop();
		}
		else if(msg == CRCInput::RC_green)
		{
			if (m_key_level == 0 && !isURL)
			{
				openFilebrowser();
				update = true;
			}
			else if (m_key_level == 1)
			{
				if(m_curr_audiofile.FileExtension != CFile::EXTENSION_URL)
					rev();
			} 
			else if(!isURL)
			{ 
				// key_level == 2
				if(m_state == CAudioPlayerGui::STOP)
				{
					if (!m_playlist.empty()) 
					{
						savePlaylist();

						CVFD::getInstance()->setMode(CVFD::MODE_AUDIO, g_Locale->getText(m_inetmode? LOCALE_INETRADIO_NAME : LOCALE_AUDIOPLAYER_HEAD));
						
						paintLCD();
						
						update = true;
					}
				} 
				else
				{
					// keylevel 2 can only be reached if the currently played file
					// is no stream, so we do not have to test for this case
					int seconds = 0;
					CIntInput secondsInput(LOCALE_AUDIOPLAYER_JUMP_DIALOG_TITLE,
							seconds,
							5,
							LOCALE_AUDIOPLAYER_JUMP_DIALOG_HINT1,
							LOCALE_AUDIOPLAYER_JUMP_DIALOG_HINT2);
							
					int res = secondsInput.exec(NULL, "");
					
					if (seconds != 0 && res!= menu_return::RETURN_EXIT_ALL)
						rev(seconds);
					
					update = true;
				}
			}
		}
		else if( msg == CRCInput::RC_pause)
		{
			pause();
		}
		else if(msg == CRCInput::RC_yellow)
		{
			if(m_key_level == 0 && !isURL)
			{
				if (!m_playlist.empty())
				{
					//stop();
					clearPlaylist();
					clear_before_update = true;
					update = true;
				}
			}
			else if(m_key_level == 1)
			{
				pause();
			} 
			else if(!isURL)
			{ 
				// key_level==2
				m_select_title_by_name =! m_select_title_by_name;

				if (m_select_title_by_name && m_playlistHasChanged)
					buildSearchTree();
				paint();
			}
		}
		else if(msg == CRCInput::RC_blue)
		{
			if (m_key_level == 0 && !isURL)
			{
				if (m_inetmode) 
				{
					static int old_select = 0;
					char cnt[5];
					CMenuWidget InputSelector(LOCALE_AUDIOPLAYER_LOAD_RADIO_STATIONS, NEUTRINO_ICON_AUDIO);

					InputSelector.disableMenuPosition();
					int count = 0;
					int select = -1;
					
					CMenuSelectorTarget * InetRadioInputChanger = new CMenuSelectorTarget(&select);
					
					// localeradios
					sprintf(cnt, "%d", count);
					InputSelector.addItem(new CMenuForwarder(LOCALE_AUDIOPLAYER_ADD_LOC, true, NULL, InetRadioInputChanger, cnt, CRCInput::convertDigitToKey(count + 1)), old_select == count);

					// icecast
					sprintf(cnt, "%d", ++count);
					InputSelector.addItem(new CMenuForwarder(LOCALE_AUDIOPLAYER_ADD_IC, true, NULL, InetRadioInputChanger, cnt, CRCInput::convertDigitToKey(count + 1)), old_select == count);
					
					hide();
					InputSelector.exec(NULL, "");
					delete InetRadioInputChanger;
					
					if(select >= LOCALRADIO)
						old_select = select;
					
					switch (select) 
					{
						case LOCALRADIO:	
							scanXmlFile(RADIO_STATION_XML_FILE); 	

							CVFD::getInstance()->setMode(CVFD::MODE_AUDIO, g_Locale->getText(m_inetmode? LOCALE_INETRADIO_NAME : LOCALE_AUDIOPLAYER_HEAD));							
							paintLCD();
							
							break;
							
						case ICECAST:	
							readDir_ic();
							CVFD::getInstance()->setMode(CVFD::MODE_AUDIO, g_Locale->getText(m_inetmode? LOCALE_INETRADIO_NAME : LOCALE_AUDIOPLAYER_HEAD));							
							paintLCD();
							
							break;
							
						default: break;
					}
					
					update = true;
				}
				else if ( shufflePlaylist() )
				{
					update = true;
				}
			}
			else if (m_key_level == 1)
			{
				if(m_curr_audiofile.FileExtension != CFile::EXTENSION_URL)
					ff();
			} 
			else if(!isURL) // key_level == 2
			{
				if (m_state != CAudioPlayerGui::STOP)
				{
					// keylevel 2 can only be reached if the currently played file
					// is no stream, so we do not have to test for this case
					int seconds = 0;
					CIntInput secondsInput(LOCALE_AUDIOPLAYER_JUMP_DIALOG_TITLE,
							seconds,
							5,
							LOCALE_AUDIOPLAYER_JUMP_DIALOG_HINT1,
							LOCALE_AUDIOPLAYER_JUMP_DIALOG_HINT2);
							
					int res = secondsInput.exec(NULL, "");
					
					if (seconds != 0 && res!= menu_return::RETURN_EXIT_ALL)
						ff(seconds);
					
					update = true;
				}
			}
		}
		else if( msg == CRCInput::RC_info && !isURL)
		{
			if (m_key_level == 2)
				m_key_level = 0;
			else
				m_key_level++;

			if (m_state != CAudioPlayerGui::STOP)
			{
				// jumping in streams not supported
				if (m_key_level == 2 &&
						m_curr_audiofile.FileExtension == CFile::EXTENSION_URL)
				{
					m_key_level = 0;
				}
			}
			// there are only two keylevels in the "STOP"-case
			else if(m_key_level == 1)
			{
				m_key_level = 2;
			}
			
			if(hide_playlist)
				paint();
			
			paintFoot();
		}
		else if(msg == CRCInput::RC_0 && !isURL)
		{
			if(m_current >= 0)
			{
				m_selected = m_current;
				update = true;
			}
		}
		else if ( (CRCInput::isNumeric(msg) && !(m_playlist.empty())) && !isURL)
		{ 
			//numeric zap or SMS zap
			if (m_select_title_by_name)
			{
				unsigned char smsKey = 0;				
				do 
				{
					smsKey = m_SMSKeyInput.handleMsg(msg);

					g_RCInput->getMsg_ms(&msg, &data, AUDIOPLAYERGUI_SMSKEY_TIMEOUT - 200);

 					char selectedKey[1];
 					sprintf(selectedKey, "%c", smsKey);
 					int x1 = (g_settings.screen_EndX - g_settings.screen_StartX)/2 + g_settings.screen_StartX - 50;
 					int y1 = (g_settings.screen_EndY - g_settings.screen_StartY)/2 + g_settings.screen_StartY;
 					int h = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNEL_NUM_ZAP]->getHeight();
 					int w = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNEL_NUM_ZAP]->getRenderWidth(selectedKey);
					
 					m_frameBuffer->paintBoxRel(x1 - 7, y1 - h - 5, w + 14, h + 10, COL_MENUCONTENT_PLUS_6);
 					m_frameBuffer->paintBoxRel(x1 - 4, y1 - h - 3, w +  8, h +  6, COL_MENUCONTENTSELECTED_PLUS_0);
 					g_Font[SNeutrinoSettings::FONT_TYPE_CHANNEL_NUM_ZAP]->RenderString(x1, y1, w + 1, selectedKey, COL_MENUCONTENTSELECTED, 0, true);
				} while (CRCInput::isNumeric(msg) && !(m_playlist.empty()));

				if (msg == CRCInput::RC_timeout || msg == CRCInput::RC_nokey)
				{
					selectTitle(smsKey);
					update = true;
				}
				m_SMSKeyInput.resetOldKey();
			} 
			else 
			{
				int val = 0;
				if (getNumericInput(msg,val)) 
					m_selected = std::min((int)m_playlist.size(), val) - 1;
				
				update = true;
			}

		}
		else if( ((msg == CRCInput::RC_setup && !m_inetmode) || (msg == CRCInput::RC_vfdmenu && !m_inetmode)) && !isURL )
		{
			CNFSSmallMenu nfsMenu;
			nfsMenu.exec(this, "");

			CVFD::getInstance()->setMode(CVFD::MODE_AUDIO, g_Locale->getText(m_inetmode? LOCALE_INETRADIO_NAME : LOCALE_AUDIOPLAYER_HEAD));			
			paintLCD();			
			update = true;
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
	
	hide();

	if(m_state != CAudioPlayerGui::STOP)
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

bool CAudioPlayerGui::clearPlaylist(void)
{
	dprintf(DEBUG_NORMAL, "CAudioPlayerGui::clearPlaylist\n");
	
	bool result = false;

	if (!(m_playlist.empty()))
	{
		m_playlist.clear();
		m_current = -1;
		m_selected = 0;
		m_title2Pos.clear();
		result = true;
	}

	return(result);
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
		if (m_select_title_by_name)
		{
			buildSearchTree();
		}
		m_playlistHasChanged = true;
		m_selected = 0;

		result = true;
	}
	
	return(result);
}

void CAudioPlayerGui::addUrl2Playlist(const char *url, const char *name, const time_t bitrate) 
{
	dprintf(DEBUG_NORMAL, "CAudioPlayerGui::addUrl2Playlist: name = %s, url = %s\n", name, url);
	
	CAudiofileExt mp3(url, CFile::EXTENSION_URL);
	
	if (name != NULL) 
	{
		mp3.MetaData.title = name;
	} 
	else 
	{
		std::string tmp = mp3.Filename.substr(mp3.Filename.rfind('/')+1);
		mp3.MetaData.title = tmp;
	}
	
	if (bitrate)
		mp3.MetaData.total_time = bitrate;
	else
		mp3.MetaData.total_time = 0;

	if (url[0] != '#') 
	{
		addToPlaylist(mp3);
	}
}

void CAudioPlayerGui::processPlaylistUrl(const char *url, const char *name, const time_t tim) 
{
	dprintf(DEBUG_NORMAL, "CAudioPlayerGui::processPlaylistUrl\n");
	
	CURL *curl_handle;
	struct MemoryStruct chunk;
	
	chunk.memory = NULL; 	/* we expect realloc(NULL, size) to work */
	chunk.size = 0;    	/* no data at this point */

	curl_global_init(CURL_GLOBAL_ALL);

	/* init the curl session */
	curl_handle = curl_easy_init();

	/* specify URL to get */
	curl_easy_setopt(curl_handle, CURLOPT_URL, url);

	/* send all data to this function  */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

	/* some servers don't like requests that are made without a user-agent field, so we provide one */
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

	/* don't use signal for timeout */
	curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, (long)1);

	/* set timeout to 10 seconds */
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, GET_PLAYLIST_TIMEOUT);
	
	if(strcmp(g_settings.softupdate_proxyserver, "") != 0)
	{
		curl_easy_setopt(curl_handle, CURLOPT_PROXY, g_settings.softupdate_proxyserver);
		
		if(strcmp(g_settings.softupdate_proxyusername, "") != 0)
		{
			char tmp[200];
			strcpy(tmp, g_settings.softupdate_proxyusername);
			strcat(tmp, ":");
			strcat(tmp, g_settings.softupdate_proxypassword);
			curl_easy_setopt(curl_handle, CURLOPT_PROXYUSERPWD, tmp);
		}
	}

	/* get it! */
	curl_easy_perform(curl_handle);

	/* cleanup curl stuff */
	curl_easy_cleanup(curl_handle);

	/*
	* Now, our chunk.memory points to a memory block that is chunk.size
	* bytes big and contains the remote file.
	*
	* Do something nice with it!
	*
	* You should be aware of the fact that at this point we might have an
	* allocated data block, and nothing has yet deallocated that data. So when
	* you're done with it, you should free() it as a nice application.
	*/

	long res_code;
	if (curl_easy_getinfo(curl_handle, CURLINFO_HTTP_CODE, &res_code ) ==  CURLE_OK) 
	{
		if (200 == res_code) 
		{
			//printf("\nchunk = %s\n", chunk.memory);
			std::istringstream iss;
			iss.str (std::string(chunk.memory, chunk.size));
			char line[512];
			char *ptr;
			
			while (iss.rdstate() == std::ifstream::goodbit) 
			{
				iss.getline(line, 512);
				if (line[0] != '#') 
				{
					//printf("chunk: line = %s\n", line);
					ptr = strstr(line, "http://");
					if (ptr != NULL) 
					{
						char *tmp;
						// strip \n and \r characters from url
						tmp = strchr(line, '\r');
						if (tmp != NULL)
							*tmp = '\0';
						tmp = strchr(line, '\n');
						if (tmp != NULL)
							*tmp = '\0';
						
						addUrl2Playlist(ptr, name, tim);
					}
				}
			}
		}
	}

	if(chunk.memory)
		free(chunk.memory);
 
	/* we're done with libcurl, so clean it up */
	curl_global_cleanup();
}

void CAudioPlayerGui::readDir_ic(void)
{
	dprintf(DEBUG_NORMAL, "CAudioPlayerGui::readDir_ic\n");
	
	std::string answer = "";
	std::cout << "CAudioPlayerGui::readDir_ic: IC URL: " << icecasturl << std::endl;
	CURL *curl_handle;
	CURLcode httpres;
	
	/* init the curl session */
	curl_handle = curl_easy_init();
	/* specify URL to get */
	curl_easy_setopt(curl_handle, CURLOPT_URL, icecasturl.c_str());
	/* send all data to this function  */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, CurlWriteToString);
	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl_handle, CURLOPT_FILE, (void *)&answer);
	/* Generate error if http error >= 400 occurs */
	curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1);
	/* set timeout to 30 seconds */
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, GET_ICECAST_TIMEOUT);
	
	if(strcmp(g_settings.softupdate_proxyserver, "")!=0)
	{
		curl_easy_setopt(curl_handle, CURLOPT_PROXY, g_settings.softupdate_proxyserver);
		
		if(strcmp(g_settings.softupdate_proxyusername, "") != 0)
		{
			char tmp[200];
			strcpy(tmp, g_settings.softupdate_proxyusername);
			strcat(tmp, ":");
			strcat(tmp, g_settings.softupdate_proxypassword);
			curl_easy_setopt(curl_handle, CURLOPT_PROXYUSERPWD, tmp);
		}
	}

	/* error handling */
	char error[CURL_ERROR_SIZE];
	curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, error);
	
	/* get it! */
	CHintBox *scanBox = new CHintBox(LOCALE_AUDIOPLAYER_ADD_IC, g_Locale->getText(LOCALE_AUDIOPLAYER_RECEIVING_LIST)); // UTF-8
	scanBox->paint();

	httpres = curl_easy_perform(curl_handle);

	/* cleanup curl stuff */
	curl_easy_cleanup(curl_handle);

	if (!answer.empty() && httpres == 0)
	{
		xmlDocPtr answer_parser = parseXml(answer.c_str());
		scanBox->hide();
		scanXmlData(answer_parser, "server_name", "listen_url", "bitrate", true);
	}
	else 
		scanBox->hide();

	delete scanBox;
}

void CAudioPlayerGui::scanXmlFile(std::string filename)
{
	xmlDocPtr answer_parser = parseXmlFile(filename.c_str());
	scanXmlData(answer_parser, "name", "url");
}

void CAudioPlayerGui::scanXmlData(xmlDocPtr answer_parser, const char *nametag, const char *urltag, const char *bitratetag, bool usechild)
{
	dprintf(DEBUG_NORMAL, "CAudioPlayerGui::scanXmlData\n");
	
#define IC_typetag "server_type"

	if (answer_parser != NULL) 
	{
		xmlNodePtr element = xmlDocGetRootElement(answer_parser);
		element = element->xmlChildrenNode;
		xmlNodePtr element_tmp = element;
		
		if (element == NULL) 
		{
			dprintf(DEBUG_NORMAL, "CAudioPlayerGui::scanXmlData: No valid XML File.\n");
		} 
		else 
		{
			CProgressWindow progress;
			long maxProgress = 1;
			
			// count # of entries
			while (element) 
			{
				maxProgress++;
				element = element->xmlNextNode;
			}
			
			element = element_tmp;
			long listPos = -1;
			
			progress.setTitle(LOCALE_AUDIOPLAYER_LOAD_RADIO_STATIONS);
			progress.exec(this, "");
			
			neutrino_msg_t      msg;
			neutrino_msg_data_t data;
			
			g_RCInput->getMsg(&msg, &data, 0);
			
			while (element && msg != CRCInput::RC_home) 
			{
				char *ptr = NULL;
				char *name = NULL;
				char *url = NULL;
				char *type = NULL;
				time_t bitrate = 0;
				bool skip = true;
				listPos++;
				
				// show status
				int global = 100*listPos / maxProgress;
				progress.showGlobalStatus(global);
#if ENABLE_LCD
#ifdef LCD_UPDATE
				CVFD::getInstance()->showProgressBar(global, "read xmldata...");
				CVFD::getInstance()->setMode(CVFD::MODE_PROGRESSBAR);
#endif				
#endif // LCD_UPDATE

				if (usechild) 
				{
					xmlNodePtr child = element->xmlChildrenNode;
					while (child) 
					{
						if (strcmp(xmlGetName(child), nametag) == 0)
							name = xmlGetData(child);
						else if (strcmp(xmlGetName(child), urltag) == 0)
							url = xmlGetData(child);
						else if (strcmp(xmlGetName(child), IC_typetag) == 0)
							type = xmlGetData(child);
						else if (bitratetag && strcmp(xmlGetName(child), bitratetag) == 0) {
							ptr = xmlGetData(child);
							if (ptr) 
								bitrate = atoi(ptr);
						}
						child = child->xmlNextNode;
					}
					if 	(strcmp("audio/mpeg", type) == 0) 	skip = false;
					else if (strcmp("mp3", type) == 0) 		skip = false;
					else if (strcmp("application/mp3", type) == 0) 	skip = false;
				} 
				else 
				{
					url = xmlGetAttribute(element, (char *) urltag);
					name = xmlGetAttribute(element, (char *) nametag);
					
					if (bitratetag) 
					{
						ptr = xmlGetAttribute(element, (char *) bitratetag);
						if (ptr)
							bitrate = atoi(ptr);
					}
					skip = false;
				}

				if ((url != NULL) && !skip) 
				{
					progress.showStatusMessageUTF(url);
					
					if (strstr(url, ".m3u") || strstr(url, ".pls"))
						processPlaylistUrl(url, name);
					else 
						addUrl2Playlist(url, name, bitrate);
				}
				element = element->xmlNextNode;
				g_RCInput->getMsg(&msg, &data, 0);

			}
			progress.hide();
		}
		xmlFreeDoc(answer_parser);
	}
}

bool CAudioPlayerGui::openFilebrowser(void)
{
	dprintf(DEBUG_INFO, "CAudioPlayerGui::openFilebrowser\n");
	
	bool result = false;
	CFileBrowser filebrowser((g_settings.filebrowser_denydirectoryleave) ? g_settings.network_nfs_audioplayerdir : "");

	filebrowser.Multi_Select = true;
	filebrowser.Dirs_Selectable = true;
	filebrowser.Filter = &audiofilefilter;

	hide();

	if (filebrowser.exec(m_Path.c_str()))
	{
		CProgressWindow progress;
		long maxProgress = (filebrowser.getSelectedFiles().size() > 1) ? filebrowser.getSelectedFiles().size() - 1 : 1;
		long currentProgress = -1;
		
		if (maxProgress > SHOW_FILE_LOAD_LIMIT)
		{
			progress.setTitle(LOCALE_AUDIOPLAYER_READING_FILES);
			progress.exec(this, "");	
		}

		m_Path = filebrowser.getCurrentDir();
		CFileList::const_iterator files = filebrowser.getSelectedFiles().begin();
		
		for(; files != filebrowser.getSelectedFiles().end(); files++)
		{
			if (maxProgress > SHOW_FILE_LOAD_LIMIT)
			{
				currentProgress++;
				// show status
				int global = 100*currentProgress/maxProgress;
				progress.showGlobalStatus(global);
				progress.showStatusMessageUTF(files->Name);
#if ENABLE_LCD
#ifdef LCD_UPDATE
				CVFD::getInstance()->showProgressBar(global, "read metadata...");
				CVFD::getInstance()->setMode(CVFD::MODE_PROGRESSBAR);
#endif				
#endif // LCD_UPDATE
			}
			
			if ( (files->getExtension() == CFile::EXTENSION_CDR)
					||  (files->getExtension() == CFile::EXTENSION_MP3)
					||  (files->getExtension() == CFile::EXTENSION_WAV)
					||  (files->getExtension() == CFile::EXTENSION_FLAC)
			)
			{
				CAudiofileExt audiofile(files->Name, files->getExtension());
				addToPlaylist(audiofile);
			}
			
			if(files->getType() == CFile::FILE_URL)
			{
				std::string filename = files->Name;
				FILE *fd = fopen(filename.c_str(), "r");
				
				if(fd)
				{
					char buf[512];
					unsigned int len = fread(buf, sizeof(char), 512, fd);
					fclose(fd);

					if (len && (strstr(buf, ".m3u") || strstr(buf, ".pls")))
					{
						dprintf(DEBUG_INFO, "CAudioPlayerGui::openFilebrowser: m3u/pls Playlist found: %s\n", buf);
						
						filename = buf;
						processPlaylistUrl(files->Name.c_str());
					}
					else
					{
						addUrl2Playlist(filename.c_str());
					}
				}
			}
			else if(files->getType() == CFile::FILE_PLAYLIST)
			{
				std::string sPath = files->Name.substr(0, files->Name.rfind('/'));
				std::ifstream infile;
				char cLine[1024];
				char name[1024] = { 0 };
				int duration;
				
				infile.open(files->Name.c_str(), std::ifstream::in);

				while (infile.good())
				{
					infile.getline(cLine, sizeof(cLine));
					
					// remove CR
					if(cLine[strlen(cLine) - 1] == '\r')
						cLine[strlen(cLine) - 1] = 0;
					
					sscanf(cLine, "#EXTINF:%d,%[^\n]\n", &duration, name);
					
					if(strlen(cLine) > 0 && cLine[0] != '#')
					{
						char *url = strstr(cLine, "http://");
						if (url != NULL) 
						{
							if (strstr(url, ".m3u") || strstr(url, ".pls"))
								processPlaylistUrl(url);
							else
								addUrl2Playlist(url, name, duration);
						} 
						else if ((url = strstr(cLine, "icy://")) != NULL) 
						{
							addUrl2Playlist(url);
						} 
						else if ((url = strstr(cLine, "scast:://")) != NULL) 
						{
							addUrl2Playlist(url);
						}
						else
						{
							std::string filename = sPath + '/' + cLine;

							std::string::size_type pos;
							while((pos = filename.find('\\')) != std::string::npos)
								filename[pos] = '/';

							std::ifstream testfile;
							testfile.open(filename.c_str(), std::ifstream::in);
							
							if(testfile.good())
							{
#ifdef AUDIOPLAYER_CHECK_FOR_DUPLICATES
								// Check for duplicates and remove (new entry has higher prio)
								// this really needs some time :(
								for (unsigned long i = 0; i < m_playlist.size(); i++)
								{
									if(m_playlist[i].Filename == filename)
										removeFromPlaylist(i);
								}
#endif
								if(strcasecmp(filename.substr(filename.length() - 3, 3).c_str(), "url")==0)
								{
									addUrl2Playlist(filename.c_str());
								}
								else
								{
									CFile playlistItem;
									playlistItem.Name = filename;
									CFile::FileExtension fileExtension = playlistItem.getExtension();
									
									if (fileExtension == CFile::EXTENSION_CDR
											|| fileExtension == CFile::EXTENSION_MP3
											|| fileExtension == CFile::EXTENSION_WAV
											|| fileExtension == CFile::EXTENSION_FLAC
									)
									{
										CAudiofileExt audioFile(filename, fileExtension);
										addToPlaylist(audioFile);
									} 
									else
									{
										dprintf(DEBUG_NORMAL, "CAudioPlayerGui::openFilebrowser: file type (%d) is *not* supported in playlists\n(%s)\n", fileExtension, filename.c_str());
									}
								}
							}
							testfile.close();
						}
					}
				}
				infile.close();
			}
			else if(files->getType() == CFile::FILE_XML)
			{
				if (!files->Name.empty())
				{
					scanXmlFile(files->Name);
				}
			}
		}
		
		progress.hide();
		
		if (m_select_title_by_name)
		{
			buildSearchTree();
		}
		
		result = true;
	}
	
	CVFD::getInstance()->setMode(CVFD::MODE_AUDIO, g_Locale->getText(m_inetmode? LOCALE_INETRADIO_NAME : LOCALE_AUDIOPLAYER_HEAD));	
	paintLCD();

	return ( result);
}

void CAudioPlayerGui::hide()
{
	if(m_visible)
	{
		// main box
		m_frameBuffer->paintBackgroundBoxRel(m_x - ConnectLineBox_Width - 1, m_y + m_title_height - 1, m_width + ConnectLineBox_Width + 2, m_height + 2 - m_title_height);

		clearItemID3DetailsLine();
		
		// title
		m_frameBuffer->paintBackgroundBoxRel(m_x, m_y, m_width, m_title_height);

		m_frameBuffer->blit();
	
		m_visible = false;
		info_visible = false;
	}
}

void CAudioPlayerGui::paintItem(int pos)
{
	if(isURL)
		return;
	
	int ypos = m_y + m_title_height + m_theight + pos*m_fheight;
	
	uint8_t color;
	fb_pixel_t bgcolor;

	if ((pos + m_liststart) == m_selected)
	{
		color   = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;

		paintItemID3DetailsLine(pos);
	}
	else
	{
		if ((pos + m_liststart) == (unsigned)m_current)
		{
			color   = COL_MENUCONTENT + 2;
			bgcolor = COL_MENUCONTENT_PLUS_2;
		}
		else
		{
			color   = COL_MENUCONTENT;
			bgcolor = COL_MENUCONTENT_PLUS_0;
		}
	}

	// itembox
	m_frameBuffer->paintBoxRel(m_x, ypos, m_width - SCROLLBAR_WIDTH, m_fheight, bgcolor);

	if ((pos + m_liststart) < m_playlist.size())
	{
		if (m_playlist[pos + m_liststart].FileExtension != CFile::EXTENSION_URL && !m_playlist[pos + m_liststart].MetaData.bitrate)
		{
			// id3tag noch nicht geholt
			GetMetaData(m_playlist[pos + m_liststart]);

			if(m_state != CAudioPlayerGui::STOP && !g_settings.audioplayer_highprio)
				usleep(100*1000);
		}

		char sNr[20];
		sprintf(sNr, "%2d : ", pos + m_liststart + 1);
		std::string tmp = sNr;
		getFileInfoToDisplay(tmp, m_playlist[pos + m_liststart]);

		char dura[9];
		if (m_inetmode)
			snprintf(dura, 8, "%ldk", m_playlist[pos + m_liststart].MetaData.total_time);
		else
			snprintf(dura, 8, "%ld:%02ld", m_playlist[pos + m_liststart].MetaData.total_time / 60, m_playlist[pos + m_liststart].MetaData.total_time % 60);
		
		// number/title
		int w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(dura) + 5;
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + 10, ypos + m_fheight, m_width - 30 - w, tmp, color, m_fheight, true); // UTF-8

		// duration
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + m_width - 15 - w, ypos + m_fheight, w, dura, color, m_fheight);
		
		// lcd	
		if ((pos + m_liststart) == m_selected)
		{
			if (m_state == CAudioPlayerGui::STOP)
			{
				//VFD/LCD
				if (CVFD::getInstance()->is4digits)
					CVFD::getInstance()->LCDshowText(m_selected + 1);
				else
					CVFD::getInstance()->showAudioTrack(m_playlist[pos + m_liststart].MetaData.artist, m_playlist[pos + m_liststart].MetaData.title, m_playlist[pos + m_liststart].MetaData.album);
			}
		}		
	}
}

// playlist
// paint head
void CAudioPlayerGui::paintHead()
{
	if(isURL)
		return;
	
	std::string strCaption;
	
	if (m_inetmode)
		strCaption = g_Locale->getText(LOCALE_INETRADIO_NAME);
	else 
		strCaption = g_Locale->getText(LOCALE_AUDIOPLAYER_HEAD);
	
	// head box
	m_frameBuffer->paintBoxRel(m_x, m_y + m_title_height, m_width, m_theight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, g_settings.menu_Head_gradient);
	
	// head icon
	m_frameBuffer->getIconSize(NEUTRINO_ICON_MP3, &icon_head_w, &icon_head_h);
	m_frameBuffer->paintIcon(NEUTRINO_ICON_MP3, m_x + BORDER_LEFT, m_y + m_title_height + (m_theight - icon_head_h)/2);
	
	//head title
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(m_x + BORDER_LEFT + icon_head_w + 5, m_y + m_title_height + (m_theight - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), m_width - (BORDER_LEFT + BORDER_RIGHT + icon_head_w + 10), strCaption, COL_MENUHEAD, 0, true); // UTF-8
	
	// icon setup
	if (!m_inetmode)
	{
		int icon_w, icon_h;
		m_frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_SETUP, &icon_w, &icon_h);
		m_frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_SETUP, m_x + m_width - BORDER_RIGHT - icon_w, m_y + m_title_height + (m_theight - icon_h)/2);
	}
}

//
const struct button_label AudioPlayerButtons[][4] =
{
	{
		{ NEUTRINO_ICON_BUTTON_RED, LOCALE_AUDIOPLAYER_STOP, NULL },
		{ NEUTRINO_ICON_BUTTON_GREEN, LOCALE_AUDIOPLAYER_REWIND, NULL },
		{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_AUDIOPLAYER_PAUSE, NULL},
		{ NEUTRINO_ICON_BUTTON_BLUE, LOCALE_AUDIOPLAYER_FASTFORWARD, NULL},
	},
	{
		{ NEUTRINO_ICON_BUTTON_RED   , LOCALE_AUDIOPLAYER_DELETE, NULL },
		{ NEUTRINO_ICON_BUTTON_GREEN , LOCALE_AUDIOPLAYER_ADD, NULL },
		{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_AUDIOPLAYER_DELETEALL, NULL },
		{ NEUTRINO_ICON_BUTTON_BLUE  , LOCALE_AUDIOPLAYER_SHUFFLE, NULL },
	},
	{		
		{ NEUTRINO_ICON_BUTTON_GREEN , LOCALE_AUDIOPLAYER_JUMP_BACKWARDS, NULL },
		{ NEUTRINO_ICON_BUTTON_BLUE  , LOCALE_AUDIOPLAYER_JUMP_FORWARDS, NULL },
	},
	{
		{ NEUTRINO_ICON_BUTTON_GREEN , LOCALE_AUDIOPLAYER_JUMP_BACKWARDS, NULL },
		{ NEUTRINO_ICON_BUTTON_BLUE  , LOCALE_AUDIOPLAYER_JUMP_FORWARDS, NULL },
	},
	{
		{ NEUTRINO_ICON_BUTTON_GREEN , LOCALE_AUDIOPLAYER_SAVE_PLAYLIST, NULL },
		{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_AUDIOPLAYER_BUTTON_SELECT_TITLE_BY_ID, NULL },
	},
	{
		{ NEUTRINO_ICON_BUTTON_GREEN , LOCALE_AUDIOPLAYER_SAVE_PLAYLIST, NULL },
		{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_AUDIOPLAYER_BUTTON_SELECT_TITLE_BY_NAME, NULL },
	},
	{
		{ NEUTRINO_ICON_BUTTON_RED   , LOCALE_AUDIOPLAYER_STOP, NULL },
		{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_AUDIOPLAYER_PAUSE, NULL },
	},
	{
		{ NEUTRINO_ICON_BUTTON_GREEN , LOCALE_AUDIOPLAYER_ADD, NULL },
		{ NEUTRINO_ICON_BUTTON_BLUE  , LOCALE_INETRADIO_NAME, NULL },
	},
	{
		{ NEUTRINO_ICON_BUTTON_RED   , LOCALE_AUDIOPLAYER_DELETE, NULL },
		{ NEUTRINO_ICON_BUTTON_GREEN , LOCALE_AUDIOPLAYER_ADD, NULL },
		{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_AUDIOPLAYER_DELETEALL, NULL },
		{ NEUTRINO_ICON_BUTTON_BLUE  , LOCALE_INETRADIO_NAME, NULL },
	},
};

void CAudioPlayerGui::paintFoot()
{
	if(isURL)
		return;
	
	int top = m_y + m_height - (m_buttonHeight + m_info_height); //doppel button foot

	int ButtonWidth = (m_width - 20) / 4;
	int ButtonWidth2 = (m_width - 50) / 2;
	
	// foot
	m_frameBuffer->paintBoxRel(m_x, top, m_width, m_buttonHeight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.menu_Foot_gradient);

	if (!m_playlist.empty())
	{
		// play
		m_frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_OKAY, m_x + ButtonWidth2 + 25, top + (m_buttonHeight/2 - icon_foot_h)/2);
		g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(m_x + ButtonWidth2 + 53, top + 2 + (m_buttonHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight(), ButtonWidth2 - 28, g_Locale->getText(LOCALE_AUDIOPLAYER_PLAY), COL_INFOBAR, 0, true); // UTF-8
		
		// keylevel switch
		m_frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_HELP, m_x + ButtonWidth + 25, top + (m_buttonHeight/2 - icon_foot_h)/2);
		g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(m_x + ButtonWidth + 53, top + 2 +(m_buttonHeight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight(), ButtonWidth2 - 28, g_Locale->getText(LOCALE_AUDIOPLAYER_KEYLEVEL), COL_INFOBAR, 0, true); // UTF-8
	}

	if (m_key_level == 0)
	{
		if (m_playlist.empty()) 
		{
			if (m_inetmode)
				::paintButtons(m_frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, m_x + BORDER_LEFT + ButtonWidth, top + m_buttonHeight/2, ButtonWidth*2, 2, AudioPlayerButtons[7], m_buttonHeight/2);
			else
				::paintButtons(m_frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, m_x + BORDER_LEFT + ButtonWidth, top + m_buttonHeight/2, ButtonWidth, 1, &(AudioPlayerButtons[7][0]), m_buttonHeight/2);
		} 
		else
		{
			if (m_inetmode)
				::paintButtons(m_frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, m_x + BORDER_LEFT, top + m_buttonHeight/2, ButtonWidth, 4, AudioPlayerButtons[8], m_buttonHeight/2);
			else
				::paintButtons(m_frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, m_x + BORDER_LEFT, top + m_buttonHeight/2, ButtonWidth, 4, AudioPlayerButtons[1], m_buttonHeight/2);
		}
	}
	else if (m_key_level == 1)
	{
		if (m_curr_audiofile.FileExtension != CFile::EXTENSION_URL)
		{
			::paintButtons(m_frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, m_x + BORDER_LEFT, top + m_buttonHeight/2, ButtonWidth, 4, AudioPlayerButtons[0], m_buttonHeight/2);
		}
		else
		{
			::paintButtons(m_frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, m_x + BORDER_LEFT, top + m_buttonHeight/2, ButtonWidth*2, 2, AudioPlayerButtons[6], m_buttonHeight/2);
		}
	} 
	else 
	{ 
		// key_level == 2
		if (m_state == CAudioPlayerGui::STOP)
		{
			if (m_select_title_by_name)
			{
				::paintButtons(m_frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, m_x + ButtonWidth + BORDER_LEFT, top + m_buttonHeight/2, ButtonWidth, 2, AudioPlayerButtons[5], m_buttonHeight/2);
			} 
			else 
			{
				::paintButtons(m_frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, m_x + ButtonWidth + BORDER_LEFT, top + m_buttonHeight/2, ButtonWidth, 2, AudioPlayerButtons[4], m_buttonHeight/2);
			}
		} 
		else 
		{
			if (m_select_title_by_name)
			{
				::paintButtons(m_frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, m_x + ButtonWidth + BORDER_LEFT, top + m_buttonHeight/2, ButtonWidth*2, 2, AudioPlayerButtons[3], m_buttonHeight/2);
			} 
			else 
			{
				::paintButtons(m_frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, m_x + ButtonWidth + BORDER_LEFT, top + m_buttonHeight/2, ButtonWidth *2, 2, AudioPlayerButtons[2], m_buttonHeight/2);
			}
		}
	}	
}

void CAudioPlayerGui::paintInfo()
{
	if(m_state == CAudioPlayerGui::STOP )
	{
		m_frameBuffer->paintBackgroundBoxRel(m_x, m_y, m_width, m_title_height);
	}
	else
	{
		// infobox
		m_frameBuffer->paintBoxRel(m_x, m_y, m_width, m_title_height - 10, COL_MENUCONTENT_PLUS_6 );
		
		// infobox refresh
		m_frameBuffer->paintBoxRel(m_x + 2, m_y + 2 , m_width - 4, m_title_height - 14, COL_MENUCONTENTSELECTED_PLUS_0);

		// first line (Track number)
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

		// first line
		int w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp, true); // UTF-8
		int xstart = (m_width - w) / 2;
		if(xstart < BORDER_LEFT)
			xstart = BORDER_LEFT;

		g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + xstart, m_y + 4 + 1*m_fheight, m_width - 20, tmp, COL_MENUCONTENTSELECTED, 0, true); // UTF-8

		// second line (Artist/Title...)
		if (m_curr_audiofile.FileExtension != CFile::EXTENSION_URL) //FIXME: need to relaod id3tag
		{
			GetMetaData(m_curr_audiofile);
		}

		if (m_curr_audiofile.MetaData.title.empty())
			tmp = m_curr_audiofile.MetaData.artist;
		else if (m_curr_audiofile.MetaData.artist.empty())
			tmp = m_curr_audiofile.MetaData.title;
		else if (g_settings.audioplayer_display == TITLE_ARTIST)
		{
			tmp = m_curr_audiofile.MetaData.title;
			tmp += " / ";
			tmp += m_curr_audiofile.MetaData.artist;
		}
		else //if(g_settings.audioplayer_display == ARTIST_TITLE)
		{
			tmp = m_curr_audiofile.MetaData.artist;
			tmp += " / ";
			tmp += m_curr_audiofile.MetaData.title;
		}

		w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp, true); // UTF-8
		xstart = (m_width - w)/2;
		if(xstart < BORDER_LEFT)
			xstart = BORDER_LEFT;

		if (!m_curr_audiofile.MetaData.cover.empty())
		{
			if(xstart < (BORDER_LEFT + m_title_height - 14))
				xstart = BORDER_LEFT + m_title_height - 14;
		}
		
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + xstart, m_y + 4 + 2*m_fheight, m_width - BORDER_LEFT - BORDER_RIGHT, tmp, COL_MENUCONTENTSELECTED, 0, true); // UTF-8		
		
		// cover
		if (!m_curr_audiofile.MetaData.cover.empty())
		{
			if(!access("/tmp/cover.jpg", F_OK))
				m_frameBuffer->DisplayImage("/tmp/cover.jpg", m_x + 2, m_y + 2, m_title_height - 14, m_title_height - 14);		
		}

		// reset so fields get painted always
		m_metainfo.clear();
		m_time_total = 0;
		m_time_played = 0;
		
		updateMetaData();
		
		info_visible = true;

		updateTimes(true);
	}
}

void CAudioPlayerGui::paint()
{
	if(isURL)
		return;
	
	m_liststart = (m_selected / m_listmaxshow) * m_listmaxshow;
		
	// head
	paintHead();
	
	// items	
	for (unsigned int count = 0; count < m_listmaxshow; count++)
	{
		paintItem(count);
	}

	// scrollbar
	int ypos = m_y + m_title_height + m_theight;
	int sb = m_fheight * m_listmaxshow;

	m_frameBuffer->paintBoxRel(m_x + m_width - SCROLLBAR_WIDTH, ypos, SCROLLBAR_WIDTH, sb, COL_MENUCONTENT_PLUS_1);

	int sbc = ((m_playlist.size() - 1) / m_listmaxshow) + 1;
	int sbs = (m_selected / m_listmaxshow);

	m_frameBuffer->paintBoxRel(m_x + m_width - 13, ypos + 2 + sbs*(sb-4)/sbc , 11, (sb-4)/sbc, COL_MENUCONTENT_PLUS_3);

	// foot
	paintFoot();

	// upper infos
	paintInfo();
		
	m_visible = true;
}

void CAudioPlayerGui::clearItemID3DetailsLine()
{
	paintItemID3DetailsLine(-1);
}

void CAudioPlayerGui::paintItemID3DetailsLine(int pos)
{
	int xpos  = m_x - ConnectLineBox_Width;
	int ypos1 = m_y + m_title_height + m_theight + pos*m_fheight;
	int ypos2 = m_y + (m_height - m_info_height);
	int ypos1a = ypos1 + (m_fheight / 2) - 2;
	int ypos2a = ypos2 + (m_info_height / 2) - 2;
	fb_pixel_t col1 = COL_MENUCONTENT_PLUS_6;
	fb_pixel_t col2 = COL_MENUCONTENT_PLUS_1;


	// Clear
	m_frameBuffer->paintBackgroundBoxRel(xpos - 1, m_y + m_title_height, ConnectLineBox_Width + 1, m_height - m_title_height);	

	// paint Line if detail info (and not valid list pos)
	if (!m_playlist.empty() && (pos >= 0))
	{
		// 1. col thick line
		m_frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 4, ypos1, 4, m_fheight, col2);//FIXME
		m_frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 3, ypos1, 8, m_fheight, col1); // item marker

		m_frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 4, ypos2, 4, m_info_height, col1);

		m_frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 16, ypos1a, 4, ypos2a - ypos1a, col1);

		m_frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 16, ypos1a, 12, 4, col1);
		m_frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 16, ypos2a, 12, 4, col1);

		// 2. col small line
		m_frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 4, ypos2, 1, m_info_height, col2);

		m_frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 16, ypos1a, 1, ypos2a - ypos1a + 4, col2);

		m_frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 16, ypos1a, 12, 1, col2);
		m_frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 12, ypos2a,  8, 1, col2);

		// -- small Frame around infobox
		m_frameBuffer->paintBoxRel(m_x,			ypos2			, 2	 	, m_info_height	, col1);
		m_frameBuffer->paintBoxRel(m_x + m_width - 2,	ypos2			, 2		, m_info_height	, col1);
		m_frameBuffer->paintBoxRel(m_x,			ypos2			, m_width -2	, 2		, col1);
		m_frameBuffer->paintBoxRel(m_x,			ypos2 + m_info_height -2, m_width -2	, 2		, col1);

		// paint id3 infobox 
		// refresh box
		m_frameBuffer->paintBoxRel(m_x + 2, ypos2 + 2 , m_width - 4, m_info_height - 4, COL_MENUCONTENTDARK_PLUS_0, NO_RADIUS, CORNER_NONE, g_settings.menu_Head_gradient);

		g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + 10, ypos2 + 2 + 1*m_fheight, m_width- 80, m_playlist[m_selected].MetaData.title, COL_MENUCONTENTDARK, 0, true); // UTF-8

		std::string tmp;
		if (m_playlist[m_selected].MetaData.genre.empty())
			tmp = m_playlist[m_selected].MetaData.date;
		else if (m_playlist[m_selected].MetaData.date.empty())
			tmp = m_playlist[m_selected].MetaData.genre;
		else
		{
			tmp = m_playlist[m_selected].MetaData.genre;
			tmp += " / ";
			tmp += m_playlist[m_selected].MetaData.date;
		}
		int w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tmp, true) + 10; // UTF-8
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + m_width - w - 5, ypos2 + 2 + 1*m_fheight, w, tmp, COL_MENUCONTENTDARK, 0, true); // UTF-8
		tmp = m_playlist[m_selected].MetaData.artist;
		if (!(m_playlist[m_selected].MetaData.album.empty()))
		{
			tmp += " (";
			tmp += m_playlist[m_selected].MetaData.album;
			tmp += ')';
		}
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + 10, ypos2 + 2*m_fheight - 2, m_width - 20, tmp, COL_MENUCONTENTDARK, 0, true); // UTF-8		
	}
	else
	{
		m_frameBuffer->paintBackgroundBoxRel(m_x, ypos2, m_width, m_info_height);
	}
}

void CAudioPlayerGui::stop()
{
	m_state = CAudioPlayerGui::STOP;
	m_current = 0;
		
	//LCD
	paintLCD();	
	
	m_key_level = 0;
	
	paint();
	paintFoot();

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
	if(!m_playlist.size())
		return;
	
	unsigned int old_current = m_current;
	unsigned int old_selected = m_selected;

	m_current = pos;
	if(g_settings.audioplayer_follow)
		m_selected = pos;

	if(m_selected - m_liststart >= m_listmaxshow && g_settings.audioplayer_follow)
	{
		m_liststart = m_selected;
		
		if(!hide_playlist)
			paint();
	}
	else if(m_liststart < m_selected && g_settings.audioplayer_follow)
	{
		m_liststart = m_selected - m_listmaxshow + 1;
		
		if(!hide_playlist)
			paint();
	}
	else
	{
		if(old_current >= m_liststart && old_current - m_liststart < m_listmaxshow)
		{
			if(!hide_playlist)
				paintItem(old_current - m_liststart);
		}
		
		if(pos >= m_liststart && pos - m_liststart < m_listmaxshow)
		{
			if(!hide_playlist)
				paintItem(pos - m_liststart);
		}
		
		if(g_settings.audioplayer_follow)
		{
			if(old_selected >= m_liststart && old_selected - m_liststart < m_listmaxshow)
			{
				if(!hide_playlist)
					paintItem(old_selected - m_liststart);
			}
		}
	}

	// metadata
	if ( (m_playlist[pos].FileExtension != CFile::EXTENSION_M3U || m_playlist[pos].FileExtension != CFile::EXTENSION_URL || m_playlist[pos].FileExtension != CFile::EXTENSION_PLS)&& !m_playlist[pos].MetaData.bitrate)
	{
		GetMetaData(m_playlist[pos]);
	}
	
	m_metainfo.clear();
	m_time_played = 0;
	m_time_total = m_playlist[m_current].MetaData.total_time;
	m_state = CAudioPlayerGui::PLAY;
	m_curr_audiofile = m_playlist[m_current];

	// play
	CAudioPlayer::getInstance()->play(&m_curr_audiofile, g_settings.audioplayer_highprio == 1);

	//lcd	
	paintLCD();
	
	// info/cover
	if(!hide_playlist)
		paintInfo();
	
	m_key_level = 1;
	
	// foot
	if(!hide_playlist)
		paintFoot();
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
	bool updateMeta = false;
	bool updateLcd = false;
	bool updateScreen = false;

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

	if(updateScreen)
		paintInfo();
		
	if(updateMeta || updateScreen)
	{
		// refresh box
		m_frameBuffer->paintBoxRel(m_x + 10 + m_title_height, m_y + 4 + 2*m_fheight, m_width - 20 - m_title_height, m_sheight, COL_MENUCONTENTSELECTED_PLUS_0);
		
		int xstart = ((m_width - 20 - g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getRenderWidth(m_metainfo))/2)+10;

		g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(m_x + xstart, m_y + 4 + 2*m_fheight + m_sheight, m_width- 2*xstart, m_metainfo, COL_MENUCONTENTSELECTED);
	}
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
		if(info_visible)
		{
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
				// refresh box (time total)
				m_frameBuffer->paintBoxRel(m_x + m_width - w1 - 10, m_y + 4, w1 + 4, m_fheight, COL_MENUCONTENTSELECTED_PLUS_0);

				if(m_time_total > 0)
					g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + m_width - w1 - 10, m_y + 4 + m_fheight, w1, tot_time, COL_MENUCONTENTSELECTED);
			}
			
			if (updatePlayed || (m_state == CAudioPlayerGui::PAUSE))
			{
				// refresh box (played time)
				m_frameBuffer->paintBoxRel(m_x + m_width - w1 - w2 - 16, m_y + 4, w2 + 5, m_fheight, COL_MENUCONTENTSELECTED_PLUS_0);

				struct timeval tv;
				gettimeofday(&tv, NULL);
				if ((m_state != CAudioPlayerGui::PAUSE) || (tv.tv_sec & 1))
				{
					g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(m_x + m_width - w1 - w2 - 11, m_y + 4 + m_fheight, w2 + 4, play_time, COL_MENUCONTENTSELECTED);
				}
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
		
		File.MetaData.artist = FILESYSTEM_ENCODING_TO_UTF8_STRING(File.MetaData.artist);
		File.MetaData.title  = FILESYSTEM_ENCODING_TO_UTF8_STRING(File.MetaData.title );
	}
}

bool CAudioPlayerGui::getNumericInput(neutrino_msg_t& msg, int& val) 
{
	neutrino_msg_data_t data;
	int x1 = (g_settings.screen_EndX - g_settings.screen_StartX) / 2 + g_settings.screen_StartX - 50;
	int y1 = (g_settings.screen_EndY - g_settings.screen_StartY) / 2 + g_settings.screen_StartY;
	char str[11];
	
	do
	{
		val = val * 10 + CRCInput::getNumericValue(msg);
		sprintf(str, "%d", val);
		int w = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNEL_NUM_ZAP]->getRenderWidth(str);
		int h = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNEL_NUM_ZAP]->getHeight();
		m_frameBuffer->paintBoxRel(x1 - 7, y1 - h - 5, w + 14, h + 10, COL_MENUCONTENT_PLUS_6);
		m_frameBuffer->paintBoxRel(x1 - 4, y1 - h - 3, w +  8, h +  6, COL_MENUCONTENTSELECTED_PLUS_0);
		g_Font[SNeutrinoSettings::FONT_TYPE_CHANNEL_NUM_ZAP]->RenderString(x1, y1, w + 1, str, COL_MENUCONTENTSELECTED, 0);
		
		while (true)
		{
			g_RCInput->getMsg(&msg, &data, 100); 
			if (msg > CRCInput::RC_MaxRC && msg != CRCInput::RC_timeout)
			{	// not a key event
				CNeutrinoApp::getInstance()->handleMsg(msg, data);
				continue;
			}
			if (msg & (CRCInput::RC_Repeat|CRCInput::RC_Release)) // repeat / release
				continue;
			break;
		}		
	} while (g_RCInput->isNumeric(msg) && val < 1000000);
	
	return (msg == CRCInput::RC_ok);
}

void CAudioPlayerGui::getFileInfoToDisplay(std::string &info, CAudiofileExt &file)
{
	std::string fileInfo;
	std::string artist;
	std::string title;

	if (!m_inetmode) 
	{
		artist = "Artist?";
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

	if(g_settings.audioplayer_display == TITLE_ARTIST)
	{
		fileInfo += title;
		if (!title.empty() && !artist.empty()) 
			fileInfo += ", ";
		
		fileInfo += artist;
	}
	else //if(g_settings.audioplayer_display == ARTIST_TITLE)
	{
		fileInfo += artist;
		if (!title.empty() && !artist.empty()) fileInfo += ", ";
		fileInfo += title;
	}

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
	
	if (m_select_title_by_name)
	{	
		if (!file.MetaData.bitrate)
		{
			std::string t = "";
			getFileInfoToDisplay(t, file);
		}
	}
	m_playlist.push_back(file);
	m_playlistHasChanged = true;
}

void CAudioPlayerGui::removeFromPlaylist(long pos)
{
	unsigned char firstChar = ' ';
	if (m_select_title_by_name)
	{
		// must be called before m_playlist.erase()
		firstChar = getFirstChar(m_playlist[pos]);
	}

	m_playlist.erase(m_playlist.begin() + pos); 
	m_playlistHasChanged = true;

	if (m_select_title_by_name)
	{
		CTitle2Pos::iterator item = m_title2Pos.find(firstChar);
		if (item != m_title2Pos.end())
		{
			item->second.erase(pos);

			// delete empty entries
			if (item->second.size() == 0)
			{
				m_title2Pos.erase(item);
			}
		} 
		else
		{
			dprintf(DEBUG_NORMAL, "could not find key: %c pos: %ld\n", firstChar, pos);
		}
		// decrease position information for all titles with a position 
		// behind item to delete
		long p = 0;
		for (CTitle2Pos::iterator title = m_title2Pos.begin(); title!=m_title2Pos.end(); title++)
		{
			CPosList newList;
			for (CPosList::iterator posIt = title->second.begin(); posIt!=title->second.end(); posIt++)
			{
				p = *(posIt);
				if (*posIt > pos)
					p--;
				// old list is sorted so we can give a hint to insert at the end
				newList.insert(newList.end(), p);
			}
			//title->second.clear();
			title->second = newList;
		}		
	}
}

void CAudioPlayerGui::selectTitle(unsigned char selectionChar)
{
	unsigned long i;

	CTitle2Pos::iterator it = m_title2Pos.find(selectionChar);

	if (it!=m_title2Pos.end())
	{
		// search for the next greater id
		// if nothing found take the first
		CPosList::iterator posIt = it->second.upper_bound(m_selected);
		if (posIt != it->second.end())
		{
			i = *posIt; 
		} 
		else
		{
			if (it->second.size() > 0)
			{
				i = *(it->second.begin());
			} 
			else
			{
				return;
			}
		}	
	} 
	else
	{
		return;
	}		

	int prevselected = m_selected;
	m_selected = i;

	paintItem(prevselected - m_liststart);
	unsigned int oldliststart = m_liststart;
	m_liststart = (m_selected / m_listmaxshow)*m_listmaxshow;

	if(oldliststart != m_liststart)
	{
		paint();
	}
	else
	{
		paintItem(m_selected - m_liststart);
	}
}

void CAudioPlayerGui::printSearchTree()
{
	for (CTitle2Pos::iterator it = m_title2Pos.begin(); it!=m_title2Pos.end(); it++)
	{
		dprintf(DEBUG_NORMAL, "key: %c\n",it->first);
		
		long pos = -1;
		for (CPosList::iterator it2 = it->second.begin(); it2!=it->second.end(); it2++)
		{
			pos++;
		}
	}
}

void CAudioPlayerGui::buildSearchTree()
{
	CProgressWindow progress;
	progress.setTitle(LOCALE_AUDIOPLAYER_BUILDING_SEARCH_INDEX);
	progress.exec(this, "");

	long maxProgress = (m_playlist.size() > 1) ? m_playlist.size() - 1 : 1;

	m_title2Pos.clear();
	long listPos = -1;

	for (CAudioPlayList::iterator it = m_playlist.begin(); it!=m_playlist.end(); it++)
	{
		listPos++;
		progress.showGlobalStatus(100*listPos / maxProgress);

		progress.showStatusMessageUTF(it->Filename);
		unsigned char firstChar = getFirstChar(*it);
		const std::pair<CTitle2Pos::iterator,bool> item = m_title2Pos.insert(CTitle2PosItem(firstChar, CPosList()));
		item.first->second.insert(listPos);
	}
	
	progress.hide();
	m_playlistHasChanged = false;
}

unsigned char CAudioPlayerGui::getFirstChar(CAudiofileExt &file)
{
	if (file.firstChar == '\0')
	{
		std::string info;
		getFileInfoToDisplay(info, file);
	}

	return file.firstChar;
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
		this->paint();
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
			this->paint();
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
			this->paint();
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
	this->paint();
}

bool CAudioPlayerGui::askToOverwriteFile(const std::string& filename) 
{
	char msg[filename.length() + 127];
	
	snprintf(msg, filename.length() + 126, "%s\n%s", g_Locale->getText(LOCALE_AUDIOPLAYER_PLAYLIST_FILEOVERWRITE_MSG), filename.c_str());
	bool res = (MessageBox(LOCALE_AUDIOPLAYER_PLAYLIST_FILEOVERWRITE_TITLE, msg, CMessageBox::mbrYes, CMessageBox::mbYes | CMessageBox::mbNo) == CMessageBox::mbrYes);
	this->paint();
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

