/*
	Neutrino-GUI  -   DBoxII-Project

	$id: satip_setup.h 2016.01.14 11:37:30 mohousch $
	
	Copyright (C) 2001 Steffen Hehn 'McClean'
	and some other guys
	Homepage: http://dbox.cyberphoria.org/

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

#ifndef __satip_setup__
#define __satip_setup__

#include <gui/widget/menue.h>

#include <string>


class CSatIPSetup : public CMenuTarget
{
	private:
		void showMenu();
		
	public:
		CSatIPSetup();
		~CSatIPSetup();
		
		int exec(CMenuTarget* parent, const std::string& actionKey);
};

// satip notifier
class CSatIPNotifier : public CChangeObserver
{
        public:
                bool changeNotify(const neutrino_locale_t, void * );
};

#endif //__satip_setup__

