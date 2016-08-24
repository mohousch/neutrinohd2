/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: bouqueteditor_channels.cpp 2013/10/12 mohousch Exp $

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

#include <gui/widget/buttons.h>
#include <gui/widget/icons.h>
#include <gui/widget/messagebox.h>
#include <gui/widget/items2detailsline.h>

#include <gui/bedit/bouqueteditor_channels.h>

#include <global.h>
#include <neutrino.h>

#include <driver/fontrenderer.h>
#include <driver/screen_max.h>
#include <gui/bedit/bouqueteditor_chanselect.h>
#include <gui/widget/buttons.h>
#include <gui/widget/icons.h>

// zapit includes
#include <client/zapitclient.h>
#include <client/zapittools.h>
#include <bouquets.h>
#include <satconfig.h>
#include <getservices.h>

#include <system/debug.h>


extern tallchans allchans;
extern CBouquetManager * g_bouquetManager;

CBEChannelWidget::CBEChannelWidget(const std::string & Caption, unsigned int Bouquet)
{
	frameBuffer = CFrameBuffer::getInstance();
	selected = 0;
	
	// foot
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_RED, &icon_foot_w, &icon_foot_h);
	ButtonHeight = std::max(g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight(), icon_foot_h) + 10;

	// head
	theight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight();
	
	// item height
	frameBuffer->getIconSize(NEUTRINO_ICON_HD, &icon_w_hd, &icon_h_hd);
	frameBuffer->getIconSize(NEUTRINO_ICON_SCRAMBLED, &icon_w_s, &icon_h_s);
	
	fheight = std::max(g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), icon_h_s) + 2;

	liststart = 0;
	state = beDefault;
	caption = Caption;
	bouquet = Bouquet;
	mode = CZapitClient::MODE_TV;
}

void CBEChannelWidget::paintItem(int pos)
{
	uint8_t    color;
	fb_pixel_t bgcolor;
	int ypos = y + theight + pos*fheight;
	unsigned int current = liststart + pos;

	if(current == selected) 
	{
		color = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
		
		// itemlines	
		paintItem2DetailsLine(pos);		
		
		// details
		paintDetails(current);
	} 
	else 
	{
		color   = COL_MENUCONTENT;
		bgcolor = COL_MENUCONTENT_PLUS_0;
	}
	
	// itemBox
	frameBuffer->paintBoxRel(x, ypos, width - SCROLLBAR_WIDTH, fheight, bgcolor);

	if ((current == selected) && (state == beMoving))
	{
		int icon_w, icon_h;
		frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_YELLOW, &icon_w, &icon_h);
		frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_YELLOW, x + 8, ypos + (fheight - icon_h)/2);
	}
	
	if(current < Channels->size())
	{
		g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(x + 5 + numwidth + BORDER_LEFT, ypos + (fheight - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), width - numwidth - 20 - 15, (*Channels)[current]->getName(), color, 0, true);
	}
	
	//FIXME???
	// hd/scrambled icons
	if (g_settings.channellist_ca)
	{
		if(current < Channels->size())
		{
			// scrambled icon
			if((*Channels)[current]->scrambled) 
				frameBuffer->paintIcon(NEUTRINO_ICON_SCRAMBLED, x + width - (SCROLLBAR_WIDTH + 2 + icon_w_s), ypos + (fheight - icon_h_s)/2 );
			
			// hd icon
			if((*Channels)[current]->isHD()) 
				frameBuffer->paintIcon(NEUTRINO_ICON_HD, x + width - (SCROLLBAR_WIDTH + 2 + icon_w_s + 2 + icon_w_hd), ypos + (fheight - icon_h_hd)/2 );

			// uhd icon
			else if((*Channels)[current]->isUHD())
				frameBuffer->paintIcon(NEUTRINO_ICON_UHD, x + width - (SCROLLBAR_WIDTH + 2 + icon_w_s + 2 + icon_w_hd), ypos + (fheight - icon_h_hd)/2 );
		}
	}
}

void CBEChannelWidget::paint()
{
	dprintf(DEBUG_NORMAL, "CBEChannelWidget::paint:\n");

	liststart = (selected/listmaxshow)*listmaxshow;
	int lastnum =  liststart + listmaxshow;

	if(lastnum<10)
		numwidth = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth("0");
	else if(lastnum<100)
		numwidth = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth("00");
	else if(lastnum<1000)
		numwidth = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth("000");
	else if(lastnum<10000)
		numwidth = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth("0000");
	else // if(lastnum<100000)
		numwidth = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth("00000");

	for(unsigned int count = 0; count < listmaxshow; count++)
	{
		paintItem(count);
	}

	int ypos = y + theight;
	
	// scrollbar
	int sb = fheight*listmaxshow;
	frameBuffer->paintBoxRel(x + width - SCROLLBAR_WIDTH, ypos, SCROLLBAR_WIDTH, sb,  COL_MENUCONTENT_PLUS_1);

	int sbc= ((Channels->size() - 1)/ listmaxshow) + 1;
	float sbh = (sb - 4)/ sbc;
	int sbs = (selected/listmaxshow);

	frameBuffer->paintBoxRel(x + width - 13, ypos + 2 + int(sbs* sbh), 11, int(sbh), COL_MENUCONTENT_PLUS_3);
}

void CBEChannelWidget::paintHead()
{
	frameBuffer->paintBoxRel(x, y, width, theight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, true, gradientLight2Dark);
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(x + BORDER_LEFT, y + theight, width - BORDER_LEFT - BORDER_RIGHT, caption.c_str() , COL_MENUHEAD, 0, true);
}

const struct button_label CBEChannelWidgetButtons[4] =
{
	{ NEUTRINO_ICON_BUTTON_RED   , LOCALE_BOUQUETEDITOR_DELETE, NULL     },
	{ NEUTRINO_ICON_BUTTON_GREEN , LOCALE_BOUQUETEDITOR_ADD, NULL        },
	{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_BOUQUETEDITOR_MOVE, NULL       },
	{ NEUTRINO_ICON_BUTTON_BLUE  , LOCALE_BOUQUETEDITOR_SWITCHMODE, NULL }
};

void CBEChannelWidget::paintFoot()
{
	frameBuffer->paintBoxRel(x, y + height, width, ButtonHeight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_BOTTOM, true, gradientDark2Light);

	::paintButtons(frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, x + ICON_OFFSET, y + height, (width - 2*ICON_OFFSET) / 4, 4, CBEChannelWidgetButtons, ButtonHeight);
}

//
void CBEChannelWidget::paintDetails(int index)
{
	if(Channels->empty())
		return;
		
	char buf[128];
	transponder_id_t ct = (*Channels)[index]->getTransponderId();
	transponder_list_t::iterator tpI = transponders.find(ct);
	int len = snprintf(buf, sizeof(buf), "%d ", (*Channels)[index]->getFreqId());
	
	// infobox refresh
	frameBuffer->paintBoxRel(x + 2, y + height + ButtonHeight + 2, width - 4, info_height - 4, COL_MENUCONTENTDARK_PLUS_0, 0, 0, true, gradientLight2Dark);

	sat_iterator_t sit = satellitePositions.find((*Channels)[index]->getSatellitePosition());
		
	if(sit != satellitePositions.end()) 
	{
		//int satNameWidth = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getRenderWidth (sit->second.name);
		snprintf(&buf[len], sizeof(buf) - len, "(%s)\n", sit->second.name.c_str());
	}
	
	g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(x + 10, y + height + ButtonHeight + 5 + fheight, width - 30, buf, COL_MENUCONTENTDARK, 0, true);
}

void CBEChannelWidget::clearItem2DetailsLine()
{  
	::clearItem2DetailsLine(x, y, width, height + ButtonHeight, info_height);   
}

void CBEChannelWidget::paintItem2DetailsLine(int pos)
{
	::paintItem2DetailsLine(x, y, width, height + ButtonHeight, info_height, theight, fheight, pos);
}

void CBEChannelWidget::hide()
{
	frameBuffer->paintBackgroundBoxRel(x, y, width, height + ButtonHeight + 2 + info_height);
	
	clearItem2DetailsLine();
	
	frameBuffer->blit();
}

int CBEChannelWidget::exec(CMenuTarget* parent, const std::string &/*actionKey*/)
{
	dprintf(DEBUG_NORMAL, "CBEChannelWidget::exec:\n");

	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	int res = menu_return::RETURN_REPAINT;

	if (parent)
		parent->hide();

	width = w_max ( (frameBuffer->getScreenWidth() / 20 * 17), (frameBuffer->getScreenWidth() / 20 ));
	height = h_max ( (frameBuffer->getScreenHeight() / 20 * 16), (frameBuffer->getScreenHeight() / 20));
	
	listmaxshow = (height - theight)/fheight;
	height = theight + listmaxshow*fheight; // recalc height
	
	// info height
	info_height = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight() + 10;
	
	// coordinate
	x = frameBuffer->getScreenX() + (frameBuffer->getScreenWidth() - (width + ConnectLineBox_Width)) / 2 + ConnectLineBox_Width;
	y = frameBuffer->getScreenY() + (frameBuffer->getScreenHeight() - height -ButtonHeight - 2 - info_height) / 2;

	Channels = mode == CZapitClient::MODE_TV ? &(g_bouquetManager->Bouquets[bouquet]->tvChannels) : &(g_bouquetManager->Bouquets[bouquet]->radioChannels);
	
	paintHead();
	paint();
	paintFoot();
	
	frameBuffer->blit();	

	channelsChanged = false;

	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_EPG]);

	bool loop = true;
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_EPG]);

		if ((msg == CRCInput::RC_timeout) || (msg == (neutrino_msg_t)g_settings.key_channelList_cancel))
		{
			if (state == beDefault)
			{
				loop = false;
			}
			else if (state == beMoving)
			{
				cancelMoveChannel();
			}
		}
		else if (msg==CRCInput::RC_up || msg==(neutrino_msg_t)g_settings.key_channelList_pageup)
		{
			if (!(Channels->empty()))
			{
				int step = 0;
				int prev_selected = selected;

				step = (msg == (neutrino_msg_t)g_settings.key_channelList_pageup) ? listmaxshow : 1;  // browse or step 1
				selected -= step;
				if((prev_selected-step) < 0)		// because of uint
				{
					selected = Channels->size() - 1;
				}

				if (state == beDefault)
				{
					paintItem(prev_selected - liststart);
					unsigned int oldliststart = liststart;
					liststart = (selected/listmaxshow)*listmaxshow;
					
					if(oldliststart!=liststart)
					{
						paint();
					}
					else
					{
						paintItem(selected - liststart);
					}
				}
				else if (state == beMoving)
				{
					internalMoveChannel(prev_selected, selected);
				}
			}
		}
		else if (msg == CRCInput::RC_down || msg == (neutrino_msg_t)g_settings.key_channelList_pagedown)
		{
			unsigned int step = 0;
			int prev_selected = selected;

			step = (msg == (neutrino_msg_t)g_settings.key_channelList_pagedown) ? listmaxshow : 1;  // browse or step 1
			selected += step;

			if(selected >= Channels->size())
			{
				if (((Channels->size() / listmaxshow) + 1)*listmaxshow == Channels->size() + listmaxshow) // last page has full entries
					selected = 0;
				else
					selected = ((step == listmaxshow) && (selected < (((Channels->size() / listmaxshow) + 1)*listmaxshow))) ? (Channels->size() - 1) : 0;
			}

			if (state == beDefault)
			{
				paintItem(prev_selected - liststart);
				unsigned int oldliststart = liststart;
				liststart = (selected/listmaxshow)*listmaxshow;
				
				if(oldliststart!=liststart)
				{
					paint();
				}
				else
				{
					paintItem(selected - liststart);
				}
			}
			else if (state == beMoving)
			{
				internalMoveChannel(prev_selected, selected);
			}
		}
		else if(msg == CRCInput::RC_red)
		{
			if (state == beDefault)
				deleteChannel();
		}
		else if(msg == CRCInput::RC_green)
		{
			if (state == beDefault)
				addChannel();
		}
		else if(msg == CRCInput::RC_blue)
		{
			if (state == beDefault)
			{
				if (mode == CZapitClient::MODE_TV)
					mode = CZapitClient::MODE_RADIO;
				else
					mode = CZapitClient::MODE_TV;

				Channels = mode == CZapitClient::MODE_TV ? &(g_bouquetManager->Bouquets[bouquet]->tvChannels) : &(g_bouquetManager->Bouquets[bouquet]->radioChannels);

				selected = 0;
				paint();
			}
		}
		else if(msg == CRCInput::RC_yellow)
		{
			liststart = (selected/listmaxshow)*listmaxshow;
			if (state == beDefault)
				beginMoveChannel();
			paintItem(selected - liststart);
		}
		else if(msg == CRCInput::RC_ok)
		{
			if (state == beDefault)
			{
				if (selected < Channels->size()) /* Channels.size() might be 0 */
					g_Zapit->zapTo_serviceID((*Channels)[selected]->channel_id);

			} 
			else if (state == beMoving) 
			{
				finishMoveChannel();
			}
		}
		else if( CRCInput::isNumeric(msg) )
		{
			if (state == beDefault)
			{
				//kein pushback - wenn man versehentlich wo draufkommt is die edit-arbeit umsonst
				//selected = oldselected;
				//g_RCInput->postMsg( msg, data );
				//loop=false;
			}
			else if (state == beMoving)
			{
				cancelMoveChannel();
			}
		}
		else
		{
			CNeutrinoApp::getInstance()->handleMsg( msg, data );
		}

		frameBuffer->blit();	
	}
	
	hide();
	
	return res;
}

void CBEChannelWidget::deleteChannel()
{
	if (selected >= Channels->size()) /* Channels.size() might be 0 */
		return;

	if (MessageBox(LOCALE_FILEBROWSER_DELETE, (*Channels)[selected]->getName(), CMessageBox::mbrNo, CMessageBox::mbYes|CMessageBox::mbNo)!=CMessageBox::mbrYes)
		return;

	g_bouquetManager->Bouquets[bouquet]->removeService((*Channels)[selected]->channel_id);

	Channels = mode == CZapitClient::MODE_TV ? &(g_bouquetManager->Bouquets[bouquet]->tvChannels) : &(g_bouquetManager->Bouquets[bouquet]->radioChannels);

	if (selected >= Channels->size())
		selected = Channels->empty() ? 0 : (Channels->size() - 1);
	
	channelsChanged = true;
	paint();
}

void CBEChannelWidget::addChannel()
{
	CBEChannelSelectWidget * channelSelectWidget = new CBEChannelSelectWidget(caption, bouquet, mode);

	channelSelectWidget->exec(this, "");
	
	if (channelSelectWidget->hasChanged())
	{
		channelsChanged = true;
		Channels = mode == CZapitClient::MODE_TV ? &(g_bouquetManager->Bouquets[bouquet]->tvChannels) : &(g_bouquetManager->Bouquets[bouquet]->radioChannels);
	}
	
	delete channelSelectWidget;
	
	paintHead();
	paint();
	paintFoot();
}

void CBEChannelWidget::beginMoveChannel()
{
	state = beMoving;
	origPosition = selected;
	newPosition = selected;
}

void CBEChannelWidget::finishMoveChannel()
{
	state = beDefault;
	paint();
}

void CBEChannelWidget::cancelMoveChannel()
{
	state = beDefault;
	internalMoveChannel( newPosition, origPosition);
	channelsChanged = false;
}

void CBEChannelWidget::internalMoveChannel( unsigned int fromPosition, unsigned int toPosition)
{
	if ( (int) toPosition == -1 ) 
		return;
	
	if ( toPosition == Channels->size()) 
		return;

	g_bouquetManager->Bouquets[bouquet]->moveService(fromPosition, toPosition, mode == CZapitClient::MODE_TV ? 1 : 2);

	channelsChanged = true;
	Channels = mode == CZapitClient::MODE_TV ? &(g_bouquetManager->Bouquets[bouquet]->tvChannels) : &(g_bouquetManager->Bouquets[bouquet]->radioChannels);

	selected = toPosition;
	newPosition = toPosition;
	paint();
}

bool CBEChannelWidget::hasChanged()
{
	return (channelsChanged);
}
