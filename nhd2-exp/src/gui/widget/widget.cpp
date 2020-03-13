/*
	$Id: widget.cpp 12.03.2020 mohousch Exp $


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

#include <unistd.h> //acces
#include <cctype>

#include <global.h>
#include <neutrino.h>

#include <driver/rcinput.h>

#include <gui/widget/widget.h>

#include <system/debug.h>


CWidget::CWidget(const int dx, const int dy)
{
	frameBuffer = CFrameBuffer::getInstance();

	mainFrameBox.iWidth = dx;
	mainFrameBox.iHeight = dy;

	full_width = mainFrameBox.iWidth;
	full_height = mainFrameBox.iHeight;

	mainFrameBox.iX = frameBuffer->getScreenX() + ((frameBuffer->getScreenWidth() - full_width ) >> 1 );
	mainFrameBox.iY = frameBuffer->getScreenY() + ((frameBuffer->getScreenHeight() - full_height) >> 1 );

	savescreen = false;
	background = NULL;
}

CWidget::~CWidget()
{
	dprintf(DEBUG_NORMAL, "CWidget:: del\n");

	items.clear();
}

void CWidget::addItem(CWidgetItem *widgetItem, const int x, const int y, const int dx, const int dy, const bool defaultselected)
{
	if (defaultselected)
		selected = items.size();
	
	items.push_back(widgetItem);
}

bool CWidget::hasItem()
{
	return !items.empty();
}

void CWidget::paintItems()
{
	dprintf(DEBUG_NORMAL, "CWidget:: paintItems\n");
}

void CWidget::paint()
{
	dprintf(DEBUG_NORMAL, "CWidget:: paint\n");

	frameBuffer->paintBoxRel(mainFrameBox.iX, mainFrameBox.iY, mainFrameBox.iWidth, mainFrameBox.iHeight, COL_MENUCONTENT_PLUS_0);

	paintItems();
}


void CWidget::hide()
{
	dprintf(DEBUG_NORMAL, "CWidget:: hide\n");

	frameBuffer->paintBackgroundBoxRel(mainFrameBox.iX, mainFrameBox.iY, mainFrameBox.iWidth, mainFrameBox.iHeight);
}

int CWidget::exec(CMenuTarget *parent, const std::string &actionKey)
{
	dprintf(DEBUG_NORMAL, "CWidget:: exec\n");

	paint();
	
	g_RCInput->messageLoop();

	hide();

	return menu_return::RETURN_REPAINT;
}



