#include <algorithm>
#include <sys/time.h>
#include <fstream>
#include <iostream>

# include <plugin.h>


std::string icecasturl = "http://dir.xiph.org/yp.xml";
const long int GET_ICECAST_TIMEOUT = 90; 		// list is about 500kB!
CAudioPlayerGui tmpAudioPlayerGui;

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
	std::string answer = "";

	if(::getUrl(icecasturl, answer, " ", GET_ICECAST_TIMEOUT))
	{
		xmlDocPtr answer_parser = parseXml(answer.c_str());
		
		tmpAudioPlayerGui.scanXmlData(answer_parser, "server_name", "listen_url", "bitrate", true);
		tmpAudioPlayerGui.setTitle("Ice Cast");
		tmpAudioPlayerGui.setInetMode();
		tmpAudioPlayerGui.exec(NULL, "");
	}
	else
		HintBox(LOCALE_MESSAGEBOX_INFO, "can't load icecast list");
}
