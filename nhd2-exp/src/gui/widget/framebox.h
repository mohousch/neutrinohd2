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
#include <driver/fontrenderer.h>

#include <gui/widget/window.h>
#include <gui/widget/widget.h>


enum{
	FRAMEBOX_MODE_HORIZONTAL = 0,
	FRAMEBOX_MODE_VERTICAL,
	FRAMEBOX_MODE_RANDOM
};

enum {
	FRAME_BOX = 0,
	FRAME_PICTURE,
	FRAME_PICTURE_NOTSELECTABLE,
	FRAME_ICON,
	FRAME_ICON_NOTSELECTABLE,
	FRAME_TEXT,
	FRAME_TEXT_NOTSELECTABLE,
	FRAME_TEXT_LINE,
	FRAME_TEXT_LINE_NOTSELECTABLE,
	FRAME_PLUGIN,
	FRAME_LINE_VERTICAL,
	FRAME_LINE_HORIZONTAL
};

class CFrame
{
	public:
		CWindow window;
		std::string iconName;
		std::string caption;
		std::string option;

		CFont *captionFont;
		CFont *optionFont;

		fb_pixel_t item_backgroundColor;

		CMenuTarget* jumpTarget;
		std::string actionKey;
		neutrino_msg_t directKey;

		int mode;
		bool shadow;
		bool paintFrame;
		bool pluginOrigName;
		
		CFrame(int m = FRAME_BOX);
		virtual ~CFrame(){}

		int paint(bool selected = false, bool AfterPulldown = false);

		virtual void setTitle(const char *text){if (text != NULL) caption = text;};
		virtual void setIconName(const char *icon){ if (icon != NULL) iconName = icon;};
		virtual void setOption(const char *text){if (text != NULL) option = text;};
		virtual void setPlugin(const char * const pluginName);
		virtual void showPluginName(){pluginOrigName = true;};
		virtual void setMode(int m = FRAME_BOX);
		virtual void setActionKey(CMenuTarget *Target, const char *const ActionKey){jumpTarget = Target; actionKey = ActionKey;};
		virtual void setDirectKey(neutrino_msg_t key){directKey = key;};
		virtual void setCaptionFont(CFont * font){captionFont = font;};
		virtual void setOptionFont(CFont *font){optionFont = font;}; 

		int exec(CMenuTarget *parent);

		virtual bool isSelectable(void)
		{
			if ((mode == FRAME_PICTURE_NOTSELECTABLE) || (mode == FRAME_LINE_HORIZONTAL) || (mode == FRAME_LINE_VERTICAL) || (mode == FRAME_TEXT_NOTSELECTABLE) || (mode == FRAME_TEXT_LINE_NOTSELECTABLE) || (mode == FRAME_ICON_NOTSELECTABLE)) 
				return false; 
			else 
				return true;
		}

		virtual void disableShadow(void){shadow = false;};
		virtual void disablePaintFrame(void){paintFrame = false;};
		virtual void setBackgroundColor(fb_pixel_t col) {item_backgroundColor = col;};
		virtual void setPosition(int x, int y, int dx, int dy){window.setPosition(x, y, dx, dy);};
		virtual void setPosition(CBox *position){window.setPosition(position);};
};

//// CFrameBox
class CFrameBox : public CWidgetItem
{
	private:
		CFrameBuffer* frameBuffer;
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
		CFrameBox(const int x = 0, int const y = 0, const int dx = 0, const int dy = 0);
		CFrameBox(CBox* position);
		virtual ~CFrameBox();

		void setPosition(const int x, const int y, const int dx, const int dy)
		{
			itemBox.iX = x;
			itemBox.iY = y;
			itemBox.iWidth = dx;
			itemBox.iHeight = dy;
		};
		void setPosition(CBox* position){itemBox = *position;};

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
		std::string getActionKey(void){if (hasItem() && selected >= 0 && frames[selected]->isSelectable()) return frames[selected]->actionKey; else return actionKey;};

		void setBackgroundColor(fb_pixel_t col) {backgroundColor = col;};

		void setMode(int mode = FRAMEBOX_MODE_HORIZONTAL)
		{
			frameMode = mode;

			if (frameMode == FRAMEBOX_MODE_RANDOM)
				paintFrame = false;
		};

		void disablePaintFrame(void){paintFrame = false;};

		//
		bool isSelectable(void) const {return true;};

		int oKKeyPressed(CMenuTarget *parent);
		
		//
		virtual void onHomeKeyPressed();
		virtual void onUpKeyPressed();
		virtual void onDownKeyPressed();
		virtual void onRightKeyPressed();
		virtual void onLeftKeyPressed();
		virtual void onPageUpKeyPressed();
		virtual void onPageDownKeyPressed();
};

#endif
