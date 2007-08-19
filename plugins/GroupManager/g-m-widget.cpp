/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2007 <nedo80@gmail.com>
 * 
 * wokjab is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * wokjab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with wokjab.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#include "g-m-widget.hpp"
#include <sstream>

GM_Widget::GM_Widget(WLSignal *wls, GroupManager *gm): WLSignalInstance(wls),
gm(gm)
{	
	config = new WokXMLTag(NULL, "NULL");
	EXP_SIGHOOK("Config XML Change /groupmanager/window", &GM_Widget::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/groupmanager/window");
	wls->SendSignal("Config XML Trigger", &conftag);
	
	xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/group-manager.glade", NULL, NULL);
	GtkWidget *window = glade_xml_get_widget(xml, "window");
	
	GtkWidget *sessionchooser;
	GtkCellRenderer *renderer;
	
	sessionmenu = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	grouplist = gtk_list_store_new(1, G_TYPE_STRING);
	inlist = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	rosterlist = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(glade_xml_get_widget(xml, "groups")), GTK_TREE_MODEL(grouplist));	
	gtk_tree_view_set_model(GTK_TREE_VIEW(glade_xml_get_widget(xml, "in_jid")), GTK_TREE_MODEL(inlist));	
	gtk_tree_view_set_model(GTK_TREE_VIEW(glade_xml_get_widget(xml, "roster_jid")), GTK_TREE_MODEL(rosterlist));	
	
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "editable", TRUE, NULL);
	g_signal_connect (renderer, "edited", G_CALLBACK (GM_Widget::cell_edited), this);
	g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (0));
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (glade_xml_get_widget(xml, "groups")),
        -1, "Groups", renderer, "text", 0, NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (glade_xml_get_widget(xml, "in_jid")),
        -1, "User", renderer, "text", 1, NULL);
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (glade_xml_get_widget(xml, "roster_jid")),
        -1, "User", renderer, "text", 1, NULL);
	
	sessionchooser = glade_xml_get_widget(xml, "session");
	gtk_combo_box_set_model (GTK_COMBO_BOX(sessionchooser), GTK_TREE_MODEL(sessionmenu) );
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (sessionchooser), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (sessionchooser), renderer,
					"text", 0,
					NULL);
	
	WokXMLTag querytag(NULL,"session");
	wls->SendSignal("Jabber GetSessions", &querytag);
	
	std::list <WokXMLTag *>::iterator iter;
	
	for( iter = querytag.GetTagList("item").begin() ; iter != querytag.GetTagList("item").end() ; iter++)
	{
		WokXMLTag querytag(NULL, "query");
		WokXMLTag &itemtag = querytag.AddTag("item");
		itemtag.AddAttr("session", (*iter)->GetAttr("name"));
		wls->SendSignal("Jabber Connection GetUserData", &querytag);
		std::string name= (*iter)->GetAttr("name") + ": " + itemtag.GetFirstTag("jid").GetBody();
		
		GtkTreeIter treeiter;
		gtk_list_store_append(GTK_LIST_STORE(sessionmenu), &treeiter);
		gtk_list_store_set(GTK_LIST_STORE(sessionmenu), &treeiter, 0 , itemtag.GetFirstTag("jid").GetBody().c_str(),
						1, (*iter)->GetAttr("name").c_str(), -1);
	}
	
	
	g_signal_connect (G_OBJECT (sessionchooser), "changed", 
		  	G_CALLBACK (GM_Widget::SessionChange), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget(xml, "groups")), "cursor-changed", 
		  	G_CALLBACK (GM_Widget::GroupChange), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (xml, "join")), "clicked",
			G_CALLBACK (GM_Widget::Join), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (xml, "part")), "clicked",
			G_CALLBACK (GM_Widget::Part), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (xml, "add")), "clicked",
			G_CALLBACK (GM_Widget::Add), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (xml, "delete")), "clicked",
			G_CALLBACK (GM_Widget::Remove), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (xml, "window")), "delete-event",
			G_CALLBACK (GM_Widget::Delete), this);
	
	gtk_window_set_default_size(GTK_WINDOW(glade_xml_get_widget(xml,"window")), 
				atoi(config->GetFirstTag("size").GetAttr("width").c_str()),
				atoi(config->GetFirstTag("size").GetAttr("height").c_str()));
	
	
	gtk_widget_show_all(window);
}

GM_Widget::~GM_Widget()
{
	GtkWidget *window = glade_xml_get_widget(xml,"window");	
	
	int width, height;
	gtk_window_get_size(GTK_WINDOW(window), &width, &height);

	std::stringstream s_width, s_height;
	s_width << width;
	s_height << height;

	config->GetFirstTag("size").AddAttr("width", s_width.str().c_str());
	config->GetFirstTag("size").AddAttr("height", s_height.str().c_str());

	SaveConfig();
	g_object_unref(xml);
	gtk_widget_destroy(window);
}

gboolean
GM_Widget::Delete( GtkWidget *widget, GdkEvent *event, GM_Widget *c)
{
	c->gm->DialogOpenerRemove(c);
	return TRUE;
}


void
GM_Widget::SaveConfig()
{
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/groupmanager/window");
	conftag.AddTag(config);

	EXP_SIGUNHOOK("Config XML Change /groupmanager/window", &GM_Widget::ReadConfig, 500);
	wls->SendSignal("Config XML Store", &conftag);
	EXP_SIGHOOK("Config XML Change /groupmanager/window", &GM_Widget::ReadConfig, 500);
}


int
GM_Widget::ReadConfig(WokXMLTag *tag)
{
	delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));
	return 1;
}


void
GM_Widget::cell_edited (GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text, GM_Widget *c)
{
	GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
	GtkTreeIter iter;

	gint column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (cell), "column"));
	gtk_tree_model_get_iter (GTK_TREE_MODEL(c->grouplist), &iter, path);

	gchar *old_text;

	gtk_tree_model_get (GTK_TREE_MODEL(c->grouplist), &iter, column, &old_text, -1);
	
	GtkTreeIter useriter;
	
	if ( gtk_tree_model_get_iter_first(GTK_TREE_MODEL(c->inlist), &useriter) )
	{
		do
		{
			gchar *jid;
			gtk_tree_model_get(GTK_TREE_MODEL(c->inlist), &useriter, 0, &jid, -1);
			WokXMLTag remove_tag("remove_tag");
			remove_tag.AddAttr("jid", jid);
			remove_tag.AddAttr("group", old_text);
			remove_tag.AddAttr("session", c->selected_session);
			c->wls->SendSignal("Roster Remove User From Group", remove_tag);
			g_free(jid);
		}
		while ( gtk_tree_model_iter_next(GTK_TREE_MODEL(c->inlist), &useriter) );
	}
	
	if ( gtk_tree_model_get_iter_first(GTK_TREE_MODEL(c->inlist), &useriter) )
	{
		do
		{
			gchar *jid;
			gtk_tree_model_get(GTK_TREE_MODEL(c->inlist), &useriter, 0, &jid, -1);
			WokXMLTag add_tag("add_tag");
			add_tag.AddAttr("jid", jid);
			add_tag.AddAttr("group", new_text);
			add_tag.AddAttr("session", c->selected_session);
			c->wls->SendSignal("Roster Add User To Group", add_tag);
			g_free(jid);
		}
		while ( gtk_tree_model_iter_next(GTK_TREE_MODEL(c->inlist), &useriter) );
	}
	
	g_free (old_text);

	gtk_list_store_set (GTK_LIST_STORE (c->grouplist), &iter, column, new_text, -1);

	gtk_tree_path_free (path);
}

void
GM_Widget::Add(GtkButton *button, GM_Widget *c)
{
	GtkTreeIter treeiter;
	gtk_list_store_append(GTK_LIST_STORE(c->grouplist), &treeiter);
	gtk_list_store_set(GTK_LIST_STORE(c->grouplist), &treeiter, 0 , "Group", -1);
}

void
GM_Widget::Remove(GtkButton *button, GM_Widget *c)
{
}


void
GM_Widget::Part(GtkButton *button, GM_Widget *c)
{
	GtkTreeIter treeiter;
	GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW(glade_xml_get_widget(c->xml, "in_jid")));
	
	if( !gtk_tree_selection_get_selected (sel, NULL , &treeiter))
		return;
	
	GtkTreeIter groupiter;
	sel = gtk_tree_view_get_selection (GTK_TREE_VIEW(glade_xml_get_widget(c->xml, "groups")));
	
	if( !gtk_tree_selection_get_selected (sel, NULL , &groupiter))
		return;
	
	gchar *jid, *nick, *group;
	
	gtk_tree_model_get(GTK_TREE_MODEL(c->grouplist), &groupiter, 0, &group, -1);
	gtk_tree_model_get(GTK_TREE_MODEL(c->inlist), &treeiter, 0, &jid, 1, &nick, -1);
	
	GtkTreeIter newiter;
	gtk_list_store_append(GTK_LIST_STORE(c->rosterlist), &newiter);
	gtk_list_store_set(GTK_LIST_STORE(c->rosterlist), &newiter, 0 , jid, 1, nick, -1);
	gtk_list_store_remove(GTK_LIST_STORE(c->inlist), &treeiter);

	WokXMLTag remove_tag("remove_tag");
	remove_tag.AddAttr("jid", jid);
	remove_tag.AddAttr("group", group);
	remove_tag.AddAttr("session", c->selected_session);
	c->wls->SendSignal("Roster Remove User From Group", remove_tag);
	
	g_free(jid);
	g_free(nick);
	g_free(group);
}

void
GM_Widget::Join(GtkButton *button, GM_Widget *c)
{
	GtkTreeIter treeiter;
	GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW(glade_xml_get_widget(c->xml, "roster_jid")));
	
	if( !gtk_tree_selection_get_selected (sel, NULL , &treeiter))
		return;
	
	GtkTreeIter groupiter;
	sel = gtk_tree_view_get_selection (GTK_TREE_VIEW(glade_xml_get_widget(c->xml, "groups")));
	
	if( !gtk_tree_selection_get_selected (sel, NULL , &groupiter))
		return;
	
	gchar *jid, *nick, *group;
	
	gtk_tree_model_get(GTK_TREE_MODEL(c->grouplist), &groupiter, 0, &group, -1);
	gtk_tree_model_get(GTK_TREE_MODEL(c->rosterlist), &treeiter, 0, &jid, 1, &nick, -1);
	
	GtkTreeIter newiter;
	gtk_list_store_append(GTK_LIST_STORE(c->inlist), &newiter);
	gtk_list_store_set(GTK_LIST_STORE(c->inlist), &newiter, 0 , jid, 1, nick, -1);
	gtk_list_store_remove(GTK_LIST_STORE(c->rosterlist), &treeiter);

	WokXMLTag add_tag("add_tag");
	add_tag.AddAttr("jid", jid);
	add_tag.AddAttr("group", group);
	add_tag.AddAttr("session", c->selected_session);
	c->wls->SendSignal("Roster Add User To Group", add_tag);
	
	g_free(jid);
	g_free(nick);
	g_free(group);
}

void
GM_Widget::GroupChange(GtkTreeView *tree_view, GM_Widget *c)
{
	GtkTreeIter treeiter;
	GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW(glade_xml_get_widget(c->xml, "groups")));
	
	if( !gtk_tree_selection_get_selected (sel, NULL , &treeiter))
		return;
	
	gchar *groupname;
	gtk_tree_model_get(GTK_TREE_MODEL(c->grouplist), &treeiter, 0, &groupname, -1);
		
	WokXMLTag roster("roster");
	c->wls->SendSignal("Roster Get Roster", roster);
	std::list <WokXMLTag *>::iterator sesiter; 
	gtk_list_store_clear(c->inlist);
	gtk_list_store_clear(c->rosterlist);
	
	for( sesiter = roster.GetTagList("session").begin(); sesiter != roster.GetTagList("session").end() ; sesiter++)
	{
		if ( (*sesiter)->GetAttr("name") == c->selected_session )
		{
			std::list <WokXMLTag *>::iterator useriter;
			for( useriter = (*sesiter)->GetTagList("user").begin() ; useriter != (*sesiter)->GetTagList("user").end() ; useriter++)
			{
				bool in_the_group = false;
				
				std::list <WokXMLTag *>::iterator groupiter;
				
				for ( groupiter = (*useriter)->GetTagList("group").begin() ; groupiter != (*useriter)->GetTagList("group").end() ; groupiter++)
				{
					if ( (*groupiter)->GetBody() == groupname )
						in_the_group = true;
				}
				
				if ( in_the_group )
				{
					GtkTreeIter treeiter;
					gtk_list_store_append(GTK_LIST_STORE(c->inlist), &treeiter);
					std::string nick;
					if ( (*useriter)->GetAttr("nick").empty() )
						nick = (*useriter)->GetAttr("jid");
					else
						nick = (*useriter)->GetAttr("nick");
					gtk_list_store_set(GTK_LIST_STORE(c->inlist), &treeiter, 0 , (*useriter)->GetAttr("jid").c_str(), 1, nick.c_str(), -1);
				}
				else
				{
					GtkTreeIter treeiter;
					gtk_list_store_append(GTK_LIST_STORE(c->rosterlist), &treeiter);
					std::string nick;
					if ( (*useriter)->GetAttr("nick").empty() )
						nick = (*useriter)->GetAttr("jid");
					else
						nick = (*useriter)->GetAttr("nick");
					gtk_list_store_set(GTK_LIST_STORE(c->rosterlist), &treeiter, 0 , (*useriter)->GetAttr("jid").c_str(), 1, nick.c_str(), -1);
				}
			}
			
		}
		
	}
	
	
	g_free(groupname);
}


void
GM_Widget::SessionChange(GtkComboBox *widget, GM_Widget *c)
{	
	gtk_list_store_clear(c->jidlist);
	GtkTreeIter treeiter;
	if( !gtk_combo_box_get_active_iter(GTK_COMBO_BOX(glade_xml_get_widget(c->xml, "session")), &treeiter) )
	{
		return;
	}
		
	gchar *session;
	
	gtk_tree_model_get (GTK_TREE_MODEL(c->sessionmenu), &treeiter, 1, &session, -1);
	c->selected_session = session;
	
	WokXMLTag groups("groups");
	groups.AddAttr("session", session);
	g_free(session);
	
	c->wls->SendSignal("Roster Get All Groups", groups);
	
	std::list <WokXMLTag *>::iterator iter;
	
	for( iter = groups.GetTagList("group").begin() ; iter != groups.GetTagList("group").end() ; iter++)
	{
		GtkTreeIter treeiter;
		gtk_list_store_append(GTK_LIST_STORE(c->grouplist), &treeiter);
		gtk_list_store_set(GTK_LIST_STORE(c->grouplist), &treeiter, 0 , (*iter)->GetBody().c_str(), -1);
	}
	
	
}
