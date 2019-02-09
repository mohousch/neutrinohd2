/*
	$Id: framebox.h 09.02.2019 mohousch Exp $


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

#if !defined(_FRAMEBOX_H_)
#define _FRAMEBOX_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>
#include <vector>

#include <driver/framebuffer.h>

#include <gui/widget/window.h>


class CFrame
{
	public:
		CWindow window;
		std::string caption;

		CFrame(const std::string title);
		virtual ~CFrame(){}

		//virtual void init(const int X, const int Y, const int DX, const int DY);
		int paint(bool selected = false);
		int getHeight(void) const;
		int getWidth(void) const;
};

class CFrameBox
{
	private:
		CFrameBuffer* frameBuffer;
		CBox cFrameBox;
		CWindow cFrameWindow;
		int selected;
		int pos;

		bool outFocus;

		std::vector<CFrame*> frames;

		virtual void paintFrames();

	public:
		CFrameBox(const int x, int const y, const int dx, const int dy);
		CFrameBox(CBox* position);
		virtual ~CFrameBox();

		virtual void addFrame(CFrame *frame, const bool defaultselected = false);
		bool hasFrame();
		void clearFrames(void){frames.clear();};
		void setSelected(unsigned int _new) { /*if(_new <= frames.size())*/ selected = _new; };

		virtual void initFrames();
		virtual void paint();
		virtual void hide();

		virtual void swipRight();
		virtual void swipLeft();

		int getSelected(){return selected;};

		void setOutFocus(bool focus){outFocus = focus;};
};

#endif
