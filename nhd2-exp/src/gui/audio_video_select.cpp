/*
	$Id: audio_video_select.cpp 2015/07/26 10:45:30 mohousch Exp $

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

#include <stdio.h>

#include <algorithm>    // std::sort
#include <fstream>
#include <iostream>

#include <global.h>
#include <neutrino.h>

#include <driver/screen_max.h>

#include <system/debug.h>
#include <system/helpers.h>

/* libdvbapi */
#include <playback_cs.h>
#include <video_cs.h>
#include <audio_cs.h>

#include <gui/audio_video_select.h>
#include <gui/video_setup.h>
#include <gui/audio_setup.h>



extern cPlayback *playback;

//
unsigned short apids[10];
unsigned short ac3flags[10];
unsigned short numpida = 0;
unsigned short vpid = 0;
unsigned short vtype = 0;
std::string language[10];

unsigned int currentapid = 0;
unsigned int currentac3 = 0;

unsigned int ac3state = CInfoViewer::NO_AC3;

// aspect ratio
#if defined (__sh__)
#define VIDEOMENU_VIDEORATIO_OPTION_COUNT 2
const keyval VIDEOMENU_VIDEORATIO_OPTIONS[VIDEOMENU_VIDEORATIO_OPTION_COUNT] =
{
	{ ASPECTRATIO_43, LOCALE_VIDEOMENU_VIDEORATIO_43, NULL },
	{ ASPECTRATIO_169, LOCALE_VIDEOMENU_VIDEORATIO_169, NULL }
};
#else
#define VIDEOMENU_VIDEORATIO_OPTION_COUNT 3
const keyval VIDEOMENU_VIDEORATIO_OPTIONS[VIDEOMENU_VIDEORATIO_OPTION_COUNT] =
{
	{ ASPECTRATIO_43, LOCALE_VIDEOMENU_VIDEORATIO_43, NULL },
	{ ASPECTRATIO_169, LOCALE_VIDEOMENU_VIDEORATIO_169, NULL },
	{ ASPECTRATIO_AUTO, NONEXISTANT_LOCALE, "Auto" }
};
#endif

// policy
#if defined (__sh__)
/*
letterbox 
panscan 
non 
bestfit
*/
#define VIDEOMENU_VIDEOFORMAT_OPTION_COUNT 4
const keyval VIDEOMENU_VIDEOFORMAT_OPTIONS[VIDEOMENU_VIDEOFORMAT_OPTION_COUNT] = 
{
	{ VIDEOFORMAT_LETTERBOX, LOCALE_VIDEOMENU_LETTERBOX, NULL },
	{ VIDEOFORMAT_PANSCAN, LOCALE_VIDEOMENU_PANSCAN, NULL },
	{ VIDEOFORMAT_FULLSCREEN, LOCALE_VIDEOMENU_FULLSCREEN, NULL },
	{ VIDEOFORMAT_PANSCAN2, LOCALE_VIDEOMENU_PANSCAN2, NULL }
};
#else
// giga/generic
/*
letterbox 
panscan 
bestfit 
nonlinear
*/
#define VIDEOMENU_VIDEOFORMAT_OPTION_COUNT 4
const keyval VIDEOMENU_VIDEOFORMAT_OPTIONS[VIDEOMENU_VIDEOFORMAT_OPTION_COUNT] = 
{
	{ VIDEOFORMAT_LETTERBOX, LOCALE_VIDEOMENU_LETTERBOX, NULL },
	{ VIDEOFORMAT_PANSCAN, LOCALE_VIDEOMENU_PANSCAN, NULL },
	{ VIDEOFORMAT_PANSCAN2, LOCALE_VIDEOMENU_PANSCAN2, NULL },
	{ VIDEOFORMAT_FULLSCREEN, LOCALE_VIDEOMENU_FULLSCREEN, NULL }
};
#endif

#if !defined (PLATFORM_COOLSTREAM)
#define AC3_OPTION_COUNT 2
const keyval AC3_OPTIONS[AC3_OPTION_COUNT] =
{
	{ AC3_PASSTHROUGH, NONEXISTANT_LOCALE, "passthrough" },
	{ AC3_DOWNMIX, NONEXISTANT_LOCALE, "downmix" }
};
#endif

int CAVPIDChangeExec::exec(CMenuTarget */*parent*/, const std::string & actionKey)
{
	dprintf(DEBUG_NORMAL, "CAVPIDSelectWidget::exec: %s (currentapid:%d)\n", actionKey.c_str(), currentapid);
	
	unsigned int sel = atoi(actionKey.c_str());

	if (currentapid != apids[sel]) 
	{
		currentapid = apids[sel];
		currentac3 = ac3flags[sel];
		
		if(playback)
			playback->SetAPid(currentapid, currentac3);
		
		if(currentac3)
			ac3state = CInfoViewer::AC3_ACTIVE;
		
		dprintf(DEBUG_NORMAL, "CAPIDSelect::exec: apid changed to %d\n", currentapid);
	}
	
	return menu_return::RETURN_EXIT;
}

//
int CAVPIDSelectWidget::exec(CMenuTarget * parent, const std::string & actionKey)
{
	dprintf(DEBUG_NORMAL, "CAVPIDSelectWidget::exec: %s\n", actionKey.c_str());
	
	int res = menu_return::RETURN_REPAINT;

	if (parent) 
		parent->hide();

	showAudioDialog();

	return res;
}

void CAVPIDSelectWidget::showAudioDialog(void)
{
	dprintf(DEBUG_NORMAL, "CAVPIDSelectWidget::showAudioDialog (currentapid:%d)\n", currentapid);

	CMenuWidget * AVPIDSelector = new CMenuWidget(LOCALE_APIDSELECTOR_HEAD, NEUTRINO_ICON_AUDIO);
	
	CAVPIDChangeExec AVPIDChanger;
	if(playback)
		playback->FindAllPids(apids, ac3flags, &numpida, language);
			
	if (numpida > 0) 
	{
		bool enabled;

		for (unsigned int count = 0; count < numpida; count++) 
		{
			bool name_ok = false;
			char apidnumber[10];
			sprintf(apidnumber, "%d", count);
			enabled = true;
			
			std::string apidtitle = "Stream ";

			// language
			if (!language[count].empty())
			{
				apidtitle = language[count];
				name_ok = true;
			}

			if (!name_ok)
			{
				apidtitle = "Stream ";
				name_ok = true;
			}

			// title (name)
			switch(ac3flags[count])
			{
				case 1: /*AC3,EAC3*/
					{
						apidtitle.append(" (AC3)");
								
						// ac3 state
						ac3state = CInfoViewer::AC3_AVAILABLE;
					}
					break;

				case 2: /*teletext*/
					apidtitle.append(" (Teletext)");
					enabled = false;
					break;

				case 3: /*MP2*/
					apidtitle.append(" (MP2)");
					break;

				case 4: /*MP3*/
					apidtitle.append(" (MP3)");
					break;

				case 5: /*AAC*/
					apidtitle.append(" (AAC)");
					break;

				case 6: /*DTS*/
					apidtitle.append(" (DTS)");
					break;

				case 7: /*MLP*/
					apidtitle.append(" (MLP)");
					break;

				default:
					break;
			}

			if (!name_ok)
				apidtitle.append(apidnumber);

			AVPIDSelector->addItem(new CMenuForwarder(apidtitle.c_str(), enabled, NULL, &AVPIDChanger, apidnumber, CRCInput::convertDigitToKey(count + 1)), (count == currentapid) );
		}
		
		AVPIDSelector->addItem(new CMenuSeparator(LINE));
	} 
	
	//ac3
#if !defined (PLATFORM_COOLSTREAM)				
	AVPIDSelector->addItem(new CMenuOptionChooser(LOCALE_AUDIOMENU_HDMI_DD, &g_settings.hdmi_dd, AC3_OPTIONS, AC3_OPTION_COUNT, true, CAudioSettings::getInstance()->audioSetupNotifier, RC_red, NEUTRINO_ICON_BUTTON_RED ));
#endif				
				
	// policy/aspect ratio
	AVPIDSelector->addItem(new CMenuSeparator(LINE));
				
	// video aspect ratio 4:3/16:9
	AVPIDSelector->addItem(new CMenuOptionChooser(LOCALE_VIDEOMENU_VIDEORATIO, &g_settings.video_Ratio, VIDEOMENU_VIDEORATIO_OPTIONS, VIDEOMENU_VIDEORATIO_OPTION_COUNT, true, CVideoSettings::getInstance()->videoSetupNotifier, RC_green, NEUTRINO_ICON_BUTTON_GREEN, true, true));
	
	// video format bestfit/letterbox/panscan/non
	AVPIDSelector->addItem(new CMenuOptionChooser(LOCALE_VIDEOMENU_VIDEOFORMAT, &g_settings.video_Format, VIDEOMENU_VIDEOFORMAT_OPTIONS, VIDEOMENU_VIDEOFORMAT_OPTION_COUNT, true, CVideoSettings::getInstance()->videoSetupNotifier, RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW, true, true));
	
	AVPIDSelector->exec(NULL, "");
	AVPIDSelector->hide();
	delete AVPIDSelector;
	AVPIDSelector = NULL;
}

