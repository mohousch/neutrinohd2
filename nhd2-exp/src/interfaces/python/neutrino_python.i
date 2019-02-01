/*
	$Id: neutrino_python.i 20.01.2019 mohousch Exp $

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

%module neutrino
%{
#define SWIG_COMPILE

#include <config.h>
#include <src/global.h>
#include <src/neutrino.h>

// libs
#include <libconfigfile/configfile.h>
#include <libdvbapi/playback_cs.h>

// driver
#include <driver/framebuffer.h>
#include <driver/fontrenderer.h>
#include <driver/color.h>
#include <driver/rcinput.h>
#include <driver/file.h>
#include <driver/audiofile.h>
#include <driver/audiometadata.h>
#include <driver/screen_max.h>
#include <driver/encoding.h>
#include <driver/pictureviewer/pictureviewer.h>
#include <driver/audioplay.h>
#include <driver/audiodec/basedec.h>

// system
#include <system/settings.h>
#include <system/debug.h>
#include <system/localize.h>
#include <system/locals.h>
#include <system/setting_helpers.h>
#include <system/helpers.h>
#include <system/tmdbparser.h>
#include <system/ytparser.h>

// widget
#include <gui/widget/icons.h>
#include <gui/widget/drawable.h>
#include <gui/widget/window.h>
#include <gui/widget/headers.h>
#include <gui/widget/scrollbar.h>
#include <gui/widget/icons.h>
#include <gui/widget/items2detailsline.h>
#include <gui/widget/progressbar.h>
#include <gui/widget/progresswindow.h>
#include <gui/widget/messagebox.h>
#include <gui/widget/helpbox.h>
#include <gui/widget/listbox.h>
#include <gui/widget/hintbox.h>
#include <gui/widget/infobox.h>
#include <gui/widget/listframe.h>
#include <gui/widget/stringinput.h>
#include <gui/widget/textbox.h>
#include <gui/widget/menue.h>

// gui
#include <gui/movieinfo.h>
#include <gui/audioplayer.h>
#include <gui/movieplayer.h>
#include <gui/pictureviewer.h>
#include <gui/filebrowser.h>
%}

%include "typemaps.i"
%include "std_string.i"
%include "stdint.i"
%include "carrays.i"
%include "cpointer.i"
//%include "std_vector.i"

%include <config.h>
%include <src/global.h>
%include <src/neutrino.h>

// libs
%include <lib/libconfigfile/configfile.h>
%include <lib/libdvbapi/playback_cs.h>

// driver
%include <src/driver/fontrenderer.h>
%include <src/driver/framebuffer.h>
%include <src/driver/color.h>
%include <src/driver/rcinput.h>
%include <src/driver/file.h>
%include <src/driver/audiofile.h>
%include <src/driver/audiometadata.h>
%include <src/driver/screen_max.h>
%include <src/driver/encoding.h>
%include <src/driver/pictureviewer/pictureviewer.h>
%include <src/driver/audioplay.h>
%include <src/driver/audiodec/basedec.h>

// system
%include <src/system/settings.h>
%include <src/system/debug.h>
%include <src/system/localize.h>
%include <src/system/locals.h>
%include <src/system/setting_helpers.h>
%include <src/system/helpers.h>
%include <src/system/tmdbparser.h>
%include <src/system/ytparser.h>

// widget
%include <src/gui/widget/icons.h>
%include <src/gui/widget/drawable.h>
%include <src/gui/widget/window.h>
%include <src/gui/widget/headers.h>
%include <src/gui/widget/scrollbar.h>
%include <src/gui/widget/icons.h>
%include <src/gui/widget/items2detailsline.h>
%include <src/gui/widget/progressbar.h>
%include <src/gui/widget/progresswindow.h>
%include <src/gui/widget/messagebox.h>
%include <src/gui/widget/helpbox.h>
%include <src/gui/widget/listbox.h>
%include <src/gui/widget/hintbox.h>
%include <src/gui/widget/infobox.h>
%include <src/gui/widget/listframe.h>
%include <src/gui/widget/stringinput.h>
%include <src/gui/widget/textbox.h>
%include <src/gui/widget/menue.h>

// gui
%include <src/gui/movieinfo.h>
%include <src/gui/audioplayer.h>
%include <src/gui/movieplayer.h>
%include <src/gui/pictureviewer.h>
%include <src/gui/filebrowser.h>






