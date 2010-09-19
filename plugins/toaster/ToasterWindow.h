/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <nedo80@gmail.com>
 ****************************************************************************/
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
// Class: ToasterWindow
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Aug  8 16:29:20 2006
//

#ifndef _TOASTERWINDOW_H_
#define _TOASTERWINDOW_H_

#include <gtk/gtk.h>

class ToasterWindow;

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

class ToasterWindow : public WLSignalInstance
{
	public:
		ToasterWindow(WLSignal *wls, WokXMLTag *config, WokXMLTag *xml, int x, int y);
		 ~ToasterWindow();
	
		int GetHeight();
		void MoveTo(int x, int y);
		std::string GetID();
		static gboolean Timeout(ToasterWindow *c);
		static gboolean TimeoutRemove(ToasterWindow *c);
		static gboolean button_press_event(GtkWidget *widget, GdkEventButton *event, ToasterWindow *c);
		static gboolean CommandExec(GtkWidget *button, GdkEventButton *event, ToasterWindow *c);
	protected:
		WokXMLTag *orig;
		GdkColor noticable_color1;
		GdkColor noticable_color2;
		GtkWidget *window;
		GtkWidget *port;
		int t;
		guint timeoutid;
		guint timeoutremoveid;
};


#endif	//_TOASTERWINDOW_H_

