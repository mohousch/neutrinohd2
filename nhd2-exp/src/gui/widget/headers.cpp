/*
 * $Id: headers.cpp 2018/08/19 mohousch Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <global.h>

#include <gui/color.h>
#include <system/settings.h>

#include <gui/widget/headers.h>


void paintHead(int x, int y, int dx, int dy, const char* icon, const neutrino_locale_t caption, bool paintDate, const unsigned int count, const struct button_label* const content, bool logo)
{
	// box
	CFrameBuffer::getInstance()->paintBoxRel(x, y, dx, dy, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, g_settings.Head_gradient);

	// left icon
	int i_w = 0;
	int i_h = 0;
	if(icon != NULL)
	{
		CFrameBuffer::getInstance()->getIconSize(icon, &i_w, &i_h);

		// limit icon dimensions
		if(i_h > dy)
			i_h = dy - 2;

		if(logo)
		{
			i_w = i_h*1.67;

			CFrameBuffer::getInstance()->paintIcon(icon, x + BORDER_LEFT, y + (dy - i_h)/2, 0, true, i_w, i_h);
		}
		else
			CFrameBuffer::getInstance()->paintIcon(icon, x + BORDER_LEFT, y + (dy - i_h)/2);
	}

	// right buttons
	int iw[count], ih[count];
	int startx = x + dx - BORDER_RIGHT;
	int buttonWidth = 0;

	if(count)
	{
		for (unsigned int i = 0; i < count; i++)
		{
			CFrameBuffer::getInstance()->getIconSize(content[i].button, &iw[i], &ih[i]);
		
			startx -= (iw[i] + ICON_TO_ICON_OFFSET);
			buttonWidth += iw[i];

			CFrameBuffer::getInstance()->paintIcon(content[i].button, startx, y + (dy - ih[i])/2);
		}
	}

	// paint time/date
	int timestr_len = 0;
	if(paintDate)
	{
		std::string timestr = getNowTimeStr("%d.%m.%Y %H:%M");;
		
		timestr_len = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getRenderWidth(timestr.c_str(), true); // UTF-8
	
		g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->RenderString(startx - timestr_len, y + (dy - g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight(), timestr_len + 1, timestr.c_str(), COL_MENUHEAD, 0, true); 
	}

	// title
	std::string l_name = g_Locale->getText(caption);

	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(x + BORDER_LEFT + i_w + ICON_OFFSET, y + (dy - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), dx - BORDER_LEFT - BORDER_RIGHT - i_w - 2*ICON_OFFSET - buttonWidth - (count - 1)*ICON_TO_ICON_OFFSET - timestr_len, l_name, COL_MENUHEAD);
	
}

void paintHead(int x, int y, int dx, int dy, const char* icon, const std::string caption, bool paintDate, const unsigned int count, const struct button_label* const content, bool logo)
{
	// box
	CFrameBuffer::getInstance()->paintBoxRel(x, y, dx, dy, COL_MENUHEAD_PLUS_0, RADIUS_MID, CORNER_TOP, g_settings.Head_gradient);

	// left icon
	int i_w = 0;
	int i_h = 0;
	if(icon != NULL)
	{
		CFrameBuffer::getInstance()->getIconSize(icon, &i_w, &i_h);

		// limit icon dimensions
		if(i_h > dy)
			i_h = dy - 2;

		if(logo)
		{
			i_w = i_h*1.67;

			CFrameBuffer::getInstance()->paintIcon(icon, x + BORDER_LEFT, y + (dy - i_h)/2, 0, true, i_w, i_h);
		}
		else
			CFrameBuffer::getInstance()->paintIcon(icon, x + BORDER_LEFT, y + (dy - i_h)/2);
	}

	// right buttons
	int iw[count], ih[count];
	int startx = x + dx - BORDER_RIGHT;
	int buttonWidth = 0;

	if(count)
	{
		for (unsigned int i = 0; i < count; i++)
		{
			CFrameBuffer::getInstance()->getIconSize(content[i].button, &iw[i], &ih[i]);
		
			startx -= (iw[i] + ICON_TO_ICON_OFFSET);
			buttonWidth += iw[i];

			CFrameBuffer::getInstance()->paintIcon(content[i].button, startx, y + (dy - ih[i])/2);
		}
	}

	// paint time/date
	int timestr_len = 0;
	if(paintDate)
	{
		std::string timestr = getNowTimeStr("%d.%m.%Y %H:%M");;
		
		timestr_len = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getRenderWidth(timestr.c_str(), true); // UTF-8
	
		g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->RenderString(startx - timestr_len, y + (dy - g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight(), timestr_len + 1, timestr.c_str(), COL_MENUHEAD, 0, true); 
	}

	// title
	if(!caption.empty())
	{
		std::string l_name = caption;

		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(x + BORDER_LEFT + i_w + ICON_OFFSET, y + (dy - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), dx - BORDER_LEFT - BORDER_RIGHT - i_w - 2*ICON_OFFSET - buttonWidth - (count - 1)*ICON_TO_ICON_OFFSET - timestr_len, l_name, COL_MENUHEAD);
	}
}

// foot
void paintFoot(int x, int y, int dx, int dy, const unsigned int buttonwidth, const unsigned int count, const struct button_label * const content)
{
	// box
	CFrameBuffer::getInstance()->paintBoxRel(x, y, dx, dy, COL_MENUFOOT_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.Foot_gradient);


	// buttons
	if(count)
	{
		::paintButtons(CFrameBuffer::getInstance(), g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL], g_Locale, x + BORDER_LEFT, y, buttonwidth, count, content, dy);
	}
}


