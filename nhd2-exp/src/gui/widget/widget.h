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

#include <gui/widget/widget_helpers.h>


//
enum
{
	RETURN_NONE	= 0,
	RETURN_REPAINT 	= 1,
	RETURN_EXIT 	= 2,
	RETURN_EXIT_ALL = 4
};

class CMenuTarget
{
	protected:
		std::string *valueString;
		std::string valueStringTmp;

	public:
		CMenuTarget(){ valueStringTmp = std::string(); valueString = &valueStringTmp; };
		virtual ~CMenuTarget(){};
		virtual void hide(){valueString->clear();};
		virtual int exec(CMenuTarget *parent, const std::string &actionKey) = 0;
		virtual std::string &getString(void) { return *valueString; };
		//virtual std::string getName(void){std::string ret = ""; return ret;};
};

class CWidget : public CMenuTarget
{
	protected:
		CFrameBuffer *frameBuffer;
		CBox mainFrameBox;

		std::vector<CWidgetItem*> items;

		bool paintMainFrame;

		//
		neutrino_msg_t      msg;
		neutrino_msg_data_t data;
		
		int selected;
		bool exit_pressed;
		int retval;
		int pos;
		
		fb_pixel_t * background;
		bool savescreen;
		void saveScreen();
		void restoreScreen();

		bool enableCenter;

		struct keyAction { std::string action; CMenuTarget *menue; };
		std::map<neutrino_msg_t, keyAction> keyActionMap;

		unsigned long long int timeout;
		uint32_t sec_timer_id;

		fb_pixel_t backgroundColor;

		//
		std::string actionKey;

	public:
		CWidget(const int x = 0, const int y = 0, const int dx = MENU_WIDTH, const int dy = MENU_HEIGHT);
		CWidget(CBox *position);
		virtual ~CWidget();

		virtual void initFrames();
		virtual void paintItems();
		virtual void paint();
		virtual void hide();
		virtual int exec(CMenuTarget *parent, const std::string &actionKey);

		virtual void addItem(CWidgetItem *widgetItem, const int x = 0, const int y = 0, const int dx = 0, const int dy = 0, const bool defaultselected = false);
		bool hasItem();
		int getItemsCount()const{return items.size();};
		virtual void clearItems(void){items.clear();};

		void enableCenterPos(){enableCenter = true;};

		void setTimeOut(unsigned long long int to = 0){timeout = to;};

		void addKey(neutrino_msg_t key, CMenuTarget *menue = NULL, const std::string &action = "");
		neutrino_msg_t getKey(){return msg;};

		inline CBox getWindowsPos(void){return(mainFrameBox);};
		bool getExitPressed(){return exit_pressed;};

		void setSelected(unsigned int _new) { if(_new <= items.size()) selected = _new; if (selected < 0) selected = 0;};
		int getSelected(){return exit_pressed ? -1 : selected;};

		void enablePaintMainFrame(){paintMainFrame = true;};
		void setBackgroundColor(fb_pixel_t col) {backgroundColor = col;};

		void enableSaveScreen();

		//
		std::string getActionKey(){return actionKey;};

		// events
		virtual void onOKKeyPressed();
		virtual void onHomeKeyPressed();

		virtual void onUpKeyPressed();
		virtual void onDownKeyPressed();
		virtual void onRightKeyPressed();
		virtual void onLeftKeyPressed();

		virtual void onPageUpKeyPressed();
		virtual void onPageDownKeyPressed();

		virtual void onRedKeyPressed(){};
		virtual void onGreenKeyPressed(){};
		virtual void onYellowKeyPressed();
		virtual void onBlueKeyPressed(){};

		virtual void onMenuKeyPressed(){};
		virtual void onPowerKeyPressed(){};

		virtual void onMuteKeyPressed(){};
		virtual void onVolumeUpKeyPressed(){};
		virtual void onVolumeDownKeyPressed(){};

		virtual void on0KeyPressed(){};
		virtual void on1KeyPressed(){};
		virtual void on2KeyPressed(){};
		virtual void on3KeyPressed(){};
		virtual void on4KeyPressed(){};
		virtual void on5KeyPressed(){};
		virtual void on6KeyPressed(){};
		virtual void on7KeyPressed(){};
		virtual void on8KeyPressed(){};
		virtual void on9KeyPressed(){};

		virtual void onAudioKeyPressed(){};
		virtual void onVideoKeyPressed(){};

		virtual void onTextKeyPressed(){};
		virtual void onInfoKeyPressed(){};
		virtual void onEPGKeyPressed(){};
		virtual void onBackKeyPressed(){};
		virtual void onFavoritesKeyPressed(){};
		virtual void onSatKeyPressed(){};

		virtual void onRecordKeyPressed(){};
		virtual void onPlayKeyPressed(){};
		virtual void onPauseKeyPressed(){};
		virtual void onFastForwardKeyPressed(){};
		virtual void onRewindKeyPressed(){};
		virtual void onStopKeyPressed(){};

		virtual void onTimeshiftKeyPressed(){};
		virtual void onModeKeyPressed(){};
		virtual void onNextKeyPressed(){};
		virtual void onPrevKeyPressed(){};

		virtual void onMusicKeyPressed(){};
		virtual void onPictureKeyPressed(){};
		virtual void onLoopKeyPressed(){};
		virtual void onSlowKeyPressed(){};
		virtual void onDVBSubKeyPressed(){};
		virtual void onPIPKeyPressed(){};
		virtual void onPIPPosKeyPressed(){};
		virtual void onPIPSwapKeyPressed(){};
		virtual void onPIPSubChanKeyPressed(){};
		virtual void onNetKeyPressed(){};
		virtual void onBookmarkKeyPressed(){};
		virtual void onMultifeedKeyPressed(){};
		virtual void onF1KeyPressed(){};
		virtual void onF2KeyPressed(){};
		virtual void onF3KeyPressed(){};
		virtual void onF4KeyPressed(){};
};

#endif // WIDGET_H_

