/*
 * $Id: getservices.cpp 16.11.2020 mohousch Exp $
 *
 * (C) 2002, 2003 by Andreas Oberritter <obi@tuxbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <math.h>
#include <sys/time.h>
#include <set>
#include <sys/stat.h>
#include <sys/vfs.h>

#include <errno.h>

#include <unistd.h>

#include <fstream>
#include <iostream>

#include <global.h>

#include <bouquets.h>
#include <channel.h>
#include <frontend_c.h>
#include <getservices.h>
#include <settings.h>
#include <satconfig.h>

#include <xmlinterface.h>

#include <system/debug.h>
#include <system/helpers.h>	// needed for safe_mkdir
#include <system/settings.h>

#include <dirent.h>


extern _xmlDocPtr scanInputParser;				// defined in zapit.cpp
extern transponder_list_t transponders;				// defined in zapit.cpp // from services.xml
extern tallchans allchans;					// defined in zapit.cpp
extern int scanSDT;						// defined in zapit.cpp
static int newfound;

satellite_map_t satellitePositions;		// satellite position as specified in satellites.xml
std::map<transponder_id_t, transponder> select_transponders;	// TP map all tps from sats liste

int newtpid;
int tcnt = 0;
int scnt = 0;

extern map<t_channel_id, audio_map_set_t> audio_map;		// defined in zapit.cpp

extern int FrontendCount;
extern CFrontend * getFE(int index);

bool have_s = false;
bool have_c = false;
bool have_t = false;

extern void parseScanInputXml(fe_type_t fe_type);	// defined in zapit.cpp

void ParseTransponders(_xmlNodePtr node, t_satellite_position satellitePosition, uint8_t Source )
{
	dprintf(DEBUG_NORMAL, "[getservices] ParseTransponders:\n");

	t_transport_stream_id transport_stream_id;
	t_original_network_id original_network_id;
	FrontendParameters feparams;
	uint8_t polarization = 0;
	freq_id_t freq;
	tcnt = 0;

	memset(&feparams, 0, sizeof(FrontendParameters));

	// read all transponders
	while ((node = xmlGetNextOccurence(node, "TS")) != NULL) 
	{
		// common
		transport_stream_id = xmlGetNumericAttribute(node, "id", 16);
		original_network_id = xmlGetNumericAttribute(node, "on", 16);
		feparams.frequency = xmlGetNumericAttribute(node, "frq", 0);
		feparams.inversion = (fe_spectral_inversion) xmlGetNumericAttribute(node, "inv", 0);

		// DVB-C
		if(Source == DVB_C)
		{
			feparams.u.qam.symbol_rate = xmlGetNumericAttribute(node, "sr", 0);
			feparams.u.qam.fec_inner = (fe_code_rate_t) xmlGetNumericAttribute(node, "fec", 0);
			feparams.u.qam.modulation = (fe_modulation_t) xmlGetNumericAttribute(node, "mod", 0);
		}
		// DVB-T
		else if(Source == DVB_T)
		{
			feparams.u.ofdm.bandwidth = (fe_bandwidth_t) xmlGetNumericAttribute(node, "band", 0);
			feparams.u.ofdm.code_rate_HP = (fe_code_rate_t) xmlGetNumericAttribute(node, "HP", 0);
			feparams.u.ofdm.code_rate_LP = (fe_code_rate_t) xmlGetNumericAttribute(node, "LP", 0);
			feparams.u.ofdm.constellation = (fe_modulation_t) xmlGetNumericAttribute(node, "const", 0);
			feparams.u.ofdm.transmission_mode = (fe_transmit_mode_t) xmlGetNumericAttribute(node, "trans", 0);
			feparams.u.ofdm.guard_interval = (fe_guard_interval_t) xmlGetNumericAttribute(node, "guard", 0);
			feparams.u.ofdm.hierarchy_information = (fe_hierarchy_t) xmlGetNumericAttribute(node, "hierarchy", 0);
		}
		// DVB-S
		else if(Source == DVB_S)
		{
			feparams.u.qpsk.fec_inner = (fe_code_rate_t) xmlGetNumericAttribute(node, "fec", 0);
			feparams.u.qpsk.symbol_rate = xmlGetNumericAttribute(node, "sr", 0);
			polarization = xmlGetNumericAttribute(node, "pol", 0);

			if(feparams.u.qpsk.symbol_rate < 50000) 
				feparams.u.qpsk.symbol_rate = feparams.u.qpsk.symbol_rate * 1000;
			
			// ???
			if(feparams.frequency < 20000) 
				feparams.frequency = feparams.frequency*1000;
		}

		if(Source == DVB_C)
			freq = feparams.frequency/100;
		else if(Source == DVB_S)
			freq = feparams.frequency/1000;
		else if(Source == DVB_T)
			freq = feparams.frequency/1000000;

		// add current transponder to TP list
		transponder_id_t tid = CREATE_TRANSPONDER_ID_FROM_SATELLITEPOSITION_ORIGINALNETWORK_TRANSPORTSTREAM_ID(freq, satellitePosition, original_network_id, transport_stream_id);

		pair<map<transponder_id_t, transponder>::iterator, bool> ret;

		ret = transponders.insert (std::pair <transponder_id_t, transponder> ( tid, transponder(transport_stream_id, feparams, polarization, original_network_id)));
		
		if (ret.second == false)
			printf("[getservices] duplicate transponder id %llx freq %d\n", tid, feparams.frequency);

		// read channels that belong to the current transponder
		ParseChannels(node->xmlChildrenNode, transport_stream_id, original_network_id, satellitePosition, freq, polarization);

		// hop to next transponder
		node = node->xmlNextNode;
	}

	return;
}

void ParseChannels(_xmlNodePtr node, const t_transport_stream_id transport_stream_id, const t_original_network_id original_network_id, t_satellite_position satellitePosition, freq_id_t freq, uint8_t polarisation)
{
	dprintf(DEBUG_NORMAL, "[getservices] ParseChannels:\n");

	t_service_id service_id;
	std::string  name;
	uint8_t service_type;
	unsigned short vpid, apid, pcrpid, pmtpid, txpid, vtype, scrambled;
	std::string desc = "";
	desc += "Preset";
	t_channel_id chid;
	int dummy;
	int * have_ptr = &dummy;

	sat_iterator_t sit = satellitePositions.find(satellitePosition);

	if(sit != satellitePositions.end())
		have_ptr = &sit->second.have_channels;

	while ((node = xmlGetNextOccurence(node, "S")) != NULL) 
	{
		*have_ptr = 1;
		service_id = xmlGetNumericAttribute(node, "i", 16);
		name = xmlGetAttribute(node, "n");
		service_type = xmlGetNumericAttribute(node, "t", 16);
		vpid = xmlGetNumericAttribute(node, "v", 16);
		apid = xmlGetNumericAttribute(node, "a", 16);
		pcrpid = xmlGetNumericAttribute(node, "p", 16);
		pmtpid = xmlGetNumericAttribute(node, "pmt", 16);
		txpid = xmlGetNumericAttribute(node, "tx", 16);
		vtype = xmlGetNumericAttribute(node, "vt", 16);
		scrambled = xmlGetNumericAttribute(node, "s", 16);

		chid = CREATE_CHANNEL_ID64;
		char *ptr = xmlGetAttribute(node, "action");
		bool remove = ptr ? (!strcmp(ptr, "remove") || !strcmp(ptr, "replace")) : false;
		bool add    = ptr ? (!strcmp(ptr, "add")    || !strcmp(ptr, "replace")) : true;

		if (remove) 
		{
			int result = allchans.erase(chid);
			dprintf(DEBUG_INFO, "[getservices] %s '%s' (sid=0x%x): %s", add ? "replacing" : "removing", name.c_str(), service_id, result ? "succeded.\n" : "FAILED!\n");
		}

		if(!add) 
		{
			node = node->xmlNextNode;
			continue;
		}

		map<t_channel_id, audio_map_set_t>::iterator audio_map_it;
		audio_map_it = audio_map.find(chid);
		
		if((audio_map_it != audio_map.end()) && (audio_map_it->second.apid != 0)) 
		{
			apid = audio_map_it->second.apid;
		}

		// insert channels
		pair<map<t_channel_id, CZapitChannel>::iterator, bool> ret;

		ret = allchans.insert (std::pair <t_channel_id, CZapitChannel> (chid, CZapitChannel( name, 
												     service_id, 
												     transport_stream_id,
												     original_network_id, 
												     service_type, 
												     satellitePosition, 
												     freq)));

		if(ret.second == false) 
		{
			dprintf(DEBUG_DEBUG, "[getservices] ParseChannels: duplicate channel %s id %llx freq %d (old %s at %d)\n", name.c_str(), chid, freq, ret.first->second.getName().c_str(), ret.first->second.getFreqId());
		} 
		else 
		{
			scnt++;
			tallchans_iterator cit1 = ret.first;
			cit1->second.scrambled = scrambled;
			service_type = cit1->second.getServiceType();
			cit1->second.polarization = polarisation;

			if(pmtpid != 0 && (((service_type == ST_DIGITAL_RADIO_SOUND_SERVICE) && (apid > 0)) || ( (service_type == ST_DIGITAL_TELEVISION_SERVICE)  && (vpid > 0) && (apid > 0))) ) 
			{			
				cit1->second.setVideoPid(vpid);
				cit1->second.setAudioPid(apid);
				cit1->second.setPcrPid(pcrpid);
				cit1->second.setPmtPid(pmtpid);
				cit1->second.setTeletextPid(txpid);
				cit1->second.setPidsFlag();
				cit1->second.videoType = vtype;
			}
		}

		node = node->xmlNextNode;
	}

	return;
}

// scan services.xml
void FindTransponder(_xmlNodePtr search)
{
	dprintf(DEBUG_NORMAL, "[getservices] FindTransponder:\n");

	t_satellite_position satellitePosition = 0;
	uint8_t Source;
	newtpid = 0xC000;
	
	// frontend type
	for(int i = 0; i < FrontendCount; i++)
	{
		CFrontend * fe = getFE(i);
		
		if( fe->getDeliverySystem() == DVB_S ) 
			have_s = true;
		if( fe->getDeliverySystem() == DVB_C ) 
			have_c = true;
		if( fe->getDeliverySystem() == DVB_T ) 
			have_t = true;
	}
	
	while (search) 
	{
		if ( !(strcmp(xmlGetName(search), "cable")) && have_c)
		{
			Source = DVB_C;
			
			for (sat_iterator_t spos_it = satellitePositions.begin(); spos_it != satellitePositions.end(); spos_it++) 
			{
				if( !strcmp(spos_it->second.name.c_str(), xmlGetAttribute(search, "name")) ) 
				{
					satellitePosition = spos_it->first;
					break;
				}
			}
			
			dprintf(DEBUG_NORMAL, "[getservices] FindTransponder: going to parse dvb-%c provider %s\n", xmlGetName(search)[0], xmlGetAttribute(search, "name"));
		}
		else if ( !(strcmp(xmlGetName(search), "terrestrial")) && have_t)
		{
			Source = DVB_T;
			
			for (sat_iterator_t spos_it = satellitePositions.begin(); spos_it != satellitePositions.end(); spos_it++) 
			{
				if( !strcmp(spos_it->second.name.c_str(), xmlGetAttribute(search, "name")) ) 
				{
					satellitePosition = spos_it->first;
					break;
				}
			}
			
			dprintf(DEBUG_NORMAL, "[getservices] FindTransponder: going to parse dvb-%c provider %s\n", xmlGetName(search)[0], xmlGetAttribute(search, "name"));
		}
		else if ( !(strcmp(xmlGetName(search), "sat")) && have_s) 
		{
			Source = DVB_S;
			satellitePosition = xmlGetSignedNumericAttribute(search, "position", 10);
			
			dprintf(DEBUG_NORMAL, "[getservices] FindTransponder: going to parse dvb-%c provider %s position %d\n", xmlGetName(search)[0], xmlGetAttribute(search, "name"), satellitePosition);
		}
		else // unknow
		{
			search = search->xmlNextNode;
			continue;
		}
		
		// parse TP
		ParseTransponders(search->xmlChildrenNode, satellitePosition, Source );

		newfound++;
		
		search = search->xmlNextNode;
	}
}

// parse sat transponder from satellites/cables/terrestrials.xml
static uint32_t fake_tid, fake_nid;
void ParseSatTransponders(fe_type_t frontendType, _xmlNodePtr search, t_satellite_position satellitePosition)
{
	dprintf(DEBUG_NORMAL, "[getservices] ParseSatTransponders:\n");

	uint8_t polarization = 0;
	uint8_t system = 0;
	uint8_t modulation = 1;
	int xml_fec;
	FrontendParameters feparams;
	fake_tid = fake_nid = 0;

	_xmlNodePtr tps = search->xmlChildrenNode;

	while ((tps = xmlGetNextOccurence(tps, "transponder")) != NULL) 
	{
		memset(&feparams, 0, sizeof(FrontendParameters));

		freq_id_t freq;

		// frequency 
		//if (frontendType == FE_OFDM)
			//feparams.frequency = xmlGetNumericAttribute(tps, "centre_frequency", 0);
		//else
			feparams.frequency = xmlGetNumericAttribute(tps, "frequency", 0);

		// inversion
		feparams.inversion = INVERSION_AUTO;

		if (frontendType == FE_QAM) 		//DVB-C
		{
			feparams.u.qam.symbol_rate = xmlGetNumericAttribute(tps, "symbol_rate", 0);
			feparams.u.qam.fec_inner = (fe_code_rate_t) xmlGetNumericAttribute(tps, "fec_inner", 0);
			feparams.u.qam.modulation = (fe_modulation_t) xmlGetNumericAttribute(tps, "modulation", 0);
		}
		else if (frontendType == FE_OFDM)	//DVB-T
		{
			feparams.u.ofdm.bandwidth = (fe_bandwidth_t) xmlGetNumericAttribute(tps, "bandwidth", 0);
			feparams.u.ofdm.code_rate_HP = (fe_code_rate_t) xmlGetNumericAttribute(tps, "code_rate_hp", 0);
			feparams.u.ofdm.code_rate_LP = (fe_code_rate_t) xmlGetNumericAttribute(tps, "code_rate_lp", 0);
			feparams.u.ofdm.constellation = (fe_modulation_t) xmlGetNumericAttribute(tps, "constellation", 0);
			feparams.u.ofdm.transmission_mode = (fe_transmit_mode_t) xmlGetNumericAttribute(tps, "transmission_mode", 0);
			feparams.u.ofdm.guard_interval = (fe_guard_interval_t) xmlGetNumericAttribute(tps, "guard_interval", 0);
			feparams.u.ofdm.hierarchy_information = (fe_hierarchy_t) xmlGetNumericAttribute(tps, "hierarchy_information", 0);
			feparams.inversion = (fe_spectral_inversion_t)xmlGetNumericAttribute(tps, "inversion", 0);
		}
		else if (frontendType == FE_QPSK) 	//DVB-S
		{
			feparams.u.qpsk.symbol_rate = xmlGetNumericAttribute(tps, "symbol_rate", 0);
			polarization = xmlGetNumericAttribute(tps, "polarization", 0);
			system = xmlGetNumericAttribute(tps, "system", 0);
			modulation = xmlGetNumericAttribute(tps, "modulation", 0);
			xml_fec = xmlGetNumericAttribute(tps, "fec_inner", 0);

			xml_fec = CFrontend::getCodeRate(xml_fec, system);

			// DVB-S2
			if(modulation == 2)
				xml_fec += 9;

			feparams.u.qpsk.fec_inner = (fe_code_rate_t)xml_fec;
		}
		
		if (frontendType == FE_QAM) 
			freq = feparams.frequency/100;
		else if(frontendType == FE_QPSK)
			freq = feparams.frequency/1000;
		else if(frontendType == FE_OFDM)
			freq = feparams.frequency/1000000;
			
		transponder_id_t tid = CREATE_TRANSPONDER_ID_FROM_SATELLITEPOSITION_ORIGINALNETWORK_TRANSPORTSTREAM_ID(freq, satellitePosition, fake_nid, fake_tid);

		//polarization &= 1;
		polarization &= 7;
		
		// insert TPs list
		select_transponders.insert( std::pair <transponder_id_t, transponder> (tid, transponder(fake_tid, feparams, polarization, fake_nid)));
		
		fake_nid ++; 
		fake_tid ++;

		tps = tps->xmlNextNode;
	}
}

int loadMotorPositions(void)
{
	dprintf(DEBUG_NORMAL, "[getservices] loadMotorPositions:\n");

	FILE *fd = NULL;
	char buffer[256] = "";
	t_satellite_position satellitePosition;
	int spos = 0, mpos = 0, diseqc = 0, uncom = 0, com = 0, usals = 0, inuse;
	int offH = 10600, offL = 9750, sw = 11700;

	dprintf(DEBUG_NORMAL, "[getservices] loadingmotorpositions...\n");

	if ((fd = fopen(SATCONFIG, "r"))) 
	{
		fgets(buffer, 255, fd);

		while(!feof(fd)) 
		{
			sscanf(buffer, "%d %d %d %d %d %d %d %d %d %d", &spos, &mpos, &diseqc, &com, &uncom, &offL, &offH, &sw, &inuse, &usals);

			satellitePosition = spos;
			sat_iterator_t sit = satellitePositions.find(satellitePosition);

			if(sit != satellitePositions.end()) 
			{
				sit->second.motor_position = mpos;
				sit->second.diseqc = diseqc;
				sit->second.commited = com;
				sit->second.uncommited = uncom;
				sit->second.lnbOffsetLow = offL;
				sit->second.lnbOffsetHigh = offH;
				sit->second.lnbSwitch = sw;
				sit->second.use_in_scan = inuse;
				sit->second.use_usals = usals;
			}
			fgets(buffer, 255, fd);
		}
		fclose(fd);
	}
	else
		printf("[getservices] %s not found.\n", SATCONFIG);

	return 0;
}

void SaveMotorPositions()
{
	FILE * fd;
	sat_iterator_t sit;
	
	dprintf(DEBUG_NORMAL, "[getservices] saving motor positions...\n");

	fd = fopen(SATCONFIG, "w");
	if(fd == NULL) 
	{
		printf("[getservices] cannot open %s\n", SATCONFIG);
		return;
	}
	
	fprintf(fd, "# sat position, stored rotor, diseqc, commited, uncommited, low, high, switch, use in full scan, use usals\n");
	
	for(sit = satellitePositions.begin(); sit != satellitePositions.end(); sit++) 
	{
		if(sit->second.type == DVB_S)
			fprintf(fd, "%d %d %d %d %d %d %d %d %d %d\n", 
				sit->first, 
				sit->second.motor_position,
				sit->second.diseqc, 
				sit->second.commited, 
				sit->second.uncommited, 
				sit->second.lnbOffsetLow,
				sit->second.lnbOffsetHigh, 
				sit->second.lnbSwitch, 
				sit->second.use_in_scan, 
				sit->second.use_usals);
	
	}
	fdatasync(fileno(fd));
	fclose(fd);
}

void init_sat(t_satellite_position position)
{
	dprintf(DEBUG_NORMAL, "[getservices] init_sat:\n");

	satellitePositions[position].position = 0;
	satellitePositions[position].diseqc = -1;
	satellitePositions[position].commited = -1;
	satellitePositions[position].uncommited = -1;
	satellitePositions[position].motor_position = 0;
	satellitePositions[position].diseqc_order = 0;
	satellitePositions[position].lnbOffsetLow = 9750;
	satellitePositions[position].lnbOffsetHigh = 10600;
	satellitePositions[position].lnbSwitch = 11700;
	satellitePositions[position].use_in_scan = 0;
	satellitePositions[position].use_usals = 0;
}

// load transponders
int loadTransponders()
{
	bool satcleared = 0;
	scnt = 0;
	
	t_satellite_position position = 0; //first position

	dprintf(DEBUG_NORMAL, "[getservices] loadTransponders:\n");
	
	select_transponders.clear();
	fake_tid = fake_nid = 0;
	
	if(!satcleared)
		satellitePositions.clear();

	satcleared = 1;

	//
	fe_type_t fe_type = FE_QAM;

	// parse sat tp
	for(int i = 0; i < FrontendCount; i++)
	{
		CFrontend * fe = getFE(i);
		fe_type = fe->getInfo()->type;

		parseScanInputXml(fe_type);
			
		if ( scanInputParser != NULL ) 
		{
			_xmlNodePtr search = xmlDocGetRootElement(scanInputParser)->xmlChildrenNode;

			while (search) 
			{
				if (!(strcmp(xmlGetName(search), "sat"))) 
				{
					// position
					position = xmlGetSignedNumericAttribute(search, "position", 10);
					
					char * name = xmlGetAttribute(search, "name");

					if(satellitePositions.find(position) == satellitePositions.end()) 
					{
						init_sat(position);
					}

					// name
					satellitePositions[position].name = name;
					
					// type
					satellitePositions[position].type = DVB_S;
				}
				else if(!(strcmp(xmlGetName(search), "cable"))) 
				{
					//flags ???
					//satfeed ???
					
					char * name = xmlGetAttribute(search, "name");

					if(satellitePositions.find(position) == satellitePositions.end()) 
					{
						init_sat(position);
					}

					// name
					satellitePositions[position].name = name;
					
					// type //needed to resort list for scan menue
					satellitePositions[position].type = DVB_C;
				}
				else if(!(strcmp(xmlGetName(search), "terrestrial"))) 
				{
					char * name = xmlGetAttribute(search, "name");

					if(satellitePositions.find(position) == satellitePositions.end()) 
					{
						init_sat(position);
					}

					// name
					satellitePositions[position].name = name;
					
					// type //needed to resort list for scan menue
					satellitePositions[position].type = DVB_T;
				}

				// parse sat TP
				ParseSatTransponders(fe_type, search, position);
				
				position++;
				
				search = search->xmlNextNode;
			}
		}
	}

	// remove this
#if defined (ENABLE_FAKE_TUNER)
	have_s = true;
	have_c = true;
	have_t = true;

	for(int i = 0; i < 3; i++)
	{
		if(scanInputParser) 
		{
			delete scanInputParser;
			scanInputParser = NULL;
		}

		if (i == 0)
			scanInputParser = parseXmlFile(SATELLITES_XML);
		else if(i == 1)
			scanInputParser = parseXmlFile(CABLES_XML);
		else if(i == 2)
			scanInputParser = parseXmlFile(TERRESTRIALS_XML);			
			
		if ( scanInputParser != NULL ) 
		{
			_xmlNodePtr search = xmlDocGetRootElement(scanInputParser)->xmlChildrenNode;

			while (search) 
			{
				if (!(strcmp(xmlGetName(search), "sat"))) 
				{
					// position
					position = xmlGetSignedNumericAttribute(search, "position", 10);
					
					char * name = xmlGetAttribute(search, "name");

					if(satellitePositions.find(position) == satellitePositions.end()) 
					{
						init_sat(position);
					}

					// name
					satellitePositions[position].name = name;
					
					// type
					satellitePositions[position].type = DVB_S;
				}
				else if(!(strcmp(xmlGetName(search), "cable"))) 
				{
					//flags ???
					//satfeed ???
					
					char * name = xmlGetAttribute(search, "name");

					if(satellitePositions.find(position) == satellitePositions.end()) 
					{
						init_sat(position);
					}

					// name
					satellitePositions[position].name = name;
					
					// type //needed to resort list for scan menue
					satellitePositions[position].type = DVB_C;
				}
				else if(!(strcmp(xmlGetName(search), "terrestrial"))) 
				{
					char * name = xmlGetAttribute(search, "name");

					if(satellitePositions.find(position) == satellitePositions.end()) 
					{
						init_sat(position);
					}

					// name
					satellitePositions[position].name = name;
					
					// type //needed to resort list for scan menue
					satellitePositions[position].type = DVB_T;
				}

				// parse sat TP
				ParseSatTransponders(fe_type, search, position);
				
				position++;
				
				search = search->xmlNextNode;
			}
		}
	}
#endif
	
	return 0;
}	

// load services
int loadServices(bool only_current)
{
	_xmlDocPtr parser;
	scnt = 0;

	dprintf(DEBUG_NORMAL, "[getservices] loadServices:\n");

	if(only_current)
		goto do_current;

	// parse services.xml
	parser = parseXmlFile(SERVICES_XML);

	if (parser != NULL) 
	{
		_xmlNodePtr search = xmlDocGetRootElement(parser)->xmlChildrenNode;

		while (search) 
		{
			if (!(strcmp(xmlGetName(search), "sat"))) 
			{
				// position
				t_satellite_position position = xmlGetSignedNumericAttribute(search, "position", 10);
				char * name = xmlGetAttribute(search, "name");

				if(satellitePositions.find(position) == satellitePositions.end()) 
				{
					init_sat(position);
							
					satellitePositions[position].name = name;
				}
			}

			// jump to the next node
			search = search->xmlNextNode;
		}

		FindTransponder( xmlDocGetRootElement(parser)->xmlChildrenNode );
		
		xmlFreeDoc(parser);
	}

	// load motor position
	for(int i = 0; i < FrontendCount; i++)
	{
		if( getFE(i)->getInfo()->type == FE_QPSK)
		{
			loadMotorPositions();
			break;
		}
	}

do_current:
	dprintf(DEBUG_DEBUG, "[getservices] loadServices: loading current services\n");

	if (scanSDT && (parser = parseXmlFile(CURRENTSERVICES_XML))) 
	{
		newfound = 0;
		
		dprintf(DEBUG_INFO, "[getservices] loadServices: " CURRENTSERVICES_XML "  found.\n");
		
		FindTransponder( xmlDocGetRootElement(parser)->xmlChildrenNode );
		
		xmlFreeDoc(parser);
		unlink(CURRENTSERVICES_XML);
		
		if(newfound)
			SaveServices(true); //FIXME for second tuner
	}

	if(!only_current) 
	{
		parser = parseXmlFile(MYSERVICES_XML);
		if (parser != NULL) 
		{
			FindTransponder(xmlDocGetRootElement(parser)->xmlChildrenNode);
			
			xmlFreeDoc(parser);
		}
	}

	// webtv services
	loadWebTVBouquet();

	dprintf(DEBUG_NORMAL, "[getservices] loadServices: services loaded (%d)...\n", allchans.size());

	return 0;
}

void zapit_cp(char * from, char * to)
{
        char cmd[256] = "cp -f ";
        strcat(cmd, from);
        strcat(cmd, " ");
        strcat(cmd, to);
        system(cmd);
	sync();
}

void SaveServices(bool tocopy)
{
	transponder_id_t tpid = 0;
	FILE * fd = 0;
	bool updated = 0;

	tallchans_iterator ccI;
	tallchans_iterator dI;
	transponder_list_t::iterator tI;
	char tpstr[256];
	bool tpdone = 0;
	bool satdone = 0;
	int processed = 0;
	sat_iterator_t spos_it;
	updated = 0;

	dprintf(DEBUG_INFO, "[getservices] total channels: %d\n", allchans.size());
	
	fd = fopen(SERVICES_TMP, "w");
	if(!fd) 
	{
		perror(SERVICES_TMP);
		return;
	}

	// headers
	fprintf(fd, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<zapit api=\"3\">\n");
	
	// loop througth satpos
	for (spos_it = satellitePositions.begin(); spos_it != satellitePositions.end(); spos_it++) 
	{
		satdone = 0;

		// loop througth TPs
		for(tI = transponders.begin(); tI != transponders.end(); tI++) 
		{
			t_satellite_position satpos = GET_SATELLITEPOSITION_FROM_TRANSPONDER_ID(tI->first) & 0xFFF;
			tpdone = 0;

			if(GET_SATELLITEPOSITION_FROM_TRANSPONDER_ID(tI->first) & 0xF000)
				satpos = -satpos;

			if(satpos != spos_it->first) 
			{
				dprintf(DEBUG_DEBUG, "[getservices] Sat position %d not found !!\n", satpos);

				continue;
			}
			
			switch(spos_it->second.type)
			{
				case DVB_S:
					sprintf(tpstr, "\t\t<TS id=\"%04x\" on=\"%04x\" frq=\"%u\" inv=\"%hu\" sr=\"%u\" fec=\"%hu\" pol=\"%hu\">\n",
							tI->second.transport_stream_id, tI->second.original_network_id,
							tI->second.feparams.frequency, tI->second.feparams.inversion,
							tI->second.feparams.u.qpsk.symbol_rate, tI->second.feparams.u.qpsk.fec_inner,
							tI->second.polarization);
					break;

				case DVB_C:
					sprintf(tpstr, "\t\t<TS id=\"%04x\" on=\"%04x\" frq=\"%u\" inv=\"%hu\" sr=\"%u\" fec=\"%hu\" mod=\"%hu\">\n",
							tI->second.transport_stream_id, tI->second.original_network_id,
							tI->second.feparams.frequency, tI->second.feparams.inversion,
							tI->second.feparams.u.qam.symbol_rate, tI->second.feparams.u.qam.fec_inner,
							tI->second.feparams.u.qam.modulation);
					break;

				case DVB_T:
					sprintf(tpstr, "\t\t<TS id=\"%04x\" on=\"%04x\" frq=\"%u\" inv=\"%hu\" band=\"%hu\" HP=\"%hu\" LP=\"%hu\" const=\"%hu\" trans=\"%hu\" guard=\"%hu\" hierarchy=\"%hu\">\n",
                                        tI->second.transport_stream_id, tI->second.original_network_id,
                                        tI->second.feparams.frequency, tI->second.feparams.inversion,
                                        tI->second.feparams.u.ofdm.bandwidth, tI->second.feparams.u.ofdm.code_rate_HP,
                                        tI->second.feparams.u.ofdm.code_rate_LP, tI->second.feparams.u.ofdm.constellation,tI->second.feparams.u.ofdm.transmission_mode, tI->second.feparams.u.ofdm.guard_interval, tI->second.feparams.u.ofdm.hierarchy_information);
					break;

				default:
					break;
			}

			tpid = tI->first;
			
			// loop througth channels
			for (ccI = allchans.begin(); ccI != allchans.end(); ccI++) 
			{
				if(ccI->second.getTransponderId() == tpid) 
				{
					if(!satdone) 
					{
						switch(spos_it->second.type)
						{
							case DVB_S:
							{
								fprintf(fd, "\t<sat name=\"%s\" position=\"%hd\" diseqc=\"%hd\" uncommited=\"%hd\">\n",spos_it->second.name.c_str(), spos_it->first, spos_it->second.diseqc, spos_it->second.uncommited);
							}
							break;

							case DVB_C:
							{
								fprintf(fd, "\t<cable name=\"%s\">\n", spos_it->second.name.c_str());
							}
							break;

							case DVB_T:
							{
								fprintf(fd, "\t<terrestrial name=\"%s\">\n", spos_it->second.name.c_str());
							}
							break;

							default:
							break;
						}

						satdone = 1;
					}

					if(!tpdone) 
					{
						fprintf(fd, "%s", tpstr);
						tpdone = 1;
					}

					if(ccI->second.getPidsFlag()) 
					{
						fprintf(fd, "\t\t\t<S i=\"%04x\" n=\"%s\" v=\"%x\" a=\"%x\" p=\"%x\" pmt=\"%x\" tx=\"%x\" t=\"%x\" vt=\"%d\" s=\"%d\"/>\n",
								ccI->second.getServiceId(), 
								convert_UTF8_To_UTF8_XML(ccI->second.getName().c_str()).c_str(),
								ccI->second.getVideoPid(), 
								ccI->second.getPreAudioPid(),
								ccI->second.getPcrPid(), 
								ccI->second.getPmtPid(), 
								ccI->second.getTeletextPid(),
								ccI->second.getServiceType(true), 
								ccI->second.videoType, 
								ccI->second.scrambled);
					} 
					else 
					{
						fprintf(fd, "\t\t\t<S i=\"%04x\" n=\"%s\" t=\"%x\" s=\"%d\"/>\n",
								ccI->second.getServiceId(), 
								convert_UTF8_To_UTF8_XML(ccI->second.getName().c_str()).c_str(),
								ccI->second.getServiceType(true), 
								ccI->second.scrambled);
					}
					processed++;
				}
			}
			if(tpdone) fprintf(fd, "\t\t</TS>\n");
		}

		if(satdone) 
		{
			switch(spos_it->second.type)
			{
				case DVB_S:
					fprintf(fd, "\t</sat>\n");
					break;
					
				case DVB_C:
					fprintf(fd, "\t</cable>\n");
					break;
					
				case DVB_T:
					fprintf(fd, "\t</terrestrial>\n");
					break;
				default:
					break;
			}
		}
	}
	fprintf(fd, "</zapit>\n");
	fclose(fd);
	
	// create zapit ordner if missed
	struct stat statInfo;
	int res = stat(CONFIGDIR "/zapit", &statInfo);
	
	if (res == -1) 
	{
		if (errno == ENOENT) 
		{
			res = safe_mkdir((char *)CONFIGDIR "/zapit");

			if (res != 0) 
				perror("[getservices] mkdir");
		} 
		else 
		{
			perror("[getservices] stat");
		}
	} 

	if(tocopy) 
	{
		zapit_cp((char *) SERVICES_TMP, (char *) SERVICES_XML);
		unlink(SERVICES_TMP);
	}

	dprintf(DEBUG_INFO, "[getservices] processed channels: %d\n", processed);
}

// webtv
void parseWebTVServices(std::string filename)
{
	int cnt = 0;

	dprintf(DEBUG_NORMAL, "[getservices] parseWebTVServices: parsing %s\n", filename.c_str());

	_xmlDocPtr parser = NULL;
	
	// check for extension
	bool iptv = false;
	bool webtv = false;
	bool playlist = false;
					
	std::string extension = getFileExt(filename);
						
	if( strcasecmp("tv", extension.c_str()) == 0)
		iptv = true;
	else if( strcasecmp("m3u", extension.c_str()) == 0)
		playlist = true;
	if( strcasecmp("xml", extension.c_str()) == 0)
		webtv = true;
	
	if(iptv)
	{
		FILE * f = fopen(filename.c_str(), "r");

		std::string title;
		std::string url;
		std::string description;
		
		if(f != NULL)
		{
			while(true)
			{
				t_channel_id id = 0;

				char line[1024];
				if (!fgets(line, 1024, f))
					break;
				
				size_t len = strlen(line);
				if (len < 2)
					// Lines with less than one char aren't meaningful
					continue;
				
				// strip newline
				line[--len] = 0;
				
				// strip carriage return (when found)
				if (line[len - 1] == '\r')
					line[len - 1 ] = 0;
				
				if (strncmp(line, "#SERVICE 4097:0:1:0:0:0:0:0:0:0:", 32) == 0)
					url = line + 32;
				else if (strncmp(line, "#DESCRIPTION", 12) == 0)
				{
					int offs = line[12] == ':' ? 14 : 13;
			
					title = line + offs;
				
					description = "stream";
				
					if(!url.empty())
					{
						// grab channel id from channellist
						for (tallchans_iterator it = allchans.begin(); it != allchans.end(); it++)
						{
							if(strcasecmp(it->second.getName().c_str(), title.c_str()) == 0)
								id = it->second.getChannelID();
						}

						if(id == 0)
							id = create_channel_id64(0, 0, 0, 0, 0, url.c_str());
					
						pair<map<t_channel_id, CZapitChannel>::iterator, bool> ret;

						ret = allchans.insert (std::pair <t_channel_id, CZapitChannel> (id, CZapitChannel(title, id, url, description)));

						//ret.first->second.number = cnt + 1;
						ret.first->second.setServiceType(ST_WEBTV);
						cnt++;
					}
				}
			}
			
			fclose(f);
		}
	}
	else if(webtv)
	{
		if (parser != NULL)
		{
			xmlFreeDoc(parser);
			parser = NULL;
		}

		parser = parseXmlFile(filename.c_str());
		
		if (parser) 
		{
			_xmlNodePtr l0 = NULL;
			_xmlNodePtr l1 = NULL;
			l0 = xmlDocGetRootElement(parser);
			l1 = l0->xmlChildrenNode;
			
			if (l1) 
			{
				while ( ((xmlGetNextOccurence(l1, "webtv")) || (xmlGetNextOccurence(l1, "station")))) 
				{
					const char * title;
					const char * url;
					const char * description;
					t_channel_id id = 0;
					
					// title
					if(xmlGetNextOccurence(l1, "webtv"))
					{
						title = xmlGetAttribute(l1, (const char *)"title");
						url = xmlGetAttribute(l1, (const char *)"url");
						description = xmlGetAttribute(l1, (const char *)"description");
						const char *epgid = xmlGetAttribute(l1, "epgid");

						if (epgid)
							id = strtoull(epgid, NULL, 16);

						if(id == 0)
						{
							// grab channel id from channellist
							for (tallchans_iterator it = allchans.begin(); it != allchans.end(); it++)
							{
								if(strcasecmp(it->second.getName().c_str(), title) == 0)
									id = it->second.getChannelID();
							}
						}

						if(id == 0)
							id = create_channel_id64(0, 0, 0, 0, 0, url);
						
						pair<map<t_channel_id, CZapitChannel>::iterator, bool> ret;
						ret = allchans.insert(std::pair <t_channel_id, CZapitChannel> (id, CZapitChannel(title, id, url, description)));

						//ret.first->second.number = cnt + 1;
						ret.first->second.setServiceType(ST_WEBTV);
						cnt++;
					}

					l1 = l1->xmlNextNode;
				}
			}
		}
		
		xmlFreeDoc(parser);
		parser = NULL;
	}
	else if(playlist)
	{
		std::ifstream infile;
		char cLine[1024];
		char title[1024] = { 0 };
				
		infile.open(filename.c_str(), std::ifstream::in);

		while (infile.good())
		{
			int duration;
			std::string description;
			t_channel_id id = 0;

			infile.getline(cLine, sizeof(cLine));
					
			// remove CR
			if(cLine[strlen(cLine) - 1] == '\r')
				cLine[strlen(cLine) - 1] = 0;
					
			sscanf(cLine, "#EXTINF:%d,%[^\n]\n", &duration, title);
					
			if(strlen(cLine) > 0 && cLine[0] != '#')
			{
				char *url = NULL;
				if ((url = strstr(cLine, "http://")) || (url = strstr(cLine, "rtmp://")) || (url = strstr(cLine, "rtsp://")) || (url = strstr(cLine, "mmsh://")) ) 
				{
					if (url != NULL) 
					{
						description = "stream";

						// grab channel id from channellist
						for (tallchans_iterator it = allchans.begin(); it != allchans.end(); it++)
						{
							if(strcasecmp(it->second.getName().c_str(), title) == 0)
								id = it->second.getChannelID();
						}

						if(id == 0)
							id = create_channel_id64(0, 0, 0, 0, 0, url);
					
						pair<map<t_channel_id, CZapitChannel>::iterator, bool> ret;
						ret = allchans.insert(std::pair <t_channel_id, CZapitChannel> (id, CZapitChannel(title, id, url, description)));

						//ret.first->second.number = cnt + 1;
						ret.first->second.setServiceType(ST_WEBTV);
						cnt++;
					}
				}
			}
		}

		infile.close();
	}

	dprintf(DEBUG_NORMAL, "[getservices] parseWebTVServices: load %d WEBTV services\n", cnt);
}

#if defined (__USE_FILE_OFFSET64) || defined (_DARWIN_USE_64_BIT_INODE)
typedef struct dirent64 dirent_struct;
#define my_alphasort alphasort64
#define my_scandir scandir64
typedef struct stat64 stat_struct;
#define my_stat stat64
#define my_lstat lstat64
#else
typedef struct dirent dirent_struct;
#define my_alphasort alphasort
#define my_scandir scandir
typedef struct stat stat_struct;
#define my_stat stat
#define my_lstat lstat
#error not using 64 bit file offsets
#endif

void loadWebTVBouquet(void)
{
	dprintf(DEBUG_NORMAL, "[getservcices] loadWebTVBouquet:\n");

	std::string dirname = CONFIGDIR "/webtv";
	dirent_struct **namelist;
	int n;

	n = my_scandir(dirname.c_str(), &namelist, 0, my_alphasort);

	if (n < 0)
	{
		perror(("getServices: scandir: " + dirname).c_str());
		return;
	}
	
	for(int i = 0; i < n; i++)
	{
		std::string file;
		if( (strcmp(namelist[i]->d_name, ".") != 0) && (strcmp(namelist[i]->d_name, "..") != 0) )
		{
			// name
			file = dirname + "/" + namelist[i]->d_name;

			parseWebTVServices(file);
		}
		free(namelist[i]);
	}

	free(namelist);
}


