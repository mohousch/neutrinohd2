#include <algorithm>
#include <sys/time.h>
#include <fstream>
#include <iostream>

# include <plugin.h>


const std::string icecasturl = "http://dir.xiph.org/yp.xml";
const long int GET_ICECAST_TIMEOUT = 90; 		// list is about 500kB!
const long int GET_PLAYLIST_TIMEOUT = 10;
CAudioPlayerGui tmpAudioPlayerGui;

class CIceCast
{
	private:
		void addUrl2Playlist(const char *url, const char *name = NULL, const time_t bitrate = 0);
		void processPlaylistUrl(const char *url, const char *name = NULL, const time_t bitrate = 0);
		void scanXmlData(xmlDocPtr answer_parser, const char *nametag, const char *urltag, const char *bitratetag = NULL, bool usechild = false);

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

void CIceCast::addUrl2Playlist(const char *url, const char *name, const time_t bitrate) 
{
	dprintf(DEBUG_NORMAL, "CIceCast::addUrl2Playlist: name = %s, url = %s\n", name, url);
	
	CAudiofileExt mp3(url, CFile::EXTENSION_URL);
	
	if (name != NULL) 
	{
		mp3.MetaData.title = name;
	} 
	else 
	{
		std::string tmp = mp3.Filename.substr(mp3.Filename.rfind('/')+1);
		mp3.MetaData.title = tmp;
	}
	
	if (bitrate)
		mp3.MetaData.total_time = bitrate;
	else
		mp3.MetaData.total_time = 0;

	if (url[0] != '#') 
	{
		tmpAudioPlayerGui.addToPlaylist(mp3);
	}
}

void CIceCast::processPlaylistUrl(const char *url, const char *name, const time_t tim) 
{
	dprintf(DEBUG_NORMAL, "CIceCast::processPlaylistUrl\n");
	
	CURL *curl_handle;
	struct MemoryStruct chunk;
	
	chunk.memory = NULL; 	/* we expect realloc(NULL, size) to work */
	chunk.size = 0;    	/* no data at this point */

	curl_global_init(CURL_GLOBAL_ALL);

	/* init the curl session */
	curl_handle = curl_easy_init();

	/* specify URL to get */
	curl_easy_setopt(curl_handle, CURLOPT_URL, url);

	/* send all data to this function  */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

	/* some servers don't like requests that are made without a user-agent field, so we provide one */
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

	/* don't use signal for timeout */
	curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, (long)1);

	/* set timeout to 10 seconds */
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, GET_PLAYLIST_TIMEOUT);
	
	if(strcmp(g_settings.softupdate_proxyserver, "") != 0)
	{
		curl_easy_setopt(curl_handle, CURLOPT_PROXY, g_settings.softupdate_proxyserver);
		
		if(strcmp(g_settings.softupdate_proxyusername, "") != 0)
		{
			char tmp[200];
			strcpy(tmp, g_settings.softupdate_proxyusername);
			strcat(tmp, ":");
			strcat(tmp, g_settings.softupdate_proxypassword);
			curl_easy_setopt(curl_handle, CURLOPT_PROXYUSERPWD, tmp);
		}
	}

	/* get it! */
	curl_easy_perform(curl_handle);

	/* cleanup curl stuff */
	curl_easy_cleanup(curl_handle);

	/*
	* Now, our chunk.memory points to a memory block that is chunk.size
	* bytes big and contains the remote file.
	*
	* Do something nice with it!
	*
	* You should be aware of the fact that at this point we might have an
	* allocated data block, and nothing has yet deallocated that data. So when
	* you're done with it, you should free() it as a nice application.
	*/

	long res_code;
	if (curl_easy_getinfo(curl_handle, CURLINFO_HTTP_CODE, &res_code ) ==  CURLE_OK) 
	{
		if (200 == res_code) 
		{
			//printf("\nchunk = %s\n", chunk.memory);
			std::istringstream iss;
			iss.str (std::string(chunk.memory, chunk.size));
			char line[512];
			char *ptr;
			
			while (iss.rdstate() == std::ifstream::goodbit) 
			{
				iss.getline(line, 512);
				if (line[0] != '#') 
				{
					//printf("chunk: line = %s\n", line);
					ptr = strstr(line, "http://");
					if (ptr != NULL) 
					{
						char *tmp;
						// strip \n and \r characters from url
						tmp = strchr(line, '\r');
						if (tmp != NULL)
							*tmp = '\0';
						tmp = strchr(line, '\n');
						if (tmp != NULL)
							*tmp = '\0';
						
						addUrl2Playlist(ptr, name, tim);
					}
				}
			}
		}
	}

	if(chunk.memory)
		free(chunk.memory);
 
	/* we're done with libcurl, so clean it up */
	curl_global_cleanup();
}

void CIceCast::scanXmlData(xmlDocPtr answer_parser, const char *nametag, const char *urltag, const char *bitratetag, bool usechild)
{
	dprintf(DEBUG_NORMAL, "CIceCast::scanXmlData\n");
	
#define IC_typetag "server_type"

	if (answer_parser != NULL) 
	{
		xmlNodePtr element = xmlDocGetRootElement(answer_parser);
		element = element->xmlChildrenNode;
		xmlNodePtr element_tmp = element;
		
		if (element == NULL) 
		{
			dprintf(DEBUG_NORMAL, "CAudioPlayerGui::scanXmlData: No valid XML File.\n");
		} 
		else 
		{
			// count # of entries
			while (element) 
			{
				element = element->xmlNextNode;
			}
			
			element = element_tmp;
			long listPos = -1;
			
			neutrino_msg_t      msg;
			neutrino_msg_data_t data;
			
			g_RCInput->getMsg(&msg, &data, 0);
			
			while (element && msg != CRCInput::RC_home) 
			{
				char *ptr = NULL;
				char *name = NULL;
				char *url = NULL;
				char *type = NULL;
				time_t bitrate = 0;
				bool skip = true;
				listPos++;

				if (usechild) 
				{
					xmlNodePtr child = element->xmlChildrenNode;
					while (child) 
					{
						if (strcmp(xmlGetName(child), nametag) == 0)
							name = xmlGetData(child);
						else if (strcmp(xmlGetName(child), urltag) == 0)
							url = xmlGetData(child);
						else if (strcmp(xmlGetName(child), IC_typetag) == 0)
							type = xmlGetData(child);
						else if (bitratetag && strcmp(xmlGetName(child), bitratetag) == 0) {
							ptr = xmlGetData(child);
							if (ptr) 
								bitrate = atoi(ptr);
						}
						child = child->xmlNextNode;
					}

					if 	(strcmp("audio/mpeg", type) == 0) 	skip = false;
					else if (strcmp("mp3", type) == 0) 		skip = false;
					else if (strcmp("application/mp3", type) == 0) 	skip = false;
				} 
				else 
				{
					url = xmlGetAttribute(element, (char *) urltag);
					name = xmlGetAttribute(element, (char *) nametag);
					
					if (bitratetag) 
					{
						ptr = xmlGetAttribute(element, (char *) bitratetag);
						if (ptr)
							bitrate = atoi(ptr);
					}
					skip = false;
				}

				if ((url != NULL) && !skip) 
				{
					if (strstr(url, ".m3u") || strstr(url, ".pls"))
						processPlaylistUrl(url, name);
					else 
						addUrl2Playlist(url, name, bitrate);
				}
				element = element->xmlNextNode;
				g_RCInput->getMsg(&msg, &data, 0);

			}
		}
		xmlFreeDoc(answer_parser);
	}
}

void CIceCast::readDir_ic(void)
{
	dprintf(DEBUG_NORMAL, "CIceCast::readDir_ic\n");
	
	std::string answer = "";
	std::cout << "CIceCast::readDir_ic: IC URL: " << icecasturl << std::endl;
	CURL *curl_handle;
	CURLcode httpres;
	
	/* init the curl session */
	curl_handle = curl_easy_init();
	/* specify URL to get */
	curl_easy_setopt(curl_handle, CURLOPT_URL, icecasturl.c_str());
	/* send all data to this function  */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, CurlWriteToString);
	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl_handle, CURLOPT_FILE, (void *)&answer);
	/* Generate error if http error >= 400 occurs */
	curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1);
	/* set timeout to 30 seconds */
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, GET_ICECAST_TIMEOUT);
	
	if(strcmp(g_settings.softupdate_proxyserver, "")!=0)
	{
		curl_easy_setopt(curl_handle, CURLOPT_PROXY, g_settings.softupdate_proxyserver);
		
		if(strcmp(g_settings.softupdate_proxyusername, "") != 0)
		{
			char tmp[200];
			strcpy(tmp, g_settings.softupdate_proxyusername);
			strcat(tmp, ":");
			strcat(tmp, g_settings.softupdate_proxypassword);
			curl_easy_setopt(curl_handle, CURLOPT_PROXYUSERPWD, tmp);
		}
	}

	/* error handling */
	char error[CURL_ERROR_SIZE];
	curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, error);
	
	// get it!
	CHintBox *scanBox = new CHintBox(LOCALE_AUDIOPLAYER_ADD_IC, g_Locale->getText(LOCALE_AUDIOPLAYER_RECEIVING_LIST)); // UTF-8
	scanBox->paint();

	httpres = curl_easy_perform(curl_handle);

	/* cleanup curl stuff */
	curl_easy_cleanup(curl_handle);

	if (!answer.empty() && httpres == 0)
	{
		xmlDocPtr answer_parser = parseXml(answer.c_str());
		scanBox->hide();
		scanXmlData(answer_parser, "server_name", "listen_url", "bitrate", true);
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
	
	tmpAudioPlayerGui.hidePlayList(false);
	tmpAudioPlayerGui.setTitle("Ice Cast");
	tmpAudioPlayerGui.exec(NULL, "urlplayback");

	delete icecastHandler;
	icecastHandler = NULL;
}
