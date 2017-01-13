/*
*/

#include <ard.h>
#include <system/debug.h>


// verpasst
CARDverpasst::CARDverpasst()
{
}

CARDverpasst::~CARDverpasst()
{
}

void CARDverpasst::showMenu(void)
{
	CMenuWidget * catMenu = new CMenuWidget("Sendung verpasst", PLUGINDIR "/mediaportal/ard_small.png");

	catMenu->disableMenuPosition();

	catMenu->addItem(new CMenuForwarder("Das Erste", true, NULL, new CARDverpasstSub("Das Erste"), ""));
	catMenu->addItem(new CMenuForwarder("Tagesschau24", true, NULL, new CARDverpasstSub("Tagesschau24"), ""));
	catMenu->addItem(new CMenuForwarder("Einsplus", true, NULL, new CARDverpasstSub("Einsplus"), ""));
	catMenu->addItem(new CMenuForwarder("Einfestival", true, NULL, new CARDverpasstSub("Einsfestival"), ""));
	catMenu->addItem(new CMenuForwarder("BR", true, NULL, new CARDverpasstSub("BR"), ""));
	catMenu->addItem(new CMenuForwarder("HR", true, NULL, new CARDverpasstSub("HR"), ""));
	catMenu->addItem(new CMenuForwarder("MDR", true, NULL, new CARDverpasstSub("MDR"), ""));
	catMenu->addItem(new CMenuForwarder("MDR Sachseninhalt", true, NULL, new CARDverpasstSub("MDR Sachseninhalt"), ""));
	catMenu->addItem(new CMenuForwarder("NDR", true, NULL, new CARDverpasstSub("NDR"), ""));
	catMenu->addItem(new CMenuForwarder("RB", true, NULL, new CARDverpasstSub("RB"), ""));
	catMenu->addItem(new CMenuForwarder("SR", true, NULL, new CARDverpasstSub("SR"), ""));
	catMenu->addItem(new CMenuForwarder("SWR", true, NULL, new CARDverpasstSub("SWR"), ""));
	catMenu->addItem(new CMenuForwarder("WDR", true, NULL, new CARDverpasstSub("WDR"), ""));	
	
	catMenu->exec(NULL, "");
	catMenu->hide();
	delete catMenu;
	catMenu = NULL;
}

int CARDverpasst::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CARDverpasst::exec: actionKey:%s\n", actionKey.c_str());

	if(parent)
		parent->hide();

	showMenu();

	return menu_return::RETURN_REPAINT;
}

// verpasstSub
CARDverpasstSub::CARDverpasstSub(const std::string& title)
{
	Title = title;
}

CARDverpasstSub::~CARDverpasstSub()
{
}

void CARDverpasstSub::showMenu(void)
{
	CMenuWidget * catMenu = new CMenuWidget(Title.c_str(), PLUGINDIR "/mediaportal/ard_small.png");

	catMenu->disableMenuPosition();

	catMenu->addItem(new CMenuForwarder("Heute", true, __DATE__, new CARDtime("Heute"), ""));
	catMenu->addItem(new CMenuForwarder("Gestern", true, NULL, new CARDtime("Gestern"), ""));
	catMenu->addItem(new CMenuForwarder("Mittwoch", true, NULL, new CARDtime("Mittwoch"), ""));
	catMenu->addItem(new CMenuForwarder("Dienstag", true, NULL, new CARDtime("Dienstag"), ""));
	catMenu->addItem(new CMenuForwarder("Montag", true, NULL, new CARDtime("Montag"), ""));
	catMenu->addItem(new CMenuForwarder("Sonntag", true, NULL, new CARDtime("Sonntag"), ""));
	catMenu->addItem(new CMenuForwarder("Samstag", true, NULL, new CARDtime("Samstag"), ""));	
	
	catMenu->exec(NULL, "");
	catMenu->hide();
	delete catMenu;
	catMenu = NULL;
}

int CARDverpasstSub::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CARDverpasst::exec: actionKey:%s\n", actionKey.c_str());

	if(parent)
		parent->hide();

	showMenu();

	return menu_return::RETURN_REPAINT;
}

// time
CARDtime::CARDtime(const std::string& title)
{
	Title = title;
}

CARDtime::~CARDtime()
{
}

void CARDtime::showMenu(void)
{
	CMenuWidget * catMenu = new CMenuWidget(Title.c_str(), PLUGINDIR "/mediaportal/ard_small.png");

	catMenu->disableMenuPosition();

	catMenu->addItem(new CMenuForwarder("00:00-12:00", true, NULL, new CARDstreams("00:00-12:00"), ""));
	catMenu->addItem(new CMenuForwarder("12:00-18:00", true, NULL, new CARDstreams("12:00-18:00"), ""));
	catMenu->addItem(new CMenuForwarder("18:00-20:15", true, NULL, new CARDstreams("18:00-20:15"), ""));
	catMenu->addItem(new CMenuForwarder("20:15-00:00", true, NULL, new CARDstreams("20:15-00:00"), ""));	
	
	catMenu->exec(NULL, "");
	catMenu->hide();
	delete catMenu;
	catMenu = NULL;
}

int CARDtime::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CARDverpasst::exec: actionKey:%s\n", actionKey.c_str());

	if(parent)
		parent->hide();

	showMenu();

	return menu_return::RETURN_REPAINT;
}

// A-Z
CARDaz::CARDaz()
{
}

CARDaz::~CARDaz()
{
}

void CARDaz::showMenu(void)
{
	CMenuFrameBox * catMenu = new CMenuFrameBox("Sendungen A-Z", PLUGINDIR "/mediaportal/ard_small.png");

	// items
	
	catMenu->exec(NULL, "");
	catMenu->hide();
	delete catMenu;
	catMenu = NULL;
}

int CARDaz::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CARDaz::exec: actionKey:%s\n", actionKey.c_str());

	if(parent)
		parent->hide();

	showMenu();

	return menu_return::RETURN_REPAINT;
}

// TVlive
CARDTVlive::CARDTVlive()
{
}

CARDTVlive::~CARDTVlive()
{
}

void CARDTVlive::showMenu(void)
{
	CMenuFrameBox * catMenu = new CMenuFrameBox("TV-Livestreams", PLUGINDIR "/mediaportal/ard_small.png");

	// items
	
	catMenu->exec(NULL, "");
	catMenu->hide();
	delete catMenu;
	catMenu = NULL;
}

int CARDTVlive::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CARDlive::exec: actionKey:%s\n", actionKey.c_str());

	if(parent)
		parent->hide();

	showMenu();

	return menu_return::RETURN_REPAINT;
}

// radio live
CARDRadiolive::CARDRadiolive()
{
}

CARDRadiolive::~CARDRadiolive()
{
}

void CARDRadiolive::showMenu(void)
{
	CMenuFrameBox * catMenu = new CMenuFrameBox("Radio-Livestreams", PLUGINDIR "/mediaportal/ard_small.png");

	// items
	
	catMenu->exec(NULL, "");
	catMenu->hide();
	delete catMenu;
	catMenu = NULL;
}

int CARDRadiolive::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CARDRadiolive::exec: actionKey:%s\n", actionKey.c_str());

	if(parent)
		parent->hide();

	showMenu();

	return menu_return::RETURN_REPAINT;
}

// streams
CARDstreams::CARDstreams(const std::string& title)
{
	Title = title;
}

CARDstreams::~CARDstreams()
{
}

void CARDstreams::showMenu(void)
{
	CMenuFrameBox * catMenu = new CMenuFrameBox(Title.c_str(), PLUGINDIR "/mediaportal/ard_small.png");	
	
	//items

	catMenu->exec(NULL, "");
	catMenu->hide();
	delete catMenu;
	catMenu = NULL;
}

int CARDstreams::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CARDstreams::exec: actionKey:%s\n", actionKey.c_str());

	if(parent)
		parent->hide();

	showMenu();

	return menu_return::RETURN_REPAINT;
}

// ard
CARD::CARD()
{
}

CARD::~CARD()
{
}

void CARD::showMenu(void)
{
	CMenuWidget * catMenu = new CMenuWidget("ARD Mediathek", PLUGINDIR "/mediaportal/ard_small.png");

	catMenu->disableMenuPosition();

	catMenu->addItem(new CMenuForwarder("Sendung verpasst", true, NULL, new CARDverpasst(), ""));
	catMenu->addItem(new CMenuForwarder("Sendungen A-Z", true, NULL, new CARDaz(), ""));
	catMenu->addItem(new CMenuForwarder("TV-Livestreams", true, NULL, new CARDTVlive(), ""));
	catMenu->addItem(new CMenuForwarder("Radio-Livestreams", false, NULL, new CARDRadiolive(), ""));
	
	catMenu->exec(NULL, "");
	catMenu->hide();
	delete catMenu;
	catMenu = NULL;
}

int CARD::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CARD::exec: actionKey:%s\n", actionKey.c_str());

	if(parent)
		parent->hide();

	showMenu();

	return menu_return::RETURN_REPAINT;
}


