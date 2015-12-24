#ifndef __setting_helpers__
#define __setting_helpers__

/*
	Neutrino-GUI  -   DBoxII-Project
	
	$id: setting_helpers.h 2015.12.22 15:24:30 mohousch $

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


#include <gui/widget/menue.h>

/* zapit includes */
#include <client/zapittypes.h>

#include <string>


// dhcp notifier
class CDHCPNotifier : public CChangeObserver
{
	private:
		CMenuForwarder * toDisable[5];
	public:
		CDHCPNotifier( CMenuForwarder*, CMenuForwarder*, CMenuForwarder*, CMenuForwarder*, CMenuForwarder*);
		bool changeNotify(const neutrino_locale_t, void * data);
};

// onoff notifier //needed by moviebrowser
class COnOffNotifier : public CChangeObserver
{
        private:
                int number;
                CMenuItem * toDisable[15];
        public:
                COnOffNotifier(CMenuItem* a1, CMenuItem* a2 = NULL, CMenuItem* a3 = NULL, CMenuItem* a4 = NULL, CMenuItem* a5 = NULL);
		COnOffNotifier(int OffValue = 0);
                bool changeNotify(const neutrino_locale_t, void *Data);
		void addItem(CMenuItem* menuItem);
};

// recording safety notifier
class CRecordingSafetyNotifier : public CChangeObserver
{
	public:
		bool changeNotify(const neutrino_locale_t, void *);
};

// miscnotifier
class CMiscNotifier : public CChangeObserver
{
	private:
		CMenuItem * toDisable[1];
	public:
		CMiscNotifier( CMenuItem * );
		bool changeNotify(const neutrino_locale_t, void *);
};

// lcd notifier
class CLcdNotifier : public CChangeObserver
{
	public:
		bool changeNotify(const neutrino_locale_t, void * Data);
};

// pause sectionsd notifier
class CPauseSectionsdNotifier : public CChangeObserver
{
	public:
		bool changeNotify(const neutrino_locale_t, void * Data);
};

// sectionsd config notifier
class CSectionsdConfigNotifier : public CChangeObserver
{
        public:
                bool changeNotify(const neutrino_locale_t, void * );
};

// color setup notifier
class CColorSetupNotifier : public CChangeObserver
{
	public:
		bool changeNotify(const neutrino_locale_t, void *);
};

// video setup notifier
class CVideoSetupNotifier : public CChangeObserver
{
	public:
		bool changeNotify(const neutrino_locale_t OptionName, void *);
};

// audio setup notifier
class CAudioSetupNotifier : public CChangeObserver
{
	public:
		bool changeNotify(const neutrino_locale_t OptionName, void *);
};

// key setup notifier
class CKeySetupNotifier : public CChangeObserver
{
	public:
		bool changeNotify(const neutrino_locale_t, void *);
};

// IP change notifier
class CIPChangeNotifier : public CChangeObserver
{
	public:
		bool changeNotify(const neutrino_locale_t locale, void * Data);
};

// timing setup notifier
class CTimingSettingsNotifier : public CChangeObserver
{
	public:
		bool changeNotify(const neutrino_locale_t OptionName, void *);
};

// rec apids notifier
class CRecAPIDSettingsNotifier : public CChangeObserver
{
	public:
		bool changeNotify(const neutrino_locale_t OptionName, void*);
};

// subtitle change notifier
class CSubtitleChangeExec : public CMenuTarget
{
	public:
		int exec(CMenuTarget* parent, const std::string & actionKey);
};

// NVOD change notifier
class CNVODChangeExec : public CMenuTarget
{
	public:
		int exec(CMenuTarget* parent, const std::string & actionKey);
};

// tuxtxt notifier
class CTuxtxtChangeExec : public CMenuTarget
{
	public:
		int exec(CMenuTarget* parent, const std::string & actionKey);
};

// USERMENU
class CUserMenuMenu : public CMenuTarget
{
        private:
                int button;
                neutrino_locale_t local;
        public:
                CUserMenuMenu(neutrino_locale_t _local, int _button){local = _local; button = _button;};
                int exec(CMenuTarget* parent, const std::string & actionKey);
};

// TZ change notifier
class CTZChangeNotifier : public CChangeObserver
{
	public:
		bool changeNotify(const neutrino_locale_t, void * data);
};

// data reset notifier
class CDataResetNotifier : public CMenuTarget
{
	public:
		int exec(CMenuTarget* parent, const std::string& actionKey);
};

// autoaudio select notifier
class CAutoAudioNotifier : public CChangeObserver
{
	private:
		CMenuItem * toDisable[4];
		
	public:
		CAutoAudioNotifier(CMenuItem * item1, CMenuItem * item2, CMenuItem * item3, CMenuItem * item4);
		bool changeNotify(const neutrino_locale_t, void * /*data*/);
};

// autoaudio select notifier
class CSubLangSelectNotifier : public CChangeObserver
{
	private:
		CMenuItem * toDisable[3];
		
	public:
		CSubLangSelectNotifier(CMenuItem * item1, CMenuItem * item2, CMenuItem * item3);
		bool changeNotify(const neutrino_locale_t, void * /*data*/);
};

// epglanguage select notifier
class CEPGlangSelectNotifier : public CChangeObserver
{	
	public:
		bool changeNotify(const neutrino_locale_t, void * /*data*/);
};

// volume conf
class CAudioSetupNotifierVolPercent : public CChangeObserver
{
		int apid;
		t_channel_id channel_id;
	public:
		bool changeNotify(const neutrino_locale_t OptionName, void *);
};

void testNetworkSettings(const char* ip, const char* netmask, const char* broadcast, const char* gateway, const char* nameserver, bool dhcp);
void showCurrentNetworkSettings();

#endif
