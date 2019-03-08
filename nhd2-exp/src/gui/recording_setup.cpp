/*
	Neutrino-GUI  -   DBoxII-Project

	$id: recording_setup.cpp 2016.01.02 21:43:30 mohousch $
	
	Copyright (C) 2001 Steffen Hehn 'McClean'
	and some other guys
	Homepage: http://dbox.cyberphoria.org/

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

#include <stdio.h> 

#include <gui/widget/hintbox.h>
#include <gui/widget/stringinput.h>

#include <gui/filebrowser.h>
#include <gui/recording_setup.h>

#include <system/debug.h>
#include <system/setting_helpers.h>
#include <system/helpers.h>


#define OPTIONS_OFF0_ON1_OPTION_COUNT 2
const keyval OPTIONS_OFF0_ON1_OPTIONS[OPTIONS_OFF0_ON1_OPTION_COUNT] =
{
        { 0, LOCALE_OPTIONS_OFF, NULL },
        { 1, LOCALE_OPTIONS_ON, NULL }
};

#define MESSAGEBOX_NO_YES_OPTION_COUNT 2
const keyval MESSAGEBOX_NO_YES_OPTIONS[MESSAGEBOX_NO_YES_OPTION_COUNT] =
{
	{ 0, LOCALE_MESSAGEBOX_NO, NULL },
	{ 1, LOCALE_MESSAGEBOX_YES, NULL }
};

// recording settings
extern char recDir[255];			// defined in neutrino.cpp
extern char timeshiftDir[255];			// defined in neutrino.cpp
extern bool autoshift;				// defined in neutrino.cpp
extern int startAutoRecord(bool addTimer);	// defined in neutrino.cpp
extern void stopAutoRecord();			// defined in neutrino.cpp

CRecordingSettings::CRecordingSettings()
{
}

CRecordingSettings::~CRecordingSettings()
{
}

int CRecordingSettings::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CRecordingSettings::exec: actionKey: %s\n", actionKey.c_str());
	
	int ret = menu_return::RETURN_REPAINT;
	
	if(parent)
		parent->hide();
	
	if(actionKey == "savesettings")
	{
		CNeutrinoApp::getInstance()->exec(NULL, "savesettings");
		
		return ret;
	}
	else if(actionKey == "recording")
	{
		CHintBox * hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_MAINSETTINGS_SAVESETTINGSNOW_HINT)); // UTF-8
		hintBox->paint();
		
		CNeutrinoApp::getInstance()->setupRecordingDevice();
		
		hintBox->hide();
		delete hintBox;
		hintBox = NULL;
		
		return ret;
	}
	else if(actionKey == "recordingdir")
	{
		if(parent)
			parent->hide();
		
		CFileBrowser b;
		b.Dir_Mode = true;

		if (b.exec(g_settings.network_nfs_recordingdir)) 
		{
			const char * newdir = b.getSelectedFile()->Name.c_str();
			dprintf(DEBUG_NORMAL, "CRecordingSettings::exec: New recordingdir: selected %s\n", newdir);

			if(check_dir(newdir))
				printf("CRecordingSettings::exec: Wrong/unsupported recording dir %s\n", newdir);
			else
			{
				strncpy(g_settings.network_nfs_recordingdir, b.getSelectedFile()->Name.c_str(), sizeof(g_settings.network_nfs_recordingdir)-1 );
				
				dprintf(DEBUG_NORMAL, "CRecordingSettings::exec: New recordingdir: %s\n", g_settings.network_nfs_recordingdir);
				
				sprintf(timeshiftDir, "%s/.timeshift", g_settings.network_nfs_recordingdir);
					
				safe_mkdir(timeshiftDir);
				dprintf(DEBUG_NORMAL, "CRecordingSettings::exec: New timeshift dir: %s\n", timeshiftDir);
			}
		}
		
		return ret;
	}
	
	showMenu();
	
	return ret;
}

bool CRecordingSettings::changeNotify(const neutrino_locale_t OptionName, void */*data*/)
{
	dprintf(DEBUG_NORMAL, "CRecordingSettings::changeNotify:\n");
	
	if(ARE_LOCALES_EQUAL(OptionName, LOCALE_EXTRA_AUTO_TIMESHIFT)) 
	{	  
		if(g_settings.auto_timeshift)
			startAutoRecord(true);
		else
		{
			if(autoshift) 
			{
				stopAutoRecord();
				
				CNeutrinoApp::getInstance()->recordingstatus = 0;
				CNeutrinoApp::getInstance()->timeshiftstatus = 0;
			}
		}
	
		return true;
	}

	return false;
}

void CRecordingSettings::showMenu()
{
	dprintf(DEBUG_NORMAL, "CRecordingSettings::showMenu:\n");
	
	CMenuWidget recordingSettings(LOCALE_RECORDINGMENU_HEAD, NEUTRINO_ICON_RECORDING );
	
	int rec_pre = 0;
	int rec_post = 0;
	
	g_Timerd->getRecordingSafety(rec_pre, rec_post);

	sprintf(g_settings.record_safety_time_before, "%02d", rec_pre/60);
	sprintf(g_settings.record_safety_time_after, "%02d", rec_post/60);

	CRecordingSafetyNotifier *RecordingSafetyNotifier = new CRecordingSafetyNotifier;

	//safety time befor
	CStringInput * timerBefore = new CStringInput(LOCALE_TIMERSETTINGS_RECORD_SAFETY_TIME_BEFORE, g_settings.record_safety_time_before, 2, LOCALE_TIMERSETTINGS_RECORD_SAFETY_TIME_BEFORE_HINT_1, LOCALE_TIMERSETTINGS_RECORD_SAFETY_TIME_BEFORE_HINT_2,"0123456789 ", RecordingSafetyNotifier);
	CMenuForwarder *fTimerBefore = new CMenuForwarder(LOCALE_TIMERSETTINGS_RECORD_SAFETY_TIME_BEFORE, true, g_settings.record_safety_time_before, timerBefore);

	//safety time after
	CStringInput * timerAfter = new CStringInput(LOCALE_TIMERSETTINGS_RECORD_SAFETY_TIME_AFTER, g_settings.record_safety_time_after, 2, LOCALE_TIMERSETTINGS_RECORD_SAFETY_TIME_AFTER_HINT_1, LOCALE_TIMERSETTINGS_RECORD_SAFETY_TIME_AFTER_HINT_2,"0123456789 ", RecordingSafetyNotifier);
	CMenuForwarder *fTimerAfter = new CMenuForwarder(LOCALE_TIMERSETTINGS_RECORD_SAFETY_TIME_AFTER, true, g_settings.record_safety_time_after, timerAfter);

	//audiopids
	g_settings.recording_audio_pids_std = ( g_settings.recording_audio_pids_default & TIMERD_APIDS_STD ) ? 1 : 0 ;
	g_settings.recording_audio_pids_alt = ( g_settings.recording_audio_pids_default & TIMERD_APIDS_ALT ) ? 1 : 0 ;
	g_settings.recording_audio_pids_ac3 = ( g_settings.recording_audio_pids_default & TIMERD_APIDS_AC3 ) ? 1 : 0 ;
	
	CRecAPIDSettingsNotifier * an = new CRecAPIDSettingsNotifier;

	//default
	CMenuOptionChooser* aoj1 = new CMenuOptionChooser(LOCALE_RECORDINGMENU_APIDS_STD, &g_settings.recording_audio_pids_std, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, an);

	//alt
	CMenuOptionChooser* aoj2 = new CMenuOptionChooser(LOCALE_RECORDINGMENU_APIDS_ALT, &g_settings.recording_audio_pids_alt, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, an);

	//ac3
	CMenuOptionChooser* aoj3 = new CMenuOptionChooser(LOCALE_RECORDINGMENU_APIDS_AC3, &g_settings.recording_audio_pids_ac3, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true, an);

	//epg in name format
	CMenuOptionChooser* oj11 = new CMenuOptionChooser(LOCALE_RECORDINGMENU_EPG_FOR_FILENAME, &g_settings.recording_epg_for_filename, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true);

	// save in channeldir
	CMenuOptionChooser* oj13 = new CMenuOptionChooser(LOCALE_RECORDINGMENU_SAVE_IN_CHANNELDIR, &g_settings.recording_save_in_channeldir, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true);

	//RecDir
	CMenuForwarder* fRecDir = new CMenuForwarder(LOCALE_RECORDINGMENU_DEFDIR, true, g_settings.network_nfs_recordingdir, this, "recordingdir");

	// intros
	recordingSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	recordingSettings.addItem( new CMenuSeparator(LINE) );
	
	// save settings
	recordingSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", RC_red, NEUTRINO_ICON_BUTTON_RED));
	recordingSettings.addItem(new CMenuForwarder(LOCALE_RECORDINGMENU_SETUPNOW, true, NULL, this, "recording", RC_green, NEUTRINO_ICON_BUTTON_GREEN));

	recordingSettings.addItem(new CMenuSeparator(LINE | STRING, g_Locale->getText(LOCALE_TIMERSETTINGS_SEPARATOR)));
	recordingSettings.addItem(fTimerBefore);
	recordingSettings.addItem(fTimerAfter);

	//apids
	recordingSettings.addItem(new CMenuSeparator(LINE | STRING, g_Locale->getText(LOCALE_RECORDINGMENU_APIDS)));
	recordingSettings.addItem(aoj1);
	recordingSettings.addItem(aoj2);
	recordingSettings.addItem(aoj3);

	//
	recordingSettings.addItem(new CMenuSeparator(LINE));
	//epg in name format
	recordingSettings.addItem(oj11);
	
	// save in channeldir
	recordingSettings.addItem(oj13);

	recordingSettings.addItem(new CMenuSeparator(LINE));

	//recdir
	recordingSettings.addItem(fRecDir);
	
	// timeshift
	recordingSettings.addItem(new CMenuSeparator(LINE | STRING, g_Locale->getText(LOCALE_EXTRA_TIMESHIFT)));
	
	// record time
	recordingSettings.addItem(new CMenuOptionNumberChooser(LOCALE_EXTRA_RECORD_TIME, &g_settings.record_hours, true, 1, 24, NULL) );

	// timeshift
	if (recDir != NULL)
	{
		// permanent timeshift
		recordingSettings.addItem(new CMenuOptionChooser(LOCALE_EXTRA_AUTO_TIMESHIFT, &g_settings.auto_timeshift, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, this));
	}
	
	recordingSettings.exec(NULL, "");
	recordingSettings.hide();
}

// recording safety notifier
bool CRecordingSafetyNotifier::changeNotify(const neutrino_locale_t, void *)
{
	g_Timerd->setRecordingSafety(atoi(g_settings.record_safety_time_before)*60, atoi(g_settings.record_safety_time_after)*60);

   	return true;
}

// rec apids notifier
bool CRecAPIDSettingsNotifier::changeNotify(const neutrino_locale_t, void *)
{
	g_settings.recording_audio_pids_default = ( (g_settings.recording_audio_pids_std ? TIMERD_APIDS_STD : 0) | (g_settings.recording_audio_pids_alt ? TIMERD_APIDS_ALT : 0) | (g_settings.recording_audio_pids_ac3 ? TIMERD_APIDS_AC3 : 0));

	return true;
}



