/*
  $Id: restart_enigma.cpp 2018/07/22 mohousch Exp $

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

#include <plugin.h>

 
extern "C" void plugin_exec(void);
extern "C" void plugin_init(void);
extern "C" void plugin_del(void);

class CRestartEnigma : public CMenuTarget
{
	private:
		void showMenu();

	public:
		CRestartEnigma();
		~CRestartEnigma();
		int exec(CMenuTarget* parent, const std::string& actionKey);
};

CRestartEnigma::CRestartEnigma()
{
}

CRestartEnigma::~CRestartEnigma()
{
}

void CRestartEnigma::showMenu()
{
	if(MessageBox(LOCALE_MESSAGEBOX_INFO, "do you want really to restart enigma2?", CMessageBox::mbrYes, CMessageBox::mbYes | CMessageBox::mbNo, NULL, 450, 30, true) == CMessageBox::mbrYes)
					
	{
		system("rm -f /etc/.nhd2");
		system("init 4");
		system("init 3");
	}
}

int CRestartEnigma::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CRestartEnigma::exec: %s\n", actionKey.c_str());

	if(parent)
		hide();

	showMenu();

	return menu_return::RETURN_REPAINT;
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
	CRestartEnigma* restartEnigmaHandler = new CRestartEnigma();

	restartEnigmaHandler->exec(NULL, "");

	delete restartEnigmaHandler;
	restartEnigmaHandler = NULL;
}


