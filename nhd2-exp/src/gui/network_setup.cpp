/*
	Neutrino-GUI  -   DBoxII-Project

	$id: network_setup.cpp 2016.01.02 20:19:30 mohousch $
	
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
#include <sys/stat.h>
#include <dirent.h>

#include <libnet.h>

#include <gui/widget/hintbox.h>
#include <gui/widget/stringinput.h>
#include <gui/widget/stringinput_ext.h>
#include <gui/widget/messagebox.h>

#include <gui/network_setup.h>


#include <gui/proxyserver_setup.h>
#include <gui/nfs.h>

#include <system/debug.h>
#include <system/setting_helpers.h>
#include <system/helpers.h>

extern "C" int pinghost( const char *hostname );


#define OPTIONS_OFF0_ON1_OPTION_COUNT 2
const keyval OPTIONS_OFF0_ON1_OPTIONS[OPTIONS_OFF0_ON1_OPTION_COUNT] =
{
        { 0, LOCALE_OPTIONS_OFF, NULL },
        { 1, LOCALE_OPTIONS_ON, NULL }
};

#define OPTIONS_NTPENABLE_OPTION_COUNT 2
const keyval OPTIONS_NTPENABLE_OPTIONS[OPTIONS_NTPENABLE_OPTION_COUNT] =
{
	{ 0, NONEXISTANT_LOCALE, "DVB" },
	{ 1, NONEXISTANT_LOCALE, "NTP" }
};

#define OPTIONS_WLAN_SECURITY_OPTION_COUNT 2
const keyval OPTIONS_WLAN_SECURITY_OPTIONS[OPTIONS_WLAN_SECURITY_OPTION_COUNT] =
{
        { 0, NONEXISTANT_LOCALE, "WPA" },
        { 1, NONEXISTANT_LOCALE, "WPA2"  }
};

static int my_filter(const struct dirent * dent)
{
	if(dent->d_name[0] == 'l' && dent->d_name[1] == 'o')
		return 0;
	if(dent->d_name[0] == '.')
		return 0;
	
	return 1;
}

CNetworkSettings::CNetworkSettings()
{
	networkConfig = CNetworkConfig::getInstance();
}

CNetworkSettings *CNetworkSettings::getInstance()
{
	static CNetworkSettings *networkSettings = NULL;

	if(!networkSettings)
	{
		networkSettings = new CNetworkSettings();
		dprintf(DEBUG_NORMAL, "CNetworkSettings::getInstance: Instance created\n");
	}
	
	return networkSettings;
}

CNetworkSettings::~CNetworkSettings()
{
	//delete networkConfig;
}

void CNetworkSettings::readNetworkSettings(std::string iname)
{
	networkConfig->readConfig(iname);

	network_automatic_start = networkConfig->automatic_start ? 1 : 0;
	network_dhcp = networkConfig->inet_static ? 0 : 1;
	network_hostname = networkConfig->hostname;
	mac_addr = networkConfig->mac_addr;

	//
	network_ssid = networkConfig->ssid;
	network_key = networkConfig->key;
	network_encryption = (networkConfig->encryption == "WPA") ? 0 : 1;
}

void CNetworkSettings::setNetwork()
{
	networkConfig->commitConfig();
	networkConfig->startNetwork();
}

int CNetworkSettings::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CNetworkSettings::exec: actionKey: %s\n", actionKey.c_str());
	
	int ret = RETURN_REPAINT;
	
	if(parent)
		parent->hide();
	
	if(actionKey == "savesettings")
	{
		networkConfig->automatic_start = (network_automatic_start == 1);
		networkConfig->commitConfig();

		CNeutrinoApp::getInstance()->exec(NULL, "savesettings");
		
		return ret;
	}
	else if(actionKey == "network") 
	{
		networkConfig->automatic_start = (network_automatic_start == 1);

		networkConfig->stopNetwork();
		networkConfig->commitConfig();
		networkConfig->startNetwork();

		HintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_NETWORKMENU_SETUPNOW));
		
		return ret;
	}
	else if(actionKey == "networktest") 
	{
		dprintf(DEBUG_INFO, "CNeutrinoApp::exec: doing network test...\n");

		testNetworkSettings(networkConfig->address.c_str(), networkConfig->netmask.c_str(), networkConfig->broadcast.c_str(), networkConfig->gateway.c_str(), networkConfig->nameserver.c_str(), networkConfig->inet_static);
		
		return ret;
	}
	else if(actionKey == "networkshow") 
	{
		dprintf(DEBUG_INFO, "CNeutrinoApp::exec: showing current network settings...\n");
		showCurrentNetworkSettings();
		
		return ret;
	}
	
	showMenu();
	
	return ret;
}

void CNetworkSettings::showMenu()
{
	dprintf(DEBUG_NORMAL, "CNetworkSettings::showMenu:\n");
	
	CMenuWidget networkSettings(LOCALE_NETWORKMENU_HEAD, NEUTRINO_ICON_NETWORK);

	networkSettings.setMode(MODE_SETUP);
	networkSettings.enableShrinkMenu();
	
	struct dirent **namelist;

	//interface
	int ifcount = scandir("/sys/class/net", &namelist, my_filter, alphasort);

	CMenuOptionStringChooser * ifSelect = new CMenuOptionStringChooser(LOCALE_NETWORKMENU_SELECT_IF, g_settings.ifname, ifcount > 1, NULL, RC_nokey, "", true);

	bool found = false;

	for(int i = 0; i < ifcount; i++) 
	{
		ifSelect->addOption(namelist[i]->d_name);
		
		if(strcmp(g_settings.ifname, namelist[i]->d_name) == 0)
			found = true;
		free(namelist[i]);
	}

	if (ifcount >= 0)
		free(namelist);

	if(!found)
		strcpy(g_settings.ifname, "eth0");
	
	// read network settings
	readNetworkSettings(g_settings.ifname);

	// init IP changer
	MyIPChanger = new CIPChangeNotifier;
	
	//eth id
	CMenuForwarder * mac = new CMenuForwarder("MAC", false, mac_addr.c_str());
	
	CIPInput * networkSettings_NetworkIP = new CIPInput(LOCALE_NETWORKMENU_IPADDRESS, networkConfig->address, LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2, MyIPChanger);

	CIPInput * networkSettings_NetMask = new CIPInput(LOCALE_NETWORKMENU_NETMASK, networkConfig->netmask, LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2);

	CIPInput * networkSettings_Broadcast = new CIPInput(LOCALE_NETWORKMENU_BROADCAST, networkConfig->broadcast, LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2);

	CIPInput * networkSettings_Gateway = new CIPInput(LOCALE_NETWORKMENU_GATEWAY, networkConfig->gateway, LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2);

	CIPInput * networkSettings_NameServer = new CIPInput(LOCALE_NETWORKMENU_NAMESERVER, networkConfig->nameserver, LOCALE_IPSETUP_HINT_1, LOCALE_IPSETUP_HINT_2);
	
	//hostname
	CStringInputSMS * networkSettings_Hostname = new CStringInputSMS(LOCALE_NETWORKMENU_HOSTNAME, network_hostname.c_str());

        CSectionsdConfigNotifier * sectionsdConfigNotifier = new CSectionsdConfigNotifier;
	// ntp server
        CStringInputSMS * networkSettings_NtpServer = new CStringInputSMS(LOCALE_NETWORKMENU_NTPSERVER, g_settings.network_ntpserver.c_str(), MAX_INPUT_CHARS, LOCALE_NETWORKMENU_NTPSERVER_HINT1, LOCALE_NETWORKMENU_NTPSERVER_HINT2, "abcdefghijklmnopqrstuvwxyz0123456789-. ", sectionsdConfigNotifier);
        CStringInput * networkSettings_NtpRefresh = new CStringInput(LOCALE_NETWORKMENU_NTPREFRESH, g_settings.network_ntprefresh.c_str(), 3, LOCALE_NETWORKMENU_NTPREFRESH_HINT1, LOCALE_NETWORKMENU_NTPREFRESH_HINT2 , "0123456789 ", sectionsdConfigNotifier);

	CMenuForwarder * m0 = new CMenuForwarder(LOCALE_NETWORKMENU_SETUPNOW, true, NULL, this, "network", RC_green, NEUTRINO_ICON_BUTTON_GREEN);

	CMenuForwarder * m1 = new CMenuForwarder(LOCALE_NETWORKMENU_IPADDRESS, networkConfig->inet_static, networkConfig->address.c_str(), networkSettings_NetworkIP);

	CMenuForwarder * m2 = new CMenuForwarder(LOCALE_NETWORKMENU_NETMASK, networkConfig->inet_static, networkConfig->netmask.c_str(), networkSettings_NetMask);

	CMenuForwarder * m3 = new CMenuForwarder(LOCALE_NETWORKMENU_BROADCAST, networkConfig->inet_static, networkConfig->broadcast.c_str(), networkSettings_Broadcast );

	CMenuForwarder * m4 = new CMenuForwarder(LOCALE_NETWORKMENU_GATEWAY, networkConfig->inet_static, networkConfig->gateway.c_str(), networkSettings_Gateway   );

	CMenuForwarder * m5 = new CMenuForwarder(LOCALE_NETWORKMENU_NAMESERVER, networkConfig->inet_static, networkConfig->nameserver.c_str(), networkSettings_NameServer);

        CMenuForwarder * m6 = new CMenuForwarder( LOCALE_NETWORKMENU_NTPSERVER, true, g_settings.network_ntpserver.c_str(), networkSettings_NtpServer );

        CMenuForwarder * m7 = new CMenuForwarder( LOCALE_NETWORKMENU_NTPREFRESH, true, g_settings.network_ntprefresh.c_str(), networkSettings_NtpRefresh );
	
	CMenuForwarder * m8 = new CMenuForwarder(LOCALE_NETWORKMENU_HOSTNAME, true, network_hostname.c_str(), networkSettings_Hostname);

	CDHCPNotifier * dhcpNotifier = new CDHCPNotifier(m1, m2, m3, m4, m5);

	// setup network on startup
	CMenuOptionChooser * oj = new CMenuOptionChooser(LOCALE_NETWORKMENU_SETUPONSTARTUP, &network_automatic_start, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true);

	// intros
	networkSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	networkSettings.addItem( new CMenuSeparator(LINE) );
	
	// save settings
	networkSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", RC_red, NEUTRINO_ICON_BUTTON_RED));
	networkSettings.addItem( new CMenuSeparator(LINE) );
	
	// setup network on start
	networkSettings.addItem( oj );

	// test network now
	networkSettings.addItem(new CMenuForwarder(LOCALE_NETWORKMENU_TEST, true, NULL, this, "networktest"));

	// show active network settings
	networkSettings.addItem(new CMenuForwarder(LOCALE_NETWORKMENU_SHOW, true, NULL, this, "networkshow", RC_info, NEUTRINO_ICON_BUTTON_HELP_SMALL));
	
	// setup network now
	networkSettings.addItem( m0 );
	
	// mac id
	networkSettings.addItem(new CMenuSeparator(LINE));
	networkSettings.addItem(mac);	//eth id
	
	// interface
	if(ifcount)
		networkSettings.addItem(ifSelect);	//if select
	else
		delete ifSelect;

	networkSettings.addItem(new CMenuSeparator(LINE));

	// dhcp on/off
	oj = new CMenuOptionChooser(LOCALE_NETWORKMENU_DHCP, &network_dhcp, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, dhcpNotifier);
	networkSettings.addItem(oj);

	// hostname
	networkSettings.addItem( m8);

	// ip
	networkSettings.addItem(new CMenuSeparator(LINE));
	networkSettings.addItem( m1);

	// netmask
	networkSettings.addItem( m2);

	// broadcast
	networkSettings.addItem( m3);

	// default gateway
	networkSettings.addItem(new CMenuSeparator(LINE));
	networkSettings.addItem( m4);

	// nameserver
	networkSettings.addItem( m5);
	
	//
	if(ifcount > 1) // if there is only one, its probably wired
	{
		//ssid
		CStringInputSMS * networkSettings_ssid = new CStringInputSMS(LOCALE_NETWORKMENU_SSID, network_ssid.c_str());
		CMenuForwarder * m9 = new CMenuForwarder(LOCALE_NETWORKMENU_SSID, networkConfig->wireless, network_ssid.c_str(), networkSettings_ssid );

		//key
		CStringInputSMS *networkSettings_key = new CStringInputSMS(LOCALE_NETWORKMENU_PASSWORD, network_key.c_str());
		CMenuForwarder *m10 = new CMenuForwarder(LOCALE_NETWORKMENU_PASSWORD, networkConfig->wireless, network_key.c_str(), networkSettings_key );

		wlanEnable[0] = m9;
		wlanEnable[1] = m10;
		
		// ssid
		networkSettings.addItem(new CMenuSeparator(LINE));
		networkSettings.addItem( m9);

		// key
		networkSettings.addItem( m10);

		//encryption
		CMenuOptionChooser * m11 = new CMenuOptionChooser(LOCALE_NETWORKMENU_WLAN_SECURITY, &network_encryption, OPTIONS_WLAN_SECURITY_OPTIONS, OPTIONS_WLAN_SECURITY_OPTION_COUNT, true);
		wlanEnable[2] = m11;
		networkSettings.addItem( m11); //encryption
	}
	
	// ntp
	networkSettings.addItem(new CMenuSeparator(LINE | STRING, g_Locale->getText(LOCALE_NETWORKMENU_NTPTITLE)));

	networkSettings.addItem(new CMenuOptionChooser(LOCALE_NETWORKMENU_NTPENABLE, &g_settings.network_ntpenable, OPTIONS_NTPENABLE_OPTIONS, OPTIONS_NTPENABLE_OPTION_COUNT, true, sectionsdConfigNotifier));

	// ntp server
        networkSettings.addItem( m6);

	// ntp refresh
        networkSettings.addItem( m7);
	
	//proxyserver submenu
	networkSettings.addItem(new CMenuSeparator(LINE));
	networkSettings.addItem(new CMenuForwarder(LOCALE_FLASHUPDATE_PROXYSERVER_SEP, true, NULL, new CProxySetup(LOCALE_FLASHUPDATE_PROXYSERVER_SEP), NULL, RC_nokey, NULL));

	// mount manager
	networkSettings.addItem(new CMenuSeparator(LINE | STRING, g_Locale->getText(LOCALE_NETWORKMENU_MOUNT)));

	networkSettings.addItem(new CMenuForwarder(LOCALE_NFS_MOUNT , true, NULL, new CNFSMountGui(), NULL, RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW));

	networkSettings.addItem(new CMenuForwarder(LOCALE_NFS_UMOUNT, true, NULL, new CNFSUmountGui(), NULL, RC_blue, NEUTRINO_ICON_BUTTON_BLUE));
	
	networkSettings.exec(NULL, "");
	networkSettings.hide();

	delete MyIPChanger;
	delete dhcpNotifier;
	delete sectionsdConfigNotifier;
}

// IP notifier
bool CIPChangeNotifier::changeNotify(const neutrino_locale_t locale, void * Data)
{
	if(locale == LOCALE_NETWORKMENU_IPADDRESS) 
	{
		char ip[16];
		unsigned char _ip[4];
		sscanf((char*) Data, "%hhu.%hhu.%hhu.%hhu", &_ip[0], &_ip[1], &_ip[2], &_ip[3]);

		sprintf(ip, "%hhu.%hhu.%hhu.255", _ip[0], _ip[1], _ip[2]);
		CNetworkSettings::getInstance()->networkConfig->broadcast = ip;

		CNetworkSettings::getInstance()->networkConfig->netmask = (_ip[0] == 10) ? "255.0.0.0" : "255.255.255.0";
	}
	else if(locale == LOCALE_NETWORKMENU_SELECT_IF) 
	{
		CNetworkSettings::getInstance()->networkConfig->readConfig(g_settings.ifname);
		//readNetworkSettings(); //???
		
		dprintf(DEBUG_NORMAL, "CNetworkSetup::changeNotify: using %s, static %d\n", g_settings.ifname, CNetworkSettings::getInstance()->networkConfig->inet_static);

		changeNotify(LOCALE_NETWORKMENU_DHCP, &CNetworkSettings::getInstance()->networkConfig->inet_static);

		int ecnt = sizeof(CNetworkSettings::getInstance()->wlanEnable) / sizeof(CMenuItem*);

		for(int i = 0; i < ecnt; i++)
			CNetworkSettings::getInstance()->wlanEnable[i]->setActive(CNetworkSettings::getInstance()->networkConfig->wireless);

	}

	return true;
}

// dhcp notifier
CDHCPNotifier::CDHCPNotifier( CMenuForwarder* a1, CMenuForwarder* a2, CMenuForwarder* a3, CMenuForwarder* a4, CMenuForwarder* a5)
{
	toDisable[0] = a1;
	toDisable[1] = a2;
	toDisable[2] = a3;
	toDisable[3] = a4;
	toDisable[4] = a5;
}


bool CDHCPNotifier::changeNotify(const neutrino_locale_t, void * data)
{
	CNetworkSettings::getInstance()->networkConfig->inet_static = ((*(int*)(data)) == 0);
	
	for(int x = 0; x < 5; x++)
		toDisable[x]->setActive(CNetworkSettings::getInstance()->networkConfig->inet_static);
	
	return true;
}

//
const char * mypinghost(const char * const host)
{
	int retvalue = pinghost(host);
	switch (retvalue)
	{
		case 1: return (g_Locale->getText(LOCALE_PING_OK));
		case 0: return (g_Locale->getText(LOCALE_PING_UNREACHABLE));
		case -1: return (g_Locale->getText(LOCALE_PING_PROTOCOL));
		case -2: return (g_Locale->getText(LOCALE_PING_SOCKET));
	}
	return "";
}

void testNetworkSettings(const char* ip, const char* netmask, const char* broadcast, const char* gateway, const char* nameserver, bool ip_static)
{
	char our_ip[16];
	char our_mask[16];
	char our_broadcast[16];
	char our_gateway[16];
	char our_nameserver[16];
	std::string text;

	if (ip_static) 
	{
		strcpy(our_ip,ip);
		strcpy(our_mask,netmask);
		strcpy(our_broadcast,broadcast);
		strcpy(our_gateway,gateway);
		strcpy(our_nameserver,nameserver);
	}
	else 
	{
		netGetIP((char *) "eth0",our_ip,our_mask,our_broadcast);
		netGetDefaultRoute(our_gateway);
		netGetNameserver(our_nameserver);
	}

	dprintf(DEBUG_NORMAL, "testNw IP       : %s\n", our_ip);
	dprintf(DEBUG_NORMAL, "testNw Netmask  : %s\n", our_mask);
	dprintf(DEBUG_NORMAL, "testNw Broadcast: %s\n", our_broadcast);
	dprintf(DEBUG_NORMAL, "testNw Gateway: %s\n", our_gateway);
	dprintf(DEBUG_NORMAL, "testNw Nameserver: %s\n", our_nameserver);

	text = our_ip;
	text += ": ";
	text += mypinghost(our_ip);
	text += '\n';
	text += g_Locale->getText(LOCALE_NETWORKMENU_GATEWAY);
	text += ": ";
	text += our_gateway;
	text += ' ';
	text += mypinghost(our_gateway);
	text += '\n';
	text += g_Locale->getText(LOCALE_NETWORKMENU_NAMESERVER);
	text += ": ";
	text += our_nameserver;
	text += ' ';
	text += mypinghost(our_nameserver);
	text += "\nwww.google.de: ";
	text += mypinghost("173.194.35.152");

	MessageBox(LOCALE_NETWORKMENU_TEST, text.c_str(), mbrBack, mbBack, NEUTRINO_ICON_INFO); // UTF-8
}

void showCurrentNetworkSettings()
{
	char ip[16];
	char mask[16];
	char broadcast[16];
	char router[16];
	char nameserver[16];
	std::string mac;
	std::string text;

	//netGetIP((char *) "eth0",ip,mask,broadcast);
	netGetIP(g_settings.ifname, ip, mask, broadcast);
	
	if (ip[0] == 0) 
	{
		text = "Network inactive\n";
	}
	else 
	{
		netGetNameserver(nameserver);
		netGetDefaultRoute(router);

		//netGetMacAddr(g_settings.ifname, (unsigned char *)mac.c_str());
		//text = "Box: " + mac + "\n    ";
		
		text  = g_Locale->getText(LOCALE_NETWORKMENU_IPADDRESS );
		text += ": ";
		text += ip;
		text += '\n';
		text += g_Locale->getText(LOCALE_NETWORKMENU_NETMASK   );
		text += ": ";
		text += mask;
		text += '\n';
		text += g_Locale->getText(LOCALE_NETWORKMENU_BROADCAST );
		text += ": ";
		text += broadcast;
		text += '\n';
		text += g_Locale->getText(LOCALE_NETWORKMENU_NAMESERVER);
		text += ": ";
		text += nameserver;
		text += '\n';
		text += g_Locale->getText(LOCALE_NETWORKMENU_GATEWAY   );
		text += ": ";
		text += router;
	}
	
	MessageBox(LOCALE_NETWORKMENU_SHOW, text.c_str(), mbrBack, mbBack, NEUTRINO_ICON_INFO); // UTF-8
}



