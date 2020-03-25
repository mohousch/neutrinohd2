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

	savescreen = false;
	background = NULL;

	enableCenter = true;

	mainFrameBox.iX = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - mainFrameBox.iWidth ) >> 1 );
	mainFrameBox.iY = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - mainFrameBox.iHeight) >> 1 );

	timeout = 0;
	selected = 0;

	backgroundColor = COL_MENUCONTENT_PLUS_0;
}

CWidget::CWidget(const int x, const int y, const int dx, const int dy)
{
	frameBuffer = CFrameBuffer::getInstance();

	mainFrameBox.iX = x;
	mainFrameBox.iY = y;
	mainFrameBox.iWidth = dx;
	mainFrameBox.iHeight = dy;

	savescreen = false;
	background = NULL;

	enableCenter = true;

	timeout = 0;
	selected = 0;

	backgroundColor = COL_MENUCONTENT_PLUS_0;
}

CWidget::CWidget(CBox *position)
{
	frameBuffer = CFrameBuffer::getInstance();

	mainFrameBox = *position;

	savescreen = false;
	background = NULL;

	enableCenter = true;

	timeout = 0;
	selected = 0;

	backgroundColor = COL_MENUCONTENT_PLUS_0;
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
		mainFrameBox.iX = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - mainFrameBox.iWidth) >> 1 );
		mainFrameBox.iY = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - mainFrameBox.iHeight) >> 1 );
	}

	if(savescreen) 
		saveScreen();
}

void CWidget::paintItems()
{
	dprintf(DEBUG_NORMAL, "CWidget:: paintItems\n");

	for (unsigned int i = 0; i < items.size(); i++)
	{
		if( (items[i]->isSelectable()) && (selected == -1)) 
		{
			selected = i;
		}

		items[i]->paint();
	}
}

void CWidget::paint()
{
	dprintf(DEBUG_NORMAL, "CWidget:: paint\n");

	frameBuffer->paintBoxRel(mainFrameBox.iX, mainFrameBox.iY, mainFrameBox.iWidth, mainFrameBox.iHeight, backgroundColor);

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

	if( savescreen && background)
		restoreScreen();
	else
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
	dprintf(DEBUG_NORMAL, "CWidget:: exec: actionKey:%s\n", actionKey.c_str());

	int pos = 0;
	exit_pressed = false;

	if (parent)
		parent->hide();

	initFrames();

	printf("items.size: %d\d", (int)items.size());

	if(hasItem() && items.size() > 1)
	{
		for (unsigned int i = 0; i < items.size(); i++)
		{
			items[i]->setOutFocus(true);
		}

		for (unsigned int i = 0; i < items.size(); i++)
		{
			if(items[i]->isSelectable())
			{
				items[i]->setOutFocus(false);
				selected = i;
				break;
			}
		}
	}

	paint();

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

					//FIXME:review this
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
						case menu_return::RETURN_NONE:
							g_RCInput->killTimer(sec_timer_id);
							sec_timer_id = 0;
							retval = menu_return::RETURN_NONE;
							msg = RC_timeout;
							break;	
					}
				}
				else
				{
					//selected = -1;
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

			//
			if ( msg <= RC_MaxRC )
			{
				items[selected]->otherKeyPressed(msg);
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
				
				#if 0
				case (RC_up) :
					/*
					{
						if(items[selected]->isSelectable() && (items[selected]->itemType == WIDGET_ITEM_FRAMEBOX))
						{
							for (unsigned int count = 1; count < items.size(); count++) 
							{
								pos = (selected - count)%items.size();

								CWidgetItem * item = items[pos];

								if(item->isSelectable() && item->hasItem())
								{
									items[selected]->setOutFocus(true);

									selected = pos;

									item->setOutFocus(false);

									paint();

									break;
								}
							}
						}
					}
					*/
					break;
					
				case (RC_down) :
					/*
					{
						if(items[selected]->isSelectable() && (items[selected]->itemType == WIDGET_ITEM_FRAMEBOX))
						{
							for (unsigned int count = 1; count < items.size(); count++) 
							{
								pos = (selected + count)%items.size();

								CWidgetItem * item = items[pos];

								if(item->isSelectable() && item->hasItem())
								{
									items[selected]->setOutFocus(true);

									selected = pos;

									item->setOutFocus(false);

									paint();

									break;
								}
							}
						}
					}
					*/
					break;

				case (RC_left):
					/*
					{
						if(items[selected]->isSelectable() && ((items[selected]->itemType == WIDGET_ITEM_LISTBOX) && (items[selected]->getWidgetType() != WIDGET_TYPE_FRAME)))
						{
							for (unsigned int count = 1; count < items.size(); count++) 
							{
								pos = (selected - count)%items.size();

								CWidgetItem * item = items[pos];

								if(item->isSelectable() && item->hasItem())
								{
									items[selected]->setOutFocus(true);

									selected = pos;

									item->setOutFocus(false);

									paint();

									break;
								}
							}
						}
					}
					*/
					break;
					
				case (RC_right):
					/*
					{
						if(items[selected]->isSelectable() && ((items[selected]->itemType == WIDGET_ITEM_LISTBOX) && (items[selected]->getWidgetType() != WIDGET_TYPE_FRAME)))
						{
							for (unsigned int count = 1; count < items.size(); count++) 
							{
								pos = (selected + count)%items.size();

								CWidgetItem * item = items[pos];

								if(item->isSelectable() && item->hasItem())
								{
									items[selected]->setOutFocus(true);

									selected = pos;

									item->setOutFocus(false);

									paint();

									break;
								}
							}
						}
					}
					*/
					break;
				#endif

				case (RC_yellow):
					{
						for (unsigned int count = 1; count < items.size(); count++) 
						{
							pos = (selected + count)%items.size();

							CWidgetItem * item = items[pos];

							if(item->isSelectable() && item->hasItem())
							{
								items[selected]->setOutFocus(true);

								selected = pos;

								item->setOutFocus(false);

								paint();

								break;
							}
						}
					}
					break;

				case (RC_home):
					exit_pressed = true;
					dprintf(DEBUG_NORMAL, "exit_pressed\n");
					msg = RC_timeout;
					selected = -1;
					break;

				case (RC_ok):
					{
						if((items[selected]->itemType == WIDGET_ITEM_LISTBOX) || (items[selected]->itemType == WIDGET_ITEM_FRAMEBOX))
						{
							int rv = items[selected]->oKKeyPressed(this);

							//FIXME:review this
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
								case menu_return::RETURN_NONE:
									g_RCInput->killTimer(sec_timer_id);
									sec_timer_id = 0;
									retval = menu_return::RETURN_NONE;
									msg = RC_timeout;
									break;	
							}
						}
					}
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

	printf("CWidget: retval: (%d)\n", retval);
	
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



