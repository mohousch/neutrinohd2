/*
 * $Id: buttons.h 2016/01/12 mohousch Exp $
 *
 * (C) 2003 by thegoodguy <thegoodguy@berlios.de>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __gui_widget_helpers_h__
#define __gui_widget_helpers_h__

#include <driver/fontrenderer.h>
#include <driver/framebuffer.h>

#include <system/localize.h>
#include <system/settings.h>

#include <driver/rcinput.h>


class CMenuTarget;

//
enum {
	WIDGET_ITEM_HEAD = 0,
	WIDGET_ITEM_FOOT,
	WIDGET_ITEM_LISTBOX,
	WIDGET_ITEM_FRAMEBOX,
	WIDGET_ITEM_LISTFRAME,
	WIDGET_ITEM_TEXTBOX,
	WIDGET_ITEM_WINDOW,
	WIDGET_ITEM_PIG,
	WIDGET_ITEM_GRID,
	WIDGET_ITEM_BUTTON,
	WIDGET_ITEM_SCROLLBAR,
	WIDGET_ITEM_DETAILSLINE
};

//// helpers
class CBox
{
	public:
		// Variables
		int iX;
		int iY;
		int iWidth;
		int iHeight;

		// Constructor
		inline CBox(){;};
		inline CBox( const int _iX, const int _iY, const int _iWidth, const int _iHeight){iX =_iX; iY=_iY; iWidth =_iWidth; iHeight =_iHeight;};
		inline ~CBox(){;};
};

class CIcon
{
	public:
		int iWidth;
		int iHeight;
		std::string iconName;

		inline CIcon(){;};

		void setIcon(const char* icon)
		{
			iconName = std::string(icon); 
			CFrameBuffer::getInstance()->getIconSize(iconName.c_str(), &iWidth, &iHeight);
		};

		inline CIcon(const char* icon)
		{
			iconName = std::string(icon); 
			CFrameBuffer::getInstance()->getIconSize(iconName.c_str(), &iWidth, &iHeight);
		};
};

class CImage
{
	public:
		int iWidth;
		int iHeight;
		int iNbp;
		std::string imageName;

		inline CImage(){;};

		void setImage(const char* image)
		{
			imageName = std::string(image); 
			CFrameBuffer::getInstance()->getSize(imageName, &iWidth, &iHeight, &iNbp);
		};

		inline CImage(const char* image)
		{
			imageName = std::string(image); 
			CFrameBuffer::getInstance()->getSize(imageName, &iWidth, &iHeight, &iNbp);
		};
};

// buttons
typedef struct button_label
{
	const char * button;
	neutrino_locale_t locale;
	const char * localename;
} button_label_struct;

// CButtons
class CButtons
{
	private:
	public:
		CButtons(){};

		void paintFootButtons(const int x, const int y, const int dx, const int dy, const unsigned int count, const struct button_label* const content);

		// head buttons right
		void paintHeadButtons(const int x, const int y, const int dx, const int dy, const unsigned int count, const struct button_label * const content);
};

//CScrollBar
class CScrollBar
{
	private:
	public:
		CScrollBar(){};
		virtual ~CScrollBar(){};

		void paint(const int x, const int y, const int dy, const int NrOfPages, const int CurrentPage);
		void paint(CBox* position, const int NrOfPages, const int CurrentPage);
};

// CProgressbar
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
		CProgressBar(int w, int h, int r = 40, int g = 100, int b = 70, bool inv = true);
		void paint(unsigned int x, unsigned int y, unsigned char pcr);
		void reset();
		int getPercent() { return percent; };
};

// detailsLine
class CItems2DetailsLine
{
	private:
	public:
		CItems2DetailsLine(){};
		virtual ~CItems2DetailsLine(){};
		
		void paint(int x, int y, int width, int height, int info_height, int iheight, int iy);
		void clear(int x, int y, int width, int height, int info_height);
};

// CWidgetItem
class CWidgetItem
{
	private:
	public:
		CBox itemBox;

		int itemType;
		bool outFocus;
		bool paintDate;

		CWidgetItem(){};
		virtual ~CWidgetItem(){};

		virtual bool isSelectable(void) const {return false;}
		virtual bool hasItem(){return false;};

		virtual void paint(){};
		virtual void hide(){};

		virtual void scrollLineDown(const int lines = 1){};
		virtual void scrollLineUp(const int lines = 1){};
		virtual void scrollPageDown(const int pages = 1){};
		virtual void scrollPageUp(const int pages = 1){};
		virtual void swipLeft(){};
		virtual void swipRight(){};

		virtual void setOutFocus(bool focus){outFocus = focus;};
		virtual void setSelected(unsigned int _new) {};

		virtual inline CBox getItemPos(void){return (itemBox);};

		virtual int getWidgetType(){return (4);};
		virtual void enablePaintDate(void){paintDate = true;};

		virtual int oKKeyPressed(CMenuTarget *parent){return 0;};
		
		//
		virtual void onUpKeyPressed(){};
		virtual void onDownKeyPressed(){};
		virtual void onRightKeyPressed(){};
		virtual void onLeftKeyPressed(){};

		virtual void onPageUpKeyPressed(){};
		virtual void onPageDownKeyPressed(){};
};

// CHeaders
class CHeaders : public CWidgetItem
{
	private:
		fb_pixel_t bgcolor;
		int radius;
		int corner;
		int gradient;
		bool paintDate;
		bool logo;
		int hbutton_count;
		const struct button_label* hbutton_labels;
		const char *htitle;
		const char *hicon;
	
	public:
		CHeaders(const int x, const int y, const int dx, const int dy, const char * const title = NULL, const char * const icon = NULL);
		CHeaders(CBox position, const char * const title = NULL, const char * const icon = NULL);
		virtual ~CHeaders(){};

		void setTitle(const char * const title){htitle = title;};
		void setIcon(const char * const icon){hicon = icon;};

		// head
		void setColor(fb_pixel_t col){bgcolor = col;};
		void setCorner(int ra = NO_RADIUS, int co = CORNER_NONE){radius = ra; corner = co;};
		void setGradient(int grad = nogradient){gradient = grad;};

		void setButtons(const struct button_label* _hbutton_labels, const int _hbutton_count){hbutton_count = _hbutton_count; hbutton_labels = _hbutton_labels;}

		void enablePaintDate(void){paintDate = true;};
		void enableLogo(void){logo = true;};

		void paint();
};

// CFooters
class CFooters : public CWidgetItem
{
	private:
		unsigned int fcount;
		const struct button_label *fcontent;

		fb_pixel_t fbgcolor;
		int fradius;
		int fcorner;
		int fgradient;
	
	public:
		CFooters(int x, int y, int dx, int dy, const unsigned int count = 0, const struct button_label *content = NULL);
		CFooters(CBox position, const unsigned int count = 0, const struct button_label *content = NULL);
		virtual ~CFooters(){};

		void setColor(fb_pixel_t col){fbgcolor = col;};
		void setCorner(int ra = NO_RADIUS, int co = CORNER_NONE){fradius = ra; fcorner = co;};
		void setGradient(int grad = nogradient){fgradient = grad;};

		void setButtons(const struct button_label* button_label, const int button_count){fcontent = button_label; fcount = button_count;};

		void paint();
};

#endif /* __gui_widget_helpers_h__ */
