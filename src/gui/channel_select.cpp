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


t_channel_id CSelectChannelWidget_TVChanID;
std::string CSelectChannelWidget_TVChanName;
t_channel_id CSelectChannelWidget_RadioChanID;
std::string CSelectChannelWidget_RadioChanName;

extern CBouquetList * bouquetList;

//select menu
CSelectChannelWidget::CSelectChannelWidget()
{
	CSelectChannelWidget_TVChanID = 0;
	CSelectChannelWidget_TVChanName.clear();
	
	CSelectChannelWidget_RadioChanID = 0;
	CSelectChannelWidget_RadioChanName.clear();
}

CSelectChannelWidget::~CSelectChannelWidget()
{
	CSelectChannelWidget_TVChanID = 0;
	CSelectChannelWidget_TVChanName.clear();
	
	CSelectChannelWidget_RadioChanID = 0;
	CSelectChannelWidget_RadioChanName.clear();
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
	
_repeat:
	// set channel mode
	if(mode == CZapitClient::MODE_TV)
		CNeutrinoApp::getInstance()->SetChannelMode(channelMode, NeutrinoMessages::mode_tv);
	else if(mode == CZapitClient::MODE_RADIO)
		CNeutrinoApp::getInstance()->SetChannelMode(channelMode, NeutrinoMessages::mode_radio);
	
	// get activ bouquet number
	activBouquet = bouquetList->getActiveBouquetNumber();
	
	// get activ channel number
	if(bouquetList->Bouquets.size()) 
		activChannel = bouquetList->Bouquets[activBouquet]->channelList->getActiveChannelNumber();

	// show channel list
	if(bouquetList->Bouquets[activBouquet]->channelList->getSize() > 0)
		nNewChannel = bouquetList->Bouquets[activBouquet]->channelList->show();
	else
		nNewChannel = bouquetList->show();

	if(nNewChannel == -3) // channel mode changed
	{ 
		//bouquetList->show();
		nNewChannel = bouquetList->exec(true);
		goto _repeat;
	}	
	else if (nNewChannel > -1)
	{
		if(mode == CZapitClient::MODE_TV)
		{
			CSelectChannelWidget_TVChanID = bouquetList->Bouquets[activBouquet]->channelList->getActiveChannel_ChannelID();
			CSelectChannelWidget_TVChanName = g_Zapit->getChannelName(CSelectChannelWidget_TVChanID);
		}
		else if(mode == CZapitClient::MODE_RADIO)
		{
			CSelectChannelWidget_RadioChanID = bouquetList->Bouquets[activBouquet]->channelList->getActiveChannel_ChannelID();
			CSelectChannelWidget_RadioChanName = g_Zapit->getChannelName(CSelectChannelWidget_RadioChanID);
		}
	}
	
	// set channel mode
	if(CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_tv)
	{
		CNeutrinoApp::getInstance()->SetChannelMode(channelMode, NeutrinoMessages::mode_tv);
	}
	else if(CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_radio)
	{
		CNeutrinoApp::getInstance()->SetChannelMode(channelMode, NeutrinoMessages::mode_radio);
	}
	
	bouquetList->activateBouquet(activBouquet, false);
				
	if(bouquetList->Bouquets.size())
		bouquetList->Bouquets[activBouquet]->channelList->setSelected(activChannel - 1);
}
