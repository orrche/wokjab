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

#include "p-e-p-widget.hpp"
#include <sstream>

PEP_Widget::PEP_Widget(WLSignal *wls, PEPManager *pm): WLSignalInstance(wls), pm(pm)
{
	config = new WokXMLTag(NULL, "NULL");
	EXP_SIGHOOK("Config XML Change /PEP_Manager/window", &PEP_Widget::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/PEP_Manager/window");
	wls->SendSignal("Config XML Trigger", &conftag);
	
	xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/p-e-p-manager.glade", NULL, NULL);
		
	GtkWidget *sessionchooser;
	GtkCellRenderer *renderer;
	
	sessionmenu = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	pep_type_list = gtk_list_store_new(2, G_TYPE_BOOLEAN, G_TYPE_STRING);
	jid_list = gtk_list_store_new(3, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING);
	
	sessionchooser = glade_xml_get_widget(xml, "session");
	gtk_combo_box_set_model (GTK_COMBO_BOX(sessionchooser), GTK_TREE_MODEL(sessionmenu) );
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (sessionchooser), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (sessionchooser), renderer,
					"text", 0,
					NULL);
		

	GtkTreeViewColumn *column;
	/*
	renderer = gtk_cell_renderer_toggle_new ();
	g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (0));
	column = gtk_tree_view_column_new_with_attributes("Registered",
							     renderer, "active",
							     0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(xml, "pep_type")), column);
	*/
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("PEP type",
							     renderer, "text",
							     1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(xml, "pep_type")), column);
	
	renderer = gtk_cell_renderer_toggle_new ();
	//g_signal_connect (renderer, "toggled", G_CALLBACK (AutoJoinWid::cell_toggled), this);
	g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (0));
	column = gtk_tree_view_column_new_with_attributes("Req",
							     renderer, "active",
							     0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(xml, "jid_list")), column);
		
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Nick",
							     renderer, "text",
							     2, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(xml, "jid_list")), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("JID",
							     renderer, "text",
							     1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(xml, "jid_list")), column);


	
	gtk_tree_view_set_model (GTK_TREE_VIEW
		(glade_xml_get_widget(xml, "jid_list")), GTK_TREE_MODEL (jid_list));
	gtk_tree_view_set_model (GTK_TREE_VIEW
		(glade_xml_get_widget(xml, "pep_type")), GTK_TREE_MODEL (pep_type_list));
	
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
		  	G_CALLBACK (PEP_Widget::SessionChange), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (xml, "window")), "delete-event",
			G_CALLBACK (PEP_Widget::Delete), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget(xml, "pep_type")), "cursor-changed", 
		  	G_CALLBACK (PEP_Widget::TypeChange), this);
	
	gtk_window_set_default_size(GTK_WINDOW(glade_xml_get_widget(xml,"window")), 
				atoi(config->GetFirstTag("size").GetAttr("width").c_str()),
				atoi(config->GetFirstTag("size").GetAttr("height").c_str()));
	
	gtk_widget_show_all(glade_xml_get_widget(xml,"window"));	
	
}


PEP_Widget::~PEP_Widget()
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
	
	
	std::list <PEPUser*>::iterator useriter;
	for ( useriter = userlist.begin() ; useriter != userlist.end() ; useriter++)
	{
		delete *useriter;
	}
	
	gtk_widget_destroy(window);
}


gboolean
PEP_Widget::Delete( GtkWidget *widget, GdkEvent *event, PEP_Widget *c)
{
	c->pm->DialogOpenerRemove(c);
	return TRUE;
}

void
PEP_Widget::SaveConfig()
{
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/PEP_Manager/window");
	conftag.AddTag(config);

	EXP_SIGUNHOOK("Config XML Change /PEP_Manager/window", &PEP_Widget::ReadConfig, 500);
	wls->SendSignal("Config XML Store", &conftag);
	EXP_SIGHOOK("Config XML Change /PEP_Manager/window", &PEP_Widget::ReadConfig, 500);
}

int
PEP_Widget::ReadConfig(WokXMLTag *tag)
{
	delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));
	return 1;
}

void 
PEP_Widget::SesChange()
{
	std::list <PEPUser*>::iterator useriter;
	for ( useriter = userlist.begin() ; useriter != userlist.end() ; useriter++)
	{
		delete *useriter;
	}
	userlist.clear();
	typelist.clear();
	
	gtk_list_store_clear(pep_type_list);
	gtk_list_store_clear(jid_list);
	
	GtkTreeIter treeiter;
	if( !gtk_combo_box_get_active_iter(GTK_COMBO_BOX(glade_xml_get_widget(xml, "session")), &treeiter) )
	{
		return;
	}
		
	gchar *session;
	
	gtk_tree_model_get (GTK_TREE_MODEL(sessionmenu), &treeiter, 1, &session, -1);
	selected_session = session;
	
	WokXMLTag jids("groups");
	
	wls->SendSignal("Roster Get Roster", jids);
	
	std::list <WokXMLTag *>::iterator sesiter;
	for ( sesiter = jids.GetTagList("session").begin() ; sesiter != jids.GetTagList("session").end() ; sesiter++)
	{
		if( (*sesiter)->GetAttr("name") == session )
		{
			std::list <WokXMLTag *>::iterator useriter;
			for ( useriter = (*sesiter)->GetTagList("user").begin() ; useriter != (*sesiter)->GetTagList("user").end() ; useriter++)
			{
				userlist.push_back( new PEPUser(wls, this, session, (*useriter)->GetAttr("jid")));
				/*
				<iq from='benvolio@montague.net/home'
					to='juliet@capulet.com'
					id='disco3'
					type='get'>
				  <query xmlns='http://jabber.org/protocol/disco#items'/>
				</iq>
				*/
				
			}			
		}		
	}
	g_free(session);
}

void
PEP_Widget::AddType(std::string type)
{
	if ( std::find(typelist.begin() , typelist.end() , type ) == typelist.end() )
	{
		GtkTreeIter tIter;
		gtk_list_store_append(pep_type_list, &tIter);
		gtk_list_store_set (pep_type_list, &tIter,
								1, type.c_str(),
								-1);
		
		typelist.push_back(type);
	}	
}



void
PEP_Widget::TypeChange(GtkTreeView *tree_view, PEP_Widget *c)
{
	gtk_list_store_clear(c->jid_list);
	GtkTreeIter treeiter;
	GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW(glade_xml_get_widget(c->xml, "pep_type")));
	
	if( !gtk_tree_selection_get_selected (sel, NULL , &treeiter))
		return;
	
	gchar *type_name;
	gtk_tree_model_get(GTK_TREE_MODEL(c->pep_type_list), &treeiter, 1, &type_name, -1);
	
	std::list <PEPUser*>::iterator useriter;
	for ( useriter = c->userlist.begin() ; useriter != c->userlist.end() ; useriter++)
	{
		if( (*useriter)->SetNode(type_name) )
		{
			(*useriter)->Show();
		}
		else
			(*useriter)->Hide();
		
	}
	/*
	if ( c->userlist.find(type_name) != c->userlist.end() )
	{
		std::list<std::string>::iterator jiditer;
		
		for( jiditer = c->userlist[type_name].begin() ; jiditer != c->userlist[type_name].end() ; jiditer++)
		{
			GtkTreeIter tIter;			 
			gtk_list_store_append (c->jid_list, &tIter);
			gtk_list_store_set (c->jid_list, &tIter,
						0, FALSE,
						1, jiditer->c_str(),
						-1);
		}
	}
	*/
	g_free(type_name);
}

void
PEP_Widget::SessionChange(GtkComboBox *widget, PEP_Widget *c)
{	
	c->SesChange();
}
