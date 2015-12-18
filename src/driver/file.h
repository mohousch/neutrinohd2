/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: non-nil; c-basic-offset: 4 -*- */
/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: file.h 2013/10/12 mohousch Exp $

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


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

#ifndef __FILE_H__
#define __FILE_H__

#include <features.h> /* make sure off_t has size 8 in __USE_FILE_OFFSET64 mode */

#include <config.h>

#if !defined (_FILE_OFFSET_BITS) && !defined (__USE_FILE_OFFSET64) && !defined (_DARWIN_USE_64_BIT_INODE)
#error not using 64 bit file offsets
#endif /* __USE_FILE__OFFSET64 */

#include <sys/types.h>

#include <string>
#include <vector>


class CFile
{
	public:
		enum FileExtension
		{
			EXTENSION_UNKNOWN = 0,
			EXTENSION_DIR,
			EXTENSION_AVI,
			EXTENSION_ASF,
			EXTENSION_TEXT,
			EXTENSION_CDR,
			EXTENSION_MP3,
			EXTENSION_MKV,
			EXTENSION_OGG,
			EXTENSION_WAV,
			EXTENSION_FLAC,
			EXTENSION_XML,
			EXTENSION_VOB,
			EXTENSION_MPG,
			EXTENSION_TS,
			EXTENSION_AAC,
			EXTENSION_DTS,
			EXTENSION_DIVX,
			EXTENSION_AIFF,
			EXTENSION_M2P,
			EXTENSION_MPV,
			EXTENSION_M2TS,
			EXTENSION_MOV,
			EXTENSION_DAT,
			EXTENSION_TRP,
			EXTENSION_VDR,
			EXTENSION_MTS,
			EXTENSION_WMV,
			EXTENSION_URL,
			EXTENSION_IMU,
			EXTENSION_FLV,
			EXTENSION_BMP,
			EXTENSION_CRW,
			EXTENSION_GIF,
			EXTENSION_JPEG,
			EXTENSION_JPG,
			EXTENSION_PNG,
			EXTENSION_M3U,
			EXTENSION_M4A,
			EXTENSION_PLS,
			EXTENSION_M2A,
			EXTENSION_MP2,
			EXTENSION_MP4,
			EXTENSION_MPA,
			EXTENSION_MPEG,
			EXTENSION_SH
		};

		enum FileType
		{
			FILE_UNKNOWN = 0,
			FILE_DIR,
			FILE_AUDIO,
			FILE_VIDEO,
			FILE_PICTURE,
			FILE_TEXT,
			FILE_URL,
			FILE_PLAYLIST,
			FILE_XML
		};

		FileExtension	getExtension(void) const;
		FileType	getType(void) const;
		std::string	getFileName(void) const;
		std::string	getPath(void) const;

		CFile();
		
		off_t Size;
		std::string Name;
		std::string Url;
		std::string Title;
		std::string Info1;
		std::string Info2;
		std::string Thumbnail;
		unsigned int Index;
		mode_t Mode;
		bool Marked;
		time_t Time;
};

typedef std::vector<CFile> CFileList;

#endif /* __FILE_H__ */
