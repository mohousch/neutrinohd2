/*
	Neutrino-GUI  -   DBoxII-Project

	MP3Player by Dirch
	
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

#include <gui/pictureviewer.h>

#include <global.h>
#include <neutrino.h>

#include <daemonc/remotecontrol.h>

#include <driver/fontrenderer.h>
#include <driver/rcinput.h>

#include <gui/nfs.h>

#include <gui/widget/buttons.h>
#include <gui/widget/icons.h>
#include <gui/widget/menue.h>
#include <gui/widget/messagebox.h>

// remove this
#include <gui/widget/hintbox.h>

#include <gui/widget/helpbox.h>
#include <gui/widget/stringinput.h>

#include <gui/pictureviewer_setup.h>

#include <system/settings.h>
#include <system/debug.h>

#include <driver/screen_max.h>

#include <algorithm>
#include <sys/stat.h>
#include <sys/time.h>


CPictureViewerGui::CPictureViewerGui()
{
	frameBuffer = CFrameBuffer::getInstance();

	m_state = SINGLE;

	g_PicViewer = new CPictureViewer();

	selected = 0;
}

CPictureViewerGui::~CPictureViewerGui()
{
	playlist.clear();

	// free picviewer mem
	g_PicViewer->Cleanup();
	delete g_PicViewer;
	g_PicViewer = NULL;
}

int CPictureViewerGui::exec(CMenuTarget* parent, const std::string &actionKey)
{
	dprintf(DEBUG_NORMAL, "CPictureViewerGui::exec: actionKey:%s\n", actionKey.c_str());
	
	// 
	g_PicViewer->SetScaling( (CFrameBuffer::ScalingMode)g_settings.picviewer_scaling);
	g_PicViewer->SetVisible(g_settings.screen_StartX, g_settings.screen_EndX, g_settings.screen_StartY, g_settings.screen_EndY);

	if(g_settings.video_Ratio == 1)
		g_PicViewer->SetAspectRatio(16.0/9);
	else
		g_PicViewer->SetAspectRatio(4.0/3);

	if(parent)
		parent->hide();

	// tell neutrino we're in pic_mode
	CNeutrinoApp::getInstance()->handleMsg( NeutrinoMessages::CHANGEMODE , NeutrinoMessages::mode_pic );
	
	// remember last mode
	m_LastMode = (CNeutrinoApp::getInstance()->getLastMode() | NeutrinoMessages::norezap);
	
	// Save and Clear background
	bool usedBackground = frameBuffer->getuseBackground();
	if (usedBackground) 
	{
		frameBuffer->saveBackgroundImage();
		frameBuffer->ClearFrameBuffer();
		frameBuffer->blit();
	}
	
	//
	CNeutrinoApp::getInstance()->lockPlayBack();

	show();

	// free picviewer mem
	g_PicViewer->Cleanup();

	// Restore previous background	
	if (usedBackground) 
	{
		frameBuffer->restoreBackgroundImage();
		frameBuffer->useBackground(true);
		frameBuffer->paintBackground();
		frameBuffer->blit();	
	}
	
	//
	CNeutrinoApp::getInstance()->unlockPlayBack();

	// Restore last mode
	CNeutrinoApp::getInstance()->handleMsg( NeutrinoMessages::CHANGEMODE , m_LastMode );

	if(!playlist.empty())
		playlist.clear();

	// 
	return menu_return::RETURN_EXIT;
}

int CPictureViewerGui::show()
{
	dprintf(DEBUG_NORMAL, "CPictureViewerGui::show\n");

	neutrino_msg_t msg;
	neutrino_msg_data_t data;

	int res = -1;

	CVFD::getInstance()->setMode(CVFD::MODE_PIC, g_Locale->getText(LOCALE_PICTUREVIEWER_HEAD));

	int timeout;

	bool loop = true;

	//		
	if (!playlist.empty())
		view(selected);
	
	while(loop)
	{
		if(m_state != SLIDESHOW)
			timeout = 10; 
		else
		{
			timeout = (m_time + atoi(g_settings.picviewer_slide_time) - (long)time(NULL))*10;
			if(timeout < 0 )
				timeout = 1;
		}

		g_RCInput->getMsg(&msg, &data, timeout);

		if( msg == CRCInput::RC_home)
		{
			loop = false;
		}
		else if (msg == CRCInput::RC_timeout)
		{
			if(m_state == SLIDESHOW)
			{
				m_time = (long)time(NULL);
				unsigned int next = selected + 1;
				if (next >= playlist.size())
					next = 0;
				
				view(next);
			}
		}
		else if (msg == CRCInput::RC_left)
		{
			if (!playlist.empty())
			{
				view((selected == 0) ? (playlist.size() - 1) : (selected - 1));
			}
		}
		else if (msg == CRCInput::RC_right)
		{
			if (!playlist.empty())
			{
				unsigned int next = selected + 1;
				if (next >= playlist.size())
					next = 0;
				view(next);
			}
		}
		else if( msg == CRCInput::RC_1 )
		{ 
			g_PicViewer->Zoom(2.0/3);
		}
		else if( msg == CRCInput::RC_2 )
		{ 
			g_PicViewer->Move(0, -10);
		}
		else if( msg == CRCInput::RC_3 )
		{ 
			g_PicViewer->Zoom(1.5);
		}
		else if( msg == CRCInput::RC_4 )
		{ 
			g_PicViewer->Move(-10, 0);
		}
		else if( msg == CRCInput::RC_6 )
		{ 
			if (playlist.empty())
			{
				g_PicViewer->Move(10, 0);
			}
		}
		else if( msg == CRCInput::RC_8 )
		{ 
			if (playlist.empty())
			{
				g_PicViewer->Move(0, 10);
			}
		}
		else if(msg == CRCInput::RC_0)
		{
			if (!playlist.empty())
				view(selected);
		}
		else if(msg == CRCInput::RC_setup)
		{
			CPictureViewerSettings * pictureViewerSettingsMenu = new CPictureViewerSettings();
			pictureViewerSettingsMenu->exec(NULL, "");
			delete pictureViewerSettingsMenu;
			pictureViewerSettingsMenu = NULL;					
		}
		else if(msg == NeutrinoMessages::CHANGEMODE)
		{
			if((data & NeutrinoMessages::mode_mask) !=NeutrinoMessages::mode_pic)
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
		else
		{
			if( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
			{
				loop = false;
			}
		}

		frameBuffer->blit();	
	}

	frameBuffer->paintBackground();
	frameBuffer->blit();

	return(res);
}

void CPictureViewerGui::view(unsigned int index)
{
	selected = index;
	
	if (CVFD::getInstance()->is4digits)
		CVFD::getInstance()->LCDshowText(selected + 1);
	else
		CVFD::getInstance()->showMenuText(0, playlist[index].Name.c_str());
	
	g_PicViewer->ShowImage(playlist[index].Filename);
}

void CPictureViewerGui::addToPlaylist(CPicture& file)
{	
	dprintf(DEBUG_NORMAL, "CPictureViewerGui::addToPlaylist: %s\n", file.Filename.c_str());
	
	playlist.push_back(file);
}

void CPictureViewerGui::clearPlaylist(void)
{
	dprintf(DEBUG_NORMAL, "CPictureViewerGui::clearPlaylist:\n");

	if (!playlist.empty())
	{
		playlist.clear();
		selected = 0;
	}
}

void CPictureViewerGui::removeFromPlaylist(long pos)
{
	dprintf(DEBUG_NORMAL, "CPictureViewerGui::removeFromPlayList:\n");

	playlist.erase(playlist.begin() + pos); 
}

void CPictureViewerGui::showHelp()
{
	CHelpBox helpbox;

	helpbox.addLine(g_Locale->getText(LOCALE_PICTUREVIEWER_HELP1));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_OKAY, g_Locale->getText(LOCALE_PICTUREVIEWER_HELP2));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_0, g_Locale->getText(LOCALE_PICTUREVIEWER_HELP4));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_5, g_Locale->getText(LOCALE_PICTUREVIEWER_HELP3));
	helpbox.addPagebreak();
	helpbox.addLine(g_Locale->getText(LOCALE_PICTUREVIEWER_HELP5));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_LEFT, g_Locale->getText(LOCALE_PICTUREVIEWER_HELP6));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_RIGHT, g_Locale->getText(LOCALE_PICTUREVIEWER_HELP7));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_5, g_Locale->getText(LOCALE_PICTUREVIEWER_HELP8));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_HOME, g_Locale->getText(LOCALE_PICTUREVIEWER_HELP9));
	helpbox.addPagebreak();
	helpbox.addLine(g_Locale->getText(LOCALE_PICTUREVIEWER_HELP10));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_OKAY, g_Locale->getText(LOCALE_PICTUREVIEWER_HELP11));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_LEFT, g_Locale->getText(LOCALE_PICTUREVIEWER_HELP12));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_RIGHT, g_Locale->getText(LOCALE_PICTUREVIEWER_HELP13));
	helpbox.addPagebreak();
	helpbox.addLine(NEUTRINO_ICON_BUTTON_0, g_Locale->getText(LOCALE_PICTUREVIEWER_HELP21));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_1, g_Locale->getText(LOCALE_PICTUREVIEWER_HELP14));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_2, g_Locale->getText(LOCALE_PICTUREVIEWER_HELP16));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_3, g_Locale->getText(LOCALE_PICTUREVIEWER_HELP15));
	//helpbox.addPagebreak();
	helpbox.addLine(NEUTRINO_ICON_BUTTON_4, g_Locale->getText(LOCALE_PICTUREVIEWER_HELP17));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_5, g_Locale->getText(LOCALE_PICTUREVIEWER_HELP20));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_6, g_Locale->getText(LOCALE_PICTUREVIEWER_HELP18));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_8, g_Locale->getText(LOCALE_PICTUREVIEWER_HELP19));
	helpbox.addLine(NEUTRINO_ICON_BUTTON_HOME, g_Locale->getText(LOCALE_PICTUREVIEWER_HELP22));
	helpbox.addPagebreak();

	helpbox.addLine("Version: $Revision: 2.0 $");
	hide();
	helpbox.show(LOCALE_MESSAGEBOX_INFO);
}

