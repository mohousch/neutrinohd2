/*
	nglcd.h -- Neutrino GraphLCD driver

	Copyright (C) 2012 martii

	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __glcd_h__
#define __glcd_h__

#include <string>
#include <vector>
#include <time.h>
#include <string>
#include <sys/types.h>
#include <semaphore.h>

#include <glcdgraphics/bitmap.h>
#include <glcdgraphics/font.h>
#include <glcddrivers/config.h>
#include <glcddrivers/driver.h>
#include <glcddrivers/drivers.h>


typedef unsigned char * raw_ngdisplay_t;
 
struct raw_nglcd_element_header_t
{
	uint16_t width;
	uint16_t height;
	uint8_t bpp;
} __attribute__ ((packed));

struct raw_nglcd_element_t
{
	raw_nglcd_element_header_t header;
	int buffer_size;
	raw_ngdisplay_t buffer;
};

class nGLCD
{
	private:
		GLCD::cDriver * lcd;
		GLCD::cFont font_channel;
		GLCD::cFont font_epg;
		GLCD::cFont font_time;
		int fontsize_channel;
		int fontsize_epg;
		int fontsize_time;
		int percent_channel;
		int percent_time;
		int percent_epg;
		int percent_bar;
		int percent_space;
		GLCD::cBitmap * bitmap;
		t_channel_id ChannelID;
		std::string Channel;
		std::string Epg;
		std::string stagingChannel;
		std::string stagingEpg;
		int Scale;
		time_t now;
		struct tm *tm;
		bool channelLocked;
		bool doRescan;
		bool doSuspend;
		bool doStandby;
		bool doStandbyTime;
		bool doExit;
		bool doScrollChannel;
		bool doScrollEpg;
		bool doShowVolume;
		pthread_t thrGLCD;
		pthread_mutex_t mutex;
		void updateFonts();
		void Exec();
		std::string scrollChannel;
		std::string scrollEpg;
	public:
		bool fonts_initialized;
		bool doMirrorOSD;
		nGLCD();
		~nGLCD();
		static nGLCD * getInstance();
		void DeInit();
		static void Lock();
		static void Unlock();
		void mainLock();
		void mainUnlock();
		static void lockChannel(std::string &c);
		static void unlockChannel();
		static void * Run(void *);
		static void MirrorOSD(bool b = true);
		static void Update();
		static void Suspend();
		static void StandbyMode(bool);
		static void ShowVolume(bool);
		static void Resume();
		static void Exit();
		void Rescan();
		sem_t sem;
		bool load_png_element(const char * const filename, raw_nglcd_element_t * element);
		void draw_screen_element(const raw_nglcd_element_t * element, int x, int y, bool upscale);
		bool dump_png_element(const char * const filename, raw_nglcd_element_t * element);
		bool dump_png(const char * const filename);
		bool ShowPng(char *filename);
		bool DumpPng(char *filename);
};

#endif
