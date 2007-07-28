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

GM_Widget::GM_Widget(WLSignal *wls, GroupManager *gm): WLSignalInstance(wls),
gm(gm)
{
	xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/group-manager.glade", NULL, NULL);
	GtkWidget *window = glade_xml_get_widget(xml, "window");
	
	gtk_widget_show_all(window);
	
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
}

GM_Widget::~GM_Widget()
{
	
	
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
	
	std::cout << "Groupname: " << groupname << std::endl;
	
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
	gtk_list_store_clear(c->grouplist);
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
