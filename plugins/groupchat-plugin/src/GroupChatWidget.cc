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
#include "GroupChatWidget.h"
#include "ownerwidget.h"
#include <iostream>
#include <gdk/gdkkeysyms.h>
#include <sstream>


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

using std::cout;
using std::endl;

GroupChatWidget::GroupChatWidget (WLSignal * wls, std::string session, std::string roomjid , std::string mynick, GroupChat *mclass) : WLSignalInstance(wls),
mclass(mclass),
session(session)
{
	std::cout << "Starting a groupchat widgget" << std::endl;
	this->roomjid = roomjid;
	this->mynick = mynick;
	fontsize = 1;
	minimized = false;
	
	xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/groupchat.glade", "groupchat", NULL);
	inputview = glade_xml_get_widget (xml, "input");
	outputview = glade_xml_get_widget(xml, "view");
	occupantlist = glade_xml_get_widget(xml, "occupantlist");
	eventbox = glade_xml_get_widget(xml, "groupchat");
	subject_entry = glade_xml_get_widget(xml, "subject");
	
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTooltips *tooltip;
	
	g_signal_connect ((gpointer) inputview, "focus-in-event",
                    G_CALLBACK (GroupChatWidget::focus_event),this);
	g_signal_connect ((gpointer) inputview, "focus-out-event",
                    G_CALLBACK (GroupChatWidget::focus_event),this);
	g_signal_connect ((gpointer) eventbox, "focus-in-event",
										G_CALLBACK (GroupChatWidget::refocus), this);
	gtk_signal_connect (GTK_OBJECT (inputview), "key_press_event",
										GTK_SIGNAL_FUNC (GroupChatWidget::key_press_handler), this);
	g_signal_connect((gpointer) outputview, "scroll-event",
										G_CALLBACK (GroupChatWidget::Scroll), this);
										
	outputbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(outputview));
	inputbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(inputview));
		
	GtkTextIter iter;
	gtk_text_buffer_get_start_iter (outputbuffer, &iter);
	gtk_text_buffer_insert(outputbuffer, &iter, " ", 1);
	gtk_text_buffer_get_start_iter (outputbuffer, &iter);
	end_mark = gtk_text_buffer_create_mark (outputbuffer,"EndMark", &iter, FALSE);

	// The liste views settings
	model = gtk_list_store_new (4, G_TYPE_STRING, G_TYPE_STRING, GDK_TYPE_PIXBUF, GDK_TYPE_PIXBUF);
				  
	renderer = gtk_cell_renderer_pixbuf_new ();
	column = gtk_tree_view_column_new_with_attributes ("status",
							 renderer,
							 "pixbuf",
							 2,
							 NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(occupantlist), column);
	renderer = gtk_cell_renderer_pixbuf_new ();
	column = gtk_tree_view_column_new_with_attributes ("Away",
							 renderer,
							 "pixbuf",
							 3,
							 NULL);				 
	gtk_tree_view_append_column (GTK_TREE_VIEW(occupantlist), column);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Nick",
							 renderer,
							 "text",
							 0,
							 NULL);				 
	gtk_tree_view_append_column (GTK_TREE_VIEW(occupantlist), column);
	
	gtk_tree_view_set_model (GTK_TREE_VIEW(occupantlist), GTK_TREE_MODEL (model));

	g_signal_connect (G_OBJECT (occupantlist), "button_press_event", 
		G_CALLBACK (GroupChatWidget::popup_menu), this);
	// Colors
	
	// Assigning a color for the tab
	// Should make this a configurable color
	color_red.red = 66535/2;
	color_red.green = 0;
	color_red.blue = 0;
	gdk_color_parse ("red", &color_red);
	color_blue.red = 0;
	color_blue.green = 0;
	color_blue.blue = 66535/2;
	gdk_color_parse ("blue", &color_blue);
	
	// The label
	GtkWidget *label_hbox;
	GtkWidget *label_image;
	GtkWidget *minimize_button;
	label_image = gtk_image_new_from_file(PACKAGE_DATA_DIR"/wokjab/groupchat/wokjab/gicon.png");
	label_hbox = gtk_hbox_new(FALSE, 2);
	tooltip = gtk_tooltips_new();
	label_eventbox = gtk_event_box_new();
	label_label = gtk_label_new(roomjid.substr(0, roomjid.find("@")).c_str());
	minimize_button = gtk_button_new_with_label("_");
	gtk_box_pack_start(GTK_BOX(label_hbox), label_image, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(label_hbox), label_label, TRUE, TRUE, 0);
	gtk_box_pack_end(GTK_BOX(label_hbox), minimize_button, FALSE, FALSE, 0);
	gtk_tooltips_set_tip(tooltip, label_eventbox, roomjid.c_str(), roomjid.c_str());
	gtk_container_add(GTK_CONTAINER(label_eventbox), label_hbox);
	g_signal_connect((gpointer) minimize_button, "clicked",
									G_CALLBACK (GroupChatWidget::Minimize_button), this);
	
	
	Defocus();
	
	config = new WokXMLTag (NULL, "NULL");
	EXP_SIGHOOK("Config XML Change /groupchat/window", &GroupChatWidget::Config, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/groupchat/window");
	wls->SendSignal("Config XML Trigger", &conftag);
		
	mainwindowplug = gtk_plug_new(0);
	labelplug = gtk_plug_new(0);
	
	gtk_container_add(GTK_CONTAINER(mainwindowplug), eventbox);
	gtk_container_add(GTK_CONTAINER(labelplug), label_eventbox);
	
	g_object_ref(eventbox);
	g_object_ref(label_eventbox);
	
	gtk_widget_show_all(mainwindowplug);
	gtk_widget_show_all(labelplug);
	
	/* Uhm...
	std::stringstream sig;
	EXP_SIGHOOK(sig.str(), &GroupChatWidget::Close, 500);	
	*/
	
	GError *err = NULL;
	roles["moderator"] = gdk_pixbuf_new_from_file(PACKAGE_DATA_DIR"/wokjab/groupchat/wokjab/moderator.png", &err);
	roles["participant"] = gdk_pixbuf_new_from_file(PACKAGE_DATA_DIR"/wokjab/groupchat/wokjab/participant.png", &err);
	roles["visitor"] = gdk_pixbuf_new_from_file(PACKAGE_DATA_DIR"/wokjab/groupchat/wokjab/visitor.png", &err);
	
	presence[""] = gdk_pixbuf_new_from_file(PACKAGE_DATA_DIR"/wokjab/online.png", &err);
	presence["away"] = gdk_pixbuf_new_from_file(PACKAGE_DATA_DIR"/wokjab/away.png", &err);
	presence["dnd"] = gdk_pixbuf_new_from_file(PACKAGE_DATA_DIR"/wokjab/dnd.png", &err);
	presence["xa"] = gdk_pixbuf_new_from_file(PACKAGE_DATA_DIR"/wokjab/xa.png", &err);
}

GroupChatWidget::~GroupChatWidget ()
{
	EXP_SIGUNHOOK("Config XML Change /groupchat/window", &GroupChatWidget::Config, 500);
	
	if ( mainwindowplug )
	{
		char buf[20];
		sprintf(buf, "%d", gtk_paned_get_position (GTK_PANED(glade_xml_get_widget(xml, "vpaned"))));
		config->GetFirstTag("textarea").AddAttr("height", buf);
		sprintf(buf, "%d", gtk_paned_get_position (GTK_PANED(glade_xml_get_widget(xml, "hpaned"))));
		config->GetFirstTag("textarea").AddAttr("width", buf);
		
		WokXMLTag conftag(NULL, "config");
		conftag.AddAttr("path", "/groupchat/window");
		conftag.AddTag(config);
		
		wls->SendSignal("Config XML Store", &conftag);
		
		gtk_widget_destroy(mainwindowplug);
	
	}
	if( labelplug )
		gtk_widget_destroy(labelplug);
	
	std::string message;
	message = "<presence to='" + roomjid + "' type='unavailable'/>";	
	
	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", session);
	WokXMLTag &presencetag = msgtag.AddTag("presence");
	presencetag.AddAttr("to",roomjid);
	presencetag.AddAttr("type","unavailable");
	
	
	wls->SendSignal("Jabber XML Send", &msgtag);
	
	mclass->Remove(session, roomjid, this);
	
	if( config ) 
		delete config;
		
	g_object_unref(xml);
}

gboolean
GroupChatWidget::Scroll (GtkWidget *widget, GdkEventScroll *event, GroupChatWidget *c)
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
		gtk_text_buffer_get_start_iter(c->outputbuffer, &startiter);
		gtk_text_buffer_get_end_iter(c->outputbuffer, &enditer);
		GtkTextTagTable *tag_table =  gtk_text_buffer_get_tag_table (c->outputbuffer);
		GtkTextTag *oldtag = gtk_text_tag_table_lookup(tag_table, "fontsize");
		if ( oldtag )
			gtk_text_tag_table_remove(tag_table, oldtag);
		
		GtkTextTag *tag = gtk_text_buffer_create_tag (c->outputbuffer, "fontsize","scale", c->fontsize, NULL);
		gtk_text_buffer_remove_tag_by_name (c->outputbuffer, "fontsize", &startiter, &enditer);
		gtk_text_buffer_apply_tag (c->outputbuffer, tag, &startiter, &enditer);
		return TRUE;
	}
	return FALSE;
}

int
GroupChatWidget::Close(WokXMLTag *tag)
{
	delete this;
	return 1;
}

void
GroupChatWidget::Minimize_button(GtkButton *button, GroupChatWidget *c)
{
	char buf[10];
	sprintf(buf, "%d", c->GetWidget());
	WokXMLTag widget(NULL, "widget");
	widget.AddAttr("widget", buf);
	
	c->wls->SendSignal("GUI WindowDock HideWidget", widget);
}

void
GroupChatWidget::Destroy(GtkWidget *widget, GroupChatWidget *c)
{
	c->mainwindowplug = NULL;
}


gboolean
GroupChatWidget::refocus ( GtkWidget *widget, GdkEventFocus *event, GroupChatWidget *c)
{
	gtk_widget_grab_focus(c->inputview);
	return true;
}

gboolean
GroupChatWidget::focus_event (GtkWidget *widget, GdkEventFocus *event, GroupChatWidget *c)
{
	if(event->in)
		c->Focus();
	else
		c->Defocus();
	
	return false;
}

gboolean
GroupChatWidget::popup_menu(GtkTreeView *tree_view, GdkEventButton *event, GroupChatWidget *c)
{
	GtkTreePath      *path;

	if (event->button == 3 && 
		gtk_tree_view_get_path_at_pos (tree_view, (int)event->x, (int)event->y, &path, NULL, NULL, NULL)) 
	{
		GtkTreeSelection *selection;
		GtkTreeIter iter;
		gchar *jid;
		
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(c->occupantlist));
		gtk_tree_selection_unselect_all (selection);
		gtk_tree_selection_select_path (selection, path);
		gtk_tree_model_get_iter (GTK_TREE_MODEL(c->model), &iter, path);

		gtk_tree_model_get(GTK_TREE_MODEL(c->model), &iter, 1, &jid, -1);
		
		if(!jid)
		{
			WokXMLTag sigtag(NULL, "message");
			sigtag.AddAttr("type", "in");
			sigtag.AddTag("body").AddText("No jid for this entry");
			c->wls->SendSignal("Display Message", sigtag);
			return false;			
		}
		
		char buf[20];
		WokXMLTag MenuXML(NULL, "menu");
		sprintf(buf, "%d", event->button);
		MenuXML.AddAttr("button", buf);
		sprintf(buf, "%d", event->time);
		MenuXML.AddAttr("time", buf);
		MenuXML.AddTag("item").AddAttr("signal", "Jabber GUI GroupChat GetJIDMenu");
		MenuXML.AddTag("item").AddAttr("signal", "Jabber GUI GetJIDMenu");
		WokXMLTag &data = MenuXML.AddTag("data");
		data.AddAttr("jid", jid);
		data.AddAttr("session", c->session);
		c->wls->SendSignal("Jabber GUI JIDMenu", &MenuXML);
				
		gtk_tree_path_free (path);
		
		return true;
	}
	return false;
}

gboolean 
GroupChatWidget::subject_activation( GtkWidget *widget, gpointer user_data )
{
	GroupChatWidget *data;
	data = static_cast < GroupChatWidget *>(user_data);
	
	std::string text = XMLisize(gtk_entry_get_text(GTK_ENTRY(widget)));
	
	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", data->session);
	WokXMLTag &mtag = msgtag.AddTag("message");
	mtag.AddAttr("to", data->roomjid);
	mtag.AddAttr("type", "groupchat");
	mtag.AddTag("subject").AddText(text);
	
	data->wls->SendSignal("Jabber XML Send", &mtag);

	return true;
}

int
GroupChatWidget::own_message(std::string msg )
{
	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", session);
	WokXMLTag &message = msgtag.AddTag("message");
	message.AddAttr("to", roomjid);
	message.AddAttr("type", "groupchat");
	message.AddTag("body").AddText(XMLisize(msg));
	
	wls->SendSignal("Jabber XML Send", &msgtag);

	return 1;
}

int
GroupChatWidget::GetWidget ()
{
	return gtk_plug_get_id(GTK_PLUG(mainwindowplug));
}

int
GroupChatWidget::GetLabel()
{
	return gtk_plug_get_id(GTK_PLUG(labelplug));
}

int
GroupChatWidget::Config (WokXMLTag *tag)
{	
	if( config ) 
		delete config;
	config = new WokXMLTag (tag->GetFirstTag("config"));

	std::list <WokXMLTag *>::iterator iter;
	std::list <WokXMLTag *> *list;
	
	list = &config->GetFirstTag("color").GetTags();
	for( iter = list->begin() ; iter != list->end(); iter++)
	{
		tags[(*iter)->GetName()] = gtk_text_buffer_create_tag( outputbuffer, NULL, "foreground", (*iter)->GetAttr("data").c_str(), NULL);
	}
	
	/*
	int width = atoi(config->GetFirstTag("textarea").GetAttr("width").c_str());
	int height = atoi(config->GetFirstTag("textarea").GetAttr("height").c_str());
	
	gtk_paned_set_position (GTK_PANED(glade_xml_get_widget(xml, "hpaned")), width);
	gtk_paned_set_position (GTK_PANED(glade_xml_get_widget(xml, "vpaned")), height);
	*/
	
	return true;
}

int
GroupChatWidget::Message (WokXMLTag& tag_msg)
{
	GtkTextIter iter;
	std::string str;
	
	gtk_text_buffer_get_iter_at_mark(outputbuffer, &iter, end_mark);
	if(tag_msg.GetFirstTag("subject").GetBody().size())
		gtk_entry_set_text(GTK_ENTRY(subject_entry), tag_msg.GetFirstTag("subject").GetBody().c_str());

	if(tag_msg.GetFirstTag("body").GetBody().size())
	{	
		std::string message = tag_msg.GetFirstTag("body").GetBody() + "\n";
		std::string timestamp = GetTimeStamp(tag_msg);
		
		if(!focus)
		{
			gtk_widget_modify_fg(GTK_WIDGET(label_label), GTK_STATE_ACTIVE, &color_red);
			gtk_widget_modify_fg(GTK_WIDGET(label_label), GTK_STATE_NORMAL, &color_red);
		}
		gtk_text_buffer_insert_with_tags (outputbuffer, &iter, timestamp.c_str(), timestamp.size(), tags["timestamp"], NULL);
		if( tag_msg.GetAttr("from").find("/") == std::string::npos )
		{
			gtk_text_buffer_insert_with_tags (outputbuffer, &iter, "- ", 2, tags["notice"], NULL);
			gtk_text_buffer_insert_with_tags (outputbuffer, &iter, message.c_str(), message.length(), tags["notice"], NULL);
		}
		else
		{
			std::string nick;
			nick = tag_msg.GetAttr("from").substr( tag_msg.GetAttr("from").find("/") + 1, tag_msg.GetAttr("from").length() );
			str = '<' + nick + "> ";
			if(nick == mynick)
			{
				gtk_text_buffer_insert_with_tags (outputbuffer, &iter, str.c_str(), str.length(), tags["my_name"], NULL);
				gtk_text_buffer_insert_with_tags (outputbuffer, &iter, message.c_str(), message.length(), tags["my_text"], NULL);
			}
			else
			{
				if( tag_msg.GetFirstTag("body").GetBody().find(mynick) == std::string::npos )
					gtk_text_buffer_insert_with_tags (outputbuffer, &iter, str.c_str(), str.length(), tags["forreign_name"], NULL);
				else
				{
					if(!focus)
					{
						gtk_widget_modify_fg(GTK_WIDGET(label_label), GTK_STATE_ACTIVE, &color_blue);
						gtk_widget_modify_fg(GTK_WIDGET(label_label), GTK_STATE_NORMAL, &color_blue);
					}
		
					gtk_text_buffer_insert_with_tags (outputbuffer, &iter, str.c_str(), str.length(), tags["highlight"], NULL);
				}
					
				gtk_text_buffer_insert_with_tags (outputbuffer, &iter, message.c_str(), message.length(), tags["forreign_text"], NULL);
			}
		}
	}
	gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (outputview),
				      end_mark, 0.4, TRUE, 1, 1);
	
	return 1;
}

gboolean
GroupChatWidget::key_press_handler(GtkWidget * widget, GdkEventKey * event,
			     gpointer data)
{
	GroupChatWidget *obj;
	obj = static_cast < GroupChatWidget * > ( data );
	
	if ((event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
	    && !(event->state & GDK_CONTROL_MASK))
	{
		std::string str;
		GtkTextIter start_iter, end_iter;
		
		gtk_text_buffer_get_start_iter(obj->inputbuffer, &start_iter);
		gtk_text_buffer_get_end_iter(obj->inputbuffer, &end_iter);
		
		str = gtk_text_buffer_get_text(obj->inputbuffer,&start_iter, &end_iter, false);
		gtk_text_buffer_set_text(obj->inputbuffer, "", 0 );
		obj->own_message(str);
		return true;
	}
	else if ( event->keyval == GDK_Tab )
	{
		GtkTextIter start;
		GtkTextIter end;
		
		GtkTextMark *cursor =  gtk_text_buffer_get_insert(obj->inputbuffer);
		gtk_text_buffer_get_iter_at_mark (obj->inputbuffer, &start, cursor);
		gtk_text_buffer_get_iter_at_mark (obj->inputbuffer, &end, cursor);
		
		if( gtk_text_iter_backward_word_start(&start) )
		{
			std::string nick;
			std::string word;
			int num = 0;
			
			word = gtk_text_buffer_get_text(obj->inputbuffer,&start, &end, false);
			
			std::map<std::string, GtkTreeRowReference*>::iterator ppl;
			
			for( ppl = obj->participants.begin() ; ppl != obj->participants.end() ; ppl++)
			{
				if ( ppl->first.substr(0, word.size()) == word )
				{
					if ( num++ )
						return true;
					
					nick = ppl->first;
				}
			}
			
			if( num == 1 )
			{
				gtk_text_buffer_insert_interactive_at_cursor
                                            (obj->inputbuffer,
                                             nick.c_str() + word.size(),
                                             nick.size() - word.size(),
                                             true);
				gtk_text_buffer_insert_interactive_at_cursor
                                            (obj->inputbuffer,
                                             obj->config->GetFirstTag("nickcompletion").GetFirstTag("separator").GetBody().c_str(),
                                             obj->config->GetFirstTag("nickcompletion").GetFirstTag("separator").GetBody().size(),
                                             true);
			}
		}
		
		return true;
	}
	
	return false;
}

std::string
GroupChatWidget::GetTimeStamp(WokXMLTag& tag_body)
{
	std::list<WokXMLTag *>::iterator iter;
	std::string stamp = "";
	std::string ret;
	
	for( iter = tag_body.GetTagList("x").begin() ; iter != tag_body.GetTagList("x").end() ; iter++)
	{
		if( (*iter)->GetAttr("xmlns") == "jabber:x:delay")
			stamp = (*iter)->GetAttr("stamp");
	}
	
	if( stamp.size())
	{
		struct tm tp;
		strptime (stamp.c_str(), "%Y%m%dT%T", &tp);
		ret = GetTimeStamp(mktime(&tp));
	}
	else
	{
		ret = GetTimeStamp(time(0));
	}
	
	return ret;
}


std::string
GroupChatWidget::GetTimeStamp(time_t t)
{
	struct tm *tm;
	char      buf[128];
	const char *timestamp;

	tm = localtime (&t);
	
	buf[0] = 0;

	timestamp = config->GetFirstTag("timestamp").GetFirstTag("format").GetBody().c_str();
	
	strftime (buf, sizeof (buf),timestamp, tm);
	return buf;
}

int
GroupChatWidget::Presence(WokXMLTag *tag)
{
	WokXMLTag *tag_presence;
	GtkTreeIter RosterIter;
	std::string message;

	tag_presence = &tag->GetFirstTag("presence");
	if(tag_presence->GetAttr("type") == "error")
	{
		GtkTextIter iter;
		gtk_text_buffer_get_iter_at_mark(outputbuffer, &iter, end_mark);
		std::string str = "Error presence recived, probably you wasnt allowed to enter the room\n";
		
		gtk_text_buffer_insert (outputbuffer, &iter, str.c_str(), str.length());
		gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (outputview),
				      end_mark, 0.4, TRUE, 1, 1);
		return 0;
	}
	if(tag_presence->GetAttr("type") != "unavailable")
	{
		std::list <WokXMLTag *>::iterator tagiter;
		std::string role;
		for( tagiter = tag_presence->GetTagList("x").begin() ; tagiter != tag_presence->GetTagList("x").end() ; tagiter++)
		{
			role = (*tagiter)->GetFirstTag("item").GetAttr("role");
		}
		if( participants.find(tag->GetAttr("nick")) == participants.end() )
		{
			gtk_list_store_append (model, &RosterIter);
			participants[tag->GetAttr("nick")] = gtk_tree_row_reference_new(GTK_TREE_MODEL(model),gtk_tree_model_get_path(GTK_TREE_MODEL(model), &RosterIter));
			message = tag->GetAttr("nick") + " has joined";// + roomjid;
		}
		else
		{
			GtkTreePath *path = gtk_tree_row_reference_get_path (participants[tag->GetAttr("nick")]);
			
			gtk_tree_model_get_iter(GTK_TREE_MODEL(model), &RosterIter, path);
			gtk_tree_path_free (path);
		}
		
		if ( roles.find(role) == roles.end() )
				roles[role] = NULL;
		if ( presence.find(tag_presence->GetFirstTag("show").GetBody()) == presence.end() )
				presence[tag_presence->GetFirstTag("show").GetBody()] = NULL;
			
		gtk_list_store_set (model, &RosterIter,
					0, tag->GetAttr("nick").c_str(), 
					1, tag_presence->GetAttr("from").c_str(), 
					2, roles[role], 
					3, presence[tag_presence->GetFirstTag("show").GetBody()], -1);
	}
	else
	{
		if( gtk_tree_model_get_iter(GTK_TREE_MODEL(model), &RosterIter, gtk_tree_row_reference_get_path(participants[tag->GetAttr("nick")])))
		{
			gtk_list_store_remove (GTK_LIST_STORE (model), &RosterIter);
			if( participants.find(tag->GetAttr("nick")) != participants.end() )
				participants.erase(tag->GetAttr("nick"));
			message = tag->GetAttr("nick") + " has left";// + roomjid;
		}
		else
			message = tag->GetAttr("nick") + " has left but something wierd happend";
		
	}
	if(message.size())
	{	
		GtkTextIter iter;
		gtk_text_buffer_get_iter_at_mark(outputbuffer, &iter, end_mark);
	
		message += '\n';
		gtk_text_buffer_insert_with_tags (outputbuffer, &iter, message.c_str(), message.length(), tags["notice"], NULL);
		gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (outputview),
				      end_mark, 0.4, TRUE, 1, 1);
	}

	if ( tag->GetAttr("nick") == mynick )
	{
		if(tag_presence->GetFirstTag("x").GetFirstTag("item").GetAttr("affiliation") == "owner")
		{
			/*
			std::string iqmsg = "<query xmlns='http://jabber.org/protocol/muc#owner'/>";
			*/
			
			WokXMLTag msgtag(NULL, "message");
			msgtag.AddAttr("session", session);
			WokXMLTag &iqtag = msgtag.AddTag("iq");
			iqtag.AddAttr("type", "get");
			iqtag.AddAttr("to", roomjid);
			WokXMLTag &querytag = iqtag.AddTag("query");
			querytag.AddAttr("xmlns", "http://jabber.org/protocol/muc#owner");
			
			wls->SendSignal("Jabber XML IQ Send", &msgtag);
			
			new ownerwidget(wls, session, roomjid, iqtag.GetAttr("id")); // Has selfdestruct...
		}
	}

	return 1;
}

void
GroupChatWidget::Focus()
{
	focus = true;

	gtk_widget_modify_fg(GTK_WIDGET(label_label), GTK_STATE_ACTIVE, NULL);
	gtk_widget_modify_fg(GTK_WIDGET(label_label), GTK_STATE_NORMAL, NULL);
	
	gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (outputview),
				      end_mark, .4, TRUE, 1, 1);
}

void
GroupChatWidget::Defocus()
{
	focus = false;
}
