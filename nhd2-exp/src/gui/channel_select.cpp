/*
	* $Id: channel_select.cpp 2015/07/26 11:23:30 mohousch Exp $
	
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

/*zapit includes*/
#include <bouquets.h>

#include "gui/channel_select.h"

#include <global.h>
#include <neutrino.h>

#include <gui/bouquetlist.h>

#include <system/debug.h>


extern CBouquetList * bouquetList;

//select menu
CSelectChannelWidget::CSelectChannelWidget()
{
	ChannelTVID = g_settings.startchanneltv_id;
	ChannelRadioID = g_settings.startchannelradio_id;
}

CSelectChannelWidget::~CSelectChannelWidget()
{
	ChannelTVID = -1;
	ChannelRadioID = -1;
}

int CSelectChannelWidget::exec(CMenuTarget *parent, const std::string &actionKey)
{
	int   res = menu_return::RETURN_REPAINT;
	
	dprintf(DEBUG_NORMAL, "CSelectChannelWidget::exec: actionKey:%s\n", actionKey.c_str());

	if (parent)
		parent->hide();

	if(actionKey == "tv")
	{
		InitZapitChannelHelper(CZapitClient::MODE_TV);
		return res;
	}
	else if(actionKey == "radio")
	{
		InitZapitChannelHelper(CZapitClient::MODE_RADIO);
		return res;
	}

	return res;
}

void CSelectChannelWidget::InitZapitChannelHelper(CZapitClient::channelsMode mode)
{
	dprintf(DEBUG_NORMAL, "CSelectChannelWidget::InitZapitChannelHelper:\n");

	// save channel mode
	int channelMode = g_settings.channel_mode;
	int nNewChannel = -1;
	int activBouquet = 0;
	int activChannel = 0;
	int nMode = CNeutrinoApp::getInstance()->getMode();
	
_repeat:
	// set channel mode (allways list mode all)
	if(mode == CZapitClient::MODE_TV)
		CNeutrinoApp::getInstance()->SetChannelMode(LIST_MODE_ALL, NeutrinoMessages::mode_tv);
	else if(mode == CZapitClient::MODE_RADIO)
		CNeutrinoApp::getInstance()->SetChannelMode(LIST_MODE_ALL, NeutrinoMessages::mode_radio);
	
	// get activ channel number
	if(bouquetList->Bouquets.size())
	{ 
		activBouquet = bouquetList->getActiveBouquetNumber();
		activChannel = bouquetList->Bouquets[activBouquet]->channelList->getActiveChannelNumber();
	}

	// show channel list
	if(bouquetList->Bouquets.size() && bouquetList->Bouquets[activBouquet]->channelList->getSize() > 0)
		nNewChannel = bouquetList->Bouquets[activBouquet]->channelList->show();
	else
		nNewChannel = bouquetList->show();

	// handle list mode changed
	if(nNewChannel == -3) // channel mode changed
	{ 
		goto _repeat;
	}

	// get our channel
	if (nNewChannel == -1)
	{
		if(mode == CZapitClient::MODE_TV)
			ChannelTVID = g_settings.startchanneltv_id;
		else if(mode == CZapitClient::MODE_RADIO)
			ChannelRadioID = g_settings.startchannelradio_id;
	}
	else
	{
		if(mode == CZapitClient::MODE_TV)
			ChannelTVID = bouquetList->Bouquets[activBouquet]->channelList->getActiveChannel_ChannelID();
		else if(mode == CZapitClient::MODE_RADIO)
			ChannelRadioID = bouquetList->Bouquets[activBouquet]->channelList->getActiveChannel_ChannelID();
	}
	
	// set last channel mode
	CNeutrinoApp::getInstance()->SetChannelMode(channelMode, nMode);

	// set last activ bouquet and channel
	if(bouquetList->Bouquets.size()) 
	{
		bouquetList->activateBouquet(activBouquet, false);
		bouquetList->Bouquets[activBouquet]->channelList->setSelected(activChannel - 1);
	}
}
