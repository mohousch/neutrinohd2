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

#include <driver/color.h>
#include <system/settings.h>

#include <gui/widget/headers.h>


CHeaders::CHeaders()
{
	// head
	radius = RADIUS_MID;
	corner = CORNER_TOP;
	bgcolor = COL_MENUHEAD_PLUS_0;
	gradient = g_settings.Head_gradient;
	paintDate = false;
	logo = false;
	hbutton_count	= 0;
	hbutton_labels	= NULL;

	// foot
	fradius = RADIUS_MID;
	fcorner = CORNER_BOTTOM;
	fbgcolor = COL_MENUFOOT_PLUS_0;
	fgradient = g_settings.Foot_gradient;
}

void CHeaders::setHeaderButtons(const struct button_label* _hbutton_labels, const int _hbutton_count)
{
	hbutton_count = _hbutton_count;
	hbutton_labels = _hbutton_labels;
}

void CHeaders::paintHead(int x, int y, int dx, int dy, const char* icon, const neutrino_locale_t caption)
{
	// box
	CFrameBuffer::getInstance()->paintBoxRel(x, y, dx, dy, bgcolor, radius, corner, gradient);

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
	int iw[hbutton_count], ih[hbutton_count];
	int startx = x + dx - BORDER_RIGHT;
	int buttonWidth = 0;

	if(hbutton_count)
	{
		for (unsigned int i = 0; i < hbutton_count; i++)
		{
			CFrameBuffer::getInstance()->getIconSize(hbutton_labels[i].button, &iw[i], &ih[i]);
		
			startx -= (iw[i] + ICON_TO_ICON_OFFSET);
			buttonWidth += iw[i];

			CFrameBuffer::getInstance()->paintIcon(hbutton_labels[i].button, startx, y + (dy - ih[i])/2);
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

	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(x + BORDER_LEFT + i_w + ICON_OFFSET, y + (dy - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), dx - BORDER_LEFT - BORDER_RIGHT - i_w - 2*ICON_OFFSET - buttonWidth - (hbutton_count - 1)*ICON_TO_ICON_OFFSET - timestr_len, l_name, COL_MENUHEAD);
	
}

void CHeaders::paintHead(CBox position, const char* icon, const neutrino_locale_t caption)
{
	// box
	CFrameBuffer::getInstance()->paintBoxRel(position.iX, position.iY, position.iWidth, position.iHeight, bgcolor, radius, corner, gradient);

	// left icon
	int i_w = 0;
	int i_h = 0;
	if(icon != NULL)
	{
		CFrameBuffer::getInstance()->getIconSize(icon, &i_w, &i_h);

		// limit icon dimensions
		if(i_h > position.iHeight)
			i_h = position.iHeight - 2;

		if(logo)
		{
			i_w = i_h*1.67;

			CFrameBuffer::getInstance()->paintIcon(icon, position.iX + BORDER_LEFT, position.iY + (position.iHeight - i_h)/2, 0, true, i_w, i_h);
		}
		else
			CFrameBuffer::getInstance()->paintIcon(icon, position.iX + BORDER_LEFT, position.iY + (position.iHeight - i_h)/2);
	}

	// right buttons
	int iw[hbutton_count], ih[hbutton_count];
	int startx = position.iX + position.iWidth - BORDER_RIGHT;
	int buttonWidth = 0;

	if(hbutton_count)
	{
		for (unsigned int i = 0; i < hbutton_count; i++)
		{
			CFrameBuffer::getInstance()->getIconSize(hbutton_labels[i].button, &iw[i], &ih[i]);
		
			startx -= (iw[i] + ICON_TO_ICON_OFFSET);
			buttonWidth += iw[i];

			CFrameBuffer::getInstance()->paintIcon(hbutton_labels[i].button, startx, position.iY + (position.iHeight - ih[i])/2);
		}
	}

	// paint time/date
	int timestr_len = 0;
	if(paintDate)
	{
		std::string timestr = getNowTimeStr("%d.%m.%Y %H:%M");;
		
		timestr_len = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getRenderWidth(timestr.c_str(), true); // UTF-8
	
		g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->RenderString(startx - timestr_len, position.iY + (position.iHeight - g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight(), timestr_len + 1, timestr.c_str(), COL_MENUHEAD, 0, true); 
	}

	// title
	std::string l_name = g_Locale->getText(caption);

	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(position.iX + BORDER_LEFT + i_w + ICON_OFFSET, position.iY + (position.iHeight - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), position.iWidth - BORDER_LEFT - BORDER_RIGHT - i_w - 2*ICON_OFFSET - buttonWidth - (hbutton_count - 1)*ICON_TO_ICON_OFFSET - timestr_len, l_name, COL_MENUHEAD);
	
}

void CHeaders::paintHead(int x, int y, int dx, int dy, const char* icon, const std::string caption)
{
	// box
	CFrameBuffer::getInstance()->paintBoxRel(x, y, dx, dy, bgcolor, radius, corner, gradient);

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
	int iw[hbutton_count], ih[hbutton_count];
	int startx = x + dx - BORDER_RIGHT;
	int buttonWidth = 0;

	if(hbutton_count)
	{
		for (unsigned int i = 0; i < hbutton_count; i++)
		{
			CFrameBuffer::getInstance()->getIconSize(hbutton_labels[i].button, &iw[i], &ih[i]);
		
			startx -= (iw[i] + ICON_TO_ICON_OFFSET);
			buttonWidth += iw[i];

			CFrameBuffer::getInstance()->paintIcon(hbutton_labels[i].button, startx, y + (dy - ih[i])/2);
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

		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(x + BORDER_LEFT + i_w + ICON_OFFSET, y + (dy - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), dx - BORDER_LEFT - BORDER_RIGHT - i_w - 2*ICON_OFFSET - buttonWidth - (hbutton_count - 1)*ICON_TO_ICON_OFFSET - timestr_len, l_name, COL_MENUHEAD);
	}
}

void CHeaders::paintHead(CBox position, const char* icon, const std::string caption)
{
	// box
	CFrameBuffer::getInstance()->paintBoxRel(position.iX, position.iY, position.iWidth, position.iHeight, bgcolor, radius, corner, gradient);

	// left icon
	int i_w = 0;
	int i_h = 0;
	if(icon != NULL)
	{
		CFrameBuffer::getInstance()->getIconSize(icon, &i_w, &i_h);

		// limit icon dimensions
		if(i_h > position.iHeight)
			i_h = position.iHeight - 2;

		if(logo)
		{
			i_w = i_h*1.67;

			CFrameBuffer::getInstance()->paintIcon(icon, position.iX + BORDER_LEFT, position.iY + (position.iHeight - i_h)/2, 0, true, i_w, i_h);
		}
		else
			CFrameBuffer::getInstance()->paintIcon(icon, position.iX + BORDER_LEFT, position.iY + (position.iHeight - i_h)/2);
	}

	// right buttons
	int iw[hbutton_count], ih[hbutton_count];
	int startx = position.iX + position.iWidth - BORDER_RIGHT;
	int buttonWidth = 0;

	if(hbutton_count)
	{
		for (unsigned int i = 0; i < hbutton_count; i++)
		{
			CFrameBuffer::getInstance()->getIconSize(hbutton_labels[i].button, &iw[i], &ih[i]);
		
			startx -= (iw[i] + ICON_TO_ICON_OFFSET);
			buttonWidth += iw[i];

			CFrameBuffer::getInstance()->paintIcon(hbutton_labels[i].button, startx, position.iY + (position.iHeight - ih[i])/2);
		}
	}

	// paint time/date
	int timestr_len = 0;
	if(paintDate)
	{
		std::string timestr = getNowTimeStr("%d.%m.%Y %H:%M");;
		
		timestr_len = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getRenderWidth(timestr.c_str(), true); // UTF-8
	
		g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->RenderString(startx - timestr_len, position.iY + (position.iHeight - g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getHeight(), timestr_len + 1, timestr.c_str(), COL_MENUHEAD, 0, true); 
	}

	// title
	if(!caption.empty())
	{
		std::string l_name = caption;

		g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(position.iX + BORDER_LEFT + i_w + ICON_OFFSET, position.iY + (position.iHeight - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), position.iWidth - BORDER_LEFT - BORDER_RIGHT - i_w - 2*ICON_OFFSET - buttonWidth - (hbutton_count - 1)*ICON_TO_ICON_OFFSET - timestr_len, l_name, COL_MENUHEAD);
	}
}

// foot
void CHeaders::paintFoot(int x, int y, int dx, int dy, const unsigned int count, const struct button_label * const content)
{
	// box
	CFrameBuffer::getInstance()->paintBoxRel(x, y, dx, dy, COL_MENUFOOT_PLUS_0, RADIUS_MID, CORNER_BOTTOM, g_settings.Foot_gradient);


	// buttons
	int buttonWidth = 0;

	if(count)
	{
		buttonWidth = (dx - BORDER_LEFT - BORDER_RIGHT)/count;
		int iw, ih;
		const char *l_option;
	
		for (unsigned int i = 0; i < count; i++)
		{
			if(content[i].button != NULL)
			{
				CFrameBuffer::getInstance()->getIconSize(content[i].button, &iw, &ih);
				int f_h = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight();

				if(content[i].localename != 0)
					l_option = content[i].localename;
				else
					l_option = g_Locale->getText(content[i].locale);
		
				CFrameBuffer::getInstance()->paintIcon(content[i].button, x + BORDER_LEFT + i*buttonWidth, y + (dy - ih)/2);

				g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(x + BORDER_LEFT + iw + ICON_OFFSET + i*buttonWidth, y + f_h + (dy - f_h)/2, buttonWidth - iw - ICON_OFFSET, l_option, COL_MENUFOOT, 0, true); // UTF-8
			}
		}
	}
}

void CHeaders::paintFoot(CBox position, const unsigned int count, const struct button_label * const content)
{
	// box
	CFrameBuffer::getInstance()->paintBoxRel(position.iX, position.iY, position.iWidth, position.iHeight, fbgcolor, fradius, fcorner, fgradient);

	// buttons
	int buttonWidth = 0;
	if(count)
	{
		buttonWidth = (position.iWidth - BORDER_LEFT - BORDER_RIGHT)/count;
		int iw, ih;
		const char *l_option;
	
		for (unsigned int i = 0; i < count; i++)
		{
			if(content[i].button != NULL)
			{
				CFrameBuffer::getInstance()->getIconSize(content[i].button, &iw, &ih);
				int f_h = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight();

				if(content[i].localename != 0)
					l_option = content[i].localename;
				else
					l_option = g_Locale->getText(content[i].locale);
		
				CFrameBuffer::getInstance()->paintIcon(content[i].button, position.iX + BORDER_LEFT+ i * buttonWidth, position.iY + (position.iHeight - ih)/2);

				g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(position.iX + BORDER_LEFT + iw + ICON_OFFSET + i * buttonWidth, position.iY + f_h + (position.iHeight - f_h)/2, buttonWidth - iw - ICON_OFFSET, l_option, COL_MENUFOOT, 0, true); // UTF-8
			}
		}
	}
}




