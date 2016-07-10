/*
	Neutrino graphlcd menue

	(c) 2012 by martii

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

#include <glcdsetup.h>

extern "C" void plugin_exec(void);
extern "C" void plugin_init(void);
extern "C" void plugin_del(void);

#define ONOFF_OPTION_COUNT 2
static const CMenuOptionChooser::keyval ONOFF_OPTIONS[ONOFF_OPTION_COUNT] = {
	{ 0, LOCALE_OPTIONS_OFF, NULL},
	{ 1, LOCALE_OPTIONS_ON, NULL}
};

// config
int		glcd_enable;
uint32_t	glcd_color_fg;
uint32_t	glcd_color_bg;
uint32_t	glcd_color_bar;
std::string	glcd_font;
int		glcd_percent_channel;
int		glcd_percent_epg;
int		glcd_percent_bar;
int		glcd_percent_time;
int		glcd_mirror_osd;
int		glcd_time_in_standby;
//

#define KEY_GLCD_BLACK			0
#define KEY_GLCD_WHITE			1
#define KEY_GLCD_RED			2
#define KEY_GLCD_GREEN			3
#define KEY_GLCD_BLUE			4
#define KEY_GLCD_MAGENTA		5
#define KEY_GLCD_CYAN			6
#define KEY_GLCD_YELLOW			7
#define KEY_GLCD_ORANGE		 	8
#define KEY_GLCD_LIGHT_GRAY		9
#define KEY_GLCD_GRAY			10
#define KEY_GLCD_DARK_GRAY		11
#define KEY_GLCD_DARK_RED		12
#define KEY_GLCD_DARK_GREEN		13
#define KEY_GLCD_DARK_BLUE		14
#define KEY_GLCD_PURPLE			15
#define KEY_GLCD_MINT			16
#define KEY_GLCD_GOLDEN			17

#define GLCD_COLOR_OPTION_COUNT 	18

const uint32_t GLCD_EXTRA_COLOR_Orange           = 0xFFFF4000;
const uint32_t GLCD_EXTRA_COLOR_LIGHT_Gray       = 0xFFBFBFBF;
const uint32_t GLCD_EXTRA_COLOR_Gray             = 0xFF7F7F7F;
const uint32_t GLCD_EXTRA_COLOR_DARK_Gray        = 0xFF3F3F3F;
const uint32_t GLCD_EXTRA_COLOR_DARK_Red         = 0xFF7F0000;
const uint32_t GLCD_EXTRA_COLOR_DARK_Green       = 0xFF00FF00;
const uint32_t GLCD_EXTRA_COLOR_DARK_Blue        = 0xFF00007F;
const uint32_t GLCD_EXTRA_COLOR_Purple           = 0xFF7F007F;
const uint32_t GLCD_EXTRA_COLOR_Mint             = 0xFF007F7F;
const uint32_t GLCD_EXTRA_COLOR_Golden           = 0xFF7F7F00;

static const CMenuOptionChooser::keyval GLCD_COLOR_OPTIONS[GLCD_COLOR_OPTION_COUNT] =
{
	{ KEY_GLCD_BLACK,	LOCALE_GLCD_COLOR_BLACK },
	{ KEY_GLCD_WHITE,	LOCALE_GLCD_COLOR_WHITE },
	{ KEY_GLCD_RED,		LOCALE_GLCD_COLOR_RED },
	{ KEY_GLCD_GREEN,	LOCALE_GLCD_COLOR_GREEN },
	{ KEY_GLCD_BLUE,	LOCALE_GLCD_COLOR_BLUE },
	{ KEY_GLCD_MAGENTA,	LOCALE_GLCD_COLOR_MAGENTA },
	{ KEY_GLCD_CYAN,	LOCALE_GLCD_COLOR_CYAN },
	{ KEY_GLCD_YELLOW,	LOCALE_GLCD_COLOR_YELLOW },
	{ KEY_GLCD_ORANGE,	LOCALE_GLCD_COLOR_ORANGE },
	{ KEY_GLCD_LIGHT_GRAY,	LOCALE_GLCD_COLOR_LIGHT_GRAY },
	{ KEY_GLCD_GRAY,	LOCALE_GLCD_COLOR_GRAY },
	{ KEY_GLCD_DARK_GRAY,	LOCALE_GLCD_COLOR_DARK_GRAY },
	{ KEY_GLCD_DARK_RED,	LOCALE_GLCD_COLOR_DARK_RED },
	{ KEY_GLCD_DARK_GREEN,	LOCALE_GLCD_COLOR_DARK_GREEN },
	{ KEY_GLCD_DARK_BLUE,	LOCALE_GLCD_COLOR_DARK_BLUE },
	{ KEY_GLCD_PURPLE,	LOCALE_GLCD_COLOR_PURPLE },
	{ KEY_GLCD_MINT,	LOCALE_GLCD_COLOR_MINT },
	{ KEY_GLCD_GOLDEN,	LOCALE_GLCD_COLOR_GOLDEN },
};

int GLCD_Menu::color2index(uint32_t color) 
{
	if (color == GLCD::cColor::Black)
		return KEY_GLCD_BLACK;
	if (color == GLCD::cColor::White)
		return KEY_GLCD_WHITE;
	if (color == GLCD::cColor::Red)
		return KEY_GLCD_RED;
	if (color == GLCD::cColor::Green)
		return KEY_GLCD_GREEN;
	if (color == GLCD::cColor::Blue)
		return KEY_GLCD_BLUE;
	if (color == GLCD::cColor::Magenta)
		return KEY_GLCD_MAGENTA;
	if (color == GLCD::cColor::Cyan)
		return KEY_GLCD_CYAN;
	if (color == GLCD::cColor::Yellow)
		return KEY_GLCD_YELLOW;
	if (color == GLCD_EXTRA_COLOR_Orange)
		return KEY_GLCD_ORANGE;
	if (color == GLCD_EXTRA_COLOR_LIGHT_Gray)
		return KEY_GLCD_LIGHT_GRAY;
	if (color == GLCD_EXTRA_COLOR_Gray)
		return KEY_GLCD_GRAY;
	if (color == GLCD_EXTRA_COLOR_DARK_Gray)
		return KEY_GLCD_DARK_GRAY;
	if (color == GLCD_EXTRA_COLOR_DARK_Red)
		return KEY_GLCD_DARK_RED;
	if (color == GLCD_EXTRA_COLOR_DARK_Green)
		return KEY_GLCD_DARK_GREEN;
	if (color == GLCD_EXTRA_COLOR_DARK_Blue)
		return KEY_GLCD_DARK_BLUE;
	if (color == GLCD_EXTRA_COLOR_Purple)
		return KEY_GLCD_PURPLE;
	if (color == GLCD_EXTRA_COLOR_Mint)
		return KEY_GLCD_MINT;
	if (color == GLCD_EXTRA_COLOR_Golden)
		return KEY_GLCD_GOLDEN;
	
	return KEY_GLCD_BLACK;
}

uint32_t GLCD_Menu::index2color(int i) 
{
	switch(i) 
	{
		case KEY_GLCD_BLACK:
			return GLCD::cColor::Black;
		case KEY_GLCD_WHITE:
			return GLCD::cColor::White;
		case KEY_GLCD_RED:
			return GLCD::cColor::Red;
		case KEY_GLCD_GREEN:
			return GLCD::cColor::Green;
		case KEY_GLCD_BLUE:
			return GLCD::cColor::Blue;
		case KEY_GLCD_MAGENTA:
			return GLCD::cColor::Magenta;
		case KEY_GLCD_CYAN:
			return GLCD::cColor::Cyan;
		case KEY_GLCD_YELLOW:
			return GLCD::cColor::Yellow;
		case KEY_GLCD_ORANGE:
			return GLCD_EXTRA_COLOR_Orange;
		case KEY_GLCD_LIGHT_GRAY:
			return GLCD_EXTRA_COLOR_LIGHT_Gray;
		case KEY_GLCD_GRAY:
			return GLCD_EXTRA_COLOR_Gray;
		case KEY_GLCD_DARK_GRAY:
			return GLCD_EXTRA_COLOR_DARK_Gray;
		case KEY_GLCD_DARK_RED:
			return GLCD_EXTRA_COLOR_DARK_Red;
		case KEY_GLCD_DARK_GREEN:
			return GLCD_EXTRA_COLOR_DARK_Green;
		case KEY_GLCD_DARK_BLUE:
			return GLCD_EXTRA_COLOR_DARK_Blue;
		case KEY_GLCD_PURPLE:
			return GLCD_EXTRA_COLOR_Purple;
		case KEY_GLCD_MINT:
			return GLCD_EXTRA_COLOR_Mint;
		case KEY_GLCD_GOLDEN:
			return GLCD_EXTRA_COLOR_Golden;
	}
	
	return GLCD::cColor::ERRCOL;
}

GLCD_Menu::GLCD_Menu()
{
	notifier = new GLCD_Menu_Notifier();
}

void GLCD_Menu::hide()
{
}

int GLCD_Menu::exec(CMenuTarget* parent, const std::string & actionKey)
{
	int res = menu_return::RETURN_REPAINT;
	nGLCD *nglcd = nGLCD::getInstance();
	
	if(actionKey == "rescan") 
	{
		nglcd->Rescan();
		return res;
	}
	
	if(actionKey == "select_font") 
	{
		if(parent)
			parent->hide();
		
		CFileBrowser fileBrowser;
		CFileFilter fileFilter;
		fileFilter.addFilter("ttf");
		fileBrowser.Filter = &fileFilter;
		
		if (fileBrowser.exec(FONTDIR) == true) 
		{
			glcd_font = fileBrowser.getSelectedFile()->Name;
			nglcd->fonts_initialized = false;
			nglcd->Rescan();
		}
		
		return res;
	}
	else if(actionKey == "save")
	{
		//SaveSettings();
		if(this->SaveSettings())
		 	HintBox(LOCALE_MESSAGEBOX_INFO, "Einstellungen werden gespeichert !");
		else
		 	HintBox(LOCALE_MESSAGEBOX_INFO, "Einstellungen NICHT gespeichert !");
	}

	if (parent)
		parent->hide();

	GLCD_Menu_Settings();

	return res;
}

//
void GLCD_Menu::ReadSettings() 
{
	CConfigFile *configfile = new CConfigFile(',');
	configfile->clear();
	configfile->loadConfig(CONFIG_FILE);
	
	glcd_enable = configfile->getInt32("glcd_enable", 0);
	glcd_color_fg = configfile->getInt32("glcd_color_fg", GLCD::cColor::White);
	glcd_color_bg = configfile->getInt32("glcd_color_bg", GLCD::cColor::Blue);
	glcd_color_bar = configfile->getInt32("glcd_color_bar", GLCD::cColor::Red);
	glcd_percent_channel = configfile->getInt32("glcd_percent_channel", 18);
	glcd_percent_epg = configfile->getInt32("glcd_percent_epg", 8);
	glcd_percent_bar = configfile->getInt32("glcd_percent_bar", 6);
	glcd_percent_time = configfile->getInt32("glcd_percent_time", 22);
	glcd_mirror_osd = configfile->getInt32("glcd_mirror_osd", 0);
	glcd_time_in_standby = configfile->getInt32("glcd_time_in_standby", 0);
	glcd_font = configfile->getString("glcd_font", FONTDIR "/micron.ttf");
}

bool GLCD_Menu::SaveSettings() 
{
	CConfigFile *configfile = new CConfigFile(',');
	
	configfile->setInt32("glcd_enable", glcd_enable);
	configfile->setInt32("glcd_color_fg", glcd_color_fg);
	configfile->setInt32("glcd_color_bg", glcd_color_bg);
	configfile->setInt32("glcd_color_bar", glcd_color_bar);
	configfile->setInt32("glcd_percent_channel", glcd_percent_channel);
	configfile->setInt32("glcd_percent_epg", glcd_percent_epg);
	configfile->setInt32("glcd_percent_bar", glcd_percent_bar);
	configfile->setInt32("glcd_percent_time", glcd_percent_time);
	configfile->setInt32("glcd_mirror_osd", glcd_mirror_osd);
	configfile->setInt32("glcd_time_in_standby", glcd_time_in_standby);
	configfile->setString("glcd_font", glcd_font);
	
	configfile->saveConfig(CONFIG_FILE);
	
	return true;
}

void GLCD_Menu::GLCD_Menu_Settings()
{
	// read settings
	ReadSettings();
	
	//
	int color_bg = color2index(glcd_color_bg);
	int color_fg = color2index(glcd_color_fg);
	int color_bar = color2index(glcd_color_bar);

	CMenuWidget * m = new CMenuWidget(LOCALE_GLCD_HEAD, NEUTRINO_ICON_SETTINGS);
	
	m->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	m->addItem(new CMenuSeparator(CMenuSeparator::LINE));
	m->addItem(new CMenuForwarder("Einstellungen speichern", true, NULL, this, "save", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	m->addItem(new CMenuSeparator(CMenuSeparator::LINE));

	m->addItem(new CMenuOptionChooser(LOCALE_GLCD_ENABLE, &glcd_enable, ONOFF_OPTIONS, ONOFF_OPTION_COUNT, true, notifier));
	int shortcut = 1;
	m->addItem(new CMenuSeparator(CMenuSeparator::LINE));
	m->addItem(new CMenuOptionChooser(LOCALE_GLCD_SELECT_FG, &color_fg, GLCD_COLOR_OPTIONS, GLCD_COLOR_OPTION_COUNT, true, notifier, CRCInput::convertDigitToKey(shortcut++)));
	m->addItem(new CMenuOptionChooser(LOCALE_GLCD_SELECT_BG, &color_bg, GLCD_COLOR_OPTIONS, GLCD_COLOR_OPTION_COUNT, true, notifier, CRCInput::convertDigitToKey(shortcut++)));
	m->addItem(new CMenuOptionChooser(LOCALE_GLCD_SELECT_BAR, &color_bar, GLCD_COLOR_OPTIONS, GLCD_COLOR_OPTION_COUNT, true, notifier, CRCInput::convertDigitToKey(shortcut++)));
	m->addItem(new CMenuForwarder(LOCALE_GLCD_FONT, true, glcd_font, this, "select_font", CRCInput::convertDigitToKey(shortcut++)));
	m->addItem(new CMenuOptionNumberChooser(LOCALE_GLCD_SIZE_CHANNEL, &glcd_percent_channel, true, 0, 100, notifier));
	m->addItem(new CMenuOptionNumberChooser(LOCALE_GLCD_SIZE_EPG, &glcd_percent_epg, true, 0, 100, notifier));
	m->addItem(new CMenuOptionNumberChooser(LOCALE_GLCD_SIZE_BAR, &glcd_percent_bar, true, 0, 100, notifier));
	m->addItem(new CMenuOptionNumberChooser(LOCALE_GLCD_SIZE_TIME, &glcd_percent_time, true, 0, 100, notifier));
	m->addItem(new CMenuSeparator(CMenuSeparator::LINE));
	m->addItem(new CMenuOptionChooser(LOCALE_GLCD_TIME_IN_STANDBY, &glcd_time_in_standby, ONOFF_OPTIONS, ONOFF_OPTION_COUNT, true, notifier, CRCInput::convertDigitToKey(shortcut++)));
	m->addItem(new CMenuOptionChooser(LOCALE_GLCD_MIRROR_OSD, &glcd_mirror_osd, ONOFF_OPTIONS, ONOFF_OPTION_COUNT, true, notifier, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN));
	m->addItem(new CMenuSeparator(CMenuSeparator::LINE));
	m->addItem(new CMenuForwarder(LOCALE_GLCD_RESTART, true, "", this, "rescan", CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW));
	
	m->exec(NULL, "");
	m->hide();
	
	delete m;
}

GLCD_Menu_Notifier::GLCD_Menu_Notifier()
{
}

bool GLCD_Menu_Notifier::changeNotify(const neutrino_locale_t OptionName, void *Data)
{
	if (!Data)
		return false;
	
	nGLCD *nglcd = nGLCD::getInstance();
	
	switch(OptionName) 
	{
		case LOCALE_GLCD_SELECT_FG:
			glcd_color_fg = GLCD_Menu::index2color(*((int *) Data));
			break;
			
		case LOCALE_GLCD_SELECT_BG:
			glcd_color_bg = GLCD_Menu::index2color(*((int *) Data));
			break;
			
		case LOCALE_GLCD_SELECT_BAR:
			glcd_color_bar = GLCD_Menu::index2color(*((int *) Data));
			break;
			
		case LOCALE_GLCD_ENABLE:
			if (glcd_enable)
				nglcd->Resume();
			else
				nglcd->Suspend();
			return true;
			break;
			
		case LOCALE_GLCD_MIRROR_OSD:
			nglcd->doMirrorOSD = glcd_mirror_osd;
			break;
			
		case LOCALE_GLCD_TIME_IN_STANDBY:
		case LOCALE_GLCD_SIZE_CHANNEL:
		case LOCALE_GLCD_SIZE_EPG:
		case LOCALE_GLCD_SIZE_BAR:
		case LOCALE_GLCD_SIZE_TIME:
			break;
			
		default:
			return false;
	}

	nglcd->Update();
	return true;
}

// plugin API
void plugin_init(void)
{
	//FIXME
	nGLCD::getInstance();
}

void plugin_del(void)
{
	//FIXME
	nGLCD::Exit();
}

void plugin_exec(void)
{
	GLCD_Menu * glcdMenu = new GLCD_Menu();
	
	glcdMenu->exec(NULL, "");
	glcdMenu->hide();
	
	delete glcdMenu;
}

