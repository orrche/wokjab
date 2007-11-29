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
// Class: GUIRoster
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Thu Aug 31 12:39:29 2006
//

#ifndef _GUIROSTER_H_
#define _GUIROSTER_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <gtk/gtk.h>
#include <glade/glade.h>

#include "RosterItem.h"

using namespace Woklib;

class GUIRoster : public WoklibPlugin
{
	public:
		GUIRoster(WLSignal *wls);
		 ~GUIRoster();
	
		void CreateWid();
		int AddItem(WokXMLTag *tag);
		int UpdateItem(WokXMLTag *tag);
		int RemoveItem(WokXMLTag *tag);
		int Close(WokXMLTag *tag);
		int GUIWindowInit(WokXMLTag *tag);
		
		static int row_activated (GtkTreeView *treeview, GtkTreePath *arg1, GtkTreeViewColumn *arg2, GUIRoster *c);
		static gboolean popup_menu(GtkTreeView *tree_view, GdkEventButton *event, GUIRoster *c);
		static gboolean MouseMotion (GtkWidget *treeview, GdkEventMotion *event, GUIRoster *c);
		static gboolean MouseLeave(GtkWidget *treeview, GdkEventCrossing *event, GUIRoster *c);
		static void SizeChange(GtkWidget *widget, GtkAllocation *requisition, GUIRoster *c);
		void SetHover (gchar *hid);
		void CleanHover ();
	protected:
		gchar *hoverid;
		std::map <std::string, RosterItem*> item;
		GladeXML *xml;
		GtkWidget *mainwindowplug;
		WokXMLTag *inittag;
		
		
		GtkTreeViewColumn *pre_pix_column;
		/*GtkTreeViewColumn *text_column;
		GtkTreeViewColumn *post_pix_column;*/
};


#endif	//_GUIROSTER_H_

