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
		ClistBox* listMenu;
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
		CFileList audioFileList;

		//
		CMovieInfo m_movieInfo;
		std::vector<MI_MOVIE_INFO> m_vMovieInfo;

		//
		CChannelList* webTVchannelList;
		CBouquetList* webTVBouquetList;

		CButtons buttons;
		CHeaders headers;

		// testing
		void testCFrameBox();

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

		//
		void testCTextBox();
		void testCListFrame();
		void testClistBoxEntry();

		//
		void testClistBox();
		void testCMenuWidget();

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
		//void testAlphaSetupWidget();
		//void testPSISetup();
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

		// our widgetMenu
		void showMenu();

	public:
		CTestMenu();
		~CTestMenu();
		int exec(CMenuTarget* parent, const std::string& actionKey);
		void hide();
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

// CFrameBox
void CTestMenu::testCFrameBox()
{
	dprintf(DEBUG_NORMAL, "\ntesting CFrameBox\n");

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

	// frameBox
	CBox Box;
	
	Box.iX = g_settings.screen_StartX + 10;
	Box.iY = headBox.iY + headBox.iHeight + 5;
	Box.iWidth = (g_settings.screen_EndX - g_settings.screen_StartX - 20);
	Box.iHeight = 120; //(g_settings.screen_EndY - g_settings.screen_StartY - 20);

	CFrameBox *frameBox = new CFrameBox(&Box);
	CFrame * frame = NULL;

	for(unsigned int count = 0; count < 5; count++)
	{
		std::string tmp = "frame-";
		tmp += to_string(count).c_str();
		frame = new CFrame(tmp);

		frameBox->addFrame(frame);
	}

	frameBox->setSelected(0);
	//frameBox->setOutFocus(false);

	// menu left
	CBox leftBox;
	leftBox.iX = g_settings.screen_StartX + 10;
	leftBox.iY = headBox.iY + headBox.iHeight + 5 + Box.iHeight + 5;
	leftBox.iWidth = 150;
	leftBox.iHeight = (g_settings.screen_EndY - g_settings.screen_StartY - 20) - headBox.iHeight - 5 - Box.iHeight - 5 - footBox.iHeight - 5;

	ClistBoxEntry *listBoxEntry = new ClistBoxEntry(&leftBox);

	ClistBoxEntryItem *item1 = new ClistBoxEntryItem("item 1");
	ClistBoxEntryItem *item2 = new ClistBoxEntryItem("item 2");
	item2->setOption("option");
	item2->setnLinesItem();
	ClistBoxEntryItem *item3 = new ClistBoxEntryItem("item 3");
	ClistBoxEntryItem *item4 = new ClistBoxEntryItem("item 4");
	
	CMenuForwarder *item5 = new CMenuForwarder("item 1", true, "", NULL, "");

	listBoxEntry->addItem(item1);
	listBoxEntry->addItem(item2);
	listBoxEntry->addItem(item3);
	listBoxEntry->addItem(item4);
	listBoxEntry->addItem(item5);

	listBoxEntry->disableCenter();
	listBoxEntry->setSelected(-1);
	listBoxEntry->setOutFocus(true);
	listBoxEntry->disableShrinkMenu();

	// right menu
	CBox rightBox;

	rightBox.iX = g_settings.screen_StartX + 10 + leftBox.iWidth + 5;
	rightBox.iY = headBox.iY + headBox.iHeight + 5 + Box.iHeight + 5;
	rightBox.iWidth = (g_settings.screen_EndX - g_settings.screen_StartX - 20) - 150 - 5;
	rightBox.iHeight = (g_settings.screen_EndY - g_settings.screen_StartY - 20) - headBox.iHeight - 5 - Box.iHeight - 5 - footBox.iHeight - 5;

	CTextBox * textBox = new CTextBox("CTextBox", g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1], CTextBox::SCROLL, &rightBox, COL_MENUCONTENT_PLUS_0);
	
	std::string text = "testing CTextBox: blah blah blah boaah";
		
	int pich = 246;	//FIXME
	int picw = 162; 	//FIXME
		
	std::string fname = PLUGINDIR "/netzkino/netzkino.png";
		
	if(access(fname.c_str(), F_OK))
		fname = "";
	
	textBox->setText(&text, fname, picw, pich);
	

	int focus = 0; // frameBox

	// loop
	//g_RCInput->messageLoop();
REPEAT:
	// paint all
	headers.paintHead(headBox, NEUTRINO_ICON_MP3, "CFrameBox", true);
	headers.paintFoot(footBox);
	frameBox->paint();
	listBoxEntry->paint();
	textBox->paint();

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
			headers.paintHead(headBox, NEUTRINO_ICON_MP3, "CFrameBox", true);
		} 
		else if (msg == CRCInput::RC_home) 
		{
			loop = false;
		}
		else if(msg == CRCInput::RC_right)
		{
			if(focus == 0)
				frameBox->swipRight();
		}
		else if(msg == CRCInput::RC_left)
		{
			if(focus == 0)
				frameBox->swipLeft();
		}
		else if(msg == CRCInput::RC_up)
		{
			if(focus == 1)
				listBoxEntry->scrollLineUp();
		}
		else if(msg == CRCInput::RC_down)
		{
			if(focus == 1)
				listBoxEntry->scrollLineDown();
		}
		else if(msg == CRCInput::RC_yellow)
		{
			if(focus == 0)
			{
				focus = 1;

				listBoxEntry->setSelected(0);
				listBoxEntry->setOutFocus(false);

				frameBox->setSelected(-1);
				frameBox->setOutFocus(true);
			}
			else if (focus == 1)
			{
				focus = 2;

				frameBox->setSelected(-1);
				frameBox->setOutFocus(true);

				listBoxEntry->setSelected(-1);
				listBoxEntry->setOutFocus(true);
			}
			else if (focus == 2)
			{
				focus = 0;

				listBoxEntry->setSelected(-1);
				listBoxEntry->setOutFocus(true);

				frameBox->setSelected(0);
				frameBox->setOutFocus(false);
			}

			goto REPEAT;
		}
		else if(msg == CRCInput::RC_ok)
		{

			//hide();

			selected = frameBox->getSelected();

			MessageBox(LOCALE_MESSAGEBOX_INFO, "\nhuhu\nHUHUUU\n", CMessageBox::mbrBack, CMessageBox::mbBack, NEUTRINO_ICON_INFO);

			goto REPEAT;
		}

		CFrameBuffer::getInstance()->blit();
	}


	hide();

	delete frameBox;
	frameBox = NULL;

	delete textBox;
	textBox = NULL;

	delete listBoxEntry;
	listBoxEntry = NULL;

	g_RCInput->killTimer(sec_timer_id);
	sec_timer_id = 0;
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

	// set custom color
#define COL_TEST 254-8*13
#define COL_TEST_PLUS_0              (CFrameBuffer::getInstance()->realcolor[(COL_TEST + 0)])

	setColor(COL_TEST, 50, 50, 50, 50);
	//

	CBox Box;
	
	Box.iX = g_settings.screen_StartX + 10;
	Box.iY = g_settings.screen_StartY + 10;
	Box.iWidth = (g_settings.screen_EndX - g_settings.screen_StartX - 20)/2;
	Box.iHeight = 200; //(g_settings.screen_EndY - g_settings.screen_StartY - 20);

	//
	CWindow* window = new CWindow(&Box);

	window->setColor(COL_TEST_PLUS_0);
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

// CStringInput
void CTestMenu::testCStringInput()
{
	dprintf(DEBUG_NORMAL, "\ntesting CStringInput\n");

	std::string value;
	CStringInput * stringInput = new CStringInput("CStringInput", &value);
	
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
	CStringInputSMS * stringInputSMS = new CStringInputSMS("CStringInputSMS", &value);
	
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
	CPINInput * pinInput = new CPINInput("CPINInput", (char*)value.c_str());
	
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

// CMessageBox
void CTestMenu::testCMessageBox()
{
	dprintf(DEBUG_NORMAL, "\ntesting CMessageBox\n");

	CMessageBox * messageBox = new CMessageBox(LOCALE_MESSAGEBOX_INFO, "testing CMessageBox"/*, 600, NEUTRINO_ICON_INFO, CMessageBox::mbrYes, CMessageBox::mbNone*/);
	
	int res = messageBox->exec();

	printf("res:%d messageBox->result:%d\n", res, messageBox->result);

	delete messageBox;
	messageBox = NULL;
}

// MessageBox
void CTestMenu::testCMessageBoxInfoMsg()
{
	dprintf(DEBUG_NORMAL, "\ntesting MessageBox\n");

	MessageBox(LOCALE_MESSAGEBOX_INFO, "testing CMessageBoxInfoMsg", CMessageBox::mbrBack, CMessageBox::mbBack, NEUTRINO_ICON_INFO);
}

// MessageBox
void CTestMenu::testCMessageBoxErrorMsg()
{
	dprintf(DEBUG_NORMAL, "\ntesting MessageBox\n");

	MessageBox(LOCALE_MESSAGEBOX_ERROR, "testing CMessageBoxErrorMsg", CMessageBox::mbrCancel, CMessageBox::mbCancel, NEUTRINO_ICON_ERROR);
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

	helpBox->show(LOCALE_MESSAGEBOX_INFO, HELPBOX_WIDTH, -1, CMessageBox::mbrBack, CMessageBox::mbNone);
	
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
	
	CTextBox * textBox = new CTextBox("CTextBox", g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1], CTextBox::SCROLL, &Box, COL_MENUCONTENT_PLUS_0);
	
	std::string text = "testing CTextBox: blah blah blah boaah";
		
	int pich = 246;	//FIXME
	int picw = 162; 	//FIXME
		
	std::string fname = PLUGINDIR "/netzkino/netzkino.png";
		
	if(access(fname.c_str(), F_OK))
		fname = "";
	
	textBox->setText(&text, fname, picw, pich);
	
	textBox->paint();
	CFrameBuffer::getInstance()->blit();
	
	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
	bool bigFonts = false;
	bool loop = true;

	while(loop)
	{
		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg == CRCInput::RC_home) 
		{
			textBox->hide();
			CFrameBuffer::getInstance()->blit();

			loop = false;
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
	
	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;

	bool loop = true;

REPEAT:
	listFrame->refresh();
	while(loop)
	{
		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
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

			loop = false;
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

			for (unsigned int i = 0; i < audioFileList.size(); i++)
			{
				tmpAudioPlayerGui.addToPlaylist(audioFileList[i]);
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
	dprintf(DEBUG_NORMAL, "\ntesting CButtons\n");

	int icon_w, icon_h;
	CFrameBuffer::getInstance()->getIconSize(NEUTRINO_ICON_BUTTON_RED, &icon_w, &icon_h);
	buttons.paintButtons(CFrameBuffer::getInstance(), g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, g_settings.screen_StartX + 50 + BORDER_LEFT, g_settings.screen_StartY + 50, (g_settings.screen_EndX - g_settings.screen_StartX - 100)/BUTTONS_COUNT, BUTTONS_COUNT, Buttons, icon_h);

	CFrameBuffer::getInstance()->blit();

	// loop
	g_RCInput->messageLoop();
	hide();
}

// ClistBoxEntry
void CTestMenu::testClistBoxEntry()
{
	dprintf(DEBUG_NORMAL, "\ntesting ClistBoxEntry\n");

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
		item = new ClistBoxEntryItem(m_vMovieInfo[i].epgTitle.c_str());

		item->setOption(m_vMovieInfo[i].epgChannel.c_str());
		//item->setOptionInfo("OptionInfo");

		item->setInfo1(m_vMovieInfo[i].epgInfo1.c_str());
		//item->setOptionInfo1("OptionInfo1");

		item->setInfo2(m_vMovieInfo[i].epgInfo2.c_str());
		//item->setOptionInfo2("OptionInfo2");

		item->setnLinesItem();

		listBox->addItem(item);
	}

	// head
	listBox->setTitle("ClistBoxEntry", NEUTRINO_ICON_MOVIE);
	listBox->enablePaintHead();
	listBox->setHeaderButtons(Buttons, BUTTONS_COUNT);
	listBox->enablePaintDate();

	// footer
	listBox->enablePaintFoot();
	listBox->setFooterButtons(Buttons, BUTTONS_COUNT);

	// footinfo
	listBox->enablePaintFootInfo(80);

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

// play Movie Url
void CTestMenu::testPlayMovieURL()
{
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
		
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
			tmpMoviePlayerGui.addToPlaylist(*file);
			tmpMoviePlayerGui.exec(NULL, "");
		}

		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg != CRCInput::RC_home) 
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
			if (file->getType() == CFile::FILE_AUDIO)
			{
				tmpAudioPlayerGui.addToPlaylist(*file);
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

// show pic url
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
			tmpPictureViewerGui.addToPlaylist(*file);
			tmpPictureViewerGui.exec(NULL, "");
		}

		g_RCInput->getMsg_ms(&msg, &data, 10); // 1 sec
		
		if (msg != CRCInput::RC_home) 
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
		
		CFileList::const_iterator files = fileBrowser->getSelectedFiles().begin();
		for(; files != fileBrowser->getSelectedFiles().end(); files++)
		{		
			tmpMoviePlayerGui.addToPlaylist(*files);
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

// play audio folder
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
		
		if (msg != CRCInput::RC_home) 
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
	
	CFileFilter fileFilter;
	
	CFileList filelist;
	
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
		
		if (msg != CRCInput::RC_home) 
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

// CChannelSelect
void CTestMenu::testChannelSelectWidget()
{
	CSelectChannelWidget * CSelectChannelWidgetHandler = new CSelectChannelWidget();
	CSelectChannelWidgetHandler->exec(NULL, "tv");
		
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
	CPluginList * pluginList = new CPluginList();
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
	
	std::string Path_local = g_settings.network_nfs_moviedir;

	CFileList filelist;

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
				tmpAudioPlayerGui.addToPlaylist(*files);
			}
		}
		
		tmpAudioPlayerGui.exec(NULL, "");
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
	
	std::string Path_local = g_settings.network_nfs_picturedir;

	CFileList filelist;

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

// ClistBox
#define mHEAD_BUTTONS_COUNT	2
const struct button_label mHeadButtons[mHEAD_BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_HELP, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_SETUP, NONEXISTANT_LOCALE, NULL },
};

void CTestMenu::testClistBox()
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

// CChannellist
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


// CBouquetlist
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

int CTestMenu::exec(CMenuTarget *parent, const std::string &actionKey)
{
	dprintf(DEBUG_NORMAL, "\nCTestMenu::exec: actionKey:%s\n", actionKey.c_str());
	
	if(parent)
		hide();
	
	if(actionKey == "framebox")
	{
		testCFrameBox();

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
	else if(actionKey == "listboxentry")
	{
		testClistBoxEntry();

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
/*
	else if(actionKey == "alphasetup")
	{
		testAlphaSetupWidget();

		return menu_return::RETURN_REPAINT;
	}
*/
/*
	else if(actionKey == "psisetup")
	{
		testPSISetup();

		return menu_return::RETURN_REPAINT;
	}
*/
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
	else if(actionKey == "listbox")
	{
		testClistBox();

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
/*
	else if(actionKey == "menuwidgetlistbox")
	{
		testCMenuWidgetListBox();

		return menu_return::RETURN_REPAINT;
	}
*/
/*
	else if(actionKey == "RC_setup")
	{
		CChannelListSettings * testChannelMenu = new CChannelListSettings();
		testChannelMenu->exec(NULL, "");
		delete testChannelMenu;
		testChannelMenu = NULL;	

		return menu_return::RETURN_REPAINT;	
	}
	else if(actionKey == "RC_info")
	{
		g_EpgData->show(Channels[listMenu->getSelected()]->channel_id);

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "RC_red")
	{
		g_EventList->exec(Channels[listMenu->getSelected()]->channel_id, Channels[listMenu->getSelected()]->getName());

		return menu_return::RETURN_REPAINT;
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

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "RC_blue")
	{
		CEPGplusHandler eplus;
		eplus.exec(NULL, "");

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "zapto")
	{
		selected = listMenu->getSelected();
		g_Zapit->zapTo_serviceID(Channels[listMenu->getSelected()]->channel_id);

		return menu_return::RETURN_REPAINT;
	}
*/

/*
	else if(actionKey == "menuwidgetlistbox1")
	{
		testCMenuWidgetListBox1();

		return menu_return::RETURN_REPAINT;
	}
*/

/*
	else if(actionKey == "aplay")
	{
		selected = listMenu->getSelected();

		tmpAudioPlayerGui.addToPlaylist(audioFileList[listMenu->getSelected()].Name.c_str());
		tmpAudioPlayerGui.exec(NULL, "");

		return menu_return::RETURN_REPAINT;		
	}
	else if(actionKey == "asetup")
	{
		CAudioPlayerSettings * audioPlayerSettingsMenu = new CAudioPlayerSettings();
		audioPlayerSettingsMenu->exec(this, "");
		delete audioPlayerSettingsMenu;
		audioPlayerSettingsMenu = NULL;

		return menu_return::RETURN_REPAINT;	
	}
*/

	else if(actionKey == "testmenuwidget")
	{
		testCMenuWidget();

		return menu_return::RETURN_REPAINT;
	}

/*
	else if(actionKey == "listboxnlines")
	{
		testClistBoxnLines();

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "pred")
	{
		g_PluginList->removePlugin(listMenu->getSelected());
		//return menu_return::RETURN_EXIT_ALL;
		listMenu->hide();
		listMenu->exec(NULL, "");

		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "pgreen")
	{
		g_PluginList->startPlugin(listMenu->getSelected());

		return menu_return::RETURN_REPAINT;
	}
*/

	else if(actionKey == "mplay")
	{
		selected = listMenu->getSelected();

		if (&m_vMovieInfo[listMenu->getSelected()].file != NULL) 
		{
			tmpMoviePlayerGui.addToPlaylist(m_vMovieInfo[listMenu->getSelected()]);
			tmpMoviePlayerGui.exec(NULL, "urlplayback");
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

	showMenu();
	
	return menu_return::RETURN_REPAINT;
}

void CTestMenu::showMenu()
{
	CMenuWidget * mainMenu = new CMenuWidget("testMenu", NEUTRINO_ICON_BUTTON_SETUP);

	mainMenu->enableMenuPosition();
	
	mainMenu->addItem(new CMenuForwarder("CFrameBox", true, NULL, this, "framebox"));
	mainMenu->addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	mainMenu->addItem(new CMenuForwarder("CBox", true, NULL, this, "box"));
	mainMenu->addItem(new CMenuForwarder("CIcon", true, NULL, this, "icon"));
	mainMenu->addItem(new CMenuForwarder("CImage", true, NULL, this, "image"));
	mainMenu->addItem(new CMenuForwarder("CWindow", true, NULL, this, "window"));
	mainMenu->addItem(new CMenuForwarder("CWindow(with shadow)", true, NULL, this, "windowshadow"));
	mainMenu->addItem(new CMenuForwarder("customColor", true, NULL, this, "windowcustomcolor"));
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
	mainMenu->addItem(new CMenuForwarder("ClistBox(movie browser)", true, NULL, this, "listbox"));
	mainMenu->addItem(new CMenuForwarder("CMenuWidget", true, NULL, this, "testmenuwidget"));
	
	mainMenu->addItem( new CMenuSeparator(CMenuSeparator::LINE) );
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
	mainMenu->addItem(new CMenuForwarder("PluginsList", true, NULL, this, "pluginslist"));

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
	mainMenu->addItem(new CMenuForwarder("ShowActuellEPG", true, NULL, this, "showepg"));

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
	CTestMenu *testMenu = new CTestMenu();
	
	testMenu->exec(NULL, "");
	
	delete testMenu;
	testMenu = NULL;
}


