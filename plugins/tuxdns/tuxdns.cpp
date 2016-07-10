/* 
  $Id: tuxdns.cpp 2014/01/22 mohousch Exp $

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

#include <tuxdns.h>


extern "C" void plugin_exec(void);
extern "C" void plugin_init(void);
extern "C" void plugin_del(void);


#define TuxdnsCFG PLUGINDIR "/tuxdns/tuxdns.conf"

#define OPTIONS_OFF_ON_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_OFF_ON_OPTIONS[OPTIONS_OFF_ON_OPTION_COUNT] =
{
	{ 0, LOCALE_OPTIONS_OFF, NULL },
	{ 1, LOCALE_OPTIONS_ON, NULL }
};

CTuxdnsConf::CTuxdnsConf()
{
	sprintf(pause, "%s", "600");
	verbose = 1;
	sprintf(user, "%s", "benutzer");
	sprintf(pass, "%s", "passwort");
	sprintf(host, "%s", "myhost.dyndns.org");
}

CTuxdnsConf::~CTuxdnsConf()
{
}

void CTuxdnsConf::hide()
{
	CFrameBuffer::getInstance()->paintBackground();
	CFrameBuffer::getInstance()->blit();
}

void CTuxdnsConf::readSettings()
{
	FILE* fd = fopen(TuxdnsCFG, "r");
	char buffer[120];

	if(fd) 
	{
		while(fgets(buffer, 120, fd)!=NULL) 
		{
			sscanf(buffer, "pause = %s"	, (char *) pause);
			sscanf(buffer, "verbose = %d"	, &verbose);
			sscanf(buffer, "user = %s"	, (char *) user);
			sscanf(buffer, "pass = %s"	, (char *) pass);
			sscanf(buffer, "host = %s"	, (char *) host);
		}
		fclose(fd);
	}
	else 
	{
		printf("tuxdns.conf: File %s not found. Using defaults.\n",TuxdnsCFG);
	}
}

bool CTuxdnsConf::SaveSettings()
{
	FILE* out = 0;
	out = fopen(TuxdnsCFG, "w");
	if(!out) 
	{
		printf("unable to write file %s\n", TuxdnsCFG);
		return false;
	}

	printf("write to file %s\n", TuxdnsCFG);
	fprintf(out, "<global>\n");
	fprintf(out, "daemon = 1\n");
	fprintf(out, "pause = %s\n"	, pause);
	fprintf(out, "verbose = %d\n"	, verbose);
	fprintf(out, "</global>\n");
	fprintf(out, "\n");
	fprintf(out, "<record>\n");
	fprintf(out, "user = %s\n"	, user);
	fprintf(out, "pass = %s\n"	, pass);
	fprintf(out, "host = %s\n"	, host);
	fprintf(out, "</record>\n");

	fclose(out);

	return true;
}

int CTuxdnsConf::exec(CMenuTarget* parent, const std::string & actionKey)
{
	if (parent)
		parent->hide();
	
	if(actionKey == "savesettings") 
	{
		if(this->SaveSettings())
		 	HintBox(LOCALE_MESSAGEBOX_INFO, "Einstellungen werden gespeichert!");
		else
		 	HintBox(LOCALE_MESSAGEBOX_INFO, "Einstellungen NICHT gespeichert!");
	}

	return menu_return::RETURN_REPAINT;
}

void CTuxdnsConf::TuxdnsSettings()
{
	readSettings();
	
	//
	CMenuWidget * tuxDNSMenu = new CMenuWidget("TuxDNS", NEUTRINO_ICON_STREAMING);
	
	tuxDNSMenu->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	tuxDNSMenu->addItem(new CMenuSeparator(CMenuSeparator::LINE));
	tuxDNSMenu->addItem(new CMenuForwarder("Save settings", true, "", this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED) );
	tuxDNSMenu->addItem(new CMenuSeparator(CMenuSeparator::LINE));

	//user
	CStringInputSMS*  ojuser = new CStringInputSMS((char *)"User Name", user);
	tuxDNSMenu->addItem( new CMenuForwarder("User Name", true, user, ojuser ));
	
	//passwd
	CStringInputSMS*  ojpass = new CStringInputSMS((char *)"Password", pass);
	tuxDNSMenu->addItem( new CMenuForwarder("Password", true, pass, ojpass ));
	
	//host name
	CStringInputSMS*  ojhost = new CStringInputSMS((char *)"Host Name", host);
	tuxDNSMenu->addItem( new CMenuForwarder("Host Name", true, host, ojhost ));
	
	//pause
	CStringInput*  ojpause = new CStringInput((char *)"Pause", pause);
	tuxDNSMenu->addItem( new CMenuForwarder("Pause", true, pause, ojpause ));
	
	//verbose
	tuxDNSMenu->addItem(new CMenuOptionChooser("verbose", &verbose, OPTIONS_OFF_ON_OPTIONS, OPTIONS_OFF_ON_OPTION_COUNT,true));

	tuxDNSMenu->exec(NULL, "");
	tuxDNSMenu->hide();
	delete tuxDNSMenu;
}

//
void plugin_init(void)
{
}

void plugin_del(void)
{
}

void plugin_exec(void)
{
	CTuxdnsConf * TuxdnsConf = new CTuxdnsConf();
	
	TuxdnsConf->TuxdnsSettings();
	
	delete TuxdnsConf;
}


