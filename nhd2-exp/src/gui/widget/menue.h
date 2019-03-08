/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: menue.h 2013/10/12 mohousch Exp $

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


#ifndef __MENU__
#define __MENU__

#include <string>
#include <vector>

#include <driver/framebuffer.h>
#include <driver/rcinput.h>

#include <system/localize.h>
#include <system/helpers.h>

#include <gui/widget/widget_helpers.h>
#include <gui/widget/headers.h>

#include <driver/color.h>
#include <gui/plugins.h>

#include <system/settings.h>

//extern CFont* g_Font[FONT_TYPE_COUNT];


#define MENU_WIDTH			590
#define MENU_HEIGHT			700

enum 
{
	ITEM_TYPE_OPTION_CHOOSER,
	ITEM_TYPE_OPTION_NUMBER_CHOOSER,
	ITEM_TYPE_OPTION_STRING_CHOOSER,
	ITEM_TYPE_OPTION_LANGUAGE_CHOOSER,
	ITEM_TYPE_SEPARATOR,
	ITEM_TYPE_FORWARDER,
	ITEM_TYPE_SELECTOR,
	ITEM_TYPE_LISTBOX,
	ITEM_TYPE_LISTBOX_ENTRY
};

enum
{
	WIDGET_TYPE_STANDARD = 0,
	WIDGET_TYPE_CLASSIC,
	WIDGET_TYPE_EXTENDED,
	WIDGET_TYPE_FRAME
};

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

enum
{
	EMPTY =	0,
	LINE = 1,
	STRING = 2,
	ALIGN_CENTER = 4,
	ALIGN_LEFT = 8,
	ALIGN_RIGHT = 16
};

struct keyval
{
	const int key;
	const neutrino_locale_t value;
	const char* valname;
};

// CChangeObserver
class CChangeObserver
{
	public:
		virtual ~CChangeObserver(){}
		virtual bool changeNotify(const neutrino_locale_t, void *)
		{
			return false;
		}

		virtual bool changeNotify(const std::string&, void *)
		{
			return false;
		}
};

// CMenuTarget
class CMenuTarget
{
	public:
		CMenuTarget(){};
		virtual ~CMenuTarget(){};
		virtual void hide(){};
		virtual int exec(CMenuTarget *parent, const std::string &actionKey) = 0;
};

// CMenuItem
class CMenuItem
{
	protected:
		int x, y, dx, offx;
		
	public:
		bool active;
		bool marked;
		neutrino_msg_t directKey;
		neutrino_msg_t msg;
		bool can_arrow;
		std::string iconName;
		std::string itemName;
		std::string option;
		std::string optionInfo;
		std::string itemHelpText;
		std::string itemIcon;
		std::string info1, option_info1;
		std::string info2, option_info2;

		//
		std::string icon1;
		std::string icon2;
		int number;
		int runningPercent;

		//
		int itemType;
		int widgetType;

		//
		int item_height;
		int item_width;
		fb_pixel_t item_backgroundColor;
		fb_pixel_t item_selectedColor;

		//
		//static CFont* nameFont;
		//static CFont* optionFont;

		bool nLinesItem; // 2 lines Item (classicWidget)

		CMenuItem();
		virtual ~CMenuItem(){}

		virtual void init(const int X, const int Y, const int DX, const int OFFX);
		virtual int paint(bool selected = false, bool AfterPulldown = false) = 0;
		virtual int getHeight(void) const = 0;
		virtual int getWidth(void) const
		{
			return 0;
		}

		virtual bool isSelectable(void) const {return false;}

		virtual int exec(CMenuTarget */*parent*/) {return 0;}
		
		//
		virtual void setActive(const bool Active);
		virtual void setMarked(const bool Marked);

		//
		virtual int getYPosition(void) const { return y; }
		virtual int getItemType(){ return itemType;};

		//
		virtual void setOption(const char* text){option = text;};
		virtual void setOptionInfo(const char* text){optionInfo = text;};
		virtual void setInfo1(const char* const text){info1 = text;};
		virtual void setInfo2(const char* const text){info2 = text;};
		virtual void setOptionInfo1(const char* const text){option_info1 = text;};
		virtual void setOptionInfo2(const char* const text){option_info2 = text;};

		//
		virtual void setHelpText(const char* const Text){itemHelpText =  Text;};

		//
		virtual void setIconName(const char* const icon){iconName = icon;};
		virtual void setItemIcon(const char* const icon){itemIcon = icon;};

		//
		virtual void setIcon1(const char* const icon){icon1 = icon;};
		virtual void setIcon2(const char* const icon){icon2 = icon;};
		virtual void setNumber(int nr){number = nr;};
		virtual void setPercent(int percent = -1){runningPercent = percent;};

		//virtual void setNameFont(CFont* font = g_Font[SNeutrinoSettings::FONT_TYPE_MENU]){nameFont = font;};
		//virtual void setOptionFont(CFont* font = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]){optionFont = font;};

		virtual void setnLinesItem(void){nLinesItem = true;};
		virtual void setWidgetType(int type){widgetType = type;};
};

// CMenuOptionChooser
class CMenuOptionChooser : public CMenuItem
{
	protected:
		neutrino_locale_t optionName;
		int height;
		int * optionValue;

		int getHeight(void) const
		{
			return height;
		}
		
		bool isSelectable(void) const
		{
			return active;
		}

	private:
		const struct keyval* options;
		unsigned number_of_options;
		CChangeObserver* observ;
		
		std::string optionNameString;
		
		bool pulldown;
		bool enableMenuPos;

	public:
		CMenuOptionChooser(const neutrino_locale_t OptionName, int * const OptionValue, const struct keyval * const Options, const unsigned Number_Of_Options, const bool Active = false, CChangeObserver * const Observ = NULL, const neutrino_msg_t DirectKey = RC_nokey, const std::string & IconName= "", bool Pulldown = false, bool EnableMenuPos = false);
		CMenuOptionChooser(const char* const OptionName, int * const OptionValue, const struct keyval * const Options, const unsigned Number_Of_Options, const bool Active = false, CChangeObserver * const Observ = NULL, const neutrino_msg_t DirectKey = RC_nokey, const std::string & IconName= "", bool Pulldown = false, bool EnableMenuPos = false); 

		void setOptionValue(const int newvalue);
		int getOptionValue(void) const;

		int paint(bool selected, bool AfterPulldown = false);

		int exec(CMenuTarget* parent);
};

// CMenuOptionNumberChooser
class CMenuOptionNumberChooser : public CMenuItem
{
	const char* optionString;

	int lower_bound;
	int upper_bound;

	int display_offset;

	int localized_value;
	neutrino_locale_t localized_value_name;
	
	std::string nameString;
	neutrino_locale_t name;

	protected:
		neutrino_locale_t optionName;
		int height;
		int* optionValue;

		int getHeight(void) const
		{
			return height;
		}
		
		bool isSelectable(void) const
		{
			return active;
		}

	private:
		CChangeObserver * observ;

	public:
		CMenuOptionNumberChooser(const neutrino_locale_t Name, int * const OptionValue, const bool Active, const int min_value, const int max_value, CChangeObserver * const Observ = NULL, const int print_offset = 0, const int special_value = 0, const neutrino_locale_t special_value_name = NONEXISTANT_LOCALE, const char * non_localized_name = NULL);
		CMenuOptionNumberChooser(const char * const Name, int * const OptionValue, const bool Active, const int min_value, const int max_value, CChangeObserver * const Observ = NULL, const int print_offset = 0, const int special_value = 0, const neutrino_locale_t special_value_name = NONEXISTANT_LOCALE, const char * non_localized_name = NULL);
		
		int paint(bool selected, bool AfterPulldown = false);

		int exec(CMenuTarget *parent);
};

// CMenuOptionStringChooser
class CMenuOptionStringChooser : public CMenuItem
{
	std::string nameString;
	neutrino_locale_t name;
	int height;
	char * optionValue;
	std::vector<std::string> options;
	CChangeObserver * observ;
	bool pulldown;
	bool enableMenuPos;

	public:
		CMenuOptionStringChooser(const neutrino_locale_t Name, char* OptionValue, bool Active = false, CChangeObserver* Observ = NULL, const neutrino_msg_t DirectKey = RC_nokey, const std::string & IconName= "", bool Pulldown = false, bool EnableMenuPos = false);
		CMenuOptionStringChooser(const char * Name, char * OptionValue, bool Active = false, CChangeObserver* Observ = NULL, const neutrino_msg_t DirectKey = RC_nokey, const std::string & IconName= "", bool Pulldown = false, bool EnableMenuPos = false);
		~CMenuOptionStringChooser();

		void addOption(const char * value);

		int paint(bool selected, bool AfterPulldown = false);
		int getHeight(void) const {return height;}

		bool isSelectable(void) const {return active;}

		int exec(CMenuTarget* parent);
};

// CMenuOptionLanguageChooser
class CMenuOptionLanguageChooser : public CMenuItem
{
	int height;
	char * optionValue;
	std::vector<std::string> options;
	CChangeObserver * observ;

	public:
		CMenuOptionLanguageChooser(char *Name, CChangeObserver *Observ = NULL, const char *const IconName = NULL);
		virtual ~CMenuOptionLanguageChooser();

		void addOption(const char* value);

		int paint(bool selected, bool AfterPulldown = false);
		int getHeight(void) const {return height;}

		bool isSelectable(void) const {return true;}

		int exec(CMenuTarget * parent);
};

// CMenuSeparator
class CMenuSeparator : public CMenuItem
{
	int type;

	public:
		const char * textString;

/*
		enum
		{
			EMPTY =	0,
			LINE = 1,
			STRING = 2,
			ALIGN_CENTER = 4,
			ALIGN_LEFT = 8,
			ALIGN_RIGHT = 16
		};
*/

		CMenuSeparator(const int Type = EMPTY, const char * const Text = NULL);

		int paint(bool selected = false, bool AfterPulldown = false);
		int getHeight(void) const;
		int getWidth(void) const;

		virtual const char * getString(void);
};

// CMenuSelector
class CMenuSelector : public CMenuItem
{
	private:
		const char * optionName;
		char * optionValue;
		std::string* optionValueString;
		int  returnIntValue;
		int * returnInt;
		int height;
		char buffer[20];
	public:
		CMenuSelector(const char * OptionName, const bool Active = true, const char * const OptionValue = NULL, int * ReturnInt = NULL, int ReturnIntValue = 0);

		int exec(CMenuTarget * parent);

		int paint(bool selected, bool AfterPulldown = false);
		int getHeight(void) const{return height;};

		bool isSelectable(void) const {	return active;}
};

// CMenuForwarder
class CMenuForwarder : public CMenuItem
{
	CMenuTarget* jumpTarget;
	std::string actionKey;

	protected:
		std::string textString;
		neutrino_locale_t text;

		virtual const char* getName(void);
		virtual const char* getOption(void);
	public:

		CMenuForwarder(const neutrino_locale_t Text, const bool Active = true, const char * const Option = NULL, CMenuTarget* Target = NULL, const char* const ActionKey = NULL, const neutrino_msg_t DirectKey = RC_nokey, const char* const IconName = NULL, const char* const ItemIcon = NULL, const neutrino_locale_t HelpText = NONEXISTANT_LOCALE );
		
		CMenuForwarder(const char* const Text, const bool Active = true, const char* const Option = NULL, CMenuTarget* Target = NULL, const char* const ActionKey = NULL, const neutrino_msg_t DirectKey = RC_nokey, const char* const IconName = NULL, const char* const ItemIcon = NULL, const neutrino_locale_t HelpText = NONEXISTANT_LOCALE );
		
		int paint(bool selected = false, bool AfterPulldown = false);
		int getHeight(void) const;
		int getWidth(void) const;

		int exec(CMenuTarget* parent);
		bool isSelectable(void) const {return active;}
};

// CPINProtection
class CPINProtection
{
	protected:
		char * validPIN;
		bool check();
		virtual CMenuTarget * getParent() = 0;
	public:
		CPINProtection( char *validpin){ validPIN = validpin;};
		virtual ~CPINProtection(){}
};

// CZapProtection
class CZapProtection : public CPINProtection
{
	protected:
		virtual CMenuTarget * getParent() { return( NULL);};
	public:
		int fsk;

		CZapProtection( char * validpin, int FSK ) : CPINProtection(validpin){ fsk = FSK; };
		bool check();
};

// CLockedMenuForwarder
class CLockedMenuForwarder : public CMenuForwarder, public CPINProtection
{
	CMenuTarget * Parent;
	bool AlwaysAsk;

	protected:
		virtual CMenuTarget* getParent(){ return Parent;};
	public:
		CLockedMenuForwarder(const neutrino_locale_t Text, char * _validPIN, bool alwaysAsk = false, const bool Active = true, char * Option = NULL, CMenuTarget * Target = NULL, const char * const ActionKey = NULL, neutrino_msg_t DirectKey = RC_nokey, const char * const IconName = NULL, const char * const ItemIcon = NULL, const neutrino_locale_t HelpText = NONEXISTANT_LOCALE )
		 : CMenuForwarder(Text, Active, Option, Target, ActionKey, DirectKey, IconName, ItemIcon, HelpText) ,
		   CPINProtection( _validPIN){AlwaysAsk = alwaysAsk;};
		   
		CLockedMenuForwarder(const char * const Text, char * _validPIN, bool alwaysAsk = false, const bool Active = true, char * Option = NULL, CMenuTarget * Target = NULL, const char * const ActionKey = NULL, neutrino_msg_t DirectKey = RC_nokey, const char * const IconName = NULL, const char * const ItemIcon = NULL, const neutrino_locale_t HelpText = NONEXISTANT_LOCALE )
		 : CMenuForwarder(Text, Active, Option, Target, ActionKey, DirectKey, IconName, ItemIcon, HelpText) ,
		   CPINProtection( _validPIN){AlwaysAsk = alwaysAsk;};

		virtual int exec(CMenuTarget* parent);
};

// CMenulistBoxItem
class ClistBoxItem : public CMenuItem
{
	CMenuTarget* jumpTarget;
	std::string actionKey;

	protected:
		//
		neutrino_locale_t text;
		std::string textString;

		//
		virtual const char* getName(void);

	public:
		ClistBoxItem(const neutrino_locale_t Text, const bool Active = true, const char* const Option = NULL, CMenuTarget * Target = NULL, const char* const ActionKey = NULL, const char* const Icon = NULL, const char* const ItemIcon = NULL);

		ClistBoxItem(const char* const Text, const bool Active = true, const char* const Option = NULL, CMenuTarget * Target = NULL, const char* const ActionKey = NULL, const char* const IconName = NULL, const char* const ItemIcon = NULL);
		
		int paint(bool selected = false, bool AfterPulldown = false);
		int getHeight(void) const;
		int getWidth(void) const;

		int exec(CMenuTarget* parent);
		bool isSelectable(void) const {return active;}
};

// CMenulistBoxEntryItem
class ClistBoxEntryItem : public CMenuItem
{
	protected:
		neutrino_locale_t text;
		std::string textString;

		//
		virtual const char* getName(void);

	public:
		ClistBoxEntryItem(const neutrino_locale_t Text, const bool Active = true, const char * const Option = NULL);

		ClistBoxEntryItem(const char * const Text, const bool Active = true, const char * const Option = NULL);
		
		int paint(bool selected = false, bool AfterPulldown = false);
		int getHeight(void) const;
		int getWidth(void) const;

		bool isSelectable(void) const {return active;}

		int exec(CMenuTarget * parent);
};

// CMenuWidget
class CMenuWidget : public CMenuTarget
{
	protected:
		CFrameBuffer *frameBuffer;

		//
		std::string nameString;
		neutrino_locale_t name;
		std::vector<CMenuItem*>	items;
		std::vector<unsigned int> page_start;
		std::string iconfile;

		int width;
		int height;
		int wanted_height;
		int x;
		int y;
		int offx, offy;
		int iconOffset;
		unsigned int item_start_y;
		unsigned int current_page;
		unsigned int total_pages;
		
		int selected;
		bool exit_pressed;
		
		fb_pixel_t * background;
		int full_width;
		int full_height;
		bool savescreen;
		
		void Init(const std::string & Icon, const int mwidth, const int mheight);
		virtual void paintItems();
		
		void saveScreen();
		void restoreScreen();
		
		int hheight;
		int fheight;
		int item_height;
		int sb_width;
		int items_height;
		int items_width;
		int listmaxshow;

		bool MenuPos;

		//
		int widgetType;
		bool widgetChange;

		// frame
		fb_pixel_t backgroundColor;
		fb_pixel_t itemBoxColor;
		int itemsPerX;
		int itemsPerY;
		int maxItemsPerPage;
		int item_width;

		// footInfo
		bool FootInfo;
		CBox cFrameFootInfo;
		int interFrame;

		//
		CHeaders headers;
		CItems2DetailsLine itemsLine;
		CScrollBar scrollBar;
		
	public:
		CMenuWidget();
		CMenuWidget(const char * const Name, const std::string & Icon = "", const int mwidth = MENU_WIDTH, const int mheight = MENU_HEIGHT);
		CMenuWidget(const neutrino_locale_t Name, const std::string & Icon = "", const int mwidth = MENU_WIDTH, const int mheight = MENU_HEIGHT);
		
		~CMenuWidget();

		virtual void addItem(CMenuItem * menuItem, const bool defaultselected = false);
		bool hasItem();
		virtual void paint();
		virtual void hide();
		virtual int exec(CMenuTarget* parent, const std::string& actionKey);
		void setSelected(unsigned int _new) { if(_new <= items.size()) selected = _new; };
		int getSelected() {return exit_pressed ? -1 : selected;};
		void move(int xoff, int yoff);
		
		int getHeight(void) const {return height;}
		
		void enableSaveScreen();
		void enableMenuPosition(){MenuPos = true;};

		virtual void paintItemInfo(int pos);
		virtual void hideItemInfo();

		virtual void integratePlugins(CPlugins::i_type_t integration = CPlugins::I_TYPE_DISABLED, const unsigned int shortcut = RC_nokey, bool enabled = true);

		//
		void enableWidgetChange(){widgetChange = true;};
		void setWidgetType(int type){widgetType = type;};
		int getWidgetType(){return widgetType;};

		// Frame
		void setBackgroundColor(fb_pixel_t col) {backgroundColor = col;};
		void setItemBoxColor(fb_pixel_t col) {itemBoxColor = col;};
		void setItemsPerPage(int itemsX = 6, int itemsY = 3){itemsPerX = itemsX; itemsPerY = itemsY; maxItemsPerPage = itemsPerX*itemsPerY;};

		//
		void enablePaintFootInfo(){FootInfo = true;};
};

/// CMenulistBox
class ClistBoxWidget : public CMenuTarget
{
	protected:
		//
		CFrameBuffer *frameBuffer;

		//
		std::string nameString;
		neutrino_locale_t name;
		std::vector<CMenuItem*>	items;
		std::vector<unsigned int> page_start;
		std::string iconfile;

		int width;
		int height;
		int wanted_height;
		int wanted_width;
		int x;
		int y;
		int offx, offy;
		int iconOffset;
		unsigned int item_start_y;
		unsigned int current_page;
		unsigned int total_pages;
		
		int selected;
		bool exit_pressed;
		
		fb_pixel_t * background;
		int full_width;
		int full_height;
		bool savescreen;
		
		void Init(const std::string & Icon, const int mwidth, const int mheight);
		virtual void paintItems();
		
		void saveScreen();
		void restoreScreen();
		
		int hheight;
		int fheight;
		int item_height;
		int item_width;
		int sb_width;
		int items_height;
		int items_width;
		int listmaxshow;

		//
		int fbutton_count;
		int fbutton_width;
		const struct button_label* fbutton_labels;

		//
		int hbutton_count;
		const struct button_label* hbutton_labels;

		//
		struct keyAction { std::string action; CMenuTarget *menue; };
		std::map<neutrino_msg_t, keyAction> keyActionMap;

		//
		bool PaintDate;
		int timestr_len;
		uint32_t sec_timer_id;

		// head
		int icon_head_w;
		int icon_head_h;
		const char * l_name;

		// footInfo
		bool FootInfo;
		CBox cFrameFootInfo;
		int footInfoHeight;
		int interFrame;

		//
		unsigned long long int timeout;

		//
		int widgetType;
		bool widgetChange;
		std::vector<int> widget;

		// frame
		fb_pixel_t backgroundColor;
		fb_pixel_t itemBoxColor;
		int itemsPerX;
		int itemsPerY;
		int maxItemsPerPage;

		bool shrinkMenu;

		//
		CHeaders headers;
		CItems2DetailsLine itemsLine;
		CScrollBar scrollBar;
		CButtons buttons;
		
	public:
		ClistBoxWidget();
		ClistBoxWidget(const char * const Name, const std::string& Icon = "", const int mwidth = MENU_WIDTH, const int mheight = MENU_HEIGHT);
		ClistBoxWidget(const neutrino_locale_t Name, const std::string& Icon = "", const int mwidth = MENU_WIDTH, const int mheight = MENU_HEIGHT);
		
		~ClistBoxWidget();

		virtual void addItem(CMenuItem * menuItem, const bool defaultselected = false);
		bool hasItem();
		int getItemsCount()const{return items.size();};
		void clearItems(void){items.clear();};

		//
		void initFrames();
		virtual void paintHead();
		virtual void paintFoot();
		virtual void paint();
		virtual void paintItemInfo(int pos);
		virtual void hideItemInfo();
		virtual void hide();

		//
		virtual int exec(CMenuTarget* parent, const std::string& actionKey);

		void setSelected(unsigned int _new) { if(_new <= items.size()) selected = _new; };
		int getSelected(){return exit_pressed ? -1 : selected;};

		void move(int xoff, int yoff);
		//int getSelectedLine(void){return exit_pressed ? -1 : selected;};
		int getHeight(void) const {return height;}
		int getWidth(void) const {return width;};
		int getX(void) const {return x;};
		int getY(void) const {return y;};
		bool getExitPressed(){return exit_pressed;};
		int getListMaxShow(void) const {return listmaxshow;};
		
		//
		void enableSaveScreen();

		//
		void addKey(neutrino_msg_t key, CMenuTarget *menue, const std::string &action);

		//
		void setFooterButtons(const struct button_label* _fbutton_label, const int _fbutton_count, const int _fbutton_width = 0);
		void setHeaderButtons(const struct button_label* _hbutton_label, const int _hbutton_count);

		//
		void enablePaintDate(void){PaintDate = true;};

		//
		void resizeFrames();
		void enablePaintFootInfo(){FootInfo = true; initFrames();};
		void setFootInfoHeight(int _height = 70){footInfoHeight = _height; resizeFrames();};

		void setTimeOut(int to = 0){timeout = to;};

		//
		void setWidgetType(int type){widgetType = type; widget.push_back(widgetType);};
		int getWidgetType(){return widgetType;};
		void enableWidgetChange(){widgetChange = true;};
		void addWidget(int wtype){widget.push_back(wtype);};

		// Frame
		void setBackgroundColor(fb_pixel_t col) {backgroundColor = col;};
		void setItemBoxColor(fb_pixel_t col) {itemBoxColor = col;};
		void setItemsPerPage(int itemsX = 6, int itemsY = 3){itemsPerX = itemsX; itemsPerY = itemsY; maxItemsPerPage = itemsPerX*itemsPerY;};

		void setName(const std::string& p_name){nameString = p_name;};

		void enableShrinkMenu(){shrinkMenu = true;};

		virtual void integratePlugins(CPlugins::i_type_t integration = CPlugins::I_TYPE_DISABLED, const unsigned int shortcut = RC_nokey, bool enabled = true);
};

#endif
