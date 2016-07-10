/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: colorchooser.cpp 2013/10/12 mohousch Exp $

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

#ifndef __progressbar_
#define __progressbar_

#include <driver/framebuffer.h>


class CProgressBar
{
	private:
		CFrameBuffer * frameBuffer;
		short width;
		short height;
		unsigned char percent;
		short red, green, yellow;
		bool inverse;

	public:
		CProgressBar(int w, int h, int r, int g, int b, bool inv = false);
		void paint(unsigned int x, unsigned int y, unsigned char pcr);
		void reset();
		int getPercent() { return percent; };
};


#endif
