/*
  pictureviewer  -   DBoxII-Project
  
  $Id: pictureviewer.cpp 2013/10/12 mohousch Exp $

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

#include <global.h>
#include <neutrino.h>
#include "pictureviewer.h"
#include "config.h"
#include "driver/framebuffer.h"


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <unistd.h>

#include <system/debug.h>
#include <system/helpers.h>


double CPictureViewer::m_aspect_ratio_correction;

bool CPictureViewer::DecodeImage(const std::string & name, bool showBusySign, bool unscaled)
{
	dprintf(DEBUG_NORMAL, "CPictureViewer::DecodeImage: %s\n", name.c_str()); 
	
	/*
	if (name == m_NextPic_Name) 
	{
		return true;
	}
	*/
	
	int x;
	int y;
	int imx;
	int imy;
	
	// Show red block for "next ready" in view state
	if (showBusySign)
	{
		dprintf(DEBUG_INFO, "CPictureViewer::DecodeImage: show red block\n");
		showBusy(m_startx + 3, m_starty + 3, 10, 0xff, 00, 00);
	}
	
	CFormathandler * fh;

	if (unscaled)
		fh = fh_getsize(name.c_str (), &x, &y, INT_MAX, INT_MAX);
	else
		fh = fh_getsize(name.c_str (), &x, &y, m_endx - m_startx, m_endy - m_starty);

	if (fh) 
	{
		if (m_NextPic_Buffer != NULL) 
		{
			free(m_NextPic_Buffer);
		}
		
		m_NextPic_Buffer = (unsigned char *) malloc (x*y*3);

		if (m_NextPic_Buffer == NULL) 
		{
			printf ("CPictureViewer::DecodeImage: Error: malloc\n");
			return false;
		}
		
		dprintf(DEBUG_INFO, "CPictureViewer::DecodeImage: --->Decoding Start(%d/%d)\n", x, y);

		if (fh->get_pic(name.c_str(), &m_NextPic_Buffer, &x, &y) == FH_ERROR_OK) 
		{
			dprintf(DEBUG_INFO, "CPictureViewer::DecodeImage: <---Decoding Done\n");
			
			if ((x > (m_endx - m_startx) || y > (m_endy - m_starty)) && m_scaling != CFrameBuffer::NONE && !unscaled) 
			{
				if ((m_aspect_ratio_correction * y * (m_endx - m_startx) / x) <= (m_endy - m_starty)) 
				{
					imx = (m_endx - m_startx);
					imy = (int) (m_aspect_ratio_correction * y * (m_endx - m_startx) / x);
				} 
				else 
				{
					imx = (int) ((1.0 / m_aspect_ratio_correction) * x * (m_endy - m_starty) / y);
					imy = (m_endy - m_starty);
				}

				// resize
				m_NextPic_Buffer = CFrameBuffer::getInstance()->Resize(m_NextPic_Buffer, x, y, imx, imy, m_scaling);

				x = imx;
				y = imy;
			}
			
			m_NextPic_X = x;
			m_NextPic_Y = y;

			if (x < (m_endx - m_startx))
				m_NextPic_XPos = (m_endx - m_startx - x) / 2 + m_startx;
			else
				m_NextPic_XPos = m_startx;

			if (y < (m_endy - m_starty))
				m_NextPic_YPos = (m_endy - m_starty - y) / 2 + m_starty;
			else
				m_NextPic_YPos = m_starty;

			if (x > (m_endx - m_startx))
				m_NextPic_XPan = (x - (m_endx - m_startx)) / 2;
			else
				m_NextPic_XPan = 0;
			
			if (y > (m_endy - m_starty))
				m_NextPic_YPan = (y - (m_endy - m_starty)) / 2;
			else
				m_NextPic_YPan = 0;
		} 
		else 
		{
			printf("CPictureViewer::DecodeImage: Unable to read file !\n");
			free (m_NextPic_Buffer);
			m_NextPic_Buffer = (unsigned char *) malloc (3);

			if (m_NextPic_Buffer == NULL) 
			{
				printf ("CPictureViewer::DecodeImage: Error: malloc\n");
				return false;
			}
			memset(m_NextPic_Buffer, 0, 3);
			m_NextPic_X = 1;
			m_NextPic_Y = 1;
			m_NextPic_XPos = 0;
			m_NextPic_YPos = 0;
			m_NextPic_XPan = 0;
			m_NextPic_YPan = 0;
		}
	} 
	else 
	{
		printf ("CPictureViewer::DecodeImage: Unable to read file or format not recognized!\n");
		
		if (m_NextPic_Buffer != NULL) 
		{
			free (m_NextPic_Buffer);
		}

		m_NextPic_Buffer = (unsigned char *) malloc (3);
		if (m_NextPic_Buffer == NULL) 
		{
			printf ("CPictureViewer::DecodeImage: Error: malloc\n");
			return false;
		}

		memset(m_NextPic_Buffer, 0, 3);
		m_NextPic_X = 1;
		m_NextPic_Y = 1;
		m_NextPic_XPos = 0;
		m_NextPic_YPos = 0;
		m_NextPic_XPan = 0;
		m_NextPic_YPan = 0;
	}
	
	m_NextPic_Name = name;
	hideBusy ();

	return (m_NextPic_Buffer != NULL);
}

void CPictureViewer::SetVisible (int startx, int endx, int starty, int endy)
{
	m_startx = startx;
	m_endx = endx;
	m_starty = starty;
	m_endy = endy;
}

bool CPictureViewer::ShowImage(const std::string & filename, bool unscaled)
{
	dprintf(DEBUG_NORMAL, "CPictureViewer::Show Image: %s\n", filename.c_str());
	
  	if (m_CurrentPic_Buffer != NULL) 
	{
		free(m_CurrentPic_Buffer);
		m_CurrentPic_Buffer = NULL;
  	}

	// decode image
  	DecodeImage(filename, true, unscaled);
	
	// display next image
  	DisplayNextImage();
	
  	return true;
}

bool CPictureViewer::DisplayNextImage ()
{
	dprintf(DEBUG_NORMAL, "CPictureViewer::DisplayNextImage\n");
	
  	if (m_CurrentPic_Buffer != NULL) 
	{
		free(m_CurrentPic_Buffer);
		m_CurrentPic_Buffer = NULL;
  	}

  	if (m_NextPic_Buffer != NULL)
		CFrameBuffer::getInstance()->displayRGB(m_NextPic_Buffer, m_NextPic_X, m_NextPic_Y, m_NextPic_XPan, m_NextPic_YPan, m_NextPic_XPos, m_NextPic_YPos);
	
	// copy next pic in current pic
  	m_CurrentPic_Buffer = m_NextPic_Buffer;
  	m_NextPic_Buffer = NULL;
  	m_CurrentPic_Name = m_NextPic_Name;
  	m_CurrentPic_X = m_NextPic_X;
  	m_CurrentPic_Y = m_NextPic_Y;
  	m_CurrentPic_XPos = m_NextPic_XPos;
  	m_CurrentPic_YPos = m_NextPic_YPos;
  	m_CurrentPic_XPan = m_NextPic_XPan;
  	m_CurrentPic_YPan = m_NextPic_YPan;

  	return true;
}

void CPictureViewer::Zoom(float factor)
{
	dprintf(DEBUG_INFO, "CPictureViewer::Zoom %f\n",factor);
	
	showBusy(m_startx + 3, m_starty + 3, 10, 0xff, 0xff, 00);
	
	int oldx = m_CurrentPic_X;
	int oldy = m_CurrentPic_Y;
	unsigned char * oldBuf = m_CurrentPic_Buffer;
	m_CurrentPic_X = (int) (factor * m_CurrentPic_X);
	m_CurrentPic_Y = (int) (factor * m_CurrentPic_Y);
	
	m_CurrentPic_Buffer = CFrameBuffer::getInstance()->Resize(m_CurrentPic_Buffer, oldx, oldy, m_CurrentPic_X, m_CurrentPic_Y, m_scaling);
	
	if (m_CurrentPic_Buffer == oldBuf) 
	{
		// resize failed
		hideBusy ();
		return;
	}
	
	if (m_CurrentPic_X < (m_endx - m_startx))
		m_CurrentPic_XPos = (m_endx - m_startx - m_CurrentPic_X) / 2 + m_startx;
	else
		m_CurrentPic_XPos = m_startx;
	
	if (m_CurrentPic_Y < (m_endy - m_starty))
		m_CurrentPic_YPos = (m_endy - m_starty - m_CurrentPic_Y) / 2 + m_starty;
	else
		m_CurrentPic_YPos = m_starty;
	
	if (m_CurrentPic_X > (m_endx - m_startx))
		m_CurrentPic_XPan = (m_CurrentPic_X - (m_endx - m_startx)) / 2;
	else
		m_CurrentPic_XPan = 0;
	
	if (m_CurrentPic_Y > (m_endy - m_starty))
		m_CurrentPic_YPan = (m_CurrentPic_Y - (m_endy - m_starty)) / 2;
	else
		m_CurrentPic_YPan = 0;

	CFrameBuffer::getInstance()->displayRGB(m_CurrentPic_Buffer, m_CurrentPic_X, m_CurrentPic_Y, m_CurrentPic_XPan, m_CurrentPic_YPan, m_CurrentPic_XPos, m_CurrentPic_YPos);
}

void CPictureViewer::Move(int dx, int dy)
{
	dprintf(DEBUG_INFO, "CPictureViewer::Move %d %d\n", dx, dy);
	
	showBusy(m_startx + 3, m_starty + 3, 10, 0x00, 0xff, 00);
	
	int xs, ys;
	xs = CFrameBuffer::getInstance()->getScreenWidth(true);
	ys = CFrameBuffer::getInstance()->getScreenHeight(true);
	
	
	m_CurrentPic_XPan += dx;

	if (m_CurrentPic_XPan + xs >= m_CurrentPic_X)
		m_CurrentPic_XPan = m_CurrentPic_X - xs - 1;
	
	if (m_CurrentPic_XPan < 0)
		m_CurrentPic_XPan = 0;
	
	m_CurrentPic_YPan += dy;
	
	if (m_CurrentPic_YPan + ys >= m_CurrentPic_Y)
		m_CurrentPic_YPan = m_CurrentPic_Y - ys - 1;
	
	if (m_CurrentPic_YPan < 0)
		m_CurrentPic_YPan = 0;
	
	if (m_CurrentPic_X < (m_endx - m_startx))
		m_CurrentPic_XPos = (m_endx - m_startx - m_CurrentPic_X) / 2 + m_startx;
	else
		m_CurrentPic_XPos = m_startx;
	
	if (m_CurrentPic_Y < (m_endy - m_starty))
		m_CurrentPic_YPos = (m_endy - m_starty - m_CurrentPic_Y) / 2 + m_starty;
	else
		m_CurrentPic_YPos = m_starty;
	
	CFrameBuffer::getInstance()->displayRGB(m_CurrentPic_Buffer, m_CurrentPic_X, m_CurrentPic_Y, m_CurrentPic_XPan, m_CurrentPic_YPan, m_CurrentPic_XPos, m_CurrentPic_YPos);
}

CPictureViewer::CPictureViewer ()
{
	m_scaling = CFrameBuffer::COLOR;
	m_CurrentPic_Name = "";
	m_CurrentPic_Buffer = NULL;
	m_CurrentPic_X = 0;
	m_CurrentPic_Y = 0;
	m_CurrentPic_XPos = 0;
	m_CurrentPic_YPos = 0;
	m_CurrentPic_XPan = 0;
	m_CurrentPic_YPan = 0;
	m_NextPic_Name = "";
	m_NextPic_Buffer = NULL;
	m_NextPic_X = 0;
	m_NextPic_Y = 0;
	m_NextPic_XPos = 0;
	m_NextPic_YPos = 0;
	m_NextPic_XPan = 0;
	m_NextPic_YPan = 0;
	
	int xs, ys;
	xs = CFrameBuffer::getInstance()->getScreenWidth(true);
	ys = CFrameBuffer::getInstance()->getScreenHeight(true);
	
	m_aspect = 16.0/9;
	
	m_startx = 0;
	m_endx = xs - 1;
	m_starty = 0;
	m_endy = ys - 1;
	m_aspect_ratio_correction = m_aspect / ((double) xs / ys);
	
	m_busy_buffer = NULL;
}

void CPictureViewer::showBusy(int sx, int sy, int width, char r, char g, char b)
{
	dprintf(DEBUG_INFO, "CPictureViewer::Show Busy\n");
	
	unsigned char rgb_buffer[3];
	unsigned char *fb_buffer;
	unsigned char *busy_buffer_wrk;
	int cpp = 4;
	
	rgb_buffer[0] = r;
	rgb_buffer[1] = g;
	rgb_buffer[2] = b;
	
	fb_buffer = (unsigned char *) CFrameBuffer::getInstance()->convertRGB2FB(rgb_buffer, 1, 1);
	
	if (fb_buffer == NULL) 
	{
		printf ("CPictureViewer::showBusy: Error: malloc\n");
		return;
	}
	
	if (m_busy_buffer != NULL) 
	{
		free (m_busy_buffer);
		m_busy_buffer = NULL;
	}
	
	m_busy_buffer = (unsigned char *) malloc (width * width * cpp);
	
	if (m_busy_buffer == NULL) 
	{
		printf ("showBusy: Error: malloc\n");
		return;
	}
	
	busy_buffer_wrk = m_busy_buffer;
	unsigned char * fb = (unsigned char *) CFrameBuffer::getInstance()->getFrameBufferPointer();
	unsigned int stride = CFrameBuffer::getInstance()->getStride();
	
	for (int y = sy; y < sy + width; y++) 
	{
		for (int x = sx; x < sx + width; x++) 
		{
			memcpy (busy_buffer_wrk, fb + y * stride + x * cpp, cpp);
			busy_buffer_wrk += cpp;
			memcpy (fb + y * stride + x * cpp, fb_buffer, cpp);
		}
	}
	m_busy_x = sx;
	m_busy_y = sy;
	m_busy_width = width;
	m_busy_cpp = cpp;
	free (fb_buffer);

	CFrameBuffer::getInstance()->blit();
}

void CPictureViewer::hideBusy()
{
	dprintf(DEBUG_INFO, "CPictureViewer::Hide Busy\n");
	
	if (m_busy_buffer != NULL) 
	{
		unsigned char * fb = (unsigned char *) CFrameBuffer::getInstance()->getFrameBufferPointer();
		unsigned int stride = CFrameBuffer::getInstance()->getStride();
		unsigned char * busy_buffer_wrk = m_busy_buffer;
	
		for (int y = m_busy_y; y < m_busy_y + m_busy_width; y++) 
		{
			for (int x = m_busy_x; x < m_busy_x + m_busy_width; x++) 
			{
				memcpy (fb + y * stride + x * m_busy_cpp, busy_buffer_wrk, m_busy_cpp);
				busy_buffer_wrk += m_busy_cpp;
			}
		}
		free (m_busy_buffer);
		m_busy_buffer = NULL;
	}
	
	CFrameBuffer::getInstance()->blit();	
}

void CPictureViewer::Cleanup()
{
	dprintf(DEBUG_INFO, "CPictureViewer::Cleanup\n");
	
	if (m_busy_buffer != NULL) 
	{
		free (m_busy_buffer);
		m_busy_buffer = NULL;
	}

	if (m_NextPic_Buffer != NULL) 
	{
		free (m_NextPic_Buffer);
		m_NextPic_Buffer = NULL;
	}

	if (m_CurrentPic_Buffer != NULL) 
	{
		free(m_CurrentPic_Buffer);
		m_CurrentPic_Buffer = NULL;
	}
}

// channels logos
// check for logo
bool CPictureViewer::checkLogo(t_channel_id channel_id)
{	
        std::string logo_name;
	bool logo_ok = false;
	
	// first png, then jpg, then gif
	std::string strLogoExt[2] = { ".png", ".jpg" };
	
	// check for logo
	for (int i = 0; i < 2; i++)
	{
		logo_name = g_settings.logos_dir;
		logo_name += "/";
		logo_name += to_hexstring(channel_id & 0xFFFFFFFFFFFFULL);
		logo_name += strLogoExt[i].c_str();

		if(!access(logo_name.c_str(), F_OK)) 
		{
			logo_ok = true;
			dprintf(DEBUG_INFO, "%s found logo: %s\n", __FUNCTION__, logo_name.c_str());
			break;
		}
	}
	
	return logo_ok;
}

void CPictureViewer::getLogoSize(t_channel_id channel_id, int * width, int * height, int * bpp)
{
	std::string logo_name;
	bool logo_ok = false;
	
	// check for logo/convert channelid to logo
	std::string strLogoExt[2] = { ".png", ".jpg" };
	
	// check for logo
	for (int i = 0; i < 2; i++)
	{
		logo_name = g_settings.logos_dir;
		logo_name += "/";
		logo_name += to_hexstring(channel_id & 0xFFFFFFFFFFFFULL);
		logo_name += strLogoExt[i].c_str();

		if(!access(logo_name.c_str(), F_OK)) 
		{
			logo_ok = true;
			break;
		}
	}
	
	if(logo_ok)
	{
		// get logo real size
		CFrameBuffer::getInstance()->getSize(logo_name.c_str(), width, height, bpp);
		
		dprintf(DEBUG_INFO, "%s logo: %s (%dx%d) %dbpp\n", __FUNCTION__, logo_name.c_str(), *width, *height, *bpp);
	}
}

// display logo
bool CPictureViewer::DisplayLogo(t_channel_id channel_id, int posx, int posy, int width, int height, bool upscale, bool center_x, bool center_y)
{	
        std::string logo_name;
	bool ret = false;
	bool logo_ok = false;
	
	int logo_w = width;
	int logo_h = height;
	int logo_bpp = 0;
	
	
	// check for logo
	std::string strLogoExt[2] = { ".png", ".jpg" };
	
	// check for logo
	for (int i = 0; i < 2; i++)
	{
		logo_name = g_settings.logos_dir;
		logo_name += "/";
		logo_name += to_hexstring(channel_id & 0xFFFFFFFFFFFFULL);
		logo_name += strLogoExt[i].c_str();

		if(!access(logo_name.c_str(), F_OK)) 
		{
			logo_ok = true;
			break;
		}
	}
	
	if(logo_ok)
	{
		// get logo real size
		CFrameBuffer::getInstance()->getSize(logo_name, &logo_w, &logo_h, &logo_bpp);
	
		// scale only PNG logos
		if( logo_name.find(".png") == (logo_name.length() - 4) )
		{
			// upscale
			if(upscale)
			{	
				//rescale logo image
				float aspect = (float)(logo_w) / (float)(logo_h);
					
				if (((float)(logo_w) / (float)width) > ((float)(logo_h) / (float)height)) 
				{
					logo_w = width;
					logo_h = (int)(width / aspect);
				}
				else
				{
					logo_h = height;
					logo_w = (int)(height * aspect);
				}
			}
			
			ret = CFrameBuffer::getInstance()->DisplayImage(logo_name, center_x?posx + (width - logo_w)/2 : posx, center_y?posy + (height - logo_h)/2 : posy, logo_w, logo_h);
		}
		else
		{
			ret = CFrameBuffer::getInstance()->DisplayImage(logo_name, posx, posy, width, height);
		}
        }

	return ret;
}

