/*
	Neutrino-GUI  -   DBoxII-Project

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

#ifndef __pictureviewergui__
#define __pictureviewergui__

#include <string>

#include <driver/framebuffer.h>
#include <driver/pictureviewer/pictureviewer.h>
#include <gui/widget/menue.h>
#include <gui/filebrowser.h>


class CPicture
{
	public:
		std::string Filename;
		std::string Name;
		std::string Type;
		time_t Date;
};

typedef std::vector<CPicture> CPicturePlayList;

//
class CPictureViewerGui : public CMenuTarget
{
	public:
		enum State
		{
			VIEW = 0,
			MENU,
			SLIDESHOW
		};
		
		enum SortOrder
		{
			DATE = 0,
			FILENAME,
			NAME
		};
		
	private:

		CFrameBuffer		* frameBuffer;

		//
		int 			width;
		int 			height;
		int 			x;
		int 			y;
		int         		m_title_w;

		int icon_foot_w;
		int icon_foot_h;
		int icon_head_w;
		int icon_head_h;

		unsigned int		liststart;
		unsigned int		listmaxshow;
		int			fheight; // Fonthoehe Playlist-Inhalt
		int			theight; // Fonthoehe Playlist-Titel
		int			sheight; // Fonthoehe MP Info
		int			buttonHeight;

		void paintItem(int pos);
		void paint();
		void paintHead();
		void paintFoot();
		void hide();

		//
		CPictureViewer * g_PicViewer;

		CFileFilter picture_filter;

		unsigned int selected;
		bool isURL;
		bool visible;			
		State m_state;
		SortOrder m_sort;

		CPicturePlayList playlist;
		std::string Path;

		long m_time;
		int m_LastMode;

		void paintLCD();

		void view(unsigned int nr);
		void endView();
		int  show();

		void showHelp();
		
	public:
		CPictureViewerGui();
		~CPictureViewerGui();
		int  exec(CMenuTarget* parent, const std::string& actionKey);
		void addToPlaylist(CPicture& file);
		void clearPlaylist(void);
		void removeFromPlaylist(long pos);
		void setState(State state = VIEW){m_state = state;};
};

#endif


