/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: neutrino.cpp 2013/10/12 mohousch Exp $

	Copyright (C) 2001 Steffen Hehn 'McClean' and some other guys
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
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define NEUTRINO_CPP

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <dlfcn.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/statvfs.h>
#include <sys/vfs.h>

#include <sys/socket.h>

#include <iostream>
#include <fstream>
#include <string>

#include <linux/fs.h>

#include <string.h>
#include <linux/reboot.h>
#include <sys/reboot.h>

#include <global.h>
#include <neutrino.h>

#include <daemonc/remotecontrol.h>

#include <driver/encoding.h>
#include <driver/framebuffer.h>
#include <driver/fontrenderer.h>
#include <driver/rcinput.h>
#include <driver/stream2file.h>
#include <driver/vcrcontrol.h>
#include <driver/shutdown_count.h>

#include <gui/epgplus.h>
#include <gui/streaminfo2.h>

#include <gui/widget/colorchooser.h>
#include <gui/widget/menue.h>
#include <gui/widget/messagebox.h>
#include <gui/widget/hintbox.h>
#include <gui/widget/icons.h>
#include <gui/widget/vfdcontroler.h>
#include <gui/widget/keychooser.h>
#include <gui/widget/stringinput.h>
#include <gui/widget/stringinput_ext.h>
#include <gui/widget/mountchooser.h>

#include <gui/color.h>

#include <gui/bedit/bouqueteditor_bouquets.h>
#include <gui/bouquetlist.h>
#include <gui/eventlist.h>
#include <gui/channellist.h>
#include <gui/screensetup.h>
#include <gui/pluginlist.h>
#include <gui/plugins.h>
#include <gui/infoviewer.h>
#include <gui/epgview.h>
#include <gui/update.h>
#include <gui/scan.h>
#include <gui/sleeptimer.h>
#include <gui/rc_lock.h>
#include <gui/timerlist.h>
#include <gui/alphasetup.h>
#include <gui/audioplayer.h>
#include <gui/imageinfo.h>
#include <gui/movieplayer.h>
#include <gui/nfs.h>
#include <gui/pictureviewer.h>
#include <gui/motorcontrol.h>
#include <gui/filebrowser.h>
#include <gui/widget/progressbar.h>
#if !defined (PLATFORM_COOLSTREAM)
#include <gui/cam_menu.h>
#endif
#include <gui/hdd_menu.h>
#include <gui/dboxinfo.h>
#include <gui/audio_select.h>
#include <gui/scan_setup.h>
#include <gui/webtv.h>
#include <gui/audio_video_select.h>
#include <gui/mediaplayer.h>
#include <gui/service_setup.h>
#include <gui/main_setup.h>
#include <gui/audio_setup.h>
#include <gui/video_setup.h>
#include <gui/parentallock_setup.h>
#include <gui/network_setup.h>
#include <gui/movieplayer_setup.h>
#include <gui/osd_setup.h>
#include <gui/audioplayer_setup.h>
#include <gui/pictureviewer_setup.h>
#include <gui/lcd_setup.h>
#include <gui/rc_setup.h>
#include <gui/recording_setup.h>
#include <gui/misc_setup.h>

#include <system/setting_helpers.h>
#include <system/settings.h>
#include <system/debug.h>
#include <system/flashtool.h>
#include <system/fsmounter.h>
#include <system/helpers.h>

#include <timerdclient/timerdmsg.h>

// zapit includes
#include <frontend_c.h>
#include <getservices.h>
#include <satconfig.h>

// libdvbapi
#include <video_cs.h>
#include <audio_cs.h>

// dvbsubs selct menu
#include <gui/dvbsub_select.h>

// zapit includes
#include <channel.h>
#include <bouquets.h>

// libcoolstream
#if defined (PLATFORM_COOLSTREAM)
#include <cs_api.h>
#endif

#include <playback_cs.h>
cPlayback * playback = NULL;

// ugly and dirty://FIXME
#if defined (USE_OPENGL)
extern char rec_filename[1024];				// defined in stream2file.cpp
#endif


extern tallchans allchans;				// defined in zapit.cpp
extern CBouquetManager * g_bouquetManager;		// defined in zapit.cpp

int old_b_id = -1;

// record and timeshift
bool autoshift = false;
uint32_t shift_timer;
uint32_t scrambled_timer;
char recDir[255];
char timeshiftDir[255];
std::string tmode;

// tuxtxt
//extern int  tuxtxt_init();
//extern void tuxtxt_start(int tpid, int source );
extern int  tuxtxt_stop();
extern void tuxtxt_close();
extern void tuxtx_pause_subtitle(bool pause, int source);
extern void tuxtx_stop_subtitle();
extern void tuxtx_set_pid(int pid, int page, const char * cc);
extern int tuxtx_subtitle_running(int *pid, int *page, int *running);
extern int tuxtx_main(int pid, int page, int source );

// dvbsub
//extern int dvbsub_initialise();
extern int dvbsub_init( /*int source*/);
extern int dvbsub_stop();
extern int dvbsub_close();
extern int dvbsub_start(int pid);
extern int dvbsub_pause();
extern int dvbsub_getpid();
extern void dvbsub_setpid(int pid);
extern int dvbsub_terminate();

// volume bar
static CProgressBar * g_volscale;

// timerd thread
static pthread_t timer_thread;
void * timerd_main_thread(void *data);

// streamts thread
extern int streamts_stop;				// defined in streamts.cpp
void * streamts_main_thread(void *data);
static pthread_t stream_thread ;

// zapit thread
extern int zapit_ready;					//defined in zapit.cpp
static pthread_t zapit_thread ;
void * zapit_main_thread(void *data);
extern t_channel_id live_channel_id; 			//defined in zapit.cpp
Zapit_config zapitCfg;
void setZapitConfig(Zapit_config * Cfg);
void getZapitConfig(Zapit_config * Cfg);
extern CZapitChannel * live_channel;			// defined in zapit.cpp
extern CFrontend * live_fe;
extern CScanSettings * scanSettings;			// defined in scan_setup.cpp
extern int FrontendCount;				// defined in zapit.cpp

// nhttpd thread
void * nhttpd_main_thread(void *data);
static pthread_t nhttpd_thread ;

// sectionsd thread
extern int sectionsd_stop;				// defined in sectionsd.cpp
static pthread_t sections_thread;
void * sectionsd_main_thread(void *data);
extern bool timeset;

// Audio/Video Decoder
extern cVideo 		* videoDecoder;		//libcoolstream (video_cs.cpp)
extern cAudio 		* audioDecoder;		//libcoolstream (audio_cs.cpp)

int prev_video_Mode;

void stop_daemons();

int current_volume;
int current_muted;

// bouquets lists
CBouquetList   		* bouquetList; 				//current bqt list

CBouquetList   		* TVbouquetList;
CBouquetList   		* TVsatList;
CBouquetList   		* TVfavList;
CBouquetList   		* TVallList;

CBouquetList   		* RADIObouquetList;
CBouquetList   		* RADIOsatList;
CBouquetList   		* RADIOfavList;
CBouquetList   		* RADIOallList;

CPlugins       		* g_PluginList;
CRemoteControl 		* g_RemoteControl;

#if defined (ENABLE_CI)
CCAMMenuHandler 	* g_CamHandler;
#endif

// webtv
CWebTV * webtv;

// timezone for wizard
extern CMenuOptionStringChooser * tzSelect;

bool parentallocked = false;
static char **global_argv;

extern const char * locale_real_names[]; 		//#include <system/locals_intern.h>

//user menu
const char *usermenu_button_def[SNeutrinoSettings::BUTTON_MAX]={
	"red", 
	"green", 
	"yellow", 
	"blue",
#if defined (ENABLE_FUNCTIONKEYS)
	"f1",
	"f2",
	"f3",
	"f4"
#endif
};

CVCRControl::CDevice * recordingdevice = NULL;

// init globals
static void initGlobals(void)
{
	g_fontRenderer  = NULL;
	g_RCInput       = NULL;
	g_Timerd        = NULL;
	g_Zapit 	= new CZapitClient;
	g_RemoteControl = NULL;
	g_EpgData       = NULL;
	g_InfoViewer    = NULL;
	g_EventList     = NULL;
	g_Locale        = new CLocaleManager;
	g_PluginList    = NULL;
#if defined (ENABLE_CI)	
	g_CamHandler 	= NULL;
#endif	

	g_Radiotext     = NULL;
	
	webtv = new CWebTV();
}

// CNeutrinoApp - Constructor, initialize g_fontRenderer
CNeutrinoApp::CNeutrinoApp()
: configfile('\t')
{
	standby_pressed_at.tv_sec = 0;

	frameBuffer = CFrameBuffer::getInstance();
	frameBuffer->setIconBasePath(DATADIR "/neutrino/icons/");
	SetupFrameBuffer();

	mode = mode_unknown;
	
	channelList       = NULL;
	TVchannelList       = NULL;
	RADIOchannelList       = NULL;
	
	nextRecordingInfo = NULL;
	skipShutdownTimer = false;
	current_muted = 0;
	
	memset(&font, 0, sizeof(neutrino_font_descr_struct));
	
	//
	recordingstatus = 0;
	timeshiftstatus = 0;
#if defined (USE_OPENGL)
	playbackstatus = 0;
#endif
}

// CNeutrinoApp - Destructor
CNeutrinoApp::~CNeutrinoApp()
{
	if (channelList)
		delete channelList;
}

// getInstance
CNeutrinoApp * CNeutrinoApp::getInstance()
{
	static CNeutrinoApp * neutrinoApp = NULL;

	if(!neutrinoApp) 
	{
		neutrinoApp = new CNeutrinoApp();
		dprintf(DEBUG_NORMAL, "NeutrinoApp Instance created\n");
	}

	return neutrinoApp;
}

// fonts
#define FONT_STYLE_REGULAR 0
#define FONT_STYLE_BOLD    1
#define FONT_STYLE_ITALIC  2

typedef struct font_sizes
{
        const neutrino_locale_t name;
        const unsigned int      defaultsize;
        const unsigned int      style;
        const unsigned int      size_offset;
} font_sizes_struct;

// neutrino_font
font_sizes_struct neutrino_font[FONT_TYPE_COUNT] =
{
        {LOCALE_FONTSIZE_MENU               ,  20, FONT_STYLE_BOLD   , 0},
        {LOCALE_FONTSIZE_MENU_TITLE         ,  30, FONT_STYLE_BOLD   , 0},
        {LOCALE_FONTSIZE_MENU_INFO          ,  16, FONT_STYLE_REGULAR, 0},
        {LOCALE_FONTSIZE_EPG_TITLE          ,  25, FONT_STYLE_REGULAR, 1},
        {LOCALE_FONTSIZE_EPG_INFO1          ,  17, FONT_STYLE_ITALIC , 2},
        {LOCALE_FONTSIZE_EPG_INFO2          ,  17, FONT_STYLE_REGULAR, 2},
        {LOCALE_FONTSIZE_EPG_DATE           ,  15, FONT_STYLE_REGULAR, 2},
        {LOCALE_FONTSIZE_EVENTLIST_TITLE    ,  30, FONT_STYLE_REGULAR, 0},
        {LOCALE_FONTSIZE_EVENTLIST_ITEMLARGE,  20, FONT_STYLE_BOLD   , 1},
        {LOCALE_FONTSIZE_EVENTLIST_ITEMSMALL,  14, FONT_STYLE_REGULAR, 1},
        {LOCALE_FONTSIZE_EVENTLIST_DATETIME ,  16, FONT_STYLE_REGULAR, 1},
        {LOCALE_FONTSIZE_GAMELIST_ITEMLARGE ,  20, FONT_STYLE_BOLD   , 1},
        {LOCALE_FONTSIZE_GAMELIST_ITEMSMALL ,  16, FONT_STYLE_REGULAR, 1},
        {LOCALE_FONTSIZE_CHANNELLIST        ,  20, FONT_STYLE_BOLD   , 1},
        {LOCALE_FONTSIZE_CHANNELLIST_DESCR  ,  20, FONT_STYLE_REGULAR, 1},
        {LOCALE_FONTSIZE_CHANNELLIST_NUMBER ,  14, FONT_STYLE_BOLD   , 2},
        {LOCALE_FONTSIZE_CHANNEL_NUM_ZAP    ,  40, FONT_STYLE_BOLD   , 0},
        {LOCALE_FONTSIZE_INFOBAR_NUMBER     ,  30, FONT_STYLE_BOLD   , 0},
        {LOCALE_FONTSIZE_INFOBAR_CHANNAME   ,  30, FONT_STYLE_BOLD   , 0},
        {LOCALE_FONTSIZE_INFOBAR_INFO       ,  20, FONT_STYLE_REGULAR, 1},
        {LOCALE_FONTSIZE_INFOBAR_SMALL      ,  14, FONT_STYLE_REGULAR, 1},
        {LOCALE_FONTSIZE_FILEBROWSER_ITEM   ,  16, FONT_STYLE_BOLD   , 1}
};

// signal font
const font_sizes_struct signal_font = {LOCALE_FONTSIZE_INFOBAR_SMALL,  14, FONT_STYLE_REGULAR, 1};

// LCD settings
typedef struct lcd_setting_t
{
	const char * const name;
	const unsigned int default_value;
} lcd_setting_struct_t;

const lcd_setting_struct_t lcd_setting[LCD_SETTING_COUNT] =
{
	{"lcd_brightness"       , DEFAULT_LCD_BRIGHTNESS       },
	{"lcd_standbybrightness", DEFAULT_LCD_STANDBYBRIGHTNESS},
	{"lcd_contrast"         , DEFAULT_LCD_CONTRAST         },
	{"lcd_power"            , DEFAULT_LCD_POWER            },
	{"lcd_inverse"          , DEFAULT_LCD_INVERSE          },
	{"lcd_show_volume"      , DEFAULT_LCD_SHOW_VOLUME      },
	{"lcd_autodimm"         , DEFAULT_LCD_AUTODIMM         },
	{"lcd_scroll_text"      , DEFAULT_LCD_SCROLL_TEXT      },
#if ENABLE_LCD
	{"lcd_epgmode"          , DEFAULT_LCD_EPGMODE          },
	{"lcd_epgalign"         , DEFAULT_LCD_EPGALIGN         },
	{"lcd_dump_png"         , DEFAULT_LCD_DUMP_PNG         },
#endif	
};

// loadSetup, load the application-settings
int CNeutrinoApp::loadSetup(const char * fname)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::loadSetup\n");
	
	char cfg_key[81];
	int erg = 0;

	configfile.clear();

	// settings laden - und dabei Defaults setzen!
	if(!configfile.loadConfig(fname)) 
	{
		//file existiert nicht
		erg = 1;
	}

	// parentallock check
        std::ifstream checkParentallocked(NEUTRINO_PARENTALLOCKED_FILE);
	if(checkParentallocked) 
	{
	        parentallocked = true;
	        checkParentallocked.close();
	}

	// video
#ifdef __sh__	
	g_settings.video_Mode = configfile.getInt32("video_Mode", VIDEO_STD_PAL);
#else
	g_settings.video_Mode = configfile.getInt32("video_Mode", VIDEO_STD_720P50);
#endif
	prev_video_Mode = g_settings.video_Mode;
	
	//analog mode
#if defined (PLATFORM_COOLSTREAM)
	g_settings.analog_mode = configfile.getInt32("analog_mode", (int)ANALOG_SD_RGB_SCART); // default RGB
#else
	g_settings.analog_mode = configfile.getInt32("analog_mode", ANALOG_YUV); 	// default yuv
	
	g_settings.hdmi_color_space = configfile.getInt32("hdmi_color_space", HDMI_RGB); //default RGB
#endif	

	//aspect ratio
#if defined (PLATFORM_COOLSTREAM)
	g_settings.video_Ratio = configfile.getInt32("video_Ratio", DISPLAY_AR_16_9);		// 16:9
#else
	g_settings.video_Ratio = configfile.getInt32("video_Ratio", ASPECTRATIO_169);		// 16:9
#endif	
	 
	// policy
#if defined (PLATFORM_COOLSTREAM)
	g_settings.video_Format = configfile.getInt32("video_Format", DISPLAY_AR_MODE_LETTERBOX);
#else
	g_settings.video_Format = configfile.getInt32("video_Format", VIDEOFORMAT_PANSCAN2);
#endif	

	//wss
#if !defined (PLATFORM_COOLSTREAM)	
	g_settings.wss_mode = configfile.getInt32("wss_mode", WSS_AUTO);
#endif	
	
	// psi
	g_settings.contrast = configfile.getInt32( "contrast", 130);
	g_settings.saturation = configfile.getInt32( "saturation", 130);
	g_settings.brightness = configfile.getInt32( "brightness", 130);
	g_settings.tint = configfile.getInt32( "tint", 130);
	// end video

	// audio
	g_settings.audio_AnalogMode = configfile.getInt32( "audio_AnalogMode", 0 );		//default=stereo
	g_settings.audio_DolbyDigital    = configfile.getBool("audio_DolbyDigital", false);
	
	// ac3
#if !defined (PLATFORM_COOLSTREAM)	
	g_settings.hdmi_dd = configfile.getInt32( "hdmi_dd", AC3_DOWNMIX);	// downmix
#endif	
	
	// avsync
#if defined (PLATFORM_COOLSTREAM)
	g_settings.avsync = configfile.getInt32( "avsync", 1);
#else
	g_settings.avsync = configfile.getInt32( "avsync", AVSYNC_ON);
#endif	
	
	// ac3 delay
	g_settings.ac3_delay = configfile.getInt32( "ac3_delay", 0);
	
	// pcm delay
	g_settings.pcm_delay = configfile.getInt32( "pcm_delay", 0);
	
	g_settings.auto_lang = configfile.getInt32( "auto_lang", 0 );
	g_settings.auto_subs = configfile.getInt32( "auto_subs", 0 );

	for(int i = 0; i < 3; i++) 
	{
		sprintf(cfg_key, "pref_lang_%d", i);
		strncpy(g_settings.pref_lang[i], configfile.getString(cfg_key, "German").c_str(), 30);
		
		sprintf(cfg_key, "pref_subs_%d", i);
		strncpy(g_settings.pref_subs[i], configfile.getString(cfg_key, "German").c_str(), 30);
	}
	// end audio

	// parentallock
	if (!parentallocked) 
	{
	  	g_settings.parentallock_prompt = configfile.getInt32( "parentallock_prompt", 0 );
		g_settings.parentallock_lockage = configfile.getInt32( "parentallock_lockage", 12 );
	} 
	else 
	{
	        g_settings.parentallock_prompt = 3;
	        g_settings.parentallock_lockage = 18;
	}
	strcpy( g_settings.parentallock_pincode, configfile.getString( "parentallock_pincode", "0000" ).c_str() );
	// end parentallock

	// network
	g_settings.network_ntpserver = configfile.getString("network_ntpserver", "time.fu-berlin.de");
        g_settings.network_ntprefresh = configfile.getString("network_ntprefresh", "30" );
        g_settings.network_ntpenable = configfile.getBool("network_ntpenable", false);
	
	snprintf(g_settings.ifname, sizeof(g_settings.ifname), "%s", configfile.getString("ifname", "eth0").c_str());

	// nfs entries
	for(int i = 0 ; i < NETWORK_NFS_NR_OF_ENTRIES ; i++) 
	{
		sprintf(cfg_key, "network_nfs_ip_%d", i);
		g_settings.network_nfs_ip[i] = configfile.getString(cfg_key, "");
		sprintf(cfg_key, "network_nfs_dir_%d", i);
		strcpy( g_settings.network_nfs_dir[i], configfile.getString( cfg_key, "" ).c_str() );
		sprintf(cfg_key, "network_nfs_local_dir_%d", i);
		strcpy( g_settings.network_nfs_local_dir[i], configfile.getString( cfg_key, "" ).c_str() );
		sprintf(cfg_key, "network_nfs_automount_%d", i);
		g_settings.network_nfs_automount[i] = configfile.getInt32( cfg_key, 0);
		sprintf(cfg_key, "network_nfs_type_%d", i);
		g_settings.network_nfs_type[i] = configfile.getInt32( cfg_key, 0);
		sprintf(cfg_key,"network_nfs_username_%d", i);
		strcpy( g_settings.network_nfs_username[i], configfile.getString( cfg_key, "" ).c_str() );
		sprintf(cfg_key, "network_nfs_password_%d", i);
		strcpy( g_settings.network_nfs_password[i], configfile.getString( cfg_key, "" ).c_str() );
		sprintf(cfg_key, "network_nfs_mount_options1_%d", i);
		strcpy( g_settings.network_nfs_mount_options1[i], configfile.getString( cfg_key, "ro,soft,udp" ).c_str() );
		sprintf(cfg_key, "network_nfs_mount_options2_%d", i);
		strcpy( g_settings.network_nfs_mount_options2[i], configfile.getString( cfg_key, "nolock,rsize=8192,wsize=8192" ).c_str() );
		sprintf(cfg_key, "network_nfs_mac_%d", i);
		strcpy( g_settings.network_nfs_mac[i], configfile.getString( cfg_key, "11:22:33:44:55:66").c_str() );
	}
	// end network

	// recording
	strcpy( g_settings.network_nfs_recordingdir, configfile.getString( "network_nfs_recordingdir", "/media/hdd/record" ).c_str() );

	// permanent timeshift
	g_settings.auto_timeshift = configfile.getInt32( "auto_timeshift", 0 );	

	// timeshift dir
	sprintf(timeshiftDir, "%s/.timeshift", g_settings.network_nfs_recordingdir);
	safe_mkdir(timeshiftDir);
	
	dprintf(DEBUG_INFO, "CNeutrinoApp::loadSetup: rec dir %s timeshift dir %s\n", g_settings.network_nfs_recordingdir, timeshiftDir);

	g_settings.record_hours = configfile.getInt32( "record_hours", 4 );
	g_settings.recording_audio_pids_default = configfile.getInt32("recording_audio_pids_default", TIMERD_APIDS_STD | TIMERD_APIDS_AC3);
	g_settings.recording_epg_for_filename = configfile.getBool("recording_epg_for_filename", false);
	g_settings.recording_save_in_channeldir = configfile.getBool("recording_save_in_channeldir", false);
	// end recording

	// movieplayer
	strcpy( g_settings.network_nfs_moviedir, configfile.getString( "network_nfs_moviedir", "/media/hdd/movie" ).c_str() );
	
	// multi select
	g_settings.movieplayer_allow_multiselect = configfile.getBool("movieplayer_allow_multiselect", false);
	// end movieplayer

	// OSD
	g_settings.gtx_alpha = configfile.getInt32( "gtx_alpha", 255);
	strcpy(g_settings.language, configfile.getString("language", "english").c_str());

	// themes
	g_settings.menu_Head_alpha = configfile.getInt32( "menu_Head_alpha", 0);
	g_settings.menu_Head_red = configfile.getInt32( "menu_Head_red", 35);
	g_settings.menu_Head_green = configfile.getInt32( "menu_Head_green", 35);
	g_settings.menu_Head_blue = configfile.getInt32( "menu_Head_blue", 35);
	g_settings.menu_Head_gradient = configfile.getInt32("menu_Head_gradient", gradientLight2Dark);

	g_settings.menu_Head_Text_alpha = configfile.getInt32( "menu_Head_Text_alpha", 0);
	g_settings.menu_Head_Text_red = configfile.getInt32( "menu_Head_Text_red", 100 );
	g_settings.menu_Head_Text_green = configfile.getInt32( "menu_Head_Text_green", 100 );
	g_settings.menu_Head_Text_blue = configfile.getInt32( "menu_Head_Text_blue", 100 );
	
	g_settings.menu_Content_alpha = configfile.getInt32( "menu_Content_alpha", 20);
	g_settings.menu_Content_red = configfile.getInt32( "menu_Content_red", 50);
	g_settings.menu_Content_green = configfile.getInt32( "menu_Content_green", 50);
	g_settings.menu_Content_blue = configfile.getInt32( "menu_Content_blue", 50);
	g_settings.menu_Content_Text_alpha = configfile.getInt32( "menu_Content_Text_alpha", 0);
	g_settings.menu_Content_Text_red = configfile.getInt32( "menu_Content_Text_red", 100 );
	g_settings.menu_Content_Text_green = configfile.getInt32( "menu_Content_Text_green", 100 );
	g_settings.menu_Content_Text_blue = configfile.getInt32( "menu_Content_Text_blue", 100 );
	
	g_settings.menu_Content_Selected_alpha = configfile.getInt32( "menu_Content_Selected_alpha", 15);
	g_settings.menu_Content_Selected_red = configfile.getInt32( "menu_Content_Selected_red", 20);
	g_settings.menu_Content_Selected_green = configfile.getInt32( "menu_Content_Selected_green", 20);
	g_settings.menu_Content_Selected_blue = configfile.getInt32( "menu_Content_Selected_blue", 20);
	g_settings.menu_Content_Selected_Text_alpha = configfile.getInt32( "menu_Content_Selected_Text_alpha", 0);
	g_settings.menu_Content_Selected_Text_red = configfile.getInt32( "menu_Content_Selected_Text_red", 100 );
	g_settings.menu_Content_Selected_Text_green = configfile.getInt32( "menu_Content_Selected_Text_green", 100 );
	g_settings.menu_Content_Selected_Text_blue = configfile.getInt32( "menu_Content_Selected_Text_blue", 100 );
	
	g_settings.menu_Content_inactive_alpha = configfile.getInt32( "menu_Content_inactive_alpha", 20);
	g_settings.menu_Content_inactive_red = configfile.getInt32( "menu_Content_inactive_red", 50);
	g_settings.menu_Content_inactive_green = configfile.getInt32( "menu_Content_inactive_green", 50);
	g_settings.menu_Content_inactive_blue = configfile.getInt32( "menu_Content_inactive_blue", 50);
	g_settings.menu_Content_inactive_Text_alpha = configfile.getInt32( "menu_Content_inactive_Text_alpha", 0);
	g_settings.menu_Content_inactive_Text_red = configfile.getInt32( "menu_Content_inactive_Text_red", 70);
	g_settings.menu_Content_inactive_Text_green = configfile.getInt32( "menu_Content_inactive_Text_green", 70);
	g_settings.menu_Content_inactive_Text_blue = configfile.getInt32( "menu_Content_inactive_Text_blue", 70);

	g_settings.infobar_alpha = configfile.getInt32( "infobar_alpha", 20 );
	g_settings.infobar_red = configfile.getInt32( "infobar_red", 50 );
	g_settings.infobar_green = configfile.getInt32( "infobar_green", 50 );
	g_settings.infobar_blue = configfile.getInt32( "infobar_blue", 50 );
	g_settings.infobar_Text_alpha = configfile.getInt32( "infobar_Text_alpha", 0 );
	g_settings.infobar_Text_red = configfile.getInt32( "infobar_Text_red", 100 );
	g_settings.infobar_Text_green = configfile.getInt32( "infobar_Text_green", 100 );
	g_settings.infobar_Text_blue = configfile.getInt32( "infobar_Text_blue", 100 );
		
	g_settings.infobar_colored_events_alpha = configfile.getInt32( "infobar_colored_events_alpha", 0);
	g_settings.infobar_colored_events_red = configfile.getInt32( "infobar_colored_events_red", 95);
	g_settings.infobar_colored_events_green = configfile.getInt32( "infobar_colored_events_green", 70);
	g_settings.infobar_colored_events_blue = configfile.getInt32( "infobar_colored_events_blue", 0);
	
	g_settings.menu_Foot_alpha = configfile.getInt32( "menu_Foot_alpha", 0);
	g_settings.menu_Foot_red = configfile.getInt32( "menu_Foot_red", 28);
	g_settings.menu_Foot_green = configfile.getInt32( "menu_Foot_green", 28);
	g_settings.menu_Foot_blue = configfile.getInt32( "menu_Foot_blue", 28);
	g_settings.menu_Foot_gradient = configfile.getInt32("menu_Foot_gradient", gradientDark2Light);
		
	g_settings.menu_Foot_Text_alpha = configfile.getInt32( "menu_Foot_Text_alpha", 0);
	g_settings.menu_Foot_Text_red = configfile.getInt32( "menu_Foot_Text_red", 50);
	g_settings.menu_Foot_Text_green = configfile.getInt32( "menu_Foot_Text_green", 50);
	g_settings.menu_Foot_Text_blue = configfile.getInt32( "menu_Foot_Text_blue", 50);

	strcpy( g_settings.font_file, configfile.getString( "font_file", FONTDIR "/micron.ttf" ).c_str() );

	// menue timing
	for (int i = 0; i < TIMING_SETTING_COUNT; i++)
		g_settings.timing[i] = configfile.getInt32(locale_real_names[timing_setting_name[i]], default_timing[i]);

	// screen setup
#if defined (USE_OPENGL)
	g_settings.screen_StartX = configfile.getInt32( "screen_StartX", 145 );
#else
	g_settings.screen_StartX = configfile.getInt32( "screen_StartX", 35 );
#endif	
	g_settings.screen_StartY = configfile.getInt32( "screen_StartY", 35 );
#if defined (USE_OPENGL)
	g_settings.screen_EndX = configfile.getInt32( "screen_EndX", frameBuffer->getScreenWidth(true) - 145 );
#else
	g_settings.screen_EndX = configfile.getInt32( "screen_EndX", frameBuffer->getScreenWidth(true) - 35 );
#endif	
	g_settings.screen_EndY = configfile.getInt32( "screen_EndY", frameBuffer->getScreenHeight(true) - 35 );
	g_settings.screen_width = configfile.getInt32("screen_width", frameBuffer->getScreenWidth(true) );
	g_settings.screen_height = configfile.getInt32("screen_height", frameBuffer->getScreenHeight(true) );
	g_settings.screen_xres = configfile.getInt32("screen_xres", 120);
	g_settings.screen_yres = configfile.getInt32("screen_yres", 90);
	
	g_settings.rounded_corners = configfile.getInt32("rounded_corners", 1);
	
	g_settings.menu_design = configfile.getInt32("menu_design", SNeutrinoSettings::MENU_DESIGN_CLASSIC);
	g_settings.menu_position = configfile.getInt32("menu_position", SNeutrinoSettings::MENU_POSITION_LEFT);
	// END OSD

	// keysbinding
	strcpy(g_settings.repeat_blocker, configfile.getString("repeat_blocker", "250").c_str());
	strcpy(g_settings.repeat_genericblocker, configfile.getString("repeat_genericblocker", "25").c_str());

	g_settings.key_tvradio_mode = configfile.getInt32( "key_tvradio_mode", CRCInput::RC_mode );
	g_settings.key_channelList_pageup = configfile.getInt32( "key_channelList_pageup",  CRCInput::RC_page_up );
	g_settings.key_channelList_pagedown = configfile.getInt32( "key_channelList_pagedown", CRCInput::RC_page_down );
	g_settings.key_channelList_cancel = configfile.getInt32( "key_channelList_cancel",  CRCInput::RC_home );
	g_settings.key_channelList_reload = configfile.getInt32( "key_channelList_reload",  CRCInput::RC_setup );
	g_settings.key_channelList_sort = configfile.getInt32( "key_channelList_sort",  CRCInput::RC_blue );
	g_settings.key_channelList_addrecord = configfile.getInt32( "key_channelList_addrecord", CRCInput::RC_red );
	g_settings.key_channelList_search = configfile.getInt32( "key_channelList_search", CRCInput::RC_green );
	g_settings.key_channelList_addremind = configfile.getInt32( "key_channelList_addremind", CRCInput::RC_yellow );
	g_settings.key_list_start = configfile.getInt32( "key_list_start", CRCInput::RC_nokey );
	g_settings.key_list_end = configfile.getInt32( "key_list_end", CRCInput::RC_nokey );
	
	g_settings.key_bouquet_up = configfile.getInt32( "key_bouquet_up",  CRCInput::RC_right);
	g_settings.key_bouquet_down = configfile.getInt32( "key_bouquet_down",  CRCInput::RC_left);

	g_settings.key_quickzap_up = configfile.getInt32( "key_quickzap_up",  CRCInput::RC_up );
	g_settings.key_quickzap_down = configfile.getInt32( "key_quickzap_down",  CRCInput::RC_down );
	g_settings.key_subchannel_up = configfile.getInt32( "key_subchannel_up",  CRCInput::RC_right );
	g_settings.key_subchannel_down = configfile.getInt32( "key_subchannel_down",  CRCInput::RC_left );
	g_settings.key_zaphistory = configfile.getInt32( "key_zaphistory",  CRCInput::RC_home );	
	g_settings.key_lastchannel = configfile.getInt32( "key_lastchannel",  CRCInput::RC_recall );
	
	// pip keys
	g_settings.key_pip = configfile.getInt32("key_pip", CRCInput::RC_pip);
	//g_settings.key_pip_subchannel = configfile.getInt32("key_pip_subchannel", CRCInput::RC_pipsubch);

	// mpkeys
	g_settings.mpkey_rewind = configfile.getInt32( "mpkey.rewind", CRCInput::RC_rewind );
	g_settings.mpkey_forward = configfile.getInt32( "mpkey.forward", CRCInput::RC_forward );
	g_settings.mpkey_pause = configfile.getInt32( "mpkey.pause", CRCInput::RC_pause );
	g_settings.mpkey_stop = configfile.getInt32( "mpkey.stop", CRCInput::RC_stop );
	g_settings.mpkey_play = configfile.getInt32( "mpkey.play", CRCInput::RC_play );
	g_settings.mpkey_audio = configfile.getInt32( "mpkey.audio", CRCInput::RC_green );
	g_settings.mpkey_time = configfile.getInt32( "mpkey.time", CRCInput::RC_setup );
	g_settings.mpkey_bookmark = configfile.getInt32( "mpkey.bookmark", CRCInput::RC_blue );
	g_settings.key_timeshift = configfile.getInt32( "key_timeshift", CRCInput::RC_pause );	

	// media keys
	g_settings.key_recordsbrowser = configfile.getInt32( "key_recordsbrowser", CRCInput::RC_nokey );
	g_settings.key_audioplayer = configfile.getInt32( "key_audioplayer", CRCInput::RC_nokey );
	g_settings.key_pictureviewer = configfile.getInt32( "key_pictureviewer", CRCInput::RC_nokey );
	g_settings.key_timerlist = configfile.getInt32( "key_timerlist", CRCInput::RC_nokey );
	g_settings.key_inetradio = configfile.getInt32( "key_inetraio", CRCInput::RC_nokey );
	g_settings.key_moviebrowser = configfile.getInt32( "key_moviebrowser", CRCInput::RC_nokey );
	g_settings.key_filebrowser = configfile.getInt32( "key_filebrowser", CRCInput::RC_nokey );
	g_settings.key_webtv = configfile.getInt32( "key_webtv", CRCInput::RC_nokey );
	
	g_settings.key_screenshot = configfile.getInt32( "key_screenshot", CRCInput::RC_record );
	
	// mb
	g_settings.mb_copy_jump = configfile.getInt32( "mb_copy_jump", CRCInput::RC_nokey );
	g_settings.mb_cut_jump = configfile.getInt32( "mb_cut_jump", CRCInput::RC_nokey );
	g_settings.mb_truncate = configfile.getInt32( "mb_truncate", CRCInput::RC_nokey );
	
	// webtv
	g_settings.webtv_userBouquet = configfile.getString("webtv_userBouquet", DEFAULT_WEBTV_FILE);
	g_settings.webtv_lastselectedchannel = configfile.getInt32("webtv_lastselectedchannel", 0);
	
        // USERMENU -> in system/settings.h
        //-------------------------------------------
        // this is as the current neutrino usermen
        const char * usermenu_default[SNeutrinoSettings::BUTTON_MAX]={
                "2, 3, 4, 10",                  // RED
                "5",                            // GREEN
                "6",                            // YELLOW
                "8, 9, 12, 11, 13, 14",   	// BLUE
#if defined (ENABLE_FUNCTIONKEYS)
		"0",				// F1
		"0",				// F2
		"0",				// F3
		"0",				// F4
#endif
        };
        char txt1[81];
        std::string txt2;
        const char* txt2ptr;

        for(int button = 0; button < SNeutrinoSettings::BUTTON_MAX; button++)
        {
                snprintf(txt1,80,"usermenu_tv_%s_text",usermenu_button_def[button]);
                txt1[80] = 0; // terminate for sure
                g_settings.usermenu_text[button] = configfile.getString(txt1, "" );

                snprintf(txt1,80,"usermenu_tv_%s",usermenu_button_def[button]);
                txt2 = configfile.getString(txt1,usermenu_default[button]);
                txt2ptr = txt2.c_str();

                for( int pos = 0; pos < SNeutrinoSettings::ITEM_MAX; pos++)
                {
                        // find next comma or end of string - if it's not the first round
                        if(pos != 0)
                        {
                                while(*txt2ptr != 0 && *txt2ptr != ',')
                                        txt2ptr++;
                                if(*txt2ptr != 0)
                                        txt2ptr++;
                        }

                        if(*txt2ptr != 0)
                        {
                                g_settings.usermenu[button][pos] = atoi(txt2ptr);  // there is still a string
                                if(g_settings.usermenu[button][pos] >= SNeutrinoSettings::ITEM_MAX)
                                        g_settings.usermenu[button][pos] = 0;
                        }
                        else
                                g_settings.usermenu[button][pos] = 0;     // string empty, fill up with 0

                }
        }
	// end keysbinding

	// audioplayer
	strcpy( g_settings.network_nfs_audioplayerdir, configfile.getString( "network_nfs_audioplayerdir", "/media/hdd/music" ).c_str() );

	g_settings.audioplayer_display = configfile.getInt32("audioplayer_display",(int)CAudioPlayerGui::ARTIST_TITLE);
	g_settings.audioplayer_follow  = configfile.getInt32("audioplayer_follow",0);
	g_settings.audioplayer_highprio  = configfile.getInt32("audioplayer_highprio",0);
	g_settings.audioplayer_select_title_by_name = configfile.getInt32("audioplayer_select_title_by_name", 0);
	g_settings.audioplayer_repeat_on = configfile.getInt32("audioplayer_repeat_on",0);
	g_settings.audioplayer_hide_playlist = configfile.getInt32("audioplayer_hide_playlist", 0);
	// end audioplayer

	// pictureviewer
	strcpy( g_settings.network_nfs_picturedir, configfile.getString( "network_nfs_picturedir", "/media/hdd/picture" ).c_str() );

	strcpy( g_settings.picviewer_slide_time, configfile.getString( "picviewer_slide_time", "10" ).c_str() );
	g_settings.picviewer_scaling = configfile.getInt32("picviewer_scaling", (int)CFrameBuffer::SIMPLE);
	// end pictureviewer

	// misc opts
	g_settings.channel_mode = configfile.getInt32("channel_mode", LIST_MODE_ALL);

	//misc
	g_settings.power_standby = configfile.getInt32( "power_standby", 0);

	g_settings.shutdown_real = configfile.getBool("shutdown_real", false);
	g_settings.shutdown_real_rcdelay = configfile.getBool("shutdown_real_rcdelay", false);
        strcpy(g_settings.shutdown_count, configfile.getString("shutdown_count", "0").c_str());

	g_settings.infobar_sat_display   = configfile.getBool("infobar_sat_display"  , true );
	g_settings.infobar_subchan_disp_pos = configfile.getInt32("infobar_subchan_disp_pos"  , 0 );

	g_settings.rotor_swap = configfile.getInt32( "rotor_swap", 0);
	g_settings.zap_cycle = configfile.getInt32( "zap_cycle", 1 );
	g_settings.sms_channel = configfile.getInt32( "sms_channel", 0 );

	//timezone
	strcpy(g_settings.timezone, configfile.getString("timezone", "(GMT+01:00) Amsterdam, Berlin, Bern, Rome, Vienna").c_str());
	
	//zapit setup
	g_settings.lastChannelMode = configfile.getInt32("lastChannelMode", 1);		//TV mode
	g_settings.StartChannelTV = configfile.getString("startchanneltv", "");
	g_settings.StartChannelRadio = configfile.getString("startchannelradio", "");
	g_settings.startchanneltv_id =  configfile.getInt64("startchanneltv_id", 0) & 0xFFFFFFFFFFFFULL; 
	g_settings.startchannelradio_id = configfile.getInt64("startchannelradio_id", 0);
	g_settings.startchanneltv_nr =  configfile.getInt32("startchanneltv_nr", 0);
	g_settings.startchannelradio_nr = configfile.getInt32("startchannelradio_nr", 0);
	g_settings.uselastchannel = configfile.getInt32("uselastchannel" , 1);

	// epg
        g_settings.epg_cache            = configfile.getString("epg_cache_time", "14");
        g_settings.epg_extendedcache    = configfile.getString("epg_extendedcache_time", "360");
        g_settings.epg_old_events       = configfile.getString("epg_old_events", "1");
        g_settings.epg_max_events       = configfile.getString("epg_max_events", "50000");
        g_settings.epg_dir              = configfile.getString("epg_dir", "/media/hdd/epg");
	g_settings.epg_save = configfile.getBool("epg_save", false);
	g_settings.epg_read = configfile.getBool("epg_read", true);
	
	for(int i = 0; i < 3; i++) 
	{
		sprintf(cfg_key, "pref_epgs_%d", i);
		strncpy(g_settings.pref_epgs[i], configfile.getString(cfg_key, "German").c_str(), 30);
	}
	//
	
	// channellist 
	g_settings.virtual_zap_mode = configfile.getBool("virtual_zap_mode", false);
	g_settings.make_hd_list = configfile.getInt32("make_hd_list", 0);
	
	//crypticon on channellist
	g_settings.channellist_ca = configfile.getInt32("channellist_ca", 1);
	g_settings.channellist_extended	= configfile.getBool("channellist_extended", true);
	
	// record screenshot
	g_settings.recording_screenshot = configfile.getInt32("recording_screenshot", 1);
	//

	// Filebrowser
	g_settings.filesystem_is_utf8 = configfile.getBool("filesystem_is_utf8", true );
	g_settings.filebrowser_showrights = configfile.getInt32("filebrowser_showrights", 1);
	g_settings.filebrowser_sortmethod = configfile.getInt32("filebrowser_sortmethod", 0);
	if ((g_settings.filebrowser_sortmethod < 0) || (g_settings.filebrowser_sortmethod >= FILEBROWSER_NUMBER_OF_SORT_VARIANTS))
		g_settings.filebrowser_sortmethod = 0;
	g_settings.filebrowser_denydirectoryleave = configfile.getBool("filebrowser_denydirectoryleave", false);
	
	// radiotext	
	g_settings.radiotext_enable = configfile.getBool("radiotext_enable", false);
	
	// logos_dir
	g_settings.logos_dir = configfile.getString("logos_dir", "/var/tuxbox/logos");
	
	// epgplus logos
	g_settings.epgplus_show_logo = configfile.getBool("epgplus_show_logo", false);
	
	// infobar show channel name
	g_settings.show_channelname = configfile.getBool("show_channelname", true);
	
	// vol
	g_settings.volume_pos = configfile.getInt32( "volume_pos", 1);		//top_left
	g_settings.current_volume = configfile.getInt32("current_volume", 25);
	strcpy( g_settings.audio_step, configfile.getString( "audio_step" , "5" ).c_str() );

	g_settings.progressbar_color = configfile.getInt32("progressbar_color", 0);
	// END MISC OPTS

	// HDD
	g_settings.hdd_sleep = configfile.getInt32( "hdd_sleep", 120);
	g_settings.hdd_noise = configfile.getInt32( "hdd_noise", 254);
	// END HDD

	// SOFT UPDATE
	strcpy(g_settings.softupdate_proxyserver, configfile.getString("softupdate_proxyserver", "" ).c_str());
	strcpy(g_settings.softupdate_proxyusername, configfile.getString("softupdate_proxyusername", "" ).c_str());
	strcpy(g_settings.softupdate_proxypassword, configfile.getString("softupdate_proxypassword", "" ).c_str());
	strcpy( g_settings.update_dir, configfile.getString( "update_dir", "/tmp" ).c_str() );
	strcpy(g_settings.softupdate_url_file, configfile.getString("softupdate_url_file", "/var/etc/update.urls").c_str());
	// END UPDATE

	// VFD
	for (int i = 0; i < LCD_SETTING_COUNT; i++)
		g_settings.lcd_setting[i] = configfile.getInt32(lcd_setting[i].name, lcd_setting[i].default_value);
	
	strcpy(g_settings.lcd_setting_dim_time, configfile.getString("lcd_dim_time", "0").c_str());
	g_settings.lcd_setting_dim_brightness = configfile.getInt32("lcd_dim_brightness", 0);
	
	g_settings.lcd_ledcolor = configfile.getInt32("lcd_ledcolor", 1);
	// END VFD

	// satip
	g_settings.satip_allow_satip = configfile.getBool("satip_allow_satip", false);
	g_settings.satip_serverbox_ip = configfile.getString("satip_serverbox_ip", "192.168.0.12");
	g_settings.satip_serverbox_type = configfile.getInt32("satip_serverbox_type", DVB_C);
	g_settings.satip_serverbox_gui = configfile.getInt32("satip_serverbox_gui", SNeutrinoSettings::SATIP_SERVERBOX_GUI_NEUTRINO_HD);

	// mode
	//g_settings.mode = configfile.getInt32("mode", mode_tv);

	g_settings.tmdbkey = configfile.getString("tmdbkey", "");
	
	//set OSD resolution
#if defined (USE_OPENGL)
#define DEFAULT_X_OFF 145
#else
#define DEFAULT_X_OFF 35
#endif
#define DEFAULT_Y_OFF 35
	if((g_settings.screen_width != (int) frameBuffer->getScreenWidth(true)) || (g_settings.screen_height != (int) frameBuffer->getScreenHeight(true))) 
	{
		g_settings.screen_StartX = DEFAULT_X_OFF;
		g_settings.screen_StartY = DEFAULT_Y_OFF;
		g_settings.screen_EndX = frameBuffer->getScreenWidth(true) - DEFAULT_X_OFF;
		g_settings.screen_EndY = frameBuffer->getScreenHeight(true) - DEFAULT_Y_OFF;
		g_settings.screen_width = frameBuffer->getScreenWidth(true);
		g_settings.screen_height = frameBuffer->getScreenHeight(true);
	}	

	if(configfile.getUnknownKeyQueryedFlag() && (erg==0)) 
	{
		erg = 2;
	}

	if(erg)
		configfile.setModifiedFlag(true);

	return erg;
}

// saveSetup, save the application-settings
void CNeutrinoApp::saveSetup(const char * fname)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::saveSetup\n");
	
	char cfg_key[81];

	// VIDEO
	configfile.setInt32( "video_Mode", g_settings.video_Mode );
	configfile.setInt32( "analog_mode", g_settings.analog_mode );
	
	// hdmi space colour
	configfile.setInt32( "hdmi_color_space", g_settings.hdmi_color_space );
	
	//aspect ratio
	configfile.setInt32( "video_Ratio", g_settings.video_Ratio );
	
	//display format
	configfile.setInt32( "video_Format", g_settings.video_Format );

	// wss
	configfile.setInt32("wss_mode", g_settings.wss_mode);
	
	configfile.setInt32( "contrast", g_settings.contrast);
	configfile.setInt32( "saturation", g_settings.saturation);
	configfile.setInt32( "brightness", g_settings.brightness);
	configfile.setInt32( "tint", g_settings.tint);
	// END VIDEO

	// AUDIO
	configfile.setInt32( "audio_AnalogMode", g_settings.audio_AnalogMode );
	configfile.setBool("audio_DolbyDigital", g_settings.audio_DolbyDigital);
	configfile.setInt32( "hdmi_dd", g_settings.hdmi_dd);
	
	configfile.setInt32( "avsync", g_settings.avsync);
	
	// ac3 delay
	configfile.setInt32( "ac3_delay", g_settings.ac3_delay);
	
	// pcm delay
	configfile.setInt32( "pcm_delay", g_settings.pcm_delay);
	
	// auto langs/subs/ac3
	configfile.setInt32( "auto_lang", g_settings.auto_lang );
	configfile.setInt32( "auto_subs", g_settings.auto_subs );
	for(int i = 0; i < 3; i++) {
		sprintf(cfg_key, "pref_lang_%d", i);
		configfile.setString(cfg_key, g_settings.pref_lang[i]);
		sprintf(cfg_key, "pref_subs_%d", i);
		configfile.setString(cfg_key, g_settings.pref_subs[i]);
	}
	// END AUDIO

	// PARENTALLOCK
	configfile.setInt32( "parentallock_prompt", g_settings.parentallock_prompt );
	configfile.setInt32( "parentallock_lockage", g_settings.parentallock_lockage );
	configfile.setString( "parentallock_pincode", g_settings.parentallock_pincode );
	// END PARENTALLOCK

	// NETWORK
        configfile.setString( "network_ntpserver", g_settings.network_ntpserver);
        configfile.setString( "network_ntprefresh", g_settings.network_ntprefresh);
        configfile.setBool( "network_ntpenable", g_settings.network_ntpenable);
	
	configfile.setString("ifname", g_settings.ifname);

	for(int i = 0 ; i < NETWORK_NFS_NR_OF_ENTRIES ; i++) 
	{
		sprintf(cfg_key, "network_nfs_ip_%d", i);
		configfile.setString( cfg_key, g_settings.network_nfs_ip[i] );
		sprintf(cfg_key, "network_nfs_dir_%d", i);
		configfile.setString( cfg_key, g_settings.network_nfs_dir[i] );
		sprintf(cfg_key, "network_nfs_local_dir_%d", i);
		configfile.setString( cfg_key, g_settings.network_nfs_local_dir[i] );
		sprintf(cfg_key, "network_nfs_automount_%d", i);
		configfile.setInt32( cfg_key, g_settings.network_nfs_automount[i]);
		sprintf(cfg_key, "network_nfs_type_%d", i);
		configfile.setInt32( cfg_key, g_settings.network_nfs_type[i]);
		sprintf(cfg_key,"network_nfs_username_%d", i);
		configfile.setString( cfg_key, g_settings.network_nfs_username[i] );
		sprintf(cfg_key, "network_nfs_password_%d", i);
		configfile.setString( cfg_key, g_settings.network_nfs_password[i] );
		sprintf(cfg_key, "network_nfs_mount_options1_%d", i);
		configfile.setString( cfg_key, g_settings.network_nfs_mount_options1[i]);
		sprintf(cfg_key, "network_nfs_mount_options2_%d", i);
		configfile.setString( cfg_key, g_settings.network_nfs_mount_options2[i]);
		sprintf(cfg_key, "network_nfs_mac_%d", i);
		configfile.setString( cfg_key, g_settings.network_nfs_mac[i]);
	}
	// END NETWORK

	// RECORDING
	configfile.setString( "network_nfs_recordingdir", g_settings.network_nfs_recordingdir);
	configfile.setInt32( "auto_timeshift", g_settings.auto_timeshift );
	configfile.setInt32( "record_hours", g_settings.record_hours );
	configfile.setInt32 ("recording_audio_pids_default"       , g_settings.recording_audio_pids_default);
	configfile.setBool  ("recording_epg_for_filename"         , g_settings.recording_epg_for_filename);
	configfile.setBool  ("recording_save_in_channeldir"       , g_settings.recording_save_in_channeldir);
	// END RECORDING

	// MOVIEPLAYER
	configfile.setString( "network_nfs_moviedir", g_settings.network_nfs_moviedir);
	
	// multi select
	configfile.setBool ("movieplayer_allow_multiselect", g_settings.movieplayer_allow_multiselect);
	// END MOVIEPLAYER

	// OSD
	configfile.setInt32( "gtx_alpha", g_settings.gtx_alpha);

	configfile.setString("language", g_settings.language);

	// theme
	configfile.setInt32( "menu_Head_alpha", g_settings.menu_Head_alpha );
	configfile.setInt32( "menu_Head_red", g_settings.menu_Head_red );
	configfile.setInt32( "menu_Head_green", g_settings.menu_Head_green );
	configfile.setInt32( "menu_Head_blue", g_settings.menu_Head_blue );
	configfile.setInt32("menu_Head_gradient", g_settings.menu_Head_gradient);

	configfile.setInt32( "menu_Head_Text_alpha", g_settings.menu_Head_Text_alpha );
	configfile.setInt32( "menu_Head_Text_red", g_settings.menu_Head_Text_red );
	configfile.setInt32( "menu_Head_Text_green", g_settings.menu_Head_Text_green );
	configfile.setInt32( "menu_Head_Text_blue", g_settings.menu_Head_Text_blue );

	configfile.setInt32( "menu_Content_alpha", g_settings.menu_Content_alpha );
	configfile.setInt32( "menu_Content_red", g_settings.menu_Content_red );
	configfile.setInt32( "menu_Content_green", g_settings.menu_Content_green );
	configfile.setInt32( "menu_Content_blue", g_settings.menu_Content_blue );

	configfile.setInt32( "menu_Content_Text_alpha", g_settings.menu_Content_Text_alpha );
	configfile.setInt32( "menu_Content_Text_red", g_settings.menu_Content_Text_red );
	configfile.setInt32( "menu_Content_Text_green", g_settings.menu_Content_Text_green );
	configfile.setInt32( "menu_Content_Text_blue", g_settings.menu_Content_Text_blue );

	configfile.setInt32( "menu_Content_Selected_alpha", g_settings.menu_Content_Selected_alpha );
	configfile.setInt32( "menu_Content_Selected_red", g_settings.menu_Content_Selected_red );
	configfile.setInt32( "menu_Content_Selected_green", g_settings.menu_Content_Selected_green );
	configfile.setInt32( "menu_Content_Selected_blue", g_settings.menu_Content_Selected_blue );

	configfile.setInt32( "menu_Content_Selected_Text_alpha", g_settings.menu_Content_Selected_Text_alpha );
	configfile.setInt32( "menu_Content_Selected_Text_red", g_settings.menu_Content_Selected_Text_red );
	configfile.setInt32( "menu_Content_Selected_Text_green", g_settings.menu_Content_Selected_Text_green );
	configfile.setInt32( "menu_Content_Selected_Text_blue", g_settings.menu_Content_Selected_Text_blue );

	configfile.setInt32( "menu_Content_inactive_alpha", g_settings.menu_Content_inactive_alpha );
	configfile.setInt32( "menu_Content_inactive_red", g_settings.menu_Content_inactive_red );
	configfile.setInt32( "menu_Content_inactive_green", g_settings.menu_Content_inactive_green );
	configfile.setInt32( "menu_Content_inactive_blue", g_settings.menu_Content_inactive_blue );

	configfile.setInt32( "menu_Content_inactive_Text_alpha", g_settings.menu_Content_inactive_Text_alpha );
	configfile.setInt32( "menu_Content_inactive_Text_red", g_settings.menu_Content_inactive_Text_red );
	configfile.setInt32( "menu_Content_inactive_Text_green", g_settings.menu_Content_inactive_Text_green );
	configfile.setInt32( "menu_Content_inactive_Text_blue", g_settings.menu_Content_inactive_Text_blue );

	configfile.setInt32( "infobar_alpha", g_settings.infobar_alpha );
	configfile.setInt32( "infobar_red", g_settings.infobar_red );
	configfile.setInt32( "infobar_green", g_settings.infobar_green );
	configfile.setInt32( "infobar_blue", g_settings.infobar_blue );

	configfile.setInt32( "infobar_Text_alpha", g_settings.infobar_Text_alpha );
	configfile.setInt32( "infobar_Text_red", g_settings.infobar_Text_red );
	configfile.setInt32( "infobar_Text_green", g_settings.infobar_Text_green );
	configfile.setInt32( "infobar_Text_blue", g_settings.infobar_Text_blue );
	
	configfile.setInt32( "infobar_colored_events_alpha", g_settings.infobar_colored_events_alpha );
	configfile.setInt32( "infobar_colored_events_red", g_settings.infobar_colored_events_red );
	configfile.setInt32( "infobar_colored_events_green", g_settings.infobar_colored_events_green );
	configfile.setInt32( "infobar_colored_events_blue", g_settings.infobar_colored_events_blue );
	
	configfile.setInt32( "menu_Foot_alpha", g_settings.menu_Foot_alpha );
	configfile.setInt32( "menu_Foot_red", g_settings.menu_Foot_red );
	configfile.setInt32( "menu_Foot_green", g_settings.menu_Foot_green );
	configfile.setInt32( "menu_Foot_blue", g_settings.menu_Foot_blue );
	configfile.setInt32("menu_Foot_gradient", g_settings.menu_Foot_gradient);
	
	configfile.setInt32( "menu_Foot_Text_alpha", g_settings.menu_Foot_Text_alpha );
	configfile.setInt32( "menu_Foot_Text_red", g_settings.menu_Foot_Text_red );
	configfile.setInt32( "menu_Foot_Text_green", g_settings.menu_Foot_Text_green );
	configfile.setInt32( "menu_Foot_Text_blue", g_settings.menu_Foot_Text_blue );

	configfile.setInt32( "screen_StartX", g_settings.screen_StartX );
	configfile.setInt32( "screen_StartY", g_settings.screen_StartY );
	configfile.setInt32( "screen_EndX", g_settings.screen_EndX );
	configfile.setInt32( "screen_EndY", g_settings.screen_EndY );
	configfile.setInt32( "screen_width", g_settings.screen_width);
	configfile.setInt32( "screen_height", g_settings.screen_height);
	configfile.setInt32( "screen_xres", g_settings.screen_xres);
	configfile.setInt32( "screen_yres", g_settings.screen_yres);

	configfile.setString("font_file", g_settings.font_file);

	// menue timing
	for (int i = 0; i < TIMING_SETTING_COUNT; i++)
		configfile.setInt32(locale_real_names[timing_setting_name[i]], g_settings.timing[i]);
	
	configfile.setInt32("rounded_corners", g_settings.rounded_corners);
	
	configfile.setInt32("menu_design", g_settings.menu_design);
	configfile.setInt32("menu_position", g_settings.menu_position);
	// END OSD

	// KEYS
	configfile.setString( "repeat_blocker", g_settings.repeat_blocker );
	configfile.setString( "repeat_genericblocker", g_settings.repeat_genericblocker );

	configfile.setInt32( "key_tvradio_mode", g_settings.key_tvradio_mode );

	configfile.setInt32( "key_channelList_pageup", g_settings.key_channelList_pageup );
	configfile.setInt32( "key_channelList_pagedown", g_settings.key_channelList_pagedown );
	configfile.setInt32( "key_channelList_cancel", g_settings.key_channelList_cancel );
	configfile.setInt32( "key_channelList_reload", g_settings.key_channelList_reload );
	configfile.setInt32( "key_channelList_sort", g_settings.key_channelList_sort );
	configfile.setInt32( "key_channelList_addrecord", g_settings.key_channelList_addrecord );
	configfile.setInt32( "key_channelList_addremind", g_settings.key_channelList_addremind );
	configfile.setInt32( "key_channelList_search", g_settings.key_channelList_search );

	configfile.setInt32( "key_quickzap_up", g_settings.key_quickzap_up );
	configfile.setInt32( "key_quickzap_down", g_settings.key_quickzap_down );
	configfile.setInt32( "key_bouquet_up", g_settings.key_bouquet_up );
	configfile.setInt32( "key_bouquet_down", g_settings.key_bouquet_down );
	configfile.setInt32( "key_subchannel_up", g_settings.key_subchannel_up );
	configfile.setInt32( "key_subchannel_down", g_settings.key_subchannel_down );
	configfile.setInt32( "key_zaphistory", g_settings.key_zaphistory );
	configfile.setInt32( "key_lastchannel", g_settings.key_lastchannel );

	configfile.setInt32( "key_list_start", g_settings.key_list_start );
	configfile.setInt32( "key_list_end", g_settings.key_list_end );
	
	configfile.setInt32( "key_pip", g_settings.key_pip );
	
	// mp keys
	configfile.setInt32( "mpkey.rewind", g_settings.mpkey_rewind );
	configfile.setInt32( "mpkey.forward", g_settings.mpkey_forward );
	configfile.setInt32( "mpkey.pause", g_settings.mpkey_pause );
	configfile.setInt32( "mpkey.stop", g_settings.mpkey_stop );
	configfile.setInt32( "mpkey.play", g_settings.mpkey_play );
	configfile.setInt32( "mpkey.audio", g_settings.mpkey_audio );
	configfile.setInt32( "mpkey.time", g_settings.mpkey_time );
	configfile.setInt32( "mpkey.bookmark", g_settings.mpkey_bookmark );
	configfile.setInt32( "key_timeshift", g_settings.key_timeshift );
	
	// media keys
	configfile.setInt32( "key_recordsbrowser", g_settings.key_recordsbrowser );
	configfile.setInt32( "key_audioplayer", g_settings.key_audioplayer );
	configfile.setInt32( "key_pictureviewer", g_settings.key_pictureviewer );
	configfile.setInt32( "key_timerlist", g_settings.key_timerlist );
	configfile.setInt32( "key_inetradio", g_settings.key_inetradio );
	configfile.setInt32( "key_moviebrowser", g_settings.key_moviebrowser );
	configfile.setInt32( "key_filebrowser", g_settings.key_filebrowser );
	configfile.setInt32( "key_webtv", g_settings.key_webtv );
	
	configfile.setInt32( "key_screenshot", g_settings.key_screenshot );
	
	// mb
	configfile.setInt32( "mb_copy_jump", g_settings.mb_copy_jump );
	configfile.setInt32( "mb_cut_jump", g_settings.mb_cut_jump );
	configfile.setInt32( "mb_truncate", g_settings.mb_truncate );
	
	// webtv
	configfile.setString( "webtv_userBouquet", g_settings.webtv_userBouquet);
	//configfile.setInt32("webtv_lastselectedchannel", webtv->getlastSelectedChannel());
	
        // USERMENU
        char txt1[81];
        char txt2[81];
        for(int button = 0; button < SNeutrinoSettings::BUTTON_MAX; button++) 
	{
                snprintf(txt1,80,"usermenu_tv_%s_text",usermenu_button_def[button]);
                txt1[80] = 0; // terminate for sure
                configfile.setString(txt1,g_settings.usermenu_text[button]);

                char* txt2ptr = txt2;
                snprintf(txt1,80,"usermenu_tv_%s",usermenu_button_def[button]);

                for(int pos = 0; pos < SNeutrinoSettings::ITEM_MAX; pos++) 
		{
                        if( g_settings.usermenu[button][pos] != 0) 
			{
                                if(pos != 0)
                                        *txt2ptr++ = ',';
                                txt2ptr += snprintf(txt2ptr,80,"%d",g_settings.usermenu[button][pos]);
                        }
                }
                configfile.setString(txt1,txt2);
        }
	// END KEYS

	// AUDIOPLAYER
	configfile.setString( "network_nfs_audioplayerdir", g_settings.network_nfs_audioplayerdir);

	configfile.setInt32( "audioplayer_display", g_settings.audioplayer_display );
	configfile.setInt32( "audioplayer_follow", g_settings.audioplayer_follow );
	configfile.setInt32( "audioplayer_highprio", g_settings.audioplayer_highprio );
	configfile.setInt32( "audioplayer_select_title_by_name", g_settings.audioplayer_select_title_by_name );
	configfile.setInt32( "audioplayer_repeat_on", g_settings.audioplayer_repeat_on );
	configfile.setInt32("audioplayer_hide_playlist", g_settings.audioplayer_hide_playlist);
	// END AUDIOPLAYER

	// PICVIEWER
	configfile.setString( "network_nfs_picturedir", g_settings.network_nfs_picturedir);

	configfile.setString( "picviewer_slide_time", g_settings.picviewer_slide_time );
	configfile.setInt32( "picviewer_scaling", g_settings.picviewer_scaling );
	// END PICVIEWER

	// MISC OPTS
	configfile.setInt32( "power_standby", g_settings.power_standby);
	configfile.setBool("shutdown_real", g_settings.shutdown_real);
	configfile.setBool("shutdown_real_rcdelay", g_settings.shutdown_real_rcdelay);
	configfile.setString("shutdown_count", g_settings.shutdown_count);

	configfile.setBool("infobar_sat_display", g_settings.infobar_sat_display  );
	configfile.setInt32("infobar_subchan_disp_pos", g_settings.infobar_subchan_disp_pos  );
	
	//crypticon channellist
	configfile.setInt32("channellist_ca", g_settings.channellist_ca);
	configfile.setInt32("make_hd_list", g_settings.make_hd_list);
	configfile.setBool("channellist_extended", g_settings.channellist_extended);
	//

	configfile.setString("timezone", g_settings.timezone);

	// epg
	configfile.setBool("epg_save", g_settings.epg_save);
	configfile.setBool("epg_read", g_settings.epg_read);
        configfile.setString("epg_cache_time", g_settings.epg_cache );
        configfile.setString("epg_extendedcache_time", g_settings.epg_extendedcache);
        configfile.setString("epg_old_events", g_settings.epg_old_events );
        configfile.setString("epg_max_events", g_settings.epg_max_events );
        configfile.setString("epg_dir", g_settings.epg_dir);
	
	for(int i = 0; i < 3; i++) 
	{
		sprintf(cfg_key, "pref_epgs_%d", i);
		configfile.setString(cfg_key, g_settings.pref_epgs[i]);
	}

	//filebrowser
	configfile.setBool  ("filesystem_is_utf8", g_settings.filesystem_is_utf8);
	configfile.setInt32("filebrowser_showrights", g_settings.filebrowser_showrights);
	configfile.setInt32("filebrowser_sortmethod", g_settings.filebrowser_sortmethod);
	configfile.setBool("filebrowser_denydirectoryleave", g_settings.filebrowser_denydirectoryleave);

	//
	configfile.setInt32( "channel_mode", g_settings.channel_mode );

	// scan/channellist
	configfile.setInt32( "rotor_swap", g_settings.rotor_swap);
	configfile.setInt32( "zap_cycle", g_settings.zap_cycle );
	configfile.setInt32( "sms_channel", g_settings.sms_channel );
	configfile.setBool("virtual_zap_mode", g_settings.virtual_zap_mode);
	
	//zapit setup
	configfile.setInt32("lastChannelMode", g_settings.lastChannelMode);
	configfile.setString("startchanneltv", g_settings.StartChannelTV );
	configfile.setString("startchannelradio", g_settings.StartChannelRadio );
	configfile.setInt64("startchanneltv_id", g_settings.startchanneltv_id);
	configfile.setInt64("startchannelradio_id", g_settings.startchannelradio_id);
	configfile.setInt32("startchanneltv_nr", g_settings.startchanneltv_nr);
	configfile.setInt32("startchannelradio_nr", g_settings.startchannelradio_nr);
	configfile.setInt32("uselastchannel", g_settings.uselastchannel);
	
	// radiotext	
	configfile.setBool("radiotext_enable", g_settings.radiotext_enable);	
	
	// logos_dir
	configfile.setString("logos_dir", g_settings.logos_dir);
	
	// epgplus logos
	configfile.setBool("epgplus_show_logo", g_settings.epgplus_show_logo);
	
	// infobar show channelname
	configfile.setBool("show_channelname", g_settings.show_channelname);
	
	// record screenshot
	configfile.setInt32("recording_screenshot", g_settings.recording_screenshot);
	
	// vol
	configfile.setInt32( "volume_pos", g_settings.volume_pos);
	configfile.setInt32( "current_volume", g_settings.current_volume );
	configfile.setString( "audio_step"	, g_settings.audio_step);

	configfile.setInt32("progressbar_color", g_settings.progressbar_color);
	// END MISC OPTS

	// HDD
	configfile.setInt32( "hdd_sleep", g_settings.hdd_sleep);
	configfile.setInt32( "hdd_noise", g_settings.hdd_noise);
	// END HDD

	// SOFT UPDATE
	configfile.setString("update_dir", g_settings.update_dir);
	configfile.setString("softupdate_proxyserver", g_settings.softupdate_proxyserver );
	configfile.setString("softupdate_proxyusername", g_settings.softupdate_proxyusername );
	configfile.setString("softupdate_proxypassword", g_settings.softupdate_proxypassword );
	configfile.setString("softupdate_url_file"      , g_settings.softupdate_url_file      );
	// END UPDATE

	// VFD 
	for (int i = 0; i < LCD_SETTING_COUNT; i++)
		configfile.setInt32(lcd_setting[i].name, g_settings.lcd_setting[i]);
	
	configfile.setString("lcd_dim_time", g_settings.lcd_setting_dim_time);
	configfile.setInt32("lcd_dim_brightness", g_settings.lcd_setting_dim_brightness);
	
	configfile.setInt32("lcd_ledcolor", g_settings.lcd_ledcolor);
	// END VFD

	// satip
	configfile.setBool("satip_allow_satip", g_settings.satip_allow_satip);
	configfile.setString("satip_serverbox_ip", g_settings.satip_serverbox_ip);
	configfile.setInt32("satip_serverbox_type", g_settings.satip_serverbox_type);
	configfile.setInt32("satip_serverbox_gui", g_settings.satip_serverbox_gui);

	// mode
	//configfile.setInt32("mode", mode);	

	if(strcmp(fname, NEUTRINO_SETTINGS_FILE))
		configfile.saveConfig(fname);

	else if (configfile.getModifiedFlag())
	{
		configfile.saveConfig(fname);
		configfile.setModifiedFlag(false);
	}
}

// firstChannel, get the initial channel
void CNeutrinoApp::firstChannel()
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::firstChannel\n");

	g_Zapit->getLastChannel(firstchannel.channelNumber, firstchannel.mode);
}

// CNeutrinoApp -  channelsInit, get the Channellist from zapit
void CNeutrinoApp::channelsInit(bool /*bOnly*/)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::channelsInit\n");

	const char * fav_bouquetname = g_Locale->getText(LOCALE_FAVORITES_BOUQUETNAME);

	if(g_bouquetManager->existsUBouquet(fav_bouquetname, true) == -1)
		g_bouquetManager->addBouquet(fav_bouquetname, true, true);

	if(TVallList) 
		delete TVallList;

	if(RADIOallList) 
		delete RADIOallList;

	if(TVbouquetList) 
		delete TVbouquetList;

	if(TVsatList) 
		delete TVsatList;

	if(TVfavList) 
		delete TVfavList;

	if(RADIObouquetList) 
		delete RADIObouquetList;

	if(RADIOsatList) 
		delete RADIOsatList;

	if(RADIOfavList) 
		delete RADIOfavList;

	if(TVchannelList) 
		delete TVchannelList;

	if(RADIOchannelList) 
		delete RADIOchannelList;

	TVchannelList = new CChannelList(g_Locale->getText(LOCALE_CHANNELLIST_HEAD));
	RADIOchannelList = new CChannelList(g_Locale->getText(LOCALE_CHANNELLIST_HEAD));

	TVbouquetList = new CBouquetList(g_Locale->getText(LOCALE_CHANNELLIST_PROVS));
	TVbouquetList->orgChannelList = TVchannelList;

	TVsatList = new CBouquetList(g_Locale->getText(LOCALE_CHANNELLIST_SATS));
	TVsatList->orgChannelList = TVchannelList;

	TVfavList = new CBouquetList(g_Locale->getText(LOCALE_CHANNELLIST_FAVS));
	TVfavList->orgChannelList = TVchannelList;

	RADIObouquetList = new CBouquetList(g_Locale->getText(LOCALE_CHANNELLIST_PROVS));
	RADIObouquetList->orgChannelList = RADIOchannelList;

	RADIOsatList = new CBouquetList(g_Locale->getText(LOCALE_CHANNELLIST_SATS));
	RADIOsatList->orgChannelList = RADIOchannelList;

	RADIOfavList = new CBouquetList(g_Locale->getText(LOCALE_CHANNELLIST_FAVS));
	RADIOfavList->orgChannelList = RADIOchannelList;

	uint32_t i;
	i = 1;

	int tvi = 0, ri = 0, hi = 0;
	
	// hd bouquet
	CBouquet * hdBouquet;
	if(g_settings.make_hd_list)
		hdBouquet = new CBouquet(0, (char *) "HD", 0);

	for (tallchans_iterator it = allchans.begin(); it != allchans.end(); it++) 
	{
		if (it->second.getServiceType() == ST_DIGITAL_TELEVISION_SERVICE) 
		{
			TVchannelList->putChannel(&(it->second));
			tvi++;

			if(it->second.isHD()) 
			{
				if(g_settings.make_hd_list)
					hdBouquet->channelList->addChannel(&(it->second));
				hi++;
			}
		}
		else if (it->second.getServiceType() == ST_DIGITAL_RADIO_SOUND_SERVICE) 
		{
			RADIOchannelList->putChannel(&(it->second));
			ri++;
		}
	}
	
	if(g_settings.make_hd_list)
		hdBouquet->channelList->SortSat();

	dprintf(DEBUG_NORMAL, "CNeutrinoApp::channelsInit: got %d TV (%d is HD) and %d RADIO channels\n", tvi, hi, ri);

	CBouquet * tmp;

	// tv all list
	TVallList = new CBouquetList(g_Locale->getText(LOCALE_CHANNELLIST_HEAD));
	tmp = TVallList->addBouquet(g_Locale->getText(LOCALE_CHANNELLIST_HEAD));
	*(tmp->channelList) = *TVchannelList;
	tmp->channelList->SortAlpha();
	TVallList->orgChannelList = TVchannelList;

	// radio all list
	RADIOallList = new CBouquetList(g_Locale->getText(LOCALE_CHANNELLIST_HEAD));
	tmp = RADIOallList->addBouquet(g_Locale->getText(LOCALE_CHANNELLIST_HEAD));
	*(tmp->channelList) = *RADIOchannelList;
	tmp->channelList->SortAlpha();
	RADIOallList->orgChannelList = RADIOchannelList;

	int bnum;
	sat_iterator_t sit;
	for(sit = satellitePositions.begin(); sit != satellitePositions.end(); sit++) 
	{
		tvi = 0, ri = 0;
		CBouquet * tmp1 = TVsatList->addBouquet(sit->second.name.c_str());
		CBouquet * tmp2 = RADIOsatList->addBouquet(sit->second.name.c_str());

		for (tallchans_iterator it = allchans.begin(); it != allchans.end(); it++) 
		{
			if(it->second.getSatellitePosition() == sit->first) 
			{
				if (it->second.getServiceType() == ST_DIGITAL_TELEVISION_SERVICE) 
				{
					tmp1->channelList->addChannel(&(it->second));
					tvi++;
				}
				else if (it->second.getServiceType() == ST_DIGITAL_RADIO_SOUND_SERVICE) 
				{
					tmp2->channelList->addChannel(&(it->second));
					ri++;
				}
			}
		}

		if(tvi)
			tmp1->channelList->SortAlpha();
		else
			TVsatList->deleteBouquet(tmp1);

		if(ri)
			tmp2->channelList->SortAlpha();
		else
			RADIOsatList->deleteBouquet(tmp2);

		if(tvi || ri)
			printf("CNeutrinoApp::channelsInit: created %s with %d TV and %d RADIO channels\n", sit->second.name.c_str(), tvi, ri);
	}

	// tv fav list
	bnum = 0;
	for (i = 0; i < g_bouquetManager->Bouquets.size(); i++) 
	{
		if (!g_bouquetManager->Bouquets[i]->bHidden && !g_bouquetManager->Bouquets[i]->tvChannels.empty())
		{
			CBouquet * ltmp;
			if(g_bouquetManager->Bouquets[i]->bUser) 
				ltmp = TVfavList->addBouquet(g_bouquetManager->Bouquets[i]);
			else
				ltmp = TVbouquetList->addBouquet(g_bouquetManager->Bouquets[i]);

			ZapitChannelList * channels = &(g_bouquetManager->Bouquets[i]->tvChannels);
			ltmp->channelList->setSize(channels->size());
			
			for(int j = 0; j < (int) channels->size(); j++) 
			{
				ltmp->channelList->addChannel((*channels)[j]);
			}
			bnum++;
		}
	}
	
	if(g_settings.make_hd_list)
		TVfavList->Bouquets.push_back(hdBouquet);
	
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::channelsInit: got %d TV bouquets\n", bnum);

	// radio fav list
	bnum = 0;
	for (i = 0; i < g_bouquetManager->Bouquets.size(); i++) 
	{	
		if (!g_bouquetManager->Bouquets[i]->bHidden && !g_bouquetManager->Bouquets[i]->radioChannels.empty() )
		{
			CBouquet * ltmp;
			if(g_bouquetManager->Bouquets[i]->bUser) 
				ltmp = RADIOfavList->addBouquet(g_bouquetManager->Bouquets[i]);
			else
				ltmp = RADIObouquetList->addBouquet(g_bouquetManager->Bouquets[i]);

			ZapitChannelList *channels = &(g_bouquetManager->Bouquets[i]->radioChannels);
			ltmp->channelList->setSize(channels->size());
			for(int j = 0; j < (int) channels->size(); j++) 
			{
				ltmp->channelList->addChannel((*channels)[j]);
			}
			bnum++;
		}
	}
	
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::channelsInit: got %d RADIO bouquets\n", bnum);

	SetChannelMode( g_settings.channel_mode, mode);
}

void CNeutrinoApp::SetChannelMode(int newmode, int nMode)
{
	const char *aLISTMODE[] = {
		"LIST_MODE_FAV",
		"LIST_MODE_PROV",
		"LIST_MODE_SAT",
		"LIST_MODE_ALL"
	};
	
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::SetChannelMode: ChannelsMode%s\n", aLISTMODE[newmode]);

	if(nMode == mode_radio)
		channelList = RADIOchannelList;
	else if(nMode = mode_tv)
		channelList = TVchannelList;

	switch(newmode) 
	{
		case LIST_MODE_FAV:
			if(nMode == mode_radio) 
			{
				bouquetList = RADIOfavList;
			} 
			else 
			{
				bouquetList = TVfavList;
			}
			break;

		case LIST_MODE_SAT:
			if(nMode == mode_radio) 
			{
				bouquetList = RADIOsatList;
			} 
			else 
			{
				bouquetList = TVsatList;
			}
			break;

		case LIST_MODE_ALL:
			if(nMode == mode_radio) 
			{
				bouquetList = RADIOallList;
			} 
			else 
			{
				bouquetList = TVallList;
			}
			break;

		default:
		case LIST_MODE_PROV:
			if(nMode == mode_radio) 
			{
				bouquetList = RADIObouquetList;
			} 
			else 
			{
				bouquetList = TVbouquetList;
			}
			break;
	}

	g_settings.channel_mode = newmode;
}

// CNeutrinoApp -  run, the main runloop
void CNeutrinoApp::CmdParser(int argc, char **argv)
{
        global_argv = new char *[argc + 1];
	
        for (int i = 0; i < argc; i++)
                global_argv[i] = argv[i];
	
        global_argv[argc] = NULL;

	for(int x = 1; x < argc; x++) 
	{
		if (((!strcmp(argv[x], "-v")) || (!strcmp(argv[x], "--verbose"))) && (x + 1 < argc)) 
		{
			int dl = atoi(argv[x + 1]);
			dprintf(DEBUG_NORMAL, "set debuglevel: %d\n", dl);
			setDebugLevel(dl);
			x++;
		}
		else 
		{
			dprintf(DEBUG_NORMAL, "Usage: neutrino [-v | --verbose 0..2]\n");
		}
	}
}

// setup the framebuffer
void CNeutrinoApp::SetupFrameBuffer()
{
	frameBuffer->init();
	
	if(frameBuffer->setMode() ) 
	{
		dprintf(DEBUG_NORMAL, "CNeutrinoApp::SetupFrameBuffer: Error while setting framebuffer mode\n");
		exit(-1);
	}	
}

// setup fonts
void CNeutrinoApp::SetupFonts()
{
	const char * style[3];

	if (g_fontRenderer != NULL)
		delete g_fontRenderer;

	g_fontRenderer = new FBFontRenderClass(72 * g_settings.screen_xres / 100, 72 * g_settings.screen_yres / 100);

	if(font.filename != NULL)
		free((void *)font.filename);

	dprintf(DEBUG_NORMAL, "CNeutrinoApp::SetupFonts: settings font file %s\n", g_settings.font_file);

	if(access(g_settings.font_file, F_OK)) 
	{
		if(!access(FONTDIR "/micron.ttf", F_OK))
		{
			font.filename = strdup(FONTDIR "/micron.ttf");
			strcpy(g_settings.font_file, font.filename);
		}
		else
		{
			  fprintf( stderr,"CNeutrinoApp::SetupFonts: font file [%s] not found\n neutrino exit\n", FONTDIR "/micron.ttf");
			  _exit(0);
		}
	}
	else
	{
		font.filename = strdup(g_settings.font_file);
		
		// check??? (use only true type fonts or fallback to micron.ttf
		if( !strstr(font.filename, ".ttf") )
		{
			dprintf(DEBUG_NORMAL, "CNeutrinoApp::SetupFonts: font file %s not ok falling back to micron.ttf\n", g_settings.font_file);
			
			if(!access(FONTDIR "/micron.ttf", F_OK))
			{
				font.filename = strdup(FONTDIR "/micron.ttf");
				strcpy(g_settings.font_file, font.filename);
			}
			else
			{
				  fprintf( stderr,"CNeutrinoApp::SetupFonts: font file [%s] not found\n neutrino exit\n", FONTDIR "/micron.ttf");
				  _exit(0);
			}
		}
	}

	style[0] = g_fontRenderer->AddFont(font.filename);

	if(font.name != NULL)
		free((void *)font.name);

	font.name = strdup(g_fontRenderer->getFamily(font.filename).c_str());

	dprintf(DEBUG_DEBUG, "CNeutrinoApp::SetupFonts: font family %s\n", font.name);

	style[1] = "Bold Regular";

	g_fontRenderer->AddFont(font.filename, true);  // make italics
	style[2] = "Italic";

	// set neutrino font
	for (int i = 0; i < FONT_TYPE_COUNT; i++)
	{
		if(g_Font[i] != NULL) 
			delete g_Font[i];

		g_Font[i] = g_fontRenderer->getFont(font.name, style[neutrino_font[i].style], neutrino_font[i].defaultsize + neutrino_font[i].size_offset * font.size_offset);
	}

	// set signal font
	g_SignalFont = g_fontRenderer->getFont(font.name, style[signal_font.style], signal_font.defaultsize + signal_font.size_offset * font.size_offset);

	// recalculate infobar position
	if (g_InfoViewer)
		g_InfoViewer->start();
}

// setup the menu timeouts
void CNeutrinoApp::SetupTiming()
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::SetupTiming\n");

	for (int i = 0; i < TIMING_SETTING_COUNT; i++)
		sprintf(g_settings.timing_string[i], "%d", g_settings.timing[i]);
};

// setup recording device
void CNeutrinoApp::setupRecordingDevice(void)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::setupRecordingDevice\n");

	if (recDir != NULL)
	{
		recordingdevice = new CVCRControl::CFileDevice(g_settings.network_nfs_recordingdir );

		CVCRControl::getInstance()->registerDevice(recordingdevice);
	}
	else
	{
		if (CVCRControl::getInstance()->isDeviceRegistered())
			CVCRControl::getInstance()->unregisterDevice();
	}
}

bool sectionsd_getActualEPGServiceKey(const t_channel_id uniqueServiceKey, CEPGData * epgdata);
bool sectionsd_getEPGid(const event_id_t epgID, const time_t startzeit, CEPGData * epgdata);

#if defined (USE_OPENGL)
int startOpenGLplayback()
{
	CTimerd::RecordingInfo eventinfo;

	if( !CVCRControl::getInstance()->isDeviceRegistered() )
		return 0;

	eventinfo.channel_id = live_channel_id;
	CEPGData epgData;
	
	if (sectionsd_getActualEPGServiceKey(live_channel_id&0xFFFFFFFFFFFFULL, &epgData ))
	{
		eventinfo.epgID = epgData.eventID;
		eventinfo.epg_starttime = epgData.epg_times.startzeit;
		strncpy(eventinfo.epgTitle, epgData.title.c_str(), EPG_TITLE_MAXLEN-1);
		eventinfo.epgTitle[EPG_TITLE_MAXLEN - 1] = 0;
	}
	else 
	{
		eventinfo.epgID = 0;
		eventinfo.epg_starttime = 0;
		strcpy(eventinfo.epgTitle, "");
	}

	eventinfo.apids = TIMERD_APIDS_CONF;

	(static_cast<CVCRControl::CFileDevice*>(recordingdevice))->Directory = timeshiftDir;

	if( CVCRControl::getInstance()->Record(&eventinfo))
		CNeutrinoApp::getInstance()->playbackstatus = 1;
	else
		CNeutrinoApp::getInstance()->playbackstatus = 0;
	
	// start playback	
	char fname[255];
	int cnt = 10 * 1000000;

	while (!strlen(rec_filename)) 
	{
			usleep(1000);
			cnt -= 1000;
			if (!cnt)
				break;
	}

	if(strlen(rec_filename))
	{
		sprintf(fname, "%s.ts", rec_filename);
		
		usleep(10000000);
		playback->Open();
		playback->Start(fname);
	} 
	
	return 0;
}

void stopOpenGLplayback()
{
	// stop playback
	playback->Close();
	
	// stop recording
	if(CNeutrinoApp::getInstance()->playbackstatus) 
	{
		CNeutrinoApp::getInstance()->playbackstatus = 0;
		
		CVCRControl::getInstance()->Stop();

		char buf[1024];
		char buf1[1024];

		sprintf(buf, "rm -f %s.ts &", rec_filename);
		sprintf(buf1, "%s.xml", rec_filename);

		system(buf);
		unlink(buf1);
	}
}
#endif

// start auto record (permanent/temp timeshift)
int startAutoRecord(bool addTimer)
{
	CTimerd::RecordingInfo eventinfo;

	if(CNeutrinoApp::getInstance()->recordingstatus || !CVCRControl::getInstance()->isDeviceRegistered() )
		return 0;

	eventinfo.channel_id = live_channel_id;
	CEPGData epgData;
	
	if (sectionsd_getActualEPGServiceKey(live_channel_id&0xFFFFFFFFFFFFULL, &epgData ))
	{
		eventinfo.epgID = epgData.eventID;
		eventinfo.epg_starttime = epgData.epg_times.startzeit;
		strncpy(eventinfo.epgTitle, epgData.title.c_str(), EPG_TITLE_MAXLEN-1);
		eventinfo.epgTitle[EPG_TITLE_MAXLEN-1]=0;
	}
	else 
	{
		eventinfo.epgID = 0;
		eventinfo.epg_starttime = 0;
		strcpy(eventinfo.epgTitle, "");
	}

	eventinfo.apids = TIMERD_APIDS_CONF;
	
	dprintf(DEBUG_NORMAL, "startAutoRecord: dir %s\n", timeshiftDir);

	(static_cast<CVCRControl::CFileDevice*>(recordingdevice))->Directory = timeshiftDir;

	autoshift = 1;
	CNeutrinoApp::getInstance()->recordingstatus = 1;
	CNeutrinoApp::getInstance()->timeshiftstatus = 1;
	tmode = "ptimeshift";

	if( CVCRControl::getInstance()->Record(&eventinfo) == false ) 
	{
		CNeutrinoApp::getInstance()->recordingstatus = 0;
		CNeutrinoApp::getInstance()->timeshiftstatus = 0;
		autoshift = 0;
		
		CVFD::getInstance()->ShowIcon(VFD_ICON_TIMESHIFT, false );	
	}
	else if (addTimer) 
	{
		time_t now = time(NULL);
		CNeutrinoApp::getInstance()->recording_id = g_Timerd->addImmediateRecordTimerEvent(eventinfo.channel_id, now, now+g_settings.record_hours*60*60, eventinfo.epgID, eventinfo.epg_starttime, eventinfo.apids);
	}	

	CVFD::getInstance()->ShowIcon(VFD_ICON_TIMESHIFT, true);

	return 0;
}

// stop auto record
void stopAutoRecord()
{
	if(autoshift && CNeutrinoApp::getInstance()->recordingstatus) 
	{
		dprintf(DEBUG_NORMAL, "stopAutoRecord: autoshift, recordingstatus %d, stopping ...\n", CNeutrinoApp::getInstance()->recordingstatus);

		CVCRControl::getInstance()->Stop();
		autoshift = false;
		
		if(CNeutrinoApp::getInstance()->recording_id) 
		{
			g_Timerd->stopTimerEvent(CNeutrinoApp::getInstance()->recording_id);
			CNeutrinoApp::getInstance()->recording_id = 0;
		}
	} 
	else if(shift_timer)  
	{
		g_RCInput->killTimer(shift_timer);
		shift_timer = 0;
	}
	
	CVFD::getInstance()->ShowIcon(VFD_ICON_TIMESHIFT, false);
}

// do gui-record
bool CNeutrinoApp::doGuiRecord(char * preselectedDir, bool addTimer)
{
	CTimerd::RecordingInfo eventinfo;
	bool refreshGui = false;

	if(CVCRControl::getInstance()->isDeviceRegistered()) 
	{
		// stop auto record
		if(autoshift) 
			stopAutoRecord();

		//
		if(recordingstatus == 1) 
		{
			puts("CNeutrinoApp::doGuiRecord: executing " NEUTRINO_RECORDING_START_SCRIPT ".");

			if (system(NEUTRINO_RECORDING_START_SCRIPT) != 0)
				perror(NEUTRINO_RECORDING_START_SCRIPT " failed");
			
			//
			time_t now = time(NULL);
			int record_end = now + g_settings.record_hours*60*60;
			int pre = 0, post = 0;

			// get EPG info
			eventinfo.channel_id = live_channel_id;
			CEPGData epgData;

			if (sectionsd_getActualEPGServiceKey(live_channel_id&0xFFFFFFFFFFFFULL, &epgData ))
			{
				eventinfo.epgID = epgData.eventID;
				eventinfo.epg_starttime = epgData.epg_times.startzeit;
				strncpy(eventinfo.epgTitle, epgData.title.c_str(), EPG_TITLE_MAXLEN-1);
				eventinfo.epgTitle[EPG_TITLE_MAXLEN - 1] = 0;
				
				// record end time
				g_Timerd->getRecordingSafety(pre, post);
				
				if (epgData.epg_times.startzeit > 0)
					record_end = epgData.epg_times.startzeit + epgData.epg_times.dauer + post;
			}
			else 
			{
				eventinfo.epgID = 0;
				eventinfo.epg_starttime = 0;
				strcpy(eventinfo.epgTitle, "");
			}

			eventinfo.apids = TIMERD_APIDS_CONF;

			bool doRecord = true;

			// rec dir
			strcpy(recDir, (preselectedDir != NULL) ? preselectedDir : g_settings.network_nfs_recordingdir);
				
			(static_cast<CVCRControl::CFileDevice*>(recordingdevice))->Directory = recDir;
			
			dprintf(DEBUG_NORMAL, "CNeutrinoApp::doGuiRecord: start record to dir %s\n", recDir);

			// start to record baby
			if( !doRecord || (CVCRControl::getInstance()->Record(&eventinfo) == false ) ) 
			{
				recordingstatus = 0;

				if(doRecord)
					return true;// try to refresh gui if record was not ok ?

				return refreshGui;
			}
			else if (addTimer) // add timer
			{
				recording_id = g_Timerd->addImmediateRecordTimerEvent(eventinfo.channel_id, now, record_end, eventinfo.epgID, eventinfo.epg_starttime, eventinfo.apids);
			}
		} 
		else 
		{
			g_Timerd->stopTimerEvent(recording_id);
			
			startNextRecording();
		}
		
		CVFD::getInstance()->ShowIcon(VFD_ICON_TIMESHIFT, true);

		return refreshGui;
	}
	else
		puts("CNeutrinoApp::doGuiRecord: no recording devices");


	return false;
}

// start next recording
void CNeutrinoApp::startNextRecording()
{
	if ( nextRecordingInfo != NULL ) 
	{
		bool doRecord = true;
		if (CVCRControl::getInstance()->isDeviceRegistered()) 
		{
			recording_id = nextRecordingInfo->eventID;
			
			if (recDir != NULL)
			{
				char *lrecDir = strlen(nextRecordingInfo->recordingDir) > 0 ? nextRecordingInfo->recordingDir : g_settings.network_nfs_recordingdir;

				if (!CFSMounter::isMounted(lrecDir)) 
				{
					doRecord = false;
					
					for(int i = 0 ; i < NETWORK_NFS_NR_OF_ENTRIES ; i++) 
					{
						if (strcmp(g_settings.network_nfs_local_dir[i], lrecDir) == 0) 
						{
							CFSMounter::MountRes mres =
								CFSMounter::mount(g_settings.network_nfs_ip[i].c_str(), g_settings.network_nfs_dir[i],
										  g_settings.network_nfs_local_dir[i], (CFSMounter::FSType) g_settings.network_nfs_type[i],
										  g_settings.network_nfs_username[i], g_settings.network_nfs_password[i],
										  g_settings.network_nfs_mount_options1[i], g_settings.network_nfs_mount_options2[i]);
										  
							if (mres == CFSMounter::MRES_OK) 
							{
								doRecord = true;
							} 
							else 
							{
								const char * merr = mntRes2Str(mres);
								int msglen = strlen(merr) + strlen(nextRecordingInfo->recordingDir) + 7;
								char msg[msglen];
								strcpy(msg, merr);
								strcat(msg, "\nDir: ");
								strcat(msg, nextRecordingInfo->recordingDir);

								HintBox(LOCALE_MESSAGEBOX_ERROR, msg); // UTF-8
								doRecord = false;
							}
							break;
						}
					}

					if (!doRecord) 
					{
						// recording dir does not seem to exist in config anymore
						// or an error occured while mounting
						// -> try default dir
						lrecDir = g_settings.network_nfs_recordingdir;
					
						doRecord = true;
					}
				}

				(static_cast<CVCRControl::CFileDevice*>(recordingdevice))->Directory = std::string(lrecDir);
				dprintf(DEBUG_NORMAL, "CNeutrinoApp::startNextRecording: start to dir %s\n", lrecDir);

				CVFD::getInstance()->ShowIcon(VFD_ICON_TIMESHIFT, true);
			}
			
			if(doRecord && CVCRControl::getInstance()->Record(nextRecordingInfo))
				recordingstatus = 1;
			else
				recordingstatus = 0;
		}
		else
			puts("CNeutrinoApp::startNextRecording: no recording devices");

		/* Note: CTimerd::RecordingInfo is a class!
		 * What a brilliant idea to send classes via the eventserver!
		 * => typecast to avoid destructor call
		 */
		delete [](unsigned char *)nextRecordingInfo;

		nextRecordingInfo = NULL;
	}
}

#define LCD_UPDATE_TIME_RADIO_MODE (6 * 1000 * 1000)
#define LCD_UPDATE_TIME_TV_MODE (60 * 1000 * 1000)

// send sectionsd config
void CNeutrinoApp::SendSectionsdConfig(void)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::SendSectionsdConfig\n");

        CSectionsdClient::epg_config config;
	
        config.epg_cache                = atoi(g_settings.epg_cache.c_str());
        config.epg_old_events           = atoi(g_settings.epg_old_events.c_str());
        config.epg_max_events           = atoi(g_settings.epg_max_events.c_str());
        config.epg_extendedcache        = atoi(g_settings.epg_extendedcache.c_str());
        config.epg_dir                  = g_settings.epg_dir;
        config.network_ntpserver        = g_settings.network_ntpserver;
        config.network_ntprefresh       = atoi(g_settings.network_ntprefresh.c_str());
        config.network_ntpenable        = g_settings.network_ntpenable;
	
        g_Sectionsd->setConfig(config);
}

// init zapper
void CNeutrinoApp::InitZapper()
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::InitZapper\n");

	// start infobar
	g_InfoViewer->start();
	
	// send sectionsd config
	SendSectionsdConfig();

	// read saved epg
	if (g_settings.epg_read)
		readEPG();

	// first channel
	firstChannel();
	
	// init channel
	channelsInit();

	// mode ? tv:radio
	if(firstchannel.mode == 't') 
	{
		tvMode(false);
	} 
	else 
	{
#if defined (ENABLE_LCD)	  
			g_RCInput->killTimer(g_InfoViewer->lcdUpdateTimer);
			g_InfoViewer->lcdUpdateTimer = g_RCInput->addTimer( LCD_UPDATE_TIME_RADIO_MODE, false );
#endif		
		
			radioMode(false);
	}

	// zap
	if(channelList->getSize() && live_channel_id) 
	{
		channelList->adjustToChannelID(live_channel_id);

		// show service name in vfd (250hd has only 4 digit so we show service number)
		if (CVFD::getInstance()->is4digits)
			CVFD::getInstance()->LCDshowText(channelList->getActiveChannelNumber());
		else
			CVFD::getInstance()->showServicename(channelList->getActiveChannelName());	

		// start epg scanning
		g_Sectionsd->setPauseScanning(false);
		g_Sectionsd->setServiceChanged(live_channel_id&0xFFFFFFFFFFFFULL, true );
		
		// process apids
		g_Zapit->getPIDS(g_RemoteControl->current_PIDs);
		g_RemoteControl->processAPIDnames();
				
		// permenant timeshift
		if(g_settings.auto_timeshift)
			startAutoRecord(true);
		
		// show info bar
		g_RCInput->postMsg(NeutrinoMessages::SHOW_INFOBAR, 0);
		
		SelectSubtitles();
		
		StartSubtitles();
	}
}

#if defined (PLATFORM_COOLSTREAM)
static void CSSendMessage(uint32_t msg, uint32_t data)
{
	if (g_RCInput)
		g_RCInput->postMsg(msg, data);
}
#endif

//
void CNeutrinoApp::initSectionsdClient()
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::initSectionsdClient\n");

	g_Sectionsd->registerEvent(CSectionsdClient::EVT_TIMESET, 222, NEUTRINO_UDS_NAME);
	g_Sectionsd->registerEvent(CSectionsdClient::EVT_GOT_CN_EPG, 222, NEUTRINO_UDS_NAME);
	g_Sectionsd->registerEvent(CSectionsdClient::EVT_SERVICES_UPDATE, 222, NEUTRINO_UDS_NAME);
	g_Sectionsd->registerEvent(CSectionsdClient::EVT_BOUQUETS_UPDATE, 222, NEUTRINO_UDS_NAME);
	g_Sectionsd->registerEvent(CSectionsdClient::EVT_WRITE_SI_FINISHED, 222, NEUTRINO_UDS_NAME);
}

void CNeutrinoApp::initZapitClient()
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::initZapitClient\n");

#define ZAPIT_EVENT_COUNT 30
	const CZapitClient::events zapit_event[ZAPIT_EVENT_COUNT] =
	{
		CZapitClient::EVT_ZAP_COMPLETE,
		CZapitClient::EVT_ZAP_COMPLETE_IS_NVOD,
		CZapitClient::EVT_ZAP_FAILED,
		CZapitClient::EVT_ZAP_SUB_COMPLETE,
		CZapitClient::EVT_ZAP_SUB_FAILED,
		CZapitClient::EVT_ZAP_MOTOR,
		CZapitClient::EVT_ZAP_CA_CLEAR,
		CZapitClient::EVT_ZAP_CA_LOCK,
		CZapitClient::EVT_ZAP_CA_FTA,
		CZapitClient::EVT_ZAP_CA_ID,
		CZapitClient::EVT_RECORDMODE_ACTIVATED,
		CZapitClient::EVT_RECORDMODE_DEACTIVATED,
		CZapitClient::EVT_SCAN_COMPLETE,
		CZapitClient::EVT_SCAN_FAILED,
		CZapitClient::EVT_SCAN_NUM_TRANSPONDERS,
		CZapitClient::EVT_SCAN_REPORT_NUM_SCANNED_TRANSPONDERS,
		CZapitClient::EVT_SCAN_REPORT_FREQUENCY,
		CZapitClient::EVT_SCAN_REPORT_FREQUENCYP,
		CZapitClient::EVT_SCAN_SATELLITE,
		CZapitClient::EVT_SCAN_NUM_CHANNELS,
		CZapitClient::EVT_SCAN_PROVIDER,
		CZapitClient::EVT_BOUQUETS_CHANGED,
		CZapitClient::EVT_SERVICES_CHANGED,
		CZapitClient::EVT_SCAN_SERVICENAME,
		CZapitClient::EVT_SCAN_FOUND_A_CHAN,
		CZapitClient::EVT_SCAN_FOUND_TV_CHAN,
		CZapitClient::EVT_SCAN_FOUND_RADIO_CHAN,
		CZapitClient::EVT_SCAN_FOUND_DATA_CHAN,
		CZapitClient::EVT_SDT_CHANGED,
		CZapitClient::EVT_PMT_CHANGED
	};

	for (int i = 0; i < ZAPIT_EVENT_COUNT; i++)
		g_Zapit->registerEvent(zapit_event[i], 222, NEUTRINO_UDS_NAME);
}

void CNeutrinoApp::initTimerdClient()
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::initTimerdClient\n");

	g_Timerd->registerEvent(CTimerdClient::EVT_ANNOUNCE_SHUTDOWN, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_SHUTDOWN, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_ANNOUNCE_NEXTPROGRAM, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_NEXTPROGRAM, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_STANDBY_ON, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_STANDBY_OFF, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_ANNOUNCE_RECORD, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_RECORD_START, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_RECORD_STOP, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_ANNOUNCE_ZAPTO, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_ZAPTO, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_SLEEPTIMER, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_ANNOUNCE_SLEEPTIMER, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_REMIND, 222, NEUTRINO_UDS_NAME);
	g_Timerd->registerEvent(CTimerdClient::EVT_EXEC_PLUGIN, 222, NEUTRINO_UDS_NAME);
}

void CISendMessage(uint32_t msg, uint32_t data)
{
	if (g_RCInput)
	      g_RCInput->postMsg(msg, data);
}

int CNeutrinoApp::run(int argc, char **argv)
{
	dprintf( DEBUG_NORMAL, "CNeutrinoApp::run:\n");

	CmdParser(argc, argv);
	
#if defined (PLATFORM_COOLSTREAM)
	cs_api_init();
	cs_register_messenger(CSSendMessage);
#endif
	
	// font
	font.name = NULL;
	font.filename = NULL;

	// load settings
	int loadSettingsErg = loadSetup(NEUTRINO_SETTINGS_FILE);
	
	// init isomap
	initialize_iso639_map();

	// check locale language
	CLocaleManager::loadLocale_ret_t loadLocale_ret = g_Locale->loadLocale(g_settings.language);

	if (loadLocale_ret == CLocaleManager::NO_SUCH_LOCALE)
	{
		strcpy(g_settings.language, "english");
		loadLocale_ret = g_Locale->loadLocale(g_settings.language);
	}

	// setup fonts
	SetupFonts();
	
	// setup menue timing
	SetupTiming();
	
	// setup color
	colorSetupNotifier = new CColorSetupNotifier;
	colorSetupNotifier->changeNotify(NONEXISTANT_LOCALE, NULL);

	// init vfd/lcd display
#if ENABLE_LCD
	CVFD::getInstance()->init(font.filename, font.name);
#else	
	CVFD::getInstance()->init();
#endif	

	// VFD clear	
	CVFD::getInstance()->Clear();	

	// show start up msg in vfd
	CVFD::getInstance()->ShowText( (char *)"NHD2");	

	// rc 
	g_RCInput = new CRCInput;
	g_RCInput->setRepeat(atoi(g_settings.repeat_blocker), atoi(g_settings.repeat_genericblocker));

	// playback
	playback = new cPlayback();

	// plugins
	g_PluginList = new CPlugins;
	g_PluginList->setPluginDir(PLUGINDIR);

	// load Pluginlist before main menu (only show script menu if at least one script is available
	g_PluginList->loadPlugins();
	
	// zapit	
	Z_start_arg ZapStart_arg;
	
	ZapStart_arg.lastchannelmode = g_settings.lastChannelMode;
	ZapStart_arg.startchanneltv_id = g_settings.startchanneltv_id;
	ZapStart_arg.startchannelradio_id = g_settings.startchannelradio_id;
	ZapStart_arg.startchanneltv_nr = g_settings.startchanneltv_nr;
	ZapStart_arg.startchannelradio_nr = g_settings.startchannelradio_nr;
	ZapStart_arg.uselastchannel = g_settings.uselastchannel;
	
	ZapStart_arg.video_mode = g_settings.video_Mode;
	
	current_volume = g_settings.current_volume;

	// zapit
	pthread_create(&zapit_thread, NULL, zapit_main_thread, (void *) &ZapStart_arg);	

	// wait until zapit is ready
	while(!zapit_ready)
		usleep(0);
	
	// dvbsub thread
	dvbsub_init();

	// audio volume (default)
	if(audioDecoder)
		audioDecoder->setVolume(g_settings.current_volume, g_settings.current_volume);

	// video format
	if(videoDecoder)
		videoDecoder->setAspectRatio(g_settings.video_Ratio, g_settings.video_Format);
	
	// wss
	if(videoDecoder)
		videoDecoder->SetWideScreen(g_settings.wss_mode);
	
	// avsync
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

	if(audioDecoder)
	{
		// hdmi_dd
		audioDecoder->SetHdmiDD(g_settings.hdmi_dd );

		// ac3 delay
		audioDecoder->setHwAC3Delay(g_settings.ac3_delay);

		// pcm delay 
		audioDecoder->setHwPCMDelay(g_settings.pcm_delay);
	}

	// timerd thread
	pthread_create(&timer_thread, NULL, timerd_main_thread, (void *) NULL);

	// nhttpd thread
	pthread_create(&nhttpd_thread, NULL, nhttpd_main_thread, (void *) NULL);	

	// streamts thread
	pthread_create(&stream_thread, NULL, streamts_main_thread, (void *) NULL);	

	// sectionsd thread
	pthread_create(&sections_thread, NULL, sectionsd_main_thread, (void *) NULL);

	// for boxes with lcd :-)
#if ENABLE_LCD	
	CVFD::getInstance()->showVolume(g_settings.current_volume);
	CVFD::getInstance()->setMuted(current_muted);
#endif
	
	// sectionsd client
	g_Sectionsd = new CSectionsdClient;
	
	// timed client
	g_Timerd = new CTimerdClient;
	
	// remote control
	g_RemoteControl = new CRemoteControl;
	
	// epg Data
	g_EpgData = new CEpgData;
	
	// channel infoviewer
	g_InfoViewer = new CInfoViewer;
	
	// event list
	g_EventList = new EventList;
	
	// volume bar
	g_volscale = new CProgressBar(200, 15, 50, 100, 80, true);

	// Ci Cam handler
#if defined (ENABLE_CI)	
	g_CamHandler = new CCAMMenuHandler();
	g_CamHandler->init();	
#endif
	
	// init network at startup
	CNetworkSettings::getInstance()->readNetworkSettings(g_settings.ifname);

	if(CNetworkSettings::getInstance()->network_automatic_start == 1)
		CNetworkSettings::getInstance()->setNetwork();	
	
	// mount shares before scanning for plugins
	CFSMounter::automount();

	// assuming that mdev/fstab has mounted devices
	CHDDDestExec * hdd = new CHDDDestExec();
	hdd->exec(NULL, "");
	delete hdd;
	hdd = NULL;

	// plugins
	//g_PluginList = new CPlugins;
	//g_PluginList->setPluginDir(PLUGINDIR);

	// load Pluginlist before main menu (only show script menu if at least one script is available
	//g_PluginList->loadPlugins();
	
	// init nvod changer
	NVODChanger = new CNVODChangeExec;
	
	// init rclock
	rcLock = new CRCLock();

	// setup recording device
	setupRecordingDevice();

	// init main menu
	CMenuWidgetExtended mainMenu(LOCALE_MAINMENU_HEAD, NEUTRINO_ICON_MAINMENU);

	InitMainMenu(mainMenu);
	
	sleep(1);

	// init sectionsd client
	initSectionsdClient();

	// init zapit client
	initZapitClient();

	// init timerd client
	initTimerdClient();

	CVFD::getInstance()->setPower(g_settings.lcd_setting[SNeutrinoSettings::LCD_POWER]);
	CVFD::getInstance()->setlcdparameter();	
	
	// start assistant
	if(loadSettingsErg) 
	{
		// startup pic
		frameBuffer->loadBackgroundPic("start.jpg");	
		frameBuffer->blit();
	
		// setup languages
		CLanguageSettings languageSettings;
		int ret = languageSettings.exec(NULL, "");
	
		// video setup wizard
		if(ret != menu_return::RETURN_EXIT_ALL)
		{
			CVideoSettings videoSettings;
			videoSettings.exec(NULL, "");
		}
		
		 // audio setup wizard
		if(ret != menu_return::RETURN_EXIT_ALL)
		{
			CAudioSettings audioSettings;
			audioSettings.exec(NULL, "");
		}		

		// setup color
		if(ret != menu_return::RETURN_EXIT_ALL)
		{
			COSDSettings colorSettings;
			colorSettings.exec(NULL, "");
		}

		// setup timezone
		if(ret != menu_return::RETURN_EXIT_ALL)
			if(tzSelect)
				tzSelect->exec(NULL);

		// setup network
		if(ret != menu_return::RETURN_EXIT_ALL)
		{
			CNetworkSettings * networkSettings = new CNetworkSettings();
			networkSettings->exec(NULL, "");
			delete networkSettings;
		}
		
		// recordingsettings
		if(ret != menu_return::RETURN_EXIT_ALL)
		{
			CRecordingSettings recordingSettings;
			recordingSettings.exec(NULL, "");
		}
		
		// movieplayer settings
		if(ret != menu_return::RETURN_EXIT_ALL)
		{
			CMoviePlayerSettings moviePlayerSettings;
			moviePlayerSettings.exec(NULL, "");
		}
		
		// audioplayersettings
		if(ret != menu_return::RETURN_EXIT_ALL)
		{
			CAudioPlayerSettings audioPlayerSettings;
			audioPlayerSettings.exec(NULL, "");
		}
		
		// picviewersettings
		if(ret != menu_return::RETURN_EXIT_ALL)
		{
			CPictureViewerSettings PicViewerSettings;
			PicViewerSettings.exec(NULL, "");
		}
		
		// keysettings
		if(ret != menu_return::RETURN_EXIT_ALL)
		{
			CRemoteControlSettings bindSettings;
			bindSettings.exec(NULL, "");
		}
		
		// misc settings
		if(ret != menu_return::RETURN_EXIT_ALL)
		{
			CMiscSettings miscSettings;
			miscSettings.exec(NULL, "");
		}
		
		// service settings
		if(ret != menu_return::RETURN_EXIT_ALL)
		{
			CServiceSetup service;
			service.exec(NULL, "");
		}

		dprintf(DEBUG_INFO, "config file or options missing\n");

		HintBox(LOCALE_MESSAGEBOX_INFO, loadSettingsErg ==  1 ? g_Locale->getText(LOCALE_SETTINGS_NOCONFFILE) : g_Locale->getText(LOCALE_SETTINGS_MISSINGOPTIONSCONFFILE));
		
		configfile.setModifiedFlag(true);

		saveSetup(NEUTRINO_SETTINGS_FILE);
	}
	
	// zapper
	InitZapper();
	
	// audio mute
	AudioMute(current_muted, true);	

// Cam-Ci
#if defined (ENABLE_CI)	
	cDvbCi::getInstance()->SetHook(CISendMessage);	
#endif	
	
	// init shutdown count
	SHTDCNT::getInstance()->init();

	// real run ;-)
	RealRun(mainMenu);

	// exitRun
	ExitRun(SHUTDOWN);

	// never reached
	return 0;
}

// quickZap
void CNeutrinoApp::quickZap(int msg)
{
	StopSubtitles();
	
	if(mode == mode_iptv)
	{
		if(webtv)
			webtv->quickZap(msg);
	}
	else
	{
		if(g_settings.zap_cycle && (bouquetList!=NULL) && !(bouquetList->Bouquets.empty()))
			bouquetList->Bouquets[bouquetList->getActiveBouquetNumber()]->channelList->quickZap(msg, true);
		else
			channelList->quickZap(msg);
	}
}

// showInfo
void CNeutrinoApp::showInfo()
{
	StopSubtitles();

	g_InfoViewer->showTitle(channelList->getActiveChannelNumber(), channelList->getActiveChannelName(), channelList->getActiveSatellitePosition(), channelList->getActiveChannel_ChannelID());

	StartSubtitles();
}

// checking timer
//FIXME: @dbo: why? cant your vfd do this?
#if defined(PLATFORM_KATHREIN) || defined(PLATFORM_SPARK7162)
static void check_timer() 
{
	CTimerd::TimerList tmpTimerList;
	CTimerdClient tmpTimerdClient;
	tmpTimerList.clear();
	tmpTimerdClient.getTimerList(tmpTimerList);
	
	if(tmpTimerList.size() > 0) 
	{
		CVFD::getInstance()->ShowIcon(VFD_ICON_CLOCK, true);
	} 
	else 
	{
		CVFD::getInstance()->ShowIcon(VFD_ICON_CLOCK, false);
	}
	tmpTimerList.clear();
}
#endif

// real run
void CNeutrinoApp::RealRun(CMenuWidgetExtended& _mainMenu)
{
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	dprintf(DEBUG_NORMAL, "CNeutrinoApp::RealRun: initialized everything\n");

	// clear msg 
	g_RCInput->clearRCMsg();

	// if start to standby */
	if(g_settings.power_standby)
	{
		standbyMode(true);
	}

	// neutrino run loop
	while( true ) 
	{
		g_RCInput->getMsg(&msg, &data, 100);	// 10 secs..
		
		//FIXME: @dbo???
#if defined(PLATFORM_KATHREIN) || defined(PLATFORM_SPARK7162)
		check_timer();
#endif		

		// mode TV/Radio/IPTV
		if( (mode == mode_tv) || (mode == mode_radio) || (mode == mode_iptv) ) 
		{
			if(msg == NeutrinoMessages::SHOW_EPG && (mode != mode_iptv)) 
			{
				StopSubtitles();
				
				g_EpgData->show(live_channel_id);

				StartSubtitles();
			}
			else if( msg == CRCInput::RC_epg && (mode != mode_iptv)) 
			{
				StopSubtitles();
				
				g_EventList->exec(live_channel_id, channelList->getActiveChannelName());

				StartSubtitles();
			}
			else if( msg == CRCInput::RC_text && (mode != mode_iptv)) 
			{
				g_RCInput->clearRCMsg();

				StopSubtitles();
				
				tuxtx_stop_subtitle();

				tuxtx_main(g_RemoteControl->current_PIDs.PIDs.vtxtpid, 0, live_fe?live_fe->fenumber : 0 );

				frameBuffer->paintBackground();

				frameBuffer->blit();
				
				g_RCInput->clearRCMsg();
				
				AudioMute(current_muted, true);

				StartSubtitles();
			}			
			else if( msg == (neutrino_msg_t)g_settings.key_timerlist && (mode != mode_iptv)) //timerlist
			{
				StopSubtitles();
				
				CTimerList * Timerlist = new CTimerList;
				Timerlist->exec(NULL, "");
				delete Timerlist;
				Timerlist = NULL;
				
				StartSubtitles();
			}		
			else if( msg == CRCInput::RC_setup ) 
			{
				StopSubtitles();

				if(g_settings.menu_design == SNeutrinoSettings::MENU_DESIGN_STANDARD)
					_mainMenu.exec(NULL, "");
				else if(g_settings.menu_design == SNeutrinoSettings::MENU_DESIGN_CLASSIC)
					classicMenu();
				else if(g_settings.menu_design == SNeutrinoSettings::MENU_DESIGN_SMART)
					smartMenu();

				// restore mute symbol
				AudioMute(current_muted, true);

				StartSubtitles();
			}
			else if( msg == (neutrino_msg_t) g_settings.key_tvradio_mode && (mode != mode_iptv)) 
			{
				if( mode == mode_tv )
					radioMode();
				else if( mode == mode_radio )
					tvMode();
			}
			else if(( msg == (neutrino_msg_t) g_settings.key_quickzap_up ) || ( msg == (neutrino_msg_t) g_settings.key_quickzap_down ))
			{
				quickZap(msg);
			}
			else if( msg == (neutrino_msg_t) g_settings.key_subchannel_up && (mode != mode_iptv)) 
			{
			   	if(g_RemoteControl->subChannels.size() > 0) 
				{
					StopSubtitles();
					g_RemoteControl->subChannelUp();
					g_InfoViewer->showSubchan(); 
			    	} 
			    	else if(g_settings.virtual_zap_mode) 
				{
					if(channelList->getSize()) 
						showInfo();	
				}
				else
				{
					quickZap(msg);
				}
			}
			else if( msg == (neutrino_msg_t) g_settings.key_subchannel_down && (mode != mode_iptv)) 
			{
			   	if(g_RemoteControl->subChannels.size()> 0) 
				{
					StopSubtitles();
					g_RemoteControl->subChannelDown();
					g_InfoViewer->showSubchan();
			    	} 
			    	else if(g_settings.virtual_zap_mode) 
				{
					if(channelList->getSize()) 
						showInfo();	
				}
				else
				{
					quickZap(msg);
				}
			}
			// in case key_subchannel_up/down redefined
			else if( (msg == CRCInput::RC_left || msg == CRCInput::RC_right) && (mode != mode_iptv) ) 
			{
				if(channelList->getSize()) 
				{
					showInfo();
				}
			}
			else if( msg == (neutrino_msg_t) g_settings.key_zaphistory && (mode != mode_iptv)) 
			{
				StopSubtitles();
				
				// Zap-History "Bouquet"
				int res = channelList->numericZap( msg );

				StartSubtitles(res < 0);
			}
			else if( msg == (neutrino_msg_t) g_settings.key_lastchannel && (mode != mode_iptv)) 
			{
				StopSubtitles();
				
				// Quick Zap
				int res = channelList->numericZap( msg );

				StartSubtitles(res < 0);
			}
			else if( msg == (neutrino_msg_t) g_settings.key_timeshift) // start timeshift recording
			{
				if(mode == mode_iptv)
				{
					if(webtv)
					{
						if(webtv->playstate == CWebTV::PAUSE)
							webtv->continuePlayBack();
						else if(webtv->playstate == CWebTV::PLAY)
							webtv->pausePlayBack();
					}
				}
				else
				{
					if (recDir != NULL)
					{
						if(recordingstatus)
							tmode = "ptimeshift"; 	// already recording, pause(timeshift)
						else
							tmode = "timeshift";

						if(g_RemoteControl->is_video_started) 
						{		
							// ptimeshift
							if(recordingstatus) 
							{
								timeshiftstatus = recordingstatus;
							} 
							else
							{
								// timeshift
								recordingstatus = 1;
									
								timeshiftstatus = recordingstatus;

								doGuiRecord(timeshiftDir, true);
							}
							
							// freeze audio/video
							audioDecoder->Stop();
							videoDecoder->Stop(false); // dont blank
						}
					}
				}
			}
			else if( ((msg == (neutrino_msg_t)g_settings.mpkey_play || msg == (neutrino_msg_t)g_settings.mpkey_rewind) && timeshiftstatus) && (mode != mode_iptv)) // play timeshift
			{
				if(msg == CRCInput::RC_rewind)
					tmode = "rtimeshift"; // rewind
						
				dprintf(DEBUG_NORMAL, "[neutrino] %s\n", tmode.c_str());
						
				if(g_RemoteControl->is_video_started) 
				{
					CMoviePlayerGui tmpMoviePlayerGui;
					tmpMoviePlayerGui.exec(NULL, tmode);
				}
			}
			else if(msg == (neutrino_msg_t)g_settings.mpkey_play && mode == mode_iptv)
			{
				if(mode == mode_iptv)
				{
					if(webtv)
					{
						if(webtv->playstate == CWebTV::PAUSE)
							webtv->continuePlayBack();
						else if(webtv->playstate == CWebTV::STOPPED)
							webtv->startPlayBack(webtv->getTunedChannel());
					}
				}
			}
			else if( (msg == CRCInput::RC_record || msg == CRCInput::RC_stop) && (mode != mode_iptv) ) 
			{
				dprintf(DEBUG_NORMAL, "CNeutrinoApp::RealRun\n");
				
				if(autoshift) 
				{
					stopAutoRecord();
					recordingstatus = 0;
					timeshiftstatus = 0;
				}

				// stop record if recording
				if( recordingstatus ) 
				{
					if(MessageBox(LOCALE_MESSAGEBOX_INFO, LOCALE_SHUTDOWN_RECODING_QUERY, CMessageBox::mbrYes, CMessageBox::mbYes | CMessageBox::mbNo, NULL, 450, 30, true) == CMessageBox::mbrYes)
					{
						g_Timerd->stopTimerEvent(recording_id);
						CVFD::getInstance()->ShowIcon(VFD_ICON_TIMESHIFT, false );
					}
				} 
				// start record
				else if(msg != CRCInput::RC_stop )
				{
					recordingstatus = 1;
					doGuiRecord( g_settings.network_nfs_recordingdir, true );
				}
			}
			else if(msg == CRCInput::RC_stop && mode == mode_iptv) 
			{
				if(mode == mode_iptv)
				{
					if(webtv)
					{
						if(webtv->playstate == CWebTV::PLAY || webtv->playstate == CWebTV::PAUSE)
							webtv->stopPlayBack();
					}
				}
			}
			else if( msg == CRCInput::RC_red ) 
			{
				if(mode == mode_iptv)
				{
					if(webtv)
						webtv->showFileInfoWebTV(webtv->getTunedChannel());
				}
				else
				{
					StopSubtitles();
					// event list
					showUserMenu(SNeutrinoSettings::BUTTON_RED);
					StartSubtitles();
				}
			}
			else if( ( msg == CRCInput::RC_green) || ( msg == CRCInput::RC_audio) )
			{
				if(mode == mode_iptv)
				{
					CAVPIDSelectWidget * AVSelectHandler = new CAVPIDSelectWidget();
					AVSelectHandler->exec(NULL, "");
							
					delete AVSelectHandler;
					AVSelectHandler = NULL;
				}
				else
				{
					StopSubtitles();
					// audio
					showUserMenu(SNeutrinoSettings::BUTTON_GREEN);
					StartSubtitles();
				}
			}
			else if( (msg == CRCInput::RC_yellow || msg == CRCInput::RC_multifeed) && (mode != mode_iptv))
			{ 
				StopSubtitles();
				// NVODs
				showUserMenu(SNeutrinoSettings::BUTTON_YELLOW);
				StartSubtitles();
			}
			else if( msg == CRCInput::RC_blue ) 
			{
				StopSubtitles();

				// features
				showUserMenu(SNeutrinoSettings::BUTTON_BLUE);

				StartSubtitles();
			}
#if defined (ENABLE_FUNCTIONKEYS)			
			else if( msg == CRCInput::RC_f1 ) 
			{
				StopSubtitles();
				showUserMenu(SNeutrinoSettings::BUTTON_F1);
				StartSubtitles();
			}
			else if( msg == CRCInput::RC_f2 )
			{
				StopSubtitles();
				showUserMenu(SNeutrinoSettings::BUTTON_F2);
				StartSubtitles();
			}
			else if( msg == CRCInput::RC_f3 ) 
			{ 
				StopSubtitles();
				showUserMenu(SNeutrinoSettings::BUTTON_F3);
				StartSubtitles();
			}
			else if( msg == CRCInput::RC_f4 ) 
			{
				StopSubtitles();
				showUserMenu(SNeutrinoSettings::BUTTON_F4);
				StartSubtitles();
			}
#endif			
			else if( (msg == CRCInput::RC_dvbsub) && (mode != mode_iptv) )
			{
				StopSubtitles();
				
				// show list only if we have subs
				if(live_channel)
				{
					if(live_channel->getSubtitleCount() > 0)
					{
						CDVBSubSelectMenuHandler tmpDVBSubSelectMenuHandler;
						tmpDVBSubSelectMenuHandler.exec(NULL, "");
					}
				}
				
				StartSubtitles();
			}
			else if( msg == (neutrino_msg_t)g_settings.key_audioplayer ) 
			{
				CAudioPlayerGui tmpAudioPlayerGui;
				tmpAudioPlayerGui.exec(NULL, "");
			}
			else if( msg == (neutrino_msg_t)g_settings.key_inetradio ) 	// internet radio
			{	  
				StopSubtitles();

				CAudioPlayerGui tmpAudioPlayerGui(true);
				tmpAudioPlayerGui.exec(NULL, "");

				StartSubtitles();	
			}			
			else if( msg == (neutrino_msg_t)g_settings.key_recordsbrowser )	// recordsbrowser
			{
				CMoviePlayerGui tmpMoviePlayerGui;
				tmpMoviePlayerGui.exec(NULL, "tsmoviebrowser");

				if( mode == mode_radio )
				{
					if (!g_settings.radiotext_enable)
					{
						frameBuffer->loadBackgroundPic("radiomode.jpg");
						frameBuffer->blit();	
					}
				}			
			}
			else if( msg == (neutrino_msg_t)g_settings.key_moviebrowser )	// moviebrowser
			{
				CMoviePlayerGui tmpMoviePlayerGui;
				tmpMoviePlayerGui.exec(NULL, "moviebrowser");

				if( mode == mode_radio )
				{
					if (!g_settings.radiotext_enable)
					{
						frameBuffer->loadBackgroundPic("radiomode.jpg");
						frameBuffer->blit();	
					}
				}	
			}
			else if( msg == (neutrino_msg_t)g_settings.key_filebrowser )	// filebrowser player
			{
				CMoviePlayerGui tmpMoviePlayerGui;
				tmpMoviePlayerGui.exec(NULL, "fileplayback");

				if( mode == mode_radio )
				{
					if (!g_settings.radiotext_enable)
					{
						frameBuffer->loadBackgroundPic("radiomode.jpg");
						frameBuffer->blit();	
					}
				}	
			}
			else if( msg == (neutrino_msg_t)g_settings.key_webtv)	// webtv
			{
				StopSubtitles();

				webtvMode();
					
				StartSubtitles();
			}	
			else if( msg == (neutrino_msg_t)g_settings.key_pictureviewer ) 	// picture viewer
			{
				CPictureViewerGui tmpPictureViewerGui;
				tmpPictureViewerGui.exec(NULL, "");
			}			
			else if ( CRCInput::isNumeric(msg) && g_RemoteControl->director_mode && (mode != mode_iptv)) 
			{
				StopSubtitles();
				
				g_RemoteControl->setSubChannel(CRCInput::getNumericValue(msg));
				
				g_InfoViewer->showSubchan();
				
				StartSubtitles();
			}
			else if (CRCInput::isNumeric(msg) && (mode != mode_iptv)) 
			{
				StopSubtitles();
				
				channelList->numericZap( msg );
				
				StartSubtitles();
			}			
			else if (CRCInput::isNumeric(msg) && (mode == mode_radio && g_settings.radiotext_enable && g_Radiotext != NULL && g_Radiotext->Rass_Show) ) 
			{
				// convert msg to int
				int QKey = 0;
				if(msg == CRCInput::RC_0)
					QKey = 0;
				else if(msg == CRCInput::RC_1)
					QKey = 1;
				else if(msg == CRCInput::RC_2)
					QKey = 2;
				else if(msg == CRCInput::RC_3)
					QKey = 3;
				else if(msg == CRCInput::RC_4)
					QKey = 4;
				else if(msg == CRCInput::RC_5)
					QKey = 5;
				else if(msg == CRCInput::RC_6)
					QKey = 6;
				else if(msg == CRCInput::RC_7)
					QKey = 7;
				else if(msg == CRCInput::RC_8)
					QKey = 8;
				else if(msg == CRCInput::RC_9)
					QKey = 9;

				//FIXME: think about gallery i.e QKey > 9

				g_Radiotext->RassImage(0, QKey, true);
			}			
			else if((msg == CRCInput::RC_info) || ( msg == NeutrinoMessages::SHOW_INFOBAR ))
			{
				if(mode == mode_iptv)
				{
					if(webtv)
					{
						webtv->showInfo();
					}
				}
				else
				{
					bool show_info = ((msg != NeutrinoMessages::SHOW_INFOBAR) || (g_InfoViewer->is_visible || g_settings.timing[SNeutrinoSettings::TIMING_INFOBAR] != 0));
					
					// turn on LCD display
					CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
					
					if(show_info && channelList->getSize()) 
					{
						showInfo();
					}
				}
			}
			else if( msg == (neutrino_msg_t) g_settings.key_pip && (mode != mode_iptv))
			{
				StopSubtitles();
				
				// first steo show channels from the same TP
				channelList->numericZap( msg );
				
				StartSubtitles();
			}
			else 
			{
				if ( msg == CRCInput::RC_home )
				{ 
  					CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
				}

				handleMsg(msg, data);
			}
		}
		else //other modes
		{
			if( msg == CRCInput::RC_home ) 
			{
				if( mode == mode_scart ) 
				{
					//wenn VCR Aufnahme dann stoppen
					if(CVCRControl::getInstance()->isDeviceRegistered()) 
					{
						if ((CVCRControl::getInstance()->Device->getDeviceType() == CVCRControl::DEVICE_VCR) &&
						    (CVCRControl::getInstance()->getDeviceState() == CVCRControl::CMD_VCR_RECORD ||
						     CVCRControl::getInstance()->getDeviceState() == CVCRControl::CMD_VCR_PAUSE))
						{
							CVCRControl::getInstance()->Stop();
							recordingstatus = 0;
							startNextRecording();
						}
					}
					// Scart-Mode verlassen
					scartMode( false );
				}
			}
			else 
			{
				// hier geht es weiter
				handleMsg(msg, data);
			}
		}
	}
}

// handle msg
int CNeutrinoApp::handleMsg(const neutrino_msg_t msg, neutrino_msg_data_t data)
{
	int res = 0;

	// handle neutrino msg
	if(msg == NeutrinoMessages::EVT_ZAP_COMPLETE) 
	{
		// set audio map after channel zap
		g_Zapit->getAudioMode(&g_settings.audio_AnalogMode);

		if(g_settings.audio_AnalogMode < 0 || g_settings.audio_AnalogMode > 2)
			g_settings.audio_AnalogMode = 0;

		// kill shift timer
		if(shift_timer) 
		{
			g_RCInput->killTimer(shift_timer);
			shift_timer = 0;
		}	

		// auto timeshift
		if (!recordingstatus && g_settings.auto_timeshift) 		  
		{
			int delay = g_settings.auto_timeshift;
			
			// add shift timer
			shift_timer = g_RCInput->addTimer( delay*1000*1000, true );
			
			// infoviewer handle msg
			g_InfoViewer->handleMsg(NeutrinoMessages::EVT_RECORDMODE, 1);
		}	

		// scrambled timer
		if(scrambled_timer) 
		{
			g_RCInput->killTimer(scrambled_timer);
			scrambled_timer = 0;
		}

		scrambled_timer = g_RCInput->addTimer(10*1000*1000, true);
		
		// select subtitle
		SelectSubtitles();
		
		StartSubtitles(!g_InfoViewer->is_visible);
	}

	if ((msg == NeutrinoMessages::EVT_TIMER)) 
	{
		if(data == shift_timer) 
		{
			shift_timer = 0;
			startAutoRecord(true);
			
			return messages_return::handled;
		} 
		else if(data == scrambled_timer) 
		{
			scrambled_timer = 0;
			
			//if(videoDecoder->getBlank() && videoDecoder->getPlayState()) 
			//{
			//	const char * text = g_Locale->getText(LOCALE_SCRAMBLED_CHANNEL);
			//	HintBox(LOCALE_MESSAGEBOX_INFO, text, g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth (text, true) + 10, 5);
			//}

			return messages_return::handled;	
		}
	}

	// handle msg with remotecontrol/Infoviewer/Channellist
	res = res | g_RemoteControl->handleMsg(msg, data);
	res = res | g_InfoViewer->handleMsg(msg, data);
	res = res | channelList->handleMsg(msg, data);
	
	if( res != messages_return::unhandled ) 
	{
		if( ( msg>= CRCInput::RC_WithData ) && ( msg< CRCInput::RC_WithData + 0x10000000 ) )
			delete (unsigned char*) data;

		return( res & ( 0xFFFFFFFF - messages_return::unhandled ) );
	}

	// we assume g_CamHandler free/delete data if needed
#if defined (ENABLE_CI)	
	res = g_CamHandler->handleMsg(msg, data);
	if( res != messages_return::unhandled ) 
	{
		return(res & (0xFFFFFFFF - messages_return::unhandled));
	}
#endif	

	// handle Keys
	if( msg == CRCInput::RC_ok || msg == CRCInput::RC_sat || msg == CRCInput::RC_favorites)
	{
		if( (mode == mode_tv) || (mode == mode_radio))
		{
			StopSubtitles();

			// pre-selected channel-num/bouquet-num/channel-mode
			int nNewChannel = -1;
			int old_num = 0;
			int old_b = bouquetList->getActiveBouquetNumber();
			int old_mode = g_settings.channel_mode;
			
			dprintf(DEBUG_NORMAL, "CNeutrinoApp::handleMsg: ZAP START: bouquetList %x size %d old_b %d\n", (size_t) bouquetList, bouquetList->Bouquets.size(), old_b);

			if(bouquetList->Bouquets.size()) 
			{
				old_num = bouquetList->Bouquets[old_b]->channelList->getActiveChannelNumber();
			}

			if( msg == CRCInput::RC_ok ) 
			{
				if(bouquetList->Bouquets.size() && bouquetList->Bouquets[old_b]->channelList->getSize() > 0)
					nNewChannel = bouquetList->Bouquets[old_b]->channelList->exec();	//with ZAP!
				else
					nNewChannel = bouquetList->exec(true);
			}
			else if(msg == CRCInput::RC_sat) 
			{
				SetChannelMode(LIST_MODE_SAT, mode);
				nNewChannel = bouquetList->exec(true);
			}
			else if(msg == CRCInput::RC_favorites) 
			{
				SetChannelMode(LIST_MODE_FAV, mode);
				nNewChannel = bouquetList->exec(true);
			}
_repeat:
			dprintf(DEBUG_NORMAL, "CNeutrinoApp::handleMsg: ZAP RES: nNewChannel %d\n", nNewChannel);

			if(nNewChannel == -1) // on cancel
			{
				// restore orig. bouquet and selected channel on cancel
				SetChannelMode(old_mode, mode);
				bouquetList->activateBouquet(old_b, false);
				
				if(bouquetList->Bouquets.size())
					bouquetList->Bouquets[old_b]->channelList->setSelected(old_num - 1);
				
				StartSubtitles(mode == mode_tv);
			}
			else if(nNewChannel == -3) // list mode changed
			{ 
				dprintf(DEBUG_NORMAL, "CNeutrinoApp::handleMsg: bouquetList %x size %d\n", (size_t) bouquetList, bouquetList->Bouquets.size());
				
				nNewChannel = bouquetList->exec(true);
				goto _repeat;
			}
			else if(nNewChannel == -4) // list edited
			{
				if(old_b_id < 0) 
					old_b_id = old_b;

				g_Zapit->saveBouquets();
			}

			return messages_return::handled;
		}
		else if(mode == mode_iptv)
		{
			if(webtv)
			{
				webtv->show();
			}
			
			return messages_return::handled;
		}
	}
	else if (msg == CRCInput::RC_standby ) 
	{
		if (data == 0) 
		{
			neutrino_msg_t new_msg;

			/* 
			* Note: pressing the power button on the box (not the remote control) over 1 second
			* shuts down the system even if !g_settings.shutdown_real_rcdelay (see below)
			*/
			gettimeofday(&standby_pressed_at, NULL);

			if ((mode != mode_standby) && (g_settings.shutdown_real)) 
			{
				new_msg = NeutrinoMessages::SHUTDOWN;
			}
			else 
			{
				new_msg = (mode == mode_standby) ? NeutrinoMessages::STANDBY_OFF : NeutrinoMessages::STANDBY_ON;
				
				if ((g_settings.shutdown_real_rcdelay)) 
				{
					neutrino_msg_t      lmsg;
					neutrino_msg_data_t ldata;
					struct timeval      endtime;
					time_t              seconds;

					int timeout = 0;
					int timeout1 = 0;

					sscanf(g_settings.repeat_blocker, "%d", &timeout);
					sscanf(g_settings.repeat_genericblocker, "%d", &timeout1);

					if (timeout1 > timeout)
						timeout = timeout1;

					timeout += 500;

					while(true) 
					{
						g_RCInput->getMsg_ms(&lmsg, &ldata, timeout);
						
						if (lmsg == CRCInput::RC_timeout)
							break;

						gettimeofday(&endtime, NULL);
						seconds = endtime.tv_sec - standby_pressed_at.tv_sec;
						if (endtime.tv_usec < standby_pressed_at.tv_usec)
							seconds--;
						
						if (seconds >= 1) 
						{
							if ( lmsg == CRCInput::RC_standby )
								new_msg = NeutrinoMessages::SHUTDOWN;
							break;
						}
					}
				}
			}
			
			g_RCInput->postMsg(new_msg, 0);
			return messages_return::cancel_all | messages_return::handled;
		}
		else if (standby_pressed_at.tv_sec != 0)
		{
			/* check if we received a KEY_POWER pressed event before */
		        /* required for correct handling of KEY_POWER events of  */
			/* the power button on the dbox (not the remote control) */
			
			struct timeval endtime;
			gettimeofday(&endtime, NULL);
			time_t seconds = endtime.tv_sec - standby_pressed_at.tv_sec;
			if (endtime.tv_usec < standby_pressed_at.tv_usec)
				seconds--;
			if (seconds >= 1) 
			{
				g_RCInput->postMsg(NeutrinoMessages::SHUTDOWN, 0);
				return messages_return::cancel_all | messages_return::handled;
			}
		}
	}
	else if ( (msg == CRCInput::RC_plus) || (msg == CRCInput::RC_minus) )
	{
		setVolume(msg, (mode != mode_scart));
		return messages_return::handled;
	}
	else if( msg == CRCInput::RC_spkr ) 
	{
		if( mode == mode_standby ) 
		{
			//switch lcd off/on
			CVFD::getInstance()->togglePower();
		}
		else 
		{
			//mute
			AudioMute( !current_muted, true);
		}
		return messages_return::handled;
	}	
	// event messages
	else if (msg == NeutrinoMessages::EVT_VOLCHANGED) 
	{
		//setVolume(msg, false, true);
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::EVT_MUTECHANGED ) 
	{
		delete[] (unsigned char*) data;

		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::EVT_SERVICESCHANGED ) 
	{
		channelsInit();

		channelList->adjustToChannelID(live_channel_id);//FIXME
		
		if(old_b_id >= 0) 
		{
			bouquetList->activateBouquet(old_b_id, false);
			old_b_id = -1;
			g_RCInput->postMsg(CRCInput::RC_ok, 0);
		}
	}
	else if( msg == NeutrinoMessages::EVT_BOUQUETSCHANGED ) 
	{
		//CHintBox * reloadhintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_SERVICEMENU_RELOAD_HINT));

		//reloadhintBox->paint();

		channelsInit();
		channelList->adjustToChannelID(live_channel_id);//FIXME what if deleted ?
		
		//reloadhintBox->hide();
		//delete reloadhintBox;
		//reloadhintBox = NULL;

		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::EVT_RECORDMODE ) 
	{
		// sent by rcinput, then got msg from zapit about record activated/deactivated
		dprintf(DEBUG_NORMAL, "CNeutrinoApp::handleMsg: recordmode %s\n", ( data ) ? "on":"off" );
		
		if(!recordingstatus && (!data)) 
		{
			if( mode == mode_standby )
			{
				// set standby
				g_Zapit->setStandby(true);				
			}
		}
		
//#if defined (USE_OPENGL)
//		if(!playbackstatus)
//			playbackstatus = data;
//#else
		recordingstatus = data;
//#endif
		
		if( ( !g_InfoViewer->is_visible ) && data && !autoshift)
			g_RCInput->postMsg( NeutrinoMessages::SHOW_INFOBAR, 0 );

		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::RECORD_START) 
	{
		if(autoshift) 
		{
			stopAutoRecord();
			recordingstatus = 0;
			timeshiftstatus = 0;
		}

		puts("CNeutrinoApp::handleMsg: executing " NEUTRINO_RECORDING_START_SCRIPT ".");
		if (system(NEUTRINO_RECORDING_START_SCRIPT) != 0)
			perror(NEUTRINO_RECORDING_START_SCRIPT " failed");
		/* set nextRecordingInfo to current event (replace other scheduled recording if available) */
		/* Note: CTimerd::RecordingInfo is a class!
		 * => typecast to avoid destructor call */

		if (nextRecordingInfo != NULL)
			delete[] (unsigned char *) nextRecordingInfo;

		nextRecordingInfo = (CTimerd::RecordingInfo *) data;
		startNextRecording();

		return messages_return::handled | messages_return::cancel_all;
	}
	else if( msg == NeutrinoMessages::RECORD_STOP) 
	{
		if(((CTimerd::RecordingStopInfo*)data)->eventID == recording_id)
		{ 
			if (CVCRControl::getInstance()->isDeviceRegistered()) 
			{
				if ((CVCRControl::getInstance()->getDeviceState() == CVCRControl::CMD_VCR_RECORD) || (CVCRControl::getInstance()->getDeviceState() == CVCRControl::CMD_VCR_PAUSE ))
				{
					CVCRControl::getInstance()->Stop();
					
					recordingstatus = 0;
					autoshift = 0;
					
					if(timeshiftstatus)
					{
						// set timeshift status to false
						timeshiftstatus = 0;
					}
				}
				else
					printf("CNeutrinoApp::handleMsg: falscher state\n");
			}
			else
				puts("CNeutrinoApp::handleMsg: no recording devices");

			startNextRecording();

			if ( recordingstatus == 0 ) 
			{
				puts("CNeutrinoApp::handleMsg: executing " NEUTRINO_RECORDING_ENDED_SCRIPT ".");
				if (system(NEUTRINO_RECORDING_ENDED_SCRIPT) != 0)
					perror(NEUTRINO_RECORDING_ENDED_SCRIPT " failed");

				CVFD::getInstance()->ShowIcon(VFD_ICON_TIMESHIFT, false);
			}
		}
		else if(nextRecordingInfo != NULL) 
		{
			if(((CTimerd::RecordingStopInfo*)data)->eventID == nextRecordingInfo->eventID) 
			{
				/* Note: CTimerd::RecordingInfo is a class!
				 * => typecast to avoid destructor call */
				delete[] (unsigned char *) nextRecordingInfo;
				nextRecordingInfo=NULL;
			}
		}
		
		delete[] (unsigned char*) data;
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::EVT_PMT_CHANGED) 
	{
		res = messages_return::handled;
		//t_channel_id channel_id = *(t_channel_id*) data;
		//CRecordManager::getInstance()->Update(channel_id); //FIXME

		return res;
	}
	else if( msg == NeutrinoMessages::ZAPTO ) 
	{
		CTimerd::EventInfo * eventinfo;
		eventinfo = (CTimerd::EventInfo *) data;
		
		if(recordingstatus == 0) 
		{
			bool isTVMode = g_Zapit->isChannelTVChannel(eventinfo->channel_id);

			dvbsub_stop();

			if ((!isTVMode) && (mode != mode_radio)) 
			{
				radioMode(false);
			}
			else if (isTVMode && (mode != mode_tv)) 
			{
				tvMode(false);
			}
			
			channelList->zapTo_ChannelID(eventinfo->channel_id);
		}
		
		delete[] (unsigned char*) data;
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::ANNOUNCE_ZAPTO) 
	{
		if( mode == mode_standby ) 
		{
			standbyMode( false );
		}
		
		if( mode != mode_scart ) 
		{
			std::string name = g_Locale->getText(LOCALE_ZAPTOTIMER_ANNOUNCE);

			CTimerd::TimerList tmpTimerList;
			CTimerdClient tmpTimerdClient;

			tmpTimerList.clear();
			tmpTimerdClient.getTimerList( tmpTimerList );

			if(tmpTimerList.size() > 0) 
			{
				sort( tmpTimerList.begin(), tmpTimerList.end() );

				CTimerd::responseGetTimer &timer = tmpTimerList[0];

				CZapitClient Zapit;
				name += "\n";

				std::string zAddData = Zapit.getChannelName( timer.channel_id ); // UTF-8
				if( zAddData.empty()) 
				{
					zAddData = g_Locale->getText(LOCALE_TIMERLIST_PROGRAM_UNKNOWN);
				}

				if(timer.epgID!=0) 
				{
					CEPGData epgdata;
					zAddData += " :\n";
					//if (g_Sectionsd->getEPGid(timer.epgID, timer.epg_starttime, &epgdata))
					if (sectionsd_getEPGid(timer.epgID, timer.epg_starttime, &epgdata)) 
					{
						zAddData += epgdata.title;
					}
					else if(strlen(timer.epgTitle)!=0) 
					{
						zAddData += timer.epgTitle;
					}
				}
				else if(strlen(timer.epgTitle)!=0) 
				{
					zAddData += timer.epgTitle;
				}

				name += zAddData;
			}
			HintBox( LOCALE_MESSAGEBOX_INFO, name.c_str() );
		}

		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::ANNOUNCE_RECORD) 
	{
		system(NEUTRINO_RECORDING_TIMER_SCRIPT);

		char * lrecDir = ((CTimerd::RecordingInfo*)data)->recordingDir;

		// ether-wake
		for(int i = 0 ; i < NETWORK_NFS_NR_OF_ENTRIES ; i++) 
		{
			if (strcmp(g_settings.network_nfs_local_dir[i], lrecDir) == 0) 
			{
				dprintf(DEBUG_NORMAL, "CNeutrinoApp::handleMsg: waking up %s (%s)\n",g_settings.network_nfs_ip[i].c_str(),lrecDir);
					
				std::string command = "etherwake ";
				command += g_settings.network_nfs_mac[i];

				if(system(command.c_str()) != 0)
					perror("etherwake failed");
				break;
			}
		}

		//stop autoshift
		if(autoshift) 
		{
			stopAutoRecord();
			recordingstatus = 0;
			timeshiftstatus = 0;
		}

		delete[] (unsigned char*) data;
		
		if( mode != mode_scart )
			HintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_RECORDTIMER_ANNOUNCE));
		
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::ANNOUNCE_SLEEPTIMER) 
	{
		if( mode != mode_scart )
			HintBox( LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_SLEEPTIMERBOX_ANNOUNCE) );
		
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::SLEEPTIMER) 
	{
		if(g_settings.shutdown_real)
			ExitRun(SHUTDOWN);
		else
			standbyMode( true );
		
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::STANDBY_TOGGLE ) 
	{
		standbyMode( !(mode & mode_standby) );
		g_RCInput->clearRCMsg();
		
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::STANDBY_ON ) 
	{
		if( mode != mode_standby ) 
		{
			standbyMode( true );
		}
		g_RCInput->clearRCMsg();
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::STANDBY_OFF ) 
	{
		if( mode == mode_standby ) 
		{
			standbyMode( false );
		}
		g_RCInput->clearRCMsg();
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::ANNOUNCE_SHUTDOWN) 
	{
		if( mode != mode_scart )
			skipShutdownTimer = (MessageBox(LOCALE_MESSAGEBOX_INFO, LOCALE_SHUTDOWNTIMER_ANNOUNCE, CMessageBox::mbrNo, CMessageBox::mbYes | CMessageBox::mbNo, NULL, 450, 5) == CMessageBox::mbrYes);
	}
	else if( msg == NeutrinoMessages::SHUTDOWN ) 
	{
		if(!skipShutdownTimer) 
		{
			ExitRun(SHUTDOWN);
		}
		else 
		{
			skipShutdownTimer = false;
		}
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::REBOOT ) 
	{
		if(!skipShutdownTimer) 
		{
			ExitRun(REBOOT);
		}
		else 
		{
			skipShutdownTimer = false;
		}
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::RESTART ) 
	{
		if(!skipShutdownTimer) 
		{
			ExitRun(RESTART);
		}
		else 
		{
			skipShutdownTimer = false;
		}
		return messages_return::handled;
	}
	else if ( msg == NeutrinoMessages::EVT_POPUP ) 
	{
		if (mode != mode_scart)
			HintBox(LOCALE_MESSAGEBOX_INFO, (const char *) data); // UTF-8
		
		delete (unsigned char*) data;
		
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_EXTMSG) 
	{
		if (mode != mode_scart)
			MessageBox(LOCALE_MESSAGEBOX_INFO, (const char *) data, CMessageBox::mbrBack, CMessageBox::mbBack, NEUTRINO_ICON_INFO); // UTF-8
		delete[] (unsigned char*) data;
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_RECORDING_ENDED) 
	{
		if (mode != mode_scart) 
		{
			neutrino_locale_t msgbody;
			if ((* (stream2file_status2_t *) data).status == STREAM2FILE_STATUS_BUFFER_OVERFLOW)
				msgbody = LOCALE_STREAMING_BUFFER_OVERFLOW;
			else if ((* (stream2file_status2_t *) data).status == STREAM2FILE_STATUS_WRITE_OPEN_FAILURE)
				msgbody = LOCALE_STREAMING_WRITE_ERROR_OPEN;
			else if ((* (stream2file_status2_t *) data).status == STREAM2FILE_STATUS_WRITE_FAILURE)
				msgbody = LOCALE_STREAMING_WRITE_ERROR;
			else
				goto skip_message;

			/* use a short timeout of only 5 seconds in case it was only a temporary network problem
			 in case of STREAM2FILE_STATUS_IDLE we might even have to immediately start the next recording 
			*/
			MessageBox(LOCALE_MESSAGEBOX_INFO, msgbody, CMessageBox::mbrBack, CMessageBox::mbBack, NEUTRINO_ICON_INFO, 450, 5);

skip_message:
			;
		}
		
		if ((* (stream2file_status2_t *) data).status != STREAM2FILE_STATUS_IDLE) 
		{
			/*
			 * note that changeNotify does not distinguish between LOCALE_MAINMENU_RECORDING_START and LOCALE_MAINMENU_RECORDING_STOP
			 * instead it checks the state of the variable recordingstatus
			 */
			/* restart recording */
			//FIXME doGuiRecord((*(stream2file_status2_t *) data).dir);
			//changeNotify(LOCALE_MAINMENU_RECORDING_START, data);
		}

		delete[] (unsigned char*) data;
		
		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::REMIND) 
	{
		std::string text = (char*)data;
		std::string::size_type pos;
		while((pos=text.find('/'))!= std::string::npos)
		{
			text[pos] = '\n';
		}
		
		if( mode != mode_scart )
			MessageBox(LOCALE_TIMERLIST_TYPE_REMIND, text, CMessageBox::mbrBack, CMessageBox::mbBack, NEUTRINO_ICON_INFO); // UTF-8
			
		delete[] (unsigned char*) data;
		
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::LOCK_RC) 
	{
		this->rcLock->exec(NULL, CRCLock::NO_USER_INPUT);

		return messages_return::handled;
	}
	else if( msg == NeutrinoMessages::CHANGEMODE ) 
	{
		if((data & mode_mask) != mode_radio)
		{		  
			if (g_Radiotext)
			{
				delete g_Radiotext;
				g_Radiotext = NULL;
			}
		}

		if((data & mode_mask) == mode_radio) 
		{
			if( mode != mode_radio ) 
			{
				if((data & norezap) == norezap)
					radioMode(false);
				else
					radioMode(true);
	
				//FIXME: this sucks when no DVB device is present
				if (g_settings.radiotext_enable && g_Radiotext)
					g_Radiotext->setPid(g_RemoteControl->current_PIDs.APIDs[g_RemoteControl->current_PIDs.PIDs.selected_apid].pid);				
			}
		}
		
		if((data & mode_mask) == mode_tv) 
		{
			if( mode != mode_tv ) 
			{
				if((data & norezap) == norezap)
					tvMode(false);
				else
					tvMode(true);
			}
		}
		
		if((data &mode_mask) == mode_standby) 
		{
			if(mode != mode_standby)
				standbyMode( true );
		}
		
		if((data &mode_mask) == mode_audio) 
		{
			lastMode = mode;
			mode = mode_audio;
		}
		
		if((data &mode_mask) == mode_pic) 
		{
			lastMode = mode;
			mode = mode_pic;
		}
		
		if((data &mode_mask) == mode_ts) 
		{
			lastMode = mode;
			mode = mode_ts;
		}
		
		if((data &mode_mask) == mode_iptv) 
		{
			lastMode = mode;
			mode = mode_iptv;
		}
	}	
	else if( msg == NeutrinoMessages::VCR_ON ) 
	{
		if( mode != mode_scart ) 
		{
			scartMode( true );
		}
		else
		{
			CVFD::getInstance()->setMode(CVFD::MODE_SCART);
		}
	}	
	else if( msg == NeutrinoMessages::VCR_OFF ) 
	{
		if( mode == mode_scart ) 
		{
			scartMode( false );
		}
	}	
	else if (msg == NeutrinoMessages::EVT_START_PLUGIN) 
	{
		dprintf(DEBUG_NORMAL, "CNeutrinoApp::handleMsg: start Plugin: %s\n", (char *)data);
		
		g_PluginList->startPlugin((const char *)data);
		
		delete[] (unsigned char*) data;
		return messages_return::handled;
	}
	else if (msg == NeutrinoMessages::EVT_SERVICES_UPD) 
	{
		// dont show this msg
		//MessageBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_ZAPIT_SDTCHANGED), CMessageBox::mbrBack,CMessageBox::mbBack, NEUTRINO_ICON_INFO);
	}
	
	if ((msg >= CRCInput::RC_WithData) && (msg < CRCInput::RC_WithData + 0x10000000))
		delete [] (unsigned char*) data;

	return messages_return::unhandled;
}

// exit run
void CNeutrinoApp::ExitRun(int retcode)
{ 
	// break silently autotimeshift
	if(autoshift) 
	{
		stopAutoRecord();
		recordingstatus = 0;
		timeshiftstatus = 0;
	}
	
	//
	if (!recordingstatus || MessageBox(LOCALE_MESSAGEBOX_INFO, LOCALE_SHUTDOWN_RECODING_QUERY, CMessageBox::mbrNo, CMessageBox::mbYes | CMessageBox::mbNo, NULL, 450, 30, true) == CMessageBox::mbrYes)  
	{
		// stop recording
		if(recordingstatus) 
		{
			CVCRControl::getInstance()->Stop();
			g_Timerd->stopTimerEvent(recording_id);
		}

		if(retcode > RESTART)
		{
			// vfd mode shutdown
			CVFD::getInstance()->setMode(CVFD::MODE_SHUTDOWN);
		
			// show good bye in VFD
			if (!CVFD::getInstance()->is4digits)
				CVFD::getInstance()->ShowText((char *) "BYE");
		}

		// stop playback
		g_Zapit->stopPlayBack();

		if(retcode > RESTART)
		{
			frameBuffer->loadBackgroundPic("shutdown.jpg");
		
			frameBuffer->blit();
		}

		// network down
		CNetworkSettings::getInstance()->networkConfig->automatic_start = (CNetworkSettings::getInstance()->network_automatic_start == 1);

		CNetworkSettings::getInstance()->networkConfig->commitConfig();
		
		// save neutrino.conf
		saveSetup(NEUTRINO_SETTINGS_FILE);

		// save epg
		if(g_settings.epg_save ) 
			saveEpg();

		// stop dvbsub
		dvbsub_close();

		// stop txt
		tuxtxt_stop();
		tuxtxt_close();

		tuxtx_stop_subtitle();
		
		dprintf(DEBUG_NORMAL, "CNeutrinoApp::ExitRun: entering off state (retcode:%d)\n", retcode);
			
		stop_daemons();

#if defined (PLATFORM_COOLSTREAM)
		CVFD::getInstance()->Clear();
		cs_deregister_messenger();
		cs_api_exit();
#endif
			
		//
		if(playback)
			delete playback;
		
		if (webtv)
			delete webtv;
			
		if (g_RCInput != NULL)
			delete g_RCInput;
			
		if(g_Sectionsd)
			delete g_Sectionsd;
			
		if(g_Timerd)
			delete g_Timerd;
			
		if(g_RemoteControl)
			delete g_RemoteControl;
			
		if(g_fontRenderer)
			delete g_fontRenderer;
			
		if(g_Zapit)
			delete g_Zapit;
			
		delete CVFD::getInstance();
			
		if (frameBuffer != NULL)
			delete frameBuffer;

		dprintf(DEBUG_NORMAL, ">>> CNeutrinoApp::ExitRun: Good bye <<<\n");
		
#if defined (USE_OPENGL)		
		if(retcode == RESTART)
		{		  
			execvp(global_argv[0], global_argv); // no return if successful
		}
#endif		
		
		_exit(retcode);	
	}
}

void CNeutrinoApp::readEPG()
{
	// read saved epg
	struct stat my_stat;
	
	if(stat(g_settings.epg_dir.c_str(), &my_stat) == 0)
	{
		dprintf(DEBUG_NORMAL, "CNeutrinoApp::readEpg: read EPG from: %s....\n", g_settings.epg_dir.c_str());

		g_Sectionsd->readSIfromXML(g_settings.epg_dir.c_str());
	}
}

// save epg
void CNeutrinoApp::saveEpg()
{
	struct stat my_stat;
   
	if(stat(g_settings.epg_dir.c_str(), &my_stat) == 0)
	{
		dprintf(DEBUG_NORMAL, "CNeutrinoApp::saveEpg: Saving EPG to %s....\n", g_settings.epg_dir.c_str());
		
		neutrino_msg_t      msg;
		neutrino_msg_data_t data;
		
		g_Sectionsd->writeSI2XML(g_settings.epg_dir.c_str());

		while( true ) 
		{
			g_RCInput->getMsg(&msg, &data, 300); // 30 secs..
			if (( msg == CRCInput::RC_timeout ) || (msg == NeutrinoMessages::EVT_SI_FINISHED)) 
			{
				//printf("Msg %x timeout %d EVT_SI_FINISHED %x\n", msg, CRCInput::RC_timeout, NeutrinoMessages::EVT_SI_FINISHED);
				break;
			}
		}
	}
}

// mute
void CNeutrinoApp::AudioMute( int newValue, bool isEvent )
{
	int dx = 32;
	int dy = 32;
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_MUTE, &dx, &dy);
	int offset = (dx/4);
	dx += offset;
	dy += offset;

	int x = g_settings.screen_EndX - 10 - dx;
	int y = g_settings.screen_StartY + 10;

#if ENABLE_LCD
	CVFD::getInstance()->setMuted(newValue);
#endif

	current_muted = newValue;

	dprintf(DEBUG_NORMAL, "CNeutrinoApp::AudioMute: current_muted %d new %d isEvent: %d\n", current_muted, newValue, isEvent);
	
	g_Zapit->muteAudio(current_muted);

	if( isEvent && ( mode != mode_scart ) && ( mode != mode_audio) && ( mode != mode_pic))
	{
		//FIXME:
		fb_pixel_t * mute_pixbuf = NULL;

		if( current_muted ) 
		{
			if(!mute_pixbuf)
				mute_pixbuf = new fb_pixel_t[dx * dy];
			
			if(mute_pixbuf)
			{
				frameBuffer->SaveScreen(x, y, dx, dy, mute_pixbuf);
			
				frameBuffer->blit();						
			}
		
			frameBuffer->paintBoxRel(x, y, dx, dy, COL_MENUCONTENT_PLUS_0);
			frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_MUTE, x + offset/2, y + offset/2 );
		}
		else
		{
			if( mute_pixbuf) 
			{
				frameBuffer->RestoreScreen(x, y, dx, dy, mute_pixbuf);

				frameBuffer->blit();
	
				delete [] mute_pixbuf;
			}
			else
				frameBuffer->paintBackgroundBoxRel(x, y, dx, dy);
		}
		
		frameBuffer->blit();	
	}
}

// set vol
void CNeutrinoApp::setvol(int vol)
{
	if(audioDecoder)
		audioDecoder->setVolume(vol, vol);
}

// set volume
void CNeutrinoApp::setVolume(const neutrino_msg_t key, const bool bDoPaint, bool nowait)
{
	neutrino_msg_t msg = key;

	int dx = 296;	//256 (16*16) for vulme bar + 40 for volume digits
	int dy = 32;

	int x = frameBuffer->getScreenX() + 10;
	int y = frameBuffer->getScreenY() + 10;

	current_volume = g_settings.current_volume;
	
	int sw = frameBuffer->getScreenWidth();
	int sh = frameBuffer->getScreenHeight();
	
	int a_step = atoi(g_settings.audio_step);
	
	switch( g_settings.volume_pos )
	{
		case 0:// upper right
			x = sw - dx - 6;
			break;
			
		case 1:// upper left
			break;
			
		case 2:// bottom left
			y = sh - dy;
			break;
			
		case 3:// bottom right
			x = sw - dx;
			y = sh - dy;
			break;
			
		case 4:// center default
			x = ((sw - dx) / 2) + x;
			break;
			
		case 5:// center higher
			x = ((sw - dx) / 2) + x;
			y = sh - sh/15;
			break;
	}

	fb_pixel_t * pixbuf = NULL;

	if(bDoPaint) 
	{
		pixbuf = new fb_pixel_t[dx * dy];

		if(pixbuf != NULL)
		{
			frameBuffer->SaveScreen(x, y, dx, dy, pixbuf);
			
			frameBuffer->blit();				
		}

		// background box
		frameBuffer->paintBoxRel(x , y , dx, dy, COL_MENUCONTENT_PLUS_0, RADIUS_MID, CORNER_BOTH);
		
		// vol box aussen
		frameBuffer->paintBoxRel(x + dy + dy/4 - 2, y + dy/4 - 2, dy*25/4 + 4, dy/2 + 4, COL_MENUCONTENT_PLUS_3);
		
		// vol box innen
		frameBuffer->paintBoxRel(x + dy + dy/4, y + dy/4, dy*25/4, dy/2,   COL_MENUCONTENT_PLUS_1);
		
		//icon
		frameBuffer->paintIcon(NEUTRINO_ICON_VOLUME, x+dy/2,y + (dy/4), 0, COL_MENUCONTENT_PLUS_0);

		g_volscale->reset();
		g_volscale->paint(x + dy+ (dy/4), y +(dy/4), current_volume);
		

		char p1[4]; // 3 digits + '\0'
		sprintf(p1, "%3d", current_volume);

		// erase the numbers
		frameBuffer->paintBoxRel(x + dx - 50, y , 40, dy, COL_MENUCONTENT_PLUS_0);

		g_Font[SNeutrinoSettings::FONT_TYPE_EPG_DATE]->RenderString(x + dx - 45, y + dy/2 + 14, 36, p1, COL_MENUCONTENT );

		frameBuffer->blit();
	}

	neutrino_msg_data_t data;

	unsigned long long timeoutEnd;

	do {
		if (msg <= CRCInput::RC_MaxRC) 
		{
			if ( msg == CRCInput::RC_plus ) 
			{ 
				if (g_settings.current_volume < 100 - a_step )
					g_settings.current_volume += a_step;
				else
					g_settings.current_volume = 100;
			}
			else if ( msg == CRCInput::RC_minus ) 
			{ 
				if (g_settings.current_volume > a_step)
					g_settings.current_volume -= a_step;
				else
					g_settings.current_volume = 0;
			}
			else 
			{
				g_RCInput->postMsg(msg, data);
				break;
			}

			setvol(g_settings.current_volume);	
			
			//FIXME
			if (current_muted && msg == CRCInput::RC_plus)
				AudioMute(0, true);

			timeoutEnd = CRCInput::calcTimeoutEnd(nowait ? 1 : 3);
		}
		else if (msg == NeutrinoMessages::EVT_VOLCHANGED) 
		{
			timeoutEnd = CRCInput::calcTimeoutEnd(3);
		}
		else if (handleMsg(msg, data) & messages_return::unhandled) 
		{
			g_RCInput->postMsg(msg, data);
			break;
		}

		if (bDoPaint) 
		{
			if(current_volume != g_settings.current_volume) 
			{
				current_volume = g_settings.current_volume;
				
				dprintf(DEBUG_NORMAL, "CNeutrinoApp::setVolume: current_volume %d\n", current_volume);

				g_volscale->paint(x + dy+ (dy/4), y +(dy/4), current_volume);

				char p[4]; // 3 digits + '\0'
				sprintf(p, "%3d", current_volume);

				// erase the numbers
				frameBuffer->paintBoxRel(x + dx - 50, y , 40, dy, COL_MENUCONTENT_PLUS_0);

				g_Font[SNeutrinoSettings::FONT_TYPE_EPG_DATE]->RenderString(x + dx - 45, y + dy/2 + 14, 36, p, COL_MENUCONTENT);
				
				frameBuffer->blit();

				//FIXME
				if (mode != mode_scart && mode != mode_pic && (g_settings.current_volume == 0) )
					AudioMute(true, true);
			}
		}

#if ENABLE_LCD
		CVFD::getInstance()->showVolume(g_settings.current_volume);
#endif

		if (msg != CRCInput::RC_timeout) 
		{
			g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd );
		}

		frameBuffer->blit();	
	} while (msg != CRCInput::RC_timeout);

	if( (bDoPaint) && (pixbuf != NULL) ) 
	{
		frameBuffer->RestoreScreen(x, y, dx, dy, pixbuf);

		frameBuffer->blit();
	
		delete [] pixbuf;
	}
}

// tv mode
void CNeutrinoApp::tvMode( bool rezap )
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::tvMode: rezap %s\n", rezap ? "yes" : "no");
	
	if(mode == mode_radio ) 
	{	  
		if (g_settings.radiotext_enable && g_Radiotext) 
		{
			videoDecoder->finishShowSinglePic();
			
			delete g_Radiotext;
			g_Radiotext = NULL;
		}		

#if defined (ENABLE_LCD)
		g_RCInput->killTimer(g_InfoViewer->lcdUpdateTimer);
		g_InfoViewer->lcdUpdateTimer = g_RCInput->addTimer( LCD_UPDATE_TIME_TV_MODE, false );
#endif		

		CVFD::getInstance()->ShowIcon(VFD_ICON_RADIO, false);

		StartSubtitles(!rezap);
	}

	CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);

        if( mode == mode_tv ) 
	{
                return;
	}
	else if( mode == mode_scart )
	{
#if !defined (PLATFORM_COOLSTREAM)	  
		if(videoDecoder)
			videoDecoder->SetInput(INPUT_ENCODER);
#endif		
	}
	else if( mode == mode_standby ) 
	{
		CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
		
		// set video standby
#if !defined (PLATFORM_COOLSTREAM)		
		if(videoDecoder)
			videoDecoder->SetInput(STANDBY_OFF);
#endif		
	}
	else if(mode == mode_iptv)
	{
		if(webtv)
		{
			webtv->stopPlayBack();
			webtv->ClearChannels();
		}

		// start epg scanning
		g_Sectionsd->setPauseScanning(false);
			
		// start playback
		g_Zapit->startPlayBack();
	}

	bool stopauto = (mode != mode_ts);	
	mode = mode_tv;
	
	if(stopauto && autoshift) 
	{
		//printf("standby on: autoshift ! stopping ...\n");
		
		stopAutoRecord();
		recordingstatus = 0;
		timeshiftstatus = 0;
	}	

	frameBuffer->useBackground(false);
	frameBuffer->paintBackground();
	frameBuffer->blit();

	g_RemoteControl->tvMode();
	
	SetChannelMode( g_settings.channel_mode, mode);//FIXME needed ?

	if( rezap ) 
	{
		firstChannel();
		channelList->tuned = 0xfffffff;;
		channelList->zapTo( firstchannel.channelNumber - 1 );
	}
}

// Radio Mode
void CNeutrinoApp::radioMode( bool rezap)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::radioMode: rezap %s\n", rezap ? "yes" : "no");

	if(mode == mode_tv ) 
	{
#if defined (ENABLE_LCD)	  
		g_RCInput->killTimer(g_InfoViewer->lcdUpdateTimer);
		g_InfoViewer->lcdUpdateTimer = g_RCInput->addTimer( LCD_UPDATE_TIME_RADIO_MODE, false );
#endif		

		StopSubtitles();
	}

	CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);

	if( mode == mode_radio ) 
	{
		return;
	}
	else if( mode == mode_scart ) 
	{
#if !defined (PLATFORM_COOLSTREAM)	  
		if(videoDecoder)
			videoDecoder->SetInput(INPUT_ENCODER);
#endif		
	}
	else if( mode == mode_standby ) 
	{	  
		CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);

		// set video standby
#if !defined (PLATFORM_COOLSTREAM)
		if(videoDecoder)
			videoDecoder->SetInput(STANDBY_OFF);
#endif		
	}
	if(mode == mode_iptv)
	{
		if(webtv)
		{
			webtv->stopPlayBack();
			webtv->ClearChannels();
		}

		// start epg scanning
		g_Sectionsd->setPauseScanning(false);
			
		// start playback
		g_Zapit->startPlayBack();
	}

	mode = mode_radio;

	if(autoshift) 
	{
		dprintf(DEBUG_NORMAL, "CNeutrinoApp::radioMode: standby on: autoshift ! stopping ...\n");
		stopAutoRecord();
		recordingstatus = 0;
		timeshiftstatus = 0;
	}

	g_RemoteControl->radioMode();
	SetChannelMode( g_settings.channel_mode, mode);//FIXME needed?

	if( rezap ) 
	{
		firstChannel();
		channelList->tuned = 0xfffffff;;
		channelList->zapTo( firstchannel.channelNumber -1 );
	}

	if (!g_settings.radiotext_enable)
	{
		frameBuffer->loadBackgroundPic("radiomode.jpg");
		frameBuffer->blit();
	}

	if (g_settings.radiotext_enable) 
	{
		if(g_Radiotext == NULL)
			g_Radiotext = new CRadioText;
	}	
}

// webtv mode
void CNeutrinoApp::webtvMode( bool rezap)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::webtvMode: rezap %s\n", rezap ? "yes" : "no");

	if(mode == mode_tv ) 
	{
#if defined (ENABLE_LCD)	  
		g_RCInput->killTimer(g_InfoViewer->lcdUpdateTimer);
		g_InfoViewer->lcdUpdateTimer = g_RCInput->addTimer( LCD_UPDATE_TIME_RADIO_MODE, false );
#endif		

		StopSubtitles();
	}
	else if( mode == mode_radio ) 
	{  
		if (g_settings.radiotext_enable && g_Radiotext) 
		{
			videoDecoder->finishShowSinglePic();
			
			delete g_Radiotext;
			g_Radiotext = NULL;
		}	

#if defined (ENABLE_LCD)
		g_RCInput->killTimer(g_InfoViewer->lcdUpdateTimer);
		g_InfoViewer->lcdUpdateTimer = g_RCInput->addTimer( LCD_UPDATE_TIME_TV_MODE, false );
#endif		

		CVFD::getInstance()->ShowIcon(VFD_ICON_RADIO, false);

		StartSubtitles(!rezap);
	}
	else if( mode == mode_scart ) 
	{
#if !defined (PLATFORM_COOLSTREAM)	  
		if(videoDecoder)
			videoDecoder->SetInput(INPUT_ENCODER);
#endif		
	}
	else if( mode == mode_standby ) 
	{	  
		CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);

		// set video standby
#if !defined (PLATFORM_COOLSTREAM)
		if(videoDecoder)
			videoDecoder->SetInput(STANDBY_OFF);
#endif		
	}
	else if(mode == mode_iptv)
	{
		return;
	}

	if(autoshift) 
	{
		dprintf(DEBUG_NORMAL, "CNeutrinoApp::radioMode: standby on: autoshift ! stopping ...\n");
		stopAutoRecord();
		recordingstatus = 0;
		timeshiftstatus = 0;
	}
	
	frameBuffer->useBackground(false);
	frameBuffer->paintBackground();
	frameBuffer->blit();

	// pause epg scanning
	g_Sectionsd->setPauseScanning(true);
			
	// stop playback
	g_Zapit->stopPlayBack();
	
	mode = mode_iptv;

	// show streams channel list
	if(webtv)
	{
		webtv->show(true);
	}
}

// scart Mode
void CNeutrinoApp::scartMode( bool bOnOff )
{
	printf( ( bOnOff ) ? "CNeutrinoApp::scartMode: scart on\n" : "CNeutrinoApp::scartMode: mode: scart off\n" );

	if( bOnOff ) 
	{
		frameBuffer->useBackground(false);
		frameBuffer->paintBackground();
		frameBuffer->blit();

		CVFD::getInstance()->setMode(CVFD::MODE_SCART);
		
		// FIXME:modes?
		if(mode == mode_iptv)
		{
			if(webtv)
				webtv->stopPlayBack();
		}

		lastMode = mode;
		mode = mode_scart;
		
#if !defined (PLATFORM_COOLSTREAM)	  
		if(videoDecoder)
		{
			videoDecoder->SetInput(INPUT_SCART);
			videoDecoder->SetStandby(STANDBY_ON);
		}
#endif		
	} 
	else 
	{
#if !defined (PLATFORM_COOLSTREAM)	  
		if(videoDecoder)
		{
			videoDecoder->SetInput(INPUT_ENCODER);
			videoDecoder->SetStandby(STANDBY_OFF);
		}
#endif
		mode = mode_unknown;
		
		//re-set mode
		if( lastMode == mode_radio ) 
		{
			radioMode( false );
		}
		else if( lastMode == mode_tv ) 
		{
			tvMode( false );
		}
		else if( lastMode == mode_standby ) 
		{
			standbyMode( true );
		}
		else if(lastMode == mode_iptv)
		{
			webtvMode(true);
		}
	}
}

// standby mode
void CNeutrinoApp::standbyMode( bool bOnOff )
{
	static bool wasshift = false;

	dprintf(DEBUG_NORMAL, "CNeutrinoApp::standbyMode: recordingstatus (%d) timeshiftstatus(%d) bOnOff (%d)\n", recordingstatus, timeshiftstatus, bOnOff);

	if( bOnOff ) 
	{
		if(autoshift) 
		{
			stopAutoRecord();
			wasshift = true;
			recordingstatus = 0;
			timeshiftstatus = 0;
		}
		
		if( mode == mode_scart ) 
		{
			// do not things
		}
		
		if (mode == mode_radio && g_settings.radiotext_enable && g_Radiotext != NULL)
		{
			delete g_Radiotext;
			g_Radiotext = NULL;
		}

		StopSubtitles();

		frameBuffer->useBackground(false);
		frameBuffer->paintBackground();
		frameBuffer->blit();
	
#if defined (PLATFORM_COOLSTREAM)
		CVFD::getInstance()->Clear();
#endif		
		
		// show time in vfd
		CVFD::getInstance()->setMode(CVFD::MODE_STANDBY);
		
#if !defined (PLATFORM_COOLSTREAM)	  
		if(videoDecoder)
			videoDecoder->SetInput(STANDBY_ON);
#endif		
		
		if(mode == mode_iptv)
		{
			if(webtv)
				webtv->stopPlayBack();
		}
		else
		{
			// zapit standby
			if(!recordingstatus && !timeshiftstatus)
			{
				g_Zapit->setStandby(true);
			} 
			else
			{
				//zapit stop playback
				g_Zapit->stopPlayBack();
			}

			// stop sectionsd
			g_Sectionsd->setServiceChanged(0, false);
			g_Sectionsd->setPauseScanning(true);

			//save epg
			if(!recordingstatus && !timeshiftstatus)
			{
				if(g_settings.epg_save) 
				{
					saveEpg();
				}
			}			
		}

		//run script
		puts("CNeutrinoApp::standbyMode: executing " NEUTRINO_ENTER_STANDBY_SCRIPT ".");
		
		if (system(NEUTRINO_ENTER_STANDBY_SCRIPT) != 0)
			perror(NEUTRINO_ENTER_STANDBY_SCRIPT " failed");

		lastMode = mode;
		mode = mode_standby;

		frameBuffer->setActive(false);
			
		// set fan off
#if !ENABLE_LCD		
		CVFD::getInstance()->setFan(false);
#endif		
	} 
	else 
	{	
		// set fan on
#if !ENABLE_LCD		
		CVFD::getInstance()->setFan(true);
#endif		

		// set fb active
		frameBuffer->setActive(true);

		puts("CNeutrinoApp::standbyMode: executing " NEUTRINO_LEAVE_STANDBY_SCRIPT ".");
		if (system(NEUTRINO_LEAVE_STANDBY_SCRIPT) != 0)
			perror(NEUTRINO_LEAVE_STANDBY_SCRIPT " failed");

		// vfd mode
		CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
		
#if !defined (PLATFORM_COOLSTREAM)	  
		if(videoDecoder)
			videoDecoder->SetInput(STANDBY_OFF);
#endif		
				
		if (CVFD::getInstance()->is4digits)
			CVFD::getInstance()->LCDshowText(channelList->getActiveChannelNumber());
		
		// setmode?radio:tv/iptv
		mode = mode_unknown;

		if(lastMode == mode_iptv)
		{
			if(webtv)
				webtv->startPlayBack(webtv->getTunedChannel());
			
			mode = mode_iptv;
		}
		else
		{
			// zapit startplayback
			g_Zapit->setStandby(false);

			// this is buggy don't respect parentallock
			if(!recordingstatus && !timeshiftstatus)
				g_Zapit->startPlayBack();


			g_Sectionsd->setPauseScanning(false);
			g_Sectionsd->setServiceChanged(live_channel_id&0xFFFFFFFFFFFFULL, true );
		}

		if( lastMode == mode_radio ) 
		{
			radioMode(false);
		} 
		else if(lastMode == mode_tv)
		{
			tvMode(false);
		}
		else if(lastMode == mode_iptv)
		{
			webtvMode(false);
		}

		// set vol (saved)
		AudioMute(current_muted, false );					

		// start record if
		if((mode == mode_tv) && wasshift) 
		{
			startAutoRecord(true);
		}

		wasshift = false;

		StartSubtitles();
		
		g_RCInput->postMsg( NeutrinoMessages::SHOW_INFOBAR, 0 );
	}
}

// exec, menuitem callback (shutdown)
int CNeutrinoApp::exec(CMenuTarget * parent, const std::string & actionKey)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::exec: actionKey: %s\n", actionKey.c_str());

	int returnval = menu_return::RETURN_REPAINT;

	if(actionKey == "shutdown") 
	{
		ExitRun(SHUTDOWN);
	}
	else if(actionKey == "reboot")
	{
		ExitRun(REBOOT);
	}
	else if(actionKey == "restart") 
	{		
		ExitRun(RESTART);
	}
	else if(actionKey == "standby")
	{
		standbyMode(true);
	}	
	else if(actionKey == "tv") 
	{
		tvMode();
		returnval = menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey == "radio") 
	{
		radioMode();
		returnval = menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey == "webtv") 
	{
		webtvMode();
		return menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey == "scart") 
	{
		g_RCInput->postMsg( NeutrinoMessages::VCR_ON, 0 );
		returnval = menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey == "savesettings") 
	{
		CHintBox * hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_MAINSETTINGS_SAVESETTINGSNOW_HINT)); // UTF-8
		hintBox->paint();
		
		saveSetup(NEUTRINO_SETTINGS_FILE);

		tuxtxt_close();
		
		zapitCfg.saveLastChannel = g_settings.uselastchannel;
		setZapitConfig(&zapitCfg);

		hintBox->hide();
		delete hintBox;
		hintBox = NULL;
	}
	else if(actionKey == "features")
	{
		if(parent)
			parent->hide();
		
		StopSubtitles();
		showUserMenu(SNeutrinoSettings::BUTTON_BLUE);
		StartSubtitles();
				
		return menu_return::RETURN_REPAINT;	
	}

	return returnval;
}

// changeNotify - features menu recording start / stop
bool CNeutrinoApp::changeNotify(const neutrino_locale_t OptionName, void */*data*/)
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::changeNotify\n");

	if ((ARE_LOCALES_EQUAL(OptionName, LOCALE_MAINMENU_RECORDING_START)) || (ARE_LOCALES_EQUAL(OptionName, LOCALE_MAINMENU_RECORDING)))
	{
		if(g_RemoteControl->is_video_started) 
		{
			bool res = doGuiRecord(NULL, ARE_LOCALES_EQUAL(OptionName, LOCALE_MAINMENU_RECORDING));
			return res;
		}
		else 
		{
			recordingstatus = 0;
			return false;
		}
	}

	return false;
}

void stop_daemons()
{
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::stop_daemons\n");

	// stop nhttpd	
	dprintf(DEBUG_NORMAL, "stop_daemons: httpd shutdown\n");
	pthread_cancel(nhttpd_thread);
	pthread_join(nhttpd_thread, NULL);
	dprintf(DEBUG_NORMAL, "stop_daemons: httpd shutdown done\n");	

	// stop streamts	
	streamts_stop = 1;
	pthread_join(stream_thread, NULL);	

	// stop timerd	  
	dprintf(DEBUG_NORMAL, "stop_daemons: timerd shutdown\n");
	g_Timerd->shutdown();
	pthread_join(timer_thread, NULL);
	dprintf(DEBUG_NORMAL, "stop_daemons: timerd shutdown done\n");		

	// stop sectionsd
	sectionsd_stop = 1;
	dprintf(DEBUG_NORMAL, "stop_daemons: sectionsd shutdown\n");
	pthread_join(sections_thread, NULL);
	dprintf(DEBUG_NORMAL, "stop_daemons: sectionsd shutdown done\n");

	// zapit stop	
	dprintf(DEBUG_NORMAL, "stop_daemons: zapit shutdown\n");
	g_Zapit->shutdown();
	pthread_join(zapit_thread, NULL);
	dprintf(DEBUG_NORMAL, "stop_daemons: zapit shutdown done\n");	
}

// stop subtitle
void CNeutrinoApp::StopSubtitles()
{
	if(mode == mode_iptv)
		return;
	
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::StopSubtitles\n");
	
	int ttx, ttxpid, ttxpage;

	// dvbsub
	int dvbpid = dvbsub_getpid();
	
	if(dvbpid)
	{
		dvbsub_pause();
	}
	
	// tuxtxt
	tuxtx_subtitle_running(&ttxpid, &ttxpage, &ttx);
	
	if(ttx) 
	{
		tuxtx_pause_subtitle(true, live_fe?live_fe->fenumber:0 );
		
		frameBuffer->paintBackground();

		frameBuffer->blit();
	}
}

// start subtitle
void CNeutrinoApp::StartSubtitles(bool show)
{
	if(mode == mode_iptv)
		return;
	
	dprintf(DEBUG_NORMAL, "CNeutrinoApp::StartSubtitles\n");
	
	if(!show)
		return;
	
	//dvbsub
	dvbsub_start(dvbsub_getpid());
	
	// tuxtxt
	tuxtx_pause_subtitle( false, live_fe?live_fe->fenumber:0 );
}

// select subtitle
void CNeutrinoApp::SelectSubtitles()
{
	if(!g_settings.auto_subs)
		return;

	int curnum = channelList->getActiveChannelNumber();
	CZapitChannel * cc = channelList->getChannel(curnum);

	for(int i = 0; i < 3; i++) 
	{
		if(strlen(g_settings.pref_subs[i]) == 0)
			continue;

		std::string temp(g_settings.pref_subs[i]);

		// dvbsub
		for(int j = 0 ; j < (int)cc->getSubtitleCount() ; j++) 
		{
			CZapitAbsSub* s = cc->getChannelSub(j);
			if (s->thisSubType == CZapitAbsSub::DVB) 
			{
				CZapitDVBSub* sd = reinterpret_cast<CZapitDVBSub*>(s);
				std::map<std::string, std::string>::const_iterator it;
				for(it = iso639.begin(); it != iso639.end(); it++) 
				{
					if(temp == it->second && sd->ISO639_language_code == it->first) 
					{
						printf("CNeutrinoApp::SelectSubtitles: found DVB %s, pid %x\n", sd->ISO639_language_code.c_str(), sd->pId);
						dvbsub_stop();
						dvbsub_setpid(sd->pId);
						return;
					}
				}
			}
		}
		
		// tuxtxtsub
		for(int j = 0 ; j < (int)cc->getSubtitleCount() ; j++) 
		{
			CZapitAbsSub* s = cc->getChannelSub(j);
			if (s->thisSubType == CZapitAbsSub::TTX) 
			{
				CZapitTTXSub* sd = reinterpret_cast<CZapitTTXSub*>(s);
				std::map<std::string, std::string>::const_iterator it;
				for(it = iso639.begin(); it != iso639.end(); it++) 
				{
					if(temp == it->second && sd->ISO639_language_code == it->first) 
					{
						int page = ((sd->teletext_magazine_number & 0xFF) << 8) | sd->teletext_page_number;
						printf("CNeutrinoApp::SelectSubtitles: found TTX %s, pid %x page %03X\n", sd->ISO639_language_code.c_str(), sd->pId, page);

						tuxtx_stop_subtitle();

						tuxtx_set_pid(sd->pId, page, (char *) sd->ISO639_language_code.c_str());
						return;
					}
				}
			}
		}
	}
}

// signal handler
void sighandler (int signum)
{
	signal(signum, SIG_IGN);
	
        switch (signum) 
	{
		case SIGTERM:
		case SIGINT:
			stop_daemons();
			_exit(0);
			
		  default:
			break;
        }
}

// main function
int main(int argc, char *argv[])
{
	// build date
	printf(">>> neutrinoHD2 (compiled %s %s) <<<\n", __DATE__, __TIME__);
	
	// set debug level (default normal)
	setDebugLevel(DEBUG_INFO);

	// sighandler
        signal(SIGTERM, sighandler);
        signal(SIGINT, sighandler);
        signal(SIGHUP, SIG_IGN);
	signal(SIGHUP, sighandler);
	signal(SIGPIPE, SIG_IGN);
	
	// set localtime
	tzset();
	
	// init globals
	initGlobals();

	char * buf = (char *)malloc(64);
	int count;
	if (buf && (count = readlink("/proc/self/exe", buf, 63)) >= 0) 
	{
		buf[count] = '\0';
		printf("starting %s\n", buf);
		free(buf);
	}
	
	for(int i = 3; i < 256; i++)
		close(i);

	return CNeutrinoApp::getInstance()->run(argc, argv);
}

