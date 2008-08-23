/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <nedo80@gmail.com>
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



#ifndef _GUIWINDOW_H_
#define _GUIWINDOW_H_

#include <gtk/gtk.h>
#include <glade/glade.h>

#include <Woklib/WLSignalInstance.h>

#include "include/GUIMessageHandler.h"
#include "include/GUIConnectWindow.h"
#include "include/GUIPluginWindow.h"

#include <sys/types.h>

/*
#include <gdk/gdkx.h>
#include <X11/Xmd.h>
#include <X11/SM/SMlib.h>
#include <X11/Xatom.h>
*/

class GUIWindow : public WLSignalInstance
{
	public:
		GUIWindow(WLSignal *wls);
		 ~GUIWindow();

		static void Destroy( GtkWidget *widget,
                     GUIWindow * c );
		static gboolean Delete( GtkWidget *widget, GdkEvent *event, GUIWindow *c);
		static void MenuActivate (GtkComboBox *widget, GUIWindow *c);
		static gboolean MainMenu (GtkButton *button, GdkEventButton *event, GUIWindow *data);
		static gboolean WindowMove(GtkWidget *widget, GdkEventMotion *event, GUIWindow *c);
		static void SpinBtnPrio(GtkSpinButton *spinbutton, GUIWindow *c);
		static gboolean SetPresence(GUIWindow * c);
		static void EntryStatusActivate(GtkEntry *entry, GUIWindow *c);
		static gboolean EntryStatusLeft(GtkWidget *widget, GdkEventFocus *event, GUIWindow *c);
		static gboolean StorePresence(GUIWindow *c);
			
		int AddWidget(WokXMLTag *tag);
		int Connect (WLSignalData *wlsd);
		int PluginWin (WLSignalData *wlsd);
		int show(WLSignalData *wlsd);
		int hide(WLSignalData *wlsd);
		int toggle(WLSignalData *wlsd);
		int Loggedin(WokXMLTag *tag);
		int Loggedout(WokXMLTag *tag);
		int GetMainMenu(WokXMLTag *menu_tag);
		int QuitRequest(WLSignalData *wlsd);
		int SendingPresence(WokXMLTag *tag);
		int  PresenceClose(WokXMLTag *tag);
		int GUIWindowInit(WokXMLTag *tag);

		void UpdateStruts();
		void PopulateShowEntry();
		void SetActivePresence(int i);
	protected:
		GladeXML *xml;
		GladeXML *preferencexml;
		GtkWidget *mainwindowplug;
		
		gulong connected;
		guint priotimeid;
		guint storetimeid;
		
		int ReadConfig(WokXMLTag *tag);
		void SaveConfig ();
		void SetStatusTo(const std::string &status);

		std::map <GtkWidget *, std::string> MainMenuSignals;
		//std::map <std::string, int> SavedStatusMsges;
		std::list <std::string> ActiveSessions;
		std::vector <int> Widgets;
		std::string current_status;
		std::string status_show;
		
		GUIMessageHandler *gmsghandler;
		int connect_win_open;
		int plugin_win_open;
		int window_x, window_y;
		bool visible;

		GtkListStore *showmenu;

		WokXMLTag *config;
};


#endif	//_GUIWINDOW_H_
