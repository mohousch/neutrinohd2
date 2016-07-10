/*
  pictureviewer  -   DBoxII-Project
  
  $Id: pictureviewer.h 2013/10/12 mohousch Exp $

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

#ifndef __pictureviewer__
#define __pictureviewer__


#include <string>
#include <stdio.h>    /* printf       */
#include <sys/time.h> /* gettimeofday */
#include "driver/framebuffer.h"

// zapit types
#include <client/zapittypes.h>


class CPictureViewer
{
	private:
		CFrameBuffer::ScalingMode m_scaling;
		float m_aspect;
		
		std::string m_NextPic_Name;
		unsigned char* m_NextPic_Buffer;
		int m_NextPic_X;
		int m_NextPic_Y;
		int m_NextPic_XPos;
		int m_NextPic_YPos;
		int m_NextPic_XPan;
		int m_NextPic_YPan;
		
		std::string m_CurrentPic_Name;
		unsigned char* m_CurrentPic_Buffer;
		int m_CurrentPic_X;
		int m_CurrentPic_Y;
		int m_CurrentPic_XPos;
		int m_CurrentPic_YPos;
		int m_CurrentPic_XPan;
		int m_CurrentPic_YPan;
		
		unsigned char * m_busy_buffer;
		int m_busy_x;
		int m_busy_y;
		int m_busy_width;
		int m_busy_cpp;

		int m_startx;
		int m_starty;
		int m_endx;
		int m_endy;
		
	public:
		CPictureViewer();
		~CPictureViewer(){Cleanup();};
		bool ShowImage(const std::string & filename, bool unscaled = false);
		bool DecodeImage(const std::string & name, bool showBusySign = false, bool unscaled = false);
		bool DisplayNextImage();
		void SetScaling( CFrameBuffer::ScalingMode s){m_scaling = s;}
		void SetAspectRatio(float aspect_ratio) {m_aspect = aspect_ratio;}
		void showBusy(int sx, int sy, int width, char r, char g, char b);
		void hideBusy();
		void Zoom(float factor);
		void Move(int dx, int dy);
		void Cleanup();
		void SetVisible(int startx, int endx, int starty, int endy);
		static double m_aspect_ratio_correction;
};

#endif
