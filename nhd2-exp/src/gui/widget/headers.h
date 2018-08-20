/*
 * $Id: headers.h 2018/08/19 mohousch Exp $
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

#ifndef __HEADERS_H__
#define __HEADERS_H__

#include <driver/fontrenderer.h>
#include <driver/framebuffer.h>


// head
void paintHead(int x, int y, int dx, int dy, const char* icon, const neutrino_locale_t caption, bool paintDate = false, const unsigned int count = 0, const struct button_label* const content = NULL);

void paintHead(int x, int y, int dx, int dy, const char* icon, const std::string caption, bool paintDate = false, const unsigned int count = 0, const struct button_label* const content = NULL);

// foot
void paintFoot(int x, int y, int dx, int dy, const unsigned int buttonwidth = 0, const unsigned int count = 0, const struct button_label * const content = NULL);

#endif
