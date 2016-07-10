/*
	Neutrino-GUI  -   DBoxII-Project

	$id: audio_setup.cpp 2016.01.02 19:38:30 mohousch $
	
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

#include <gui/audio_setup.h>


#include <system/debug.h>
#include <system/setting_helpers.h>
#include <system/helpers.h>

#include <audio_cs.h>
#include <video_cs.h>


extern cVideo 		* videoDecoder;		//libcoolstream (video_cs.cpp)
extern cAudio 		* audioDecoder;		//libcoolstream (audio_cs.cpp)

#define OPTIONS_OFF0_ON1_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_OFF0_ON1_OPTIONS[OPTIONS_OFF0_ON1_OPTION_COUNT] =
{
        { 0, LOCALE_OPTIONS_OFF, NULL },
        { 1, LOCALE_OPTIONS_ON, NULL }
};

#define AUDIOMENU_ANALOGOUT_OPTION_COUNT 3
const CMenuOptionChooser::keyval AUDIOMENU_ANALOGOUT_OPTIONS[AUDIOMENU_ANALOGOUT_OPTION_COUNT] =
{
	{ 0, LOCALE_AUDIOMENU_STEREO, NULL },
	{ 1, LOCALE_AUDIOMENU_MONOLEFT, NULL },
	{ 2, LOCALE_AUDIOMENU_MONORIGHT, NULL }
};

#if defined (PLATFORM_COOLSTREAM)
#define AUDIOMENU_AVSYNC_OPTION_COUNT 3
const CMenuOptionChooser::keyval AUDIOMENU_AVSYNC_OPTIONS[AUDIOMENU_AVSYNC_OPTION_COUNT] =
{
        { 0, LOCALE_OPTIONS_OFF, NULL },
        { 1, LOCALE_OPTIONS_ON, NULL },
        { 2, LOCALE_AUDIOMENU_AVSYNC_AM, NULL }
};
#else
#define AUDIOMENU_AVSYNC_OPTION_COUNT 3
const CMenuOptionChooser::keyval AUDIOMENU_AVSYNC_OPTIONS[AUDIOMENU_AVSYNC_OPTION_COUNT] =
{
        { AVSYNC_OFF, LOCALE_OPTIONS_OFF, NULL },
        { AVSYNC_ON, LOCALE_OPTIONS_ON, NULL },
        { AVSYNC_AM, LOCALE_AUDIOMENU_AVSYNC_AM, NULL }
};
#endif

// ac3
#if !defined (PLATFORM_COOLSTREAM)
#define AC3_OPTION_COUNT 2
const CMenuOptionChooser::keyval AC3_OPTIONS[AC3_OPTION_COUNT] =
{
	{ AC3_DOWNMIX, NONEXISTANT_LOCALE, "downmix" },
	{ AC3_PASSTHROUGH, NONEXISTANT_LOCALE, "passthrough" }
};

#define AUDIODELAY_OPTION_COUNT 9
const CMenuOptionChooser::keyval AUDIODELAY_OPTIONS[AUDIODELAY_OPTION_COUNT] =
{
	{ -1000, NONEXISTANT_LOCALE, "-1000" },
	{ -750, NONEXISTANT_LOCALE, "-750" },
	{ -500, NONEXISTANT_LOCALE, "-500" },
	{ -250, NONEXISTANT_LOCALE, "-250" },
	{ 0, NONEXISTANT_LOCALE, "0" },
	{ 250, NONEXISTANT_LOCALE, "250" },
	{ 500, NONEXISTANT_LOCALE, "500" },
	{ 750, NONEXISTANT_LOCALE, "750" },
	{ 1000, NONEXISTANT_LOCALE, "1000" }
};
#endif

CAudioSettings::CAudioSettings()
{
	audioSetupNotifier = new CAudioSetupNotifier;
}

CAudioSettings *CAudioSettings::getInstance()
{
	static CAudioSettings *audioSettings = NULL;

	if(!audioSettings)
	{
		audioSettings = new CAudioSettings();
		dprintf(DEBUG_NORMAL, "CAudioSettings::getInstance: Instance created\n");
	}
	
	return audioSettings;
}

CAudioSettings::~CAudioSettings()
{
	delete audioSetupNotifier;
}

int CAudioSettings::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CAudioSettings::exec: actionKey: %s\n", actionKey.c_str());
	
	int ret = menu_return::RETURN_REPAINT;
	
	if(parent)
		parent->hide();
	
	if(actionKey == "savesettings")
	{
		CNeutrinoApp::getInstance()->exec(NULL, "savesettings");
		
		return ret;
	}
	
	showMenu();
	
	return ret;
}

void CAudioSettings::showMenu()
{
	dprintf(DEBUG_NORMAL, "CAudioSettings::showMenu:\n");
	
	CMenuWidget audioSettings(LOCALE_AUDIOMENU_HEAD, NEUTRINO_ICON_AUDIO);
	
	int shortcutAudio = 1;
	
	// intros
	audioSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	audioSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// save settings
	audioSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	audioSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	// analog output
	audioSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOMENU_ANALOGOUT, &g_settings.audio_AnalogMode, AUDIOMENU_ANALOGOUT_OPTIONS, AUDIOMENU_ANALOGOUT_OPTION_COUNT, true, audioSetupNotifier, CRCInput::convertDigitToKey(shortcutAudio++), "", true ));

#if !defined (PLATFORM_COOLSTREAM)	
	// hdmi-dd
	audioSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOMENU_HDMI_DD, &g_settings.hdmi_dd, AC3_OPTIONS, AC3_OPTION_COUNT, true, audioSetupNotifier, CRCInput::convertDigitToKey(shortcutAudio++) ));	
#endif	

	// A/V sync
	audioSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOMENU_AVSYNC, &g_settings.avsync, AUDIOMENU_AVSYNC_OPTIONS, AUDIOMENU_AVSYNC_OPTION_COUNT, true, audioSetupNotifier, CRCInput::convertDigitToKey(shortcutAudio++), "", true ));
	
#if !defined (PLATFORM_COOLSTREAM)	
	// ac3 delay
	audioSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOMENU_AC3_DELAY, &g_settings.ac3_delay, AUDIODELAY_OPTIONS, AUDIODELAY_OPTION_COUNT, true, audioSetupNotifier, CRCInput::convertDigitToKey(shortcutAudio++) ));
	
	// pcm delay
	audioSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOMENU_PCM_DELAY, &g_settings.pcm_delay, AUDIODELAY_OPTIONS, AUDIODELAY_OPTION_COUNT, true, audioSetupNotifier, CRCInput::convertDigitToKey(shortcutAudio++) ));
#endif	
	
	// pref sub/lang
	audioSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_AUDIOMENU_PREF_LANG_HEAD));
	
	// auto ac3 
	CMenuOptionChooser * a1 = new CMenuOptionChooser(LOCALE_AUDIOMENU_DOLBYDIGITAL, &g_settings.audio_DolbyDigital, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, g_settings.auto_lang, audioSetupNotifier );
	
	// audiolang
	CMenuOptionStringChooser * audiolangSelect[3];
	
	for(int i = 0; i < 3; i++) 
	{
		audiolangSelect[i] = new CMenuOptionStringChooser(LOCALE_AUDIOMENU_PREF_LANG, g_settings.pref_lang[i], g_settings.auto_lang, NULL, CRCInput::RC_nokey, "", true);
		
		audiolangSelect[i]->addOption("");
		std::map<std::string, std::string>::const_iterator it;
		for(it = iso639rev.begin(); it != iso639rev.end(); it++) 
			audiolangSelect[i]->addOption(it->first.c_str());
	}
	
	CAutoAudioNotifier * autoAudioNotifier = new CAutoAudioNotifier(a1, audiolangSelect[0], audiolangSelect[1], audiolangSelect[2]);
	
	// auto lang
	audioSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOMENU_AUTO_LANG, &g_settings.auto_lang, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, autoAudioNotifier));
	
	// ac3
	audioSettings.addItem(a1);
	
	// lang
	for(int i = 0; i < 3; i++) 
		audioSettings.addItem(audiolangSelect[i]);
	
	// sublang
	audioSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_AUDIOMENU_PREF_SUBS_HEAD));
	
	CMenuOptionStringChooser * sublangSelect[3];
	for(int i = 0; i < 3; i++) 
	{
		sublangSelect[i] = new CMenuOptionStringChooser(LOCALE_AUDIOMENU_PREF_SUBS, g_settings.pref_subs[i], g_settings.auto_subs, NULL, CRCInput::RC_nokey, "", true);
		std::map<std::string, std::string>::const_iterator it;
		
		sublangSelect[i]->addOption("");
		for(it = iso639rev.begin(); it != iso639rev.end(); it++) 
			sublangSelect[i]->addOption(it->first.c_str());
	}
	
	CSubLangSelectNotifier * subLangSelectNotifier = new CSubLangSelectNotifier(sublangSelect[0], sublangSelect[1], sublangSelect[2]);
	
	// auto sublang
	audioSettings.addItem(new CMenuOptionChooser(LOCALE_AUDIOMENU_AUTO_SUBS, &g_settings.auto_subs, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, subLangSelectNotifier));
	
	// sublang
	for(int i = 0; i < 3; i++) 
		audioSettings.addItem(sublangSelect[i]);
	
	audioSettings.exec(NULL, "");
	audioSettings.hide();

	delete subLangSelectNotifier;
	delete autoAudioNotifier;
}

bool CAudioSetupNotifier::changeNotify(const neutrino_locale_t OptionName, void *)
{
	dprintf(DEBUG_NORMAL, "CAudioSetupNotifier::changeNotify\n");

	if (ARE_LOCALES_EQUAL(OptionName, LOCALE_AUDIOMENU_ANALOGOUT)) 
	{
		//g_Zapit->setAudioMode(g_settings.audio_AnalogMode);
		if(audioDecoder) 
				audioDecoder->setChannel(g_settings.audio_AnalogMode);
	} 
	else if (ARE_LOCALES_EQUAL(OptionName, LOCALE_AUDIOMENU_HDMI_DD)) 
	{
		if(audioDecoder)
			audioDecoder->SetHdmiDD(g_settings.hdmi_dd );
	}
	else if (ARE_LOCALES_EQUAL(OptionName, LOCALE_AUDIOMENU_AVSYNC)) 
	{
#if defined (PLATFORM_COOLSTREAM)
		videoDecoder->SetSyncMode((AVSYNC_TYPE)g_settings.avsync);
		audioDecoder->SetSyncMode((AVSYNC_TYPE)g_settings.avsync);
		videoDemux->SetSyncMode((AVSYNC_TYPE)g_settings.avsync);
		audioDemux->SetSyncMode((AVSYNC_TYPE)g_settings.avsync);
		pcrDemux->SetSyncMode((AVSYNC_TYPE)g_settings.avsync);
#else
		if(videoDecoder)
			videoDecoder->SetSyncMode(g_settings.avsync);			
		
		if(audioDecoder)
			audioDecoder->SetSyncMode(g_settings.avsync);
#endif			
	}
#if !defined (PLATFORM_COOLSTREAM)	
	else if( ARE_LOCALES_EQUAL(OptionName, LOCALE_AUDIOMENU_AC3_DELAY) )
	{
		if(audioDecoder)
			audioDecoder->setHwAC3Delay(g_settings.ac3_delay);
	}
	else if( ARE_LOCALES_EQUAL(OptionName, LOCALE_AUDIOMENU_PCM_DELAY) )
	{
		if(audioDecoder)
			audioDecoder->setHwPCMDelay(g_settings.pcm_delay);
	}
#endif	

	return true;
}

CAutoAudioNotifier::CAutoAudioNotifier(CMenuItem * item1, CMenuItem * item2, CMenuItem * item3, CMenuItem * item4)
{
	toDisable[0] = item1;
	toDisable[1] = item2;
	toDisable[2] = item3;
	toDisable[3] = item4;
}

bool CAutoAudioNotifier::changeNotify(const neutrino_locale_t, void *)
{
	// only ac3
	toDisable[0]->setActive(g_settings.auto_lang);
	toDisable[1]->setActive(g_settings.auto_lang);
	toDisable[2]->setActive(g_settings.auto_lang);
	toDisable[3]->setActive(g_settings.auto_lang);
	
	return true;
}

// sublang select notifier
CSubLangSelectNotifier::CSubLangSelectNotifier(CMenuItem * item1, CMenuItem * item2, CMenuItem * item3)
{
	toDisable[0] = item1;
	toDisable[1] = item2;
	toDisable[2] = item3;
}

bool CSubLangSelectNotifier::changeNotify(const neutrino_locale_t, void *)
{
	// only ac3
	toDisable[0]->setActive(g_settings.auto_subs);
	toDisable[1]->setActive(g_settings.auto_subs);
	toDisable[2]->setActive(g_settings.auto_subs);
	
	return true;
}





