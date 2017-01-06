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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <gui/scan.h>

#include <driver/rcinput.h>
#include <driver/screen_max.h>

#include <gui/color.h>

#include <gui/widget/menue.h>
#include <gui/widget/messagebox.h>

#include <system/settings.h>
#include <system/debug.h>

#include <global.h>
#include <neutrino.h>

/*zapit includes*/
#include <satconfig.h>
#include <frontend_c.h>

#include <gui/pictureviewer.h>


#define NEUTRINO_SCAN_START_SCRIPT	CONFIGDIR "/scan.start"
#define NEUTRINO_SCAN_STOP_SCRIPT	CONFIGDIR "/scan.stop"

#define RED_BAR 40
#define YELLOW_BAR 70
#define GREEN_BAR 100
#define BAR_BORDER 2
#define BAR_WIDTH 150
#define BAR_HEIGHT 16//(13 + BAR_BORDER*2)

TP_params TP;
CFrontend * getFE(int index);
extern CScanSettings * scanSettings;		// defined in 


CScanTs::CScanTs(int num)
{
	frameBuffer = CFrameBuffer::getInstance();
	radar = 0;
	total = done = 0;
	freqready = 0;

	sigscale = new CProgressBar(BAR_WIDTH, BAR_HEIGHT, RED_BAR, GREEN_BAR, YELLOW_BAR);
	snrscale = new CProgressBar(BAR_WIDTH, BAR_HEIGHT, RED_BAR, GREEN_BAR, YELLOW_BAR);
	
	feindex = num;
}

int CScanTs::exec(CMenuTarget * parent, const std::string & actionKey)
{
	dprintf(DEBUG_DEBUG, "CScanTs::exec: ectionKey: %s\n", actionKey.c_str());

	if(parent)
		parent->hide();
	
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	int scan_mode = scanSettings->scan_mode;
	bool scan_all = actionKey == "all";
	bool test = actionKey == "test";
	bool manual = (actionKey == "manual") || test;
	
	sat_iterator_t sit;
	CZapitClient::ScanSatelliteList satList;
	CZapitClient::commandSetScanSatelliteList sat;

	// window size
	int _iw, _ih;
	frameBuffer->getIconSize(NEUTRINO_ICON_SETTINGS, &_iw, &_ih);
	hheight = std::max(g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), _ih) + 6;
	
	//
	mheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	
	width = w_max(MENU_WIDTH + 100, 0);
	height = h_max(hheight + (10 * mheight), 0); //9 lines
	
	x = frameBuffer->getScreenX() + (frameBuffer->getScreenWidth() - width) / 2;
	y = frameBuffer->getScreenY() + (frameBuffer->getScreenHeight() - height) / 2;
	
	xpos_radar = x + width - 90;
	ypos_radar = y + hheight + (mheight >> 1);
	xpos1 = x + BORDER_LEFT;

	if(scan_all)
		scan_mode |= 0xFF00;

	sigscale->reset();
	snrscale->reset();

	if (!frameBuffer->getActive())
		return menu_return::RETURN_EXIT_ALL;

        g_Zapit->stopPlayBack();

	g_Sectionsd->setPauseScanning(true);

	CVFD::getInstance()->setMode(CVFD::MODE_MENU_UTF8);

	// refill sat list and set feparams for manuel scan
	satList.clear();

	if(manual || !scan_all) 
	{
		for(sit = satellitePositions.begin(); sit != satellitePositions.end(); sit++) 
		{
			if(!strcmp(sit->second.name.c_str(), scanSettings->satNameNoDiseqc)) 
			{
				sat.position = sit->first;
				strncpy(sat.satName, scanSettings->satNameNoDiseqc, 50);
			
				satList.push_back(sat);
				break;
			}
		}
		
		// scan mode
		TP.scan_mode = scanSettings->scan_mode;
		
		// freq
		TP.feparams.frequency = atoi(scanSettings->TP_freq);
		
		if( getFE(feindex)->getInfo()->type == FE_QPSK )
		{
			TP.feparams.u.qpsk.symbol_rate = atoi(scanSettings->TP_rate);
			TP.feparams.u.qpsk.fec_inner = (fe_code_rate_t) scanSettings->TP_fec;
			TP.polarization = scanSettings->TP_pol;

			printf("CScanTs::exec: fe(%d) freq %d rate %d fec %d pol %d\n", feindex, TP.feparams.frequency, TP.feparams.u.qpsk.symbol_rate, TP.feparams.u.qpsk.fec_inner, TP.polarization/*, TP.feparams.u.qpsk.modulation*/ );
		} 
		else if( getFE(feindex)->getInfo()->type == FE_QAM )
		{
			TP.feparams.u.qam.symbol_rate	= atoi(scanSettings->TP_rate);
			TP.feparams.u.qam.fec_inner	= (fe_code_rate_t)scanSettings->TP_fec;
			TP.feparams.u.qam.modulation	= (fe_modulation_t) scanSettings->TP_mod;

			printf("CScanTs::exec: fe(%d) freq %d rate %d fec %d mod %d\n", feindex, TP.feparams.frequency, TP.feparams.u.qam.symbol_rate, TP.feparams.u.qam.fec_inner, TP.feparams.u.qam.modulation);
		}
		else if( getFE(feindex)->getInfo()->type == FE_OFDM )
		{
			TP.feparams.u.ofdm.bandwidth =  (fe_bandwidth_t)scanSettings->TP_band;
			TP.feparams.u.ofdm.code_rate_HP = (fe_code_rate_t)scanSettings->TP_HP; 
			TP.feparams.u.ofdm.code_rate_LP = (fe_code_rate_t)scanSettings->TP_LP; 
			TP.feparams.u.ofdm.constellation = (fe_modulation_t)scanSettings->TP_const; 
			TP.feparams.u.ofdm.transmission_mode = (fe_transmit_mode_t)scanSettings->TP_trans;
			TP.feparams.u.ofdm.guard_interval = (fe_guard_interval_t)scanSettings->TP_guard;
			TP.feparams.u.ofdm.hierarchy_information = (fe_hierarchy_t)scanSettings->TP_hierarchy;

			printf("CScanTs::exec: fe(%d) freq %d band %d HP %d LP %d const %d trans %d guard %d hierarchy %d\n", feindex, TP.feparams.frequency, TP.feparams.u.ofdm.bandwidth, TP.feparams.u.ofdm.code_rate_HP, TP.feparams.u.ofdm.code_rate_LP, TP.feparams.u.ofdm.constellation, TP.feparams.u.ofdm.transmission_mode, TP.feparams.u.ofdm.guard_interval, TP.feparams.u.ofdm.hierarchy_information);
		}
	} 
	else 
	{
		for(sit = satellitePositions.begin(); sit != satellitePositions.end(); sit++) 
		{
			if(sit->second.use_in_scan) 
			{
				// pos
				sat.position = sit->first;
				// name
				strncpy(sat.satName, sit->second.name.c_str(), 50);
				
				satList.push_back(sat);
			}
		}
	}
	
	success = false;

	if(!manual) 
	{
                if (system(NEUTRINO_SCAN_START_SCRIPT) != 0)
                	perror(NEUTRINO_SCAN_START_SCRIPT " failed");
	}
	
	/*
	// send fe mode
	g_Zapit->setFEMode((fe_mode_t)scanSettings->femode, feindex);

	if( getFE(feindex)->getInfo()->type == FE_QPSK )
	{
		// send diseqc type to zapit
		diseqcType = (diseqc_t) scanSettings->diseqcMode;
		
		g_Zapit->setDiseqcType(diseqcType, feindex);
		//printf("scan.cpp send to zapit diseqctype: %d\n", diseqcType);
			
		// send diseqc repeat to zapit
		g_Zapit->setDiseqcRepeat( scanSettings->diseqcRepeat, feindex);
	}
	*/
	
	// send bouquets mode
	g_Zapit->setScanBouquetMode( (CZapitClient::bouquetMode) scanSettings->bouquetMode);

	// send satellite list to zapit
	g_Zapit->setScanSatelliteList(satList);

        // send scantype to zapit
        g_Zapit->setScanType((CZapitClient::scanType) scanSettings->scanType );
	
	paint(test);
	
	frameBuffer->blit();	

	// go
	if(test) 
	{
		int w = x + width - xpos2;
		char buffer[128];
		char * f, *s, *m;

		if( getFE(feindex)->getInfo()->type == FE_QPSK) 
		{
			getFE(feindex)->getDelSys(scanSettings->TP_fec, dvbs_get_modulation((fe_code_rate_t)scanSettings->TP_fec), f, s, m);

			sprintf(buffer, "%u %c %d %s %s %s", atoi(scanSettings->TP_freq)/1000, scanSettings->TP_pol == 0 ? 'H' : 'V', atoi(/*get_set.*/scanSettings->TP_rate)/1000, f, s, m);
		} 
		else if( getFE(feindex)->getInfo()->type == FE_QAM) 
		{
			getFE(feindex)->getDelSys(scanSettings->TP_fec, scanSettings->TP_mod, f, s, m);

			sprintf(buffer, "%u %d %s %s %s", atoi(scanSettings->TP_freq), atoi(scanSettings->TP_rate)/1000, f, s, m);
		}
		else if( getFE(feindex)->getInfo()->type == FE_OFDM)
		{
			getFE(feindex)->getDelSys(scanSettings->TP_HP, scanSettings->TP_const, f, s, m);

			sprintf(buffer, "%u %s %s %s", atoi(scanSettings->TP_freq)/1000, f, s, m);
		}

		paintLine(xpos2, ypos_cur_satellite, w - 95, scanSettings->satNameNoDiseqc);
		paintLine(xpos2, ypos_frequency, w, buffer);

		success = g_Zapit->tune_TP(TP, feindex);
	} 
	else if(manual)
		success = g_Zapit->scan_TP(TP, feindex);
	else
		success = g_Zapit->startScan(scan_mode, feindex);

	// poll for messages
	istheend = !success;
	while (!istheend) 
	{
		paintRadar();

		unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd_MS( 250 );

		do {
			g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd);

			if (test && (msg <= CRCInput::RC_MaxRC)) 
			{
				// rezap
				g_Zapit->Rezap();
				
				istheend = true;
				msg = CRCInput::RC_timeout;
			}
			else if(msg == CRCInput::RC_home) 
			{
				// dont abort scan
				if(manual && scanSettings->scan_mode)
					continue;
				
				if (MessageBox(LOCALE_SCANTS_ABORT_HEADER, LOCALE_SCANTS_ABORT_BODY, CMessageBox::mbrNo, CMessageBox::mbYes | CMessageBox::mbNo) == CMessageBox::mbrYes) 
				{
					g_Zapit->stopScan();
				}
			}
			else
				msg = handleMsg(msg, data);
		}
		while (!(msg == CRCInput::RC_timeout));
		
		showSNR(); // FIXME commented until scan slowdown will be solved
		
		frameBuffer->blit();	
	}
	
	// to join scan thread
	g_Zapit->stopScan();

	if(!manual) 
	{
                if (system(NEUTRINO_SCAN_STOP_SCRIPT) != 0)
                	perror(NEUTRINO_SCAN_STOP_SCRIPT " failed");
				
	}

	if(!test) 
	{
		const char * text = g_Locale->getText(success ? LOCALE_SCANTS_FINISHED : LOCALE_SCANTS_FAILED);
		
		// head
		frameBuffer->paintBoxRel(x, y, width, hheight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, g_settings.Head_gradient);
		
		// exit icon
		frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_HOME, &_iw, &_ih);
		frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_HOME, x + width - (BORDER_RIGHT + _iw), y + (hheight - _ih)/2);
		
		// setup icon
		frameBuffer->getIconSize(NEUTRINO_ICON_SETTINGS, &_iw, &_ih);
		frameBuffer->paintIcon(NEUTRINO_ICON_SETTINGS, x + BORDER_LEFT, y + (hheight - _ih)/2);
		
		// title
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(xpos1 + _iw + 5, y + hheight - (hheight - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2, width - BORDER_RIGHT - BORDER_LEFT - 2*_iw, text, COL_MENUHEAD, 0, true); // UTF-8
			
		frameBuffer->blit();		
	
		unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd(0xFFFF);

		do {
			g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd);
			if ( msg <= CRCInput::RC_MaxRC )
				msg = CRCInput::RC_timeout;
			else
				CNeutrinoApp::getInstance()->handleMsg( msg, data );
		} while (!(msg == CRCInput::RC_timeout));
	}

	hide();
	
	// start sectionsd
	g_Sectionsd->setPauseScanning(false);
	
#if defined (USE_OPENGL)
	g_Zapit->startPlayBack();
#endif
	
	CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);

	return menu_return::RETURN_REPAINT;
}

int CScanTs::handleMsg(neutrino_msg_t msg, neutrino_msg_data_t data)
{
	int w = x + width - xpos2;
	
	//printf("CScanTs::handleMsg: x %d xpos2 %d width %d w %d\n", x, xpos2, width, w);
	
	char buffer[128];
	char str[256];

	switch (msg) 
	{
		case NeutrinoMessages::EVT_SCAN_SATELLITE:
			paintLine(xpos2, ypos_cur_satellite, w - 100, (char *)data);
			break;
			
		case NeutrinoMessages::EVT_SCAN_NUM_TRANSPONDERS:
			sprintf(buffer, "%d", data);
			paintLine(xpos2, ypos_transponder, w - 100, buffer);
			total = data;
	
			snprintf(str, 255, "scan: %d/%d", done, total);
			CVFD::getInstance()->showMenuText(0, str, -1, true);		
			break;
			
		case NeutrinoMessages::EVT_SCAN_REPORT_NUM_SCANNED_TRANSPONDERS:
			if (total == 0) data = 0;
			done = data;
			sprintf(buffer, "%d/%d", done, total);
			paintLine(xpos2, ypos_transponder, w - 100, buffer);
	
			snprintf(str, 255, "scan %d/%d", done, total);
			CVFD::getInstance()->showMenuText(0, str, -1, true);		
			break;

		case NeutrinoMessages::EVT_SCAN_REPORT_FREQUENCY:
			freqready = 1;
			sprintf(buffer, "%u", data);
			xpos_frequency = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(buffer, true);
			paintLine(xpos2, ypos_frequency, xpos_frequency, buffer);
			break;
			
		case NeutrinoMessages::EVT_SCAN_REPORT_FREQUENCYP: 
			{
				int pol = data & 0xFF;
				int fec = (data >> 8) & 0xFF;
				int rate = data >> 16;
				char * f, *s, *m;
				
				getFE(feindex)->getDelSys(fec, (fe_modulation_t)0, f, s, m); // FIXME
				
				sprintf(buffer, " %c %d %s %s %s", pol == 0 ? 'H' : 'V', rate, f, s, m);
				
				//(pol == 0) ? sprintf(buffer, "-H") : sprintf(buffer, "-V");
				paintLine(xpos2 + xpos_frequency, ypos_frequency, w - xpos_frequency - 80, buffer);
			}
			break;
			
		case NeutrinoMessages::EVT_SCAN_PROVIDER:
			paintLine(xpos2, ypos_provider, w, (char*)data); // UTF-8
			break;
			
		case NeutrinoMessages::EVT_SCAN_SERVICENAME:
			paintLine(xpos2, ypos_channel, w, (char *)data); // UTF-8
			break;
			
		case NeutrinoMessages::EVT_SCAN_NUM_CHANNELS:
			sprintf(buffer, " = %d", data);
			paintLine(xpos1 + 3 * 72, ypos_service_numbers + mheight, width - 3 * 72 - 10, buffer);
			break;
			
		case NeutrinoMessages::EVT_SCAN_FOUND_TV_CHAN:
			sprintf(buffer, "%d", data);
			paintLine(xpos1, ypos_service_numbers + mheight, 72, buffer);
			break;
			
		case NeutrinoMessages::EVT_SCAN_FOUND_RADIO_CHAN:
			sprintf(buffer, "%d", data);
			paintLine(xpos1 + 72, ypos_service_numbers + mheight, 72, buffer);
			break;
			
		case NeutrinoMessages::EVT_SCAN_FOUND_DATA_CHAN:
			sprintf(buffer, "%d", data);
			paintLine(xpos1 + 2 * 72, ypos_service_numbers + mheight, 72, buffer);
			break;
			
		case NeutrinoMessages::EVT_SCAN_COMPLETE:
		case NeutrinoMessages::EVT_SCAN_FAILED:
			success = (msg == NeutrinoMessages::EVT_SCAN_COMPLETE);
			istheend = true;
			msg = CRCInput::RC_timeout;
			break;
			
		case CRCInput::RC_plus:
		case CRCInput::RC_minus:
		case CRCInput::RC_left:
		case CRCInput::RC_right:
			CNeutrinoApp::getInstance()->setVolume(msg, true, true);
			break;
			
		default:
			if ((msg >= CRCInput::RC_WithData) && (msg < CRCInput::RC_WithData + 0x10000000)) 
				delete (unsigned char*) data;
			break;
	}
	
	return msg;
}

void CScanTs::paintRadar(void)
{
	char filename[30];
	
	sprintf(filename, "radar%d.raw", radar);
	radar = (radar + 1) % 10;
	frameBuffer->paintIcon8(filename, xpos_radar, ypos_radar, 17);
}

void CScanTs::hide()
{
	frameBuffer->paintBackground();

	frameBuffer->blit();

	freqready = 0;
}

void CScanTs::paintLineLocale(int _x, int * _y, int _width, const neutrino_locale_t l)
{
	frameBuffer->paintBoxRel(_x, *_y, _width, mheight, COL_MENUCONTENT_PLUS_0);
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(_x, *_y + mheight, _width, g_Locale->getText(l), COL_MENUCONTENTINACTIVE, 0, true); // UTF-8
	*_y += mheight;
}

void CScanTs::paintLine(int _x, int _y, int w, const char * const txt)
{
	//printf("CScanTs::paintLine x %d y %d w %d width %d xpos2 %d: %s\n", x, y, w, width, xpos2, txt);
	
	frameBuffer->paintBoxRel(_x, _y, w, mheight, COL_MENUCONTENT_PLUS_0);
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(_x, _y + mheight, w, txt, COL_MENUCONTENT, 0, true); // UTF-8
}

void CScanTs::paint(bool fortest)
{
	int ypos;
	int iw, ih;

	ypos = y;
	
	// head 
	frameBuffer->paintBoxRel(x, ypos, width, hheight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, g_settings.Head_gradient);
	
	// icon
	frameBuffer->getIconSize(NEUTRINO_ICON_SETTINGS, &iw, &ih);
	frameBuffer->paintIcon(NEUTRINO_ICON_SETTINGS, x + BORDER_LEFT, ypos + (hheight - ih)/2);
	
	// head title
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(xpos1 + iw + 5, ypos + hheight - (hheight - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2, width, fortest ? g_Locale->getText(LOCALE_SCANTS_TEST) : g_Locale->getText(LOCALE_SCANTS_HEAD), COL_MENUHEAD, 0, true); // UTF-8
	
	// exit icon
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_HOME, &iw, &ih);
	frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_HOME, x + width - (BORDER_RIGHT + iw), ypos + (hheight - ih)/2);
	
	// main box
	frameBuffer->paintBoxRel(x, ypos + hheight, width, height - hheight, COL_MENUCONTENT_PLUS_0, RADIUS_MID, CORNER_BOTTOM);
	
	// radar
	frameBuffer->loadPal("radar.pal", 17, 37);
	
	// satellites
	ypos = y + hheight + (mheight >> 1);
	
	ypos_cur_satellite = ypos;
	

	if ( getFE(feindex)->getInfo()->type == FE_QPSK)
	{	//sat
		paintLineLocale(xpos1, &ypos, width - xpos1, LOCALE_SCANTS_ACTSATELLITE);
		xpos2 = xpos1 + 10 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(g_Locale->getText(LOCALE_SCANTS_ACTSATELLITE), true); // UTF-8
	}

	//CABLE
	else if ( getFE(feindex)->getInfo()->type == FE_QAM)
	{	//cable
		paintLineLocale(xpos1, &ypos, width - xpos1, LOCALE_SCANTS_ACTCABLE);
		xpos2 = xpos1 + 10 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(g_Locale->getText(LOCALE_SCANTS_ACTCABLE), true); // UTF-8
	}

	//DVB-T
	else if ( getFE(feindex)->getInfo()->type == FE_OFDM)
	{	//terrestrial
		paintLineLocale(xpos1, &ypos, width - xpos1, LOCALE_SCANTS_ACTTERRESTRIAL);
		xpos2 = xpos1 + 10 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(g_Locale->getText(LOCALE_SCANTS_ACTTERRESTRIAL), true); // UTF-8
	}

	// transponder
	ypos_transponder = ypos;
	paintLineLocale(xpos1, &ypos, width - xpos1, LOCALE_SCANTS_TRANSPONDERS);
	xpos2 = greater_xpos(xpos2, LOCALE_SCANTS_TRANSPONDERS);

	// frequency
	ypos_frequency = ypos;
	paintLineLocale(xpos1, &ypos, width - xpos1, LOCALE_SCANTS_FREQDATA);
	xpos2 = greater_xpos(xpos2, LOCALE_SCANTS_FREQDATA);

	// provider
	ypos += mheight >> 1; // 1/2 blank line
	
	ypos_provider = ypos;
	paintLineLocale(xpos1, &ypos, width - xpos1, LOCALE_SCANTS_PROVIDER);
	xpos2 = greater_xpos(xpos2, LOCALE_SCANTS_PROVIDER);
	
	// channel
	ypos_channel = ypos;
	paintLineLocale(xpos1, &ypos, width - xpos1, LOCALE_SCANTS_CHANNEL);
	xpos2 = greater_xpos(xpos2, LOCALE_SCANTS_CHANNEL);

	// services (tv/radio/data/summe)
	ypos += mheight >> 1; // 1/2 blank line

	ypos_service_numbers = ypos; paintLineLocale(xpos1         , &ypos, 72                 , LOCALE_SCANTS_NUMBEROFTVSERVICES   );
	ypos = ypos_service_numbers; paintLineLocale(xpos1 +     56, &ypos, 72                 , LOCALE_SCANTS_NUMBEROFRADIOSERVICES);
	ypos = ypos_service_numbers; paintLineLocale(xpos1 + 2 * 72, &ypos, 72                 , LOCALE_SCANTS_NUMBEROFDATASERVICES );
	ypos = ypos_service_numbers; paintLineLocale(xpos1 + 3 * 72, &ypos, width - 3 * 72 - 10, LOCALE_SCANTS_NUMBEROFTOTALSERVICES);
}

int CScanTs::greater_xpos(int xpos, const neutrino_locale_t txt)
{
	int txt_xpos = xpos1 + 10 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(g_Locale->getText(txt), true); // UTF-8
	if (txt_xpos > xpos)
		return txt_xpos;
	else 
		return xpos;
}

void CScanTs::showSNR()
{
	char percent[10];
	int barwidth = 150;
	uint16_t ssig, ssnr;
	int sig, snr;
	int posx, posy;
	int sw;

	ssig = getFE(feindex)->getSignalStrength();
	ssnr = getFE(feindex)->getSignalNoiseRatio();

	snr = (ssnr & 0xFFFF) * 100 / 65535;
	sig = (ssig & 0xFFFF) * 100 / 65535;

	posy = y + height - mheight - 5;

	if(sigscale->getPercent() != sig) 
	{
		posx = x + 20;
		sprintf(percent, "%d%% SIG", sig);
		sw = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth ("100% SIG");

		sigscale->paint(posx - 1, posy+2, sig);

		posx = posx + barwidth + 3;
		sw = x + 247 - posx;
		frameBuffer->paintBoxRel(posx, posy - 2, sw, mheight, COL_MENUCONTENT_PLUS_0);
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString (posx+2, posy + mheight, sw, percent, COL_MENUCONTENT);
	}

	if(snrscale->getPercent() != snr) 
	{
		posx = x + 20 + 260;
		sprintf(percent, "%d%% SNR", snr);
		sw = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth ("100% SNR");
		snrscale->paint(posx - 1, posy+2, snr);

		posx = posx + barwidth + 3;
		sw = x + width - posx;
		frameBuffer->paintBoxRel(posx, posy - 2, sw, mheight, COL_MENUCONTENT_PLUS_0);
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString (posx+2, posy + mheight, sw, percent, COL_MENUCONTENT);
	}
}
