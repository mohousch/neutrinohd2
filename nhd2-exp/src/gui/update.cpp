/*
	Neutrino-GUI  -   DBoxII-Project

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <global.h>
#include <neutrino.h>

#include <gui/update.h>

#include <driver/encoding.h>
#include <driver/fontrenderer.h>
#include <driver/rcinput.h>

#include <gui/color.h>
#include <gui/filebrowser.h>
#include <system/fsmounter.h>

#include <gui/widget/messagebox.h>
#include <gui/widget/hintbox.h>
#include <gui/widget/stringinput.h>

#include <system/flashtool.h>
#include <system/httptool.h>
#include <system/helpers.h>

#include <curl/curl.h>
//#include <curl/types.h>
#include <curl/easy.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <dirent.h>

#include <sys/stat.h>
#include <sys/vfs.h>

#include <fstream>

#include <system/debug.h>


#define gTmpPath 				"/var/tmp/"
#define gUserAgent 				"neutrino/softupdater 1.0"

#define LIST_OF_UPDATES_LOCAL_FILENAME 		"update.list"
#define RELEASE_CYCLE                  		"4.0"
#define RELEASE_TYPE				"Snapshot"
#define FILEBROWSER_UPDATE_FILTER      		"img"

#define MTD_OF_WHOLE_IMAGE             		0

//FIXME: add the right mtd part (meaned is roofs, on some boxes this contains also kernel) for your boxtype bevor u use this
//NOTE: be carefull with this
#if defined (PLATFORM_DGS)	
#define MTD_DEVICE_OF_UPDATE_PART      "/dev/mtd5"
#elif defined (PLATFORM_GIGABLUE_800SE)
#define MTD_DEVICE_OF_UPDATE_PART      "/dev/mtd0"
#else
#define MTD_DEVICE_OF_UPDATE_PART      "/dev/mtd5"
#endif


CFlashUpdate::CFlashUpdate(int uMode)
	:CProgressWindow()
{
	updateMode = uMode;
	
	setTitle(LOCALE_FLASHUPDATE_HEAD);
	
	// check rootfs, allow flashing only when rootfs is jffs2/yaffs2/squashfs
	struct statfs s;
	
	if (::statfs("/", &s) == 0) 
	{
		if (s.f_type == 0xEF53L /*ext2*/|| s.f_type == 0x6969L/*nfs*/)
			allow_flash = false;
		else if (s.f_type == 0x72b6L/*jffs2*/ || s.f_type == 0x5941ff53L /*yaffs2*/ || s.f_type == 0x73717368L /*squashfs*/ || s.f_type == 0x24051905L/*ubifs*/)
			allow_flash = true;
	}
	else
		//printf("cant check rootfs\n");
		allow_flash = false;
}

class CUpdateMenuTarget : public CMenuTarget
{
	int    myID;
	int *  myselectedID;

	public:
		CUpdateMenuTarget(const int id, int * const selectedID)
		{
			myID = id;
			myselectedID = selectedID;
		}

		virtual int exec(CMenuTarget *, const std::string &)
		{
			*myselectedID = myID;
			return menu_return::RETURN_EXIT_ALL;
		}
};

class CNonLocalizedMenuSeparator : public CMenuSeparator
{
	const char * the_text;

	public:
		CNonLocalizedMenuSeparator(const char *ltext, const neutrino_locale_t Text) : CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, Text)
		{
			the_text = ltext;
		}

		virtual const char * getString(void)
		{
			return the_text;
		}
};

bool CFlashUpdate::selectHttpImage(void)
{
	CHTTPTool httpTool;
	std::string url;
	std::string name;
	std::string version;
	std::string md5;
	std::vector<std::string> updates_lists, urls, names, versions, descriptions, md5s;
	char fileTypes[128];
	int selected = -1;

	httpTool.setStatusViewer(this);
	showStatusMessageUTF(g_Locale->getText(LOCALE_FLASHUPDATE_GETINFOFILE)); // UTF-8

	// NOTE: remember me : i dont like this menu GUI :-(
	CMenuWidget SelectionWidget(LOCALE_FLASHUPDATE_SELECTIMAGE, NEUTRINO_ICON_UPDATE , MENU_WIDTH + 50);
	
	// intros
	SelectionWidget.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));

	std::ifstream urlFile(g_settings.softupdate_url_file);

	dprintf(DEBUG_NORMAL, "[update] file %s\n", g_settings.softupdate_url_file);

	unsigned int i = 0;
	while (urlFile >> url)
	{
		std::string::size_type startpos, endpos;

		dprintf(DEBUG_NORMAL, "[update] url %s\n", url.c_str());

		// extract domain name 
		startpos = url.find("//");
		if (startpos == std::string::npos)
		{
			startpos = 0;
			endpos   = std::string::npos;
			updates_lists.push_back(url.substr(startpos, endpos - startpos));
		}
		else
		{
			startpos = url.find('/', startpos + 2) + 1;
			endpos   = std::string::npos;
			updates_lists.push_back(url.substr(startpos, endpos - startpos));
		}

		SelectionWidget.addItem(new CNonLocalizedMenuSeparator(updates_lists.rbegin()->c_str(), LOCALE_FLASHUPDATE_SELECTIMAGE));
		
		if (httpTool.downloadFile(url, gTmpPath LIST_OF_UPDATES_LOCAL_FILENAME, 20))
		{
			std::ifstream in(gTmpPath LIST_OF_UPDATES_LOCAL_FILENAME);
			
			bool enabled = true; 

			while (in >> url >> version >> md5 >> std::ws)
			{
				urls.push_back(url);
				versions.push_back(version);
				std::getline(in, name);
				names.push_back(name);
				//std::getline(in, md5);
				md5s.push_back(md5);

				dprintf(DEBUG_NORMAL, "[update] url %s version %s md5 %s name %s\n", url.c_str(), version.c_str(), md5.c_str(), name.c_str());

				CFlashVersionInfo versionInfo(versions[i]);

				fileTypes[i] = versionInfo.snapshot;
				std::string description = versionInfo.getType();
				description += ' ';
				description += versionInfo.getDate();
				description += ' ';
				description += versionInfo.getTime();
				
				descriptions.push_back(description); /* workaround since CMenuForwarder does not store the Option String itself */
				
				//if( versionInfo.getType() < '3' && !allow_flash)
				if(!allow_flash && (versionInfo.snapshot < '3'))
					enabled = false;

				SelectionWidget.addItem(new CMenuForwarder(names[i].c_str(), enabled, descriptions[i].c_str(), new CUpdateMenuTarget(i, &selected), NULL, NULL, NEUTRINO_ICON_UPDATE_SMALL ));
				i++;
			}
		}
	}

	hide();

	if (urls.empty())
	{
		HintBox(LOCALE_MESSAGEBOX_ERROR, g_Locale->getText(LOCALE_FLASHUPDATE_GETINFOFILEERROR)); // UTF-8
		return false;
	}
		
	SelectionWidget.exec(NULL, "");

	if (selected == -1)
		return false;

	filename = urls[selected];
	newVersion = versions[selected];
	file_md5 = md5s[selected];
	fileType = fileTypes[selected];

	dprintf(DEBUG_NORMAL, "[update] filename %s type %c newVersion %s md5 %s\n", filename.c_str(), fileType, newVersion.c_str(), file_md5.c_str());

	return true;
}

bool CFlashUpdate::getUpdateImage(const std::string & version)
{
	CHTTPTool httpTool;
	char * fname, dest_name[100];
	httpTool.setStatusViewer(this);

	fname = rindex(const_cast<char *>(filename.c_str()), '/');
	if(fname != NULL) 
		fname++;
	else 
		return false;

	sprintf(dest_name, "%s/%s", g_settings.update_dir, fname);
	showStatusMessageUTF(std::string(g_Locale->getText(LOCALE_FLASHUPDATE_GETUPDATEFILE)) + ' ' + version); // UTF-8

	dprintf(DEBUG_NORMAL, "get update (url): %s - %s\n", filename.c_str(), dest_name);
	
	return httpTool.downloadFile(filename, dest_name, 40 );
}

bool CFlashUpdate::checkVersion4Update()
{
	char msg[400];
	CFlashVersionInfo * versionInfo;
	neutrino_locale_t msg_body;

	dprintf(DEBUG_NORMAL, "[update] mode is %d\n", updateMode);

	if(updateMode == UPDATEMODE_INTERNET) //internet-update
	{
		// get image/package
		if(!selectHttpImage())
			return false;

		//showLocalStatus(100);
		showGlobalStatus(20);
		showStatusMessageUTF(g_Locale->getText(LOCALE_FLASHUPDATE_VERSIONCHECK) ); // UTF-8

		dprintf(DEBUG_NORMAL, "internet version: %s\n", newVersion.c_str());

		//showLocalStatus(100);
		showGlobalStatus(20);
		hide();
		
		msg_body = (fileType < '3')? LOCALE_FLASHUPDATE_FLASHMSGBOX : LOCALE_FLASHUPDATE_PACKAGEMSGBOX;
		
		versionInfo = new CFlashVersionInfo(newVersion);	//NOTE: Memory leak: versionInfo
		sprintf(msg, g_Locale->getText(msg_body), filename.c_str(), versionInfo->getDate(), versionInfo->getTime(), versionInfo->getReleaseCycle(), versionInfo->getType());

		// flash
		if(fileType < '3') 
		{
			// check release cycle
			if ((strncmp(RELEASE_CYCLE, versionInfo->getReleaseCycle(), 2) != 0) &&
			(MessageBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_FLASHUPDATE_WRONGBASE), CMessageBox::mbrYes, CMessageBox::mbYes | CMessageBox::mbNo, NEUTRINO_ICON_UPDATE) != CMessageBox::mbrYes))
			{
				delete versionInfo;
				return false;
			}

			// check if not release ask to install (beta + snapshot)
			if ((strcmp(RELEASE_TYPE, versionInfo->getType()) != 0) &&
		    	    (MessageBox(LOCALE_MESSAGEBOX_INFO, LOCALE_FLASHUPDATE_EXPERIMENTALIMAGE, CMessageBox::mbrYes, CMessageBox::mbYes | CMessageBox::mbNo, NEUTRINO_ICON_UPDATE) != CMessageBox::mbrYes))
			{
				delete versionInfo;
				return false;
			}
		}

		delete versionInfo;
	}
	else if(updateMode == UPDATEMODE_MANUAL)// manual update (ftp)
	{
		CFileBrowser UpdatesBrowser;
		CFileFilter UpdatesFilter; 
		
		if(allow_flash) 
			UpdatesFilter.addFilter(FILEBROWSER_UPDATE_FILTER);

		UpdatesFilter.addFilter("bin");
		UpdatesFilter.addFilter("tar");
		UpdatesFilter.addFilter("gz");
		UpdatesFilter.addFilter("txt");

		UpdatesBrowser.Filter = &UpdatesFilter;

		CFile * CFileSelected = NULL;
		
		if (!(UpdatesBrowser.exec(g_settings.update_dir)))
			return false;

		CFileSelected = UpdatesBrowser.getSelectedFile();

		if (CFileSelected == NULL)
			return false;

		filename = CFileSelected->Name;

		FILE * fd = fopen(filename.c_str(), "r");
		if(fd)
			fclose(fd);
		else 
		{
			hide();
			
			dprintf(DEBUG_NORMAL, "flash-file not found: %s\n", filename.c_str());
			
			HintBox(LOCALE_MESSAGEBOX_ERROR, g_Locale->getText(LOCALE_FLASHUPDATE_CANTOPENFILE)); // UTF-8
			return false;
		}
		
		hide();
		
		// just only for correct msg
		char * ptr = rindex(const_cast<char *>(filename.c_str()), '.');
		if(ptr) 
		{
			ptr++;

			if( (!strcmp(ptr, "bin")) || (!strcmp(ptr, "tar")) || (!strcmp(ptr, "gz"))) 
				fileType = 'A';
			else if(!strcmp(ptr, "txt")) 
				fileType = 'T';
			else if(!allow_flash) 
				return false;
			else 
				fileType = 0;

			dprintf(DEBUG_NORMAL, "[update] manual file type: %s %c\n", ptr, fileType);
		}

		strcpy(msg, g_Locale->getText( (fileType < '3')? LOCALE_FLASHUPDATE_SQUASHFS_NOVERSION : LOCALE_FLASHUPDATE_NOVERSION ));
		msg_body = (fileType < '3')? LOCALE_FLASHUPDATE_FLASHMSGBOX : LOCALE_FLASHUPDATE_PACKAGEMSGBOX;
	}
	
	return ( (fileType == 'T')? true : MessageBox(LOCALE_MESSAGEBOX_INFO, msg, CMessageBox::mbrYes, CMessageBox::mbYes | CMessageBox::mbNo, NEUTRINO_ICON_UPDATE) == CMessageBox::mbrYes); // UTF-8
}

int CFlashUpdate::exec(CMenuTarget * parent, const std::string &)
{
	dprintf(DEBUG_NORMAL, "CFlashUpdate::exec\n");

	if(parent)
		parent->hide();

	paint();

	if(!checkVersion4Update()) 
	{
		hide();
		return menu_return::RETURN_REPAINT;
	}

	// install
#ifdef LCD_UPDATE
	CVFD::getInstance()->showProgressBar2(0, "checking", 0, "Update Neutrino");
	CVFD::getInstance()->setMode(CLCD::MODE_PROGRESSBAR2);	
#endif // VFD_UPDATE

	showGlobalStatus(19);
	paint();
	showGlobalStatus(20);

	// check image version
	if(updateMode == UPDATEMODE_INTERNET) //internet-update
	{
		char * fname = rindex(const_cast<char *>(filename.c_str()), '/') +1;
		char fullname[255];

		if(!getUpdateImage(newVersion)) 
		{
			hide();
			HintBox(LOCALE_MESSAGEBOX_ERROR, g_Locale->getText(LOCALE_FLASHUPDATE_GETUPDATEFILEERROR)); // UTF-8
			return menu_return::RETURN_REPAINT;
		}
		
		sprintf(fullname, "%s/%s", g_settings.update_dir, fname);
		filename = std::string(fullname);
	}

	showGlobalStatus(40);

	CFlashTool ft;
	
	// flash image
	if(fileType < '3') 
	{
		ft.setMTDDevice(MTD_DEVICE_OF_UPDATE_PART);
		ft.setStatusViewer(this);
	}

	// MD5summ check
	showStatusMessageUTF(g_Locale->getText(LOCALE_FLASHUPDATE_MD5CHECK)); // UTF-8
	
	if((updateMode == UPDATEMODE_INTERNET) && !ft.check_md5(filename, file_md5)) 
	{
		// remove flash/package
		remove(filename.c_str());
		hide();
		HintBox(LOCALE_MESSAGEBOX_ERROR, g_Locale->getText( (fileType < '3') ? LOCALE_FLASHUPDATE_FLASHMD5SUMERROR : LOCALE_FLASHUPDATE_PACKAGEMD5SUMERROR)); // UTF-8
		return menu_return::RETURN_REPAINT;
	}
	
	// download or not???
	if(updateMode == UPDATEMODE_INTERNET) 
	{ 
		//internet-update
		if ( MessageBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText( (fileType < '3') ? LOCALE_FLASHUPDATE_DOWNLOADEDIMAGE : LOCALE_FLASHUPDATE_INSTALLPACKAGE ), CMessageBox::mbrYes, CMessageBox::mbYes | CMessageBox::mbNo, NEUTRINO_ICON_UPDATE) != CMessageBox::mbrYes) // UTF-8
		{
			// remove flash/package
			remove(filename.c_str());
			hide();
			return menu_return::RETURN_REPAINT;
		}
	}

	showGlobalStatus(60);

	// flash/install
	dprintf(DEBUG_NORMAL, "[update] filename %s type %c\n", filename.c_str(), fileType);

	// flash image
	if(fileType < '3') 
	{
		CNeutrinoApp::getInstance()->exec(NULL, "savesettings");
		sleep(2);
		
		// flash it...
		if(!ft.program(filename, 80, 100))
		{
			// remove flash if flashing failed
			remove(filename.c_str());
			hide();
			HintBox(LOCALE_MESSAGEBOX_ERROR, ft.getErrorMessage().c_str()); // UTF-8
			return menu_return::RETURN_REPAINT;
		}

		//status anzeigen
		showGlobalStatus(100);
		showStatusMessageUTF(g_Locale->getText(LOCALE_FLASHUPDATE_READY)); // UTF-8

		hide();

		// Unmount all NFS & CIFS volumes
		nfs_mounted_once = false; /* needed by update.cpp to prevent removal of modules after flashing a new cramfs, since rmmod (busybox) might no longer be available */
		CFSMounter::umount();

		HintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_FLASHUPDATE_FLASHREADYREBOOT)); // UTF-8
		
		ft.reboot();
		sleep(20000);
	}
	else if(fileType == 'T') // display file contents
	{
		FILE* fd = fopen(filename.c_str(), "r");
		if(fd) 
		{
			char * buffer;
			off_t filesize = lseek(fileno(fd), 0, SEEK_END);
			lseek(fileno(fd), 0, SEEK_SET);
			buffer =(char *) malloc(filesize+1);
			fread(buffer, filesize, 1, fd);
			fclose(fd);
			buffer[filesize] = 0;
			MessageBox(LOCALE_MESSAGEBOX_INFO, buffer, CMessageBox::mbrBack, CMessageBox::mbBack); // UTF-8
			free(buffer);
		}
	}
	else // package 
	{
		char cmd[100];
		const char install_sh[] = "install.sh";
		
		sprintf(cmd, "%s %s %s", install_sh, g_settings.update_dir, filename.c_str());

		if( system(cmd) )
		{
			hide();
			HintBox(LOCALE_MESSAGEBOX_ERROR, g_Locale->getText(LOCALE_FLASHUPDATE_INSTALLFAILED)); // UTF-8
			return menu_return::RETURN_REPAINT;
		}
		
		// 100% status
		showGlobalStatus(100);
		
		// show successfull msg :-)
		HintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_FLASHUPDATE_READY)); // UTF-8
	}
	
	hide();
	
	return menu_return::RETURN_REPAINT;
}

CFlashExpert::CFlashExpert()
	:CProgressWindow()
{
	selectedMTD = -1;
}

void CFlashExpert::readmtd(int _readmtd)
{
	char tmp[10];
	sprintf(tmp, "%d", _readmtd);
	std::string filename = "/tmp/mtd";
	filename += tmp;
	filename += ".img"; // US-ASCII (subset of UTF-8 and ISO8859-1)

	if (_readmtd == -1) 
	{
		filename = "/tmp/flashimage.img"; // US-ASCII (subset of UTF-8 and ISO8859-1)
		_readmtd = MTD_OF_WHOLE_IMAGE;
	}
	
	setTitle(LOCALE_FLASHUPDATE_TITLEREADFLASH);
	paint();
	showGlobalStatus(0);
	showStatusMessageUTF((std::string(g_Locale->getText(LOCALE_FLASHUPDATE_ACTIONREADFLASH)) + " (" + CMTDInfo::getInstance()->getMTDName(_readmtd) + ')')); // UTF-8
	CFlashTool ft;
	ft.setStatusViewer( this );
	ft.setMTDDevice(CMTDInfo::getInstance()->getMTDFileName(_readmtd));

	if(!ft.readFromMTD(filename, 100)) 
	{
		showStatusMessageUTF(ft.getErrorMessage()); // UTF-8
		sleep(10);
	} 
	else 
	{
		showGlobalStatus(100);
		showStatusMessageUTF(g_Locale->getText(LOCALE_FLASHUPDATE_READY)); // UTF-8
		char message[500];
		sprintf(message, g_Locale->getText(LOCALE_FLASHUPDATE_SAVESUCCESS), filename.c_str());
		sleep(1);
		hide();
		HintBox(LOCALE_MESSAGEBOX_INFO, message);
	}
}

void CFlashExpert::writemtd(const std::string & filename, int mtdNumber)
{
	char message[500];

	sprintf(message,
		g_Locale->getText(LOCALE_FLASHUPDATE_REALLYFLASHMTD), FILESYSTEM_ENCODING_TO_UTF8(std::string(filename).c_str()), CMTDInfo::getInstance()->getMTDName(mtdNumber).c_str());

	if (MessageBox(LOCALE_MESSAGEBOX_INFO, message, CMessageBox::mbrNo, CMessageBox::mbYes | CMessageBox::mbNo, NEUTRINO_ICON_UPDATE) != CMessageBox::mbrYes) // UTF-8
		return;

#ifdef ENABLE_LCD
        CVFD::getInstance()->showProgressBar2(0,"checking",0,"Update Neutrino");
        CVFD::getInstance()->setMode(CLCD::MODE_PROGRESSBAR2);	
#endif // VFD_UPDATE

	setTitle(LOCALE_FLASHUPDATE_TITLEWRITEFLASH);
	paint();
	showGlobalStatus(0);
	CFlashTool ft;
	ft.setStatusViewer( this );
	ft.setMTDDevice( CMTDInfo::getInstance()->getMTDFileName(mtdNumber) );

	if(!ft.program( "/tmp/" + filename, 50, 100)) 
	{
		showStatusMessageUTF(ft.getErrorMessage()); // UTF-8
		sleep(10);
	} 
	else 
	{
		showGlobalStatus(100);
		showStatusMessageUTF(g_Locale->getText(LOCALE_FLASHUPDATE_READY)); // UTF-8
		sleep(1);
		hide();
		HintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_FLASHUPDATE_FLASHREADYREBOOT)); // UTF-8
		ft.reboot();
	}
}

void CFlashExpert::showMTDSelector(const std::string & actionkey)
{
	//mtd-selector erzeugen
	CMenuWidget * mtdselector = new CMenuWidget(LOCALE_FLASHUPDATE_MTDSELECTOR, NEUTRINO_ICON_UPDATE);
	
	// intros
	mtdselector->addItem(new CMenuForwarder(LOCALE_MESSAGEBOX_CANCEL));
	mtdselector->addItem(new CMenuSeparator(CMenuSeparator::LINE));
	
	CMTDInfo* mtdInfo =CMTDInfo::getInstance();

	for(int x1 = 0; x1 < mtdInfo->getMTDCount(); x1++) 
	{
		char sActionKey[20];
		sprintf(sActionKey, "%s%d", actionkey.c_str(), x1);

		/* for Cuberevo family boxes */
		/*
		* dev:    size   erasesize  name
		* mtd0: 00040000 00020000 "nor.boot"
		* mtd1: 00020000 00020000 "nor.config_welcome"
		* mtd2: 00200000 00020000 "nor.kernel"
		* mtd3: 013a0000 00020000 "nor.root"
		* mtd4: 00a00000 00020000 "nor.db"
		* mtd5: 015a0000 00020000 "nor.kernel_root"
		* mtd6: 01fa0000 00020000 "nor.kernel_root_db"
		* mtd7: 01fc0000 00020000 "nor.all_noboot"
		* mtd8: 02000000 00020000 "nor.all"
		*/
		/* mtd0 to mtd4 are R_ONLY */
		/* mtd5-mtd8 are RW */
		/* we excluse mtd0(nor.boot) and mtd8(not.all) */
		
		/* giga */
		/*
		root@Giga:~# cat /proc/mtd
		dev:    size   erasesize  name
		mtd0: 07800000 00020000 "rootfs"
		mtd1: 07f00000 00020000 "all"
		mtd2: 00400000 00020000 "kernel"

		*/
		
		if(actionkey == "writemtd")
		{			  
			mtdselector->addItem(new CMenuForwarder(mtdInfo->getMTDName(x1).c_str(), true, NULL, this, sActionKey));
		}
		else if(actionkey == "readmtd")
		{
			mtdselector->addItem(new CMenuForwarder(mtdInfo->getMTDName(x1).c_str(), true, NULL, this, sActionKey));
		}
	}
	
	mtdselector->exec(NULL,"");
	delete mtdselector;
}

void CFlashExpert::showFileSelector(const std::string & actionkey)
{
	CMenuWidget * fileselector = new CMenuWidget(LOCALE_FLASHUPDATE_FILESELECTOR, NEUTRINO_ICON_UPDATE);
	
	// intros
	fileselector->addItem(new CMenuForwarder(LOCALE_MESSAGEBOX_CANCEL));
	fileselector->addItem(new CMenuSeparator(CMenuSeparator::LINE));

	struct dirent **namelist;
	int n = scandir("/tmp", &namelist, 0, alphasort);
	if (n < 0)
	{
		perror("no flashimages available");
		//should be available...
	}
	else
	{
		for(int count = 0; count < n; count++)
		{
			std::string filen = namelist[count]->d_name;
			int pos = filen.find(".img");
			if(pos != -1)
			{
				fileselector->addItem(new CMenuForwarder(filen.c_str(), true, NULL, this, (actionkey + filen).c_str()));
//#warning TODO: make sure file is UTF-8 encoded
			}
			free(namelist[count]);
		}
		free(namelist);
	}

	fileselector->exec(NULL,"");
	delete fileselector;
}

int CFlashExpert::exec(CMenuTarget* parent, const std::string & actionKey)
{
	if(parent)
		parent->hide();

	if(actionKey=="readflash") 
	{
		readmtd(-1);
	}
	else if(actionKey=="writeflash") 
	{
		showFileSelector("");
	}
	else if(actionKey=="readflashmtd") 
	{
		showMTDSelector("readmtd");
	}
	else if(actionKey=="writeflashmtd") 
	{
		showMTDSelector("writemtd");
	}
	else 
	{
		int iReadmtd = -1;
		int iWritemtd = -1;
		sscanf(actionKey.c_str(), "readmtd%d", &iReadmtd);
		sscanf(actionKey.c_str(), "writemtd%d", &iWritemtd);
		
		if(iReadmtd != -1) 
		{
			readmtd(iReadmtd);
		}
		else if(iWritemtd != -1) 
		{
			selectedMTD = iWritemtd;
			showFileSelector("");
		} 
		else 
		{
			if(selectedMTD == -1) 
			{
				writemtd(actionKey, MTD_OF_WHOLE_IMAGE);
			} 
			else 
			{
				writemtd(actionKey, selectedMTD);
				selectedMTD=-1;
			}
		}
		hide();
		return menu_return::RETURN_EXIT_ALL;
	}

	hide();
	
	return menu_return::RETURN_REPAINT;
}

//
CUpdateSettings::CUpdateSettings()
{
}

CUpdateSettings::~CUpdateSettings()
{
}

int CUpdateSettings::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CUpdateSettings::exec: actionKey:%s\n", actionKey.c_str());
	
	int ret = menu_return::RETURN_REPAINT;
	
	if(parent)
		parent->hide();
	
	if(actionKey == "savesettings")
	{
		CNeutrinoApp::getInstance()->exec(NULL, "savesettings");
		
		return ret;
	}
	else if (actionKey == "update_dir")
	{
		if(parent)
			parent->hide();
		
		CFileBrowser fileBrowser;
		fileBrowser.Dir_Mode = true;
		
		if (fileBrowser.exec(g_settings.update_dir) == true) 
		{
			const char * newdir = fileBrowser.getSelectedFile()->Name.c_str();
			if(check_dir(newdir))
				printf("CUpdateSettings::exec: Wrong/unsupported update dir %s\n", newdir);
			else
			{
				strcpy(g_settings.update_dir, fileBrowser.getSelectedFile()->Name.c_str());
				
				dprintf(DEBUG_NORMAL, "CNeutrinoApp::exec: new update dir %s\n", fileBrowser.getSelectedFile()->Name.c_str());
			}
		}
		
		return ret;
	}
	
	showMenu();
	
	return ret;
}

void CUpdateSettings::showMenu()
{
	dprintf(DEBUG_NORMAL, "CUpdateSettings::showMenu\n");
	
	CMenuWidget updateSettings(LOCALE_SERVICEMENU_UPDATE, NEUTRINO_ICON_UPDATE);
		
	// intros
	updateSettings.addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	updateSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	
	// save settings
	updateSettings.addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savesettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	updateSettings.addItem( new CMenuSeparator(CMenuSeparator::LINE) );

	// expert-function for mtd read/write
	CMenuWidget * mtdexpert = new CMenuWidget(LOCALE_FLASHUPDATE_EXPERTFUNCTIONS, NEUTRINO_ICON_UPDATE);
		
	// intros
	mtdexpert->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	mtdexpert->addItem(new CMenuSeparator(CMenuSeparator::LINE));
		
	CFlashExpert * fe = new CFlashExpert();

	// read mtd 
	mtdexpert->addItem(new CMenuForwarder(LOCALE_FLASHUPDATE_READFLASHMTD , true, NULL, fe, "readflashmtd" ));

	// write mtd
	mtdexpert->addItem(new CMenuForwarder(LOCALE_FLASHUPDATE_WRITEFLASHMTD, true, NULL, fe, "writeflashmtd"));

	// experten function
	//FIXME: allow update only when the rootfs is jffs2/squashfs
	updateSettings.addItem(new CMenuForwarder(LOCALE_FLASHUPDATE_EXPERTFUNCTIONS, true, NULL, mtdexpert, "", CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN));
	updateSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));
		
	// update dir
	updateSettings.addItem( new CMenuForwarder(LOCALE_EXTRA_UPDATE_DIR, true, g_settings.update_dir , this, "update_dir") );
	
	// url
	CStringInputSMS * updateSettings_url_file = new CStringInputSMS(LOCALE_FLASHUPDATE_URL_FILE, g_settings.softupdate_url_file);
	updateSettings.addItem(new CMenuForwarder(LOCALE_FLASHUPDATE_URL_FILE, true, g_settings.softupdate_url_file, updateSettings_url_file));

	// show current version
	updateSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE | CMenuSeparator::STRING, LOCALE_FLASHUPDATE_CURRENTVERSION_SEP));

	// get current version SBBB YYYY MM TT HH MM -- formatsting
	//CConfigFile lconfigfile('\t');

	//const char * versionString = (lconfigfile.loadConfig("/etc/.version")) ? (lconfigfile.getString( "version", "1201201205091849").c_str()) : "1201201602031021";

	//dprintf(DEBUG_INFO, "CNeutrinoApp::InitServiceSettings: current flash-version: %s\n", versionString);

	//static CFlashVersionInfo versionInfo(versionString);

	// release cycle
	updateSettings.addItem(new CMenuForwarder(LOCALE_FLASHUPDATE_CURRENTRELEASECYCLE, false, /*versionInfo.getReleaseCycle()*/ RELEASE_CYCLE));
		
	// date
	updateSettings.addItem(new CMenuForwarder(LOCALE_FLASHUPDATE_CURRENTVERSIONDATE, false, /*versionInfo.getDate()*/ __DATE__ ));
		
	// time
	updateSettings.addItem(new CMenuForwarder(LOCALE_FLASHUPDATE_CURRENTVERSIONTIME, false, /*versionInfo.getTime()*/ __TIME__));
		
	// type
	// versionInfo.getType() returns const char * which is never deallocated
	updateSettings.addItem(new CMenuForwarder(LOCALE_FLASHUPDATE_CURRENTVERSIONSNAPSHOT, false, /*versionInfo.getType()*/ "Snapshot" ));

	// check update
	//FIXME: allow update only when the rootfs is jffs2/squashfs
	updateSettings.addItem(new CMenuSeparator(CMenuSeparator::LINE));
	
	// offline
	updateSettings.addItem(new CMenuForwarder(LOCALE_FLASHUPDATE_UPDATEMODE_MANUAL, false, NULL, new CFlashUpdate(CFlashUpdate::UPDATEMODE_MANUAL), "", CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW));

	// online
	updateSettings.addItem(new CMenuForwarder(LOCALE_FLASHUPDATE_UPDATEMODE_INTERNET, false, NULL, new CFlashUpdate(CFlashUpdate::UPDATEMODE_INTERNET), "", CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE));
	
	updateSettings.exec(NULL, "");
	updateSettings.hide();
	
	delete fe;
	delete mtdexpert;
}

