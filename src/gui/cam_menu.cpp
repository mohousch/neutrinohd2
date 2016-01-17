/*
	Neutrino-GUI  -   DBoxII-Project

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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/mount.h>

#include <global.h>
#include <neutrino.h>
#include <gui/widget/icons.h>
#include "gui/widget/menue.h"
#include "gui/widget/stringinput.h"
#include "gui/widget/messagebox.h"
#include "gui/widget/hintbox.h"
#include "gui/widget/progresswindow.h"

#include "system/setting_helpers.h"
#include "system/settings.h"
#include "system/debug.h"

#include <gui/cam_menu.h>

#include <dvb-ci.h>
#include <sectionsd/edvbstring.h>


void CCAMMenuHandler::init(void)
{
	hintBox = NULL;
	ci = cDvbCi::getInstance();
}

int CCAMMenuHandler::exec(CMenuTarget * parent, const std::string &actionKey)
{
	dprintf(DEBUG_NORMAL, "CCAMMenuHandler::exec: actionkey %s\n", actionKey.c_str());

	int ret = menu_return::RETURN_REPAINT;

        if (parent)
                parent->hide();

	if(actionKey == "cam1") 
	{
		return doMenu(0);
	}	
	else if(actionKey == "cam2") 
	{
		return doMenu(1);
	}	
	if(actionKey == "reset1") 
	{
		ci->reset(0);
		return menu_return::RETURN_EXIT_ALL;
	}	
	if(actionKey == "reset2") 
	{
		ci->reset(1);
		return menu_return::RETURN_EXIT_ALL;
	}	

	doMainMenu();

	return ret;
}

void CCAMMenuHandler::doMainMenu()
{
	dprintf(DEBUG_NORMAL, "CCAMMenuHandler::doMainMenu\n");

	char name[255];
	char str[255];

	CMenuWidget * cammenu = new CMenuWidget(LOCALE_CAM_SETTINGS, NEUTRINO_ICON_SETTINGS);
	
	// intros
	cammenu->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));

	CMenuWidget * tempMenu;
	
	for (unsigned int i = 0; i < ci->ci_num; i++)
	{
		if(ci->CamPresent(i)) 
		{
			ci->GetName(i, name);
			
			dprintf(DEBUG_NORMAL, "CCAMMenuHandler::doMenu cam %d name %s\n", i, name);
			
			char CAM[255];
			sprintf(CAM, "cam%d", i + 1);

			cammenu->addItem(new CMenuForwarder(name, true, NULL, this, CAM, CRCInput::RC_nokey));

			char RESET[32];
			sprintf(RESET, "reset%d", i + 1);

			cammenu->addItem(new CMenuSeparator(CMenuSeparator::LINE));

			cammenu->addItem(new CMenuForwarder(LOCALE_CAM_RESET, true, NULL, this, RESET, CRCInput::RC_nokey));
		} 
		else 
		{
			sprintf(str, "%s %d", g_Locale->getText(LOCALE_CAM_EMPTY), i + 1);
			tempMenu = new CMenuWidget(str, NEUTRINO_ICON_SETTINGS);

			cammenu->addItem(new CMenuSeparator(CMenuSeparator::LINE));
			cammenu->addItem(new CMenuForwarder(str, false, NULL, tempMenu));
		}
	}	

	cammenu->exec(NULL, "");
	delete cammenu;
}

#define CI_MSG_TIME 5
int CCAMMenuHandler::handleMsg(const neutrino_msg_t msg, neutrino_msg_data_t data)
{
	int ret = messages_return::handled;

	int camret = handleCamMsg(msg, data);
	
	if(camret < 0) 
	{
		ret = messages_return::unhandled;
	}
	
	return ret;
}

int CCAMMenuHandler::handleCamMsg (const neutrino_msg_t msg, neutrino_msg_data_t data, bool from_menu)
{
	int ret = 0;
	char str[255];
	char cnt[5];
	int i;
	MMI_MENU_LIST_INFO Menu;
	MMI_ENGUIRY_INFO MmiEnquiry;
	int curslot;
	MMI_MENU_LIST_INFO * pMenu = &Menu;
	MMI_ENGUIRY_INFO * pMmiEnquiry = &MmiEnquiry;

	if(msg == NeutrinoMessages::EVT_CI_INSERTED) 
	{
		if(hintBox != NULL) 
		{
			hintBox->hide();
			delete hintBox;
			hintBox = NULL;
		}
		
		sprintf(str, "%s %d", g_Locale->getText(LOCACE_CAM_INSERTED), (int) data + 1);

		dprintf(DEBUG_NORMAL, "CCAMMenuHandler::handleMsg: %s\n", str);
		
		hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, str);

		if(hintBox != NULL) 
			hintBox->paint();

		sleep(CI_MSG_TIME);

		if(hintBox != NULL) 
		{
			hintBox->hide();
			delete hintBox;
			hintBox = NULL;
		}

	} 
	else if (msg == NeutrinoMessages::EVT_CI_REMOVED) 
	{
		if(hintBox != NULL) 
		{
			hintBox->hide();
			delete hintBox;
			hintBox = NULL;
		}

		sprintf(str, "%s %d", g_Locale->getText(LOCALE_CAM_REMOVED), (int) data + 1);

		dprintf(DEBUG_NORMAL, "CCAMMenuHandler::handleMsg: %s\n", str);
		
		hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, str);

		if(hintBox != NULL) 
			hintBox->paint();

		sleep(CI_MSG_TIME);

		if(hintBox != NULL) 
		{
			hintBox->hide();
			delete hintBox;
			hintBox = NULL;	
		}
	} 
	else if(msg == NeutrinoMessages::EVT_CI_INIT_OK) 
	{
		if(hintBox != NULL) 
		{
			hintBox->hide();
			delete hintBox;
			hintBox = NULL;
		}
		char name[255] = "Unknown";
		
		ci->GetName((int) data, name);
		sprintf(str, "%s %d: %s", g_Locale->getText(LOCALE_CAM_INIT_OK), (int) data+1, name);

		dprintf(DEBUG_NORMAL, "CCAMMenuHandler::handleMsg: %s\n", str);
		
		hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, str);

		if(hintBox != NULL) 
			hintBox->paint();

		sleep(CI_MSG_TIME);

		if(hintBox != NULL) 
		{
			hintBox->hide();
			delete hintBox;
			hintBox = NULL;
		}
	}
	else if(msg == NeutrinoMessages::EVT_CI_MMI_MENU || msg == NeutrinoMessages::EVT_CI_MMI_LIST) 
	{
		bool sublevel = false;
		if(msg != NeutrinoMessages::EVT_CI_MMI_MENU)
			sublevel = true;
		
		memcpy(pMenu, (MMI_MENU_LIST_INFO*) data, sizeof(MMI_MENU_LIST_INFO));
		free((void *)data);

		curslot = pMenu->slot;

		dprintf(DEBUG_NORMAL, "CCAMMenuHandler::handleCamMsg: slot %d menu ready, title %s choices %d\n", curslot, convertDVBUTF8(pMenu->title, strlen(pMenu->title), 0).c_str(), pMenu->choice_nb);

		if(hintBox) 
			hintBox->hide();

		int selected = -1;
		if(pMenu->choice_nb) 
		{
			CMenuWidget * menu = new CMenuWidget(convertDVBUTF8(pMenu->title, strlen(pMenu->title), 0).c_str(), NEUTRINO_ICON_SETTINGS);

			menu->enableSaveScreen(true);

			CMenuSelectorTarget * selector = new CMenuSelectorTarget(&selected);
			int slen = strlen(pMenu->subtitle);
			
			if(slen) 
			{
				char * sptr = pMenu->subtitle;
				char * tptr = sptr;
				int bpos = 0;
				for(int i1 = 0; i1 < slen; i1++) 
				{
					if((tptr[i1] == 0x8A) || ((bpos >= 38) && (tptr[i1] == 0x20)) ) 
					{
						bpos = 0;
						tptr[i1] = 0;
						
						dprintf(DEBUG_NORMAL, "CCAMMenuHandler::handleCamMsg: subtitle: %s\n", sptr);
						
						menu->addItem(new CMenuForwarder(convertDVBUTF8(sptr, strlen(sptr), 0).c_str(), false));
						sptr = &tptr[i1+1];
					}
					bpos++;
				}
				
				if(strlen(sptr)) 
				{
					dprintf(DEBUG_NORMAL, "CCAMMenuHandler::handleCamMsg: subtitle: %s\n", sptr);
					
					menu->addItem(new CMenuForwarder(convertDVBUTF8(sptr, strlen(sptr), 0).c_str(), false));
				}
			}
			
			for(i = 0; i < pMenu->choice_nb; i++) 
			{
				sprintf(cnt, "%d", i);
				if(sublevel)
					menu->addItem(new CMenuForwarder(convertDVBUTF8(pMenu->choice_item[i], strlen(pMenu->choice_item[i]), 0).c_str(), true, NULL, selector, cnt));
				else
					menu->addItem(new CMenuForwarder(convertDVBUTF8(pMenu->choice_item[i], strlen(pMenu->choice_item[i]), 0).c_str(), true, NULL, selector, cnt, CRCInput::convertDigitToKey(i+1)));
			}
			slen = strlen(pMenu->bottom);
			
			if(slen) 
			{
				dprintf(DEBUG_NORMAL, "CCAMMenuHandler::handleCamMsg: bottom: %s\n", pMenu->bottom);
				
				menu->addItem(new CMenuForwarder(convertDVBUTF8(pMenu->bottom, slen, 0).c_str(), false));
			}

			menu->exec(NULL, "");

			delete menu;
			delete selector;
		} 
		else 
		{
			char _str[255];
			snprintf(_str, 255, "%s\n%s\n%s", pMenu->title, pMenu->subtitle, pMenu->bottom);
			
			if(hintBox)
			{
				delete hintBox;
				hintBox = NULL;
			}

			hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, convertDVBUTF8(_str, strlen(_str), 0).c_str());
			if(hintBox != NULL) 
				hintBox->paint();

			sleep(4);//FIXME
			
			if(!from_menu) 
			{
				if(hintBox != NULL) 
				{
					delete hintBox;
					hintBox = NULL;
				}
			}
			return 1;
		}

		if(sublevel)
			return 0;

		if(selected >= 0) 
		{
			dprintf(DEBUG_NORMAL, "CCAMMenuHandler::handleCamMsg: selected %d:%s sublevel %s\n", selected, pMenu->choice_item[i], sublevel ? "yes" : "no");

			ci->CI_MenuAnswer(curslot, selected+1);

			timeoutEnd = CRCInput::calcTimeoutEnd(10);
			return 1;
		} 
		else 
		{
			return 2;
		}
	}
	else if(msg == NeutrinoMessages::EVT_CI_MMI_REQUEST_INPUT) 
	{
		memcpy(pMmiEnquiry, (MMI_ENGUIRY_INFO*) data, sizeof(MMI_ENGUIRY_INFO));
		free((void *)data);
		curslot = pMmiEnquiry->slot;
		
		dprintf(DEBUG_NORMAL, "CCAMMenuHandler::handleCamMsg: slot %d input request, text %s\n", curslot, convertDVBUTF8(pMmiEnquiry->enguiryText, strlen(pMmiEnquiry->enguiryText), 0).c_str());
		
		if(hintBox)
			hintBox->hide();

		char cPIN[pMmiEnquiry->answerlen+1];
		cPIN[0] = 0;

		CPINInput *PINInput = new CPINInput((char *) convertDVBUTF8(pMmiEnquiry->enguiryText, strlen(pMmiEnquiry->enguiryText), 0).c_str(), cPIN, 4, NONEXISTANT_LOCALE);
		PINInput->exec(NULL, "");
		delete PINInput;

		dprintf(DEBUG_NORMAL, "CCAMMenuHandler::handleCamMsg: input=[%s]\n", cPIN);

		if((int) strlen(cPIN) != pMmiEnquiry->answerlen) 
		{
			dprintf(DEBUG_NORMAL, "CCAMMenuHandler::handleCamMsg: wrong input len\n");

			ci->CI_Answer(curslot, (unsigned char *) cPIN, 0);

			return 0;
		} 
		else 
		{
			ci->CI_Answer(curslot, (unsigned char *) cPIN, pMmiEnquiry->answerlen);

			return 1;
		}
	}
	else if(msg == NeutrinoMessages::EVT_CI_MMI_CLOSE) 
	{
		curslot = (int) data;
		
		dprintf(DEBUG_NORMAL, "CCAMMenuHandler::handleCamMsg: close request slot: %d\n", curslot);

		ci->CI_CloseMMI(curslot);

		return 0;
	}
	else if(msg == NeutrinoMessages::EVT_CI_MMI_TEXT) 
	{
		curslot = (int) data;
		
		dprintf(DEBUG_NORMAL, "CCAMMenuHandler::handleCamMsg: text\n");
	} 
	else
		ret = -1;

	return ret;
}

int CCAMMenuHandler::doMenu(int slot)
{
	int res = menu_return::RETURN_REPAINT;
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
	bool doexit = false;

	while(!doexit) 
	{
		dprintf(DEBUG_NORMAL, "CCAMMenuHandler::doMenu: slot %d\n", slot);

		timeoutEnd = CRCInput::calcTimeoutEnd(10);

		ci->CI_EnterMenu(slot);

		while(true) 
		{
			if(hintBox)
			{
				delete hintBox;
				hintBox = NULL;
			}
			
			hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_CAM_WAITING));

			if(hintBox != NULL) 
				hintBox->paint();

			g_RCInput->getMsgAbsoluteTimeout (&msg, &data, &timeoutEnd);
			
			dprintf(DEBUG_NORMAL, "CCAMMenuHandler::doMenu: msg %x data %x\n", msg, data);
			
			if (msg == CRCInput::RC_timeout) 
			{
				if(hintBox)
				{
					delete hintBox;
					hintBox = NULL;
				}

				hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_CAM_TIMEOUT));
				
				if(hintBox != NULL) 
					hintBox->paint();

				dprintf(DEBUG_NORMAL, "CCAMMenuHandler::doMenu: menu timeout\n");
				
				sleep(5);

				if(hintBox != NULL) 
				{
					delete hintBox;
					hintBox = NULL;
				}

				ci->CI_CloseMMI(slot);

				return menu_return::RETURN_REPAINT;
			} 
			
			/* -1 = not our event, 0 = back to top menu, 1 = continue loop, 2 = quit */
			int ret = handleCamMsg(msg, data, true);
			if(ret < 0 && (msg > CRCInput::RC_Messages)) 
			{
				if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & ( messages_return::cancel_all | messages_return::cancel_info ) )
				{
					doexit = true;
					res = menu_return::RETURN_EXIT_ALL;
				}
			} 
			else if (ret == 1) 
			{
				timeoutEnd = CRCInput::calcTimeoutEnd(10);
				continue;
			} 
			else if (ret == 2) 
			{
				doexit = true;
				break;
			} 
			else 
			{
				break;
			}
		}
	}

	ci->CI_CloseMMI(slot);

	if(hintBox) 
	{
		delete hintBox;
		hintBox = NULL;
	}
	
	dprintf(DEBUG_NORMAL, "CCAMMenuHandler::doMenu: return\n");
	
	return res; 
}
