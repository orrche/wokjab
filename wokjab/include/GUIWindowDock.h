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


#ifndef _GUIWINDOWDOCK_H_
#define _GUIWINDOWDOCK_H_

#include <list>

#include <Woklib/WokXMLTag.h>
#include <gtk/gtk.h>
#include <vector>

using namespace Woklib;

class GUIWindowDock : public WLSignalInstance
{
	public:
		GUIWindowDock(WLSignal *wls);
		 ~GUIWindowDock();
	
		GtkWidget *GetWidget();
		int RemovePage(WokXMLTag *tag);
		int AppendPlugPage(WokXMLTag *tag);
		int HidePage(WokXMLTag *tag);
		int ShowPage(WokXMLTag *tag);
		int Activate(WokXMLTag *tag);
		int Config(WokXMLTag *tag);
		void SaveConfig();
		
		static gboolean widget_destroy(GtkWidget *widget, GdkEvent *event, GUIWindowDock *c);
		static void page_delete(GtkWidget *widget, GUIWindowDock *c);
		static gboolean key_press_handler(GtkWidget * widget, GdkEventKey * event,gpointer data);
		void CreateWidget();
	protected:
		GtkWidget *hiddenwindow;
		GtkWidget *hiddenvbox;
		
		GtkWidget *window;
		GtkWidget *notebook;
		GtkWidget *testwid;
		WokXMLTag *config;
		std::vector <int> Widgets;
		std::vector <int> HiddenWidgets;
		std::map <int, GtkWidget *> sockets;
		std::map <int, GtkWidget *> labelsockets;
};


#endif	//_GUIWINDOWDOCK_H_
