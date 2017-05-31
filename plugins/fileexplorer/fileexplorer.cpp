/*
  $Id: fileexplorer.cpp 2015/06/26 mohousch Exp $

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

#include <plugin.h>


extern "C" void plugin_exec(void);
extern "C" void plugin_init(void);
extern "C" void plugin_del(void);

void plugin_init(void)
{
}

void plugin_del(void)
{
}

void plugin_exec(void)
{
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
		
	CFileBrowser filebrowser;
	
	std::string Path_local = "/media/hdd";
	
	
BROWSER:	
	if (filebrowser.exec(Path_local.c_str())) 
	{
		Path_local = filebrowser.getCurrentDir(); // remark path
		
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
				CMoviePlayerGui tmpMoviePlayerGui;

				// fill file info
				file->Title = file->getFileName();
				file->Info1 = file->getFileName();	// IMDB
				//file.Info2 = files->getFileName(); 	// IMDB

				std::string fname = "";
				fname = file->Name;
				changeFileNameExt(fname, ".jpg");
						
				if(!access(fname.c_str(), F_OK) )
					file->Thumbnail = fname.c_str();
					
				tmpMoviePlayerGui.addToPlaylist(*file);
				tmpMoviePlayerGui.exec(NULL, "urlplayback");
			}
			else if(file->getType() == CFile::FILE_AUDIO)
			{
				CAudioPlayerGui tmpAudioPlayerGui;
			
				CAudiofileExt audiofile(file->Name, file->getExtension());
				tmpAudioPlayerGui.addToPlaylist(audiofile);
				tmpAudioPlayerGui.hidePlayList(true);
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
			goto BROWSER;
		}
	}
}
