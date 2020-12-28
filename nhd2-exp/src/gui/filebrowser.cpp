/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: filebrowser.cpp 2018/08/24 mohousch Exp $

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

#include <unistd.h>

/* include <config.h> before <gui/filebrowser.h> to enable 64 bit file offsets */
#include <gui/filebrowser.h>

#include <gui/widget/widget_helpers.h>
#include <gui/widget/icons.h>
#include <gui/widget/messagebox.h>

#include <gui/widget/hintbox.h>

#include <driver/encoding.h>

#include <algorithm>
#include <iostream>
#include <cctype>

#include <global.h>
#include <neutrino.h>

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sstream>

#include <sys/stat.h>

#include <driver/encoding.h>

#include <xmlinterface.h>
#include <system/debug.h>

#if defined (__USE_FILE_OFFSET64) || defined (_DARWIN_USE_64_BIT_INODE)
typedef struct dirent64 dirent_struct;
#define my_alphasort alphasort64
#define my_scandir scandir64
typedef struct stat64 stat_struct;
#define my_stat stat64
#define my_lstat lstat64
#else
typedef struct dirent dirent_struct;
#define my_alphasort alphasort
#define my_scandir scandir
typedef struct stat stat_struct;
#define my_stat stat
#define my_lstat lstat
#error not using 64 bit file offsets
#endif


#define FILEBROWSER_SMSKEY_TIMEOUT 2000

bool comparetolower(const char a, const char b)
{
	return tolower(a) < tolower(b);
};

// sort operators
bool sortByName (const CFile& a, const CFile& b)
{
	if (std::lexicographical_compare(a.Name.begin(), a.Name.end(), b.Name.begin(), b.Name.end(), comparetolower))
		return true;

	if (std::lexicographical_compare(b.Name.begin(), b.Name.end(), a.Name.begin(), a.Name.end(), comparetolower))
		return false;

	return a.Mode < b.Mode;
}

// Sorts alphabetically with Directories first
bool sortByNameDirsFirst(const CFile& a, const CFile& b)
{
	int typea, typeb;
	typea = a.getType();
	typeb = b.getType();

	if (typea == CFile::FILE_DIR)
		if (typeb == CFile::FILE_DIR)
			//both directories
			return sortByName(a, b);
		else
			//only a is directory
			return true;
	else if (typeb == CFile::FILE_DIR)
		//only b is directory
		return false;
	else
		//no directory
		return sortByName(a, b);
}

bool sortByType (const CFile& a, const CFile& b)
{
	if(a.Mode == b.Mode)
		return sortByName(a, b);
	else
		return a.Mode < b.Mode;
}

bool sortByDate (const CFile& a, const CFile& b)
{
	if(a.getFileName() == "..")
		return true;
	if(b.getFileName() == "..")
		return false;
	return a.Time < b.Time ;
}

bool sortBySize (const CFile& a, const CFile& b)
{
	if(a.getFileName()=="..")
		return true;
	if(b.getFileName()=="..")
		return false;
	return a.Size < b.Size;
}

bool (* const sortBy[FILEBROWSER_NUMBER_OF_SORT_VARIANTS])(const CFile& a, const CFile& b) =
{
	&sortByName,
	&sortByNameDirsFirst,
	&sortByType,
	&sortByDate,
	&sortBySize
};

const neutrino_locale_t sortByNames[FILEBROWSER_NUMBER_OF_SORT_VARIANTS] =
{
	LOCALE_FILEBROWSER_SORT_NAME,
	LOCALE_FILEBROWSER_SORT_NAMEDIRSFIRST,
	LOCALE_FILEBROWSER_SORT_TYPE,
	LOCALE_FILEBROWSER_SORT_DATE,
	LOCALE_FILEBROWSER_SORT_SIZE
};

CFileBrowser::CFileBrowser()
{
	commonInit();
	base = "";
}

CFileBrowser::CFileBrowser(const char * const _base)
{
	commonInit();
	base = _base;
}

void CFileBrowser::commonInit()
{
	frameBuffer = CFrameBuffer::getInstance();

	Filter = NULL;
	use_filter = true;
	Multi_Select = false;
	Dirs_Selectable = false;
	Dir_Mode = false;
	Hide_records = false;
	
	selected = 0;
	
	selections.clear();
	selected_filelist.clear();
	filelist.clear();

	// sms key input timeout
	m_SMSKeyInput.setTimeout(FILEBROWSER_SMSKEY_TIMEOUT);

	listBox = NULL;
	item = NULL;

	// box	
	cFrameBox.iWidth = g_settings.screen_EndX - g_settings.screen_StartX - 40;
	cFrameBox.iHeight = g_settings.screen_EndY - g_settings.screen_StartY - 40;
	
	cFrameBox.iX = frameBuffer->getScreenX() + (frameBuffer->getScreenWidth() - cFrameBox.iWidth) / 2;
	cFrameBox.iY = frameBuffer->getScreenY() + (frameBuffer->getScreenHeight() - cFrameBox.iHeight) / 2;
}

CFileBrowser::~CFileBrowser()
{
}

CFile * CFileBrowser::getSelectedFile()
{
	if(exit_pressed)
		return NULL;
	else
	{
		if ((!(filelist.empty())) && (!(filelist[selected].Name.empty())))
			return &filelist[selected];
		else
			return NULL;
	}
}

void CFileBrowser::ChangeDir(const std::string& filename, int selection)
{
	dprintf(DEBUG_INFO, "CFileBrowser::ChangeDir %s\n", filename.c_str());

	std::string newpath;
	
	if((filename == ".."))
	{
		std::string::size_type pos = Path.substr(0, Path.length()-1).rfind('/');

		if (pos == std::string::npos)
		{
			newpath = Path;
		}
		else
		{
			newpath = Path.substr(0, pos + 1);
		}

		if (strncmp(newpath.c_str(), base.c_str(), base.length()) != 0)
			return;
	}
	else
	{
		newpath = filename;
	}
	
	if((newpath.rfind('/') != newpath.length()- 1 || newpath.length() == 0))
	{
		newpath += '/';
	}
	
	Path = newpath;
	name = newpath;
	
	CFileList allfiles;
	
	readDir(newpath, &allfiles);

	filelist.clear();
	
	// filter
	CFileList::iterator file = allfiles.begin();
	for(; file != allfiles.end() ; file++)
	{
		if(Filter != NULL && (!S_ISDIR(file->Mode)) && use_filter)
		{
			if(!Filter->matchFilter(file->Name))
			{
				continue;
			}
			
			// ts filter
			if(Hide_records) 
			{
				int ext_pos = file->Name.rfind('.');
				if( ext_pos > 0) 
				{
					std::string extension = file->Name.substr(ext_pos + 1, name.length() - ext_pos);
					if(strcasecmp(extension.c_str(), "ts") == 0) 
					{
						std::string fname = file->Name.substr(0, ext_pos) + ".xml";
						if(access(fname.c_str(), F_OK) == 0)
							continue;
					}
				}
			}
		}
		
		if(Dir_Mode && (!S_ISDIR(file->Mode)))
		{
			continue;
		}
		
		filelist.push_back(*file);
	}
	
	// sort result
	sort(filelist.begin(), filelist.end(), sortBy[g_settings.filebrowser_sortmethod]);

	selected = 0;
	if ((selection != -1) && (selection < (int)filelist.size()))
		selected = selection;
}

bool CFileBrowser::readDir(const std::string& dirname, CFileList* flist)
{
	dprintf(DEBUG_NORMAL, "CFileBrowser::readDir %s\n", dirname.c_str());
	
	stat_struct statbuf;
	dirent_struct **namelist;
	int n;

	n = my_scandir(dirname.c_str(), &namelist, 0, my_alphasort);

	if (n < 0)
	{
		perror(("Filebrowser scandir: " + dirname).c_str());
		return false;
	}
	
	for(int i = 0; i < n; i++)
	{
		CFile file;
		if(strcmp(namelist[i]->d_name, ".") != 0)
		{
			// name
			file.Name = dirname + namelist[i]->d_name;

			// stat
			if(my_stat((file.Name).c_str(),&statbuf) != 0)
				perror("stat error");
			else
			{
				file.Mode = statbuf.st_mode;
				file.Size = statbuf.st_size;
				file.Time = statbuf.st_mtime;
				
				flist->push_back(file);
			}
		}
		free(namelist[i]);
	}

	free(namelist);

	return true;
}

bool CFileBrowser::exec(const char * const dirname)
{
	dprintf(DEBUG_NORMAL, "CFileBrowser::exec:\n");

	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	bool res = false;
	exit_pressed = false;

	// create listBox
	listBox = new ClistBox(&cFrameBox);

	listBox->initFrames();
	//listBox->enableSaveScreen();
	//listBox->enableShrinkMenu();
	listBox->enableCenterPos();

	name = dirname;
	std::replace(name.begin(), name.end(), '\\', '/');
	
	int selection = -1;
	
	if (name == Path)
		selection = selected;
		
	ChangeDir(name, selection);

	paint();
	frameBuffer->blit();

	int oldselected = selected;

	uint64_t timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_FILEBROWSER]);

	bool loop = true;
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );
		neutrino_msg_t msg_repeatok = msg & ~RC_Repeat;

		if ( msg <= RC_MaxRC )
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_FILEBROWSER]);

		if(!CRCInput::isNumeric(msg))
		{
			m_SMSKeyInput.resetOldKey();
		}

		if (msg == RC_yellow)
		{
			selected = listBox->getSelected();

			if ((Multi_Select) && (selected < filelist.size()))
			{
				if(filelist[selected].getFileName() != "..")
				{
					if( (S_ISDIR(filelist[selected].Mode) && Dirs_Selectable) || !S_ISDIR(filelist[selected].Mode) )
					{
						filelist[selected].Marked = !filelist[selected].Marked;

						//listBox->clearItems();
						paint();
					}
				}
				msg_repeatok = RC_down;	// jump to next item
			}
		}

		if ((msg == RC_red) || msg == RC_page_down)
		{
			listBox->scrollPageDown();
		}
		else if ((msg == RC_green) || (msg == RC_page_up) )
		{
			listBox->scrollPageUp();
		}
		else if (msg_repeatok == RC_up)
		{
			listBox->scrollLineUp();
		}
		else if (msg_repeatok == RC_down)
		{
			listBox->scrollLineDown();
		}
		else if ( ( msg == RC_timeout ) )
		{
			selected = oldselected;
			exit_pressed = true; //FIXME:
			loop = false;
		}
		else if ( msg == RC_right )
		{
			selected = listBox->getSelected();

			if (!(filelist.empty()))
			{
				if (S_ISDIR(filelist[selected].Mode))
				{
	 				if (filelist[selected].getFileName() != "..") 
					{
						selections.push_back(selected);
						ChangeDir(filelist[selected].Name);
						//hide();
						//listBox->clearItems();
						paint();
					}
				}
			}
		}
		else if ( msg == RC_left )
		{
			if (selections.size() > 0)
			{
				ChangeDir("..", selections.back());
				selections.pop_back();
				//hide();
				//listBox->clearItems();
				paint();
			} 
			else
			{
				ChangeDir("..");
				//hide();
				//listBox->clearItems();
				paint();
			}

			if (!(filelist.empty()))	
			{
				//listBox->clearItems();
				paint();
			}
		}
		else if ( msg == RC_blue )
		{
			if(Filter != NULL)
			{
				use_filter = !use_filter;

				ChangeDir(Path);
				//hide();
				//listBox->clearItems();
				paint();
			}
		}
		else if ( msg == RC_home )
		{
			selected = -1;
			exit_pressed = true;
			loop = false;
		}
		else if ( msg == RC_spkr)
		{
			selected = listBox->getSelected();

			if(".." !=(filelist[selected].getFileName().substr(0, 2))) // do not delete that
			{
				std::stringstream _msg;
				_msg << g_Locale->getText(LOCALE_FILEBROWSER_DODELETE1) << " ";
				if (filelist[selected].getFileName().length() > 25)
				{
					_msg << filelist[selected].getFileName().substr(0, 25) << "...";
				}
				else
					_msg << filelist[selected].getFileName();

				_msg << " " << g_Locale->getText(LOCALE_FILEBROWSER_DODELETE2);
				if (MessageBox(LOCALE_FILEBROWSER_DELETE, _msg.str().c_str(), mbrNo, mbYes|mbNo) == mbrYes)
				{
					recursiveDelete(filelist[selected].Name.c_str());
					
					if(".ts" ==(filelist[selected].getFileName().substr(filelist[selected].getFileName().length() - 3, filelist[selected].getFileName().length())))//if bla.ts
					{
						recursiveDelete((filelist[selected].Name.substr(0,filelist[selected].Name.length()-7)+".xml").c_str());//remove bla.xml von bla.ts
					}
					ChangeDir(Path);
					//hide();
					//listBox->clearItems();
					paint();
				}
			}
		}
		else if (msg == RC_ok)
		{
			selected = listBox->getSelected();

			if (!(filelist.empty()))
			{
				if (filelist[selected].getFileName() == "..")
				{
					if (selections.size() > 0)
					{
						ChangeDir("..", selections.back());
						selections.pop_back();
						//hide();
						//listBox->clearItems();
						paint();
					} 
					else
					{
						std::string::size_type pos = Path.substr(0, Path.length()-1).rfind('/');
						if (pos != std::string::npos) 
						{
							ChangeDir("..");
							//hide();
							//listBox->clearItems();
							paint();
						}
						else 
						{
							loop = false;
							res = true;
							filelist[selected].Name = "/";
						}
					}
				}
				else
				{
					std::string filename = filelist[selected].Name;
					if ( filename.length() > 1 )
					{
						if((!Multi_Select) && S_ISDIR(filelist[selected].Mode) && !Dir_Mode)
						{
							selections.push_back(selected);
							ChangeDir(filelist[selected].Name);
							//hide();
							//listBox->clearItems();
							paint();
						}
						else
						{
							filelist[selected].Marked = true;
							loop = false;
							res = true;
						}
					}
				}
			}
		}
		else if ( msg == RC_info )
		{
			if (++g_settings.filebrowser_sortmethod >= FILEBROWSER_NUMBER_OF_SORT_VARIANTS)
				g_settings.filebrowser_sortmethod = 0;

			sort(filelist.begin(), filelist.end(), sortBy[g_settings.filebrowser_sortmethod]);

			//listBox->clearItems();
			paint();
		}
		else if (CRCInput::isNumeric(msg_repeatok))
		{
			if (!(filelist.empty()))
				SMSInput(msg_repeatok);
		}
		else
		{
			if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
			{
				loop = false;
			}
		}
	
		frameBuffer->blit();	
	}

	hide();

	selected_filelist.clear();

	if(res && Multi_Select)
	{
		for(unsigned int i = 0; i < filelist.size(); i++)
		{
			if(filelist[i].Marked)
			{
				if(S_ISDIR(filelist[i].Mode)) 
				{
					addRecursiveDir(&selected_filelist, filelist[i].Name, true);
				} 
				else
					selected_filelist.push_back(filelist[i]);
			}
		}
	}

	delete listBox;
	listBox = NULL;

	return res;
}

void CFileBrowser::addRecursiveDir(CFileList * re_filelist, std::string rpath, bool bRootCall)
{
	neutrino_msg_t      msg;
	neutrino_msg_data_t data;

	int n;

	dprintf(DEBUG_INFO, "CFileBrowser::addRecursiveDir %s\n", rpath.c_str());

	if (bRootCall) 
		bCancel = false;

	g_RCInput->getMsg_us(&msg, &data, 1);

	if (msg == RC_home)
	{
		// home key cancel scan
		bCancel = true;
	}
	else if (msg != RC_timeout)
	{
		// other event, save to low priority queue
		g_RCInput->postMsg( msg, data, false );
	}
	
	if(bCancel)
		return;

	if ( ((rpath.empty()) || ((*rpath.rbegin()) != '/')))
	{
		rpath += '/';
	}

	CFileList tmplist;
	if(!readDir(rpath, &tmplist))
	{
		perror(("Recursive scandir: " + rpath).c_str());
	}
	else
	{
		n = tmplist.size();
		
		for(int i = 0; i < n; i++)
		{
			std::string basename = tmplist[i].Name.substr(tmplist[i].Name.rfind('/')+1);
			
			if( basename != ".." )
			{
				if(Filter != NULL && (!S_ISDIR(tmplist[i].Mode)) && use_filter)
				{
					if(!Filter->matchFilter(tmplist[i].Name))
					{
						continue;
					}
				}
				if(!S_ISDIR(tmplist[i].Mode))
					re_filelist->push_back(tmplist[i]);
				else
					addRecursiveDir(re_filelist,tmplist[i].Name, false);
			}
		}
	}
}

void CFileBrowser::hide()
{
	dprintf(DEBUG_NORMAL, "CFileBrowser::hide:\n");

	frameBuffer->paintBackgroundBoxRel(cFrameBox.iX, cFrameBox.iY, cFrameBox.iWidth, cFrameBox.iHeight);
	listBox->hide();
}

const struct button_label FileBrowserButtons[4] =
{
	{ NEUTRINO_ICON_BUTTON_RED, LOCALE_FILEBROWSER_NEXTPAGE, NULL },
	{ NEUTRINO_ICON_BUTTON_GREEN, LOCALE_FILEBROWSER_PREVPAGE, NULL },
	{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_FILEBROWSER_MARK, NULL },
	{ NEUTRINO_ICON_BUTTON_BLUE, LOCALE_FILEBROWSER_FILTER_INACTIVE, NULL },
};

const struct button_label HButtons[2] =
{
	{ NEUTRINO_ICON_BUTTON_MUTE_SMALL, NONEXISTANT_LOCALE, NULL },
	{ NEUTRINO_ICON_BUTTON_HELP, NONEXISTANT_LOCALE, NULL },
};

void CFileBrowser::paint()
{
	dprintf(DEBUG_NORMAL, "CFileBrowser::paint:\n");

	listBox->clearItems();

	for (unsigned int count = 0; count < filelist.size(); count++)
	{
		item = new ClistBoxItem(FILESYSTEM_ENCODING_TO_UTF8(std::string(filelist[count].getFileName()).c_str()));

		// marked
		if (filelist[count].Marked)
		{
			item->setMarked(true);
		}

		// iconName
		std::string fileicon = "";

		switch(filelist[count].getType())
		{
			case CFile::FILE_AUDIO:
				fileicon = NEUTRINO_ICON_MP3;
				break;

			case CFile::FILE_DIR:
				fileicon = NEUTRINO_ICON_FOLDER;
				break;

			case CFile::FILE_PICTURE:
				fileicon = NEUTRINO_ICON_PICTURE;
				break;
				
			case CFile::FILE_VIDEO:
				fileicon = NEUTRINO_ICON_MOVIE;
				break;
					
			case CFile::FILE_TEXT:
			default:
				fileicon = NEUTRINO_ICON_FILE;
		}

		item->setIconName(fileicon.c_str());

		std::string tmp;

		//
		if( S_ISREG(filelist[count].Mode) )
		{
			if (g_settings.filebrowser_showrights != 0)
			{
				const char * attribute = "xwr";
				char modestring[9 + 1];

				for (int m = 8; m >= 0; m--)
				{
					modestring[8 - m] = (filelist[count].Mode & (1 << m)) ? attribute[m % 3] : '-';
				}

				modestring[9] = 0;

				tmp = modestring;
			}

#define GIGABYTE 1073741824LL
#define MEGABYTE 1048576LL
#define KILOBYTE 1024LL
			char tmpstr[256];
			const char *unit = "";
			long long factor = 0;

			if (filelist[count].Size >= GIGABYTE)
			{
				factor = GIGABYTE;
				unit = "G";
			}
			else if (filelist[count].Size >= MEGABYTE)
			{
				factor = MEGABYTE;
				unit = "M";
			}
			else if (filelist[count].Size >= KILOBYTE)
			{
				factor = KILOBYTE;
				unit = "k";
			}

			if (factor)
			{
				int a = filelist[count].Size / factor;
				int b = (filelist[count].Size - a * factor) * 1000 / factor;
				snprintf(tmpstr, sizeof(tmpstr), "%d.%03d%s", a, b, unit);
			}
			else
				snprintf(tmpstr,sizeof(tmpstr),"%d", (int)filelist[count].Size);

			tmp += " ";
			tmp += tmpstr;

			if( S_ISDIR(filelist[count].Mode) )
			{
				char timestring[18];
				time_t rawtime;

				rawtime = filelist[count].Time;
				strftime(timestring, 18, "%d-%m-%Y %H:%M", gmtime(&rawtime));

				tmp = timestring;
			}
		}

		item->setOptionInfo(tmp.c_str()); 

		listBox->addItem(item);
	}

	// head
	char l_name[100];
	snprintf(l_name, sizeof(l_name), "%s %s", g_Locale->getText(LOCALE_FILEBROWSER_HEAD), FILESYSTEM_ENCODING_TO_UTF8(std::string(name).c_str())); // UTF-8

	listBox->enablePaintHead();
	listBox->setTitle(l_name);
	listBox->enablePaintDate();
	listBox->setHeaderButtons(HButtons, 2);

	// foot
	listBox->enablePaintFoot();

	struct button_label Button[4];
	Button[0] = FileBrowserButtons[0];
	Button[1] = FileBrowserButtons[1];

	Button[2].button = Multi_Select? NEUTRINO_ICON_BUTTON_YELLOW : NULL;
	Button[2].locale = Multi_Select?LOCALE_FILEBROWSER_MARK : NONEXISTANT_LOCALE;
	Button[2].localename = NULL;

	Button[3].button = (Filter != NULL)? NEUTRINO_ICON_BUTTON_BLUE : NULL;
	Button[3].locale = (Filter != NULL)? (use_filter)?LOCALE_FILEBROWSER_FILTER_INACTIVE : LOCALE_FILEBROWSER_FILTER_ACTIVE : NONEXISTANT_LOCALE;
	Button[3].localename = NULL;

	listBox->setFooterButtons(Button, 4);

	//
	listBox->setSelected(selected);
	listBox->paint();
}

void CFileBrowser::SMSInput(const neutrino_msg_t msg)
{
	unsigned char key = m_SMSKeyInput.handleMsg(msg);

	unsigned int i;
	for(i = (selected + 1) % filelist.size(); i != selected ; i= (i + 1) % filelist.size())
	{
		if(tolower(filelist[i].getFileName()[0]) == key)
		{
			break;
		}
	}

	//listBox->clearItems();
	paint();
}

void CFileBrowser::recursiveDelete(const char *file)
{
	stat_struct statbuf;
	dirent_struct **namelist;
	int n;
	
	dprintf(DEBUG_INFO, "CFileBrowser::Delete %s\n", file);
	
	if(my_lstat(file, &statbuf) == 0)
	{
		if(S_ISDIR(statbuf.st_mode))
		{
			n = my_scandir(file, &namelist, 0, my_alphasort);
			printf("CFileBrowser::Delete: n:%d\n", n);

			if(n > 0)
			{
				while(n--)
				{
					if(strcmp(namelist[n]->d_name, ".") != 0 && strcmp(namelist[n]->d_name, "..") != 0)
					{
						std::string fullname = (std::string)file + "/" + namelist[n]->d_name;
						recursiveDelete(fullname.c_str());
					}
					free(namelist[n]);
				}

				free(namelist);

				rmdir(file);
			}
		}
		else
		{
			unlink(file);
		}
	}
	else
		perror(file);
}
