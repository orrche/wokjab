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


#ifndef _GUICONNECTWINDOW_H_
#define _GUICONNECTWINDOW_H_

#include <gtk/gtk.h>
#include <Woklib/WLSignal.h>
#include <Woklib/WokXMLTag.h>
#include <glade/glade.h>

using namespace Woklib;

class GUIConnectWindow : public WLSignalInstance
{
	public:
		GUIConnectWindow(int *feedback, WLSignal *wls);
		 ~GUIConnectWindow();
		static void Destroy( GtkWidget *widget, gpointer user_data );
		static void Connect_Button( GtkWidget *widget, gpointer data );
		static void Cancel_Button( GtkWidget *widget, gpointer data );
		static void Add_Button( GtkWidget *widget, GUIConnectWindow *c );
		static void Remove_Button( GtkWidget *widget, GUIConnectWindow *c );
		static void RowActivated(GtkTreeView *treeview, GtkTreePath *arg1, GtkTreeViewColumn *arg2, GUIConnectWindow *c);
		static void RowChanged(GtkTreeView *treeview, GUIConnectWindow *c);
	
		int Config(WokXMLTag *);
	protected:
  	GtkWidget *conn_win;
		GtkWidget *password_entry;
		GtkWidget *username_entry;
		GtkWidget *server_entry;
		GtkWidget *port_entry;
		GtkWidget *resource_entry;
		GtkWidget *prio_entry;
		GtkWidget *accounts;
		GtkListStore *accountlist;

		int *feedback;
		WokXMLTag *config;
		GladeXML *xml;
		
		enum
		{
			SERVER_COLUMN = 0,
			USER_COLUMN,
			PASSWORD_COLUMN,
			RESOURCE_COLUMN,
			PORT_COLUMN,
			PRIO_COLUMN,
			AUTO_COLUMN,
			NUM_COLUMNS
		};
};


#endif	//_GUICONNECTWINDOW_H_
