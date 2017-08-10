#include <algorithm>
#include <sys/time.h>
#include <fstream>
#include <iostream>

# include <plugin.h>


std::string icecasturl = "http://dir.xiph.org/yp.xml";
const long int GET_ICECAST_TIMEOUT = 90; 		// list is about 500kB!
CAudioPlayerGui tmpAudioPlayerGui;

class CIceCast
{
	public:
		CIceCast();
		~CIceCast();

		void readDir_ic(void);
};

//
CIceCast::CIceCast()
{
}

CIceCast::~CIceCast()
{
}

void CIceCast::readDir_ic(void)
{
	dprintf(DEBUG_NORMAL, "CIceCast::readDir_ic\n");
	
	std::string answer = "";
	std::cout << "CIceCast::readDir_ic: IC URL: " << icecasturl << std::endl;
	
	// get it!
	CHintBox *scanBox = new CHintBox(LOCALE_AUDIOPLAYER_ADD_IC, g_Locale->getText(LOCALE_AUDIOPLAYER_RECEIVING_LIST)); // UTF-8
	scanBox->paint();

	//
	if(::getUrl(icecasturl, answer, " ", GET_ICECAST_TIMEOUT))
	{
		xmlDocPtr answer_parser = parseXml(answer.c_str());
		scanBox->hide();
		tmpAudioPlayerGui.scanXmlData(answer_parser, "server_name", "listen_url", "bitrate", true);
	}
	else 
		scanBox->hide();

	delete scanBox;
}

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
	CIceCast* icecastHandler = new CIceCast();

	icecastHandler->readDir_ic();
	
	//tmpAudioPlayerGui.hidePlayList(false);
	tmpAudioPlayerGui.setTitle("Ice Cast");
	tmpAudioPlayerGui.exec(NULL, "");

	delete icecastHandler;
	icecastHandler = NULL;
}
