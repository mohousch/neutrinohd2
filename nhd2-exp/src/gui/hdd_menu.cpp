/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: hdd_menu.cpp 2013/10/12 mohousch Exp $

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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/mount.h>

#include <global.h>
#include <neutrino.h>
#include <gui/widget/icons.h>
#include "gui/widget/menue.h"
#include "gui/widget/stringinput.h"
#include "gui/widget/messagebox.h"
#include "gui/widget/hintbox.h"
#include "gui/widget/progresswindow.h"
#include <gui/widget/infobox.h>

#include "system/setting_helpers.h"
#include "system/settings.h"
#include "system/debug.h"
#include <system/helpers.h>

#include <driver/audioplay.h>
#include <pictureviewer.h>
#include <movieplayer.h>

#include <gui/audioplayer.h>
#include <gui/hdd_menu.h>

#include <blkid/blkid.h>
#include <mntent.h>

#include <system/tmdbparser.h>


#define HDD_NOISE_OPTION_COUNT 4
const CMenuOptionChooser::keyval HDD_NOISE_OPTIONS[HDD_NOISE_OPTION_COUNT] =
{
	{ 0,   LOCALE_OPTIONS_OFF, NULL },
	{ 128, LOCALE_HDD_SLOW, NULL },
	{ 190, LOCALE_HDD_MIDDLE, NULL },
	{ 254, LOCALE_HDD_FAST, NULL }
};

#define HDD_SLEEP_OPTION_COUNT 7
const CMenuOptionChooser::keyval HDD_SLEEP_OPTIONS[HDD_SLEEP_OPTION_COUNT] =
{
	{0, LOCALE_OPTIONS_OFF, NULL},
	{12, LOCALE_HDD_1MIN, NULL},
	{60, LOCALE_HDD_5MIN, NULL},
	{120, LOCALE_HDD_10MIN, NULL},
	{240, LOCALE_HDD_20MIN, NULL},
	{241, LOCALE_HDD_30MIN, NULL},
	{242, LOCALE_HDD_60MIN, NULL}
};

static int my_filter(const struct dirent * dent)
{
	if(dent->d_name[0] == 's' && dent->d_name[1] == 'd')
		return 1;
	
	return 0;
}

/* return 1 if mounted and 0 if not mounted */
static int check_if_mounted(char * dev)
{
	char buffer[255];
	FILE *f = fopen("/proc/mounts", "r");
	
	if(f != NULL)
	{
		while (fgets (buffer, 255, f) != NULL) 
		{
			if(strstr(buffer, dev)) 
			{
				fclose(f);
				return 1;
			}
		}
		fclose(f);
	}

	return 0;
}

int CHDDMenuHandler::exec(CMenuTarget * parent, const std::string &actionKey)
{
	dprintf(DEBUG_NORMAL, "CHDDMenuHandler::exec:\n");

	if (parent)
		parent->hide();
	
	if(actionKey == "savehddsettings") 
	{
		//CNeutrinoApp::getInstance()->saveSetup(NEUTRINO_SETTINGS_FILE);
		CNeutrinoApp::getInstance()->exec(NULL, "savesettings");
		
		return menu_return::RETURN_REPAINT;
	}
	else if(actionKey == "activateNow")
	{
		CHintBox * hintBox = new CHintBox(LOCALE_MESSAGEBOX_INFO, g_Locale->getText(LOCALE_MAINSETTINGS_SAVESETTINGSNOW_HINT)); // UTF-8
		hintBox->paint();
		
		CHDDDestExec * hddActiv = new CHDDDestExec();
		hddActiv->exec(NULL, "");
	
		delete hddActiv;
		
		hintBox->hide();
		delete hintBox;
		
		return menu_return::RETURN_REPAINT;
	}
	
	hddMenu();
	
	return menu_return::RETURN_REPAINT;
}

int CHDDMenuHandler::hddMenu()
{
	FILE * f;
	int fd;
	struct dirent **namelist;
	int ret;
	
	struct stat s;
	int root_dev = -1;
	bool hdd_found = 0;

	int n = scandir("/sys/block", &namelist, my_filter, alphasort);

	if (n < 0) 
	{
                perror("CHDDMenuHandler::doMenu: scandir(\"/sys/block\") failed");

                return menu_return::RETURN_REPAINT;
        }

	CMenuWidget * hddmenu = new CMenuWidget(LOCALE_HDD_SETTINGS, NEUTRINO_ICON_SETTINGS);
	
	hddmenu->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
	hddmenu->addItem(new CMenuSeparator(CMenuSeparator::LINE));
	
	// save settings
	hddmenu->addItem(new CMenuForwarder(LOCALE_MAINSETTINGS_SAVESETTINGSNOW, true, NULL, this, "savehddsettings", CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
	hddmenu->addItem( new CMenuSeparator(CMenuSeparator::LINE) );
	
	// activate settings
	hddmenu->addItem(new CMenuForwarder(LOCALE_HDD_ACTIVATE, true, NULL, this, "activateNow", CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN));

	// sleep time
	hddmenu->addItem( new CMenuOptionChooser(LOCALE_HDD_SLEEP, &g_settings.hdd_sleep, HDD_SLEEP_OPTIONS, HDD_SLEEP_OPTION_COUNT, true, NULL, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW, true));
	
	// noise
	hddmenu->addItem( new CMenuOptionChooser(LOCALE_HDD_NOISE, &g_settings.hdd_noise, HDD_NOISE_OPTIONS, HDD_NOISE_OPTION_COUNT, true, NULL, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE, true ));
	
	// HDDs
	hddmenu->addItem(new CMenuSeparator(CMenuSeparator::LINE));
	
	ret = stat("/", &s);
	int drive_mask = 0xfff0;
	if (ret != -1) 
	{
		if ((s.st_dev & drive_mask) == 0x0300) /* hda, hdb,... has max 63 partitions */
			drive_mask = 0xffc0; /* hda: 0x0300, hdb: 0x0340, sda: 0x0800, sdb: 0x0810 */
		root_dev = (s.st_dev & drive_mask);
	}
	printf("HDD: root_dev: 0x%04x\n", root_dev);
	
	//hdd manage
	CMenuWidget * tempMenu[n];

	for(int i = 0; i < n; i++) 
	{
		char str[256];
		char vendor[128];
		char model[128];
		int64_t bytes;
		int64_t megabytes;
		int removable = 0;
		bool isroot = false;

		printf("[neutrino] HDD: checking /sys/block/%s\n", namelist[i]->d_name);
		
		sprintf(str, "/dev/%s", namelist[i]->d_name);
		fd = open(str, O_RDONLY);

		if(fd < 0) 
		{
			printf("[neutrino] HDD: Cant open %s\n", str);
			continue;
		}
		
		if (ioctl(fd, BLKGETSIZE64, &bytes))
			perror("BLKGETSIZE64");
		
		//
		ret = fstat(fd, &s);
		if (ret != -1) 
		{
			if ((int)(s.st_rdev & drive_mask) == root_dev) 
			{
				isroot = true;
			}
		}
                
                close(fd);

		megabytes = bytes/1000000;

		// vendor
		sprintf(str, "/sys/block/%s/device/vendor", namelist[i]->d_name);
		f = fopen(str, "r");

		if(!f) 
		{
			printf("[neutrino] HDD: Cant open %s\n", str);
			continue;
		}
		fscanf(f, "%s", vendor);
		fclose(f);

		// model
		sprintf(str, "/sys/block/%s/device/model", namelist[i]->d_name);
		f = fopen(str, "r");
		
		if(!f) 
		{
			printf("[neutrino] HDD: Cant open %s\n", str);
			continue;
		}
		fscanf(f, "%s", model);
		fclose(f);

		// removable
		sprintf(str, "/sys/block/%s/removable", namelist[i]->d_name);
		f = fopen(str, "r");
		
		if(!f) 
		{
			printf("[neutrino] HDD: Cant open %s\n", str);
			continue;
		}
		fscanf(f, "%d", &removable);
		fclose(f);

		sprintf(str, "%s (%s-%s %lld %s)", namelist[i]->d_name, vendor, model, megabytes < 10000 ? megabytes : megabytes/1000, megabytes < 10000 ? "MB" : "GB");
		
		bool enabled = !CNeutrinoApp::getInstance()->recordingstatus && !isroot;

		/* hdd menu */
		tempMenu[i] = new CMenuWidget(str, NEUTRINO_ICON_SETTINGS);
		tempMenu[i]->enableSaveScreen(true);
		
		tempMenu[i]->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
		tempMenu[i]->addItem(new CMenuSeparator(CMenuSeparator::LINE));
		
		//init hdd	
		tempMenu[i]->addItem(new CMenuForwarder(LOCALE_HDD_INIT, enabled, "", new CHDDInit, namelist[i]->d_name, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
		tempMenu[i]->addItem(new CMenuSeparator(CMenuSeparator::LINE));
		
		/* check for parts */
		#define MAX_PARTS 4
		char DEVICE[256];
		char PART[256];
		
		CMenuWidget * PartMenu[MAX_PARTS];
		
		for (int j = 1; j<= MAX_PARTS; j++)
		{
			bool mounted = false;
			
			memset(PART, 0, sizeof(PART));
			memset(DEVICE, 0, sizeof(DEVICE));
			
			sprintf(PART, "%s%d", namelist[i]->d_name, j);
			sprintf(DEVICE, "/dev/%s%d", namelist[i]->d_name, j);
			
			fd = open(DEVICE, O_RDONLY);

			if( fd < 0) 
			{
				//printf("[neutrino] HDD: %s not exist\n", DEVICE);
				close(fd);
				continue;
			}
			
			// check if mounted
			mounted = check_if_mounted(DEVICE);
			
			/* part submenu */
			PartMenu[j] = new CMenuWidget(PART, NEUTRINO_ICON_SETTINGS);
			PartMenu[j]->enableSaveScreen(true);
			

			PartMenu[j]->addItem(new CMenuForwarder(LOCALE_MENU_BACK, true, NULL, NULL, NULL, CRCInput::RC_nokey, NEUTRINO_ICON_BUTTON_LEFT));
			PartMenu[j]->addItem(new CMenuSeparator(CMenuSeparator::LINE));
			
			/* format part */
			PartMenu[j]->addItem(new CMenuForwarder(LOCALE_HDD_FORMAT, true, NULL, new CHDDFmtExec, PART, CRCInput::RC_red, NEUTRINO_ICON_BUTTON_RED));
			
			/* fs check */
			PartMenu[j]->addItem(new CMenuForwarder(LOCALE_HDD_CHECK, true, NULL, new CHDDChkExec, PART, CRCInput::RC_green, NEUTRINO_ICON_BUTTON_GREEN));
			
			/* mount part */
			PartMenu[j]->addItem(new CMenuForwarder(LOCALE_HDD_MOUNT, true, NULL, new CHDDMountMSGExec, PART, CRCInput::RC_yellow, NEUTRINO_ICON_BUTTON_YELLOW));

			/* umount part */
			PartMenu[j]->addItem(new CMenuForwarder(LOCALE_HDD_UMOUNT, true, NULL, new CHDDuMountMSGExec, PART, CRCInput::RC_blue, NEUTRINO_ICON_BUTTON_BLUE));
			
			// hdd explorer
			PartMenu[j]->addItem(new CMenuSeparator(CMenuSeparator::LINE));
			PartMenu[j]->addItem(new CMenuForwarder(LOCALE_HDD_BROWSER, mounted, NULL, new CHDDBrowser(), DEVICE));
			
			// part
			tempMenu[i]->addItem(new CMenuForwarder(PART, true, mounted? g_Locale->getText(LOCALE_HDD_MOUNTED) : g_Locale->getText(LOCALE_HDD_UMOUNTED), PartMenu[j]));
			
			close(fd);
		}
		/* END check for Parts */
		
		//hddmenu->addItem(new CMenuSeparator(CMenuSeparator::LINE));
		hddmenu->addItem(new CMenuForwarder(str, enabled, NULL, tempMenu[i]));

		/* result */
		hdd_found = 1;
		
		// free
		free(namelist[i]);
	}
	
	if (n >= 0)
                free(namelist);
	
	/* no parts found */
	ret = hddmenu->exec(NULL, "");
	
	// delet temp menus
	/*
	for(int i = 0; i < n; i++) 
	{     
                if( hdd_found && tempMenu[i] != NULL )
		{
                        delete tempMenu[i];
                }
        }
	*/
	
	delete hddmenu;

	return ret;
}

// hdd init
int CHDDInit::exec(CMenuTarget * /*parent*/, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CHDDInit::exec: actionKey:%s\n", actionKey.c_str());

	char cmd[100];
	CHintBox * hintbox;
	int res;
	FILE * f;
	const char *dst = NULL;
	char src[128];
	CProgressWindow * progress;
	bool idone;
	
	sprintf(src, "/dev/%s", actionKey.c_str());

	res = MessageBox( LOCALE_HDD_FORMAT, g_Locale->getText(LOCALE_HDD_INIT_WARN), CMessageBox::mbrNo, CMessageBox::mbYes | CMessageBox::mbNo );

	if(res != CMessageBox::mbrYes)
		return 0;
	
	f = fopen("/proc/sys/kernel/hotplug", "w");
	if(f) 
	{
		fprintf(f, "none\n");
		fclose(f);
	}
	
	// find mount point
	FILE * fstab = setmntent("/etc/mtab", "r");
	struct mntent *e;

	while ((e = getmntent(fstab))) 
	{
		if (strcmp(src, e->mnt_fsname) == 0) 
		{
			dst = e->mnt_dir;
			break;
		}
	}
		
	printf("mount point is %s\n", dst);
	endmntent(fstab);
		
	// umount /media/sda%
	res = umount(dst);
	printf("CHDDuMountExec: umount res %d\n", res);
	
	progress = new CProgressWindow();
	progress->setTitle(LOCALE_HDD_INIT);
	progress->exec(NULL, "");
	progress->showStatusMessageUTF("HDD init");
	progress->showGlobalStatus(0);
	
	const char init[]= "init_hdd.sh";
	sprintf(cmd, "%s /dev/%s > /tmp/%s.txt", init, actionKey.c_str(), init);
	printf("CHDDInit: executing %s\n", cmd);
	
	system(cmd);
	
	// check if cmd executed
	char buffer[255];
	sprintf(buffer, "/tmp/%s.txt", init);
	
	FILE * output = fopen(buffer, "r");

	if (!output) 
	{
		hintbox = new CHintBox(LOCALE_HDD_INIT, g_Locale->getText(LOCALE_HDD_INIT_FAILED));
		hintbox->paint();
		sleep(2);
		delete hintbox;
		return menu_return::RETURN_REPAINT;
	}
	
	char buf[256];
	idone = false;
	while(fgets(buf, sizeof(buf), output) != NULL)
	{
		printf("%s", buf);
                if(!idone && strncmp(buf, "Building a new DOS disklabel.", 29)) 
		{
			idone = true;
			buf[21] = 0;
			progress->showGlobalStatus(20);
                        progress->showStatusMessageUTF(buf);
                } 
		else if(strncmp(buf, "Command (m for help): Command action", 36)) 
		{
			progress->showGlobalStatus(40);
                        progress->showStatusMessageUTF(buf);
		}
		else if(strncmp(buf, "The partition table", 19)) 
		{
			progress->showGlobalStatus(60);
                        progress->showStatusMessageUTF(buf);
		}
	}
	
	fclose(output);

	progress->showGlobalStatus(100);
	sleep(2);
	
	unlink(buffer);
	
	progress->hide();
	delete progress;

	return menu_return::RETURN_REPAINT;
}

int CHDDDestExec::exec(CMenuTarget * /*parent*/, const std::string&)
{
	dprintf(DEBUG_NORMAL, "CHDDDestExec::exec:\n");

        char cmd[100];
	char str[256];
	FILE * f;
	int removable = 0;
        struct dirent **namelist;

        int n = scandir("/sys/block", &namelist, my_filter, alphasort);

        if (n < 0)
                return 0;
	
	const char hdparm[] = "/sbin/hdparm";
	bool hdparm_link = false;
	struct stat stat_buf;
	
	if(::lstat(hdparm, &stat_buf) == 0)
		if( S_ISLNK(stat_buf.st_mode) )
			hdparm_link = true;

        for (int i = 0; i < n; i++) 
	{
                removable = 0;
		printf("CHDDDestExec: checking /sys/block/%s\n", namelist[i]->d_name);
 
		sprintf(str, "/sys/block/%s/removable", namelist[i]->d_name);
		f = fopen(str, "r");
		if (!f)
		{
			printf("Can't open %s\n", str);
			continue;
		}
		fscanf(f, "%d", &removable);
		fclose(f);
		
		if (removable)   // show USB icon, no need for hdparm
		{
#if defined(PLATFORM_KATHREIN) || defined(PLATFORM_SPARK7162)
			CVFD::getInstance()->ShowIcon(VFD_ICON_USB, true);
#endif
			printf("CHDDDestExec: /dev/%s is not a hdd, no sleep needed\n", namelist[i]->d_name);

		} 
		else 
		{
			//show HDD icon and set hdparm for all hdd's
#if defined(PLATFORM_KATHREIN)
			CVFD::getInstance()->ShowIcon(VFD_ICON_HDD, true);
#endif
	                printf("CHDDDestExec: noise %d sleep %d /dev/%s\n", g_settings.hdd_noise, g_settings.hdd_sleep, namelist[i]->d_name);

			if(hdparm_link)
			{
				//hdparm -M is not included in busybox hdparm!
	        	        snprintf(cmd, sizeof(cmd), "%s -S%d /dev/%s >/dev/null 2>/dev/null &", hdparm, g_settings.hdd_sleep, namelist[i]->d_name);
			}
			else
			{
	        	        snprintf(cmd, sizeof(cmd), "%s -M%d -S%d /dev/%s >/dev/null 2>/dev/null &", hdparm, g_settings.hdd_noise, g_settings.hdd_sleep, namelist[i]->d_name);
			}
			
        	        system(cmd);
		}

                free(namelist[i]);
        }

        free(namelist);

        return 1;
}

int CHDDFmtExec::exec(CMenuTarget *parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CHDDFmtExec::exec: actionKey:%s\n", actionKey.c_str());

	if(parent)
		hide();
	
	char cmd[100];
	CHintBox * hintbox;
	int res;
	FILE * f;
	char src[128];
	const char * dst = NULL;
	bool mountPoint = false;
	
	CProgressWindow *progress;
	bool idone;

	sprintf(src, "/dev/%s", actionKey.c_str());

	res = MessageBox(LOCALE_HDD_FORMAT, g_Locale->getText(LOCALE_HDD_FORMAT_WARN), CMessageBox::mbrNo, CMessageBox::mbYes | CMessageBox::mbNo );

	if(res != CMessageBox::mbrYes)
		return 0;

	bool srun = system("killall -9 smbd");

	/* check if mounted then umount */
	if(check_if_mounted(src) == 1)
	{
		// find mount point
		FILE * fstab = setmntent("/etc/mtab", "r");
		struct mntent *e;

		while ((e = getmntent(fstab))) 
		{
			if (strcmp(src, e->mnt_fsname) == 0) 
			{
				dst = e->mnt_dir;
				mountPoint = true;
				break;
			}
		}
		printf("mount point is %s\n", dst);
		endmntent(fstab);
		
		// can not parse mtab, fallback to /media/sda%
		if(!mountPoint)
			sprintf((char *)dst, "/media/%s", actionKey.c_str());
		
		/* umount /media/sda%n */
		res = umount(dst);
		
		if(res == -1) 
		{
			// not mounted to /media/sda%n, fallback to /hdd
			strcpy((char *)dst, "/hdd");
			res = umount(dst);
			
			if(res == -1)
			{
				hintbox = new CHintBox(LOCALE_HDD_FORMAT, g_Locale->getText(LOCALE_HDD_UMOUNT_WARN));
				hintbox->paint();
				sleep(2);
				delete hintbox;
				return menu_return::RETURN_EXIT;
			}
		}
	}

	// check hotplug
	f = fopen("/proc/sys/kernel/hotplug", "w");
	if(f) 
	{
		fprintf(f, "none\n");
		fclose(f);
	}

	progress = new CProgressWindow();
	progress->setTitle(LOCALE_HDD_FORMAT);
	progress->exec(NULL,"");
	progress->showStatusMessageUTF("Executing fdisk");
	progress->showGlobalStatus(0);
	
	//format part ext3
	const char fmt[] = "mkfs.ext3";
	//NOTE: on some arch popen is not working, so dump output of system to /tmp
	sprintf(cmd, "%s -T largefile -m0 %s > /tmp/%s.txt", fmt, src, fmt);

	printf("CHDDFmtExec: executing %s\n", cmd);
	
	system(cmd);
	
	// check if cmd executed
	char buffer[255];
	sprintf(buffer, "/tmp/%s.txt", fmt);
	
	FILE * output = fopen(buffer, "r");
	
	if (!output) 
	{
		hintbox = new CHintBox(LOCALE_HDD_INIT, g_Locale->getText(LOCALE_HDD_FORMAT_FAILED));
		hintbox->paint();
		sleep(2);
		delete hintbox;
		return menu_return::RETURN_EXIT;
	}

	char buf[256];
	idone = false;
	while(fgets(buf, sizeof(buf), output) != NULL)
	{
		printf("%s", buf);
                if(!idone && strncmp(buf, "Writing inode", 13)) 
		{
			idone = true;
			buf[21] = 0;
			progress->showGlobalStatus(20);
                        progress->showStatusMessageUTF(buf);
                } 
		else if(strncmp(buf, "Creating", 8)) 
		{
			progress->showGlobalStatus(40);
                        progress->showStatusMessageUTF(buf);
		}
		else if(strncmp(buf, "Writing superblocks", 19)) 
		{
			progress->showGlobalStatus(60);
                        progress->showStatusMessageUTF(buf);
		}
	}
	
	fclose(output);

	progress->showGlobalStatus(100);
	sleep(2);

	sprintf(cmd, "tune2fs -r 0 -c 0 -i 0 %s", src);
	printf("CHDDFmtExec: executing %s\n", cmd);
	system(cmd);
	
	unlink(buffer);

//_remount:
	progress->hide();
	delete progress;

	// mount
	res = mount(src, dst, "ext3", 0, NULL);

	f = fopen("/proc/sys/kernel/hotplug", "w");
	if(f) 
	{
		fprintf(f, "/sbin/hotplug\n");
		fclose(f);
	}

	// create directories
	if(!res) 
	{
		sprintf(cmd, "%s/record", dst);
		safe_mkdir((char *) cmd);
		sprintf(cmd, "%s/movie", dst);
		safe_mkdir((char *) cmd);
		sprintf(cmd, "%s/picture", dst);
		safe_mkdir((char *) cmd);
		sprintf(cmd, "%s/epg", dst);
		safe_mkdir((char *) cmd);
		sprintf(cmd, "%s/music", dst);
		safe_mkdir((char *) cmd);
		sprintf(cmd, "%s/backup", dst);
		safe_mkdir((char *) cmd);
		sync();
	}
	
//_return:
	if(!srun) 
		system("smbd");

	return menu_return::RETURN_EXIT;
}

int CHDDChkExec::exec(CMenuTarget *parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CHDDChkExec: actionKey %s\n", actionKey.c_str());

	if(parent)
		hide();
	
	char cmd[100];
	CHintBox * hintbox;
	int res;
	char src[128];
	const char * dst = NULL;
	const char * fstype = NULL;
	
	CProgressWindow *progress;
	int oldpass = 0, pass, step, total;
	int percent = 0, opercent = 0;

	sprintf(src, "/dev/%s", actionKey.c_str());

	bool srun = system("killall -9 smbd");

	// check if mounted
	if(check_if_mounted(src) == 1)
	{
		// find mount point
		FILE * fstab = setmntent("/etc/mtab", "r");
		struct mntent *e;

		while ((e = getmntent(fstab))) 
		{
			if (strcmp(src, e->mnt_fsname) == 0) 
			{
				dst = e->mnt_dir;
				fstype = e->mnt_type;
				//mountPoint = true;
				break;
			}
		}
		
		printf("mount point is %s type %s\n", dst, fstype);
		endmntent(fstab);
		
		// unmout /media/sda%
		res = umount(dst);
		
		// not mounted to /media/sda% fallback to /hhd
		if(res == -1)
		{
			dprintf(DEBUG_NORMAL, "CHDDChkExec::exec: can not umount %s\n", dst);
				
			hintbox = new CHintBox(LOCALE_HDD_CHECK, g_Locale->getText(LOCALE_HDD_CHECK_FAILED));
			hintbox->paint();
			sleep(2);
			delete hintbox;
			return menu_return::RETURN_EXIT;
		}
	}

	// not mounted process directly check
	const char fsck[] = "fsck.ext3";
	sprintf(cmd, "%s -C 1 -f -y %s > /tmp/%s.txt", fsck, src, fsck);

	dprintf(DEBUG_NORMAL, "CHDDChkExec: Executing %s\n", cmd);
	
	my_system(cmd);
	
	// check if cmd executed
	char buffer[255];
	sprintf(buffer, "/tmp/%s.txt", fsck);
	
	FILE * output = fopen(buffer, "r");
	
	if (!output) 
	{
		hintbox = new CHintBox(LOCALE_HDD_INIT, g_Locale->getText(LOCALE_HDD_CHECK_FAILED));
		hintbox->paint();
		sleep(2);
		delete hintbox;
		return menu_return::RETURN_EXIT;
	}

	progress = new CProgressWindow();
	progress->setTitle(LOCALE_HDD_CHECK);
	progress->exec(NULL, "");
	progress->showGlobalStatus(20);

	char buf[256];
	while(fgets(buf, sizeof(buf), output) != NULL)
	{
		if(isdigit(buf[0])) 
		{
			sscanf(buf, "%d %d %d\n", &pass, &step, &total);
			if(total == 0)
				total = 1;
			if(oldpass != pass) 
			{
				oldpass = pass;
				progress->showGlobalStatus(pass > 0 ? (pass-1)*20: 0);
			}
			percent = (step * 100) / total;
			if(opercent != percent) 
			{
				opercent = percent;
				//printf("CHDDChkExec: pass %d : %d\n", pass, percent);
				//progress->showLocalStatus(percent);
			}
		}
		else if(!strncmp(buf, "Pass", 4))
			progress->showStatusMessageUTF(buf);
	}
	
	fclose(output);

	progress->showGlobalStatus(100);
	progress->showStatusMessageUTF(buf);
	
	sleep(2);
	progress->hide();
	delete progress;
	
	unlink(buffer);

	// mount
	res = mount(src, dst, fstype, 0, NULL);

	dprintf(DEBUG_NORMAL, "CHDDChkExec: mount res %d\n", res);
	
	if(res < 0)
	{
		hintbox = new CHintBox(LOCALE_HDD_MOUNT, g_Locale->getText(LOCALE_HDD_MOUNTFAILED));
		hintbox->paint();
		sleep(2);
		delete hintbox;
	}
	
	if(!srun) 
		system("smbd");
	
	return menu_return::RETURN_EXIT;
}

int CHDDMountMSGExec::exec(CMenuTarget *parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CHDDMountMSGExec: %s\n", (char *)actionKey.c_str());

	if(parent)
		hide();
	
	CHintBox * hintbox;
	int res;
	char src[128];
	char dst[128];
	const char * fstype = NULL;

	sprintf(src, "/dev/%s", actionKey.c_str());
	sprintf(dst, "/media/%s", actionKey.c_str());

	res = check_if_mounted((char *)actionKey.c_str());

	/* if mounted */
	if(res == 1) 
	{
		hintbox = new CHintBox(LOCALE_HDD_MOUNT, g_Locale->getText(LOCALE_HDD_MOUNTED));
		hintbox->paint();
		sleep(2);
		delete hintbox;
		return menu_return::RETURN_EXIT;
	}
	else
	{
		/* get fs type */
		fstype = blkid_get_tag_value(NULL, "TYPE", src);
		
		dprintf(DEBUG_NORMAL, "fstype: %s\n", fstype);

		if(fstype != NULL)
		{
			//mount to /hdd
			res = mount(src, "/hdd", fstype, 0, NULL);
	
			dprintf(DEBUG_NORMAL, "CHDDMountExec: mount1 to /hdd res %d\n", res);

			if(res == 0) 
			{
				hintbox = new CHintBox(LOCALE_HDD_MOUNT, g_Locale->getText(LOCALE_HDD_MOUNTED));
				hintbox->paint();
				sleep(2);
				delete hintbox;
				
				return menu_return::RETURN_EXIT_ALL;
			}
			else
			{
				// mount to /media/sda%
				res = mount(src, dst, fstype, 0, NULL);
	
				dprintf(DEBUG_NORMAL, "CHDDMountExec: mount2 to %s res %d\n", dst, res);
				
				if(res == 0)
				{
					hintbox = new CHintBox(LOCALE_HDD_MOUNT, g_Locale->getText(LOCALE_HDD_MOUNTED));
					hintbox->paint();
					sleep(2);
					delete hintbox;
					
					return menu_return::RETURN_EXIT_ALL;
				}
				else //fallback to /tmp/hdd
				{
					// create /tmp/hdd
					safe_mkdir((char *)"/tmp/hdd");
					// mount to /tmp/hdd
					res = mount(src, "/tmp/hdd", fstype, 0, NULL);
					
					dprintf(DEBUG_NORMAL, "CHDDMountExec: mount3 to /tmp/hdd res %d\n", res);
					
					if(res == 0)
					{
						hintbox = new CHintBox(LOCALE_HDD_MOUNT, g_Locale->getText(LOCALE_HDD_MOUNTED));
						hintbox->paint();
						sleep(2);
						delete hintbox;
						
						return menu_return::RETURN_EXIT_ALL;
					}
					else
					{
						hintbox = new CHintBox(LOCALE_HDD_MOUNT, g_Locale->getText(LOCALE_HDD_MOUNTFAILED));
						hintbox->paint();
						sleep(2);
						delete hintbox;
						return menu_return::RETURN_EXIT;
					}
				}
			}
		}
		else // can not get fstype
		{
			hintbox = new CHintBox(LOCALE_HDD_MOUNT, g_Locale->getText(LOCALE_HDD_MOUNTFAILED));
			hintbox->paint();
			sleep(2);
			delete hintbox;
			return menu_return::RETURN_EXIT;
		}
	}
	
	return menu_return::RETURN_EXIT_ALL;
}

int CHDDMountExec::exec(CMenuTarget *parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CHDDMountExec: %s\n", actionKey.c_str());

	if(parent)
		hide();
	
	int res;
	char src[128];
	char dst[128];
	const char * fstype = NULL;

	sprintf(src, "/dev/%s", actionKey.c_str());
	sprintf(dst, "/media/%s", actionKey.c_str());

	res = check_if_mounted((char *)actionKey.c_str());

	// if mounted
	if(res == 1) 
	{
		return 0;
	}
	else
	{
		// get fs type
		fstype = blkid_get_tag_value(NULL, "TYPE", src);

		if(fstype != NULL)
		{
			//mount to /hdd
			res = mount(src, "/hdd", fstype, 0, NULL);
	
			dprintf(DEBUG_NORMAL, "CHDDMountExec: mount1 to /hdd res %d\n", res);

			if(res == 0) 
			{
				return 0;
			}
			else
			{
				// mount to /media/sda%
				res = mount(src, dst, fstype, 0, NULL);
	
				dprintf(DEBUG_NORMAL, "CHDDMountExec: mount2 to %s res %d\n", dst, res);
				
				if(res == 0)
				{
					return 0;
				}
				else //fallback to /tmp/hdd
				{
					// create /tmp/hdd
					safe_mkdir((char *)"/tmp/hdd");
					// mount to /tmp/hdd
					res = mount(src, "/tmp/hdd", fstype, 0, NULL);
					
					dprintf(DEBUG_NORMAL, "CHDDMountExec: mount3 to /tmp/hdd res %d\n", res);
					
					if(res == 0)
					{
						return 0;
					}
					else
					{
						return -1;
					}
				}
			}
		}
		else // can not get fstype
		{
			return -1;
		}
	}
	
	return -1;
}

int CHDDuMountMSGExec::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CHDDuMountExec: actionKey:%s\n", actionKey.c_str());

	if(parent)
		hide();
	
	CHintBox * hintbox;
	int res;
	char src[128];
	//char dst[128];
	const char * dst = NULL;

	sprintf(src, "/dev/%s", actionKey.c_str());

	/* umount */
	if(check_if_mounted(src) == 1)
	{
		// find mount point
		FILE * fstab = setmntent("/etc/mtab", "r");
		struct mntent *e;

		while ((e = getmntent(fstab))) 
		{
			if (strcmp(src, e->mnt_fsname) == 0) 
			{
				dst = e->mnt_dir;
				break;
			}
		}
		
		dprintf(DEBUG_NORMAL, "mount point is %s\n", dst);
		endmntent(fstab);
		
		// umount /media/sda%
		res = umount(dst);
		dprintf(DEBUG_NORMAL, "CHDDuMountExec: umount res %d\n", res);

		if(res == 0)	/* umounted */
		{
			hintbox = new CHintBox(LOCALE_HDD_MOUNT, g_Locale->getText(LOCALE_HDD_UMOUNTED));
			hintbox->paint();
			sleep(2);
			delete hintbox;
			
			return menu_return::RETURN_EXIT;
		}
		else
		{
			hintbox = new CHintBox(LOCALE_HDD_MOUNT, g_Locale->getText(LOCALE_HDD_UMOUNT_WARN));
			hintbox->paint();
			sleep(2);
			delete hintbox;
			return menu_return::RETURN_EXIT;

		}
	}

	// not mounted
	hintbox = new CHintBox(LOCALE_HDD_MOUNT, g_Locale->getText(LOCALE_HDD_UMOUNTED));
	hintbox->paint();
	sleep(2);
	delete hintbox;
	
	return menu_return::RETURN_EXIT;
}

// hdd browser
int CHDDBrowser::exec(CMenuTarget * parent, const std::string& actionKey)
{
	printf("CHDDBrowser: key %s\n", actionKey.c_str());
	
	if(parent)
		parent->hide();
	
	CFileBrowser filebrowser;
	CHintBox * hintbox;
	//char dst[128];
	const char * dst = NULL;

	// find mount point
	FILE * fstab = setmntent("/etc/mtab", "r");
	struct mntent * e;

	while ((e = getmntent(fstab))) 
	{
		if (strcmp(actionKey.c_str(), e->mnt_fsname) == 0) 
		{
			dst = e->mnt_dir;
			break;
		}
	}
		
	printf("mount point is %s\n", dst);
	endmntent(fstab);
	
	if(dst != NULL)
	{
REPEAT:	  
		if( filebrowser.exec(dst))
		{
			dst = filebrowser.getCurrentDir().c_str(); // remark path
			neutrino_msg_t msg;
			neutrino_msg_data_t data;
			
			// get the current file name
			CFile * file;

			if ((file = filebrowser.getSelectedFile()) != NULL) 
			{
				// parse file extension
				if(file->getType() == CFile::FILE_PICTURE)
				{
					CPictureViewerGui tmpPictureViewerGui;
					CPicture pic;
					struct stat statbuf;
					
					pic.Filename = file->Name;
					std::string tmp = file->Name.substr(file->Name.rfind('/') + 1);
					pic.Name = tmp.substr(0, tmp.rfind('.'));
					pic.Type = tmp.substr(tmp.rfind('.') + 1);
					
					if(stat(pic.Filename.c_str(), &statbuf) != 0)
						printf("stat error");
					pic.Date = statbuf.st_mtime;
									
					tmpPictureViewerGui.addToPlaylist(pic);
					tmpPictureViewerGui.exec(NULL, "urlplayback");	
				}
				else if(file->getType() == CFile::FILE_VIDEO)
				{
					CMovieInfo cMovieInfo;
					MI_MOVIE_INFO mfile;
					CMoviePlayerGui tmpMoviePlayerGui;
	 
					cMovieInfo.clearMovieInfo(&mfile);
					mfile.file.Name = file->Name;

					// other infos if there is xml file
					cMovieInfo.loadMovieInfo(&mfile, file);

					// epgTitle
					if(mfile.epgTitle.empty())
					{
						std::string Title = file->getFileName();
						removeExtension(Title);
						mfile.epgTitle = Title;
					}

					// tfile
					std:string fname = file->Name;
					changeFileNameExt(fname, ".jpg");
					if(!access(fname.c_str(), F_OK) )
						mfile.tfile = fname.c_str();

					if(g_settings.prefer_tmdb_info)
					{
						cTmdb * tmdb = new cTmdb(mfile.epgTitle);
	
						fname = "/tmp/" + mfile.epgTitle + ".jpg";
				
						if (tmdb->getBigCover(fname)) 
						{
							if(!access(fname.c_str(), F_OK) )
								mfile.tfile = fname.c_str();
						}

						// epgInfo1
						if((mfile.epgInfo1.empty() && mfile.epgInfo2.empty()) && !tmdb->getDescription().empty())
						{
							mfile.epgInfo2 = tmdb->getDescription();
						}

						delete tmdb;
						tmdb = NULL;
					}

					tmpMoviePlayerGui.addToPlaylist(mfile);
					tmpMoviePlayerGui.exec(NULL, "urlplayback");
				}
				else if(file->getType() == CFile::FILE_AUDIO)
				{
					CAudioPlayerGui tmpAudioPlayerGui;
			
					CAudiofileExt audiofile(file->Name, file->getExtension());
					tmpAudioPlayerGui.addToPlaylist(audiofile);
					tmpAudioPlayerGui.exec(NULL, "urlplayback");
				}
				else
				{
					std::string buffer;
					buffer.clear();
				
					char buf[6000];
					bool result = true;

					int fd = open(file->Name.c_str(), O_RDONLY);
					int bytes = read(fd, buf, 6000 - 1);
					close(fd);
					buf[bytes] = 0;
					buffer = buf;

					CBox position(g_settings.screen_StartX + 50, g_settings.screen_StartY + 50, g_settings.screen_EndX - g_settings.screen_StartX - 100, g_settings.screen_EndY - g_settings.screen_StartY - 100); 
					
					CInfoBox * infoBox = new CInfoBox(file->getFileName().c_str(), g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1], CTextBox::SCROLL, &position, file->getFileName().c_str(), g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE], NEUTRINO_ICON_FILE);
					infoBox->setText(&buffer);
					infoBox->exec();
					delete infoBox;
				}
			}
			
			g_RCInput->getMsg_ms(&msg, &data, 10);
			
			if (msg != CRCInput::RC_home) 
			{
				goto REPEAT;
			}
		}
	}
	else
	{
		hintbox = new CHintBox(LOCALE_HDD_MOUNT, g_Locale->getText(LOCALE_HDD_UMOUNTED));
		hintbox->paint();
		sleep(2);
		delete hintbox;
	}
	
	return menu_return::RETURN_EXIT;
}

