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

#ifndef _GROUPCHATJOINWINDOW_H_
#define _GROUPCHATJOINWINDOW_H_


#include <gtk/gtk.h>
#include <glade/glade.h>

#include <Woklib/WLSignal.h>
#include <Woklib/WokLib.h>
using namespace Woklib;

class GroupChatJoinWindow : public WLSignalInstance
{
	public:
	GroupChatJoinWindow (WLSignal * wls);
	~GroupChatJoinWindow ();
	
	static void Destroy (GtkWidget * widget, GroupChatJoinWindow *c);
	static void Join_Button (GtkWidget * widget, GroupChatJoinWindow *c);
	static void Cancel_Button (GtkWidget * widget, GroupChatJoinWindow *c);
	static void QuickChange(GtkComboBox *widget, GroupChatJoinWindow *c);
	
	int Config(WokXMLTag *tag);
	protected:
	GladeXML *xml;
	GtkWidget *window;
	GtkWidget *roomentry;
	GtkWidget *nickentry;
	GtkWidget *serverentry;
	GtkWidget *sessionchooser;
	GtkWidget *quickchooser;
	WokXMLTag *config;
	
	GtkListStore *sessionmenu;
	GtkListStore *quickmenu;
};


#endif //_GROUPCHATJOINWINDOW_H_
