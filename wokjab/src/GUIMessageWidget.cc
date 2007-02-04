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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "include/GUIMessageWidget.h"
#include "include/main.h"

#include <iostream>
#include <fstream>
#include <string>
#include <gdk/gdkkeysyms.h>
#include <expat.h>
#include <gdk/gdk.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sstream>

using std::cout;
using std::endl;
using std::string;

GUIMessageWidget::GUIMessageWidget(WLSignal *wls, std::string session, std::string from, int id) : WLSignalInstance(wls),
session(session),
from(from)
{
	GtkTooltips *tooltip;
	GtkWidget *eventbox;
	GtkWidget *label_eventbox;

	eventbox = gtk_event_box_new();
	vbox = gtk_vbox_new( false, 0 );
	fontsize = 1;
	cmd_count = 1;
	
	gtk_container_add (GTK_CONTAINER(eventbox), vbox);
	GTK_WIDGET_SET_FLAGS (eventbox, GTK_CAN_FOCUS);
	tophbox = gtk_hbox_new( false, 0);
	vpaned = gtk_vpaned_new();
	label_image = gtk_image_new();
	image = gtk_image_new();

	gtk_container_set_border_width (GTK_CONTAINER (vpaned), 5);
	gtk_box_pack_start( GTK_BOX(vbox), tophbox, false, false, 0 );

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
	gtk_box_pack_start( GTK_BOX(tophbox), image, false, false, 5 );
	jid_label = gtk_label_new("");
	gtk_box_pack_start( GTK_BOX(tophbox), jid_label, TRUE, TRUE, 0);
	gtk_label_set_text(GTK_LABEL(jid_label), std::string(from + " (" + nick + ")").c_str());
	gtk_label_set_ellipsize(GTK_LABEL(jid_label), PANGO_ELLIPSIZE_END);
	gtk_misc_set_alignment (GTK_MISC (jid_label), 0, 0);
	/* Reading and writing area */
	gtk_box_pack_start( GTK_BOX( vbox) , vpaned, true, true, 0 );


	/* Reading Area */
	textview1 = gtk_text_view_new();
	scroll1 = gtk_scrolled_window_new(NULL, NULL);
	gtk_paned_pack1( GTK_PANED( vpaned) , scroll1, true, true);
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
	//GTK_WIDGET_UNSET_FLAGS (textview1, GTK_CAN_FOCUS);
	buffer2 = gtk_text_view_get_buffer (GTK_TEXT_VIEW(textview2));

	gtk_text_buffer_get_start_iter (buffer1, &iter);
	gtk_text_buffer_insert(buffer1, &iter, " ", 1);
	gtk_text_buffer_get_start_iter (buffer1, &iter);
	end_mark = gtk_text_buffer_create_mark (buffer1,"EndMark", &iter, FALSE);


	/* Message Icon */
	std::string datadir = PACKAGE_DATA_DIR;
	msgicon = datadir + "/wokjab/msg.png";
	pix_msg = gdk_pixbuf_new_from_file(msgicon.c_str(),NULL);

	gtk_widget_show_all( vbox );

	gtk_signal_connect (GTK_OBJECT (textview2), "key_press_event",
		    GTK_SIGNAL_FUNC (GUIMessageWidget::key_press_handler),
			   this);

	this->from = from;

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
	//
	gtk_signal_connect (GTK_OBJECT (label_eventbox), "button_press_event",
                      GTK_SIGNAL_FUNC (GUIMessageWidget::Menu), this);

	// Initiating the icon ...

	gtk_image_set_from_file(GTK_IMAGE(image), itemtag.GetAttr("icon").c_str());
	gtk_image_set_from_pixbuf(GTK_IMAGE(label_image), gtk_image_get_pixbuf(GTK_IMAGE(image)));

	// Assigning a color for the tab
	// Should make this a configurable color
	color_red.red = 66536;
 color_red.green = 0;
 color_red.blue = 0;

	gdk_color_parse ("red", &color_red);

	gtk_widget_show_all(vbox);

	secondmessageme = secondmessageother = false;
	GTK_WIDGET_SET_FLAGS (textview2, GTK_CAN_DEFAULT);


	g_signal_connect ((gpointer) textview2, "focus-in-event",
          G_CALLBACK (GUIMessageWidget::focus_event),this);
	g_signal_connect ((gpointer) textview2, "focus-out-event",
          G_CALLBACK (GUIMessageWidget::focus_event),this);
	g_signal_connect ((gpointer) textview2, "size-allocate",
          G_CALLBACK (GUIMessageWidget::SizeAllocate),this);
	g_signal_connect ((gpointer) eventbox, "focus-in-event",
          G_CALLBACK (GUIMessageWidget::focus_event_view),this);
	g_signal_connect ((gpointer) textview1, "scroll-event",
										G_CALLBACK (GUIMessageWidget::Scroll), this);
	g_signal_connect (textview1, "event-after", 
										G_CALLBACK (GUIMessageWidget::tw1_event_after), this);
	expander_activate (GTK_EXPANDER(expander), (void*)this);
	gtk_widget_grab_focus(textview1);


	/* Drag and drop .... */
	GtkTargetEntry target_entry[3];

	target_entry[0].target = "text/plain";
	target_entry[0].flags = 0;
	target_entry[0].info = 0;
	target_entry[1].target = "test/uri-list";
	target_entry[1].flags = 0;
	target_entry[1].info = 1;
	target_entry[2].target = "STRING";
	target_entry[2].flags = 0;
	target_entry[2].info = 2;

	gtk_drag_dest_set(eventbox, (GtkDestDefaults)(GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_HIGHLIGHT |GTK_DEST_DEFAULT_DROP), target_entry, 3, (GdkDragAction) (GDK_ACTION_MOVE | GDK_ACTION_COPY));

	g_signal_connect(eventbox, "drag_data_received",G_CALLBACK (GUIMessageWidget::DragDest),this);

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
	focus = false;


	if( from.find("/") == std::string::npos )
		hasresource = false;
	else
		hasresource = true;



	char buf[20];
	WokXMLTag contag(NULL, "connect");
	contag.AddAttr("identifier", from);
	contag.AddAttr("session", session);
	sprintf(buf, "%d", gtk_plug_get_id(GTK_PLUG(mainwindowplug)));
	contag.AddAttr("mainwidget", buf);
	sprintf(buf, "%d", gtk_plug_get_id(GTK_PLUG(labelplug)));
	contag.AddAttr("labelwidget", buf);

	wls->SendSignal("GUI WindowDock AddWidget",&contag);

	g_signal_connect ((gpointer) mainwindowplug, "destroy",
                   G_CALLBACK (GUIMessageWidget::Destroy),
                   this);
	std::stringstream sig;
	sig << "GUI WindowDock Close " << gtk_plug_get_id(GTK_PLUG(mainwindowplug));
	EXP_SIGHOOK(sig.str(), &GUIMessageWidget::Close, 500);


	HookSignals();
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
			
	gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (text_view), 
																																								GTK_TEXT_WINDOW_WIDGET,
																																								event->x, event->y, &x, &y);

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
					std::cout << *(c->commands[command]) << std::endl;
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
	
}

void
GUIMessageWidget::HookSignals()
{
	EXP_SIGHOOK("Jabber XML Message To " + session + " " + from, &GUIMessageWidget::NewMessage, 500);
	EXP_SIGHOOK("Jabber XML Presence To " + session + " " + from, &GUIMessageWidget::NewPresence, 500);
	EXP_SIGHOOK("Jabber GUI Message Activate " + session + " " + from, &GUIMessageWidget::Activate, 500);
}

void
GUIMessageWidget::UnHookSignals()
{
	EXP_SIGUNHOOK("Jabber XML Message To " + session + " " + from, &GUIMessageWidget::NewMessage, 500);
	EXP_SIGUNHOOK("Jabber XML Presence To " + session + " " + from, &GUIMessageWidget::NewPresence, 500);
	EXP_SIGUNHOOK("Jabber GUI Message Activate " + session + " " + from, &GUIMessageWidget::Activate, 500);
}

int
GUIMessageWidget::InsertCommand(WokXMLTag &tag)
{
	GtkTextIter treeiter;
	gtk_text_buffer_get_iter_at_mark(buffer1, &treeiter, end_mark);
	gtk_text_buffer_insert_with_tags (buffer1, &treeiter, tag.GetAttr("name").c_str(), tag.GetAttr("name").length(), tags["forreign_text"], NULL);
	
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

			gtk_label_set_text(GTK_LABEL(jid_label), std::string(from + " (" + nick + ")").c_str());
		}

		std::string stamp = "";
		WokXMLTag tag_body = tag->GetFirstTag("message");

		for( iter = tag_body.GetTagList("x").begin() ; iter != tag_body.GetTagList("x").end() ; iter++)
		{
			if( (*iter)->GetAttr("xmlns") == "jabber:x:delay")
				stamp = (*iter)->GetAttr("stamp");
		}

		if( stamp.size() )
		{
			struct tm tp;
	#if __WIN32
	#else
			strptime (stamp.c_str(), "%Y%m%dT%T", &tp);
	#endif
			Message(msgtag.GetFirstTag("body").GetBody(), msgtag.GetAttr("from"), mktime(&tp));
		}
		else
			Message(msgtag.GetFirstTag("body").GetBody(), msgtag.GetAttr("from"));

		tag->AddAttr("displayed", "true");

		if( ! focus )
		{
			WokXMLTag eventtag(NULL, "event");
			WokXMLTag &itemtag = eventtag.AddTag("item");

			itemtag.AddAttr("jid", tag->GetFirstTag("message").GetAttr("from"));
			itemtag.AddAttr("session", tag->GetAttr("session"));
			itemtag.AddAttr("icon", msgicon);
			itemtag.AddAttr("signal", "Jabber GUI MessageDialog Open");

			WokXMLTag &desc = itemtag.AddTag("description");
			desc.AddText(tag->GetFirstTag("message").GetAttr("from"));
			desc.AddText("\n\t");
			desc.AddText(tag->GetFirstTag("message").GetFirstTag("body").GetBody().substr(0, 30));
			if( tag->GetFirstTag("message").GetFirstTag("body").GetBody().size() > 30 )
				desc.AddText("...");

			wls->SendSignal("Jabber Event Add", &eventtag);

		}
	}
	for( iter = tag->GetTagList("command").begin() ; iter != tag->GetTagList("command").end() ; iter++)
	{
		InsertCommand(**iter);
	}
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

// To damned annoying...
//	Message( tag_presence.GetAttr("from") + " sends presence " + tag_presence.GetFirstTag("status").GetBody());

	return true;
}

int
GUIMessageWidget::Activate(WokXMLTag *tag)
{
	std::stringstream str;
	WokXMLTag acttag(NULL, "window");
	str << gtk_plug_get_id(GTK_PLUG(mainwindowplug));
	acttag.AddAttr("id", str.str());
	wls->SendSignal("GUI WindowDock Activate", &acttag);
	return true;
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
GUIMessageWidget::DragDest(GtkWidget *widget, GdkDragContext *dc,gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, GUIMessageWidget *data)
{
	WokXMLTag FileTransfearTag(NULL, "ProgramEXECTag");
	FileTransfearTag.AddTag("program").AddAttr("exec", "file_transfer_script");
	FileTransfearTag.GetFirstTag("program").AddTag("attribute").AddText((const char *)selection_data->data);

	std::cout << FileTransfearTag << std::endl;
	data->wls->SendSignal("Run Program", FileTransfearTag);

	std::cout << "I got some shit !! info: " << info << " x: " << x << " y: " << y << std::endl;
	std::cout << "S Data: " << selection_data->data << std::endl;
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
GUIMessageWidget::focus_event_view           (GtkWidget       *widget,
                                        GdkEventFocus *event,
                                        gpointer         user_data)
{
	GUIMessageWidget *data;
	data = static_cast <GUIMessageWidget *> (user_data);

	gtk_widget_grab_focus(data->textview2);
	return TRUE;
}

gboolean
GUIMessageWidget::focus_event(GtkWidget       *widget,
                             GdkEventFocus *event,
                             gpointer         user_data)
{
	GUIMessageWidget *data;
	data = static_cast <GUIMessageWidget *> (user_data);

	if(event->in)
	{
		data->focus = true;
		if(data->msg_waiting)
		{
			data->msg_waiting = false;

			gtk_widget_modify_fg(GTK_WIDGET(data->label_label), GTK_STATE_ACTIVE, NULL);
			gtk_widget_modify_fg(GTK_WIDGET(data->label_label), GTK_STATE_NORMAL, NULL);

			gtk_image_set_from_pixbuf(GTK_IMAGE(data->label_image), gtk_image_get_pixbuf(GTK_IMAGE(data->image)));

			WokXMLTag eventtag(NULL, "event");
			WokXMLTag &itemtag = eventtag.AddTag("item");
			itemtag.AddAttr("jid", data->from);
			itemtag.AddAttr("session", data->session);
			itemtag.AddAttr("icon", data->msgicon);
			itemtag.AddAttr("signal", "Jabber GUI MessageDialog Open");
			data->wls->SendSignal("Jabber Event Remove", &eventtag);

			gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (data->textview1),
				data->end_mark, .4, TRUE, 1, 1);

			gtk_widget_grab_focus(data->textview1);
		}
	}
	else
		data->focus = false;
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
GUIMessageWidget::Message(std::string str, time_t t)
{
	GtkTextBuffer *buffer;
	GtkTextIter iter;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(textview1));
	gtk_text_buffer_get_iter_at_mark(buffer, &iter, end_mark);
	str += '\n';

	gtk_text_buffer_insert_with_tags (
					buffer, &iter,
					GetTimeStamp(t).c_str(), -1,
					tags["timestamp"],
					NULL);
	gtk_text_buffer_insert_with_tags (
					buffer, &iter,
					str.c_str(), str.length(),
					tags["notice"],
					NULL);

	gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (textview1),
				      end_mark, .4, TRUE, 1, 1);
	return 1;
}

int
GUIMessageWidget::Message(std::string str, std::string jid, time_t t)
{
	int i = 0;
	if( str == "" )
		return 1;

	secondmessageme = false;

	if(!focus)
	{
		gtk_widget_modify_fg(GTK_WIDGET(label_label), GTK_STATE_ACTIVE, &color_red);
		gtk_widget_modify_fg(GTK_WIDGET(label_label), GTK_STATE_NORMAL, &color_red);
		gtk_image_set_from_pixbuf(GTK_IMAGE(label_image), pix_msg);
		msg_waiting = true;
	}

	std::string nick;
	std::string msg;

	if( str.substr(0,4) == "/me " )
	{
		nick += config->GetFirstTag("display").GetFirstTag("forreignname").GetFirstTag("pre_me").GetBody();
		nick += this->nick;
		nick += config->GetFirstTag("display").GetFirstTag("forreignname").GetFirstTag("post_me").GetBody();

		msg = str.substr(3) + '\n';
	}
	else
	{
		nick += config->GetFirstTag("display").GetFirstTag("forreignname").GetFirstTag("pre").GetBody();
		nick += this->nick;
		nick += config->GetFirstTag("display").GetFirstTag("forreignname").GetFirstTag("post").GetBody();

		if(secondmessageother)
			nick += config->GetFirstTag("display").GetFirstTag("forreignname").GetFirstTag("continuing").GetBody();

		msg = str + '\n';
	}

	GtkTextBuffer *buffer;
	GtkTextIter iter;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(textview1));
	gtk_text_buffer_get_iter_at_mark(buffer, &iter, end_mark);


	gtk_text_buffer_insert_with_tags (
					buffer, &iter,
					GetTimeStamp(t).c_str(), -1,
					tags["timestamp"],
					NULL);

	gtk_text_buffer_insert_with_tags (
					buffer, &iter,
					nick.c_str(), nick.length(),
					tags["forreign_name"],
					NULL);

	gtk_text_buffer_insert_with_tags (buffer, &iter, msg.c_str(), msg.length(), tags["forreign_text"], NULL);
	gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (textview1),
				      end_mark, .4, TRUE, 1, 1);

	secondmessageother = true;
	return 1;
}

void
GUIMessageWidget::own_message(std::string str, time_t t)
{
	secondmessageother = false;

	WokXMLTag querytag(NULL, "nick");
	querytag.AddAttr("session", session);
	wls->SendSignal("Jabber GetMyNick", &querytag);
	string myname = querytag.GetAttr("nick");

	WokXMLTag msgtag(NULL,"message");
	msgtag.AddAttr("session", session);
	WokXMLTag &mtag = msgtag.AddTag("message");
	mtag.AddAttr("to", from);
	mtag.AddAttr("type", "chat");
	mtag.AddTag("body").AddText(XMLisize(str));

	wls->SendSignal("Jabber XML Message Send", &msgtag);
	str = msgtag.GetFirstTag("message").GetFirstTag("body").GetBody();

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

	MessageWithEmotions(str);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(textview1));
	gtk_text_buffer_get_iter_at_mark(buffer, &iter, end_mark);

	gtk_text_buffer_insert_with_tags (
					buffer, &iter,
					GetTimeStamp(t).c_str(), -1,
					tags["timestamp"],
					NULL);

	gtk_text_buffer_insert_with_tags (
					buffer, &iter,
					nick.c_str(), nick.length(),
					tags["my_name"],
					NULL);

	msg = DeXMLisize(msg);
	gtk_text_buffer_insert_with_tags (buffer, &iter, msg.c_str(), msg.length(),tags["my_text"],NULL);
	gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (textview1),
				      end_mark, .4, TRUE, 1, 1);

	secondmessageme = true;
}


void
GUIMessageWidget::MessageWithEmotions(std::string msg)
{
	std::string::size_type spos, epos;
	spos = 0;

	while( ( spos = msg.find("::",spos) ) != std::string::npos )
	{
		spos+=2;
		if ( ( epos = msg.find("::", spos) ) != std::string::npos )
		{
			std::cout << "Should see if finding jabber emotion " << msg.substr( spos, epos-spos ) << std::endl;
		}
	}

	while( ( spos = msg.find("(",spos) ) != std::string::npos )
	{
		spos++;
		if ( ( epos = msg.find(")", spos) ) != std::string::npos )
		{
			std::cout << "Should see if finding msn emotion " << msg.substr( spos, epos-spos ) << std::endl;
		}
	}

}


gboolean
GUIMessageWidget::key_press_handler(GtkWidget * widget, GdkEventKey * event,
			     gpointer data)
{
	GUIMessageWidget *obj;
	obj = static_cast < GUIMessageWidget * > ( data );
	if ((event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
	    && !(event->state & GDK_CONTROL_MASK) && !(gtk_expander_get_expanded(GTK_EXPANDER(obj->expander))) ||
	(event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
	    && (event->state & GDK_CONTROL_MASK) && (gtk_expander_get_expanded(GTK_EXPANDER(obj->expander))) ||
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
GUIMessageWidget::GetTimeStamp(time_t t)
{
	struct tm *tm;
	char      buf[128];

	tm = localtime (&t);

	buf[0] = 0;

	strftime (buf, sizeof (buf), config->GetFirstTag("timestamp").GetFirstTag("format").GetBody().c_str(), tm);
	return buf;
}
