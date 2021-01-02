/*
 * linuxdvb output/writer handling.
 *
 * konfetti 2010 based on linuxdvb.c code from libeplayer2
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

/* ***************************** */
/* Includes                      */
/* ***************************** */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/dvb/video.h>
#include <linux/dvb/audio.h>
#include <memory.h>
#include <asm/types.h>
#include <pthread.h>
#include <errno.h>

#include "common.h"
#include "output.h"
#include "debug.h"
#include "misc.h"
#include "pes.h"
#include "writer.h"

/* ***************************** */
/* Makros/Constants              */
/* ***************************** */

//#define MPEG2_DEBUG

#ifdef MPEG2_DEBUG

static short debug_level = 10;

#define mpeg2_printf(level, fmt, x...) do { \
if (debug_level >= level) printf("[%s:%s] " fmt, __FILE__, __FUNCTION__, ## x); } while (0)
#else
#define mpeg2_printf(level, fmt, x...)
#endif

#ifndef MPEG2_SILENT
#define mpeg2_err(fmt, x...) do { printf("[%s:%s] " fmt, __FILE__, __FUNCTION__, ## x); } while (0)
#else
#define mpeg2_err(fmt, x...)
#endif

/* ***************************** */
/* Types                         */
/* ***************************** */

/* ***************************** */
/* Varaibles                     */
/* ***************************** */

/* ***************************** */
/* Prototypes                    */
/* ***************************** */

/* ***************************** */
/* MISC Functions                */
/* ***************************** */

static int reset()
{
	return 0;
}

static int writeData(void* _call)
{
	WriterAVCallData_t* call = (WriterAVCallData_t*) _call;

	unsigned char               PesHeader[PES_MAX_HEADER_SIZE];
	int len = 0;
	int Position = 0;

	mpeg2_printf(10, "\n");

	if (call == NULL)
	{
		mpeg2_err("call data is NULL...\n");
		return 0;
	}

	mpeg2_printf(10, "VideoPts %lld\n", call->Pts);

	if ((call->data == NULL) || (call->len <= 0))
	{
		mpeg2_err("parsing NULL Data. ignoring...\n");
		return 0;
	}

#if defined (USE_OPENGL)
	if (call->fd == NULL)
#else
	if (call->fd < 0)
#endif
	{
		mpeg2_err("file pointer < 0. ignoring ...\n");
		return 0;
	}

	while(1) 
	{
		int PacketLength = (call->len - Position) <= MAX_PES_PACKET_SIZE ?
				  (call->len - Position) : MAX_PES_PACKET_SIZE;

		int Remaining = call->len - Position - PacketLength;

		mpeg2_printf(20, "PacketLength=%d, Remaining=%d, Position=%d\n", PacketLength, Remaining, Position);

		int HeaderLength = InsertPesHeader (PesHeader, PacketLength, 0xe0, call->Pts, 0);
		unsigned char* PacketStart = malloc(PacketLength + HeaderLength);
		memcpy (PacketStart, PesHeader, HeaderLength);
		memcpy (PacketStart + HeaderLength, call->data + Position, PacketLength);

#if defined (USE_OPENGL)
		len = ao_play(call->fd, PacketStart, PacketLength + HeaderLength);  
#else
		len = write(call->fd, PacketStart, PacketLength + HeaderLength);
#endif

		free(PacketStart);

		Position += PacketLength;
		call->Pts = INVALID_PTS_VALUE;

		if (Position == call->len)
			break;
	}

	mpeg2_printf(10, "< len %d\n", len);
	return len;
}

/* ***************************** */
/* Writer  Definition            */
/* ***************************** */
static WriterCaps_t caps = {
	"mpeg2",
	eVideo,
	"V_MPEG2",
	VIDEO_STREAMTYPE_MPEG2,
};

struct Writer_s WriterVideoMPEG2 = {
	&reset,
	&writeData,
	NULL,
	&caps
};

static WriterCaps_t h264_caps = {
	"mpges_h264",
	eVideo,
	"V_MPEG2/H264",
	VIDEO_STREAMTYPE_MPEG4_H264
};

struct Writer_s WriterVideoMPEGH264 = {
	&reset,
	&writeData,
	NULL,
	&h264_caps
};
