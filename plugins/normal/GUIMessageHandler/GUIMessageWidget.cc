/***************************************************************************
 *  Copyright (C) 2003-2008  Kent Gustavsson <nedo80@gmail.com>
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <gdk/gdkkeysyms.h>
#include <expat.h>
#include <gdk/gdk.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sstream>
#include <algorithm>

#include "GUIMessageWidget.h"

using std::string;

static gboolean DNDDragMotionCB(
        GtkWidget *widget, GdkDragContext *dc,
        gint x, gint y, guint t,
        gpointer data
)
{
	if((dc == NULL))
    	return(FALSE);

	gdk_drag_status(dc, GDK_ACTION_COPY, t);

	return(FALSE);
}

GUIMessageWidget::GUIMessageWidget(WLSignal *wls, std::string session, std::string in_from, int id) : WLSignalInstance(wls),
session(session),
from(in_from)
{
	GtkTooltips *tooltip;
	GtkWidget *eventbox;
	GtkWidget *label_eventbox;

	eventbox = gtk_event_box_new();
	vbox = gtk_vbox_new( FALSE, 0 );
	activitybox = gtk_vbox_new(FALSE,2);
	activitytable = gtk_table_new(0,0, FALSE);
	fontsize = 1;
	cmd_count = 1;
	
	gtk_container_add (GTK_CONTAINER(eventbox), vbox);
	GTK_WIDGET_SET_FLAGS (eventbox, GTK_CAN_FOCUS);
	tophbox = gtk_hbox_new( FALSE, 0);
	vpaned = gtk_vpaned_new();
	label_image = gtk_image_new();
	image = gtk_image_new();

	gtk_container_set_border_width (GTK_CONTAINER (vpaned), 5);
	gtk_box_pack_start( GTK_BOX(vbox), tophbox, FALSE, FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(vbox), activitybox, FALSE, FALSE, 0);
	
	WokXMLTag querytag(NULL, "query");
	WokXMLTag &itemtag = querytag.AddTag("item");
	itemtag.AddAttr("session", session);
	itemtag.AddAttr("jid", from);
	wls->SendSignal("Jabber Roster GetResource", &querytag);
	wls->SendSignal("Jabber GUI GetIcon", &querytag);

	if ( itemtag.GetAttr("nick") != "" )
		nick = itemtag.GetAttr("nick");
	else
		nick = from;

	/* Whatever to show the JID line at the top of the message window */
	gtk_box_pack_start( GTK_BOX(tophbox), image, FALSE, FALSE, 5 );
	jid_label = gtk_label_new("");
	gtk_box_pack_start( GTK_BOX(tophbox), jid_label, TRUE, TRUE, 0);
	gtk_label_set_ellipsize(GTK_LABEL(jid_label), PANGO_ELLIPSIZE_END);
	gtk_misc_set_alignment (GTK_MISC (jid_label), 0, 0);
	/* Reading and writing area */
	gtk_box_pack_start( GTK_BOX( vbox) , vpaned, TRUE, TRUE, 0 );


	/* Reading Area */
	textview1 = gtk_text_view_new();
	scroll1 = gtk_scrolled_window_new(NULL, NULL);
	incomming_box = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(incomming_box), scroll1, TRUE, TRUE, 2);
	gtk_paned_pack1( GTK_PANED( vpaned) , incomming_box, TRUE, TRUE);
	GtkTextIter iter;
	gtk_container_add (GTK_CONTAINER (scroll1), textview1);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW( scroll1 ), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroll1), GTK_SHADOW_IN);
	gtk_text_view_set_editable (GTK_TEXT_VIEW (textview1), FALSE);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview1), GTK_WRAP_WORD);
	buffer1 = gtk_text_view_get_buffer (GTK_TEXT_VIEW(textview1));

	
	
	/* Writing area */
	GtkWidget *hbox_writing;


	hbox_writing = gtk_hbox_new(false, 1);
	expander = gtk_expander_new("");
	gtk_signal_connect (GTK_OBJECT (expander), "activate",
		    GTK_SIGNAL_FUNC (GUIMessageWidget::expander_activate),
			    this);
	textview2 = gtk_text_view_new();
	scroll2 = gtk_scrolled_window_new(NULL, NULL);

	gtk_box_pack_start(GTK_BOX(hbox_writing), expander, false, false, 1);
	gtk_box_pack_start(GTK_BOX(hbox_writing), scroll2, true, true, 1);
	gtk_paned_pack2( GTK_PANED( vpaned ) , hbox_writing, false, true);
	gtk_container_add (GTK_CONTAINER (scroll2), textview2);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW( scroll2 ), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroll2), GTK_SHADOW_IN);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview2), GTK_WRAP_WORD);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textview1), FALSE);
	buffer2 = gtk_text_view_get_buffer (GTK_TEXT_VIEW(textview2));

	gtk_text_buffer_get_start_iter (buffer1, &iter);
	gtk_text_buffer_insert(buffer1, &iter, " ", 1);
	gtk_text_buffer_get_start_iter (buffer1, &iter);
	end_mark = gtk_text_buffer_create_mark (buffer1,"EndMark", &iter, FALSE);


	/* Message Icon */
	msgicon = PACKAGE_DATA_DIR"/wokjab/msg.png";
	pix_msg = gdk_pixbuf_new_from_file(msgicon.c_str(),NULL);

	gtk_widget_show_all( vbox );

	gtk_signal_connect (GTK_OBJECT (textview2), "key_press_event",
		    GTK_SIGNAL_FUNC (GUIMessageWidget::key_press_handler),
			   this);

	focus = false;
	msg_waiting = false;

	// Label for the notepad
	tooltip = gtk_tooltips_new();
	label_eventbox = gtk_event_box_new();
	label_hbox = gtk_hbox_new(false, 1);
	label_label = gtk_label_new(nick.c_str());
	gtk_tooltips_set_tip(tooltip, label_eventbox, std::string(from + " (" + nick + ")").c_str(),std::string(from + " (" + nick + ")").c_str());
	gtk_box_pack_start(GTK_BOX(label_hbox), label_image, true, true, 0);
	gtk_box_pack_start(GTK_BOX(label_hbox), label_label, true, true, 0);
	gtk_container_add(GTK_CONTAINER(label_eventbox), label_hbox);
	
	gtk_signal_connect (GTK_OBJECT (label_eventbox), "button_press_event",
                      GTK_SIGNAL_FUNC (GUIMessageWidget::Menu), this);

	// Initiating the icon ...

	gtk_image_set_from_file(GTK_IMAGE(image), itemtag.GetAttr("icon").c_str());
	gtk_image_set_from_pixbuf(GTK_IMAGE(label_image), gtk_image_get_pixbuf(GTK_IMAGE(image)));

	// Assigning a color for the tab
	// Should make this a configurable color
	gdk_color_parse ("red", &color_red);

	gtk_widget_show_all(vbox);

	secondmessageme = secondmessageother = false;
	
	g_signal_connect (textview2, "focus-in-event", 	G_CALLBACK (GUIMessageWidget::focus_event),this);
	g_signal_connect (textview2, "focus-out-event", G_CALLBACK (GUIMessageWidget::focus_event),this);
	g_signal_connect (textview1, "size-allocate", 	G_CALLBACK (GUIMessageWidget::SizeAllocate),this);
// These two are for autofocusing the input area when you start typing something
	g_signal_connect (eventbox , "key-press-event",	G_CALLBACK (GUIMessageWidget::key_press_event),this);
	g_signal_connect (textview1, "key-press-event", G_CALLBACK (GUIMessageWidget::key_press_event),this);
	
	g_signal_connect (textview1, "scroll-event", 	G_CALLBACK (GUIMessageWidget::Scroll), this);      				// Zooming
	g_signal_connect (textview1, "event-after", 	G_CALLBACK (GUIMessageWidget::tw1_event_after), this); // Command press
	expander_activate (GTK_EXPANDER(expander), (void*)this);

	/* Drag and drop .... */
	GtkTargetEntry target_entry[3];


	target_entry[0].target = (gchar*)"text/uri-list";
	target_entry[0].flags = 0;
	target_entry[0].info = 1;
	target_entry[1].target = (gchar*)"STRING";
	target_entry[1].flags = 0;
	target_entry[1].info = 2;
	target_entry[2].target = (gchar*)"text/plain";
	target_entry[2].flags = 0;
	target_entry[2].info = 0;
//	gtk_drag_dest_set(textview1, (GtkDestDefaults)(GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_DROP), target_entry, 3, (GdkDragAction) (GDK_ACTION_COPY));
	gtk_drag_dest_set(textview2, (GtkDestDefaults)(GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_DROP), target_entry, 3, (GdkDragAction) (GDK_ACTION_COPY));
	
	gtk_signal_connect(
			GTK_OBJECT(textview2), "drag_motion",
			GTK_SIGNAL_FUNC(DNDDragMotionCB),
			this
		);

//	g_signal_connect(textview1, "drag_data_received",G_CALLBACK (GUIMessageWidget::DragDest),this);
	g_signal_connect(textview2, "drag_data_received",G_CALLBACK (GUIMessageWidget::DragDest),this);
	g_signal_connect(textview2, "drag_drop",G_CALLBACK (GUIMessageWidget::DragDrop),this);

	mainwindowplug = gtk_plug_new(0);
	labelplug = gtk_plug_new(0);

	gtk_container_add(GTK_CONTAINER(mainwindowplug), eventbox);
	gtk_container_add(GTK_CONTAINER(labelplug), label_eventbox);

	gtk_widget_show_all(mainwindowplug);
	gtk_widget_show_all(labelplug);

	config = new WokXMLTag(NULL, "NULL");
	EXP_SIGHOOK("Config XML Change /chat/window", &GUIMessageWidget::Config, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/chat/window");
	wls->SendSignal("Config XML Trigger", &conftag);

	focus = true;
	
	HookSignals();
	
	/*
	WokXMLTag spooltag(NULL, "spool");
	spooltag.AddAttr("jid", from);
	wls->SendSignal("Jabber GUI Message GetSpool", &spooltag);

	std::list <WokXMLTag *>::iterator titer;
	std::list <WokXMLTag *> *list;
	list = &spooltag.GetFirstTag("spool").GetTags();
	for( titer = list->begin() ; titer != list->end() ; titer++)
	{
		if( (*titer)->GetName() == "message" )
			NewMessage(*titer);
		else if ((*titer)->GetName() == "precense")
			NewPresence(*titer);
	}
	*/
	
	focus = false;


	if( from.find("/") == std::string::npos )
	{
		from_no_resource = from;
		hasresource = false;
	}
	else
	{
		from_no_resource = from.substr(0, from.find("/"));
		hasresource = true;
	}


	char buf[20];
	WokXMLTag contag(NULL, "connect");
	if ( nick.empty() )
		contag.AddAttr("identifier", from);
	else
		contag.AddAttr("identifier", nick);
//	contag.AddAttr("session", session);
	sprintf(buf, "%d", gtk_plug_get_id(GTK_PLUG(mainwindowplug)));
	contag.AddAttr("id", buf);
	sprintf(buf, "%d", gtk_plug_get_id(GTK_PLUG(labelplug)));
	contag.AddAttr("labelid", buf);
	contag.AddAttr("type", "chat");
	contag.AddAttr("handle", "true");
	
	wls->SendSignal("Wokjab DockWindow Add",&contag);

	g_signal_connect ((gpointer) mainwindowplug, "destroy",
                   G_CALLBACK (GUIMessageWidget::Destroy),
                   this);
	std::stringstream sig;
	sig << "Wokjab DockWindow Close " << gtk_plug_get_id(GTK_PLUG(mainwindowplug));
	EXP_SIGHOOK(sig.str(), &GUIMessageWidget::Close, 500);

	gtk_widget_grab_focus(textview2);


	EXP_SIGHOOK("Jabber Event Add", &GUIMessageWidget::NewEvent, 1000);
	EXP_SIGHOOK("Jabber Event Remove", &GUIMessageWidget::RemoveEvent, 1000);
	
	EXP_SIGHOOK("Jabber UserActivityUpdate " + session + " '" + XMLisize(from_no_resource)+"'", &GUIMessageWidget::UserActivities, 1000);
	UserActivities(NULL);
	
	
	WokXMLTag history ("history");
	history.AddAttr("relation", from_no_resource);
	
	focus = true;
	
	std::string old_from_no_resource = from_no_resource;
	std::string old_from = from;
	bool old_hasresource = hasresource;
	
	wls->SendSignal("Jabber History GetLast", history);
	std::list<WokXMLTag *>::iterator hist_iter;
	for( hist_iter = history.GetFirstTag("history").GetTagList("row").begin() ; hist_iter != history.GetFirstTag("history").GetTagList("row").end() ; hist_iter++ )
	{
		if ( (*hist_iter)->GetFirstTag("xml").GetFirstTag("message").GetFirstTag("message").GetFirstTag("x", "jabber:x:delay").GetAttr("stamp").empty() )
		{
			time_t t;
			struct tm *tm;
			std::string str_time = (*hist_iter)->GetFirstTag("time").GetBody();
			t = atol(str_time.c_str());
			tm = gmtime(&t);
			
			
			char      buf[128];
			buf[0] = 0;
			strftime (buf, sizeof (buf),"%Y%m%dT%T", tm);
			
			(*hist_iter)->GetFirstTag("xml").GetFirstTag("message").GetFirstTag("message").GetFirstTag("x", "jabber:x:delay").AddAttr("stamp", buf);
		}
		
		
		if ( (*hist_iter)->GetFirstTag("xml").GetFirstTag("message").GetFirstTag("message").GetAttr("from") == "" )
			SentMessage(&((*hist_iter)->GetFirstTag("xml").GetFirstTag("message")));
		else
		{
			(*hist_iter)->GetFirstTag("xml").GetFirstTag("message").AddAttr("displayed", "false");
			NewMessage(&((*hist_iter)->GetFirstTag("xml").GetFirstTag("message")));
		}
	}		
	
	UnHookSignals();
	
	from = old_from;
	from_no_resource = old_from_no_resource;
	hasresource = old_hasresource;

	HookSignals();
	SetLabel();
}

GUIMessageWidget::~GUIMessageWidget()
{
	int vpos = textview2->allocation.height;
	char buf[20];
	sprintf(buf,"%d", vpos);
	config->GetFirstTag("window_spec").AddAttr("input_area_height", buf);

	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/chat/window");
	conftag.AddTag(config);

	EXP_SIGUNHOOK("Config XML Change /chat/window", &GUIMessageWidget::Config, 500);

	wls->SendSignal("Config XML Store", &conftag);
	wls->SendSignal("Config XML Save", &conftag);

	if( labelplug )
		gtk_widget_destroy(labelplug);
	if( mainwindowplug )
		gtk_widget_destroy(mainwindowplug);

	delete config;
}

int
GUIMessageWidget::Close(WokXMLTag *tag)
{
	delete this;
	return 1;
}

void
GUIMessageWidget::Destroy(GtkWidget *widget, GUIMessageWidget *c)
{
#warning FIXME
	c->mainwindowplug = NULL;
	//delete c;
}

gboolean
GUIMessageWidget::tw1_event_after (GtkWidget *text_view, GdkEvent  *ev, GUIMessageWidget *c)
{
	GtkTextIter start, end, iter;
	gint x, y;
	GdkEventButton *event;
	
	if (ev->type != GDK_BUTTON_RELEASE)
    return FALSE;

	event = (GdkEventButton *)ev;

	if (event->button != 1)
			return FALSE;
  
	gtk_text_buffer_get_selection_bounds (c->buffer1, &start, &end);
 if (gtk_text_iter_get_offset (&start) != gtk_text_iter_get_offset (&end))
   return FALSE;
			
	gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (text_view), GTK_TEXT_WINDOW_WIDGET,(gint)event->x, (gint)event->y, &x, &y);

	gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (text_view), &iter, x, y);

	GSList *tags = NULL, *tagp = NULL;

	tags = gtk_text_iter_get_tags (&iter);
	for (tagp = tags;  tagp != NULL;  tagp = tagp->next)
		{
			GtkTextTag *tag = GTK_TEXT_TAG(tagp->data);
			gint command = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (tag), "command"));

			if (command != 0)
			{
				if( c->commands.find(command) != c->commands.end() )
				{
					if ( !c->commands[command]->GetFirstTag("signal").GetTags().empty() )
					{
						c->wls->SendSignal(c->commands[command]->GetFirstTag("signal").GetAttr("name"), *(c->commands[command]->GetFirstTag("signal").GetTags().begin()));
					}
					break;
				}
			}
		}

	if (tags) 
		g_slist_free (tags);

	return TRUE;
}

int
GUIMessageWidget::SentMessage(WokXMLTag *tag)
{
	std::string str = tag->GetFirstTag("message").GetFirstTag("body").GetBody();
	
	WokXMLTag querytag(NULL, "nick");
	querytag.AddAttr("session", session);
	wls->SendSignal("Jabber GetMyNick", &querytag);
	string myname = querytag.GetAttr("nick");

	std::string nick;
	std::string msg;
	if( str.substr(0,4) == "/me " )
	{
		nick += config->GetFirstTag("display").GetFirstTag("myname").GetFirstTag("pre_me").GetBody();
		nick += myname;
		nick += config->GetFirstTag("display").GetFirstTag("myname").GetFirstTag("post_me").GetBody();

		msg = str.substr(3) + '\n';
	}
	else
	{
		nick += config->GetFirstTag("display").GetFirstTag("myname").GetFirstTag("pre").GetBody();

		if(config->GetFirstTag("display").GetFirstTag("myname").GetFirstTag("show").GetAttr("data") != "false")
			nick += myname;

		nick += config->GetFirstTag("display").GetFirstTag("myname").GetFirstTag("post").GetBody();

		if(secondmessageme)
			nick += config->GetFirstTag("display").GetFirstTag("myname").GetFirstTag("continuing").GetBody();

		msg = str + '\n';
	}

	GtkTextBuffer *buffer;
	GtkTextIter iter;


	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(textview1));
	gtk_text_buffer_get_iter_at_mark(buffer, &iter, end_mark);
	
	
	gtk_text_buffer_insert_with_tags (
					buffer, &iter,
					GetTimeStamp(tag).c_str(), -1,
					tags["timestamp"],
					NULL);

	gtk_text_buffer_insert_with_tags (
					buffer, &iter,
					nick.c_str(), nick.length(),
					tags["my_name"],
					NULL);

	gtk_text_buffer_insert_with_tags (buffer, &iter, msg.c_str(), msg.length(),tags["my_text"],NULL);
	gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (textview1),
				      end_mark, .4, TRUE, 1, 1);

	secondmessageme = true;
	
	std::list <WokXMLTag *>::iterator citer;
	for( citer = tag->GetTagList("command").begin() ; citer != tag->GetTagList("command").end() ; citer++)
	{
		InsertCommand(**citer);
	}
	
	return 1;
}

void
GUIMessageWidget::HookSignals()
{
	EXP_SIGHOOK("Jabber XML Message To '" + XMLisize(session) + "' '" + XMLisize(from) + "'", &GUIMessageWidget::SentMessage, 900);
	EXP_SIGHOOK("Jabber XML Message From " + session + " " + from, &GUIMessageWidget::NewMessage, 500);
	EXP_SIGHOOK("Jabber XML Presence From " + session + " " + from, &GUIMessageWidget::NewPresence, 500);
	EXP_SIGHOOK("Jabber GUI Message Activate " + session + " " + from, &GUIMessageWidget::Activate, 500);	
	
	
}

void
GUIMessageWidget::UnHookSignals()
{
	EXP_SIGUNHOOK("Jabber XML Message To '" + XMLisize(session) + "' '" + XMLisize(from) + "'", &GUIMessageWidget::SentMessage, 900);
	EXP_SIGUNHOOK("Jabber XML Message From " + session + " " + from, &GUIMessageWidget::NewMessage, 500);
	EXP_SIGUNHOOK("Jabber XML Presence From " + session + " " + from, &GUIMessageWidget::NewPresence, 500);
	EXP_SIGUNHOOK("Jabber GUI Message Activate " + session + " " + from, &GUIMessageWidget::Activate, 500);
}

int
GUIMessageWidget::UserActivities(WokXMLTag *tag)
{
	WokXMLTag entries("entries");
	entries.AddAttr("jid", from_no_resource);
	entries.AddAttr("session", session);
	
	wls->SendSignal("Jabber UserActivityGet", entries);
	wls->SendSignal("Jabber UserActivityGet " + session + " '" + XMLisize(from_no_resource) + "'", entries);
		
	std::list <WokXMLTag *>::iterator entry_iter;
	
	gtk_widget_destroy(activitytable);
	activitytable = gtk_table_new(entries.GetTagList("item").size(), 2, FALSE);
	gtk_box_pack_start(GTK_BOX(activitybox), activitytable, FALSE, 0,0);
	int n = 0;
	for( entry_iter = entries.GetTagList("item").begin() ; entry_iter != entries.GetTagList("item").end() ; entry_iter++)
	{
		GtkWidget *line = gtk_label_new((*entry_iter)->GetFirstTag("line").GetBody().c_str());
		GtkWidget *type = gtk_label_new(((*entry_iter)->GetAttr("label")+":").c_str());
			
		gtk_misc_set_alignment (GTK_MISC (line), 0, 0);
		gtk_misc_set_alignment (GTK_MISC (type), 0, 0);
		gtk_table_attach(GTK_TABLE(activitytable),
                                                         type,
                                                         0,
                                                         1,
                                                         n,
                                                         1+n,  (GtkAttachOptions)0, (GtkAttachOptions)0, 4,0);
		gtk_table_attach_defaults(GTK_TABLE(activitytable),
                                                         line,
                                                         1,
                                                         2,
                                                         n,
                                                         1+n);
		n++;
	}
	gtk_widget_show_all(activitytable);
	return 1;
}

int
GUIMessageWidget::InsertCommand(WokXMLTag &tag)
{
	GtkTextIter treeiter;
	gtk_text_buffer_get_iter_at_mark(buffer1, &treeiter, end_mark);
	gtk_text_buffer_insert_with_tags (buffer1, &treeiter, tag.GetAttr("name").c_str(), tag.GetAttr("name").length(), tags["forreign_text"], NULL);
	if ( !tag.GetTagList("message").empty() )
	{
		if ( !tag.GetFirstTag("message").GetBody().empty() )
			gtk_text_buffer_insert_with_tags (buffer1, &treeiter, tag.GetFirstTag("message").GetBody().c_str(), tag.GetFirstTag("message").GetBody().length(), tags["forreign_text"], NULL);
	
	}
	
	std::list<WokXMLTag *>::iterator iter;
	
	for( iter = tag.GetTagList("command").begin() ; iter != tag.GetTagList("command").end() ; iter++)
	{
		GtkTextTag *ctag;
 
		ctag = gtk_text_buffer_create_tag (buffer1, NULL, 
				    "foreground", "blue", 
				    "underline", PANGO_UNDERLINE_SINGLE, 
				    NULL);
	
		commands[cmd_count] = new WokXMLTag(**iter);
		g_object_set_data (G_OBJECT (ctag), "command", GINT_TO_POINTER (cmd_count++));
	
		gtk_text_buffer_insert_with_tags (buffer1, &treeiter, " ", 1, tags["forreign_text"], NULL);
		gtk_text_buffer_insert_with_tags (buffer1, &treeiter, (*iter)->GetAttr("name").c_str(), (*iter)->GetAttr("name").length(), ctag, NULL);
	}
	
	gtk_text_buffer_insert_with_tags (buffer1, &treeiter, "\n", 1, tags["forreign_text"], NULL);
	return 1;
}

int
GUIMessageWidget::NewMessage(WokXMLTag *tag)
{
	if( tag->GetAttr("displayed") == "true")
		return true;
	
	std::list<WokXMLTag *>::iterator iter;
	
	if( tag->GetFirstTag("message").GetFirstTag("body").GetBody() != "" )
	{
	
		WokXMLTag &msgtag = tag->GetFirstTag("message");

		if(!hasresource && tag->GetFirstTag("message").GetAttr("from").find("/") != std::string::npos)
		{
			UnHookSignals();
			from = tag->GetFirstTag("message").GetAttr("from");
			HookSignals();

			hasresource = true;

			SetLabel();
		}

		std::string stamp = "";
		WokXMLTag tag_body = tag->GetFirstTag("message");
		Message(*tag, msgtag.GetAttr("from"));

		tag->AddAttr("displayed", "true");

		if( ! focus )
		{
			WokXMLTag eventtag(NULL, "event");
			eventtag.AddAttr("type", "message");
			WokXMLTag &itemtag = eventtag.AddTag("item");

			//itemtag.AddAttr("id", "Message " + tag->GetAttr("session") + " " + tag->GetFirstTag("message").GetAttr("from"));
			itemtag.AddAttr("jid", tag->GetFirstTag("message").GetAttr("from"));
			itemtag.AddAttr("session", tag->GetAttr("session"));
			itemtag.AddAttr("icon", msgicon);

			WokXMLTag &desc = itemtag.AddTag("description");
			
			desc.AddText(nick);
			desc.AddText("\n\t");
			
			
			desc.AddText(tag->GetFirstTag("message").GetFirstTag("body").GetBody());

			WokXMLTag &command = itemtag.AddTag("commands").AddTag("command");
			command.AddAttr("name", "Open Dialog");
			WokXMLTag &sig = command.AddTag("signal");
			sig.AddAttr("name", "Jabber GUI MessageDialog Open");
			WokXMLTag &item = sig.AddTag("item");
			item.AddAttr("jid", tag->GetFirstTag("message").GetAttr("from"));
			item.AddAttr("session", tag->GetAttr("session"));
			
			wls->SendSignal("Jabber Event Add", &eventtag);
			eventidlist.push_back(eventtag.GetFirstTag("item").GetAttr("id"));
			
			WokXMLTag dockwid("dock");
			std::stringstream str;
			str << gtk_plug_get_id(GTK_PLUG(mainwindowplug));
			dockwid.AddAttr("id", str.str());
			dockwid.GetFirstTag("urgency").AddAttr("data", "true");
			wls->SendSignal("Wokjab DockWindow SetUrgencyHint", dockwid);

		}
	}
	for( iter = tag->GetTagList("command").begin() ; iter != tag->GetTagList("command").end() ; iter++)
	{
		InsertCommand(**iter);
	}

	return 1;
}

int
GUIMessageWidget::RemoveEvent(WokXMLTag *tag)
{
	std::list<WokXMLTag *>::iterator itemiter;
	for(itemiter = tag->GetTagList("item").begin() ; itemiter != tag->GetTagList("item").end(); itemiter++)
	{	
		std::map<GtkWidget *, WokXMLTag *>::iterator iter;
		std::map<GtkWidget *, WokXMLTag *>::iterator nxtiter;
		
		for( iter = event_list.begin() ; iter != event_list.end() ; )
		{
			if( (*itemiter)->GetAttr("id") == iter->second->GetAttr("id"))
			{
				nxtiter = iter;
				nxtiter++;
				
				std::list <GtkWidget *>::iterator list_iter;
				list_iter = std::find(event_list_order.begin(), event_list_order.end(), iter->first);

				if ( iter->first == *event_list_order.begin() )
				{
					event_list_order.erase(list_iter);
					if ( ! event_list_order.empty() )
					{	
						gtk_widget_show_all(*event_list_order.begin());
					}
				}
				else
					event_list_order.erase(list_iter);
				
				gtk_widget_destroy(iter->first);
				delete iter->second;
				event_list.erase(iter);
				iter = nxtiter;
			}
			else
				iter++;
			
			
		}
	}
	return 1;
}

int
GUIMessageWidget::NewEvent(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator itemiter;
	
	for( itemiter = tag->GetTagList("item").begin() ; itemiter != tag->GetTagList("item").end() ; itemiter++ )
	{	
		if ( (*itemiter)->GetAttr("jid") != from )
			return 1;
		GtkWidget *label = gtk_label_new((*itemiter)->GetFirstTag("description").GetBody().c_str());
		GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
		
		gtk_label_set_line_wrap (GTK_LABEL(label), TRUE);
				
		gtk_box_pack_end(GTK_BOX(incomming_box), vbox, FALSE, FALSE, 2 );
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
		std::list <WokXMLTag *> *com_list;
		std::list <WokXMLTag *>::iterator c_iter;
		
		
		
		WokXMLTag *copytag = new WokXMLTag(**itemiter);
		com_list = &copytag->GetTagList("commands");
		
		for( c_iter = com_list->begin() ; c_iter != com_list->end() ; c_iter++ )
		{
			GtkWidget *bbox;
			//GtkWidget *label;
			bbox = gtk_hbox_new(FALSE, 2);
			//label = gtk_label_new((*c_iter)->GetAttr("name").c_str());
			
			gtk_box_pack_start(GTK_BOX(vbox), bbox, FALSE, FALSE, 0);
			//gtk_box_pack_start(GTK_BOX(bbox), label, FALSE, FALSE, 0);
			
			std::list <WokXMLTag *> *list = &(*c_iter)->GetTagList("command");
			std::list <WokXMLTag *>::iterator iter;
			for ( iter = list->begin(); iter != list->end(); iter++)
			{
				GtkWidget *button = gtk_button_new_with_label((*iter)->GetAttr("name").c_str());
				
				gtk_box_pack_end(GTK_BOX(bbox), button, FALSE, FALSE, 0);
				g_object_set_data(G_OBJECT(button), "xml", *iter);
				
				g_signal_connect (button , "button-press-event",	G_CALLBACK (GUIMessageWidget::CommandExec),this);
			}
			
		}
		
		if( event_list_order.empty()) 
			gtk_widget_show_all(vbox);
		event_list[vbox] = copytag;
		event_list_order.push_back(vbox);
		
	}
	
	return 1;
}

gboolean
GUIMessageWidget::CommandExec(GtkWidget *button, GdkEventButton *event, GUIMessageWidget *c)
{
	WokXMLTag *tag = static_cast <WokXMLTag *> (g_object_get_data(G_OBJECT(button), "xml"));
	if ( tag && !tag->GetFirstTag("signal").GetTags().empty() )
	{
		WokXMLTag temptag(**tag->GetFirstTag("signal").GetTags().begin());
		c->wls->SendSignal(tag->GetFirstTag("signal").GetAttr("name"), temptag);
		
		return TRUE;
	}

	return FALSE;
}

int
GUIMessageWidget::NewPresence(WokXMLTag *tag)
{	
	
	WokXMLTag &tag_presence = tag->GetFirstTag("presence");
	WokXMLTag querytag(NULL,"query");
	WokXMLTag &itemtag = querytag.AddTag("item");

	itemtag.AddAttr("session", session);
	itemtag.AddAttr("jid", from);

	wls->SendSignal("Jabber GUI GetIcon", &querytag);

	gtk_image_set_from_file(GTK_IMAGE(image), querytag.GetFirstTag("item").GetAttr("icon").c_str());

	if(!msg_waiting)
		gtk_image_set_from_pixbuf(GTK_IMAGE(label_image), gtk_image_get_pixbuf(GTK_IMAGE(image)));

		if ( tag_presence.GetAttr("type") == "unavailable" && hasresource)
		{
			UnHookSignals();
			if ( from.find("/") != std::string::npos )
				from = from_no_resource;
			hasresource = false;

			HookSignals();


		}
// To damned annoying...
//	Message( tag_presence.GetAttr("from") + " sends presence " + tag_presence.GetFirstTag("status").GetBody());
	
	SetLabel();
	return true;
}

int
GUIMessageWidget::Activate(WokXMLTag *tag)
{
	std::stringstream str;
	WokXMLTag acttag(NULL, "window");
	str << gtk_plug_get_id(GTK_PLUG(mainwindowplug));
	acttag.AddAttr("id", str.str());
	wls->SendSignal("Wokjab DockWindow Activate", &acttag);
	return true;
}

void
GUIMessageWidget::SetLabel()
{
	WokXMLTag querytag(NULL, "query");
	WokXMLTag &itemtag = querytag.AddTag("item");
	itemtag.AddAttr("session", session);
	itemtag.AddAttr("jid", from);
	wls->SendSignal("Jabber Roster GetResource", &querytag);
	wls->SendSignal("Jabber GUI GetIcon", &querytag);
	
	std::string string = from + " (" + nick + ") [" + itemtag.GetFirstTag("resource").GetAttr("priority") + "]";
	if ( !itemtag.GetFirstTag("resource").GetFirstTag("status").GetBody().empty() )
	{
		string += "\n" +itemtag.GetFirstTag("resource").GetFirstTag("status").GetBody();
	}
	gtk_label_set_text(GTK_LABEL(jid_label), string.c_str());
}

gboolean
GUIMessageWidget::Scroll (GtkWidget *widget, GdkEventScroll *event, GUIMessageWidget *c)
{
	if ( event->state & GDK_CONTROL_MASK )
	{
		switch ( event->direction )
		{
			case GDK_SCROLL_UP:
				c->fontsize *= 1.1;
				break;
			case GDK_SCROLL_DOWN:
				c->fontsize /= 1.1;
				break;
			default:
				return FALSE;
		}

		GtkTextIter startiter;
		GtkTextIter enditer;
		gtk_text_buffer_get_start_iter(c->buffer1, &startiter);
		gtk_text_buffer_get_end_iter(c->buffer1, &enditer);
		GtkTextTagTable *tag_table =  gtk_text_buffer_get_tag_table (c->buffer1);
		GtkTextTag *oldtag = gtk_text_tag_table_lookup(tag_table, "fontsize");
		if ( oldtag )
			gtk_text_tag_table_remove(tag_table, oldtag);

		GtkTextTag *tag = gtk_text_buffer_create_tag (c->buffer1, "fontsize","scale", c->fontsize, NULL);
		gtk_text_buffer_remove_tag_by_name (c->buffer1, "fontsize", &startiter, &enditer);
		gtk_text_buffer_apply_tag (c->buffer1, tag, &startiter, &enditer);
		return TRUE;
	}
	return FALSE;
}

gboolean
GUIMessageWidget::DragDrop(GtkWidget *widget, GdkDragContext *dc, gint x, gint y, guint time, GUIMessageWidget *c)
{
	return TRUE;
}

gboolean
GUIMessageWidget::DragDest(GtkWidget *widget, GdkDragContext *dc,gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, GUIMessageWidget *c)
{
	gchar **list = gtk_selection_data_get_uris(selection_data);
	if ( list )
	{
		int x = 0;
		while ( list[x] )
		{
			gchar *file;
			file = g_filename_from_uri (list[x], NULL, NULL);
			
			WokXMLTag send(NULL, "send");
			send.AddAttr("to", c->from);
			send.AddAttr("session", c->session);
			send.AddAttr("name", file);
			send.AddAttr("proxy_type", "auto");
			c->wls->SendSignal("Jabber Stream File Send", send);
			g_free(file);
			x++;			
		}
	 
		g_strfreev(list);
	
		return TRUE;
	}
	
	return FALSE;
}


gboolean
GUIMessageWidget::Menu(GtkButton *button, GdkEventButton *event, GUIMessageWidget *c)
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 3 )
	{
		char buf[20];
		WokXMLTag MenuXML(NULL, "menu");
		sprintf(buf, "%d", event->button);
		MenuXML.AddAttr("button", buf);
		sprintf(buf, "%d", event->time);
		MenuXML.AddAttr("time", buf);
		MenuXML.AddTag("item").AddAttr("signal", "Jabber GUI Roster GetJIDMenu");
		MenuXML.AddTag("item").AddAttr("signal", "Jabber GUI GetJIDMenu NoRoster");
		MenuXML.AddTag("item").AddAttr("signal", "Jabber GUI GetJIDMenu");

		WokXMLTag &data = MenuXML.AddTag("data");
		data.AddAttr("jid", c->from);
		data.AddAttr("session", c->session);
		c->wls->SendSignal("Jabber GUI JIDMenu", &MenuXML);

		return true;
	}
	return false;
}

void
GUIMessageWidget::SizeAllocate (GtkWidget *widget,GtkRequisition *requisition,gpointer user_data)
{
	GUIMessageWidget *data;
	data = static_cast <GUIMessageWidget *> (user_data);

	gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (data->textview1),
				data->end_mark, .4, TRUE, 1, 1);

}

gboolean
GUIMessageWidget::focus_event(GtkWidget *widget, GdkEventFocus *event, GUIMessageWidget *c)
{

	if(event->in)
	{
		c->focus = true;
		if(c->msg_waiting)
		{
			c->msg_waiting = false;

			gtk_widget_modify_fg(GTK_WIDGET(c->label_label), GTK_STATE_ACTIVE, NULL);
			gtk_widget_modify_fg(GTK_WIDGET(c->label_label), GTK_STATE_NORMAL, NULL);

			gtk_image_set_from_pixbuf(GTK_IMAGE(c->label_image), gtk_image_get_pixbuf(GTK_IMAGE(c->image)));

			std::list <std::string>::iterator eventiter;
			for( eventiter = c->eventidlist.begin() ; eventiter != c->eventidlist.end() ; eventiter++)
			{
				WokXMLTag eventtag(NULL, "event");
				WokXMLTag &itemtag = eventtag.AddTag("item");
				itemtag.AddAttr("id", *eventiter);
				c->wls->SendSignal("Jabber Event Remove", &eventtag);
			}
			c->eventidlist.clear();
			gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (c->textview1),
				c->end_mark, .4, TRUE, 1, 1);
			
			
						
			WokXMLTag dockwid("dock");
			std::stringstream str;
			str << gtk_plug_get_id(GTK_PLUG(c->mainwindowplug));
			dockwid.AddAttr("id", str.str());
			dockwid.GetFirstTag("urgency").AddAttr("data", "false");
			c->wls->SendSignal("Wokjab DockWindow SetUrgencyHint", dockwid);
		}
	}
	else
		c->focus = false;
	return FALSE;
}

gboolean
GUIMessageWidget::expander_activate (GtkExpander *expander, gpointer user_data)
{
	GUIMessageWidget *data;
	data = static_cast <GUIMessageWidget*> (user_data);

	if(gtk_expander_get_expanded(expander))
	{
		gtk_paned_set_position(GTK_PANED(data->vpaned), data->vpaned->allocation.height - 40);
	}
	else
	{
		gtk_paned_set_position(GTK_PANED(data->vpaned), data->vpaned->allocation.height - 40 - 70);
	}
	return TRUE;
}

int
GUIMessageWidget::Config(WokXMLTag *tag)
{
	delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));

	std::list <WokXMLTag *>::iterator iter;
	std::list <WokXMLTag *> *list;

	list = &config->GetFirstTag("color").GetTags();
	for( iter = list->begin() ; iter != list->end(); iter++)
	{
		tags[(*iter)->GetName()] = gtk_text_buffer_create_tag( buffer1, NULL, "foreground", (*iter)->GetAttr("data").c_str(), NULL);
	}
	
	// Setting paned position
	int vpos = atoi(config->GetFirstTag("window_spec").GetAttr("input_area_height").c_str());
	if(vpos)
		gtk_widget_set_size_request(textview2, -1,vpos);


	if( config->GetFirstTag("jidbar").GetFirstTag("show").GetAttr("data") != "false" )
		gtk_widget_show(tophbox);
	else
		gtk_widget_hide(tophbox);
	return true;
}

int
GUIMessageWidget::PutText(GtkTextIter *iter, WokXMLTag &message)
{
	GtkTextBuffer *buffer;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(textview1));

	std::list <WokXMLObject *>::iterator oiter;
	for ( oiter = message.GetItemList().begin() ; oiter != message.GetItemList().end() ; oiter++)
	{
		switch ( (*oiter)->GetType() )
		{
			case 1:
				WokXMLTag *tag;
				tag = (WokXMLTag *)(*oiter);

				if ( tag->GetName() == "img")	
				{					
					GdkPixbuf *pic = gdk_pixbuf_new_from_file(tag->GetAttr("src").c_str(), NULL);
					if ( pic ) 
						gtk_text_buffer_insert_pixbuf (buffer, iter, pic);
					
				}
				if( tag->GetName() == "span")
				{
					GtkTextIter start_iter;

					GtkTextMark *mark;
					mark = gtk_text_buffer_create_mark(buffer, NULL, iter, TRUE);


					PutText(iter, *((WokXMLTag *)(*oiter)));
					gtk_text_buffer_get_iter_at_mark (buffer, &start_iter, mark);
					
					std::string style = tag->GetAttr("style");
					while( style.find(";") != std::string::npos )
					{
						int n = 0;
						for(n=0;style[n] == ' ';n++);
						style = style.substr(n);
						
						if ( style.substr(0,6) == "color:" )
						{
							std::string value = style.substr(6, style.find(";")-6);
							for(n=0;value[n] == ' ';n++);
							value = value.substr(n);
							
							GtkTextTag *tmptag;
							tmptag = gtk_text_buffer_create_tag( buffer1 , NULL, "foreground", value.c_str(), NULL);
							gtk_text_buffer_apply_tag(buffer, tmptag, &start_iter, iter);
						}
						style = style.substr(style.find(";")+1);
					}

					gtk_text_buffer_delete_mark (buffer, mark);
				}
				else
				{
					GtkTextIter start_iter;

					GtkTextMark *mark;
					mark = gtk_text_buffer_create_mark(buffer, NULL, iter, TRUE);


					PutText(iter, *((WokXMLTag *)(*oiter)));
					gtk_text_buffer_get_iter_at_mark (buffer, &start_iter, mark);
					gtk_text_buffer_apply_tag(buffer,
	                                                         tags["timestamp"],
	                                                         &start_iter,
	                                                         iter);

					gtk_text_buffer_delete_mark (buffer, mark);
				}
				break;
				
			case 2:
				WokXMLText *tt;
				tt = (WokXMLText *)(*oiter);
				gtk_text_buffer_insert_with_tags (buffer, iter, tt->GetText().c_str(), tt->GetText().length(), tags["forreign_text"], NULL);
				break;

			default:
				break;
		}
	}

	return 1;
}


int
GUIMessageWidget::Message(WokXMLTag &message)
{
	GtkTextBuffer *buffer;
	GtkTextIter iter;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(textview1));
	gtk_text_buffer_get_iter_at_mark(buffer, &iter, end_mark);

	gtk_text_buffer_insert_with_tags (
					buffer, &iter,
					GetTimeStamp(&message).c_str(), -1,
					tags["timestamp"],
					NULL);
	if ( ! message.GetTagList("body").empty() )
		PutText(&iter, message.GetFirstTag("body"));
	else
		PutText(&iter, message.GetFirstTag("message").GetFirstTag("body"));

	gtk_text_buffer_insert_with_tags (
					buffer, &iter,
					"\n", 1,
					tags["notice"],
					NULL);

	gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (textview1),
				      end_mark, .4, TRUE, 1, 1);


	return 1;
}

int
GUIMessageWidget::Message(WokXMLTag &message, std::string jid)
{
	secondmessageme = false;

	if(!focus)
	{
		gtk_widget_modify_fg(GTK_WIDGET(label_label), GTK_STATE_ACTIVE, &color_red);
		gtk_widget_modify_fg(GTK_WIDGET(label_label), GTK_STATE_NORMAL, &color_red);
		gtk_image_set_from_pixbuf(GTK_IMAGE(label_image), pix_msg);
		msg_waiting = true;
	}

	std::string nick;

	if( message.GetFirstTag("message").GetFirstTag("body").GetBody().substr(0,4) == "/me " )
	{
		nick += config->GetFirstTag("display").GetFirstTag("forreignname").GetFirstTag("pre_me").GetBody();
		nick += this->nick;
		nick += config->GetFirstTag("display").GetFirstTag("forreignname").GetFirstTag("post_me").GetBody();
	}
	else
	{
		nick += config->GetFirstTag("display").GetFirstTag("forreignname").GetFirstTag("pre").GetBody();
		nick += this->nick;
		nick += config->GetFirstTag("display").GetFirstTag("forreignname").GetFirstTag("post").GetBody();

		if(secondmessageother)
			nick += config->GetFirstTag("display").GetFirstTag("forreignname").GetFirstTag("continuing").GetBody();
	}

	GtkTextBuffer *buffer;
	GtkTextIter iter;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(textview1));
	gtk_text_buffer_get_iter_at_mark(buffer, &iter, end_mark);




	gtk_text_buffer_insert_with_tags (
					buffer, &iter,
					GetTimeStamp(&message).c_str(), -1,
					tags["timestamp"],
					NULL);

	if ( message.GetFirstTag("message").GetAttr("type") != "error")
	{
		gtk_text_buffer_insert_with_tags (
						buffer, &iter,
						nick.c_str(), nick.length(),
						tags["forreign_name"],
						NULL);
	}
	
	if ( ! message.GetTagList("body").empty() )
		PutText(&iter, message.GetFirstTag("body"));
	else
		PutText(&iter, message.GetFirstTag("message").GetFirstTag("body"));

	gtk_text_buffer_insert_with_tags (buffer, &iter, "\n", 1, tags["forreign_text"], NULL);
	gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (textview1),
				      end_mark, .4, TRUE, 1, 1);

	secondmessageother = true;
	return 1;
}

void
GUIMessageWidget::own_message(std::string str)
{
	secondmessageother = false;

	WokXMLTag msgtag(NULL,"message");
	msgtag.AddAttr("session", session);
	WokXMLTag &mtag = msgtag.AddTag("message");
	mtag.AddAttr("to", from);
	mtag.AddAttr("type", "chat");
	mtag.AddTag("body").AddText(str);

	wls->SendSignal("Jabber XML Message Send", &msgtag);
	
}

gboolean
GUIMessageWidget::key_press_event(GtkWidget * widget, GdkEventKey * event, GUIMessageWidget *c)
{
	if (event->keyval == GDK_v && (event->state & GDK_CONTROL_MASK))
		return FALSE;
	else if ( event->state & GDK_CONTROL_MASK )
		return FALSE;
	else if (event->keyval == GDK_Control_L || event->keyval == GDK_Control_R )
		return FALSE;
	gtk_widget_grab_focus(c->textview2);
		
	gint ret;
	g_signal_emit_by_name(G_OBJECT(c->textview2), "key-press-event", event, &ret, -1);
    return ret;
	return TRUE;
}

gboolean
GUIMessageWidget::key_press_handler(GtkWidget * widget, GdkEventKey * event,
			     gpointer data)
{
	GUIMessageWidget *obj;
	obj = static_cast < GUIMessageWidget * > ( data );
	if ((event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
	    && !((event->state & GDK_CONTROL_MASK) || (event->state & GDK_SHIFT_MASK)) && !(gtk_expander_get_expanded(GTK_EXPANDER(obj->expander))) ||
	((event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
	    && (event->state & GDK_CONTROL_MASK) && (gtk_expander_get_expanded(GTK_EXPANDER(obj->expander)))) ||
			( event->state & GDK_MOD1_MASK && event->keyval == GDK_s))
	{
		std::string str;
		GtkTextIter start_iter, end_iter;

		gtk_text_buffer_get_start_iter(obj->buffer2, &start_iter);
		gtk_text_buffer_get_end_iter(obj->buffer2, &end_iter);

		str = gtk_text_buffer_get_text(obj->buffer2,&start_iter, &end_iter, false);
		gtk_text_buffer_set_text(obj->buffer2, "", 0 );
		obj->own_message(str);
		return TRUE;
	}

	return FALSE;
}

std::string
GUIMessageWidget::GetTimeStamp(WokXMLTag *tag)
{
	struct tm *tm;
	char      buf[128];
	std::string stamp;
	time_t t;
	
	if ( !tag->GetFirstTag("message").GetTagList("x", "jabber:x:delay").empty())
		stamp = tag->GetFirstTag("message").GetFirstTag("x", "jabber:x:delay").GetAttr("stamp");

	if ( stamp.empty() )
		t = time(0);
	else
	{
		struct tm tp;
		strptime (stamp.c_str(), "%Y%m%dT%T", &tp);

		t = timegm(&tp);
	}
	
	tm = localtime (&t);

	buf[0] = 0;

	strftime (buf, sizeof (buf), config->GetFirstTag("timestamp").GetFirstTag("format").GetBody().c_str(), tm);
	return buf;
}
