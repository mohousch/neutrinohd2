/*
	Neutrino-GUI  -   DBoxII-Project

	Timerliste by Zwen
	
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
#include <neutrino.h>
#include <gui/timeosd.h>
#include <driver/fontrenderer.h>
#include <system/settings.h>
#include <gui/widget/progressbar.h>
#include <driver/rcinput.h>

#include <system/debug.h>


#define TIMEOSD_FONT 		SNeutrinoSettings::FONT_TYPE_INFOBAR_CHANNAME
#define TIMEBARH 		38
#define SHADOW_OFFSET		5


CTimeOSD::CTimeOSD()
{
	frameBuffer = CFrameBuffer::getInstance();
	visible = false;
	m_mode = MODE_ASC;
	GetDimensions();
}

CTimeOSD::~CTimeOSD()
{
	hide();
}

void CTimeOSD::show(int /*Position*/)
{
	dprintf(DEBUG_NORMAL, "CTimeOSD::show\n");
	 
	// show / update
	GetDimensions();
	
	visible = true;
}

void CTimeOSD::GetDimensions()
{
	// time
	m_xstart = g_settings.screen_StartX + 10;
	m_xend = g_settings.screen_EndX - 10;
	m_height = g_Font[TIMEOSD_FONT]->getHeight();
	m_y = g_settings.screen_StartY + 10;
	m_width = g_Font[TIMEOSD_FONT]->getRenderWidth("00:00:00");
	twidth = m_xend - m_xstart;
	
	// infobar
	BoxStartX = m_xstart;
	BoxWidth = m_xend - m_xstart;
	BoxHeight = TIMEBARH * 3;
	BoxStartY = g_settings.screen_EndY - BoxHeight - 10;
	BoxEndY = BoxStartY + BoxHeight;
	BoxEndX = m_xend;
}

void CTimeOSD::update(time_t time_show)
{
	time_t tDisplayTime;
	static time_t oldDisplayTime = 0;
	char cDisplayTime[8 + 1];
	fb_pixel_t color1, color2;
	
	GetDimensions();

	//printf("CTimeOSD::update time %ld\n", time_show);
	
	if(!visible)
		return;

	if(m_mode == MODE_ASC) 
	{
		color1 = COL_MENUCONTENT_PLUS_0;
		color2 = COL_MENUCONTENT;
	} 
	else 
	{
		color1 = COL_MENUCONTENTSELECTED_PLUS_0;
		color2 = COL_MENUCONTENTSELECTED;
		
		if(!time_show) 
			time_show = 1;
	}

	if(time_show) 
	{
		m_time_show = time_show;
		tDisplayTime = m_time_show;
	} 
	else 
	{
		if(m_mode == MODE_ASC) 
		{
			tDisplayTime = m_time_show + (time(NULL) - m_time_dis);
		} 
		else 
		{
			tDisplayTime = m_time_show + (m_time_dis - time(NULL));
		}
	}

	if(tDisplayTime < 0)
		tDisplayTime = 0;

	if(tDisplayTime != oldDisplayTime) 
	{
		oldDisplayTime = tDisplayTime;
		strftime(cDisplayTime, 9, "%T", gmtime(&tDisplayTime));
		
		// time shadow
		frameBuffer->paintBoxRel(m_xend - m_width - 10 + SHADOW_OFFSET, m_y + SHADOW_OFFSET, m_width + 10, m_height, COL_INFOBAR_SHADOW_PLUS_0);

		// time window
		frameBuffer->paintBoxRel(m_xend - m_width - 10, m_y, m_width + 10, m_height, color1 );

		// time
		g_Font[TIMEOSD_FONT]->RenderString(m_xend - m_width - 5, m_y + m_height, m_width + 5, cDisplayTime, color2);
	}
	
	frameBuffer->blit();
}

void CTimeOSD::hide()
{
	GetDimensions();
	
	//printf("CTimeOSD::hide: x %d y %d xend %d yend %d\n", m_xstart, m_y , m_xend, m_height + 15);

	if(!visible)
		return;

	// hide time
	frameBuffer->paintBackgroundBoxRel(m_xend - m_width - 10, m_y, m_width + 10 + SHADOW_OFFSET, m_height + SHADOW_OFFSET );
	frameBuffer->blit();

	visible = false;
}
