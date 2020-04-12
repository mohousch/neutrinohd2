/*
	Neutrino-GUI  -   DBoxII-Project

	$id: main_setup.cpp 2015.12.22 21:31:30 mohousch $
	
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

#include <gui/widget/messagebox.h>

#include <gui/video_setup.h>

#include <system/debug.h>
#include <system/setting_helpers.h>
#include <system/helpers.h>

#include <video_cs.h>


extern cVideo * videoDecoder;		//libcoolstream (video_cs.cpp)

//Video Settings
//hdmi color space
#if !defined (PLATFORM_COOLSTREAM)
#ifdef __sh__
#define VIDEOMENU_HDMI_COLOR_SPACE_OPTION_COUNT 3
const keyval VIDEOMENU_HDMI_COLOR_SPACE_OPTIONS[VIDEOMENU_HDMI_COLOR_SPACE_OPTION_COUNT] =
{
	 { HDMI_RGB, NONEXISTANT_LOCALE, "HDMI-RGB" },
	 { HDMI_YUV, NONEXISTANT_LOCALE, "HDMI-YUV" } ,
	 { HDMI_422, NONEXISTANT_LOCALE, "HDMI-422" }
};
#else
// giga
//
//Edid(Auto) 
//Hdmi_Rgb 
//Itu_R_BT_709 
//Unknown
#define VIDEOMENU_HDMI_COLOR_SPACE_OPTION_COUNT 4
const keyval VIDEOMENU_HDMI_COLOR_SPACE_OPTIONS[VIDEOMENU_HDMI_COLOR_SPACE_OPTION_COUNT] =
{
	 { HDMI_AUTO, NONEXISTANT_LOCALE, "Edid(Auto)" },
	 { HDMI_RGB, NONEXISTANT_LOCALE, "Hdmi_Rgb" } ,
	 { HDMI_ITU_R_BT_709, NONEXISTANT_LOCALE, "Itu_R_BT_709" },
	 { HDMI_UNKNOW, NONEXISTANT_LOCALE, "Unknow" }
};
#endif
#endif // !coolstream

//Analog Output 
#ifdef __sh__
//rgb/cvbs/svideo/yuv
#define VIDEOMENU_ANALOGUE_MODE_OPTION_COUNT 4
const keyval VIDEOMENU_ANALOGUE_MODE_OPTIONS[VIDEOMENU_ANALOGUE_MODE_OPTION_COUNT] =
{
	 { ANALOG_RGB, NONEXISTANT_LOCALE, "RGB" },
	 { ANALOG_CVBS, NONEXISTANT_LOCALE, "CVBS" },
	 { ANALOG_SVIDEO, NONEXISTANT_LOCALE, "SVIDEO" }, //not used
	 { ANALOG_YUV, NONEXISTANT_LOCALE, "YUV" }
};
#elif defined (PLATFORM_COOLSTREAM)
#define VIDEOMENU_ANALOGUE_MODE_OPTION_COUNT 4
const keyval VIDEOMENU_ANALOGUE_MODE_OPTIONS[VIDEOMENU_ANALOGUE_MODE_OPTION_COUNT] =
{
	{ ANALOG_SD_RGB_SCART, NONEXISTANT_LOCALE, "RGB"   }, /* composite + RGB */
	{ ANALOG_SD_YPRPB_SCART, NONEXISTANT_LOCALE, "YPbPr" }, /* YPbPr SCART */
	{ ANALOG_HD_RGB_SCART, NONEXISTANT_LOCALE, "RGB Scart"   },
	{ ANALOG_HD_YPRPB_SCART, NONEXISTANT_LOCALE, "YPbPr Scart" },
};
#else
//rgb/cvbs/yuv
#define VIDEOMENU_ANALOGUE_MODE_OPTION_COUNT 3
const keyval VIDEOMENU_ANALOGUE_MODE_OPTIONS[VIDEOMENU_ANALOGUE_MODE_OPTION_COUNT] =
{
	 { ANALOG_RGB, NONEXISTANT_LOCALE, "RGB" },
	 { ANALOG_CVBS, NONEXISTANT_LOCALE, "CVBS" },
	 { ANALOG_YUV, NONEXISTANT_LOCALE, "YUV" }
};
#endif

// aspect ratio
#ifdef __sh__
#define VIDEOMENU_VIDEORATIO_OPTION_COUNT 2
const keyval VIDEOMENU_VIDEORATIO_OPTIONS[VIDEOMENU_VIDEORATIO_OPTION_COUNT] =
{
	{ ASPECTRATIO_43, LOCALE_VIDEOMENU_VIDEORATIO_43, NULL },
	{ ASPECTRATIO_169, LOCALE_VIDEOMENU_VIDEORATIO_169, NULL }
};
#elif defined (PLATFORM_COOLSTREAM)
#define VIDEOMENU_VIDEORATIO_OPTION_COUNT 2
const keyval VIDEOMENU_VIDEORATIO_OPTIONS[VIDEOMENU_VIDEORATIO_OPTION_COUNT] =
{
	{ DISPLAY_AR_4_3, LOCALE_VIDEOMENU_VIDEORATIO_43, NULL },
	{ DISPLAY_AR_16_9, LOCALE_VIDEOMENU_VIDEORATIO_169, NULL },
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
#ifdef __sh__
//
//letterbox 
//panscan 
//non 
//bestfit
//
#define VIDEOMENU_VIDEOFORMAT_OPTION_COUNT 4
const keyval VIDEOMENU_VIDEOFORMAT_OPTIONS[VIDEOMENU_VIDEOFORMAT_OPTION_COUNT] = 
{
	{ VIDEOFORMAT_LETTERBOX, LOCALE_VIDEOMENU_LETTERBOX, NULL },
	{ VIDEOFORMAT_PANSCAN, LOCALE_VIDEOMENU_PANSCAN, NULL },
	{ VIDEOFORMAT_FULLSCREEN, LOCALE_VIDEOMENU_FULLSCREEN, NULL },
	{ VIDEOFORMAT_PANSCAN2, LOCALE_VIDEOMENU_PANSCAN2, NULL }
};
#elif defined (PLATFORM_COOLSTREAM)
#define VIDEOMENU_VIDEOFORMAT_OPTION_COUNT 4
const keyval VIDEOMENU_VIDEOFORMAT_OPTIONS[VIDEOMENU_VIDEOFORMAT_OPTION_COUNT] = 
{
	{ DISPLAY_AR_MODE_PANSCAN, LOCALE_VIDEOMENU_PANSCAN, NULL },
	{ DISPLAY_AR_MODE_PANSCAN2, LOCALE_VIDEOMENU_PANSCAN2, NULL },
	{ DISPLAY_AR_MODE_LETTERBOX, LOCALE_VIDEOMENU_LETTERBOX, NULL },
	{ DISPLAY_AR_MODE_NONE, LOCALE_VIDEOMENU_FULLSCREEN, NULL }
};
#else
// giga
//
//letterbox 
//panscan 
//bestfit 
//nonlinear
//
#define VIDEOMENU_VIDEOFORMAT_OPTION_COUNT 4
const keyval VIDEOMENU_VIDEOFORMAT_OPTIONS[VIDEOMENU_VIDEOFORMAT_OPTION_COUNT] = 
{
	{ VIDEOFORMAT_LETTERBOX, LOCALE_VIDEOMENU_LETTERBOX, NULL },
	{ VIDEOFORMAT_PANSCAN, LOCALE_VIDEOMENU_PANSCAN, NULL },
	{ VIDEOFORMAT_PANSCAN2, LOCALE_VIDEOMENU_PANSCAN2, NULL },
	{ VIDEOFORMAT_FULLSCREEN, LOCALE_VIDEOMENU_FULLSCREEN, NULL }
};
#endif

//video mode
#ifdef __sh__
// cuberevo
//
//pal 
//1080i50 
//720p50 
//576p50 
//576i50 
//1080i60 
//720p60 
//1080p24 
//1080p25 
//1080p30 
//1080p50
//PC
//
#define VIDEOMENU_VIDEOMODE_OPTION_COUNT 12
const keyval VIDEOMENU_VIDEOMODE_OPTIONS[VIDEOMENU_VIDEOMODE_OPTION_COUNT] =
{
	{ VIDEO_STD_PAL, NONEXISTANT_LOCALE, "PAL"		},
	{ VIDEO_STD_1080I50, NONEXISTANT_LOCALE, "1080i 50Hz"	},
	{ VIDEO_STD_720P50, NONEXISTANT_LOCALE, "720p 50Hz"	},
	{ VIDEO_STD_576P50, NONEXISTANT_LOCALE, "576p 50Hz"	},
	{ VIDEO_STD_576I50, NONEXISTANT_LOCALE, "576i 50Hz"	},
	{ VIDEO_STD_1080I60, NONEXISTANT_LOCALE, "1080i 60Hz"	},
	{ VIDEO_STD_720P60, NONEXISTANT_LOCALE, "720p 60Hz"	},
	{ VIDEO_STD_1080P24, NONEXISTANT_LOCALE, "1080p 24Hz"	},
	{ VIDEO_STD_1080P25, NONEXISTANT_LOCALE, "1080p 25Hz"	},
	{ VIDEO_STD_1080P30, NONEXISTANT_LOCALE, "1080p 30Hz"	},
	{ VIDEO_STD_1080P50, NONEXISTANT_LOCALE, "1080p 50Hz" 	},
	{ VIDEO_STD_PC, NONEXISTANT_LOCALE, "PC"		}
};
#elif defined (PLATFORM_COOLSTREAM)
#define VIDEOMENU_VIDEOMODE_OPTION_COUNT 12
const keyval VIDEOMENU_VIDEOMODE_OPTIONS[VIDEOMENU_VIDEOMODE_OPTION_COUNT] =
{
	{ VIDEO_STD_SECAM,   NONEXISTANT_LOCALE, "SECAM"	},
	{ VIDEO_STD_PAL,     NONEXISTANT_LOCALE, "PAL"		},
	{ VIDEO_STD_576P,    NONEXISTANT_LOCALE, "576p"		},
	{ VIDEO_STD_720P50,  NONEXISTANT_LOCALE, "720p 50Hz"	},
	{ VIDEO_STD_1080I50, NONEXISTANT_LOCALE, "1080i 50Hz"	},
	{ VIDEO_STD_1080P24, NONEXISTANT_LOCALE, "1080p 24Hz"	},
	{ VIDEO_STD_1080P25, NONEXISTANT_LOCALE, "1080p 25Hz"	},
	{ VIDEO_STD_NTSC,    NONEXISTANT_LOCALE, "NTSC"		},
	{ VIDEO_STD_480P,    NONEXISTANT_LOCALE, "480p"		},
	{ VIDEO_STD_720P60,  NONEXISTANT_LOCALE, "720p 60Hz"	},
	{ VIDEO_STD_1080I60, NONEXISTANT_LOCALE, "1080i 60Hz"	},
	{ VIDEO_STD_AUTO,    NONEXISTANT_LOCALE, "Auto"         }
};
#else
// giga
//
//pal 
//ntsc 
//480i 
//576i 
//480p 
//576p 
//720p50 
//720p 
//1080i50 
//1080i
//
#define VIDEOMENU_VIDEOMODE_OPTION_COUNT 10
const keyval VIDEOMENU_VIDEOMODE_OPTIONS[VIDEOMENU_VIDEOMODE_OPTION_COUNT] =
{
	{ VIDEO_STD_PAL, NONEXISTANT_LOCALE, "PAL"		},
	{ VIDEO_STD_NTSC, NONEXISTANT_LOCALE, "NTSC"		},
	{ VIDEO_STD_480I60, NONEXISTANT_LOCALE, "480i 60Hz"	},
	{ VIDEO_STD_576I50, NONEXISTANT_LOCALE, "576i 50Hz"	},
	{ VIDEO_STD_480P60, NONEXISTANT_LOCALE, "480p 60Hz"	},
	{ VIDEO_STD_576P50, NONEXISTANT_LOCALE, "576p 50Hz"	},
	{ VIDEO_STD_720P50, NONEXISTANT_LOCALE, "720p 50Hz"	},
	{ VIDEO_STD_720P60, NONEXISTANT_LOCALE, "720p 60Hz"	},
	{ VIDEO_STD_1080I50, NONEXISTANT_LOCALE, "1080i 50Hz"	},
	{ VIDEO_STD_1080I60, NONEXISTANT_LOCALE, "1080i 60Hz"	}
};
#endif

// wss
//
//off 
//auto 
//auto(4:3_off) 
//4:3_full_format 
//16:9_full_format 
//14:9_letterbox_center 
//14:9_letterbox_top 
//16:9_letterbox_center 
//16:9_letterbox_top 
//>16:9_letterbox_center 
//14:9_full_format
//
#if !defined (PLATFORM_COOLSTREAM)
#ifdef __sh__
#define VIDEOMENU_WSS_OPTION_COUNT 3
const keyval VIDEOMENU_WSS_OPTIONS[VIDEOMENU_WSS_OPTION_COUNT] =
{
	{ WSS_OFF, NONEXISTANT_LOCALE, "Off" },
	{ WSS_AUTO, NONEXISTANT_LOCALE, "Auto" },
	{ WSS_43_OFF, NONEXISTANT_LOCALE, "Auto(4:3_off)" },
};
#else
// giga
//
//off 
//auto 
//auto(4:3_off) 
//4:3_full_format 
//16:9_full_format 
//14:9_letterbox_center 
//14:9_letterbox_top 
//16:9_letterbox_center 
//16:9_letterbox_top 
//>16:9_letterbox_center 
//14:9_full_format
//
#define VIDEOMENU_WSS_OPTION_COUNT 11
const keyval VIDEOMENU_WSS_OPTIONS[VIDEOMENU_WSS_OPTION_COUNT] =
{
	{ WSS_OFF, NONEXISTANT_LOCALE, "Off" },
	{ WSS_AUTO, NONEXISTANT_LOCALE, "Auto" },
	{ WSS_43_OFF, NONEXISTANT_LOCALE, "Auto(4:3_off)" },
	{ WSS_43_FULL, NONEXISTANT_LOCALE, "4:3(full_format)" },
	{ WSS_169_FULL, NONEXISTANT_LOCALE, "16:9(full_format)" },
	{ WSS_149_LETTERBOX_CENTER, NONEXISTANT_LOCALE, "14:9(letterbox-center)" },
	{ WSS_149_LETTERBOX_TOP, NONEXISTANT_LOCALE, "14:9(letterbox-top)" },
	{ WSS_169_LETTERBOX_CENTER, NONEXISTANT_LOCALE, "16:9(letterbox-center)" },
	{ WSS_169_LETTERBOX_TOP, NONEXISTANT_LOCALE, "16:9(letterbox-top)" },
	{ WSS_169_LETTERBOX_CENTER_RIGHT, NONEXISTANT_LOCALE, ">16:9(letterbox-center)" },
	{ WSS_149_FULL, NONEXISTANT_LOCALE, "14:9(full)"}
	
};
#endif
#endif // !coolstream

CVideoSettings::CVideoSettings()
{
	videoSetupNotifier = new CVideoSetupNotifier;
}

CVideoSettings *CVideoSettings::getInstance()
{
	static CVideoSettings *videoSettings = NULL;

	if(!videoSettings)
	{
		videoSettings = new CVideoSettings();
		dprintf(DEBUG_NORMAL, "CVideoSettings::getInstance: Instance created\n");
	}
	
	return videoSettings;
}

CVideoSettings::~CVideoSettings()
{
	delete videoSetupNotifier;
}

int CVideoSettings::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CVideoSettings::exec: actionKey:%s\n", actionKey.c_str());
	
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

void CVideoSettings::showMenu()
{
	dprintf(DEBUG_NORMAL, "CVideoSettings::showMenu\n");
	
	int shortcutVideo = 1;
	
	ClistBoxWidget videoSettings(LOCALE_VIDEOMENU_HEAD, NEUTRINO_ICON_VIDEO);

	videoSettings.setMode(MODE_SETUP);
	videoSettings.enableShrinkMenu();
	
	// intros
	videoSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	videoSettings.addItem( new CMenuSeparator(LINE) );
	
	// save settings
	videoSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", RC_red, NEUTRINO_ICON_BUTTON_RED));
	videoSettings.addItem( new CMenuSeparator(LINE) );

	// video aspect ratio 4:3/16:9
	videoSettings.addItem(new CMenuOptionChooser(LOCALE_VIDEOMENU_VIDEORATIO, &g_settings.video_Ratio, VIDEOMENU_VIDEORATIO_OPTIONS, VIDEOMENU_VIDEORATIO_OPTION_COUNT, true, videoSetupNotifier, CRCInput::convertDigitToKey(shortcutVideo++), "", true ));
	
	// video format bestfit/letterbox/panscan/non
	videoSettings.addItem(new CMenuOptionChooser(LOCALE_VIDEOMENU_VIDEOFORMAT, &g_settings.video_Format, VIDEOMENU_VIDEOFORMAT_OPTIONS, VIDEOMENU_VIDEOFORMAT_OPTION_COUNT, true, videoSetupNotifier, CRCInput::convertDigitToKey(shortcutVideo++), "", true ));
	
	// video analogue mode
	videoSettings.addItem(new CMenuOptionChooser(LOCALE_VIDEOMENU_ANALOG_MODE, &g_settings.analog_mode, VIDEOMENU_ANALOGUE_MODE_OPTIONS, VIDEOMENU_ANALOGUE_MODE_OPTION_COUNT, true, videoSetupNotifier, CRCInput::convertDigitToKey(shortcutVideo++), "", true ));

#if !defined (PLATFORM_COOLSTREAM)
	// video hdmi space colour	
	videoSettings.addItem(new CMenuOptionChooser(LOCALE_VIDEOMENU_HDMI_COLOR_SPACE, &g_settings.hdmi_color_space, VIDEOMENU_HDMI_COLOR_SPACE_OPTIONS, VIDEOMENU_HDMI_COLOR_SPACE_OPTION_COUNT, true, videoSetupNotifier, CRCInput::convertDigitToKey(shortcutVideo++), "", true ));	
	
	// wss
	videoSettings.addItem(new CMenuOptionChooser(LOCALE_VIDEOMENU_WSS, &g_settings.wss_mode, VIDEOMENU_WSS_OPTIONS, VIDEOMENU_WSS_OPTION_COUNT, true, videoSetupNotifier, CRCInput::convertDigitToKey(shortcutVideo++), "", true ));
#endif	

	// video mode
	videoSettings.addItem(new CMenuOptionChooser(LOCALE_VIDEOMENU_VIDEOMODE, &g_settings.video_Mode, VIDEOMENU_VIDEOMODE_OPTIONS, VIDEOMENU_VIDEOMODE_OPTION_COUNT, true, videoSetupNotifier, CRCInput::convertDigitToKey(shortcutVideo++), "", true));
	
	videoSettings.exec(NULL, "");
	videoSettings.hide();
}

// video setup notifier
extern int prev_video_Mode;

bool CVideoSetupNotifier::changeNotify(const neutrino_locale_t OptionName, void *)
{
	dprintf(DEBUG_NORMAL, "CVideoSetupNotifier::changeNotify\n");
	
	CFrameBuffer *frameBuffer = CFrameBuffer::getInstance();

	if (ARE_LOCALES_EQUAL(OptionName, LOCALE_VIDEOMENU_ANALOG_MODE))	/* video analoue mode */
	{
		if(videoDecoder)
#if defined (PLATFORM_COOLSTREAM)
			videoDecoder->SetVideoMode((analog_mode_t) g_settings.analog_mode);
#else			
			videoDecoder->SetAnalogMode(g_settings.analog_mode);
#endif			
	}
	else if (ARE_LOCALES_EQUAL(OptionName, LOCALE_VIDEOMENU_VIDEORATIO) || ARE_LOCALES_EQUAL(OptionName, LOCALE_VIDEOMENU_VIDEOFORMAT ))	// format aspect-ratio
	{
		if(videoDecoder)
			videoDecoder->setAspectRatio(g_settings.video_Ratio, g_settings.video_Format);
	}
	else if (ARE_LOCALES_EQUAL(OptionName, LOCALE_VIDEOMENU_VIDEOMODE))	// mode
	{
		if(videoDecoder)
			videoDecoder->SetVideoSystem(g_settings.video_Mode);
		
		// clear screen
		frameBuffer->paintBackground();
#ifdef FB_BLIT
		frameBuffer->blit();
#endif		

		if(prev_video_Mode != g_settings.video_Mode) 
		{
			if(MessageBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_VIDEOMODE_OK), mbrNo, mbYes | mbNo, NEUTRINO_ICON_INFO) != mbrYes) 
			{
				g_settings.video_Mode = prev_video_Mode;
				if(videoDecoder)
					videoDecoder->SetVideoSystem(g_settings.video_Mode);	//no-> return to prev mode
			} 
			else
			{
				prev_video_Mode = g_settings.video_Mode;
			}
		}
	}
#if !defined (PLATFORM_COOLSTREAM)	
	else if (ARE_LOCALES_EQUAL(OptionName, LOCALE_VIDEOMENU_HDMI_COLOR_SPACE)) 
	{
		if(videoDecoder)
			videoDecoder->SetSpaceColour(g_settings.hdmi_color_space);
	}
	else if (ARE_LOCALES_EQUAL(OptionName, LOCALE_VIDEOMENU_WSS)) 
	{
		if(videoDecoder)
			videoDecoder->SetWideScreen(g_settings.wss_mode);
	}
#endif	

	return true;
}



