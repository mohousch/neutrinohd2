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

#include <global.h>
#include <neutrino.h>

#include <driver/fontrenderer.h>
#include <driver/rcinput.h>
#include <driver/screen_max.h>

#include <driver/color.h>

#include <gui/widget/messagebox.h>
#include <gui/widget/colorchooser.h>


#define VALUE_R     0
#define VALUE_G     1
#define VALUE_B     2
#define VALUE_ALPHA 3
#define ITEMS_COUNT 4

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

	// Head
	titleIcon.setIcon(NEUTRINO_ICON_COLORS);
	cFrameBoxTitle.iHeight= std::max(titleIcon.iHeight, g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight()) + 6;

	// Item
	cFrameBoxItem.iHeight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	
	cFrameBox.iWidth = MENU_WIDTH;
	cFrameBox.iHeight = cFrameBoxTitle.iHeight + cFrameBoxItem.iHeight*ITEMS_COUNT;

	cFrameBox.iX = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - cFrameBox.iWidth) >> 1);
	cFrameBox.iY = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - cFrameBox.iHeight) >>1);

	value[VALUE_R]     = R;
	value[VALUE_G]     = G;
	value[VALUE_B]     = B;
	value[VALUE_ALPHA] = Alpha;
	
	// color preview
	volumeBodyIcon.setIcon(NEUTRINO_ICON_VOLUMEBODY);
	int a_w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(g_Locale->getText(colorchooser_names[3]));

	cFrameBoxColorPreview.iHeight = cFrameBox.iHeight - cFrameBoxTitle.iHeight - cFrameBoxItem.iHeight;
	cFrameBoxColorPreview.iWidth = cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT - 2*ICON_OFFSET - volumeBodyIcon.iWidth - 2*ICON_OFFSET - a_w;
	cFrameBoxColorPreview.iX = cFrameBox.iX + cFrameBox.iWidth - BORDER_RIGHT - cFrameBoxColorPreview.iWidth;
	cFrameBoxColorPreview.iY = cFrameBox.iY + cFrameBoxTitle.iHeight + cFrameBoxItem.iHeight/2;

	//
	m_cBoxWindow.setPosition(&cFrameBox);
	m_cBoxWindow.enableSaveScreen();
}

CColorChooser::CColorChooser(const char * const Name, unsigned char *R, unsigned char *G, unsigned char *B, unsigned char* Alpha, CChangeObserver* Observer) // UTF-8
{
	observer = Observer;

	name = Name;
	nameStringOption = NONEXISTANT_LOCALE;
	
	frameBuffer = CFrameBuffer::getInstance();

	// Head
	titleIcon.setIcon(NEUTRINO_ICON_COLORS);
	cFrameBoxTitle.iHeight= std::max(titleIcon.iHeight, g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight()) + 6;

	// Item
	cFrameBoxItem.iHeight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	
	cFrameBox.iWidth = MENU_WIDTH;
	cFrameBox.iHeight = cFrameBoxTitle.iHeight + cFrameBoxItem.iHeight*ITEMS_COUNT;

	cFrameBox.iX = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - cFrameBox.iWidth) >> 1);
	cFrameBox.iY = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - cFrameBox.iHeight) >>1);

	value[VALUE_R]     = R;
	value[VALUE_G]     = G;
	value[VALUE_B]     = B;
	value[VALUE_ALPHA] = Alpha;
	
	// ColorPreview Box
	volumeBodyIcon.setIcon(NEUTRINO_ICON_VOLUMEBODY);
	int a_w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(g_Locale->getText(colorchooser_names[3]));

	cFrameBoxColorPreview.iHeight = cFrameBox.iHeight - cFrameBoxTitle.iHeight - cFrameBoxItem.iHeight;
	cFrameBoxColorPreview.iWidth = cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT - 2*ICON_OFFSET - volumeBodyIcon.iWidth - 2*ICON_OFFSET - a_w;
	cFrameBoxColorPreview.iX = cFrameBox.iX + cFrameBox.iWidth - BORDER_RIGHT - cFrameBoxColorPreview.iWidth;
	cFrameBoxColorPreview.iY = cFrameBox.iY + cFrameBoxTitle.iHeight + cFrameBoxItem.iHeight/2;

	//
	m_cBoxWindow.setPosition(&cFrameBox);
	m_cBoxWindow.enableSaveScreen();
}

void CColorChooser::setColor()
{
	int color = convertSetupColor2RGB(*(value[VALUE_R]), *(value[VALUE_G]), *(value[VALUE_B]));
	int tAlpha = (value[VALUE_ALPHA]) ? (convertSetupAlpha2Alpha(*(value[VALUE_ALPHA]))) : 0;

	if(!value[VALUE_ALPHA]) 
		tAlpha = 0xFF;

	fb_pixel_t col = ((tAlpha << 24) & 0xFF000000) | color;
	
	frameBuffer->paintBoxRel(cFrameBoxColorPreview.iX + 2, cFrameBoxColorPreview.iY + 2,  cFrameBoxColorPreview.iWidth - 4, cFrameBoxColorPreview.iHeight - 4, col);
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

	uint64_t timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_MENU] == 0 ? 0xFFFF : g_settings.timing[SNeutrinoSettings::TIMING_MENU]);

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
						paintSlider(cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBoxTitle.iHeight + cFrameBoxItem.iHeight*selected, value[selected], colorchooser_names[selected], iconnames[selected], false);
						selected++;
						paintSlider(cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBoxTitle.iHeight + cFrameBoxItem.iHeight*selected, value[selected], colorchooser_names[selected], iconnames[selected], true);
					} 
					else 
					{
						paintSlider(cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBoxTitle.iHeight + cFrameBoxItem.iHeight*selected, value[selected], colorchooser_names[selected], iconnames[selected], false);
						selected = 0;
						paintSlider(cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBoxTitle.iHeight + cFrameBoxItem.iHeight*selected, value[selected], colorchooser_names[selected], iconnames[selected], true);
					}
					break;
				}
				
			case CRCInput::RC_up:
				{
					if (selected > 0)
					{
						paintSlider(cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBoxTitle.iHeight + cFrameBoxItem.iHeight*selected, value[selected], colorchooser_names[selected], iconnames[selected], false);
						selected--;
						paintSlider(cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBoxTitle.iHeight + cFrameBoxItem.iHeight*selected, value[selected], colorchooser_names[selected], iconnames[selected], true);
					} 
					else 
					{
						paintSlider(cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBoxTitle.iHeight + cFrameBoxItem.iHeight*selected, value[selected], colorchooser_names[selected], iconnames[selected], false);
						selected = ((value[VALUE_ALPHA]) ? 3 : 2);
						paintSlider(cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBoxTitle.iHeight + cFrameBoxItem.iHeight*selected, value[selected], colorchooser_names[selected], iconnames[selected], true);
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

						paintSlider(cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBoxTitle.iHeight + cFrameBoxItem.iHeight*selected, value[selected], colorchooser_names[selected], iconnames[selected], true);
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

						paintSlider(cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBoxTitle.iHeight + cFrameBoxItem.iHeight*selected, value[selected], colorchooser_names[selected], iconnames[selected], true);
						setColor();
					}
					break;
				}
				
			case CRCInput::RC_home:
				if (((*value[VALUE_R] != r_alt) || (*value[VALUE_G] != g_alt) || (*value[VALUE_B] != b_alt) || ((value[VALUE_ALPHA]) && (*(value[VALUE_ALPHA]) != a_alt))) &&
						(MessageBox(name.c_str(), LOCALE_MESSAGEBOX_DISCARD, CMessageBox::mbrYes, CMessageBox::mbYes | CMessageBox::mbCancel) == CMessageBox::mbrCancel))
					break;

				// cancel
				if (value[VALUE_R])
					*value[VALUE_R] = r_alt;

				if (value[VALUE_G])
					*value[VALUE_G] = g_alt;

				if (value[VALUE_B])
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
	m_cBoxWindow.hide();
	frameBuffer->blit();
}

void CColorChooser::paint()
{
	// box
	m_cBoxWindow.setColor(COL_MENUCONTENT_PLUS_0);
	m_cBoxWindow.setCorner(RADIUS_MID, CORNER_ALL);
	m_cBoxWindow.paint();

	// Head
	cFrameBoxTitle.iX = cFrameBox.iX;
	cFrameBoxTitle.iY = cFrameBox.iY;
	cFrameBoxTitle.iWidth = cFrameBox.iWidth;

	headers.paintHead(cFrameBoxTitle, titleIcon.iconName.c_str(), name);

	// slider
	for (int i = 0; i < ITEMS_COUNT; i++)
		paintSlider(cFrameBox.iX + BORDER_LEFT, cFrameBox.iY + cFrameBoxTitle.iHeight + cFrameBoxItem.iHeight*i, value[i], colorchooser_names[i], iconnames[i], (i == 0));

	//color preview
	volumeBodyIcon.setIcon(NEUTRINO_ICON_VOLUMEBODY);
	int a_w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(g_Locale->getText(colorchooser_names[3]));

	cFrameBoxColorPreview.iHeight = cFrameBox.iHeight - cFrameBoxTitle.iHeight - cFrameBoxItem.iHeight;
	cFrameBoxColorPreview.iWidth = cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT - 2*ICON_OFFSET - volumeBodyIcon.iWidth - 2*ICON_OFFSET - a_w;
	cFrameBoxColorPreview.iX = cFrameBox.iX + cFrameBox.iWidth - BORDER_RIGHT - cFrameBoxColorPreview.iWidth;
	cFrameBoxColorPreview.iY = cFrameBox.iY + cFrameBoxTitle.iHeight + cFrameBoxItem.iHeight/2;

	frameBuffer->paintBoxRel(cFrameBoxColorPreview.iX, cFrameBoxColorPreview.iY, cFrameBoxColorPreview.iWidth, cFrameBoxColorPreview.iHeight, COL_MENUHEAD_PLUS_0);

	frameBuffer->paintBoxRel(cFrameBoxColorPreview.iX + 2, cFrameBoxColorPreview.iY + 2, cFrameBoxColorPreview.iWidth - 4, cFrameBoxColorPreview.iHeight - 4 - BORDER_LEFT, 254);
}

void CColorChooser::paintSlider(int _x, int _y, unsigned char *spos, const neutrino_locale_t text, const char * const iconname, const bool selected)
{
	if (!spos)
		return;

	//
	volumeBodyIcon.setIcon(NEUTRINO_ICON_VOLUMEBODY);
	//int r_w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(g_Locale->getText(colorchooser_names[0]));
	//int g_w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(g_Locale->getText(colorchooser_names[1]));
	//int b_w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(g_Locale->getText(colorchooser_names[2]));
	int a_w = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(g_Locale->getText(colorchooser_names[3]));
	
	// volumebox box
	frameBuffer->paintBoxRel(_x + a_w + 2*ICON_OFFSET, _y, volumeBodyIcon.iWidth, cFrameBoxItem.iHeight, COL_MENUCONTENT_PLUS_0);

	// volumebody icon
	frameBuffer->paintIcon(NEUTRINO_ICON_VOLUMEBODY, _x + a_w + 2*ICON_OFFSET, _y + 2 + cFrameBoxItem.iHeight/ITEMS_COUNT);

	// slider icon
	frameBuffer->paintIcon(selected ? iconname : NEUTRINO_ICON_VOLUMESLIDER2, _x + a_w + 2*ICON_OFFSET + 3 + (*spos), _y + cFrameBoxItem.iHeight/ITEMS_COUNT);

	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(_x, _y + cFrameBoxItem.iHeight, cFrameBox.iWidth - BORDER_LEFT - BORDER_RIGHT - ICON_OFFSET - volumeBodyIcon.iWidth - cFrameBoxColorPreview.iWidth, g_Locale->getText(text), COL_MENUCONTENT, 0, true); // UTF-8
}

