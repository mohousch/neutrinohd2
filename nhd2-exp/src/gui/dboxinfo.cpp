/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: dboxinfo.cpp 2013/10/12 mohousch Exp $

	Copyright (C) 2001 Steffen Hehn 'McClean'
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <gui/dboxinfo.h>

#include <global.h>

#include <driver/fontrenderer.h>
#include <driver/screen_max.h>
#include <driver/rcinput.h>

#include <sys/sysinfo.h>
#include <sys/vfs.h>

/*zapit includes*/
#include <frontend_c.h>

#include <system/debug.h>


static const int FSHIFT = 16;              /* nr of bits of precision */
#define FIXED_1         (1<<FSHIFT)     /* 1.0 as fixed-point */
#define LOAD_INT(x) ((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1-1)) * 100)

extern int FrontendCount;
CFrontend * getFE(int index);

// hdd
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/mount.h>


#define SHADOW_OFFSET	5

static int my_filter(const struct dirent * dent)
{
	if(dent->d_name[0] == 's' && dent->d_name[1] == 'd')
		return 1;
	
	return 0;
}

CDBoxInfoWidget::CDBoxInfoWidget()
{
	frameBuffer = CFrameBuffer::getInstance();
	
	// Title
	titleIcon.setIcon(NEUTRINO_ICON_INFO);
	cFrameBoxTitle.iHeight = std::max(titleIcon.iHeight, g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight()) + 6;

	// Body
	cFrameBoxItem.iHeight = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getHeight();
	
	cFrameBox.iWidth = w_max(MENU_WIDTH + 100, 0);
	cFrameBox.iHeight = h_max(cFrameBoxTitle.iHeight + (10 + FrontendCount)*cFrameBoxItem.iHeight + 20, 0);
	
    	cFrameBox.iX = (((g_settings.screen_EndX - g_settings.screen_StartX) - cFrameBox.iWidth) / 2) + g_settings.screen_StartX;
	cFrameBox.iY = (((g_settings.screen_EndY - g_settings.screen_StartY) - cFrameBox.iHeight) / 2) + g_settings.screen_StartY;
}

int CDBoxInfoWidget::exec(CMenuTarget * parent, const std::string &)
{
	dprintf(DEBUG_NORMAL, "CDBoxInfoWidget::exec:\n");

	if (parent)
		parent->hide();
	
	paint();
		
	frameBuffer->blit();

	int res = g_RCInput->messageLoop();

	hide();	

	return res;
}

void CDBoxInfoWidget::hide()
{
	frameBuffer->paintBackgroundBoxRel(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth + SHADOW_OFFSET, cFrameBox.iHeight + SHADOW_OFFSET);
	
	frameBuffer->blit();
}

void CDBoxInfoWidget::paint()
{
	dprintf(DEBUG_NORMAL, "CDBoxInfoWidget::paint:\n");

	int ypos = cFrameBox.iY;
	int i = 0;
	
	// Shadow
	cFrameBoxShadow.iX = cFrameBox.iX + SHADOW_OFFSET;
	cFrameBoxShadow.iY = cFrameBox.iY + SHADOW_OFFSET;
	cFrameBoxShadow.iWidth = cFrameBox.iWidth;
	cFrameBoxShadow.iHeight = cFrameBox.iHeight;

	frameBuffer->paintBoxRel(cFrameBoxShadow.iX, cFrameBoxShadow.iY, cFrameBoxShadow.iWidth, cFrameBoxShadow.iHeight, COL_INFOBAR_SHADOW_PLUS_0, RADIUS_MID, CORNER_BOTH);

	// head
	cFrameBoxTitle.iX = cFrameBox.iX;
	cFrameBoxTitle.iY = cFrameBox.iY;
	cFrameBoxTitle.iWidth = cFrameBox.iWidth;

	// Head Box
	frameBuffer->paintBoxRel(cFrameBoxTitle.iX, cFrameBoxTitle.iY, cFrameBoxTitle.iWidth, cFrameBoxTitle.iHeight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, g_settings.Head_gradient);

	frameBuffer->paintIcon(titleIcon.iconName.c_str(), cFrameBoxTitle.iX + BORDER_LEFT, cFrameBoxTitle.iY + (cFrameBoxTitle.iHeight - titleIcon.iHeight)/2);
	
	// Head title
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(cFrameBoxTitle.iX + BORDER_LEFT + titleIcon.iWidth + ICON_OFFSET, cFrameBoxTitle.iY + cFrameBoxTitle.iHeight, cFrameBoxTitle.iWidth - BORDER_LEFT - titleIcon.iWidth - ICON_OFFSET - BORDER_RIGHT, "Box info", COL_MENUHEAD, 0, true); // UTF-8

	// Foot/Body
	cFrameBoxBody.iX = cFrameBox.iX;
	cFrameBoxBody.iY = cFrameBox.iY + cFrameBoxTitle.iHeight;
	cFrameBoxBody.iWidth = cFrameBox.iWidth;
	cFrameBoxBody.iHeight = cFrameBox.iHeight - cFrameBoxTitle.iHeight;

	frameBuffer->paintBoxRel(cFrameBoxBody.iX, cFrameBoxBody.iY, cFrameBoxBody.iWidth, cFrameBoxBody.iHeight, COL_MENUCONTENT_PLUS_0, RADIUS_MID, CORNER_BOTTOM);

	// Items
	ypos += cFrameBoxTitle.iHeight + (cFrameBoxItem.iHeight >>1);

	//cpu
	FILE* fd = fopen("/proc/cpuinfo", "rt");

	if (fd == NULL) 
	{
		printf("error while opening proc-cpuinfo\n" );
	} 
	else 
	{
		char *buffer=NULL;
		size_t len = 0;
		ssize_t read;
		while ((read = getline(&buffer, &len, fd)) != -1) 
		{
			if (!(strncmp(const_cast<char *>("Hardware"), buffer, 8))) 
			{
				char *t = rindex(buffer, '\n');
				if (t)
					*t = '\0';

				std::string hw;
				char *p = rindex(buffer, ':');
				if (p)
					hw = ++p;
				hw += " Info";

				g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(cFrameBoxBody.iX + BORDER_LEFT, cFrameBoxBody.iY, cFrameBoxBody.iWidth - BORDER_LEFT - BORDER_RIGHT, hw.c_str(), COL_MENUCONTENTINACTIVE, 0, true); // UTF-8
				break;
			}
			i++;
			if (i > 4)
				continue;

			g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(cFrameBoxBody.iX + BORDER_LEFT, ypos + cFrameBoxItem.iHeight, cFrameBoxBody.iWidth - BORDER_LEFT - BORDER_RIGHT, buffer, COL_MENUCONTENTINACTIVE, true);
			ypos += cFrameBoxItem.iHeight;
		}
		fclose(fd);

		if (buffer)
			free(buffer);
	}
	
	// separator
	frameBuffer->paintHLineRel(cFrameBoxBody.iX + BORDER_LEFT, cFrameBoxBody.iWidth - BORDER_LEFT - BORDER_RIGHT, ypos + (cFrameBoxItem.iHeight >> 1), COL_MENUCONTENTDARK_PLUS_0 );
	frameBuffer->paintHLineRel(cFrameBoxBody.iX + BORDER_LEFT, cFrameBoxBody.iWidth - BORDER_LEFT - BORDER_RIGHT, ypos + (cFrameBoxItem.iHeight >> 1) + 1, COL_MENUCONTENTDARK_PLUS_0 );

	// up time
	ypos += cFrameBoxItem.iHeight/2;
	
	int updays, uphours, upminutes;
	struct sysinfo info;
	struct tm *current_time;
	time_t current_secs;
	char ubuf[256], sbuf[256];

	memset(sbuf, 0, 256);
	time(&current_secs);
	current_time = localtime(&current_secs);

	sysinfo(&info);

	sprintf( ubuf, "%2d:%02d%s  up ", 
			current_time->tm_hour%12 ? current_time->tm_hour%12 : 12, 
			current_time->tm_min, current_time->tm_hour > 11 ? "pm" : "am");

	strcat(sbuf, ubuf);
	updays = (int) info.uptime / (60*60*24);
	if (updays) 
	{
		sprintf(ubuf, "%d day%s, ", updays, (updays != 1) ? "s" : "");
		strcat(sbuf, ubuf);
	}
	upminutes = (int) info.uptime / 60;
	uphours = (upminutes / 60) % 24;
	upminutes %= 60;
	if(uphours)
		sprintf(ubuf,"%2d:%02d, ", uphours, upminutes);
	else
		sprintf(ubuf,"%d min, ", upminutes);
	strcat(sbuf, ubuf);

	sprintf(ubuf, "load: %ld.%02ld, %ld.%02ld, %ld.%02ld\n", 
			LOAD_INT(info.loads[0]), LOAD_FRAC(info.loads[0]), 
			LOAD_INT(info.loads[1]), LOAD_FRAC(info.loads[1]), 
			LOAD_INT(info.loads[2]), LOAD_FRAC(info.loads[2]));

	strcat(sbuf, ubuf);
	
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(cFrameBoxBody.iX + BORDER_LEFT, ypos + cFrameBoxItem.iHeight, cFrameBoxBody.iWidth - BORDER_LEFT - BORDER_RIGHT, sbuf, COL_MENUCONTENT, true);
	
	// mem
	ypos += cFrameBoxItem.iHeight;

	sprintf(ubuf, "memory total %dKb, free %dKb", (int) info.totalram/1024, (int) info.freeram/1024);
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(cFrameBoxBody.iX + BORDER_LEFT, ypos + cFrameBoxItem.iHeight, cFrameBoxBody.iWidth - BORDER_LEFT - BORDER_RIGHT, ubuf, COL_MENUCONTENT, true);
	
	// separator
	ypos += cFrameBoxItem.iHeight/2;
	frameBuffer->paintHLineRel(cFrameBoxBody.iX + BORDER_LEFT, cFrameBoxBody.iWidth - BORDER_LEFT - BORDER_RIGHT, ypos + (cFrameBoxItem.iHeight >> 1), COL_MENUCONTENTDARK_PLUS_0 );
	frameBuffer->paintHLineRel(cFrameBoxBody.iX + BORDER_LEFT, cFrameBoxBody.iWidth - BORDER_LEFT - BORDER_RIGHT, ypos + (cFrameBoxItem.iHeight >> 1) + 1, COL_MENUCONTENTDARK_PLUS_0 );
    	
    	//hdd devices
	FILE * f;
	int fd_hdd;
	struct dirent **namelist;
	
	int n = scandir("/sys/block", &namelist, my_filter, alphasort);
	
	for(int i1 = 0; i1 < n; i1++) 
	{
		ypos += cFrameBoxItem.iHeight;
		
		char str[256];
		char vendor[128];
		char model[128];
		int64_t bytes;
		int64_t megabytes;
		int removable = 0;
		
		sprintf(str, "/dev/%s", namelist[i1]->d_name);
		fd_hdd = open(str, O_RDONLY);

		if(fd_hdd < 0) 
		{
			//printf("[neutrino] HDD: Cant open %s\n", str);
			continue;
		}
		
		if (ioctl(fd_hdd, BLKGETSIZE64, &bytes))
			perror("BLKGETSIZE64");
                
                close(fd_hdd);

		megabytes = bytes/1000000;

		// vendor
		sprintf(str, "/sys/block/%s/device/vendor", namelist[i1]->d_name);
		f = fopen(str, "r");

		if(!f) 
		{
			continue;
		}
		fscanf(f, "%s", vendor);
		fclose(f);

		// model
		sprintf(str, "/sys/block/%s/device/model", namelist[i1]->d_name);
		f = fopen(str, "r");
		
		if(!f) 
		{
			continue;
		}
		fscanf(f, "%s", model);
		fclose(f);

		// removable
		sprintf(str, "/sys/block/%s/removable", namelist[i1]->d_name);
		f = fopen(str, "r");
		
		if(!f) 
		{
			continue;
		}
		fscanf(f, "%d", &removable);
		fclose(f);
		
		// free space on hdd
		struct statfs s;
		
		if (::statfs(g_settings.network_nfs_recordingdir, &s) == 0) 
		{
			sprintf(str, "%s (%s-%s %lld %s), free %ldMB", namelist[i1]->d_name, vendor, model, megabytes < 10000 ? megabytes : megabytes/1000, megabytes < 10000 ? "MB" : "GB", (long)( ((s.f_bfree/1024)/1024))*s.f_bsize);
		}
		else
			sprintf(str, "%s (%s-%s %lld %s)", namelist[i1]->d_name, vendor, model, megabytes < 10000 ? megabytes : megabytes/1000, megabytes < 10000 ? "MB" : "GB");
		
		free(namelist[i1]);
		
		int offset = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth((char *)"HDD: ", true); // UTF-8
		
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(cFrameBoxBody.iX + BORDER_LEFT, ypos + cFrameBoxItem.iHeight, cFrameBoxBody.iWidth - BORDER_LEFT - BORDER_RIGHT, (char *)"HDD:", COL_MENUCONTENTINACTIVE, true);
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(cFrameBoxBody.iX + BORDER_LEFT + offset, ypos + cFrameBoxItem.iHeight, cFrameBoxBody.iWidth - BORDER_LEFT - BORDER_RIGHT, str, COL_MENUCONTENT, true);
	}
	
	//frontend
	ypos += cFrameBoxItem.iHeight/2;
	
	for(int i2 = 0; i2 < FrontendCount; i2++)
	{
		CFrontend * fe = getFE(i2);
		ypos += cFrameBoxItem.iHeight;
		char tbuf[255];
		char tbuf1[255];
		
		sprintf(tbuf, "Tuner-%d: ", i2 + 1);
		int offset1 = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->getRenderWidth(tbuf, true); // UTF-8
		
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(cFrameBoxBody.iX + BORDER_LEFT, ypos + cFrameBoxItem.iHeight, cFrameBoxBody.iWidth - BORDER_LEFT - BORDER_RIGHT, tbuf, COL_MENUCONTENTINACTIVE, true);
		
		sprintf(tbuf1, "%s", fe->getInfo()->name);
		g_Font[SNeutrinoSettings::FONT_TYPE_MENU]->RenderString(cFrameBoxBody.iX + BORDER_LEFT + offset1, ypos + cFrameBoxItem.iHeight, cFrameBoxBody.iWidth - BORDER_LEFT - BORDER_RIGHT, tbuf1, COL_MENUCONTENT, true);
	}	
}
