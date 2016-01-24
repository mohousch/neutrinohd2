/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: bouqueteditor_chanselect.cpp 2013/10/12 mohousch Exp $

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

#include <gui/bedit/bouqueteditor_chanselect.h>

#include <global.h>
#include <neutrino.h>

#include <driver/fontrenderer.h>
#include <driver/screen_max.h>
#include <gui/widget/icons.h>

/*zapit includes*/
#include <client/zapitclient.h>
#include <channel.h>
#include <bouquets.h>
#include <client/zapittools.h>
#include <satconfig.h>
#include <getservices.h>

#include <system/debug.h>


extern tallchans allchans;
extern CBouquetManager * g_bouquetManager;
void addChannelToBouquet(const unsigned int bouquet, const t_channel_id channel_id);

CBEChannelSelectWidget::CBEChannelSelectWidget(const std::string & Caption, unsigned int Bouquet, CZapitClient::channelsMode Mode)
	:CListBox(Caption.c_str(), MENU_WIDTH, MENU_HEIGHT, true)
{	
	bouquet = Bouquet;
	mode = Mode;
	
	// foot
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_OKAY, &icon_foot_w, &icon_foot_h);
	ButtonHeight = std::max(g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight(), icon_foot_h) + 10;

	// head
	theight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	
	// item
	frameBuffer->getIconSize(NEUTRINO_ICON_RESOLUTION_HD, &icon_w_hd, &icon_h_hd);
	frameBuffer->getIconSize(NEUTRINO_ICON_SCRAMBLED2, &icon_w_s, &icon_h_s);
	
	fheight = std::max(g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), icon_h_hd);

	liststart = 0;
}

uint CBEChannelSelectWidget::getItemCount()
{
	return Channels.size();
}

bool CBEChannelSelectWidget::isChannelInBouquet( int index)
{
	for (unsigned int i = 0; i< bouquetChannels->size(); i++)
	{
		if ((*bouquetChannels)[i]->channel_id == Channels[index]->channel_id)
			return true;
	}
	
	return false;
}

bool CBEChannelSelectWidget::hasChanged()
{
	return modified;
}

void CBEChannelSelectWidget::paintItem(uint32_t itemNr, int paintNr, bool _selected)
{
	int ypos = y + theight + paintNr*fheight;

	uint8_t    color;
	fb_pixel_t bgcolor;
	
	if (_selected)
	{
		color   = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
		
		frameBuffer->paintBoxRel(x, ypos, width - SCROLLBAR_WIDTH, fheight, COL_MENUCONTENT_PLUS_0);
		
		// itemlines	
		paintItem2DetailsLine(paintNr, itemNr);		
		
		// details
		paintDetails(itemNr);
	}
	else
	{
		color   = COL_MENUCONTENT;
		bgcolor = COL_MENUCONTENT_PLUS_0;
	}
	
	// itemBox
	frameBuffer->paintBoxRel(x, ypos, width - SCROLLBAR_WIDTH, fheight, bgcolor);

	//
	int icon_w, icon_h;
	
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_MARK, &icon_w, &icon_h);
	
	if(itemNr < getItemCount())
	{
		g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(x + 8 + icon_w + 8, ypos + (fheight - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), width - (8 + icon_w + 8 + 8), Channels[itemNr]->getName(), color, 0, true);

		if( isChannelInBouquet(itemNr))
		{
			frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_MARK, x + 8, ypos + (fheight - icon_h)/2);
		}
		else
		{
			frameBuffer->paintBoxRel(x + 8, ypos + 4, icon_w, fheight - 4, bgcolor);
		}
		
		//FIXME???
		// hd/scrambled icons
		if (g_settings.channellist_ca)
		{
			// scrambled icon
			if( Channels[itemNr]->scrambled) 
				frameBuffer->paintIcon(NEUTRINO_ICON_SCRAMBLED2, x + width - (SCROLLBAR_WIDTH + 2 + icon_w_s), ypos + (fheight - icon_h_s)/2 );
			
			// hd icon
			if( Channels[itemNr]->isHD() ) 
				frameBuffer->paintIcon(NEUTRINO_ICON_RESOLUTION_HD, x + width - (SCROLLBAR_WIDTH + 2 + icon_w_s + 2 + icon_w_hd), ypos + (fheight - icon_h_hd)/2 );
		}
	}
}

void CBEChannelSelectWidget::onOkKeyPressed()
{
	if(Channels.empty())
		return;
	
	setModified();
	
	if (isChannelInBouquet(selected))
		g_bouquetManager->Bouquets[bouquet]->removeService(Channels[selected]->channel_id);
	else
		addChannelToBouquet( bouquet, Channels[selected]->channel_id);

	bouquetChannels = mode == CZapitClient::MODE_TV ? &(g_bouquetManager->Bouquets[bouquet]->tvChannels) : &(g_bouquetManager->Bouquets[bouquet]->radioChannels);
	
	paintItem(selected, selected - liststart, false);
	g_RCInput->postMsg( CRCInput::RC_down, 0 );
}

int CBEChannelSelectWidget::exec(CMenuTarget * parent, const std::string & actionKey)
{
	dprintf(DEBUG_NORMAL, "CBEChannelSelectWidget::exec: actionKey:%s\n", actionKey.c_str());

	// info height
	info_height = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight() + 10;
	
	width = w_max ( (frameBuffer->getScreenWidth() / 20 * 17), (frameBuffer->getScreenWidth() / 20 ));
	height = h_max ( (frameBuffer->getScreenHeight() / 20 * 16), (frameBuffer->getScreenHeight() / 20));
	
	listmaxshow = (height - theight - ButtonHeight)/fheight;
	height = theight + ButtonHeight + listmaxshow*fheight; // recalc height
	
	x = frameBuffer->getScreenX() + (frameBuffer->getScreenWidth() - (width + ConnectLineBox_Width)) / 2 + ConnectLineBox_Width;
	y = frameBuffer->getScreenY() + (frameBuffer->getScreenHeight() - (height + info_height)) / 2;

	bouquetChannels = mode == CZapitClient::MODE_TV ? &(g_bouquetManager->Bouquets[bouquet]->tvChannels) : &(g_bouquetManager->Bouquets[bouquet]->radioChannels);

	Channels.clear();
	
	if (mode == CZapitClient::MODE_RADIO) 
	{
		for (tallchans_iterator it = allchans.begin(); it != allchans.end(); it++)
			if (it->second.getServiceType() == ST_DIGITAL_RADIO_SOUND_SERVICE)
				Channels.push_back(&(it->second));
	} 
	else 
	{
		for (tallchans_iterator it = allchans.begin(); it != allchans.end(); it++)
			if (it->second.getServiceType() != ST_DIGITAL_RADIO_SOUND_SERVICE)
				Channels.push_back(&(it->second));
	}
	sort(Channels.begin(), Channels.end(), CmpChannelByChName());

	return CListBox::exec(parent, actionKey);
}

void CBEChannelSelectWidget::paintFoot()
{
	int ButtonWidth = width / 3;
	frameBuffer->paintBoxRel(x, y + height - ButtonHeight, width, ButtonHeight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_BOTTOM, true, gradientDark2Light);

	// ok
	frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_OKAY, x + width - 3*ButtonWidth + BORDER_LEFT, y + height - ButtonHeight + (ButtonHeight - icon_foot_h)/2);

	g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(x + width - 3*ButtonWidth + icon_foot_w + BORDER_LEFT + 5, y + height - ButtonHeight + (ButtonHeight - g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight(), width, g_Locale->getText(LOCALE_BOUQUETEDITOR_SWITCH), COL_INFOBAR, 0, true); // UTF-8

	// home
	frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_HOME, x + width - ButtonWidth + BORDER_LEFT, y + height - ButtonHeight + (ButtonHeight - icon_foot_h)/2);

	g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(x + width - ButtonWidth + icon_foot_w + BORDER_LEFT + 5, y + height - ButtonHeight + (ButtonHeight - g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight(), width, g_Locale->getText(LOCALE_BOUQUETEDITOR_RETURN), COL_INFOBAR, 0, true); // UTF-8	
}

//
void CBEChannelSelectWidget::paintDetails(int index)
{
	// infobox refresh
	frameBuffer->paintBoxRel(x + 2, y + height + 2, width - 4, info_height - 4, COL_MENUCONTENTDARK_PLUS_0, true, gradientLight2Dark);
	
	if(Channels.empty())
		return;
	
	char buf[128];
	transponder_id_t ct = Channels[index]->getTransponderId();
	transponder_list_t::iterator tpI = transponders.find(ct);
	int len = snprintf(buf, sizeof(buf), "%d ", Channels[index]->getFreqId());

	// satname
	sat_iterator_t sit = satellitePositions.find(Channels[index]->getSatellitePosition());
		
	if(sit != satellitePositions.end()) 
	{
		//int satNameWidth = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getRenderWidth (sit->second.name);
		snprintf(&buf[len], sizeof(buf) - len, "(%s)\n", sit->second.name.c_str());
	}
	
	g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(x + 10, y + height + 5 + fheight, width - 30, buf, COL_MENUCONTENTDARK, 0, true);
}

void CBEChannelSelectWidget::paintItem2DetailsLine(int pos, int /*ch_index*/)
{
	int xpos  = x - ConnectLineBox_Width;
	int ypos1 = y + theight + pos*fheight;
	int ypos2 = y + height;
	int ypos1a = ypos1 + (fheight/2) - 2;
	int ypos2a = ypos2 + (info_height/2) - 2;
	fb_pixel_t col1 = COL_MENUCONTENT_PLUS_6;
	fb_pixel_t col2 = COL_MENUCONTENT_PLUS_1;

	// Clear
	frameBuffer->paintBackgroundBoxRel(xpos, y, ConnectLineBox_Width, height + info_height);

	// blit
	frameBuffer->blit();

	// paint Line if detail info (and not valid list pos)
	if (pos >= 0) 
	{ 
		int fh = fheight > 10 ? fheight - 10 : 5;
			
		frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 4, ypos1 + 5, 4, fh, col1);
		frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 4, ypos1 + 5, 1, fh, col2);			

		frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 4, ypos2 + 7, 4, info_height - 14, col1);
		frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 4, ypos2 + 7, 1, info_height - 14, col2);			

		// vertical line
		frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 16, ypos1a, 4, ypos2a - ypos1a, col1);
		frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 16, ypos1a, 1, ypos2a - ypos1a + 4, col2);		

		// Hline Oben
		frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 15, ypos1a, 12,4, col1);
		frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 16, ypos1a, 12,1, col2);
		
		// Hline Unten
		frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 15, ypos2a, 12, 4, col1);
		frameBuffer->paintBoxRel(xpos + ConnectLineBox_Width - 12, ypos2a, 8, 1, col2);

		// untere info box lines
		frameBuffer->paintBoxRel(x, ypos2, width, info_height, col1, true);
	}
}

void CBEChannelSelectWidget::clearItem2DetailsLine()
{  
	  paintItem2DetailsLine(-1, 0);  
}
