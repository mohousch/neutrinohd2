/*
  $Id: test.cpp 2014/01/22 mohousch Exp $

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

		//
		int selected;

		//
		ClistBox* listMenu;
		ZapitChannelList Channels;
		bool displayNext;

		//
		CFileList audioFileList;

		//
		CMovieInfo m_movieInfo;
		std::vector<MI_MOVIE_INFO> m_vMovieInfo;

		CMenuItem* item;

		//
		CChannelList* webTVchannelList;
		CBouquetList* webTVBouquetList;

		// widgets
		void testCBox();
		void testCIcon();
		void testCImage();
		void testCWindow();
		void testCWindowShadow();
		void testCStringInput();
		void testCStringInputSMS();
		void testCPINInput();
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
		void testCTextBox();
		void testCListFrame();
		void testCProgressBar();
		void testCProgressWindow();
		void testCButtons();
		void testClistBoxEntry();
		void testVFDController();
		void testColorChooser();
		void testKeyChooser();
		void testMountChooser();

		//
		void testCMenuWidgetListBox();
		void testCMenuWidgetListBox1();
		void testFrameBox();
		void testCMenuWidget();
		void testClistBoxnLines();
	
		// mediapalyers
		void testAudioPlayer();
		void testTSPlayer();
		void testMoviePlayer();
		void testPictureViewer();

		//
		void testPlayMovieURL();
		void testPlayAudioURL();
		void testShowPictureURL();
		//
		void testPlayMovieFolder();
		void testPlayAudioFolder();
		void testShowPictureFolder();
		//
		void testStartPlugin();

		//
		void testShowActuellEPG();
		void testChannelSelectWidget();
		void testBEWidget();
		void testAVSelectWidget();
		void testAudioSelectWidget();
		void testDVBSubSelectWidget();
		//void testAlphaSetupWidget();
		//void testPSISetup();
		void testRCLock();
		void testSleepTimerWidget();
		void testMountGUI();
		void testUmountGUI();
		void testMountSmallMenu();
		
		//
		void testPluginsList();

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
		void hide();
		void showTestMenu();
};

CTestMenu::CTestMenu()
{
	frameBuffer = CFrameBuffer::getInstance();

	selected = 0;

	//
	displayNext = false;

	//
	webTVchannelList = NULL;
	webTVBouquetList = NULL;

	//
	listMenu = NULL;
	item = NULL;
}

CTestMenu::~CTestMenu()
{
	Channels.clear();
	audioFileList.clear();
	m_vMovieInfo.clear();

	if(webTVchannelList)
	{
		delete webTVchannelList;
		webTVchannelList = NULL;
	}
}

void CTestMenu::hide()
{
	frameBuffer->paintBackground();
	frameBuffer->blit();
}

void CTestMenu::testCBox()
{
	CBox Box;
	
	Box.iX = g_settings.screen_StartX + 10;
	Box.iY = g_settings.screen_StartY + 10;
	Box.iWidth = (g_settings.screen_EndX - g_settings.screen_StartX - 20)/2;
	Box.iHeight = 40; //(g_settings.screen_EndY - g_settings.screen_StartY - 20);

	CFrameBuffer::getInstance()->paintBoxRel(Box.iX, Box.iY, Box.iWidth, Box.iHeight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_ALL, gradientDark2Light2Dark);

	CFrameBuffer::getInstance()->blit();

	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;

	while(1)
	{
		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg == CRCInput::RC_home) 
		{
			CFrameBuffer::getInstance()->paintBackground();
			CFrameBuffer::getInstance()->blit();

			break;
		}
	}
}

void CTestMenu::testCIcon()
{
	//CIcon testIcon(NEUTRINO_ICON_BUTTON_RED);
	CIcon testIcon;
	
	CBox testBox;
	testBox.iX = g_settings.screen_StartX + 10;
	testBox.iY = g_settings.screen_StartY + 10;
	testBox.iWidth = (g_settings.screen_EndX - g_settings.screen_StartX - 20)/2;
	testBox.iHeight = 40; //(g_settings.screen_EndY - g_settings.screen_StartY - 20);

	// paint testBox
	//CFrameBuffer::getInstance()->paintBoxRel(testBox.iX, testBox.iY, testBox.iWidth, testBox.iHeight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_ALL, gradientDark2Light2Dark);

	// paint testIcon
	testIcon.setIcon(NEUTRINO_ICON_BUTTON_RED);

	CFrameBuffer::getInstance()->paintIcon(testIcon.iconName.c_str(), testBox.iX + BORDER_LEFT, testBox.iY + (testBox.iHeight - testIcon.iHeight)/2);

	CFrameBuffer::getInstance()->blit();

	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;

	while(1)
	{
		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg == CRCInput::RC_home) 
		{
			CFrameBuffer::getInstance()->paintBackground();
			CFrameBuffer::getInstance()->blit();

			break;
		}
	}
}

void CTestMenu::testCImage()
{
	//CImage testImage(PLUGINDIR "/netzkino/netzkino.png");
	CImage testImage;
	
	CBox testBox;
	testBox.iX = g_settings.screen_StartX + 10;
	testBox.iY = g_settings.screen_StartY + 10;
	testBox.iWidth = (g_settings.screen_EndX - g_settings.screen_StartX - 20)/2;
	testBox.iHeight = 40; //(g_settings.screen_EndY - g_settings.screen_StartY - 20);

	// paint testBox
	//CFrameBuffer::getInstance()->paintBoxRel(testBox.iX, testBox.iY, testBox.iWidth, testBox.iHeight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_ALL, gradientDark2Light2Dark);

	// paint testImage
	testImage.setImage(PLUGINDIR "/netzkino/netzkino.png");

	CFrameBuffer::getInstance()->displayImage(testImage.imageName.c_str(), testBox.iX + BORDER_LEFT, testBox.iY, testImage.iWidth, testImage.iHeight);

	CFrameBuffer::getInstance()->blit();

	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;

	while(1)
	{
		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg == CRCInput::RC_home) 
		{
			CFrameBuffer::getInstance()->paintBackground();
			CFrameBuffer::getInstance()->blit();

			break;
		}
	}
}

void CTestMenu::testCWindow()
{
	CBox Box;
	
	Box.iX = g_settings.screen_StartX + 10;
	Box.iY = g_settings.screen_StartY + 10;
	Box.iWidth = (g_settings.screen_EndX - g_settings.screen_StartX - 20)/2;
	Box.iHeight = 40; //(g_settings.screen_EndY - g_settings.screen_StartY - 20);

	//
	CWindow* window = new CWindow();

	window->setDimension(Box.iX, Box.iY, Box.iWidth, Box.iHeight);

	window->setColor(COL_MENUHEAD_PLUS_0);
	window->setCorner(RADIUS_MID, CORNER_ALL);
	window->setGradient(gradientDark2Light2Dark);

	window->paint();

	CFrameBuffer::getInstance()->blit();
	//

	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;

	while(1)
	{
		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg == CRCInput::RC_home) 
		{
			//CFrameBuffer::getInstance()->paintBackground();
			window->hide();
			CFrameBuffer::getInstance()->blit();

			break;
		}
	}

	delete window;
	window = NULL;
}

void CTestMenu::testCWindowShadow()
{
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
	//

	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;

	while(1)
	{
		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg == CRCInput::RC_home) 
		{
			window->hide();
			CFrameBuffer::getInstance()->blit();

			break;
		}
	}

	delete window;
	window = NULL;
}

void CTestMenu::testCStringInput()
{
	std::string value;
	CStringInput * stringInput = new CStringInput("CStringInput", &value);
	
	stringInput->exec(NULL, "");
	stringInput->hide();
	delete stringInput;
	stringInput = NULL;
	value.clear();
}
void CTestMenu::testCStringInputSMS()
{
	std::string value;
	CStringInputSMS * stringInputSMS = new CStringInputSMS("CStringInputSMS", &value);
	
	stringInputSMS->exec(NULL, "");
	stringInputSMS->hide();
	delete stringInputSMS;
	value.clear();
}

void CTestMenu::testCPINInput()
{
	std::string value;
	CPINInput * pinInput = new CPINInput("CPINInput", &value);
	
	pinInput->exec(NULL, "");
	pinInput->hide();
	delete pinInput;
	pinInput = NULL;
	value.clear();
}

void CTestMenu::testCIPInput()
{
	std::string value;
	CIPInput * ipInput = new CIPInput(LOCALE_STREAMINGMENU_SERVER_IP, value);
	
	ipInput->exec(NULL, "");
	ipInput->hide();
	delete ipInput;
	value.clear();
}

void CTestMenu::testCMACInput()
{
	std::string value;
	CMACInput * macInput = new CMACInput(LOCALE_RECORDINGMENU_SERVER_MAC, (char *)value.c_str());
	
	macInput->exec(NULL, "");
	macInput->hide();
	delete macInput;
	macInput = NULL;
	value.clear();
}

void CTestMenu::testCDateInput()
{
	time_t value;
	CDateInput * dateInput = new CDateInput(LOCALE_FILEBROWSER_SORT_DATE, &value);
	
	dateInput->exec(NULL, "");
	dateInput->hide();
	delete dateInput;
}

void CTestMenu::testCTimeInput()
{
	std::string value;
	CTimeInput * timeInput = new CTimeInput(LOCALE_FILEBROWSER_SORT_DATE, (char *)value.c_str());
	
	timeInput->exec(NULL, "");
	timeInput->hide();
	delete timeInput;
	timeInput = NULL;
	value.clear();
}

void CTestMenu::testCIntInput()
{
	int value;
	CIntInput * intInput = new CIntInput(LOCALE_FILEBROWSER_SORT_DATE, value);
	
	intInput->exec(NULL, "");
	intInput->hide();
	delete intInput;
	intInput = NULL;	
}

void CTestMenu::testCInfoBox()
{
	std::string buffer;
	
	// prepare print buffer  
	buffer = "CInfoBox";
	buffer += "\n";
	buffer += "testing CInfoBox";
	buffer += "\n";

	// thumbnail
	int pich = 246;	//FIXME
	int picw = 162; 	//FIXME
	
	std::string thumbnail = PLUGINDIR "/netzkino/netzkino.png";
	if(access(thumbnail.c_str(), F_OK))
		thumbnail = "";
	
	CBox position(g_settings.screen_StartX + 50, g_settings.screen_StartY + 50, g_settings.screen_EndX - g_settings.screen_StartX - 100, g_settings.screen_EndY - g_settings.screen_StartY - 100); 
	
	CInfoBox * infoBox = new CInfoBox("testing CInfoBox", g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1], CTextBox::SCROLL, &position, "CInfoBox", g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE], NEUTRINO_ICON_INFO);

	infoBox->setText(&buffer, thumbnail, picw, pich);
	infoBox->exec();
	delete infoBox;
	infoBox = NULL;
}

void CTestMenu::testCMessageBox()
{
	CMessageBox * messageBox = new CMessageBox(LOCALE_MESSAGEBOX_INFO, "testing CMessageBox"/*, 600, NEUTRINO_ICON_INFO, CMessageBox::mbrYes, CMessageBox::mbNone*/);
	
	int res = messageBox->exec();

	printf("res:%d messageBox->result:%d\n", res, messageBox->result);

	delete messageBox;
	messageBox = NULL;
}

void CTestMenu::testCMessageBoxInfoMsg()
{
	MessageBox(LOCALE_MESSAGEBOX_INFO, "testing CMessageBoxInfoMsg", CMessageBox::mbrBack, CMessageBox::mbBack, NEUTRINO_ICON_INFO);
}

void CTestMenu::testCMessageBoxErrorMsg()
{
	MessageBox(LOCALE_MESSAGEBOX_ERROR, "testing CMessageBoxErrorMsg", CMessageBox::mbrCancel, CMessageBox::mbCancel, NEUTRINO_ICON_ERROR);
}

void CTestMenu::testCHintBox()
{
	CHintBox * hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, "testing CHintBox");
	
	hintBox->exec();

	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;

	while(1)
	{
		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg == CRCInput::RC_home) 
		{
			delete hintBox;
			hintBox = NULL;

			break;
		}
	}
}

void CTestMenu::testCHintBoxInfo()
{
	HintBox(LOCALE_MESSAGEBOX_INFO, "testing CHintBoxInfo");
}

void CTestMenu::testCHelpBox()
{
	CHelpBox * helpBox = new CHelpBox();
	
	// text
	helpBox->addLine("helpBox");

	// icon
	helpBox->addLine(NEUTRINO_ICON_BUTTON_RED, "Huhu :-P");

	//
	helpBox->addLine("Huhu :-)", g_Font[SNeutrinoSettings::FONT_TYPE_MENU], COL_RED0, true);

	//
	helpBox->addLine("neutrinoHD2 the best GUI :-P", g_Font[SNeutrinoSettings::FONT_TYPE_MENU], COL_YELLOW0, true);
	

	//
	helpBox->addPagebreak();

	// icon
	helpBox->addLine(NEUTRINO_ICON_BUTTON_RED, "Huhu :-P");

	//
	helpBox->addLine("Huhu :-)", g_Font[SNeutrinoSettings::FONT_TYPE_MENU], COL_RED0, true);

	//
	helpBox->addLine("neutrinoHD2 the best GUI :-P", g_Font[SNeutrinoSettings::FONT_TYPE_MENU], COL_YELLOW0, true);

	helpBox->show(LOCALE_MESSAGEBOX_INFO, CMessageBox::mbrBack, CMessageBox::mbNone);
	
	delete helpBox;
	helpBox = NULL;
}

void CTestMenu::testCTextBox()
{
	CBox Box;
	
	Box.iX = g_settings.screen_StartX + 10;
	Box.iY = g_settings.screen_StartY + 10;
	Box.iWidth = g_settings.screen_EndX - g_settings.screen_StartX - 20;
	Box.iHeight = (g_settings.screen_EndY - g_settings.screen_StartY - 20);
	
	CTextBox * textBox = new CTextBox("CTextBox", NULL, CTextBox::SCROLL, &Box);
	
	std::string text = "testing CTextBox";
		
	int pich = 246;	//FIXME
	int picw = 162; 	//FIXME
		
	std::string fname;

	fname = PLUGINDIR "/netzkino/netzkino.png";
		
	if(access(fname.c_str(), F_OK))
		fname = "";
	
	textBox->setText(&text, fname, picw, pich);
	
	textBox->paint();
	CFrameBuffer::getInstance()->blit();
	
	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
	bool bigFonts = false;

	while(1)
	{
		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg == CRCInput::RC_home) 
		{
			textBox->hide();
			CFrameBuffer::getInstance()->blit();

			break;
		}
		else if(msg == CRCInput::RC_info)
		{
			bigFonts = bigFonts? false : true;
			textBox->setBigFonts(bigFonts);
		}
	}
	
	delete textBox;
	textBox = NULL;
}

void CTestMenu::testCListFrame()
{
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
	#define MAX_WINDOW_WIDTH  		(g_settings.screen_EndX - g_settings.screen_StartX - 40)
	#define MAX_WINDOW_HEIGHT 		(g_settings.screen_EndY - g_settings.screen_StartY - 40)	
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
	CFileFilter fileFilter;

	// music
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

	if(CFileHelpers::getInstance()->readDir(Path_local, &audioFileList, &fileFilter))
	{
		int count = 0;
				
		CFileList::iterator files = audioFileList.begin();
		for(; files != audioFileList.end() ; files++)
		{
			count++;
			if (files->getType() == CFile::FILE_AUDIO)
			{
				std::string title;
				std::string artist;
				std::string genre;
				std::string date;
				char duration[9] = "";

				// metaData
				CAudiofile audiofile(files->Name, files->getExtension());

				CAudioPlayer::getInstance()->init();

				int ret = CAudioPlayer::getInstance()->readMetaData(&audiofile, true);

				if (!ret || (audiofile.MetaData.artist.empty() && audiofile.MetaData.title.empty() ))
				{
					// //remove extension (.mp3)
					std::string tmp = files->getFileName().substr(files->getFileName().rfind('/') + 1);
					tmp = tmp.substr(0, tmp.length() - 4);	//remove extension (.mp3)

					std::string::size_type i = tmp.rfind(" - ");
		
					if(i != std::string::npos)
					{ 
						title = tmp.substr(0, i);
						artist = tmp.substr(i + 3);
					}
					else
					{
						i = tmp.rfind('-');
						if(i != std::string::npos)
						{
							title = tmp.substr(0, i);
							artist = tmp.substr(i + 1);
						}
						else
							title = tmp;
					}
				}
				else
				{
					title = audiofile.MetaData.title;
					artist = audiofile.MetaData.artist;
					genre = audiofile.MetaData.genre;	
					date = audiofile.MetaData.date;

					snprintf(duration, 8, "(%ld:%02ld)", audiofile.MetaData.total_time / 60, audiofile.MetaData.total_time % 60);
				}

				listFrameLines.lineArray[0].push_back(to_string(count));
				listFrameLines.lineArray[1].push_back(title);
				listFrameLines.lineArray[2].push_back(duration);
				listFrameLines.lineArray[3].push_back(genre);
				listFrameLines.lineArray[4].push_back(artist);
				listFrameLines.lineArray[5].push_back(date);
			}
		}
	}
	//
	listFrame->setLines(&listFrameLines);
	
	// paint
	listFrame->paint();
	listFrame->showSelection(true);

	CFrameBuffer::getInstance()->blit();

	CAudioPlayer::getInstance()->init();
	//CNeutrinoApp::getInstance()->handleMsg(NeutrinoMessages::CHANGEMODE, NeutrinoMessages::mode_audio);
	//int state = CAudioPlayerGui::STOP;
	int index = 0;
	CAudioPlayerGui tmpAudioPlayerGui;
	
	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;

REPEAT:
	listFrame->refresh();
	while(1)
	{
		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec

		//if (CAudioPlayer::getInstance()->getState() == CBaseDec::STOP)
		//	state = CAudioPlayerGui::STOP;
		
		if (msg == CRCInput::RC_home) 
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

			break;
		}
		else if(msg == CRCInput::RC_down)
		{
			listFrame->scrollLineDown(1);
			listFrame->refresh();
		}
		else if(msg == CRCInput::RC_up)
		{
			listFrame->scrollLineUp(1);
			listFrame->refresh();
		}
		else if(msg == CRCInput::RC_page_down)
		{
			listFrame->scrollPageDown(1);
			listFrame->refresh();
		}
		else if(msg == CRCInput::RC_page_up)
		{
			listFrame->scrollPageUp(1);
			listFrame->refresh();
		}
		else if(msg == CRCInput::RC_ok)
		{
			selected = listFrame->getSelectedLine();

			index = selected;

			CAudiofile mp3(audioFileList[index].Name, audioFileList[index].getExtension());
			//CAudioPlayer::getInstance()->play(&mp3, g_settings.audioplayer_highprio == 1);

			//state = CAudioPlayerGui::PLAY;	

			//goto REPEAT;

			tmpAudioPlayerGui.addToPlaylist(mp3);

			//tmpAudioPlayerGui.setCurrent(selected);
			tmpAudioPlayerGui.exec(NULL, "");

			//listFrame->refresh();
			goto REPEAT;
		}
		/*
		else if(msg == CRCInput::RC_stop)
		{
			if (CAudioPlayer::getInstance()->getState() != CBaseDec::STOP)
			{
				CAudioPlayer::getInstance()->stop();
			}

			state = CAudioPlayerGui::STOP;
		}
		else if(msg == CRCInput::RC_right)
		{
			//if (CAudioPlayer::getInstance()->getState() != CBaseDec::STOP)
			{
				CAudioPlayer::getInstance()->stop();
				state = CAudioPlayerGui::STOP;

				CAudiofile mp3(audioFileList[index++].Name, audioFileList[index++].getExtension());
				CAudioPlayer::getInstance()->play(&mp3, g_settings.audioplayer_highprio == 1);
				state = CAudioPlayerGui::PLAY;
			}
		}
		*/

		//listFrame->refresh();
		CFrameBuffer::getInstance()->blit();
	}
	
	delete listFrame;
	listFrame = NULL;
}

void CTestMenu::testCProgressBar()
{
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

void CTestMenu::testCProgressWindow()
{
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

#define BUTTONS_COUNT	4
const struct button_label Buttons[BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_RED, NONEXISTANT_LOCALE, "add" },
	{ NEUTRINO_ICON_BUTTON_GREEN, NONEXISTANT_LOCALE, "remove" },
	{ NEUTRINO_ICON_BUTTON_YELLOW, NONEXISTANT_LOCALE, "delete" },
	{ NEUTRINO_ICON_BUTTON_BLUE, NONEXISTANT_LOCALE, "rename" },
	
};

void CTestMenu::testCButtons()
{
	int icon_w, icon_h;
	CFrameBuffer::getInstance()->getIconSize(NEUTRINO_ICON_BUTTON_RED, &icon_w, &icon_h);
	::paintButtons(CFrameBuffer::getInstance(), g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, g_settings.screen_StartX + 50 + BORDER_LEFT, g_settings.screen_StartY + 50, (g_settings.screen_EndX - g_settings.screen_StartX - 100)/BUTTONS_COUNT, BUTTONS_COUNT, Buttons, icon_h);

	CFrameBuffer::getInstance()->blit();

	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;

	while(1)
	{
		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg == CRCInput::RC_home) 
		{
			CFrameBuffer::getInstance()->paintBackground();
			CFrameBuffer::getInstance()->blit();

			break;
		}
	}
}

void CTestMenu::testClistBoxEntry()
{
	CBox Box;
	
	Box.iWidth = g_settings.screen_EndX - g_settings.screen_StartX - 20;
	Box.iHeight = g_settings.screen_EndY - g_settings.screen_StartY - 20;

	Box.iX = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - Box.iWidth ) >> 1 );
	Box.iY = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - Box.iHeight) >> 1 );

	ClistBoxEntry* listBox = new ClistBoxEntry(&Box);

	CMenuItem* item = NULL;

	//
	CFileFilter fileFilter;
	
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

	CFileList filelist;
	
	// recordingdir
	std::string Path_local = g_settings.network_nfs_recordingdir;
	m_vMovieInfo.clear();
	
	//
	if(CFileHelpers::getInstance()->readDir(Path_local, &filelist, &fileFilter))
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

	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		item = new ClistBoxEntryItem(m_vMovieInfo[i].epgTitle.c_str(), true, m_vMovieInfo[i].epgChannel.c_str(), NULL, "", NULL, file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

		//
		std::string tmp = m_vMovieInfo[i].epgTitle;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo1;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo2;

		item->setInfo1(tmp.c_str());
		item->setWidgetType(WIDGET_CLASSIC);
		item->setnLinesItem();

		listBox->addItem(item);
	}

	listBox->setTitle("ClistBoxEntry", NEUTRINO_ICON_MP3);
	listBox->enablePaintHead();
	listBox->setHeaderButtons(Buttons, BUTTONS_COUNT);
	listBox->enablePaintDate();
	listBox->enablePaintFoot();
	listBox->setFooterButtons(Buttons, BUTTONS_COUNT);
	listBox->enablePaintFootInfo(40);

REPEAT:
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

		if ( (msg == NeutrinoMessages::EVT_TIMER) && (data == sec_timer_id) )
		{
			listBox->paintHead();
		} 
		else if (msg == CRCInput::RC_home) 
		{
			loop = false;
		}
		else if(msg == CRCInput::RC_down)
		{
			listBox->scrollLineDown();
		}
		else if(msg == CRCInput::RC_up)
		{
			listBox->scrollLineUp();
		}
		else if(msg == CRCInput::RC_page_down)
		{
			listBox->scrollPageDown();
		}
		else if(msg == CRCInput::RC_page_up)
		{
			listBox->scrollPageUp();
		}
		else if(msg == CRCInput::RC_ok)
		{

			hide();

			selected = listBox->getSelected();
			CMoviePlayerGui tmpMoviePlayerGui;

			if (&m_vMovieInfo[selected].file != NULL) 
			{
				tmpMoviePlayerGui.addToPlaylist(m_vMovieInfo[selected]);
				tmpMoviePlayerGui.exec(NULL, "");
			}

			goto REPEAT;
		}
		else if(msg == CRCInput::RC_info)
		{
			hide();
			selected = listBox->getSelected();
			m_movieInfo.showMovieInfo(m_vMovieInfo[selected]);
			goto REPEAT;
		}

		CFrameBuffer::getInstance()->blit();
	}

	hide();
	listBox->hide();
	delete listBox;
	listBox = NULL;

	g_RCInput->killTimer(sec_timer_id);
	sec_timer_id = 0;
}

void CTestMenu::testAudioPlayer()
{
	g_PluginList->startPlugin("audioplayer");
}

void CTestMenu::testTSPlayer()
{
	g_PluginList->startPlugin("tsbrowser");
}

void CTestMenu::testMoviePlayer()
{
	g_PluginList->startPlugin("moviebrowser");
}

void CTestMenu::testPictureViewer()
{
	g_PluginList->startPlugin("picviewer");
}

void CTestMenu::testPlayMovieURL()
{
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
	
	CMoviePlayerGui tmpMoviePlayerGui;	
	CFileBrowser * fileBrowser;
	
	fileBrowser = new CFileBrowser();
	
	CFileFilter fileFilter;
	
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
			MI_MOVIE_INFO mfile;
			
			mfile.file.Name = file->Name;
			mfile.epgTitle = file->getFileName();
			mfile.epgInfo1 = file->getFileName(); 

			std::string fname = "";
			fname = file->Name;
			changeFileNameExt(fname, ".jpg");
						
			if(!access(fname.c_str(), F_OK) )
				mfile.tfile = fname.c_str();
					
			tmpMoviePlayerGui.addToPlaylist(mfile);
			tmpMoviePlayerGui.exec(NULL, "urlplayback");
		}

		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg != CRCInput::RC_home) 
		{
			goto BROWSER;
		}
	}
	
	delete fileBrowser;
}

void CTestMenu::testPlayAudioURL()
{
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
		
	CFileBrowser * fileBrowser;
	
	fileBrowser = new CFileBrowser();
	
	CFileFilter fileFilter;
	
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
			CAudioPlayerGui tmpAudioPlayerGui;
			
			if (file->getType() == CFile::FILE_AUDIO)
			{
				CAudiofile audiofile(file->Name, file->getExtension());
				tmpAudioPlayerGui.addToPlaylist(audiofile);
				tmpAudioPlayerGui.exec(NULL, "urlplayback");
			}
		}

		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg != CRCInput::RC_home) 
		{
			goto BROWSER;
		}
	}
	
	delete fileBrowser;
}

void CTestMenu::testShowPictureURL()
{
	neutrino_msg_t msg;
	neutrino_msg_data_t data;

	CFileBrowser * fileBrowser;
	
	fileBrowser = new CFileBrowser();
	
	CFileFilter fileFilter;
	
	fileFilter.addFilter("png");
	fileFilter.addFilter("bmp");
	fileFilter.addFilter("jpg");
	fileFilter.addFilter("jpeg");
	
	fileBrowser->Multi_Select    = false;
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
			CPictureViewerGui tmpPictureViewerGui;
			CPicture pic;
			struct stat statbuf;
			
			pic.Filename = file->Name;
			std::string tmp = file->Name.substr(file->Name.rfind('/') + 1);
			pic.Name = tmp.substr(0, tmp.rfind('.'));
			pic.Type = tmp.substr(tmp.rfind('.') + 1);
			
			if(stat(pic.Filename.c_str(), &statbuf) != 0)
				printf("stat error");
			pic.Date = statbuf.st_mtime;
							
			tmpPictureViewerGui.addToPlaylist(pic);
			tmpPictureViewerGui.exec(NULL, "urlplayback");
		}

		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg != CRCInput::RC_home) 
		{
			goto BROWSER;
		}
	}
	
	delete fileBrowser;
}

void CTestMenu::testPlayMovieFolder()
{
	CMoviePlayerGui tmpMoviePlayerGui;
	
	CFileBrowser * fileBrowser;
	
	fileBrowser = new CFileBrowser();
	
	CFileFilter fileFilter;
	
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
		MI_MOVIE_INFO mfile;
		CFileList::const_iterator files = fileBrowser->getSelectedFiles().begin();
		for(; files != fileBrowser->getSelectedFiles().end(); files++)
		{
			mfile.file.Name = files->Name;
			mfile.epgTitle = files->getFileName();
			mfile.epgInfo1 = files->getFileName();

			std::string fname = "";
			fname = files->Name;
			changeFileNameExt(fname, ".jpg");
						
			if(!access(fname.c_str(), F_OK) )
				mfile.tfile = fname.c_str();
					
			tmpMoviePlayerGui.addToPlaylist(mfile);
		}
		
		tmpMoviePlayerGui.exec(NULL, "urlplayback");
		
		neutrino_msg_t msg;
		neutrino_msg_data_t data;

		g_RCInput->getMsg_ms(&msg, &data, 10);
		
		if (msg != CRCInput::RC_home) 
		{
			goto BROWSER;
		}
	}
	
	delete fileBrowser;
}

void CTestMenu::testPlayAudioFolder()
{
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
		
	CFileBrowser * fileBrowser;

	CFileFilter fileFilter;
	
	CFileList filelist;
	
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
		
		CAudioPlayerGui tmpAudioPlayerGui;
		CFileList::const_iterator files = fileBrowser->getSelectedFiles().begin();
		
		for(; files != fileBrowser->getSelectedFiles().end(); files++)
		{

			if ( (files->getExtension() == CFile::EXTENSION_CDR)
					||  (files->getExtension() == CFile::EXTENSION_MP3)
					||  (files->getExtension() == CFile::EXTENSION_WAV)
					||  (files->getExtension() == CFile::EXTENSION_FLAC)
			)
			{
				CAudiofile audiofile(files->Name, files->getExtension());
				tmpAudioPlayerGui.addToPlaylist(audiofile);
			}
		}
		
		tmpAudioPlayerGui.exec(NULL, "urlplayback");

		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg != CRCInput::RC_home) 
		{
			goto BROWSER;
		}
	}
	
	delete fileBrowser;
}

void CTestMenu::testShowPictureFolder()
{
	neutrino_msg_t msg;
	neutrino_msg_data_t data;

	CFileBrowser * fileBrowser;
	
	CFileFilter fileFilter;
	
	CFileList filelist;
	//int selected = 0;
	
	fileFilter.addFilter("png");
	fileFilter.addFilter("bmp");
	fileFilter.addFilter("jpg");
	fileFilter.addFilter("jpeg");
	
	fileBrowser = new CFileBrowser();
	fileBrowser->Multi_Select    = true;
	fileBrowser->Dirs_Selectable = true;
	fileBrowser->Filter = &fileFilter;
	
	std::string Path_local = g_settings.network_nfs_audioplayerdir;

BROWSER:
	if (fileBrowser->exec(Path_local.c_str()))
	{
		Path_local = fileBrowser->getCurrentDir();
		
		CPictureViewerGui tmpPictureViewerGui;
		CPicture pic;
		struct stat statbuf;
				
		CFileList::const_iterator files = fileBrowser->getSelectedFiles().begin();
		
		for(; files != fileBrowser->getSelectedFiles().end(); files++)
		{

			if (files->getType() == CFile::FILE_PICTURE)
			{
				pic.Filename = files->Name;
				std::string tmp = files->Name.substr(files->Name.rfind('/') + 1);
				pic.Name = tmp.substr(0, tmp.rfind('.'));
				pic.Type = tmp.substr(tmp.rfind('.') + 1);
			
				if(stat(pic.Filename.c_str(), &statbuf) != 0)
					printf("stat error");
				pic.Date = statbuf.st_mtime;
				
				tmpPictureViewerGui.addToPlaylist(pic);
			}
		}
		
		tmpPictureViewerGui.exec(NULL, "urlplayback");

		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg != CRCInput::RC_home) 
		{
			goto BROWSER;
		}
	}
	
	delete fileBrowser;
}

void CTestMenu::testStartPlugin()
{
	g_PluginList->startPlugin("youtube");
}

void CTestMenu::testShowActuellEPG()
{
	std::string title = "testShowActuellEPG:";
	std::string buffer;

	// get EPG
	CEPGData epgData;
	event_id_t epgid = 0;
			
	if(sectionsd_getActualEPGServiceKey(live_channel_id&0xFFFFFFFFFFFFULL, &epgData))
		epgid = epgData.eventID;

	if(epgid != 0) 
	{
		CShortEPGData epgdata;
				
		if(sectionsd_getEPGidShort(epgid, &epgdata)) 
		{
			title += g_Zapit->getChannelName(live_channel_id);
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
	
	CInfoBox * infoBox = new CInfoBox(buffer.c_str(), g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1], CTextBox::SCROLL, &position, title.c_str(), g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE], NULL);
	
	infoBox->exec();
	delete infoBox;
	infoBox = NULL;	
}

void CTestMenu::testChannelSelectWidget()
{
	CSelectChannelWidget * CSelectChannelWidgetHandler = new CSelectChannelWidget();
	CSelectChannelWidgetHandler->exec(NULL, "tv");
		
	//CSelectChannelWidget_TVChanID;
	//CSelectChannelWidget_TVChanName.c_str();
		
	delete CSelectChannelWidgetHandler;
	CSelectChannelWidgetHandler = NULL;
}

void CTestMenu::testBEWidget()
{	
	CBEBouquetWidget* BEWidget = new CBEBouquetWidget();

	BEWidget->exec(NULL, "");

	delete BEWidget;
	BEWidget = NULL;
}

void CTestMenu::testAVSelectWidget()
{
	CAVPIDSelectWidget * AVSelectHandler = new CAVPIDSelectWidget();
	AVSelectHandler->exec(NULL, "");
		
	delete AVSelectHandler;
	AVSelectHandler = NULL;
}

void CTestMenu::testAudioSelectWidget()
{
	CAudioSelectMenuHandler * ASelectHandler = new CAudioSelectMenuHandler();
	ASelectHandler->exec(NULL, "");
	delete ASelectHandler;
	ASelectHandler = NULL;
}

void CTestMenu::testDVBSubSelectWidget()
{
	CDVBSubSelectMenuHandler * dvbSubSelectHandler = new CDVBSubSelectMenuHandler();
	dvbSubSelectHandler->exec(NULL, "");
	delete dvbSubSelectHandler;
	dvbSubSelectHandler = NULL;
}

/*
void CTestMenu::testAlphaSetupWidget()
{
	CAlphaSetup * alphaSetup = new CAlphaSetup(LOCALE_COLORMENU_GTX_ALPHA, &g_settings.gtx_alpha);
	alphaSetup->exec(NULL, "");
	delete alphaSetup;
	alphaSetup = NULL;
}
*/

/*
void CTestMenu::testPSISetup()
{
	CPSISetup * psiSetup = new CPSISetup(LOCALE_VIDEOMENU_PSISETUP, &g_settings.contrast, &g_settings.saturation, &g_settings.brightness, &g_settings.tint);
	psiSetup->exec(NULL, "");
	delete psiSetup;
	psiSetup = NULL;
}
*/

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
	CPluginList * pluginList = new CPluginList(/*LOCALE_USERMENU_ITEM_PLUGINS*/);
	pluginList->exec(NULL, "");
	delete pluginList;
	pluginList = NULL;
}

void CTestMenu::testPlayMovieDir()
{
	//
	CFileFilter fileFilter;
	
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
	//

	CMoviePlayerGui tmpMoviePlayerGui;
	
	std::string Path_local = g_settings.network_nfs_moviedir;

	CFileList filelist;

	if(CFileHelpers::getInstance()->readDir(Path_local, &filelist, &fileFilter))
	{
		// filter them
		MI_MOVIE_INFO mfile;
		CFileList::iterator files = filelist.begin();
		for(; files != filelist.end() ; files++)
		{
			mfile.file.Name = files->Name;
			mfile.epgTitle = files->getFileName();
			mfile.epgInfo1 = files->getFileName();

			std::string fname = "";
			fname = files->Name;
			changeFileNameExt(fname, ".jpg");
						
			if(!access(fname.c_str(), F_OK) )
				mfile.tfile = fname.c_str();
	
			tmpMoviePlayerGui.addToPlaylist(mfile);
		}
		
		tmpMoviePlayerGui.exec(NULL, "urlplayback");
	}
}

void CTestMenu::testPlayAudioDir()
{
	//
	CFileFilter fileFilter;
	
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

	CAudioPlayerGui tmpAudioPlayerGui;
	
	std::string Path_local = g_settings.network_nfs_audioplayerdir;

	CFileList filelist;

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
				CAudiofile audiofile(files->Name, files->getExtension());
				tmpAudioPlayerGui.addToPlaylist(audiofile);
			}
		}
		
		//tmpAudioPlayerGui.hidePlayList(true);
		tmpAudioPlayerGui.exec(NULL, "urlplayback");
	}
}

void CTestMenu::testShowPictureDir()
{
	//
	CFileFilter fileFilter;

	fileFilter.addFilter("png");
	fileFilter.addFilter("bmp");
	fileFilter.addFilter("jpg");
	fileFilter.addFilter("jpeg");

	CPictureViewerGui tmpPictureViewerGui;
	
	std::string Path_local = g_settings.network_nfs_picturedir;

	CFileList filelist;

	if(CFileHelpers::getInstance()->readDir(Path_local, &filelist, &fileFilter))
	{
		CPicture pic;
		struct stat statbuf;
				
		CFileList::iterator files = filelist.begin();
		for(; files != filelist.end() ; files++)
		{
			if (files->getType() == CFile::FILE_PICTURE)
			{
				pic.Filename = files->Name;
				std::string tmp = files->Name.substr(files->Name.rfind('/') + 1);
				pic.Name = tmp.substr(0, tmp.rfind('.'));
				pic.Type = tmp.substr(tmp.rfind('.') + 1);
			
				if(stat(pic.Filename.c_str(), &statbuf) != 0)
					printf("stat error");
				pic.Date = statbuf.st_mtime;
				
				tmpPictureViewerGui.addToPlaylist(pic);
			}
		}
		
		tmpPictureViewerGui.exec(NULL, "urlplayback");
	}
}

//// channel list
#define HEAD_BUTTONS_COUNT	3
const struct button_label HeadButtons[HEAD_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_HELP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_SETUP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_MUTE_ZAP_ACTIVE, NONEXISTANT_LOCALE, NULL }
};

#define FOOT_BUTTONS_COUNT 4
struct button_label FButtons[FOOT_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_RED, LOCALE_INFOVIEWER_EVENTLIST, NULL},
	{ NEUTRINO_ICON_BUTTON_GREEN, LOCALE_INFOVIEWER_NEXT, NULL},
	{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_BOUQUETLIST_HEAD, NULL},
	{ NEUTRINO_ICON_BUTTON_BLUE, LOCALE_EPGMENU_EPGPLUS, NULL},
};

void CTestMenu::testCMenuWidgetListBox()
{
	dprintf(DEBUG_NORMAL, "CTestMenu::testCMenuWidgetListBox\n");

	// itemBox
	listMenu = new ClistBox(LOCALE_CHANNELLIST_HEAD, "", w_max ( (frameBuffer->getScreenWidth() / 20 * 17), (frameBuffer->getScreenWidth() / 20 )), h_max ( (frameBuffer->getScreenHeight() / 20 * 16), (frameBuffer->getScreenHeight() / 20)));

	// load all tv channels
	Channels.clear();

	for (tallchans_iterator it = allchans.begin(); it != allchans.end(); it++)
	{
		if (it->second.getServiceType() != ST_DIGITAL_RADIO_SOUND_SERVICE)
			Channels.push_back(&(it->second));
	}

	// sort them
	sort(Channels.begin(), Channels.end(), CmpChannelByChName());

	// channels events
	CChannelEvent * p_event = NULL;

	//
	CNeutrinoApp::getInstance()->TVchannelList->updateEvents();

	//bool displayNext = false;
	if (displayNext) 
	{
		FButtons[1].locale = LOCALE_INFOVIEWER_NOW;
	} 
	else 
	{
		FButtons[1].locale = LOCALE_INFOVIEWER_NEXT;
	}

	std::string title;
	time_t jetzt = time(NULL);
	int runningPercent = 0;

	for(unsigned int i = 0; i < Channels.size(); i++)
	{
		// item description
		if (displayNext) 
		{
			p_event = &Channels[i]->nextEvent;
		} 
		else 
		{
			p_event = &Channels[i]->currentEvent;
		}

		// runningPercent
		runningPercent = 0;
			
		if (((jetzt - p_event->startTime + 30) / 60) < 0 )
		{
			runningPercent = 0;
		}
		else
		{
			//printf("(jetzt:%d) (p_event->startTime:%d) (p_event->duration:%d)\n", jetzt, p_event->startTime, p_event->duration);

			if(p_event->duration > 0)
				runningPercent = (jetzt - p_event->startTime) * 30 / p_event->duration;
		}

		//
		char cSeit[50] = " ";
		char cNoch[50] = " ";

		if ( p_event != NULL && !p_event->description.empty()) 
		{
			struct tm * pStartZeit = localtime(&p_event->startTime);
			unsigned seit = ( time(NULL) - p_event->startTime ) / 60;

			if (displayNext) 
			{
				sprintf(cNoch, "(%d min)", p_event->duration / 60);
				sprintf(cSeit, g_Locale->getText(LOCALE_CHANNELLIST_START), pStartZeit->tm_hour, pStartZeit->tm_min);
			} 
			else 
			{
				sprintf(cSeit, g_Locale->getText(LOCALE_CHANNELLIST_SINCE), pStartZeit->tm_hour, pStartZeit->tm_min);
				int noch = (p_event->startTime + p_event->duration - time(NULL)) / 60;
				if ((noch < 0) || (noch >= 10000))
					noch = 0;
				sprintf(cNoch, "(%d / %d min)", seit, noch);
			}
		}

		// a la Channelist
		item = new ClistBoxItem(Channels[i]->getName().c_str(), true, p_event->description.c_str(), this, "zapto");

		item->setNumber(i + 1);
		item->setPercent(runningPercent);
		item->setIcon1(Channels[i]->isHD() ? NEUTRINO_ICON_HD : "");
		item->setIcon2(Channels[i]->scrambled ? NEUTRINO_ICON_SCRAMBLED : "");
		item->setInfo1(p_event->description.c_str());
		item->setOptionInfo1(cSeit);
		item->setInfo2(p_event->text.c_str());
		item->setOptionInfo2(cNoch);

		listMenu->addItem(item);
	}

	listMenu->setWidgetType(WIDGET_STANDARD);
	listMenu->enableSaveScreen();
	listMenu->setTimeOut(g_settings.timing[SNeutrinoSettings::TIMING_CHANLIST]);
	listMenu->setSelected(selected);

	listMenu->setHeaderButtons(HeadButtons, HEAD_BUTTONS_COUNT);
	listMenu->setFooterButtons(FButtons, FOOT_BUTTONS_COUNT);
	
	listMenu->enablePaintDate();
	listMenu->enableFootInfo(); 

	// head
	listMenu->addKey(CRCInput::RC_info, this, CRCInput::getSpecialKeyName(CRCInput::RC_info));
	listMenu->addKey(CRCInput::RC_setup, this, CRCInput::getSpecialKeyName(CRCInput::RC_setup));

	// footer
	listMenu->addKey(CRCInput::RC_red, this, CRCInput::getSpecialKeyName(CRCInput::RC_red));
	listMenu->addKey(CRCInput::RC_green, this, CRCInput::getSpecialKeyName(CRCInput::RC_green));
	listMenu->addKey(CRCInput::RC_yellow, this, CRCInput::getSpecialKeyName(CRCInput::RC_yellow));
	listMenu->addKey(CRCInput::RC_blue, this, CRCInput::getSpecialKeyName(CRCInput::RC_blue));

	listMenu->exec(NULL, "");
	//listMenu->hide();
	delete listMenu;
	listMenu = NULL;
}

//// plugins list
#define NUM_LIST_BUTTONS 2
struct button_label CPluginListButtons[NUM_LIST_BUTTONS] =
{
	{ NEUTRINO_ICON_BUTTON_RED, LOCALE_PLUGINLIST_REMOVE_PLUGIN },
	{ NEUTRINO_ICON_BUTTON_GREEN, LOCALE_PLUGINLIST_START_PLUGIN }
};

void CTestMenu::testClistBoxnLines()
{
	dprintf(DEBUG_NORMAL, "CTestMenu::testClistBoxnLines\n");


	// itemBox
	listMenu = new ClistBox("ClistBox (plugins list)", NEUTRINO_ICON_SHELL, MENU_WIDTH, MENU_HEIGHT - 100);

	//
	for(unsigned int count = 0; count < (unsigned int)g_PluginList->getNumberOfPlugins(); count++)
	{
		std::string IconName = "";

		IconName = PLUGINDIR;
		IconName += "/";
		IconName += g_PluginList->getFileName(count);
		IconName += "/";
		IconName += g_PluginList->getIcon(count);

			
		item = new ClistBoxItem(g_PluginList->getName(count), true, g_PluginList->getDescription(count).c_str(), CPluginsExec::getInstance(), to_string(count).c_str(), NULL, file_exists(IconName.c_str())? IconName.c_str() : NEUTRINO_ICON_MENUITEM_PLUGIN);

		item->setInfo1(g_PluginList->getDescription(count).c_str());

		item->setnLinesItem();

		listMenu->addItem(item);
	}

	listMenu->setWidgetType(WIDGET_CLASSIC);
	listMenu->enablePaintDate();
	listMenu->enableFootInfo();
	listMenu->setSelected(selected);

	listMenu->addWidget(WIDGET_STANDARD);
	listMenu->addWidget(WIDGET_EXTENDED);
	listMenu->addWidget(WIDGET_FRAME);
	listMenu->addWidget(WIDGET_INFO);
	listMenu->enableWidgetChange();

	// footer
	listMenu->setFooterButtons(CPluginListButtons, NUM_LIST_BUTTONS, MENU_WIDTH/2);

	//
	listMenu->addKey(CRCInput::RC_red, this, "pred");
	listMenu->addKey(CRCInput::RC_green, this, "pgreen");

	listMenu->exec(NULL, "");
	listMenu->hide();
	delete listMenu;
	listMenu = NULL;
}

//// audioplayer
#define HEAD1_BUTTONS_COUNT	1
const struct button_label Head1Buttons[HEAD1_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_SETUP, NONEXISTANT_LOCALE, NULL }
};

const struct button_label AudioPlayerButtons[][4] =
{
	{
		{ NEUTRINO_ICON_BUTTON_RED, LOCALE_AUDIOPLAYER_STOP, NULL },
		{ NEUTRINO_ICON_BUTTON_GREEN, LOCALE_AUDIOPLAYER_REWIND, NULL },
		{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_AUDIOPLAYER_PAUSE, NULL},
		{ NEUTRINO_ICON_BUTTON_BLUE, LOCALE_AUDIOPLAYER_FASTFORWARD, NULL},
	},
	{
		{ NEUTRINO_ICON_BUTTON_RED   , LOCALE_AUDIOPLAYER_DELETE, NULL },
		{ NEUTRINO_ICON_BUTTON_GREEN , LOCALE_AUDIOPLAYER_ADD, NULL },
		{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_AUDIOPLAYER_DELETEALL, NULL },
		{ NEUTRINO_ICON_BUTTON_BLUE  , LOCALE_AUDIOPLAYER_SHUFFLE, NULL },
	},
	{		
		{ NEUTRINO_ICON_BUTTON_GREEN , LOCALE_AUDIOPLAYER_JUMP_BACKWARDS, NULL },
		{ NEUTRINO_ICON_BUTTON_BLUE  , LOCALE_AUDIOPLAYER_JUMP_FORWARDS, NULL },
	},
	{
		{ NEUTRINO_ICON_BUTTON_GREEN , LOCALE_AUDIOPLAYER_JUMP_BACKWARDS, NULL },
		{ NEUTRINO_ICON_BUTTON_BLUE  , LOCALE_AUDIOPLAYER_JUMP_FORWARDS, NULL },
	},
	{
		{ NEUTRINO_ICON_BUTTON_GREEN , LOCALE_AUDIOPLAYER_SAVE_PLAYLIST, NULL },
		{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_AUDIOPLAYER_BUTTON_SELECT_TITLE_BY_ID, NULL },
	},
	{
		{ NEUTRINO_ICON_BUTTON_GREEN , LOCALE_AUDIOPLAYER_SAVE_PLAYLIST, NULL },
		{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_AUDIOPLAYER_BUTTON_SELECT_TITLE_BY_NAME, NULL },
	},
	{
		{ NEUTRINO_ICON_BUTTON_RED   , LOCALE_AUDIOPLAYER_STOP, NULL },
		{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_AUDIOPLAYER_PAUSE, NULL },
	},
	{
		{ NEUTRINO_ICON_BUTTON_GREEN , LOCALE_AUDIOPLAYER_ADD, NULL },
		{ NEUTRINO_ICON_BUTTON_BLUE  , LOCALE_INETRADIO_NAME, NULL },
	},
	{
		{ NEUTRINO_ICON_BUTTON_RED   , LOCALE_AUDIOPLAYER_DELETE, NULL },
		{ NEUTRINO_ICON_BUTTON_GREEN , LOCALE_AUDIOPLAYER_ADD, NULL },
		{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_AUDIOPLAYER_DELETEALL, NULL },
		{ NEUTRINO_ICON_BUTTON_BLUE  , LOCALE_INETRADIO_NAME, NULL },
	},
};

void CTestMenu::testCMenuWidgetListBox1()
{
	//
	CFileFilter fileFilter;

	// music
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

	// itemBox
	listMenu = new ClistBox("ClistBox (audioplayer)", NEUTRINO_ICON_MP3, w_max ( (frameBuffer->getScreenWidth() / 20 * 17), (frameBuffer->getScreenWidth() / 20 )), h_max ( (frameBuffer->getScreenHeight() / 20 * 16), (frameBuffer->getScreenHeight() / 20)));

	audioFileList.clear();

	if(CFileHelpers::getInstance()->readDir(Path_local, &audioFileList, &fileFilter))
	{
		int count = 0;
				
		CFileList::iterator files = audioFileList.begin();
		for(; files != audioFileList.end() ; files++)
		{
			count++;
			if (files->getType() == CFile::FILE_AUDIO)
			{
				std::string title;
				std::string artist;
				std::string genre;
				std::string date;
				char duration[9] = "";

				// metaData
				CAudiofile audiofile(files->Name, files->getExtension());

				CAudioPlayer::getInstance()->init();

				int ret = CAudioPlayer::getInstance()->readMetaData(&audiofile, true);

				if (!ret || (audiofile.MetaData.artist.empty() && audiofile.MetaData.title.empty() ))
				{
					// //remove extension (.mp3)
					std::string tmp = files->getFileName().substr(files->getFileName().rfind('/') + 1);
					tmp = tmp.substr(0, tmp.length() - 4);	//remove extension (.mp3)

					std::string::size_type i = tmp.rfind(" - ");
		
					if(i != std::string::npos)
					{ 
						title = tmp.substr(0, i);
						artist = tmp.substr(i + 3);
					}
					else
					{
						i = tmp.rfind('-');
						if(i != std::string::npos)
						{
							title = tmp.substr(0, i);
							artist = tmp.substr(i + 1);
						}
						else
							title = tmp;
					}
				}
				else
				{
					title = audiofile.MetaData.title;
					artist = audiofile.MetaData.artist;
					genre = audiofile.MetaData.genre;	
					date = audiofile.MetaData.date;

					snprintf(duration, 8, "(%ld:%02ld)", audiofile.MetaData.total_time / 60, audiofile.MetaData.total_time % 60);
				}

				//
				item = new ClistBoxItem(title.c_str(), true, "", this, "aplay");
			
				item->setOptionInfo(duration);
				item->setNumber(count);

				// details Box
				item->setInfo1(title.c_str());
				item->setOptionInfo1(genre.c_str());
				item->setInfo2(artist.c_str());
				item->setOptionInfo2(date.c_str());

				listMenu->addItem(item);
			}
		}
	}

	//listMenu->move(0, 40);
	//listMenu->setTimeOut(g_settings.timing[SNeutrinoSettings::TIMING_CHANLIST]);
	listMenu->setSelected(selected);

	listMenu->setHeaderButtons(Head1Buttons, HEAD1_BUTTONS_COUNT);
	listMenu->setFooterButtons(AudioPlayerButtons[0], FOOT_BUTTONS_COUNT);
	
	listMenu->enablePaintDate();
	listMenu->enableFootInfo();
	//listMenu->setFootInfoHeight(30); 

	listMenu->addKey(CRCInput::RC_info, this, "ainfo");
	listMenu->addKey(CRCInput::RC_setup, this, "asetup");
	listMenu->addKey(CRCInput::RC_red, this, "ared");
	listMenu->addKey(CRCInput::RC_green, this, "agreen");
	listMenu->addKey(CRCInput::RC_yellow, this, "ayellow");
	listMenu->addKey(CRCInput::RC_blue, this, "ablue");
	listMenu->addKey(CRCInput::RC_stop, this, "astop");

	listMenu->exec(NULL, "");
	//listMenu->hide();
	delete listMenu;
	listMenu = NULL;
}

//// CMenulist (frame| extended widget) movie browser
#define mHEAD_BUTTONS_COUNT	2
const struct button_label mHeadButtons[mHEAD_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_HELP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_SETUP, NONEXISTANT_LOCALE, NULL },
};

void CTestMenu::testFrameBox()
{
	listMenu = new ClistBox("Movie Browser", NEUTRINO_ICON_MOVIE, w_max ( (CFrameBuffer::getInstance()->getScreenWidth() / 20 * 17), (CFrameBuffer::getInstance()->getScreenWidth() / 20 )), h_max ( (CFrameBuffer::getInstance()->getScreenHeight() / 20 * 17), (CFrameBuffer::getInstance()->getScreenHeight() / 20)));
	
	//
	CFileFilter fileFilter;
	
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

	CFileList filelist;
	
	// recordingdir
	std::string Path_local = g_settings.network_nfs_recordingdir;
	
	//
	if(CFileHelpers::getInstance()->readDir(Path_local, &filelist, &fileFilter))
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

	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, m_vMovieInfo[i].epgChannel.c_str(), this, "mplay", NULL, file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

		//
		std::string tmp = m_vMovieInfo[i].epgTitle;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo1;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo2;

		item->setInfo1(tmp.c_str());

		//item->setOptionFont(g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]);

		listMenu->addItem(item);
	}

	listMenu->setWidgetType(WIDGET_CLASSIC);
	listMenu->setItemsPerPage(6, 2);
	listMenu->setItemBoxColor(COL_YELLOW);

	listMenu->addWidget(WIDGET_STANDARD);
	listMenu->addWidget(WIDGET_FRAME);
	listMenu->addWidget(WIDGET_EXTENDED);
	listMenu->addWidget(WIDGET_INFO);
	listMenu->enableWidgetChange();

	listMenu->setSelected(selected);

	listMenu->setHeaderButtons(mHeadButtons, mHEAD_BUTTONS_COUNT);

	listMenu->enablePaintDate();

	listMenu->addKey(CRCInput::RC_info, this, "minfo");

	listMenu->exec(NULL, "");
	listMenu->hide();
	delete listMenu;
	listMenu = NULL;
}

// CMenuWidget
void CTestMenu::testCMenuWidget()
{
	int shortcut = 1;

	dprintf(DEBUG_NORMAL, "testCMenuWidget\n");

	CMenuWidget* mainMenu = new CMenuWidget(LOCALE_MAINMENU_HEAD, NEUTRINO_ICON_MAINMENU);

	mainMenu->enableMenuPosition();
	mainMenu->enableWidgetChange();
	  
	// tv modus
	mainMenu->addItem(new CMenuForwarder(LOCALE_MAINMENU_TVMODE, true, NULL, this, "tv", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED, NEUTRINO_ICON_MENUITEM_TV, LOCALE_HELPTEXT_TVMODE ), true);

	// radio modus
	mainMenu->addItem(new CMenuForwarder(LOCALE_MAINMENU_RADIOMODE, true, NULL, this, "radio", CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN, NEUTRINO_ICON_MENUITEM_RADIO, LOCALE_HELPTEXT_RADIOMODE ));	
	
	// webtv
	mainMenu->addItem(new CMenuForwarder(LOCALE_MAINMENU_WEBTVMODE, true, NULL, this, "webtv", CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW, NEUTRINO_ICON_MENUITEM_WEBTV, LOCALE_HELPTEXT_WEBTVMODE) );
	
#if defined (ENABLE_SCART)
	// scart
	mainMenu->addItem(new CMenuForwarder(LOCALE_MAINMENU_SCARTMODE, true, NULL, this, "scart", CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE, NEUTRINO_ICON_MENUITEM_SCART, LOCALE_HELPTEXT_SCART) );
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
	mainMenu->addItem(new CMenuForwarder(LOCALE_MAINMENU_POWERMENU, true, NULL, new CPowerMenu(), NULL, CRCInput::RC_standby, NEUTRINO_ICON_BUTTON_POWER, NEUTRINO_ICON_MENUITEM_POWERMENU, LOCALE_HELPTEXT_POWERMENU ));

	//box info
	mainMenu->addItem( new CMenuForwarder(LOCALE_DBOXINFO, true, NULL, new CDBoxInfoWidget, NULL, CRCInput::RC_info, NEUTRINO_ICON_BUTTON_HELP, NEUTRINO_ICON_MENUITEM_BOXINFO, LOCALE_HELPTEXT_BOXINFO ));

	mainMenu->integratePlugins(CPlugins::I_TYPE_MAIN, shortcut++);

	mainMenu->exec(NULL, "");
	mainMenu->hide();
	delete mainMenu;
	mainMenu = NULL;
}

void CTestMenu::testChannellist()
{
	g_WebTV->loadChannels();

	Channels.clear();

	Channels = g_WebTV->getChannels();

	webTVchannelList = new CChannelList("CTestMenu::testChannellist:");

	webTVchannelList->setSize(Channels.size());

	for(unsigned count = 0; count < Channels.size(); count++)
	{
		webTVchannelList->addChannel(Channels[count]);
	}

	webTVchannelList->exec();
}

void CTestMenu::testBouquetlist()
{
	webTVBouquetList = new CBouquetList("CTestMenu::testBouquetlist");

	CBouquet* webBouquet = NULL;

	CFileFilter fileFilter;
	
	fileFilter.addFilter("xml");
	fileFilter.addFilter("tv");
	fileFilter.addFilter("m3u");

	//
	CFileList filelist;

	if(CFileHelpers::getInstance()->readDir(CONFIGDIR "/webtv", &filelist, &fileFilter))
	{
		std::string bTitle;

		for (unsigned int i = 0; i < filelist.size(); i++)
		{
			g_settings.webtv_userBouquet = filelist[i].Name.c_str();
			g_WebTV->loadChannels();
			Channels.clear();
			Channels = g_WebTV->getChannels();

			bTitle = filelist[i].getFileName();

			removeExtension(bTitle);

			webBouquet = new CBouquet(0, (char *)bTitle.c_str(), 0);

			for(unsigned count = 0; count < Channels.size(); count++)
			{
				webBouquet->channelList->addChannel(Channels[count]);
			}

			webTVBouquetList->Bouquets.push_back(webBouquet);
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
		
		if (msg == CRCInput::RC_home) 
		{
			break;
		}
	}
}

int CTestMenu::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "\nCTestMenu::exec: actionKey:%s\n", actionKey.c_str());
	
	if(parent)
		hide();
	
	if(actionKey == "box")
	{
		testCBox();
	}
	else if(actionKey == "icon")
	{
		testCIcon();
	}
	else if(actionKey == "image")
	{
		testCImage();
	}
	else if(actionKey == "window")
	{
		testCWindow();
	}
	else if(actionKey == "windowshadow")
	{
		testCWindowShadow();
	}
	else if(actionKey == "stringinput")
	{
		testCStringInput();
	}
	else if(actionKey == "stringinputsms")
	{
		testCStringInputSMS();
	}
	else if(actionKey == "pininput")
	{
		testCPINInput();
	}
	else if(actionKey == "ipinput")
	{
		testCIPInput();
	}
	else if(actionKey == "macinput")
	{
		testCMACInput();
	}
	else if(actionKey == "dateinput")
	{
		testCDateInput();
	}
	else if(actionKey == "timeinput")
	{
		testCTimeInput();
	}
	else if(actionKey == "intinput")
	{
		testCIntInput();
	}
	else if(actionKey == "infobox")
	{
		testCInfoBox();
	}
	else if(actionKey == "messagebox")
	{
		testCMessageBox();
	}
	else if(actionKey == "messageboxinfomsg")
	{
		testCMessageBoxInfoMsg();
	}
	else if(actionKey == "messageboxerrormsg")
	{
		testCMessageBoxErrorMsg();
	}
	else if(actionKey == "hintbox")
	{
		testCHintBox();
	}
	else if(actionKey == "hintboxinfo")
	{
		testCHintBoxInfo();
	}
	else if(actionKey == "helpbox")
	{
		testCHelpBox();
	}
	else if(actionKey == "textbox")
	{
		testCTextBox();
	}
	else if(actionKey == "listframe")
	{
		testCListFrame();
	}
	else if(actionKey == "progressbar")
	{
		testCProgressBar();
	}
	else if(actionKey == "progresswindow")
	{
		testCProgressWindow();
	}
	else if(actionKey == "buttons")
	{
		testCButtons();
	}
	else if(actionKey == "listboxentry")
	{
		testClistBoxEntry();
	}
	else if(actionKey == "audioplayer")
	{
		testAudioPlayer();
	}
	else if(actionKey == "tsplayer")
	{
		testTSPlayer();
	}
	else if(actionKey == "movieplayer")
	{
		testMoviePlayer();
	}
	else if(actionKey == "pictureviewer")
	{
		testPictureViewer();
	}
	else if(actionKey == "playmovieurl")
	{
		testPlayMovieURL();
	}
	else if(actionKey == "playaudiourl")
	{
		testPlayAudioURL();
	}
	else if(actionKey == "showpictureurl")
	{
		testShowPictureURL();
	}
	else if(actionKey == "playmoviefolder")
	{
		testPlayMovieFolder();
	}
	else if(actionKey == "playaudiofolder")
	{
		testPlayAudioFolder();
	}
	else if(actionKey == "showpicturefolder")
	{
		testShowPictureFolder();
	}
	else if(actionKey == "startplugin")
	{
		testStartPlugin();
	}
	else if(actionKey == "showepg")
	{
		testShowActuellEPG();
	}
	else if(actionKey == "channelselect")
	{
		testChannelSelectWidget();
	}
	else if(actionKey == "bewidget")
	{
		testBEWidget();
	}
	else if(actionKey == "avselect")
	{
		testAVSelectWidget();
	}
	else if(actionKey == "aselect")
	{
		testAudioSelectWidget();
	}
	else if(actionKey == "dvbsubselect")
	{
		testDVBSubSelectWidget();
	}
	/*
	else if(actionKey == "alphasetup")
	{
		testAlphaSetupWidget();
	}
	*/
	/*
	else if(actionKey == "psisetup")
	{
		testPSISetup();
	}
	*/
	else if(actionKey == "rclock")
	{
		testRCLock();
	}
	else if(actionKey == "sleeptimer")
	{
		testSleepTimerWidget();
	}
	else if(actionKey == "mountgui")
	{
		testMountGUI();
	}
	else if(actionKey == "umountgui")
	{
		testUmountGUI();
	}
	else if(actionKey == "mountsmallmenu")
	{
		testMountSmallMenu();
	}
	else if(actionKey == "vfdcontroller")
	{
		testVFDController();
	}
	else if(actionKey == "colorchooser")
	{
		testColorChooser();
	}
	else if(actionKey == "keychooser")
	{
		testKeyChooser();
	}
	else if(actionKey == "mountchooser")
	{
		testMountChooser();
	}
	else if(actionKey == "framebox")
	{
		testFrameBox();
	}
	else if(actionKey == "pluginslist")
	{
		testPluginsList();
	}
	else if(actionKey == "playmoviedir")
	{
		testPlayMovieDir();
	}
	else if(actionKey == "playaudiodir")
	{
		testPlayAudioDir();
	}
	else if(actionKey == "showpicturedir")
	{
		testShowPictureDir();
	}
	else if(actionKey == "menuwidgetlistbox")
	{
		testCMenuWidgetListBox();
	}
	else if(actionKey == "RC_setup")
	{
		CChannelListSettings * testChannelMenu = new CChannelListSettings();
		testChannelMenu->exec(NULL, "");
		delete testChannelMenu;
		testChannelMenu = NULL;		
	}
	else if(actionKey == "RC_info")
	{
		g_EpgData->show(Channels[listMenu->getSelected()]->channel_id);
		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "RC_red")
	{
		g_EventList->exec(Channels[listMenu->getSelected()]->channel_id, Channels[listMenu->getSelected()]->getName());
	}
	else if(actionKey == "RC_green")
	{
		selected = listMenu->getSelected();
		displayNext = !displayNext;
		testCMenuWidgetListBox();
		return menu_return::RETURN_EXIT_ALL;		
	}
	else if(actionKey == "RC_yellow")
	{
		bouquetList->exec(true);
	}
	else if(actionKey == "RC_blue")
	{
		CEPGplusHandler eplus;
		eplus.exec(NULL, "");
	}
	else if(actionKey == "zapto")
	{
		selected = listMenu->getSelected();
		g_Zapit->zapTo_serviceID(Channels[listMenu->getSelected()]->channel_id);
		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "menuwidgetlistbox1")
	{
		testCMenuWidgetListBox1();
	}
	else if(actionKey == "aplay")
	{
		selected = listMenu->getSelected();
		CAudiofile audiofile(audioFileList[listMenu->getSelected()].Name, audioFileList[listMenu->getSelected()].getExtension());

		CAudioPlayer::getInstance()->init();
		CNeutrinoApp::getInstance()->handleMsg(NeutrinoMessages::CHANGEMODE, NeutrinoMessages::mode_audio);

		CAudiofile mp3(audioFileList[listMenu->getSelected()].Name, audioFileList[listMenu->getSelected()].getExtension());
		CAudioPlayer::getInstance()->play(&mp3, g_settings.audioplayer_highprio == 1);		
	}
	else if(actionKey == "astop")
	{

		if (CAudioPlayer::getInstance()->getState() != CBaseDec::STOP)
		{
			CAudioPlayer::getInstance()->stop();
		}

	}
	else if(actionKey == "asetup")
	{
		CAudioPlayerSettings * audioPlayerSettingsMenu = new CAudioPlayerSettings();
		audioPlayerSettingsMenu->exec(this, "");
		delete audioPlayerSettingsMenu;
		audioPlayerSettingsMenu = NULL;	
	}
	else if(actionKey == "testmenuwidget")
	{
		testCMenuWidget();
	}
	else if(actionKey == "listboxnlines")
	{
		testClistBoxnLines();
	}
	else if(actionKey == "pred")
	{
		g_PluginList->removePlugin(listMenu->getSelected());
		//return menu_return::RETURN_EXIT_ALL;
		listMenu->hide();
		listMenu->exec(NULL, "");
	}
	else if(actionKey == "pgreen")
	{
		g_PluginList->startPlugin(listMenu->getSelected());
	}
	else if(actionKey == "mplay")
	{
		selected = listMenu->getSelected();
		CMoviePlayerGui tmpMoviePlayerGui;

		if (&m_vMovieInfo[listMenu->getSelected()].file != NULL) 
		{
			tmpMoviePlayerGui.addToPlaylist(m_vMovieInfo[listMenu->getSelected()]);
			tmpMoviePlayerGui.exec(NULL, "urlplayback");
		}
	}
	else if(actionKey == "minfo")
	{
		selected = listMenu->getSelected();
		m_movieInfo.showMovieInfo(m_vMovieInfo[listMenu->getSelected()]);
	}
	else if(actionKey == "channellist")
	{
		testChannellist();
	}
	else if(actionKey == "bouquetlist")
	{
		testBouquetlist();
	}
	else if(actionKey == "spinner")
	{
		spinner();
	}
	
	return menu_return::RETURN_REPAINT;
}

void CTestMenu::showTestMenu()
{
	/// menue.cpp
	CMenuWidget * mainMenu = new CMenuWidget("testMenu", NEUTRINO_ICON_BUTTON_SETUP);

	mainMenu->enableMenuPosition();
	
	mainMenu->addItem(new CMenuForwarder("CBox", true, NULL, this, "box"));
	mainMenu->addItem(new CMenuForwarder("CIcon", true, NULL, this, "icon"));
	mainMenu->addItem(new CMenuForwarder("CImage", true, NULL, this, "image"));
	mainMenu->addItem(new CMenuForwarder("CWindow", true, NULL, this, "window"));
	mainMenu->addItem(new CMenuForwarder("CWindow(with shadow)", true, NULL, this, "windowshadow"));
	mainMenu->addItem(new CMenuForwarder("CStringInput", true, NULL, this, "stringinput"));
	mainMenu->addItem(new CMenuForwarder("CStringInputSMS", true, NULL, this, "stringinputsms"));
	mainMenu->addItem(new CMenuForwarder("CPINInput", true, NULL, this, "pininput"));
	mainMenu->addItem(new CMenuForwarder("CIPInput", true, NULL, this, "ipinput"));
	mainMenu->addItem(new CMenuForwarder("CMACInput", true, NULL, this, "macinput"));
	mainMenu->addItem(new CMenuForwarder("CDateInput", true, NULL, this, "dateinput"));
	mainMenu->addItem(new CMenuForwarder("CTimeInput", true, NULL, this, "timeinput"));
	mainMenu->addItem(new CMenuForwarder("CIntInput", true, NULL, this, "intinput"));
	mainMenu->addItem(new CMenuForwarder("CInfoBox", true, NULL, this, "infobox"));
	mainMenu->addItem(new CMenuForwarder("CMessageBox", true, NULL, this, "messagebox"));
	mainMenu->addItem(new CMenuForwarder("CMessageBoxInfoMsg", true, NULL, this, "messageboxinfomsg"));
	mainMenu->addItem(new CMenuForwarder("CMessageBoxErrorMsg", true, NULL, this, "messageboxerrormsg"));
	mainMenu->addItem(new CMenuForwarder("CHintBox", true, NULL, this, "hintbox"));
	mainMenu->addItem(new CMenuForwarder("CHintBoxInfo", true, NULL, this, "hintboxinfo"));
	mainMenu->addItem(new CMenuForwarder("CHelpBox", true, NULL, this, "helpbox"));
	mainMenu->addItem(new CMenuForwarder("CTextBox", true, NULL, this, "textbox"));
	mainMenu->addItem(new CMenuForwarder("CListFrame", true, NULL, this, "listframe"));
	mainMenu->addItem(new CMenuForwarder("CProgressBar", true, NULL, this, "progressbar"));
	mainMenu->addItem(new CMenuForwarder("CProgressWindow", true, NULL, this, "progresswindow"));
	mainMenu->addItem(new CMenuForwarder("ColorChooser", true, NULL, this, "colorchooser"));
	mainMenu->addItem(new CMenuForwarder("KeyChooser", true, NULL, this, "keychooser"));
	mainMenu->addItem(new CMenuForwarder("VFDController", true, NULL, this, "vfdcontroller"));
	mainMenu->addItem(new CMenuForwarder("MountChooser", true, NULL, this, "mountchooser"));
	mainMenu->addItem(new CMenuForwarder("CButtons", true, NULL, this, "buttons"));
	mainMenu->addItem(new CMenuForwarder("ClistBoxEntry", true, NULL, this, "listboxentry"));
	mainMenu->addItem(new CMenuForwarder("ClistBox(movie browser)", true, NULL, this, "framebox"));
	mainMenu->addItem(new CMenuForwarder("ClistBox(channellist)", true, NULL, this, "menuwidgetlistbox"));
	mainMenu->addItem(new CMenuForwarder("ClistBox(Audioplayer)", true, NULL, this, "menuwidgetlistbox1"));
	mainMenu->addItem(new CMenuForwarder("ClistBox(plugins list)", true, NULL, this, "listboxnlines"));
	mainMenu->addItem(new CMenuForwarder("CMenuWidget", true, NULL, this, "testmenuwidget"));
	
	mainMenu->addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	mainMenu->addItem(new CMenuForwarder("ShowActuellEPG", true, NULL, this, "showepg"));
	mainMenu->addItem(new CMenuForwarder("ChannelSelectWidget", true, NULL, this, "channelselect"));
	mainMenu->addItem(new CMenuForwarder("BEWidget", true, NULL, this, "bewidget"));
	mainMenu->addItem(new CMenuForwarder("AudioVideoSelectWidget", true, NULL, this, "avselect"));
	mainMenu->addItem(new CMenuForwarder("AudioSelectWidget", true, NULL, this, "aselect"));
	mainMenu->addItem(new CMenuForwarder("DVBSubSelectWidget", true, NULL, this, "dvbsubselect"));
	//mainMenu->addItem(new CMenuForwarder("AlphaSetup", true, NULL, this, "alphasetup"));
	//mainMenu->addItem(new CMenuForwarder("PSISetup", true, NULL, this, "psisetup"));
	mainMenu->addItem(new CMenuForwarder("RCLock", true, NULL, this, "rclock"));
	mainMenu->addItem(new CMenuForwarder("SleepTimerWidget", true, NULL, this, "sleeptimer"));
	mainMenu->addItem(new CMenuForwarder("MountGUI", true, NULL, this, "mountgui"));
	mainMenu->addItem(new CMenuForwarder("UmountGUI", true, NULL, this, "umountgui"));
	mainMenu->addItem(new CMenuForwarder("MountSmallMenu", true, NULL, this, "mountsmallmenu"));

	mainMenu->addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	mainMenu->addItem(new CMenuForwarder("PluginsList", true, NULL, this, "pluginslist"));

	mainMenu->addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	mainMenu->addItem(new CMenuForwarder("AudioPlayer", true, NULL, this, "audioplayer"));
	mainMenu->addItem(new CMenuForwarder("TS Player", true, NULL, this, "tsplayer"));
	mainMenu->addItem(new CMenuForwarder("Movie Player", true, NULL, this, "movieplayer"));
	mainMenu->addItem(new CMenuForwarder("PictureViewer", true, NULL, this, "pictureviewer"));

	mainMenu->addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	mainMenu->addItem(new CMenuForwarder("PlayMovieURL", true, NULL, this, "playmovieurl"));
	mainMenu->addItem(new CMenuForwarder("PlayAudioURL", true, NULL, this, "playaudiourl"));
	mainMenu->addItem(new CMenuForwarder("ShowPictureURL", true, NULL, this, "showpictureurl"));

	mainMenu->addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	mainMenu->addItem(new CMenuForwarder("PlayMovieFolder", true, NULL, this, "playmoviefolder"));
	mainMenu->addItem(new CMenuForwarder("PlayAudioFolder", true, NULL, this, "playaudiofolder"));
	mainMenu->addItem(new CMenuForwarder("ShowPictureFolder", true, NULL, this, "showpicturefolder"));

	mainMenu->addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	mainMenu->addItem(new CMenuForwarder("PlayMovieDir(without Browser)", true, NULL, this, "playmoviedir"));
	mainMenu->addItem(new CMenuForwarder("PlayAudioDir(without Browser)", true, NULL, this, "playaudiodir"));
	mainMenu->addItem(new CMenuForwarder("ShowPictureDir(without Browser)", true, NULL, this, "showpicturedir"));

	mainMenu->addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	mainMenu->addItem(new CMenuForwarder("StartPlugin(e.g: youtube)", true, NULL, this, "startplugin"));

	mainMenu->addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	mainMenu->addItem(new CMenuForwarder("CChannelList:", true, NULL, this, "channellist"));
	mainMenu->addItem(new CMenuForwarder("CBouquetList:", true, NULL, this, "bouquetlist"));

	//mainMenu->addItem(new CMenuForwarder("Spinner", true, NULL, this, "spinner"));
	
	mainMenu->exec(NULL, "");
	mainMenu->hide();
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
	CTestMenu* testMenu = new CTestMenu();
	
	testMenu->showTestMenu();
	
	delete testMenu;
}


