/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


//
// Class: RosterItem
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Fri Sep  1 12:38:53 2006
//

#ifndef _ROSTERITEM_H_
#define _ROSTERITEM_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <gtk/gtk.h>
#include <glade/glade.h>



#define AVATAR_SIZE 32

using namespace Woklib;

class RosterItem : public WLSignalInstance
{
	public:
		RosterItem(WLSignal *wls, GladeXML *xml, std::string id, WokXMLTag *tag, RosterItem *parant);
		 ~RosterItem();
		
		void Update(WokXMLTag *tag);
		void UpdateExpand();

		int GetIndent();
		RosterItem *GetLastItem();
		RosterItem *AddChild(RosterItem *); /// Adds a child to a roster item and returns the item that it should be added beneth for sorting
		RosterItem *UpdatePosition(RosterItem *ri);
		void RemoveChild(RosterItem *);
	protected:
		bool folded;
		std::list <RosterItem*> children;
		GtkTreeIter * GetIter();

		std::string id;
		RosterItem *parant;
		GtkTreeModel *model;
		GladeXML *xml;
		GtkTreeIter iter;
		int indent;
		std::string text;
};

enum
{
	PRE_PIX_COLUMN = 0,
	TEXT_COLUMN,
	POST_PIX_COLUMN,
	ID_COLUMN,
	INDENT_COLUMN,
	NUM_COLUMNS
};

#endif	//_ROSTERITEM_H_

