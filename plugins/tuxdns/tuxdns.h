/*
  $Id: tuxdns.h 2014/01/22 mohousch Exp $

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

#ifndef __tuxdnsconf__
#define __tuxdnsconf__

#include <plugin.h>


using namespace std;

class CTuxdnsConf : public CMenuTarget
{
	private:
		char	pause[5];
		int	verbose;
		char	user[21];
		char	pass[21];
		char	host[32];
		
		void readSettings();
		bool SaveSettings();

		void showMenu();
	public:
		CTuxdnsConf();
		~CTuxdnsConf();
		int  exec(CMenuTarget* parent, const std::string & actionKey);
		void hide();
};

#endif //__tuxdnsconf__
 
