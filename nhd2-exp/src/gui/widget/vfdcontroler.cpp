/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: vfdcontroller.cpp 2013/10/12 mohousch Exp $

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <vfdcontroler.h>

#include <driver/fontrenderer.h>
#include <driver/rcinput.h>
#include <driver/screen_max.h>

#include <gui/color.h>
#include <gui/widget/messagebox.h>

#include <global.h>
#include <neutrino.h>

#include <math.h>

#include <system/debug.h>

#if defined(ENABLE_LCD) && (defined(PLATFORM_DREAMBOX) || defined(PLATFORM_GIGABLUE) || defined(PLATFORM_VUPLUS))
#define BRIGHTNESSFACTOR 2.55 // 0 - 255
#else
#define BRIGHTNESSFACTOR 0.07 // 0 - 7
#endif


CVfdControler::CVfdControler(const neutrino_locale_t Name, CChangeObserver* Observer)
{
	frameBuffer = CFrameBuffer::getInstance();
	hheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	mheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	observer = Observer;
	
	nameStringOption = Name;
	name = g_Locale->getText(Name);

	width = w_max(MENU_WIDTH, 0);
	height = h_max(hheight+ mheight*4 + mheight/2, 0);
	x = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth()-width) >> 1);
	y = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight()-height)>> 1);

	brightness = CVFD::getInstance()->getBrightness();
	brightnessstandby = CVFD::getInstance()->getBrightnessStandby();
}

CVfdControler::CVfdControler(const char* const Name, CChangeObserver* Observer)
{
	frameBuffer = CFrameBuffer::getInstance();
	hheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	mheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	observer = Observer;

	name = Name;
	nameStringOption = NONEXISTANT_LOCALE;

	width = w_max(MENU_WIDTH, 0);
	height = h_max(hheight+ mheight*3 + mheight/2, 0);
	x = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth()-width) >> 1);
	y = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight()-height)>> 1);

	brightness = CVFD::getInstance()->getBrightness();
	brightnessstandby = CVFD::getInstance()->getBrightnessStandby();
}

void CVfdControler::setVfd()
{
	CVFD::getInstance()->setBrightness(brightness);
	CVFD::getInstance()->setBrightnessStandby(brightnessstandby);
}

int CVfdControler::exec(CMenuTarget* parent, const std::string &)
{
	dprintf(DEBUG_NORMAL, "CVfdControler::exec\n");

	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	int selected, res = menu_return::RETURN_REPAINT;
	unsigned int brightness_alt, brightnessstandby_alt;

	if (parent)
		parent->hide();
	
	brightness_alt = CVFD::getInstance()->getBrightness();
	brightnessstandby_alt = CVFD::getInstance()->getBrightnessStandby();
	selected = 0;

	setVfd();
	paint();
	
	frameBuffer->blit();

	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);

	bool loop=true;
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd, true );

		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);

		switch ( msg )
		{
			case CRCInput::RC_down:
			if(selected < 2) // max entries
			{
				paintSlider(x + BORDER_LEFT, y + hheight, brightness, BRIGHTNESSFACTOR, LOCALE_LCDCONTROLER_BRIGHTNESS, false);
				paintSlider(x + BORDER_LEFT, y + hheight + mheight, brightnessstandby, BRIGHTNESSFACTOR, LOCALE_LCDCONTROLER_BRIGHTNESSSTANDBY, false);
				selected++;
				
				switch (selected) 
				{
					case 0:
						paintSlider(x + BORDER_LEFT, y+ hheight, brightness, BRIGHTNESSFACTOR, LOCALE_LCDCONTROLER_BRIGHTNESS, true);
						break;
						
					case 1:
						paintSlider(x + BORDER_LEFT, y+ hheight+ mheight, brightnessstandby, BRIGHTNESSFACTOR, LOCALE_LCDCONTROLER_BRIGHTNESSSTANDBY, true);

						CVFD::getInstance()->setMode(CVFD::MODE_STANDBY);
						break;
						
					case 2:
						frameBuffer->paintBoxRel(x, y + hheight + mheight*2 + mheight/2, width, mheight, COL_MENUCONTENTSELECTED_PLUS_0);

						g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x + BORDER_LEFT, y + hheight + mheight*3 + mheight/2, width, g_Locale->getText(LOCALE_OPTIONS_DEFAULT), COL_MENUCONTENTSELECTED, 0, true); // UTF-8
						break;
				}
			}
			break;

			case CRCInput::RC_up:
			if (selected > 0) 
			{
				paintSlider(x + BORDER_LEFT, y + hheight, brightness, BRIGHTNESSFACTOR, LOCALE_LCDCONTROLER_BRIGHTNESS, false);

				paintSlider(x + BORDER_LEFT, y + hheight + mheight, brightnessstandby, BRIGHTNESSFACTOR, LOCALE_LCDCONTROLER_BRIGHTNESSSTANDBY, false);
				selected--;
				switch (selected) 
				{
					case 0:
						paintSlider(x + BORDER_LEFT, y+ hheight, brightness, BRIGHTNESSFACTOR, LOCALE_LCDCONTROLER_BRIGHTNESS, true);

						CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
						break;
						
					case 1:
						paintSlider(x + BORDER_LEFT, y + hheight + mheight, brightnessstandby, BRIGHTNESSFACTOR, LOCALE_LCDCONTROLER_BRIGHTNESSSTANDBY, true);

						CVFD::getInstance()->setMode(CVFD::MODE_STANDBY);

						frameBuffer->paintBoxRel(x, y + hheight + mheight*2 + mheight/2, width, mheight, COL_MENUCONTENT_PLUS_0);

						g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x + BORDER_LEFT, y + hheight + mheight*3 + mheight/2, width, g_Locale->getText(LOCALE_OPTIONS_DEFAULT), COL_MENUCONTENT, 0, true); // UTF-8
						break;
						
					case 2:
						break;
				}
			}
			break;

			case CRCInput::RC_right:
				switch (selected) 
				{
					case 0:
						if (brightness < DEFAULT_LCD_BRIGHTNESS) 
						{
							brightness ++;
							
							paintSlider(x + BORDER_LEFT, y + hheight, brightness, BRIGHTNESSFACTOR, LOCALE_LCDCONTROLER_BRIGHTNESS, true);
							setVfd();
						}
						break;
						
					case 1:
						if (brightnessstandby < DEFAULT_LCD_STANDBYBRIGHTNESS) 
						{
							brightnessstandby ++;
							
							paintSlider(x + BORDER_LEFT, y + hheight + mheight, brightnessstandby, BRIGHTNESSFACTOR, LOCALE_LCDCONTROLER_BRIGHTNESSSTANDBY, true);
							setVfd();
						}
						break;
				}
				break;

			case CRCInput::RC_left:
				switch (selected) 
				{
					case 0:
						if (brightness > 0) 
						{
							brightness--;
							
							paintSlider(x + BORDER_LEFT, y + hheight, brightness, BRIGHTNESSFACTOR, LOCALE_LCDCONTROLER_BRIGHTNESS, true);
							setVfd();
						}
						break;
						
					case 1:
						if (brightnessstandby > 0) 
						{
							brightnessstandby--;
							
							paintSlider(x + BORDER_LEFT, y + hheight + mheight, brightnessstandby, BRIGHTNESSFACTOR, LOCALE_LCDCONTROLER_BRIGHTNESSSTANDBY, true);
							setVfd();
						}
						break;
				}
				break;

			case CRCInput::RC_home:
				if ( ((brightness != brightness_alt) || (brightnessstandby != brightnessstandby_alt) ) && (MessageBox(name.c_str(), LOCALE_MESSAGEBOX_DISCARD, CMessageBox::mbrYes, CMessageBox::mbYes | CMessageBox::mbCancel) == CMessageBox::mbrCancel))
					break;

				brightness = brightness_alt;
				brightnessstandby = brightnessstandby_alt;
				setVfd();
				loop = false;
				break;

			case CRCInput::RC_ok:
				if (selected == 2) 
				{
					brightness = DEFAULT_LCD_BRIGHTNESS;
					brightnessstandby = DEFAULT_LCD_STANDBYBRIGHTNESS;
					selected = 0;
					setVfd();
					paint();
					break;
				}

			case CRCInput::RC_timeout:
				loop = false;
				break;
	
			default:
				if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
				{
					loop = false;
					res = menu_return::RETURN_EXIT_ALL;
				}
		}

		frameBuffer->blit();	
	}

	hide();

	if(observer)
		observer->changeNotify(nameStringOption, NULL);

	return res;
}

void CVfdControler::hide()
{
	frameBuffer->paintBackgroundBoxRel(x, y, width + SHADOW_OFFSET, height + SHADOW_OFFSET);

	frameBuffer->blit();
}

void CVfdControler::paint()
{
	dprintf(DEBUG_NORMAL, "CVfdControler::paint\n");

	CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);

	// top
	//shadow
	frameBuffer->paintBoxRel(x + SHADOW_OFFSET, y + SHADOW_OFFSET, width, hheight, COL_INFOBAR_SHADOW_PLUS_0, RADIUS_MID, CORNER_TOP);//round
	
	// title
	frameBuffer->paintBoxRel(x, y, width, hheight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, g_settings.menu_Head_gradient);//round
	
	// body
	//shadow
	frameBuffer->paintBoxRel(x + SHADOW_OFFSET, y + hheight + SHADOW_OFFSET, width, height - hheight, COL_INFOBAR_SHADOW_PLUS_0, RADIUS_MID, CORNER_BOTTOM);//round

	// body
	frameBuffer->paintBoxRel(x, y + hheight, width, height - hheight, COL_MENUCONTENT_PLUS_0, RADIUS_MID, CORNER_BOTTOM);//round

	// head title
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(x + BORDER_LEFT, y + hheight, width, name.c_str(), COL_MENUHEAD, 0, true); // UTF-8

	paintSlider(x + BORDER_LEFT, y + hheight, brightness, BRIGHTNESSFACTOR, LOCALE_LCDCONTROLER_BRIGHTNESS, true);
	paintSlider(x + BORDER_LEFT, y + hheight + mheight, brightnessstandby, BRIGHTNESSFACTOR, LOCALE_LCDCONTROLER_BRIGHTNESSSTANDBY, false);

	frameBuffer->paintHLineRel(x + BORDER_LEFT, width - 20, y + hheight + mheight*2 + mheight/4, COL_MENUCONTENT_PLUS_3);
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(x + BORDER_LEFT, y + hheight + mheight*3 + mheight/2, width, g_Locale->getText(LOCALE_OPTIONS_DEFAULT), COL_MENUCONTENT, 0, true); // UTF-8
}

void CVfdControler::paintSlider(int _x, int _y, unsigned int spos, float factor, const neutrino_locale_t text, bool selected)
{
	char wert[5];
	
	int icon_w = 120;
	int icon_h = 11;
	
	frameBuffer->getIconSize(NEUTRINO_ICON_VOLUMEBODY, &icon_w, &icon_h);
	
	int slider_w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth("100", true); //UTF-8
	
	int startx = width - icon_w - slider_w - 50;

	frameBuffer->paintBoxRel(_x + startx, _y, 120, mheight, COL_MENUCONTENT_PLUS_0);
	frameBuffer->paintIcon(NEUTRINO_ICON_VOLUMEBODY, _x + startx, _y + 2 + mheight/4);
	frameBuffer->paintIcon(selected ? NEUTRINO_ICON_VOLUMESLIDER2BLUE : NEUTRINO_ICON_VOLUMESLIDER2, (int)(_x + (startx + 3) + (spos / factor)), _y + mheight/4);

	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(_x, _y + mheight, width, g_Locale->getText(text), COL_MENUCONTENT, 0, true); // UTF-8
	
	// wert //FIXME
	sprintf(wert, "%3d", spos ); // UTF-8 encoded

	frameBuffer->paintBoxRel(_x + startx + 120 + 10, _y, 50, mheight, COL_MENUCONTENT_PLUS_0);
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(_x + startx + 120 + 10, _y + mheight, width, wert, COL_MENUCONTENT, 0, true); // UTF-8
}
