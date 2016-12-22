/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: filebrowser.cpp 2013/10/12 mohousch Exp $

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

#include <gui/widget/buttons.h>
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

	x = g_settings.screen_StartX + 20;
	y = g_settings.screen_StartY + 20;

	width = (g_settings.screen_EndX - g_settings.screen_StartX - 40);
	height = (g_settings.screen_EndY - g_settings.screen_StartY - 40);

	theight = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_TITLE]->getHeight();
	fheight = g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->getHeight();
	//foheight = 30;
	int ih, iw;
	frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_OKAY, &iw, &ih);
	foheight = 2*(std::max(g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight(), ih)) + 10;

	liststart = 0;
	listmaxshow = (height - theight - foheight)/fheight;

	//recalc height
	height = theight + listmaxshow*fheight + foheight;

	// sms key input timeout
	m_SMSKeyInput.setTimeout(FILEBROWSER_SMSKEY_TIMEOUT);
}

CFileBrowser::~CFileBrowser()
{
	/*
	if(!selections.empty())
		selections.clear();
	
	if(!selected_filelist.empty())
		selected_filelist.clear();
	
	if(!filelist.empty())
		filelist.clear();
	*/
}

CFile * CFileBrowser::getSelectedFile()
{
	if ((!(filelist.empty())) && (!(filelist[selected].Name.empty())))
		return &filelist[selected];
	else
		return NULL;
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
	
	if((newpath.rfind('/') != newpath.length()-1 || newpath.length() == 0))
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
	
	paintHead();
	paint();
}

bool CFileBrowser::readDir(const std::string& dirname, CFileList* flist)
{
	dprintf(DEBUG_INFO, "CFileBrowser::readDir %s\n", dirname.c_str());
	
	stat_struct statbuf;
	dirent_struct **namelist;
	int n;

	n = my_scandir(dirname.c_str(), &namelist, 0, my_alphasort);
	if (n < 0)
	{
		perror(("Filebrowser scandir: " + dirname).c_str());
		return false;
	}
	
	for(int i = 0; i < n;i++)
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

	name = dirname;
	std::replace(name.begin(), name.end(), '\\', '/');
	
	int selection = -1;
	
	if (name == Path)
		selection = selected;
		
	ChangeDir(name, selection);

	paintHead();
	paintFoot();
		
	frameBuffer->blit();

	int oldselected = selected;

	unsigned long long timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_FILEBROWSER]);

	bool loop = true;
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );
		neutrino_msg_t msg_repeatok = msg & ~CRCInput::RC_Repeat;

		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = CRCInput::calcTimeoutEnd(g_settings.timing[SNeutrinoSettings::TIMING_FILEBROWSER]);

		if(!CRCInput::isNumeric(msg))
		{
			m_SMSKeyInput.resetOldKey();
		}

		if (msg == CRCInput::RC_yellow)
		{
			if ((Multi_Select) && (selected < filelist.size()))
			{
				if(filelist[selected].getFileName() != "..")
				{
					if( (S_ISDIR(filelist[selected].Mode) && Dirs_Selectable) || !S_ISDIR(filelist[selected].Mode) )
					{
						filelist[selected].Marked = !filelist[selected].Marked;
						paintItem(selected - liststart);
					}
				}
				msg_repeatok = CRCInput::RC_down;	// jump to next item
			}
		}

		if ((msg == CRCInput::RC_red) || msg == CRCInput::RC_page_down)
		{
			selected += listmaxshow;
			if (selected >= filelist.size()) {
				if (((filelist.size() / listmaxshow) + 1) * listmaxshow == filelist.size() + listmaxshow) // last page has full entries
					selected = 0;
				else
					selected = selected < (((filelist.size() / listmaxshow) + 1) * listmaxshow) ? (filelist.size() - 1) : 0;
			}
			liststart = (selected / listmaxshow) * listmaxshow;
			paint();
		}
		else if ((msg == CRCInput::RC_green) || (msg == CRCInput::RC_page_up) )
		{
			if ((int(selected) - int(listmaxshow)) < 0)
				selected = filelist.size()-1;
			else
				selected -= listmaxshow;

			liststart = (selected/listmaxshow)*listmaxshow;
			paint();
		}
		else if (msg_repeatok == CRCInput::RC_up)
		{
			int prevselected = selected;
			if(selected == 0)
			{
				selected = filelist.size() - 1;
			}
			else
				selected--;
			paintItem(prevselected - liststart);
			unsigned int oldliststart = liststart;
			liststart = (selected/listmaxshow)*listmaxshow;
			if(oldliststart != liststart)
			{
				paint();
			}
			else
			{
				paintItem(selected - liststart);
			}
		}
		else if (msg_repeatok == CRCInput::RC_down)
		{
			if (!(filelist.empty()))
			{
				int prevselected = selected;
				selected = (selected + 1) % filelist.size();
				paintItem(prevselected - liststart);
				unsigned int oldliststart = liststart;
				liststart = (selected/listmaxshow)*listmaxshow;
				if(oldliststart != liststart)
					paint();
				else
					paintItem(selected - liststart);
			}
		}
		else if ( ( msg == CRCInput::RC_timeout ) )
		{
			selected = oldselected;
			loop = false;
		}
		else if ( msg == CRCInput::RC_right )
		{
			if (!(filelist.empty()))
			{
				if (S_ISDIR(filelist[selected].Mode))
				{
	 				if (filelist[selected].getFileName() != "..") 
					{
						selections.push_back(selected);
						ChangeDir(filelist[selected].Name);
					}
				}
			}
		}
		else if ( msg == CRCInput::RC_left )
		{
			if (selections.size() > 0)
			{
				ChangeDir("..", selections.back());
				selections.pop_back();
			} 
			else
			{
				ChangeDir("..");
			}
		}
		else if ( msg == CRCInput::RC_blue )
		{
			if(Filter != NULL)
			{
				use_filter = !use_filter;
				paintFoot();
				ChangeDir(Path);
			}
		}
		else if ( msg == CRCInput::RC_home )
		{
			loop = false;
		}
		else if ( msg == CRCInput::RC_spkr)
		{
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
				if (MessageBox(LOCALE_FILEBROWSER_DELETE, _msg.str(), CMessageBox::mbrNo, CMessageBox::mbYes|CMessageBox::mbNo)==CMessageBox::mbrYes)
				{
					recursiveDelete(filelist[selected].Name.c_str());
					
					if(".ts" ==(filelist[selected].getFileName().substr(filelist[selected].getFileName().length() - 3, filelist[selected].getFileName().length())))//if bla.ts
					{
						recursiveDelete((filelist[selected].Name.substr(0,filelist[selected].Name.length()-7)+".xml").c_str());//remove bla.xml von bla.ts
					}
					ChangeDir(Path);

				}
			}
		}
		else if (msg == CRCInput::RC_ok)
		{
			if (!(filelist.empty()))
			{
				if (filelist[selected].getFileName() == "..")
				{
					if (selections.size() > 0)
					{
						ChangeDir("..", selections.back());
						selections.pop_back();
					} 
					else
					{
						std::string::size_type pos = Path.substr(0, Path.length()-1).rfind('/');
						if (pos != std::string::npos) 
						{
							ChangeDir("..");
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
		else if ( msg == CRCInput::RC_info )
		{
			if (++g_settings.filebrowser_sortmethod >= FILEBROWSER_NUMBER_OF_SORT_VARIANTS)
				g_settings.filebrowser_sortmethod = 0;

			sort(filelist.begin(), filelist.end(), sortBy[g_settings.filebrowser_sortmethod]);

			paint();
			paintFoot();
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
	if (msg == CRCInput::RC_home)
	{
		// home key cancel scan
		bCancel = true;
	}
	else if (msg != CRCInput::RC_timeout)
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
		
		for(int i = 0; i < n;i++)
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
	frameBuffer->paintBackgroundBoxRel(x, y, width, height);
	
	frameBuffer->blit();
}

void CFileBrowser::paintItem(unsigned int pos)
{
	int colwidth1, colwidth2, colwidth3, colwidth1_dir, colwidth2_dir;
	uint8_t color;
	fb_pixel_t bgcolor;
	int ypos = y + theight + pos*fheight;
	CFile * actual_file = NULL;
	std::string fileicon;

	colwidth2_dir = 180;
	colwidth1_dir = width - 35 - colwidth2_dir - 10;

	if (liststart + pos == selected)
	{
		color   = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
	}
	else
	{
		color   = COL_MENUCONTENT;//DARK;
		bgcolor = COL_MENUCONTENT_PLUS_0;//DARK;
	}

	if( (liststart + pos) < filelist.size() )
	{
		actual_file = &filelist[liststart + pos];
		if (actual_file->Marked)
		{
			color = COL_MENUCONTENTINACTIVE;
			bgcolor = (liststart + pos == selected) ? COL_MENUCONTENTSELECTED_PLUS_2 : COL_MENUCONTENT_PLUS_2;
		}

		if (g_settings.filebrowser_showrights == 0 && S_ISREG(actual_file->Mode))
		{
			colwidth2 = 0;
			colwidth3 = 90;
		}
		else
		{
			colwidth2 = 90;
			colwidth3 = 90;
		}
		colwidth1 = width - 35 - colwidth2 - colwidth3 - 10;

		frameBuffer->paintBoxRel(x, ypos, width - SCROLLBAR_WIDTH, fheight, bgcolor);

		if ( actual_file->Name.length() > 0 )
		{
			if (liststart + pos == selected)
				CVFD::getInstance()->showMenuText(0, FILESYSTEM_ENCODING_TO_UTF8_STRING(actual_file->getFileName()).c_str(), -1, true); // UTF-8

			switch(actual_file->getType())
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
			int ih, iw;
			frameBuffer->getIconSize(fileicon.c_str(), &iw, &ih);
			frameBuffer->paintIcon(fileicon, x + ICON_OFFSET, ypos + (fheight - ih) / 2 );

			g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(x + 30 + ICON_OFFSET, ypos + fheight, colwidth1 - BORDER_RIGHT, FILESYSTEM_ENCODING_TO_UTF8_STRING(actual_file->getFileName()), color, 0, true); // UTF-8

			if( S_ISREG(actual_file->Mode) )
			{
				if (g_settings.filebrowser_showrights != 0)
				{
					const char * attribute = "xwr";
					char modestring[9 + 1];
					for (int m = 8; m >= 0; m--)
					{
						modestring[8 - m] = (actual_file->Mode & (1 << m)) ? attribute[m % 3] : '-';
					}
					modestring[9] = 0;

					g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(x + 35 + colwidth1 , ypos+ fheight, colwidth2 - 10, modestring, color, 0, true); // UTF-8
				}

#define GIGABYTE 1073741824LL
#define MEGABYTE 1048576LL
#define KILOBYTE 1024LL
				char tmpstr[256];
				const char *unit = "";
				long long factor = 0;
				if (actual_file->Size >= GIGABYTE)
				{
					factor = GIGABYTE;
					unit = "G";
				}
				else if (actual_file->Size >= MEGABYTE)
				{
					factor = MEGABYTE;
					unit = "M";
				}
				else if (actual_file->Size >= KILOBYTE)
				{
					factor = KILOBYTE;
					unit = "k";
				}
				if (factor)
				{
					int a = actual_file->Size / factor;
					int b = (actual_file->Size - a * factor) * 1000 / factor;
					snprintf(tmpstr, sizeof(tmpstr), "%d.%03d%s", a, b, unit);
				}
				else
					snprintf(tmpstr,sizeof(tmpstr),"%d", (int)actual_file->Size);

				g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(x + 35 + colwidth1 + colwidth2, ypos+ fheight, colwidth3 - 10, tmpstr, color);
			}

			if( S_ISDIR(actual_file->Mode) )
			{
				char timestring[18];
				time_t rawtime;

				rawtime = actual_file->Time;
				strftime(timestring, 18, "%d-%m-%Y %H:%M", gmtime(&rawtime));

				g_Font[SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM]->RenderString(x + 35 + colwidth1_dir, ypos+ fheight, colwidth2_dir - 10, timestring, color);
			}
		}
	}
	else
		frameBuffer->paintBoxRel(x, ypos, width - SCROLLBAR_WIDTH, fheight, COL_MENUCONTENT_PLUS_0);
}

void CFileBrowser::paintHead()
{
	char l_name[100];
	frameBuffer->paintBoxRel(x, y, width, theight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, g_settings.menu_Head_gradient);
	
	// icon
	int icon_w, icon_h;
	frameBuffer->getIconSize(NEUTRINO_ICON_FOLDER, &icon_w, &icon_h);
	frameBuffer->paintIcon(NEUTRINO_ICON_FOLDER, x + BORDER_LEFT, y + (theight - icon_h)/2);

	snprintf(l_name, sizeof(l_name), "%s %s", g_Locale->getText(LOCALE_FILEBROWSER_HEAD), FILESYSTEM_ENCODING_TO_UTF8_STRING(name).c_str()); // UTF-8

	g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_TITLE]->RenderString(x + BORDER_LEFT + icon_w + ICON_OFFSET, y + theight + 1, width - BORDER_LEFT - icon_w - ICON_OFFSET - BORDER_RIGHT, l_name, COL_MENUHEAD, 0, true); // UTF-8
}

const struct button_label FileBrowserButtons[3] =
{
	{ NEUTRINO_ICON_BUTTON_RED, LOCALE_FILEBROWSER_NEXTPAGE, NULL },
	{ NEUTRINO_ICON_BUTTON_GREEN, LOCALE_FILEBROWSER_PREVPAGE, NULL },
	{ NEUTRINO_ICON_BUTTON_YELLOW, LOCALE_FILEBROWSER_MARK, NULL },
};

const struct button_label FileBrowserFilterButton[2] =
{
	{ NEUTRINO_ICON_BUTTON_BLUE, LOCALE_FILEBROWSER_FILTER_INACTIVE, NULL },
	{ NEUTRINO_ICON_BUTTON_BLUE, LOCALE_FILEBROWSER_FILTER_ACTIVE, NULL },
};

void CFileBrowser::paintFoot()
{
	int dx = width / 4;
	//Second Line (bottom, top)
	int by2 = y + height - (foheight/2 - 4);
	int ty2 = by2 + g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight();

	// foot
	frameBuffer->paintBoxRel(x, y + height - foheight, width, foheight, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.menu_Foot_gradient);

	if (!(filelist.empty()))
	{
		int by = y + height - foheight;

		::paintButtons(frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, x + BORDER_LEFT, by, dx, Multi_Select ? 3 : 2, FileBrowserButtons, foheight/2);

		if(Filter != NULL)
			::paintButtons(frameBuffer, g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, x + BORDER_LEFT + (3 * dx), by, dx, 1, &(FileBrowserFilterButton[use_filter? 0 : 1]), foheight/2);

		//OK-Button
		int iw, ih;
		if( (filelist[selected].getType() != CFile::FILE_UNKNOWN) || (S_ISDIR(filelist[selected].Mode)) )
		{
			frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_OKAY, &iw, &ih);
			frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_OKAY, x + ICON_OFFSET, by2 - ICON_OFFSET);

			g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(x + 2*ICON_OFFSET + iw, ty2, dx - 2*ICON_OFFSET - iw, g_Locale->getText(LOCALE_FILEBROWSER_SELECT), COL_INFOBAR, 0, true); // UTF-8

		}

		// help-button
		frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_OKAY, &iw, &ih);
		frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_HELP, x + dx + ICON_OFFSET, by2 - ICON_OFFSET);
		g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(x + 2*ICON_OFFSET + iw + dx, ty2, dx - 2*ICON_OFFSET - iw, g_Locale->getText(sortByNames[g_settings.filebrowser_sortmethod]), COL_INFOBAR, 0, true); // UTF-8

		// mute button
		frameBuffer->getIconSize(NEUTRINO_ICON_BUTTON_OKAY, &iw, &ih);
		frameBuffer->paintIcon(NEUTRINO_ICON_BUTTON_MUTE_SMALL, x + 2 *dx, by2 - ICON_OFFSET);

		g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(x + 2*ICON_OFFSET + iw + 2*dx, ty2, dx - 2*ICON_OFFSET - iw, g_Locale->getText(LOCALE_FILEBROWSER_DELETE), COL_INFOBAR, 0, true); // UTF-8

		if(m_SMSKeyInput.getOldKey() != 0)
		{
			char cKey[2] = {m_SMSKeyInput.getOldKey(), 0};
			g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(x + width - 16, by2 , 16, cKey, COL_MENUHEAD, 0, true); // UTF-8
	
			frameBuffer->blit();	
		}
	}
}

void CFileBrowser::paint()
{
	liststart = (selected/listmaxshow)*listmaxshow;

	CVFD::getInstance()->setMode(CVFD::MODE_MENU_UTF8, g_Locale->getText(LOCALE_FILEBROWSER_HEAD));	

	// paint items
	for(unsigned int count = 0; count < listmaxshow; count++)
		paintItem(count);

	int ypos = y + theight;
	int sb = fheight* listmaxshow;
	frameBuffer->paintBoxRel(x + width - SCROLLBAR_WIDTH, ypos, SCROLLBAR_WIDTH, sb,  COL_MENUCONTENT_PLUS_1);

	int sbc= ((filelist.size() - 1)/ listmaxshow) + 1;
	int sbs= (selected/listmaxshow);

	// scroll bar
	frameBuffer->paintBoxRel(x + width - (SCROLLBAR_WIDTH - 2), ypos + 2 + sbs*(sb - 4)/sbc, 11, (sb - 4)/sbc, COL_MENUCONTENT_PLUS_3 );
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
	
	int prevselected = selected;
	selected = i;
	paintItem(prevselected - liststart);
	unsigned int oldliststart = liststart;
	liststart = (selected/listmaxshow)*listmaxshow;
	
	if(oldliststart!=liststart)
	{
		paint();
	}
	else
	{
		paintItem(selected - liststart);
	}
}

void CFileBrowser::recursiveDelete(const char *file)
{
	stat_struct statbuf;
	dirent_struct **namelist;
	int n;
	
	dprintf(DEBUG_INFO, "CFileBrowser::Delete %s\n", file);
	
	if(my_lstat(file,&statbuf) == 0)
	{
		if(S_ISDIR(statbuf.st_mode))
		{
			n = my_scandir(file, &namelist, 0, my_alphasort);
			while(n--)
			{
				if(strcmp(namelist[n]->d_name, ".")!=0 && strcmp(namelist[n]->d_name, "..")!=0)
				{
					std::string fullname = (std::string)file + "/" + namelist[n]->d_name;
					recursiveDelete(fullname.c_str());
				}
				free(namelist[n]);
			}
			free(namelist);
			rmdir(file);
		}
		else
		{
			unlink(file);
		}
	}
	else
		perror(file);
}
