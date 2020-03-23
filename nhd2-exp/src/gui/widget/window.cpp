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

#include <global.h>

#include <gui/widget/window.h>

#include <video_cs.h>


extern cVideo * videoDecoder;

CWindow::CWindow()
{
	cFrameBox.iX = frameBuffer->getScreenX() + 10;
	cFrameBox.iY = frameBuffer->getScreenX() + 10;
	cFrameBox.iWidth = frameBuffer->getScreenWidth() - 20;
	cFrameBox.iHeight = frameBuffer->getScreenWidth() - 20;

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
	bgcolor = COL_MENUHEAD_PLUS_0;
	gradient = nogradient;

	enableshadow = false;

	savescreen = false;
	background = NULL;

	full_width = cFrameBox.iWidth;
	full_height = cFrameBox.iHeight;

	itemType = WIDGET_ITEM_WINDOW;
}

void CWindow::saveScreen()
{
	full_width = enableshadow? cFrameBox.iWidth + 2 : cFrameBox.iWidth;
	full_height = enableshadow? cFrameBox.iHeight + 2 : cFrameBox.iHeight;

	background = new fb_pixel_t[full_width*full_height];
	
	if(background)
		frameBuffer->saveScreen(enableshadow? cFrameBox.iX - 1 : cFrameBox.iX, enableshadow? cFrameBox.iY - 1 : cFrameBox.iY, full_width, full_height, background);
}

void CWindow::restoreScreen()
{
	full_width = enableshadow? cFrameBox.iWidth + 2 : cFrameBox.iWidth;
	full_height = enableshadow? cFrameBox.iHeight + 2 : cFrameBox.iHeight;

	if(background) 
	{
		frameBuffer->restoreScreen(enableshadow? cFrameBox.iX - 1 : cFrameBox.iX, enableshadow? cFrameBox.iY - 1 : cFrameBox.iY, full_width, full_height, background);
	}

	delete[] background;
	background = NULL;
}

void CWindow::setPosition(const int x, const int y, const int dx, const int dy)
{
	cFrameBox.iX = x;
	cFrameBox.iY = y;
	cFrameBox.iWidth = dx;
	cFrameBox.iHeight = dy;

	init();
}

void CWindow::setPosition(CBox* position)
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
		frameBuffer->paintBoxRel(cFrameBox.iX - 1, cFrameBox.iY - 1, cFrameBox.iWidth + 2, cFrameBox.iHeight + 2, COL_MENUCONTENT_PLUS_6);

	// window Box
	frameBuffer->paintBoxRel(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight, bgcolor, enableshadow? NO_RADIUS : radius, enableshadow? CORNER_NONE : corner, gradient);
}

void CWindow::hide()
{
	if( savescreen && background)
		restoreScreen();
	else
		frameBuffer->paintBackgroundBoxRel(enableshadow?cFrameBox.iX - 1 : cFrameBox.iX, enableshadow?cFrameBox.iY - 1 : cFrameBox.iY, full_width, full_height);
}

// pig
CPig::CPig(const int x, const int y, const int dx, const int dy)
{
	cFrameBox.iX = x;
	cFrameBox.iY = y;
	cFrameBox.iWidth = dx;
	cFrameBox.iHeight = dy;

	init();
}

CPig::CPig(CBox* position)
{
	cFrameBox = *position;

	init();
}

CPig::~CPig(void)
{
}

void CPig::init(void)
{
	frameBuffer = CFrameBuffer::getInstance();

	itemType = WIDGET_ITEM_PIG;
}

void CPig::paint()
{
	frameBuffer->paintBackgroundBoxRel(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight);	
		

	if(videoDecoder)
		videoDecoder->Pig(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight);	
}

void CPig::hide()
{
	if(videoDecoder)  
		videoDecoder->Pig(-1, -1, -1, -1);

	frameBuffer->paintBackgroundBoxRel(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight);
}

// grid
CGrid::CGrid(const int x, const int y, const int dx, const int dy)
{
	cFrameBox.iX = x;
	cFrameBox.iY = y;
	cFrameBox.iWidth = dx;
	cFrameBox.iHeight = dy;

	init();
}

CGrid::CGrid(CBox* position)
{
	cFrameBox = *position;

	init();
}

CGrid::~CGrid(void)
{
}

void CGrid::init(void)
{
	frameBuffer = CFrameBuffer::getInstance();

	rgb = 0x505050;

	itemType = WIDGET_ITEM_GRID;
}

void CGrid::paint()
{
	// hlines grid
	for(int count = 0; count < cFrameBox.iHeight; count += 15)
		frameBuffer->paintHLine(cFrameBox.iX, cFrameBox.iX + cFrameBox.iWidth, cFrameBox.iY + count, make16color(rgb) );

	// vlines grid
	for(int count = 0; count < cFrameBox.iWidth; count += 15)
		frameBuffer->paintVLine(cFrameBox.iX + count, cFrameBox.iY, cFrameBox.iY + cFrameBox.iHeight, make16color(rgb) );
}

void CGrid::hide()
{
	frameBuffer->paintBackgroundBoxRel(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight);
}





