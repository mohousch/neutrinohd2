/*
	Neutrino-GUI  -   DBoxII-Project
	
	$id: setting_helpers.cpp 2015.12.22 15:24:30 mohousch $

	Copyright (C) 2001 Steffen Hehn 'McClean'
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

#include <system/setting_helpers.h>

#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/vfs.h>

#include <fcntl.h>
#include <signal.h>
#include "libnet.h"

#include <config.h>

#include <global.h>
#include <neutrino.h>

#include <gui/widget/stringinput.h>
#include <gui/widget/messagebox.h>
#include <gui/widget/hintbox.h>

#include <gui/plugins.h>
#include <gui/network_setup.h>

#include <daemonc/remotecontrol.h>
#include <xmlinterface.h>

#include "debug.h"

#include <audio_cs.h>
#include <video_cs.h>
#include <dmx_cs.h>

/*zapit includes*/
#include <frontend_c.h>

/*gui includes*/
#include <gui/scan_setup.h>
#include <gui/filebrowser.h>

/* configfiles */
#include <gui/moviebrowser.h>
#include <timerd/timermanager.h>
#include <nhttpd/yconfig.h>


extern CPlugins       * g_PluginList;    /* neutrino.cpp */
extern CRemoteControl * g_RemoteControl; /* neutrino.cpp */
extern cVideo *videoDecoder;
extern cAudio *audioDecoder;

extern cDemux *videoDemux;
extern cDemux *audioDemux;
extern cDemux *pcrDemux;

// dvbsub
//extern int dvbsub_init(int source);
extern int dvbsub_init();
extern int dvbsub_stop();
extern int dvbsub_close();
extern int dvbsub_start(int pid);
extern int dvbsub_pause();
//extern int dvbsub_getpid();
//extern void dvbsub_setpid(int pid);

// tuxtxt
//extern int  tuxtxt_init();
extern void tuxtxt_start(int tpid, int source);
//extern int  tuxtxt_stop();
//extern void tuxtxt_close();
//extern void tuxtx_pause_subtitle(bool pause, int source);
extern void tuxtx_stop_subtitle();
extern void tuxtx_set_pid(int pid, int page, const char * cc);
//extern int tuxtx_subtitle_running(int *pid, int *page, int *running);
extern int tuxtx_main(int pid, int page, int source);

//extern int tuner_to_scan;		//defined in scan_setup.cpp
extern CFrontend * live_fe;
extern t_channel_id live_channel_id;

// sectionsd config notifier
bool CSectionsdConfigNotifier::changeNotify(const neutrino_locale_t, void *)
{
	dprintf(DEBUG_NORMAL, "CSectionsdConfigNotifier::changeNotify\n");

        CNeutrinoApp::getInstance()->SendSectionsdConfig();
	
        return true;
}

// color setup notifier
bool CColorSetupNotifier::changeNotify(const neutrino_locale_t, void *)
{
	dprintf(DEBUG_NORMAL, "CColorSetupNotifier::changeNotify:\n");
	
	CFrameBuffer *frameBuffer = CFrameBuffer::getInstance();
	
	// head
	frameBuffer->paletteGenFade(COL_MENUHEAD, convertSetupColor2RGB(g_settings.menu_Head_red, g_settings.menu_Head_green, g_settings.menu_Head_blue), convertSetupColor2RGB(g_settings.menu_Head_Text_red, g_settings.menu_Head_Text_green, g_settings.menu_Head_Text_blue), 8, convertSetupAlpha2Alpha( g_settings.menu_Head_alpha ) );

	// menu content
	frameBuffer->paletteGenFade(COL_MENUCONTENT, convertSetupColor2RGB(g_settings.menu_Content_red, g_settings.menu_Content_green, g_settings.menu_Content_blue), convertSetupColor2RGB(g_settings.menu_Content_Text_red, g_settings.menu_Content_Text_green, g_settings.menu_Content_Text_blue), 8, convertSetupAlpha2Alpha(g_settings.menu_Content_alpha) );

	// menu content dark
	frameBuffer->paletteGenFade(COL_MENUCONTENTDARK, convertSetupColor2RGB(int(g_settings.menu_Content_red*0.6), int(g_settings.menu_Content_green*0.6), int(g_settings.menu_Content_blue*0.6)), convertSetupColor2RGB(g_settings.menu_Content_Text_red, g_settings.menu_Content_Text_green, g_settings.menu_Content_Text_blue), 8, convertSetupAlpha2Alpha(g_settings.menu_Content_alpha) );

	// menu content selected
	frameBuffer->paletteGenFade(COL_MENUCONTENTSELECTED, convertSetupColor2RGB(g_settings.menu_Content_Selected_red, g_settings.menu_Content_Selected_green, g_settings.menu_Content_Selected_blue), convertSetupColor2RGB(g_settings.menu_Content_Selected_Text_red, g_settings.menu_Content_Selected_Text_green, g_settings.menu_Content_Selected_Text_blue), 8, convertSetupAlpha2Alpha(g_settings.menu_Content_Selected_alpha) );

	// menu content inactiv
	frameBuffer->paletteGenFade(COL_MENUCONTENTINACTIVE, convertSetupColor2RGB(g_settings.menu_Content_inactive_red, g_settings.menu_Content_inactive_green, g_settings.menu_Content_inactive_blue), convertSetupColor2RGB(g_settings.menu_Content_inactive_Text_red, g_settings.menu_Content_inactive_Text_green, g_settings.menu_Content_inactive_Text_blue), 8, convertSetupAlpha2Alpha(g_settings.menu_Content_inactive_alpha) );

	// foot
	frameBuffer->paletteGenFade(COL_MENUFOOT, convertSetupColor2RGB(g_settings.menu_Foot_red, g_settings.menu_Foot_green, g_settings.menu_Foot_blue), convertSetupColor2RGB(g_settings.menu_Foot_Text_red, g_settings.menu_Foot_Text_green, g_settings.menu_Foot_Text_blue), 8, convertSetupAlpha2Alpha(g_settings.menu_Foot_alpha) );

	// infobar
	frameBuffer->paletteGenFade(COL_INFOBAR, convertSetupColor2RGB(g_settings.infobar_red, g_settings.infobar_green, g_settings.infobar_blue), convertSetupColor2RGB(g_settings.infobar_Text_red, g_settings.infobar_Text_green, g_settings.infobar_Text_blue), 8, convertSetupAlpha2Alpha(g_settings.infobar_alpha) );

	// infobar shadow
	frameBuffer->paletteGenFade(COL_INFOBAR_SHADOW, convertSetupColor2RGB(int(g_settings.infobar_red*0.4), int(g_settings.infobar_green*0.4), int(g_settings.infobar_blue*0.4)), convertSetupColor2RGB(g_settings.infobar_Text_red, g_settings.infobar_Text_green, g_settings.infobar_Text_blue), 8, convertSetupAlpha2Alpha(g_settings.infobar_alpha) );

	// infobar colored events
	frameBuffer->paletteGenFade(COL_COLORED_EVENTS_INFOBAR, convertSetupColor2RGB(g_settings.infobar_red, g_settings.infobar_green, g_settings.infobar_blue), convertSetupColor2RGB(g_settings.infobar_colored_events_red, g_settings.infobar_colored_events_green, g_settings.infobar_colored_events_blue), 8, convertSetupAlpha2Alpha(g_settings.infobar_alpha) );
				      
	// channellist colored events
	frameBuffer->paletteGenFade(COL_COLORED_EVENTS_CHANNELLIST, convertSetupColor2RGB(g_settings.menu_Content_red, g_settings.menu_Content_green, g_settings.menu_Content_blue), convertSetupColor2RGB(g_settings.infobar_colored_events_red, g_settings.infobar_colored_events_green, g_settings.infobar_colored_events_blue), 8, convertSetupAlpha2Alpha(g_settings.menu_Content_alpha) );

	// head info
	frameBuffer->paletteGenFade(COL_MENUHEAD_INFO, convertSetupColor2RGB(g_settings.menu_HeadInfo_red, g_settings.menu_HeadInfo_green, g_settings.menu_HeadInfo_blue), convertSetupColor2RGB(g_settings.menu_HeadInfo_Text_red, g_settings.menu_HeadInfo_Text_green, g_settings.menu_HeadInfo_Text_blue), 8, convertSetupAlpha2Alpha( g_settings.menu_HeadInfo_alpha ) );

	// foot info
	frameBuffer->paletteGenFade(COL_MENUFOOT_INFO, convertSetupColor2RGB(g_settings.menu_FootInfo_red, g_settings.menu_FootInfo_green, g_settings.menu_FootInfo_blue), convertSetupColor2RGB(g_settings.menu_FootInfo_Text_red, g_settings.menu_FootInfo_Text_green, g_settings.menu_FootInfo_Text_blue), 8, convertSetupAlpha2Alpha( g_settings.menu_FootInfo_alpha ) );

	frameBuffer->paletteSet();

	return false;
}

// txt/dvb sub change exec
int CSubtitleChangeExec::exec(CMenuTarget */*parent*/, const std::string & actionKey)
{
	dprintf(DEBUG_INFO, "CSubtitleChangeExec::exec: action %s\n", actionKey.c_str());
	
	if(actionKey == "off") 
	{
		// tuxtxt stop
		tuxtx_stop_subtitle(); //this kill sub thread
		
		// dvbsub stop
		dvbsub_stop();
		//dvbsub_close();
		
		return menu_return::RETURN_EXIT;
	}
	
	if(!strncmp(actionKey.c_str(), "DVB", 3)) 
	{
		char const * pidptr = strchr(actionKey.c_str(), ':');
		int pid = atoi(pidptr+1);
		
		// tuxtxt stop
		tuxtx_stop_subtitle();
		
		dvbsub_pause();
		dvbsub_start(pid);
	} 
	else 
	{
		char const * ptr = strchr(actionKey.c_str(), ':');
		ptr++;
		int pid = atoi(ptr);
		ptr = strchr(ptr, ':');
		ptr++;
		int page = strtol(ptr, NULL, 16);
		ptr = strchr(ptr, ':');
		ptr++;
		
		dprintf(DEBUG_NORMAL, "CSubtitleChangeExec::exec: TTX, pid %x page %x lang %s\n", pid, page, ptr);
		
		dvbsub_stop();
		
		tuxtx_stop_subtitle();
		
		tuxtx_set_pid(pid, page, ptr);
		
		// start tuxtxt
		tuxtx_main(pid, page, (live_fe)?live_fe->fenumber : 0 ); //FIXME
	}
	
        return menu_return::RETURN_EXIT;
}

// nvod change exec
int CNVODChangeExec::exec(CMenuTarget* parent, const std::string &actionKey)
{
	dprintf(DEBUG_INFO, "CNVODChangeExec exec: %s\n", actionKey.c_str());
	
	unsigned int sel = atoi(actionKey.c_str());
	g_RemoteControl->setSubChannel(sel);

	if(parent)
		parent->hide();
	
	g_InfoViewer->showSubchan();

	return menu_return::RETURN_EXIT;
}

// tuxtxt changer
extern int current_muted;
int CTuxtxtChangeExec::exec(CMenuTarget *parent, const std::string &actionKey)
{
	dprintf(DEBUG_INFO, "CTuxtxtChangeExec exec: %s\n", actionKey.c_str());

	if(parent)
		parent->hide();
	
	g_RCInput->clearRCMsg();

	CNeutrinoApp::getInstance()->StopSubtitles();
				
	tuxtx_stop_subtitle();

	tuxtx_main(g_RemoteControl->current_PIDs.PIDs.vtxtpid, 0, live_fe?live_fe->fenumber:0 );

	CFrameBuffer::getInstance()->paintBackground();

	CFrameBuffer::getInstance()->blit();
				
	g_RCInput->clearRCMsg();
				
	CNeutrinoApp::getInstance()->AudioMute(current_muted, true);

	CNeutrinoApp::getInstance()->StartSubtitles();

	return menu_return::RETURN_REPAINT;
}

// USERMENU
#define USERMENU_ITEM_OPTION_COUNT SNeutrinoSettings::ITEM_MAX
const CMenuOptionChooser::keyval USERMENU_ITEM_OPTIONS[USERMENU_ITEM_OPTION_COUNT] =
{
	{SNeutrinoSettings::ITEM_MOVIEPLAYER_TSMB, LOCALE_MOVIEPLAYER_RECORDS, NULL},    
        {SNeutrinoSettings::ITEM_MOVIEPLAYER_MB, LOCALE_MOVIEPLAYER_MOVIES, NULL},
        {SNeutrinoSettings::ITEM_TIMERLIST, LOCALE_TIMERLIST_NAME, NULL},
        {SNeutrinoSettings::ITEM_PLUGIN, LOCALE_USERMENU_ITEM_PLUGINS, NULL},
        {SNeutrinoSettings::ITEM_VTXT, LOCALE_USERMENU_ITEM_VTXT, NULL},
        {SNeutrinoSettings::ITEM_GAME, LOCALE_MAINMENU_GAMES, NULL},
        {SNeutrinoSettings::ITEM_OPKG, LOCALE_OPKG_MANAGER, NULL},
	{SNeutrinoSettings::ITEM_REMOTE, LOCALE_RCLOCK_MENUEADD, NULL}   
};

int CUserMenuMenu::exec(CMenuTarget *parent, const std::string &/*actionKey*/)
{
        if(parent)
                parent->hide();

        CMenuWidget menu(local, NEUTRINO_ICON_KEYBINDING);
	
        menu.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
        menu.addItem(new CMenuSeparator(CMenuSeparator::LINE));

        CStringInputSMS name(LOCALE_USERMENU_NAME, &g_settings.usermenu_text[button]);
        menu.addItem(new CMenuForwarder(LOCALE_USERMENU_NAME, true, g_settings.usermenu_text[button],&name));
        menu.addItem(new CMenuSeparator(CMenuSeparator::LINE));

        char text[10];
        for(int item = 0; item < SNeutrinoSettings::ITEM_MAX; item++)
        {
                snprintf(text, 10, "%d:", item);
                text[9] = 0;// terminate for sure
                
                menu.addItem( new CMenuOptionChooser(text, &g_settings.usermenu[button][item], USERMENU_ITEM_OPTIONS, USERMENU_ITEM_OPTION_COUNT,true, NULL, CRCInput::RC_nokey, "", true ));
        }

        menu.exec(NULL, "");

        return menu_return::RETURN_REPAINT;
}


