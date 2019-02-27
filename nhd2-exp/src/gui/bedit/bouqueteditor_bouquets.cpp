/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: bouqueteditor_bouquets.cpp 2018/08/22 mohousch Exp $

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

#include <gui/bedit/bouqueteditor_bouquets.h>

#include <global.h>
#include <neutrino.h>

#include <driver/fontrenderer.h>
#include <driver/screen_max.h>
#include <gui/bedit/bouqueteditor_channels.h>

#include <gui/widget/widget_helpers.h>
#include <gui/widget/hintbox.h>
#include <gui/widget/icons.h>
#include <gui/widget/messagebox.h>
#include <gui/widget/stringinput.h>

//
#include <client/zapitclient.h>

#include <system/debug.h>


extern CBouquetManager * g_bouquetManager;	// defined in der zapit.cpp

CBEBouquetWidget::CBEBouquetWidget()
{
	frameBuffer = CFrameBuffer::getInstance();

	selected = 0;
	state = beDefault;
	blueFunction = beRename;
	
	Bouquets = &g_bouquetManager->Bouquets;

	listBox = NULL;
	item = NULL;

	// box	
	cFrameBox.iWidth = w_max ( (frameBuffer->getScreenWidth() / 20 * 17), (frameBuffer->getScreenWidth() / 20 ));
	cFrameBox.iHeight = h_max ( (frameBuffer->getScreenHeight() / 20 * 18), (frameBuffer->getScreenHeight() / 20));
	
	cFrameBox.iX = frameBuffer->getScreenX() + (frameBuffer->getScreenWidth() - cFrameBox.iWidth) / 2;
	cFrameBox.iY = frameBuffer->getScreenY() + (frameBuffer->getScreenHeight() - cFrameBox.iHeight) / 2;
}

CBEBouquetWidget::~CBEBouquetWidget()
{
}

#define BUTTONS_COUNT	3

const struct button_label CBEBouquetWidgetButtons[BUTTONS_COUNT] =
{
	{ NEUTRINO_ICON_BUTTON_RED, LOCALE_BOUQUETEDITOR_DELETE, NULL },
	{ NEUTRINO_ICON_BUTTON_GREEN, LOCALE_BOUQUETEDITOR_ADD, NULL },
	{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_BOUQUETEDITOR_MOVE, NULL }
};

const struct button_label HButton = {NEUTRINO_ICON_BUTTON_SETUP, NONEXISTANT_LOCALE, NULL };

void CBEBouquetWidget::paint()
{
	dprintf(DEBUG_NORMAL, "CBEBouquetWidget::paint:\n");

	for (unsigned int count = 0; count < Bouquets->size(); count++)
	{
		item = new ClistBoxEntryItem((*Bouquets)[count]->bFav ? g_Locale->getText(LOCALE_FAVORITES_BOUQUETNAME) : (*Bouquets)[count]->Name.c_str(), true);

		if(state == beMoving && count == selected)
			item->setIconName(NEUTRINO_ICON_BUTTON_YELLOW);

		// locked
		std::string locked_icon = "";
		if((*Bouquets)[count]->bLocked)
			locked_icon = NEUTRINO_ICON_LOCK;

		item->setIcon1(locked_icon.c_str());

		// hidden
		std::string hiden_icon = "";
		if((*Bouquets)[count]->bHidden)
			hiden_icon = NEUTRINO_ICON_HIDDEN;

		item->setIcon2(hiden_icon.c_str()); 

		listBox->addItem(item);
	}

	listBox->setTitle(g_Locale->getText(LOCALE_BOUQUETLIST_HEAD));
	listBox->enablePaintHead();
	listBox->enablePaintDate();
	listBox->setHeaderButtons(&HButton, 1);

	// foot
	listBox->enablePaintFoot();

	struct button_label Button[4];
	Button[0] = CBEBouquetWidgetButtons[0];
	Button[1] = CBEBouquetWidgetButtons[1];
	Button[2] = CBEBouquetWidgetButtons[2];
	Button[3].button = NEUTRINO_ICON_BUTTON_BLUE;

	switch( blueFunction)
	{
		case beRename:
			Button[3].locale = LOCALE_BOUQUETEDITOR_RENAME;
			Button[3].localename = NULL;
			break;
			
		case beHide:
			Button[3].locale = LOCALE_BOUQUETEDITOR_HIDE;
			Button[3].localename = NULL;
			break;
			
		case beLock:
			Button[3].locale = LOCALE_BOUQUETEDITOR_LOCK;
			Button[3].localename = NULL;
			break;
	}

	listBox->setFooterButtons(Button, 4);

	//
	listBox->setSelected(selected);
	listBox->paint();
}

void CBEBouquetWidget::hide()
{
	listBox->hide();
}

int CBEBouquetWidget::exec(CMenuTarget* parent, const std::string &/*actionKey*/)
{
	dprintf(DEBUG_NORMAL, "CBEBouquetWidget::exec:\n");

	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	int res = menu_return::RETURN_REPAINT;

	if (parent)
		parent->hide();
	
	Bouquets = &g_bouquetManager->Bouquets;

	listBox = new ClistBox(&cFrameBox);
	
	paint();
	frameBuffer->blit();

	bouquetsChanged = false;

	// add sec timer
	sec_timer_id = g_RCInput->addTimer(1*1000*1000, false);

	uint64_t timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_EPG]);

	bool loop = true;
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if ( msg <= RC_MaxRC )
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_EPG]);

		if ((msg == RC_timeout) || (msg == (neutrino_msg_t)g_settings.key_channelList_cancel))
		{
			if (state == beDefault)
			{
				if (bouquetsChanged)
				{
					int result = MessageBox(LOCALE_BOUQUETEDITOR_NAME, LOCALE_BOUQUETEDITOR_SAVECHANGES, mbrYes, mbAll);

					switch( result )
					{
						case mbrYes :
							loop = false;
							saveChanges();
						break;
						
						case mbrNo :
							loop = false;
							discardChanges();
						break;
						
						case mbrCancel :
							listBox->clearItems();
							paint();
						break;
					}
				}
				else
				{
					loop = false;
				}
			}
			else if (state == beMoving)
			{
				cancelMoveBouquet();
			}
		}
		else if (msg == RC_page_up)
		{
			if (state == beDefault)
				listBox->scrollPageUp();
			else if(state == beMoving)
			{
				selected = listBox->getSelected();
				int next_selected = selected - listBox->getListMaxShow();

				if (next_selected < 0)
					next_selected = 0;

				internalMoveBouquet(selected, next_selected);
			}
		}
		else if (msg == RC_page_down)
		{
			if (state == beDefault)
				listBox->scrollPageDown();
			else if(state == beMoving)
			{
				selected = listBox->getSelected();

				int next_selected = selected + listBox->getListMaxShow();

				if (next_selected > (int)Bouquets->size())
					next_selected = Bouquets->size();

				internalMoveBouquet(selected, next_selected);
			}
		}
		else if (msg == RC_up)
		{
			if (!(Bouquets->empty()))
			{
				if (state == beDefault)
				{
					listBox->scrollLineUp();
				}
				else if (state == beMoving)
				{
					selected = listBox->getSelected();

					int prev_selected = selected;
					int next_selected = selected - 1;
					if (next_selected < 0)
						next_selected = 0;

					internalMoveBouquet(prev_selected, next_selected);
				}
			}
		}
		else if (msg == RC_down)
		{
			if (state == beDefault)
			{	
				listBox->scrollLineDown();
			}
			else if (state == beMoving)
			{
				selected = listBox->getSelected();

				int prev_selected = selected;
				int next_selected = selected + 1;
				if (next_selected > (int)Bouquets->size())
					next_selected = Bouquets->size();

				internalMoveBouquet(prev_selected, next_selected);
			}
		}
		else if(msg == RC_red)
		{
			selected = listBox->getSelected();

			if (state == beDefault)
				deleteBouquet();
		}
		else if(msg == RC_green)
		{
			selected = listBox->getSelected();

			if (state == beDefault)
				addBouquet();
		}
		else if(msg == RC_yellow)
		{
			selected = listBox->getSelected();

			if (selected < Bouquets->size()) // Bouquets->size() might be 0
			{
				if (state == beDefault)
					beginMoveBouquet();

				listBox->clearItems();
				paint();
			}
		}
		else if(msg == RC_blue)
		{
			if (selected < Bouquets->size()) // Bouquets->size() might be 0
			{
				selected = listBox->getSelected();

				if (state == beDefault)
				{
					switch (blueFunction)
					{
						case beRename:
							renameBouquet();
							break;
						case beHide:
							switchHideBouquet();
							break;
						case beLock:
							switchLockBouquet();
							break;
					}
				}
			}
		}
		else if(msg == RC_setup)
		{
			if (state == beDefault)
			{
				switch (blueFunction)
				{
					case beRename:
						blueFunction = beHide;
					break;
					case beHide:
						blueFunction = beLock;
					break;
					case beLock:
						blueFunction = beRename;
					break;
				}

				listBox->clearItems();
				paint();
			}
		}
		else if(msg == RC_ok)
		{
			if (state == beDefault)
			{
				selected = listBox->getSelected();

				if (selected < Bouquets->size()) /* Bouquets->size() might be 0 */
				{
					CBEChannelWidget* channelWidget = new CBEChannelWidget((*Bouquets)[selected]->bFav ? g_Locale->getText(LOCALE_FAVORITES_BOUQUETNAME) : (*Bouquets)[selected]->Name, selected);

					channelWidget->exec(this, "");
					if (channelWidget->hasChanged())
						bouquetsChanged = true;
					delete channelWidget;

					listBox->clearItems();
					paint();
				}
			}
			else if (state == beMoving)
			{
				finishMoveBouquet();
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
				cancelMoveBouquet();
			}
		}
		else if ( (msg == NeutrinoMessages::EVT_TIMER) && (data == sec_timer_id) )
		{
			listBox->paintHead();
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

	delete listBox;
	listBox = NULL;
	
	return res;
}

void CBEBouquetWidget::deleteBouquet()
{
	if (selected >= Bouquets->size()) /* Bouquets->size() might be 0 */
		return;

	if (MessageBox(LOCALE_FILEBROWSER_DELETE, (*Bouquets)[selected]->bFav ? g_Locale->getText(LOCALE_FAVORITES_BOUQUETNAME) : (*Bouquets)[selected]->Name.c_str(), mbrNo, mbYes | mbNo) != mbrYes)
		return;

	g_bouquetManager->deleteBouquet(selected);
	Bouquets = &g_bouquetManager->Bouquets;
	if (selected >= Bouquets->size())
		selected = Bouquets->empty() ? 0 : (Bouquets->size() - 1);
	bouquetsChanged = true;

	listBox->clearItems();
	paint();
}

void CBEBouquetWidget::addBouquet()
{
	std::string newName = inputName("", LOCALE_BOUQUETEDITOR_BOUQUETNAME);
	if (!(newName.empty()))
	{
		g_bouquetManager->addBouquet(newName, true);
		Bouquets = &g_bouquetManager->Bouquets;
		selected = Bouquets->empty() ? 0 : (Bouquets->size() - 1);
		bouquetsChanged = true;
	}
	
	listBox->clearItems();
	paint();
}

void CBEBouquetWidget::beginMoveBouquet()
{
	state = beMoving;
	origPosition = selected;
	newPosition = selected;
}

void CBEBouquetWidget::finishMoveBouquet()
{
	state = beDefault;
	if (newPosition != origPosition)
	{
		Bouquets = &g_bouquetManager->Bouquets;
		bouquetsChanged = true;
	}

	listBox->clearItems();
	paint();
}

void CBEBouquetWidget::cancelMoveBouquet()
{
	state = beDefault;
	internalMoveBouquet( newPosition, origPosition);
	bouquetsChanged = false;
}

void CBEBouquetWidget::internalMoveBouquet( unsigned int fromPosition, unsigned int toPosition)
{
	if ( (int) toPosition == -1 ) return;
	if ( toPosition == Bouquets->size()) return;

	g_bouquetManager->moveBouquet(fromPosition, toPosition);
	Bouquets = &g_bouquetManager->Bouquets;
	bouquetsChanged = true;

	selected = toPosition;
	newPosition = toPosition;

	listBox->clearItems();
	paint();
}

void CBEBouquetWidget::renameBouquet()
{
	if ((*Bouquets)[selected]->bFav)
		return;

	std::string newName = inputName((*Bouquets)[selected]->Name.c_str(), LOCALE_BOUQUETEDITOR_NEWBOUQUETNAME);
	if (newName != (*Bouquets)[selected]->Name)
	{
		g_bouquetManager->Bouquets[selected]->Name = newName;
		g_bouquetManager->Bouquets[selected]->bUser = true;

		bouquetsChanged = true;
	}

	listBox->clearItems();
	paint();
}

void CBEBouquetWidget::switchHideBouquet()
{
	bouquetsChanged = true;
	(*Bouquets)[selected]->bHidden = !(*Bouquets)[selected]->bHidden;

	listBox->clearItems();
	paint();
}

void CBEBouquetWidget::switchLockBouquet()
{
	bouquetsChanged = true;
	(*Bouquets)[selected]->bLocked = !(*Bouquets)[selected]->bLocked;

	listBox->clearItems();
	paint();
}

std::string CBEBouquetWidget::inputName(const char * const defaultName, const neutrino_locale_t caption)
{
	//FIXME: max input it too long than bqt window width
	char Name[MAX_INPUT_CHARS + 1];

	strncpy(Name, defaultName, MAX_INPUT_CHARS + 1);

	CStringInputSMS *nameInput = new CStringInputSMS(caption, Name);
	nameInput->exec(this, "");
	delete nameInput;

	return std::string(Name);
}

void CBEBouquetWidget::saveChanges()
{
	CHintBox* hintBox= new CHintBox(LOCALE_BOUQUETEDITOR_NAME, g_Locale->getText(LOCALE_BOUQUETEDITOR_SAVINGCHANGES), 480); // UTF-8
	hintBox->paint();
	
	g_Zapit->saveBouquets();
	g_Zapit->reinitChannels();
	
	hintBox->hide();
	delete hintBox;
}

void CBEBouquetWidget::discardChanges()
{
	CHintBox* hintBox= new CHintBox(LOCALE_BOUQUETEDITOR_NAME, g_Locale->getText(LOCALE_BOUQUETEDITOR_DISCARDINGCHANGES), 480); // UTF-8
	hintBox->paint();
	
	g_Zapit->restoreBouquets();
	
	hintBox->hide();
	delete hintBox;
}


