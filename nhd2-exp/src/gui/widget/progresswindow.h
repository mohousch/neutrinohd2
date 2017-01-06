/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: progresswindow.cpp 2013/10/12 mohousch Exp $

	Copyright (C) 2001 Steffen Hehn 'McClean'
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


#ifndef __progresswindow__
#define __progresswindow__

#include <string>

#include <driver/framebuffer.h>
#include <driver/fb_window.h>
#include <system/localize.h>

#include "menue.h"
#include "progressbar.h"


class CProgressWindow : public CMenuTarget
{
	protected:

		CFBWindow* frameBuffer;
		neutrino_locale_t caption;
		std::string captionString;

		int x;
		int y;
		int width;
		int height;
		int hheight; // head font height
		int mheight; // menu font height
		unsigned int global_progress;
		int globalstatusX;
		int globalstatusY;
		int statusTextY;
		std::string statusText;
		
		CProgressBar* progressBar;

		virtual void paint();

	public:

		CProgressWindow();
		void setTitle(const neutrino_locale_t title);
		void setTitle(const char * const title);
		void hide();

		int exec( CMenuTarget* parent, const std::string & actionKey );

		void showGlobalStatus(const unsigned int prog);
		unsigned int getGlobalStatus(void);
		void showStatusMessageUTF(const std::string & text); // UTF-8
};

#endif
