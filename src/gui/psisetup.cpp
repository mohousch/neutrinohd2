/*
	Neutrino-GUI  -   DBoxII-Project

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

#include <gui/psisetup.h>

#include <driver/fontrenderer.h>
#include <driver/rcinput.h>

#include <gui/color.h>

#include <gui/widget/messagebox.h>
#include <driver/screen_max.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>

#include <global.h>
#include <neutrino.h>

#include <video_cs.h>


#define PSI_STEP	5

extern cVideo * videoDecoder;

CPSISetup::CPSISetup(const neutrino_locale_t Name, unsigned char *Contrast, unsigned char *Saturation, unsigned char *Brightness, unsigned char *Tint, CChangeObserver *Observer)
{
	frameBuffer = CFrameBuffer::getInstance();
	observer = Observer;
	name = Name;

	contrast = Contrast;
	saturation = Saturation;
	brightness = Brightness;
	tint = Tint;
	
	videoDecoder->setContrast(*contrast);
	videoDecoder->setSaturation(*saturation);
	videoDecoder->setBrightness(*brightness);
	videoDecoder->setTint(*tint);
}

int CPSISetup::exec(CMenuTarget * parent, const std::string &)
{
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	width = w_max(MENU_WIDTH, 0);

	hheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	mheight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();

	height = hheight + mheight*4;

	x = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - width) >> 1);
	y = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - height) >> 1);

	int res = menu_return::RETURN_REPAINT;
	
	if (parent)
		parent->hide();
	
	unsigned char contrast_old = *contrast;
	unsigned char saturation_old = *saturation;
	unsigned char brightness_old = *brightness;
	unsigned char tint_old = *tint;

	videoDecoder->setContrast(*contrast);
	videoDecoder->setSaturation(*saturation);
	videoDecoder->setBrightness(*brightness);
	videoDecoder->setTint(*tint);
	
	paint();
	frameBuffer->blit();

	int selected = 0;
	int max = 3;

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
			case CRCInput::RC_vfddown:
				{
					if(selected < max)
					{
						paintSlider(x + BORDER_LEFT, y + hheight, contrast, LOCALE_PSISETUP_CONTRAST, NEUTRINO_ICON_VOLUMESLIDER2, false);
						paintSlider(x + BORDER_LEFT, y + hheight + mheight, saturation, LOCALE_PSISETUP_SATURATION, NEUTRINO_ICON_VOLUMESLIDER2     , false);
						paintSlider(x + BORDER_LEFT, y + hheight + 2*mheight, brightness, LOCALE_PSISETUP_BRIGHTNESS, NEUTRINO_ICON_VOLUMESLIDER2     , false);
						paintSlider(x + BORDER_LEFT, y + hheight + 3*mheight, tint, LOCALE_PSISETUP_TINT, NEUTRINO_ICON_VOLUMESLIDER2     , false);

						selected++;

						switch(selected)
						{
							case 0:
								paintSlider(x + BORDER_LEFT, y + hheight, contrast, LOCALE_PSISETUP_CONTRAST, NEUTRINO_ICON_VOLUMESLIDER2RED, true );
								break;
								
							case 1:
								paintSlider(x + BORDER_LEFT, y + hheight + mheight, saturation, LOCALE_PSISETUP_SATURATION, NEUTRINO_ICON_VOLUMESLIDER2RED, true );
								break;
								
							case 2:
								paintSlider(x + BORDER_LEFT, y + hheight + 2*mheight, brightness, LOCALE_PSISETUP_BRIGHTNESS, NEUTRINO_ICON_VOLUMESLIDER2RED, true );
								break;
								
							case 3:
								paintSlider(x + BORDER_LEFT, y + hheight + 3*mheight, tint, LOCALE_PSISETUP_TINT, NEUTRINO_ICON_VOLUMESLIDER2RED, true );
								break;
						}
					}
					break;
				}
				
			case CRCInput::RC_up:
			case CRCInput::RC_vfdup:
				{
					if (selected > 0)
					{
						paintSlider(x + BORDER_LEFT, y + hheight, contrast, LOCALE_PSISETUP_CONTRAST, NEUTRINO_ICON_VOLUMESLIDER2, false);
						paintSlider(x + BORDER_LEFT, y + hheight + mheight, saturation, LOCALE_PSISETUP_SATURATION, NEUTRINO_ICON_VOLUMESLIDER2, false);
						paintSlider(x + BORDER_LEFT, y + hheight + 2*mheight, brightness, LOCALE_PSISETUP_BRIGHTNESS, NEUTRINO_ICON_VOLUMESLIDER2, false);
						paintSlider(x + BORDER_LEFT, y + hheight + 3*mheight, tint, LOCALE_PSISETUP_TINT, NEUTRINO_ICON_VOLUMESLIDER2, false);

						selected--;

						switch(selected)
						{
							case 0:
								paintSlider(x + BORDER_LEFT, y + hheight, contrast, LOCALE_PSISETUP_CONTRAST, NEUTRINO_ICON_VOLUMESLIDER2RED, true );
								break;
								
							case 1:
								paintSlider(x + BORDER_LEFT, y + hheight + mheight, saturation, LOCALE_PSISETUP_SATURATION, NEUTRINO_ICON_VOLUMESLIDER2RED, true );
								break;
								
							case 2:
								paintSlider(x + BORDER_LEFT, y + hheight + 2*mheight, brightness, LOCALE_PSISETUP_BRIGHTNESS, NEUTRINO_ICON_VOLUMESLIDER2RED, true );
								break;
								
							case 3:
								paintSlider(x + BORDER_LEFT, y + hheight + 3*mheight, tint, LOCALE_PSISETUP_TINT, NEUTRINO_ICON_VOLUMESLIDER2RED, true );
								break;
						}
					}
					break;
				}

			case CRCInput::RC_right:
			case CRCInput::RC_vfdright:
				{
					switch(selected)
					{
						case 0:
							if (*contrast < 255) 
							{
								if ( *contrast < 250 )
									*contrast += PSI_STEP;
								else
									*contrast = 255;
						
								paintSlider(x + BORDER_LEFT, y + hheight, contrast, LOCALE_PSISETUP_CONTRAST, NEUTRINO_ICON_VOLUMESLIDER2RED, true );
									
								videoDecoder->setContrast(*contrast);
							}
							break;
							
						case 1:
							if ( *saturation < 255 ) 
							{
								if (*saturation < 250)
									*saturation += PSI_STEP;
								else
									*saturation = 255;
								
								paintSlider(x + BORDER_LEFT, y + hheight + mheight, saturation, LOCALE_PSISETUP_SATURATION, NEUTRINO_ICON_VOLUMESLIDER2RED, true );
									
								videoDecoder->setSaturation(*saturation);
							}
							break;
							
						case 2:
							if ( *brightness < 255 ) 
							{
								if (*brightness < 250)
									*brightness += PSI_STEP;
								else
									*brightness = 255;
								
								paintSlider(x + BORDER_LEFT, y + hheight + 2*mheight, brightness, LOCALE_PSISETUP_BRIGHTNESS, NEUTRINO_ICON_VOLUMESLIDER2RED, true );
									
								videoDecoder->setBrightness(*brightness);
							}
							break;
							
						case 3:
							if ( *tint < 255 ) 
							{
								if (*tint < 250)
									*tint += PSI_STEP;
								else
									*tint = 255;
								
								paintSlider(x + BORDER_LEFT, y + hheight + 3*mheight, tint, LOCALE_PSISETUP_TINT, NEUTRINO_ICON_VOLUMESLIDER2RED, true );
									
								videoDecoder->setTint(*tint);
							}
							break;
					}
					break;
				}
				
			case CRCInput::RC_left:
			case CRCInput::RC_vfdleft:
				{
					switch(selected)
					{
						case 0:
							if ( *contrast > 0 ) 
							{
								if (* contrast > PSI_STEP)
									*contrast -= PSI_STEP;
								else
									*contrast = 0;
						
								paintSlider(x + BORDER_LEFT, y + hheight, contrast, LOCALE_PSISETUP_CONTRAST, NEUTRINO_ICON_VOLUMESLIDER2RED, true );
									
								videoDecoder->setContrast(*contrast);
							}
							break;
							
						case 1:
							if ( *saturation > 0 ) 
							{
								if (* saturation > PSI_STEP)
									*saturation -= PSI_STEP;
								else
									*saturation = 0;
								
								paintSlider(x + BORDER_LEFT, y + hheight + mheight, saturation, LOCALE_PSISETUP_SATURATION, NEUTRINO_ICON_VOLUMESLIDER2RED, true );
									
								videoDecoder->setSaturation(*saturation);
							}
							break;
							
						case 2:
							if ( *brightness > 0 ) 
							{
								if (* brightness > PSI_STEP)
									*brightness -= PSI_STEP;
								else
									*brightness = 0;
								
								paintSlider(x + BORDER_LEFT, y + hheight + 2*mheight, brightness, LOCALE_PSISETUP_BRIGHTNESS, NEUTRINO_ICON_VOLUMESLIDER2RED, true );
									
								videoDecoder->setBrightness(*brightness);
							}
							break;
							
						case 3:
							if ( *tint > 0 ) 
							{
								if (* tint > PSI_STEP)
									*tint -= PSI_STEP;
								else
									*tint = 0;
								
								paintSlider(x + BORDER_LEFT, y + hheight + 3*mheight, tint, LOCALE_PSISETUP_TINT, NEUTRINO_ICON_VOLUMESLIDER2RED, true );
									
								videoDecoder->setTint(*tint);
							}
							break;
					}
					break;
				}

			case CRCInput::RC_home:
			case CRCInput::RC_vfdexit:
				if ( *contrast != contrast_old || *saturation != saturation_old || *brightness != brightness_old || *tint != tint_old )
				{
					if (MessageBox(name, LOCALE_MESSAGEBOX_DISCARD, CMessageBox::mbrYes, CMessageBox::mbYes | CMessageBox::mbCancel) == CMessageBox::mbrCancel)
					{
						break;
					}
				}

				// sonst abbruch...
				*contrast = contrast_old;
				*saturation = saturation_old;
				*brightness = brightness_old;
				*tint = tint_old;

			case CRCInput::RC_timeout:
			case CRCInput::RC_ok:
			case CRCInput::RC_vfdok:
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
		observer->changeNotify(name, NULL);

	return res;
}

void CPSISetup::hide()
{
	frameBuffer->paintBackgroundBoxRel(x, y, width, height);
	frameBuffer->blit();
}

void CPSISetup::paint()
{
	//box top
	frameBuffer->paintBoxRel(x, y, width, hheight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, true);
	
	//title
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(x + BORDER_LEFT, y + hheight, width, g_Locale->getText(name), COL_MENUHEAD, 0, true); // UTF-8
	
	//box bottom
	frameBuffer->paintBoxRel(x, y + hheight, width, height - hheight, COL_MENUCONTENT_PLUS_0, RADIUS_MID, CORNER_BOTTOM);
	
	//paint slider
	// contrast
	paintSlider(x + BORDER_LEFT, y + hheight, contrast, LOCALE_PSISETUP_CONTRAST, NEUTRINO_ICON_VOLUMESLIDER2RED, true );
	
	// saturation
	paintSlider(x + BORDER_LEFT, y + hheight + mheight, saturation, LOCALE_PSISETUP_SATURATION, NEUTRINO_ICON_VOLUMESLIDER2, false);
	
	// brightness
	paintSlider(x + BORDER_LEFT, y + hheight + 2*mheight, brightness, LOCALE_PSISETUP_BRIGHTNESS, NEUTRINO_ICON_VOLUMESLIDER2, false);
	
	// tint
	paintSlider(x + BORDER_LEFT, y + hheight + 3*mheight, tint, LOCALE_PSISETUP_TINT, NEUTRINO_ICON_VOLUMESLIDER2, false);
}

void CPSISetup::paintSlider(const int _x, const int _y, const unsigned char * const spos, const neutrino_locale_t text, const char * const iconname, const bool /*selected*/) // UTF-8
{
	if (!spos)
		return;

	int sspos = (*spos)*100/255;
	char wert[5];
	
	int icon_w = 120;
	int icon_h = 11;
	
	frameBuffer->getIconSize(NEUTRINO_ICON_VOLUMEBODY, &icon_w, &icon_h);
	
	int slider_w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth("100", true); //UTF-8
	
	int startx = width - icon_w - slider_w - 50;

	frameBuffer->paintBoxRel(_x + startx, _y, 120, mheight, COL_MENUCONTENT_PLUS_0);

	frameBuffer->paintIcon(NEUTRINO_ICON_VOLUMEBODY, _x + startx, _y + 2 + mheight / 4);
	frameBuffer->paintIcon(iconname, _x + startx + 3 + sspos, _y + mheight / 4);

	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(_x, _y + mheight, width, g_Locale->getText(text), COL_MENUCONTENT, 0, true); // UTF-8
	
	sprintf(wert, "%3d", sspos); // UTF-8 encoded

	frameBuffer->paintBoxRel(_x + startx + 120 + BORDER_LEFT, _y, 50, mheight, COL_MENUCONTENT_PLUS_0);
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(_x + startx + 120 + BORDER_LEFT, _y + mheight, width, wert, COL_MENUCONTENT, 0, true); // UTF-8
}

