#include <algorithm>
#include <sys/time.h>
#include <fstream>
#include <iostream>

# include <plugin.h>


#define RADIO_STATION_XML_FILE CONFIGDIR "/radio-stations.xml"


//
extern "C" void plugin_exec(void);
extern "C" void plugin_init(void);
extern "C" void plugin_del(void);

void plugin_init(void)
{
}

void plugin_del(void)
{
}

void plugin_exec(void)
{
	CAudioPlayerGui tmpAudioPlayerGui;

	tmpAudioPlayerGui.scanXmlFile(RADIO_STATION_XML_FILE);	
	tmpAudioPlayerGui.setTitle(g_Locale->getText(LOCALE_INETRADIO_NAME));
	tmpAudioPlayerGui.exec(NULL, "");
}

