/*
  $Id: satipclient.cpp 16.08.2016 22.13.30 mohousch Exp $

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

#include <satipclient.h>


extern "C" void plugin_exec(void);
extern "C" void plugin_init(void);
extern "C" void plugin_del(void);


// option off0_on1
#define OPTIONS_OFF0_ON1_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_OFF0_ON1_OPTIONS[OPTIONS_OFF0_ON1_OPTION_COUNT] =
{
        { 0, LOCALE_OPTIONS_OFF, NULL },
        { 1, LOCALE_OPTIONS_ON, NULL }
};

// frontend typ
#define SATIP_FRONTEND_TYPE_OPTION_COUNT 3
const CMenuOptionChooser::keyval SATIP_FRONTEND_TYPE_OPTIONS[SATIP_FRONTEND_TYPE_OPTION_COUNT] =
{
	{ 0, NONEXISTANT_LOCALE, "Sat" },
	{ 1, NONEXISTANT_LOCALE, "Cable" },
	{ 2, NONEXISTANT_LOCALE, "Terrestrial" }
};

CSatIPClient::CSatIPClient()
{
}

CSatIPClient::~CSatIPClient()
{
}

void CSatIPClient::hide()
{
	CFrameBuffer::getInstance()->paintBackground();
	CFrameBuffer::getInstance()->blit();
}

void CSatIPClient::ReadSettings() 
{
	CConfigFile *satipclient_config = new CConfigFile(',');
	satipclient_config->clear();
	satipclient_config->loadConfig(CONFIG_FILE);
	
	// disabled
	SatIPDisabled = satipclient_config->getInt32("DISABLED", 0);
	// satip server ip
	SatIPServerIP = satipclient_config->getString("SATIPSERVER", "192.168.0.12");
	// satip server port default 554
	//SatIPServerPort = satipclient_config->getString("SATIPPORT", "554");
	// frontend typ
	SatIPFrontendTyp = satipclient_config->getInt32("FRONTENDTYPE", 1);
	// vtuner device
	SatIPVtunerDevice = satipclient_config->getString("VTUNER", "/dev/vtunerc0");
	// debug
	SatIPDebug = satipclient_config->getInt32("LOGLEVEL", 0);
}

bool CSatIPClient::SaveSettings() 
{
	CConfigFile *satipclient_config = new CConfigFile(',');
	
	// satip disabled
	satipclient_config->setInt32("DISABLED", 0);
	// satipserver ip
	satipclient_config->setString("SATIPSERVER", SatIPServerIP);
	// satipserver port
	//satipclient_config->setString("SATIPPORT", SatIPServerPort);
	// frontend typ
	satipclient_config->setInt32("FRONTENDTYPE", SatIPFrontendTyp);
	// vtuner device
	satipclient_config->setString("VTUNER", SatIPVtunerDevice);
	// debug
	satipclient_config->setInt32("LOGLEVEL", SatIPDebug);
	
	satipclient_config->saveConfig(CONFIG_FILE);
	
	return true;
}

bool CSatIPClient::loadVTuner() 
{
	std::string cmd = "insmod /var/lib/modules/vtunerc.ko devices=1 debug=0";
	system(cmd.c_str());

	return true;
}

bool CSatIPClient::unloadVTuner() 
{
	std::string cmd = "rmmod vtunerc";
	system(cmd.c_str());

	return true;
}

bool CSatIPClient::startSatIPClient() 
{
	std::string cmd = "/var/bin/satip_client";
	cmd += " -s ";
	cmd += SatIPServerIP;
	cmd += " -t ";
	cmd += SatIPFrontendTyp;
	cmd += " -d ";
	cmd += SatIPVtunerDevice;
	cmd += " -l ";
	cmd += SatIPDebug;

	system(cmd.c_str());

	return true;
}

bool CSatIPClient::stopSatIPClient() 
{
	std::string cmd = "killall -9 satip_client";
	system(cmd.c_str());

	return true;
}

int CSatIPClient::exec(CMenuTarget* parent, const std::string &actionKey)
{
	if(parent)
		parent->hide();
	
	if(actionKey == "save")
	{
		//SaveSettings();
		if(this->SaveSettings())
		 	HintBox(LOCALE_MESSAGEBOX_INFO, "Einstellungen werden gespeichert!");
		else
		 	HintBox(LOCALE_MESSAGEBOX_INFO, "Einstellungen NICHT gespeichert!");
	}
	else if(actionKey == "start")
	{
		// load vtuner driver
		loadVTuner();

		// start satip_client
		startSatIPClient();
	}
	else if(actionKey == "stop")
	{
		// stop satip_client
		stopSatIPClient();

		// unload vtuner driver
		unloadVTuner();
	}
	
	return menu_return::RETURN_REPAINT;
}

void CSatIPClient::doMenu()
{
	// read settings
	ReadSettings();
	
	// create menu
	CMenuWidget * satIPClientMenu = new CMenuWidget("SatIP Client", NEUTRINO_ICON_SETTINGS);

	satIPClientMenu->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	satIPClientMenu->addItem(new CMenuSeparator(CMenuSeparator::LINE));
	satIPClientMenu->addItem(new CMenuForwarder("Einstellungen speichern", true, NULL, this, "save", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	satIPClientMenu->addItem(new CMenuSeparator(CMenuSeparator::LINE));

	// disabled
	satIPClientMenu->addItem(new CMenuOptionChooser("SatIP Client Disabled", &SatIPDisabled, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, NULL));
	
	// satipserver ip
	CStringInputSMS * SATIPSERVER_IP = new CStringInputSMS((char *)"SatIP Server IP", (char *)SatIPServerIP.c_str());
	satIPClientMenu->addItem(new CMenuForwarder("SatIP Server IP", true, SatIPServerIP, SATIPSERVER_IP, NULL));
	
	// satipserver port
	CStringInputSMS * SATIPSERVER_PORT = new CStringInputSMS((char *)"SatIP Server Port", (char *)SatIPServerPort.c_str());
	satIPClientMenu->addItem(new CMenuForwarder("SatIP Server Port", false, SatIPServerPort, SATIPSERVER_PORT, NULL));
	
	// frontendtyp
	satIPClientMenu->addItem(new CMenuOptionChooser("Tuner typ", &SatIPFrontendTyp, SATIP_FRONTEND_TYPE_OPTIONS, SATIP_FRONTEND_TYPE_OPTION_COUNT, true, NULL));

	// vtuner device
	CStringInputSMS * VTUNER_DEVICE = new CStringInputSMS((char *)"VTuner Device", (char *)SatIPVtunerDevice.c_str());
	satIPClientMenu->addItem(new CMenuForwarder("VTuner Device", true, SatIPVtunerDevice, VTUNER_DEVICE, NULL));

	// debug
	satIPClientMenu->addItem(new CMenuOptionChooser("SatIP Client Debug", &SatIPDebug, OPTIONS_OFF0_ON1_OPTIONS, OPTIONS_OFF0_ON1_OPTION_COUNT, true, NULL));

	satIPClientMenu->addItem(new CMenuSeparator(CMenuSeparator::LINE));

	// start satipclient
	satIPClientMenu->addItem(new CMenuForwarder("Start SatIP Client", true, NULL, this, "start", CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN));

	// stop satipclient
	satIPClientMenu->addItem(new CMenuForwarder("Stop SatIP Client", true, NULL, this, "stop", CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW));

	satIPClientMenu->exec(NULL, "");
	satIPClientMenu->hide();
	
	delete satIPClientMenu;
}

void plugin_init(void)
{
	CSatIPClient * SatIPClientHandler = new CSatIPClient();
	SatIPClientHandler->ReadSettings();

	if(SatIPClientHandler->SatIPDisabled == 0)
	{
		// load vtuner driver
		SatIPClientHandler->loadVTuner();

		// start satip_client
		SatIPClientHandler->startSatIPClient();
	}	
	
	delete SatIPClientHandler;
}

void plugin_del(void)
{
	CSatIPClient * SatIPClientHandler = new CSatIPClient();

	// stop satipclient
	SatIPClientHandler->stopSatIPClient();

	// unload vtuner driver
	SatIPClientHandler->unloadVTuner();

	delete SatIPClientHandler;
}

void plugin_exec(void)
{
	// class handler
	CSatIPClient * SatIPClientHandler = new CSatIPClient();
	SatIPClientHandler->doMenu();
	
	delete SatIPClientHandler;
}

