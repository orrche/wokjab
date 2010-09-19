/***************************************************************************
 *  Copyright (C) 2005  Kent Gustavsson <oden@gmx.net>
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
// Class: DisplayWidget
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Thu Aug 11 10:46:23 2005
//

#ifndef _DISPLAYWIDGET_H_
#define _DISPLAYWIDGET_H_

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <list>

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#define _(x) x


using namespace Woklib;

/** 
 * The actual widget for displaying the Error/Debug/Message window
 */
class DisplayWidget : public WLSignalInstance
{
	public:
		DisplayWidget(WLSignal *wls);
		 ~DisplayWidget();
	
		void Create();
		void Show();
		void CreateXMLViewer(WokXMLTag *tag);

		void Error(WokXMLTag *tag);
		void Debug(WokXMLTag *tag);
		void Signal(WokXMLTag *tag);
		void InOut(WokXMLTag *tag);
		void Message(WokXMLTag *tag);
		
		static gboolean Delete( GtkWidget *widget, GdkEvent *event, DisplayWidget *c);
		static void DebugClear(GtkButton *button, DisplayWidget *c);
		static gboolean ShowDataButton (GtkButton *button, DisplayWidget *c);
		static void InputButton (GtkButton *button, DisplayWidget *c);
		static void SignalButton (GtkButton *button, DisplayWidget *c);
		static void SignalClear (GtkButton *button, DisplayWidget *c);
		static void IOClear (GtkButton *button, DisplayWidget *c);
	protected:
		GladeXML *xml;
		GtkWidget *message_treeview;
		GtkWidget *debug_treeview;
		GtkWidget *inout_treeview;
		GtkWidget *error_treeview;
		GtkWidget *signal_treeview;
		GtkListStore *signal_store;
		GtkListStore *message_store;
		GtkListStore *inout_store;
		GtkListStore *error_store;
		GtkListStore *debug_store;
		std::list <WokXMLTag *> signal_data;
};


#endif	//_DISPLAYWIDGET_H_

