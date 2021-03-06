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
#include <iostream>
#include <glade/glade.h>

#include <Woklib/WLSignal.h>
#include <Woklib/WokLibSignal.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

class GUIPluginWindow : public WLSignalInstance
{
	public:
		GUIPluginWindow(WLSignal *wls, std::string pid);
		 ~GUIPluginWindow();
	
	void DisplayPlugins();

	int ReadConfig(WokXMLTag *tag);
	void SaveConfig();
	
	static void load_destroy( GtkWidget *widget, GUIPluginWindow *c );
	static void Destroy( GtkWidget *widget, GUIPluginWindow *c );
	static void Cancel_Button( GtkWidget *widget, GUIPluginWindow *c  );
	static void load_plugin (GtkFileSelection *file_selector, GUIPluginWindow *c);
	static void Add_Button( GtkWidget *widget, GUIPluginWindow *data );
	static void Remove_Button( GtkWidget *widget, GUIPluginWindow *c );
	static void Reload_Button( GtkWidget *widget, GUIPluginWindow *c );
	static void Toggled (GtkCellRendererToggle *cell, gchar *path_str, GUIPluginWindow *c);
	static gboolean DeleteEvent( GtkWidget *widget, GdkEvent *event, GUIPluginWindow *c);
	protected:
		WokXMLTag *config;
	
		GladeXML *filexml;
		GladeXML *xml;
		GtkListStore *model;

		std::string id;
		WLSignal *wls;
};


#endif	//_GUIPLUGINWINDOW_H_
