/*
	$Id: widget.h 11.03.2020 mohousch Exp $


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
#if !defined(WIDGET_H_)
#define WIDGET_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


// CMenuTarget
struct menu_return
{
	enum
	{
		RETURN_NONE	= 0,
		RETURN_REPAINT 	= 1,
		RETURN_EXIT 	= 2,
		RETURN_EXIT_ALL = 4
	};
};

class CMenuTarget
{
	public:
		CMenuTarget(){};
		virtual ~CMenuTarget(){};
		virtual void hide(){};
		virtual int exec(CMenuTarget *parent, const std::string &actionKey) = 0;
};

#endif // WIDGET_H_

