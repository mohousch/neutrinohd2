/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: audioplay.cpp 2013/10/12 mohousch Exp $

	audioplayer
	Copyright (C) 2002 Bjoern Kalkbrenner <terminar@cyberphoria.org>
	Copyright (C) 2002,2003 Dirch
	Copyright (C) 2002,2003,2004 Zwen
	Homepage: http://www.dbox2.info/

	Kommentar:

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

#include "global.h"
#include <stdio.h>
#include <fcntl.h>
#include <sched.h>

#include <neutrino.h>
#include <driver/audioplay.h>

#include <system/debug.h>

#include <playback_cs.h>


extern cPlayback *playback;

void CAudioPlayer::stop()
{
	state = CBaseDec::STOP_REQ;
	
	if(playback->playing)
		playback->Close();

	//
	if(thrPlay)
		pthread_join(thrPlay, NULL);
	
	thrPlay = 0;
	
	state = CBaseDec::STOP;
}

void CAudioPlayer::pause()
{
	if(state == CBaseDec::PLAY || state == CBaseDec::FF || state == CBaseDec::REV)
	{
		state = CBaseDec::PAUSE;

		playback->SetSpeed(0);
	}
	else if(state == CBaseDec::PAUSE)
	{
		state = CBaseDec::PLAY;

		playback->SetSpeed(1);		
	}
}

void CAudioPlayer::ff(unsigned int seconds)
{
	m_SecondsToSkip = seconds;

	if(state == CBaseDec::PLAY || state == CBaseDec::PAUSE || state == CBaseDec::REV)
	{
		state = CBaseDec::FF;

		playback->SetSpeed(2);
	}
	else if(state == CBaseDec::FF)
	{
		state = CBaseDec::PLAY;
		
		playback->SetSpeed(1);
	}
}

void CAudioPlayer::rev(unsigned int seconds)
{
	m_SecondsToSkip = seconds;

	if(state == CBaseDec::PLAY || state == CBaseDec::PAUSE || state == CBaseDec::FF)
	{
		state = CBaseDec::REV;

		playback->SetSpeed(-2);
	}
	else if(state == CBaseDec::REV)
	{
		state = CBaseDec::PLAY;
		
		playback->SetSpeed(1);
	}
}

CAudioPlayer * CAudioPlayer::getInstance()
{
	static CAudioPlayer * AudioPlayer = NULL;
	
	if(AudioPlayer == NULL)
	{
		AudioPlayer = new CAudioPlayer();
	}
	
	return AudioPlayer;
}

void ShoutcastCallback(void *arg)
{
	CAudioPlayer::getInstance()->sc_callback(arg);
}

void * CAudioPlayer::PlayThread( void * /*dummy*/ )
{
	//stop playing if already playing (multiselect)
	if(playback->playing)
		playback->Close();
	
#if defined (PLATFORM_COOLSTREAM)
	if(! playback->Open(PLAYMODE_FILE))
#else	
	if(! playback->Open())
#endif	  
		return NULL;
	
#if defined (PLATFORM_COOLSTREAM)
	if(!playback->Start( (char *)getInstance()->m_Audiofile.Filename.c_str(), 0, 0, 0, 0, 0 ))
#else		
	if(!playback->Start( (char *)getInstance()->m_Audiofile.Filename.c_str() ))
#endif
		return NULL;
		
	getInstance()->state = CBaseDec::PLAY;
	
	int position = 0;
	int duration = 0;
	
	do {
#if defined (PLATFORM_COOLSTREAM)
		if(!playback->GetPosition(position, duration))
#else
		if(!playback->GetPosition((int64_t &)position, (int64_t &)duration))
#endif		
		{
			getInstance()->state = CBaseDec::STOP;
			
			if(playback->playing)
				playback->Close();
				
			break;	
		}
		getInstance()->m_played_time = position/1000;	// in sec
	}while(getInstance()->state != CBaseDec::STOP_REQ);
	
	getInstance()->state = CBaseDec::STOP;
	
	//if(playback->playing)
	//	playback->Close();
	
	pthread_exit(0);

	return NULL;
}

bool CAudioPlayer::play(const CAudiofile *file, const bool highPrio)
{
	if (state != CBaseDec::STOP)
		stop();

	getInstance()->clearFileData();

	/* 
	transfer information from CAudiofile to member variable, so that it does not have to be gathered again
	this assignment is important, otherwise the player would crash if the file currently played was deleted from the playlist
	*/
	m_Audiofile = *file;
	
	state = CBaseDec::PLAY;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	if(highPrio)
	{
		struct sched_param param;
		pthread_attr_setschedpolicy(&attr, SCHED_RR);
		param.sched_priority = 1;
		pthread_attr_setschedparam(&attr, &param);
		usleep(100000); // give the event thread some time to handle his stuff without this sleep there were duplicated events...
	}

	bool ret = true;

	// play thread (retrieve position/duration etc...)
	if (pthread_create (&thrPlay, &attr, PlayThread, (void*)&ret) != 0 )
	{
		perror("audioplay: pthread_create(PlayThread)");
		ret = false;
	}	

	pthread_attr_destroy(&attr);

	return ret;
}

CAudioPlayer::CAudioPlayer()
{
	init();
}

void CAudioPlayer::init()
{
	CBaseDec::Init();
	
	state = CBaseDec::STOP;	
	thrPlay = 0;	
}

void CAudioPlayer::sc_callback(void *arg)
{
	bool changed = false;
	CSTATE *stat = (CSTATE*)arg;
	
	if(m_Audiofile.MetaData.artist != stat->artist)
	{
		m_Audiofile.MetaData.artist = stat->artist;
		changed = true;
	}
	
	if (m_Audiofile.MetaData.title != stat->title)
	{
		m_Audiofile.MetaData.title = stat->title;
		changed = true;
	}
	
	if (m_Audiofile.MetaData.sc_station != stat->station)
	{
		m_Audiofile.MetaData.sc_station = stat->station;
		changed = true;
	}
	
	if (m_Audiofile.MetaData.genre != stat->genre)
	{
		m_Audiofile.MetaData.genre = stat->genre;
		changed = true;
	}
	
	if(changed)
	{
		m_played_time = 0;
	}
	
	m_sc_buffered = stat->buffered;
	m_Audiofile.MetaData.changed = changed;
}

void CAudioPlayer::clearFileData()
{
	m_Audiofile.clear();
	m_played_time = 0;
	m_sc_buffered = 0;
}

CAudioMetaData CAudioPlayer::getMetaData()
{
	CAudioMetaData m = m_Audiofile.MetaData;
	m_Audiofile.MetaData.changed = false;
	
	return m;
}

bool CAudioPlayer::hasMetaDataChanged()
{
	return m_Audiofile.MetaData.changed;
}

bool CAudioPlayer::readMetaData(CAudiofile* const file, const bool nice)
{
	return CBaseDec::GetMetaDataBase(file, nice);
}

