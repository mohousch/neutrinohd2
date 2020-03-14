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


CWidget::CWidget()
{
	frameBuffer = CFrameBuffer::getInstance();

	mainFrameBox.iWidth = MENU_WIDTH;
	mainFrameBox.iHeight = MENU_HEIGHT;

	full_width = mainFrameBox.iWidth;
	full_height = mainFrameBox.iHeight;

	savescreen = false;
	background = NULL;

	enableCenter = true;

	mainFrameBox.iX = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - full_width ) >> 1 );
	mainFrameBox.iY = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - full_height) >> 1 );
}

CWidget::CWidget(const int x, const int y, const int dx, const int dy)
{
	frameBuffer = CFrameBuffer::getInstance();

	mainFrameBox.iX = x;
	mainFrameBox.iY = y;
	mainFrameBox.iWidth = dx;
	mainFrameBox.iHeight = dy;

	full_width = mainFrameBox.iWidth;
	full_height = mainFrameBox.iHeight;

	savescreen = false;
	background = NULL;

	enableCenter = true;
}

CWidget::CWidget(CBox *position)
{
	frameBuffer = CFrameBuffer::getInstance();

	mainFrameBox = *position;

	full_width = mainFrameBox.iWidth;
	full_height = mainFrameBox.iHeight;

	savescreen = false;
	background = NULL;

	enableCenter = true;
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
	if(enableCenter)
	{
		mainFrameBox.iX = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - full_width ) >> 1 );
		mainFrameBox.iY = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - full_height) >> 1 );
	}
}

void CWidget::paintItems()
{
	dprintf(DEBUG_NORMAL, "CWidget:: paintItems\n");

	for (unsigned int i = 0; i < items.size(); i++)
	{
		items[i]->paint();
	}
}

void CWidget::paint()
{
	dprintf(DEBUG_NORMAL, "CWidget:: paint\n");

	frameBuffer->paintBoxRel(mainFrameBox.iX, mainFrameBox.iY, mainFrameBox.iWidth, mainFrameBox.iHeight, COL_MENUCONTENT_PLUS_0);

	paintItems();
}


void CWidget::hide()
{
	dprintf(DEBUG_NORMAL, "CWidget:: hide\n");

	frameBuffer->paintBackgroundBoxRel(mainFrameBox.iX, mainFrameBox.iY, mainFrameBox.iWidth, mainFrameBox.iHeight);

	frameBuffer->blit();
}

void CWidget::addKey(neutrino_msg_t key, CMenuTarget *menue, const std::string & action)
{
	keyActionMap[key].menue = menue;
	keyActionMap[key].action = action;
}

int CWidget::exec(CMenuTarget *parent, const std::string &actionKey)
{
	dprintf(DEBUG_NORMAL, "CWidget:: exec\n");

	int pos = 0;
	exit_pressed = false;
	int cnt = 0;
	selected = 0;

	if (parent)
		parent->hide();

	initFrames();
	paint();

	//items[selected]->setOutFocus(false);

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);
	
	int retval = menu_return::RETURN_REPAINT;
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
				if (it->second.menue != NULL)
				{
					int rv = it->second.menue->exec(this, it->second.action);

					switch ( rv ) 
					{
						case menu_return::RETURN_EXIT_ALL:
							retval = menu_return::RETURN_EXIT_ALL;
						case menu_return::RETURN_EXIT:
							msg = RC_timeout;
							break;
						case menu_return::RETURN_REPAINT:
							initFrames();
							paint();
							break;
					}
				}
				else
				{
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
				//paintHead();
			} 

			switch (msg) 
			{
				case (NeutrinoMessages::EVT_TIMER):
					if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all ) 
					{
						retval = menu_return::RETURN_EXIT_ALL;
						msg = RC_timeout;
					}
					break;
					
				case (RC_page_up) :
					items[selected]->scrollPageUp();
					break;

				case (RC_page_down) :
					items[selected]->scrollPageDown();
					break;
					
				case (RC_up) :
					items[selected]->scrollLineUp();
					break;
					
				case (RC_down) :
					items[selected]->scrollLineDown();
					break;

				case (RC_left):
					items[selected]->swipLeft();
					break;
					
				case (RC_right):
					items[selected]->swipRight();
					break;

				#if 0
				case (RC_ok):
					{
						if(hasItem()) 
						{
							//exec this item...
							CWidgetItem* item = items[selected];

							item->msg = msg;
							
							int rv = item->exec(this);
							
							switch ( rv ) 
							{
								case menu_return::RETURN_EXIT_ALL:
									retval = menu_return::RETURN_EXIT_ALL;
									
								case menu_return::RETURN_EXIT:
									msg = RC_timeout;
									break;
									
								case menu_return::RETURN_REPAINT:
									hide();
									initFrames();
									paint();
									break;

								case menu_return::RETURN_NONE:
									g_RCInput->killTimer(sec_timer_id);
									sec_timer_id = 0;
									retval = menu_return::RETURN_NONE;
									msg = RC_timeout;
									break;	
							}
						} 
						else
							msg = RC_timeout;
					}
					break;
				#endif

				case (RC_yellow):
					items[selected]->setSelected(-1);
					items[selected]->setOutFocus(true);

					selected += 1;

					if(selected >= items.size())
						selected = 0;

					items[selected]->setSelected(0);
					items[selected]->setOutFocus(false);
					break;

				case (RC_home):
					exit_pressed = true;
					dprintf(DEBUG_NORMAL, "exit_pressed\n");
					msg = RC_timeout;
					break;
					
				case (RC_timeout):
					break;

				default:
					if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all ) 
					{
						retval = menu_return::RETURN_EXIT_ALL;
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
	
	if(retval != menu_return::RETURN_NONE)
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



