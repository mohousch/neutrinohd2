/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: audio_select.cpp 2013/10/12 mohousch Exp $

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <global.h>
#include <neutrino.h>
#include <gui/widget/icons.h>
#include <gui/widget/menue.h>

#include <gui/audio_select.h>

#include <system/debug.h>

#include <audio_cs.h>


extern CRemoteControl * g_RemoteControl; 		/* defined neutrino.cpp */
extern CAudioSetupNotifier * audioSetupNotifier;	/* defined neutrino.cpp */

// volume conf
extern CAudioSetupNotifierVolPercent * audioSetupNotifierVolPercent;

// dvbsub
extern int dvbsub_getpid();				// defined in libdvbsub
//extern int dvbsub_getpid(int * pid, int * running);	// defined in libdvbsub

// tuxtxt
extern int tuxtx_subtitle_running(int * pid, int * page, int * running);


// -- this is a copy from neutrino.cpp!!
/* option off0_on1 */
#define OPTIONS_OFF0_ON1_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_OFF0_ON1_OPTIONS[OPTIONS_OFF0_ON1_OPTION_COUNT] =
{
        { 0, LOCALE_OPTIONS_OFF, NULL },
        { 1, LOCALE_OPTIONS_ON, NULL  }
}; 

#define AUDIOMENU_ANALOGOUT_OPTION_COUNT 3
const CMenuOptionChooser::keyval AUDIOMENU_ANALOGOUT_OPTIONS[AUDIOMENU_ANALOGOUT_OPTION_COUNT] =
{
	{ 0, LOCALE_AUDIOMENU_STEREO, NULL   },
	{ 1, LOCALE_AUDIOMENU_MONOLEFT, NULL  },
	{ 2, LOCALE_AUDIOMENU_MONORIGHT, NULL }
};

// ac3
#if !defined (PLATFORM_COOLSTREAM)
#define AC3_OPTION_COUNT 2
const CMenuOptionChooser::keyval AC3_OPTIONS[AC3_OPTION_COUNT] =
{
	{ AC3_PASSTHROUGH, NONEXISTANT_LOCALE, "passthrough" },
	{ AC3_DOWNMIX, NONEXISTANT_LOCALE, "downmix" }
};
#endif

//
int CAPIDChangeExec::exec(CMenuTarget */*parent*/, const std::string & actionKey)
{
	dprintf(DEBUG_NORMAL, "CAPIDChangeExec exec: %s\n", actionKey.c_str());

	unsigned int sel = atoi(actionKey.c_str());
	
	if (g_RemoteControl->current_PIDs.PIDs.selected_apid != sel )
	{
		g_RemoteControl->setAPID(sel);
	}

	return menu_return::RETURN_EXIT;
}

//
int CAudioSelectMenuHandler::exec(CMenuTarget * parent, const std::string &/*actionKey*/)
{
	dprintf(DEBUG_NORMAL, "CAudioSelectMenuHandler::exec:\n");

	int res = menu_return::RETURN_REPAINT;

	if (parent) 
		parent->hide();

	doMenu();

	return res;
}

//
int CAudioSelectMenuHandler::doMenu()
{
	dprintf(DEBUG_NORMAL, "CAudioSelectMenuHandler::doMenu\n");

	CMenuWidget AudioSelector(LOCALE_APIDSELECTOR_HEAD, NEUTRINO_ICON_AUDIO);
	
	unsigned int count;
	
	CAPIDChangeExec APIDChanger;
	CSubtitleChangeExec SubtitleChanger;
	
	// audio pids
	for(count = 0; count < g_RemoteControl->current_PIDs.APIDs.size(); count++ ) 
	{
		char apid[5];
		sprintf(apid, "%d", count);
		AudioSelector.addItem(new CMenuForwarder(g_RemoteControl->current_PIDs.APIDs[count].desc, true, NULL, &APIDChanger, apid, CRCInput::convertDigitToKey(count + 1)), (count == g_RemoteControl->current_PIDs.PIDs.selected_apid));
	}

	if(g_RemoteControl->current_PIDs.APIDs.size())
		AudioSelector.addItem(new CMenuSeparator(CMenuSeparator::LINE));

	// analogue output
	AudioSelector.addItem(new CMenuOptionChooser(LOCALE_AUDIOMENU_ANALOGOUT, &g_settings.audio_AnalogMode, AUDIOMENU_ANALOGOUT_OPTIONS, AUDIOMENU_ANALOGOUT_OPTION_COUNT, true, audioSetupNotifier, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	
	// ac3
#if !defined (PLATFORM_COOLSTREAM)	
	AudioSelector.addItem(new CMenuOptionChooser(LOCALE_AUDIOMENU_HDMI_DD, &g_settings.hdmi_dd, AC3_OPTIONS, AC3_OPTION_COUNT, true, audioSetupNotifier, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN ));
#endif

	//dvb/tuxtxt subs
	CChannelList * channelList = CNeutrinoApp::getInstance()->channelList;
	int curnum = channelList->getActiveChannelNumber();
	CZapitChannel * cc = channelList->getChannel(curnum);

	bool sep_added = false;
	if(cc) 
	{
		for (int i = 0 ; i < (int)cc->getSubtitleCount() ; ++i) 
		{
			CZapitAbsSub* s = cc->getChannelSub(i);
			
			//dvbsub
			if (s->thisSubType == CZapitAbsSub::DVB) 
			{
				CZapitDVBSub* sd = reinterpret_cast<CZapitDVBSub*>(s);
				printf("[CAudioSelectMenuHandler] adding DVB subtitle %s pid 0x%x\n", sd->ISO639_language_code.c_str(), sd->pId);
				if(!sep_added) 
				{
					sep_added = true;
					AudioSelector.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_SUBTITLES_HEAD));
				}
				char spid[10];
				//int pid = sd->pId;
				snprintf(spid,sizeof(spid), "DVB:%d", sd->pId);
				char item[64];
				//snprintf(item,sizeof(item), "DVB: %s (pid %x)", sd->ISO639_language_code.c_str(), sd->pId);
				snprintf(item, sizeof(item), "DVB: %s", sd->ISO639_language_code.c_str());
				AudioSelector.addItem(new CMenuForwarder(item, sd->pId != dvbsub_getpid() /* !dvbsub_getpid(&pid, NULL)*/, NULL, &SubtitleChanger, spid, CRCInput::convertDigitToKey(++count)));
			}
			
			//txtsub
			if (s->thisSubType == CZapitAbsSub::TTX) 
			{
				CZapitTTXSub* sd = reinterpret_cast<CZapitTTXSub*>(s);
				printf("[CAudioSelectMenuHandler] adding TTX subtitle %s pid 0x%x mag 0x%X page 0x%x\n", sd->ISO639_language_code.c_str(), sd->pId, sd->teletext_magazine_number, sd->teletext_page_number);
				if(!sep_added) 
				{
					sep_added = true;
					AudioSelector.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_SUBTITLES_HEAD));
				}
				char spid[64];
				int page = ((sd->teletext_magazine_number & 0xFF) << 8) | sd->teletext_page_number;
				int pid = sd->pId;
				snprintf(spid,sizeof(spid), "TTX:%d:%03X:%s", sd->pId, page, sd->ISO639_language_code.c_str()); 
				char item[64];
				//snprintf(item, sizeof(item), "TTX: %s (pid %x page %03X)", sd->ISO639_language_code.c_str(), sd->pId, page);
				snprintf(item, sizeof(item), "TTX: %s", sd->ISO639_language_code.c_str());
				AudioSelector.addItem(new CMenuForwarder(item,  !tuxtx_subtitle_running(&pid, &page, NULL), NULL, &SubtitleChanger, spid, CRCInput::convertDigitToKey(++count)));
			}
		}
		
		if(sep_added) 
		{
			AudioSelector.addItem(new CMenuSeparator(CMenuSeparator::LINE));
			AudioSelector.addItem(new CMenuForwarder(LOCALE_SUBTITLES_STOP, true, NULL, &SubtitleChanger, "off", CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW ));
		}

	}
	
	// volume percent
	int percent[g_RemoteControl->current_PIDs.APIDs.size()];
	
	for(count = 0; count < g_RemoteControl->current_PIDs.APIDs.size(); count++ ) 
	{
		g_Zapit->getVolumePercent((unsigned int *) &percent[count], 0, g_RemoteControl->current_PIDs.APIDs[count].pid);
		int is_active = count == g_RemoteControl->current_PIDs.PIDs.selected_apid;
		
		if(!sep_added) 
		{
			sep_added = true;
			AudioSelector.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_AUDIOMENU_VOLUME_ADJUST));
		}
		
		AudioSelector.addItem(new CMenuOptionNumberChooser(NONEXISTANT_LOCALE, &percent[count],
			is_active,
			0, 100, audioSetupNotifierVolPercent, 0, 0, NONEXISTANT_LOCALE,
			g_RemoteControl->current_PIDs.APIDs[count].desc));
	}

	return AudioSelector.exec(NULL, "");
}
