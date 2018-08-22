/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: bouqueteditor_bouquets.h 2018/08/22 mohousch Exp $

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

#ifndef __bouqueteditor_bouquets__
#define __bouqueteditor_bouquets__

#include <driver/framebuffer.h>
#include <gui/widget/menue.h>

// zapit
#include <client/zapitclient.h>
#include <channel.h>
#include <bouquets.h>

#include <string>
#include <gui/widget/listbox.h>

/* class for handling when bouquets changed.                  */
/* This class should be a temporarily work around             */
/* and should be replaced by standard neutrino event handlers */
/* (libevent) */
#if 0
class CBouquetEditorEvents
{
	public:
		virtual void onBouquetsChanged() {};
		virtual ~CBouquetEditorEvents() {};
};
#endif

class CBEBouquetWidget : public CMenuWidget
{
	public:
		BouquetList* Bouquets;

	private:

		CFrameBuffer* frameBuffer;
		CBox cFrameBox;
		ClistBoxEntry* listBox;
		CMenuItem* item;

		enum
		{
			beDefault,
			beMoving
		} state;

		enum
		{
			beRename,
			beHide,
			beLock
		} blueFunction;

		unsigned int		selected;
		unsigned int		origPosition;
		unsigned int		newPosition;
		bool		bouquetsChanged;

		uint32_t sec_timer_id;

		void paint(bool reinit = true);
		void hide();

		void deleteBouquet();
		void addBouquet();
		void beginMoveBouquet();
		void finishMoveBouquet();
		void cancelMoveBouquet();
		void internalMoveBouquet( unsigned int fromPosition, unsigned int toPosition);
		void renameBouquet();
		void switchHideBouquet();
		void switchLockBouquet();

		void saveChanges();
		void discardChanges();

		std::string inputName(const char* const defaultName, const neutrino_locale_t caption);

	public:
		CBEBouquetWidget();
		~CBEBouquetWidget();
		int exec(CMenuTarget* parent, const std::string& actionKey);
};


#endif
