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


#ifndef _AUTO_JOIN_WID_H_
#define _AUTO_JOIN_WID_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;

class AutoJoinWid;

#include "AutoJoin.h"
#include <gtk/gtk.h>
#include <glade/glade.h>

class AutoJoinWid : public WLSignalInstance
{
	public:
		AutoJoinWid(WLSignal *wls, WokXMLTag *tag, AutoJoin *parent);
		 ~AutoJoinWid();


		int BookmarkResponse(WokXMLTag *tag);
		int ReadConfig(WokXMLTag *tag);

		void SaveList();
		void SaveConfig();
		void ReloadList();

		static void cell_edited (GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text, AutoJoinWid *c);
		static void cell_toggled (GtkCellRendererText *cell, const gchar *path_string, AutoJoinWid *c);
		static void button_add (GtkButton *button, AutoJoinWid *c);
		static void button_delete (GtkButton *button, AutoJoinWid *c);
		static void button_apply ( GtkButton *button, AutoJoinWid *c);
		static void button_ok ( GtkButton *button, AutoJoinWid *c);
		static void button_cancel ( GtkButton *button, AutoJoinWid *c);
		static void button_reload ( GtkButton *button, AutoJoinWid *c);
		static gboolean destroy( GtkWidget *widget, GdkEvent *event, AutoJoinWid *c);
	protected:
		GladeXML *xml;
		GtkListStore *model;
		std::string session;
		WokXMLTag *config;
		AutoJoin *parent;
};


#endif	//_AUTO_JOIN_H_
