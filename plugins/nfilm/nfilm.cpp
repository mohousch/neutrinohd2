/*
  $Id: nfilm.cpp 2018/07/31 mohousch Exp $

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

#include <fstream>
#include <jsoncpp/include/json/json.h>


extern "C" void plugin_exec(void);
extern "C" void plugin_init(void);
extern "C" void plugin_del(void);

class CNFilm : public CMenuTarget
{
	private:
		// variables
		CFrameBuffer* frameBuffer;
		int selected;

		//
		ClistBox* mlist;
		CMenuItem* item;

		//
		CTmdb* tmdb;
		std::string thumbnail_dir;
		CFileHelpers fileHelper;

		CMovieInfo m_movieInfo;
		std::vector<MI_MOVIE_INFO> m_vMovieInfo;
		MI_MOVIE_INFO movieInfo;
		CMoviePlayerGui tmpMoviePlayerGui;

		void loadPlaylist();

	public:
		CNFilm();
		~CNFilm();
		int exec(CMenuTarget* parent, const std::string& actionKey);
		void hide();

		void showMenu();
};

CNFilm::CNFilm()
{
	frameBuffer = CFrameBuffer::getInstance();

	//
	mlist = NULL;
	item = NULL;

	tmdb = NULL;
	thumbnail_dir = "/tmp/nfilm";
	fileHelper.createDir(thumbnail_dir.c_str(), 0755);

	//
	selected = 0;
}

CNFilm::~CNFilm()
{
	m_vMovieInfo.clear();
	fileHelper.removeDir(thumbnail_dir.c_str());
}

void CNFilm::hide()
{
	frameBuffer->paintBackground();
	frameBuffer->blit();
}

void CNFilm::loadPlaylist()
{
/*
	std::string url = "https://api.trakt.tv/movies/popular";
	std::vector<std::string*> title;

	std::string answer;

	if (!::getUrl(url, answer))
		return;

	Json::Value root, text;
	Json::Reader reader;

	std::ostringstream ss;
	std::ifstream fh(answer.c_str(), std::ifstream::in);
	ss << fh.rdbuf();
	std::string filedata = ss.str();

	if (!reader.parse(filedata, root))
		return;

	text = root.get("title", "" );
	if (text.type() != Json::stringValue)
		return;

	printf("title.size():%d\n", text.size());
*/

	// popular movies
	const char* filmTitle[] = {
		"Deadpool",
		"Guardians of the Galaxy",
		"The Dark Knight",
		"Inception",
		"Logan",
		"Doctor Strange",
		"The Avengers",
		"Suicide Squad",
		"Wonder Woman",
		"Interstellar",
		"avengers infinity war",
		"antman and the wasp",
		"black panther",
		"aquaman",
		"the equalizer",
		"Rampage",
		"Deadpool 2",
		"Antman",
		"thor",
		"thor ragnarok",
		"ready player one",
		"jurassic world",
		"Guardians of the Galaxy 2"	
	};

	for (unsigned int i = 0; i < sizeof(filmTitle)/sizeof(char*); i++)
	{
		tmdb = new CTmdb();

		m_movieInfo.clearMovieInfo(&movieInfo); // refresh structure

		movieInfo.file.Name = filmTitle[i];

		// load infos from tmdb
		tmdb->getMovieInfo(movieInfo.file.Name);

		if ((tmdb->getResults() > 0) && (!tmdb->getDescription().empty())) 
		{
			movieInfo.epgTitle = tmdb->getTitle();
			movieInfo.epgInfo1 = tmdb->createInfoText();
			movieInfo.epgChannel = tmdb->getReleaseDate();
			
			std::string tname = thumbnail_dir;
			tname += "/";
			tname += tmdb->getTitle();
			tname += ".jpg";

			tmdb->getBigCover(tname);

			if(!tname.empty())
				movieInfo.tfile = tname;
		}
					
		// 
		m_vMovieInfo.push_back(movieInfo);

		delete tmdb;
		tmdb = NULL;
	}
}

void CNFilm::showMenu()
{
	mlist = new ClistBox("NFilm", NEUTRINO_ICON_MOVIE, w_max ( (CFrameBuffer::getInstance()->getScreenWidth() / 20 * 17), (CFrameBuffer::getInstance()->getScreenWidth() / 20 )), h_max ( (CFrameBuffer::getInstance()->getScreenHeight() / 20 * 17), (CFrameBuffer::getInstance()->getScreenHeight() / 20)));
	
	
	// load playlist
	loadPlaylist();

	for (unsigned int i = 0; i < m_vMovieInfo.size(); i++)
	{
		item = new ClistBoxItem(m_vMovieInfo[i].epgTitle.c_str(), true, m_vMovieInfo[i].epgChannel.c_str(), this, "mplay", NULL, file_exists(m_vMovieInfo[i].tfile.c_str())? m_vMovieInfo[i].tfile.c_str() : DATADIR "/neutrino/icons/nopreview.jpg");

		std::string tmp = m_vMovieInfo[i].epgTitle;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo1;
		tmp += "\n";
		tmp += m_vMovieInfo[i].epgInfo2;

		item->setInfo1(tmp.c_str());

		item->setOptionFont(g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]);

		mlist->addItem(item);
	}


	mlist->setWidgetType(WIDGET_FRAME);
	mlist->setItemsPerPage(6, 2);
	mlist->setItemBoxColor(COL_YELLOW);

	mlist->setSelected(selected);

	//mlist->setHeaderButtons(HeadButtons, HEAD_BUTTONS_COUNT);

	mlist->enablePaintDate();

	mlist->addKey(CRCInput::RC_info, this, CRCInput::getSpecialKeyName(CRCInput::RC_info));
	//mlist->addKey(CRCInput::RC_setup, this, CRCInput::getSpecialKeyName(CRCInput::RC_setup));
	//mlist->addKey(CRCInput::RC_red, this, CRCInput::getSpecialKeyName(CRCInput::RC_red));
	//mlist->addKey(CRCInput::RC_spkr, this, CRCInput::getSpecialKeyName(CRCInput::RC_spkr));

	mlist->exec(NULL, "");
	//mlist->hide();
	delete mlist;
	mlist = NULL;
}

int CNFilm::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CNFilm::exec: actionKey: %s\n", actionKey.c_str());

	if(parent)
		hide();

	if(actionKey == "RC_info" || actionKey == "mplay")
	{
		selected = mlist->getSelected();
		m_movieInfo.showMovieInfo(m_vMovieInfo[mlist->getSelected()]);

		return menu_return::RETURN_REPAINT;
	}

	showMenu();

	return menu_return::RETURN_REPAINT;
}

void plugin_init(void)
{
	dprintf(DEBUG_NORMAL, "CNFilm: plugin_init\n");
}

void plugin_del(void)
{
	dprintf(DEBUG_NORMAL, "CNFilm: plugin_del\n");
}

void plugin_exec(void)
{
	CNFilm* nFilmHandler = new CNFilm();
	
	nFilmHandler->exec(NULL, "");
	
	delete nFilmHandler;
	nFilmHandler = NULL;
}

