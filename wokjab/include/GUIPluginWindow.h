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



#ifndef _GUIPLUGINWINDOW_H_
#define _GUIPLUGINWINDOW_H_

#include <gtk/gtk.h>
#include <Woklib/WLSignal.h>
#include <iostream>

using namespace Woklib;

class GUIPluginWindow
{
	public:
		GUIPluginWindow(int *feedback, WLSignal *wls);
		 ~GUIPluginWindow();
	
	void DisplayPlugins();


	static void Destroy( GtkWidget *widget, gpointer   user_data );
	static void Cancel_Button( GtkWidget *widget, gpointer sig_data );
	static void load_plugin (GtkFileSelection *file_selector, gpointer user_data);
	static void Add_Button( GtkWidget *widget, gpointer sig_data );
	static void Remove_Button( GtkWidget *widget, GUIPluginWindow *c );
	static void Reload_Button( GtkWidget *widget, GUIPluginWindow *c );
	protected:
		GtkWidget *window;
		GtkWidget *treeview1;
		GtkListStore *model;
		GtkWidget *file_selector;
		int *feedback;
		WLSignal *wls;
};


#endif	//_GUIPLUGINWINDOW_H_
