/*
	Neutrino graphlcd daemon thread

	(c) 2012 by martii


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

#include <png.h>

#include <unistd.h>

#include <global.h>
#include <neutrino.h>
#include <algorithm>
#include <system/debug.h>
#include <system/set_threadname.h>

#include "nglcd.h"

#ifndef max
#define max(a,b)(((a)<(b)) ? (b) : (a))
#endif

static const char * kDefaultConfigFile = "/etc/graphlcd.conf";
static nGLCD *nglcd = NULL;

void sectionsd_getEventsServiceKey(t_channel_id serviceUniqueKey, CChannelEventList &eList, char search = 0, std::string search_text = "");
void sectionsd_getCurrentNextServiceKey(t_channel_id uniqueServiceKey, CSectionsdClient::responseGetCurrentNextInfoChannelID& current_next );

// config
extern int		glcd_enable;
extern uint32_t		glcd_color_fg;
extern uint32_t		glcd_color_bg;
extern uint32_t		glcd_color_bar;
extern std::string	glcd_font;
extern int		glcd_percent_channel;
extern int		glcd_percent_epg;
extern int		glcd_percent_bar;
extern int		glcd_percent_time;
extern int		glcd_mirror_osd;
extern int		glcd_time_in_standby;

nGLCD::nGLCD() 
{
	lcd = NULL;
	ChannelID = -1;
	Channel = "Neutrino";
	Epg = "Neutrino";
	scrollChannel = "Neutrino";
	scrollEpg = "Neutrino";

	sem_init(&sem, 0, 1);

	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
	pthread_mutex_init(&mutex, &attr);

	stagingChannel = "";
	stagingEpg = "";
	channelLocked = false;
	doRescan = false;
	doStandby = false;
	doStandbyTime = false;
	doShowVolume = false;
	doSuspend = false;
	doExit = false;
	doMirrorOSD = false;
	fontsize_channel = 0;
	fontsize_epg = 0;
	fontsize_time = 0;
	doScrollChannel = false;
	doScrollEpg = false;
	percent_channel = 0;
	percent_time = 0;
	percent_epg = 0;
	percent_bar = 0;
	percent_time = 0;
	percent_space = 0;
	Scale = 0;
	bitmap = NULL;

	nglcd = this;

	if (!glcd_enable)
		doSuspend = true;

	if (pthread_create (&thrGLCD, 0, nGLCD::Run, NULL) != 0 )
		fprintf(stderr, "ERROR: pthread_create(nGLCD::Init)\n");
}

void nGLCD::Lock(void)
{
	if (nglcd)
		pthread_mutex_lock(&nglcd->mutex);
}

void nGLCD::Unlock(void)
{
	if (nglcd)
		pthread_mutex_unlock(&nglcd->mutex);
}

nGLCD::~nGLCD() 
{
	Suspend();
	nglcd = NULL;
	
	if (lcd) 
	{
		lcd->DeInit();
		delete lcd;
	}
}

nGLCD *nGLCD::getInstance()
{
	if (!nglcd)
		nglcd = new nGLCD;
	return nglcd;
}

void nGLCD::Exec() 
{
	if (!lcd)
		return;

	bitmap->Clear(glcd_color_bg);

	if (CNeutrinoApp::getInstance()->recordingstatus) 
	{
		bitmap->DrawRectangle(0, 0, bitmap->Width() - 1, bitmap->Height() - 1, glcd_color_bar, false);
		bitmap->DrawRectangle(1, 1, bitmap->Width() - 2, bitmap->Height() - 2, glcd_color_bar, false);
	}

	int off = 0;
	
	raw_nglcd_element_t channel_logo;
	channel_logo.header.width = 0;
	channel_logo.header.height = 0;
	channel_logo.buffer_size = 0;
	channel_logo.buffer = NULL;

	if (percent_channel) 
	{
		//
		char fname[255];
		bool logo_ok = false;

		// first png, then jpg, then gif
		std::string strLogoExt[3] = { ".png", ".jpg" , ".gif" };
	
		// check for logo
		for (int i = 0; i < 3; i++)
		{
			sprintf(fname, "%s/%llx%s", g_settings.logos_dir.c_str(), ChannelID & 0xFFFFFFFFFFFFULL, strLogoExt[i].c_str());
			if(!access(fname, F_OK)) 
			{
				logo_ok = true;
				break;
			}
		}
	
		if(logo_ok)
			logo_ok = load_png_element(fname, &channel_logo);

		if(logo_ok)
		{
			off += percent_space;
			if (!doStandbyTime)
			{
				int logo_top = 0;
				int logo_max_height = 0;
				if ((channel_logo.header.width*2 < bitmap->Width()) && (channel_logo.header.height*2 <= (percent_space*2+percent_channel) * bitmap->Height()/100))
				{
					logo_top = (off-percent_space) * bitmap->Height()/100;
					logo_max_height = (percent_space*2+percent_channel) * bitmap->Height()/100;
					logo_top = logo_top + (logo_max_height-channel_logo.header.height*2)/2;
					draw_screen_element(&channel_logo, (bitmap->Width() - channel_logo.header.width*2)/2, logo_top, true);
				}
				else
				{
					logo_top = (off-percent_space/2) * bitmap->Height()/100;
					logo_max_height = (percent_space+percent_channel) * bitmap->Height()/100;
					logo_top = logo_top + (logo_max_height-channel_logo.header.height)/2;
					draw_screen_element(&channel_logo, (bitmap->Width() - channel_logo.header.width)/2, logo_top, false);
				}
			}
			if (channel_logo.buffer)
			{
				free(channel_logo.buffer);
				channel_logo.buffer = NULL;
			}
			off += percent_channel;
			off += percent_space;
			if (scrollChannel.length() > Channel.length())
			{
				while (scrollChannel.length() > Channel.length())
					scrollChannel = scrollChannel.substr(1);
			}
			else
				doScrollChannel = false;
		}
		else
		{
			off += percent_space;
			int fw = font_channel.Width(scrollChannel);
			if (fw && !doStandbyTime)
				bitmap->DrawText(max(0,(bitmap->Width() - 4 - fw)/2), off * bitmap->Height()/100, bitmap->Width() - 4, scrollChannel, &font_channel, glcd_color_fg, GLCD::cColor::Transparent);
			off += percent_channel;
			off += percent_space;
			if (scrollChannel.length() > Channel.length())
				scrollChannel = scrollChannel.substr(1);
			else
				doScrollChannel = false;
		}
	}

	if (percent_epg) 
	{
		off += percent_space;
		int fw = font_epg.Width(scrollEpg);
		if (fw && !doStandbyTime)
			bitmap->DrawText(max(0,(bitmap->Width() - 4 - fw)/2), off * bitmap->Height()/100, bitmap->Width() - 4, scrollEpg, &font_epg, glcd_color_fg, GLCD::cColor::Transparent);
		off += percent_epg;
		off += percent_space;
		if (scrollEpg.length() > Epg.length())
			scrollEpg = scrollEpg.substr(1);
		else
			doScrollEpg = false;
	}

	if (percent_bar) 
	{
		off += percent_space;
		int bar_top = off * bitmap->Height()/100;
		off += percent_bar;
		int bar_bottom = off * bitmap->Height()/100;
		if (!doStandbyTime) 
		{
			bitmap->DrawHLine(0, bar_top, bitmap->Width(), glcd_color_fg);
			bitmap->DrawHLine(0, bar_bottom, bitmap->Width(), glcd_color_fg);
			if (Scale)
				bitmap->DrawRectangle(0, bar_top + 1, Scale * (bitmap->Width() - 1)/100, bar_bottom - 1, glcd_color_bar, true);
		}
		off += percent_space;
	}

	if (percent_time) 
	{
		off += percent_space;
		char timebuf[10];
		strftime(timebuf, sizeof(timebuf), "%H:%M", tm);

		std::string Time = std::string(timebuf);

		bitmap->DrawText(max(0,(bitmap->Width() - 4 - font_time.Width(Time))/2), off * bitmap->Height()/100, bitmap->Width() - 1, Time, &font_time, glcd_color_fg, GLCD::cColor::Transparent);
	}

	lcd->SetScreen(bitmap->Data(), bitmap->Width(), bitmap->Height());
	lcd->Refresh(true);
}

static bool sortByDateTime (const CChannelEvent& a, const CChannelEvent& b)
{
	return a.startTime < b.startTime;
}

void nGLCD::updateFonts() 
{
	bool changed = false;
	int percent = glcd_percent_channel + glcd_percent_epg + glcd_percent_bar + glcd_percent_time;

	if (percent < 100)
		percent = 100;

	// normalize values
	percent_channel = glcd_percent_channel * 100 / percent;
	percent_epg = glcd_percent_epg * 100 / percent;
	percent_bar = glcd_percent_bar * 100 / percent;
	percent_time = glcd_percent_time * 100 / percent;

	// calculate height
	int fontsize_channel_new = percent_channel * nglcd->lcd->Height() / 100;
	int fontsize_epg_new = percent_epg * nglcd->lcd->Height() / 100;
	int fontsize_time_new = percent_time * nglcd->lcd->Height() / 100;

	if (!fonts_initialized || (fontsize_channel_new != fontsize_channel)) 
	{
		fontsize_channel = fontsize_channel_new;
		if (!font_channel.LoadFT2(glcd_font, "UTF-8", fontsize_channel)) 
		{
			glcd_font = FONTDIR "/neutrino.ttf";
			font_channel.LoadFT2(glcd_font, "UTF-8", fontsize_channel);
		}
		changed = true;
	}
	if (!fonts_initialized || (fontsize_epg_new != fontsize_epg)) 
	{
		fontsize_epg = fontsize_epg_new;
		if (!font_epg.LoadFT2(glcd_font, "UTF-8", fontsize_epg)) 
		{
			glcd_font = FONTDIR "/neutrino.ttf";
			font_epg.LoadFT2(glcd_font, "UTF-8", fontsize_epg);
		}
		changed = true;
	}
	if (!fonts_initialized || (fontsize_time_new != fontsize_time)) 
	{
		fontsize_time = fontsize_time_new;
		if (!font_time.LoadFT2(glcd_font, "UTF-8", fontsize_time)) 
		{
			glcd_font = FONTDIR "/neutrino.ttf";
			font_time.LoadFT2(glcd_font, "UTF-8", fontsize_time);
		}
		changed = true;
	}

	if (!changed)
		return;

	int div = 0;
	if (percent_channel)
		div += 2;
	if (percent_epg)
		div += 2;
	if (percent_bar)
		div += 2;
	if (percent_time)
		div += 2;
	percent_space = (100 - percent_channel - percent_time - percent_epg - percent_bar) / div;

	fonts_initialized = true;
}

void* nGLCD::Run(void *)
{
	set_threadname("nGLCD::Run");

	if (GLCD::Config.Load(kDefaultConfigFile) == false) 
	{
		fprintf(stderr, "Error loading config file!\n");
		return NULL;
	}
	
	if ((GLCD::Config.driverConfigs.size() < 1)) 
	{
		fprintf(stderr, "No driver config found!\n");
		return NULL;
	}

	struct timespec ts;
	CChannelEventList evtlist;
	CSectionsdClient::CurrentNextInfo info_CurrentNext;                                 
	t_channel_id channel_id = -1;
	info_CurrentNext.current_zeit.startzeit = 0;
	info_CurrentNext.current_zeit.dauer = 0;
	info_CurrentNext.flags = 0;

	nglcd->fonts_initialized = false;
	bool broken = false;

	do {
		if (broken) 
		{
#ifdef GLCD_DEBUG
			fprintf(stderr, "No graphlcd display found ... sleeping for 30 seconds\n");
#endif
			clock_gettime(CLOCK_REALTIME, &ts);
			ts.tv_sec += 30;
			sem_timedwait(&nglcd->sem, &ts);
			broken = false;
			if (nglcd->doExit)
				break;
			if (!glcd_enable)
				continue;
		} else
				while ((nglcd->doSuspend || nglcd->doStandby || !glcd_enable) && !nglcd->doExit)
					sem_wait(&nglcd->sem);

		if (nglcd->doExit)
			break;

		int warmUp = 5;
		nglcd->lcd = GLCD::CreateDriver(GLCD::Config.driverConfigs[0].id, &GLCD::Config.driverConfigs[0]);
		if (!nglcd->lcd) 
		{
#ifdef GLCD_DEBUG
			fprintf(stderr, "CreateDriver failed.\n");
#endif
			broken = true;
			continue;
		}
#ifdef GLCD_DEBUG
		fprintf(stderr, "CreateDriver succeeded.\n");
#endif
		if (nglcd->lcd->Init()) 
		{
			delete nglcd->lcd;
			nglcd->lcd = NULL;
#ifdef GLCD_DEBUG
			fprintf(stderr, "LCD init failed.\n");
#endif
			broken = true;
			continue;
		}
#ifdef GLCD_DEBUG
		fprintf(stderr, "LCD init succeeded.\n");
#endif

		if (!nglcd->bitmap)
			nglcd->bitmap = new GLCD::cBitmap(nglcd->lcd->Width(), nglcd->lcd->Height(), glcd_color_bg);

		nglcd->Update();

		nglcd->doMirrorOSD = false;

		while ((!nglcd->doSuspend && !nglcd->doStandby) && !nglcd->doExit && glcd_enable) 
		{
			if (nglcd->doMirrorOSD) 
			{
				nglcd->bitmap->Clear(GLCD::cColor::Black);
				ts.tv_sec = 0; // don't wait
				static CFrameBuffer * fb = CFrameBuffer::getInstance();
				int fb_width = fb->getScreenWidth(true);
				int fb_height = fb->getScreenHeight(true);
				int lcd_width = nglcd->bitmap->Width();
				int lcd_height = nglcd->bitmap->Height();
				uint32_t *fbp = fb->getFrameBufferPointer();
				unsigned int fb_stride = fb->getStride()/4;

#ifdef GLCD_DEBUG
				printf("%s:%s(%d) fb_stride: %d, fb_width: %d, fb_height: %d\n", __FILE__, __FUNCTION__, __LINE__, fb_stride, fb_width, fb_height);
#endif

				// determine OSD frame geometry
				int y_min = 0;
				for (int y = 0; y < fb_height && !y_min; y++) 
				{
					for (int x = 0; x < fb_width; x++) 
					{
						if (*(fbp + fb_stride * y + x)) 
						{
							y_min = y;
							break;
						}
					}
				}
				int y_max = 0;
				for (int y = fb_height - 1; y_min < y && !y_max; y--) 
				{
					for (int x = 0; x < fb_width; x++) 
					{
						if (*(fbp + fb_stride * y + x)) 
						{ 
							y_max = y;
							break;
						}
					}
				}
				int x_min = fb_width - 1;
				for (int y = y_min; y < y_max; y++) 
				{
					for (int x = 0; x < fb_width; x++) 
					{
						uint32_t cl = *(fbp + fb_stride * y + x);
						if (cl && x < x_min) 
						{
							x_min = x;
							
#ifdef GLCD_DEBUG
							printf("%s:%s(%d) x_min: %d, x: %d, y: %d, cl: %x\n", __FILE__, __FUNCTION__, __LINE__, x_min, x, y, cl);
#endif

							break;
						}
					}
				}
				int x_max = x_min;
				for (int y = y_min; y < y_max; y++) 
				{
					for (int x = fb_width - 1; x > x_min; x--) 
					{
						uint32_t cl = *(fbp + fb_stride * y + x);
						if (cl && x > x_max) 
						{
							x_max = x;
							
#ifdef GLCD_DEBUG
							printf("%s:%s(%d) x_max: %d, x: %d, y: %d, cl: %x\n", __FILE__, __FUNCTION__, __LINE__, x_max, x, y, cl);
#endif

							break;
						}
					}
				}

				int fb_w = x_max - x_min;
				int fb_h = y_max - y_min;
				
#ifdef GLCD_DEBUG
				printf("%s:%s(%d) x_min: %d, x_max: %d, fb_w: %d, y_min: %d, y_max: %d, fb_h: %d \n", __FILE__, __FUNCTION__, __LINE__, x_min, x_max, fb_w, y_min, y_max, fb_h);
#endif				
				// Skip empty and infobar
				if (!fb_w || !fb_w || (y_min*100/fb_height > 66)) 
				{
					usleep(500000);
					continue;
				}

				// Keep aspect by using the smallest up-scaling factor
				if (fb_width * fb_h > fb_height * fb_w) 
				{
					int fb_w2 = fb_width * fb_h/fb_height;
					x_min += (fb_w - fb_w2)/2;
					fb_w = fb_w2;
				} 
				else 
				{
					int fb_h2 = fb_height * fb_w/fb_width;
					y_min += (fb_h - fb_h2)/2;
					fb_h = fb_h2;
				}
				// Compensate for rounding errors
				if (x_min < 0)
					x_min = 0;
				if (y_min < 0)
					y_min = 0;
				
#ifdef GLCD_DEBUG
				printf("%s:%s(%d) x_min: %d, x_max: %d, fb_w: %d, y_min: %d, y_max: %d, fb_h: %d \n", __FILE__, __FUNCTION__, __LINE__, x_min, x_max, fb_w, y_min, y_max, fb_h);
#endif				

				for (int y = 0; y < lcd_height; y++) 
				{
					int ystride = y_min * fb_stride;
					uint32_t *rp = fbp + ystride + (y * fb_h / lcd_height) * fb_stride;
					uint32_t cl = 0;
					//
					for (int x = 0; x < lcd_width; x++) 
					{
						// FIXME: There might be some oscure bug somewhere that invalidate the address of bitmap->DrawPixel()
						cl = *(rp + x_min + (x * fb_w / lcd_width));
						nglcd->bitmap->DrawPixel(x, y, cl);
					}
				}

				nglcd->lcd->SetScreen(nglcd->bitmap->Data(), lcd_width, lcd_height);
				nglcd->lcd->Refresh(true);
				continue;
			} // end of fb mirroring

			clock_gettime(CLOCK_REALTIME, &ts);
			nglcd->tm = localtime(&ts.tv_sec);
			nglcd->updateFonts();
			nglcd->Exec();
			clock_gettime(CLOCK_REALTIME, &ts);
			nglcd->tm = localtime(&ts.tv_sec);
			if (warmUp > 0) 
			{
				ts.tv_sec += 4;
				warmUp--;
			} 
			else 
			{
				ts.tv_sec += 60 - nglcd->tm->tm_sec;
				ts.tv_nsec = 0;
			}

			if (!nglcd->doScrollChannel && !nglcd->doScrollEpg)
				sem_timedwait(&nglcd->sem, &ts);

			while(!sem_trywait(&nglcd->sem));

			if(nglcd->doRescan || nglcd->doSuspend || nglcd->doStandby || nglcd->doExit)
				break;

			if (nglcd->doShowVolume) 
			{
				nglcd->Epg = "";
				if (nglcd->Channel.compare(g_Locale->getText(LOCALE_GLCD_VOLUME))) 
				{
					nglcd->Channel = g_Locale->getText(LOCALE_GLCD_VOLUME);
					if (nglcd->font_channel.Width(nglcd->Channel) > nglcd->bitmap->Width() - 4)
						nglcd->scrollChannel = nglcd->Channel + "      " + nglcd->Channel + "      " + nglcd->Channel;
					else
						nglcd->scrollChannel = nglcd->Channel;
				}
				nglcd->scrollEpg = nglcd->Epg;
				nglcd->Scale = g_settings.current_volume;
				channel_id = -1;
			} 
			else if (nglcd->channelLocked) 
			{
				nglcd->Lock();
				if (nglcd->Epg.compare(nglcd->stagingEpg)) 
				{
					nglcd->Epg = nglcd->stagingEpg;
					if (nglcd->font_epg.Width(nglcd->Epg) > nglcd->bitmap->Width() - 4)
						nglcd->scrollEpg = nglcd->Epg + "      " + nglcd->Epg + "      " + nglcd->Epg;
					else
						nglcd->scrollEpg = nglcd->Epg;
				}
				
				if (nglcd->Channel.compare(nglcd->stagingChannel)) 
				{
					nglcd->Channel = nglcd->stagingChannel;
					if (nglcd->font_channel.Width(nglcd->Channel) > nglcd->bitmap->Width() - 4)
						nglcd->scrollChannel = nglcd->Channel + "      " + nglcd->Channel + "      " + nglcd->Channel;
					else
						nglcd->scrollChannel = nglcd->Channel;
				}
				nglcd->Scale = 0;
				channel_id = -1;
				nglcd->Unlock();
			} 
			else 
			{
				CChannelList *channelList = CNeutrinoApp::getInstance ()->channelList;
				if (!channelList)
					continue;
				t_channel_id new_channel_id = channelList->getActiveChannel_ChannelID ();
				if (!new_channel_id)
					continue;

				if ((new_channel_id != channel_id)) 
				{
					nglcd->ChannelID = new_channel_id;
					nglcd->Channel = channelList->getActiveChannelName ();
					nglcd->Epg = "";
					nglcd->Scale = 0;
					nglcd->scrollEpg = nglcd->Epg;
					if (nglcd->font_channel.Width(nglcd->Channel) > nglcd->bitmap->Width() - 4) 
					{
						nglcd->scrollChannel = nglcd->Channel + "      " + nglcd->Channel + "      " + nglcd->Channel;
						nglcd->doScrollChannel = true;
					} 
					else 
					{
						nglcd->scrollChannel = nglcd->Channel;
						nglcd->doScrollChannel = false;
					}
					warmUp = 1;
				}

				sectionsd_getCurrentNextServiceKey(channel_id & 0xFFFFFFFFFFFFULL, info_CurrentNext);
				if ((channel_id != new_channel_id) || (evtlist.empty())) 
				{
					evtlist.clear();
					sectionsd_getEventsServiceKey(new_channel_id & 0xFFFFFFFFFFFFULL, evtlist);
					if (!evtlist.empty())
						sort(evtlist.begin(),evtlist.end(), sortByDateTime);
				}
				channel_id = new_channel_id;

				if (!evtlist.empty()) 
				{
					CChannelEventList::iterator eli;
					for (eli=evtlist.begin(); eli!=evtlist.end(); ++eli) 
					{
						if ((u_int)eli->startTime + (u_int)eli->duration > (u_int)ts.tv_sec)
							break;
					}
					if (eli == evtlist.end()) 
					{
						nglcd->Epg = nglcd->scrollEpg = "";
						nglcd->Scale = 0;
					} 
					else 
					{
						if (eli->description.compare(nglcd->Epg)) 
						{
							nglcd->Epg = eli->description;
							if (nglcd->font_epg.Width(nglcd->Epg) > nglcd->bitmap->Width() - 4) 
							{
								nglcd->scrollEpg = nglcd->Epg + "    " + nglcd->Epg + "    " + nglcd->Epg;
								nglcd->doScrollEpg = true;
							} 
							else 
							{
								nglcd->scrollEpg = nglcd->Epg;
								nglcd->doScrollEpg = false;
							}
						}

						if (eli->duration > 0)
						nglcd->Scale = (ts.tv_sec - eli->startTime) * 100 / eli->duration;
						if (nglcd->Scale > 100)
							nglcd->Scale = 100;
						else if (nglcd->Scale < 0)
							nglcd->Scale = 0;
					}
				}
			}
		}

		if(!glcd_enable || nglcd->doSuspend || nglcd->doStandby || nglcd->doExit) 
		{
			// for restart, don't blacken screen
			nglcd->bitmap->Clear(GLCD::cColor::Black);
			nglcd->lcd->SetScreen(nglcd->bitmap->Data(), nglcd->bitmap->Width(), nglcd->bitmap->Height());
			nglcd->lcd->Refresh(true);
		}
		if (nglcd->doRescan) 
		{
		    nglcd->doRescan = false;
			nglcd->Update();
		}
		nglcd->lcd->DeInit();
		delete nglcd->lcd;
		nglcd->lcd = NULL;
	} while(!nglcd->doExit);

	return NULL;
}

void nGLCD::Update() 
{
	if (nglcd)
		sem_post(&nglcd->sem);
}

void nGLCD::StandbyMode(bool b) 
{
	if (nglcd) 
	{
		if (glcd_time_in_standby)
			nglcd->doStandbyTime = b;
		else
		    nglcd->doStandby = b;
		nglcd->doMirrorOSD = false;
		nglcd->Update();
	}
}

void nGLCD::ShowVolume(bool b) 
{
	if (nglcd) 
	{
		nglcd->doShowVolume = b;
		nglcd->Update();
	}
}

void nGLCD::MirrorOSD(bool b) 
{
	if (nglcd && glcd_mirror_osd) 
	{
		nglcd->doMirrorOSD = b;
		nglcd->Update();
	}
}

void nGLCD::Exit() 
{
	if (nglcd) 
	{
		nglcd->doMirrorOSD = false;
		nglcd->doSuspend = false;
		nglcd->doExit = true;
		nglcd->Update();
		void *res;
		pthread_join(nglcd->thrGLCD, &res);
		delete nglcd;
		nglcd = NULL;
	}
}

void nglcd_update() 
{
	if (nglcd) 
	{
		nglcd->Update();
	}
}

void nGLCD::Rescan() 
{
	if (nglcd) 
	{
		doRescan = true;
		nglcd->Update();
	}
}

void nGLCD::Suspend() 
{
	if (nglcd) 
	{
		nglcd->doSuspend = true;
		nglcd->Update();
	}
}

void nGLCD::Resume() 
{
	if (nglcd) 
	{
		nglcd->doSuspend = false;
		nglcd->Update();
	}
}

void nGLCD::lockChannel(std::string &c)
{
	if(nglcd) 
	{
		nglcd->Lock();
		nglcd->channelLocked = true;
		nglcd->stagingChannel = c;
		nglcd->stagingEpg = "";
		nglcd->Unlock();
		nglcd->Update();
	}
}

void nGLCD::unlockChannel(void)
{
	if(nglcd) {
		nglcd->channelLocked = false;
		nglcd->Update();
	}
}

//
bool nGLCD::load_png_element(const char * const filename, raw_nglcd_element_t * element)
{
	png_structp  png_ptr;
	png_infop    info_ptr;
	unsigned int i;
	unsigned int pass;
	unsigned int number_passes;
	int          bit_depth;
	int          color_type;
	int          interlace_type;
	png_uint_32  width;
	png_uint_32  height;
	png_byte *   fbptr;
	FILE *       fh;
	bool         ret_value = false;

	if ((fh = fopen(filename, "rb")))
	{
		if ((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)))
		{
			if (!(info_ptr = png_create_info_struct(png_ptr)))
				png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
			else
			{
#if (PNG_LIBPNG_VER < 10500)
				if (!(setjmp(png_ptr->jmpbuf)))
#else
				if (!setjmp(png_jmpbuf(png_ptr)))
#endif
				{
					unsigned int lcd_height = bitmap->Height();
					unsigned int lcd_width = bitmap->Width();

					png_init_io(png_ptr,fh);
					
					png_read_info(png_ptr, info_ptr);
					png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
					
					if (
						(width      <= lcd_width             ) &&
						(height     <= lcd_height            )
						)
					{
						printf("[nGLCD] %s %s %dx%dx%d, type %d\n", __FUNCTION__, filename, width, height, bit_depth, color_type);
						element->header.width = width;
						element->header.height = height;
						element->header.bpp = bit_depth;
						if (!element->buffer)
						{
							element->buffer_size = width*height*4;
							element->buffer = new unsigned char[element->buffer_size];
							lcd_width = width;
							lcd_height = height;
						}
						
						memset(element->buffer, 0, element->buffer_size);

						png_set_packing(png_ptr); /* expand to 1 byte blocks */
						
						if ((color_type == PNG_COLOR_TYPE_PALETTE) || (bit_depth < 24))
						      png_set_expand(png_ptr);

						if ((color_type & PNG_COLOR_MASK_COLOR) == PNG_COLOR_TYPE_GRAY)
#if (PNG_LIBPNG_VER < 10200)
							png_set_gray_to_rgb(png_ptr);
#else
							png_set_gray_to_rgb(png_ptr);
#endif
						
						number_passes = png_set_interlace_handling(png_ptr);
						png_read_update_info(png_ptr,info_ptr);
						
						png_uint_32 rb = png_get_rowbytes(png_ptr, info_ptr);
						if (width*4 == rb)
						{
							printf("[nGLCD] %s read data\n", __FUNCTION__);
							ret_value = true;
							
							for (pass = 0; pass < number_passes; pass++)
							{
								fbptr = (png_byte *)element->buffer;
								for (i = 0; i < element->header.height; i++)
								{
									png_read_row(png_ptr, fbptr, NULL);
									/* if the PNG is smaller, than the display width... */
									if (width < lcd_width)	/* clear the area right of the PNG */
										memset(fbptr + width*4, 0, (lcd_width - width)*4);
									fbptr += lcd_width*4;
								}
							}
							png_read_end(png_ptr, info_ptr);
						}
						else
						{
							printf("[nGLCD] %s %d != %d\n", __FUNCTION__, width*4, rb);
						}
					}
				}
				png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			}
		}
		fclose(fh);
	}
	return ret_value;
}

void nGLCD::draw_screen_element(const raw_nglcd_element_t * element, int x, int y, bool upscale)
{
	printf("%s:%s(%d) '%03d' x '%03d' \n", __FILE__, __FUNCTION__, __LINE__, x, y);

	uint32_t cl = 0;
	uint8_t * data = element->buffer;

	int xt, yt;
	uint32_t alpha;

	if (data)
	{
		for (yt = 0; yt < element->header.height; yt++)
		{
			uint8_t * rp = data + (yt * element->header.width)*4;
			for (xt = 0; xt < element->header.width; xt++)
			{
				cl = (rp[3] << 24) | (rp[0] << 16) | (rp[1] << 8) | rp[2];
				if (cl & 0xFF000000) // only draw if alpha > 0
				{
					if (upscale)
					{
						bitmap->DrawPixel(xt*2+0+x, yt*2+0+y, cl);
						bitmap->DrawPixel(xt*2+0+x, yt*2+1+y, cl);
						bitmap->DrawPixel(xt*2+1+x, yt*2+0+y, cl);
						bitmap->DrawPixel(xt*2+1+x, yt*2+1+y, cl);
					}
					else
						bitmap->DrawPixel(xt+x, yt+y, cl);
				}
				rp += 4;
			}
		}
	}
}

bool nGLCD::dump_png_element(const char * const filename, raw_nglcd_element_t * element)
{
	png_structp  png_ptr;
	png_infop    info_ptr;
	unsigned int i,j;
	png_byte *   fbptr;
	FILE *       fp;
	bool         ret_value = false;
 
        /* create file */
        fp = fopen(filename, "wb");
        if (!fp)
                printf("[nGLCD] File %s could not be opened for writing\n", filename);
	else
	{
	        /* initialize stuff */
        	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	        if (!png_ptr)
        	        printf("[nGLCD] png_create_write_struct failed\n");
		else
		{
		        info_ptr = png_create_info_struct(png_ptr);
		        if (!info_ptr)
				printf("[nGLCD] png_create_info_struct failed\n");
			else
			{
			        if (setjmp(png_jmpbuf(png_ptr)))
			                printf("[nGLCD] Error during init_io\n");
				else
				{
					unsigned int lcd_height = bitmap->Height();
					unsigned int lcd_width = bitmap->Width();
					unsigned int png_bpp = element->header.bpp;
					if (png_bpp > 8)
						png_bpp = 8;

        				png_init_io(png_ptr, fp);


        				/* write header */
        				if (setjmp(png_jmpbuf(png_ptr)))
        				        printf("[nGLCD] Error during writing header\n");

        				png_set_IHDR(png_ptr, info_ptr, element->header.width, element->header.height,
        				             png_bpp, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
        				             PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

        				png_color_8 sBIT;
        				sBIT.gray = 0;
        				sBIT.red = 8;
        				sBIT.green = 8;
        				sBIT.blue = 8;
        				sBIT.alpha = 0;
        				png_set_sBIT(png_ptr, info_ptr, &sBIT);

        				png_write_info(png_ptr, info_ptr);


        				/* write bytes */
					if (setjmp(png_jmpbuf(png_ptr)))
					{
        				        printf("[nGLCD] Error during writing bytes\n");
						return ret_value;
					}

					ret_value = true;

					fbptr = (png_byte *)element->buffer;
					png_bytep row = (png_bytep) malloc(3 * element->header.width * sizeof(png_byte));
					for (i = 0; i < element->header.height; i++)
					{
						for (j = 0; j < element->header.width; j++)
						{
							row[j*3+0] = fbptr[2];
							row[j*3+1] = fbptr[1];
							row[j*3+2] = fbptr[0];
							fbptr += 4;
						}
						png_write_row(png_ptr, row);
					}

					/* end write */
        				if (setjmp(png_jmpbuf(png_ptr)))
					{
						printf("[nGLCD] Error during end of write\n");
						return ret_value;
					}

        				png_write_end(png_ptr, NULL);
				}
			}
		}
        	fclose(fp);
	}

	return ret_value;
}

bool nGLCD::dump_png(const char * const filename)
{
	raw_nglcd_element_t element;
	element.buffer_size = bitmap->Width()*bitmap->Height()*4;
	element.buffer = (raw_ngdisplay_t)bitmap->Data();
	element.header.width = bitmap->Width();
	element.header.height = bitmap->Height();
	element.header.bpp = 32;
	return dump_png_element(filename, &element);
}

bool nGLCD::ShowPng(char *filename)
{
	//return display.load_png(filename);
	return false;
}

bool nGLCD::DumpPng(char *filename)
{
	return dump_png(filename);
}
//


