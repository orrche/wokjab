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


#include "AutoJoinWid.h"
#include <sstream>

AutoJoinWid::AutoJoinWid(WLSignal *wls, WokXMLTag *tag, AutoJoin *parent) : WLSignalInstance(wls),
parent(parent)
{
	config = new WokXMLTag(NULL, "NULL");
	EXP_SIGHOOK("Config XML Change /groupchat/autojoin", &AutoJoinWid::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/groupchat/autojoin");
	wls->SendSignal("Config XML Trigger", &conftag);

	session = tag->GetFirstTag("session").GetBody();

	xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/AutoJoinWid.glade", NULL, NULL);
	GtkWidget *window = glade_xml_get_widget(xml, "window");

	model = gtk_list_store_new(6, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN);
	
	GtkCellRenderer *renderer;  
	GtkTreeViewColumn *column;
	renderer = gtk_cell_renderer_text_new ();  
	g_object_set (renderer, "editable", TRUE, NULL);
	g_signal_connect (renderer, "edited", G_CALLBACK (AutoJoinWid::cell_edited), this);
	g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (0));
	column = gtk_tree_view_column_new_with_attributes("Nick",
							     renderer, "text",
							     0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(xml, "treeview")), column);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "editable", TRUE, NULL);
	g_signal_connect (renderer, "edited", G_CALLBACK (AutoJoinWid::cell_edited), this);
	g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (1));
	column = gtk_tree_view_column_new_with_attributes("Room",
							     renderer, "text",
							     1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(xml, "treeview")), column);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "editable", TRUE, NULL);
	g_signal_connect (renderer, "edited", G_CALLBACK (AutoJoinWid::cell_edited), this);
	g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (2));
	column = gtk_tree_view_column_new_with_attributes("Server",
							     renderer, "text",
							     2, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(xml, "treeview")), column);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "editable", TRUE, NULL);
	g_signal_connect (renderer, "edited", G_CALLBACK (AutoJoinWid::cell_edited), this);
	g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (3));
	column = gtk_tree_view_column_new_with_attributes("Password",
							     renderer, "text",
							     3, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(xml, "treeview")), column);

	renderer = gtk_cell_renderer_toggle_new ();
	g_signal_connect (renderer, "toggled", G_CALLBACK (AutoJoinWid::cell_toggled), this);
	g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (4));
	column = gtk_tree_view_column_new_with_attributes("Auto Join",
							     renderer, "active",
							     4, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(xml, "treeview")), column);

	renderer = gtk_cell_renderer_toggle_new ();
	g_signal_connect (renderer, "toggled", G_CALLBACK (AutoJoinWid::cell_toggled), this);
	g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (5));
	column = gtk_tree_view_column_new_with_attributes("Minimize",
							     renderer, "active",
							     5, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(xml, "treeview")), column);

	gtk_tree_view_set_model (GTK_TREE_VIEW
		(glade_xml_get_widget(xml, "treeview")), GTK_TREE_MODEL (model));


	
	
	g_signal_connect (glade_xml_get_widget(xml, "button_delete"), "clicked", G_CALLBACK (AutoJoinWid::button_delete), this);
	g_signal_connect (glade_xml_get_widget(xml, "button_add"), "clicked", G_CALLBACK (AutoJoinWid::button_add), this);
	g_signal_connect (glade_xml_get_widget(xml, "button_apply"), "clicked", G_CALLBACK (AutoJoinWid::button_apply), this);
	g_signal_connect (glade_xml_get_widget(xml, "button_close"), "clicked", G_CALLBACK (AutoJoinWid::button_cancel), this);
	g_signal_connect (glade_xml_get_widget(xml, "button_ok"), "clicked", G_CALLBACK (AutoJoinWid::button_ok), this);
	g_signal_connect (glade_xml_get_widget(xml, "button_reload"), "clicked", G_CALLBACK (AutoJoinWid::button_reload), this);
	g_signal_connect (glade_xml_get_widget(xml, "button_join"), "clicked", G_CALLBACK (AutoJoinWid::button_join), this);
	g_signal_connect (glade_xml_get_widget(xml, "window"), "delete-event", G_CALLBACK (AutoJoinWid::destroy), this);

	gtk_window_set_default_size(GTK_WINDOW(glade_xml_get_widget(xml,"window")), 
				atoi(config->GetFirstTag("size").GetAttr("width").c_str()),
				atoi(config->GetFirstTag("size").GetAttr("height").c_str()));

	ReloadList();

	gtk_widget_show_all(window);
}


AutoJoinWid::~AutoJoinWid()
{
	GtkWidget *window = glade_xml_get_widget(xml, "window");
	
	int width, height;
	gtk_window_get_size(GTK_WINDOW(window), &width, &height);

	std::stringstream s_width, s_height;
	s_width << width;
	s_height << height;

	config->GetFirstTag("size").AddAttr("width", s_width.str().c_str());
	config->GetFirstTag("size").AddAttr("height", s_height.str().c_str());

	SaveConfig();

	g_object_unref(xml);
	xml = NULL;

	gtk_widget_destroy(window);
}

void
AutoJoinWid::ReloadList()
{
	WokXMLTag msg_tag(NULL, "message");
	msg_tag.AddAttr("session", session);
	WokXMLTag &iq_tag = msg_tag.AddTag("iq");
	iq_tag.AddAttr("type", "get");
	WokXMLTag &query_tag = iq_tag.AddTag("query");
	query_tag.AddAttr("xmlns", "jabber:iq:private");
	WokXMLTag &storage_tag = query_tag.AddTag("storage");
	storage_tag.AddAttr("xmlns", "storage:bookmarks");

	wls->SendSignal("Jabber XML IQ Send", msg_tag);
	EXP_SIGHOOK("Jabber XML IQ ID " + iq_tag.GetAttr("id"), &AutoJoinWid::BookmarkResponse, 1000);
}

gboolean
AutoJoinWid::destroy( GtkWidget *widget, GdkEvent *event, AutoJoinWid *c)
{
	c->parent->RemoveWid(c);
	return TRUE;
}

void
AutoJoinWid::button_reload ( GtkButton *button, AutoJoinWid *c)
{
	c->ReloadList();
}

void
AutoJoinWid::button_ok ( GtkButton *button, AutoJoinWid *c)
{
	c->SaveList();
	delete c;
}

void
AutoJoinWid::button_join ( GtkButton *button, AutoJoinWid *c)
{
	GtkTreeIter tIter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(glade_xml_get_widget(c->xml, "treeview")));
	
	if(gtk_tree_selection_get_selected(selection,  NULL, &tIter));
	{
		gchar *nick, *room, *server, *password;
		gtk_tree_model_get(GTK_TREE_MODEL(c->model), &tIter, 0, &nick, 1, &room, 2, &server, 3, &password, -1);
			
		if ( nick[0] && room[0] && server[0] )
		{
			WokXMLTag jointag(NULL, "groupchat");
			jointag.AddAttr("nick", nick);
			jointag.AddAttr("room", room);
			jointag.AddAttr("server", server);
			jointag.AddAttr("session", c->session);

			if ( !password[0] )
				jointag.AddAttr("password", password);
			
			
			c->wls->SendSignal("Jabber GroupChat Join", &jointag);
		}

		g_free(nick);
		g_free(room);
		g_free(server);
		g_free(password);	
	}
	
}

void
AutoJoinWid::button_apply ( GtkButton *button, AutoJoinWid *c)
{
	c->SaveList();
}

void
AutoJoinWid::button_cancel ( GtkButton *button, AutoJoinWid *c)
{
	delete c;
}

void
AutoJoinWid::button_add (GtkButton *button, AutoJoinWid *c)
{
	GtkTreeIter tIter;			 
	gtk_list_store_append (c->model, &tIter);
	gtk_list_store_set (c->model, &tIter,
			    0, "",
				1, "",
				2, "",
				3, "",
			    4, TRUE,
				5, FALSE,
			    -1);


}

void
AutoJoinWid::SaveList()
{
	GtkTreeIter iter;
	
	WokXMLTag messagetag(NULL, "message");
	messagetag.AddAttr("session", session);
	WokXMLTag &iq = messagetag.AddTag("iq");
	iq.AddAttr("type", "set");
	WokXMLTag &query = iq.AddTag("query");
	query.AddAttr("xmlns", "jabber:iq:private");
	WokXMLTag &storage = query.AddTag("storage");
	storage.AddAttr("xmlns", "storage:bookmarks");

	if(!gtk_tree_model_get_iter_first   (GTK_TREE_MODEL(model), &iter))
	{
		wls->SendSignal("Jabber XML IQ Send", messagetag);
		return;
	}
	gchar *nick;
	gchar *room;
	gchar *server;
	gchar *password;
	gboolean autojoin, minimize;
	gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, 0, &nick, 1, &room, 2, &server, 3, &password, 4, &autojoin, 5, &minimize, -1);
	std::string nonchanged = "--";


	if ( nick[0] && room[0] && server[0] )
	{
		WokXMLTag &conference = storage.AddTag("conference");
		conference.AddAttr("jid", std::string(room) + "@" + server);
		conference.AddTag("nick").AddText(nick);
		if ( password[0] )
			conference.AddTag("password").AddText(password);
		if ( autojoin )
			conference.AddAttr("autojoin", "true");
		if ( minimize )
			conference.AddAttr("minimize", "true");
	}
	g_free(nick);
	g_free(room);
	g_free(server);
	g_free(password);


	while(gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter))
	{
		gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, 0, &nick, 1, &room, 2, &server, 3, &password, -1);
			
		if ( nick[0] && room[0] && server[0] )
		{
			WokXMLTag &conference = storage.AddTag("conference");
			conference.AddAttr("jid", std::string(room) + "@" + server);
			conference.AddTag("nick").AddText(nick);
			if ( password[0] )
				conference.AddTag("password").AddText(password);
			if ( autojoin )
				conference.AddAttr("autojoin", "true");
			if ( minimize )
				conference.AddAttr("minimize", "true");
		}

		g_free(nick);
		g_free(room);
		g_free(server);
		g_free(password);
	}

	wls->SendSignal("Jabber XML IQ Send", messagetag);
}


void
AutoJoinWid::button_delete (GtkButton *button, AutoJoinWid *c)
{
	GtkTreeIter tIter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(glade_xml_get_widget(c->xml, "treeview")));
	
	if(gtk_tree_selection_get_selected(selection,  NULL, &tIter));
	{
		gtk_list_store_remove(c->model, &tIter);
	}
}


void
AutoJoinWid::cell_toggled (GtkCellRendererText *cell, const gchar *path_string, AutoJoinWid *c)
{
	GtkTreeIter  iter;
	GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
	gint column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (cell), "column"));
	gboolean fixed;

	/* get toggled iter */
	gtk_tree_model_get_iter (GTK_TREE_MODEL(c->model), &iter, path);
	gtk_tree_model_get (GTK_TREE_MODEL(c->model), &iter, column, &fixed, -1);

	/* do something with the value */
	fixed ^= 1;

	/* set new value */
	gtk_list_store_set (GTK_LIST_STORE (c->model), &iter, column, fixed, -1);

	/* clean up */
	gtk_tree_path_free (path);
}

void
AutoJoinWid::cell_edited (GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text, AutoJoinWid *c)
{
	GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
	GtkTreeIter iter;

	gint column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (cell), "column"));
	gtk_tree_model_get_iter (GTK_TREE_MODEL(c->model), &iter, path);

	gchar *old_text;

	gtk_tree_model_get (GTK_TREE_MODEL(c->model), &iter, column, &old_text, -1);
	g_free (old_text);

	gtk_list_store_set (GTK_LIST_STORE (c->model), &iter, column, new_text, -1);

	gtk_tree_path_free (path);
}

void
AutoJoinWid::SaveConfig()
{
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/groupchat/autojoin");
	conftag.AddTag(config);

	EXP_SIGUNHOOK("Config XML Change /groupchat/autojoin", &AutoJoinWid::ReadConfig, 500);
	wls->SendSignal("Config XML Store", &conftag);
	EXP_SIGHOOK("Config XML Change /groupchat/autojoin", &AutoJoinWid::ReadConfig, 500);

}

int
AutoJoinWid::ReadConfig(WokXMLTag *tag)
{
	delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));

	return 1;
}

int
AutoJoinWid::BookmarkResponse(WokXMLTag *tag)
{
	gtk_list_store_clear (model);

	std::list <WokXMLTag *> *tags;
	std::list <WokXMLTag *>::iterator iter;
	tags = &tag->GetFirstTag("iq").GetFirstTag("query").GetFirstTag("storage").GetTagList("conference");
	for ( iter = tags->begin() ; iter != tags->end() ; iter++)
	{
		std::string jid = (*iter)->GetAttr("jid");
		if ( jid.find("@") != std::string::npos && jid.find("/") == std::string::npos)
		{
			std::string nick = (*iter)->GetFirstTag("nick").GetBody();
			if ( nick == "" )
			{
				nick ="Unknown..";
			}
			std::string room = (*iter)->GetAttr("jid").substr(0,(*iter)->GetAttr("jid").find("@"));
			std::string server = (*iter)->GetAttr("jid").substr((*iter)->GetAttr("jid").find("@")+1);
			std::string password = (*iter)->GetFirstTag("password").GetBody();		

			gboolean autojoin = FALSE;
			gboolean minimize = FALSE;
			if ( (*iter)->GetAttr("autojoin") == "1" || (*iter)->GetAttr("autojoin") == "true" )
				autojoin = TRUE;
			if ( (*iter)->GetAttr("minimize") == "1" || (*iter)->GetAttr("minimize") == "true" )
				minimize = TRUE;

			GtkTreeIter tIter;			 
			gtk_list_store_append (model, &tIter);
			gtk_list_store_set (model, &tIter,
					    0, nick.c_str(),
						1, room.c_str(),
						2, server.c_str(),
						3, password.c_str(),
					    4, autojoin,
						5, minimize,
					    -1);
/*
			if ( (*iter)->GetAttr("autojoin") == "true" ||  (*iter)->GetAttr("autojoin") == "1" )
			{
				WokXMLTag jointag(NULL, "groupchat");
				jointag.AddAttr("nick", nick);
				jointag.AddAttr("room", room);
				jointag.AddAttr("server", server);
				jointag.AddAttr("session", tag->GetAttr("session"));
				if ( !password.empty() )
				{
					jointag.AddAttr("password", password);
				}
				if ( (*iter)->GetAttr("minimized") == "true" ||  (*iter)->GetAttr("minimized") == "1" )
					jointag.AddAttr("minimized", "true");
					
				wls->SendSignal("Jabber GroupChat Join", &jointag);
				
			}
*/
		}
	}


	return 1;
}
