/*
	$Id: listbox.h 2018.08.19 mohousch Exp $


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

#if !defined(LISTBOX_H_)
#define LISTBOX_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>
#include <vector>


class ClistBoxEntry : public CMenuItem
{
	private:
		std::vector<CMenuItem*>	items;

		// Functions 
		void onNewLineArray(void);
		void initVar(void);
		void initFramesRel(void);
		//void refreshTitle(void);
		void refreshScroll(void);
		void refreshList(void);
		//void refreshHeaderList(void);
		void reSizeMainFrameWidth(int maxTextWidth);
		void reSizeMainFrameHeight(int maxTextHeight);

		CBox m_cFrame;
		//CBox m_cFrameTitleRel;
		//CBox m_cFrameListRel;
		CBox m_cFrameScrollRel;
		//CBox m_cFrameHeaderListRel;

		int m_nMaxHeight;
		int m_nMaxWidth;

		//int m_nMode;

		int m_nNrOfPages;
		int m_nNrOfLines;
		//int m_nNrOfRows;
		int m_nMaxLineWidth;
		int m_nLinesPerPage;
		int m_nCurrentLine;
		int m_nCurrentPage;
		int m_nSelectedLine;

		bool m_showSelection;
		
		//static CFont* m_pcFontTitle;
		//std::string m_textTitle;
		//int m_nFontTitleHeight;
		
		//static CFont* m_pcFontList;
		//int m_nFontListHeight;
		
		//static CFont* m_pcFontHeaderList;
		//int m_nFontHeaderListHeight;
		
		//
		int LinesPerPage;

		CFrameBuffer * frameBuffer;
		//std::string m_iconTitle;

	public:
		ClistBoxEntry();
		virtual ~ClistBoxEntry();

		// Functions
		void    refresh(void);
		void    refreshLine(int line);
		void    scrollPageDown(const int pages);
		void    scrollPageUp(const int pages);				
		void 	scrollLineDown(const int lines);
		void 	scrollLineUp(const int lines);
		//bool	setLines(LF_LINES* lines);
		//bool	setTitle(const char* title = "", const std::string& icon = NULL);
		bool    setSelectedLine(int selection);
		void	hide(void);
		void	paint(void);

		inline	CBox	getWindowsPos(void)		{return(m_cFrame);};
		inline	int	getMaxLineWidth(void)		{return(m_nMaxLineWidth);};
		inline  int     getSelectedLine(void)		{return(m_nSelectedLine);};
		inline  int     getLines(void)			{return(m_nNrOfLines);};
		inline  int     getPages(void)			{return(m_nNrOfPages);};
		inline  void    showSelection(bool show)	{m_showSelection = show; refreshLine(m_nSelectedLine);};
		inline	void	movePosition(int x, int y)	{m_cFrame.iX = x; m_cFrame.iY = y;};
		
		inline int 	getLinesPerPage(void)		{return(LinesPerPage);};
};

#endif // LISTBOX_H_
