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


//#define H264_DEBUG

#ifdef H264_DEBUG

static short debug_level = 10;

#define h264_printf(level, fmt, x...) do { \
if (debug_level >= level) printf("[%s:%s] " fmt, __FILE__, __FUNCTION__, ## x); } while (0)
#else
#define h264_printf(level, fmt, x...)
#endif

#ifndef H264_SILENT
#define h264_err(fmt, x...) do { printf("[%s:%s] " fmt, __FILE__, __FUNCTION__, ## x); } while (0)
#else
#define h264_err(fmt, x...)
#endif

#define NALU_TYPE_PLAYER2_CONTAINER_PARAMETERS          24
#define CONTAINER_PARAMETERS_VERSION                    0x00

/* ***************************** */
/* Types                         */
/* ***************************** */
typedef struct avcC_s
{
	unsigned char       Version;                /* configurationVersion        */
	unsigned char       Profile;                /* AVCProfileIndication        */
	unsigned char       Compatibility;          /* profile_compatibility       */
	unsigned char       Level;                  /* AVCLevelIndication          */
	unsigned char       NalLengthMinusOne;      /* held in bottom two bits     */
	unsigned char       NumParamSets;           /* held in bottom 5 bits       */
	unsigned char       Params[1];              /* {length,params}{length,params}...sequence then picture*/
} avcC_t;


/* ***************************** */
/* Varaibles                     */
/* ***************************** */
const unsigned char Head[]                  = {0, 0, 0, 1};
static int initialHeader = 1;
static unsigned int        NalLengthBytes          = 1;

/* ***************************** */
/* Prototypes                    */
/* ***************************** */

/* ***************************** */
/* MISC Functions                */
/* ***************************** */

static int reset()
{
	initialHeader = 1;
	return 0;
}

static int writeData(void* _call)
{
	WriterAVCallData_t* call = (WriterAVCallData_t*) _call;

	unsigned char*          PacketStart = NULL;
	unsigned int            PacketStartSIZE = 0;
	unsigned int            HeaderLength;
	unsigned char           PesHeader[PES_MAX_HEADER_SIZE];
	unsigned long long int  VideoPts;
	unsigned int            TimeDelta;
	unsigned int            TimeScale;
	int                     len = 0;
	static int              NoOtherBeginningFound = 1;
	h264_printf(10, "\n");

	if (call == NULL)
	{
		h264_err("call data is NULL...\n");
		return 0;
	}

	TimeDelta = call->FrameRate;
	TimeScale = call->FrameScale;
	VideoPts  = call->Pts;

	h264_printf(10, "VideoPts %lld - %d %d\n", call->Pts, TimeDelta, TimeScale);

	if ((call->data == NULL) || (call->len <= 0))
	{
		h264_err("NULL Data. ignoring...\n");
		return 0;
	}

#if defined (USE_OPENGL)
	if (call->fd == NULL)
#else
	if (call->fd < 0)
#endif
	{
		h264_err("file pointer < 0. ignoring ...\n");
		return 0;
	}

	if((call->data[0] == 0x00 && call->data[1] == 0x00 && call->data[2] == 0x00 && call->data[3] == 0x01) ||
	  (call->data[0] == 0x00 && call->data[1] == 0x00 && call->data[2] == 0x01 && NoOtherBeginningFound) ||
		(call->data[0] == 0xff && call->data[1] == 0xff && call->data[2] == 0xff && call->data[3] == 0xff))
	{
		unsigned int FakeStartCode = (call->Version << 8) | PES_VERSION_FAKE_START_CODE;
		unsigned int PrivateLength=0;
		if(initialHeader)PrivateLength = call->private_size;
		HeaderLength = InsertPesHeader(PesHeader, call->len,
					      MPEG_VIDEO_PES_START_CODE, call->Pts, FakeStartCode);
		/*Hellmaster1024: some packets will only be accepted by the player if we send one byte more than
				  data is available. The content of this byte does not matter. It will be ignored
				  by the player */
		unsigned char *PacketData = malloc(HeaderLength + call->len + PrivateLength + 1);

		memcpy(PacketData, PesHeader, HeaderLength);
		if(initialHeader){
		    memcpy (PacketData + HeaderLength, call->private_data, PrivateLength);
		    initialHeader=0;
		}
		memcpy (PacketData + HeaderLength + PrivateLength, call->data, call->len);

#if defined (USE_OPENGL)
		len = ao_play(call->fd, PacketData, call->len + HeaderLength + PrivateLength + 1);  
#else
		len = write(call->fd, PacketData, call->len + HeaderLength + PrivateLength + 1);
#endif

		free(PacketData);

		return len;
	}
	NoOtherBeginningFound = 0;

	if (initialHeader)
	{
		unsigned char*  HeaderData = malloc(BUFFER_SIZE+PADDING_LENGTH);
		avcC_t*         avcCHeader          = (avcC_t*)call->private_data;
		int             i;
		unsigned int    ParamSets;
		unsigned int    ParamOffset;
		unsigned int    InitialHeaderLength     = 0;
		unsigned int    ParametersLength;

		if (avcCHeader == NULL)
		{
			h264_err("private_data NULL\n");
			free(HeaderData);
			return -1;
		}

		if (avcCHeader->Version != 1)
			h264_err("Error unknown avcC version (%x). Expect problems.\n", avcCHeader->Version);

		ParametersLength                      = 0;

		HeaderData[ParametersLength++]        = 0x00;                                         // Start code
		HeaderData[ParametersLength++]        = 0x00;
		HeaderData[ParametersLength++]        = 0x01;
		HeaderData[ParametersLength++]        = NALU_TYPE_PLAYER2_CONTAINER_PARAMETERS;
		// Container message version - changes when/if we vary the format of the message
		HeaderData[ParametersLength++]        = CONTAINER_PARAMETERS_VERSION;
		HeaderData[ParametersLength++]        = 0xff;                                         // Field separator

		if( TimeDelta == 0xffffffff )
			TimeDelta       = (TimeScale > 1000) ? 1001 : 1;

		HeaderData[ParametersLength++]        = (TimeScale >> 24) & 0xff;         // Output the timescale
		HeaderData[ParametersLength++]        = (TimeScale >> 16) & 0xff;
		HeaderData[ParametersLength++]        = 0xff;
		HeaderData[ParametersLength++]        = (TimeScale >> 8) & 0xff;
		HeaderData[ParametersLength++]        = TimeScale & 0xff;
		HeaderData[ParametersLength++]        = 0xff;

		HeaderData[ParametersLength++]        = (TimeDelta >> 24) & 0xff;         // Output frame period
		HeaderData[ParametersLength++]        = (TimeDelta >> 16) & 0xff;
		HeaderData[ParametersLength++]        = 0xff;
		HeaderData[ParametersLength++]        = (TimeDelta >> 8) & 0xff;
		HeaderData[ParametersLength++]        = TimeDelta & 0xff;
		HeaderData[ParametersLength++]        = 0xff;
		HeaderData[ParametersLength++]        = 0x80;                                         // Rsbp trailing bits

		HeaderLength = InsertPesHeader (PesHeader, ParametersLength, MPEG_VIDEO_PES_START_CODE, INVALID_PTS_VALUE, 0);

		PacketStart = malloc(HeaderLength + ParametersLength);
		PacketStartSIZE = HeaderLength + ParametersLength;
		memcpy (PacketStart, PesHeader, HeaderLength);
		memcpy (PacketStart + HeaderLength, HeaderData, ParametersLength);
		len += write (call->fd, PacketStart, HeaderLength + ParametersLength);

		NalLengthBytes  = (avcCHeader->NalLengthMinusOne & 0x03) + 1;
		ParamSets       = avcCHeader->NumParamSets & 0x1f;

		h264_printf(20, "avcC contents:\n");
		h264_printf(20, "    version:                       %d\n", avcCHeader->Version);
		h264_printf(20, "    profile:                       %d\n", avcCHeader->Profile);
		h264_printf(20, "    profile compatibility:         %d\n", avcCHeader->Compatibility);
		h264_printf(20, "    level:                         %d\n", avcCHeader->Level);
		h264_printf(20, "    nal length bytes:              %d\n", NalLengthBytes);
		h264_printf(20, "    number of sequence param sets: %d\n", ParamSets);

		ParamOffset     = 0;
		for (i = 0; i < ParamSets; i++) 
		{
			unsigned int  PsLength = (avcCHeader->Params[ParamOffset] << 8) + avcCHeader->Params[ParamOffset+1];

			h264_printf(20, "        sps %d has length           %d\n", i, PsLength);

			if (HeaderLength + InitialHeaderLength + sizeof(Head) > PacketStartSIZE) {
				PacketStart = realloc(PacketStart, HeaderLength + InitialHeaderLength + sizeof(Head));
				PacketStartSIZE = HeaderLength + InitialHeaderLength + sizeof(Head);
			}

			memcpy (PacketStart + HeaderLength + InitialHeaderLength, Head, sizeof(Head));
			InitialHeaderLength        += sizeof(Head);

			if (HeaderLength + InitialHeaderLength + PsLength > PacketStartSIZE) {
			    PacketStart = realloc(PacketStart, HeaderLength + InitialHeaderLength + PsLength);
			    PacketStartSIZE = HeaderLength + InitialHeaderLength + PsLength;
			}

			memcpy (PacketStart + HeaderLength + InitialHeaderLength, &avcCHeader->Params[ParamOffset+2], PsLength);

			InitialHeaderLength        += PsLength;
			ParamOffset                += PsLength+2;
		}

		ParamSets                       = avcCHeader->Params[ParamOffset];

		h264_printf(20,  "    number of picture param sets:  %d\n", ParamSets);

		ParamOffset++;
		for (i = 0; i < ParamSets; i++) 
		{
			unsigned int  PsLength      = (avcCHeader->Params[ParamOffset] << 8) + avcCHeader->Params[ParamOffset+1];

			h264_printf (20, "        pps %d has length           %d\n", i, PsLength);

			if (HeaderLength + InitialHeaderLength + sizeof(Head) > PacketStartSIZE) {
			    PacketStart = realloc(PacketStart, HeaderLength + InitialHeaderLength + sizeof(Head));
			    PacketStartSIZE = HeaderLength + InitialHeaderLength + sizeof(Head);
			}

			memcpy (PacketStart + HeaderLength + InitialHeaderLength, Head, sizeof(Head));
			InitialHeaderLength        += sizeof(Head);

			if (HeaderLength + InitialHeaderLength + PsLength > PacketStartSIZE) {
			    PacketStart = realloc(PacketStart, HeaderLength + InitialHeaderLength + PsLength);
			    PacketStartSIZE = HeaderLength + InitialHeaderLength + PsLength;
			}

			memcpy (PacketStart + HeaderLength + InitialHeaderLength, &avcCHeader->Params[ParamOffset+2], PsLength);
			InitialHeaderLength        += PsLength;
			ParamOffset                += PsLength+2;
		}

		HeaderLength    = InsertPesHeader (PesHeader, InitialHeaderLength, MPEG_VIDEO_PES_START_CODE, INVALID_PTS_VALUE, 0);
		memcpy (PacketStart, PesHeader, HeaderLength);

#if defined (USE_OPENGL)
		len += ao_play(call->fd, PacketStart, HeaderLength + InitialHeaderLength);  
#else
		len += write (call->fd, PacketStart, HeaderLength + InitialHeaderLength);
#endif

		initialHeader           = 0;

		free(PacketStart);
		free(HeaderData);
	}

	unsigned int SampleSize    = call->len;
	unsigned int NalStart      = 0;
	unsigned int VideoPosition = 0;

	do {
		unsigned int   NalLength;
		unsigned char  NalData[4];
		int            NalPresent = 1;

		memcpy (NalData, call->data + VideoPosition, NalLengthBytes);
		VideoPosition += NalLengthBytes;
		
		switch(NalLengthBytes) 
		{
			case 1:  NalLength = (NalData[0]); break;
			case 2:  NalLength = (NalData[0] <<  8) | (NalData[1]); break;
			case 3:  NalLength = (NalData[0] << 16) | (NalData[1] <<  8) | (NalData[2]); break;
			default: NalLength = (NalData[0] << 24) | (NalData[1] << 16) | (NalData[2] << 8) | (NalData[3]); break;
		}

		h264_printf(20, "NalStart = %u + NalLength = %u > SampleSize = %u\n", NalStart, NalLength, SampleSize);

		if (NalStart + NalLength > SampleSize) 
		{

			h264_printf(20, "nal length past end of buffer - size %u frame offset %u left %u\n", NalLength, NalStart , SampleSize - NalStart );

			NalStart    = SampleSize;
		} 
		else 
		{
			NalStart               += NalLength + NalLengthBytes;
		    
			while (NalLength > 0) 
			{
				unsigned int   PacketLength     = (NalLength < BUFFER_SIZE) ? NalLength : BUFFER_SIZE;
				int            ExtraLength      = 0;
				unsigned char* PacketStart;

				NalLength      -= PacketLength;

				if (NalPresent) 
				{
					PacketStart     = malloc(sizeof(Head) + PacketLength);
					memcpy (PacketStart + sizeof(Head), call->data + VideoPosition, PacketLength);
					VideoPosition    += PacketLength;

					memcpy (PacketStart, Head, sizeof(Head));
					ExtraLength    = sizeof(Head);
				} 
				else 
				{
					PacketStart     = malloc(PacketLength);
					memcpy (PacketStart, call->data + VideoPosition, PacketLength);
					VideoPosition    += PacketLength;
				}

				PacketLength   += ExtraLength;

				h264_printf (20, "  pts=%llu\n", VideoPts);

				HeaderLength    = InsertPesHeader (PesHeader, PacketLength, MPEG_VIDEO_PES_START_CODE, VideoPts, 0);

				unsigned char*    WritePacketStart = malloc(HeaderLength + PacketLength);
				memcpy (WritePacketStart,              PesHeader,   HeaderLength);
				memcpy (WritePacketStart+HeaderLength, PacketStart, PacketLength);
				free(PacketStart);

				PacketLength   += HeaderLength;
#if defined (USE_OPENGL)
				len += ao_play(call->fd, WritePacketStart, PacketLength);  
#else
				len += write (call->fd, WritePacketStart, PacketLength);
#endif
				free(WritePacketStart);

				NalPresent      = 0;
				VideoPts        = INVALID_PTS_VALUE;
			}
		}
	} while (NalStart < SampleSize);

	if (len < 0)
	{
	    h264_err("error writing data errno = %d\n", errno);
	    h264_err("%s\n", strerror(errno));
	}

	h264_printf (10, "< len %d\n", len);
	return len;
}

static int writeReverseData(void* _call)
{
	WriterAVCallData_t* call = (WriterAVCallData_t*) _call;

#ifndef old_reverse_playback
	h264_printf(10, "\n");

	if (call == NULL)
	{
		h264_err("call data is NULL...\n");
		return 0;
	}

	h264_printf(10, "VideoPts %lld\n", call->Pts);

	if ((call->data == NULL) || (call->len <= 0))
	{
		h264_err("NULL Data. ignoring...\n");
		return 0;
	}

#if defined (USE_OPENGL)
	if (call->fd == NULL)
#else
	if (call->fd < 0)
#endif
	{
		h264_err("file pointer < 0. ignoring ...\n");
		return 0;
	}

	return 0;

#else
	return 0;
#endif

}
/* ***************************** */
/* Writer  Definition            */
/* ***************************** */

static WriterCaps_t caps = {
	"h264",
	eVideo,
	"V_MPEG4/ISO/AVC",
	VIDEO_STREAMTYPE_MPEG4_H264
};

struct Writer_s WriterVideoH264 = {
	&reset,
	&writeData,
	&writeReverseData,
	&caps,
};

