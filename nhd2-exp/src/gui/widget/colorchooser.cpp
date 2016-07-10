/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: colorchooser.cpp 2013/10/12 mohousch Exp $

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

#include <gui/widget/colorchooser.h>

#include <global.h>
#include <neutrino.h>

#include <driver/fontrenderer.h>
#include <driver/rcinput.h>
#include <driver/screen_max.h>

#include <gui/color.h>
#include <gui/widget/messagebox.h>


#define VALUE_R     0
#define VALUE_G     1
#define VALUE_B     2
#define VALUE_ALPHA 3

static const char * const iconnames[4] = {
	"volumeslider2red",
	"volumeslider2green",
	"volumeslider2blue",
	"volumeslider2alpha"
};

static const neutrino_locale_t colorchooser_names[4] =
{
	LOCALE_COLORCHOOSER_RED  ,
	LOCALE_COLORCHOOSER_GREEN,
	LOCALE_COLORCHOOSER_BLUE ,
	LOCALE_COLORCHOOSER_ALPHA
};

CColorChooser::CColorChooser(const neutrino_locale_t Name, unsigned char *R, unsigned char *G, unsigned char *B, unsigned char* Alpha, CChangeObserver* Observer) // UTF-8
{
	observer = Observer;

	nameStringOption = Name;
	name = g_Locale->getText(Name);
	
	frameBuffer = CFrameBuffer::getInstance();
	hheight     = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	mheight     = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	
	width = w_max(MENU_WIDTH, 0);
	height = h_max(hheight + mheight*4, 0);

	x = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - width) >> 1);
	y = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - height) >>1);

	value[VALUE_R]     = R;
	value[VALUE_G]     = G;
	value[VALUE_B]     = B;
	value[VALUE_ALPHA] = Alpha;
	
	startx = width - mheight*4 - 30; //FIXME: 30?
}

CColorChooser::CColorChooser(const char * const Name, unsigned char *R, unsigned char *G, unsigned char *B, unsigned char* Alpha, CChangeObserver* Observer) // UTF-8
{
	observer = Observer;

	name = Name;
	nameStringOption = NONEXISTANT_LOCALE;
	
	frameBuffer = CFrameBuffer::getInstance();
	hheight     = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	mheight     = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	
	width = w_max(MENU_WIDTH, 0);
	height = h_max(hheight + mheight*4, 0);

	x = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - width) >> 1);
	y = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - height) >>1);

	value[VALUE_R]     = R;
	value[VALUE_G]     = G;
	value[VALUE_B]     = B;
	value[VALUE_ALPHA] = Alpha;
	
	startx = width - mheight*4 - 30; //FIXME: 30?
}

void CColorChooser::setColor()
{
	int color = convertSetupColor2RGB(*(value[VALUE_R]), *(value[VALUE_G]), *(value[VALUE_B]));
	int tAlpha = (value[VALUE_ALPHA]) ? (convertSetupAlpha2Alpha(*(value[VALUE_ALPHA]))) : 0;

	if(!value[VALUE_ALPHA]) 
		tAlpha = 0xFF;

	fb_pixel_t col = ((tAlpha << 24) & 0xFF000000) | color;
	
	frameBuffer->paintBoxRel(x + startx + 2, y + hheight + 2 + 5,  mheight*4 - 4 ,mheight*4 - 4 - 10, col);
}

int CColorChooser::exec(CMenuTarget *parent, const std::string &)
{
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	int res = menu_return::RETURN_REPAINT;
	
	if (parent)
		parent->hide();

	unsigned char r_alt= *value[VALUE_R];
	unsigned char g_alt= *value[VALUE_G];
	unsigned char b_alt= *value[VALUE_B];
	unsigned char a_alt = (value[VALUE_ALPHA]) ? (*(value[VALUE_ALPHA])) : 0;

	paint();
	setColor();
	
	frameBuffer->blit();

	int selected = 0;

	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);

	bool loop = true;
	while (loop) 
	{
		g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd, true);

		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);

		switch ( msg ) 
		{
			case CRCInput::RC_down:
				{
					if (selected < ((value[VALUE_ALPHA]) ? 3 : 2))
					{
						paintSlider(x + 10, y + hheight + mheight * selected, value[selected], colorchooser_names[selected], iconnames[selected], false);
						selected++;
						paintSlider(x + 10, y + hheight + mheight * selected, value[selected], colorchooser_names[selected], iconnames[selected], true);
					} 
					else 
					{
						paintSlider(x + 10, y + hheight + mheight * selected, value[selected], colorchooser_names[selected], iconnames[selected], false);
						selected = 0;
						paintSlider(x + 10, y + hheight + mheight * selected, value[selected], colorchooser_names[selected], iconnames[selected], true);
					}
					break;

				}
				
			case CRCInput::RC_up:
				{
					if (selected > 0)
					{
						paintSlider(x + 10, y + hheight + mheight * selected, value[selected], colorchooser_names[selected], iconnames[selected], false);
						selected--;
						paintSlider(x + 10, y + hheight + mheight * selected, value[selected], colorchooser_names[selected], iconnames[selected], true);
					} else {
						paintSlider(x + 10, y + hheight + mheight * selected, value[selected], colorchooser_names[selected], iconnames[selected], false);
						selected = ((value[VALUE_ALPHA]) ? 3 : 2);
						paintSlider(x + 10, y + hheight + mheight * selected, value[selected], colorchooser_names[selected], iconnames[selected], true);
					}
					break;
				}
				
			case CRCInput::RC_right:
				{
					if ((*value[selected]) < 100)
					{
						if ((*value[selected]) < 98)
							(*value[selected]) += 2;
						else
							(*value[selected]) = 100;

						paintSlider(x + 10, y + hheight + mheight * selected, value[selected], colorchooser_names[selected], iconnames[selected], true);
						setColor();
					}
					break;
				}
				
			case CRCInput::RC_left:
				{
					if ((*value[selected]) > 0)
					{
						if ((*value[selected]) > 2)
							(*value[selected]) -= 2;
						else
							(*value[selected]) = 0;

						paintSlider(x + 10, y + hheight + mheight * selected, value[selected], colorchooser_names[selected], iconnames[selected], true);
						setColor();
					}
					break;
				}
				
			case CRCInput::RC_home:
				if (((*value[VALUE_R] != r_alt) || (*value[VALUE_G] != g_alt) || (*value[VALUE_B] != b_alt) || ((value[VALUE_ALPHA]) && (*(value[VALUE_ALPHA]) != a_alt))) &&
						(MessageBox(name.c_str(), LOCALE_MESSAGEBOX_DISCARD, CMessageBox::mbrYes, CMessageBox::mbYes | CMessageBox::mbCancel) == CMessageBox::mbrCancel))
					break;

				// sonst abbruch...
				*value[VALUE_R] = r_alt;
				*value[VALUE_G] = g_alt;
				*value[VALUE_B] = b_alt;
				if (value[VALUE_ALPHA])
					*value[VALUE_ALPHA] = a_alt;
	
			case CRCInput::RC_timeout:
			case CRCInput::RC_ok:
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

void CColorChooser::hide()
{
	frameBuffer->paintBackgroundBoxRel(x, y, width + SHADOW_OFFSET, height + SHADOW_OFFSET);
	
	frameBuffer->blit();
}

void CColorChooser::paint()
{
	// head
	//shadow
	frameBuffer->paintBoxRel(x + SHADOW_OFFSET, y + SHADOW_OFFSET, width, hheight, COL_INFOBAR_SHADOW_PLUS_0, RADIUS_MID, CORNER_TOP); //round

	// head
	frameBuffer->paintBoxRel(x, y, width, hheight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, true, gradientLight2Dark); //round
	
	// head title
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(x + BORDER_LEFT, y + hheight, width, name.c_str(), COL_MENUHEAD, 0, true); // UTF-8

	// menu box shadow
	frameBuffer->paintBoxRel(x + SHADOW_OFFSET, y + hheight + SHADOW_OFFSET, width, height - hheight, COL_INFOBAR_SHADOW_PLUS_0, RADIUS_MID, CORNER_BOTTOM);//round

	// menu box
	frameBuffer->paintBoxRel(x, y + hheight, width, height - hheight, COL_MENUCONTENT_PLUS_0, RADIUS_MID, CORNER_BOTTOM);//round

	// slider
	for (int i = 0; i < 4; i++)
		paintSlider(x + BORDER_LEFT, y + hheight + mheight*i, value[i], colorchooser_names[i], iconnames[i], (i == 0));

	//color preview
	frameBuffer->paintBoxRel(x + startx, y + hheight + 5, mheight*4, mheight*4 - 10, COL_MENUHEAD_PLUS_0);
	frameBuffer->paintBoxRel(x + startx + 2, y + hheight + 2 + 5, mheight*4 - 4, mheight*4 - 4 - 10, 254);
}

void CColorChooser::paintSlider(int _x, int _y, unsigned char *spos, const neutrino_locale_t text, const char * const iconname, const bool selected)
{
	if (!spos)
		return;
	
	frameBuffer->paintBoxRel(_x + 70, _y, 120, mheight, COL_MENUCONTENT_PLUS_0); //FIXME: 70? 120?
	frameBuffer->paintIcon(NEUTRINO_ICON_VOLUMEBODY, _x + 70, _y + 2 + mheight/4);
	frameBuffer->paintIcon(selected ? iconname : NEUTRINO_ICON_VOLUMESLIDER2, _x + 73 + (*spos), _y + mheight/4);

	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(_x, _y + mheight, width, g_Locale->getText(text), COL_MENUCONTENT, 0, true); // UTF-8
}
