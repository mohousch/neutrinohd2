/*
  $Id: test.cpp 07.02.2019 mohousch Exp $

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

#include <plugin.h>


extern "C" void plugin_exec(void);
extern "C" void plugin_init(void);
extern "C" void plugin_del(void);

class CTestMenu : public CMenuTarget
{
	private:
		// variables
		CFrameBuffer* frameBuffer;

		ClistBoxWidget * mainMenu;
		int select;

		//
		ClistBoxWidget* listMenu;
		CMenuItem* item;

		CAudioPlayerGui tmpAudioPlayerGui;
		CMoviePlayerGui tmpMoviePlayerGui;
		CPictureViewerGui tmpPictureViewerGui;

		//
		int selected;

		//
		ZapitChannelList Channels;
		//bool displayNext;

		//
		CFileFilter fileFilter;
		CFileList filelist;

		// movie
		CMovieInfo m_movieInfo;
		std::vector<MI_MOVIE_INFO> m_vMovieInfo;

		// audio
		CAudioPlayList AudioPlaylist;

		// pictures
		CPicture pic;
		CPicturePlayList PicPlaylist;

		//
		CTmdb* tmdb;
		std::string thumbnail_dir;
		CFileHelpers fileHelper;
		cYTFeedParser ytparser;
		std::string plist;
		int page;
		std::string TVShows;
		std::vector<tmdbinfo> mvlist;
		std::string tmdbsearch;

		//
		CChannelList* webTVchannelList;
		CBouquetList* webTVBouquetList;

		CButtons buttons;
		CHeaders * headers;
		CFooters *footers;

		CWidget *testWidget;

		CBox topBox;
		CFrameBox *topWidget;
		int top_selected;

		CBox leftBox;
		ClistBox *leftWidget;
		int left_selected;

		CBox rightBox;
		ClistBox *rightWidget;
		int right_selected;

		//
		CListFrame *listFrame;
		CTextBox *textWidget;
		CWindow *windowWidget;

		CBox headBox;
		CBox footBox;
		CHeaders *headersWidget;
		CFooters *footersWidget;

		CPig *pig;
		CGrid *grid;

		bool bigFonts;

		CFrameBox *leftFrame;

		ClistBox *listBox;

		void loadTMDBPlaylist(const char *txt = "movie", const char *list = "popular", const int seite = 1, bool search = false);

		void loadMoviePlaylist();
		void openMovieFileBrowser();
		void loadAudioPlaylist();
		void openAudioFileBrowser();
		void loadPicturePlaylist();
		void openPictureFileBrowser();	

		// testing
		void test();
		void widget();
		void listFrameWidget();
		void listBoxWidget();
		void textBoxWidget();
		void testWindowWidget();

		// widgets
		void testCBox();
		void testCIcon();
		void testCImage();
		void testCWindow();
		void testCWindowShadow();
		void testCWindowCustomColor();
		void testCStringInput();
		void testCStringInputSMS();
		void testCPINInput();
		void testCPLPINInput();
		void testCPINChangeWidget();
		void testCIPInput();
		void testCDateInput();
		void testCMACInput();
		void testCTimeInput();
		void testCIntInput();
		void testCInfoBox();
		void testCMessageBox();
		void testCMessageBoxInfoMsg();
		void testCMessageBoxErrorMsg();
		void testCHintBox();
		void testCHintBoxInfo();
		void testCHelpBox();
		void testCProgressBar();
		void testCProgressWindow();
		void testCButtons();
		void testVFDController();
		void testColorChooser();
		void testKeyChooser();
		void testMountChooser();
		void testCHeaders();

		//
		void testCTextBox();
		void testCListFrame();
		void testCFrameBox();

		//
		void testClistBox();
		void testClistBox2();
		void testClistBox3();
		void testClistBox4();
		void testClistBox5();
		void testClistBox6();

		//
		void testClistBoxWidget();
		void testClistBoxWidget1();
		void testClistBoxWidget2();

		//
		void testStartPlugin();

		//
		void testShowActuellEPG();

		// gui
		void testChannelSelectWidget();
		void testBEWidget();
		void testAVSelectWidget();
		void testAudioSelectWidget();
		void testDVBSubSelectWidget();
		void testAlphaSetupWidget();
		void testPSISetup();
		void testRCLock();
		void testSleepTimerWidget();
		void testMountGUI();
		void testUmountGUI();
		void testMountSmallMenu();
		void testPluginsList();

		//
		void testPlayMovieURL();
		void testPlayAudioURL();
		void testShowPictureURL();

		//
		void testPlayMovieFolder();
		void testPlayAudioFolder();
		void testShowPictureFolder();

		//
		void testPlayMovieDir();
		void testPlayAudioDir();
		void testShowPictureDir();

		//
		void testChannellist();
		void testBouquetlist();

		// new
		void spinner(void);

	public:
		CTestMenu();
		~CTestMenu();
		int exec(CMenuTarget* parent, const std::string& actionKey);

		// paint()
		void showMenu();

		// hide()
		void hide();
};

#define HEAD_BUTTONS_COUNT	3
const struct button_label HeadButtons[HEAD_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_HELP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_SETUP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_MUTE_SMALL, NONEXISTANT_LOCALE, NULL}
};

#define FOOT_BUTTONS_COUNT	4
const struct button_label FootButtons[FOOT_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_RED, NONEXISTANT_LOCALE, "next Page" },
	{ NEUTRINO_ICON_BUTTON_GREEN, NONEXISTANT_LOCALE, "prev Page" },
	{ NEUTRINO_ICON_BUTTON_YELLOW, NONEXISTANT_LOCALE, "Focus" },
	{ NEUTRINO_ICON_BUTTON_BLUE, NONEXISTANT_LOCALE, "Neue Filme" },
	
};

CTestMenu::CTestMenu()
{
	frameBuffer = CFrameBuffer::getInstance();

	selected = 0;

	//
	webTVchannelList = NULL;
	webTVBouquetList = NULL;

	//
	listMenu = NULL;
	item = NULL;

	// 
	mainMenu = NULL;
	select = -1;

	//
	bigFonts = false;

	plist = "popular";
	page = 1;
	TVShows = "movies";

	//
	testWidget = NULL;
	topWidget = NULL;
	leftWidget = NULL;
	rightWidget = NULL;
	listFrame = NULL;
	textWidget = NULL;
	windowWidget = NULL;
	pig = NULL;
	grid = NULL;
	leftFrame = NULL;

	listBox = NULL;
}

CTestMenu::~CTestMenu()
{
	Channels.clear();
	filelist.clear();
	fileFilter.clear();
	m_vMovieInfo.clear();

	if(webTVchannelList)
	{
		delete webTVchannelList;
		webTVchannelList = NULL;
	}

	if(listBox)
	{
		delete listBox;
		listBox = NULL;
	}
}

void CTestMenu::hide()
{
	frameBuffer->paintBackground();
	frameBuffer->blit();
}

void CTestMenu::loadAudioPlaylist()
{
	CHintBox loadBox("CWidget", g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
	loadBox.paint();

	fileFilter.clear();
	filelist.clear();
	AudioPlaylist.clear();

	fileFilter.addFilter("cdr");
	fileFilter.addFilter("mp3");
	fileFilter.addFilter("m2a");
	fileFilter.addFilter("mpa");
	fileFilter.addFilter("mp2");
	fileFilter.addFilter("ogg");
	fileFilter.addFilter("wav");
	fileFilter.addFilter("flac");
	fileFilter.addFilter("aac");
	fileFilter.addFilter("dts");
	fileFilter.addFilter("m4a");

	std::string Path = g_settings.network_nfs_audioplayerdir;

	if(CFileHelpers::getInstance()->readDir(Path, &filelist, &fileFilter))
	{		
		CFileList::iterator files = filelist.begin();
		for(; files != filelist.end() ; files++)
		{
			if ( (files->getExtension() == CFile::EXTENSION_CDR)
					||  (files->getExtension() == CFile::EXTENSION_MP3)
					||  (files->getExtension() == CFile::EXTENSION_WAV)
					||  (files->getExtension() == CFile::EXTENSION_FLAC))
			{
				CAudiofile audiofile(files->Name, files->getExtension());

				// refill
				std::string title;
				std::string artist;
				std::string genre;
				std::string date;
				char duration[9] = "";

				CAudioPlayer::getInstance()->init();

				int ret = CAudioPlayer::getInstance()->readMetaData(&audiofile, true);

				if (!ret || (audiofile.MetaData.artist.empty() && audiofile.MetaData.title.empty() ))
				{
					//remove extension (.mp3)
					std::string tmp = files->getFileName().substr(files->getFileName().rfind('/') + 1);
					tmp = tmp.substr(0, tmp.length() - 4);	//remove extension (.mp3)

					std::string::size_type i = tmp.rfind(" - ");
		
					if(i != std::string::npos)
					{ 
						audiofile.MetaData.title = tmp.substr(0, i);
						audiofile.MetaData.artist = tmp.substr(i + 3);
					}
					else
					{
						i = tmp.rfind('-');
						if(i != std::string::npos)
						{
							audiofile.MetaData.title = tmp.substr(0, i);
							audiofile.MetaData.artist = tmp.substr(i + 1);
						}
						else
							audiofile.MetaData.title = tmp;
					}
				}
				
				AudioPlaylist.push_back(audiofile);
			}
		}
	}

	loadBox.hide();
}

void CTestMenu::openAudioFileBrowser()
{
	CFileBrowser filebrowser((g_settings.filebrowser_denydirectoryleave) ? g_settings.network_nfs_audioplayerdir : "");

	fileFilter.clear();
	filelist.clear();

	fileFilter.addFilter("cdr");
	fileFilter.addFilter("mp3");
	fileFilter.addFilter("m2a");
	fileFilter.addFilter("mpa");
	fileFilter.addFilter("mp2");
	fileFilter.addFilter("ogg");
	fileFilter.addFilter("wav");
	fileFilter.addFilter("flac");
	fileFilter.addFilter("aac");
	fileFilter.addFilter("dts");
	fileFilter.addFilter("m4a");

	filebrowser.Multi_Select = true;
	filebrowser.Dirs_Selectable = true;
	filebrowser.Filter = &fileFilter;

	std::string Path = g_settings.network_nfs_audioplayerdir;

	if (filebrowser.exec(Path.c_str()))
	{
		Path = filebrowser.getCurrentDir();
		CFileList::const_iterator files = filebrowser.getSelectedFiles().begin();
		for(; files != filebrowser.getSelectedFiles().end(); files++)
		{
			if ( (files->getExtension() == CFile::EXTENSION_CDR)
					||  (files->getExtension() == CFile::EXTENSION_MP3)
					||  (files->getExtension() == CFile::EXTENSION_WAV)
					||  (files->getExtension() == CFile::EXTENSION_FLAC))
			{
				CAudiofile audiofile(files->Name, files->getExtension());

				// skip duplicate
				for (unsigned long i = 0; i < AudioPlaylist.size(); i++)
				{
					if(AudioPlaylist[i].Filename == audiofile.Filename)
						AudioPlaylist.erase(AudioPlaylist.begin() + i); 
				}

				// refill
				std::string title;
				std::string artist;
				std::string genre;
				std::string date;
				char duration[9] = "";

				CAudioPlayer::getInstance()->init();

				int ret = CAudioPlayer::getInstance()->readMetaData(&audiofile, true);

				if (!ret || (audiofile.MetaData.artist.empty() && audiofile.MetaData.title.empty() ))
				{
					//remove extension (.mp3)
					std::string tmp = files->getFileName().substr(files->getFileName().rfind('/') + 1);
					tmp = tmp.substr(0, tmp.length() - 4);	//remove extension (.mp3)

					std::string::size_type i = tmp.rfind(" - ");
		
					if(i != std::string::npos)
					{ 
						audiofile.MetaData.title = tmp.substr(0, i);
						audiofile.MetaData.artist = tmp.substr(i + 3);
					}
					else
					{
						i = tmp.rfind('-');
						if(i != std::string::npos)
						{
							audiofile.MetaData.title = tmp.substr(0, i);
							audiofile.MetaData.artist = tmp.substr(i + 1);
						}
						else
							audiofile.MetaData.title = tmp;
					}
				}
		
				AudioPlaylist.push_back(audiofile);
			}
		}
	}
}

void CTestMenu::loadMoviePlaylist()
{
	fileFilter.clear();
	filelist.clear();

	fileFilter.addFilter("ts");
	fileFilter.addFilter("mpg");
	fileFilter.addFilter("mpeg");
	fileFilter.addFilter("divx");
	fileFilter.addFilter("avi");
	fileFilter.addFilter("mkv");
	fileFilter.addFilter("asf");
	fileFilter.addFilter("aiff");
	fileFilter.addFilter("m2p");
	fileFilter.addFilter("mpv");
	fileFilter.addFilter("m2ts");
	fileFilter.addFilter("vob");
	fileFilter.addFilter("mp4");
	fileFilter.addFilter("mov");	
	fileFilter.addFilter("flv");	
	fileFilter.addFilter("dat");
	fileFilter.addFilter("trp");
	fileFilter.addFilter("vdr");
	fileFilter.addFilter("mts");
	fileFilter.addFilter("wmv");
	fileFilter.addFilter("wav");
	fileFilter.addFilter("flac");
	fileFilter.addFilter("mp3");
	fileFilter.addFilter("wma");
	fileFilter.addFilter("ogg");
	
	// recordingdir
	std::string Path = g_settings.network_nfs_recordingdir;
	m_vMovieInfo.clear();

	CHintBox loadBox("CWidget", g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
	loadBox.paint();
	
	//
	if(CFileHelpers::getInstance()->readDir(Path, &filelist, &fileFilter))
	{
		// filter them
		MI_MOVIE_INFO movieInfo;
		m_movieInfo.clearMovieInfo(&movieInfo); // refresh structure

		CFileList::iterator files = filelist.begin();
		for(; files != filelist.end() ; files++)
		{
			//
			m_movieInfo.clearMovieInfo(&movieInfo); // refresh structure
					
			movieInfo.file.Name = files->Name;
					
			// load movie infos (from xml file)
			m_movieInfo.loadMovieInfo(&movieInfo);

			std::string tmp_str = files->getFileName();

			removeExtension(tmp_str);

			// refill if empty
			if(movieInfo.epgTitle.empty())
				movieInfo.epgTitle = tmp_str;

			if(movieInfo.epgInfo1.empty())
				movieInfo.epgInfo1 = tmp_str;

			//if(movieInfo.epgInfo2.empty())
			//	movieInfo.epgInfo2 = tmp_str;

			//thumbnail
			std::string fname = "";
			fname = files->Name;
			changeFileNameExt(fname, ".jpg");
					
			if(!access(fname.c_str(), F_OK) )
				movieInfo.tfile = fname.c_str();
					
			// 
			m_vMovieInfo.push_back(movieInfo);
		}
	}

	loadBox.hide();
}

void CTestMenu::openMovieFileBrowser()
{
	CFileBrowser filebrowser((g_settings.filebrowser_denydirectoryleave) ? g_settings.network_nfs_recordingdir : "");

	fileFilter.clear();
	filelist.clear();

	fileFilter.addFilter("ts");
	fileFilter.addFilter("mpg");
	fileFilter.addFilter("mpeg");
	fileFilter.addFilter("divx");
	fileFilter.addFilter("avi");
	fileFilter.addFilter("mkv");
	fileFilter.addFilter("asf");
	fileFilter.addFilter("aiff");
	fileFilter.addFilter("m2p");
	fileFilter.addFilter("mpv");
	fileFilter.addFilter("m2ts");
	fileFilter.addFilter("vob");
	fileFilter.addFilter("mp4");
	fileFilter.addFilter("mov");	
	fileFilter.addFilter("flv");	
	fileFilter.addFilter("dat");
	fileFilter.addFilter("trp");
	fileFilter.addFilter("vdr");
	fileFilter.addFilter("mts");
	fileFilter.addFilter("wmv");
	fileFilter.addFilter("wav");
	fileFilter.addFilter("flac");
	fileFilter.addFilter("mp3");
	fileFilter.addFilter("wma");
	fileFilter.addFilter("ogg");

	filebrowser.Multi_Select = true;
	filebrowser.Dirs_Selectable = true;
	filebrowser.Filter = &fileFilter;

	std::string Path = g_settings.network_nfs_recordingdir;

	if (filebrowser.exec(Path.c_str()))
	{
		Path = filebrowser.getCurrentDir();

		MI_MOVIE_INFO movieInfo;
		m_movieInfo.clearMovieInfo(&movieInfo); // refresh structure

		CFileList::const_iterator files = filebrowser.getSelectedFiles().begin();
		for(; files != filebrowser.getSelectedFiles().end(); files++)
		{
			// filter them
			MI_MOVIE_INFO movieInfo;
			m_movieInfo.clearMovieInfo(&movieInfo); // refresh structure

			//
			m_movieInfo.clearMovieInfo(&movieInfo); // refresh structure
					
			movieInfo.file.Name = files->Name;
					
			// load movie infos (from xml file)
			m_movieInfo.loadMovieInfo(&movieInfo);

			std::string tmp_str = files->getFileName();

			removeExtension(tmp_str);

			// refill if empty
			if(movieInfo.epgTitle.empty())
				movieInfo.epgTitle = tmp_str;

			if(movieInfo.epgInfo1.empty())
				movieInfo.epgInfo1 = tmp_str;

			//if(movieInfo.epgInfo2.empty())
			//	movieInfo.epgInfo2 = tmp_str;

			//thumbnail
			std::string fname = "";
			fname = files->Name;
			changeFileNameExt(fname, ".jpg");
					
			if(!access(fname.c_str(), F_OK) )
				movieInfo.tfile = fname.c_str();

			// skip duplicate
			for (unsigned long i = 0; i < m_vMovieInfo.size(); i++)
			{
				if(m_vMovieInfo[i].file.getFileName() == movieInfo.file.getFileName())
					m_vMovieInfo.erase(m_vMovieInfo.begin() + i); 
			}
					
			// 
			m_vMovieInfo.push_back(movieInfo);
		}
	}
}

void CTestMenu::loadPicturePlaylist()
{
	CHintBox loadBox("CWidget", g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
	loadBox.paint();

	fileFilter.clear();
	filelist.clear();
	PicPlaylist.clear();

	fileFilter.addFilter("png");
	fileFilter.addFilter("bmp");
	fileFilter.addFilter("jpg");
	fileFilter.addFilter("jpeg");

	std::string Path = g_settings.network_nfs_picturedir;

	if(CFileHelpers::getInstance()->readDir(Path, &filelist, &fileFilter))
	{
		struct stat statbuf;
				
		CFileList::iterator files = filelist.begin();
		for(; files != filelist.end() ; files++)
		{
			if (files->getType() == CFile::FILE_PICTURE)
			{
				// fill 
				pic.Filename = files->Name;
				std::string tmp = files->Name.substr(files->Name.rfind('/') + 1);
				pic.Name = tmp.substr(0, tmp.rfind('.'));
				pic.Type = tmp.substr(tmp.rfind('.') + 1);
			
				if(stat(pic.Filename.c_str(), &statbuf) != 0)
					printf("stat error");
				pic.Date = statbuf.st_mtime;
				
				PicPlaylist.push_back(pic);
			}
		}
	}

	loadBox.hide();
}

void CTestMenu::openPictureFileBrowser()
{
	CFileBrowser filebrowser((g_settings.filebrowser_denydirectoryleave) ? g_settings.network_nfs_picturedir : "");

	fileFilter.clear();
	filelist.clear();

	fileFilter.addFilter("png");
	fileFilter.addFilter("bmp");
	fileFilter.addFilter("jpg");
	fileFilter.addFilter("jpeg");

	filebrowser.Multi_Select = true;
	filebrowser.Dirs_Selectable = true;
	filebrowser.Filter = &fileFilter;

	std::string Path = g_settings.network_nfs_picturedir;

	if (filebrowser.exec(Path.c_str()))
	{
		Path = filebrowser.getCurrentDir();
		CFileList::const_iterator files = filebrowser.getSelectedFiles().begin();
		for(; files != filebrowser.getSelectedFiles().end(); files++)
		{
			if(files->getType() == CFile::FILE_PICTURE)
			{
				CPicture pic;
				pic.Filename = files->Name;
				std::string tmp = files->Name.substr(files->Name.rfind('/') + 1);
				pic.Name = tmp.substr(0, tmp.rfind('.'));
				pic.Type = tmp.substr(tmp.rfind('.') + 1);
				struct stat statbuf;
				if(stat(pic.Filename.c_str(),&statbuf) != 0)
					printf("stat error");
				pic.Date = statbuf.st_mtime;

				// skip duplicate
				for (unsigned long i = 0; i < PicPlaylist.size(); i++)
				{
					if(PicPlaylist[i].Filename == pic.Filename)
						PicPlaylist.erase(PicPlaylist.begin() + i); 
				}
							
				PicPlaylist.push_back(pic);
			}
		}
	}
}

void CTestMenu::loadTMDBPlaylist(const char *txt, const char *list, const int seite, bool search)
{
	thumbnail_dir = "/tmp/nfilm";
	page = seite;
	plist = list;
	TVShows = txt;

	//
	tmdb = new CTmdb();
	mvlist.clear();

	fileHelper.removeDir(thumbnail_dir.c_str());
	fileHelper.createDir(thumbnail_dir.c_str(), 0755);

	CHintBox loadBox("CWidget", g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
	loadBox.paint();

	if(search)
	{
		tmdb->clearMInfo();
		tmdb->searchMovieInfo(txt);
		mvlist = tmdb->getMInfos();
	}
	else
	{
		tmdb->clearMovieList();

		tmdb->getMovieTVList(TVShows, plist, page);

		mvlist = tmdb->getMovies();
	}

	m_vMovieInfo.clear();
	
	// fill our structure
	for(unsigned int i = 0; i < mvlist.size(); i++)
	{
		MI_MOVIE_INFO movieInfo;
		m_movieInfo.clearMovieInfo(&movieInfo);

		movieInfo.epgTitle = mvlist[i].title;
		
		movieInfo.epgInfo1 = mvlist[i].overview;
		movieInfo.ytdate = mvlist[i].release_date;
		movieInfo.vote_average = mvlist[i].vote_average;
		movieInfo.vote_count = mvlist[i].vote_count;
		movieInfo.original_title = mvlist[i].original_title;
		movieInfo.release_date = mvlist[i].release_date;
		movieInfo.media_type = mvlist[i].media_type;
		movieInfo.length = mvlist[i].runtime;
		movieInfo.runtimes = mvlist[i].runtimes;
		movieInfo.genres = mvlist[i].genres;
		movieInfo.cast = mvlist[i].cast;
		movieInfo.seasons = mvlist[i].seasons;
		movieInfo.episodes = mvlist[i].episodes;
			
		std::string tname = thumbnail_dir;
		tname += "/";
		tname += movieInfo.epgTitle;
		tname += ".jpg";

		tmdb->getSmallCover(mvlist[i].poster_path, tname);

		if(!tname.empty())
			movieInfo.tfile = tname;

		// video url (the first one)
		tmdb->clearVideoInfo();
		tmdb->getVideoInfo("movie", mvlist[i].id);

		std::vector<tmdbinfo>& videoInfo_list = tmdb->getVideoInfos();

		movieInfo.vid = videoInfo_list[0].vid;
		movieInfo.vkey = videoInfo_list[0].vkey;
		movieInfo.vname = videoInfo_list[0].vname;

		m_vMovieInfo.push_back(movieInfo);
	}

	loadBox.hide();
}

void CTestMenu::widget()
{
	testWidget = new CWidget(frameBuffer->getScreenX(), frameBuffer->getScreenY(), frameBuffer->getScreenWidth(), frameBuffer->getScreenHeight());

	testWidget->enableSaveScreen();
	testWidget->setSelected(selected);
	testWidget->setBackgroundColor(COL_DARK_TURQUOISE);
	testWidget->enablePaintMainFrame();

	// head
	headBox.iWidth = frameBuffer->getScreenWidth();
	headBox.iHeight = 40;
	headBox.iX = frameBuffer->getScreenX();
	headBox.iY = frameBuffer->getScreenY();

	headersWidget = new CHeaders(headBox.iX, headBox.iY, headBox.iWidth, headBox.iHeight, "CWidget(Multi Widget)", NEUTRINO_ICON_MP3);

	headersWidget->setButtons(HeadButtons, HEAD_BUTTONS_COUNT);
	headersWidget->enablePaintDate();

	// foot
	footBox.iWidth = frameBuffer->getScreenWidth();
	footBox.iHeight = 40;
	footBox.iX = frameBuffer->getScreenX();
	footBox.iY = frameBuffer->getScreenY() + frameBuffer->getScreenHeight() - footBox.iHeight;

	footersWidget = new CFooters(footBox.iX, footBox.iY, footBox.iWidth, footBox.iHeight, FOOT_BUTTONS_COUNT, FootButtons);

	footersWidget->setCorner(RADIUS_MID, CORNER_BOTTOM);
	
	// topwidget
	topBox.iWidth = testWidget->getWindowsPos().iWidth;
	topBox.iHeight = 100;
	topBox.iX = testWidget->getWindowsPos().iX;
	topBox.iY = testWidget->getWindowsPos().iY + headBox.iHeight + INTER_FRAME_SPACE;

	top_selected = 0;

	topWidget = new CFrameBox(&topBox);
	//topWidget->setBackgroundColor(COL_DARK_TURQUOISE);
	//topWidget->setMode(FRAME_MODE_VERTICAL);

	CFrame * frame = NULL;

	frame = new CFrame("Filme");
	frame->setMenuTarget(this, "movie");
	topWidget->addFrame(frame);
	
	frame = new CFrame("Serien");
	frame->setMenuTarget(this, "tv");
	topWidget->addFrame(frame);

	topWidget->addFrame(new CFrameSeparator());

	frame = new CFrame("Suche");
	frame->setOption(tmdbsearch.c_str());
	frame->setMenuTarget(this, "search");
	topWidget->addFrame(frame);

	topWidget->setSelected(top_selected); 

	// leftWidget
	leftBox.iWidth = 200;
	leftBox.iHeight = testWidget->getWindowsPos().iHeight - headBox.iHeight - INTER_FRAME_SPACE - topBox.iHeight - 2*INTER_FRAME_SPACE - footBox.iHeight;
	leftBox.iX = testWidget->getWindowsPos().iX;
	leftBox.iY = testWidget->getWindowsPos().iY + headBox.iHeight + INTER_FRAME_SPACE + topBox.iHeight + INTER_FRAME_SPACE;

	left_selected = 0;

	leftWidget = new ClistBox(&leftBox);
	leftFrame = new CFrameBox(&leftBox);
	leftFrame->setMode(FRAME_MODE_VERTICAL);

	leftWidget->setSelected(left_selected);
//	leftWidget->enableShrinkMenu();

	ClistBoxItem *item1 = new ClistBoxItem("In den Kinos", true, NULL, this, "movie_in_cinema");
	ClistBoxItem *item2 = new ClistBoxItem("Am", true, NULL, this, "movie_popular");
	item2->setOption("populärsten");
	item2->set2lines();
	ClistBoxItem *item3 = new ClistBoxItem("Am besten", true, NULL, this, "movie_top_rated");
	item3->setOption("bewertet");
	item3->set2lines();
	ClistBoxItem *item4 = new ClistBoxItem("Neue Filme", true, NULL, this, "movie_new");
	CMenuSeparator *item5 = new CMenuSeparator();
	CMenuSeparator *item6 = new CMenuSeparator();
	CMenuSeparator *item7 = new CMenuSeparator();
	CMenuSeparator *item8 = new CMenuSeparator();
	ClistBoxItem *item9 = new ClistBoxItem("Beenden", true, NULL, this, "exit");

	CFrame *frame1 = new CFrame("in den");
	frame1->setOption("kinos");
	frame1->setMenuTarget(this, "movie_in_cinema");

	CFrame *frame2 = new CFrame("Am");
	frame2->setOption("populärsten");
	frame2->setMenuTarget(this, "movie_popular");

	CFrame *frame3 = new CFrame("am besten");
	frame3->setOption("bewertet");
	frame3->setMenuTarget(this, "movie_top_rated");

	CFrame *frame4 = new CFrameSeparator();
	CFrame *frame5 = new CFrameSeparator();

	CFrame *frame6 = new CFrame("Beenden");
	frame6->setMenuTarget(this, "exit");

	leftFrame->addFrame(frame1);
	leftFrame->addFrame(frame2);
	leftFrame->addFrame(frame3);
	leftFrame->addFrame(frame4);
	leftFrame->addFrame(frame5);
	leftFrame->addFrame(frame6);

	leftWidget->addItem(item1);
	leftWidget->addItem(new CMenuSeparator(LINE));
	leftWidget->addItem(item2);
	leftWidget->addItem(new CMenuSeparator(LINE));
	leftWidget->addItem(item3);
	leftWidget->addItem(new CMenuSeparator(LINE));
	leftWidget->addItem(item4);
	leftWidget->addItem(new CMenuSeparator(LINE));
	leftWidget->addItem(item5);
	leftWidget->addItem(item6);
	leftWidget->addItem(item7);
	leftWidget->addItem(item8);
	leftWidget->addItem(new CMenuSeparator(LINE));
	leftWidget->addItem(item9);
	leftWidget->addItem(new CMenuSeparator(LINE));

	// rightwidget
	rightBox.iWidth = testWidget->getWindowsPos().iWidth - INTER_FRAME_SPACE - leftBox.iWidth;
	rightBox.iHeight = testWidget->getWindowsPos().iHeight - headBox.iHeight - INTER_FRAME_SPACE - topBox.iHeight - 2*INTER_FRAME_SPACE - footBox.iHeight;
	rightBox.iX = testWidget->getWindowsPos().iX + leftBox.iWidth + INTER_FRAME_SPACE;
	rightBox.iY = testWidget->getWindowsPos().iY + headBox.iHeight + INTER_FRAME_SPACE + topBox.iHeight + INTER_FRAME_SPACE;

	right_selected = 0;

	//
	rightWidget = new ClistBox(&rightBox);
	rightWidget->setWidgetType(WIDGET_TYPE_FRAME);
	rightWidget->setItemsPerPage(6,2);
	rightWidget->setSelected(right_selected);
	rightWidget->enablePaintFootInfo();

	// loadPlaylist
	loadTMDBPlaylist();

	// load items
	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "yplay");

		item->setOption(m_vMovieInfo[i].epgChannel.c_str());

		item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

		item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

		item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

		item->set2lines();

		std::string tmp = m_vMovieInfo[i].epgTitle;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo1;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo2;

		item->setHelpText(tmp.c_str());

		rightWidget->addItem(item);
	}

	testWidget->addItem(headersWidget);
	testWidget->addItem(topWidget);
	testWidget->addItem(leftWidget);
	//testWidget->addItem(leftFrame);
	testWidget->addItem(rightWidget);
	testWidget->addItem(footersWidget);

	testWidget->addKey(RC_info, this, "winfo");
	//testWidget->addKey(RC_ok, this, "wok");
	testWidget->addKey(RC_red, this, "nextPage");
	testWidget->addKey(RC_green, this, "prevPage");

	testWidget->exec(NULL, "");

	delete testWidget;
	testWidget = NULL;

	delete topWidget;
	topWidget = NULL;

	delete leftWidget;
	leftWidget = NULL;

	delete rightWidget;
	rightWidget = NULL;

	delete headersWidget;
	headersWidget = NULL;

	delete footersWidget;
	footersWidget = NULL;

	delete leftFrame;
	leftFrame = NULL;
}

void CTestMenu::listFrameWidget()
{
	testWidget = new CWidget(frameBuffer->getScreenX(), frameBuffer->getScreenY(), frameBuffer->getScreenWidth(), frameBuffer->getScreenHeight());

	testWidget->setBackgroundColor(COL_DARK_TURQUOISE);

	// head
	headBox.iWidth = frameBuffer->getScreenWidth();
	headBox.iHeight = 40;
	headBox.iX = frameBuffer->getScreenX();
	headBox.iY = frameBuffer->getScreenY();

	headersWidget = new CHeaders(headBox.iX, headBox.iY, headBox.iWidth, headBox.iHeight, "CWidget(ClistFrame)", NEUTRINO_ICON_MP3);

	headersWidget->setButtons(HeadButtons, HEAD_BUTTONS_COUNT);
	headersWidget->enablePaintDate();

	// foot
	footBox.iWidth = frameBuffer->getScreenWidth();
	footBox.iHeight = 40;
	footBox.iX = frameBuffer->getScreenX();
	footBox.iY = frameBuffer->getScreenY() + frameBuffer->getScreenHeight() - footBox.iHeight;

	footersWidget = new CFooters(footBox.iX, footBox.iY, footBox.iWidth, footBox.iHeight, FOOT_BUTTONS_COUNT, FootButtons);

	footersWidget->setCorner(RADIUS_MID, CORNER_BOTTOM);

	//
	CBox listFrameBox;
	LF_LINES listFrameLines;
	int selected = 0;
	
	listFrameBox.iX = frameBuffer->getScreenX();
	listFrameBox.iY = frameBuffer->getScreenY() + headBox.iHeight;
	listFrameBox.iWidth = frameBuffer->getScreenWidth();
	listFrameBox.iHeight = frameBuffer->getScreenHeight() - headBox.iHeight - footBox.iHeight;

	//
#define MAX_ROWS 		LF_MAX_ROWS //6

	// init
	listFrameLines.rows = MAX_ROWS;

	for(int row = 0; row < MAX_ROWS; row++)
	{
		listFrameLines.lineArray[row].clear();
	}

	// rowwidth
	listFrameLines.rowWidth[0] = MAX_WINDOW_WIDTH / 20;
	listFrameLines.rowWidth[1] = MAX_WINDOW_WIDTH / 3;
	listFrameLines.rowWidth[2] = MAX_WINDOW_WIDTH / 10;
	listFrameLines.rowWidth[3] = MAX_WINDOW_WIDTH / 8;
	listFrameLines.rowWidth[4] = MAX_WINDOW_WIDTH / 5;
	listFrameLines.rowWidth[5] = MAX_WINDOW_WIDTH / 10;

	// headertitle
	listFrameLines.lineHeader[0] = "Nr";
	listFrameLines.lineHeader[1] = "title";
	listFrameLines.lineHeader[2] = "duration";
	listFrameLines.lineHeader[3] = "genre";
	listFrameLines.lineHeader[4] = "artist";
	listFrameLines.lineHeader[5] = "date";
	

	listFrame = new CListFrame(&listFrameLines, NULL, CListFrame::CListFrame::HEADER_LINE | CListFrame::SCROLL, &listFrameBox);

	// title
	//listFrame->setTitle("CWidget(ClistFrame)", NEUTRINO_ICON_MOVIE);

	loadAudioPlaylist();

	//
	int count = 0;
	for (unsigned int i = 0; i < AudioPlaylist.size(); i++)
	{
		std::string title;
		std::string artist;
		std::string genre;
		std::string date;
		char duration[9] = "";

		title = AudioPlaylist[i].MetaData.title;
		artist = AudioPlaylist[i].MetaData.artist;
		genre = AudioPlaylist[i].MetaData.genre;	
		date = AudioPlaylist[i].MetaData.date;

		snprintf(duration, 8, "(%ld:%02ld)", AudioPlaylist[i].MetaData.total_time / 60, AudioPlaylist[i].MetaData.total_time % 60);

		listFrameLines.lineArray[0].push_back(to_string(i + 1));
		listFrameLines.lineArray[1].push_back(title);
		listFrameLines.lineArray[2].push_back(duration);
		listFrameLines.lineArray[3].push_back(genre);
		listFrameLines.lineArray[4].push_back(artist);
		listFrameLines.lineArray[5].push_back(date);
	}
	
	// fill lineArrays list
	listFrame->setLines(&listFrameLines);

	// set selected line
	listFrame->setSelectedLine(selected);
	
	// paint
	listFrame->showSelection(true);

	//
	testWidget->addItem(headersWidget);
	testWidget->addItem(listFrame);
	testWidget->addItem(footersWidget);

	testWidget->addKey(RC_ok, this, "aok");

	testWidget->exec(NULL, "");

	delete testWidget;
	testWidget = NULL;

	delete listFrame;
	listFrame = NULL;

	delete headersWidget;
	headersWidget = NULL;

	delete footersWidget;
	footersWidget = NULL;
}

void CTestMenu::listBoxWidget()
{
	testWidget = new CWidget(frameBuffer->getScreenX(), frameBuffer->getScreenY(), frameBuffer->getScreenWidth(), frameBuffer->getScreenHeight());

	testWidget->setBackgroundColor(COL_DARK_TURQUOISE);
	testWidget->enableSaveScreen();

	//
	rightBox.iWidth = testWidget->getWindowsPos().iWidth;
	rightBox.iHeight = testWidget->getWindowsPos().iHeight;
	rightBox.iX = testWidget->getWindowsPos().iX;
	rightBox.iY = testWidget->getWindowsPos().iY;

	rightWidget = new ClistBox(&rightBox);

	rightWidget->setWidgetType(WIDGET_TYPE_FRAME);
	rightWidget->enableShrinkMenu();
	rightWidget->enableCenterPos();
	rightWidget->setItemsPerPage(6,2);
	rightWidget->setSelected(selected);
	rightWidget->enablePaintHead();
	rightWidget->setTitle("CWidget(ClistBox)", NEUTRINO_ICON_MP3);
	rightWidget->setHeaderButtons(HeadButtons, HEAD_BUTTONS_COUNT);
	rightWidget->enablePaintDate();
	rightWidget->enablePaintFoot();
	rightWidget->setFooterButtons(FootButtons, FOOT_BUTTONS_COUNT);
	rightWidget->enablePaintFootInfo();
	rightWidget->enableWidgetChange();
	rightWidget->addWidget(WIDGET_TYPE_STANDARD);
	rightWidget->addWidget(WIDGET_TYPE_CLASSIC);
	rightWidget->addWidget(WIDGET_TYPE_EXTENDED);

	// loadPlaylist
	loadMoviePlaylist();

	// load items
	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "mmwplay");

		item->setOption(m_vMovieInfo[i].epgChannel.c_str());

		item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

		item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

		item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

		item->set2lines();

		std::string tmp = m_vMovieInfo[i].epgTitle;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo1;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo2;

		item->setHelpText(tmp.c_str());

		rightWidget->addItem(item);
	}


	testWidget->addKey(RC_info, this, "linfo");
	testWidget->addKey(RC_setup, this, "lsetup");

	testWidget->addItem(rightWidget);

	testWidget->exec(NULL, "");

	delete testWidget;
	testWidget = NULL;

	delete rightWidget;
	rightWidget = NULL;
}

void CTestMenu::textBoxWidget()
{
	testWidget = new CWidget(frameBuffer->getScreenX(), frameBuffer->getScreenY(), frameBuffer->getScreenWidth(), frameBuffer->getScreenHeight());

	testWidget->setBackgroundColor(COL_DARK_TURQUOISE);

	// head
	headBox.iWidth = frameBuffer->getScreenWidth();
	headBox.iHeight = 40;
	headBox.iX = frameBuffer->getScreenX();
	headBox.iY = frameBuffer->getScreenY();

	headersWidget = new CHeaders(headBox.iX, headBox.iY, headBox.iWidth, headBox.iHeight, "CWidget(CTextBox)", NEUTRINO_ICON_MP3);

	headersWidget->setButtons(HeadButtons, HEAD_BUTTONS_COUNT);
	//ĥeadersWidget->setCorner(RADIUS_MID, CORNER_TOP);
	headersWidget->enablePaintDate();

	// foot
	footBox.iWidth = frameBuffer->getScreenWidth();
	footBox.iHeight = 40;
	footBox.iX = frameBuffer->getScreenX();
	footBox.iY = frameBuffer->getScreenY() + frameBuffer->getScreenHeight() - footBox.iHeight;

	footersWidget = new CFooters(footBox.iX, footBox.iY, footBox.iWidth, footBox.iHeight, FOOT_BUTTONS_COUNT, FootButtons);

	footersWidget->setCorner(RADIUS_MID, CORNER_BOTTOM);

	//textBox
	textWidget = new CTextBox(frameBuffer->getScreenX(), frameBuffer->getScreenY() + headBox.iHeight, frameBuffer->getScreenWidth(), frameBuffer->getScreenHeight() - headBox.iHeight - footBox.iHeight);
	
	const char *buffer = NULL;
	
	// prepare print buffer
	buffer = "CTextBox\ntesting CTextBox with CWidget\n";  
		
	std::string tname = PLUGINDIR "/netzkino/netzkino.png";
	
	// scale pic
	int p_w = 0;
	int p_h = 0;
	
	CFrameBuffer::getInstance()->scaleImage(tname, &p_w, &p_h);
	
	textWidget->setText(buffer, tname.c_str(), p_w, p_h);

	testWidget->addKey(RC_info, this, "txtinfo");

	testWidget->addItem(headersWidget);
	testWidget->addItem(textWidget);
	testWidget->addItem(footersWidget);

	testWidget->exec(NULL, "");

	delete testWidget;
	testWidget = NULL;

	delete textWidget;
	textWidget = NULL;

	delete headersWidget;
	headersWidget = NULL;

	delete footersWidget;
	footersWidget = NULL;
}

void CTestMenu::testWindowWidget()
{
	testWidget = new CWidget(frameBuffer->getScreenX(), frameBuffer->getScreenY(), frameBuffer->getScreenWidth(), frameBuffer->getScreenHeight());

	windowWidget = new CWindow(frameBuffer->getScreenX() + 10, frameBuffer->getScreenY() + 10, frameBuffer->getScreenWidth() - 20, frameBuffer->getScreenHeight() - 20);

	windowWidget->setColor(COL_DARK_TURQUOISE);
	windowWidget->setCorner(RADIUS_MID, CORNER_ALL);

	headersWidget = new CHeaders(frameBuffer->getScreenX() + 10, frameBuffer->getScreenY() + 10, frameBuffer->getScreenWidth() - 20, 40, "CWidget(CWindow)", NEUTRINO_ICON_MP3);

	footersWidget = new CFooters(frameBuffer->getScreenX() + 10, frameBuffer->getScreenY() + 10 + frameBuffer->getScreenHeight() - 20 - 40, frameBuffer->getScreenWidth() - 20, 40, FOOT_BUTTONS_COUNT, FootButtons);

	//
	pig = new CPig(frameBuffer->getScreenX() + 10 + frameBuffer->getScreenWidth() - 20 - 400, frameBuffer->getScreenY() + 10 + 40 + 20, 380, 260);

	// grid
	grid = new CGrid(frameBuffer->getScreenX() + 10, frameBuffer->getScreenY() + 10 + 40, frameBuffer->getScreenWidth() - 20, frameBuffer->getScreenHeight() - 20 - 80);

	headersWidget->enablePaintDate();
	headersWidget->setButtons(HeadButtons, HEAD_BUTTONS_COUNT);
	headersWidget->setColor(COL_DARK_TURQUOISE);
	//headersWidget->setCorner();
	//headersWidget->setGradient(nogradient);

	footersWidget->setColor(COL_DARK_TURQUOISE);
	//footersWidget->setCorner();
	//footersWidget->setGradient(nogradient);

	testWidget->addItem(windowWidget);
	testWidget->addItem(headersWidget);
	testWidget->addItem(footersWidget);
	testWidget->addItem(grid);
	testWidget->addItem(pig);

	testWidget->exec(NULL, "");

	delete testWidget;
	testWidget = NULL;

	delete windowWidget;
	windowWidget = NULL;

	delete headersWidget;
	headersWidget = NULL;

	delete footersWidget;
	footersWidget = NULL;

	delete pig;
	pig = NULL;

	delete grid;
	grid = NULL;
}

// CFrameBox
void CTestMenu::test()
{
	dprintf(DEBUG_NORMAL, "\ntesting multi Widgets\n");

	top_selected = 0;
	left_selected = 0;
	right_selected = 0;

	CBox headBox;
	headBox.iX = g_settings.screen_StartX + 10;
	headBox.iY = g_settings.screen_StartY + 10;
	headBox.iWidth = (g_settings.screen_EndX - g_settings.screen_StartX - 20);
	headBox.iHeight = 40;

	CBox footBox;
	footBox.iHeight = 40;
	footBox.iX = g_settings.screen_StartX + 10;
	footBox.iY = g_settings.screen_EndY - 10 - footBox.iHeight;
	footBox.iWidth = (g_settings.screen_EndX - g_settings.screen_StartX - 20);

	headers = new CHeaders(headBox, "Movie Trailer", NEUTRINO_ICON_MP3);

	headers->enablePaintDate();
	headers->setButtons(HeadButtons, HEAD_BUTTONS_COUNT);
	headers->setColor(COL_DARK_TURQUOISE);
	headers->setCorner();
	headers->setGradient(nogradient);

	footers = new CFooters(footBox, FOOT_BUTTONS_COUNT, FootButtons);

	footers->setColor(COL_DARK_TURQUOISE);
	footers->setCorner();
	footers->setGradient(nogradient);

	// top widget
	CBox topBox;
	
	topBox.iX = g_settings.screen_StartX + 10;
	topBox.iY = headBox.iY + headBox.iHeight;
	topBox.iWidth = (g_settings.screen_EndX - g_settings.screen_StartX - 20);
	topBox.iHeight = 120; //(g_settings.screen_EndY - g_settings.screen_StartY - 20);

	topWidget = new CFrameBox(&topBox);
	CFrame * frame = NULL;

	frame = new CFrame("Filme");
	topWidget->addFrame(frame);
	
	frame = new CFrame("Serien");
	topWidget->addFrame(frame);

	frame = new CFrame("Suche");
	topWidget->addFrame(frame);

	topWidget->setSelected(top_selected); 
	topWidget->setOutFocus(true);

	// leftWidget
	CBox leftBox;

	leftBox.iX = g_settings.screen_StartX + 10;
	leftBox.iY = topBox.iY + topBox.iHeight + INTER_FRAME_SPACE;
	leftBox.iWidth = 200;
	leftBox.iHeight = (g_settings.screen_EndY - g_settings.screen_StartY - 20) - headBox.iHeight - topBox.iHeight - 2*INTER_FRAME_SPACE - footBox.iHeight;

	leftWidget = new ClistBox(&leftBox);

	leftWidget->setSelected(left_selected);
	leftWidget->setOutFocus(false);
	leftWidget->enableShrinkMenu();

	leftWidget->setBackgroundColor(COL_DARK_TURQUOISE);

	ClistBoxItem *item1 = new ClistBoxItem("In den Kinos");
	ClistBoxItem *item2 = new ClistBoxItem("Am");
	item2->setOption("populärsten");
	item2->set2lines();
	ClistBoxItem *item3 = new ClistBoxItem("Am besten");
	item3->setOption("bewertet");
	item3->set2lines();
	ClistBoxItem *item4 = new ClistBoxItem("Neue Filme");
	ClistBoxItem *item5 = new ClistBoxItem(NULL, false);
	ClistBoxItem *item6 = new ClistBoxItem(NULL, false);
	ClistBoxItem *item7 = new ClistBoxItem(NULL, false);
	ClistBoxItem *item8 = new ClistBoxItem(NULL, false);
	ClistBoxItem *item9 = new ClistBoxItem("Beenden");

	leftWidget->addItem(item1);
	leftWidget->addItem(item2);
	leftWidget->addItem(item3);
	leftWidget->addItem(item4);
	leftWidget->addItem(item5);
	leftWidget->addItem(item6);
	leftWidget->addItem(item7);
	leftWidget->addItem(item8);
	leftWidget->addItem(item9);

	// right menu
	CBox rightBox;

	rightBox.iX = g_settings.screen_StartX + 10 + leftBox.iWidth + INTER_FRAME_SPACE;
	rightBox.iY = /*g_settings.screen_StartY + 10 + headBox.iHeight*/topBox.iY + topBox.iHeight + INTER_FRAME_SPACE;
	rightBox.iWidth = (g_settings.screen_EndX - g_settings.screen_StartX - 20) - leftBox.iWidth - INTER_FRAME_SPACE;
	rightBox.iHeight = (g_settings.screen_EndY - g_settings.screen_StartY - 20) - headBox.iHeight - topBox.iHeight - 2*INTER_FRAME_SPACE - footBox.iHeight;

	//
	rightWidget = new ClistBox(&rightBox);
	rightWidget->setWidgetType(WIDGET_TYPE_FRAME);
	rightWidget->setItemsPerPage(6,2);
	rightWidget->setSelected(right_selected);
	rightWidget->setOutFocus(true);

	rightWidget->setBackgroundColor(COL_LIGHT_BLUE);

	rightWidget->enablePaintFootInfo();

	enum {
		WIDGET_TOP,
		WIDGET_LEFT,
		WIDGET_RIGHT
	};
	
	int focus = WIDGET_LEFT; // frameBox

	thumbnail_dir = "/tmp/nfilm";
	page = 1;
	plist = "popular";

	//
	tmdb = new CTmdb();

	TVShows = "movie";

DOFILM:
	fileHelper.removeDir(thumbnail_dir.c_str());
	fileHelper.createDir(thumbnail_dir.c_str(), 0755);

	CHintBox loadBox("Movie Trailer", g_Locale->getText(LOCALE_MOVIEBROWSER_SCAN_FOR_MOVIES));
	loadBox.paint();

	tmdb->clearMovieList();

	tmdb->getMovieTVList(TVShows, plist, page);

	std::vector<tmdbinfo> &mvlist = tmdb->getMovies();

	m_vMovieInfo.clear();
	
	// fill our structure
	for(unsigned int i = 0; i < mvlist.size(); i++)
	{
		MI_MOVIE_INFO movieInfo;
		m_movieInfo.clearMovieInfo(&movieInfo); 

		movieInfo.epgTitle = mvlist[i].title;

		movieInfo.epgInfo1 = mvlist[i].overview;
		movieInfo.ytdate = mvlist[i].release_date;
		movieInfo.vote_average = mvlist[i].vote_average;
		movieInfo.vote_count = mvlist[i].vote_count;
		movieInfo.original_title = mvlist[i].original_title;
		movieInfo.release_date = mvlist[i].release_date;
		movieInfo.media_type = mvlist[i].media_type;
		movieInfo.length = mvlist[i].runtime;
		movieInfo.runtimes = mvlist[i].runtimes;
		movieInfo.genres = mvlist[i].genres;
		movieInfo.cast = mvlist[i].cast;
		movieInfo.seasons = mvlist[i].seasons;
		movieInfo.episodes = mvlist[i].episodes;
			
		std::string tname = thumbnail_dir;
		tname += "/";
		tname += movieInfo.epgTitle;
		tname += ".jpg";

		tmdb->getSmallCover(mvlist[i].poster_path, tname);

		if(!tname.empty())
			movieInfo.tfile = tname;

		// video url
		tmdb->clearVideoInfo();
		tmdb->getVideoInfo("movie", mvlist[i].id);

		std::vector<tmdbinfo>& videoInfo_list = tmdb->getVideoInfos();

		movieInfo.vid = videoInfo_list[0].vid;
		movieInfo.vkey = videoInfo_list[0].vkey;
		movieInfo.vname = videoInfo_list[0].vname;

		m_vMovieInfo.push_back(movieInfo);
	}

	loadBox.hide();

	// load items
	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str());

		item->setOption(m_vMovieInfo[i].epgChannel.c_str());

		item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

		item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

		item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

		rightWidget->addItem(item);
	}

REPAINT:
	// background
	bool usedBackground = CFrameBuffer::getInstance()->getuseBackground();
	if (usedBackground)
	{
		//CFrameBuffer::getInstance()->saveBackgroundImage();

		std::string fname = PLUGINDIR "/test/ard_mediathek.jpg";
		//CFrameBuffer::getInstance()->loadBackgroundPic(fname);

		CFrameBuffer::getInstance()->setBackgroundColor(COL_RED);
		CFrameBuffer::getInstance()->paintBackground();
	}

	// paint all widget
	//headers->paintHead(headBox, "Movie Trailer", NEUTRINO_ICON_MP3);
	headers->paint();
	//headers->paintFoot(footBox, FOOT_BUTTONS_COUNT, FootButtons);
	footers->paint();
	topWidget->paint();
	leftWidget->paint();
	rightWidget->paint();

	CFrameBuffer::getInstance()->blit();

	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;

	uint32_t sec_timer_id = 0;

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);

	bool loop = true;

	while(loop)
	{
		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec

		if ( (msg == NeutrinoMessages::EVT_TIMER) && (data == sec_timer_id) )
		{
			//headers->paintHead(headBox, "Movie Trailer", NEUTRINO_ICON_MP3);
			headers->paint();
		} 
		else if (msg == RC_home) 
		{
			loop = false;
		}
		else if(msg == RC_right)
		{
			if(focus == WIDGET_TOP)
				topWidget->swipRight();
			else if(focus == WIDGET_RIGHT)
				rightWidget->swipRight();
		}
		else if(msg == RC_left)
		{
			if(focus == WIDGET_TOP)
				topWidget->swipLeft();
			else if(focus == WIDGET_RIGHT)
				rightWidget->swipLeft();
		}
		else if(msg == RC_up)
		{
			if(focus == WIDGET_LEFT)
				leftWidget->scrollLineUp();
			else if(focus == WIDGET_RIGHT)
				rightWidget->scrollLineUp();
		}
		else if(msg == RC_down)
		{
			if(focus == WIDGET_LEFT)
				leftWidget->scrollLineDown();
			else if(focus == WIDGET_RIGHT)
				rightWidget->scrollLineDown();
		}
		else if(msg == RC_yellow)
		{
			if(focus == WIDGET_TOP)
			{
				focus = WIDGET_LEFT;

				//topWidget->setSelected(-1);
				topWidget->setOutFocus(true);

				//rightWidget->setSelected(-1);
				rightWidget->setOutFocus(true);

				leftWidget->setSelected(left_selected);
				leftWidget->setOutFocus(false);
			}
			else if (focus == WIDGET_LEFT)
			{
				focus = WIDGET_RIGHT;

				//leftWidget->setSelected(-1);
				leftWidget->setOutFocus(true);

				//topWidget->setSelected(-1);
				topWidget->setOutFocus(true);

				rightWidget->setSelected(right_selected);
				rightWidget->setOutFocus(false);
			}
			else if (focus == WIDGET_RIGHT)
			{
				focus = WIDGET_TOP;

				//leftWidget->setSelected(-1);
				leftWidget->setOutFocus(true);

				//rightWidget->setSelected(-1);
				rightWidget->setOutFocus(true);

				topWidget->setSelected(top_selected);
				topWidget->setOutFocus(false);
			}

			goto REPAINT;
		}
		else if(msg == RC_red)
		{
			page++;
			right_selected = 0;
			hide();
			rightWidget->clearItems();
			goto DOFILM; // include REPAINT
		}
		else if(msg == RC_green)
		{
			page--;

			if(page <= 1)
				page = 1;

			right_selected = 0;

			hide();
			rightWidget->clearItems();
			goto DOFILM; // include REPAINT
		}
		else if(msg == RC_blue)
		{
			right_selected = 0;
			left_selected = 3;
			plist = "upcoming";
			page = 1;
			hide();
			rightWidget->clearItems();
			goto DOFILM;
		}
		else if(msg == RC_ok)
		{
			if(focus == WIDGET_RIGHT)
			{
				hide();

				right_selected = rightWidget->getSelected();

				///
				ytparser.Cleanup();

				// setregion
				ytparser.SetRegion("DE");

				// set max result
				ytparser.SetMaxResults(1);
			
				// parse feed
				if (ytparser.ParseFeed(cYTFeedParser::SEARCH_BY_ID, m_vMovieInfo[right_selected].vname, m_vMovieInfo[right_selected].vkey))
				{
					yt_video_list_t &ylist = ytparser.GetVideoList();
	
					for (unsigned int j = 0; j < ylist.size(); j++) 
					{
						m_vMovieInfo[right_selected].ytid = ylist[j].id;
						m_vMovieInfo[right_selected].file.Name = ylist[j].GetUrl();
					}
				} 
				///

				if (&m_vMovieInfo[right_selected].file != NULL) 
				{
					tmpMoviePlayerGui.addToPlaylist(m_vMovieInfo[right_selected]);
					tmpMoviePlayerGui.exec(NULL, "");
				}

				goto REPAINT;
			}
			else if(focus == WIDGET_LEFT)
			{
				left_selected = leftWidget->getSelected();

				if(top_selected == 0) // movies
				{
					if(left_selected == 0)
					{
						right_selected = 0;
						plist = "now_playing";
						page = 1;
						hide();
						rightWidget->clearItems();
						goto DOFILM; // include REPAINT
					}
					else if(left_selected == 1)
					{
						right_selected = 0;
						plist = "popular";
						page = 1;
						hide();
						rightWidget->clearItems();
						goto DOFILM; // include REPAINT
					}
					else if(left_selected == 2)
					{
						right_selected = 0;
						plist = "top_rated";
						page = 1;
						hide();
						rightWidget->clearItems();
						goto DOFILM; // include REPAINT
					}
					else if(left_selected == 3)
					{
						right_selected = 0;
						plist = "upcoming";
						page = 1;
						hide();
						rightWidget->clearItems();
						goto DOFILM; // include REPAINT
					}
					else if(left_selected == 8)
						loop = false;
				}
				else if(top_selected == 1) // tv
				{
					if(left_selected == 0)
					{
						right_selected = 0;
						plist = "airing_today";
						page = 1;
						hide();
						rightWidget->clearItems();
						goto DOFILM; // include REPAINT
					}
					else if(left_selected == 1)
					{
						right_selected = 0;
						plist = "on_the_air";
						page = 1;
						hide();
						rightWidget->clearItems();
						goto DOFILM; // include REPAINT
					}
					else if(left_selected == 2)
					{
						right_selected = 0;
						plist = "popular";
						page = 1;
						hide();
						rightWidget->clearItems();
						goto DOFILM; // include REPAINT
					}
					else if(left_selected == 3)
					{
						right_selected = 0;
						plist = "top_rated";
						page = 1;
						hide();
						rightWidget->clearItems();
						goto DOFILM; // include REPAINT
					}
					else if(left_selected == 8)
						loop = false;
				}
			}
			else if(focus == WIDGET_TOP)
			{
				top_selected = topWidget->getSelected();

				if(top_selected == 1)
				{
					right_selected = 0;
					left_selected = 0;

					TVShows = "tv";
					plist = "airing_today";
					page = 1;
					hide();
					rightWidget->clearItems();
					//
					leftWidget->clearItems();
					ClistBoxItem *item1 = new ClistBoxItem("Heute auf Sendung");
					ClistBoxItem *item2 = new ClistBoxItem("Auf Sendung");
					ClistBoxItem *item3 = new ClistBoxItem("Am");
					item3->setOption("populärsten");
					item3->set2lines();
					ClistBoxItem *item4 = new ClistBoxItem("am");
					item4->setOption("besten bewertet");
					item4->set2lines();
					ClistBoxItem *item5 = new ClistBoxItem(NULL, false);
					ClistBoxItem *item6 = new ClistBoxItem(NULL, false);
					ClistBoxItem *item7 = new ClistBoxItem(NULL, false);
					ClistBoxItem *item8 = new ClistBoxItem(NULL, false);
					ClistBoxItem *item9 = new ClistBoxItem("Beenden");

					leftWidget->addItem(item1);
					leftWidget->addItem(item2);
					leftWidget->addItem(item3);
					leftWidget->addItem(item4);
					leftWidget->addItem(item5);
					leftWidget->addItem(item6);
					leftWidget->addItem(item7);
					leftWidget->addItem(item8);
					leftWidget->addItem(item9);
					//
					goto DOFILM; // include REPAINT
				}
				else if(top_selected == 0)
				{
					right_selected = 0;
					left_selected = 0;

					TVShows = "movie";
					plist = "popular";
					page = 1;
					hide();
					rightWidget->clearItems();
					leftWidget->clearItems();

					ClistBoxItem *item1 = new ClistBoxItem("In den Kinos");
					ClistBoxItem *item2 = new ClistBoxItem("Am");
					item2->setOption("populärsten");
					item2->set2lines();
					ClistBoxItem *item3 = new ClistBoxItem("Am besten");
					item3->setOption("bewertet");
					item3->set2lines();
					ClistBoxItem *item4 = new ClistBoxItem("Neue Filme");
					ClistBoxItem *item5 = new ClistBoxItem(NULL, false);
					ClistBoxItem *item6 = new ClistBoxItem(NULL, false);
					ClistBoxItem *item7 = new ClistBoxItem(NULL, false);
					ClistBoxItem *item8 = new ClistBoxItem(NULL, false);
					ClistBoxItem *item9 = new ClistBoxItem("Beenden");

					leftWidget->addItem(item1);
					leftWidget->addItem(item2);
					leftWidget->addItem(item3);
					leftWidget->addItem(item4);
					leftWidget->addItem(item5);
					leftWidget->addItem(item6);
					leftWidget->addItem(item7);
					leftWidget->addItem(item8);
					leftWidget->addItem(item9);
					
					goto DOFILM; // include REPAINT
				}
			}
		}
		else if(msg == RC_info)
		{
			if(focus == WIDGET_RIGHT)
			{
				hide();
				right_selected = rightWidget->getSelected();
				m_movieInfo.showMovieInfo(m_vMovieInfo[right_selected]);
				goto REPAINT;
			}
		}
		else if(msg == RC_page_down)
		{
			if(focus == WIDGET_RIGHT)
			{
				rightWidget->scrollPageDown();
			}
		}
		else if(msg == RC_page_up)
		{
			if(focus == WIDGET_RIGHT)
			{
				rightWidget->scrollPageUp();
			}
		}

		CFrameBuffer::getInstance()->blit();
	}

	//restore previous background
	if (usedBackground)
	{
		//CFrameBuffer::getInstance()->restoreBackgroundImage();
		//CFrameBuffer::getInstance()->useBackground(usedBackground);
		CFrameBuffer::getInstance()->setBackgroundColor(COL_BACKGROUND);
	}

	hide();

	delete topWidget;
	topWidget = NULL;

	delete leftWidget;
	leftWidget = NULL;

	delete rightWidget;
	rightWidget = NULL;

	g_RCInput->killTimer(sec_timer_id);
	sec_timer_id = 0;

	fileHelper.removeDir(thumbnail_dir.c_str());
	m_vMovieInfo.clear();
	
	delete tmdb;
	tmdb = NULL;

	if(headers)
	{
		delete headers;
		headers = NULL;
	}

	if(footers)
	{
		delete footers;
		footers = NULL;
	}
}

// CBox
void CTestMenu::testCBox()
{
	dprintf(DEBUG_NORMAL, "\ntesting CBox\n");

	CBox Box;
	
	Box.iX = g_settings.screen_StartX + 10;
	Box.iY = g_settings.screen_StartY + 10;
	Box.iWidth = (g_settings.screen_EndX - g_settings.screen_StartX - 20)/2;
	Box.iHeight = 40; //(g_settings.screen_EndY - g_settings.screen_StartY - 20);

	CFrameBuffer::getInstance()->paintBoxRel(Box.iX, Box.iY, Box.iWidth, Box.iHeight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_ALL, gradientDark2Light2Dark);

	CFrameBuffer::getInstance()->blit();

	// loop
	g_RCInput->messageLoop();

	hide();
}

// CIcon
void CTestMenu::testCIcon()
{
	dprintf(DEBUG_NORMAL, "\ntesting CIcon\n");

	//CIcon testIcon(NEUTRINO_ICON_BUTTON_RED);
	CIcon testIcon;

	// paint testIcon
	testIcon.setIcon(NEUTRINO_ICON_BUTTON_RED);

	CFrameBuffer::getInstance()->paintIcon(testIcon.iconName.c_str(), 150 + BORDER_LEFT, 150);

	CFrameBuffer::getInstance()->blit();

	// loop
	g_RCInput->messageLoop();

	hide();
}

// CImage
void CTestMenu::testCImage()
{
	dprintf(DEBUG_NORMAL, "\ntesting CImage\n");

	//CImage testImage(PLUGINDIR "/netzkino/netzkino.png");
	CImage testImage;

	// paint testImage
	testImage.setImage(PLUGINDIR "/netzkino/netzkino.png");

	CFrameBuffer::getInstance()->displayImage(testImage.imageName.c_str(), 150 + BORDER_LEFT, 150, 100, 40);

	CFrameBuffer::getInstance()->blit();

	// loop
	g_RCInput->messageLoop();

	hide();
}

// Cwindow
void CTestMenu::testCWindow()
{
	dprintf(DEBUG_NORMAL, "\ntesting CWindow\n");

	CBox Box;
	
	Box.iX = g_settings.screen_StartX + 10;
	Box.iY = g_settings.screen_StartY + 10;
	Box.iWidth = (g_settings.screen_EndX - g_settings.screen_StartX - 20);
	Box.iHeight = (g_settings.screen_EndY - g_settings.screen_StartY - 20);

	//
	CWindow* window = new CWindow();

	window->setPosition(Box.iX, Box.iY, Box.iWidth, Box.iHeight);

	window->setColor(COL_MENUHEAD_PLUS_0);
	window->setCorner(RADIUS_MID, CORNER_ALL);
	//window->setGradient(gradientDark2Light2Dark);

	window->paint();
	CFrameBuffer::getInstance()->blit();

	// loop
	g_RCInput->messageLoop();

	window->hide();
	delete window;
	window = NULL;
}

// CWindow
void CTestMenu::testCWindowShadow()
{
	dprintf(DEBUG_NORMAL, "\ntesting CWindow with shadow\n");

	CBox Box;
	
	Box.iX = g_settings.screen_StartX + 10;
	Box.iY = g_settings.screen_StartY + 10;
	Box.iWidth = (g_settings.screen_EndX - g_settings.screen_StartX - 20)/2;
	Box.iHeight = 200; //(g_settings.screen_EndY - g_settings.screen_StartY - 20);

	//
	CWindow* window = new CWindow(&Box);

	window->setColor(COL_MENUCONTENT_PLUS_0);
	window->setCorner(RADIUS_MID, CORNER_ALL);
	window->enableShadow();
	window->enableSaveScreen();

	window->paint();
	CFrameBuffer::getInstance()->blit();

	// loop
	g_RCInput->messageLoop();

	window->hide();

	delete window;
	window = NULL;
}

// custom Color
void CTestMenu::testCWindowCustomColor()
{
	dprintf(DEBUG_NORMAL, "\ntesting CWindow with custom color\n");

	CBox Box;
	
	Box.iX = g_settings.screen_StartX + 10;
	Box.iY = g_settings.screen_StartY + 10;
	Box.iWidth = (g_settings.screen_EndX - g_settings.screen_StartX - 20)/2;
	Box.iHeight = 200;

	//
	CWindow* window = new CWindow(&Box);

	window->setColor(0xFF808080);
	window->setCorner(RADIUS_MID, CORNER_ALL);
	window->enableShadow();
	window->enableSaveScreen();

	window->paint();
	CFrameBuffer::getInstance()->blit();

	// loop
	g_RCInput->messageLoop();

	window->hide();

	delete window;
	window = NULL;
}

void CTestMenu::testCHeaders()
{
	CBox headBox;
	headBox.iX = g_settings.screen_StartX + 10;
	headBox.iY = g_settings.screen_StartY + 10;
	headBox.iWidth = (g_settings.screen_EndX - g_settings.screen_StartX - 20);
	headBox.iHeight = 40;

	CBox footBox;
	footBox.iHeight = 40;
	footBox.iX = g_settings.screen_StartX + 10;
	footBox.iY = g_settings.screen_EndY - 10 - footBox.iHeight;
	footBox.iWidth = (g_settings.screen_EndX - g_settings.screen_StartX - 20);

	headers = new CHeaders(headBox, "test CHeaders", NEUTRINO_ICON_MP3);

	headers->enablePaintDate();
	headers->setButtons(HeadButtons, HEAD_BUTTONS_COUNT);
	//headers->setColor(COL_BLUE);
	//headers->setCorner();
	//headers->setGradient(nogradient);

	footers = new CFooters(footBox, FOOT_BUTTONS_COUNT, FootButtons);
	//headers->setColor(COL_BLUE);
	//headers->setCorner();
	//headers->setGradient(nogradient);

	//headers->paintHead(headBox, "test CHeaders", NEUTRINO_ICON_MP3);
	//headers->paintFoot(footBox, FOOT_BUTTONS_COUNT, FootButtons);	
	headers->paint();
	footers->paint();
	CFrameBuffer::getInstance()->blit();

	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;

	uint32_t sec_timer_id = 0;

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);

	bool loop = true;

	while(loop)
	{
		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec

		if ( (msg == NeutrinoMessages::EVT_TIMER) && (data == sec_timer_id) )
		{
			//headers->paintHead(headBox, "test CHeaders", NEUTRINO_ICON_MP3);
			headers->paint();
		} 
		else if (msg == RC_home) 
		{
			loop = false;
		}

		CFrameBuffer::getInstance()->blit();
	}
	
	hide();

	g_RCInput->killTimer(sec_timer_id);
	sec_timer_id = 0;

	if(headers)
	{
		delete headers;
		headers = NULL;
	}

	if(footers)
	{
		delete footers;
		footers = NULL;
	}
}

// CStringInput
void CTestMenu::testCStringInput()
{
	dprintf(DEBUG_NORMAL, "\ntesting CStringInput\n");

	std::string value;
	CStringInput * stringInput = new CStringInput("CStringInput", value.c_str());
	
	stringInput->exec(NULL, "");
	stringInput->hide();
	delete stringInput;
	stringInput = NULL;
	value.clear();
}

// CStringInputSMS
void CTestMenu::testCStringInputSMS()
{
	dprintf(DEBUG_NORMAL, "\ntesting CStringInputSMS\n");

	std::string value;
	CStringInputSMS * stringInputSMS = new CStringInputSMS("CStringInputSMS", value.c_str());
	
	stringInputSMS->exec(NULL, "");
	stringInputSMS->hide();
	delete stringInputSMS;
	value.clear();
}

// CPINInput
void CTestMenu::testCPINInput()
{
	dprintf(DEBUG_NORMAL, "\ntesting CPINInput\n");

	std::string value;
	CPINInput * pinInput = new CPINInput("CPINInput", value.c_str());
	
	pinInput->exec(NULL, "");
	pinInput->hide();
	delete pinInput;
	pinInput = NULL;
	value.clear();
}

// CPLPINInput
void CTestMenu::testCPLPINInput()
{
	dprintf(DEBUG_NORMAL, "\ntesting CPLPINInput\n");

	std::string value;
	CPLPINInput * pinInput = new CPLPINInput("CPLPINInput", value.c_str());
	
	pinInput->exec(NULL, "");
	pinInput->hide();
	delete pinInput;
	pinInput = NULL;
	value.clear();
}

// CPINChangeWidget
void CTestMenu::testCPINChangeWidget()
{
	dprintf(DEBUG_NORMAL, "\ntesting CPINChangeWidget\n");

	std::string value;
	CPINChangeWidget * pinInput = new CPINChangeWidget("CPINChangeWidget", value.c_str());
	
	pinInput->exec(NULL, "");
	pinInput->hide();
	delete pinInput;
	pinInput = NULL;
	value.clear();
}

// CIPInput
void CTestMenu::testCIPInput()
{
	dprintf(DEBUG_NORMAL, "\ntesting CIPInput\n");

	std::string value;
	CIPInput * ipInput = new CIPInput(LOCALE_STREAMINGMENU_SERVER_IP, value);
	
	ipInput->exec(NULL, "");
	ipInput->hide();
	delete ipInput;
	value.clear();
}

// CMACInput
void CTestMenu::testCMACInput()
{
	dprintf(DEBUG_NORMAL, "\ntesting CMACInput\n");

	std::string value;
	CMACInput * macInput = new CMACInput(LOCALE_RECORDINGMENU_SERVER_MAC, (char *)value.c_str());
	
	macInput->exec(NULL, "");
	macInput->hide();
	delete macInput;
	macInput = NULL;
	value.clear();
}

// CDateInput
void CTestMenu::testCDateInput()
{
	dprintf(DEBUG_NORMAL, "\ntesting CDateInput\n");

	time_t value;
	CDateInput * dateInput = new CDateInput(LOCALE_FILEBROWSER_SORT_DATE, &value);
	
	dateInput->exec(NULL, "");
	dateInput->hide();
	delete dateInput;
}

// CTimeInput
void CTestMenu::testCTimeInput()
{
	dprintf(DEBUG_NORMAL, "\ntesting CTimeInput\n");

	std::string value;
	CTimeInput * timeInput = new CTimeInput(LOCALE_FILEBROWSER_SORT_DATE, (char *)value.c_str());
	
	timeInput->exec(NULL, "");
	timeInput->hide();
	delete timeInput;
	timeInput = NULL;
	value.clear();
}

// CIntInput
void CTestMenu::testCIntInput()
{
	dprintf(DEBUG_NORMAL, "\ntesting CIntInput\n");

	int value;
	CIntInput * intInput = new CIntInput(LOCALE_FILEBROWSER_SORT_DATE, value);
	
	intInput->exec(NULL, "");
	intInput->hide();
	delete intInput;
	intInput = NULL;	
}

// CInfoBox
void CTestMenu::testCInfoBox()
{
	dprintf(DEBUG_NORMAL, "\ntesting CInfoBox\n");

	std::string buffer;
	
	// prepare print buffer  
	buffer = "CInfoBox";
	buffer += "\n";
	buffer += "testing CInfoBox";
	buffer += "\n";

	// thumbnail
	std::string tname = PLUGINDIR "/netzkino/netzkino.png";
	
	// scale pic
	int p_w = 0;
	int p_h = 0;

	CFrameBuffer::getInstance()->scaleImage(tname, &p_w, &p_h);
		
	
	CBox position(g_settings.screen_StartX + 50, g_settings.screen_StartY + 50, g_settings.screen_EndX - g_settings.screen_StartX - 100, g_settings.screen_EndY - g_settings.screen_StartY - 100); 
	
	CInfoBox * infoBox = new CInfoBox(g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1], SCROLL, &position, "CInfoBox", g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE], NEUTRINO_ICON_INFO);

	infoBox->setText(buffer.c_str(), tname.c_str(), p_w, p_h);
	infoBox->exec();
	delete infoBox;
	infoBox = NULL;
}

// CMessageBox
void CTestMenu::testCMessageBox()
{
	dprintf(DEBUG_NORMAL, "\ntesting CMessageBox\n");

	CMessageBox * messageBox = new CMessageBox(LOCALE_MESSAGEBOX_INFO, "testing CMessageBox"/*, 600, NEUTRINO_ICON_INFO, mbrYes, mbNone*/);
	
	int res = messageBox->exec();

	printf("res:%d messageBox->result:%d\n", res, messageBox->result);

	delete messageBox;
	messageBox = NULL;
}

// MessageBox
void CTestMenu::testCMessageBoxInfoMsg()
{
	dprintf(DEBUG_NORMAL, "\ntesting MessageBox\n");

	MessageBox(LOCALE_MESSAGEBOX_INFO, "testing CMessageBoxInfoMsg", mbrBack, mbBack, NEUTRINO_ICON_INFO);
}

// MessageBox
void CTestMenu::testCMessageBoxErrorMsg()
{
	dprintf(DEBUG_NORMAL, "\ntesting MessageBox\n");

	MessageBox(LOCALE_MESSAGEBOX_ERROR, "testing CMessageBoxErrorMsg", mbrCancel, mbCancel, NEUTRINO_ICON_ERROR);
}

// CHintBox
void CTestMenu::testCHintBox()
{
	dprintf(DEBUG_NORMAL, "\ntesting CHintBox\n");

	CHintBox * hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, "testing CHintBox");
	
	hintBox->exec(10);

	delete hintBox;
	hintBox = NULL;
}

// HintBox
void CTestMenu::testCHintBoxInfo()
{
	dprintf(DEBUG_NORMAL, "\ntesting HintBox\n");

	HintBox(LOCALE_MESSAGEBOX_INFO, "testing CHintBoxInfo", HINTBOX_WIDTH, 10, NEUTRINO_ICON_INFO);
}

// CHelpBox
void CTestMenu::testCHelpBox()
{
	dprintf(DEBUG_NORMAL, "\ntesting CHelpBox\n");

	CHelpBox * helpBox = new CHelpBox();
	
	// text
	helpBox->addLine("helpBox");

	// icon|text
	helpBox->addLine(NEUTRINO_ICON_BUTTON_RED, "Huhu :-P", g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1], COL_RED0, true);

	//
	helpBox->addLine(NEUTRINO_ICON_BUTTON_GREEN, "Huhu :-)", g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1], COL_BLUE0, true);

	helpBox->addSeparator();

	//
	helpBox->addLine("neutrinoHD2 the best GUI :-P", g_Font[SNeutrinoSettings::FONT_TYPE_MENU], COL_MENUCONTENTINACTIVE, true);
	

	//
	helpBox->addSeparator();
	
	helpBox->add2Line("Gui: ", "neutrinoHD2 the best GUI :-P", g_Font[SNeutrinoSettings::FONT_TYPE_MENU], COL_MENUCONTENTINACTIVE, true, g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1], COL_BLUE0, true);

	// icon
	helpBox->addLine(NEUTRINO_ICON_BUTTON_YELLOW, "Huhu :-P", g_Font[SNeutrinoSettings::FONT_TYPE_MENU], COL_GREEN0, true);

	helpBox->addSeparator();

	//
	helpBox->addLine("Huhu :-)");

	//
	helpBox->addLine(NEUTRINO_ICON_BUTTON_BLUE, "neutrinoHD2 the best GUI :-P", g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1], COL_YELLOW0, true);

	helpBox->addSeparator();

	helpBox->addLine(NEUTRINO_ICON_HDD, "neutrinoHD2 the best GUI :-P", g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1], COL_YELLOW0, true);

	helpBox->addLine(NEUTRINO_ICON_TUNER, "neutrinoHD2 the best GUI :-P", g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1], COL_YELLOW0, true);

	helpBox->show(LOCALE_MESSAGEBOX_INFO, HELPBOX_WIDTH, -1, mbrBack, mbNone);
	
	delete helpBox;
	helpBox = NULL;
}

// CTextBox
void CTestMenu::testCTextBox()
{
	dprintf(DEBUG_NORMAL, "\ntesting CTextBox\n");

	CBox Box;
	
	Box.iX = g_settings.screen_StartX + 10;
	Box.iY = g_settings.screen_StartY + 10;
	Box.iWidth = g_settings.screen_EndX - g_settings.screen_StartX - 20;
	Box.iHeight = (g_settings.screen_EndY - g_settings.screen_StartY - 20);
	
	CTextBox * textBox = new CTextBox();

	textBox->setPosition(&Box);
	
	const char * buffer = NULL;
	
	// prepare print buffer  
	buffer = "CTextBox\ntesting CTextBox\n";
		
	std::string tname = PLUGINDIR "/netzkino/netzkino.png";
	
	// scale pic
	int p_w = 0;
	int p_h = 0;
	
	CFrameBuffer::getInstance()->scaleImage(tname, &p_w, &p_h);
	
	textBox->setText(buffer, tname.c_str(), p_w, p_h);
	
	textBox->paint();
	CFrameBuffer::getInstance()->blit();
	
	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
	bool loop = true;

	while(loop)
	{
		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg == RC_home) 
		{
			loop = false;
		}
		else if(msg == RC_info)
		{
			bigFonts = bigFonts? false : true;
			textBox->setBigFonts(bigFonts);
		}

		CFrameBuffer::getInstance()->blit();
	}

	textBox->hide();
	
	delete textBox;
	textBox = NULL;
}

// ClistFrame
void CTestMenu::testCListFrame()
{
	dprintf(DEBUG_NORMAL, "\ntesting ClistFrame\n");

	CBox listFrameBox;
	LF_LINES listFrameLines;
	int selected = 0;
	
	listFrameBox.iX = g_settings.screen_StartX + 10;
	listFrameBox.iY = g_settings.screen_StartY + 10;
	listFrameBox.iWidth = w_max ( (CFrameBuffer::getInstance()->getScreenWidth() / 20 * 17), (CFrameBuffer::getInstance()->getScreenWidth() / 20 ));
	listFrameBox.iHeight = h_max ( (CFrameBuffer::getInstance()->getScreenHeight() / 20 * 16), (CFrameBuffer::getInstance()->getScreenHeight() / 20));

	// recalculate x and y
	listFrameBox.iX = CFrameBuffer::getInstance()->getScreenX() + ((CFrameBuffer::getInstance()->getScreenWidth() - (listFrameBox.iWidth)) / 2);
	listFrameBox.iY = CFrameBuffer::getInstance()->getScreenY() + ((CFrameBuffer::getInstance()->getScreenHeight() - listFrameBox.iHeight) / 2);

	//
	#define MAX_ROWS 		LF_MAX_ROWS //6

	

	// init
	listFrameLines.rows = MAX_ROWS;

	for(int row = 0; row < MAX_ROWS; row++)
	{
		listFrameLines.lineArray[row].clear();
	}

	// rowwidth
	listFrameLines.rowWidth[0] = MAX_WINDOW_WIDTH / 20;
	listFrameLines.rowWidth[1] = MAX_WINDOW_WIDTH / 4;
	listFrameLines.rowWidth[2] = MAX_WINDOW_WIDTH / 12;
	listFrameLines.rowWidth[3] = MAX_WINDOW_WIDTH / 8;
	listFrameLines.rowWidth[4] = MAX_WINDOW_WIDTH / 5;
	listFrameLines.rowWidth[5] = MAX_WINDOW_WIDTH / 10;

	// headertitle
	listFrameLines.lineHeader[0] = "Nr";
	listFrameLines.lineHeader[1] = "title";
	listFrameLines.lineHeader[2] = "duration";
	listFrameLines.lineHeader[3] = "genre";
	listFrameLines.lineHeader[4] = "artist";
	listFrameLines.lineHeader[5] = "date";
	

	CListFrame * listFrame = new CListFrame(&listFrameLines, NULL, CListFrame::TITLE | CListFrame::HEADER_LINE | CListFrame::SCROLL, &listFrameBox);

	// title
	listFrame->setTitle("listFrame (AudioPlayer)", NEUTRINO_ICON_MOVIE);
	
	// fill lineArrays list
	loadAudioPlaylist();

	//
	int count = 0;
	for (unsigned int i = 0; i < AudioPlaylist.size(); i++)
	{
		std::string title;
		std::string artist;
		std::string genre;
		std::string date;
		char duration[9] = "";

		title = AudioPlaylist[i].MetaData.title;
		artist = AudioPlaylist[i].MetaData.artist;
		genre = AudioPlaylist[i].MetaData.genre;	
		date = AudioPlaylist[i].MetaData.date;

		snprintf(duration, 8, "(%ld:%02ld)", AudioPlaylist[i].MetaData.total_time / 60, AudioPlaylist[i].MetaData.total_time % 60);

		listFrameLines.lineArray[0].push_back(to_string(i + 1));
		listFrameLines.lineArray[1].push_back(title);
		listFrameLines.lineArray[2].push_back(duration);
		listFrameLines.lineArray[3].push_back(genre);
		listFrameLines.lineArray[4].push_back(artist);
		listFrameLines.lineArray[5].push_back(date);
	}
	//
	listFrame->setLines(&listFrameLines);
	
	// paint
	listFrame->paint();
	listFrame->showSelection(true);

	CFrameBuffer::getInstance()->blit();

	CAudioPlayer::getInstance()->init();
	
	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;

	bool loop = true;

REPEAT:
	listFrame->refresh();
	while(loop)
	{
		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg == RC_home) 
		{
			//
			if (CAudioPlayer::getInstance()->getState() != CBaseDec::STOP)
			{
				CAudioPlayer::getInstance()->stop();
			}

			//
			for(int i = 0; i < MAX_ROWS; i++)
			{
				listFrameLines.lineArray[i].clear();
			}

			listFrame->hide();

			loop = false;
		}
		else if(msg == RC_down)
		{
			listFrame->scrollLineDown(1);
			listFrame->refresh();
		}
		else if(msg == RC_up)
		{
			listFrame->scrollLineUp(1);
			listFrame->refresh();
		}
		else if(msg == RC_page_down)
		{
			listFrame->scrollPageDown(1);
			listFrame->refresh();
		}
		else if(msg == RC_page_up)
		{
			listFrame->scrollPageUp(1);
			listFrame->refresh();
		}
		else if(msg == RC_ok)
		{
			selected = listFrame->getSelectedLine();

			for (unsigned int i = 0; i < AudioPlaylist.size(); i++)
			{
				tmpAudioPlayerGui.addToPlaylist(AudioPlaylist[i]);
			}

			tmpAudioPlayerGui.setCurrent(selected);
			tmpAudioPlayerGui.exec(NULL, "");

			//listFrame->refresh();
			goto REPEAT;
		}

		//listFrame->refresh();
		CFrameBuffer::getInstance()->blit();
	}
	
	delete listFrame;
	listFrame = NULL;
}

// CProgressBar
void CTestMenu::testCProgressBar()
{
	dprintf(DEBUG_NORMAL, "\ntesting CProgressBar\n");

	CProgressBar *timescale = NULL;
	
	CBox Box;
	
	Box.iX = g_settings.screen_StartX + 10;
	Box.iY = g_settings.screen_StartY + 10;
	Box.iWidth = (g_settings.screen_EndX - g_settings.screen_StartX - 20);
	Box.iHeight = (g_settings.screen_EndY - g_settings.screen_StartY - 20)/40;
	
	timescale = new CProgressBar(Box.iWidth, Box.iHeight);
	timescale->reset();
	
	timescale->paint(Box.iX, Box.iY, 10);
	CFrameBuffer::getInstance()->blit();
	usleep(1000000);
	timescale->paint(Box.iX, Box.iY, 20);
	CFrameBuffer::getInstance()->blit();
	usleep(1000000);
	timescale->paint(Box.iX, Box.iY, 30);
	CFrameBuffer::getInstance()->blit();
	usleep(1000000);
	timescale->paint(Box.iX, Box.iY, 40);
	CFrameBuffer::getInstance()->blit();
	usleep(1000000);
	timescale->paint(Box.iX, Box.iY, 50);
	CFrameBuffer::getInstance()->blit();
	usleep(1000000);
	timescale->paint(Box.iX, Box.iY, 60);
	CFrameBuffer::getInstance()->blit();
	usleep(1000000);
	timescale->paint(Box.iX, Box.iY, 70);
	CFrameBuffer::getInstance()->blit();
	usleep(1000000);
	timescale->paint(Box.iX, Box.iY, 80);
	CFrameBuffer::getInstance()->blit();
	usleep(1000000);
	timescale->paint(Box.iX, Box.iY, 90);
	CFrameBuffer::getInstance()->blit();
	usleep(1000000);
	timescale->paint(Box.iX, Box.iY, 100);
	CFrameBuffer::getInstance()->blit();
	
	delete timescale;
	timescale = NULL;
	//
	hide();
}

// CProgressWindow
void CTestMenu::testCProgressWindow()
{
	dprintf(DEBUG_NORMAL, "\ntesting CProgressWindow\n");

	CProgressWindow * progress;
	
	progress = new CProgressWindow();
	progress->setTitle("CProgressWindow");
	progress->exec(NULL, "");
	
	progress->showStatusMessageUTF("testing CProgressWindow");
	progress->showGlobalStatus(0);
	usleep(1000000);
	progress->showGlobalStatus(10);
	usleep(1000000);
	progress->showGlobalStatus(20);
	usleep(1000000);
	progress->showGlobalStatus(30);
	usleep(1000000);
	progress->showGlobalStatus(40);
	usleep(1000000);
	progress->showGlobalStatus(50);
	usleep(1000000);
	progress->showGlobalStatus(60);
	usleep(1000000);
	progress->showGlobalStatus(70);
	usleep(1000000);
	progress->showGlobalStatus(80);
	usleep(1000000);
	progress->showGlobalStatus(90);
	usleep(1000000);
	progress->showGlobalStatus(100);
	usleep(1000000);
	
	progress->hide();
	delete progress;
	progress = NULL;
        
}

// CButtons
void CTestMenu::testCButtons()
{
	dprintf(DEBUG_NORMAL, "\ntesting CButtons\n");

	int icon_w, icon_h;
	CFrameBuffer::getInstance()->getIconSize(NEUTRINO_ICON_BUTTON_RED, &icon_w, &icon_h);
	buttons.paintFootButtons(g_settings.screen_StartX + 50 + BORDER_LEFT, g_settings.screen_StartY + 50, (g_settings.screen_EndX - g_settings.screen_StartX - 100), icon_h, FOOT_BUTTONS_COUNT, FootButtons);

	CFrameBuffer::getInstance()->blit();

	// loop
	g_RCInput->messageLoop();
	hide();
}

// ClistBox(standard)
void CTestMenu::testClistBox()
{
	dprintf(DEBUG_NORMAL, "\ntesting ClistBox(standard)\n");

	CBox Box;
	
	Box.iWidth = g_settings.screen_EndX - g_settings.screen_StartX - 20;
	Box.iHeight = g_settings.screen_EndY - g_settings.screen_StartY - 20;

	Box.iX = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - Box.iWidth ) >> 1 );
	Box.iY = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - Box.iHeight) >> 1 );

	listBox = new ClistBox(&Box);

	item = NULL;
	
	loadMoviePlaylist();

	// load items
	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "listplay");

		item->setOption(m_vMovieInfo[i].epgChannel.c_str());
		//item->setOptionInfo("OptionInfo");

		item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());
		//item->setOptionInfo1("OptionInfo1");

		item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());
		//item->setOptionInfo2("OptionInfo2");

		item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

		item->set2lines();

		listBox->addItem(item);

		std::string tmp = m_vMovieInfo[i].epgTitle;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo1;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo2;

		item->setHelpText(tmp.c_str());
	}

	// mode
	listBox->setWidgetType(WIDGET_TYPE_STANDARD);
	listBox->setWidgetMode(MODE_LISTBOX);
	listBox->enableCenterPos();

	// head
	listBox->setTitle("ClistBox(standard)", NEUTRINO_ICON_MOVIE);
	listBox->enablePaintHead();
	listBox->setHeaderButtons(HeadButtons, HEAD_BUTTONS_COUNT);
	listBox->enablePaintDate();

	// footer
	listBox->enablePaintFoot();
	listBox->setFooterButtons(FootButtons, FOOT_BUTTONS_COUNT);

	// footinfo
	listBox->enablePaintFootInfo(80);

	listBox->setSelected(selected);
	listBox->paint();

	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
	uint32_t sec_timer_id = 0;

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);

	bool loop = true;

	while(loop)
	{
		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec

		listBox->otherKeyPressed(msg);

		if ( (msg == NeutrinoMessages::EVT_TIMER) && (data == sec_timer_id) )
		{
			listBox->paintHead();
		} 
		else if (msg == RC_home) 
		{
			loop = false;
		}
		else if(msg == RC_ok)
		{
			listBox->oKKeyPressed(this);
			listBox->paint();
		}
		else if(msg == RC_info)
		{
			listBox->hide();

			selected = listBox->getSelected();
			m_movieInfo.showMovieInfo(m_vMovieInfo[selected]);

			listBox->paint();
		}

		CFrameBuffer::getInstance()->blit();
	}

	listBox->hide();
	delete listBox;
	listBox = NULL;

	g_RCInput->killTimer(sec_timer_id);
	sec_timer_id = 0;
}

// ClistBox(classic)
void CTestMenu::testClistBox2()
{
	dprintf(DEBUG_NORMAL, "\ntesting ClistBox(classic)\n");

	CBox Box;
	
	Box.iWidth = g_settings.screen_EndX - g_settings.screen_StartX - 20;
	Box.iHeight = g_settings.screen_EndY - g_settings.screen_StartY - 20;

	Box.iX = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - Box.iWidth ) >> 1 );
	Box.iY = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - Box.iHeight) >> 1 );

	listBox = new ClistBox(&Box);

	item = NULL;

	loadMoviePlaylist();

	// load items
	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "listplay");

		item->setOption(m_vMovieInfo[i].epgChannel.c_str());
		//item->setOptionInfo("OptionInfo");

		item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());
		//item->setOptionInfo1("OptionInfo1");

		item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());
		//item->setOptionInfo2("OptionInfo2");

		item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

		item->set2lines();

		std::string tmp = m_vMovieInfo[i].epgTitle;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo1;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo2;

		item->setHelpText(tmp.c_str());

		listBox->addItem(item);
	}

	// widgettype
	listBox->setWidgetType(WIDGET_TYPE_CLASSIC);
	listBox->setWidgetMode(MODE_LISTBOX);
	listBox->enableCenterPos();

	// head
	listBox->setTitle("ClistBox(classic)", NEUTRINO_ICON_MOVIE);
	listBox->enablePaintHead();
	listBox->setHeaderButtons(HeadButtons, HEAD_BUTTONS_COUNT);
	listBox->enablePaintDate();

	// footer
	listBox->enablePaintFoot();
	listBox->setFooterButtons(FootButtons, FOOT_BUTTONS_COUNT);

	// footinfo
	listBox->enablePaintFootInfo(80);

	listBox->setSelected(selected);
	listBox->paint();

	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
	uint32_t sec_timer_id = 0;

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);

	bool loop = true;

	while(loop)
	{
		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec

		listBox->otherKeyPressed(msg);

		if ( (msg == NeutrinoMessages::EVT_TIMER) && (data == sec_timer_id) )
		{
			listBox->paintHead();
		} 
		else if (msg == RC_home) 
		{
			loop = false;
		}
		else if(msg == RC_ok)
		{
			listBox->oKKeyPressed(this);
			listBox->paint();
		}
		else if(msg == RC_info)
		{
			listBox->hide();

			selected = listBox->getSelected();
			m_movieInfo.showMovieInfo(m_vMovieInfo[selected]);
			
			listBox->paint();
		}

		CFrameBuffer::getInstance()->blit();
	}

	listBox->hide();
	delete listBox;
	listBox = NULL;

	g_RCInput->killTimer(sec_timer_id);
	sec_timer_id = 0;
}

// ClistBox(extended)
void CTestMenu::testClistBox3()
{
	dprintf(DEBUG_NORMAL, "\ntesting ClistBox(extended)\n");

	CBox Box;
	
	Box.iWidth = g_settings.screen_EndX - g_settings.screen_StartX - 20;
	Box.iHeight = g_settings.screen_EndY - g_settings.screen_StartY - 20;

	Box.iX = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - Box.iWidth ) >> 1 );
	Box.iY = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - Box.iHeight) >> 1 );

	listBox = new ClistBox(&Box);
	item = NULL;

	loadMoviePlaylist();

	// load items
	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "listplay");

		item->setOption(m_vMovieInfo[i].epgChannel.c_str());
		//item->setOptionInfo("OptionInfo");

		item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());
		//item->setOptionInfo1("OptionInfo1");

		item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());
		//item->setOptionInfo2("OptionInfo2");

		item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

		item->set2lines();

		std::string tmp = m_vMovieInfo[i].epgTitle;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo1;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo2;

		item->setHelpText(tmp.c_str());

		listBox->addItem(item);
	}

	// widgettype
	listBox->setWidgetType(WIDGET_TYPE_EXTENDED);
	listBox->setWidgetMode(MODE_LISTBOX);
	listBox->enableCenterPos();

	// head
	listBox->setTitle("ClistBox(extended)", NEUTRINO_ICON_MOVIE);
	listBox->enablePaintHead();
	listBox->setHeaderButtons(HeadButtons, HEAD_BUTTONS_COUNT);
	listBox->enablePaintDate();

	// footer
	listBox->enablePaintFoot();
	listBox->setFooterButtons(FootButtons, FOOT_BUTTONS_COUNT);

	// footinfo
	listBox->enablePaintFootInfo(80);

	listBox->setSelected(selected);
	listBox->paint();

	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
	uint32_t sec_timer_id = 0;

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);

	bool loop = true;

	while(loop)
	{
		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec

		listBox->otherKeyPressed(msg);

		if ( (msg == NeutrinoMessages::EVT_TIMER) && (data == sec_timer_id) )
		{
			listBox->paintHead();
		} 
		else if (msg == RC_home) 
		{
			loop = false;
		}
		else if(msg == RC_ok)
		{
			listBox->oKKeyPressed(this);
			listBox->paint();
		}
		else if(msg == RC_info)
		{
			listBox->hide();

			selected = listBox->getSelected();
			m_movieInfo.showMovieInfo(m_vMovieInfo[selected]);
			
			listBox->paint();
		}

		CFrameBuffer::getInstance()->blit();
	}

	listBox->hide();
	delete listBox;
	listBox = NULL;

	g_RCInput->killTimer(sec_timer_id);
	sec_timer_id = 0;
}

// ClistBox(frame)
void CTestMenu::testClistBox4()
{
	dprintf(DEBUG_NORMAL, "\ntesting ClistBox(frame)\n");

	CBox Box;
	
	Box.iWidth = g_settings.screen_EndX - g_settings.screen_StartX - 20;
	Box.iHeight = g_settings.screen_EndY - g_settings.screen_StartY - 20;

	Box.iX = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - Box.iWidth ) >> 1 );
	Box.iY = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - Box.iHeight) >> 1 );

	listBox = new ClistBox(&Box);
	item = NULL;

	loadMoviePlaylist();

	// load items
	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "listplay");

		item->setOption(m_vMovieInfo[i].epgChannel.c_str());

		item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

		item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

		item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

		item->set2lines();

		std::string tmp = m_vMovieInfo[i].epgTitle;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo1;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo2;

		item->setHelpText(tmp.c_str());

		listBox->addItem(item);
	}

	// widgettype
	listBox->setWidgetType(WIDGET_TYPE_FRAME);
	listBox->setItemsPerPage(6,2);
	listBox->setWidgetMode(MODE_LISTBOX);
	listBox->enableCenterPos();

	// head
	listBox->setTitle("ClistBox(Frame)", NEUTRINO_ICON_MOVIE);
	listBox->enablePaintHead();
	listBox->setHeaderButtons(HeadButtons, HEAD_BUTTONS_COUNT);
	listBox->enablePaintDate();

	// footer
	listBox->enablePaintFoot();
	listBox->setFooterButtons(FootButtons, FOOT_BUTTONS_COUNT);

	// footinfo
	listBox->enablePaintFootInfo(80);

	listBox->setSelected(selected);
	listBox->paint();

	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
	uint32_t sec_timer_id = 0;

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);

	bool loop = true;

	while(loop)
	{
		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec

		listBox->otherKeyPressed(msg);

		if ( (msg == NeutrinoMessages::EVT_TIMER) && (data == sec_timer_id) )
		{
			listBox->paintHead();
		} 
		else if (msg == RC_home) 
		{
			loop = false;
		}
		else if(msg == RC_ok)
		{
			listBox->oKKeyPressed(this);
			listBox->paint();
		}
		else if(msg == RC_info)
		{
			listBox->hide();

			selected = listBox->getSelected();
			m_movieInfo.showMovieInfo(m_vMovieInfo[selected]);
			
			listBox->paint();
		}

		CFrameBuffer::getInstance()->blit();
	}

	listBox->hide();
	delete listBox;
	listBox = NULL;

	g_RCInput->killTimer(sec_timer_id);
	sec_timer_id = 0;
}

// ClistBox(mode menu)
void CTestMenu::testClistBox5()
{
	dprintf(DEBUG_NORMAL, "\ntesting ClistBox(menu mode)\n");

	CBox Box;
	
	Box.iWidth = g_settings.screen_EndX - g_settings.screen_StartX - 20;
	Box.iHeight = g_settings.screen_EndY - g_settings.screen_StartY - 20;

	Box.iX = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - Box.iWidth ) >> 1 );
	Box.iY = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - Box.iHeight) >> 1 );

	listBox = new ClistBox(&Box);
	item = NULL;

	loadMoviePlaylist();

	// load items
	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "listplay");

		item->setOption(m_vMovieInfo[i].epgChannel.c_str());
		//item->setOptionInfo("OptionInfo");

		item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());
		//item->setOptionInfo1("OptionInfo1");

		item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());
		//item->setOptionInfo2("OptionInfo2");

		item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

		item->set2lines();

		listBox->addItem(item);

		std::string tmp = m_vMovieInfo[i].epgTitle;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo1;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo2;

		item->setHelpText(tmp.c_str());
	}

	// mode
	listBox->setWidgetType(WIDGET_TYPE_STANDARD);
	listBox->setWidgetMode(MODE_MENU);
	listBox->enableCenterPos();

	//
	listBox->addWidget(WIDGET_TYPE_CLASSIC);
	listBox->addWidget(WIDGET_TYPE_EXTENDED);
	listBox->addWidget(WIDGET_TYPE_FRAME);
	listBox->enableWidgetChange();

	listBox->setItemsPerPage(5, 2);

	// head
	listBox->setTitle("ClistBox(menu)", NEUTRINO_ICON_MOVIE);
	listBox->enablePaintHead();
	listBox->setHeaderButtons(HeadButtons, HEAD_BUTTONS_COUNT);
	listBox->enablePaintDate();

	// footer
	listBox->enablePaintFoot();
	listBox->setFooterButtons(FootButtons, FOOT_BUTTONS_COUNT);

	// footinfo
	listBox->enablePaintFootInfo(80);

	listBox->setSelected(selected);
	listBox->paint();

	CFrameBuffer::getInstance()->blit();

	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
	uint32_t sec_timer_id = 0;

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);

	bool loop = true;

	while(loop)
	{
		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec

		listBox->otherKeyPressed(msg);

		if ( (msg == NeutrinoMessages::EVT_TIMER) && (data == sec_timer_id) )
		{
			listBox->paintHead();
		} 
		else if (msg == RC_home) 
		{
			loop = false;
		}
		else if(msg == RC_ok)
		{
			listBox->oKKeyPressed(this);
			listBox->paint();
		}
		else if(msg == RC_info)
		{
			listBox->hide();

			selected = listBox->getSelected();
			m_movieInfo.showMovieInfo(m_vMovieInfo[selected]);
			
			listBox->paint();
		}
		else if(msg == RC_setup)
		{
			listBox->changeWidgetType();
		}

		CFrameBuffer::getInstance()->blit();
	}

	listBox->hide();
	delete listBox;
	listBox = NULL;

	g_RCInput->killTimer(sec_timer_id);
	sec_timer_id = 0;
}

// ClistBox(list mode)
void CTestMenu::testClistBox6()
{
	dprintf(DEBUG_NORMAL, "\ntesting ClistBox(list mode)\n");

	CBox Box;
	
	Box.iWidth = g_settings.screen_EndX - g_settings.screen_StartX - 20;
	Box.iHeight = g_settings.screen_EndY - g_settings.screen_StartY - 20;

	Box.iX = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - Box.iWidth ) >> 1 );
	Box.iY = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - Box.iHeight) >> 1 );

	listBox = new ClistBox(&Box);

	item = NULL;

	loadMoviePlaylist();

	// load items
	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "listplay");

		item->setOption(m_vMovieInfo[i].epgChannel.c_str());
		//item->setOptionInfo("OptionInfo");

		item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());
		//item->setOptionInfo1("OptionInfo1");

		item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());
		//item->setOptionInfo2("OptionInfo2");

		item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

		item->set2lines();

		listBox->addItem(item);

		std::string tmp = m_vMovieInfo[i].epgTitle;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo1;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo2;

		item->setHelpText(tmp.c_str());
	}

	// mode
	listBox->setWidgetType(WIDGET_TYPE_STANDARD);
	listBox->setWidgetMode(MODE_LISTBOX);
	listBox->enableCenterPos();

	//
	listBox->addWidget(WIDGET_TYPE_CLASSIC);
	listBox->addWidget(WIDGET_TYPE_EXTENDED);
	listBox->addWidget(WIDGET_TYPE_FRAME);
	listBox->enableWidgetChange();

	listBox->setItemsPerPage(5, 2);

	// head
	listBox->setTitle("ClistBox(list mode)", NEUTRINO_ICON_MOVIE);
	listBox->enablePaintHead();
	listBox->setHeaderButtons(HeadButtons, HEAD_BUTTONS_COUNT);
	listBox->enablePaintDate();

	// footer
	listBox->enablePaintFoot();
	listBox->setFooterButtons(FootButtons, FOOT_BUTTONS_COUNT);

	// footinfo
	listBox->enablePaintFootInfo(80);

	listBox->setSelected(selected);
	listBox->paint();

	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
	uint32_t sec_timer_id = 0;

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);

	bool loop = true;

	while(loop)
	{
		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec

		listBox->otherKeyPressed(msg);

		if ( (msg == NeutrinoMessages::EVT_TIMER) && (data == sec_timer_id) )
		{
			listBox->paintHead();
		} 
		else if (msg == RC_home) 
		{
			loop = false;
		}
		else if(msg == RC_ok)
		{
			listBox->oKKeyPressed(this);
			listBox->paint();
		}
		else if(msg == RC_info)
		{
			listBox->hide();

			selected = listBox->getSelected();
			m_movieInfo.showMovieInfo(m_vMovieInfo[selected]);
			
			listBox->paint();
		}
		//
		else if(msg == RC_setup)
		{
			listBox->changeWidgetType();
		}

		CFrameBuffer::getInstance()->blit();
	}

	listBox->hide();
	delete listBox;
	listBox = NULL;

	g_RCInput->killTimer(sec_timer_id);
	sec_timer_id = 0;
}

void CTestMenu::testCFrameBox()
{
	dprintf(DEBUG_NORMAL, "\ntesting CFrameBox\n");

	// frameBox
	CBox topBox;
	
	topBox.iX = g_settings.screen_StartX + 10;
	topBox.iY = g_settings.screen_StartY + 10;
	topBox.iWidth = (g_settings.screen_EndX - g_settings.screen_StartX - 20);
	topBox.iHeight = 120; //(g_settings.screen_EndY - g_settings.screen_StartY - 20);

	CFrameBox *topWidget = new CFrameBox(&topBox);
	CFrame * frame = NULL;

	frame = new CFrame("Neu Filme");
	topWidget->addFrame(frame);
	
	frame = new CFrame("Im Kino");
	topWidget->addFrame(frame);

	frame = new CFrame("Am populärsten");
	frame->setOption("(2019)");
	topWidget->addFrame(frame);

	topWidget->setSelected(selected);
	//topWidget->setOutFocus(false);
	//topWidget->setBackgroundColor(COL_RED);

//REPEAT:
	topWidget->paint();

	CFrameBuffer::getInstance()->blit();

	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;

	bool loop = true;

	while(loop)
	{
		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec

		if (msg == RC_home) 
		{
			loop = false;
		}
		else if(msg == RC_right)
		{
			topWidget->swipRight();
		}
		else if(msg == RC_left)
		{
			topWidget->swipLeft();
		}
		else if(msg == RC_ok)
		{
			MessageBox(LOCALE_MESSAGEBOX_INFO, "testing CFrameBox\ncoole Widget ;-)", mbrBack, mbBack, NEUTRINO_ICON_INFO);
		}

		CFrameBuffer::getInstance()->blit();
	}

	hide();

	delete topWidget;
	topWidget = NULL;
}

// play Movie Url
void CTestMenu::testPlayMovieURL()
{
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
		
	CFileBrowser * fileBrowser;
	
	fileBrowser = new CFileBrowser();

	fileFilter.clear();
	
	fileFilter.addFilter("ts");
	fileFilter.addFilter("mpg");
	fileFilter.addFilter("mpeg");
	fileFilter.addFilter("divx");
	fileFilter.addFilter("avi");
	fileFilter.addFilter("mkv");
	fileFilter.addFilter("asf");
	fileFilter.addFilter("aiff");
	fileFilter.addFilter("m2p");
	fileFilter.addFilter("mpv");
	fileFilter.addFilter("m2ts");
	fileFilter.addFilter("vob");
	fileFilter.addFilter("mp4");
	fileFilter.addFilter("mov");	
	fileFilter.addFilter("flv");	
	fileFilter.addFilter("dat");
	fileFilter.addFilter("trp");
	fileFilter.addFilter("vdr");
	fileFilter.addFilter("mts");
	fileFilter.addFilter("wmv");
	fileFilter.addFilter("wav");
	fileFilter.addFilter("flac");
	fileFilter.addFilter("mp3");
	fileFilter.addFilter("wma");
	fileFilter.addFilter("ogg");

	fileBrowser->Multi_Select    = false;
	fileBrowser->Dirs_Selectable = false;
	fileBrowser->Filter = &fileFilter;
	
	std::string Path_local = g_settings.network_nfs_moviedir;

BROWSER:
	if (fileBrowser->exec(Path_local.c_str()))
	{
		Path_local = fileBrowser->getCurrentDir();
		
		CFile * file;
		
		if ((file = fileBrowser->getSelectedFile()) != NULL) 
		{		
			tmpMoviePlayerGui.addToPlaylist(*file);
			tmpMoviePlayerGui.exec(NULL, "");
		}

		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg != RC_home) 
		{
			goto BROWSER;
		}
	}
	
	delete fileBrowser;
}

// play audio Url
void CTestMenu::testPlayAudioURL()
{
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
		
	CFileBrowser * fileBrowser;
	
	fileBrowser = new CFileBrowser();

	fileFilter.clear();
	
	fileFilter.addFilter("cdr");
	fileFilter.addFilter("mp3");
	fileFilter.addFilter("mpa");
	fileFilter.addFilter("mp2");
	fileFilter.addFilter("ogg");
	fileFilter.addFilter("wav");
	fileFilter.addFilter("flac");
	fileFilter.addFilter("aac");
	fileFilter.addFilter("dts");
	fileFilter.addFilter("m4a");
	
	fileBrowser->Multi_Select = false;
	fileBrowser->Dirs_Selectable = false;
	fileBrowser->Filter = &fileFilter;
	
	std::string Path_local = g_settings.network_nfs_audioplayerdir;

BROWSER:
	if (fileBrowser->exec(Path_local.c_str()))
	{
		Path_local = fileBrowser->getCurrentDir();
		
		CFile * file;
		
		if ((file = fileBrowser->getSelectedFile()) != NULL) 
		{	
			if (file->getType() == CFile::FILE_AUDIO)
			{
				tmpAudioPlayerGui.addToPlaylist(*file);
				tmpAudioPlayerGui.exec(NULL, "urlplayback");
			}
		}

		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg != RC_home) 
		{
			goto BROWSER;
		}
	}
	
	delete fileBrowser;
}

// show pic url
void CTestMenu::testShowPictureURL()
{
	neutrino_msg_t msg;
	neutrino_msg_data_t data;

	CFileBrowser * fileBrowser;
	
	fileBrowser = new CFileBrowser();

	fileFilter.clear();
	
	fileFilter.addFilter("png");
	fileFilter.addFilter("bmp");
	fileFilter.addFilter("jpg");
	fileFilter.addFilter("jpeg");
	
	fileBrowser->Multi_Select    = false;
	fileBrowser->Dirs_Selectable = false;
	fileBrowser->Filter = &fileFilter;
	
	std::string Path_local = g_settings.network_nfs_picturedir;

BROWSER:
	if (fileBrowser->exec(Path_local.c_str()))
	{
		Path_local = fileBrowser->getCurrentDir();
		
		CFile * file;
		
		if ((file = fileBrowser->getSelectedFile()) != NULL) 
		{					
			tmpPictureViewerGui.addToPlaylist(*file);
			tmpPictureViewerGui.exec(NULL, "");
		}

		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg != RC_home) 
		{
			goto BROWSER;
		}
	}
	
	delete fileBrowser;
}

// play movie folder
void CTestMenu::testPlayMovieFolder()
{
	CFileBrowser * fileBrowser;
	
	fileBrowser = new CFileBrowser();

	fileFilter.clear();
	
	fileFilter.addFilter("ts");
	fileFilter.addFilter("mpg");
	fileFilter.addFilter("mpeg");
	fileFilter.addFilter("divx");
	fileFilter.addFilter("avi");
	fileFilter.addFilter("mkv");
	fileFilter.addFilter("asf");
	fileFilter.addFilter("aiff");
	fileFilter.addFilter("m2p");
	fileFilter.addFilter("mpv");
	fileFilter.addFilter("m2ts");
	fileFilter.addFilter("vob");
	fileFilter.addFilter("mp4");
	fileFilter.addFilter("mov");	
	fileFilter.addFilter("flv");	
	fileFilter.addFilter("dat");
	fileFilter.addFilter("trp");
	fileFilter.addFilter("vdr");
	fileFilter.addFilter("mts");
	fileFilter.addFilter("wmv");
	fileFilter.addFilter("wav");
	fileFilter.addFilter("flac");
	fileFilter.addFilter("mp3");
	fileFilter.addFilter("wma");
	fileFilter.addFilter("ogg");

	fileBrowser->Multi_Select = true;
	fileBrowser->Filter = &fileFilter;
	
	std::string Path_local = g_settings.network_nfs_moviedir;

BROWSER:
	if (fileBrowser->exec(Path_local.c_str()))
	{
		Path_local = fileBrowser->getCurrentDir();
		
		CFileList::const_iterator files = fileBrowser->getSelectedFiles().begin();
		for(; files != fileBrowser->getSelectedFiles().end(); files++)
		{		
			tmpMoviePlayerGui.addToPlaylist(*files);
		}
		
		tmpMoviePlayerGui.exec(NULL, "urlplayback");
		
		neutrino_msg_t msg;
		neutrino_msg_data_t data;

		g_RCInput->getMsg_ms(&msg, &data, 10);
		
		if (msg != RC_home) 
		{
			goto BROWSER;
		}
	}
	
	delete fileBrowser;
}

// play audio folder
void CTestMenu::testPlayAudioFolder()
{
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
		
	CFileBrowser * fileBrowser;

	fileFilter.clear();
	
	fileFilter.addFilter("cdr");
	fileFilter.addFilter("mp3");
	fileFilter.addFilter("m2a");
	fileFilter.addFilter("mpa");
	fileFilter.addFilter("mp2");
	fileFilter.addFilter("ogg");
	fileFilter.addFilter("wav");
	fileFilter.addFilter("flac");
	fileFilter.addFilter("aac");
	fileFilter.addFilter("dts");
	fileFilter.addFilter("m4a");
	
	fileBrowser = new CFileBrowser();
	fileBrowser->Multi_Select = true;
	fileBrowser->Dirs_Selectable = false;
	fileBrowser->Filter = &fileFilter;
	
	std::string Path_local = g_settings.network_nfs_audioplayerdir;

BROWSER:
	if (fileBrowser->exec(Path_local.c_str()))
	{
		Path_local = fileBrowser->getCurrentDir();

		CFileList::const_iterator files = fileBrowser->getSelectedFiles().begin();
		
		for(; files != fileBrowser->getSelectedFiles().end(); files++)
		{

			if ( (files->getExtension() == CFile::EXTENSION_CDR)
					||  (files->getExtension() == CFile::EXTENSION_MP3)
					||  (files->getExtension() == CFile::EXTENSION_WAV)
					||  (files->getExtension() == CFile::EXTENSION_FLAC)
			)
			{
				tmpAudioPlayerGui.addToPlaylist(*files);
			}
		}
		
		tmpAudioPlayerGui.exec(NULL, "urlplayback");

		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg != RC_home) 
		{
			goto BROWSER;
		}
	}
	
	delete fileBrowser;
}

// show pic folder
void CTestMenu::testShowPictureFolder()
{
	neutrino_msg_t msg;
	neutrino_msg_data_t data;

	CFileBrowser * fileBrowser;
	
	fileFilter.clear();
	
	fileFilter.addFilter("png");
	fileFilter.addFilter("bmp");
	fileFilter.addFilter("jpg");
	fileFilter.addFilter("jpeg");
	
	fileBrowser = new CFileBrowser();
	fileBrowser->Multi_Select    = true;
	fileBrowser->Dirs_Selectable = true;
	fileBrowser->Filter = &fileFilter;
	
	std::string Path_local = g_settings.network_nfs_picturedir;

BROWSER:
	if (fileBrowser->exec(Path_local.c_str()))
	{
		Path_local = fileBrowser->getCurrentDir();
				
		CFileList::const_iterator files = fileBrowser->getSelectedFiles().begin();
		
		for(; files != fileBrowser->getSelectedFiles().end(); files++)
		{

			if (files->getType() == CFile::FILE_PICTURE)
			{
				tmpPictureViewerGui.addToPlaylist(*files);
			}
		}
		
		tmpPictureViewerGui.exec(NULL, "urlplayback");

		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg != RC_home) 
		{
			goto BROWSER;
		}
	}
	
	delete fileBrowser;
}

// start plugin
void CTestMenu::testStartPlugin()
{
	g_PluginList->startPlugin("youtube");
}

// show actuell epg
void CTestMenu::testShowActuellEPG()
{
	std::string title = "testShowActuellEPG:";
	std::string buffer;

	// get EPG
	CEPGData epgData;
	event_id_t epgid = 0;
			
	if(sectionsd_getActualEPGServiceKey(CNeutrinoApp::getInstance()->channelList->getActiveChannel_ChannelID(), &epgData))
		epgid = epgData.eventID;

	if(epgid != 0) 
	{
		CShortEPGData epgdata;
				
		if(sectionsd_getEPGidShort(epgid, &epgdata)) 
		{
			title += CNeutrinoApp::getInstance()->channelList->getActiveChannelName();
			title += ":";
			title += epgdata.title;

			buffer = epgdata.info1;
			buffer += "\n";
			buffer += epgdata.info2;	
		}
	}

	title += getNowTimeStr("%d.%m.%Y %H:%M");
	//
	
	CBox position(g_settings.screen_StartX + 50, g_settings.screen_StartY + 50, g_settings.screen_EndX - g_settings.screen_StartX - 100, g_settings.screen_EndY - g_settings.screen_StartY - 100); 
	
	CInfoBox * infoBox = new CInfoBox(g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1], SCROLL, &position, title.c_str(), g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE], NULL);

	infoBox->setText(buffer.c_str());
	
	infoBox->exec();
	delete infoBox;
	infoBox = NULL;	
}

// CChannelSelect
void CTestMenu::testChannelSelectWidget()
{
	CSelectChannelWidget * CSelectChannelWidgetHandler = new CSelectChannelWidget();

	std::string mode = "tv";

	if(CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_tv)
		mode = "tv";
	else if(CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_radio)
		mode = "radio";
	else if(CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_webtv)
		mode = "webtv";

	CSelectChannelWidgetHandler->exec(NULL, mode);
		
	//CSelectChannelWidget_TVChanID;
	//CSelectChannelWidget_TVChanName.c_str();
		
	delete CSelectChannelWidgetHandler;
	CSelectChannelWidgetHandler = NULL;
}

// Bedit
void CTestMenu::testBEWidget()
{	
	CBEBouquetWidget* BEWidget = new CBEBouquetWidget();

	BEWidget->exec(NULL, "");

	delete BEWidget;
	BEWidget = NULL;
}

// AVSelect
void CTestMenu::testAVSelectWidget()
{
	CAVPIDSelectWidget * AVSelectHandler = new CAVPIDSelectWidget();
	AVSelectHandler->exec(NULL, "");
		
	delete AVSelectHandler;
	AVSelectHandler = NULL;
}

// CAudioSelect
void CTestMenu::testAudioSelectWidget()
{
	CAudioSelectMenuHandler * ASelectHandler = new CAudioSelectMenuHandler();
	ASelectHandler->exec(NULL, "");
	delete ASelectHandler;
	ASelectHandler = NULL;
}

// DVBSubSelect
void CTestMenu::testDVBSubSelectWidget()
{
	CDVBSubSelectMenuHandler * dvbSubSelectHandler = new CDVBSubSelectMenuHandler();
	dvbSubSelectHandler->exec(NULL, "");
	delete dvbSubSelectHandler;
	dvbSubSelectHandler = NULL;
}

void CTestMenu::testAlphaSetupWidget()
{
	CAlphaSetup * alphaSetup = new CAlphaSetup(LOCALE_COLORMENU_GTX_ALPHA, &g_settings.gtx_alpha);
	alphaSetup->exec(NULL, "");
	delete alphaSetup;
	alphaSetup = NULL;
}

void CTestMenu::testPSISetup()
{
	CPSISetup * psiSetup = new CPSISetup(LOCALE_VIDEOMENU_PSISETUP, &g_settings.contrast, &g_settings.saturation, &g_settings.brightness, &g_settings.tint);
	psiSetup->exec(NULL, "");
	delete psiSetup;
	psiSetup = NULL;
}

void CTestMenu::testRCLock()
{
	CRCLock * rcLock = new CRCLock();
	rcLock->exec(NULL, CRCLock::NO_USER_INPUT);
	delete rcLock;
	rcLock = NULL;
}

void CTestMenu::testSleepTimerWidget()
{
	CSleepTimerWidget * sleepTimerHandler = new CSleepTimerWidget();
	sleepTimerHandler->exec(NULL, "");
	delete sleepTimerHandler;
	sleepTimerHandler = NULL;
}

void CTestMenu::testMountGUI()
{
	CNFSMountGui * mountGUI = new CNFSMountGui();
	mountGUI->exec(NULL, "");
	delete mountGUI;
	mountGUI = NULL;
}

void CTestMenu::testUmountGUI()
{
	CNFSUmountGui * umountGUI = new CNFSUmountGui();
	umountGUI->exec(NULL, "");
	delete umountGUI;
	umountGUI = NULL;
}

void CTestMenu::testMountSmallMenu()
{
	CNFSSmallMenu * mountSmallMenu = new CNFSSmallMenu();
	mountSmallMenu->exec(NULL, "");
	delete mountSmallMenu;
	mountSmallMenu = NULL;
}

void CTestMenu::testVFDController()
{
	CVfdControler * vfdControllerHandler = new CVfdControler(LOCALE_LCDMENU_HEAD, NULL);
	vfdControllerHandler->exec(NULL, "");
	delete vfdControllerHandler;
	vfdControllerHandler = NULL;
}

void CTestMenu::testColorChooser()
{
	CColorChooser * colorChooserHandler = new CColorChooser(LOCALE_COLORMENU_BACKGROUND, &g_settings.menu_Head_red, &g_settings.menu_Head_green, &g_settings.menu_Head_blue, &g_settings.menu_Head_alpha, CNeutrinoApp::getInstance()->colorSetupNotifier);

	colorChooserHandler->exec(NULL, "");
	//delete colorChooserHandler;
	//colorChooserHandler = NULL;
}

void CTestMenu::testKeyChooser()
{
	CKeyChooserItem * keyChooser = new CKeyChooserItem("testing CKeyChooser", &g_settings.key_screenshot);

	keyChooser->exec(NULL, "");
	delete keyChooser;
	keyChooser = NULL;
}

void CTestMenu::testMountChooser()
{
	CMountChooser * mountChooser = new CMountChooser("testing CMountChooser", NEUTRINO_ICON_SETTINGS, NULL, g_settings.network_nfs_moviedir, g_settings.network_nfs_recordingdir);

	mountChooser->exec(NULL, "");
	delete mountChooser;
	mountChooser = NULL;
}

void CTestMenu::testPluginsList()
{
	CPluginList * pluginList = new CPluginList();
	pluginList->exec(NULL, "");
	delete pluginList;
	pluginList = NULL;
}

void CTestMenu::testPlayMovieDir()
{
	filelist.clear();
	fileFilter.clear();
	
	fileFilter.addFilter("ts");
	fileFilter.addFilter("mpg");
	fileFilter.addFilter("mpeg");
	fileFilter.addFilter("divx");
	fileFilter.addFilter("avi");
	fileFilter.addFilter("mkv");
	fileFilter.addFilter("asf");
	fileFilter.addFilter("aiff");
	fileFilter.addFilter("m2p");
	fileFilter.addFilter("mpv");
	fileFilter.addFilter("m2ts");
	fileFilter.addFilter("vob");
	fileFilter.addFilter("mp4");
	fileFilter.addFilter("mov");	
	fileFilter.addFilter("flv");	
	fileFilter.addFilter("dat");
	fileFilter.addFilter("trp");
	fileFilter.addFilter("vdr");
	fileFilter.addFilter("mts");
	fileFilter.addFilter("wmv");
	fileFilter.addFilter("wav");
	fileFilter.addFilter("flac");
	fileFilter.addFilter("mp3");
	fileFilter.addFilter("wma");
	fileFilter.addFilter("ogg");
	
	std::string Path_local = g_settings.network_nfs_moviedir;

	if(CFileHelpers::getInstance()->readDir(Path_local, &filelist, &fileFilter))
	{
		CFileList::iterator files = filelist.begin();
		for(; files != filelist.end() ; files++)
		{
			tmpMoviePlayerGui.addToPlaylist(*files);
		}
		
		tmpMoviePlayerGui.exec(NULL, "urlplayback");
	}
}

void CTestMenu::testPlayAudioDir()
{
	filelist.clear();
	fileFilter.clear();
	
	fileFilter.addFilter("cdr");
	fileFilter.addFilter("mp3");
	fileFilter.addFilter("m2a");
	fileFilter.addFilter("mpa");
	fileFilter.addFilter("mp2");
	fileFilter.addFilter("ogg");
	fileFilter.addFilter("wav");
	fileFilter.addFilter("flac");
	fileFilter.addFilter("aac");
	fileFilter.addFilter("dts");
	fileFilter.addFilter("m4a");
	
	std::string Path_local = g_settings.network_nfs_audioplayerdir;

	if(CFileHelpers::getInstance()->readDir(Path_local, &filelist, &fileFilter))
	{
		// filter them
		CFileList::iterator files = filelist.begin();
		for(; files != filelist.end() ; files++)
		{
			if ( (files->getExtension() == CFile::EXTENSION_CDR)
					||  (files->getExtension() == CFile::EXTENSION_MP3)
					||  (files->getExtension() == CFile::EXTENSION_WAV)
					||  (files->getExtension() == CFile::EXTENSION_FLAC)
			)
			{
				tmpAudioPlayerGui.addToPlaylist(*files);
			}
		}
		
		tmpAudioPlayerGui.exec(NULL, "");
	}
}

void CTestMenu::testShowPictureDir()
{
	filelist.clear();
	fileFilter.clear();

	fileFilter.addFilter("png");
	fileFilter.addFilter("bmp");
	fileFilter.addFilter("jpg");
	fileFilter.addFilter("jpeg");
	
	std::string Path_local = g_settings.network_nfs_picturedir;

	if(CFileHelpers::getInstance()->readDir(Path_local, &filelist, &fileFilter))
	{		
		CFileList::iterator files = filelist.begin();
		for(; files != filelist.end() ; files++)
		{
			if (files->getType() == CFile::FILE_PICTURE)
			{
				tmpPictureViewerGui.addToPlaylist(*files);
			}
		}
		
		tmpPictureViewerGui.exec(NULL, "urlplayback");
	}
}

// ClistBoxWidget
void CTestMenu::testClistBoxWidget()
{
	// our listBox
	listMenu = new ClistBoxWidget("Movie Browser", NEUTRINO_ICON_MOVIE, w_max ( (CFrameBuffer::getInstance()->getScreenWidth() / 20 * 17), (CFrameBuffer::getInstance()->getScreenWidth() / 20 )), h_max ( (CFrameBuffer::getInstance()->getScreenHeight() / 20 * 17), (CFrameBuffer::getInstance()->getScreenHeight() / 20)));

	//
	loadMoviePlaylist();
	
	// add items
	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "mplay", RC_nokey, NULL, file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

		item->setOption(m_vMovieInfo[i].epgChannel.c_str());
		item->set2lines();

		item->setHelpText(m_vMovieInfo[i].epgInfo2.c_str());

		// standard
		item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());
		item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

		listMenu->addItem(item);
	}

	listMenu->setMode(MODE_LISTBOX);
	listMenu->setWidgetType(WIDGET_TYPE_STANDARD);
	listMenu->setItemsPerPage(6, 2);
	//listMenu->setItemBoxColor(COL_YELLOW);

	listMenu->addWidget(WIDGET_TYPE_CLASSIC);
	listMenu->addWidget(WIDGET_TYPE_EXTENDED);
	listMenu->addWidget(WIDGET_TYPE_FRAME);
	listMenu->enableWidgetChange();

	listMenu->setSelected(selected);

	listMenu->setHeaderButtons(HeadButtons, HEAD_BUTTONS_COUNT);
	listMenu->setFooterButtons(FootButtons, FOOT_BUTTONS_COUNT);

	listMenu->enablePaintDate();

	listMenu->enablePaintFootInfo();

	listMenu->addKey(RC_info, this, "minfo");

	listMenu->exec(NULL, "");
	listMenu->hide();
	delete listMenu;
	listMenu = NULL;
}

// ClistBoxWidget1
void CTestMenu::testClistBoxWidget1()
{
	int shortcut = 1;

	dprintf(DEBUG_NORMAL, "testClistBoxWidget\n");

	ClistBoxWidget * mainMenu = new ClistBoxWidget(LOCALE_MAINMENU_HEAD, NEUTRINO_ICON_MAINMENU);
	
	mainMenu->setMode(MODE_MENU);
	mainMenu->enableMenuPosition();
	mainMenu->enableShrinkMenu();
	mainMenu->enableWidgetChange();
	mainMenu->enablePaintFootInfo();
	  
	// tv modus
	mainMenu->addItem(new CMenuForwarder(LOCALE_MAINMENU_TVMODE, true, NULL, this, "tv", RC_red, NEUTRINO_ICON_BUTTON_RED, NEUTRINO_ICON_MENUITEM_TV, LOCALE_HELPTEXT_TVMODE ), true);

	// radio modus
	mainMenu->addItem(new CMenuForwarder(LOCALE_MAINMENU_RADIOMODE, true, NULL, this, "radio", RC_green, NEUTRINO_ICON_BUTTON_GREEN, NEUTRINO_ICON_MENUITEM_RADIO, LOCALE_HELPTEXT_RADIOMODE ));	
	
	// webtv
	mainMenu->addItem(new CMenuForwarder(LOCALE_MAINMENU_WEBTVMODE, true, NULL, this, "webtv", RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW, NEUTRINO_ICON_MENUITEM_WEBTV, LOCALE_HELPTEXT_WEBTVMODE) );
	
#if defined (ENABLE_SCART)
	// scart
	mainMenu->addItem(new CMenuForwarder(LOCALE_MAINMENU_SCARTMODE, true, NULL, this, "scart", RC_blue, NEUTRINO_ICON_BUTTON_BLUE, NEUTRINO_ICON_MENUITEM_SCART, LOCALE_HELPTEXT_SCART) );
#endif

	// mediaplayer
	mainMenu->addItem(new CMenuForwarder(LOCALE_MAINMENU_MEDIAPLAYER, true, NULL, new CMediaPlayerMenu(), NULL, CRCInput::convertDigitToKey(shortcut++), "", NEUTRINO_ICON_MENUITEM_MEDIAPLAYER, LOCALE_HELPTEXT_MEDIAPLAYER ));
	
	// main setting menu
	mainMenu->addItem(new CMenuForwarder(LOCALE_MAINMENU_SETTINGS, true, NULL, new CMainSetup(), NULL, CRCInput::convertDigitToKey(shortcut++), "", NEUTRINO_ICON_MENUITEM_SETTINGS, LOCALE_HELPTEXT_MAINSETTINGS ));

	// service
	mainMenu->addItem(new CMenuForwarder(LOCALE_MAINMENU_SERVICE, true, NULL, new CServiceSetup(), NULL, CRCInput::convertDigitToKey(shortcut++), "", NEUTRINO_ICON_MENUITEM_SERVICE, LOCALE_HELPTEXT_SERVICE ));
	
	
	// timerlist
	mainMenu->addItem(new CMenuForwarder(LOCALE_TIMERLIST_NAME, true, NULL, new CTimerList, NULL, CRCInput::convertDigitToKey(shortcut++), "", NEUTRINO_ICON_MENUITEM_TIMERLIST, LOCALE_HELPTEXT_TIMERLIST ));
	
	// features
	mainMenu->addItem(new CMenuForwarder(LOCALE_MAINMENU_FEATURES, true, NULL, this, "features", CRCInput::convertDigitToKey(shortcut++), "", NEUTRINO_ICON_MENUITEM_FEATURES, LOCALE_HELPTEXT_FEATURES ));

	// power menu
	mainMenu->addItem(new CMenuForwarder(LOCALE_MAINMENU_POWERMENU, true, NULL, new CPowerMenu(), NULL, RC_standby, NEUTRINO_ICON_BUTTON_POWER, NEUTRINO_ICON_MENUITEM_POWERMENU, LOCALE_HELPTEXT_POWERMENU ));

	//box info
	mainMenu->addItem( new CMenuForwarder(LOCALE_DBOXINFO, true, NULL, new CDBoxInfoWidget, NULL, RC_info, NEUTRINO_ICON_BUTTON_HELP, NEUTRINO_ICON_MENUITEM_BOXINFO, LOCALE_HELPTEXT_BOXINFO ));

	mainMenu->integratePlugins(CPlugins::I_TYPE_MAIN, shortcut++);

	mainMenu->exec(NULL, "");
	mainMenu->hide();
	delete mainMenu;
	mainMenu = NULL;
}

void CTestMenu::testClistBoxWidget2()
{
	CAudioPlayerSettings * audioPlayerSettingsMenu = new CAudioPlayerSettings();

	audioPlayerSettingsMenu->exec(this, "");
	delete audioPlayerSettingsMenu;
	audioPlayerSettingsMenu = NULL;	
}

// CChannellist
void CTestMenu::testChannellist()
{
	webTVchannelList = new CChannelList("CTestMenu::testWebTVChannellist:");

	int cnt = 0;
	for (tallchans_iterator it = allchans.begin(); it != allchans.end(); it++) 
	{
		if (it->second.isWebTV) 
		{
			cnt++;
			//webTVchannelList->putChannel(&(it->second));
			webTVchannelList->addChannel(&(it->second));
		}
	}

	webTVchannelList->setSize(cnt);
	webTVchannelList->exec();
}


// CBouquetlist
void CTestMenu::testBouquetlist()
{
	webTVBouquetList = new CBouquetList("CTestMenu::testWebTVBouquetlist");

	filelist.clear();
	fileFilter.clear();

	CBouquet* webTVBouquet = NULL;
	
	fileFilter.addFilter("xml");
	fileFilter.addFilter("tv");
	fileFilter.addFilter("m3u");

	if(CFileHelpers::getInstance()->readDir(CONFIGDIR "/webtv", &filelist, &fileFilter))
	{
		std::string bTitle;

		for (unsigned int i = 0; i < filelist.size(); i++)
		{
			bTitle = filelist[i].getFileName();

			removeExtension(bTitle);

			webTVBouquet = new CBouquet(0, (char *)bTitle.c_str(), 0);

			for (tallchans_iterator it = allchans.begin(); it != allchans.end(); it++) 
			{
				if (it->second.isWebTV) 
				{
					webTVBouquet->channelList->addChannel(&(it->second));
				}
			}

			webTVBouquetList->Bouquets.push_back(webTVBouquet);
		}
	}

	webTVBouquetList->exec(true);
}

void CTestMenu::spinner(void)
{
	CBox Box;
	
	Box.iX = g_settings.screen_StartX + 10;
	Box.iY = g_settings.screen_StartY + 10;

	//
	int count = 0;

	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;

	while(true)
	{
		char filename[30];
		sprintf(filename, PLUGINDIR "/test/" "%d.png", count);
		//printf("SPINNER:%s\n", filename);
		CFrameBuffer::getInstance()->paintIcon(filename, Box.iX, Box.iY);
		CFrameBuffer::getInstance()->blit();

		count = (count + 1) % 11;

		usleep(500000);

		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg == RC_home) 
		{
			break;
		}
	}
}

int CTestMenu::exec(CMenuTarget *parent, const std::string &actionKey)
{
	dprintf(DEBUG_NORMAL, "\nCTestMenu::exec: actionKey:%s\n", actionKey.c_str());

	int cnt = 0;
	
	if(parent)
		hide();
	
	if(actionKey == "testing")
	{
		test();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "widget")
	{
		widget();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "menuforwarder")
	{
		MessageBox(LOCALE_MESSAGEBOX_INFO, "testing CMenuForwarder", mbrBack, mbBack, NEUTRINO_ICON_INFO);

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "listboxitem")
	{
		MessageBox(LOCALE_MESSAGEBOX_INFO, "testing ClistBoxItem", mbrBack, mbBack, NEUTRINO_ICON_INFO);

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "box")
	{
		testCBox();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "icon")
	{
		testCIcon();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "image")
	{
		testCImage();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "window")
	{
		testCWindow();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "windowshadow")
	{
		testCWindowShadow();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "windowcustomcolor")
	{
		testCWindowCustomColor();
		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "headers")
	{
		testCHeaders();
		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "stringinput")
	{
		testCStringInput();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "stringinputsms")
	{
		testCStringInputSMS();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "pininput")
	{
		testCPINInput();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "plpininput")
	{
		testCPLPINInput();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "pinchangewidget")
	{
		testCPINChangeWidget();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "ipinput")
	{
		testCIPInput();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "macinput")
	{
		testCMACInput();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "dateinput")
	{
		testCDateInput();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "timeinput")
	{
		testCTimeInput();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "intinput")
	{
		testCIntInput();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "infobox")
	{
		testCInfoBox();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "messagebox")
	{
		testCMessageBox();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "messageboxinfomsg")
	{
		testCMessageBoxInfoMsg();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "messageboxerrormsg")
	{
		testCMessageBoxErrorMsg();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "hintbox")
	{
		testCHintBox();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "hintboxinfo")
	{
		testCHintBoxInfo();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "helpbox")
	{
		testCHelpBox();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "textbox")
	{
		testCTextBox();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "listframe")
	{
		testCListFrame();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "progressbar")
	{
		testCProgressBar();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "progresswindow")
	{
		testCProgressWindow();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "buttons")
	{
		testCButtons();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "listbox")
	{
		testClistBox();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "listbox2")
	{
		testClistBox2();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "listbox3")
	{
		testClistBox3();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "listbox4")
	{
		testClistBox4();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "listbox5")
	{
		testClistBox5();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "listbox6")
	{
		testClistBox6();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "framebox")
	{
		testCFrameBox();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "playmovieurl")
	{
		testPlayMovieURL();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "playaudiourl")
	{
		testPlayAudioURL();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "showpictureurl")
	{
		testShowPictureURL();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "playmoviefolder")
	{
		testPlayMovieFolder();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "playaudiofolder")
	{
		testPlayAudioFolder();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "showpicturefolder")
	{
		testShowPictureFolder();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "startplugin")
	{
		testStartPlugin();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "showepg")
	{
		testShowActuellEPG();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "channelselect")
	{
		testChannelSelectWidget();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "bewidget")
	{
		testBEWidget();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "avselect")
	{
		testAVSelectWidget();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "aselect")
	{
		testAudioSelectWidget();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "dvbsubselect")
	{
		testDVBSubSelectWidget();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "alphasetup")
	{
		testAlphaSetupWidget();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "psisetup")
	{
		testPSISetup();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "rclock")
	{
		testRCLock();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "sleeptimer")
	{
		testSleepTimerWidget();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "mountgui")
	{
		testMountGUI();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "umountgui")
	{
		testUmountGUI();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "mountsmallmenu")
	{
		testMountSmallMenu();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "vfdcontroller")
	{
		testVFDController();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "colorchooser")
	{
		testColorChooser();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "keychooser")
	{
		testKeyChooser();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "mountchooser")
	{
		testMountChooser();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "listboxwidget")
	{
		testClistBoxWidget();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "pluginslist")
	{
		testPluginsList();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "playmoviedir")
	{
		testPlayMovieDir();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "playaudiodir")
	{
		testPlayAudioDir();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "showpicturedir")
	{
		testShowPictureDir();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "listboxwidget1")
	{
		testClistBoxWidget1();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "listboxwidget2")
	{
		testClistBoxWidget2();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "mplay")
	{
		selected = listMenu->getSelected();

		if (&m_vMovieInfo[selected].file != NULL) 
		{
			tmpMoviePlayerGui.addToPlaylist(m_vMovieInfo[selected]);
			tmpMoviePlayerGui.exec(NULL, "");
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "listplay")
	{
		selected = listBox->getSelected();

		if (&m_vMovieInfo[selected].file != NULL) 
		{
			tmpMoviePlayerGui.addToPlaylist(m_vMovieInfo[selected]);
			tmpMoviePlayerGui.exec(NULL, "");
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "minfo")
	{
		selected = listMenu->getSelected();
		m_movieInfo.showMovieInfo(m_vMovieInfo[listMenu->getSelected()]);

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "channellist")
	{
		testChannellist();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "bouquetlist")
	{
		testBouquetlist();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "spinner")
	{
		spinner();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "winfo")
	{
		if(testWidget->getSelected() == 3)
		{
			right_selected = rightWidget->getSelected();
			m_movieInfo.showMovieInfo(m_vMovieInfo[right_selected]);
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "wok")
	{
		int focus = testWidget->getSelected();

		if(focus == 3)
		{
			//hide();
/*
			right_selected = rightWidget->getSelected();

			///
			ytparser.Cleanup();

			// setregion
			ytparser.SetRegion("DE");

			// set max result
			ytparser.SetMaxResults(1);
			
			// parse feed
			if (ytparser.ParseFeed(cYTFeedParser::SEARCH_BY_ID, m_vMovieInfo[right_selected].vname, m_vMovieInfo[right_selected].vkey))
			{
				yt_video_list_t &ylist = ytparser.GetVideoList();
	
				for (unsigned int j = 0; j < ylist.size(); j++) 
				{
					m_vMovieInfo[right_selected].ytid = ylist[j].id;
					m_vMovieInfo[right_selected].file.Name = ylist[j].GetUrl();
				}
			} 
				///

			if (&m_vMovieInfo[right_selected].file != NULL) 
			{
				tmpMoviePlayerGui.addToPlaylist(m_vMovieInfo[right_selected]);
				tmpMoviePlayerGui.exec(NULL, "");
			}
*/
		}
		else if(focus == 2)
		{
			left_selected = leftWidget->getSelected();

			if(top_selected == 0) // movies
			{
				if(left_selected == 0)
				{
					right_selected = 0;
					rightWidget->clearItems();
					//loadTMDBPlaylist("movie", "now_playing", 1);
					TVShows = "movie";
					plist = "now_playing";
					page = 1;

					loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

					// load items
					for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
					{
						item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str());

						item->setOption(m_vMovieInfo[i].epgChannel.c_str());

						item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

						item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

						item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

						rightWidget->addItem(item);
					}
				}
				else if(left_selected == 1)
				{
					right_selected = 0;
					rightWidget->clearItems();
					//loadTMDBPlaylist("movie", "popular", 1);
					TVShows = "movie";
					plist = "popular";
					page = 1;

					loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

					// load items
					for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
					{
						item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str());

						item->setOption(m_vMovieInfo[i].epgChannel.c_str());

						item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

						item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

						item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

						rightWidget->addItem(item);
					}
				}
				else if(left_selected == 2)
				{
					right_selected = 0;
					rightWidget->clearItems();
					//loadTMDBPlaylist("movie", "top_rated", 1);
					TVShows = "movie";
					plist = "top_rated";
					page = 1;

					loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);
					// load items
					for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
					{
						item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str());

						item->setOption(m_vMovieInfo[i].epgChannel.c_str());

						item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

						item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

						item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

						rightWidget->addItem(item);
					}
				}
				else if(left_selected == 3)
				{
					right_selected = 0;
					rightWidget->clearItems();

					TVShows = "movie";
					plist = "upcoming";
					page = 1;

					loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

					// load items
					for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
					{
						item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str());

						item->setOption(m_vMovieInfo[i].epgChannel.c_str());

						item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

						item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

						item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

						rightWidget->addItem(item);
					}
				}
			}
			else if(top_selected == 1) // tv
			{
				if(left_selected == 0)
				{
					right_selected = 0;
					rightWidget->clearItems();
					//loadTMDBPlaylist("tv", "airing_today", 1);
					TVShows = "tv";
					plist = "airing_today";
					page = 1;

					loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

					// load items
					for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
					{
						item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str());

						item->setOption(m_vMovieInfo[i].epgChannel.c_str());

						item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

						item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

						item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

						rightWidget->addItem(item);
					}
				}
				else if(left_selected == 1)
				{
					right_selected = 0;
					rightWidget->clearItems();
					//loadTMDBPlaylist("tv", "on_the_air", 1);
					TVShows = "tv";
					plist = "on_the_air";
					page = 1;

					loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

					// load items
					for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
					{
						item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str());

						item->setOption(m_vMovieInfo[i].epgChannel.c_str());

						item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

						item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

						item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

						rightWidget->addItem(item);
					}
				}
				else if(left_selected == 2)
				{
					right_selected = 0;
					rightWidget->clearItems();
					//loadTMDBPlaylist("tv", "popular", 1);
					TVShows = "tv";
					plist = "popular";
					page = 1;

					loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

					// load items
					for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
					{
						item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str());

						item->setOption(m_vMovieInfo[i].epgChannel.c_str());

						item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

						item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

						item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

						rightWidget->addItem(item);
					}
				}
				else if(left_selected == 3)
				{
					right_selected = 0;
					rightWidget->clearItems();
					//loadTMDBPlaylist("tv", "top_rated", 1);
					TVShows = "tv";
					plist = "top_rated";
					page = 1;

					loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

					// load items
					for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
					{
						item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str());

						item->setOption(m_vMovieInfo[i].epgChannel.c_str());

						item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

						item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

						item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

						rightWidget->addItem(item);
					}
				}
			}

			if(left_selected == 8)
				return menu_return::RETURN_EXIT_ALL;
		}
		else if(focus == 1)
		{
			top_selected = topWidget->getSelected();

			if(top_selected == 1) //tv
			{
				right_selected = 0;
				left_selected = 0;

				rightWidget->clearItems();
				//
				leftWidget->clearItems();

				ClistBoxItem *item1 = new ClistBoxItem("Heute auf");
				item1->setOption("Sendung");
				item1->set2lines();
				ClistBoxItem *item2 = new ClistBoxItem("Auf Sendung");
				ClistBoxItem *item3 = new ClistBoxItem("Am");
				item3->setOption("populärsten");
				item3->set2lines();
				ClistBoxItem *item4 = new ClistBoxItem("am");
				item4->setOption("besten bewertet");
				item4->set2lines();
				ClistBoxItem *item5 = new ClistBoxItem(NULL, false);
				ClistBoxItem *item6 = new ClistBoxItem(NULL, false);
				ClistBoxItem *item7 = new ClistBoxItem(NULL, false);
				ClistBoxItem *item8 = new ClistBoxItem(NULL, false);
				ClistBoxItem *item9 = new ClistBoxItem("Beenden");

				leftWidget->addItem(item1);
				leftWidget->addItem(new CMenuSeparator(LINE));
				leftWidget->addItem(item2);
				leftWidget->addItem(new CMenuSeparator(LINE));
				leftWidget->addItem(item3);
				leftWidget->addItem(new CMenuSeparator(LINE));
				leftWidget->addItem(item4);
				leftWidget->addItem(new CMenuSeparator(LINE));
				leftWidget->addItem(item5);
				leftWidget->addItem(item6);
				leftWidget->addItem(item7);
				leftWidget->addItem(item8);
				leftWidget->addItem(new CMenuSeparator(LINE));
				leftWidget->addItem(item9);
				leftWidget->addItem(new CMenuSeparator(LINE));

				//loadTMDBPlaylist("tv", "airing_today", 1);
				TVShows = "tv";
				plist = "airing_today";
				page = 1;

				loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

				// load items
				for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
				{
					item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str());

					item->setOption(m_vMovieInfo[i].epgChannel.c_str());

					item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

					item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

					item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

					rightWidget->addItem(item);
				}

				leftWidget->setSelected(0);
				rightWidget->setSelected(0);
			}
			else if(top_selected == 0) // movie
			{
				right_selected = 0;
				left_selected = 0;

				rightWidget->clearItems();
				leftWidget->clearItems();

				ClistBoxItem *item1 = new ClistBoxItem("In den Kinos");
				ClistBoxItem *item2 = new ClistBoxItem("Am");
				item2->setOption("populärsten");
				item2->set2lines();
				ClistBoxItem *item3 = new ClistBoxItem("Am besten");
				item3->setOption("bewertet");
				item3->set2lines();
				ClistBoxItem *item4 = new ClistBoxItem("Neue Filme");
				ClistBoxItem *item5 = new ClistBoxItem(NULL, false);
				ClistBoxItem *item6 = new ClistBoxItem(NULL, false);
				ClistBoxItem *item7 = new ClistBoxItem(NULL, false);
				ClistBoxItem *item8 = new ClistBoxItem(NULL, false);
				ClistBoxItem *item9 = new ClistBoxItem("Beenden");

				leftWidget->addItem(item1);
				leftWidget->addItem(new CMenuSeparator(LINE));
				leftWidget->addItem(item2);
				leftWidget->addItem(new CMenuSeparator(LINE));
				leftWidget->addItem(item3);
				leftWidget->addItem(new CMenuSeparator(LINE));
				leftWidget->addItem(item4);
				leftWidget->addItem(new CMenuSeparator(LINE));
				leftWidget->addItem(item5);
				leftWidget->addItem(item6);
				leftWidget->addItem(item7);
				leftWidget->addItem(item8);
				leftWidget->addItem(new CMenuSeparator(LINE));
				leftWidget->addItem(item9);
				leftWidget->addItem(new CMenuSeparator(LINE));

				loadTMDBPlaylist();

				// load items
				for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
				{
					item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str());

					item->setOption(m_vMovieInfo[i].epgChannel.c_str());

					item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

					item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

					item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

					rightWidget->addItem(item);
				}

				leftWidget->setSelected(0);
				rightWidget->setSelected(0);
			}
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "listframewidget")
	{
		listFrameWidget();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "aok")
	{
		for (unsigned int i = 0; i < AudioPlaylist.size(); i++)
		{
			tmpAudioPlayerGui.addToPlaylist(AudioPlaylist[i]);
		}

		tmpAudioPlayerGui.setCurrent(listFrame->getSelectedLine());
		tmpAudioPlayerGui.exec(NULL, "");

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "listboxmwidget")
	{
		listBoxWidget();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "mmwplay")
	{
		selected = rightWidget->getSelected();

		if (&m_vMovieInfo[selected].file != NULL) 
		{
			tmpMoviePlayerGui.addToPlaylist(m_vMovieInfo[selected]);
			tmpMoviePlayerGui.exec(NULL, "");
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "linfo")
	{
		selected = rightWidget->getSelected();
		m_movieInfo.showMovieInfo(m_vMovieInfo[selected]);

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "lsetup")
	{
		rightWidget->changeWidgetType();

		return menu_return::RETURN_NONE;
	}
	else if(actionKey == "textboxwidget")
	{
		textBoxWidget();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "exit")
	{
		return menu_return::RETURN_EXIT_ALL;
	}
	else if(actionKey == "movie_in_cinema")
	{
		right_selected = 0;
		rightWidget->clearItems();
		//loadTMDBPlaylist("movie", "now_playing", 1);
		TVShows = "movie";
		plist = "now_playing";
		page = 1;

		loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

		// load items
		for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
		{
			item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "yplay");

			item->setOption(m_vMovieInfo[i].epgChannel.c_str());

			item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

			item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

			item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

			item->set2lines();

			std::string tmp = m_vMovieInfo[i].epgTitle;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo1;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo2;

		item->setHelpText(tmp.c_str());

			rightWidget->addItem(item);
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "movie_popular")
	{
		right_selected = 0;
		rightWidget->clearItems();
		//loadTMDBPlaylist("movie", "now_playing", 1);
		TVShows = "movie";
		plist = "popular";
		page = 1;

		loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

		// load items
		for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
		{
			item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "yplay");

			item->setOption(m_vMovieInfo[i].epgChannel.c_str());

			item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

			item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

			item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

			item->set2lines();

			std::string tmp = m_vMovieInfo[i].epgTitle;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo1;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo2;

			item->setHelpText(tmp.c_str());

			rightWidget->addItem(item);
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "movie_top_rated")
	{
		right_selected = 0;
		rightWidget->clearItems();
		//loadTMDBPlaylist("movie", "now_playing", 1);
		TVShows = "movie";
		plist = "top_rated";
		page = 1;

		loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

		// load items
		for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
		{
			item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "yplay");

			item->setOption(m_vMovieInfo[i].epgChannel.c_str());

			item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

			item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

			item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

			item->set2lines();

			std::string tmp = m_vMovieInfo[i].epgTitle;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo1;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo2;

			item->setHelpText(tmp.c_str());

			rightWidget->addItem(item);
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "movie_new")
	{
		right_selected = 0;
		rightWidget->clearItems();
		//loadTMDBPlaylist("movie", "now_playing", 1);
		TVShows = "movie";
		plist = "upcoming";
		page = 1;

		loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

		// load items
		for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
		{
			item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "yplay");

			item->setOption(m_vMovieInfo[i].epgChannel.c_str());

			item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

			item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

			item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

			item->set2lines();

			std::string tmp = m_vMovieInfo[i].epgTitle;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo1;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo2;

			item->setHelpText(tmp.c_str());

			rightWidget->addItem(item);
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "tv_today")
	{
		right_selected = 0;
		rightWidget->clearItems();
		//loadTMDBPlaylist("movie", "now_playing", 1);
		TVShows = "tv";
		plist = "airing_today";
		page = 1;

		loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

		// load items
		for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
		{
			item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "yplay");

			item->setOption(m_vMovieInfo[i].epgChannel.c_str());

			item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

			item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

			item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

			item->set2lines();

			std::string tmp = m_vMovieInfo[i].epgTitle;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo1;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo2;

			item->setHelpText(tmp.c_str());

			rightWidget->addItem(item);
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "tv_on_air")
	{
		right_selected = 0;
		rightWidget->clearItems();
		//loadTMDBPlaylist("movie", "now_playing", 1);
		TVShows = "tv";
		plist = "on_the_air";
		page = 1;

		loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

		// load items
		for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
		{
			item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "yplay");

			item->setOption(m_vMovieInfo[i].epgChannel.c_str());

			item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

			item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

			item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

			item->set2lines();

			std::string tmp = m_vMovieInfo[i].epgTitle;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo1;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo2;

			item->setHelpText(tmp.c_str());

			rightWidget->addItem(item);
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "tv_popular")
	{
		right_selected = 0;
		rightWidget->clearItems();
		//loadTMDBPlaylist("movie", "now_playing", 1);
		TVShows = "tv";
		plist = "popular";
		page = 1;

		loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

		// load items
		for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
		{
			item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "yplay");

			item->setOption(m_vMovieInfo[i].epgChannel.c_str());

			item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

			item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

			item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

			item->set2lines();

			std::string tmp = m_vMovieInfo[i].epgTitle;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo1;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo2;

			item->setHelpText(tmp.c_str());

			rightWidget->addItem(item);
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "tv_top_rated")
	{
		right_selected = 0;
		rightWidget->clearItems();
		//loadTMDBPlaylist("movie", "now_playing", 1);
		TVShows = "tv";
		plist = "top_rated";
		page = 1;

		loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

		// load items
		for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
		{
			item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "yplay");

			item->setOption(m_vMovieInfo[i].epgChannel.c_str());

			item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

			item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

			item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

			item->set2lines();

			std::string tmp = m_vMovieInfo[i].epgTitle;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo1;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo2;

			item->setHelpText(tmp.c_str());

			rightWidget->addItem(item);
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "yplay")
	{
		right_selected = rightWidget->getSelected();

		///
		ytparser.Cleanup();

		// setregion
		ytparser.SetRegion("DE");

		// set max result
		ytparser.SetMaxResults(1);
			
		// parse feed
		if (ytparser.ParseFeed(cYTFeedParser::SEARCH_BY_ID, m_vMovieInfo[right_selected].vname, m_vMovieInfo[right_selected].vkey))
		{
			yt_video_list_t &ylist = ytparser.GetVideoList();
	
			for (unsigned int j = 0; j < ylist.size(); j++) 
			{
				m_vMovieInfo[right_selected].ytid = ylist[j].id;
				m_vMovieInfo[right_selected].file.Name = ylist[j].GetUrl();
			}
		} 
			///

		if (&m_vMovieInfo[right_selected].file != NULL) 
		{
			tmpMoviePlayerGui.addToPlaylist(m_vMovieInfo[right_selected]);
			tmpMoviePlayerGui.exec(NULL, "");
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "movie")
	{
		top_selected = topWidget->getSelected();

		right_selected = 0;
		left_selected = 0;

		rightWidget->clearItems();
		leftWidget->clearItems();

		ClistBoxItem *item1 = new ClistBoxItem("In den Kinos", true, NULL, this, "movie_in_cinema");
		ClistBoxItem *item2 = new ClistBoxItem("Am", true, NULL, this, "movie_popular");
		item2->setOption("populärsten");
		item2->set2lines();
		ClistBoxItem *item3 = new ClistBoxItem("Am besten", true, NULL, this, "movie_top_rated");
		item3->setOption("bewertet");
		item3->set2lines();
		ClistBoxItem *item4 = new ClistBoxItem("Neue Filme", true, NULL, this, "movie_new");
		CMenuSeparator *item5 = new CMenuSeparator();
		CMenuSeparator *item6 = new CMenuSeparator();
		CMenuSeparator *item7 = new CMenuSeparator();
		CMenuSeparator *item8 = new CMenuSeparator();
		ClistBoxItem *item9 = new ClistBoxItem("Beenden", true, NULL, this, "exit");

		leftWidget->addItem(item1);
		leftWidget->addItem(new CMenuSeparator(LINE));
		leftWidget->addItem(item2);
		leftWidget->addItem(new CMenuSeparator(LINE));
		leftWidget->addItem(item3);
		leftWidget->addItem(new CMenuSeparator(LINE));
		leftWidget->addItem(item4);
		leftWidget->addItem(new CMenuSeparator(LINE));
		leftWidget->addItem(item5);
		leftWidget->addItem(item6);
		leftWidget->addItem(item7);
		leftWidget->addItem(item8);
		leftWidget->addItem(new CMenuSeparator(LINE));
		leftWidget->addItem(item9);
		leftWidget->addItem(new CMenuSeparator(LINE));

		loadTMDBPlaylist();

		// load items
		for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
		{
			item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "yplay");

			item->setOption(m_vMovieInfo[i].epgChannel.c_str());

			item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

			item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

			item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

			item->set2lines();

			std::string tmp = m_vMovieInfo[i].epgTitle;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo1;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo2;

			item->setHelpText(tmp.c_str());

			rightWidget->addItem(item);
		}

		leftWidget->setSelected(0);
		rightWidget->setSelected(0);

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "tv")
	{
		right_selected = 0;
		left_selected = 0;

		rightWidget->clearItems();
				//
		leftWidget->clearItems();

		ClistBoxItem *item1 = new ClistBoxItem("Heute auf", true, NULL, this, "tv_today");
		item1->setOption("Sendung");
		item1->set2lines();
		ClistBoxItem *item2 = new ClistBoxItem("Auf Sendung", true, NULL, this, "tv_on_air");
		ClistBoxItem *item3 = new ClistBoxItem("Am", true, NULL, this, "tv_popular");
		item3->setOption("populärsten");
		item3->set2lines();
		ClistBoxItem *item4 = new ClistBoxItem("am", true, NULL, this, "tv_top_rated");
		item4->setOption("besten bewertet");
		item4->set2lines();
		CMenuSeparator *item5 = new CMenuSeparator();
		CMenuSeparator *item6 = new CMenuSeparator();
		CMenuSeparator *item7 = new CMenuSeparator();
		CMenuSeparator *item8 = new CMenuSeparator();
		ClistBoxItem *item9 = new ClistBoxItem("Beenden", true, NULL, this, "exit");

		leftWidget->addItem(item1);
		leftWidget->addItem(new CMenuSeparator(LINE));
		leftWidget->addItem(item2);
		leftWidget->addItem(new CMenuSeparator(LINE));
		leftWidget->addItem(item3);
		leftWidget->addItem(new CMenuSeparator(LINE));
		leftWidget->addItem(item4);
		leftWidget->addItem(new CMenuSeparator(LINE));
		leftWidget->addItem(item5);
		leftWidget->addItem(item6);
		leftWidget->addItem(item7);
		leftWidget->addItem(item8);
		leftWidget->addItem(new CMenuSeparator(LINE));
		leftWidget->addItem(item9);
		leftWidget->addItem(new CMenuSeparator(LINE));

		//loadTMDBPlaylist("tv", "airing_today", 1);
		TVShows = "tv";
		plist = "airing_today";
		page = 1;

		loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

		// load items
		for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
		{
			item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "yplay");

			item->setOption(m_vMovieInfo[i].epgChannel.c_str());

			item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

			item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

			item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

			item->set2lines();

			std::string tmp = m_vMovieInfo[i].epgTitle;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo1;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo2;

			item->setHelpText(tmp.c_str());

			rightWidget->addItem(item);
		}

		leftWidget->setSelected(0);
		rightWidget->setSelected(0);

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "nextPage")
	{
		top_selected = topWidget->getSelected();
		page++;
		right_selected = 0;
		rightWidget->clearItems();

		if(top_selected == 2) // search
		{
			loadTMDBPlaylist("transformers", "", page, true);
		}
		else
			loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

		// load items
		for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
		{
			item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "yplay");

			item->setOption(m_vMovieInfo[i].epgChannel.c_str());

			item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

			item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

			item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

			item->set2lines();

			std::string tmp = m_vMovieInfo[i].epgTitle;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo1;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo2;

			item->setHelpText(tmp.c_str());

			rightWidget->addItem(item);
		}

		rightWidget->setSelected(0);

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "prevPage")
	{
		top_selected = topWidget->getSelected();

		page--;
		if(page <= 1)
			page = 1;
		right_selected = 0;
		rightWidget->clearItems();

		if(top_selected == 2) // search
		{
			loadTMDBPlaylist("transformers", "", page, true);
		}
		else
			loadTMDBPlaylist(TVShows.c_str(), plist.c_str(), page);

		// load items
		for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
		{
			item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "yplay");

			item->setOption(m_vMovieInfo[i].epgChannel.c_str());

			item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

			item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

			item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

			item->set2lines();

			std::string tmp = m_vMovieInfo[i].epgTitle;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo1;
			tmp += "\n";
			tmp += m_vMovieInfo[i].epgInfo2;

			item->setHelpText(tmp.c_str());

			rightWidget->addItem(item);
		}

		rightWidget->setSelected(0);

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "search")
	{
		leftWidget->clearItems();

		tmdbsearch.clear();

		CStringInputSMS stringInput(LOCALE_YT_SEARCH, tmdbsearch.c_str());
		stringInput.exec(NULL, "");

		if(!stringInput.getExitPressed())
		{
			rightWidget->clearItems();

			loadTMDBPlaylist(tmdbsearch.c_str(), "", 1, true);

			// load items
			for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
			{
				item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, NULL, this, "yplay");

				item->setOption(m_vMovieInfo[i].epgChannel.c_str());

				item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());

				item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());

				item->setItemIcon(file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

				item->set2lines();

				std::string tmp = m_vMovieInfo[i].epgTitle;
				tmp += "\n";
				tmp += m_vMovieInfo[i].epgInfo1;
				tmp += "\n";
				tmp += m_vMovieInfo[i].epgInfo2;

				item->setHelpText(tmp.c_str());

				rightWidget->addItem(item);
			}

			rightWidget->setSelected(0);

			tmdbsearch.clear();
		}

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "windowwidget")
	{
		testWindowWidget();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "txtinfo")
	{
		bigFonts = bigFonts? false : true;

		if(textWidget)
			textWidget->setBigFonts(bigFonts);

		return menu_return::RETURN_REPAINT;
	}

	showMenu();
	
	return menu_return::RETURN_REPAINT;
}

#define MESSAGEBOX_NO_YES_OPTION_COUNT 2
const keyval MESSAGEBOX_NO_YES_OPTIONS[MESSAGEBOX_NO_YES_OPTION_COUNT] =
{
	{ 0, LOCALE_MESSAGEBOX_NO, NULL },
	{ 1, LOCALE_MESSAGEBOX_YES, NULL }
};

char * testValue = NULL;

void CTestMenu::showMenu()
{
	dprintf(DEBUG_NORMAL, "CTestMenu::showMenu:\n");

	mainMenu = new ClistBoxWidget(/*"testMenu", NEUTRINO_ICON_BUTTON_SETUP*/);

	mainMenu->setTitle("testMenu", NEUTRINO_ICON_BUTTON_SETUP);

	mainMenu->setMode(MODE_MENU);
	mainMenu->enableShrinkMenu(),
	mainMenu->enableMenuPosition();
	
	mainMenu->addItem(new CMenuForwarder("CWidget(multi widget)", true, NULL, this, "widget"));
	mainMenu->addItem(new CMenuForwarder("CWidget(listFrame)", true, NULL, this, "listframewidget"));
	mainMenu->addItem(new CMenuForwarder("CWidget(listBox)", true, NULL, this, "listboxmwidget"));
	mainMenu->addItem(new CMenuForwarder("CWidget(textBox)", true, NULL, this, "textboxwidget"));
	mainMenu->addItem( new CMenuSeparator(LINE) );
	mainMenu->addItem(new CMenuForwarder("CWidget(CWindow)", true, NULL, this, "windowwidget"));
	mainMenu->addItem( new CMenuSeparator(LINE) );
	mainMenu->addItem(new CMenuForwarder("TEST", true, NULL, this, "testing"));

	//
	mainMenu->addItem( new CMenuSeparator(LINE | STRING, "menu CMenuItems") );
	mainMenu->addItem(new CMenuForwarder("CMenuForwarder", true, NULL, this, "menuforwarder"));

	//
	mainMenu->addItem( new CMenuSeparator(LINE | STRING, "list CMenuItems") );
	mainMenu->addItem(new ClistBoxItem("listBoxItem", true, NULL, this, "listboxitem"));

	mainMenu->addItem( new CMenuSeparator(LINE | STRING, "setup CMenuItems") );
	mainMenu->addItem(new CMenuOptionChooser("CMenuOptionChooser:", &selected, MESSAGEBOX_NO_YES_OPTIONS, MESSAGEBOX_NO_YES_OPTION_COUNT, true));

	mainMenu->addItem(new CMenuOptionNumberChooser("CMenuOptionNumberChooser:", &selected, true, 0, 100));
/*
	CMenuOptionStringChooser * item = new CMenuOptionStringChooser("CMenuOptionStringChooser:", testValue, true);
	item->addOption("test");
	mainMenu->addItem(item);
	delete item;
	item = NULL;
*/
	
	//
	mainMenu->addItem( new CMenuSeparator(LINE | STRING, "widget Helpers") );
	mainMenu->addItem(new CMenuForwarder("CBox", true, NULL, this, "box"));
	mainMenu->addItem(new CMenuForwarder("CIcon", true, NULL, this, "icon"));
	mainMenu->addItem(new CMenuForwarder("CImage", true, NULL, this, "image"));
	mainMenu->addItem(new CMenuForwarder("CButtons", true, NULL, this, "buttons"));
	mainMenu->addItem(new CMenuForwarder("CProgressBar", true, NULL, this, "progressbar"));
	mainMenu->addItem(new CMenuForwarder("CScrollBar", false, NULL, this, "scrollbar"));
	mainMenu->addItem(new CMenuForwarder("CItems2DetailsLine", false, NULL, this, "detailsline"));
	mainMenu->addItem( new CMenuSeparator(LINE | STRING, "Widget Components") );
	mainMenu->addItem(new CMenuForwarder("CHeaders", true, NULL, this, "headers"));
	mainMenu->addItem(new CMenuForwarder("CWindow", true, NULL, this, "window"));
	mainMenu->addItem(new CMenuForwarder("CWindow(with shadow)", true, NULL, this, "windowshadow"));
	mainMenu->addItem(new CMenuForwarder("CWindow(customColor)", true, NULL, this, "windowcustomcolor"));
	mainMenu->addItem(new CMenuForwarder("CTextBox", true, NULL, this, "textbox"));
	mainMenu->addItem(new CMenuForwarder("CListFrame", true, NULL, this, "listframe"));
	mainMenu->addItem(new CMenuForwarder("ClistBox(standard)", true, NULL, this, "listbox"));
	mainMenu->addItem(new CMenuForwarder("ClistBox(classic)", true, NULL, this, "listbox2"));
	mainMenu->addItem(new CMenuForwarder("ClistBox(extended)", true, NULL, this, "listbox3"));
	mainMenu->addItem(new CMenuForwarder("ClistBox(Frame)", true, NULL, this, "listbox4"));
	mainMenu->addItem(new CMenuForwarder("ClistBox(menu mode)", true, NULL, this, "listbox5"));
	mainMenu->addItem(new CMenuForwarder("ClistBox(list mode)", true, NULL, this, "listbox6"));
	mainMenu->addItem(new CMenuForwarder("CFrameBox", true, NULL, this, "framebox"));
	
	//
	mainMenu->addItem( new CMenuSeparator(LINE | STRING, "ClistBoxWidget") );
	mainMenu->addItem(new CMenuForwarder("ClistBoxWidget(MODE_LISTBOX)", true, NULL, this, "listboxwidget"));
	mainMenu->addItem(new CMenuForwarder("ClistBoxWidget(MODE_MENU)", true, NULL, this, "listboxwidget1"));
	mainMenu->addItem(new CMenuForwarder("ClistBoxWidget(MODE_SETUP)", true, NULL, this, "listboxwidget2"));

	//
	mainMenu->addItem( new CMenuSeparator(LINE | STRING, "other Widget") );
	mainMenu->addItem(new CMenuForwarder("CStringInput", true, NULL, this, "stringinput"));
	mainMenu->addItem(new CMenuForwarder("CStringInputSMS", true, NULL, this, "stringinputsms"));
	mainMenu->addItem(new CMenuForwarder("CPINInput", true, NULL, this, "pininput"));
	mainMenu->addItem(new CMenuForwarder("CPLPINInput", true, NULL, this, "plpininput"));
	mainMenu->addItem(new CMenuForwarder("CPINChangeWidget", true, NULL, this, "pinchangewidget"));
	mainMenu->addItem(new CMenuForwarder("CIPInput", true, NULL, this, "ipinput"));
	mainMenu->addItem(new CMenuForwarder("CMACInput", true, NULL, this, "macinput"));
	//mainMenu->addItem(new CMenuForwarder("CDateInput", true, NULL, this, "dateinput"));
	//mainMenu->addItem(new CMenuForwarder("CTimeInput", true, NULL, this, "timeinput"));
	//mainMenu->addItem(new CMenuForwarder("CIntInput", true, NULL, this, "intinput"));
	mainMenu->addItem(new CMenuForwarder("CInfoBox", true, NULL, this, "infobox"));
	mainMenu->addItem(new CMenuForwarder("CMessageBox", true, NULL, this, "messagebox"));
	mainMenu->addItem(new CMenuForwarder("CMessageBoxInfoMsg", true, NULL, this, "messageboxinfomsg"));
	mainMenu->addItem(new CMenuForwarder("CMessageBoxErrorMsg", true, NULL, this, "messageboxerrormsg"));
	mainMenu->addItem(new CMenuForwarder("CHintBox", true, NULL, this, "hintbox"));
	mainMenu->addItem(new CMenuForwarder("CHintBoxInfo", true, NULL, this, "hintboxinfo"));
	mainMenu->addItem(new CMenuForwarder("CHelpBox", true, NULL, this, "helpbox"));
	mainMenu->addItem(new CMenuForwarder("CProgressWindow", true, NULL, this, "progresswindow"));

	//
	mainMenu->addItem( new CMenuSeparator(LINE) );
	mainMenu->addItem(new CMenuForwarder("ColorChooser", true, NULL, this, "colorchooser"));
	mainMenu->addItem(new CMenuForwarder("KeyChooser", true, NULL, this, "keychooser"));
	mainMenu->addItem(new CMenuForwarder("VFDController", true, NULL, this, "vfdcontroller"));
	mainMenu->addItem(new CMenuForwarder("MountChooser", true, NULL, this, "mountchooser"));
	
	//
	mainMenu->addItem( new CMenuSeparator(LINE) );
	mainMenu->addItem(new CMenuForwarder("ChannelSelectWidget", true, NULL, this, "channelselect"));
	mainMenu->addItem(new CMenuForwarder("BEWidget", true, NULL, this, "bewidget"));
	mainMenu->addItem(new CMenuForwarder("AudioVideoSelectWidget", true, NULL, this, "avselect"));
	mainMenu->addItem(new CMenuForwarder("AudioSelectWidget", true, NULL, this, "aselect"));
	mainMenu->addItem(new CMenuForwarder("DVBSubSelectWidget", true, NULL, this, "dvbsubselect"));
	mainMenu->addItem(new CMenuForwarder("AlphaSetup", true, NULL, this, "alphasetup"));
	mainMenu->addItem(new CMenuForwarder("PSISetup", true, NULL, this, "psisetup"));
	mainMenu->addItem(new CMenuForwarder("RCLock", true, NULL, this, "rclock"));
	mainMenu->addItem(new CMenuForwarder("SleepTimerWidget", true, NULL, this, "sleeptimer"));
	mainMenu->addItem(new CMenuForwarder("MountGUI", true, NULL, this, "mountgui"));
	mainMenu->addItem(new CMenuForwarder("UmountGUI", true, NULL, this, "umountgui"));
	mainMenu->addItem(new CMenuForwarder("MountSmallMenu", true, NULL, this, "mountsmallmenu"));
	mainMenu->addItem(new CMenuForwarder("PluginsList", true, NULL, this, "pluginslist"));

	//
	mainMenu->addItem( new CMenuSeparator(LINE | STRING, "Players") );
	mainMenu->addItem(new CMenuForwarder("PlayMovieURL", true, NULL, this, "playmovieurl"));
	mainMenu->addItem(new CMenuForwarder("PlayAudioURL", true, NULL, this, "playaudiourl"));
	mainMenu->addItem(new CMenuForwarder("ShowPictureURL", true, NULL, this, "showpictureurl"));

	//
	mainMenu->addItem( new CMenuSeparator(LINE) );
	mainMenu->addItem(new CMenuForwarder("PlayMovieFolder", true, NULL, this, "playmoviefolder"));
	mainMenu->addItem(new CMenuForwarder("PlayAudioFolder", true, NULL, this, "playaudiofolder"));
	mainMenu->addItem(new CMenuForwarder("ShowPictureFolder", true, NULL, this, "showpicturefolder"));

	//
	mainMenu->addItem( new CMenuSeparator(LINE) );
	mainMenu->addItem(new CMenuForwarder("PlayMovieDir(without Browser)", true, NULL, this, "playmoviedir"));
	mainMenu->addItem(new CMenuForwarder("PlayAudioDir(without Browser)", true, NULL, this, "playaudiodir"));
	mainMenu->addItem(new CMenuForwarder("ShowPictureDir(without Browser)", true, NULL, this, "showpicturedir"));

	//
	mainMenu->addItem( new CMenuSeparator(LINE | STRING, "Plugins") );
	mainMenu->addItem(new CMenuForwarder("StartPlugin(e.g: youtube)", true, NULL, this, "startplugin"));

	mainMenu->addItem( new CMenuSeparator(LINE | STRING, "EPG") );
	mainMenu->addItem(new CMenuForwarder("ShowActuellEPG", true, NULL, this, "showepg"));

	//
	mainMenu->addItem( new CMenuSeparator(LINE | STRING, "Channellist") );
	mainMenu->addItem(new CMenuForwarder("CChannelList:", true, NULL, this, "channellist"));
	mainMenu->addItem(new CMenuForwarder("CBouquetList:", true, NULL, this, "bouquetlist"));

	//
	mainMenu->addItem( new CMenuSeparator(LINE) );
	mainMenu->addItem(new CMenuForwarder("Spinner", false, NULL, this, "spinner"));
	
	mainMenu->exec(NULL, "");
	//mainMenu->hide();

	select = mainMenu->getSelected();
	printf("\nCTestMenu::exec: select:%d\n", select);

	delete mainMenu;
	mainMenu = NULL;
}

void plugin_init(void)
{
	dprintf(DEBUG_NORMAL, "test: plugin_init\n");
}

void plugin_del(void)
{
	dprintf(DEBUG_NORMAL, "test: plugin_del\n");
}

void plugin_exec(void)
{
	CTestMenu *testMenu = new CTestMenu();
	
	testMenu->exec(NULL, "");
	
	delete testMenu;
	testMenu = NULL;
}


