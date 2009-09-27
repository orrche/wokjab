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
#ifndef _GNOMETRAYICON_H_
#define _GNOMETRAYICON_H_

#include <list>

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>
#include <gtk/gtk.h>


using std::list;

using namespace Woklib;

/**
 * Class to control the trayicon
 */
class GnomeTrayIcon : public WoklibPlugin
{
	public:
		GnomeTrayIcon(WLSignal *wls);
		 ~GnomeTrayIcon();

		virtual std::string GetInfo() {return "Status Icon";};
		virtual std::string GetVersion() {return "0.0.1";};
	
		void ResetList();
		void UpdateList();
		 
		int AddJIDEvent( WokXMLTag *tag);
		int RemoveJIDEvent( WokXMLTag *tag);
		int Presence(WokXMLTag *tag);
		void UpdateTooltip();
				
		static void tray_icon_pressed (GtkWidget *button, GnomeTrayIcon *c);
		static void tray_popup (GtkWidget *widget, guint button, guint activate_time,GnomeTrayIcon *c);
		static void MenuActivate(GtkMenuItem *menuitem,GnomeTrayIcon *data);
		static gboolean TimeOut (GnomeTrayIcon * c);
	protected:
		GtkStatusIcon *tray_icon;
//		GtkWidget *eventbox;	
//		GtkTooltips *tray_icon_tips;
//		GtkWidget *image;
	
		gchar sz_tip[100];
		int to;
		
		std::string presence_tip;
		std::map <GtkWidget *, std::string> MainMenuSignals;
		list <WokXMLTag *> EventList;
		list <WokXMLTag *>::iterator CurrentEvent;
};


#endif	//_GNOMETRAYICON_H_
