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
#include <gui/widget/items2detailsline.h>
#include <gui/widget/buttons.h>

// zapit includes
#include <client/zapitclient.h>
#include <channel.h>
#include <bouquets.h>
#include <client/zapittools.h>
#include <satconfig.h>
#include <getservices.h>

#include <system/debug.h>
#include <system/helpers.h>


extern tallchans allchans;
extern CBouquetManager * g_bouquetManager;
void addChannelToBouquet(const unsigned int bouquet, const t_channel_id channel_id);

CBEChannelSelectWidget::CBEChannelSelectWidget(const std::string & Caption, unsigned int Bouquet, CZapitClient::channelsMode Mode)
{
	frameBuffer = CFrameBuffer::getInstance();

	//
	liststart = 0;
	selected =  0;

	caption = Caption;

	modified = false;

	//	
	bouquet = Bouquet;
	mode = Mode;

	// Title
	cFrameTitle.iHeight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight() + 10;
	
	// Foot
	footIcon.setIcon(NEUTRINO_ICON_BUTTON_OKAY);
	cFrameFoot.iHeight = std::max(g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), footIcon.iHeight) + 10;
	
	// Item
	itemIcon1.setIcon(NEUTRINO_ICON_HD);
	itemIcon2.setIcon(NEUTRINO_ICON_SCRAMBLED);
	
	cFrameItem.iHeight = std::max(g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), itemIcon1.iHeight);
	cFrameItem.iHeight = std::max(g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), itemIcon2.iHeight);

	// FootInfo
	cFrameFootInfo.iHeight = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight() + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight() + 10;
	
	cFrameBox.iWidth = w_max ( (frameBuffer->getScreenWidth() / 20 * 17), (frameBuffer->getScreenWidth() / 20 ));
	cFrameBox.iHeight = h_max ( (frameBuffer->getScreenHeight() / 20 * 16), (frameBuffer->getScreenHeight() / 20));
	
	listmaxshow = (cFrameBox.iHeight - cFrameTitle.iHeight - cFrameFoot.iHeight - cFrameFootInfo.iHeight)/cFrameItem.iHeight;

	// recalculate height
	cFrameBox.iHeight = cFrameTitle.iHeight + listmaxshow*cFrameItem.iHeight + cFrameFoot.iHeight + cFrameFootInfo.iHeight;
	
	cFrameBox.iX = frameBuffer->getScreenX() + (frameBuffer->getScreenWidth() - (cFrameBox.iWidth + ConnectLineBox_Width)) / 2 + ConnectLineBox_Width;
	cFrameBox.iY = frameBuffer->getScreenY() + (frameBuffer->getScreenHeight() - cFrameBox.iHeight) / 2;
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

void CBEChannelSelectWidget::paintHead()
{
	// headBox
	cFrameTitle.iX = cFrameBox.iX;
	cFrameTitle.iY = cFrameBox.iY;
	cFrameTitle.iWidth = cFrameBox.iWidth;
	
	cWindowTitle.setDimension(&cFrameTitle);
	cWindowTitle.setColor(COL_MENUHEAD_PLUS_0);
	cWindowTitle.setCorner(RADIUS_MID, CORNER_TOP);
	cWindowTitle.setGradient(g_settings.menu_Head_gradient);
	cWindowTitle.paint();
	
	// title
	int timestr_len = 0;
	std::string timestr = getNowTimeStr("%d.%m.%Y %H:%M");;
		
	timestr_len = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getRenderWidth(timestr.c_str(), true); // UTF-8
	
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(cFrameTitle.iX + BORDER_LEFT, cFrameTitle.iY + cFrameTitle.iHeight, cFrameTitle.iWidth - BORDER_LEFT - BORDER_RIGHT - timestr_len, caption.c_str() , COL_MENUHEAD);

	// paint time/date	
	g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->RenderString(cFrameTitle.iX + cFrameTitle.iWidth - BORDER_RIGHT - timestr_len, cFrameTitle.iY + (cFrameTitle.iHeight - g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight(), timestr_len + 1, timestr.c_str(), COL_MENUHEAD, 0, true); 
}

void CBEChannelSelectWidget::paintItem(uint32_t itemNr, int paintNr, bool _selected)
{
	int ypos = cFrameBox.iY + cFrameTitle.iHeight + paintNr*cFrameItem.iHeight;

	uint8_t    color = COL_MENUCONTENT;
	fb_pixel_t bgcolor = COL_MENUCONTENT_PLUS_0;
	
	if (_selected)
	{
		color   = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
		
		// itemlines	
		paintItem2DetailsLine(paintNr);		
		
		// details
		paintDetails(itemNr);
	}
	
	// itemBox
	frameBuffer->paintBoxRel(cFrameBox.iX, ypos, cFrameBox.iWidth - SCROLLBAR_WIDTH, cFrameItem.iHeight, bgcolor);

	//
	itemIcon3.setIcon(NEUTRINO_ICON_BUTTON_MARK);
	
	if(itemNr < getItemCount())
	{
		g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(cFrameBox.iX + BORDER_LEFT + itemIcon3.iWidth + ICON_OFFSET, ypos + (cFrameItem.iHeight - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), cFrameBox.iWidth - (BORDER_LEFT + itemIcon3.iWidth + ICON_OFFSET + BORDER_RIGHT), Channels[itemNr]->getName(), color, 0, true);

		if( isChannelInBouquet(itemNr))
		{
			frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_MARK, cFrameBox.iX + BORDER_LEFT, ypos + (cFrameItem.iHeight - itemIcon3.iHeight)/2);
		}
		else
		{
			frameBuffer->paintBoxRel(cFrameBox.iX + BORDER_LEFT, ypos + 4, itemIcon3.iWidth, cFrameItem.iHeight - 4, bgcolor);
		}
		
		//FIXME???
		// hd/scrambled icons
		if (g_settings.channellist_ca)
		{
			// scrambled icon
			if( Channels[itemNr]->scrambled) 
				frameBuffer->paintIcon(NEUTRINO_ICON_SCRAMBLED, cFrameBox.iX + cFrameBox.iWidth - (SCROLLBAR_WIDTH + ICON_OFFSET + itemIcon2.iWidth), ypos + (cFrameItem.iHeight - itemIcon2.iHeight)/2 );
			
			// hd icon
			if( Channels[itemNr]->isHD() ) 
				frameBuffer->paintIcon(NEUTRINO_ICON_HD, cFrameBox.iX + cFrameBox.iWidth - (SCROLLBAR_WIDTH + ICON_OFFSET + itemIcon2.iWidth + ICON_OFFSET + itemIcon1.iWidth), ypos + (cFrameItem.iHeight - itemIcon1.iHeight)/2 );

			// uhd icon
			else if(Channels[itemNr]->isUHD()) 
				frameBuffer->paintIcon(NEUTRINO_ICON_UHD, cFrameBox.iX + cFrameBox.iWidth - (SCROLLBAR_WIDTH + ICON_OFFSET + itemIcon2.iWidth + ICON_OFFSET + itemIcon1.iWidth), ypos + (cFrameItem.iHeight - itemIcon1.iHeight)/2 );
		}
	}
}

void CBEChannelSelectWidget::paintItem(int pos)
{
	paintItem(liststart + pos, pos, (liststart + pos == selected) );
}

#define BUTTONS_COUNT 2
const struct button_label Buttons[BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_OKAY, LOCALE_BOUQUETEDITOR_SWITCH, NULL },
	{ NEUTRINO_ICON_BUTTON_HOME, LOCALE_BOUQUETEDITOR_RETURN, NULL }	
};

void CBEChannelSelectWidget::paintFoot()
{
	cFrameFoot.iX = cFrameBox.iX;
	cFrameFoot.iY = cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight - cFrameFoot.iHeight;
	cFrameFoot.iWidth = cFrameBox.iWidth;

	cWindowFoot.setDimension(&cFrameFoot);
	cWindowFoot.setColor(COL_MENUHEAD_PLUS_0);
	cWindowFoot.setCorner(RADIUS_MID, CORNER_BOTTOM);
	cWindowFoot.setGradient(g_settings.menu_Foot_gradient);
	cWindowFoot.paint();

	// Buttons
	int ButtonWidth = cFrameBox.iWidth / BUTTONS_COUNT;

	::paintButtons(frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, cFrameFoot.iX + BORDER_LEFT, cFrameFoot.iY, ButtonWidth, BUTTONS_COUNT, Buttons, cFrameFoot.iHeight);	
}

void CBEChannelSelectWidget::paint()
{
	dprintf(DEBUG_DEBUG, "CBEChannelSelectWidget::paint\n");

	liststart = (selected/listmaxshow)*listmaxshow;

	// items
	for(unsigned int count = 0; count < listmaxshow; count++)
	{
		paintItem(count);
	}

	// scrollbar
	cFrameScrollBar.iX = cFrameBox.iX + cFrameBox.iWidth - SCROLLBAR_WIDTH;
	cFrameScrollBar.iY = cFrameBox.iY + cFrameTitle.iHeight;
	cFrameScrollBar.iWidth = SCROLLBAR_WIDTH;
	cFrameScrollBar.iHeight = cFrameItem.iHeight*listmaxshow;

	::paintScrollBar(&cFrameScrollBar, ((getItemCount() - 1)/ listmaxshow) + 1, (selected/listmaxshow));
}

//
void CBEChannelSelectWidget::paintDetails(int index)
{
	// infobox refresh
	cFrameFootInfo.iX = cFrameBox.iX;
	cFrameFootInfo.iY = cFrameBox.iY + cFrameBox.iHeight - cFrameFootInfo.iHeight;
	cFrameFootInfo.iWidth = cFrameBox.iWidth;

	frameBuffer->paintBoxRel(cFrameFootInfo.iX + 2, cFrameFootInfo.iY + 2, cFrameFootInfo.iWidth - 4, cFrameFootInfo.iHeight - 4, COL_MENUCONTENTDARK_PLUS_0, NO_RADIUS, CORNER_NONE, g_settings.menu_Head_gradient);
	
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
	
	g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(cFrameFootInfo.iX + BORDER_LEFT, cFrameFootInfo.iY + (cFrameFootInfo.iHeight - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), cFrameFootInfo.iWidth - BORDER_LEFT - BORDER_RIGHT, buf, COL_MENUCONTENTDARK);
}

void CBEChannelSelectWidget::paintItem2DetailsLine(int pos)
{
	::paintItem2DetailsLine(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight - cFrameFootInfo.iHeight, cFrameFootInfo.iHeight, cFrameTitle.iHeight, cFrameItem.iHeight, pos);
}

void CBEChannelSelectWidget::clearItem2DetailsLine()
{  
	::clearItem2DetailsLine(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight - cFrameFootInfo.iHeight, cFrameFootInfo.iHeight);  
}

void CBEChannelSelectWidget::hide()
{
	frameBuffer->paintBackgroundBoxRel(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight);
	
	clearItem2DetailsLine();
	
	frameBuffer->blit();
}

int CBEChannelSelectWidget::exec(CMenuTarget * parent, const std::string & actionKey)
{
	dprintf(DEBUG_NORMAL, "CBEChannelSelectWidget::exec: actionKey:%s\n", actionKey.c_str());

	//
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

	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	int res = menu_return::RETURN_REPAINT;
	selected = 0;

	if (parent)
		parent->hide();

	paintHead();
	paint();
	paintFoot();
	
	frameBuffer->blit();

	bool loop = true;
	modified = false;

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);

	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_EPG]);
	
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );
		neutrino_msg_t msg_repeatok = msg & ~CRCInput::RC_Repeat;

		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_EPG]);

		if( msg == CRCInput::RC_ok)
		{
			setModified();
	
			if (isChannelInBouquet(selected))
				g_bouquetManager->Bouquets[bouquet]->removeService(Channels[selected]->channel_id);
			else
				addChannelToBouquet( bouquet, Channels[selected]->channel_id);

			bouquetChannels = mode == CZapitClient::MODE_TV ? &(g_bouquetManager->Bouquets[bouquet]->tvChannels) : &(g_bouquetManager->Bouquets[bouquet]->radioChannels);
	
			paintItem(selected);

			msg_repeatok = CRCInput::RC_down; // jump to next item
		}

		if (msg == CRCInput::RC_home)
		{
			loop = false;
		}
		else if (msg_repeatok == CRCInput::RC_up || msg == CRCInput::RC_page_up)
		{
			if(getItemCount() != 0) 
			{
				int step = 0;
				int prev_selected = selected;

				step = (msg == CRCInput::RC_page_up) ? listmaxshow : 1;  // browse or step 1
				selected -= step;
				if((prev_selected - step) < 0)            // because of uint
					selected = getItemCount() - 1;

				paintItem(prev_selected - liststart);

				unsigned int oldliststart = liststart;
				liststart = (selected/listmaxshow)*listmaxshow;

				if(oldliststart != liststart)
					paint();
				else
					paintItem(selected - liststart);
			}
		}
		else if (msg_repeatok == CRCInput::RC_down || msg == CRCInput::RC_page_down)
		{
			if(getItemCount() != 0) 
			{
				unsigned int step = 0;
				int prev_selected = selected;

				step = (msg == CRCInput::RC_page_down) ? listmaxshow : 1;  // browse or step 1
				selected += step;

				if(selected >= getItemCount()) 
				{
					if (((getItemCount() / listmaxshow) + 1) * listmaxshow == getItemCount() + listmaxshow) // last page has full entries
						selected = 0;
					else
						selected = ((step == listmaxshow) && (selected < (((getItemCount() / listmaxshow) + 1) * listmaxshow))) ? (getItemCount() - 1) : 0;
				}

				paintItem(prev_selected - liststart);

				unsigned int oldliststart = liststart;
				liststart = (selected/listmaxshow)*listmaxshow;

				if(oldliststart != liststart)
					paint();
				else
					paintItem(selected - liststart);
			}
		}
		else if ( (msg == NeutrinoMessages::EVT_TIMER) && (data == sec_timer_id) )
		{
			paintHead();
			//paint();
			//paintFoot();
		}
		else
		{
			CNeutrinoApp::getInstance()->handleMsg( msg, data );
			// kein canceling...
		}

		frameBuffer->blit();	
	}

	hide();
	
	g_RCInput->killTimer(sec_timer_id);
	sec_timer_id = 0;
	
	return res;
}


