/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: neutrino.h 2013/10/12 mohousch Exp $

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


#ifndef __neutrino__
#define __neutrino__

#include <string>

#include <global.h>

#include <configfile.h>

#include <neutrinoMessages.h>

#include <driver/framebuffer.h>

#include <system/setting_helpers.h>
#include <system/configure_network.h>

#include <timerdclient/timerdtypes.h>

#include <gui/timerlist.h>
#include <gui/channellist.h>          		/* CChannelList */
#include <gui/rc_lock.h>
#include <gui/scan_setup.h>

#include <daemonc/remotecontrol.h>    		/* st_rmsg      */

#include <gui/bouquetlist.h>

// zapit
#include <client/zapitclient.h>


// CNeutrinoApp -  main run-class
typedef struct neutrino_font_descr
{
	const char * name;
	const char * filename;
	int          size_offset;
} neutrino_font_descr_struct;

class CNeutrinoApp : public CMenuTarget, CChangeObserver
{
 	private:
		CFrameBuffer * frameBuffer;

		enum
		{
			mode_unknown = -1,
			mode_tv = 1,		// tv mode
			mode_radio = 2,		// radio mode
			mode_scart = 3,		// scart mode
			mode_standby = 4,	// standby mode
			mode_audio = 5,		// audioplayer mode
			mode_pic = 6,		// pictureviewer mode
			mode_ts = 7,		// movieplayer mode
			mode_webtv = 8,		// webtv mode
			mode_mask = 0xFF,	//
			norezap = 0x100		//
		};

		CConfigFile configfile;

		// font
		neutrino_font_descr_struct font;

		// modes
		int mode;
		int lastMode;
		
		CTimerd::RecordingInfo * nextRecordingInfo;

		struct timeval standby_pressed_at;

		CZapitClient::responseGetLastChannel firstchannel;
		st_rmsg sendmessage;

		bool skipShutdownTimer;

		CNVODChangeExec *NVODChanger;
		CRCLock *rcLock;

		void firstChannel();
		
		void tvMode( bool rezap = true );
		void radioMode( bool rezap = true );
		void webtvMode(bool rezap = true);
		void standbyMode( bool bOnOff );
		void scartMode( bool bOnOff );		// not used
		
		void setvol(int vol);
		
		void RealRun(void);
		void InitZapper();

		void SetupFrameBuffer();
		
		void CmdParser(int argc, char **argv);
	
		bool doGuiRecord(char * preselectedDir, bool addTimer = false);
		
		CNeutrinoApp();

	public:
		void saveSetup(const char * fname);
		int loadSetup(const char * fname);
		void SetupTiming();
		void SetupFonts();
		
		//
		void mainMenu(void);
		bool showUserMenu(int button);
		void SelectNVOD();
                bool getNVODMenu(CMenuWidget * menu);

		void AudioMute( int newValue, bool isEvent= false );
		void setVolume(const neutrino_msg_t key, const bool bDoPaint = true, bool nowait = false);
		~CNeutrinoApp();

		// channellist
		CChannelList* TVchannelList;
		CChannelList* RADIOchannelList;
		CChannelList* webTVchannelList;
		CChannelList* channelList;

		//
		void channelsInit(bool bOnly = false);
		void webTVChannelsInit(bool bOnly = false);
		
		CColorSetupNotifier *colorSetupNotifier;

		static CNeutrinoApp * getInstance();

		//
		void initSectionsdClient();
		void initZapitClient();
		void initTimerdClient();

		void readEPG();
		void saveEpg();

		int run(int argc, char **argv);

		//callback stuff only....
		int exec(CMenuTarget * parent, const std::string & actionKey);

		//onchange
		bool changeNotify(const neutrino_locale_t OptionName, void *);

		int handleMsg(const neutrino_msg_t msg, neutrino_msg_data_t data);

		int getMode() { return mode; }
		int getLastMode() { return lastMode; }
		
		void setupRecordingDevice(void);
		void startNextRecording();
		
		// recording flag
		int recordingstatus;
		// timeshift flag
		int timeshiftstatus;
		// recording_id 
		int recording_id;
		
#if defined (USE_PLAYBACK)
		int playbackstatus;
#endif		
		
		void SendSectionsdConfig(void);
		int GetChannelMode(void) { return g_settings.channel_mode; };
		void SetChannelMode(int newmode, int nMode);
		
		//dvb/txt subs
		void quickZap(int msg);
		void showInfo();
		void StopSubtitles();
		void StartSubtitles(bool show = true);
		void SelectSubtitles();

		//
		void lockPlayBack(void);
		void unlockPlayBack(void);
		
		// 0 - restart 
		// 1 - halt
		// 2 - reboot
		enum {
			RESTART = 0,
			SHUTDOWN,
			REBOOT
		};
		
		void ExitRun(int retcode = SHUTDOWN);
};

#endif
