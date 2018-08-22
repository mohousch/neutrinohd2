/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: bouqueteditor_chanselect.cpp 2018/08/21 mohousch Exp $

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
#include <driver/screen_max.h>

#include <gui/widget/icons.h>
#include <gui/widget/items2detailsline.h>
#include <gui/widget/buttons.h>

//
#include <client/zapitclient.h>
#include <channel.h>
#include <bouquets.h>
#include <satconfig.h>
#include <getservices.h>

#include <system/debug.h>
#include <system/helpers.h>

#include <gui/bedit/bouqueteditor_chanselect.h>


extern tallchans allchans;			// defined in zapit.cpp
extern CBouquetManager* g_bouquetManager;	// defined in zapit.cpp
void addChannelToBouquet(const unsigned int bouquet, const t_channel_id channel_id); // defined in zapit.cpp

CBEChannelSelectWidget::CBEChannelSelectWidget(const std::string& Caption, unsigned int Bouquet, CZapitClient::channelsMode Mode)
{
	frameBuffer = CFrameBuffer::getInstance();

	//
	selected =  0;

	caption = Caption;

	modified = false;

	//	
	bouquet = Bouquet;
	mode = Mode;

	// box	
	cFrameBox.iWidth = w_max ( (frameBuffer->getScreenWidth() / 20 * 17), (frameBuffer->getScreenWidth() / 20 ));
	cFrameBox.iHeight = h_max ( (frameBuffer->getScreenHeight() / 20 * 20), (frameBuffer->getScreenHeight() / 20));
	
	cFrameBox.iX = frameBuffer->getScreenX() + (frameBuffer->getScreenWidth() - cFrameBox.iWidth) / 2;
	cFrameBox.iY = frameBuffer->getScreenY() + (frameBuffer->getScreenHeight() - cFrameBox.iHeight) / 2;

	listBox = NULL;
	item = NULL;

	listBox = new ClistBoxEntry(&cFrameBox);
}

bool CBEChannelSelectWidget::isChannelInBouquet(int index)
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

#define BUTTONS_COUNT 2
const struct button_label Buttons[BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_OKAY, LOCALE_BOUQUETEDITOR_SWITCH, NULL },
	{ NEUTRINO_ICON_BUTTON_HOME, LOCALE_BOUQUETEDITOR_RETURN, NULL }	
};

void CBEChannelSelectWidget::paint()
{
	dprintf(DEBUG_DEBUG, "CBEChannelSelectWidget::paint\n");

	for (unsigned int count = 0; count < Channels.size(); count++)
	{
		item = new ClistBoxEntryItem(Channels[count]->getName().c_str(), true);

		// marked
		std::string marked_icon = "";
		if(isChannelInBouquet(count))
			marked_icon = NEUTRINO_ICON_BUTTON_MARK;

		item->setIconName(marked_icon.c_str());

		// scrambled
		std::string scrambled_icon = "";
		if(Channels[count]->scrambled)
			scrambled_icon = NEUTRINO_ICON_SCRAMBLED;

		item->setIcon1(scrambled_icon.c_str());

		// hd/uhd
		std::string hd_icon = "";
		if(Channels[count]->isHD())
			hd_icon = NEUTRINO_ICON_HD;
		else if(Channels[count]->isUHD()) 
			hd_icon = NEUTRINO_ICON_UHD,

		item->setIcon2(hd_icon.c_str()); 

		// info1
		char buf[128];
		transponder_id_t ct = Channels[count]->getTransponderId();
		transponder_list_t::iterator tpI = transponders.find(ct);
		int len = snprintf(buf, sizeof(buf), "%d ", Channels[count]->getFreqId());

		// satname
		sat_iterator_t sit = satellitePositions.find(Channels[count]->getSatellitePosition());
		
		if(sit != satellitePositions.end()) 
		{
			snprintf(&buf[len], sizeof(buf) - len, "(%s)\n", sit->second.name.c_str());
		}

		item->setInfo1(buf);

		listBox->addItem(item);
	}

	listBox->setTitle(caption.c_str());
	listBox->enablePaintHead();
	listBox->enablePaintDate();
	listBox->enablePaintFoot();
	listBox->setFooterButtons(Buttons, BUTTONS_COUNT);
	listBox->enablePaintFootInfo();

	//
	listBox->setSelected(selected);
	listBox->paint();
}

void CBEChannelSelectWidget::hide()
{
	listBox->hide();
}

int CBEChannelSelectWidget::exec(CMenuTarget* parent, const std::string& actionKey)
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

	paint();
	frameBuffer->blit();

	bool loop = true;
	modified = false;

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);

	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_EPG]);
	
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_EPG]);

		if( msg == CRCInput::RC_ok)
		{
			selected = listBox->getSelected();

			setModified();
	
			if (isChannelInBouquet(selected))
				g_bouquetManager->Bouquets[bouquet]->removeService(Channels[selected]->channel_id);
			else
				addChannelToBouquet(bouquet, Channels[selected]->channel_id);

			bouquetChannels = mode == CZapitClient::MODE_TV ? &(g_bouquetManager->Bouquets[bouquet]->tvChannels) : &(g_bouquetManager->Bouquets[bouquet]->radioChannels);
	
			listBox->clearItems();
			paint();
			g_RCInput->postMsg(CRCInput::RC_down, 0);
		}
		else if (msg == CRCInput::RC_home)
		{
			loop = false;
		}
		else if (msg == CRCInput::RC_page_up)
		{
			listBox->scrollPageUp();
		}
		else if (msg == CRCInput::RC_page_down)
		{
			listBox->scrollPageDown();
		}
		else if (msg == CRCInput::RC_up)
		{
			listBox->scrollLineUp();
		}
		else if (msg == CRCInput::RC_down)
		{
			listBox->scrollLineDown();
		}
		else if ( (msg == NeutrinoMessages::EVT_TIMER) && (data == sec_timer_id) )
		{
			listBox->paintHead();
		}
		else if (CNeutrinoApp::getInstance()->handleMsg(msg, data) & messages_return::cancel_all)
		{
			loop = false;
			res = menu_return::RETURN_EXIT_ALL;
		}

		frameBuffer->blit();	
	}

	hide();
	
	g_RCInput->killTimer(sec_timer_id);
	sec_timer_id = 0;

	delete listBox;
	listBox = NULL;
	
	return res;
}


