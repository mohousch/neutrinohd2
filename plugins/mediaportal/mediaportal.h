/*
  $Id: mediaportal.h 2016.02.10 19:03:30 mohousch Exp $

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

#ifndef __mediaportal__
#define __mediaportal__

#include <plugin.h>


class CMediaPortal : public CMenuTarget
{	
	public:
		CMediaPortal();
		~CMediaPortal();
		
		int exec(CMenuTarget* parent, const std::string& actionKey);
		void showMenu(void);

};

#endif //__mediaportal__

