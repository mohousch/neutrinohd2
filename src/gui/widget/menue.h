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

#include <driver/framebuffer.h>
#include <driver/rcinput.h>
#include <system/localize.h>

#include <string>
#include <vector>


#define MENU_WIDTH			DEFAULT_XRES/2 - 50
#define MENU_HEIGHT			576
#define HINTBOX_WIDTH			MENU_WIDTH + 50

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

class CChangeObserver
{
	public:
		virtual ~CChangeObserver(){}
		virtual bool changeNotify(const neutrino_locale_t, void *)
		{
			return false;
		}
};

class CMenuTarget
{
	public:
		CMenuTarget(){}
		virtual ~CMenuTarget(){}
		virtual void hide(){}
		virtual int exec(CMenuTarget* parent, const std::string & actionKey) = 0;
};

class CMenuItem
{
	protected:
		int x, y, dx, offx;
		
	public:
		bool active;
		neutrino_msg_t directKey;
		neutrino_msg_t msg;
		bool can_arrow;
		std::string iconName;

		CMenuItem()
		{
			x = -1;
			directKey = CRCInput::RC_nokey;
			iconName = "";
			can_arrow = false;
		}
		virtual ~CMenuItem(){}
		virtual void init(const int X, const int Y, const int DX, const int OFFX);
		virtual int paint(bool selected = false, bool AfterPulldown = false) = 0;
		virtual int getHeight(void) const = 0;
		virtual int getWidth(void) const
		{
			return 0;
		}

		virtual bool isSelectable(void) const
		{
			return false;
		}

		virtual int exec(CMenuTarget */*parent*/)
		{
			return 0;
		}
		
		virtual void setActive(const bool Active);
};

// Chooser
class CAbstractMenuOptionChooser : public CMenuItem
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
};

// OptionChooser
class CMenuOptionChooser : public CAbstractMenuOptionChooser
{
	public:
		struct keyval
		{
			const int key;
			const neutrino_locale_t value;
			const char * valname;
		};

	private:
		const struct keyval * options;
		unsigned number_of_options;
		CChangeObserver * observ;
		
		std::string optionNameString;
		
		bool pulldown;

	public:
		CMenuOptionChooser(const neutrino_locale_t OptionName, int * const OptionValue, const struct keyval * const Options, const unsigned Number_Of_Options, const bool Active = false, CChangeObserver * const Observ = NULL, const neutrino_msg_t DirectKey = CRCInput::RC_nokey, const std::string & IconName= "", bool Pulldown = false);
		CMenuOptionChooser(const char* OptionName, int * const OptionValue, const struct keyval * const Options, const unsigned Number_Of_Options, const bool Active = false, CChangeObserver * const Observ = NULL, const neutrino_msg_t DirectKey = CRCInput::RC_nokey, const std::string & IconName= "", bool Pulldown = false); 

		void setOptionValue(const int newvalue);
		int getOptionValue(void) const;
		int paint(bool selected, bool AfterPulldown = false);
		int exec(CMenuTarget * parent);
};

// OptionNumberChooser
class CMenuOptionNumberChooser : public CAbstractMenuOptionChooser
{
	const char * optionString;

	int lower_bound;
	int upper_bound;

	int display_offset;

	int localized_value;
	neutrino_locale_t localized_value_name;
	
	std::string nameString;
	neutrino_locale_t name;

	private:
		CChangeObserver * observ;

	public:
		CMenuOptionNumberChooser(const neutrino_locale_t Name, int * const OptionValue, const bool Active, const int min_value, const int max_value, CChangeObserver * const Observ = NULL, const int print_offset = 0, const int special_value = 0, const neutrino_locale_t special_value_name = NONEXISTANT_LOCALE, const char * non_localized_name = NULL);
		CMenuOptionNumberChooser(const char * const Name, int * const OptionValue, const bool Active, const int min_value, const int max_value, CChangeObserver * const Observ = NULL, const int print_offset = 0, const int special_value = 0, const neutrino_locale_t special_value_name = NONEXISTANT_LOCALE, const char * non_localized_name = NULL);
		
		int paint(bool selected, bool AfterPulldown = false);

		int exec(CMenuTarget* parent);
};

class CMenuOptionStringChooser : public CMenuItem
{
		std::string nameString;
		neutrino_locale_t name;
		int height;
		char * optionValue;
		std::vector<std::string> options;
		CChangeObserver * observ;
		bool pulldown;

	public:
		CMenuOptionStringChooser(const neutrino_locale_t Name, char* OptionValue, bool Active = false, CChangeObserver* Observ = NULL, const neutrino_msg_t DirectKey = CRCInput::RC_nokey, const std::string & IconName= "", bool Pulldown = false);
		CMenuOptionStringChooser(const char * Name, char* OptionValue, bool Active = false, CChangeObserver* Observ = NULL, const neutrino_msg_t DirectKey = CRCInput::RC_nokey, const std::string & IconName= "", bool Pulldown = false);
		~CMenuOptionStringChooser();

		void addOption(const char * value);
		int paint(bool selected, bool AfterPulldown = false);
		int getHeight(void) const
		{
			return height;
		}
		bool isSelectable(void) const
		{
			return active;
		}

		int exec(CMenuTarget *parent);
};

class CMenuOptionLanguageChooser : public CMenuItem
{
		int height;
		char * optionValue;
		std::vector<std::string> options;
		CChangeObserver * observ;

	public:
		CMenuOptionLanguageChooser(char *Name, CChangeObserver *Observ = NULL, const char *const IconName = NULL);
		~CMenuOptionLanguageChooser();

		void addOption(const char * value);
		int paint(bool selected, bool AfterPulldown = false);
		int getHeight(void) const
		{
			return height;
		}
		bool isSelectable(void) const
		{
			return true;
		}

		int exec(CMenuTarget *parent);
};

// menuforwarder
class CMenuForwarder : public CMenuItem
{
	const char * option;
	const std::string * option_string;
	CMenuTarget * jumpTarget;
	std::string actionKey;

	protected:
		neutrino_locale_t text;
		std::string textString;

		virtual const char * getOption(void);
		virtual const char * getName(void);
		
	public:

		CMenuForwarder(const neutrino_locale_t Text, const bool Active = true, const char * const Option = NULL, CMenuTarget * Target = NULL, const char * const ActionKey = NULL, const neutrino_msg_t DirectKey = CRCInput::RC_nokey, const char * const IconName = NULL);
		CMenuForwarder(const neutrino_locale_t Text, const bool Active, const std::string &Option, CMenuTarget * Target = NULL, const char * const ActionKey = NULL, const neutrino_msg_t DirectKey = CRCInput::RC_nokey, const char * const IconName = NULL);
		CMenuForwarder(const char * const Text, const bool Active = true, const char * const Option = NULL, CMenuTarget * Target = NULL, const char * const ActionKey = NULL, const neutrino_msg_t DirectKey = CRCInput::RC_nokey, const char * const IconName = NULL);
		CMenuForwarder(const char * const Text, const bool Active, const std::string &Option, CMenuTarget * Target = NULL, const char * const ActionKey = NULL, const neutrino_msg_t DirectKey = CRCInput::RC_nokey, const char * const IconName = NULL);
		
		int paint(bool selected = false, bool AfterPulldown = false);
		int getHeight(void) const;
		int getWidth(void) const;
		int exec(CMenuTarget * parent);
		bool isSelectable(void) const
		{
			return active;
		}
};

class CMenuSeparator : public CMenuItem
{
	int type;

	public:
		neutrino_locale_t text;

		enum
		{
			EMPTY =	0,
			LINE =	1,
			STRING =	2,
			ALIGN_CENTER = 4,
			ALIGN_LEFT =   8,
			ALIGN_RIGHT = 16
		};


		CMenuSeparator(const int Type = EMPTY, const neutrino_locale_t Text = NONEXISTANT_LOCALE);

		int paint(bool selected = false, bool AfterPulldown = false);
		int getHeight(void) const;
		int getWidth(void) const;

		virtual const char * getString(void);
};

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

class CZapProtection : public CPINProtection
{
	protected:
		virtual CMenuTarget * getParent() { return( NULL);};
	public:
		int fsk;

		CZapProtection( char * validpin, int FSK ) : CPINProtection(validpin){ fsk = FSK; };
		bool check();
};

class CLockedMenuForwarder : public CMenuForwarder, public CPINProtection
{
	CMenuTarget * Parent;
	bool AlwaysAsk;

	protected:
		virtual CMenuTarget* getParent(){ return Parent;};
	public:
		CLockedMenuForwarder(const neutrino_locale_t Text, char * _validPIN, bool alwaysAsk = false, const bool Active = true, char * Option = NULL, CMenuTarget * Target = NULL, const char * const ActionKey = NULL, neutrino_msg_t DirectKey = CRCInput::RC_nokey, const char * const IconName = NULL)
		: CMenuForwarder(Text, Active, Option, Target, ActionKey, DirectKey, IconName) ,
		  CPINProtection( _validPIN){AlwaysAsk = alwaysAsk;};
		  
		CLockedMenuForwarder(const char * const Text, char * _validPIN, bool alwaysAsk = false, const bool Active = true, char * Option = NULL, CMenuTarget * Target = NULL, const char * const ActionKey = NULL, neutrino_msg_t DirectKey = CRCInput::RC_nokey, const char * const IconName = NULL)
		: CMenuForwarder(Text, Active, Option, Target, ActionKey, DirectKey, IconName) ,
		  CPINProtection( _validPIN){AlwaysAsk = alwaysAsk;};

		virtual int exec(CMenuTarget * parent);
};

// This Class creates a menue item, which writes its caption to an given string (or an given int value to an given variable). 
// The programm could use this class to verify, what menu was selected. 
// A good listbox class might do the same. There might be better ways to do so.
class CMenuSelector : public CMenuItem
{
	private:
		const char * optionName;
		char * optionValue;
		std::string* optionValueString;
		int  returnIntValue;
		int* returnInt;
		int height;
		char buffer[20];
	public:
		CMenuSelector(const char * OptionName, const bool Active = true, char * OptionValue = NULL, int* ReturnInt = NULL,int ReturnIntValue = 0);
		CMenuSelector(const char * OptionName, const bool Active , std::string & OptionValue, int* ReturnInt = NULL,int ReturnIntValue = 0);
		int exec(CMenuTarget* parent);
		int paint(bool selected, bool AfterPulldown = false);
		int getHeight(void) const{return height;};
		bool isSelectable(void) const {	return active;}
};

// new (items with icons)
class CMenuForwarderExtended : public CMenuItem
{
	CMenuTarget * jumpTarget;
	std::string actionKey;

	protected:
		std::string textString;
		neutrino_locale_t text;
		
		std::string helptext;
		std::string itemIcon;

		virtual const char * getName(void);
		virtual const char * getHelpText(void);
		
	public:

		CMenuForwarderExtended(const neutrino_locale_t Text, const bool Active = true, CMenuTarget* Target = NULL, const char * const ActionKey = NULL, const neutrino_msg_t DirectKey = CRCInput::RC_nokey, const char * const IconName = NULL, const char * const ItemIcon = NULL, const neutrino_locale_t HelpText = NONEXISTANT_LOCALE );
		CMenuForwarderExtended(const char * const Text, const bool Active = true, CMenuTarget* Target = NULL, const char * const ActionKey = NULL, const neutrino_msg_t DirectKey = CRCInput::RC_nokey, const char * const IconName = NULL, const char * const ItemIcon = NULL, const neutrino_locale_t HelpText = NONEXISTANT_LOCALE );
		
		int paint(bool selected = false, bool AfterPulldown = false);
		int getHeight(void) const;
		int getWidth(void) const;
		int exec(CMenuTarget * parent);
		bool isSelectable(void) const
		{
			return active;
		}
};

class CLockedMenuForwarderExtended : public CMenuForwarderExtended, public CPINProtection
{
	CMenuTarget * Parent;
	bool AlwaysAsk;

	protected:
		virtual CMenuTarget* getParent(){ return Parent;};
	public:
		CLockedMenuForwarderExtended(const neutrino_locale_t Text, char * _validPIN, bool alwaysAsk = false, const bool Active = true, CMenuTarget * Target = NULL, const char * const ActionKey = NULL, neutrino_msg_t DirectKey = CRCInput::RC_nokey, const char * const IconName = NULL, const char * const ItemIcon = NULL, const neutrino_locale_t HelpText = NONEXISTANT_LOCALE )
		 : CMenuForwarderExtended(Text, Active, Target, ActionKey, DirectKey, IconName, ItemIcon, HelpText) ,
		   CPINProtection( _validPIN){AlwaysAsk = alwaysAsk;};
		   
		CLockedMenuForwarderExtended(const char * const Text, char * _validPIN, bool alwaysAsk = false, const bool Active = true, CMenuTarget * Target = NULL, const char * const ActionKey = NULL, neutrino_msg_t DirectKey = CRCInput::RC_nokey, const char * const IconName = NULL, const char * const ItemIcon = NULL, const neutrino_locale_t HelpText = NONEXISTANT_LOCALE )
		 : CMenuForwarderExtended(Text, Active, Target, ActionKey, DirectKey, IconName, ItemIcon, HelpText) ,
		   CPINProtection( _validPIN){AlwaysAsk = alwaysAsk;};

		virtual int exec(CMenuTarget* parent);
};

// CMenuWidget
class CMenuWidget : public CMenuTarget
{
	protected:
		std::string nameString;
		neutrino_locale_t name;
		CFrameBuffer *frameBuffer;
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
		int sp_height;
		int item_height;
		int item_width;
		int sb_width;
		int itemHeightTotal;
		int heightCurrPage;
		int items_height;
		int items_width;
		int heightFirstPage;
		
	public:
		CMenuWidget();
		CMenuWidget(const char * const Name, const std::string & Icon = "", const int mwidth = MENU_WIDTH, const int mheight = MENU_HEIGHT);
		CMenuWidget(const neutrino_locale_t Name, const std::string & Icon = "", const int mwidth = MENU_WIDTH, const int mheight = MENU_HEIGHT);
		
		~CMenuWidget();

		virtual void addItem(CMenuItem * menuItem, const bool defaultselected = false);
		bool hasItem();
		virtual void paint();
		virtual void hide();
		virtual int exec(CMenuTarget* parent, const std::string & actionKey);
		void setSelected(unsigned int _new) { if(_new <= items.size()) selected = _new; };
		int getSelected() { return selected; };
		void move(int xoff, int yoff);
		int getSelectedLine(void){return exit_pressed ? -1 : selected;};
		
		int getHeight(void) const
		{
			return height;
		}
		
		void enableSaveScreen(bool enable);
};

// CLass to get the menu line selected by the user. There might be better ways to do so.
class CMenuWidgetSelection : public CMenuWidget
{
	public:
		CMenuWidgetSelection(const neutrino_locale_t Name, const std::string & Icon = "", const int mwidth = MENU_WIDTH, const int mheight = MENU_HEIGHT) : CMenuWidget( Name, Icon, mwidth, mheight){;};
		int getSelectedLine(void){return exit_pressed ? -1 : selected;};
};

class CMenuSelectorTarget : public CMenuTarget
{
        public:
                CMenuSelectorTarget(int *select) {m_select = select;};
                int exec(CMenuTarget* parent, const std::string & actionKey);

        private:
                int *m_select;
};

// I tried a lot to use the menu.cpp as ListBox selection, and I got three solution which are all garbage. 
//Might be replaced by somebody who is familiar with this stuff .
class CSelectedMenu : public CMenuTarget
{
	public:
		bool selected;
		CSelectedMenu(void){selected = false;};
		inline	int exec(CMenuTarget */*parent*/, const std::string &/*actionKey*/){selected = true; return menu_return::RETURN_EXIT;};
};

#endif
