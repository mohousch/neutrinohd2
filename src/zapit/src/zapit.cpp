/*
 * $Id: zapit.cpp,v 1.3 2013/08/18 11:23:30 mohousch Exp $
 *
 * zapit - d-box2 linux project
 *
 * (C) 2001, 2002 by Philipp Leusmann <faralla@berlios.de>
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


// system headers
#include <csignal>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <syscall.h>

#include <pthread.h>

#include <cerrno>
#include <cstdio>
#include <cstring>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <global.h>

// tuxbox headers
#include <configfile.h>
#include <connection/basicserver.h>

// system
#include <system/debug.h>
#include <system/settings.h>

// zapit headers
#include <cam.h>
#include <client/msgtypes.h>
#include <getservices.h>
#include <pat.h>
#include <pmt.h>
#include <scan.h>
#include <settings.h>
#include <zapit.h>
#include <satconfig.h>
#include <frontend_c.h>

// libxmltree
#include <xmlinterface.h>

// libcoolstream
#include <dmx_cs.h>
#include <audio_cs.h>
#include <video_cs.h>
#if defined (ENABLE_CI)
#include <dvb-ci.h>
#endif


// opengl liveplayback
#if defined (USE_OPENGL)
int startOpenGLplayback();
void stopOpenGLplayback();
#endif

// globals 
int zapit_ready;
int abort_zapit;

// ci
#if defined (ENABLE_CI)
cDvbCi * ci;
#endif

// audio conf
map<t_channel_id, audio_map_set_t> audio_map;
map<t_channel_id, audio_map_set_t>::iterator audio_map_it;
unsigned int volume_left = 100, volume_right = 100;
int audio_mode = 0;
int def_audio_mode = 0;

// volume percent conf
#define VOLUME_DEFAULT_PCM 0
#define VOLUME_DEFAULT_AC3 25
typedef std::pair<int, int> pid_pair_t;
typedef std::pair<t_channel_id, pid_pair_t> volume_pair_t;
typedef std::multimap<t_channel_id, pid_pair_t> volume_map_t;
volume_map_t vol_map;
typedef volume_map_t::iterator volume_map_iterator_t;
typedef std::pair<volume_map_iterator_t, volume_map_iterator_t> volume_map_range_t;

int volume_percent;
extern int current_volume;
extern int current_muted;

int getPidVolume(t_channel_id channel_id, int pid, bool ac3);
void setPidVolume(t_channel_id channel_id, int pid, int percent);

void setVolumePercent(int percent);

// live/record channel id
t_channel_id live_channel_id;
t_channel_id rec_channel_id;

bool firstzap = true;
bool playing = false;
bool g_list_changed = false; 		/* flag to indicate, allchans was changed */

int change_audio_pid(uint8_t index);

// SDT
int scanSDT;
void * sdt_thread(void * arg);
void SaveServices(bool tocopy);
pthread_t tsdt;
pthread_mutex_t chan_mutex = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
bool sdt_wakeup;

// the conditional access module
CCam * cam0 = NULL;
CCam * cam1 = NULL;

// the configuration file
CConfigFile config(',', false);

// the event server
CEventServer *eventServer = NULL;

// the current channel
CZapitChannel * live_channel = NULL;

// record channel
CZapitChannel * rec_channel = NULL;

// transponder scan xml input
xmlDocPtr scanInputParser = NULL;

// bouquet manager
CBouquetManager * g_bouquetManager = NULL;

// Audio/Video Decoder
extern cVideo * videoDecoder;			// defined in video_cs.pp (libdvbapi)
extern cAudio * audioDecoder;			// defined in audio_cs.pp (libdvbapi)

// Demuxes
extern cDemux * audioDemux;			// defined in dmx_cs.pp (libdvbapi)
extern cDemux * videoDemux;			// defined in dmx_cs.pp (libdvbapi)
cDemux * pcrDemux = NULL;			// defined in dmx_cs.pp (libdvbapi)
extern cDemux * pmtDemux;			// defined in pmt.cpp

// map which stores the wanted scanned cables/satellites/terrestrials
scan_list_t scanProviders;

// zapit mode
enum {
	TV_MODE = 0x01,
	RADIO_MODE = 0x02,
	RECORD_MODE = 0x04,
};

int currentMode;
bool playbackStopForced = false;

// list of near video on demand
tallchans nvodchannels;         	// tallchans defined in "bouquets.h"
bool current_is_nvod = false;

// list of all channels (services)
tallchans allchans;             	// tallchans defined in "bouquets.h"
tallchans curchans;             	// tallchans defined in "bouquets.h"

// transponder scan
transponder_list_t transponders;
pthread_t scan_thread;
extern int found_transponders;		// defined in descriptors.cpp
extern int processed_transponders;	// defined in scan.cpp
extern int found_channels;		// defined in descriptors.cpp
extern short curr_sat;			// defined in scan.cpp
extern short scan_runs;			// defined in scan.cpp
extern short abort_scan;		// defined in scan.cpp

CZapitClient::bouquetMode bouquetMode = CZapitClient::BM_UPDATEBOUQUETS;
CZapitClient::scanType scanType = CZapitClient::ST_TVRADIO;

bool standby = true;
void * scan_transponder(void * arg);

// zapit config
bool saveLastChannel;
int lastChannelMode;
uint32_t  lastChannelRadio;
uint32_t  lastChannelTV;
bool makeRemainingChannelsBouquet;

// set/get zapit.config
void setZapitConfig(Zapit_config * Cfg);
void sendConfig(int connfd);

// pmt update filter
static int pmt_update_fd = -1;

// dvbsub
//extern int dvbsub_initialise();
extern int dvbsub_init();
extern int dvbsub_pause();
extern int dvbsub_stop();
extern int dvbsub_getpid();
//extern int dvbsub_getpid(int *pid, int *running);
//extern int dvbsub_start(int pid);
extern void dvbsub_setpid(int pid);
//extern int dvbsub_close();
//extern int dvbsub_terminate();

// tuxtxt
extern void tuxtx_stop_subtitle();
extern int tuxtx_subtitle_running(int *pid, int *page, int *running);
extern void tuxtx_set_pid(int pid, int page, const char * cc);

// multi frontend stuff
int FrontendCount = 0;
fe_map_t femap;

// frontend config
CConfigFile fe_configfile(',', false);
CFrontend * live_fe = NULL;
CFrontend * record_fe = NULL;

bool retune = false;

// variables for EN 50494 (a.k.a Unicable)
//int uni_scr = -1;	/* the unicable SCR address,     -1 == no unicable */
//int uni_qrg = 0;	/* the unicable frequency in MHz, 0 == from spec */

bool initFrontend()
{
	// scan for frontend
	int i, j;
	
	CFrontend * fe;
	int index = -1;
	
	// fill map
	for(i = 0; i < DVBADAPTER_MAX; i++)
	{
		for(j = 0; j < FRONTEND_MAX; j++)
		{
			fe = new CFrontend(j, i);
			
			if(fe->Open()) 
			{
				index++;
				femap.insert(std::pair <unsigned short, CFrontend*> (index, fe));
				
				live_fe = fe;
				
				// set it to standby
				fe->Close();
			}
			else
				delete fe;
		}
	}
	
	FrontendCount = femap.size();
	
	dprintf(DEBUG_INFO, "%s found %d frontends\n", __FUNCTION__, femap.size());
	
	if(femap.size() == 0)
		return false;
		
	return true;
}

void OpenFE()
{
	for(fe_map_iterator_t it = femap.begin(); it != femap.end(); it++) 
	{
		CFrontend * fe = it->second;
		
		fe->Open();
	}
}

void CloseFE()
{

	for(fe_map_iterator_t it = femap.begin(); it != femap.end(); it++) 
	{
		CFrontend * fe = it->second;
		
		if(!fe->locked)
			fe->Close();
	}
}

CFrontend * getFE(int index)
{
	if((unsigned int) index < femap.size())
		return femap[index];
	
	dprintf(DEBUG_INFO, "getFE: Frontend #%d not found\n", index);
	
	return NULL;
}

void setMode(fe_mode_t newmode, int feindex)
{
	// set mode
	getFE(feindex)->mode = newmode;
	
	// set not connected frontend to standby
	if( getFE(feindex)->mode == (fe_mode_t)FE_NOTCONNECTED )
		getFE(feindex)->Close();

	// set loop frontend as slave
	bool setslave = ( getFE(feindex)->mode == FE_LOOP );
	
	if(setslave)
	{
		dprintf(DEBUG_INFO, "Frontend (%d,%d) as slave: %s\n", getFE(feindex)->fe_adapter, getFE(feindex)->fenumber, setslave ? "yes" : "no");
		getFE(feindex)->setMasterSlave(setslave);
	}
}

void initTuner(CFrontend * fe)
{
	if(fe->standby)
	{
		dprintf(DEBUG_INFO, "%s Frontend (%d,%d)\n", __FUNCTION__, fe->fe_adapter, fe->fenumber);
		
		// open frontend
		fe->Open();
				
		// set loop frontend as slave 
		bool setslave = ( fe->mode == FE_LOOP );
		
		dprintf(DEBUG_INFO, "Frontend (%d,%d) as slave: %s\n", fe->fe_adapter, fe->fenumber, setslave ? "yes" : "no");
					
		if(setslave)
			fe->setMasterSlave(setslave);
		else
			fe->Init();

		// fe functions at start
		fe->setDiseqcRepeats( fe->diseqcRepeats );
		fe->setCurrentSatellitePosition( fe->lastSatellitePosition );
		//fe->setDiseqcType( fe->diseqcType );
	}
}

// compare polarization and band with fe values
bool loopCanTune(CFrontend * fe, CZapitChannel * thischannel)
{
	if(fe->getInfo()->type != FE_QPSK)
		return true;

	if(fe->tuned && (fe->getCurrentSatellitePosition() != thischannel->getSatellitePosition()))
		return false;

	bool tp_band = ((int)thischannel->getFreqId()*1000 >= fe->lnbSwitch);
	uint8_t tp_pol = thischannel->polarization & 1;
	uint8_t fe_pol = fe->getPolarization() & 1;

	dprintf(DEBUG_DEBUG, "%s fe(%d,%d): locked %d pol:band %d:%d vs %d:%d (%d:%d)\n", __FUNCTION__, fe->fe_adapter, fe->fenumber, fe->locked, fe_pol, fe->getHighBand(), tp_pol, tp_band, fe->getFrequency(), thischannel->getFreqId()*1000);
		
	if(!fe->tuned || (fe_pol == tp_pol && fe->getHighBand() == tp_band))
		return true;
	
	return false;
}

// getPreferredFrontend
CFrontend * getPreferredFrontend(CZapitChannel * thischannel)
{
	// check for frontend
	CFrontend * pref_frontend = NULL;
	
	t_satellite_position satellitePosition = thischannel->getSatellitePosition();
	sat_iterator_t sit = satellitePositions.find(satellitePosition);
	
	// get preferred frontend
	for(fe_map_iterator_t fe_it = femap.begin(); fe_it != femap.end(); fe_it++) 
	{
		CFrontend * fe = fe_it->second;
		
		dprintf(DEBUG_DEBUG, "getPreferredFrontend: fe(%d,%d): tuned:%d (locked:%d) fe_freq: %d fe_TP: %llx - chan_freq: %d chan_TP: %llx sat-position: %d sat-name:%s input-type:%d\n",
				fe->fe_adapter,
				fe->fenumber,
				fe->tuned,
				fe->locked,
				fe->getFrequency(), 
				fe->getTsidOnid(), 
				thischannel->getFreqId(), 
				thischannel->getTransponderId(), 
				satellitePosition,
				sit->second.name.c_str(),
				sit->second.type);
				
		// skip not connected frontend
		if( fe->mode == (fe_mode_t)FE_NOTCONNECTED )
			continue;

		// same tid frontend (locked)
		if(fe->locked && fe->getTsidOnid() == thischannel->getTransponderId())
		{
			pref_frontend = fe;
			break;
		}
		// first zap/record/other frontend type
		else if (sit != satellitePositions.end()) 
		{
			if( (sit->second.type == fe->getDeliverySystem()) && (!fe->locked) && ( fe->mode == (fe_mode_t)FE_SINGLE || (fe->mode == (fe_mode_t)FE_LOOP && loopCanTune(fe, thischannel)) ) )
			{
				pref_frontend = fe;
				break;
			}
		}
	}
	
	return pref_frontend;
}

// NOTE: this can be used only after we found our record_fe???
bool CanZap(CZapitChannel * thischannel)
{	
	CFrontend * fe = getPreferredFrontend(thischannel);
	return (fe != NULL);
}

CFrontend * getFrontend(CZapitChannel * thischannel)
{
	const char *FEMODE[] = {
		"SINGLE",
		"LOOP",
		"NOTCONNECTED"
	 };
	 
	// check for frontend
	CFrontend * free_frontend = NULL;
	
	t_satellite_position satellitePosition = thischannel->getSatellitePosition();
	sat_iterator_t sit = satellitePositions.find(satellitePosition);
	
	// close unused frontend
	for(fe_map_iterator_t fe_it = femap.begin(); fe_it != femap.end(); fe_it++) 
	{
		CFrontend * fe = fe_it->second;
			
		// skip tuned frontend and have same tid or same type as channel to tune
		if( (fe->tuned) && (fe->getTsidOnid() == thischannel->getTransponderId() || fe->getDeliverySystem() == sit->second.type) )
			continue;

		// close not locked tuner
		if(!fe->locked && femap.size() > 1)
			fe->Close();
	}
	
	// get preferred frontend and initialize it
	for(fe_map_iterator_t fe_it = femap.begin(); fe_it != femap.end(); fe_it++) 
	{
		CFrontend * fe = fe_it->second;
		
		dprintf(DEBUG_INFO, "getFrontend: fe(%d,%d): (%s) tuned:%d (locked:%d) fe_freq: %d fe_TP: %llx - chan_freq: %d chan_TP: %llx sat-position: %d sat-name:%s input-type:%d\n",
				fe->fe_adapter,
				fe->fenumber,
				FEMODE[fe->mode],
				fe->tuned,
				fe->locked,
				fe->getFrequency(), 
				fe->getTsidOnid(), 
				thischannel->getFreqId(), 
				thischannel->getTransponderId(), 
				satellitePosition,
				sit->second.name.c_str(),
				sit->second.type);
				
		// skip not connected frontend
		if( fe->mode == (fe_mode_t)FE_NOTCONNECTED )
			continue;
		
		// same tid
		if(fe->tuned && fe->getTsidOnid() == thischannel->getTransponderId())
		{
			free_frontend = fe;
			break;
		}
		// first zap/record/other frontend type
		else if (sit != satellitePositions.end()) 
		{
			if ( (sit->second.type == fe->getDeliverySystem()) && (!fe->locked) && ( fe->mode == (fe_mode_t)FE_SINGLE || (fe->mode == (fe_mode_t)FE_LOOP && loopCanTune(fe, thischannel)) ) )
			{
				free_frontend = fe;
				break;
			}
		}
	}
	
	if(free_frontend)
	{
		printf("%s Selected fe: (%d,%d)\n", __FUNCTION__, free_frontend->fe_adapter, free_frontend->fenumber);
		
		if(free_frontend->standby)
			initTuner(free_frontend);
		
	}
	else
		printf("%s can not get free frontend\n", __FUNCTION__);
	
	return free_frontend;
}

//
CFrontend * getRecordFrontend(CZapitChannel * thischannel)
{
	 const char *FEMODE[] = {
		"SINGLE",
		"LOOP",
		"NOTCONNECTED"
	 };
	 
	// check for frontend
	CFrontend * rec_frontend = NULL;
	
	t_satellite_position satellitePosition = thischannel->getSatellitePosition();
	sat_iterator_t sit = satellitePositions.find(satellitePosition);
	
	// get record frontend
	for(fe_map_iterator_t fe_it = femap.begin(); fe_it != femap.end(); fe_it++) 
	{
		CFrontend * fe = fe_it->second;
		
		dprintf(DEBUG_INFO, "getRecordFrontend: fe(%d,%d): (%s) tuned:%d (locked:%d) fe_freq: %d fe_TP: %llx - chan_freq: %d chan_TP: %llx sat-position: %d sat-name:%s input-type:%d\n",
				fe->fe_adapter,
				fe->fenumber,
				FEMODE[fe->mode],
				fe->tuned,
				fe->locked,
				fe->getFrequency(), 
				fe->getTsidOnid(), 
				thischannel->getFreqId(), 
				thischannel->getTransponderId(), 
				satellitePosition,
				sit->second.name.c_str(),
				sit->second.type);
				
		// skip not connected frontend
		if( fe->mode == (fe_mode_t)FE_NOTCONNECTED )
			continue;
		
		// frontend on same tid
		if( (fe->tuned) && (fe->getTsidOnid() == thischannel->getTransponderId()) )
		{
			rec_frontend = fe;
			break;
		}
		
		// second tuner (twin)
		else if (sit != satellitePositions.end()) 
		{
			bool twin = false;
			
			if( (fe->getInfo()->type == live_fe->getInfo()->type) && (fe->fenumber != live_fe->fenumber) )
				twin = true;
			
			if ( (fe->getDeliverySystem() == sit->second.type) && (twin? !fe->tuned : !fe->locked) && ( fe->mode == (fe_mode_t)FE_SINGLE || (fe->mode == (fe_mode_t)FE_LOOP && loopCanTune(fe, thischannel)) ) )
			{
				rec_frontend = fe;
				//break;
			}
		}
	}
	
	if(rec_frontend)
	{
		printf("%s Selected fe: (%d,%d)\n", __FUNCTION__, rec_frontend->fe_adapter, rec_frontend->fenumber);
		
		if(rec_frontend->standby)
			initTuner(rec_frontend);
		
	}
	else
		printf("%s can not get record frontend\n", __FUNCTION__);
	
	return rec_frontend;
}

void lockFrontend(CFrontend *fe)
{
	if(fe)
	{
		if(fe->tuned)
		      fe->locked = true;
	}
}

void unlockFrontend(CFrontend *fe)
{
	if(fe)
	{
		if(fe->locked)
		      fe->locked = false;
	}
}

// borrowed from cst neutrino-hd (femanager.cpp)
uint32_t getConfigValue(int num, const char * name, uint32_t defval)
{
	char cfg_key[81];
	sprintf(cfg_key, "fe%d_%s", num, name);
	
	return fe_configfile.getInt32(cfg_key, defval);
}

// borrowed from cst neutrino-hd (femanger.cpp)
void setConfigValue(int num, const char * name, uint32_t val)
{
	char cfg_key[81];
	
	sprintf(cfg_key, "fe%d_%s", num, name);
	fe_configfile.setInt32(cfg_key, val);
}

// save frontend config
void saveFrontendConfig(int feindex)
{
	dprintf(DEBUG_INFO, "zapit: saveFrontendConfig\n");
	
	for(feindex = 0; feindex < FrontendCount; feindex++)
	{
		// common
		setConfigValue(feindex, "mode", getFE(feindex)->mode);
			
		// sat
		if(getFE(feindex)->getInfo()->type == FE_QPSK)
		{
			setConfigValue(feindex, "lastSatellitePosition", getFE(feindex)->getCurrentSatellitePosition());
			setConfigValue(feindex, "diseqcRepeats", getFE(feindex)->getDiseqcRepeats());
			setConfigValue(feindex, "diseqcType", getFE(feindex)->getDiseqcType() );
					
			char tempd[12];
			char cfg_key[81];
				
			sprintf(tempd, "%3.6f", getFE(feindex)->gotoXXLatitude);
			sprintf(cfg_key, "fe%d_gotoXXLatitude", feindex);
			fe_configfile.setString(cfg_key, tempd );
				
			sprintf(tempd, "%3.6f", getFE(feindex)->gotoXXLongitude);
			sprintf(cfg_key, "fe%d_gotoXXLongitude", feindex);
			fe_configfile.setString(cfg_key, tempd );
				
			setConfigValue(feindex, "gotoXXLaDirection", getFE(feindex)->gotoXXLaDirection);
			setConfigValue(feindex, "gotoXXLoDirection", getFE(feindex)->gotoXXLoDirection);
			setConfigValue(feindex, "useGotoXX", getFE(feindex)->useGotoXX);
			setConfigValue(feindex, "repeatUsals", getFE(feindex)->repeatUsals);
		}
	}
	
	fe_configfile.saveConfig(FRONTEND_CONFIGFILE);
}

void loadFrontendConfig()
{
	dprintf(DEBUG_INFO, "zapit: loadFrontendConfig\n");
	
	if (!fe_configfile.loadConfig(FRONTEND_CONFIGFILE))
		printf("%s not found\n", FRONTEND_CONFIGFILE);
	
	for(fe_map_iterator_t fe_it = femap.begin(); fe_it != femap.end(); fe_it++) 
	{
		CFrontend * fe = fe_it->second;
		
		// common
		fe->mode = (fe_mode_t)getConfigValue(fe_it->first, "mode", (fe_mode_t)FE_SINGLE);
		
		// sat
		if(fe->getInfo()->type == FE_QPSK)
		{
			fe->useGotoXX = getConfigValue(fe_it->first, "useGotoXX", 0);
			
			char cfg_key[81];
			
			sprintf(cfg_key, "fe%d_gotoXXLatitude", fe_it->first );
			fe->gotoXXLatitude = strtod( fe_configfile.getString(cfg_key, "0.0").c_str(), NULL);
			
			sprintf(cfg_key, "fe%d_gotoXXLongitude", fe_it->first );
			fe->gotoXXLongitude = strtod(fe_configfile.getString(cfg_key, "0.0").c_str(), NULL);
			
			fe->gotoXXLaDirection = getConfigValue(fe_it->first, "gotoXXLaDirection", 0);
			fe->gotoXXLoDirection = getConfigValue(fe_it->first, "gotoXXLoDirection", 0);
			
			fe->repeatUsals = getConfigValue(fe_it->first, "repeatUsals", 0);
			fe->diseqcType = (diseqc_t)getConfigValue(fe_it->first, "diseqcType", (diseqc_t)NO_DISEQC);
			fe->diseqcRepeats = getConfigValue(fe_it->first, "diseqcRepeats", 0);
			fe->motorRotationSpeed = getConfigValue(fe_it->first, "motorRotationSpeed", 18); // default: 1.8 degrees per second
			
			fe->lastSatellitePosition = getConfigValue(fe_it->first, "lastSatellitePosition", 0);
		}
	}
}

void loadAudioMap()
{
	dprintf(DEBUG_INFO, "zapit:loadAudioMap\n");
	
        FILE *audio_config_file = fopen(AUDIO_CONFIG_FILE, "r");
	audio_map.clear();
	
        if (audio_config_file) 
	{
          	t_channel_id chan;
          	int apid = 0;
          	int subpid = 0;
		int ttxpid = 0, ttxpage = 0;
          	int mode = 0;
		int volume = 0;
          	char s[1000];

          	while (fgets(s, 1000, audio_config_file)) 
		{
			sscanf(s, "%llx %d %d %d %d %d %d", &chan, &apid, &mode, &volume, &subpid, &ttxpid, &ttxpage);
			
            		audio_map[chan].apid = apid;
            		audio_map[chan].subpid = subpid;
            		audio_map[chan].mode = mode;
            		audio_map[chan].volume = volume;
			audio_map[chan].ttxpid = ttxpid;
			audio_map[chan].ttxpage = ttxpage;
          	}

          	fclose(audio_config_file);
        }
}

void saveAudioMap()
{
	dprintf(DEBUG_INFO, "zapit:saveAudioMap\n");
	
	FILE *audio_config_file = fopen(AUDIO_CONFIG_FILE, "w");
        if (audio_config_file) 
	{
		dprintf(DEBUG_INFO, "[zapit] saving audio.conf \n");
			
		fprintf(audio_config_file, "# chan_id a_pid a_mode a_volume a_subpid a_txtpid a_txtpage\n");
			
		for (audio_map_it = audio_map.begin(); audio_map_it != audio_map.end(); audio_map_it++) 
		{
			fprintf(audio_config_file, "%llx %d %d %d %d %d %d\n", (uint64_t) audio_map_it->first,
                        (int) audio_map_it->second.apid, (int) audio_map_it->second.mode, (int) audio_map_it->second.volume, 
			(int) audio_map_it->second.subpid, (int) audio_map_it->second.ttxpid, (int) audio_map_it->second.ttxpage);
		}
			
		fdatasync(fileno(audio_config_file));
                fclose(audio_config_file);
        }
}

void loadVolumeMap()
{
	vol_map.clear();
	FILE * volume_config_file = fopen(VOLUME_CONFIG_FILE, "r");
	if (!volume_config_file) 
	{
		perror(VOLUME_CONFIG_FILE);
		return;
	}
	t_channel_id chan;
	int apid = 0;
	int volume = 0;
	char s[1000];
	
	while (fgets(s, 1000, volume_config_file)) 
	{
		if (sscanf(s, "%llx %d %d", &chan, &apid, &volume) == 3)
			vol_map.insert(volume_pair_t(chan, pid_pair_t(apid, volume)));
	}
	fclose(volume_config_file);
}

void saveVolumeMap()
{
	FILE * volume_config_file = fopen(VOLUME_CONFIG_FILE, "w");
	if (!volume_config_file) 
	{
		perror(VOLUME_CONFIG_FILE);
		return;
	}
	for (volume_map_iterator_t it = vol_map.begin(); it != vol_map.end(); ++it)
		fprintf(volume_config_file, "%llx %d %d\n", (uint64_t) it->first, it->second.first, it->second.second);

	fdatasync(fileno(volume_config_file));
	fclose(volume_config_file);
}

void saveZapitSettings(bool write, bool write_a)
{
	dprintf(DEBUG_INFO, "zapit:saveZapitSettings\n");
	
	// last channel
	if (live_channel) 
	{
		// now save the lowest channel number with the current channel_id
		int c = ((currentMode & RADIO_MODE) ? g_bouquetManager->radioChannelsBegin() : g_bouquetManager->tvChannelsBegin()).getLowestChannelNumberWithChannelID(live_channel->getChannelID());
		
		if (c >= 0) 
		{
			if ((currentMode & RADIO_MODE))
				lastChannelRadio = c;
			else
				lastChannelTV = c;
		}
	}

	// write zapit config
	if (write) 
	{
		dprintf(DEBUG_INFO, "[zapit]saveZapitSettings:\n");
		
		if (config.getBool("saveLastChannel", true)) 
		{
			config.setInt32("lastChannelMode", (currentMode & RADIO_MODE) ? 1 : 0);
			config.setInt32("lastChannelRadio", lastChannelRadio);
			config.setInt32("lastChannelTV", lastChannelTV);
			config.setInt64("lastChannel", live_channel_id);
		}
		
		config.setBool("makeRemainingChannelsBouquet", makeRemainingChannelsBouquet);

		config.setInt32("scanSDT", scanSDT);

		if (config.getModifiedFlag())
			config.saveConfig(ZAPIT_CONFIGFILE);

	}

	// write audio config
        if (write_a) 
	{
                // audio map
		saveAudioMap();
                
                // volume map
		saveVolumeMap();
        }
}

void loadZapitSettings()
{
	dprintf(DEBUG_INFO, "zapit:loadZapitSettings\n");
	
	if (!config.loadConfig(ZAPIT_CONFIGFILE))
		printf("%s not found\n", ZAPIT_CONFIGFILE);

	saveLastChannel = config.getBool("saveLastChannel", true);
	lastChannelMode = config.getInt32("lastChannelMode", 1);
	live_channel_id = config.getInt64("lastChannel", 0) & 0xFFFFFFFFFFFFULL;
	lastChannelRadio = config.getInt32("lastChannelRadio", 0);
	lastChannelTV = config.getInt32("lastChannelTV", 0);
	
	makeRemainingChannelsBouquet = config.getBool("makeRemainingChannelsBouquet", false);
	
	scanSDT = config.getInt32("scanSDT", 0);

	//load audio map
	loadAudioMap();
	
	// load volume map
	loadVolumeMap();
}

CZapitClient::responseGetLastChannel load_settings(void)
{
	dprintf(DEBUG_INFO, "CZapitClient::responseGetLastChannel load_settings\n");
	
	CZapitClient::responseGetLastChannel lastchannel;

	if (currentMode & RADIO_MODE)
		lastchannel.mode = 'r';
	else
		lastchannel.mode = 't';

	lastchannel.channelNumber = (currentMode & RADIO_MODE) ? lastChannelRadio : lastChannelTV;
	
	return lastchannel;
}
 
void sendCaPmtPlayBackStart(CZapitChannel * thischannel, CFrontend * fe)
{
	if(!thischannel)
		return;
	
	if(!fe)
		return;
	
	dprintf(DEBUG_NORMAL, "%s sending capmt....\n", __FUNCTION__);
	
	int demux_index = -1;
	int ca_mask = 0;
	
	// cam
	demux_index = fe->fenumber;
	
	ca_mask |= 1 << demux_index;

	if(currentMode & RECORD_MODE) 
	{
		if(rec_channel_id != live_channel_id) 
		{
			// zap from rec. channel
			cam1->setCaPmt(thischannel, thischannel->getCaPmt(), demux_index, ca_mask); //start cam1
                } 
                else 
		{
			// zap back to rec. channel
			cam0->setCaPmt(thischannel, thischannel->getCaPmt(), demux_index, ca_mask, true); // update
			cam1->sendMessage(0, 0); // stop/close
		}
	} 
	else 
	{
		cam0->setCaPmt(thischannel, thischannel->getCaPmt(), demux_index, ca_mask); //start cam0
	}	
}

void sendcapmtPlayBackStop(bool _sendPmt)
{
	dprintf(DEBUG_NORMAL, "%s sending capmtstopplayback... (sendPmt:%d)\n", __FUNCTION__, _sendPmt);
	
	if(_sendPmt) 
	{
		int demux_index = -1;
		int ca_mask = 0;
	
		if(currentMode & RECORD_MODE) 
		{
			if(record_fe != NULL)
				demux_index = record_fe->fenumber;

			ca_mask |= 1 << demux_index;

			if(live_channel_id == rec_channel_id)
				cam0->setCaPmt(rec_channel, rec_channel->getCaPmt(), demux_index, ca_mask, true); //update cam0
			else
				cam1->sendMessage(0,0); // stop cam1
		} 
		else 
		{
			cam0->sendMessage(0,0); // stop cam0
		}
	}
}

void sendCaPmtRecordStop(void)
{
	dprintf(DEBUG_NORMAL, "%s sending capmt record end....\n", __FUNCTION__);
	
	int demux_index = -1;
	int ca_mask = 0;
	
	// cam1 stop
	cam1->sendMessage(0, 0); // cam1 stop
	
	// cam0 update	
	if(live_fe != NULL)
		demux_index = live_fe->fenumber;

	ca_mask |= 1 << demux_index;
	
	if(standby)
		cam0->sendMessage(0, 0); // cam0 stop
	else if(live_channel_id == rec_channel_id) 
	{
		if(live_channel != NULL)
		{
			cam0->setCaPmt(live_channel, live_channel->getCaPmt(), demux_index, ca_mask, true); // cam0 update
#if defined (ENABLE_CI)
			if(live_fe != NULL)
				ci->SendCaPMT(NULL, live_fe->fenumber);
#endif
		}
	} 
	else 
	{
		if(live_channel != NULL)
			cam0->setCaPmt(live_channel, live_channel->getCaPmt(), demux_index, ca_mask); //cam0 start
#if defined (ENABLE_CI)
		if(rec_channel != NULL)
		{
			if(record_fe != NULL)
				ci->SendCaPMT(NULL, record_fe->fenumber);
		}
#endif
	}
	
	// ci cam
#if defined (ENABLE_CI)
	if(live_channel != NULL)
	{
		if(live_fe != NULL)
			ci->SendCaPMT(live_channel->getCaPmt(), live_fe->fenumber);
	}
#endif
}

// save pids
static void save_channel_pids(CZapitChannel * thischannel)
{
	if(thischannel == NULL)
		return;

	dprintf(DEBUG_INFO, "[zapit] saving channel (%llx), apid %x mode %d volume %d\n", thischannel->getChannelID(), thischannel->getAudioPid(), audio_mode, volume_right);
	
	audio_map[thischannel->getChannelID()].apid = thischannel->getAudioPid();
	audio_map[thischannel->getChannelID()].mode = audio_mode;
	audio_map[thischannel->getChannelID()].volume = audioDecoder->getVolume();
	audio_map[thischannel->getChannelID()].subpid = dvbsub_getpid();
	tuxtx_subtitle_running(&audio_map[thischannel->getChannelID()].ttxpid, &audio_map[thischannel->getChannelID()].ttxpage, NULL);
	
	// save pid volume
	setPidVolume(thischannel->getChannelID(), thischannel->getAudioPid(), volume_percent);
}

static CZapitChannel * find_channel_tozap(const t_channel_id channel_id, bool in_nvod)
{
	tallchans_iterator cit;
	
	if (in_nvod) //nvod
	{
		current_is_nvod = true;

		cit = nvodchannels.find(channel_id);

		if (cit == nvodchannels.end()) 
		{
			dprintf(DEBUG_INFO, "%s channel_id (%llx) not found\n", __FUNCTION__, channel_id);
			return NULL;
		}
	} 
	else 
	{
		current_is_nvod = false;

		cit = allchans.find(channel_id);

		if (cit == allchans.end()) 
		{
			// check again if we have nvod channel
			cit = nvodchannels.find(channel_id);
			if (cit == nvodchannels.end()) 
			{
				dprintf(DEBUG_INFO, "channel_id (%llx) AS NVOD not found\n", channel_id);
				return NULL;
			}
			current_is_nvod = true;
		}
	}
	
	return &cit->second;
}

static bool tune_to_channel(CFrontend * frontend, CZapitChannel * thischannel, bool &transponder_change)
{
	int waitForMotor = 0;

	transponder_change = false;
		  
	transponder_change = frontend->setInput(thischannel, current_is_nvod);
	
	if(retune)
		transponder_change = true;
	
	// drive rotor
	if(transponder_change && !current_is_nvod) 
	{
		waitForMotor = frontend->driveToSatellitePosition(thischannel->getSatellitePosition());
			
		if(waitForMotor > 0) 
		{
			dprintf(DEBUG_INFO, "[zapit] waiting %d seconds for motor to turn satellite dish.\n", waitForMotor);
			eventServer->sendEvent(CZapitClient::EVT_ZAP_MOTOR, CEventServer::INITID_ZAPIT, &waitForMotor, sizeof(waitForMotor));
				
			for(int i = 0; i < waitForMotor; i++) 
			{
				sleep(1);
					
				if(abort_zapit) 
				{
					abort_zapit = 0;
					return false;
				}
			}
		}
	}

	// tune fe (by TP change, nvod, twin_mode)
	if (transponder_change || current_is_nvod ) 
	{
		//
		if(retune)
			retune = false;
		
		if ( frontend->tuneChannel(thischannel, current_is_nvod) == false) 
		{
			return false;
		}
	}

	return true;
}

static bool parse_channel_pat_pmt(CZapitChannel * thischannel, CFrontend * fe)
{
	dprintf(DEBUG_NORMAL, "%s looking up pids for channel_id (%llx)\n", __FUNCTION__, thischannel->getChannelID());
	
	bool failed = false;
	
	// get program map table pid from program association table
	if (thischannel->getPmtPid() == 0) 
	{
		dprintf(DEBUG_NORMAL, "[zapit] no pmt pid, going to parse pat\n");	
		
		if (parse_pat(thischannel, fe) < 0)
		{
			dprintf(DEBUG_NORMAL, "[zapit] pat parsing failed\n");
			
			failed = true;
		}
	}

	// parse program map table and store pids
	if ( !failed && parse_pmt(thischannel, fe) < 0) 
	{
		dprintf(DEBUG_NORMAL, "[zapit] pmt parsing failed\n");	
		
		if (parse_pat(thischannel, fe) < 0) 
		{
			dprintf(DEBUG_NORMAL, "pat parsing failed\n");
			
			failed = true;
		}
		else if (parse_pmt(thischannel, fe) < 0) 
		{
			dprintf(DEBUG_NORMAL, "[zapit] pmt parsing failed\n");
			
			failed = true;
		}
	}
	
	return !failed;
}

static void restore_channel_pids(CZapitChannel * thischannel)
{
	audio_map_it = audio_map.find(thischannel->getChannelID());
	
	if((audio_map_it != audio_map.end()) ) 
	{
		dprintf(DEBUG_INFO, "[zapit] channel found, audio pid %x, subtitle pid %x mode %d volume %d\n", audio_map_it->second.apid, audio_map_it->second.subpid, audio_map_it->second.mode, audio_map_it->second.volume);
				
		if(thischannel->getAudioChannelCount() > 1) 
		{
			for (int i = 0; i < thischannel->getAudioChannelCount(); i++) 
			{
				if (thischannel->getAudioChannel(i)->pid == audio_map_it->second.apid ) 
				{
					dprintf(DEBUG_INFO, "Setting audio!\n");
					thischannel->setAudioChannel(i);
				}
			}
		}

		volume_left = volume_right = audio_map_it->second.volume;
		audio_mode = audio_map_it->second.mode;
		//FIXME: what shall neutrino do with saved volume (volume_left/volume_right)

		// set dvbsub pid
		dvbsub_setpid(audio_map_it->second.subpid);

		// set txtsub pid
		std::string tmplang;
		for (int i = 0 ; i < (int)thischannel->getSubtitleCount() ; ++i) 
		{
			CZapitAbsSub* s = thischannel->getChannelSub(i);
			
			if(s->pId == audio_map_it->second.ttxpid) 
			{
				tmplang = s->ISO639_language_code;
				break;
			}
		}
		
		if(tmplang.empty())
			tuxtx_set_pid(audio_map_it->second.ttxpid, audio_map_it->second.ttxpage, (char *) thischannel->getTeletextLang());
		else
			tuxtx_set_pid(audio_map_it->second.ttxpid, audio_map_it->second.ttxpage, (char *) tmplang.c_str());
	} 
	else 
	{
		if(thischannel->getAudioChannel() != NULL)
			volume_left = volume_right = (thischannel->getAudioChannel()->audioChannelType == CZapitAudioChannel::AC3)? VOLUME_DEFAULT_AC3 : VOLUME_DEFAULT_PCM;
		audio_mode = def_audio_mode;
		
		// set default tuxtxt pid
		tuxtx_set_pid(0, 0, (char *) thischannel->getTeletextLang());
	}

	// set saved volume pro pid
	if(thischannel->getAudioChannel() != NULL)
		volume_percent = getPidVolume(thischannel->getChannelID(), thischannel->getAudioPid(), thischannel->getAudioChannel()->audioChannelType == CZapitAudioChannel::AC3);
	setVolumePercent(volume_percent);
	
	//FIXME: is is muted
	if(current_muted)
		audioDecoder->SetMute(true);
}

// return 0, -1 fails
int zapit(const t_channel_id channel_id, bool in_nvod, bool forupdate = 0)
{
	bool transponder_change = false;
	tallchans_iterator cit;
	bool failed = false;
	CZapitChannel * newchannel;

	dprintf(DEBUG_NORMAL, "zapit: zapto channel id %llx nvod %d\n", channel_id, in_nvod);

	// find channel to zap
	if( (newchannel = find_channel_tozap(channel_id, in_nvod)) == NULL ) 
	{
		dprintf(DEBUG_INFO, "channel_id " PRINTF_CHANNEL_ID_TYPE " not found\n", channel_id);
		return -1;
	}
	
	// save pids
	if (!firstzap && live_channel)
		save_channel_pids(live_channel);

	// firstzap right now does nothing but control saving the audio channel
	firstzap = false;

	// stop update pmt filter
	if(!g_settings.satip_allow_satip)
	{
		pmt_stop_update_filter(&pmt_update_fd);
	}
	
	// FIXME: how to stop ci_capmt or we dont need this???
	stopPlayBack(!forupdate);

	// reset channel pids
	if(!g_settings.satip_allow_satip)
	{
		if(!forupdate && live_channel)
			live_channel->resetPids();
	}

	live_channel = newchannel;

	live_channel_id = live_channel->getChannelID();

	saveZapitSettings(false, false);
	
	// find live_fe to tune
	if(!g_settings.satip_allow_satip)
	{
		CFrontend * fe = getFrontend(live_channel);
		if(fe == NULL) 
		{
			dprintf(DEBUG_INFO, "%s can not allocate live frontend\n", __FUNCTION__);
			return -1;
		}
	
		live_fe = fe;
	
		dprintf(DEBUG_NORMAL, "%s zap to %s(%llx) fe(%d,%d)\n", __FUNCTION__, live_channel->getName().c_str(), live_channel_id, live_fe->fe_adapter, live_fe->fenumber );

		// tune live frontend
		if(!tune_to_channel(live_fe, live_channel, transponder_change))
			return -1;

		// check if nvod
		if (live_channel->getServiceType() == ST_NVOD_REFERENCE_SERVICE) 
		{
			current_is_nvod = true;
			return 0;
		}
		
		int retry = false;
	
tune_again:
		// parse pat pmt
		failed = !parse_channel_pat_pmt(live_channel, live_fe);

		if(failed && !retry)
		{
			usleep(2500);  /* give some 2500us for demuxer: borrowed from e2*/
			retry = true;
			dprintf(DEBUG_NORMAL, "[zapit] trying again\n");
			goto tune_again;
		}	

		if ((!failed) && (live_channel->getAudioPid() == 0) && (live_channel->getVideoPid() == 0)) 
		{
			dprintf(DEBUG_NORMAL, "[zapit] neither audio nor video pid found\n");
			failed = true;
		}

		/* 
		 * start sdt scan even if the service was not found in pat or pmt
		 * if the frontend did not tune, we don't get here, so this is fine 
		 */
		if (transponder_change)
			sdt_wakeup = true;

		if (failed)
			return -1;

		live_channel->getCaPmt()->ca_pmt_list_management = transponder_change ? 0x03 : 0x04;
	}

	// restore channel pids
	restore_channel_pids(live_channel);

	// start playback (live)
	startPlayBack(live_channel);

	if(!g_settings.satip_allow_satip)
	{
		// cam
		sendCaPmtPlayBackStart(live_channel, live_fe);
	
		// ci cam
#if defined (ENABLE_CI)	
		if(live_channel != NULL)
		{
			if(live_fe != NULL)
				ci->SendCaPMT(live_channel->getCaPmt(), live_fe->fenumber);
		}
#endif		
	
		// send caid
		int caid = 1;

		eventServer->sendEvent(CZapitClient::EVT_ZAP_CA_ID, CEventServer::INITID_ZAPIT, &caid, sizeof(int));

		// start pmt update filter
		pmt_set_update_filter(live_channel, &pmt_update_fd, live_fe);
	}	

	return 0;
}

int zapTo_RecordID(const t_channel_id channel_id)
{
	bool transponder_change = false;
	
	// find channel
	if((rec_channel = find_channel_tozap(channel_id, false)) == NULL) 
	{
		dprintf(DEBUG_NORMAL, "zapTo_RecordID: channel_id (%llx) not found\n", channel_id);
		return -1;
	}
	
	rec_channel_id = channel_id;
	
	// find record frontend
	CFrontend * frontend = getRecordFrontend(rec_channel);
	if(frontend == NULL) 
	{
		dprintf(DEBUG_NORMAL, "%s can not allocate record frontend\n", __FUNCTION__);
		return -1;
	}
		
	record_fe = frontend;
	
	// single/multi on the same frontend
	if(record_fe == live_fe)
	{
		if( (rec_channel_id != live_channel_id) && !SAME_TRANSPONDER(live_channel_id, rec_channel_id) )
		{
			// zap to record channel
			zapTo_ChannelID(rec_channel_id, false);
			return 0;
		}
	}
	// twin/multi other frontend as live frontend
	else
	{
		// just tune
		if(!tune_to_channel(record_fe, rec_channel, transponder_change))
			return -1;
	}
	
	// parse channel pat_pmt
	if(!parse_channel_pat_pmt(rec_channel, record_fe))
		return -1;
						
	// cam
	sendCaPmtPlayBackStart(rec_channel, record_fe);
	
	// ci cam
#if defined (ENABLE_CI)	
	if(rec_channel != NULL)
	{
		if(record_fe != NULL)
			ci->SendCaPMT(rec_channel->getCaPmt(), record_fe->fenumber);
	}
#endif		
	
	dprintf(DEBUG_NORMAL, "%s: zapped to %s (%llx) fe(%d,%d)\n", __FUNCTION__, rec_channel->getName().c_str(), rec_channel_id, record_fe->fe_adapter, record_fe->fenumber);
	
	return 0;
}

// set channel/pid volume percent, using current channel_id and pid
void setPidVolume(t_channel_id channel_id, int pid, int percent)
{
	if (!channel_id)
		channel_id = live_channel_id;

	if (!pid && (channel_id == live_channel_id) && live_channel)
		pid = live_channel->getAudioPid();

	dprintf(DEBUG_INFO, "[zapit] setPidVolume: channel %llx pid %x percent %d\n", channel_id, pid, percent);
	
	volume_map_range_t pids = vol_map.equal_range(channel_id);
	for (volume_map_iterator_t it = pids.first; it != pids.second; ++it) 
	{
		if (it->second.first == pid) 
		{
			it->second.second = percent;
			return;
		}
	}
	
	vol_map.insert(volume_pair_t(channel_id, pid_pair_t(pid, percent)));
}

// return channel/pid volume percent, using current channel_id and pid
int getPidVolume(t_channel_id channel_id, int pid, bool ac3)
{
	int percent = -1;

	if (!channel_id)
		channel_id = live_channel_id;

	if (!pid && (channel_id == live_channel_id) && live_channel)
		pid = live_channel->getAudioPid();

	volume_map_range_t pids = vol_map.equal_range(channel_id);
	for (volume_map_iterator_t it = pids.first; it != pids.second; ++it) 
	{
		if (it->second.first == pid) 
		{
			percent = it->second.second;
			break;
		}
	}
	
	if (percent < 0) 
	{
		percent = ac3 ? VOLUME_DEFAULT_AC3 : VOLUME_DEFAULT_PCM;
		if ((channel_id == live_channel_id) && live_channel) 
		{
			for (int  i = 0; i < live_channel->getAudioChannelCount(); i++) 
			{
				if (pid == live_channel->getAudioPid(i)) 
				{
					percent = live_channel->getAudioChannel(i)->audioChannelType == CZapitAudioChannel::AC3 ? VOLUME_DEFAULT_AC3 : VOLUME_DEFAULT_PCM;
					break;
				}
			}
		}
	}
	
	dprintf(DEBUG_INFO, "[zapit] getPidVolume: channel %llx pid %x percent %d\n", channel_id, pid, percent);
	
	return percent;
}

void setVolumePercent(int percent)
{
	dprintf(DEBUG_NORMAL, "[zapit] setVolumePercent: current_volume %d volume_percent %d percent %d\n", current_volume, volume_percent, percent);
		
	if (volume_percent != percent) 
		volume_percent = percent;
		
	int vol = current_volume + (current_volume*volume_percent)/100;
		
	dprintf(DEBUG_NORMAL, "[zapit] setVolumePercent: vol %d current_volume %d volume_percent %d\n", vol, current_volume, volume_percent);
		
	audioDecoder->setVolume(vol, vol);
}

int change_audio_pid(uint8_t index)
{
	if ((!audioDemux) || (!audioDecoder) || (!live_channel))
		return -1;

	//stop audio demux filter
	if (audioDemux->Stop() < 0)
		return -1;

	//stop audio playback
	if (audioDecoder->Stop() < 0)
		return -1;

	//update current channel
	live_channel->setAudioChannel(index);

	//set bypass mode
	CZapitAudioChannel * currentAudioChannel = live_channel->getAudioChannel();

	if (!currentAudioChannel) 
	{
		dprintf(DEBUG_INFO, "[zapit] change_audio_pid: No current audio live_channel\n");
		return -1;
	}
	
	//set audio pid
	if(audioDecoder)
	{
		switch (currentAudioChannel->audioChannelType) 
		{
			case CZapitAudioChannel::AC3:
#if defined (__sh__)			  
				audioDecoder->SetEncoding(AUDIO_ENCODING_AC3);
#elif defined (PLATFORM_COOLSTREAM)
				audioDecoder->SetStreamType(AUDIO_FMT_DOLBY_DIGITAL);				
#else
				audioDecoder->SetStreamType(AUDIO_STREAMTYPE_AC3);
#endif
				break;
			
			case CZapitAudioChannel::MPEG:
#if defined (__sh__)			  
				audioDecoder->SetEncoding(AUDIO_ENCODING_MPEG2);
#elif defined (PLATFORM_COOLSTREAM)
				audioDecoder->SetStreamType(AUDIO_FMT_MPEG);
#else
				audioDecoder->SetStreamType(AUDIO_STREAMTYPE_MPEG);
#endif
				break;
				
			case CZapitAudioChannel::AAC:
#if defined (__sh__)			  
				audioDecoder->SetEncoding(AUDIO_ENCODING_AAC);
#elif defined (PLATFORM_COOLSTREAM)
				audioDecoder->SetStreamType(AUDIO_FMT_AAC);
#else				
				audioDecoder->SetStreamType(AUDIO_STREAMTYPE_AAC);
#endif				
				break;
			
			case CZapitAudioChannel::AACPLUS:
#if defined (PLATFORM_COOLSTREAM)			  
				audioDecoder->SetStreamType(AUDIO_FMT_AAC_PLUS);
#elif !defined (__sh__)
				audioDecoder->SetStreamType(AUDIO_STREAMTYPE_AACPLUS);
#endif
				break;
			
			case CZapitAudioChannel::DTS:
#if defined (__sh__)			  
				audioDecoder->SetEncoding(AUDIO_ENCODING_DTS);
#elif defined (PLATFORM_COOLSTREAM)
				audioDecoder->SetStreamType(AUDIO_FMT_DTS);
#else
				audioDecoder->SetStreamType(AUDIO_STREAMTYPE_DTS);
#endif
				break;
				
			case CZapitAudioChannel::DTSHD:
#if !defined (__sh__) && !defined (PLATFORM_COOLSTREAM)
				audioDecoder->SetStreamType(AUDIO_STREAMTYPE_DTSHD);
#endif
				break;
				
			case CZapitAudioChannel::EAC3:
#if defined (__sh__)
				audioDecoder->SetEncoding(AUDIO_ENCODING_AC3);
#elif defined (PLATFORM_COOLSTREAM)
				audioDecoder->SetStreamType(AUDIO_FMT_DD_PLUS);
#else
				audioDecoder->SetStreamType(AUDIO_STREAMTYPE_EAC3);
#endif
				break;
				
			case CZapitAudioChannel::LPCM:
#if defined (__sh__)			  
				audioDecoder->SetEncoding(AUDIO_ENCODING_LPCM);
#elif !defined (PLATFORM_COOLSTREAM)
				audioDecoder->SetStreamType(AUDIO_STREAMTYPE_LPCMDVD);
#endif
				break;
				
			default:
				dprintf(DEBUG_NORMAL, "[zapit] change_audio_pid: unknown audio live_channel audiotype 0x%x\n", currentAudioChannel->audioChannelType);
				break;
		}
	}

	dprintf(DEBUG_NORMAL, "[zapit] change_audio_pid: change apid to 0x%x\n", live_channel->getAudioPid());

	//set audio-demux filter
	if (audioDemux->pesFilter( live_channel->getAudioPid() ) < 0)
		return -1;

	//start demux filter
	if (audioDemux->Start() < 0)
		return -1;
	
	// set saved volume pro pid
	volume_percent = getPidVolume(live_channel_id, live_channel->getAudioPid(), currentAudioChannel->audioChannelType == CZapitAudioChannel::AC3);
	setVolumePercent(volume_percent);
	
	//FIXME: is muted
	if(current_muted)
		audioDecoder->SetMute(true);
			
	//start audio playback
	if (audioDecoder && (audioDecoder->Start() < 0))
		return -1;

	return 0;
}

void setRadioMode(void)
{
	currentMode |= RADIO_MODE;
	currentMode &= ~TV_MODE;
}

void setTVMode(void)
{
	currentMode |= TV_MODE;
	currentMode &= ~RADIO_MODE;
}

int getMode(void)
{
	if (currentMode & TV_MODE)
		return CZapitClient::MODE_TV;

	if (currentMode & RADIO_MODE)
		return CZapitClient::MODE_RADIO;

	return 0;
}

void setRecordMode(void)
{
	if(currentMode & RECORD_MODE) 
		return;

	currentMode |= RECORD_MODE;
	
	// lock frontend
	lockFrontend(record_fe);
	 
	eventServer->sendEvent(CZapitClient::EVT_RECORDMODE_ACTIVATED, CEventServer::INITID_ZAPIT );
}

void unsetRecordMode(void)
{
	if(!(currentMode & RECORD_MODE)) 
		return;
	
	// capmt
	sendCaPmtRecordStop();
	
	rec_channel_id = 0;
	rec_channel = NULL;
	
	// unlock record frontend
	unlockFrontend(record_fe);

	// zapit mode
	currentMode &= ~RECORD_MODE;
 
	eventServer->sendEvent(CZapitClient::EVT_RECORDMODE_DEACTIVATED, CEventServer::INITID_ZAPIT );
}

int prepare_channels()
{
	live_channel = 0;
	
	// clear all channels/bouquets/TP's lists
	transponders.clear();
	g_bouquetManager->clearAll();
	allchans.clear();  				// <- this invalidates all bouquets, too!
	
	// load frontend config
	loadFrontendConfig();
        
        // load sats/tps
        loadTransponders();

	// load services
	if (loadServices(false) < 0)
	{
		dprintf(DEBUG_NORMAL, "prepare_channels: loadServices: failed\n");
		return -1;
	}

	dprintf(DEBUG_INFO, "prepare_channels: loadServices: success\n");

	// load bouquets
	g_bouquetManager->loadBouquets();		// 2004.08.02 g_bouquetManager->storeBouquets();

	return 0;
}

void parseScanInputXml(fe_type_t fe_type)
{
	if(scanInputParser) 
	{
		delete scanInputParser;
		scanInputParser = NULL;
	}
		
	switch (fe_type) 
	{
		case FE_QPSK:
			scanInputParser = parseXmlFile(SATELLITES_XML);
			break;
				
		case FE_QAM:
			scanInputParser = parseXmlFile(CABLES_XML);
			break;

		case FE_OFDM:
			scanInputParser = parseXmlFile(TERRESTRIALS_XML);
			break;
			
		default:
			dprintf(DEBUG_INFO, "[zapit] parseScanInputXml: Unknown type %d\n", fe_type);
			return;
	}
}

/*
 * return 0 on success
 * return -1 otherwise
 */
int start_scan(CZapitMessages::commandStartScan StartScan)
{
	// reread scaninputParser
        if(scanInputParser) 
	{
                delete scanInputParser;
                scanInputParser = NULL;

		CFrontend * fe = getFE(StartScan.feindex);
		parseScanInputXml(fe->getInfo()->type);

		if (!scanInputParser) 
		{
			dprintf(DEBUG_INFO, "[zapit] start_scan: scan not configured\n");
			return -1;
		}
	}

	scan_runs = 1;
	
	//stop playback
	stopPlayBack();
	
	// stop pmt update filter
        pmt_stop_update_filter(&pmt_update_fd);	

	found_transponders = 0;
	found_channels = 0;

	if (pthread_create(&scan_thread, 0, start_scanthread,  (void*)&StartScan)) 
	{
		dprintf(DEBUG_INFO, "pthread_create\n");
		scan_runs = 0;
		return -1;
	}

	return 0;
}

bool zapit_parse_command(CBasicMessage::Header &rmsg, int connfd)
{
	if ((standby) && ((rmsg.cmd != CZapitMessages::CMD_SET_VOLUME) && (rmsg.cmd != CZapitMessages::CMD_MUTE) && (rmsg.cmd != CZapitMessages::CMD_IS_TV_CHANNEL) && (rmsg.cmd != CZapitMessages::CMD_SET_STANDBY))) 
	{
		dprintf(DEBUG_DEBUG, "cmd %d in standby mode\n", rmsg.cmd);

		//return true;
	}

	switch (rmsg.cmd) 
	{
		case CZapitMessages::CMD_SHUTDOWN:
			return false;
	
		case CZapitMessages::CMD_ZAPTO:
		{
			CZapitMessages::commandZapto msgZapto;
			CBasicServer::receive_data(connfd, &msgZapto, sizeof(msgZapto)); // bouquet & channel number are already starting at 0!
			zapTo(msgZapto.bouquet, msgZapto.channel);
			break;
		}
		
		case CZapitMessages::CMD_ZAPTO_CHANNELNR: 
		{
			CZapitMessages::commandZaptoChannelNr msgZaptoChannelNr;
			CBasicServer::receive_data(connfd, &msgZaptoChannelNr, sizeof(msgZaptoChannelNr)); // bouquet & channel number are already starting at 0!
			zapTo(msgZaptoChannelNr.channel);
			break;
		}
	
		case CZapitMessages::CMD_ZAPTO_SERVICEID:
		case CZapitMessages::CMD_ZAPTO_SUBSERVICEID: 
		{
			CZapitMessages::commandZaptoServiceID msgZaptoServiceID;
			CZapitMessages::responseZapComplete msgResponseZapComplete;
			CBasicServer::receive_data(connfd, &msgZaptoServiceID, sizeof(msgZaptoServiceID));
			
			if(msgZaptoServiceID.record) 
				msgResponseZapComplete.zapStatus = zapTo_RecordID(msgZaptoServiceID.channel_id);
			else 
				msgResponseZapComplete.zapStatus = zapTo_ChannelID(msgZaptoServiceID.channel_id, (rmsg.cmd == CZapitMessages::CMD_ZAPTO_SUBSERVICEID));

			CBasicServer::send_data(connfd, &msgResponseZapComplete, sizeof(msgResponseZapComplete));
			break;
		}
	
		case CZapitMessages::CMD_ZAPTO_SERVICEID_NOWAIT:
		case CZapitMessages::CMD_ZAPTO_SUBSERVICEID_NOWAIT: 
		{
			CZapitMessages::commandZaptoServiceID msgZaptoServiceID;
			CBasicServer::receive_data(connfd, &msgZaptoServiceID, sizeof(msgZaptoServiceID));
			zapTo_ChannelID(msgZaptoServiceID.channel_id, (rmsg.cmd == CZapitMessages::CMD_ZAPTO_SUBSERVICEID_NOWAIT));
			break;
		}
		
		case CZapitMessages::CMD_GET_LAST_CHANNEL: 
		{
			CZapitClient::responseGetLastChannel responseGetLastChannel;
			responseGetLastChannel = load_settings();
			CBasicServer::send_data(connfd, &responseGetLastChannel, sizeof(responseGetLastChannel)); // bouquet & channel number are already starting at 0!
			break;
		}
		
		case CZapitMessages::CMD_GET_CURRENT_SATELLITE_POSITION: 
		{
			int32_t currentSatellitePosition = live_channel ? live_channel->getSatellitePosition() : live_fe->getCurrentSatellitePosition();
			CBasicServer::send_data(connfd, &currentSatellitePosition, sizeof(currentSatellitePosition));
			break;
		}
		
		case CZapitMessages::CMD_SET_AUDIOCHAN: 
		{
			CZapitMessages::commandSetAudioChannel msgSetAudioChannel;
			CBasicServer::receive_data(connfd, &msgSetAudioChannel, sizeof(msgSetAudioChannel));
			
			change_audio_pid(msgSetAudioChannel.channel);
			break;
		}
		
		case CZapitMessages::CMD_SET_MODE: 
		{
			CZapitMessages::commandSetMode msgSetMode;
			CBasicServer::receive_data(connfd, &msgSetMode, sizeof(msgSetMode));
			
			if (msgSetMode.mode == CZapitClient::MODE_TV)
				setTVMode();
			else if (msgSetMode.mode == CZapitClient::MODE_RADIO)
				setRadioMode();
			break;
		}
		
		case CZapitMessages::CMD_GET_MODE: 
		{
			CZapitMessages::responseGetMode msgGetMode;
			msgGetMode.mode = (CZapitClient::channelsMode) getMode();
			CBasicServer::send_data(connfd, &msgGetMode, sizeof(msgGetMode));
			break;
		}
		
		case CZapitMessages::CMD_GET_CURRENT_SERVICEID: 
		{
			CZapitMessages::responseGetCurrentServiceID msgCurrentSID;
			msgCurrentSID.channel_id = (live_channel != 0) ? live_channel->getChannelID() : 0;
			CBasicServer::send_data(connfd, &msgCurrentSID, sizeof(msgCurrentSID));
			break;
		}
		
		case CZapitMessages::CMD_GET_CURRENT_SERVICEINFO: 
		{
			CZapitClient::CCurrentServiceInfo msgCurrentServiceInfo;
			memset(&msgCurrentServiceInfo, 0, sizeof(CZapitClient::CCurrentServiceInfo));
			
			if(live_channel) 
			{
				msgCurrentServiceInfo.onid = live_channel->getOriginalNetworkId();
				msgCurrentServiceInfo.sid = live_channel->getServiceId();
				msgCurrentServiceInfo.tsid = live_channel->getTransportStreamId();
				msgCurrentServiceInfo.vpid = live_channel->getVideoPid();
				msgCurrentServiceInfo.apid = live_channel->getAudioPid();
				msgCurrentServiceInfo.vtxtpid = live_channel->getTeletextPid();
				msgCurrentServiceInfo.pmtpid = live_channel->getPmtPid();
				
				msgCurrentServiceInfo.pmt_version = (live_channel->getCaPmt() != NULL) ? live_channel->getCaPmt()->version_number : 0xff;
				
				msgCurrentServiceInfo.pcrpid = live_channel->getPcrPid();
				
				if(live_fe != NULL)
				{
					msgCurrentServiceInfo.tsfrequency = live_fe->getFrequency();
					msgCurrentServiceInfo.rate = live_fe->getRate();
					msgCurrentServiceInfo.fec = live_fe->getCFEC();
						
					if ( live_fe->getInfo()->type == FE_QPSK)
						msgCurrentServiceInfo.polarisation = live_fe->getPolarization();
					else
						msgCurrentServiceInfo.polarisation = 2;
				}
				
				msgCurrentServiceInfo.vtype = live_channel->type;
			}
			
			if(!msgCurrentServiceInfo.fec)
				msgCurrentServiceInfo.fec = (fe_code_rate)3;
			
			CBasicServer::send_data(connfd, &msgCurrentServiceInfo, sizeof(msgCurrentServiceInfo));
			break;
		}
		
		case CZapitMessages::CMD_GET_RECORD_SERVICEID: 
		{
			CZapitMessages::responseGetCurrentServiceID msgRecordSID;
			msgRecordSID.channel_id = (rec_channel != 0) ? rec_channel->getChannelID() : 0;
			CBasicServer::send_data(connfd, &msgRecordSID, sizeof(msgRecordSID));
			break;
		}
		
		case CZapitMessages::CMD_GET_RECORD_SERVICEINFO: 
		{
			CZapitClient::CCurrentServiceInfo msgRecordServiceInfo;
			memset(&msgRecordServiceInfo, 0, sizeof(CZapitClient::CCurrentServiceInfo));
			
			if(rec_channel) 
			{
				msgRecordServiceInfo.onid = rec_channel->getOriginalNetworkId();
				msgRecordServiceInfo.sid = rec_channel->getServiceId();
				msgRecordServiceInfo.tsid = rec_channel->getTransportStreamId();
				msgRecordServiceInfo.vpid = rec_channel->getVideoPid();
				msgRecordServiceInfo.apid = rec_channel->getAudioPid();
				msgRecordServiceInfo.vtxtpid = rec_channel->getTeletextPid();
				msgRecordServiceInfo.pmtpid = rec_channel->getPmtPid();
				
				msgRecordServiceInfo.pmt_version = (rec_channel->getCaPmt() != NULL) ? rec_channel->getCaPmt()->version_number : 0xff;
				
				msgRecordServiceInfo.pcrpid = rec_channel->getPcrPid();
				
				if(live_fe != NULL)
				{
					msgRecordServiceInfo.tsfrequency = live_fe->getFrequency();
					msgRecordServiceInfo.rate = live_fe->getRate();
					msgRecordServiceInfo.fec = live_fe->getCFEC();
						
					if ( live_fe->getInfo()->type == FE_QPSK)
						msgRecordServiceInfo.polarisation = live_fe->getPolarization();
					else
						msgRecordServiceInfo.polarisation = 2;
				}
				
				msgRecordServiceInfo.vtype = rec_channel->type;
			}
			
			if(!msgRecordServiceInfo.fec)
				msgRecordServiceInfo.fec = (fe_code_rate)3;
			
			CBasicServer::send_data(connfd, &msgRecordServiceInfo, sizeof(msgRecordServiceInfo));
			break;
		}
		//
		
		/* used by neutrino at start, this deliver infos only about the first tuner */
		case CZapitMessages::CMD_GET_DELIVERY_SYSTEM: 
		{
			CZapitMessages::responseDeliverySystem response;
			
			switch ( live_fe->getInfo()->type) 
			{
				case FE_QAM:
					response.system = DVB_C;
					break;

				case FE_QPSK:
					response.system = DVB_S;
					break;

				case FE_OFDM:
					response.system = DVB_T;
					break;

				default:
					dprintf(DEBUG_INFO, "Unknown type %d\n", live_fe->getInfo()->type);
					return false;
			
			}
			
			CBasicServer::send_data(connfd, &response, sizeof(response));
			break;
		}
	
		case CZapitMessages::CMD_GET_BOUQUETS: 
		{
			CZapitMessages::commandGetBouquets msgGetBouquets;
			CBasicServer::receive_data(connfd, &msgGetBouquets, sizeof(msgGetBouquets));
			sendBouquets(connfd, msgGetBouquets.emptyBouquetsToo, msgGetBouquets.mode); // bouquet & channel number are already starting at 0!
			break;
		}
		
		case CZapitMessages::CMD_GET_BOUQUET_CHANNELS: 
		{
			CZapitMessages::commandGetBouquetChannels msgGetBouquetChannels;
			CBasicServer::receive_data(connfd, &msgGetBouquetChannels, sizeof(msgGetBouquetChannels));
			sendBouquetChannels(connfd, msgGetBouquetChannels.bouquet, msgGetBouquetChannels.mode, false); // bouquet & channel number are already starting at 0!
			break;
		}
		case CZapitMessages::CMD_GET_BOUQUET_NCHANNELS: 
		{
			CZapitMessages::commandGetBouquetChannels msgGetBouquetChannels;
			CBasicServer::receive_data(connfd, &msgGetBouquetChannels, sizeof(msgGetBouquetChannels));
			sendBouquetChannels(connfd, msgGetBouquetChannels.bouquet, msgGetBouquetChannels.mode, true); // bouquet & channel number are already starting at 0!
			break;
		}
		
		case CZapitMessages::CMD_GET_CHANNELS: 
		{
			CZapitMessages::commandGetChannels msgGetChannels;
			CBasicServer::receive_data(connfd, &msgGetChannels, sizeof(msgGetChannels));
			sendChannels(connfd, msgGetChannels.mode, msgGetChannels.order); // bouquet & channel number are already starting at 0!
			break;
		}
	
		case CZapitMessages::CMD_GET_CHANNEL_NAME: 
		{
			t_channel_id requested_channel_id;
			CZapitMessages::responseGetChannelName response;
			CBasicServer::receive_data(connfd, &requested_channel_id, sizeof(requested_channel_id));
			if(requested_channel_id == 0) 
			{
				if(live_channel) 
				{
					strncpy(response.name, live_channel->getName().c_str(), CHANNEL_NAME_SIZE);
					response.name[CHANNEL_NAME_SIZE-1] = 0;
				} 
				else
					response.name[0] = 0;
			} 
			else 
			{
				tallchans_iterator it = allchans.find(requested_channel_id);
				if (it == allchans.end())
					response.name[0] = 0;
				else
					strncpy(response.name, it->second.getName().c_str(), CHANNEL_NAME_SIZE);

				response.name[CHANNEL_NAME_SIZE-1] = 0;
			}
			CBasicServer::send_data(connfd, &response, sizeof(response));
			break;
		}
		
		case CZapitMessages::CMD_IS_TV_CHANNEL: 
		{
			t_channel_id                             requested_channel_id;
			CZapitMessages::responseGeneralTrueFalse response;
			CBasicServer::receive_data(connfd, &requested_channel_id, sizeof(requested_channel_id));
			tallchans_iterator it = allchans.find(requested_channel_id);
			if (it == allchans.end()) 
			{
				it = nvodchannels.find(requested_channel_id);
				/* if in doubt (i.e. unknown channel) answer yes */
				if (it == nvodchannels.end())
					response.status = true;
				else
				/* FIXME: the following check is no even remotely accurate */
					response.status = (it->second.getServiceType() != ST_DIGITAL_RADIO_SOUND_SERVICE);
				} else
				/* FIXME: the following check is no even remotely accurate */
				response.status = (it->second.getServiceType() != ST_DIGITAL_RADIO_SOUND_SERVICE);
		
			CBasicServer::send_data(connfd, &response, sizeof(response));
			break;
		}
	
		case CZapitMessages::CMD_BQ_RESTORE: 
		{
			CZapitMessages::responseCmd response;
			//2004.08.02 g_bouquetManager->restoreBouquets();
			if(g_list_changed) 
			{
				prepare_channels();
				
				g_list_changed = 0;
			} 
			else 
			{
				g_bouquetManager->clearAll();
				g_bouquetManager->loadBouquets();
			}
			response.cmd = CZapitMessages::CMD_READY;
			CBasicServer::send_data(connfd, &response, sizeof(response));
			break;
		}
		
		case CZapitMessages::CMD_REINIT_CHANNELS: 
		{
			CZapitMessages::responseCmd response;
			// Houdini: save actual channel to restore it later, old version's channel was set to scans.conf initial channel
			t_channel_id cid = live_channel ? live_channel->getChannelID() : 0; 
	
			prepare_channels();
			
			tallchans_iterator cit = allchans.find(cid);
			if (cit != allchans.end()) 
				live_channel = &(cit->second); 
	
			response.cmd = CZapitMessages::CMD_READY;
			CBasicServer::send_data(connfd, &response, sizeof(response));
			eventServer->sendEvent(CZapitClient::EVT_SERVICES_CHANGED, CEventServer::INITID_ZAPIT);
			break;
		}
	
		case CZapitMessages::CMD_RELOAD_CURRENTSERVICES: 
		{
			CZapitMessages::responseCmd response;
			response.cmd = CZapitMessages::CMD_READY;
			CBasicServer::send_data(connfd, &response, sizeof(response));
			
			//live_fe->setTsidOnid(0);
			zapit(live_channel_id, current_is_nvod);

			eventServer->sendEvent(CZapitClient::EVT_BOUQUETS_CHANGED, CEventServer::INITID_ZAPIT);
			break;
		}
		case CZapitMessages::CMD_SCANSTART: 
		{
			CZapitMessages::commandStartScan StartScan;
			CBasicServer::receive_data(connfd, &StartScan, sizeof(StartScan));
			
			int scan_mode = StartScan.scan_mode;
			
			printf("[zapit] CMD_SCANSTART: fe(%d) scan_mode: %d\n", StartScan.feindex, scan_mode);
	
			// start scan thread
			if(start_scan(StartScan) == -1)
				eventServer->sendEvent(CZapitClient::EVT_SCAN_FAILED, CEventServer::INITID_ZAPIT);
			
			retune = true;
	
			break;
		}
		case CZapitMessages::CMD_SCANSTOP: 
		{
			if(scan_runs) 
			{
				abort_scan = 1;
				pthread_join(scan_thread, NULL);
				abort_scan = 0;
				scan_runs = 0;
			}
			break;
		}
	
		case CZapitMessages::CMD_SETCONFIG:
			Zapit_config Cfg;
			CBasicServer::receive_data(connfd, &Cfg, sizeof(Cfg));
			setZapitConfig(&Cfg);
			break;
	
		case CZapitMessages::CMD_GETCONFIG:
			sendConfig(connfd);
			break;
	
		case CZapitMessages::CMD_REZAP:
			if (currentMode & RECORD_MODE)
				break;
			
			if(live_channel)
				zapit(live_channel->getChannelID(), current_is_nvod);
			break;
	
		case CZapitMessages::CMD_TUNE_TP: 
		{
			CZapitMessages::commandTuneTP TuneTP;
			CBasicServer::receive_data(connfd, &TuneTP, sizeof(TuneTP));
			
			initTuner(getFE(TuneTP.feindex));
			
			// inversion
			TuneTP.TP.feparams.inversion = INVERSION_AUTO;
			
			// satname
			const char *name = scanProviders.size() > 0  ? scanProviders.begin()->second.c_str() : "unknown";
			
			t_satellite_position satellitePosition = scanProviders.begin()->first;
	
			// tune
			getFE(TuneTP.feindex)->setInput(satellitePosition, TuneTP.TP.feparams.frequency, TuneTP.TP.polarization);
					
			switch ( getFE(TuneTP.feindex)->getInfo()->type) 
			{
				case FE_QPSK:
				{
					dprintf(DEBUG_INFO, "[zapit] tune to sat %s freq %d rate %d fec %d pol %d\n", name, TuneTP.TP.feparams.frequency, TuneTP.TP.feparams.u.qpsk.symbol_rate, TuneTP.TP.feparams.u.qpsk.fec_inner, TuneTP.TP.polarization);
					live_fe->driveToSatellitePosition(satellitePosition);
					break;
				}
		
				case FE_QAM:
					dprintf(DEBUG_INFO, "[zapit] tune to cable %s freq %d rate %d fec %d\n", name, TuneTP.TP.feparams.frequency * 1000, TuneTP.TP.feparams.u.qam.symbol_rate, TuneTP.TP.feparams.u.qam.fec_inner);
		
					break;
		
				case FE_OFDM:
					dprintf(DEBUG_INFO, "[zapit] tune to terrestrial %s freq %d band %d HP %d LP %d const %d transmission_mode %d guard_interval %d hierarchy_infomation %d\n", name, TuneTP.TP.feparams.frequency * 1000, TuneTP.TP.feparams.u.ofdm.bandwidth, TuneTP.TP.feparams.u.ofdm.code_rate_HP, TuneTP.TP.feparams.u.ofdm.code_rate_LP, TuneTP.TP.feparams.u.ofdm.constellation, TuneTP.TP.feparams.u.ofdm.transmission_mode, TuneTP.TP.feparams.u.ofdm.guard_interval, TuneTP.TP.feparams.u.ofdm.hierarchy_information);
		
					break;
		
				default:
					dprintf(DEBUG_INFO, "Unknown type %d\n", live_fe->getInfo()->type);
					return false;
			}
		
			// tune it
			getFE(TuneTP.feindex)->tuneFrequency(&TuneTP.TP.feparams, TuneTP.TP.polarization, true);
			
			// set retune flag
			retune = true;
		}
		break;
	
		case CZapitMessages::CMD_SCAN_TP: 
		{
			CZapitMessages::commandScanTP ScanTP;
			CBasicServer::receive_data(connfd, &ScanTP, sizeof(ScanTP));
			
			if(!(ScanTP.TP.feparams.frequency > 0) && live_channel) 
			{
				// TP
				transponder_list_t::iterator transponder = transponders.find(live_channel->getTransponderId());
	
				// freq
				ScanTP.TP.feparams.frequency = transponder->second.feparams.frequency;
				
				switch ( getFE(ScanTP.feindex)->getInfo()->type) 
				{
					case FE_QPSK:
						ScanTP.TP.feparams.u.qpsk.symbol_rate = transponder->second.feparams.u.qpsk.symbol_rate;
						ScanTP.TP.feparams.u.qpsk.fec_inner = transponder->second.feparams.u.qpsk.fec_inner;
						ScanTP.TP.polarization = transponder->second.polarization;
						break;

					case FE_QAM:
						ScanTP.TP.feparams.u.qam.symbol_rate = transponder->second.feparams.u.qam.symbol_rate;
						ScanTP.TP.feparams.u.qam.fec_inner = transponder->second.feparams.u.qam.fec_inner;
						ScanTP.TP.feparams.u.qam.modulation = transponder->second.feparams.u.qam.modulation;
						break;

					case FE_OFDM:
						ScanTP.TP.feparams.u.ofdm.bandwidth =  transponder->second.feparams.u.ofdm.bandwidth;
						ScanTP.TP.feparams.u.ofdm.code_rate_HP = transponder->second.feparams.u.ofdm.code_rate_HP; 
						ScanTP.TP.feparams.u.ofdm.code_rate_LP = transponder->second.feparams.u.ofdm.code_rate_LP; 
						ScanTP.TP.feparams.u.ofdm.constellation = transponder->second.feparams.u.ofdm.constellation;
						ScanTP.TP.feparams.u.ofdm.transmission_mode = transponder->second.feparams.u.ofdm.transmission_mode;
						ScanTP.TP.feparams.u.ofdm.guard_interval = transponder->second.feparams.u.ofdm.guard_interval;
						ScanTP.TP.feparams.u.ofdm.hierarchy_information = transponder->second.feparams.u.ofdm.hierarchy_information;
						break;

					default:
						dprintf(DEBUG_INFO, "Unknown type %d\n", getFE(ScanTP.feindex)->getInfo()->type);
						return false;
				}
	
				if(scanProviders.size() > 0)
					scanProviders.clear();
			}
	
			stopPlayBack();
				
			// stop update pmt filter
			pmt_stop_update_filter(&pmt_update_fd);
	
			scan_runs = 1;
	
			if (pthread_create(&scan_thread, 0, scan_transponder, (void*) &ScanTP)) 
			{
				dprintf(DEBUG_INFO, "pthread_create\n");
				scan_runs = 0;
			} 
			
			retune = true;
	
			break;
		}
	
		case CZapitMessages::CMD_SCANREADY: 
		{
			CZapitMessages::responseIsScanReady msgResponseIsScanReady;
			msgResponseIsScanReady.satellite = curr_sat;
			msgResponseIsScanReady.transponder = found_transponders;
			msgResponseIsScanReady.processed_transponder = processed_transponders;
			msgResponseIsScanReady.services = found_channels;
	
			if (scan_runs > 0)
			{
				msgResponseIsScanReady.scanReady = false;
				dprintf(DEBUG_INFO, "[] scan not ready\n");
			}
			else
			{
				msgResponseIsScanReady.scanReady = true;
				dprintf(DEBUG_INFO, "[] scan ready\n");
			}
			
			CBasicServer::send_data(connfd, &msgResponseIsScanReady, sizeof(msgResponseIsScanReady));
			break;
		}
	
		case CZapitMessages::CMD_SCANGETSATLIST: 
		{
			uint32_t  satlength;
			CZapitClient::responseGetSatelliteList sat;
			satlength = sizeof(sat);
	
			sat_iterator_t sit;
			for(sit = satellitePositions.begin(); sit != satellitePositions.end(); sit++) 
			{
				strncpy(sat.satName, sit->second.name.c_str(), 50);
				sat.satName[49] = 0;
				sat.satPosition = sit->first;
				sat.motorPosition = sit->second.motor_position;

				sat.type = sit->second.type;
				
				CBasicServer::send_data(connfd, &satlength, sizeof(satlength));
				CBasicServer::send_data(connfd, (char *)&sat, satlength);
			}
			satlength = SATNAMES_END_MARKER;
			CBasicServer::send_data(connfd, &satlength, sizeof(satlength));
			break;
		}
	
		case CZapitMessages::CMD_SCANSETSCANSATLIST: 
		{
			CZapitClient::commandSetScanSatelliteList sat;
			scanProviders.clear();
			
			while (CBasicServer::receive_data(connfd, &sat, sizeof(sat))) 
			{
				//printf("[zapit] adding to scan %s (position %d) fe(%d)\n", sat.satName, sat.position, sat.feindex);
				scanProviders[sat.position] = sat.satName;
			}
			break;
		}
		
		case CZapitMessages::CMD_SCANSETSCANMOTORPOSLIST: 
		{
#if 0 // absolute
			CZapitClient::commandSetScanMotorPosList pos;
			bool changed = false;
			while (CBasicServer::receive_data(connfd, &pos, sizeof(pos))) 
			{
				//printf("adding %d (motorPos %d)\n", pos.satPosition, pos.motorPos);
				changed |= (motorPositions[pos.satPosition] != pos.motorPos);
				motorPositions[pos.satPosition] = pos.motorPos;
			}
			
			if (changed) 
				SaveMotorPositions();
#endif
			break;
		}
		
		case CZapitMessages::CMD_SCANSETDISEQCTYPE: 
		{
			CZapitMessages::commandSetDiseqcType msgSetDiseqcType;
			CBasicServer::receive_data(connfd, &msgSetDiseqcType, sizeof(msgSetDiseqcType)); // bouquet & channel number are already starting at 0!
			
			// fe set diseqc type
			if( getFE(msgSetDiseqcType.feindex)->getInfo()->type == FE_QPSK)
			{
				getFE(msgSetDiseqcType.feindex)->setDiseqcType(msgSetDiseqcType.diseqc );
			}
			
			break;
		}
		
		case CZapitMessages::CMD_SCANSETDISEQCREPEAT: 
		{
			uint32_t  repeats;

			CZapitMessages::commandSetDiseqcRepeat msgSetDiseqcRepeat;
			CBasicServer::receive_data(connfd, &msgSetDiseqcRepeat, sizeof(msgSetDiseqcRepeat));
			
			repeats = msgSetDiseqcRepeat.repeat;
			
			if( getFE(msgSetDiseqcRepeat.feindex)->getInfo()->type == FE_QPSK)
			{
				getFE(msgSetDiseqcRepeat.feindex)->setDiseqcRepeats(repeats);
				dprintf(DEBUG_INFO, "set diseqc repeats to %d", repeats);
			}
			
			break;
		}
		
		case CZapitMessages::CMD_SCANSETBOUQUETMODE:
			CBasicServer::receive_data(connfd, &bouquetMode, sizeof(bouquetMode));
			break;
	
		case CZapitMessages::CMD_SCANSETTYPE:
			CBasicServer::receive_data(connfd, &scanType, sizeof(scanType));
			break;
			
		case CZapitMessages::CMD_SCANSETFEMODE: 
		{
			CZapitMessages::commandSetFEMode msgSetFEMode;
			CBasicServer::receive_data(connfd, &msgSetFEMode, sizeof(msgSetFEMode));
			
			// fe set femode
			//getFE(msgSetFEMode.feindex)->mode = msgSetFEMode.mode;
			setMode(msgSetFEMode.mode, msgSetFEMode.feindex);
			//saveFrontendConfig();
			
			break;
		}
		
		case CZapitMessages::CMD_SET_RECORD_MODE: 
		{
			CZapitMessages::commandSetRecordMode msgSetRecordMode;
			CBasicServer::receive_data(connfd, &msgSetRecordMode, sizeof(msgSetRecordMode));
			dprintf(DEBUG_INFO, "[zapit] recording mode: %d\n", msgSetRecordMode.activate);
			if (msgSetRecordMode.activate)
				setRecordMode();
			else
				unsetRecordMode();
			break;
		}
	
		case CZapitMessages::CMD_GET_RECORD_MODE: 
		{
			CZapitMessages::responseGetRecordModeState msgGetRecordModeState;
			msgGetRecordModeState.activated = (currentMode & RECORD_MODE);
			CBasicServer::send_data(connfd, &msgGetRecordModeState, sizeof(msgGetRecordModeState));
			break;
		}
		
		case CZapitMessages::CMD_SB_GET_PLAYBACK_ACTIVE: 
		{
			CZapitMessages::responseGetPlaybackState msgGetPlaybackState;
			msgGetPlaybackState.activated = playing;

			CBasicServer::send_data(connfd, &msgGetPlaybackState, sizeof(msgGetPlaybackState));
			break;
		}
	
		case CZapitMessages::CMD_BQ_ADD_BOUQUET: 
		{
			char * name = CBasicServer::receive_string(connfd);
			g_bouquetManager->addBouquet(name, true);
			CBasicServer::delete_string(name);
			break;
		}
		
		case CZapitMessages::CMD_BQ_DELETE_BOUQUET: 
		{
			CZapitMessages::commandDeleteBouquet msgDeleteBouquet;
			CBasicServer::receive_data(connfd, &msgDeleteBouquet, sizeof(msgDeleteBouquet)); // bouquet & channel number are already starting at 0!
			g_bouquetManager->deleteBouquet(msgDeleteBouquet.bouquet);
			break;
		}
		
		case CZapitMessages::CMD_BQ_RENAME_BOUQUET: 
		{
			CZapitMessages::commandRenameBouquet msgRenameBouquet;
			CBasicServer::receive_data(connfd, &msgRenameBouquet, sizeof(msgRenameBouquet)); // bouquet & channel number are already starting at 0!
			char * name = CBasicServer::receive_string(connfd);
			if (msgRenameBouquet.bouquet < g_bouquetManager->Bouquets.size()) 
			{
				g_bouquetManager->Bouquets[msgRenameBouquet.bouquet]->Name = name;
				g_bouquetManager->Bouquets[msgRenameBouquet.bouquet]->bUser = true;
			}
			CBasicServer::delete_string(name);
			break;
		}
		
		case CZapitMessages::CMD_BQ_EXISTS_BOUQUET: 
		{
			CZapitMessages::responseGeneralInteger responseInteger;
	
			char * name = CBasicServer::receive_string(connfd);
			responseInteger.number = g_bouquetManager->existsBouquet(name);
			CBasicServer::delete_string(name);
			CBasicServer::send_data(connfd, &responseInteger, sizeof(responseInteger)); // bouquet & channel number are already starting at 0!
			break;
		}
		
		case CZapitMessages::CMD_BQ_EXISTS_CHANNEL_IN_BOUQUET: 
		{
			CZapitMessages::commandExistsChannelInBouquet msgExistsChInBq;
			CZapitMessages::responseGeneralTrueFalse responseBool;
			CBasicServer::receive_data(connfd, &msgExistsChInBq, sizeof(msgExistsChInBq)); // bouquet & channel number are already starting at 0!
			responseBool.status = g_bouquetManager->existsChannelInBouquet(msgExistsChInBq.bouquet, msgExistsChInBq.channel_id);
			CBasicServer::send_data(connfd, &responseBool, sizeof(responseBool));
			break;
		}
		
		case CZapitMessages::CMD_BQ_MOVE_BOUQUET: 
		{
			CZapitMessages::commandMoveBouquet msgMoveBouquet;
			CBasicServer::receive_data(connfd, &msgMoveBouquet, sizeof(msgMoveBouquet)); // bouquet & channel number are already starting at 0!
			g_bouquetManager->moveBouquet(msgMoveBouquet.bouquet, msgMoveBouquet.newPos);
			break;
		}
		
		case CZapitMessages::CMD_BQ_ADD_CHANNEL_TO_BOUQUET: 
		{
			CZapitMessages::commandAddChannelToBouquet msgAddChannelToBouquet;
			CBasicServer::receive_data(connfd, &msgAddChannelToBouquet, sizeof(msgAddChannelToBouquet)); // bouquet & channel number are already starting at 0!
			addChannelToBouquet(msgAddChannelToBouquet.bouquet, msgAddChannelToBouquet.channel_id);
			break;
		}
		
		case CZapitMessages::CMD_BQ_REMOVE_CHANNEL_FROM_BOUQUET: 
		{
			CZapitMessages::commandRemoveChannelFromBouquet msgRemoveChannelFromBouquet;
			CBasicServer::receive_data(connfd, &msgRemoveChannelFromBouquet, sizeof(msgRemoveChannelFromBouquet)); // bouquet & channel number are already starting at 0!
			if (msgRemoveChannelFromBouquet.bouquet < g_bouquetManager->Bouquets.size())
				g_bouquetManager->Bouquets[msgRemoveChannelFromBouquet.bouquet]->removeService(msgRemoveChannelFromBouquet.channel_id);
#if 1 
			// REAL_REMOVE
			bool status = 0;
			for (unsigned int i = 0; i < g_bouquetManager->Bouquets.size(); i++) 
			{
				status = g_bouquetManager->existsChannelInBouquet(i, msgRemoveChannelFromBouquet.channel_id);
				if(status)
					break;
			}
	
			if(!status) 
			{
				allchans.erase(msgRemoveChannelFromBouquet.channel_id);
				live_channel = 0;
				g_list_changed = 1;
			}
#endif
			break;
		}
		
		case CZapitMessages::CMD_BQ_MOVE_CHANNEL: 
		{
			CZapitMessages::commandMoveChannel msgMoveChannel;
			CBasicServer::receive_data(connfd, &msgMoveChannel, sizeof(msgMoveChannel)); // bouquet & channel number are already starting at 0!
			if (msgMoveChannel.bouquet < g_bouquetManager->Bouquets.size())
				g_bouquetManager->Bouquets[msgMoveChannel.bouquet]->moveService(msgMoveChannel.oldPos, msgMoveChannel.newPos,
						(((currentMode & RADIO_MODE) && msgMoveChannel.mode == CZapitClient::MODE_CURRENT)
						|| (msgMoveChannel.mode==CZapitClient::MODE_RADIO)) ? 2 : 1);
			break;
		}
	
		case CZapitMessages::CMD_BQ_SET_LOCKSTATE: 
		{
			CZapitMessages::commandBouquetState msgBouquetLockState;
			CBasicServer::receive_data(connfd, &msgBouquetLockState, sizeof(msgBouquetLockState)); // bouquet & channel number are already starting at 0!
			if (msgBouquetLockState.bouquet < g_bouquetManager->Bouquets.size())
				g_bouquetManager->Bouquets[msgBouquetLockState.bouquet]->bLocked = msgBouquetLockState.state;
			break;
		}
		
		case CZapitMessages::CMD_BQ_SET_HIDDENSTATE: 
		{
			CZapitMessages::commandBouquetState msgBouquetHiddenState;
			CBasicServer::receive_data(connfd, &msgBouquetHiddenState, sizeof(msgBouquetHiddenState)); // bouquet & channel number are already starting at 0!
			if (msgBouquetHiddenState.bouquet < g_bouquetManager->Bouquets.size())
				g_bouquetManager->Bouquets[msgBouquetHiddenState.bouquet]->bHidden = msgBouquetHiddenState.state;
			break;
		}
		
		case CZapitMessages::CMD_BQ_RENUM_CHANNELLIST:
			g_bouquetManager->renumServices();
			// 2004.08.02 g_bouquetManager->storeBouquets();
			break;
	
		case CZapitMessages::CMD_BQ_SAVE_BOUQUETS: 
		{
			CZapitMessages::responseCmd response;
			response.cmd = CZapitMessages::CMD_READY;
			CBasicServer::send_data(connfd, &response, sizeof(response));

			g_bouquetManager->saveBouquets();
			g_bouquetManager->saveUBouquets();
			g_bouquetManager->renumServices();
			
			eventServer->sendEvent(CZapitClient::EVT_SERVICES_CHANGED, CEventServer::INITID_ZAPIT);
	
			if(g_list_changed) 
			{
				SaveServices(true); //FIXME
				g_list_changed = 0;
			}
			break;
		}
		
		case CZapitMessages::CMD_SET_VIDEO_SYSTEM: 
		{
			CZapitMessages::commandInt msg;
			CBasicServer::receive_data(connfd, &msg, sizeof(msg));
			
			if(videoDecoder)
				videoDecoder->SetVideoSystem(msg.val);
			break;
		}		
	
		case CZapitMessages::CMD_SB_START_PLAYBACK:
			startPlayBack(live_channel);
						
			break;
	
		case CZapitMessages::CMD_SB_STOP_PLAYBACK:
			stopPlayBack(true);
			
			CZapitMessages::responseCmd response;
			response.cmd = CZapitMessages::CMD_READY;
			CBasicServer::send_data(connfd, &response, sizeof(response));
						
			break;
	
		case CZapitMessages::CMD_SB_LOCK_PLAYBACK:		
			stopPlayBack(true);
			
#if !defined (PLATFORM_COOLSTREAM)			
			closeAVDecoder();
#endif			
			
			playbackStopForced = true;

			break;
	
		case CZapitMessages::CMD_SB_UNLOCK_PLAYBACK:
			playbackStopForced = false;
			
#if !defined (PLATFORM_COOLSTREAM)
			openAVDecoder();
#endif			

			startPlayBack(live_channel);
			
			// cam
			sendCaPmtPlayBackStart(live_channel, live_fe);
			
			// ci cam
#if defined (ENABLE_CI)	
			if(live_channel != NULL)
			{
				if(live_fe != NULL)
					ci->SendCaPMT(live_channel->getCaPmt(), live_fe->fenumber);
			}
#endif					

#if defined (ENABLE_GSTREAMER)
			if (! (currentMode & RECORD_MODE))
				if(live_channel)
					zapit(live_channel->getChannelID(), current_is_nvod);
#endif				
			
			break;
	
		case CZapitMessages::CMD_SET_AUDIO_MODE: 
		{
			CZapitMessages::commandInt msg;
			CBasicServer::receive_data(connfd, &msg, sizeof(msg));
			if(audioDecoder) 
				audioDecoder->setChannel((int) msg.val);
			
			audio_mode = msg.val;
			break;
		}
	
		case CZapitMessages::CMD_GET_AUDIO_MODE: 
		{
			CZapitMessages::commandInt msg;
			msg.val = (int) audio_mode;
			CBasicServer::send_data(connfd, &msg, sizeof(msg));
			break;
		}
		
		case CZapitMessages::CMD_GETPIDS: 
		{
			if (live_channel) 
			{
				CZapitClient::responseGetOtherPIDs responseGetOtherPIDs;
				responseGetOtherPIDs.vpid = live_channel->getVideoPid();
				responseGetOtherPIDs.vtxtpid = live_channel->getTeletextPid();
				responseGetOtherPIDs.pmtpid = live_channel->getPmtPid();
				responseGetOtherPIDs.pcrpid = live_channel->getPcrPid();
				responseGetOtherPIDs.selected_apid = live_channel->getAudioChannelIndex();
				responseGetOtherPIDs.privatepid = live_channel->getPrivatePid();
				
				CBasicServer::send_data(connfd, &responseGetOtherPIDs, sizeof(responseGetOtherPIDs));
				sendAPIDs(connfd);
				sendSubPIDs(connfd);
			}
			break;
		}
		
		case CZapitMessages::CMD_GETRECORDPIDS: 
		{
			if (rec_channel) 
			{
				CZapitClient::responseGetOtherPIDs responseGetOtherPIDs;
				responseGetOtherPIDs.vpid = rec_channel->getVideoPid();
				responseGetOtherPIDs.vtxtpid = rec_channel->getTeletextPid();
				responseGetOtherPIDs.pmtpid = rec_channel->getPmtPid();
				responseGetOtherPIDs.pcrpid = rec_channel->getPcrPid();
				responseGetOtherPIDs.selected_apid = rec_channel->getAudioChannelIndex();
				responseGetOtherPIDs.privatepid = rec_channel->getPrivatePid();
				
				CBasicServer::send_data(connfd, &responseGetOtherPIDs, sizeof(responseGetOtherPIDs));
				sendRecordAPIDs(connfd);
				sendRecordSubPIDs(connfd);
			}
			break;
		}
	
		case CZapitMessages::CMD_SETSUBSERVICES: 
		{
			CZapitClient::commandAddSubServices msgAddSubService;
	
			while (CBasicServer::receive_data(connfd, &msgAddSubService, sizeof(msgAddSubService))) 
			{
				dprintf(DEBUG_DEBUG, "NVOD insert %llx\n", CREATE_CHANNEL_ID_FROM_SERVICE_ORIGINALNETWORK_TRANSPORTSTREAM_ID(msgAddSubService.service_id, msgAddSubService.original_network_id, msgAddSubService.transport_stream_id));
				
				nvodchannels.insert (
				std::pair <t_channel_id, CZapitChannel> (
					CREATE_CHANNEL_ID_FROM_SERVICE_ORIGINALNETWORK_TRANSPORTSTREAM_ID(msgAddSubService.service_id, msgAddSubService.original_network_id, msgAddSubService.transport_stream_id),
					CZapitChannel (
					"NVOD",
					//service_id,
					msgAddSubService.service_id,
					msgAddSubService.transport_stream_id,
					//original_network_id,
					msgAddSubService.original_network_id,
					1,
					live_channel ? live_channel->getSatellitePosition() : 0,
					0) //FIXME: global for more than one tuner???
				)
				);
			}
	
			current_is_nvod = true;
			break;
		}
	
		case CZapitMessages::CMD_REGISTEREVENTS:
			eventServer->registerEvent(connfd);
			break;
	
		case CZapitMessages::CMD_UNREGISTEREVENTS:
			eventServer->unRegisterEvent(connfd);
			break;
	
		case CZapitMessages::CMD_MUTE: 
		{
			CZapitMessages::commandBoolean msgBoolean;
			CBasicServer::receive_data(connfd, &msgBoolean, sizeof(msgBoolean));

			if(!audioDecoder) 
				break;
			
			if (msgBoolean.truefalse)
				audioDecoder->SetMute(true);
			else
				audioDecoder->SetMute(false);
			break;
		}
	
		case CZapitMessages::CMD_SET_VOLUME: 
		{
			CZapitMessages::commandVolume msgVolume;
			CBasicServer::receive_data(connfd, &msgVolume, sizeof(msgVolume));
			
			audioDecoder->setVolume(msgVolume.left, msgVolume.right);
			
			break;
		}
	
		case CZapitMessages::CMD_GET_VOLUME: 
		{
			CZapitMessages::commandVolume msgVolume;
			
			msgVolume.left = msgVolume.right = audioDecoder->getVolume();

			CBasicServer::send_data(connfd, &msgVolume, sizeof(msgVolume));
			break;
		}
		
		case CZapitMessages::CMD_SET_VOLUME_PERCENT: 
		{
			CZapitMessages::commandVolumePercent msgVolumePercent;
			CBasicServer::receive_data(connfd, &msgVolumePercent, sizeof(msgVolumePercent));
			
			if (!msgVolumePercent.channel_id)
				msgVolumePercent.channel_id = live_channel_id;
			
			if (!msgVolumePercent.apid)
				msgVolumePercent.apid = live_channel->getAudioPid();
			
			// set/save pid volume
			setPidVolume(msgVolumePercent.channel_id, msgVolumePercent.apid, msgVolumePercent.percent);
			
			// set volume percent
			setVolumePercent(msgVolumePercent.percent);
			
			//FIXME: is is muted
			if(current_muted)
				audioDecoder->SetMute(true);
			
			break;
		}

		case CZapitMessages::CMD_GET_VOLUME_PERCENT: 
		{
			CZapitMessages::commandVolumePercent msgVolumePercent;
			CBasicServer::receive_data(connfd, &msgVolumePercent, sizeof(msgVolumePercent));
			
			bool isMoviePlayer = msgVolumePercent.channel_id && msgVolumePercent.apid;
			
			if (!isMoviePlayer) 
			{
				if (!msgVolumePercent.channel_id)
					msgVolumePercent.channel_id = live_channel_id;
				
				if (!msgVolumePercent.apid)
					msgVolumePercent.apid = live_channel->getAudioPid();
			}
			
			for (int  i = 0; i < live_channel->getAudioChannelCount(); i++)
			{
				if (msgVolumePercent.apid == live_channel->getAudioPid(i)) 
				{
					msgVolumePercent.percent = getPidVolume(live_channel_id, live_channel->getAudioPid(i), live_channel->getAudioChannel(i)->audioChannelType == CZapitAudioChannel::AC3);
					break;
				}
			}

			CBasicServer::send_data(connfd, &msgVolumePercent, sizeof(msgVolumePercent));
			break;
		}

		case CZapitMessages::CMD_SET_STANDBY: 
		{
			CZapitMessages::commandBoolean msgBoolean;
			CBasicServer::receive_data(connfd, &msgBoolean, sizeof(msgBoolean));
			
			if (msgBoolean.truefalse) 
			{
				enterStandby();
			} 
			else
			{
				leaveStandby();
			}
			
			// 
			CZapitMessages::responseCmd response1;
			response1.cmd = CZapitMessages::CMD_READY;
			CBasicServer::send_data(connfd, &response1, sizeof(response1));
			
			break;
		}
	
		case CZapitMessages::CMD_NVOD_SUBSERVICE_NUM: 
		{
			CZapitMessages::commandInt msg;
			CBasicServer::receive_data(connfd, &msg, sizeof(msg));
		}
		
		case CZapitMessages::CMD_SEND_MOTOR_COMMAND: 
		{
			CZapitMessages::commandMotor msgMotor;
			CBasicServer::receive_data(connfd, &msgMotor, sizeof(msgMotor));
			dprintf(DEBUG_INFO, "[zapit] received motor command: %x %x %x %x %x %x fe(%d)\n", msgMotor.cmdtype, msgMotor.address, msgMotor.cmd, msgMotor.num_parameters, msgMotor.param1, msgMotor.param2, msgMotor.feindex);
			if(msgMotor.cmdtype > 0x20)
				getFE(msgMotor.feindex)->sendMotorCommand(msgMotor.cmdtype, msgMotor.address, msgMotor.cmd, msgMotor.num_parameters, msgMotor.param1, msgMotor.param2);

			break;
		}
		
		default:
			dprintf(DEBUG_DEBUG, "unknown command %d (version %d)\n", rmsg.cmd, CZapitMessages::ACTVERSION);
			break;
	}

	return true;
}

/****************************************************************/
/*  functions for new command handling via CZapitClient		*/
/*  these functions should be encapsulated in a class CZapit	*/
/****************************************************************/

void addChannelToBouquet(const unsigned int bouquet, const t_channel_id channel_id)
{
	CZapitChannel * chan = g_bouquetManager->findChannelByChannelID(channel_id);

	if (chan != NULL)
	{
		if (bouquet < g_bouquetManager->Bouquets.size())
			g_bouquetManager->Bouquets[bouquet]->addService(chan);
		else
			printf("bouquet not found\n");
	}
	else
		printf("channel_id not found in channellist\n");
}

bool send_data_count(int connfd, int data_count)
{
	CZapitMessages::responseGeneralInteger responseInteger;
	responseInteger.number = data_count;
	if (CBasicServer::send_data(connfd, &responseInteger, sizeof(responseInteger)) == false) 
	{
		dprintf(DEBUG_INFO, "could not send any return\n");
		return false;
	}
	return true;
}

void sendAPIDs(int connfd)
{
	if (!send_data_count(connfd, live_channel->getAudioChannelCount()))
		return;

	for (uint32_t  i = 0; i < live_channel->getAudioChannelCount(); i++) 
	{
		CZapitClient::responseGetAPIDs response;
		response.pid = live_channel->getAudioPid(i);
		strncpy(response.desc, live_channel->getAudioChannel(i)->description.c_str(), 25);

		response.is_ac3 = 0;
		
		if (live_channel->getAudioChannel(i)->audioChannelType == CZapitAudioChannel::AC3
			|| live_channel->getAudioChannel(i)->audioChannelType == CZapitAudioChannel::AAC
			|| live_channel->getAudioChannel(i)->audioChannelType == CZapitAudioChannel::AACPLUS
			|| live_channel->getAudioChannel(i)->audioChannelType == CZapitAudioChannel::EAC3
			|| live_channel->getAudioChannel(i)->audioChannelType == CZapitAudioChannel::DTS
			|| live_channel->getAudioChannel(i)->audioChannelType == CZapitAudioChannel::DTSHD
			) 
		{
			response.is_ac3 = 1;
		} 
		
		response.component_tag = live_channel->getAudioChannel(i)->componentTag;

		if (CBasicServer::send_data(connfd, &response, sizeof(response)) == false) 
		{
			dprintf(DEBUG_INFO, "could not send any return\n");
			return;
		}
	}
}

void sendSubPIDs(int connfd)
{
	if (!send_data_count(connfd, live_channel->getSubtitleCount()))
		return;
	for (int i = 0 ; i < (int)live_channel->getSubtitleCount() ; ++i) 
	{
		CZapitClient::responseGetSubPIDs response;
		CZapitAbsSub* s = live_channel->getChannelSub(i);
		CZapitDVBSub* sd = reinterpret_cast<CZapitDVBSub*>(s);
		CZapitTTXSub* st = reinterpret_cast<CZapitTTXSub*>(s);

		response.pid = sd->pId;
		strncpy(response.desc, sd->ISO639_language_code.c_str(), 4);
		if (s->thisSubType == CZapitAbsSub::DVB) 
		{
			response.composition_page = sd->composition_page_id;
			response.ancillary_page = sd->ancillary_page_id;
			if (sd->subtitling_type >= 0x20) 
			{
				response.hearingImpaired = true;
			} 
			else 
			{
				response.hearingImpaired = false;
			}
			
			if (CBasicServer::send_data(connfd, &response, sizeof(response)) == false) 
			{
				dprintf(DEBUG_INFO, "could not send any return\n");
	                        return;
        	        }
		} 
		else if (s->thisSubType == CZapitAbsSub::TTX) 
		{
			response.composition_page = (st->teletext_magazine_number * 100) + ((st->teletext_page_number >> 4) * 10) + (st->teletext_page_number & 0xf);
			response.ancillary_page = 0;
			response.hearingImpaired = st->hearingImpaired;
			if (CBasicServer::send_data(connfd, &response, sizeof(response)) == false) 
			{
				dprintf(DEBUG_INFO, "could not send any return\n");
	                        return;
        	        }
		}
	}
}

//
void sendRecordAPIDs(int connfd)
{
	if (!send_data_count(connfd, rec_channel->getAudioChannelCount()))
		return;

	for (uint32_t  i = 0; i < rec_channel->getAudioChannelCount(); i++) 
	{
		CZapitClient::responseGetAPIDs response;
		response.pid = rec_channel->getAudioPid(i);
		strncpy(response.desc, rec_channel->getAudioChannel(i)->description.c_str(), 25);

		response.is_ac3 = 0;
		
		if (rec_channel->getAudioChannel(i)->audioChannelType == CZapitAudioChannel::AC3
			|| rec_channel->getAudioChannel(i)->audioChannelType == CZapitAudioChannel::AAC
			|| rec_channel->getAudioChannel(i)->audioChannelType == CZapitAudioChannel::AACPLUS
			|| rec_channel->getAudioChannel(i)->audioChannelType == CZapitAudioChannel::EAC3
			|| rec_channel->getAudioChannel(i)->audioChannelType == CZapitAudioChannel::DTS
			|| rec_channel->getAudioChannel(i)->audioChannelType == CZapitAudioChannel::DTSHD
			) 
		{
			response.is_ac3 = 1;
		} 
		
		response.component_tag = rec_channel->getAudioChannel(i)->componentTag;

		if (CBasicServer::send_data(connfd, &response, sizeof(response)) == false) 
		{
			dprintf(DEBUG_INFO, "could not send any return\n");
			return;
		}
	}
}

void sendRecordSubPIDs(int connfd)
{
	if (!send_data_count(connfd, rec_channel->getSubtitleCount()))
		return;
	for (int i = 0 ; i < (int)rec_channel->getSubtitleCount() ; ++i) 
	{
		CZapitClient::responseGetSubPIDs response;
		CZapitAbsSub* s = rec_channel->getChannelSub(i);
		CZapitDVBSub* sd = reinterpret_cast<CZapitDVBSub*>(s);
		CZapitTTXSub* st = reinterpret_cast<CZapitTTXSub*>(s);

		response.pid = sd->pId;
		strncpy(response.desc, sd->ISO639_language_code.c_str(), 4);
		if (s->thisSubType == CZapitAbsSub::DVB) 
		{
			response.composition_page = sd->composition_page_id;
			response.ancillary_page = sd->ancillary_page_id;
			if (sd->subtitling_type >= 0x20) 
			{
				response.hearingImpaired = true;
			} 
			else 
			{
				response.hearingImpaired = false;
			}
			
			if (CBasicServer::send_data(connfd, &response, sizeof(response)) == false) 
			{
				dprintf(DEBUG_INFO, "could not send any return\n");
	                        return;
        	        }
		} 
		else if (s->thisSubType == CZapitAbsSub::TTX) 
		{
			response.composition_page = (st->teletext_magazine_number * 100) + ((st->teletext_page_number >> 4) * 10) + (st->teletext_page_number & 0xf);
			response.ancillary_page = 0;
			response.hearingImpaired = st->hearingImpaired;
			if (CBasicServer::send_data(connfd, &response, sizeof(response)) == false) 
			{
				dprintf(DEBUG_INFO, "could not send any return\n");
	                        return;
        	        }
		}
	}
}

void internalSendChannels(int connfd, ZapitChannelList* channels, const unsigned int first_channel_nr, bool nonames)
{
	int data_count = channels->size();

#if RECORD_RESEND // old, before tv/radio resend
	if (currentMode & RECORD_MODE) 
	{
		for (uint32_t  i = 0; i < channels->size(); i++)
			if ((*channels)[i]->getTransponderId() != live_channel->getTransponderId())
				data_count--;
	}
#endif
	if (!send_data_count(connfd, data_count))
		return;

	for (uint32_t  i = 0; i < channels->size();i++) 
	{
#if RECORD_RESEND // old, before tv/radio resend
		if ((currentMode & RECORD_MODE) && ((*channels)[i]->getTransponderId() != live_fe->getTsidOnid()))
			continue;
#endif

		if(nonames) 
		{
			CZapitClient::responseGetBouquetNChannels response;
			response.nr = first_channel_nr + i;

			if (CBasicServer::send_data(connfd, &response, sizeof(response)) == false) 
			{
				dprintf(DEBUG_INFO, "could not send any return\n");
				if (CBasicServer::send_data(connfd, &response, sizeof(response)) == false) 
				{
					dprintf(DEBUG_INFO, "could not send any return, stop\n");
					return;
				}
			}
		} 
		else 
		{
			CZapitClient::responseGetBouquetChannels response;
			strncpy(response.name, ((*channels)[i]->getName()).c_str(), CHANNEL_NAME_SIZE);
			response.name[CHANNEL_NAME_SIZE-1] = 0;
			
			//printf("internalSendChannels: name %s\n", response.name);
			
			response.satellitePosition = (*channels)[i]->getSatellitePosition();
			response.channel_id = (*channels)[i]->getChannelID();
			response.nr = first_channel_nr + i;

			if (CBasicServer::send_data(connfd, &response, sizeof(response)) == false) 
			{
				dprintf(DEBUG_INFO, "could not send any return\n");
				dprintf(DEBUG_DEBUG, "current: %d name %s total %d\n", i, response.name, data_count);
				
				if (CBasicServer::send_data(connfd, &response, sizeof(response)) == false) 
				{
					dprintf(DEBUG_INFO, "could not send any return, stop\n");
					return;
				}
			}
		}
	}
}

void sendBouquets(int connfd, const bool emptyBouquetsToo, CZapitClient::channelsMode mode)
{
	CZapitClient::responseGetBouquets msgBouquet;
        int curMode;
	
        switch(mode) 
	{
                case CZapitClient::MODE_TV:
                        curMode = TV_MODE;
                        break;
                case CZapitClient::MODE_RADIO:
                        curMode = RADIO_MODE;
                        break;
                case CZapitClient::MODE_CURRENT:
                default:
                        curMode = currentMode;
                        break;
        }

        for (uint32_t i = 0; i < g_bouquetManager->Bouquets.size(); i++) 
	{
                if (emptyBouquetsToo || (!g_bouquetManager->Bouquets[i]->bHidden && g_bouquetManager->Bouquets[i]->bUser)
                    || ((!g_bouquetManager->Bouquets[i]->bHidden)
                     && (((curMode & RADIO_MODE) && !g_bouquetManager->Bouquets[i]->radioChannels.empty()) ||
                      ((curMode & TV_MODE) && !g_bouquetManager->Bouquets[i]->tvChannels.empty())))
                   )
		{
// ATTENTION: in RECORD_MODE empty bouquets are not send!
#if RECORD_RESEND // old, before tv/radio resend
			if ((!(currentMode & RECORD_MODE)) || (( live_fe != NULL) &&
			     (((currentMode & RADIO_MODE) && (g_bouquetManager->Bouquets[i]->recModeRadioSize( live_fe->getTsidOnid()) > 0)) ||
			      ((currentMode & TV_MODE)    && (g_bouquetManager->Bouquets[i]->recModeTVSize   ( live_fe->getTsidOnid()) > 0)))))
#endif
			{
				msgBouquet.bouquet_nr = i;
				strncpy(msgBouquet.name, g_bouquetManager->Bouquets[i]->Name.c_str(), 30);
				msgBouquet.name[29] = 0;
				msgBouquet.locked     = g_bouquetManager->Bouquets[i]->bLocked;
				msgBouquet.hidden     = g_bouquetManager->Bouquets[i]->bHidden;
				if (CBasicServer::send_data(connfd, &msgBouquet, sizeof(msgBouquet)) == false) 
				{
					dprintf(DEBUG_INFO, "could not send any return\n");
					return;
				}
			}
		}
	}
	msgBouquet.bouquet_nr = RESPONSE_GET_BOUQUETS_END_MARKER;
	if (CBasicServer::send_data(connfd, &msgBouquet, sizeof(msgBouquet)) == false) 
	{
		dprintf(DEBUG_INFO, "could not send end marker\n");
		return;
	}
}

void sendBouquetChannels(int connfd, const unsigned int bouquet, const CZapitClient::channelsMode mode, bool nonames)
{
	if (bouquet >= g_bouquetManager->Bouquets.size()) 
	{
		dprintf(DEBUG_INFO, "invalid bouquet number: %d\n", bouquet);
		return;
	}

	if (((currentMode & RADIO_MODE) && (mode == CZapitClient::MODE_CURRENT)) || (mode == CZapitClient::MODE_RADIO))
		internalSendChannels(connfd, &(g_bouquetManager->Bouquets[bouquet]->radioChannels), g_bouquetManager->radioChannelsBegin().getNrofFirstChannelofBouquet(bouquet), nonames);
	else
		internalSendChannels(connfd, &(g_bouquetManager->Bouquets[bouquet]->tvChannels), g_bouquetManager->tvChannelsBegin().getNrofFirstChannelofBouquet(bouquet), nonames);
}

void sendChannels(int connfd, const CZapitClient::channelsMode mode, const CZapitClient::channelsOrder order)
{
	ZapitChannelList channels;

	if (order == CZapitClient::SORT_BOUQUET) 
	{
		CBouquetManager::ChannelIterator cit = (((currentMode & RADIO_MODE) && (mode == CZapitClient::MODE_CURRENT)) || (mode==CZapitClient::MODE_RADIO)) ? g_bouquetManager->radioChannelsBegin() : g_bouquetManager->tvChannelsBegin();
		for (; !(cit.EndOfChannels()); cit++)
			channels.push_back(*cit);
	}
	else if (order == CZapitClient::SORT_ALPHA)   // ATTENTION: in this case response.nr does not return the actual number of the channel for zapping!
	{
		if (((currentMode & RADIO_MODE) && (mode == CZapitClient::MODE_CURRENT)) || (mode==CZapitClient::MODE_RADIO)) 
		{
			for (tallchans_iterator it = allchans.begin(); it != allchans.end(); it++)
				if (it->second.getServiceType() == ST_DIGITAL_RADIO_SOUND_SERVICE)
					channels.push_back(&(it->second));
		} 
		else 
		{
			for (tallchans_iterator it = allchans.begin(); it != allchans.end(); it++)
				if (it->second.getServiceType() != ST_DIGITAL_RADIO_SOUND_SERVICE)
					channels.push_back(&(it->second));
		}
		sort(channels.begin(), channels.end(), CmpChannelByChName());
	}

	internalSendChannels(connfd, &channels, 0, false);
}

#include <playback_cs.h>
#include <system/helpers.h>
extern cPlayback *playback;
// startplayback
int startPlayBack(CZapitChannel * thisChannel)
{
	if(g_settings.satip_allow_satip)
	{
		dprintf(DEBUG_NORMAL, "zapit:startPlayBack: pmtpid 0x%X videopid 0x%X audiopid 0x%X\n", thisChannel->getPmtPid(), thisChannel->getVideoPid(), thisChannel->getPreAudioPid() );

		closeAVDecoder();

		// build channel url
		std::string ChannelURL;

		ChannelURL = "http://";
		ChannelURL += g_settings.satip_serverbox_ip;
		ChannelURL += ":31339/0,";

		ChannelURL += to_hexstring(thisChannel->getPmtPid());
		ChannelURL += ",";
		ChannelURL += to_hexstring(thisChannel->getVideoPid());
		ChannelURL += ",";
		ChannelURL += to_hexstring(thisChannel->getPreAudioPid());

		playback->Open();
		playback->Start((char *)ChannelURL.c_str());
	}
	else
	{
		bool have_pcr = false;
		bool have_audio = false;
		bool have_video = false;
		bool have_teletext = false;

		if(!thisChannel)
			thisChannel = live_channel;

		if ((playbackStopForced == true) || (!thisChannel) || playing)
			return -1;

		dprintf(DEBUG_NORMAL, "zapit:startPlayBack: vpid 0x%X apid 0x%X pcrpid 0x%X\n", thisChannel->getVideoPid(), thisChannel->getAudioPid(), thisChannel->getPcrPid() );

		if(standby) 
			return 0;

		if (thisChannel->getPcrPid() != 0)
			have_pcr = true;

		if (thisChannel->getAudioPid() != 0)
			have_audio = true;
		
		if ((thisChannel->getVideoPid() != 0) && (currentMode & TV_MODE))
			have_video = true;
		
		if (thisChannel->getTeletextPid() != 0)
			have_teletext = true;

		if ((!have_audio) && (!have_video) && (!have_teletext))
			return -1;

		if(have_video && (thisChannel->getPcrPid() == 0x1FFF)) 
		{ 
			//FIXME
			thisChannel->setPcrPid(thisChannel->getVideoPid());
			have_pcr = true;
		}
		
		// pcr pid
		if (have_pcr) 
		{
			if(!pcrDemux)
				pcrDemux = new cDemux();
		
			// open pcr demux
#if defined (PLATFORM_COOLSTREAM)
			pcrDemux->Open(DMX_PCR_ONLY_CHANNEL, videoDemux->getBuffer());
#else		
			if( pcrDemux->Open(DMX_PCR_ONLY_CHANNEL, VIDEO_STREAM_BUFFER_SIZE, live_fe) < 0 )
				return -1;
#endif		
		
			// set pes filter
			if( pcrDemux->pesFilter(thisChannel->getPcrPid() ) < 0 )
				return -1;
		
			if ( pcrDemux->Start() < 0 )
				return -1;
		}
	
		// audio pid
		if (have_audio) 
		{
			if( !audioDemux )
				audioDemux = new cDemux();
		
			// open audio demux
#if defined (PLATFORM_COOLSTREAM)
			audioDemux->Open(DMX_AUDIO_CHANNEL);
#else		
			if( audioDemux->Open(DMX_AUDIO_CHANNEL, AUDIO_STREAM_BUFFER_SIZE, live_fe ) < 0 )
				return -1;
#endif		
		
			// set pes filter
			if( audioDemux->pesFilter(thisChannel->getAudioPid() ) < 0 )
				return -1;
		
			if ( audioDemux->Start() < 0 )
				return -1;
		}
	
		// video pid
		if (have_video) 
		{
			if( !videoDemux )
				videoDemux = new cDemux(); 
		
			// open Video Demux
#if defined (PLATFORM_COOLSTREAM)
			videoDemux->Open(DMX_VIDEO_CHANNEL);
#else		
			if( videoDemux->Open(DMX_VIDEO_CHANNEL, VIDEO_STREAM_BUFFER_SIZE, live_fe ) < 0 )
				return -1;
#endif		
		
			// video pes filter
			if( videoDemux->pesFilter(thisChannel->getVideoPid() ) < 0)
				return -1;		
		
			if ( videoDemux->Start() < 0 )
				return -1;
		}

#if defined (USE_OPENGL)
		startOpenGLplayback();
#else
	
		// select audio output and start audio
		if (have_audio) 
		{	
			//audio codec
			const char *audioStr = "UNKNOWN";
		
			if(audioDecoder)
			{
				switch (thisChannel->getAudioChannel()->audioChannelType) 
				{
					case CZapitAudioChannel::AC3:
						audioStr = "AC3";
#if defined (__sh__)
						audioDecoder->SetEncoding(AUDIO_ENCODING_AC3);
#elif defined (PLATFORM_COOLSTREAM)
						audioDecoder->SetStreamType(AUDIO_FMT_DOLBY_DIGITAL);
#else
						audioDecoder->SetStreamType(AUDIO_STREAMTYPE_AC3);
#endif
						break;
					
					case CZapitAudioChannel::MPEG:
						audioStr = "MPEG2";
#if defined (__sh__)
						audioDecoder->SetEncoding(AUDIO_ENCODING_MPEG2);
#elif defined (PLATFORM_COOLSTREAM)
						audioDecoder->SetStreamType(AUDIO_FMT_MPEG);
#else
						audioDecoder->SetStreamType(AUDIO_STREAMTYPE_MPEG);
#endif
						break;
					
					case CZapitAudioChannel::AAC:
						audioStr = "AAC";
#if defined (__sh__)
						audioDecoder->SetEncoding(AUDIO_ENCODING_AAC);
#elif defined (PLATFORM_COOLSTREAM)
						audioDecoder->SetStreamType(AUDIO_FMT_AAC);
#else
						audioDecoder->SetStreamType(AUDIO_STREAMTYPE_AAC);
#endif					
						break;
					
					case CZapitAudioChannel::AACPLUS:
						audioStr = "AAC-PLUS";
#if defined (PLATFORM_COOLSTREAM)
						audioDecoder->SetStreamType(AUDIO_FMT_AAC);
#elif !defined (__sh__)
						audioDecoder->SetStreamType(AUDIO_STREAMTYPE_AACPLUS);
#endif
						break;
					
					case CZapitAudioChannel::DTS:
						audioStr = "DTS";
#if defined (__sh__)
						audioDecoder->SetEncoding(AUDIO_ENCODING_DTS);
#elif defined (PLATFORM_COOLSTREAM)
						audioDecoder->SetStreamType(AUDIO_FMT_DTS);
#else
						audioDecoder->SetStreamType(AUDIO_STREAMTYPE_DTS);
#endif
						break;
					
					case CZapitAudioChannel::DTSHD:
						audioStr = "DTSHD";
#if !defined (__sh__) && !defined (PLATFORM_COOLSTREAM)
						audioDecoder->SetStreamType(AUDIO_STREAMTYPE_DTSHD);
#endif
						break;
					
					case CZapitAudioChannel::EAC3:
						audioStr = "EAC3";
#if defined (__sh__)
					audioDecoder->SetEncoding(AUDIO_ENCODING_AC3);
#elif defined (PLATFORM_COOLSTREAM)
					audioDecoder->SetStreamType(AUDIO_FMT_DD_PLUS);
#else
					audioDecoder->SetStreamType(AUDIO_STREAMTYPE_EAC3);
#endif
						break;
					
					case CZapitAudioChannel::LPCM:
						audioStr = "LPCM";
#if defined (__sh__)			  
						audioDecoder->SetEncoding(AUDIO_ENCODING_LPCM);
#elif !defined (PLATFORM_COOLSTREAM)
						audioDecoder->SetStreamType(AUDIO_STREAMTYPE_LPCMDVD);
#endif
						break;
					
					default:
						dprintf(DEBUG_INFO, "[zapit] unknown audio live_channel type 0x%x\n", thisChannel->getAudioChannel()->audioChannelType);
						break;
				}
			}
		
			dprintf(DEBUG_NORMAL, "[zapit] starting %s audio Pid: 0x%X\n", audioStr, thisChannel->getAudioPid());		
		
			// start Audio Deocder
			if(audioDecoder)
			{			
#if !defined (__sh__)			
				audioDecoder->Resume();
				audioDecoder->Stop();
#endif				  
				audioDecoder->Start();
			}
		}

		// start video
		if (have_video) 
		{		
			const char *videoStr = "UNKNOWN";
		
			if(videoDecoder)
			{
				if(thisChannel->type == 0)
				{
					videoStr = "MPEG2";
#if defined (__sh__)
					videoDecoder->SetEncoding(VIDEO_ENCODING_MPEG2);
#elif defined (PLATFORM_COOLSTREAM)
					g->SetStreamType(VIDEO_FORMAT_MPEG2);
#else
					videoDecoder->SetStreamType(VIDEO_STREAMTYPE_MPEG2);
#endif
				}
				else if(thisChannel->type == 1)
				{
					videoStr = "H.264/MPEG-4 AVC";
#if defined (__sh__)
					videoDecoder->SetEncoding(VIDEO_ENCODING_H264);
#elif defined (PLATFORM_COOLSTREAM)
					videoDecoder->SetStreamType(VIDEO_FORMAT_MPEG4);
#else
					videoDecoder->SetStreamType(VIDEO_STREAMTYPE_MPEG4_H264);
#endif				
				}
			}
	
			dprintf(DEBUG_NORMAL, "[zapit] starting %s video Pid: 0x%x\n", videoStr, thisChannel->getVideoPid());	
		
			// start Video Decoder
			if(videoDecoder)
			{
#if !defined (__sh__)
				videoDecoder->Resume();
				videoDecoder->Stop();
#endif
								  
#if defined (PLATFORM_COOLSTREAM)
				videoDecoder->Start(0, thisChannel->getPcrPid(), thisChannel->getVideoPid());
#else			
				videoDecoder->Start();
#endif	
			}
		}
#endif
	}

	playing = true;
	
	return 0;
}

int stopPlayBack(bool sendPmt)
{
	dprintf(DEBUG_NORMAL, "[zapit] stopPlayBack: standby %d forced %d\n", standby, playbackStopForced);
	
	if(!g_settings.satip_allow_satip)
	{
		// capmt
		sendcapmtPlayBackStop(sendPmt);

		if (!playing)
			return 0;

		if (playbackStopForced)
			return -1;

		// stop video
		if (videoDemux)
		{
			// stop
			videoDemux->Stop();
			delete videoDemux;	//destructor closes dmx
			videoDemux = NULL;
		}
	
		if (audioDemux)
		{
			// stop
			audioDemux->Stop();
			delete audioDemux;  //destructor closes dmx
			audioDemux = NULL;
		}
	
		if (pcrDemux)
		{
			// stop
			pcrDemux->Stop();
			delete pcrDemux; //destructor closes dmx
			pcrDemux = NULL;
		}

#if defined (USE_OPENGL)
		stopOpenGLplayback();
#else
		
		// audio decoder stop
		audioDecoder->Stop();
	
		// video decoder stop (blanking)
		videoDecoder->Stop();
#endif	

		playing = false;
	
		// stop tuxtxt subtitle
		tuxtx_stop_subtitle();

		// stop?pause dvbsubtitle
		if(standby)
			dvbsub_pause();
		else
		{
			dvbsub_stop();
		}
	}
	else
	{
		playback->Close();

		openAVDecoder();
	}

	return 0;
}

void closeAVDecoder(void)
{
#if !defined (USE_OPENGL)
	if(!g_settings.satip_allow_satip)
	{
		// close videodecoder
		if(videoDecoder)
			videoDecoder->Close();
	
		// close audiodecoder
		if(audioDecoder)
			audioDecoder->Close();
	}
#endif
}

void openAVDecoder(void)
{
#if !defined (USE_OPENGL)
	if(!g_settings.satip_allow_satip)
	{
		if(videoDecoder)
		{
			// open video decoder
			videoDecoder->Open(live_fe);
	
			// set source
			videoDecoder->setSource(VIDEO_SOURCE_DEMUX);	
		
#if defined (__sh__)
			// StreamType
			videoDecoder->SetStreamType(STREAM_TYPE_TRANSPORT);
#endif
			}
		}	
	
		if(audioDecoder)
		{
			// open audiodecoder
			audioDecoder->Open(live_fe);
		
			// set source
			audioDecoder->setSource(AUDIO_SOURCE_DEMUX);
	
#if defined (__sh__)		
			// StreamType
			audioDecoder->SetStreamType(STREAM_TYPE_TRANSPORT);
#endif
		}
	}	
#endif
}

void enterStandby(void)
{ 
	if (standby)
		return;

	standby = true;

	// save zapitconfig
	saveZapitSettings(true, true);
	
	// stop playback
	stopPlayBack(true);
	
#if !defined (PLATFORM_COOLSTREAM)
	// close AVdecoder
	closeAVDecoder();
#endif	
	
	//close frontend	
	CloseFE();
}

void leaveStandby(void)
{ 
	dprintf(DEBUG_NORMAL, "zapit:leaveStandby\n");
	
	if(!standby) 
		return;
	
	standby = false;
	
	// live cam
	if (!cam0) 
		cam0 = new CCam();
	
	// record cam
	if(!cam1)
		cam1 = new CCam();
	
#if !defined (PLATFORM_COOLSTREAM)	
	openAVDecoder();
#endif	

	// if we have already zapped channel
	if (live_channel)
		zapTo_ChannelID(live_channel_id, current_is_nvod);
}

unsigned zapTo(const unsigned int bouquet, const unsigned int channel)
{
	if (bouquet >= g_bouquetManager->Bouquets.size()) 
	{
		dprintf(DEBUG_INFO, "Invalid bouquet %d\n", bouquet);
		return CZapitClient::ZAP_INVALID_PARAM;
	}

	ZapitChannelList * channels;

	if (currentMode & RADIO_MODE)
		channels = &(g_bouquetManager->Bouquets[bouquet]->radioChannels);
	else
		channels = &(g_bouquetManager->Bouquets[bouquet]->tvChannels);

	if (channel >= channels->size()) 
	{
		dprintf(DEBUG_INFO, "Invalid channel %d in bouquet %d\n", channel, bouquet);
		return CZapitClient::ZAP_INVALID_PARAM;
	}

	return zapTo_ChannelID((*channels)[channel]->getChannelID(), false);
}

unsigned int zapTo_ChannelID(t_channel_id channel_id, bool isSubService)
{
	unsigned int result = 0;

	if (zapit(channel_id, isSubService) < 0) 
	{
		dprintf(DEBUG_NORMAL, "zapTo_ChannelID: zapit failed, chid %llx\n", channel_id);
		
		eventServer->sendEvent((isSubService ? CZapitClient::EVT_ZAP_SUB_FAILED : CZapitClient::EVT_ZAP_FAILED), CEventServer::INITID_ZAPIT, &channel_id, sizeof(channel_id));
		
		return result;
	}

	result |= CZapitClient::ZAP_OK;

	dprintf(DEBUG_NORMAL, "zapTo_ChannelID: zapit OK, chid %llx\n", channel_id);
	
	if (isSubService) 
	{
		dprintf(DEBUG_NORMAL, "zapTo_ChannelID: isSubService chid %llx\n", channel_id);
		
		eventServer->sendEvent(CZapitClient::EVT_ZAP_SUB_COMPLETE, CEventServer::INITID_ZAPIT, &channel_id, sizeof(channel_id));
	}
	else if (current_is_nvod) 
	{
		dprintf(DEBUG_NORMAL, "zapTo_ChannelID: NVOD chid %llx\n", channel_id);
		
		eventServer->sendEvent(CZapitClient::EVT_ZAP_COMPLETE_IS_NVOD, CEventServer::INITID_ZAPIT, &channel_id, sizeof(channel_id));
		
		result |= CZapitClient::ZAP_IS_NVOD;
	}
	else
		eventServer->sendEvent(CZapitClient::EVT_ZAP_COMPLETE, CEventServer::INITID_ZAPIT, &channel_id, sizeof(channel_id));

	return result;
}

unsigned zapTo(const unsigned int channel)
{
	CBouquetManager::ChannelIterator cit = ((currentMode & RADIO_MODE) ? g_bouquetManager->radioChannelsBegin() : g_bouquetManager->tvChannelsBegin()).FindChannelNr(channel);
	if (!(cit.EndOfChannels()))
		return zapTo_ChannelID((*cit)->getChannelID(), false);
	else
		return 0;
}

void setZapitConfig(Zapit_config * Cfg)
{
	makeRemainingChannelsBouquet = Cfg->makeRemainingChannelsBouquet;
	config.setBool("saveLastChannel", Cfg->saveLastChannel);
	scanSDT = Cfg->scanSDT;
	
	// save it
	saveZapitSettings(true, false);
}

void sendConfig(int connfd)
{
	dprintf(DEBUG_INFO, "\n[zapit]sendConfig:\n");
	
	Zapit_config Cfg;

	Cfg.makeRemainingChannelsBouquet = makeRemainingChannelsBouquet;
	Cfg.saveLastChannel = config.getBool("saveLastChannel", true);
	Cfg.scanSDT = scanSDT;

	// send
	CBasicServer::send_data(connfd, &Cfg, sizeof(Cfg));
}

void getZapitConfig(Zapit_config *Cfg)
{
        Cfg->makeRemainingChannelsBouquet = makeRemainingChannelsBouquet;
        Cfg->saveLastChannel = config.getBool("saveLastChannel", true);
        Cfg->scanSDT = scanSDT;
}

sdt_tp_t sdt_tp;
void * sdt_thread(void */*arg*/)
{
	dprintf(DEBUG_INFO, "[zapit] sdt_thread: starting... tid %ld\n", syscall(__NR_gettid));
	
	time_t tstart, tcur, wtime = 0;
	int ret;
	t_transport_stream_id           transport_stream_id = 0;
	t_original_network_id           original_network_id = 0;
	t_satellite_position            satellitePosition = 0;
	freq_id_t                       freq = 0;

	transponder_id_t 		tpid = 0;
	FILE * fd = 0;
	FILE * fd1 = 0;
	bool updated = 0;

	tcur = time(0);
	tstart = time(0);
	sdt_tp.clear();
	
	dprintf(DEBUG_INFO, "[zapit] sdt monitor started\n");

	while(zapit_ready) 
	{
		sleep(1);

		if(sdt_wakeup) 
		{
			sdt_wakeup = 0;

			if(live_channel) 
			{
				wtime = time(0);
				transport_stream_id = live_channel->getTransportStreamId();
				original_network_id = live_channel->getOriginalNetworkId();
				satellitePosition = live_channel->getSatellitePosition();
				freq = live_channel->getFreqId();
				tpid = live_channel->getTransponderId();
			}
		}
		
		if(!scanSDT)
			continue;

		tcur = time(0);
		
		if(wtime && ((tcur - wtime) > 2) && !sdt_wakeup) 
		{
			dprintf(DEBUG_INFO, "[sdt monitor] wakeup...\n");
			
			wtime = 0;

			if(scan_runs)
				continue;

			updated = 0;
			tallchans_iterator ccI;
			tallchans_iterator dI;
			transponder_list_t::iterator tI;
			sdt_tp_t::iterator stI;
			char tpstr[256];
			char satstr[256];
			bool tpdone = 0;
			//bool satfound = 0;
			//FIXME: writing current services is brocken ;(
			//NOTE: think about multi services

			tI = transponders.find(tpid);
			if(tI == transponders.end()) 
			{
				dprintf(DEBUG_INFO, "[sdt monitor] tp not found ?!\n");
				continue;
			}
			stI = sdt_tp.find(tpid);

			if((stI != sdt_tp.end()) && stI->second) 
			{
				dprintf(DEBUG_INFO, "[sdt monitor] TP already updated.\n");
				continue;
			}

			if(live_channel) 
			{
				if( parse_current_sdt(transport_stream_id, original_network_id, satellitePosition, freq, live_fe) < 0 )
					continue;
			}

			sdt_tp.insert(std::pair <transponder_id_t, bool> (tpid, true) );

			char buffer[256];
			fd = fopen(CURRENTSERVICES_TMP, "w");
			if(!fd) {
				dprintf(DEBUG_INFO, "[sdt monitor] " CURRENTSERVICES_TMP ": cant open!\n");
				continue;
			}

			sat_iterator_t spos_it = satellitePositions.find(satellitePosition); 
			if(spos_it == satellitePositions.end())
				continue;

			if(live_channel) 
			{
				switch(spos_it->second.type)
				{
					case DVB_S: /* satellite */
						sprintf(satstr, "\t<%s name=\"%s\" position=\"%hd\">\n", "sat", spos_it->second.name.c_str(), satellitePosition);
						sprintf(tpstr, "\t\t<TS id=\"%04x\" on=\"%04x\" frq=\"%u\" inv=\"%hu\" sr=\"%u\" fec=\"%hu\" pol=\"%hu\">\n",
						tI->second.transport_stream_id, tI->second.original_network_id,
						tI->second.feparams.frequency, tI->second.feparams.inversion,
						tI->second.feparams.u.qpsk.symbol_rate, tI->second.feparams.u.qpsk.fec_inner,
						tI->second.polarization);
						break;

					case DVB_C: /* cable */
						sprintf(satstr, "\t<%s name=\"%s\">\n", "cable", spos_it->second.name.c_str());
						sprintf(tpstr, "\t\t<TS id=\"%04x\" on=\"%04x\" frq=\"%u\" inv=\"%hu\" sr=\"%u\" fec=\"%hu\" mod=\"%hu\">\n",
						tI->second.transport_stream_id, tI->second.original_network_id,
						tI->second.feparams.frequency, tI->second.feparams.inversion,
						tI->second.feparams.u.qam.symbol_rate, tI->second.feparams.u.qam.fec_inner,
						tI->second.feparams.u.qam.modulation);
						break;
						
					case DVB_T: /* terrestrial */
						sprintf(satstr, "\t<%s name=\"%s\">\n", "terrestrial", spos_it->second.name.c_str());
						sprintf(tpstr, "\t\t<TS id=\"%04x\" on=\"%04x\" frq=\"%u\" inv=\"%hu\" band=\"%hu\" HP=\"%hu\" LP=\"%hu\" const=\"%hu\" trans=\"%hu\" guard=\"%hu\" hierarchy=\"%hu\">\n",
						tI->second.transport_stream_id, tI->second.original_network_id,
						tI->second.feparams.frequency, tI->second.feparams.inversion,
						tI->second.feparams.u.ofdm.bandwidth, tI->second.feparams.u.ofdm.code_rate_HP,
						tI->second.feparams.u.ofdm.code_rate_LP, tI->second.feparams.u.ofdm.constellation,tI->second.feparams.u.ofdm.transmission_mode, tI->second.feparams.u.ofdm.guard_interval, tI->second.feparams.u.ofdm.hierarchy_information);
						break;

					default:
						break;
				}
			}

			fd1 = fopen(CURRENTSERVICES_XML, "r");

			if(!fd1) 
			{
				fprintf(fd, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<zapit>\n");
			} 
			else 
			{
				fgets(buffer, 255, fd1);
				//while(!feof(fd1) && !strstr(buffer, satfound ? "</sat>" : "</zapit>")) 
				while( !feof(fd1) ) 
				{
					//if(!satfound && !strcmp(buffer, satstr))
					//	satfound = 1;
					fputs(buffer, fd);
					fgets(buffer, 255, fd1);
				}
				//fclose(fd1);
			}

			for (tallchans_iterator cI = curchans.begin(); cI != curchans.end(); cI++) 
			{
				ccI = allchans.find(cI->second.getChannelID());
				if(ccI == allchans.end()) 
				{
					if(!tpdone) 
					{
						//if(!satfound) 
							fprintf(fd, "%s", satstr);
						fprintf(fd, "%s", tpstr);
						tpdone = 1;
					}
					updated = 1;

					fprintf(fd, "\t\t\t<S action=\"add\" i=\"%04x\" n=\"%s\" t=\"%x\"/>\n",
                                        	cI->second.getServiceId(), convert_UTF8_To_UTF8_XML(cI->second.getName().c_str()).c_str(),
                                        	cI->second.getServiceType());
				} 
				else 
				{
					if(strcmp(cI->second.getName().c_str(), ccI->second.getName().c_str())) 
					{
					   if(!tpdone) 
					   {
						//if(!satfound) 
							fprintf(fd, "%s", satstr);
						fprintf(fd, "%s", tpstr);
						tpdone = 1;
					   }
					   updated = 1;
					   fprintf(fd, "\t\t\t<S action=\"replace\" i=\"%04x\" n=\"%s\" t=\"%x\"/>\n",
                                        	cI->second.getServiceId(), convert_UTF8_To_UTF8_XML(cI->second.getName().c_str()).c_str(),
                                        	cI->second.getServiceType());
					}
				}
			}

			for (ccI = allchans.begin(); ccI != allchans.end(); ccI++) 
			{
				if(ccI->second.getTransponderId() == tpid) 
				{
					dI = curchans.find(ccI->second.getChannelID());
					if(dI == curchans.end()) 
					{
					   	if(!tpdone) 
						{
							//if(!satfound) 
								fprintf(fd, "%s", satstr);

							fprintf(fd, "%s", tpstr);
							tpdone = 1;
					   	}

					   	updated = 1;
					   	fprintf(fd, "\t\t\t<S action=\"remove\" i=\"%04x\" n=\"%s\" t=\"%x\"/>\n",
                                        	ccI->second.getServiceId(), convert_UTF8_To_UTF8_XML(ccI->second.getName().c_str()).c_str(),
                                        	ccI->second.getServiceType());
					}
				}
			}

			if(tpdone) 
			{
				//fprintf(fd, "\t\t</TS>\n");
				//switch ( live_fe->getInfo()->type)
				switch(spos_it->second.type)
				{
					case DVB_S: /* satellite */
						fprintf(fd, "\t</sat>\n");
						break;
						
					case DVB_C: /* cable */
						fprintf(fd, "\t</cable>\n");
						break;
						
					case DVB_T: /* terrestrial */
						fprintf(fd, "\t</terrestrial>\n");
						break;
						
					default:
						break;
				}
			}
			#if 0
			else if(satfound)
			{
				//fprintf(fd, "\t</sat>\n");
				switch ( live_fe->getInfo()->type)
				{
					case FE_QPSK: /* satellite */
						fprintf(fd, "\t</sat>\n");
						break;
						
					case FE_QAM: /* cable */
						fprintf(fd, "\t</cable>\n");
						break;
						
					case FE_OFDM: /* satellite */
						fprintf(fd, "\t</terrestrial>\n");
						break;
						
					default:
						break;
				}
			}
			#endif

			if(fd1) 
			{
				fgets(buffer, 255, fd1);
				while(!feof(fd1)) 
				{
					fputs(buffer, fd);
					fgets(buffer, 255, fd1);
				}

				//if(!satfound) 
					fprintf(fd, "</zapit>\n");

				fclose(fd1);
			} 
			//else
			//	fprintf(fd, "</zapit>\n");
			fclose(fd);

			rename(CURRENTSERVICES_TMP, CURRENTSERVICES_XML);

			if(updated && (scanSDT == 1))
			  	eventServer->sendEvent(CZapitClient::EVT_SDT_CHANGED, CEventServer::INITID_ZAPIT);

			dprintf(DEBUG_INFO, "[sdt monitor] %s\n", updated? "found changes": "no changes found");
		}
	}

	return 0;
}

//#define CHECK_FOR_LOCK
//#endif
int zapit_main_thread(void *data)
{
	Z_start_arg *ZapStart_arg = (Z_start_arg *) data;
	
	dprintf(DEBUG_INFO, "[zapit] zapit_main_thread: starting... tid %ld\n", syscall(__NR_gettid));
	
	abort_zapit = 0;
	
	//scan for dvb adapter/frontend and feed them in map
	initFrontend();
	
	/* load frontend config */
	loadFrontendConfig();
		
	// video/audio decoder
	int video_mode = ZapStart_arg->video_mode;
	
	// video decoder
#if defined (PLATFORM_COOLSTREAM)
	videoDecoder = new cVideo(video_mode, videoDemux->getChannel(), videoDemux->getBuffer());
	videoDecoder->Standby(false);
	
	audioDecoder = new cAudio(audioDemux->getBuffer(), videoDecoder->GetTVEnc(), NULL /*videoDecoder->GetTVEncSD()*/);
	videoDecoder->SetAudioHandle(audioDecoder->GetHandle());
#else	
	videoDecoder = new cVideo();
		
	// set video system
	if(videoDecoder)
		videoDecoder->SetVideoSystem(video_mode);	
	
	// audio decoder
	audioDecoder = new cAudio();
#endif	
	
#if defined (__sh__)
	if(FrontendCount > 1)
	{
		//lib-stb-hal/libspark
		/* 
		* this is a strange hack: the drivers seem to only work correctly after
		* demux0 has been used once. After that, we can use demux1,2,... 
		*/
		struct dmx_pes_filter_params p;
		int dmx = open("/dev/dvb/adapter0/demux0", O_RDWR );
		if (dmx < 0)
			printf("%s: ERROR open /dev/dvb/adapter0/demux0 (%m)\n", __func__);
		else
		{
			memset(&p, 0, sizeof(p));
			p.output = DMX_OUT_DECODER;
			p.input  = DMX_IN_FRONTEND;
			p.flags  = DMX_IMMEDIATE_START;
			p.pes_type = DMX_PES_VIDEO;
			ioctl(dmx, DMX_SET_PES_FILTER, &p);
			ioctl(dmx, DMX_STOP);
			close(dmx);
		}
	}
#endif	

	//CI init
#if defined (ENABLE_CI)	
	ci = cDvbCi::getInstance();
#endif	
	
	//globals
	scan_runs = 0;
	found_channels = 0;
	curr_sat = 0;

	// load configuration or set defaults if no configuration file exists
	loadZapitSettings();

	//create Bouquet Manager
	g_bouquetManager = new CBouquetManager();
	
	//start channel
	if(ZapStart_arg->uselastchannel == 0)
	{
		// mode
		if (ZapStart_arg->lastchannelmode == 0)
			setRadioMode();
		else
			setTVMode();
		
		// last channel
		live_channel_id = (currentMode & RADIO_MODE) ? ZapStart_arg->startchannelradio_id & 0xFFFFFFFFFFFFULL : ZapStart_arg->startchanneltv_id & 0xFFFFFFFFFFFFULL;
		lastChannelRadio = ZapStart_arg->startchannelradio_nr;
		lastChannelTV    = ZapStart_arg->startchanneltv_nr;
	}
	else
	{
		if (config.getInt32("lastChannelMode", 0))
			setRadioMode();
		else
			setTVMode();
	}

	// load services
	prepare_channels();

	//set basic server
	CBasicServer zapit_server;

	//set zapit socket
	if (!zapit_server.prepare(ZAPIT_UDS_NAME))
		return -1;

	// init event server
	eventServer = new CEventServer;

	//create sdt thread
	pthread_create(&tsdt, NULL, sdt_thread, (void *) NULL);

	//get live channel
	tallchans_iterator cit;
	cit = allchans.find(live_channel_id);

	if(cit != allchans.end())
		live_channel = &(cit->second);
	
	//wakeup from standby and zap it to live channel
	leaveStandby(); 
	
	zapit_ready = 1;
	
	//check for lock
#ifdef CHECK_FOR_LOCK	
	bool check_lock = true;
	time_t lastlockcheck = 0;
#endif	
	
	while (zapit_server.run(zapit_parse_command, CZapitMessages::ACTVERSION, true)) 
	{
		//check for lock
#ifdef CHECK_FOR_LOCK
		if (check_lock && !standby && live_channel && time(NULL) > lastlockcheck && scan_runs == 0) 
		{
			if ( (live_fe->getStatus() & FE_HAS_LOCK) == 0) 
			{
				zapit( live_channel->getChannelID(), current_is_nvod, true);
			}
			
			lastlockcheck = time(NULL);
		}
#endif
		
		if (pmt_update_fd != -1) 
		{
			unsigned char buf[4096];
			int ret = pmtDemux->Read(buf, 4095, 10); /* every 10 msec */

			if (ret > 0) 
			{
				pmt_stop_update_filter(&pmt_update_fd);

				dprintf(DEBUG_INFO, "[zapit] pmt updated, sid 0x%x new version 0x%x\n", (buf[3] << 8) + buf[4], (buf[5] >> 1) & 0x1F);

				// zap channel
				if(live_channel) 
				{
					t_channel_id channel_id = live_channel->getChannelID();
					int vpid = live_channel->getVideoPid();
					int apid = live_channel->getAudioPid();
					
					parse_pmt(live_channel, live_fe);
					
					bool apid_found = false;
					// check if selected audio pid still present
					for (int i = 0; i <  live_channel->getAudioChannelCount(); i++) 
					{
						if (live_channel->getAudioChannel(i)->pid == apid) 
						{
							apid_found = true;
							break;
						}
					}
					
					if(!apid_found || vpid != live_channel->getVideoPid()) 
					{
						zapit(live_channel->getChannelID(), current_is_nvod, true);
					} 
					else 
					{
						sendCaPmtPlayBackStart(live_channel, live_fe);
						
						// ci cam
#if defined (ENABLE_CI)
						if(live_channel != NULL)
						{
							if(live_fe != NULL)
								ci->SendCaPMT(live_channel->getCaPmt(), live_fe->fenumber);
						}
#endif	

						pmt_set_update_filter(live_channel, &pmt_update_fd, live_fe);
					}
						
					eventServer->sendEvent(CZapitClient::EVT_PMT_CHANGED, CEventServer::INITID_ZAPIT, &channel_id, sizeof(channel_id));
				}
			}
		}

		usleep(0);
	}

	//HOUSEKEPPING
	dprintf(DEBUG_INFO, "zapit: shutdown started\n\n");

	//save audio map
	if(live_channel)
		save_channel_pids(live_channel);
	
	// save setting
	saveZapitSettings(true, true);
	
	// stop playback (stop capmt)
	stopPlayBack();
	
	// stop std thread
	pthread_cancel(tsdt);
	pthread_join(tsdt, NULL);

	if (pmtDemux)
		delete pmtDemux;
	
	if(audioDecoder)
		delete audioDecoder;
	
	if(videoDecoder)
		delete videoDecoder;

	//close frontend	
	for(fe_map_iterator_t it = femap.begin(); it != femap.end(); it++)
		delete it->second;

	zapit_ready = 0;

	dprintf(DEBUG_INFO, "zapit shutdown complete :-)\n");

	return 0;
}


