/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: window.cpp 2016.12.12 11:43:30 mohousch Exp $

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

#include "window.h"


CWindow::CWindow()
{
	init();
}

CWindow::CWindow(const int x, const int y, const int dx, const int dy)
{
	cFrameBox.iX = x;
	cFrameBox.iY = y;
	cFrameBox.iWidth = dx;
	cFrameBox.iHeight = dy;

	init();
}

CWindow::CWindow(CBox* position)
{
	cFrameBox = *position;

	init();
}

CWindow::~CWindow(void)
{
}

void CWindow::init(void)
{
	frameBuffer = CFrameBuffer::getInstance();

	radius = NO_RADIUS;
	corner = CORNER_NONE;
	color = COL_MENUHEAD_PLUS_0;
	gradient = nogradient;

	enableshadow = false;

	savescreen = false;
	background	= NULL;
	full_width = enableshadow? cFrameBox.iWidth + SHADOW_OFFSET : cFrameBox.iWidth;
	full_height = enableshadow? cFrameBox.iHeight + SHADOW_OFFSET : cFrameBox.iHeight;
}

void CWindow::saveScreen()
{
	background = new fb_pixel_t[full_width*full_height];
	
	if(background)
		frameBuffer->SaveScreen(cFrameBox.iX, cFrameBox.iY, full_width, full_height, background);
}

void CWindow::restoreScreen()
{
	if(background) 
	{
		frameBuffer->RestoreScreen(cFrameBox.iX, cFrameBox.iY, full_width, full_height, background);
	}

	delete[] background;
	background = NULL;
}

void CWindow::setDimension(const int x, const int y, const int dx, const int dy)
{
	cFrameBox.iX = x;
	cFrameBox.iY = y;
	cFrameBox.iWidth = dx;
	cFrameBox.iHeight = dy;

	init();
}

void CWindow::setDimension(CBox* position)
{
	cFrameBox = *position;

	init();
}

void CWindow::paint()
{
	if(savescreen) 
		saveScreen();

	// shadow Box
	if(enableshadow)
		frameBuffer->paintBoxRel(cFrameBox.iX + SHADOW_OFFSET, cFrameBox.iY + SHADOW_OFFSET, cFrameBox.iWidth, cFrameBox.iHeight, COL_INFOBAR_SHADOW_PLUS_0, radius, corner);

	// window Box
	frameBuffer->paintBoxRel(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight, color, radius, corner, gradient);
}

void CWindow::hide()
{
	if( savescreen && background)
		restoreScreen();
	else
		frameBuffer->paintBackgroundBoxRel(cFrameBox.iX, cFrameBox.iY, full_width, full_height);
}




