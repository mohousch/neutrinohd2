/*
  Neutrino-GUI  -   DBoxII-Project

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

#ifndef __settings__
#define __settings__

#include <string>

#include <client/zapitclient.h>

#include <configfile.h>

#include <system/localize.h>

#include <gui/widget/icons.h>
#include <gui/widget/stringinput.h>


struct SNeutrinoSettings
{
	// VIDEO
	int video_Mode;
	int analog_mode;	//scart

	int hdmi_color_space;

	int video_Ratio;
	int video_Format;

	int wss_mode;
	
	unsigned char contrast;
	unsigned char saturation;
	unsigned char brightness;
	unsigned char tint;
	// END VIDEO

	// AUDIO
	int audio_AnalogMode;
	int audio_DolbyDigital;

	int hdmi_dd;

	int avsync;
	
	int ac3_delay;
	int pcm_delay;
	//
	
	// subs
	int auto_lang;
	int auto_subs;
	char pref_lang[3][30];
	char pref_subs[3][30];
	char pref_epgs[3][30];
	// END AUDIO

	// PARENTALLOCK
	int parentallock_prompt;
	int parentallock_lockage;
	char parentallock_pincode[5];
	// END PARENTALLOCK

	// NETWORK
#define NETWORK_NFS_NR_OF_ENTRIES 8
	std::string network_nfs_ip[NETWORK_NFS_NR_OF_ENTRIES];
	char network_nfs_mac[NETWORK_NFS_NR_OF_ENTRIES][MAX_INPUT_CHARS + 1];
	char network_nfs_local_dir[NETWORK_NFS_NR_OF_ENTRIES][100];
	char network_nfs_dir[NETWORK_NFS_NR_OF_ENTRIES][100];
	int  network_nfs_automount[NETWORK_NFS_NR_OF_ENTRIES];
	char network_nfs_mount_options1[NETWORK_NFS_NR_OF_ENTRIES][MAX_INPUT_CHARS + 1];
	char network_nfs_mount_options2[NETWORK_NFS_NR_OF_ENTRIES][MAX_INPUT_CHARS +  1];
	int  network_nfs_type[NETWORK_NFS_NR_OF_ENTRIES];
	char network_nfs_username[NETWORK_NFS_NR_OF_ENTRIES][MAX_INPUT_CHARS + 1];
	char network_nfs_password[NETWORK_NFS_NR_OF_ENTRIES][MAX_INPUT_CHARS + 1];

	std::string network_ntpserver;
	std::string network_ntprefresh;
	int network_ntpenable;
	char ifname[10];
	// END NETWORK

	// RECORDING
	char record_safety_time_before[3];
	char record_safety_time_after[3];
	char network_nfs_recordingdir[100];
	int auto_timeshift; 	// permanent timeshift
	int record_hours;
	unsigned char recording_audio_pids_default;
	int recording_audio_pids_std;
	int recording_audio_pids_alt;
	int recording_audio_pids_ac3;

	int recording_epg_for_filename;
	int recording_save_in_channeldir;
	
	int recording_screenshot;
	// END RECORDING

	// MOVIEPLAYER
	char network_nfs_moviedir[100];
	// END MOVIEPLAYER

	// OSD
	unsigned char gtx_alpha;
	
	char language[25];

	unsigned char menu_Head_alpha;
	unsigned char menu_Head_red;
	unsigned char menu_Head_green;
	unsigned char menu_Head_blue;

	unsigned char menu_Head_Text_alpha;
	unsigned char menu_Head_Text_red;
	unsigned char menu_Head_Text_green;
	unsigned char menu_Head_Text_blue;

	unsigned char menu_Content_alpha;
	unsigned char menu_Content_red;
	unsigned char menu_Content_green;
	unsigned char menu_Content_blue;

	unsigned char menu_Content_Text_alpha;
	unsigned char menu_Content_Text_red;
	unsigned char menu_Content_Text_green;
	unsigned char menu_Content_Text_blue;

	unsigned char menu_Content_Selected_alpha;
	unsigned char menu_Content_Selected_red;
	unsigned char menu_Content_Selected_green;
	unsigned char menu_Content_Selected_blue;

	unsigned char menu_Content_Selected_Text_alpha;
	unsigned char menu_Content_Selected_Text_red;
	unsigned char menu_Content_Selected_Text_green;
	unsigned char menu_Content_Selected_Text_blue;

	unsigned char menu_Content_inactive_alpha;
	unsigned char menu_Content_inactive_red;
	unsigned char menu_Content_inactive_green;
	unsigned char menu_Content_inactive_blue;

	unsigned char menu_Content_inactive_Text_alpha;
	unsigned char menu_Content_inactive_Text_red;
	unsigned char menu_Content_inactive_Text_green;
	unsigned char menu_Content_inactive_Text_blue;

	unsigned char infobar_alpha;
	unsigned char infobar_red;
	unsigned char infobar_green;
	unsigned char infobar_blue;

	unsigned char infobar_Text_alpha;
	unsigned char infobar_Text_red;
	unsigned char infobar_Text_green;
	unsigned char infobar_Text_blue;
	
	unsigned char infobar_colored_events_alpha;
	unsigned char infobar_colored_events_red;
	unsigned char infobar_colored_events_green;
	unsigned char infobar_colored_events_blue;
	
	unsigned char menu_Foot_alpha;
	unsigned char menu_Foot_red;
	unsigned char menu_Foot_green;
	unsigned char menu_Foot_blue;
	
	unsigned char menu_Foot_Text_alpha;
	unsigned char menu_Foot_Text_red;
	unsigned char menu_Foot_Text_green;
	unsigned char menu_Foot_Text_blue;

	unsigned char menu_HeadInfo_alpha;
	unsigned char menu_HeadInfo_red;
	unsigned char menu_HeadInfo_green;
	unsigned char menu_HeadInfo_blue;

	unsigned char menu_HeadInfo_Text_alpha;
	unsigned char menu_HeadInfo_Text_red;
	unsigned char menu_HeadInfo_Text_green;
	unsigned char menu_HeadInfo_Text_blue;

	unsigned char menu_FootInfo_alpha;
	unsigned char menu_FootInfo_red;
	unsigned char menu_FootInfo_green;
	unsigned char menu_FootInfo_blue;
	
	unsigned char menu_FootInfo_Text_alpha;
	unsigned char menu_FootInfo_Text_red;
	unsigned char menu_FootInfo_Text_green;
	unsigned char menu_FootInfo_Text_blue;

	char	font_file[100];

#define TIMING_SETTING_COUNT 6
	enum TIMING_SETTINGS {
		TIMING_MENU        = 0,
		TIMING_CHANLIST    = 1,
		TIMING_EPG         = 2,
		TIMING_INFOBAR     = 3,
		TIMING_FILEBROWSER = 4,
		TIMING_NUMERICZAP  = 5
	};

	int  timing[TIMING_SETTING_COUNT]   ;
	char timing_string[TIMING_SETTING_COUNT][4];

#define FONT_TYPE_COUNT 22
	enum FONT_TYPES {
		FONT_TYPE_MENU                =  0,
		FONT_TYPE_MENU_TITLE          =  1,
		FONT_TYPE_MENU_INFO           =  2,
		FONT_TYPE_EPG_TITLE           =  3,
		FONT_TYPE_EPG_INFO1           =  4,
		FONT_TYPE_EPG_INFO2           =  5,
		FONT_TYPE_EPG_DATE            =  6,
		FONT_TYPE_EVENTLIST_TITLE     =  7,
		FONT_TYPE_EVENTLIST_ITEMLARGE =  8,
		FONT_TYPE_EVENTLIST_ITEMSMALL =  9,
		FONT_TYPE_EVENTLIST_DATETIME  = 10,
		FONT_TYPE_GAMELIST_ITEMLARGE  = 11,
		FONT_TYPE_GAMELIST_ITEMSMALL  = 12,
		FONT_TYPE_CHANNELLIST         = 13,
		FONT_TYPE_CHANNELLIST_DESCR   = 14,
		FONT_TYPE_CHANNELLIST_NUMBER  = 15,
		FONT_TYPE_CHANNEL_NUM_ZAP     = 16,
		FONT_TYPE_INFOBAR_NUMBER      = 17,
		FONT_TYPE_INFOBAR_CHANNAME    = 18,
		FONT_TYPE_INFOBAR_INFO        = 19,
		FONT_TYPE_INFOBAR_SMALL       = 20,
		FONT_TYPE_FILEBROWSER_ITEM    = 21
	};

	int screen_StartX;
	int screen_StartY;
	int screen_EndX;
	int screen_EndY;
	int screen_width;
	int screen_height;
	int screen_xres;
	int screen_yres;
	
	int volume_pos;
	int rounded_corners;
	
	enum MENU_DESIGN {
		MENU_DESIGN_STANDARD = 0,
		MENU_DESIGN_CLASSIC = 1,
		MENU_DESIGN_EXTENDED = 2,
		MENU_DESIGN_FRAME = 3
	};

	int menu_design;

	enum MENU_POSITION {
		MENU_POSITION_LEFT = 0,
		MENU_POSITION_CENTER = 1,
		MENU_POSITION_RIGHT = 2
	};

	int menu_position;
	// END OSD

	// KEYS
	char repeat_blocker[4];
	char repeat_genericblocker[4];

	int key_tvradio_mode;
	
	int key_channelList_pageup;
	int key_channelList_pagedown;
	int key_channelList_cancel;
	int key_channelList_reload;
	int key_channelList_sort;
	int key_channelList_addrecord;
	int key_channelList_addremind;
	int key_channelList_search;

	int key_quickzap_up;
	int key_quickzap_down;

	int key_bouquet_up;
	int key_bouquet_down;

	int key_subchannel_up;
	int key_subchannel_down;

	int key_zaphistory;
	int key_lastchannel;
	int key_list_start;
	int key_list_end;
	
	//pip
	int key_pip;

	int mpkey_rewind;
	int mpkey_forward;
	int mpkey_pause;
	int mpkey_stop;
	int mpkey_play;
	int mpkey_audio;
	int mpkey_time;
	int mpkey_bookmark;

	int key_timeshift;
	
	int key_recordsbrowser;
	int key_audioplayer;
	int key_pictureviewer;
	int key_timerlist;
	int key_inetradio;
	int key_moviebrowser;
	int key_filebrowser;
	int key_webtv;
	
	int key_screenshot;
	
	// mb
	int mb_copy_jump;
	int mb_cut_jump;
	int mb_truncate;
	// END KEYBINDING
	
	// webtv
	std::string webtv_userBouquet;
	unsigned int webtv_lastselectedchannel;

	// USERMENU
        typedef enum
        {
                BUTTON_BLUE = 0,
#if defined (ENABLE_FUNCTIONKEYS)  
		BUTTON_F1 = 1,
		BUTTON_F2 = 2,
		BUTTON_F3 = 3,
		BUTTON_F4 = 4,
#endif
                BUTTON_MAX   // MUST be always the last in the list
        }USER_BUTTON;

        typedef enum
        {
		ITEM_NONE = 0,
		//ITEM_MOVIEPLAYER_TSMB = 0,
                //ITEM_MOVIEPLAYER_MB = 1,
                ITEM_TIMERLIST = 1,
                ITEM_PLUGIN = 2,
                ITEM_VTXT = 3,
                //ITEM_GAME = 5,
                ITEM_OPKG = 4, 
		ITEM_REMOTE = 5,              
                ITEM_MAX   // MUST be always the last in the list
        }USER_ITEM;
	
        std::string usermenu_text[BUTTON_MAX];
        int usermenu[BUTTON_MAX][ITEM_MAX];  // (USER_ITEM)  [button][position in Menue] = feature item
	// END KEYS

	// AUDIOPLAYER
	char network_nfs_audioplayerdir[100];
	int   audioplayer_display;
	int   audioplayer_follow;
	int   audioplayer_highprio;
	int   audioplayer_select_title_by_name;
	int   audioplayer_repeat_on;
	int   audioplayer_read_playlist_at_start;
	// END AUDIOPLAYER

	// PICVIEWER
	char network_nfs_picturedir[100];
	char   picviewer_slide_time[3];
	int    picviewer_scaling;
	int   picviewer_read_playlist_at_start;
	// END PICVIEWER

	// MISC OPTS
	int shutdown_real;
	int shutdown_real_rcdelay;
        char shutdown_count[4];
	int power_standby;
	int infobar_sat_display;
	int infobar_subchan_disp_pos;
	int rotor_swap;
	char timezone[150];

	int radiotext_enable;
	
	std::string logos_dir;
	int epgplus_show_logo;
	int show_channelname;

	// channellist
	int zap_cycle;
	int sms_channel;
	int virtual_zap_mode;
	int channellist_ca;
	int make_hd_list;
	int channellist_extended;

	// epg
	int epg_save;
	int epg_read;
	std::string epg_cache;
	std::string epg_old_events;
	std::string epg_max_events;
	std::string epg_extendedcache;
	std::string epg_dir;

	//filebrowser
	int filesystem_is_utf8;
	int filebrowser_showrights;
	int filebrowser_sortmethod;
	int filebrowser_denydirectoryleave;

	//zapit setup
	int lastChannelMode;
	std::string StartChannelTV;
	std::string StartChannelRadio;
	t_channel_id startchanneltv_id;
	t_channel_id startchannelradio_id;
	int startchanneltv_nr;
	int startchannelradio_nr;
	int uselastchannel;
	
	// inter without GUI setup
	int channel_mode;
	
	// vol
	char current_volume;
	char audio_step[3];
	// MISC OPTS

	// HDD
	int	hdd_sleep;
	int	hdd_noise;
	// END HDD

	// UPDATE
	char	update_dir[100];
	char softupdate_proxyserver[MAX_INPUT_CHARS + 1];
	char softupdate_proxyusername[MAX_INPUT_CHARS + 1];
	char softupdate_proxypassword[MAX_INPUT_CHARS + 1];
	char softupdate_url_file[MAX_INPUT_CHARS + 1];
	// END UPDATE

	// VFD
#if ENABLE_LCD
#define LCD_SETTING_COUNT 11
#else	
#define LCD_SETTING_COUNT 8
#endif
	enum LCD_SETTINGS {
		LCD_BRIGHTNESS         = 0,
		LCD_STANDBY_BRIGHTNESS = 1,
		LCD_CONTRAST           = 2,
		LCD_POWER              = 3,
		LCD_INVERSE            = 4,
		LCD_SHOW_VOLUME        = 5,
		LCD_AUTODIMM           = 6,
		LCD_SCROLL_TEXT		= 7,
#if ENABLE_LCD
		LCD_EPGMODE		= 8,
		LCD_EPGALIGN            = 9,
		LCD_DUMP_PNG		= 10,
#endif		
	};

	int lcd_setting[LCD_SETTING_COUNT];
	char lcd_setting_dim_time[4];
	int lcd_setting_dim_brightness;
	int lcd_ledcolor;
	// END VFD

	// SATIP
	enum {
		SATIP_SERVERBOX_GUI_NMP,
		SATIP_SERVERBOX_GUI_ENIGMA2
	};

	int satip_allow_satip;
	std::string satip_serverbox_ip;
	int satip_serverbox_type;
	int satip_serverbox_gui;

	// MODE
	//int mode;

	// tmdb
	std::string tmdbkey;
	int prefer_tmdb_info;

	// progressbar color
	int progressbar_color;

	// gradient
	int Head_gradient;
	int Foot_gradient;
	int Head_Info_gradient;
	int Foot_Info_gradient;
	int infobar_gradient;

#define FILESYSTEM_ENCODING_TO_UTF8(a) (g_settings.filesystem_is_utf8 ? (a) : Latin1_to_UTF8(a).c_str())
#define UTF8_TO_FILESYSTEM_ENCODING(a) (g_settings.filesystem_is_utf8 ? (a) : UTF8_to_Latin1(a).c_str())	
};

//
extern const int default_timing     [TIMING_SETTING_COUNT];
extern const neutrino_locale_t timing_setting_name[TIMING_SETTING_COUNT];

// lcdd
#if defined(ENABLE_LCD) && (defined(PLATFORM_DREAMBOX) || defined(PLATFORM_GIGABLUE) || defined(PLATFORM_VUPLUS))
#define DEFAULT_LCD_BRIGHTNESS			0xFF
#define DEFAULT_LCD_STANDBYBRIGHTNESS		0x7F
#else
#define DEFAULT_LCD_BRIGHTNESS			0x07
#define DEFAULT_LCD_STANDBYBRIGHTNESS		0x07
#endif
#define DEFAULT_LCD_CONTRAST			0x0F
#define DEFAULT_LCD_POWER			0x01
#define DEFAULT_LCD_INVERSE			0x00
#define DEFAULT_LCD_AUTODIMM			0x00
#define DEFAULT_LCD_SHOW_VOLUME			0x01
#define DEFAULT_LCD_SCROLL_TEXT			0x00
#define DEFAULT_LCD_EPGMODE			0x01	/* 0x1 = name 0x2 = epgtitle 0x4 = separator */
#define DEFAULT_LCD_EPGALIGN			0x00
#define DEFAULT_LCD_DUMP_PNG			0x00

// corners (osd)
#define NO_RADIUS			0
#define ROUNDED				1

#define RADIUS_LARGE    		(g_settings.rounded_corners ? 8 : NO_RADIUS)
#define RADIUS_MID      		(g_settings.rounded_corners ? 4 : NO_RADIUS)
#define RADIUS_SMALL    		(g_settings.rounded_corners ? 2 : NO_RADIUS)

#define BORDER_LEFT			10
#define BORDER_RIGHT			10
#define SCROLLBAR_WIDTH			15
#define ConnectLineBox_Width    	16
#define SHADOW_OFFSET 			5
#define ICON_OFFSET			5	// offset from left border
#define ICON_TO_ICON_OFFSET		2

// parentallock
const int PARENTALLOCK_PROMPT_NEVER          = 0;
const int PARENTALLOCK_PROMPT_ONSTART        = 1;
const int PARENTALLOCK_PROMPT_CHANGETOLOCKED = 2;
const int PARENTALLOCK_PROMPT_ONSIGNAL       = 3;

//
#define widest_number "2"
#define ANNOUNCETIME (1 * 60)

#endif
