/*
	$Id: framebox.cpp 09.02.2019 mohousch Exp $


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

#include <global.h>

#include <gui/widget/framebox.h>
#include <gui/widget/textbox.h>

#include <gui/pluginlist.h>

#include <system/settings.h>
#include <system/debug.h>


extern CPlugins * g_PluginList;    // defined in neutrino.cpp

// CFrame
CFrame::CFrame(int m)
{
	caption = "";
	mode = m;
	shadow = true;

	if (mode == FRAME_PLUGIN)
	{
		printf("mode: plugin\n");
		if (g_PluginList->plugin_exists(caption))
		{
			printf("mode: plugin exists\n");
			unsigned int count = g_PluginList->find_plugin(caption);

			//iconName
			iconName = NEUTRINO_ICON_MENUITEM_PLUGIN;

			std::string icon("");
			icon = g_PluginList->getIcon(count);

			if(!icon.empty())
			{
					iconName = PLUGINDIR;
					iconName += "/";
					iconName += g_PluginList->getFileName(count);
					iconName += "/";
					iconName += g_PluginList->getIcon(count);
			}

			// caption
			caption = g_PluginList->getName(count);

			// option
			option = g_PluginList->getDescription(count);

			// jumpTarget
			jumpTarget = CPluginsExec::getInstance();

			// actionKey
			actionKey = to_string(count).c_str();
		}
	}
}

CFrame::CFrame(const std::string title, int m)
{
	caption = title;
	mode = m;
	shadow = true;

	if (mode == FRAME_PLUGIN)
	{
		printf("mode: plugin\n");
		if (g_PluginList->plugin_exists(caption))
		{
			printf("mode: plugin exists\n");
			unsigned int count = g_PluginList->find_plugin(caption);

			//iconName
			iconName = NEUTRINO_ICON_MENUITEM_PLUGIN;

			std::string icon("");
			icon = g_PluginList->getIcon(count);

			if(!icon.empty())
			{
					iconName = PLUGINDIR;
					iconName += "/";
					iconName += g_PluginList->getFileName(count);
					iconName += "/";
					iconName += g_PluginList->getIcon(count);
			}

			// caption
			caption = g_PluginList->getName(count);

			// option
			option = g_PluginList->getDescription(count);

			// jumpTarget
			jumpTarget = CPluginsExec::getInstance();

			// actionKey
			actionKey = to_string(count).c_str();
		}
	}
}

int CFrame::paint(bool selected, bool /*AfterPulldown*/)
{
	dprintf(DEBUG_DEBUG, "CFrame::paint:\n");

	if (mode == FRAME_SEPARATOR)
	{
		return 0;
	}

	uint8_t color = COL_MENUCONTENT;
	fb_pixel_t bgcolor = item_backgroundColor;

	if (selected)
	{
		color = COL_MENUCONTENTSELECTED;
		bgcolor = COL_MENUCONTENTSELECTED_PLUS_0;
	}

	// paint frame
	window.setColor(bgcolor);
	window.paint();

	// icon
	int iw = 0;
	int ih = 0;
	int iconOffset = 0;

	if (mode == FRAME_BOX)
	{
		if(!iconName.empty())
		{
			iconOffset = ICON_OFFSET;

			CFrameBuffer::getInstance()->getIconSize(iconName.c_str(), &iw, &ih);

			CFrameBuffer::getInstance()->paintIcon(iconName, window.getWindowsPos().iX + ICON_OFFSET, window.getWindowsPos().iY + (window.getWindowsPos().iHeight - ih)/2);
		}

		// caption
		if(!option.empty())
		{
			// caption
			if(!caption.empty())
			{
				int c_w = g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE]->getRenderWidth(caption);

				g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE]->RenderString(window.getWindowsPos().iX + BORDER_LEFT + iconOffset + iw + ((window.getWindowsPos().iWidth - BORDER_LEFT - iconOffset - iw - c_w) >> 1), window.getWindowsPos().iY + 3 + g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE]->getHeight(), window.getWindowsPos().iWidth - BORDER_LEFT - BORDER_RIGHT - iconOffset - iw, caption.c_str(), color, 0, true); //
			}

			// option
			if(!option.empty())
			{
				int o_w = g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getRenderWidth(option);

				g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->RenderString(window.getWindowsPos().iX + BORDER_LEFT + iconOffset + iw + ((window.getWindowsPos().iWidth - BORDER_LEFT - iconOffset - iw - o_w) >> 1), window.getWindowsPos().iY + window.getWindowsPos().iHeight, window.getWindowsPos().iWidth - BORDER_LEFT - BORDER_RIGHT - iconOffset -iw, option.c_str(), color, 0, true);
			}
		}
		else
		{
			if(!caption.empty())
			{
				int c_w = g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE]->getRenderWidth(caption);

				g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE]->RenderString(window.getWindowsPos().iX + BORDER_LEFT + iconOffset + iw + ((window.getWindowsPos().iWidth - BORDER_LEFT - iconOffset - iw - c_w)>> 1), window.getWindowsPos().iY + (window.getWindowsPos().iHeight - g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_EPG_TITLE]->getHeight(), window.getWindowsPos().iWidth - BORDER_LEFT - BORDER_RIGHT - iconOffset - iw, caption.c_str(), color);
			}
		}
	}
	else if (mode == FRAME_PICTURE)
	{
		if(!iconName.empty())
		{
			CFrameBuffer::getInstance()->displayImage(iconName, window.getWindowsPos().iX + 1, window.getWindowsPos().iY + 1, window.getWindowsPos().iWidth - 2, window.getWindowsPos().iHeight - 40);
		}

		if(!caption.empty())
		{
			int c_w = g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getRenderWidth(caption);

			g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->RenderString(window.getWindowsPos().iX + ((window.getWindowsPos().iWidth - c_w)>> 1), window.getWindowsPos().iY + window.getWindowsPos().iHeight, window.getWindowsPos().iWidth, caption.c_str(), color);
		}
	}
	else if (mode == FRAME_BUTTON)
	{
		// iconName
		if(!iconName.empty())
		{
			iconOffset = ICON_OFFSET;

			CFrameBuffer::getInstance()->getIconSize(iconName.c_str(), &iw, &ih);

			CFrameBuffer::getInstance()->paintIcon(iconName, window.getWindowsPos().iX + ICON_OFFSET, window.getWindowsPos().iY + (window.getWindowsPos().iHeight - ih)/2);
		}

		// caption
		if(!caption.empty())
		{
			int c_w = g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getRenderWidth(caption);

			g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->RenderString(window.getWindowsPos().iX + BORDER_LEFT + iconOffset + iw + ((window.getWindowsPos().iWidth - BORDER_LEFT - iconOffset - iw - c_w) >> 1), window.getWindowsPos().iY + 3 + g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getHeight(), window.getWindowsPos().iWidth - BORDER_LEFT - BORDER_RIGHT - iconOffset - iw, caption.c_str(), color, 0, true); //
		}
	}
	else if (mode == FRAME_TEXT)
	{
		CTextBox * textBox = NULL;

		if(textBox)
		{
			delete textBox;
			textBox = NULL;
		}

		textBox = new CTextBox(window.getWindowsPos().iX + 1, window.getWindowsPos().iY + 1, window.getWindowsPos().iWidth - 2, window.getWindowsPos().iHeight - 2);

		textBox->disablePaintBackground();
		textBox->setMode(AUTO_WIDTH);

		// caption
		if(!caption.empty())
		{
			textBox->setText(caption.c_str());
			textBox->paint();
		}
	}
	else if (mode == FRAME_PLUGIN)
	{
		if(!iconName.empty())
		{
			CFrameBuffer::getInstance()->displayImage(iconName, window.getWindowsPos().iX + 1, window.getWindowsPos().iY + 1, window.getWindowsPos().iWidth - 2, window.getWindowsPos().iHeight - 40);
		}

		if(!caption.empty())
		{
			int c_w = g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getRenderWidth(caption);

			g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->RenderString(window.getWindowsPos().iX + ((window.getWindowsPos().iWidth - c_w)>> 1), window.getWindowsPos().iY + window.getWindowsPos().iHeight, window.getWindowsPos().iWidth, caption.c_str(), color);
		}
	}

	return 0;
}

int CFrame::exec(CMenuTarget *parent)
{
	dprintf(DEBUG_NORMAL, "CFrame::exec: actionKey:(%s)\n", actionKey.c_str());

	int ret = RETURN_EXIT;

	if (mode == FRAME_SEPARATOR)
		ret = RETURN_EXIT;
	else
	{
		if(jumpTarget)
			ret = jumpTarget->exec(parent, actionKey);
		else
			ret = RETURN_EXIT;
	}

	return ret;
}

// CFrameBox
CFrameBox::CFrameBox(const int x, int const y, const int dx, const int dy)
{
	dprintf(DEBUG_NORMAL, "CFrameBox::CFrameBox:\n");

	frameBuffer = CFrameBuffer::getInstance();

	itemBox.iX = x;
	itemBox.iY = y;
	itemBox.iWidth = dx;
	itemBox.iHeight = dy;

	selected = 0;
	pos = 0;
	inFocus = true;
	backgroundColor = COL_MENUCONTENT_PLUS_0;

	frameMode = FRAMEBOX_MODE_HORIZONTAL;

	itemType = WIDGET_ITEM_FRAMEBOX;

	actionKey = "";

	paintFrame = true;

	initFrames();
}

CFrameBox::CFrameBox(CBox* position)
{
	dprintf(DEBUG_NORMAL, "CFrameBox::CFrameBox:\n");

	frameBuffer = CFrameBuffer::getInstance();

	itemBox = *position;

	selected = 0;
	pos = 0;
	inFocus = true;
	backgroundColor = COL_MENUCONTENT_PLUS_0;

	frameMode = FRAMEBOX_MODE_HORIZONTAL;

	itemType = WIDGET_ITEM_FRAMEBOX;

	actionKey = "";

	paintFrame = true;

	initFrames();
}

CFrameBox::~CFrameBox()
{
	frames.clear();
}

void CFrameBox::addFrame(CFrame *frame, const bool defaultselected)
{
	if (defaultselected)
		selected = frames.size();
	
	frames.push_back(frame);
}

bool CFrameBox::hasItem()
{
	return !frames.empty();
}

void CFrameBox::initFrames()
{
	cFrameWindow.setPosition(&itemBox);
}

void CFrameBox::paintFrames()
{
	dprintf(DEBUG_NORMAL, "CFrameBox::paintFrames:\n");

	//
	int frame_width = 0;
	int frame_height = 0;

	if(frames.size())
	{
		if(frameMode == FRAMEBOX_MODE_HORIZONTAL)
		{
			frame_width = (itemBox.iWidth - 2*ICON_OFFSET)/((int)frames.size());
			frame_height = itemBox.iHeight - 2*ICON_OFFSET;
		}
		else
		{
			frame_width = itemBox.iWidth - 2*ICON_OFFSET;
			frame_height = (itemBox.iHeight - 2*ICON_OFFSET)/((int)frames.size());
		}
	}

	int frame_x = itemBox.iX + ICON_OFFSET;
	int frame_y = itemBox.iY + ICON_OFFSET;

	for (unsigned int count = 0; count < (unsigned int)frames.size(); count++) 
	{
		CFrame *frame = frames[count];

		// init frame
		if(frameMode == FRAMEBOX_MODE_HORIZONTAL)
			frame->window.setPosition(frame_x + count*(frame_width) + ICON_OFFSET, frame_y, frame_width - 2*ICON_OFFSET, frame_height);
		else
			frame->window.setPosition(frame_x, frame_y + count*(frame_height) + ICON_OFFSET, frame_width, frame_height - 2*ICON_OFFSET);

		if(frame->isSelectable() && frame->shadow)
			frame->window.enableShadow();

		frame->item_backgroundColor = backgroundColor;

		if(inFocus)
			frame->paint( selected == ((signed int) count));
		else
			frame->paint(false);
	}
}

void CFrameBox::paint()
{
	dprintf(DEBUG_NORMAL, "CFrameBox::paint:\n");

	cFrameWindow.setColor(backgroundColor);
	//cFrameWindow.setCorner(RADIUS_MID, CORNER_ALL);
	//cFrameWindow.enableShadow();
	//cFrameWindow.enableSaveScreen();

	if (paintFrame)
		cFrameWindow.paint();

	paintFrames();

	CFrameBuffer::getInstance()->blit();
}

void CFrameBox::hide()
{
	dprintf(DEBUG_NORMAL, "CFrameBox::hide:\n");

	cFrameWindow.hide();
}

void CFrameBox::swipRight()
{
	dprintf(DEBUG_NORMAL, "CFrameBox::swipRight:\n");

	if(frameMode == FRAMEBOX_MODE_HORIZONTAL)
	{
		for (unsigned int count = 1; count < frames.size(); count++) 
		{
			pos = (selected + count)%frames.size();

			CFrame * frame = frames[pos];

			if(frame->isSelectable())
			{
				frames[selected]->paint(false);
				frame->paint(true);

				selected = pos;
				
				break;
			}
		}
	}
}

void CFrameBox::swipLeft()
{
	dprintf(DEBUG_NORMAL, "CFrameBox::swipLeft:\n");

	if(frameMode == FRAMEBOX_MODE_HORIZONTAL)
	{
		for (unsigned int count = 1; count < frames.size(); count++) 
		{
			pos = selected - count;
			if ( pos < 0 )
				pos += frames.size();

			CFrame * frame = frames[pos];

			if(frame->isSelectable())
			{
				frames[selected]->paint(false);
				frame->paint(true);

				selected = pos;

				break;
			}
		}
	}
}

void CFrameBox::scrollLineDown(const int lines)
{
	dprintf(DEBUG_NORMAL, "CFrameBox::scrollLineDown:\n");

	if(frameMode == FRAMEBOX_MODE_VERTICAL)
	{
		for (unsigned int count = 1; count < frames.size(); count++) 
		{
			pos = (selected + count)%frames.size();

			CFrame * frame = frames[pos];

			if(frame->isSelectable())
			{
				frames[selected]->paint(false);
				frame->paint(true);

				selected = pos;

				break;
			}
		}
	}
}

void CFrameBox::scrollLineUp(const int lines)
{
	dprintf(DEBUG_NORMAL, "CFrameBox::scrollLineUp:\n");

	if(frameMode == FRAMEBOX_MODE_VERTICAL)
	{
		for (unsigned int count = 1; count < frames.size(); count++) 
		{
			pos = selected - count;
			if ( pos < 0 )
				pos += frames.size();

			CFrame * frame = frames[pos];

			if(frame->isSelectable())
			{
				frames[selected]->paint(false);
				frame->paint(true);

				selected = pos;

				break;
			}
		}
	}
}

int CFrameBox::oKKeyPressed(CMenuTarget *parent)
{
	if(parent)
		return frames[selected]->exec(parent);
	else
		return RETURN_EXIT;
}

void CFrameBox::onUpKeyPressed()
{
	dprintf(DEBUG_DEBUG, "CFrameBox::UpKeyPressed:\n");

	scrollLineUp();
}

void CFrameBox::onDownKeyPressed()
{
	dprintf(DEBUG_DEBUG, "CFrameBox::DownKeyPressed:\n");

	scrollLineDown();
}

void CFrameBox::onRightKeyPressed()
{
	dprintf(DEBUG_DEBUG, "CFrameBox::RightKeyPressed:\n");

	swipRight();
}

void CFrameBox::onLeftKeyPressed()
{
	dprintf(DEBUG_DEBUG, "CFrameBox::LeftKeyPressed:\n");

	swipLeft();
}

void CFrameBox::onPageUpKeyPressed()
{
	dprintf(DEBUG_DEBUG, "CFrameBox::PageUpKeyPressed:\n");

	//scrollPageUp();
}

void CFrameBox::onPageDownKeyPressed()
{
	dprintf(DEBUG_DEBUG, "CFrameBox::PageDownKeyPressed:\n");

	//scrollPageDown();
}




