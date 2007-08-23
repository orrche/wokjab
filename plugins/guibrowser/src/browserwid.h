/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <nedo80@gmail.com>
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

#ifndef __BROWSERWID_H
#define __BROWSERWID_H

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <gtk/gtk.h>

class BrowserWidget;
#include "browser.h"

using namespace Woklib;

/**
 * Widget for the browser
 */
class BrowserWidget : public WLSignalInstance
{
	public:
		BrowserWidget(WLSignal *wls, std::string session, std::string jid, Browser *bro);
		~BrowserWidget();
	
		void GetItemData(WokXMLTag *tag);
		int GetInfoData(WokXMLTag *tag);
		void AddNode(GtkTreeIter *parant, std::string jid, std::string name, std::string node, std::string session);
		void Connect(std::string signame);
		void InfoConnect(std::string signame);
		int FindTreeIter(GtkTreeIter *parant, std::string jid, std::string node, std::string session);
		void UpdateEntryWidget();
	
		static gboolean MenuActivate(GtkMenuItem *menuitem, BrowserWidget *c);
		static void Destroy (GtkWidget * widget, BrowserWidget *c);
		static void ButtonPress(GtkWidget *widget, BrowserWidget *c);
		static void on_nodetree_row_expanded       (GtkTreeView     *treeview,
       GtkTreeIter *arg1, GtkTreePath *arg2, BrowserWidget *c);
		static gboolean popup_menu(GtkTreeView *tree_view, GdkEventButton *event, BrowserWidget *c);
		static void ComboBoxChange(GtkComboBox *widget, BrowserWidget *c);
	protected:
		GtkWidget *nodetree;
		GtkWidget *window;
		GtkWidget *address_entry;
		GtkWidget *sessionchooser;
	
		GtkListStore *sessionmenu;
		GtkTreeModel *model;
		GtkTreeStore *store;
	
		std::map<std::string,int> nodes;
		std::map<GtkWidget *, std::string> popupmenusignals;
		std::list<std::string> iqsignalhooks;
		std::list<std::string> iqinfosignalhooks;
		GtkWidget *pop_menu;
		Browser *bro;
		
		std::string activesession;
		std::string menu_jid;
		std::string menu_node;
		std::string menu_session;

};

#endif // __BROWSERWID_H
