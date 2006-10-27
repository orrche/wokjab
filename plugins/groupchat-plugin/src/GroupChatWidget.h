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
#ifndef _GROUPCHATWIDGET_H_
#define _GROUPCHATWIDGET_H_

#include <gtk/gtk.h>
#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <map>

class GroupChatWidget;

#include "groupchat.h"
#include <glade/glade.h>

using namespace Woklib;

class GroupChatWidget : public WLSignalInstance
{
	public:
		GroupChatWidget (WLSignal *wls, std::string session, std::string roomjid, std::string mynick, GroupChat *mclass);
		~GroupChatWidget ();
		int GetWidget();
		int GetLabel();
	  
    int Message(WokXMLTag &tag_msg);
		int own_message(std::string msg );
		int Presence(WokXMLTag *tag);
		int Config(WokXMLTag *tag);
		int Close(WokXMLTag *tag);
		
		void Focus();
		void Defocus();
		
		static gboolean key_press_handler(GtkWidget * widget, GdkEventKey * event, gpointer data);
		static gboolean popup_menu(GtkTreeView *tree_view, GdkEventButton *event, GroupChatWidget *c);
		static gboolean focus_event (GtkWidget *widget, GdkEventFocus *event, GroupChatWidget *c);
		static gboolean refocus ( GtkWidget *widget, GdkEventFocus *event, GroupChatWidget *c);
		static gboolean Scroll (GtkWidget *widget, GdkEventScroll *event, GroupChatWidget *c);
		static void Destroy(GtkWidget *widget, GroupChatWidget *c);
		static void Minimize_button(GtkButton *button, GroupChatWidget *c);
	protected:
		std::string GetTimeStamp(time_t t);
		std::string GetTimeStamp(WokXMLTag &bodytag);
		void GetColor(std::string text_type, char *def = NULL);
		static gboolean subject_activation( GtkWidget *widget,
                              gpointer   data );
	
		GroupChat *mclass;
		bool focus;
		bool minimized;
		WokXMLTag *config;
		std::string session;
		std::string roomjid;
		std::string mynick;
		GdkColor color_red;
		GdkColor color_blue;
		GladeXML *xml;
		GtkWidget *occupantlist;
		GtkWidget *eventbox;
		GtkWidget *hpaned1;
		GtkWidget *vpaned2;
		GtkWidget *outputview;
		GtkWidget *inputview;
		GtkWidget *label_label;
		GtkWidget *label_eventbox;
		GtkWidget *subject_entry;
		GtkWidget *mainwindowplug;
		GtkWidget *labelplug;
		GtkTextBuffer *inputbuffer;
		GtkTextBuffer *outputbuffer;
		GtkTextMark *end_mark;
		GtkListStore *model;
		double fontsize;
		std::map<std::string, GtkTreeRowReference*> participants;
		std::map<std::string, GtkTextTag*> tags;
		std::map<std::string, GdkPixbuf *> roles;
		std::map<std::string, GdkPixbuf *> presence;
};


#endif //_GROUPCHATWIDGET_H_
