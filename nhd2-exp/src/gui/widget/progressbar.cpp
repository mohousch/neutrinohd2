/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: progressbar.cpp 2013/10/12 mohousch Exp $

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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <global.h>
#include <neutrino.h>

#include <progressbar.h>

#include <string>

#include <time.h>
#include <sys/timeb.h>
#include <sys/param.h>

#include <system/debug.h>


#define ITEMW 1


CProgressBar::CProgressBar(int w, int h, int r, int g, int b, bool inv)
{
	frameBuffer = CFrameBuffer::getInstance();
	
	double div;
	width = w;
	height = h;
	inverse = inv;
	
	div = (double) 100 / (double) width;

	red = (double) r / (double) div / (double) ITEMW;
	green = (double) g / (double) div / (double) ITEMW;
	yellow = (double) b / (double) div / (double) ITEMW;
	
	percent = 255;
}

void CProgressBar::paint(unsigned int x, unsigned int y, unsigned char pcr)
{
	//dprintf(DEBUG_NORMAL, "CProgressBar::paint (x:%d) (y:%d) (pcr:%d)\n", x, y, pcr);

	int i, siglen;
	unsigned int posx;
	unsigned int posy;
	unsigned int xpos;
	unsigned int ypos;

	double div;
	uint32_t rgb;
	
	fb_pixel_t color = COL_MENUCONTENT_PLUS_7;
	int b = 0;
	
	i = 0;
	xpos = x;
	ypos = y;

	frameBuffer->paintBoxRel(x, y, width, height, COL_MENUCONTENT_PLUS_2);	//fill passive
	
	if (pcr != percent) 
	{
		if(percent == 255) 
			percent = 0;

		div = (double) 100 / (double) width;
		siglen = (double) pcr / (double) div;
		posx = xpos;
		posy = ypos;
		int maxi = siglen/ITEMW;
		int total = width/ITEMW;
		int step = 255/total;

		if (pcr > percent) 
		{
			if(g_settings.progressbar_color == 0)
			{
				//red
				for (i = 0; (i < red) && (i < maxi); i++) 
				{
					step = 255/red;

					if(inverse) 
						rgb = COL_GREEN + ((unsigned char)(step*i) << 16); // adding red
					else
						rgb = COL_RED + ((unsigned char)(step*i) <<  8); // adding green
				
					color = rgb;
				
					frameBuffer->paintBoxRel(posx + i*ITEMW, posy, ITEMW, height, color);
				}
	
				//yellow
				for (; (i < yellow) && (i < maxi); i++) 
				{
					step = 255/yellow/2;

					if(inverse) 
						rgb = COL_YELLOW - (((unsigned char)step*(b++)) <<  8); // removing green
					else
						rgb = COL_YELLOW - ((unsigned char)(step*(b++)) << 16); // removing red
	
					color = rgb;		    
				
					frameBuffer->paintBoxRel(posx + i*ITEMW, posy, ITEMW, height, color);
				}

				//green
				for (; (i < green) && (i < maxi); i++) 
				{
					step = 255/green;

					if(inverse) 
						rgb = COL_YELLOW - ((unsigned char) (step*(b++)) <<  8); // removing green
					else
						rgb = COL_YELLOW - ((unsigned char) (step*(b++)) << 16); // removing red
				
					color = rgb;
				
					frameBuffer->paintBoxRel (posx + i*ITEMW, posy, ITEMW, height, color);
				}
			}
			else
			{
				for(; (i < maxi); i++) 
				{
					frameBuffer->paintBoxRel(posx + i*ITEMW, posy, ITEMW, height, color);
				}
			}
		}
		
		percent = pcr;
	}
}

void CProgressBar::reset()
{
  	percent = 255;
}


