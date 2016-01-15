/*
	$Id: scan_setup.cpp 2015/12/22 11:42:38 mohousch Exp $

	Neutrino-GUI  -   DBoxII-Project

	scan setup implementation - Neutrino-GUI

	Copyright (C) 2001 Steffen Hehn 'McClean'
	and some other guys
	Homepage: http://dbox.cyberphoria.org/

	Copyright (C) 2009 T. Graf 'dbt'
	Homepage: http://www.dbox2-tuning.net/

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

#include "gui/scan_setup.h"

#include <global.h>
#include <neutrino.h>

#include "gui/scan.h"
#include "gui/motorcontrol.h"

#include <gui/widget/icons.h>
#include <gui/widget/stringinput.h>
#include <gui/widget/hintbox.h>

#include "gui/widget/stringinput.h"
#include "gui/widget/stringinput_ext.h"

#include <driver/screen_max.h>

#include <system/debug.h>

#include <global.h>

/*zapit includes*/
#include <frontend_c.h>
#include <getservices.h>
#include <satconfig.h>


// global
CScanSettings * scanSettings;

char zapit_lat[20];				//defined neutrino.cpp
char zapit_long[20];				//defined neutrino.cpp

// frontend
extern int FrontendCount;			// defined in zapit.cpp
extern CFrontend * getFE(int index);
extern void saveFrontendConfig(int feindex);
extern void loadFrontendConfig();
extern void setMode(fe_mode_t newmode, int feindex);


// option off0_on1
#define OPTIONS_OFF0_ON1_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_OFF0_ON1_OPTIONS[OPTIONS_OFF0_ON1_OPTION_COUNT] =
{
        { 0, LOCALE_OPTIONS_OFF, NULL },
        { 1, LOCALE_OPTIONS_ON, NULL }
};

// option off1 on0
#define OPTIONS_OFF1_ON0_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_OFF1_ON0_OPTIONS[OPTIONS_OFF1_ON0_OPTION_COUNT] =
{
        { 1, LOCALE_OPTIONS_OFF, NULL },
        { 0, LOCALE_OPTIONS_ON, NULL }
};

#define SCANTS_BOUQUET_OPTION_COUNT 2
const CMenuOptionChooser::keyval SCANTS_BOUQUET_OPTIONS[SCANTS_BOUQUET_OPTION_COUNT] =
{
	//{ CZapitClient::BM_DELETEBOUQUETS        , LOCALE_SCANTS_BOUQUET_ERASE, NULL    },
	{ CZapitClient::BM_DONTTOUCHBOUQUETS     , LOCALE_SCANTS_BOUQUET_LEAVE, NULL     },
	{ CZapitClient::BM_UPDATEBOUQUETS        , LOCALE_SCANTS_BOUQUET_UPDATE, NULL    }
};

#define SCANTS_ZAPIT_SCANTYPE_COUNT 4
const CMenuOptionChooser::keyval SCANTS_ZAPIT_SCANTYPE[SCANTS_ZAPIT_SCANTYPE_COUNT] =
{
	{  CZapitClient::ST_TVRADIO	, LOCALE_ZAPIT_SCANTYPE_TVRADIO, NULL },
	{  CZapitClient::ST_TV		, LOCALE_ZAPIT_SCANTYPE_TV, NULL },
	{  CZapitClient::ST_RADIO	, LOCALE_ZAPIT_SCANTYPE_RADIO, NULL },
	{  CZapitClient::ST_ALL		, LOCALE_ZAPIT_SCANTYPE_ALL, NULL }
};

#define SATSETUP_DISEQC_OPTION_COUNT 6
const CMenuOptionChooser::keyval SATSETUP_DISEQC_OPTIONS[SATSETUP_DISEQC_OPTION_COUNT] =
{
	{ NO_DISEQC          , LOCALE_SATSETUP_NODISEQC,	NULL },
	{ MINI_DISEQC        , LOCALE_SATSETUP_MINIDISEQC,	NULL },
	{ DISEQC_1_0         , LOCALE_SATSETUP_DISEQC10,	NULL },
	{ DISEQC_1_1         , LOCALE_SATSETUP_DISEQC11,	NULL },
	{ DISEQC_ADVANCED    , LOCALE_SATSETUP_DISEQ_ADVANCED,	NULL },
	{ SMATV_REMOTE_TUNING, LOCALE_SATSETUP_SMATVREMOTE,	NULL }
};

#define SATSETUP_SCANTP_FEC_COUNT 24
#define CABLESETUP_SCANTP_FEC_COUNT 6
const CMenuOptionChooser::keyval SATSETUP_SCANTP_FEC[SATSETUP_SCANTP_FEC_COUNT] =
{
	{ FEC_NONE, NONEXISTANT_LOCALE, "FEC_NONE" },
	
        { FEC_1_2, LOCALE_SCANTP_FEC_1_2, NULL },
        { FEC_2_3, LOCALE_SCANTP_FEC_2_3, NULL },
        { FEC_3_4, LOCALE_SCANTP_FEC_3_4, NULL },
        { FEC_5_6, LOCALE_SCANTP_FEC_5_6, NULL },
        { FEC_7_8, LOCALE_SCANTP_FEC_7_8, NULL },

        { FEC_S2_QPSK_1_2, LOCALE_FEC_S2_QPSK_1_2, NULL },
        { FEC_S2_QPSK_2_3, LOCALE_FEC_S2_QPSK_2_3, NULL },
        { FEC_S2_QPSK_3_4, LOCALE_FEC_S2_QPSK_3_4, NULL },
        { FEC_S2_QPSK_5_6, LOCALE_FEC_S2_QPSK_5_6, NULL },
        { FEC_S2_QPSK_7_8, LOCALE_FEC_S2_QPSK_7_8, NULL },
        { FEC_S2_QPSK_8_9, LOCALE_FEC_S2_QPSK_8_9, NULL },
        { FEC_S2_QPSK_3_5, LOCALE_FEC_S2_QPSK_3_5, NULL },
        { FEC_S2_QPSK_4_5, LOCALE_FEC_S2_QPSK_4_5, NULL },
        { FEC_S2_QPSK_9_10, LOCALE_FEC_S2_QPSK_9_10, NULL },

        { FEC_S2_8PSK_1_2, LOCALE_FEC_S2_8PSK_1_2, NULL },
        { FEC_S2_8PSK_2_3, LOCALE_FEC_S2_8PSK_2_3, NULL },
        { FEC_S2_8PSK_3_4, LOCALE_FEC_S2_8PSK_3_4, NULL },
        { FEC_S2_8PSK_5_6, LOCALE_FEC_S2_8PSK_5_6, NULL },
        { FEC_S2_8PSK_7_8, LOCALE_FEC_S2_8PSK_7_8, NULL },
        { FEC_S2_8PSK_8_9, LOCALE_FEC_S2_8PSK_8_9, NULL },
        { FEC_S2_8PSK_3_5, LOCALE_FEC_S2_8PSK_3_5, NULL },
        { FEC_S2_8PSK_4_5, LOCALE_FEC_S2_8PSK_4_5, NULL },
        { FEC_S2_8PSK_9_10, LOCALE_FEC_S2_8PSK_9_10, NULL }
};

#define CABLETERRESTRIALSETUP_SCANTP_MOD_COUNT 6
const CMenuOptionChooser::keyval CABLETERRESTRIALSETUP_SCANTP_MOD[CABLETERRESTRIALSETUP_SCANTP_MOD_COUNT] =
{
	// cable
	{ QAM_16, LOCALE_SCANTP_MOD_16, NULL },
	{ QAM_32, LOCALE_SCANTP_MOD_32, NULL },
	{ QAM_64, LOCALE_SCANTP_MOD_64, NULL },
	{ QAM_128, LOCALE_SCANTP_MOD_128, NULL },
	{ QAM_256, LOCALE_SCANTP_MOD_256, NULL },
	
	{ QAM_AUTO, NONEXISTANT_LOCALE, "QAM_AUTO" }
};

#define SATSETUP_SCANTP_MOD_COUNT 2
const CMenuOptionChooser::keyval SATSETUP_SCANTP_MOD[SATSETUP_SCANTP_MOD_COUNT] =
{
	// sat
	{ QPSK, NONEXISTANT_LOCALE, "QPSK" },
#if HAVE_DVB_API_VERSION >= 5
	{ PSK_8, NONEXISTANT_LOCALE, "PSK_8" }
#else
	{ PSK8, NONEXISTANT_LOCALE, "PSK8" }
#endif
};

#define SATSETUP_SCANTP_BAND_COUNT 4
const CMenuOptionChooser::keyval SATSETUP_SCANTP_BAND[SATSETUP_SCANTP_BAND_COUNT] =
{
	{ BANDWIDTH_8_MHZ, NONEXISTANT_LOCALE, "BAND_8" },
	{ BANDWIDTH_7_MHZ, NONEXISTANT_LOCALE, "BAND_7" },
	{ BANDWIDTH_6_MHZ, NONEXISTANT_LOCALE, "BAND_6" },
	{ BANDWIDTH_AUTO, NONEXISTANT_LOCALE, "BAND_AUTO"}
};

// transmition mode
#define TERRESTRIALSETUP_TRANSMIT_MODE_COUNT 3
const CMenuOptionChooser::keyval TERRESTRIALSETUP_TRANSMIT_MODE[TERRESTRIALSETUP_TRANSMIT_MODE_COUNT] =
{
	{ TRANSMISSION_MODE_2K, NONEXISTANT_LOCALE, "2K" },
	{ TRANSMISSION_MODE_8K, NONEXISTANT_LOCALE, "8K" },
	{ TRANSMISSION_MODE_AUTO, NONEXISTANT_LOCALE, "AUTO" },
};

// guard interval
#define TERRESTRIALSETUP_GUARD_INTERVAL_COUNT 5
const CMenuOptionChooser::keyval TERRESTRIALSETUP_GUARD_INTERVAL[TERRESTRIALSETUP_GUARD_INTERVAL_COUNT] =
{
	{ GUARD_INTERVAL_1_32, NONEXISTANT_LOCALE, "1_32" },
	{ GUARD_INTERVAL_1_16, NONEXISTANT_LOCALE, "1_16" },
	{ GUARD_INTERVAL_1_8, NONEXISTANT_LOCALE, "1_8" },
	{ GUARD_INTERVAL_1_4, NONEXISTANT_LOCALE, "1_4"},
	{ GUARD_INTERVAL_AUTO, NONEXISTANT_LOCALE, "AUTO"},
};

// hierarchy
#define TERRESTRIALSETUP_HIERARCHY_COUNT 5
const CMenuOptionChooser::keyval TERRESTRIALSETUP_HIERARCHY[TERRESTRIALSETUP_HIERARCHY_COUNT] =
{
	{ HIERARCHY_NONE, NONEXISTANT_LOCALE, "NONE" },
	{ HIERARCHY_1, NONEXISTANT_LOCALE, "1" },
	{ HIERARCHY_2, NONEXISTANT_LOCALE, "2" },
	{ HIERARCHY_4, NONEXISTANT_LOCALE, "4"},
	{ HIERARCHY_AUTO, NONEXISTANT_LOCALE, "AUTO"},
};

#define SATSETUP_SCANTP_POL_COUNT 2
const CMenuOptionChooser::keyval SATSETUP_SCANTP_POL[SATSETUP_SCANTP_POL_COUNT] =
{
	{ 0, LOCALE_EXTRA_POL_H, NULL },
	{ 1, LOCALE_EXTRA_POL_V, NULL }
};

#define DISEQC_ORDER_OPTION_COUNT 2
const CMenuOptionChooser::keyval DISEQC_ORDER_OPTIONS[DISEQC_ORDER_OPTION_COUNT] =
{
	{ COMMITED_FIRST, LOCALE_SATSETUP_DISEQC_COM_UNCOM, NULL },
	{ UNCOMMITED_FIRST, LOCALE_SATSETUP_DISEQC_UNCOM_COM, NULL  }
};

#define OPTIONS_SOUTH0_NORTH1_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_SOUTH0_NORTH1_OPTIONS[OPTIONS_SOUTH0_NORTH1_OPTION_COUNT] =
{
	{0, LOCALE_EXTRA_SOUTH, NULL},
	{1, LOCALE_EXTRA_NORTH, NULL}
};

#define OPTIONS_EAST0_WEST1_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_EAST0_WEST1_OPTIONS[OPTIONS_EAST0_WEST1_OPTION_COUNT] =
{
	{0, LOCALE_EXTRA_EAST, NULL},
	{1, LOCALE_EXTRA_WEST, NULL}
};

// 
#define FRONTEND_MODE_SINGLE_OPTION_COUNT 2
#define FRONTEND_MODE_TWIN_OPTION_COUNT 3
const CMenuOptionChooser::keyval FRONTEND_MODE_OPTIONS[FRONTEND_MODE_TWIN_OPTION_COUNT] =
{
	{ (fe_mode_t)FE_SINGLE, LOCALE_SCANSETUP_FEMODE_CONNECTED, NULL },
	{ (fe_mode_t)FE_NOTCONNECTED, LOCALE_SCANSETUP_FEMODE_NOTCONNECTED, NULL },
	{ (fe_mode_t)FE_LOOP, LOCALE_SCANSETUP_FEMODE_LOOP, NULL },
};

CScanSetup::CScanSetup(int num)
{
	feindex = num;
	
	scanSettings = new CScanSettings(feindex);
}

CScanSetup::~CScanSetup()
{	
}

void CScanSetup::hide()
{
	CFrameBuffer::getInstance()->paintBackground();
	CFrameBuffer::getInstance()->blit();
}

int CScanSetup::exec(CMenuTarget * parent, const std::string &actionKey)
{
	dprintf(DEBUG_DEBUG, "CScanSetup::exec: actionKey: %s\n", actionKey.c_str());
	
	if (parent)
		parent->hide();

	if(actionKey == "save_scansettings") 
	{
		// hint box
		CHintBox * hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_MAINSETTINGS_SAVESETTINGSNOW_HINT)); // UTF-8
		hintBox->paint();
		
		// save scan.conf
		if(!scanSettings->saveSettings(NEUTRINO_SCAN_SETTINGS_FILE, feindex)) 
			dprintf(DEBUG_NORMAL, "CNeutrinoApp::exec: error while saving scan-settings!\n");
		
		// send directly diseqc
		if( getFE(feindex)->getInfo()->type == FE_QPSK )
		{
			SaveMotorPositions();
			
			//diseqc type
			getFE(feindex)->setDiseqcType((diseqc_t)getFE(feindex)->diseqcType);
			
			// diseqc repeat
			getFE(feindex)->setDiseqcRepeats(getFE(feindex)->diseqcRepeats);
		
			//gotoxx
			getFE(feindex)->gotoXXLatitude = strtod(zapit_lat, NULL);
			getFE(feindex)->gotoXXLongitude = strtod(zapit_long, NULL);
		}
		
		// set fe mode
		setMode(getFE(feindex)->mode, feindex);
		
		// save frontend.conf
		saveFrontendConfig(feindex);
		
		hintBox->hide();
		delete hintBox;
		hintBox = NULL;
		
		return menu_return::RETURN_REPAINT;
	}
	
	showScanService();
	
	return menu_return::RETURN_REPAINT;
}

void CScanSetup::showScanService()
{
	dprintf(DEBUG_NORMAL, "CScanSetup::showScanService: Tuner: %d\n", feindex);
	
	if(!getFE(feindex))
		return;
	
	//load scan settings 
	if( !scanSettings->loadSettings(NEUTRINO_SCAN_SETTINGS_FILE, feindex) ) 
		dprintf(DEBUG_NORMAL, "CScanSetup::CScanSetup: Loading of scan settings failed. Using defaults.\n");
	
	//menue init
	CMenuWidget * scansetup = new CMenuWidget(LOCALE_SERVICEMENU_SCANTS, NEUTRINO_ICON_SETTINGS);
	
	// 
	int dmode = getFE(feindex)->diseqcType;
	int shortcut = 1;
	
	sat_iterator_t sit; //sat list iterator
	
	// load frontend config
	loadFrontendConfig();
	
	// load motor position
	if( getFE(feindex)->getInfo()->type == FE_QPSK) 
		loadMotorPositions();
	
	// intros
	scansetup->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	scansetup->addItem(new CMenuSeparator(CMenuSeparator::LINE));
	
	//save settings
	scansetup->addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "save_scansettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	scansetup->addItem(new CMenuSeparator(CMenuSeparator::LINE));
			
	// init satNotify
	CSatelliteSetupNotifier * satNotify = new CSatelliteSetupNotifier(feindex);
	CScanSetupNotifier * feModeNotifier = new CScanSetupNotifier(feindex);
	
	// Sat Setup
	CMenuWidget * satSetup = new CMenuWidget(LOCALE_SATSETUP_SAT_SETUP, NEUTRINO_ICON_SETTINGS);
	
	satSetup->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	satSetup->addItem(new CMenuSeparator(CMenuSeparator::LINE));

	// satfind menu
	CMenuWidget * satfindMenu = new CMenuWidget(LOCALE_MOTORCONTROL_HEAD, NEUTRINO_ICON_SETTINGS);

	satfindMenu->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	satfindMenu->addItem(new CMenuSeparator(CMenuSeparator::LINE));
		
	// satname (list)
	CMenuOptionStringChooser * satSelect = NULL;
	CMenuWidget * satOnOff = NULL;
	
	// scan setup SAT
	if( getFE(feindex)->getInfo()->type == FE_QPSK) 
	{
		satSelect = new CMenuOptionStringChooser(LOCALE_SATSETUP_SATELLITE, scanSettings->satNameNoDiseqc, true, NULL, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN, true);
			
		satOnOff = new CMenuWidget(LOCALE_SATSETUP_SATELLITE, NEUTRINO_ICON_SETTINGS);
	
		// intros
		satOnOff->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
		satOnOff->addItem(new CMenuSeparator(CMenuSeparator::LINE));

		for(sit = satellitePositions.begin(); sit != satellitePositions.end(); sit++) 
		{
			// satname
			if(sit->second.type == DVB_S)
			{
				satSelect->addOption(sit->second.name.c_str());
				dprintf(DEBUG_DEBUG, "[neutrino] fe(%d) Adding sat menu for %s position %d\n", feindex, sit->second.name.c_str(), sit->first);

				CMenuWidget * tempsat = new CMenuWidget(sit->second.name.c_str(), NEUTRINO_ICON_SETTINGS);
				
				tempsat->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
				tempsat->addItem(new CMenuSeparator(CMenuSeparator::LINE));
				
				// save settings
				tempsat->addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "save_scansettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
				tempsat->addItem(new CMenuSeparator(CMenuSeparator::LINE));

				// satname
				CMenuOptionChooser * inuse = new CMenuOptionChooser(sit->second.name.c_str(),  &sit->second.use_in_scan, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true);

				// diseqc
				CMenuOptionNumberChooser * diseqc = new CMenuOptionNumberChooser(LOCALE_SATSETUP_DISEQC_INPUT, &sit->second.diseqc, ((dmode != NO_DISEQC) && (dmode != DISEQC_ADVANCED)), -1, 15, NULL, 1, -1, LOCALE_OPTIONS_OFF);

				// commited input
				CMenuOptionNumberChooser * comm = new CMenuOptionNumberChooser(LOCALE_SATSETUP_COMM_INPUT, &sit->second.commited, dmode == DISEQC_ADVANCED, -1, 15, NULL, 1, -1, LOCALE_OPTIONS_OFF);

				// uncommited input
				CMenuOptionNumberChooser * uncomm = new CMenuOptionNumberChooser(LOCALE_SATSETUP_UNCOMM_INPUT, &sit->second.uncommited, dmode == DISEQC_ADVANCED, -1, 15, NULL, 1, -1, LOCALE_OPTIONS_OFF);

				// motor position
				CMenuOptionNumberChooser * motor = new CMenuOptionNumberChooser(LOCALE_SATSETUP_MOTOR_POS, &sit->second.motor_position, true, 0, 64, NULL, 0, 0, LOCALE_OPTIONS_OFF);

				// usals
				CMenuOptionChooser * usals = new CMenuOptionChooser(LOCALE_EXTRA_USE_GOTOXX,  &sit->second.use_usals, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true);

				satNotify->addItem(1, diseqc);
				satNotify->addItem(0, comm);
				satNotify->addItem(0, uncomm);

				CIntInput* lofL = new CIntInput(LOCALE_SATSETUP_LOFL, (int&) sit->second.lnbOffsetLow, 5, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE);
				CIntInput* lofH = new CIntInput(LOCALE_SATSETUP_LOFH, (int&) sit->second.lnbOffsetHigh, 5, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE);
				CIntInput* lofS = new CIntInput(LOCALE_SATSETUP_LOFS, (int&) sit->second.lnbSwitch, 5, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE);

				satOnOff->addItem(inuse);
					
				tempsat->addItem(diseqc);
				tempsat->addItem(comm);
				tempsat->addItem(uncomm);
				tempsat->addItem(motor);
				tempsat->addItem(usals);
				tempsat->addItem(new CMenuForwarder(LOCALE_SATSETUP_LOFL, true, lofL->getValue(), lofL ));
				tempsat->addItem(new CMenuForwarder(LOCALE_SATSETUP_LOFH, true, lofH->getValue(), lofH ));
				tempsat->addItem(new CMenuForwarder(LOCALE_SATSETUP_LOFS, true, lofS->getValue(), lofS));
					
				// sat setup
				satSetup->addItem(new CMenuForwarder(sit->second.name.c_str(), true, NULL, tempsat));
			}
		}
	} 
	else if ( getFE(feindex)->getInfo()->type == FE_QAM) 
	{
		satSelect = new CMenuOptionStringChooser(LOCALE_CABLESETUP_PROVIDER, (char*)scanSettings->satNameNoDiseqc, true, NULL, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN, true);

		for(sit = satellitePositions.begin(); sit != satellitePositions.end(); sit++) 
		{
			if(sit->second.type == DVB_C)
			{
				satSelect->addOption(sit->second.name.c_str());
				dprintf(DEBUG_DEBUG, "[neutrino] fe(%d) Adding cable menu for %s position %d\n", feindex, sit->second.name.c_str(), sit->first);
			}
		}
	}
	else if ( getFE(feindex)->getInfo()->type == FE_OFDM) 
	{
		satSelect = new CMenuOptionStringChooser(LOCALE_TERRESTRIALSETUP_PROVIDER, (char*)scanSettings->satNameNoDiseqc, true, NULL, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN, true);

		for(sit = satellitePositions.begin(); sit != satellitePositions.end(); sit++)
		{
			if(sit->second.type == DVB_T)
			{
				satSelect->addOption(sit->second.name.c_str());
				dprintf(DEBUG_DEBUG, "CNeutrinoApp::InitScanSettings fe(%d) Adding terrestrial menu for %s position %d\n", feindex, sit->second.name.c_str(), sit->first);
			}
		}
	}

	// sat select menu
	satfindMenu->addItem(satSelect);

	// motor menu
	CMenuWidget * motorMenu = NULL;

	if ( getFE(feindex)->getInfo()->type == FE_QPSK) 
	{
		satfindMenu->addItem(new CMenuForwarder(LOCALE_MOTORCONTROL_HEAD, true, NULL, new CMotorControl(feindex), "", CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE));

		motorMenu = new CMenuWidget(LOCALE_SATSETUP_EXTENDED_MOTOR, NEUTRINO_ICON_SETTINGS);
		
		// intros
		motorMenu->addItem(new CMenuSeparator(CMenuSeparator::LINE));
		motorMenu->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));

		// save settings
		motorMenu->addItem(new CMenuForwarder(LOCALE_SATSETUP_SAVESETTINGSNOW, true, NULL, this, "save_scansettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));

		motorMenu->addItem(new CMenuForwarder(LOCALE_MOTORCONTROL_HEAD, true, NULL, satfindMenu, "", CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN));

		motorMenu->addItem(new CMenuSeparator(CMenuSeparator::LINE));

		motorMenu->addItem(new CMenuOptionNumberChooser(LOCALE_EXTRA_ZAPIT_ROTATION_SPEED, (int *)&getFE(feindex)->motorRotationSpeed, true, 0, 64, NULL) );

		motorMenu->addItem(new CMenuOptionChooser(LOCALE_EXTRA_USE_GOTOXX,  (int *)&getFE(feindex)->useGotoXX, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true));

		CStringInput * toff;
		CStringInput * taff;
		sprintf(zapit_lat, "%3.6f", getFE(feindex)->gotoXXLatitude);
		sprintf(zapit_long, "%3.6f", getFE(feindex)->gotoXXLongitude);

		// gotoxxladirection
		motorMenu->addItem(new CMenuOptionChooser(LOCALE_EXTRA_LADIR,  (int *)&getFE(feindex)->gotoXXLaDirection, OPTIONS_SOUTH0_NORTH1_OPTIONS, OPTIONS_SOUTH0_NORTH1_OPTION_COUNT, true));

		// latitude
		toff = new CStringInput(LOCALE_EXTRA_LAT, (char *) zapit_lat, 10, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE, "0123456789.");
		motorMenu->addItem(new CMenuForwarder(LOCALE_EXTRA_LAT, true, zapit_lat, toff));

		// gotoxx lodirection
		motorMenu->addItem(new CMenuOptionChooser(LOCALE_EXTRA_LODIR,  (int *)&getFE(feindex)->gotoXXLoDirection, OPTIONS_EAST0_WEST1_OPTIONS, OPTIONS_EAST0_WEST1_OPTION_COUNT, true));

		// longitude
		taff = new CStringInput(LOCALE_EXTRA_LONG, (char *) zapit_long, 10, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE, "0123456789.");
		motorMenu->addItem(new CMenuForwarder(LOCALE_EXTRA_LONG, true, zapit_long, taff));
		
		// usals repeat
		motorMenu->addItem(new CMenuOptionNumberChooser(LOCALE_SATSETUP_USALS_REPEAT, (int *)&getFE(feindex)->repeatUsals, true, 0, 10, NULL, 0, 0, LOCALE_OPTIONS_OFF) );
		
		// rotor swap east/west
		motorMenu->addItem( new CMenuOptionChooser(LOCALE_EXTRA_ROTORSWAP, &g_settings.rotor_swap, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true ));
	}
	
	// frontend mode
	// check for twin
	// mode loop can be used if we hat twice sat tuner, otherwise direct connected or not connected
	// FIXME:
	bool have_twin = false;
	if( getFE(feindex)->getInfo()->type == FE_QPSK || getFE(feindex)->getInfo()->type == FE_OFDM)
	{
		for(int i = 0; i < FrontendCount; i++) 
		{
			if( i != feindex && ( getFE(i)->getInfo()->type == getFE(feindex)->getInfo()->type) )
			{
				have_twin = true;
				break;
			}
		}
	}
	scansetup->addItem(new CMenuOptionChooser(LOCALE_SCANSETUP_FEMODE,  (int *)&getFE(feindex)->mode, FRONTEND_MODE_OPTIONS, have_twin? FRONTEND_MODE_TWIN_OPTION_COUNT:FRONTEND_MODE_SINGLE_OPTION_COUNT, true, feModeNotifier, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN, true ));
	
	scansetup->addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	// scan type
	CMenuOptionChooser * ojScantype = new CMenuOptionChooser(LOCALE_ZAPIT_SCANTYPE, (int *)&scanSettings->scanType, SCANTS_ZAPIT_SCANTYPE, SCANTS_ZAPIT_SCANTYPE_COUNT, ((getFE(feindex)->mode != (fe_mode_t)FE_NOTCONNECTED) && (getFE(feindex)->mode != (fe_mode_t)FE_LOOP)), NULL, CRCInput::convertDigitToKey(shortcut++), "", true);
	feModeNotifier->addItem(0, ojScantype);
	scansetup->addItem(ojScantype);
		
	// bqts
	CMenuOptionChooser * ojBouquets = new CMenuOptionChooser(LOCALE_SCANTS_BOUQUET, (int *)&scanSettings->bouquetMode, SCANTS_BOUQUET_OPTIONS, SCANTS_BOUQUET_OPTION_COUNT, ((getFE(feindex)->mode != (fe_mode_t)FE_NOTCONNECTED) && (getFE(feindex)->mode != (fe_mode_t)FE_LOOP)), NULL, CRCInput::convertDigitToKey(shortcut++), "", true);
	feModeNotifier->addItem(0, ojBouquets);
	scansetup->addItem(ojBouquets);
	
	// NIT
	CMenuOptionChooser * useNit = new CMenuOptionChooser(LOCALE_SATSETUP_USE_NIT, (int *)&scanSettings->scan_mode, OPTIONS_OFF1_ON0_OPTIONS, OPTIONS_OFF1_ON0_OPTION_COUNT, ( (getFE(feindex)->mode != (fe_mode_t)FE_NOTCONNECTED) && (getFE(feindex)->mode != (fe_mode_t)FE_LOOP) ), NULL, CRCInput::convertDigitToKey(shortcut++) );
	feModeNotifier->addItem(0, useNit);
	scansetup->addItem(useNit);
		
	scansetup->addItem(new CMenuSeparator(CMenuSeparator::LINE));
		
	// diseqc/diseqcrepeat/lnb/motor
	CMenuOptionChooser * ojDiseqc = NULL;
	CMenuOptionNumberChooser * ojDiseqcRepeats = NULL;
	CMenuForwarder * fsatSetup = NULL;
	CMenuForwarder * fmotorMenu = NULL;

	if( getFE(feindex)->getInfo()->type == FE_QPSK )
	{
		// diseqc
		ojDiseqc = new CMenuOptionChooser(LOCALE_SATSETUP_DISEQC, (int *)&getFE(feindex)->diseqcType, SATSETUP_DISEQC_OPTIONS, SATSETUP_DISEQC_OPTION_COUNT, ( (getFE(feindex)->mode != (fe_mode_t)FE_NOTCONNECTED) && (getFE(feindex)->mode != FE_LOOP) ), satNotify, CRCInput::convertDigitToKey(shortcut++), "", true);
		feModeNotifier->addItem(1, ojDiseqc);
		
		// diseqc repeat
		ojDiseqcRepeats = new CMenuOptionNumberChooser(LOCALE_SATSETUP_DISEQCREPEAT, (int *)&getFE(feindex)->diseqcRepeats, (dmode != NO_DISEQC) && (dmode != DISEQC_ADVANCED) && (getFE(feindex)->mode != (fe_mode_t)FE_NOTCONNECTED) && (getFE(feindex)->mode != FE_LOOP), 0, 2, NULL);
		satNotify->addItem(1, ojDiseqcRepeats);
		feModeNotifier->addItem(1, ojDiseqcRepeats);

		// lnb setup
		fsatSetup = new CMenuForwarder(LOCALE_SATSETUP_SAT_SETUP, (getFE(feindex)->mode != FE_NOTCONNECTED) && (getFE(feindex)->mode != (fe_mode_t)FE_LOOP), NULL, satSetup, "", CRCInput::convertDigitToKey(shortcut++));
		feModeNotifier->addItem(1, fsatSetup);
		
		// motor setup
		fmotorMenu = new CMenuForwarder(LOCALE_SATSETUP_EXTENDED_MOTOR, (getFE(feindex)->mode != (fe_mode_t)FE_NOTCONNECTED) && (getFE(feindex)->mode != (fe_mode_t)FE_LOOP), NULL, motorMenu, "", CRCInput::convertDigitToKey(shortcut++));
		feModeNotifier->addItem(1, fmotorMenu);
		
		scansetup->addItem(ojDiseqc);
		scansetup->addItem(ojDiseqcRepeats);
		scansetup->addItem(fsatSetup);
		scansetup->addItem(fmotorMenu);
	}

	// manuel scan menu
	CMenuWidget * manualScan = new CMenuWidget(LOCALE_SATSETUP_MANUAL_SCAN, NEUTRINO_ICON_SETTINGS);

	//int man_shortcut = 1;
	CScanTs * scanTs = new CScanTs(feindex);

	// intros
	manualScan->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	manualScan->addItem(new CMenuSeparator(CMenuSeparator::LINE));
	
	// save settings
	manualScan->addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "save_scansettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	manualScan->addItem(new CMenuSeparator(CMenuSeparator::LINE));

	// sat select
	manualScan->addItem(satSelect);
		
	// TP select
	CTPSelectHandler * tpSelect = new CTPSelectHandler(feindex);
		
	manualScan->addItem(new CMenuForwarder(LOCALE_SCANTS_SELECT_TP, true, NULL, tpSelect, "test", CRCInput::RC_nokey ));
		
	// frequency
	int freq_length = 8;

	switch (getFE(feindex)->getInfo()->type)
	{
		case FE_QPSK:
		freq_length = 8;
		break;
		
		case FE_QAM:
		freq_length = 6;
		break;
		
		case FE_OFDM:
		freq_length = 9;
		break;
		
		default:
		freq_length = 8;
		break;
	}
	
	//int freq_length = ( getFE(feindex)->getInfo()->type == FE_QPSK) ? 8 : 6;
	CStringInput * freq = new CStringInput(LOCALE_EXTRA_FREQ, (char *) scanSettings->TP_freq, freq_length, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE, "0123456789");
	CMenuForwarder * Freq = new CMenuForwarder(LOCALE_EXTRA_FREQ, true, scanSettings->TP_freq, freq, "", CRCInput::RC_nokey );
		
	manualScan->addItem(Freq);
		
	// modulation(t/c)/polarisation(sat)
	CMenuOptionChooser * mod_pol = NULL;

	if( getFE(feindex)->getInfo()->type == FE_QPSK )
	{
		mod_pol = new CMenuOptionChooser(LOCALE_EXTRA_POL, (int *)&scanSettings->TP_pol, SATSETUP_SCANTP_POL, SATSETUP_SCANTP_POL_COUNT, true, NULL, CRCInput::RC_nokey, "", true);
	}
	else if( getFE(feindex)->getInfo()->type == FE_QAM)
	{
		mod_pol = new CMenuOptionChooser(LOCALE_EXTRA_MOD, (int *)&scanSettings->TP_mod, CABLETERRESTRIALSETUP_SCANTP_MOD, CABLETERRESTRIALSETUP_SCANTP_MOD_COUNT, true, NULL, CRCInput::RC_nokey, "", true);
	}
	else if( getFE(feindex)->getInfo()->type == FE_OFDM)
	{
		mod_pol = new CMenuOptionChooser(LOCALE_EXTRA_MOD, (int *)&scanSettings->TP_const, CABLETERRESTRIALSETUP_SCANTP_MOD, CABLETERRESTRIALSETUP_SCANTP_MOD_COUNT, true, NULL, CRCInput::RC_nokey, "", true);
	}

	manualScan->addItem(mod_pol);

	// symbol rate
	CStringInput * rate = new CStringInput(LOCALE_EXTRA_RATE, (char *) scanSettings->TP_rate, 8, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE, "0123456789");
	CMenuForwarder * Rate = new CMenuForwarder(LOCALE_EXTRA_RATE, true, scanSettings->TP_rate, rate, "", CRCInput::RC_nokey );

	// fec
	int fec_count = ( getFE(feindex)->getInfo()->type == FE_QPSK) ? SATSETUP_SCANTP_FEC_COUNT : CABLESETUP_SCANTP_FEC_COUNT;
	CMenuOptionChooser * fec = new CMenuOptionChooser(LOCALE_EXTRA_FEC, (int *)&scanSettings->TP_fec, SATSETUP_SCANTP_FEC, fec_count, true, NULL, CRCInput::RC_nokey, "", true);
		
	if( getFE(feindex)->getInfo()->type != FE_OFDM)
	{
		// Rate
		manualScan->addItem(Rate);
			
		// fec
		manualScan->addItem(fec);
	}

	// band/hp/lp/
	if( getFE(feindex)->getInfo()->type == FE_OFDM)
	{
		// Band
		CMenuOptionChooser * Band = new CMenuOptionChooser(LOCALE_EXTRA_BAND, (int *)&scanSettings->TP_band, SATSETUP_SCANTP_BAND, SATSETUP_SCANTP_BAND_COUNT, true, NULL, CRCInput::RC_nokey, "", true);
		manualScan->addItem(Band);

		// HP
		CMenuOptionChooser * HP = new CMenuOptionChooser(LOCALE_EXTRA_HP, (int *)&scanSettings->TP_HP, SATSETUP_SCANTP_FEC, fec_count, true, NULL, CRCInput::RC_nokey, "", true);
		manualScan->addItem(HP);

		// LP
		CMenuOptionChooser * LP = new CMenuOptionChooser(LOCALE_EXTRA_LP, (int *)&scanSettings->TP_LP, SATSETUP_SCANTP_FEC, fec_count, true, NULL, CRCInput::RC_nokey, "", true);
		manualScan->addItem(LP);
		
		// transmition mode
		CMenuOptionChooser * TM = new CMenuOptionChooser(LOCALE_EXTRA_TM, (int *)&scanSettings->TP_trans, TERRESTRIALSETUP_TRANSMIT_MODE, TERRESTRIALSETUP_TRANSMIT_MODE_COUNT, true, NULL, CRCInput::RC_nokey, "", true);
		manualScan->addItem(TM);
		
		// guard intervall
		CMenuOptionChooser * GI = new CMenuOptionChooser(LOCALE_EXTRA_GI, (int *)&scanSettings->TP_guard, TERRESTRIALSETUP_GUARD_INTERVAL, TERRESTRIALSETUP_GUARD_INTERVAL_COUNT, true, NULL, CRCInput::RC_nokey, "", true);
		manualScan->addItem(GI);
		
		// hierarchy
		CMenuOptionChooser * HR = new CMenuOptionChooser(LOCALE_EXTRA_HR, (int *)&scanSettings->TP_hierarchy, TERRESTRIALSETUP_HIERARCHY, TERRESTRIALSETUP_HIERARCHY_COUNT, true, NULL, CRCInput::RC_nokey, "", true);
		manualScan->addItem(HR);
	}	

	manualScan->addItem(new CMenuSeparator(CMenuSeparator::LINE));
		
	// test signal
	manualScan->addItem(new CMenuForwarder(LOCALE_SCANTS_TEST, true, NULL, scanTs, "test", CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW) );
		
	// scan
	manualScan->addItem(new CMenuForwarder(LOCALE_SCANTS_STARTNOW, true, NULL, scanTs, "manual", CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE) );
		
	CMenuForwarder * manScan = new CMenuForwarder(LOCALE_SATSETUP_MANUAL_SCAN, (getFE(feindex)->mode != (fe_mode_t)FE_NOTCONNECTED) && (getFE(feindex)->mode != (fe_mode_t)FE_LOOP), NULL, manualScan, "", CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW);
	feModeNotifier->addItem(0, manScan);
	scansetup->addItem(manScan);
		
	// auto scan menu
	CMenuWidget * autoScan = new CMenuWidget(LOCALE_SATSETUP_AUTO_SCAN, NEUTRINO_ICON_SETTINGS);
	
	// intros
	autoScan->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	autoScan->addItem(new CMenuSeparator(CMenuSeparator::LINE));
	
	// save settings
	autoScan->addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "save_scansettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	autoScan->addItem(new CMenuSeparator(CMenuSeparator::LINE));
		
	// sat select
	autoScan->addItem(satSelect);
		
	// auto scan
	autoScan->addItem(new CMenuForwarder(LOCALE_SCANTS_STARTNOW, true, NULL, scanTs, "auto", CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW) );
		
	// auto scan menu item
	CMenuForwarder * auScan = new CMenuForwarder(LOCALE_SATSETUP_AUTO_SCAN, (getFE(feindex)->mode != (fe_mode_t)FE_NOTCONNECTED) && (getFE(feindex)->mode != (fe_mode_t)FE_LOOP), NULL, autoScan, "", CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE);
	feModeNotifier->addItem(0, auScan);
	
	scansetup->addItem(auScan);

	// scan all sats
	CMenuForwarder * fautoScanAll = NULL;
		
	if( getFE(feindex)->getInfo()->type == FE_QPSK )
	{
		CMenuWidget * autoScanAll = new CMenuWidget(LOCALE_SATSETUP_AUTO_SCAN_ALL, NEUTRINO_ICON_SETTINGS);
			
		fautoScanAll = new CMenuForwarder(LOCALE_SATSETUP_AUTO_SCAN_ALL, ( (dmode != NO_DISEQC) && (getFE(feindex)->mode != (fe_mode_t)FE_NOTCONNECTED) && (getFE(feindex)->mode != (fe_mode_t)FE_LOOP)), NULL, autoScanAll );
		satNotify->addItem(2, fautoScanAll);
		feModeNotifier->addItem(2, fautoScanAll);

		// intros
		autoScanAll->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
		autoScanAll->addItem(new CMenuSeparator(CMenuSeparator::LINE));
		
		// save settings
		autoScanAll->addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "save_scansettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
		autoScanAll->addItem(new CMenuSeparator(CMenuSeparator::LINE));
		
		// sat
		autoScanAll->addItem(new CMenuForwarder(LOCALE_SATSETUP_SATELLITE, true, NULL, satOnOff, "", CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN ));
			
		// scan
		autoScanAll->addItem(new CMenuForwarder(LOCALE_SCANTS_STARTNOW, true, NULL, scanTs, "all", CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW ) );

		// add item 
		scansetup->addItem(fautoScanAll);
	}

	scansetup->exec(NULL, "");
	scansetup->hide();
	delete scansetup;
}

// TPSelectHandler
CTPSelectHandler::CTPSelectHandler(int num)
{
	feindex = num;
}

extern std::map<transponder_id_t, transponder> select_transponders;
int CTPSelectHandler::exec(CMenuTarget* parent, const std::string &/*actionKey*/)
{
	transponder_list_t::iterator tI;
	sat_iterator_t sit;
	t_satellite_position position = 0;
	std::map<int, transponder> tmplist;
	std::map<int, transponder>::iterator tmpI;
	int i;
	char cnt[5];
	int select = -1;
	static int old_selected = 0;
	static t_satellite_position old_position = 0;

	if (parent)
		parent->hide();

	//loop throught satpos
	for(sit = satellitePositions.begin(); sit != satellitePositions.end(); sit++) 
	{
		if(!strcmp(sit->second.name.c_str(), scanSettings->satNameNoDiseqc)) 
		{
			position = sit->first;
			break;
		}
	}

	if(old_position != position) 
	{
		old_selected = 0;
		old_position = position;
	}
	
	printf("CTPSelectHandler::exec: fe(%d) %s position(%d)\n", feindex, scanSettings->satNameNoDiseqc, position);

        CMenuWidget * menu = new CMenuWidget(LOCALE_SCANTS_SELECT_TP, NEUTRINO_ICON_SETTINGS);
        CMenuSelectorTarget * selector = new CMenuSelectorTarget(&select);
	
	i = 0;

	for(tI = select_transponders.begin(); tI != select_transponders.end(); tI++) 
	{
		t_satellite_position satpos = GET_SATELLITEPOSITION_FROM_TRANSPONDER_ID(tI->first) & 0xFFF;
		if(GET_SATELLITEPOSITION_FROM_TRANSPONDER_ID(tI->first) & 0xF000)
			satpos = -satpos;
		
		if(satpos != position)
			continue;

		char buf[128];
		sprintf(cnt, "%d", i);
		char * f, *s, *m;
		
		switch( getFE(feindex)->getInfo()->type) 
		{
			case FE_QPSK:
			{
				getFE(feindex)->getDelSys(tI->second.feparams.u.qpsk.fec_inner, dvbs_get_modulation(tI->second.feparams.u.qpsk.fec_inner),  f, s, m);

				snprintf(buf, sizeof(buf), "%d %c %d %s %s %s ", tI->second.feparams.frequency/1000, tI->second.polarization ? 'V' : 'H', tI->second.feparams.u.qpsk.symbol_rate/1000, f, s, m);
			}
			break;

			case FE_QAM:
			{
				getFE(feindex)->getDelSys(tI->second.feparams.u.qam.fec_inner, tI->second.feparams.u.qam.modulation, f, s, m);

				snprintf(buf, sizeof(buf), "%d %d %s %s %s ", tI->second.feparams.frequency/1000, tI->second.feparams.u.qam.symbol_rate/1000, f, s, m);
			}
			break;

			case FE_OFDM:
			{
				getFE(feindex)->getDelSys(tI->second.feparams.u.ofdm.code_rate_HP, tI->second.feparams.u.ofdm.constellation, f, s, m);

				snprintf(buf, sizeof(buf), "%d %s %s %s ", tI->second.feparams.frequency/1000, f, s, m);
			}
			break;
				
			case FE_ATSC:
				break;
		}
		
		menu->addItem(new CMenuForwarder(buf, true, NULL, selector, cnt), old_selected == i);
		tmplist.insert(std::pair <int, transponder>(i, tI->second));
		i++;
	}

	int retval = menu->exec(NULL, "");
	delete menu;
	delete selector;

	if(select >= 0) 
	{
		old_selected = select;

		tmpI = tmplist.find(select);

		sprintf( scanSettings->TP_freq, "%d", tmpI->second.feparams.frequency);
		
		switch( getFE(feindex)->getInfo()->type) 
		{
			case FE_QPSK:
				printf("CTPSelectHandler::exec: fe(%d) selected TP: freq %d pol %d SR %d fec %d\n", feindex, tmpI->second.feparams.frequency, tmpI->second.polarization, tmpI->second.feparams.u.qpsk.symbol_rate, tmpI->second.feparams.u.qpsk.fec_inner);
					
				sprintf(scanSettings->TP_rate, "%d", tmpI->second.feparams.u.qpsk.symbol_rate);
				scanSettings->TP_fec = tmpI->second.feparams.u.qpsk.fec_inner;
				scanSettings->TP_pol = tmpI->second.polarization;
				break;

			case FE_QAM:
				printf("CTPSelectHandler::exec: fe(%d) selected TP: freq %d SR %d fec %d mod %d\n", feindex, tmpI->second.feparams.frequency, tmpI->second.feparams.u.qpsk.symbol_rate, tmpI->second.feparams.u.qam.fec_inner, tmpI->second.feparams.u.qam.modulation);
					
				sprintf( scanSettings->TP_rate, "%d", tmpI->second.feparams.u.qam.symbol_rate);
				scanSettings->TP_fec = tmpI->second.feparams.u.qam.fec_inner;
				scanSettings->TP_mod = tmpI->second.feparams.u.qam.modulation;
				break;

			case FE_OFDM:
			{
				printf("CTPSelectHandler::exec: fe(%d) selected TP: freq %d band %d HP %d LP %d const %d trans %d guard %d hierarchy %d\n", feindex, tmpI->second.feparams.frequency, tmpI->second.feparams.u.ofdm.bandwidth, tmpI->second.feparams.u.ofdm.code_rate_HP, tmpI->second.feparams.u.ofdm.code_rate_LP, tmpI->second.feparams.u.ofdm.constellation, tmpI->second.feparams.u.ofdm.transmission_mode, tmpI->second.feparams.u.ofdm.guard_interval, tmpI->second.feparams.u.ofdm.hierarchy_information);
					
				scanSettings->TP_band = tmpI->second.feparams.u.ofdm.bandwidth;
				scanSettings->TP_HP = tmpI->second.feparams.u.ofdm.code_rate_HP;
				scanSettings->TP_LP = tmpI->second.feparams.u.ofdm.code_rate_LP;
				scanSettings->TP_const = tmpI->second.feparams.u.ofdm.constellation;
				scanSettings->TP_trans = tmpI->second.feparams.u.ofdm.transmission_mode;
				scanSettings->TP_guard = tmpI->second.feparams.u.ofdm.guard_interval;
				scanSettings->TP_hierarchy = tmpI->second.feparams.u.ofdm.hierarchy_information;
			}
			break;

			case FE_ATSC:
				break;
		}	
	}
	
	if(retval == menu_return::RETURN_EXIT_ALL)
		return menu_return::RETURN_EXIT_ALL;

	return menu_return::RETURN_REPAINT;
}

// scan settings
CScanSettings::CScanSettings( int num)
	: configfile('\t')
{
	//satNameNoDiseqc[0] = 0;
	strcpy(satNameNoDiseqc, "none");
	bouquetMode     = CZapitClient::BM_UPDATEBOUQUETS;
	//scanType = CServiceScan::SCAN_TVRADIO;
	
	feindex = num;
}

// borrowed from cst neutrino-hd (femanager.cpp)
uint32_t CScanSettings::getConfigValue(int num, const char * name, uint32_t defval)
{
	char cfg_key[81];
	sprintf(cfg_key, "fe%d_%s", num, name);
	
	return configfile.getInt32(cfg_key, defval);
}

// borrowed from cst neutrino-hd (femanger.cpp)
void CScanSettings::setConfigValue(int num, const char * name, uint32_t val)
{
	char cfg_key[81];
	
	sprintf(cfg_key, "fe%d_%s", num, name);
	configfile.setInt32(cfg_key, val);
}

bool CScanSettings::loadSettings(const char * const fileName, int index)
{
	printf("CScanSettings::loadSettings: fe%d\n", index);
	
	/* if scan.conf not exists load default */
	if(!configfile.loadConfig(fileName))
		printf("%s not found\n", fileName);
	
	if( !getFE(index) )
		return false;
	
	// common
	scanType = (CZapitClient::scanType) getConfigValue(index, "scanType", CZapitClient::ST_ALL);
	bouquetMode = (CZapitClient::bouquetMode) getConfigValue(index, "bouquetMode", CZapitClient::BM_UPDATEBOUQUETS);
	
	char cfg_key[81];
	sprintf(cfg_key, "fe%d_satNameNoDiseqc", index);
	strcpy(satNameNoDiseqc, configfile.getString(cfg_key, "none").c_str());
	
	scan_mode = getConfigValue(index, "scan_mode", 1); // NIT (0) or fast (1)
	scanSectionsd = getConfigValue(index, "scanSectionsd", 0);
	
	// freq
	sprintf(cfg_key, "fe%d_TP_freq", index);
	strcpy(TP_freq, configfile.getString(cfg_key, "10100000").c_str());
	
	// rate
	sprintf(cfg_key, "fe%d_TP_rate", index);
	strcpy(TP_rate, configfile.getString(cfg_key, "27500000").c_str());
	
	if(getFE(index)->getInfo()->type == FE_QPSK)
	{
		TP_fec = getConfigValue(index, "TP_fec", 1);
		TP_pol = getConfigValue(index, "TP_pol", 0);
	}
		
	if(getFE(index)->getInfo()->type == FE_QAM)
	{
		TP_mod = getConfigValue(index, "TP_mod", 3);
		TP_fec = getConfigValue(index, "TP_fec", 1);
	}
	
#if HAVE_DVB_API_VERSION >= 3
	if(TP_fec == 4) 
		TP_fec = 5;
#endif

	//DVB-T
	if(getFE(index)->getInfo()->type == FE_OFDM)
	{
		TP_band = getConfigValue(index, "TP_band", 0);
		TP_HP = getConfigValue(index, "TP_HP", 2);
		TP_LP = getConfigValue(index, "TP_LP", 1);
		TP_const = getConfigValue(index, "TP_const", 1);
		TP_trans = getConfigValue(index, "TP_trans", 1);
		TP_guard = getConfigValue(index, "TP_guard", 3);
		TP_hierarchy = getConfigValue(index, "TP_hierarchy", 0);
	}

	return true;
}

bool CScanSettings::saveSettings(const char * const fileName, int index)
{
	printf("CScanSettings::saveSettings: fe%d\n", index);
	
	// common
	setConfigValue(index, "scanType", scanType );
	setConfigValue(index, "bouquetMode", bouquetMode );
	
	char cfg_key[81];
	sprintf(cfg_key, "fe%d_satNameNoDiseqc", index);
	configfile.setString(cfg_key, satNameNoDiseqc );
	
	setConfigValue(index, "scan_mode", scan_mode);
	setConfigValue(index, "scanSectionsd", scanSectionsd ); // sectionsd
	
	// freq
	sprintf(cfg_key, "fe%d_TP_freq", index);
	configfile.setString(cfg_key, TP_freq);
	
	// rate
	sprintf(cfg_key, "fe%d_TP_rate", index);
	configfile.setString(cfg_key, TP_rate);
	
	if(getFE(index)->getInfo()->type == FE_QPSK)
	{
		setConfigValue(index, "TP_pol", TP_pol);
		setConfigValue(index, "TP_fec", TP_fec);
	}
	
	if(getFE(index)->getInfo()->type == FE_QAM)
	{
		setConfigValue(index, "TP_mod", TP_mod);
		setConfigValue(index, "TP_fec", TP_fec);
	}

	if(getFE(index)->getInfo()->type == FE_OFDM)
	{
		setConfigValue(index, "TP_band", TP_band);
		setConfigValue(index, "TP_HP", TP_HP);
		setConfigValue(index, "TP_LP", TP_LP);
		setConfigValue(index, "TP_const", TP_const);
		setConfigValue(index, "TP_trans", TP_trans);
		setConfigValue(index, "TP_guard", TP_guard);
		setConfigValue(index, "TP_hierarchy", TP_hierarchy);
	}

	if(configfile.getModifiedFlag())
	{
		/* save neu configuration */
		configfile.saveConfig(fileName);
	}

	return true;
}

CSatelliteSetupNotifier::CSatelliteSetupNotifier(int num)
{
	feindex = num;
}

/* items1 enabled for advanced diseqc settings, items2 for diseqc != NO_DISEQC, items3 disabled for NO_DISEQC */
bool CSatelliteSetupNotifier::changeNotify(const neutrino_locale_t, void * Data)
{
	std::vector<CMenuItem*>::iterator it;
	int type = *((int*) Data);

	if (type == NO_DISEQC) 
	{
		for(it = items1.begin(); it != items1.end(); it++) 
		{
			(*it)->setActive(false);
		}
		for(it = items2.begin(); it != items2.end(); it++) 
		{
			(*it)->setActive(false);
		}
		for(it = items3.begin(); it != items3.end(); it++) 
		{
			(*it)->setActive(false);
		}
	}
	else if(type < DISEQC_ADVANCED) 
	{
		for(it = items1.begin(); it != items1.end(); it++) 
		{
			(*it)->setActive(false);
		}
		for(it = items2.begin(); it != items2.end(); it++) 
		{
			(*it)->setActive(true);
		}
		for(it = items3.begin(); it != items3.end(); it++) 
		{
			(*it)->setActive(true);
		}
	}
	else if(type == DISEQC_ADVANCED) 
	{
		for(it = items1.begin(); it != items1.end(); it++) 
		{
			(*it)->setActive(true);
		}
		for(it = items2.begin(); it != items2.end(); it++) 
		{
			(*it)->setActive(false);
		}
		for(it = items3.begin(); it != items3.end(); it++) 
		{
			(*it)->setActive(true);
		}
	}

	getFE(feindex)->setDiseqcType( getFE(feindex)->diseqcType );
	getFE(feindex)->setDiseqcRepeats( getFE(feindex)->diseqcRepeats );

	return true;
}

void CSatelliteSetupNotifier::addItem(int list, CMenuItem* item)
{
	switch(list) 
	{
		case 0:
			items1.push_back(item);
			break;
		case 1:
			items2.push_back(item);
			break;
		case 2:
			items3.push_back(item);
			break;
		default:
			break;
	}
}

// scansetup notifier
CScanSetupNotifier::CScanSetupNotifier(int num)
{
	feindex = num;
}

/* items1 enabled for advanced diseqc settings, items2 for diseqc != NO_DISEQC, items3 disabled for NO_DISEQC */
bool CScanSetupNotifier::changeNotify(const neutrino_locale_t, void * Data)
{
	std::vector<CMenuItem*>::iterator it;
	int FeMode = *((int*) Data);
	
	dprintf(DEBUG_NORMAL, "CScanSetupNotifier::changeNotify: Femode:%d\n", FeMode);

	if ( (FeMode == FE_NOTCONNECTED) || (FeMode == FE_LOOP) ) 
	{
		for(it = items1.begin(); it != items1.end(); it++) 
		{
			(*it)->setActive(false);
		}
		for(it = items2.begin(); it != items2.end(); it++) 
		{
			(*it)->setActive(false);
		}
		for(it = items3.begin(); it != items3.end(); it++) 
		{
			(*it)->setActive(false);
		}
	}
	else
	{
		for(it = items1.begin(); it != items1.end(); it++) 
		{
			(*it)->setActive(true);
		}
		for(it = items2.begin(); it != items2.end(); it++) 
		{
			(*it)->setActive(true);
		}
		for(it = items3.begin(); it != items3.end(); it++) 
		{
			(*it)->setActive(true);
		}
	}

	return true;
}

void CScanSetupNotifier::addItem(int list, CMenuItem *item)
{
	switch(list) 
	{
		case 0:
			items1.push_back(item);
			break;	
		case 1:
			items2.push_back(item);
			break;
		case 2:
			items3.push_back(item);
			break;
		default:
			break;
	}
}

//
CTunerSetup::CTunerSetup()
{
}

CTunerSetup::~CTunerSetup()
{
}

int CTunerSetup::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CTunerSetup::exec: actionKey:%s\n", actionKey.c_str());
	
	int ret = menu_return::RETURN_REPAINT;
	
	if(parent)
		parent->hide();
	
	showMenu();
	
	return ret;
}

void CTunerSetup::showMenu()
{
	dprintf(DEBUG_NORMAL, "CTunerSetup::showMenu\n");
	
	CMenuWidget TunerSetup( LOCALE_SERVICEMENU_SCANTS, NEUTRINO_ICON_SETTINGS);
	
	// intros
	TunerSetup.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	TunerSetup.addItem( new CMenuSeparator(CMenuSeparator::LINE) );
			
	for(int i = 0; i < FrontendCount; i++)
	{
		CFrontend * fe = getFE(i);
		char tbuf[255];
			
		sprintf(tbuf, "Tuner-%d: %s", i + 1, fe->getInfo()->name);
		TunerSetup.addItem(new CMenuForwarder(tbuf, true, NULL, new CScanSetup(i) ));
	}
	
	TunerSetup.exec(NULL, "");
	TunerSetup.hide();
}

