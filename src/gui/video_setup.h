/*
	Neutrino-GUI  -   DBoxII-Project

	$id: video_setup.h 2016.01.02 19:49:28 mohousch $
	
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

#ifndef __video_setup__
#define __video_setup__

#include <gui/widget/menue.h>

#include <string>


class CVideoSettings : public CMenuTarget
{
	private:
		void showMenu();
	  
	public:
		CVideoSettings();
		~CVideoSettings();
		
		int exec(CMenuTarget* parent, const std::string& actionKey);
};

#endif //__video_setup__
