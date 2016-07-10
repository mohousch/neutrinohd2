/*

        $Id: settings.cpp,v 1.39 2012/03/21 16:32:41 mohousch Exp $

	Neutrino-GUI  -   DBoxII-Project

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
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
*/

#include <cstring>
#include <system/settings.h>

#include <config.h>
#include <global.h>
#include <neutrino.h>

#include <gui/widget/icons.h>


const int default_timing[TIMING_SETTING_COUNT] =
{
	0,
	60,
	240,
	6,
	60,
	3
};

const neutrino_locale_t timing_setting_name[TIMING_SETTING_COUNT] =
{
	LOCALE_TIMING_MENU,
	LOCALE_TIMING_CHANLIST,
	LOCALE_TIMING_EPG,
	LOCALE_TIMING_INFOBAR,
	LOCALE_TIMING_FILEBROWSER,
	LOCALE_TIMING_NUMERICZAP
};

