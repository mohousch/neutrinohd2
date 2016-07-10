/*
  $Id: systeminfo.cpp 2014/01/22 mohousch Exp $

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

#include <systeminfo.h>


extern "C" void plugin_exec(void);
extern "C" void plugin_init(void);
extern "C" void plugin_del(void);

bool refreshIt = true;

// construktor
CSysInfoWidget::CSysInfoWidget(int m)
{
	frameBuffer = CFrameBuffer::getInstance();
	
	// windows size
	width  = w_max ( (frameBuffer->getScreenWidth() / 20 * 17), (frameBuffer->getScreenWidth() / 20 ));
	height = h_max ( (frameBuffer->getScreenHeight() / 20 * 16), (frameBuffer->getScreenHeight() / 20));
	
	//head height
	frameBuffer->getIconSize(NEUTRINO_ICON_SETTINGS, &icon_head_w, &icon_head_h);
	theight = std::max(g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), icon_head_h) + 6;
       
	//foot height
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_RED, &icon_foot_w, &icon_foot_h);
	ButtonHeight = std::max(g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight(), icon_foot_h) + 10;
	
	// coordinate
	x = (((g_settings.screen_EndX- g_settings.screen_StartX)-width) / 2) + g_settings.screen_StartX;
	y = (((g_settings.screen_EndY- g_settings.screen_StartY)-height) / 2) + g_settings.screen_StartY;
	
	mode = m;

	//
	int mode = CTextBox::SCROLL;
	sys_BoxFrameText.iX = x;
	sys_BoxFrameText.iY = y + theight;
	sys_BoxFrameText.iWidth = width;
	sys_BoxFrameText.iHeight = height - theight - ButtonHeight;

	sys_textBox = new CTextBox("", g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST], mode, &sys_BoxFrameText);
}

CSysInfoWidget::~CSysInfoWidget()
{
}

// paintlistbox
void CSysInfoWidget::paint()
{
	// paint
	sys_textBox->paint();

	// settext
	sys_textBox->setText(&buffer);
}

// paint head
void CSysInfoWidget::paintHead()
{
	char buf[100];

	frameBuffer->paintBoxRel(x, y, width, theight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, true, gradientLight2Dark);
	
	// icon
	frameBuffer->paintIcon(NEUTRINO_ICON_SETTINGS, x + BORDER_LEFT, y + (theight - icon_head_h)/2);
	
	if(mode == SYSINFO)
		sprintf((char *) buf, "%s", "System-Info:");
	
	if(mode == DMESGINFO)
		sprintf((char *) buf, "%s", "System-Messages:");
	
	if(mode == CPUINFO)
		sprintf((char *) buf, "%s", "CPU/File-Info:");
	
	if(mode == PSINFO)
		sprintf((char *) buf, "%s", "Prozess-Liste:");
	
	// title
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(x + BORDER_LEFT + icon_head_w + ICON_OFFSET, y + theight, width, buf, COL_MENUHEAD);
}

// paint foot
const struct button_label Buttons[4] =
{
	{ NEUTRINO_ICON_BUTTON_RED, NONEXISTANT_LOCALE, "info" },
	{ NEUTRINO_ICON_BUTTON_GREEN, NONEXISTANT_LOCALE, "dmesg" },
	{ NEUTRINO_ICON_BUTTON_YELLOW, NONEXISTANT_LOCALE, "cpu/file" },
	{ NEUTRINO_ICON_BUTTON_BLUE, NONEXISTANT_LOCALE, "ps" },
	
};

void CSysInfoWidget::paintFoot()
{
	int ButtonWidth = (width - 28) / 4;

	frameBuffer->paintBoxRel(x, y + height - ButtonHeight, width, ButtonHeight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_BOTTOM, true, gradientDark2Light);

	::paintButtons(frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, x + BORDER_LEFT, y + height - ButtonHeight, ButtonWidth, 4, Buttons, ButtonHeight);
}

// hide
void CSysInfoWidget::hide()
{
	frameBuffer->paintBackgroundBoxRel(x, y, width, height + ButtonHeight);
	
	frameBuffer->blit();

	if(sys_textBox != NULL)
	{
		delete sys_textBox;
		sys_textBox = NULL;
	}
}

// main
int CSysInfoWidget::exec(CMenuTarget *parent, const std::string &/*actionKey*/)
{
	dprintf(DEBUG_NORMAL, "CsysInfoWidget::exec:\n");

	int res = menu_return::RETURN_REPAINT;

	if(mode == SYSINFO)
	{
		sysinfo();
	}
	else if(mode == DMESGINFO)
	{
		dmesg();
	}
	else if(mode == CPUINFO)
	{
		cpuinfo();
	}
	else if(mode == PSINFO)
	{
		ps();
	}

	if (parent)
		parent->hide();

	paintHead();
	paint();
	paintFoot();
	
	frameBuffer->blit();

	neutrino_msg_t msg; 
	neutrino_msg_data_t data;
	int timercount = 0;
	unsigned long long timeoutEnd = g_RCInput->calcTimeoutEnd(5);

	while (msg != (neutrino_msg_t) g_settings.key_channelList_cancel)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if (msg <= CRCInput::RC_MaxRC  ) 
			timeoutEnd = g_RCInput->calcTimeoutEnd(5);
		
		if (msg == CRCInput::RC_timeout)
		{
			if (mode == SYSINFO)
			{
				timercount = 0;
				sysinfo();

				paintHead();
				paint();
				paintFoot();
			}
			
			if ((mode == DMESGINFO) && (++timercount>11))
			{
				timercount = 0;
				dmesg();

				paintHead();
				paint();
				paintFoot();
			}
			
			if ((mode == PSINFO)&&(refreshIt == true))
			{
				timercount = 0;
				ps();

				paintHead();
				paint();
				paintFoot();
			}

			timeoutEnd = g_RCInput->calcTimeoutEnd(5);
			g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );
		}
		
		if ( ((int) msg == g_settings.key_channelList_pageup) && (mode != SYSINFO))
		{
			sys_textBox->scrollPageUp(1);
		}
		else if (((int) msg == g_settings.key_channelList_pagedown) && (mode != SYSINFO))
		{
			sys_textBox->scrollPageDown(1);
		}
		else if ((msg == CRCInput::RC_red) && (mode != SYSINFO))
		{
			mode = SYSINFO;
			sysinfo();

			paintHead();
			paint();
			paintFoot();

		}
		else if ((msg == CRCInput::RC_green) && (mode != DMESGINFO))
		{
			mode = DMESGINFO;
			timercount = 0;
			dmesg();

			paintHead();
			paint();
			paintFoot();
		}
		else if ((msg == CRCInput::RC_yellow) && (mode != CPUINFO))
		{
			mode = CPUINFO;
			cpuinfo();

			paintHead();
			paint();
			paintFoot();
		}
		else if (msg == CRCInput::RC_blue)
		{
			mode = PSINFO;
			ps();

			paintHead();
			paint();
			paintFoot();
		}
		else
		{
			CNeutrinoApp::getInstance()->handleMsg( msg, data );
			// kein canceling...
		}

		frameBuffer->blit();	
	}
	
	hide();
	
	return res;
}

void CSysInfoWidget::sysinfo()
{
	static long curCPU[5] = {0, 0, 0, 0, 0};
	static long prevCPU[5] = {0, 0, 0, 0, 0};
	double value[5] = {0, 0, 0, 0, 0};
	float faktor;
	int i, j = 0;
	char strhelp[6];
	FILE *f;
	char line[256];
	const char *fmt = " %a %d %b %Y %H:%M";
	long t;

	/* Get and Format the SystemTime */
	t = time(NULL);
	struct tm *tp;
	tp = localtime(&t);
	strftime(line, sizeof(line), fmt, tp);
	/* Get and Format the SystemTime end */

	/* Create tmpfile with date /tmp/sysinfo */
	system("echo 'DATUM:' > /tmp/sysinfo");
	f=fopen("/tmp/sysinfo","a");
	if(f)
		fprintf(f,"%s\n", line);
	fclose(f);
	/* Create tmpfile with date /tmp/sysinfo end */

	/* Get the statistics from /proc/stat */
	if(prevCPU[0] == 0)
	{
		f=fopen("/proc/stat","r");
		if(f)
		{
			fgets(line, 256, f); /* cpu */
			sscanf(line,"cpu %lu %lu %lu %lu", &prevCPU[1], &prevCPU[2], &prevCPU[3], &prevCPU[4]);
			for(i = 1; i < 5; i++)
				prevCPU[0] += prevCPU[i];
		}
		fclose(f);
		sleep(1);
	}
	else
	{
		for(i=0;i<5;i++)
				prevCPU[i]=curCPU[i];
	}

	while(((curCPU[0]-prevCPU[0]) < 100) || (curCPU[0]==0))
	{
		f=fopen("/proc/stat","r");
		if(f)
		{
			curCPU[0]=0;
			fgets(line,256,f); /* cpu */
			sscanf(line,"cpu %lu %lu %lu %lu",&curCPU[1],&curCPU[2],&curCPU[3],&curCPU[4]);
			for(i=1;i<5;i++)
				curCPU[0]+=curCPU[i];
		}
		fclose(f);
		if((curCPU[0]-prevCPU[0])<100)
			sleep(1);
	}
	
	// some calculations
	if(!(curCPU[0] - prevCPU[0])==0)
	{
		faktor = 100.0/(curCPU[0] - prevCPU[0]);
		for(i=0;i<4;i++)
			value[i]=(curCPU[i]-prevCPU[i])*faktor;

		value[4]=100.0-value[1]-value[2]-value[3];

		f=fopen("/tmp/sysinfo","a");
		if(f)
		{
			memset(line,0x20,sizeof(line));
			for(i=1, j=0;i<5;i++)
			{
				memset(strhelp,0,sizeof(strhelp));
				sprintf(strhelp,"%.1f", value[i]);
				memcpy(&line[(++j*7)-2-strlen(strhelp)], &strhelp[0], strlen(strhelp));
				memcpy(&line[(j*7)-2], "%", 1);
			}
			line[(j*7)-1]='\0';
			fprintf(f,"\nPERFORMANCE:\n USER:  NICE:   SYS:  IDLE:\n%s\n", line);
		}
		fclose(f);
	}
	/* Get the statistics from /proc/stat end*/

	/* Get kernel-info from /proc/version*/
	f=fopen("/proc/version","r");
	if(f)
	{
		char* token;
		fgets(line,256,f); // version
		token = strstr(line,") (");
		if(token != NULL)
			*++token = 0x0;
		fclose(f);
		f=fopen("/tmp/sysinfo","a");
		fprintf(f, "\nKERNEL:\n %s\n %s\n", line, ++token);
	}
	fclose(f);
	/* Get kernel-info from /proc/version end*/

	/* Get uptime-info from /proc/uptime*/
	f=fopen("/proc/uptime","r");
	if(f)
	{
		fgets(line,256,f);
		float ret[4];
		const char* strTage[2] = {"Tage", "Tag"};
		const char* strStunden[2] = {"Stunden", "Stunde"};
		const char* strMinuten[2] = {"Minuten", "Minute"};
		sscanf(line,"%f",&ret[0]);
		ret[0]/=60;
		ret[1]=long(ret[0])/60/24; // Tage
		ret[2]=long(ret[0])/60-long(ret[1])*24; // Stunden
		ret[3]=long(ret[0])-long(ret[2])*60-long(ret[1])*60*24; // Minuten
		fclose(f);

		f=fopen("/tmp/sysinfo","a");
		if(f)
			fprintf(f, "UPTIME:\n System laeuft seit: %.0f %s %.0f %s %.0f %s\n", ret[1], strTage[int(ret[1])==1], ret[2], strStunden[int(ret[2])==1], ret[3], strMinuten[int(ret[3])==1]);
	}
	fclose(f);
	/* Get uptime-info from /proc/uptime end*/

	readList();
}

void CSysInfoWidget::cpuinfo()
{
	char Wert1[30];
	char Wert2[10];
	char Wert3[10];
	char Wert4[10];
	char Wert5[6];
	char Wert6[30];

	FILE *f,*w;
	char line[256];
	int i = 0;
	
	/* Get file-info from /proc/cpuinfo*/
	system("df > /tmp/systmp");
	f=fopen("/tmp/systmp","r");
	if(f)
	{
		w=fopen("/tmp/sysinfo","w");
		if(w)
		{
			while((fgets(line,256, f)!=NULL))
			{
				sscanf(line,"%s %s %s %s %s %s ", &Wert1, &Wert2, &Wert3, &Wert4, &Wert5, &Wert6);
				if(i++)
					fprintf(w,"\nFilesystem: %s\n  1-KBlocks: %s\n  Used: %s\n  Free: %s\n  Use%%: %s\nMounted on: %s\n",Wert1,Wert2,Wert3,Wert4,Wert5,Wert6);
			}
			fprintf(w,"\nCPU:\n\n");
			fclose(w);
		}
	}
	fclose(f);

	// cpuinfo from /proc/cpuinfo
	system("cat /proc/cpuinfo >> /tmp/sysinfo");
	unlink("/tmp/systmp");
	
	readList();

}

void CSysInfoWidget::dmesg()
{
	system("dmesg > /tmp/sysinfo");

	readList();
}

void CSysInfoWidget::ps()
{
	system("ps -A > /tmp/sysinfo");

	readList();
}

// read infos
void CSysInfoWidget::readList()
{
	buffer.clear();
				
	char buf[6000];

	int fd = open("/tmp/sysinfo", O_RDONLY);
	int bytes = read(fd, buf, 6000 - 1);
	close(fd);
	buf[bytes] = 0;
	buffer = buf;
}

//
void plugin_init(void)
{
}

void plugin_del(void)
{
}

void plugin_exec(void)
{
	CSysInfoWidget * SysInfoWidget = new CSysInfoWidget();
	
	SysInfoWidget->exec(NULL, "");
	SysInfoWidget->hide();
	
	delete SysInfoWidget;
}
