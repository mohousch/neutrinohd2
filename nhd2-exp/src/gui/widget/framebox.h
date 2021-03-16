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
#include <gui/widget/widget.h>


enum{
	FRAMEBOX_MODE_HORIZONTAL = 0,
	FRAMEBOX_MODE_VERTICAL
};

enum {
	FRAME_BOX = 0,
	FRAME_PICTURE,
	FRAME_BUTTON,
	FRAME_TEXT,
	FRAME_SEPARATOR,
	FRAME_PLUGIN
};

class CFrame
{
	public:
		CWindow window;
		std::string iconName;
		std::string caption;
		std::string option;

		fb_pixel_t item_backgroundColor;

		CMenuTarget* jumpTarget;
		std::string actionKey;
		neutrino_msg_t directKey;

		int mode;
		bool shadow;
		
		CFrame(int m = FRAME_BOX);
		CFrame(const std::string title, int m = FRAME_BOX);
		virtual ~CFrame(){}

		int paint(bool selected = false, bool AfterPulldown = false);

		virtual void setTitle(const char * text){caption = text;};
		virtual void setIconName(const char* const icon){iconName = icon;};
		virtual void setOption(const char* text){option = text;};
		//virtual void setMode(int m = FRAME_BOX){mode = m;};
		virtual void setActionKey(CMenuTarget *Target, const char *const ActionKey){jumpTarget = Target; actionKey = ActionKey;};
		virtual void setDirectKey(neutrino_msg_t key){directKey = key;};

		int exec(CMenuTarget *parent);

		virtual bool isSelectable(void) const 
		{
			if (mode == FRAME_SEPARATOR) 
				return false; 
			else 
				return true;
		}

		virtual void disableShadow(void){shadow = false;};
};

//// CFrameBox
class CFrameBox : public CWidgetItem
{
	private:
		CFrameBuffer* frameBuffer;
		//CBox cFrameBox;
		CWindow cFrameWindow;
		int selected;
		int pos;

		fb_pixel_t backgroundColor;

		std::vector<CFrame*> frames;

		virtual void paintFrames();

		int frameMode;

		std::string actionKey;

		bool paintFrame;

	public:
		CFrameBox(const int x, int const y, const int dx, const int dy);
		CFrameBox(CBox* position);
		virtual ~CFrameBox();

		virtual void addFrame(CFrame *frame, const bool defaultselected = false);
		bool hasItem();
		void clearFrames(void){frames.clear();};
		void setSelected(unsigned int _new) { /*if(_new <= frames.size())*/ selected = _new; };

		virtual void initFrames();
		virtual void paint();
		virtual void hide();

		virtual void swipRight();
		virtual void swipLeft();
		virtual void scrollLineDown(const int lines = 1);
		virtual void scrollLineUp(const int lines = 1);

		int getSelected(){return selected;};
		std::string getActionKey(void){if (hasItem()) return frames[selected]->actionKey; else return actionKey;};

		void setBackgroundColor(fb_pixel_t col) {backgroundColor = col;};

		void setMode(int mode = FRAMEBOX_MODE_HORIZONTAL){frameMode = mode;};

		void disablePaintFrame(void){paintFrame = false;};

		//
		bool isSelectable(void) const {return true;};

		//inline CBox getWindowsPos(void){return (cFrameBox);};

		int oKKeyPressed(CMenuTarget *parent);
		
		//
		virtual void onUpKeyPressed();
		virtual void onDownKeyPressed();
		virtual void onRightKeyPressed();
		virtual void onLeftKeyPressed();

		virtual void onPageUpKeyPressed();
		virtual void onPageDownKeyPressed();
};

#endif
