/*
	Neutrino graphlcd menue

	Copyright (C) 2012 martii

	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __glcdsetup_h__
#define __glcdsetup_h__

#include <sys/types.h>
#include <string.h>

#include <plugin.h>

#include <nglcd.h>


#define CONFIG_FILE		PLUGINDIR "/nglcd/nglcd.conf"

class GLCD_Menu;

class GLCD_Menu_Notifier : public CChangeObserver
{
	private:
		GLCD_Menu* parent;
	public:
		GLCD_Menu_Notifier();
		bool changeNotify(const neutrino_locale_t, void *);
};

class GLCD_Menu : public CMenuTarget
{
	private:
		static int color2index(uint32_t color);
		GLCD_Menu_Notifier *notifier;
	public:
		static uint32_t index2color(int i);
		GLCD_Menu();
		void hide();
		int exec(CMenuTarget* parent, const std::string & actionKey);
		void GLCD_Menu_Settings();
		void ReadSettings();
		bool SaveSettings();
};

#endif // __glcdsetup_h__

