/*
*/
#ifndef __ARD__
#define __ARD__

#include <gui/widget/menue.h>


class CARDverpasst : public CMenuTarget
{
	private:
		void showMenu(void);
	public:
		CARDverpasst();
		~CARDverpasst();

		int exec(CMenuTarget* parent, const std::string& actionKey);
};

class CARDverpasstSub : public CMenuTarget
{
	private:
		std::string Title;
		void showMenu(void);
	public:
		CARDverpasstSub(const std::string& title);
		~CARDverpasstSub();

		int exec(CMenuTarget* parent, const std::string& actionKey);
};

class CARDaz : public CMenuTarget
{
	private:
		void showMenu(void);
	public:
		CARDaz();
		~CARDaz();

		int exec(CMenuTarget* parent, const std::string& actionKey);
};

class CARDTVlive : public CMenuTarget
{
	private:
		void showMenu(void);
	public:
		CARDTVlive();
		~CARDTVlive();

		int exec(CMenuTarget* parent, const std::string& actionKey);
};

class CARDRadiolive : public CMenuTarget
{
	private:
		void showMenu(void);
	public:
		CARDRadiolive();
		~CARDRadiolive();

		int exec(CMenuTarget* parent, const std::string& actionKey);
};

class CARDstreams : public CMenuTarget
{
	private:
		std::string Title;
		void showMenu(void);
	public:
		CARDstreams(const std::string& title);
		~CARDstreams();

		int exec(CMenuTarget* parent, const std::string& actionKey);
};

class CARD : public CMenuTarget
{
	private:
		void showMenu(void);
	public:
		CARD();
		~CARD();

		int exec(CMenuTarget* parent, const std::string& actionKey);
};

// class stream url parser

#endif // __ARD__
