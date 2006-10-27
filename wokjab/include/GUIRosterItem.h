/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <oden@gmx.net>
 ****************************************************************************/
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.



#ifndef _GUIROSTERITEM_H_
#define _GUIROSTERITEM_H_

#include <gtk/gtk.h>

class GUIRosterItem;

#include <Woklib/WLSignal.h>
#include <Woklib/WokXMLTag.h>
#include "include/GUIRoster.h"

#include <list>

using std::list;
using std::string;

class GUIRosterItem
{
	public:
		GUIRosterItem();
		 ~GUIRosterItem();

		void Init( GtkWidget *treeview, GtkTreeStore *model, WokXMLTag* tag_item, WLSignal *wls);
		void UpdatePresence( WLSignalData* wlsd = NULL);
		int GetPresence();
		void AddEvent(WokXMLTag *);
		void RemoveEvent(WokXMLTag *);

		void UpdateIcons(const std::string &icon);
		bool ActivateEvent();
	protected:
		void Display();
		void DisplayInGroup(std::string group_name);
		void PutGroup(std::string group_name, GtkTreeIter *parant_iter);
		void PutItem(GtkTreeIter *parant_iter);
		void Hide();
		bool hidden;
		int presence;

		WLSignal *wls;
		string session;
		string jid;
		string nick;
		string showmsg;
		string statusmsg;
		
		list<GtkTreeRowReference*> Groups;
		list<WokXMLTag *> Events;
		GtkTreeStore *model;
		GtkWidget *treeview;	
};


#endif	//_GUIROSTERITEM_H_
