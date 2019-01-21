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

#include <driver/framebuffer.h>
#include <driver/fontrenderer.h>

#include <gui/widget/icons.h>
#include <gui/widget/drawable.h>
#include <gui/widget/buttons.h>
#include <gui/widget/headers.h>
#include <gui/widget/items2detailsline.h>
#include <gui/widget/scrollbar.h>
#include <gui/widget/window.h>
#include <gui/widget/menue.h>
#include <gui/widget/listbox.h>
#include <gui/widget/listframe.h>
#include <gui/widget/hintbox.h>
#include <gui/widget/helpbox.h>
#include <gui/widget/messagebox.h>
#include <gui/widget/textbox.h>
#include <gui/widget/infobox.h>
#include <gui/widget/stringinput.h>
#include <gui/widget/stringinput_ext.h>

#include <gui/widget/colorchooser.h>
#include <gui/widget/keychooser.h>
#include <gui/widget/mountchooser.h>
#include <gui/widget/progresswindow.h>
#include <gui/widget/progressbar.h>
%}

%include <config.h>

%include <driver/framebuffer.h>
%include <driver/fontrenderer.h>

%include <gui/widget/icons.h>
%include <gui/widget/drawable.h>
%include <gui/widget/buttons.h>
%include <gui/widget/headers.h>
%include <gui/widget/items2detailsline.h>
%include <gui/widget/scrollbar.h>
%include <gui/widget/window.h>
%include <gui/widget/menue.h>
%include <gui/widget/listbox.h>
%include <gui/widget/listframe.h>
%include <gui/widget/hintbox.h>
%include <gui/widget/helpbox.h>
%include <gui/widget/messagebox.h>
%include <gui/widget/textbox.h>
%include <gui/widget/infobox.h>
%include <gui/widget/stringinput.h>
%include <gui/widget/stringinput_ext.h>

%include <gui/widget/colorchooser.h>
%include <gui/widget/keychooser.h>
%include <gui/widget/mountchooser.h>
%include <gui/widget/progresswindow.h>
%include <gui/widget/progressbar.h>


