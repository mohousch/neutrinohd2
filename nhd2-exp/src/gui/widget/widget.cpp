/*
	$Id: widget.cpp 12.03.2020 mohousch Exp $


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

#include <unistd.h> //acces
#include <cctype>

#include <global.h>
#include <neutrino.h>

#include <driver/rcinput.h>

#include <gui/widget/widget.h>

#include <system/debug.h>


CWidget::CWidget(const int x, const int y, const int dx, const int dy)
{
	frameBuffer = CFrameBuffer::getInstance();

	mainFrameBox.iX = x;
	mainFrameBox.iY = y;
	mainFrameBox.iWidth = dx;
	mainFrameBox.iHeight = dy;

	savescreen = false;
	background = NULL;

	enableCenter = false;
	paintMainFrame = false;

	timeout = 0;
	selected = -1;

	backgroundColor = COL_MENUCONTENT_PLUS_0;

	actionKey = "";
}

CWidget::CWidget(CBox *position)
{
	frameBuffer = CFrameBuffer::getInstance();

	mainFrameBox = *position;

	savescreen = false;
	background = NULL;

	enableCenter = false;
	paintMainFrame = false;

	timeout = 0;
	selected = -1;

	backgroundColor = COL_MENUCONTENT_PLUS_0;

	actionKey = "";
}

CWidget::~CWidget()
{
	dprintf(DEBUG_NORMAL, "CWidget:: del\n");

	items.clear();
}

void CWidget::addItem(CWidgetItem *widgetItem, const int x, const int y, const int dx, const int dy, const bool defaultselected)
{
	if (defaultselected)
		selected = items.size();
	
	items.push_back(widgetItem);
}

bool CWidget::hasItem()
{
	return !items.empty();
}

void CWidget::initFrames()
{
	// sanity check
	if(mainFrameBox.iHeight > ((int)frameBuffer->getScreenHeight()))
		mainFrameBox.iHeight = frameBuffer->getScreenHeight();

	// sanity check
	if(mainFrameBox.iWidth > (int)frameBuffer->getScreenWidth())
		mainFrameBox.iWidth = frameBuffer->getScreenWidth();

	if(enableCenter)
	{
		mainFrameBox.iX = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - mainFrameBox.iWidth) >> 1 );
		mainFrameBox.iY = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - mainFrameBox.iHeight) >> 1 );
	}

	if(savescreen) 
		saveScreen();
}

void CWidget::paintItems()
{
	dprintf(DEBUG_NORMAL, "CWidget:: paintItems\n");

	if(hasItem())
	{
		for (unsigned int i = 0; i < items.size(); i++)
		{
			if( (items[i]->isSelectable()) && (selected == -1)) 
			{
				selected = i;
			}

			items[i]->paint();
		}
	}
}

void CWidget::paint()
{
	dprintf(DEBUG_NORMAL, "CWidget:: paint\n");

	// paint mainFrame
	if(paintMainFrame)
		frameBuffer->paintBoxRel(mainFrameBox.iX, mainFrameBox.iY, mainFrameBox.iWidth, mainFrameBox.iHeight, backgroundColor, RADIUS_MID, CORNER_ALL, nogradient);

	// paint items
	paintItems();
}

void CWidget::saveScreen()
{
	if(!savescreen)
		return;

	if(background)
	{
		delete[] background;
		background = NULL;
	}

	background = new fb_pixel_t[mainFrameBox.iWidth*mainFrameBox.iHeight];
	
	if(background)
	{
		frameBuffer->saveScreen(mainFrameBox.iX, mainFrameBox.iY, mainFrameBox.iWidth, mainFrameBox.iHeight, background);
	}
}

void CWidget::restoreScreen()
{
	if(background) 
	{
		if(savescreen)
			frameBuffer->restoreScreen(mainFrameBox.iX, mainFrameBox.iY, mainFrameBox.iWidth, mainFrameBox.iHeight, background);
	}
}

void CWidget::enableSaveScreen()
{
	savescreen = true;
	
	if(!savescreen && background) 
	{
		delete[] background;
		background = NULL;
	}
}

void CWidget::hide()
{
	dprintf(DEBUG_NORMAL, "CWidget:: hide\n");

	if (hasItem())
	{
		for(unsigned int i = 0; i < items.size(); i++)
		{
			items[i]->hide();
		}
	}

	if( savescreen && background)
	{
		restoreScreen();
	}
	else
	{
		if(paintMainFrame)
			frameBuffer->paintBackgroundBoxRel(mainFrameBox.iX, mainFrameBox.iY, mainFrameBox.iWidth, mainFrameBox.iHeight);
	}

	frameBuffer->blit();
}

void CWidget::addKey(neutrino_msg_t key, CMenuTarget *menue, const std::string & action)
{
	keyActionMap[key].menue = menue;
	keyActionMap[key].action = action;
}

int CWidget::exec(CMenuTarget *parent, const std::string &)
{
	dprintf(DEBUG_NORMAL, "CWidget:: exec:\n");

	retval = RETURN_REPAINT;
	pos = 0;
	exit_pressed = false;

	if (parent)
		parent->hide();

	initFrames();

	// set in focus
	if (hasItem() && items.size() > 1)
	{
		for (unsigned int i = 0; i < items.size(); i++)
		{
			if((items[i]->isSelectable()) && (items[i]->inFocus == true))
			{
				selected = i;
				break;
			}
		}
	}
	else
		selected = -1;

	paint();

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);
	
	uint64_t timeoutEnd = CRCInput::calcTimeoutEnd(timeout == 0 ? 0xFFFF : timeout);

	//control loop
	do {
		g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd);
		
		int handled = false;

		dprintf(DEBUG_DEBUG, "CWidget::exec: msg:%s\n", CRCInput::getSpecialKeyName(msg));

		if ( msg <= RC_MaxRC ) 
		{
			timeoutEnd = CRCInput::calcTimeoutEnd(timeout == 0 ? 0xFFFF : timeout);

			// keymap
			std::map<neutrino_msg_t, keyAction>::iterator it = keyActionMap.find(msg);
			
			if (it != keyActionMap.end()) 
			{
				actionKey = it->second.action;

				if (it->second.menue != NULL)
				{
					int rv = it->second.menue->exec(this, it->second.action);

					//FIXME:review this
					switch ( rv ) 
					{
						case RETURN_EXIT_ALL:
							retval = RETURN_EXIT_ALL; //fall through
						case RETURN_EXIT:
							msg = RC_timeout;
							break;
						case RETURN_REPAINT:
							hide();
							initFrames();
							paint();
							break;
					}
				}
				else
				{
					//actionKey = it->second.action;
					selected = -1;
					handled = true;

					break;
				}

				frameBuffer->blit();
				continue;
			}
		}

		if (!handled) 
		{
			if ( (msg == NeutrinoMessages::EVT_TIMER) && (data == sec_timer_id) )
			{
				// update time
				for (unsigned int i = 0; i < items.size(); i++)
				{
					if( (items[i]->itemType == WIDGET_ITEM_HEAD) /*&& (items[i]->paintDate)*/)
					{
						items[i]->paint();
						break;
					}
				}
			} 

			switch (msg) 
			{
				case (NeutrinoMessages::EVT_TIMER):
					if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all ) 
					{
						retval = RETURN_EXIT_ALL;
						msg = RC_timeout;
					}
					break;

				//
				case (RC_up):
					onUpKeyPressed();
					break;

				case (RC_down):
					onDownKeyPressed();
					break;

				case (RC_right):
					onRightKeyPressed();
					break;

				case (RC_left):
					onLeftKeyPressed();
					break;

				case (RC_page_up):
					onPageUpKeyPressed();
					break;

				case (RC_page_down):
					onPageDownKeyPressed();
					break;

				case (RC_yellow):
					onYellowKeyPressed();
					break;

				case (RC_home):
					onHomeKeyPressed();
					break;

				case (RC_ok):
					onOKKeyPressed();
					break;

				// colored keys
				case (RC_red):
					onRedKeyPressed();
					break;

				case (RC_green):
					onGreenKeyPressed();
					break;

				case (RC_blue):
					onBlueKeyPressed();
					break;

				case (RC_setup):
					onMenuKeyPressed();
					break;

				case (RC_standby):
					onPowerKeyPressed();
					break;

				case (RC_spkr):
					onMuteKeyPressed();
					break;

				case (RC_plus):
					onVolumeUpKeyPressed();
					break;

				case (RC_minus):
					onVolumeDownKeyPressed();
					break;

				case (RC_0):
					on0KeyPressed();
					break;

				case (RC_1):
					on1KeyPressed();
					break;

				case (RC_2):
					on2KeyPressed();
					break;

				case (RC_3):
					on3KeyPressed();
					break;

				case (RC_4):
					on4KeyPressed();
					break;

				case (RC_5):
					on5KeyPressed();
					break;

				case (RC_6):
					on6KeyPressed();
					break;

				case (RC_7):
					on7KeyPressed();
					break;

				case (RC_8):
					on8KeyPressed();
					break;

				case (RC_9):
					on9KeyPressed();
					break;

				case (RC_audio):
					onAudioKeyPressed();
					break;

				case (RC_text):
					onTextKeyPressed();
					break;

				case (RC_info):
					onInfoKeyPressed();
					break;

				case (RC_epg):
					onEPGKeyPressed();
					break;

				case (RC_recall):
					onBackKeyPressed();
					break;

				case (RC_favorites):
					onFavoritesKeyPressed();
					break;

				case (RC_sat):
					onSatKeyPressed();
					break;

				case (RC_record):
					onRecordKeyPressed();
					break;

				case (RC_play):
					onPlayKeyPressed();
					break;

				case (RC_pause):
					onPauseKeyPressed();
					break;

				case (RC_forward):
					onFastForwardKeyPressed();
					break;

				case (RC_rewind):
					onRewindKeyPressed();
					break;

				case (RC_stop):
					onStopKeyPressed();
					break;

				case (RC_timeshift):
					onTimeshiftKeyPressed();
					break;

				case (RC_mode):
					onModeKeyPressed();
					break;

				case (RC_next):
					onNextKeyPressed();
					break;

				case (RC_prev):
					onPrevKeyPressed();
					break;

				case (RC_music):
					onMusicKeyPressed();
					break;

				case (RC_picture):
					onPictureKeyPressed();
					break;

				case (RC_loop):
					onLoopKeyPressed();
					break;

				case (RC_slow):
					onSlowKeyPressed();
					break;

				case (RC_dvbsub):
					onDVBSubKeyPressed();
					break;

				case (RC_pip):
					onPIPKeyPressed();
					break;

				case (RC_pippos):
					onPIPPosKeyPressed();
					break;

				case (RC_pipswap):
					onPIPSwapKeyPressed();
					break;

				case (RC_pipsubch):
					onPIPSubChanKeyPressed();
					break;

				case (RC_net):
					onNetKeyPressed();
					break;

				case (RC_bookmark):
					onBookmarkKeyPressed();
					break;

				case (RC_multifeed):
					onMultifeedKeyPressed();
					break;

				case (RC_f1):
					onF1KeyPressed();
					break;

				case (RC_f2):
					onF2KeyPressed();
					break;

				case (RC_f3):
					onF3KeyPressed();
					break;

				case (RC_f4):
					onF4KeyPressed();
					break;
					
				case (RC_timeout):
					break;

				default:
					if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all ) 
					{
						retval = RETURN_EXIT_ALL;
						msg = RC_timeout;
					}
			}

			if ( msg <= RC_MaxRC )
			{
				// recalculate timeout for RC-Tasten
				timeoutEnd = CRCInput::calcTimeoutEnd(timeout == 0 ? 0xFFFF : timeout);
			}
		}
		
		frameBuffer->blit();
	}
	while ( msg != RC_timeout );

	dprintf(DEBUG_NORMAL, "CWidget: retval: (%d) selected:%d\n", retval, selected);
	
	hide();	

	//
	g_RCInput->killTimer(sec_timer_id);
	sec_timer_id = 0;	

	// vfd
	if(!parent)
	{
		if(CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_webtv)
			CVFD::getInstance()->setMode(CVFD::MODE_WEBTV);
		else
			CVFD::getInstance()->setMode(CVFD::MODE_TVRADIO);
	}
	
	return retval;
}

// events
void CWidget::onOKKeyPressed()
{
	if(hasItem() && selected >= 0)
	{
		if((items[selected]->itemType == WIDGET_ITEM_LISTBOX) || (items[selected]->itemType == WIDGET_ITEM_FRAMEBOX))
		{
			actionKey = items[selected]->getActionKey();

			int rv = items[selected]->oKKeyPressed(this);

			//FIXME:review this
			switch ( rv ) 
			{
				case RETURN_EXIT_ALL:
					retval = RETURN_EXIT_ALL; //fall through
				case RETURN_EXIT:
					msg = RC_timeout;
					break;
				case RETURN_REPAINT:
					hide();
					initFrames();
					paint();
					break;
			}
		}
	}
}

void CWidget::onHomeKeyPressed()
{
	exit_pressed = true;
	dprintf(DEBUG_NORMAL, "CWidget::exec: exit_pressed\n");
	msg = RC_timeout;
	selected = -1;
}

void CWidget::onYellowKeyPressed()
{
	if(hasItem())
	{
		for (unsigned int count = 1; count < items.size(); count++) 
		{
			pos = (selected + count)%items.size();

			CWidgetItem * item = items[pos];

			if(item->isSelectable() && item->hasItem())
			{
				items[selected]->setInFocus(false);

				selected = pos;

				item->setInFocus(true);

				paint();

				break;
			}
		}
	}
}

void CWidget::onUpKeyPressed()
{
	if(hasItem() && selected >= 0)
	{
		items[selected]->onUpKeyPressed();
	}
}

void CWidget::onDownKeyPressed()
{
	if(hasItem() && selected >= 0)
	{
		items[selected]->onDownKeyPressed();
	}
}

void CWidget::onRightKeyPressed()
{
	if(hasItem() && selected >= 0)
	{
		items[selected]->onRightKeyPressed();
	}
}

void CWidget::onLeftKeyPressed()
{
	if(hasItem() && selected >= 0)
	{
		items[selected]->onLeftKeyPressed();
	}
}

void CWidget::onPageUpKeyPressed()
{
	if(hasItem() && selected >= 0)
	{
		items[selected]->onPageUpKeyPressed();
	}
}

void CWidget::onPageDownKeyPressed()
{
	if(hasItem() && selected >= 0)
	{
		items[selected]->onPageDownKeyPressed();
	}
}



