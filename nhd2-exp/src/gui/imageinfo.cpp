/*
	Neutrino-GUI  -   DBoxII-Project

	$Id: imageinfo.cpp 2013/10/12 mohousch Exp $
	
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

#include <gui/imageinfo.h>

#include <global.h>
#include <neutrino.h>

#include <driver/fontrenderer.h>
#include <driver/rcinput.h>
#include <driver/screen_max.h>

#include <daemonc/remotecontrol.h>

#include <system/flashtool.h>
#include <system/debug.h>

#include <video_cs.h>

#include <gui/version.h>


#define GIT_BUILD "GIT Build:"
#define GIT_REV 	"GIT Rev:"

extern cVideo * videoDecoder;

CImageInfo::CImageInfo()
{
	frameBuffer = CFrameBuffer::getInstance();

	font_head   = SNeutrinoSettings::FONT_TYPE_INFOBAR_CHANNAME;;
	font_small  = SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL;
	font_info   = SNeutrinoSettings::FONT_TYPE_MENU;

	hheight     = g_Font[font_head]->getHeight();
	iheight     = g_Font[font_info]->getHeight();
	sheight     = g_Font[font_small]->getHeight();

	max_width = frameBuffer->getScreenWidth(true);
	max_height =  frameBuffer->getScreenHeight(true);

	width = frameBuffer->getScreenWidth() - 10;
	height = frameBuffer->getScreenHeight() - 10;

	x = (((g_settings.screen_EndX- g_settings.screen_StartX)-width) / 2) + g_settings.screen_StartX;
	y = (((g_settings.screen_EndY- g_settings.screen_StartY)-height) / 2) + g_settings.screen_StartY;
}

CImageInfo::~CImageInfo()
{ 
	if(videoDecoder)  
		videoDecoder->Pig(-1, -1, -1, -1);
}

int CImageInfo::exec(CMenuTarget *parent, const std::string &)
{
	dprintf(DEBUG_NORMAL, "CImageInfo::exec:\n");

	if (parent)
 		parent->hide();

	paint();

	paint_pig(x + width - BORDER_RIGHT - width/3, y, width/3, height/3);	

	frameBuffer->blit();	

	neutrino_msg_t msg;

	while (1)
	{
		neutrino_msg_data_t data;
		unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd_MS(100);
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if (msg <= CRCInput::RC_MaxRC)
		{
			break;
		}

		if ( msg >  CRCInput::RC_MaxRC && msg != CRCInput::RC_timeout)
		{
			CNeutrinoApp::getInstance()->handleMsg( msg, data );
		}
		
		frameBuffer->blit();		
	}

	hide();

	return menu_return::RETURN_REPAINT;
}

void CImageInfo::hide()
{
	if(videoDecoder)  
		videoDecoder->Pig(-1, -1, -1, -1);
	
	frameBuffer->paintBackgroundBoxRel(0, 0, max_width, max_height);

	frameBuffer->blit();
}

void CImageInfo::paint_pig(int _x, int _y, int w, int h)
{
	frameBuffer->paintBackgroundBoxRel(_x, _y, w, h);	
		
	//dont pig if we have 1980 x 1080
#if defined (__sh__)
	int xres, yres, framerate;
	if(videoDecoder)
		videoDecoder->getPictureInfo(xres, yres, framerate);
	
	if(xres <= 1280)
		if(videoDecoder)	
			videoDecoder->Pig(_x, _y, w, h);
#else
	if(videoDecoder)
		videoDecoder->Pig(_x, _y, w, h);
#endif	
}

void CImageInfo::paintLine(int xpos, int font, const char* text)
{
	char buf[100];
	sprintf((char*) buf, "%s", text);
	
	g_Font[font]->RenderString(xpos, ypos, width - BORDER_RIGHT, buf, COL_INFOBAR, 0, true);
}

void CImageInfo::paint()
{
	const char * head_string;
	//char imagedate[18] = "";
 	int  xpos = x + BORDER_LEFT;
	int x_offset = g_Font[font_info]->getRenderWidth(g_Locale->getText(LOCALE_IMAGEINFO_HOMEPAGE)) + 10;

	ypos = y + 5;

	head_string = g_Locale->getText(LOCALE_IMAGEINFO_HEAD);

	CVFD::getInstance()->setMode(CVFD::MODE_MENU_UTF8, head_string);

	//
	frameBuffer->paintBoxRel(0, 0, max_width, max_height, COL_INFOBAR_PLUS_0);
	
	// title
	g_Font[font_head]->RenderString(xpos, ypos + hheight + 1, width, head_string, COL_MENUHEAD, 0, true);

	ypos += hheight;
	ypos += (iheight >>1);


	CConfigFile config('\t');
	config.loadConfig("/etc/.version");

	const char * imagename = config.getString("imagename", "NeutrinoHD2").c_str();
	const char * homepage = config.getString("homepage", "").c_str();
	const char * creator = config.getString("creator", "").c_str();
	const char * docs = config.getString("docs", "http://wiki.neutrino-hd.de").c_str();
	const char * forum = config.getString("forum", "").c_str();
	const char * version = config.getString("version", "1202201602031021").c_str();
	const char * builddate = BUILT_DATE;
	const char * gitrev = GIT;
	

	static CFlashVersionInfo versionInfo(version);
	const char * releaseCycle = versionInfo.getReleaseCycle();
	const char * imageType = versionInfo.getType();
	//sprintf((char*) imagedate, "%s  %s", versionInfo.getDate(), versionInfo.getTime());

	// image name
	ypos += iheight;
	paintLine(xpos, font_info, g_Locale->getText(LOCALE_IMAGEINFO_IMAGE));
	paintLine(xpos + x_offset, font_info, imagename);

	// image date
	//ypos += iheight;
	//paintLine(xpos, font_info, g_Locale->getText(LOCALE_IMAGEINFO_DATE));
	//paintLine(xpos + x_offset, font_info, imagedate);

	// release cycle
	ypos += iheight;
	paintLine(xpos, font_info, g_Locale->getText(LOCALE_IMAGEINFO_VERSION));
	paintLine(xpos + x_offset, font_info, releaseCycle);
	
	// git built date
	ypos += iheight;
	paintLine(xpos, font_info, GIT_BUILD);
	paintLine(xpos + x_offset, font_info, builddate );

	// git rev
	ypos += iheight;
	paintLine(xpos, font_info, GIT_REV);
	paintLine(xpos + x_offset, font_info, gitrev );	
	
	// image type
	ypos += iheight;
	paintLine(xpos, font_info, g_Locale->getText(LOCALE_IMAGEINFO_TYPE));
	paintLine(xpos + x_offset, font_info, imageType);

	// image creator
	ypos += iheight;
	paintLine(xpos, font_info, g_Locale->getText(LOCALE_IMAGEINFO_CREATOR));
	paintLine(xpos + x_offset, font_info, creator);

	// homepage
	ypos += iheight;
	paintLine(xpos, font_info, g_Locale->getText(LOCALE_IMAGEINFO_HOMEPAGE));
	paintLine(xpos + x_offset, font_info, homepage);

	// doko
	ypos += iheight;
	paintLine(xpos, font_info, g_Locale->getText(LOCALE_IMAGEINFO_DOKUMENTATION));
	paintLine(xpos + x_offset, font_info, docs);

	// forum
	ypos += iheight;
	paintLine(xpos, font_info, g_Locale->getText(LOCALE_IMAGEINFO_FORUM));
	paintLine(xpos + x_offset, font_info, forum);

	// license
	ypos += 5*iheight;
	paintLine(xpos, font_info,g_Locale->getText(LOCALE_IMAGEINFO_LICENSE));
	paintLine(xpos + x_offset, font_small, "This program is free software; you can redistribute it and/or modify");

	ypos += sheight;
	paintLine(xpos + x_offset, font_small, "it under the terms of the GNU General Public License as published by");

	ypos += sheight;
	paintLine(xpos + x_offset, font_small, "the Free Software Foundation; either version 2 of the License, or");

	ypos += sheight;
	paintLine(xpos + x_offset, font_small, "(at your option) any later version.");

	ypos += sheight;
	paintLine(xpos + x_offset, font_small, "This program is distributed in the hope that it will be useful,");

	ypos += sheight;
	paintLine(xpos + x_offset, font_small, "but WITHOUT ANY WARRANTY; without even the implied warranty of");

	ypos += sheight;
	paintLine(xpos + x_offset, font_small, "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.");

	ypos += sheight;
	paintLine(xpos + x_offset, font_small, "See the GNU General Public License for more details.");

	ypos += sheight;
	paintLine(xpos + x_offset, font_small, "You should have received a copy of the GNU General Public License");

	ypos += sheight;
	paintLine(xpos + x_offset, font_small, "along with this program; if not, write to the Free Software");

	ypos += sheight;
	paintLine(xpos + x_offset, font_small, "Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.");	
}
