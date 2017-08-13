/*
 *
 * $Id: playback_cs.cpp,v 1.0 2013/08/18 11:23:30 mohousch Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>
#include <errno.h>

#include <sys/stat.h>

#include <pthread.h>
#include <syscall.h>

#include <unistd.h>

#include "playback_cs.h"

#include <driver/framebuffer.h>
#include <system/debug.h>


static const char * FILENAME = "[playback_cs.cpp]";


// global
bool isTS = false;

#if defined ENABLE_GSTREAMER
#include <gst/gst.h>
#include <gst/pbutils/missing-plugins.h>

#if defined (USE_OPENGL)
#include <gst/interfaces/xoverlay.h>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glx.h>

extern int GLWinID;
extern int GLxStart;
extern int GLyStart;
extern int GLWidth;
extern int GLHeight;
#endif

GstElement * m_gst_playbin = NULL;
GstElement * audioSink = NULL;
GstElement * videoSink = NULL;
GstElement * sink = NULL;
gchar * uri = NULL;
GstBus * bus = NULL;
bool end_eof = false;
#define HTTP_TIMEOUT 30
#else
#include <common.h>
#include <subtitle.h>
#include <linux/fb.h>

extern OutputHandler_t		OutputHandler;
extern PlaybackHandler_t	PlaybackHandler;
extern ContainerHandler_t	ContainerHandler;
extern ManagerHandler_t		ManagerHandler;

static Context_t * player = NULL;
#endif

#if defined ENABLE_GSTREAMER
gint match_sinktype(GstElement *element, gpointer type)
{
	return strcmp(g_type_name(G_OBJECT_TYPE(element)), (const char*)type);
}

//
void playbinNotifySource(GObject *object, GParamSpec *unused, gpointer /*user_data*/)
{
	GstElement *source = NULL;
	g_object_get(object, "source", &source, NULL);

	if (source)
	{
		if (g_object_class_find_property(G_OBJECT_GET_CLASS(source), "timeout") != 0)
		{
			GstElementFactory *factory = gst_element_get_factory(source);
			if (factory)
			{
				const gchar *sourcename = gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(factory));
				if (!strcmp(sourcename, "souphttpsrc"))
				{
					g_object_set(G_OBJECT(source), "timeout", HTTP_TIMEOUT, NULL);
				}
			}
		}

		if (g_object_class_find_property(G_OBJECT_GET_CLASS(source), "ssl-strict") != 0)
		{
			g_object_set(G_OBJECT(source), "ssl-strict", FALSE, NULL);
		}
		
		gst_object_unref(source);
	}
}
//

GstBusSyncReply Gst_bus_call(GstBus * /*bus*/, GstMessage * msg, gpointer /*user_data*/)
{
	//source name
	gchar * sourceName;
	
	// source
	GstObject * source;
	source = GST_MESSAGE_SRC(msg);
	
	if (!GST_IS_OBJECT(source))
		return GST_BUS_DROP;
	
	sourceName = gst_object_get_name(source);

	switch (GST_MESSAGE_TYPE(msg)) 
	{
		case GST_MESSAGE_EOS: 
		{
			g_message("End-of-stream");
			
			//
			dprintf(DEBUG_NORMAL, "cPlayback::%s (EOS) !!!!EOF!!!! << -1\n", __func__);
			end_eof = true;
			//
			break;
		}
		
		case GST_MESSAGE_ERROR: 
		{
			gchar * debug1;
			GError *err;
			gst_message_parse_error(msg, &err, &debug1);
			g_free(debug1);
			
			printf("cPlayback:: Gstreamer error: %s (%i)\n", err->message, err->code );
			
			if ( err->domain == GST_STREAM_ERROR )
			{
				if ( err->code == GST_STREAM_ERROR_CODEC_NOT_FOUND )
				{
					if ( g_strrstr(sourceName, "videosink") )
						printf("%s %s - videoSink\n", FILENAME, __FUNCTION__);	//FIXME: how shall playback handle this event???
					else if ( g_strrstr(sourceName, "audiosink") )
						printf("%s %s - audioSink\n", FILENAME, __FUNCTION__); //FIXME: how shall playback handle this event???
				}
			}
			g_error_free(err);
			
			end_eof = true;
			
			break;
		}

		case GST_MESSAGE_INFO:
		{
			gchar * _debug;
			GError * inf;
	
			gst_message_parse_info (msg, &inf, &_debug);
			g_free(_debug);
			
			if ( inf->domain == GST_STREAM_ERROR && inf->code == GST_STREAM_ERROR_DECODE )
			{
				if ( g_strrstr(sourceName, "videosink") )
					printf("%s %s - videoSink\n", FILENAME, __FUNCTION__); //FIXME: how shall playback handle this event???
			}
			g_error_free(inf);
			break;
		}

#if 0
#if GST_VERSION_MAJOR >= 1
		case GST_MESSAGE_WARNING:
		{
			gchar *debug_warn = NULL;
			GError *warn = NULL;
			gst_message_parse_warning (msg, &warn, &debug_warn);
			/* CVR this Warning occurs from time to time with external srt files
			When a new seek is done the problem off to long wait times before subtitles appears,
			after movie was restarted with a resume position is solved. */
			if(!strncmp(warn->message , "Internal data flow problem", 26) && !strncmp(sourceName, "subtitle_sink", 13))
			{
				printf("[eServiceMP3] Gstreamer warning : %s (%i) from %s" , warn->message, warn->code, sourceName);
				subsink = gst_bin_get_by_name(GST_BIN(m_gst_playbin), "subtitle_sink");
				if(subsink)
				{
					if (!gst_element_seek (subsink, m_currentTrickRatio, GST_FORMAT_TIME, (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
						GST_SEEK_TYPE_SET, m_last_seek_pos,
						GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE))
					{
						printf("[eServiceMP3] seekToImpl subsink failed\n");
					}
					gst_object_unref(subsink);
				}
			}
			g_free(debug_warn);
			g_error_free(warn);
			break;
		}
#endif
#endif
		
		case GST_MESSAGE_STATE_CHANGED:
		{
			if(GST_MESSAGE_SRC(msg) != GST_OBJECT(m_gst_playbin))
				break;

			GstState old_state, new_state;
			gst_message_parse_state_changed(msg, &old_state, &new_state, NULL);
			
			if(old_state == new_state)
				break;
		
			printf("cPlayback::state transition %s -> %s\n", gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));
		
			GstStateChange transition = (GstStateChange)GST_STATE_TRANSITION(old_state, new_state);
		
			switch(transition)
			{
				case GST_STATE_CHANGE_NULL_TO_READY:
				{
				}
				break;
				
				case GST_STATE_CHANGE_READY_TO_PAUSED:
				{
#if GST_VERSION_MAJOR >= 1
					GValue result = { 0, };
#endif
					GstIterator * children;
					
					if (audioSink)
					{
						gst_object_unref(GST_OBJECT(audioSink));
						audioSink = NULL;
						dprintf(DEBUG_NORMAL, "%s %s - audio sink closed\n", FILENAME, __FUNCTION__);
					}
					
					if (videoSink)
					{
						gst_object_unref(GST_OBJECT(videoSink));
						videoSink = NULL;
						dprintf(DEBUG_NORMAL, "%s %s - video sink closed\n", FILENAME, __FUNCTION__);
					}
					
					// set audio video sink
					children = gst_bin_iterate_recurse(GST_BIN(m_gst_playbin));
					
#if GST_VERSION_MAJOR < 1					
					audioSink = GST_ELEMENT_CAST(gst_iterator_find_custom(children, (GCompareFunc)match_sinktype, (gpointer)"GstDVBAudioSink"));
#else
					if (gst_iterator_find_custom(children, (GCompareFunc)match_sinktype, &result, (gpointer)"GstDVBAudioSink"))
					{
						audioSink = GST_ELEMENT_CAST(g_value_dup_object(&result));
						g_value_unset(&result);
					}
#endif	
					
					if(audioSink)
						dprintf(DEBUG_NORMAL, "%s %s - audio sink created\n", FILENAME, __FUNCTION__);
					
#if GST_VERSION_MAJOR < 1
					videoSink = GST_ELEMENT_CAST(gst_iterator_find_custom(children, (GCompareFunc)match_sinktype, (gpointer)"GstDVBVideoSink"));
#else
					if (gst_iterator_find_custom(children, (GCompareFunc)match_sinktype, &result, (gpointer)"GstDVBVideoSink"))
					{
						videoSink = GST_ELEMENT_CAST(g_value_dup_object(&result));
						g_value_unset(&result);
					}
#endif

					if(videoSink)
						dprintf(DEBUG_NORMAL, "%s %s - video sink created\n", FILENAME, __FUNCTION__);
					
					gst_iterator_free(children);
				}
				break;
				
				case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
				{
				}
				break;
				
				case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
				{
				}	
				break;
				
				case GST_STATE_CHANGE_PAUSED_TO_READY:
				{
					if (audioSink)
					{
						gst_object_unref(GST_OBJECT(audioSink));
						audioSink = NULL;
						dprintf(DEBUG_NORMAL, "%s %s - audio sink closed\n", FILENAME, __FUNCTION__);
					}
					if (videoSink)
					{
						gst_object_unref(GST_OBJECT(videoSink));
						videoSink = NULL;
						dprintf(DEBUG_NORMAL, "%s %s - video sink closed\n", FILENAME, __FUNCTION__);
					}
				}	
				break;
				
				case GST_STATE_CHANGE_READY_TO_NULL:
				{
				}	
				break;
			}
			break;
		}
#if defined (USE_OPENGL) //FIXME: ???	
		case GST_MESSAGE_ELEMENT:
		{
			if( gst_structure_has_name(gst_message_get_structure(msg), "prepare-xwindow-id") || gst_structure_has_name(gst_message_get_structure(msg), "have-xwindow-id") ) 
			{
				// set window id
				//NOTE: comment out if you want to overlay video window on osd window(still bugy)
				#if 0
				sink = gst_element_factory_make ("directdrawsink", "sink");
				GstXOverlay* ov = (GstXOverlay*)sink;

				gst_x_overlay_set_xwindow_id(/*GST_X_OVERLAY(GST_MESSAGE_SRC (msg))*/ov, (gulong)GLWinID);
				#endif
			}
		}
		break;
#endif		
			
		default:
			break;
	}
	
	g_free(sourceName);
	

	return GST_BUS_DROP;
}
#endif

/* its called only one time, (mainmenu/movieplayergui init)*/
cPlayback::cPlayback(int /*num*/)
{ 
	dprintf(DEBUG_NORMAL, "%s:%s\n", FILENAME, __FUNCTION__);

	/* init gstreamer */
#if ENABLE_GSTREAMER
	gst_init(NULL, NULL);
	
	dprintf(DEBUG_NORMAL, "%s:%s gst initialized\n", FILENAME, __FUNCTION__);
#endif	
}

/* called at housekepping */
cPlayback::~cPlayback()
{  
	dprintf(DEBUG_NORMAL, "%s:%s\n", FILENAME, __FUNCTION__);
}

bool cPlayback::Open()
{
	dprintf(DEBUG_NORMAL, "%s:%s\n", FILENAME, __FUNCTION__ );
	
	mAudioStream = 0;
	mSpeed = 0;
	playing = false;
	
#if defined (ENABLE_GSTREAMER)
	// create gst pipeline
#if GST_VERSION_MAJOR < 1
	m_gst_playbin = gst_element_factory_make("playbin2", "playbin");
#else
	m_gst_playbin = gst_element_factory_make("playbin", "playbin");
#endif
#else
	player = (Context_t*)malloc(sizeof(Context_t));

	//init player
	if(player) 
	{
		player->playback	= &PlaybackHandler;
		player->output		= &OutputHandler;
		player->container	= &ContainerHandler;
		player->manager		= &ManagerHandler;
	}

	// registration of output devices
	if(player && player->output) 
	{
		player->output->Command(player,OUTPUT_ADD, (void*)"audio");
		player->output->Command(player,OUTPUT_ADD, (void*)"video");		
		//player->output->Command(player,OUTPUT_ADD, (void*)"subtitle");		
	}

	// subtitle
	/*
	SubtitleOutputDef_t out;

	out.screen_width = CFrameBuffer::getInstance()->getScreenWidth();
	out.screen_height = CFrameBuffer::getInstance()->getScreenHeight();
	out.framebufferFD = CFrameBuffer::getInstance()->getFileHandle();
	out.destination   = (unsigned char *)CFrameBuffer::getInstance()->getFrameBufferPointer();
	out.destStride    = CFrameBuffer::getInstance()->getStride();;
	out.shareFramebuffer = 1;
    
	player->output->subtitle->Command(player, (OutputCmd_t)OUTPUT_SET_SUBTITLE_OUTPUT, (void*) &out);
	*/
#endif

	return true;
}

void cPlayback::Close(void)
{  
	dprintf(DEBUG_NORMAL, "%s:%s\n", FILENAME, __FUNCTION__);
	
	Stop();
	
#if ENABLE_GSTREAMER
	end_eof = false;
	
	// disconnect bus handler
	if (m_gst_playbin)
	{
		// disconnect sync handler callback
		bus = gst_pipeline_get_bus(GST_PIPELINE (m_gst_playbin));
#if GST_VERSION_MAJOR < 1
		gst_bus_set_sync_handler(bus, NULL, NULL);
#else
		gst_bus_set_sync_handler(bus, NULL, NULL, NULL);
#endif
		gst_object_unref(bus);
		
		dprintf(DEBUG_NORMAL, "GST bus handler closed\n");
	}
	
	// sometimes video/audio event poll close only needed device, so be sure and decrease them 
	if (audioSink)
	{
		gst_object_unref(GST_OBJECT(audioSink));
		audioSink = NULL;
		dprintf(DEBUG_NORMAL, "%s %s - audio sink closed\n", FILENAME, __FUNCTION__);
	}
	
	if (videoSink)
	{
		gst_object_unref(GST_OBJECT(videoSink));
		videoSink = NULL;
		dprintf(DEBUG_NORMAL, "%s %s - audio sink closed\n", FILENAME, __FUNCTION__);
	}

	// close gst
	if (m_gst_playbin)
	{
		// unref m_gst_playbin
		gst_object_unref (GST_OBJECT (m_gst_playbin));
		m_gst_playbin = NULL;
		
		dprintf(DEBUG_NORMAL, "GST playbin closed\n");
	}
#else
	if(player && player->output) 
	{
		player->output->Command(player, OUTPUT_CLOSE, NULL);
		player->output->Command(player,OUTPUT_DEL, (void*)"audio");
		player->output->Command(player,OUTPUT_DEL, (void*)"video");		
		//player->output->Command(player,OUTPUT_DEL, (void*)"subtitle");	
	}

	if(player && player->playback)
		player->playback->Command(player, PLAYBACK_CLOSE, NULL);


	if(player)
		free(player);

	if(player != NULL)
		player = NULL;
#endif	
}

// start
bool cPlayback::Start(char *filename)
{
	dprintf(DEBUG_NORMAL, "%s:%s - filename=%s\n", FILENAME, __FUNCTION__, filename);
	
	//
	std::string file("");
	bool isHTTP = false;
	isTS = false;

	if(!strncmp("http://", filename, 7))
	{
		isHTTP = true;
	}
	else if(!strncmp("https://", filename, 8))
	{
		isHTTP = true;
	}
	else if(!strncmp("upnp://", filename, 7))
	{
		isHTTP = true;
	}
	else if(!strncmp("rtmp://", filename, 7))
	{
		isHTTP = true;
	}
	else if(!strncmp("rtsp://", filename, 7))
	{
		isHTTP = true;
	}
	else if(!strncmp("mms://", filename, 6))
	{
		isHTTP = true;
	}
	else if(!strncmp("file://", filename, 7))
	{
		isHTTP = false;
	}
	else
		file = "file://";
	
	file.append(filename);
	
	if (file.rfind(".ts") == file.length() - 3 )
		isTS = true;

#if defined (ENABLE_GSTREAMER)
	end_eof = false;
	
	int m_buffer_size = 5*1024*1024;
	int flags = 0x47; //(GST_PLAY_FLAG_VIDEO | GST_PLAY_FLAG_AUDIO | GST_PLAY_FLAG_NATIVE_VIDEO | GST_PLAY_FLAG_TEXT);
	
	if (isHTTP)
		uri = g_strdup_printf("%s", filename);
	else
		uri = g_filename_to_uri(filename, NULL, NULL);

	if(m_gst_playbin)
	{
		// set uri
		g_object_set(G_OBJECT (m_gst_playbin), "uri", uri, NULL);	

		// increase the default 2 second / 2 MB buffer limitations to 5s / 5MB
		if(isHTTP)
		{
			g_signal_connect (G_OBJECT (m_gst_playbin), "notify::source", G_CALLBACK (playbinNotifySource), NULL);

			// set buffer size
			g_object_set(G_OBJECT(m_gst_playbin), "buffer-size", m_buffer_size, NULL);
			g_object_set(G_OBJECT(m_gst_playbin), "buffer-duration", 5LL * GST_SECOND, NULL);
			flags |= 0x100; // USE_BUFFERING
		}
		
		// set flags
		g_object_set(G_OBJECT (m_gst_playbin), "flags", flags, NULL);	
		
		//gstbus handler
		bus = gst_pipeline_get_bus(GST_PIPELINE (m_gst_playbin));

#if GST_VERSION_MAJOR < 1
		gst_bus_set_sync_handler(bus, Gst_bus_call, NULL);
#else
		gst_bus_set_sync_handler(bus, Gst_bus_call, NULL, NULL);	
#endif
		gst_object_unref(bus);
		
		// start playing
		gst_element_set_state(GST_ELEMENT(m_gst_playbin), GST_STATE_PLAYING);
		playing = true;
		mSpeed = 1;
	}
	else
	{
		gst_object_unref(GST_OBJECT(m_gst_playbin));

		m_gst_playbin = 0;
		
		dprintf(DEBUG_NORMAL, "failed to create GStreamer pipeline!, sorry we can not play\n");
		playing = false;
	}
	
	g_free(uri);
#else	
	//open file
	if(player && player->playback && player->playback->Command(player, PLAYBACK_OPEN, (char *)file.c_str()) >= 0) 
	{
		// play it baby 
		if(player && player->output && player->playback) 
		{
        		player->output->Command(player, OUTPUT_OPEN, NULL);
			
			if (player->playback->Command(player, PLAYBACK_PLAY, NULL) == 0 ) // playback.c uses "int = 0" for "true"
			{
				playing = true;
				mSpeed = 1;
			}
		}		
	}
	else
	{
		dprintf(DEBUG_NORMAL, "[playback_cs.cpp]: failed to start playing file, sorry we can not play\n");
		playing = false;
	}
#endif

	dprintf(DEBUG_NORMAL, "%s:%s (playing %d)\n", FILENAME, __FUNCTION__, playing);	

	return playing;
}

bool cPlayback::Play(void)
{
	dprintf(DEBUG_NORMAL, "%s:%s (playing %d)\n", FILENAME, __FUNCTION__, playing);	

	if(playing == true) 
		return true;
	
#if defined (ENABLE_GSTREAMER)
	if(m_gst_playbin)
	{
		gst_element_set_state(GST_ELEMENT(m_gst_playbin), GST_STATE_PLAYING);
		
		playing = true;
	}
#else
	if(player && player->output && player->playback) 
	{
        	player->output->Command(player, OUTPUT_OPEN, NULL);
			
		if (player->playback->Command(player, PLAYBACK_PLAY, NULL) == 0 ) // playback.c uses "int = 0" for "true"
		{
			playing = true;
		}
	}
#endif

	dprintf(DEBUG_INFO, "%s:%s (playing %d)\n", FILENAME, __FUNCTION__, playing);

	return playing;
}

bool cPlayback::Stop(void)
{ 
	if(playing == false) 
		return false;
	
	dprintf(DEBUG_NORMAL, "%s:%s (playing %d)\n", FILENAME, __FUNCTION__, playing);

#if defined (ENABLE_GSTREAMER)
	// stop
	if(m_gst_playbin)
	{
		gst_element_set_state(m_gst_playbin, GST_STATE_NULL);
	}
#else
	if(player && player->playback && player->output) 
		player->playback->Command(player, PLAYBACK_STOP, NULL);
	
	if(player && player->container && player->container->selectedContainer)
		player->container->selectedContainer->Command(player, CONTAINER_STOP, NULL);
#endif

	playing = false;
	
	dprintf(DEBUG_INFO, "%s:%s (playing %d)\n", FILENAME, __FUNCTION__, playing);

	return true;
}

bool cPlayback::SetAPid(unsigned short pid, int /*_ac3*/)
{
	dprintf(DEBUG_NORMAL, "%s:%s curpid:%d nextpid:%d\n", FILENAME, __FUNCTION__, mAudioStream, pid);
	
#if ENABLE_GSTREAMER
	if(pid != mAudioStream)
	{
		g_object_set (G_OBJECT (m_gst_playbin), "current-audio", pid, NULL);
		printf("%s: switched to audio stream %i\n", __FUNCTION__, pid);
		mAudioStream = pid;
	}
#else
	int track = pid;

	if(pid != mAudioStream)
	{
		if(player && player->playback)
				player->playback->Command(player, PLAYBACK_SWITCH_AUDIO, (void*)&track);

		mAudioStream = pid;
	}
#endif	

	return true;
}

#if ENABLE_GSTREAMER
void cPlayback::trickSeek(double ratio)
{	
	bool validposition = false;
	gint64 pos = 0;
	int64_t position;
	int64_t duration;
	
	// pause
	if (ratio > -0.01 && ratio < 0.01)
	{
		gst_element_set_state(m_gst_playbin, GST_STATE_PAUSED);
		return;
	}
	
	if( GetPosition(position, duration) )
	{
		validposition = true;
		pos = position*1000000;
	}

	gst_element_set_state(m_gst_playbin, GST_STATE_PLAYING);
			
	if (validposition)
	{
		if(ratio >= 0.0)
			gst_element_seek(m_gst_playbin, ratio, GST_FORMAT_TIME, (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_SKIP), GST_SEEK_TYPE_SET, pos, GST_SEEK_TYPE_SET, -1);
		else
			gst_element_seek(m_gst_playbin, ratio, GST_FORMAT_TIME, (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_SKIP), GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_SET, pos);
	}
}
#endif

bool cPlayback::SetSpeed(int speed)
{  
	dprintf(DEBUG_NORMAL, "%s:%s speed %d\n", FILENAME, __FUNCTION__, speed);	

	if(playing == false) 
		return false;

#if defined (ENABLE_GSTREAMER)
	if(m_gst_playbin)
	{
		if(speed == 0)
			trickSeek(0.0);
		else if(speed == 1)
			trickSeek(1.0);
		else
			trickSeek(speed);
	}
#else
	int speedmap = 0;
	
	if(player && player->playback) 
	{
		if(speed > 1) 		//forwarding
		{
			//
			if (speed > 7) 
				speed = 7;
			
			switch(speed)
			{
				case 2: speedmap = 3; break;
				case 3: speedmap = 7; break;
				case 4: speedmap = 15; break;
				case 5: speedmap = 31; break;
				case 6: speedmap = 63; break;
				case 7: speedmap = 127; break;
			}

			player->playback->Command(player, PLAYBACK_FASTFORWARD, (void*)&speedmap);
		}
		else if(speed == 0)	//pausing
		{
			player->playback->Command(player, PLAYBACK_PAUSE, NULL);
		}
		else if (speed < 0)	//backwarding
		{
			//
			if (speed > -1) 
				speed = -1;
			
			if (speed < -7) 
				speed = -7;
			
			switch(speed)
			{
				case -1: speedmap = -5; break;
				case -2: speedmap = -10; break;
				case -3: speedmap = -20; break;
				case -4: speedmap = -40; break;
				case -5: speedmap = -80; break;
				case -6: speedmap = -160; break;
				case -7: speedmap = -320; break;
			}
			
			player->playback->Command(player, PLAYBACK_FASTBACKWARD, (void*)&speedmap);

			// trickseek
			//player->playback->Command(player, PLAYBACK_SEEK, (void*)&speedmap);
		}
		else if(speed == 1) 	//continue
		{
			player->playback->Command(player, PLAYBACK_CONTINUE, NULL);
		}
	}
#endif

	mSpeed = speed;

	return true;
}

bool cPlayback::SetSlow(int slow)
{  
	dprintf(DEBUG_NORMAL, "%s:%s (playing %d)\n", FILENAME, __FUNCTION__, playing);	

	if(playing == false) 
		return false;

#if ENABLE_GSTREAMER
	if(m_gst_playbin)
	{
		trickSeek(0.5);
	}
#else
	if(player && player->playback) 
	{
		player->playback->Command(player, PLAYBACK_SLOWMOTION, (void*)&slow);
	}
#endif

	mSpeed = slow;

	return true;
}

bool cPlayback::GetSpeed(int &speed) const
{
	speed = mSpeed;

	return true;
}

// in milliseconds
bool cPlayback::GetPosition(int64_t &position, int64_t &duration)
{
	if(playing == false) 
		return false;

#if ENABLE_GSTREAMER
	if(end_eof)
	{
		dprintf(DEBUG_NORMAL, "cPlayback::%s !!!!EOF!!!! < -1\n", __FUNCTION__);
		return false;
	}
	
	if(m_gst_playbin)
	{
		GstFormat fmt = GST_FORMAT_TIME; //Returns time in nanosecs
		
		// position
		gint64 pts;
		position = 0;
		
#if defined (USE_OPENGL)
#if GST_VERSION_MAJOR < 1
		gst_element_query_position(m_gst_playbin, &fmt, &pts);
#else
		gst_element_query_position(m_gst_playbin, fmt, &pts);
#endif
#else
		
		if(audioSink || videoSink)
		{
			g_signal_emit_by_name(audioSink ? audioSink : videoSink, "get-decoder-time", &pts);
			GST_CLOCK_TIME_IS_VALID(pts);
		}
		
#if GST_VERSION_MAJOR < 1
		gst_element_query_position(m_gst_playbin, &fmt, &pts);
#else
		gst_element_query_position(m_gst_playbin, fmt, &pts);
#endif
#endif		
			
		position = pts / 1000000;	// in ms
		
		dprintf(DEBUG_DEBUG, "%s: position: %lld ms ", __FUNCTION__, position);
		
		//duration
		gint64 len;

#if GST_VERSION_MAJOR < 1
		gst_element_query_duration(m_gst_playbin, &fmt, &len);
#else
		gst_element_query_duration(m_gst_playbin, fmt, &len);
#endif
		
		duration = len / 1000000;	// in ms

		dprintf(DEBUG_DEBUG, "(duration: %lld ms)\n", duration);
	}
#else
	if (player && player->playback && !player->playback->isPlaying) 
	{	  
		dprintf(DEBUG_NORMAL, "cPlayback::%s !!!!EOF!!!! < -1\n", __func__);
		
		playing = false;
	
		return false;
	} 

	// position
	unsigned long long int vpts = 0;

	if(player && player->playback)
		player->playback->Command(player, PLAYBACK_PTS, &vpts);

	position = vpts/90;
	
	dprintf(DEBUG_DEBUG, "%s: position: %lld ms ", __FUNCTION__, position);
	
	// duration
	double length = 0;

	if(player && player->playback)
		player->playback->Command(player, PLAYBACK_LENGTH, &length);
	
	if(length < 0) 
		length = 0;

	duration = (int)(length*1000);
	
	dprintf(DEBUG_DEBUG, "(duration: %lld ms)\n", duration);
#endif
	
	return true;
}

bool cPlayback::SetPosition(int64_t position)
{
	if(playing == false) 
		return false;
	
	dprintf(DEBUG_NORMAL, "%s:%s position: %lld\n", FILENAME, __FUNCTION__, position);
	
#if ENABLE_GSTREAMER
	gint64 time_nanoseconds;
		
	if(m_gst_playbin)
	{
		time_nanoseconds = (position * 1000000.0);
		if(time_nanoseconds < 0) 
			time_nanoseconds = 0;
		
		gst_element_seek(m_gst_playbin, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, time_nanoseconds, GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
	}
#else
	float pos = (position/1000.0);

	if(player && player->playback)
		player->playback->Command(player, PLAYBACK_SEEK, (void*)&pos);
#endif

	return true;
}

void cPlayback::FindAllPids(uint16_t *apids, unsigned short *ac3flags, uint16_t *numpida, std::string *language)
{ 
	dprintf(DEBUG_NORMAL, "%s:%s\n", FILENAME, __FUNCTION__);

#if defined (ENABLE_GSTREAMER)
	if(m_gst_playbin)
	{
		gint i, n_audio = 0;
		
		// get audio
		g_object_get (m_gst_playbin, "n-audio", &n_audio, NULL);
		dprintf(DEBUG_NORMAL, "%s:%s: %d audio\n", FILENAME, __FUNCTION__, n_audio);
		
		if(n_audio == 0)
			return;

		language->clear();
		
		for (i = 0; i < n_audio; i++)
		{
			// apids
			apids[i] = i;
			
			GstPad * pad = 0;
			g_signal_emit_by_name (m_gst_playbin, "get-audio-pad", i, &pad);

#if GST_VERSION_MAJOR < 1
			GstCaps* caps = gst_pad_get_negotiated_caps(pad);
#else
			GstCaps* caps = gst_pad_get_current_caps(pad);
#endif
			gst_object_unref(pad);

			if (!caps)
				continue;

			// codec|language
			gchar * g_lang = NULL;
			gchar* g_codec = NULL;
			GstTagList *tags = NULL;


			g_signal_emit_by_name (m_gst_playbin, "get-audio-tags", i, &tags);
#if GST_VERSION_MAJOR < 1
			if (tags && gst_is_tag_list(tags))
#else
			if (tags && GST_IS_TAG_LIST(tags))
#endif
			{
				// language
				language[i] = "Stream";
				if (gst_tag_list_get_string(tags, GST_TAG_LANGUAGE_CODE, &g_lang))
				{
					printf("language:%s\n", std::string(g_lang).c_str());
					language[i] = std::string(g_lang).c_str();
					g_free(g_lang);
				}

				// codec
				if (gst_tag_list_get_string(tags, GST_TAG_AUDIO_CODEC, &g_codec))
				{
					printf("codec:%s\n", std::string(g_codec).c_str());

					/*
					if(std::string(g_codec) == "MPEG 1 Audio, Layer 2")
						ac3flags[i] = 3;
					else if(std::string(g_codec) == "Dolby Digital (AC-3)")
						ac3flags[i] = 1;
					else if(std::string(g_codec) == "MPEG-4 AAC")
						ac3flags[i] = 5;
					else if(std::string(g_codec) == "MPEG 1 Audio, Layer 3 (MP3)")
						ac3flags[i] = 4;
					*/

					language[i] += " (";
					language[i] += std::string(g_codec).c_str();
					language[i] += ")";
					
					g_free(g_codec);
				}

				gst_tag_list_free(tags);
			}
			
			gst_caps_unref(caps);
		}
		
		// numpids
		*numpida = i;
	}
#else
	// audio pids
	if(player && player->manager && player->manager->audio) 
	{
		char ** TrackList = NULL;
		player->manager->audio->Command(player, MANAGER_LIST, &TrackList);

		if (TrackList != NULL) 
		{
			printf("AudioTrack List\n");
			int i = 0,j = 0;

			for (i = 0, j = 0; TrackList[i] != NULL; i += 2, j++) 
			{
				printf("\t%s - %s\n", TrackList[i], TrackList[i + 1]);
				apids[j] = j;

				if( (!strncmp("A_MPEG/L3", TrackList[i + 1], 9)) || (!strncmp("A_MP3", TrackList[i + 1], 5)) )
					ac3flags[j] = 4;
				else if(!strncmp("A_AC3", TrackList[i + 1], 5))
					ac3flags[j] = 1;
				else if(!strncmp("A_DTS", TrackList[i + 1], 5))
					ac3flags[j] = 6;
				else if(!strncmp("A_AAC", TrackList[i + 1], 5))
					ac3flags[j] = 5;
				else if(!strncmp("A_PCM", TrackList[i + 1], 5))
					ac3flags[j] = 0; 	//todo
				else if(!strncmp("A_VORBIS", TrackList[i + 1], 8))
					ac3flags[j] = 0;	//todo
				else if(!strncmp("A_FLAC", TrackList[i + 1], 6))
					ac3flags[j] = 0;	//todo
				else
					ac3flags[j] = 0;	//todo

				language[j] = TrackList[i];
				
				free(TrackList[i]);
				free(TrackList[i+1]);
			}
			free(TrackList);
			*numpida = j;
		}
	}
#endif	
}

