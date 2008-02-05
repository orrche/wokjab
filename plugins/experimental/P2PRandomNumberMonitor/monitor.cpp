/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2008 <nedo80@gmail.com>
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

#include "monitor.hpp"


Monitor::Monitor(WLSignal *wls) : WoklibPlugin(wls)
{
	gxml = NULL;
	
	EXP_SIGHOOK("Get Main Menu", &Monitor::MainMenu, 1010);
	EXP_SIGHOOK("GUI P2PRandMonitor Open", &Monitor::Wid, 999);
	EXP_SIGHOOK("GUI P2PRandMonitor Menu", &Monitor::Menu, 999);
	EXP_SIGHOOK("GUI P2PRandMonitor MenuOption Cancel", &Monitor::CancelMenu, 999);
	
	
	EXP_SIGHOOK("Jabber RandomNumber Generated", &Monitor::Generated, 999);
	EXP_SIGHOOK("Jabber RandomNumber RemoveSession", &Monitor::Remove, 999);
	EXP_SIGHOOK("Jabber RandomNumber Cancel", &Monitor::Cancel, 999);
	EXP_SIGHOOK("Jabber RandomNumber SessionCreate", &Monitor::Create, 999);
	EXP_SIGHOOK("Jabber RandomNumber Progress", &Monitor::Progress, 999);
}

Monitor::~Monitor()
{
	if ( gxml ) 
		g_object_unref(gxml);
}

bool
Monitor::FindIter(const std::string &if_session, const std::string &if_id, const std::string &if_owner, const std::string &if_roomjid, GtkTreeIter &iter)
{
	bool found = true;
	if ( gtk_tree_model_get_iter_first(GTK_TREE_MODEL(sess_store), &iter) == TRUE)
	{
		do
		{
			gchar *roomjid;   
			gchar *owner;
			gchar *session;
			gchar *id;
			
			gtk_tree_model_get(GTK_TREE_MODEL(sess_store), &iter, 0, &owner, 1, &id, 2, &session, 3, &roomjid, -1);
			
			if ( if_session == session && if_owner == owner && if_id == id && if_roomjid == roomjid )
				found = true;

			g_free(roomjid);
			g_free(owner);
			g_free(session);
			g_free(id);
			
			if ( found )
				break;
		}while (gtk_tree_model_iter_next(GTK_TREE_MODEL(sess_store), &iter) == TRUE);
		
	}
	return found;
}

int
Monitor::Create(WokXMLTag *tag)
{
	if ( !gxml )
		return;
	
	GtkTreeIter iter;
	gtk_list_store_prepend (sess_store, &iter);
	gtk_list_store_set (sess_store, &iter, 	0, tag->GetAttr("owner").c_str() , 
												1, tag->GetAttr("id").c_str() , 
												2, tag->GetAttr("session").c_str() , 
												3, tag->GetAttr("roomjid").c_str(), 
												4, "-",
												5, "-", 
												6, "Created", -1);
	
	return 1;
}

int
Monitor::Remove(WokXMLTag *tag)
{
	if ( !gxml )
		return;
	
	GtkTreeIter iter;
	if ( FindIter(tag->GetAttr("session"), tag->GetAttr("id"), tag->GetAttr("owner"), tag->GetAttr("roomjid"), iter) )
	{
		gtk_list_store_set (sess_store, &iter,	6, "Removed", -1);
	}
	
	return 1;
}

int
Monitor::Progress(WokXMLTag *tag)
{
	if ( !gxml )
		return;
	
	GtkTreeIter iter;
	
	if ( FindIter(tag->GetAttr("session"), tag->GetAttr("id"), tag->GetAttr("owner"), tag->GetAttr("roomjid"), iter) )
	{
		gtk_list_store_set (sess_store, &iter,	4, (tag->GetAttr("hashes") + "/" + tag->GetAttr("requiredusers")).c_str(),
												5, (tag->GetAttr("plains") + "/" + tag->GetAttr("requiredusers")).c_str(), -1);
	}
	
	return 1;
}

int
Monitor::Generated(WokXMLTag *tag)
{
	if ( !gxml )
		return;
	
	GtkTreeIter iter;
	if ( FindIter(tag->GetAttr("session"), tag->GetAttr("id"), tag->GetAttr("owner"), tag->GetAttr("roomjid"), iter) )
	{
		gtk_list_store_set (sess_store, &iter,	6, "Generated", 
												7, (tag->GetAttr("num")).c_str(), -1);
	}
	
	return 1;
}

int
Monitor::Cancel(WokXMLTag *tag)
{
	if ( !gxml )
		return;
	
	GtkTreeIter iter;
	if ( FindIter(tag->GetAttr("session"), tag->GetAttr("id"), tag->GetAttr("owner"), tag->GetAttr("roomjid"), iter) )
	{
		gtk_list_store_set (sess_store, &iter,	6, "Canceled", -1);
	}
	
	return 1;
}

int
Monitor::CancelMenu(WokXMLTag *tag)
{	
	wls->SendSignal("Jabber RandomNumber RemoveSession", tag);
	return 1;
}

int
Monitor::Menu(WokXMLTag *tag)
{
	
	WokXMLTag &item = tag->AddTag("item");
	item.AddAttr("name", "Cancel");
	item.AddAttr("signal", "GUI P2PRandMonitor MenuOption Cancel");
	
	return 1;
}


int
Monitor::MainMenu(WokXMLTag *tag)
{
	WokXMLTag &debug = tag->GetFirstTag("item");
	debug.AddAttr("name", "Debug helpers");
	WokXMLTag &item = debug.AddTag("item");
	item.AddAttr("name", "P2P Random Number monitor");
	item.AddAttr("signal", "GUI P2PRandMonitor Open");
	
	return 1;
}


int
Monitor::Wid(WokXMLTag *tag)
{
	if ( gxml == NULL )
		CreateWid();
	
	gtk_window_present(GTK_WINDOW(glade_xml_get_widget(gxml, "window")));
	
	return 1;
}


void
Monitor::CreateWid()
{
	gxml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/p2prandmonitor.glade", NULL, NULL);
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	sess_store = gtk_list_store_new (8, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(glade_xml_get_widget (gxml, "session_view")), GTK_TREE_MODEL(sess_store));
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("owner",
							     renderer, "text",
							     0, NULL);
	gtk_tree_view_column_set_sort_column_id(column, 0);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(gxml, "session_view")), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("id",
							     renderer, "text",
							     1, NULL);
	gtk_tree_view_column_set_sort_column_id(column, 1);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(gxml, "session_view")), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("session",
							     renderer, "text",
							     2, NULL);
	gtk_tree_view_column_set_sort_column_id(column, 2);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(gxml, "session_view")), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("roomjid",
							     renderer, "text",
							     3, NULL);
	gtk_tree_view_column_set_sort_column_id(column, 3);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(gxml, "session_view")), column);	
			
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("hash",
							     renderer, "text",
							     4, NULL);
	gtk_tree_view_column_set_sort_column_id(column, 4);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(gxml, "session_view")), column);
		
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("plain",
							     renderer, "text",
							     5, NULL);
	gtk_tree_view_column_set_sort_column_id(column, 5);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(gxml, "session_view")), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("status",
							     renderer, "text",
							     6, NULL);
	gtk_tree_view_column_set_sort_column_id(column, 6);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(gxml, "session_view")), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("result",
							     renderer, "text",
							     7, NULL);
	gtk_tree_view_column_set_sort_column_id(column, 7);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(gxml, "session_view")), column);	
	
	WokXMLTag sessions("session");
	wls->SendSignal("Jabber RandomNumber GetSessions", sessions);
	
	
	std::list <WokXMLTag *>::iterator tagiter;
	
	GtkTreeIter iter;

	
	for( tagiter = sessions.GetTagList("item").begin() ; tagiter != sessions.GetTagList("item").end() ; tagiter++)
	{
		gtk_list_store_prepend (sess_store, &iter);
		gtk_list_store_set (sess_store, &iter, 	0, (*tagiter)->GetAttr("owner").c_str() , 
												1, (*tagiter)->GetAttr("id").c_str() , 
												2, (*tagiter)->GetAttr("session").c_str() , 
												3, (*tagiter)->GetAttr("roomjid").c_str(), 
												4, ((*tagiter)->GetAttr("hashes") + "/" + (*tagiter)->GetAttr("requiredusers")).c_str(),
												5, ((*tagiter)->GetAttr("plains") + "/" + (*tagiter)->GetAttr("requiredusers")).c_str(), -1);
	}
	
	g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "window")), "delete-event", G_CALLBACK (Monitor::DeleteEvent), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget(gxml, "session_view")), "button_press_event", G_CALLBACK (Monitor::PopupMenu), this);
	
}

gboolean
Monitor::DeleteEvent( GtkWidget *widget, GdkEvent *event, Monitor *c)
{
	//c->SaveConfig();
	g_object_unref (c->gxml);
	c->gxml = NULL;
	return FALSE;
}

gboolean
Monitor::PopupMenu(GtkTreeView *tree_view, GdkEventButton *event, Monitor *c)
{
	GtkTreePath      *path;
	if (event->button == 3 && 
		gtk_tree_view_get_path_at_pos (tree_view, (int)event->x, (int)event->y, &path, NULL, NULL, NULL)) 
	{
		GtkTreeIter iter;

		gtk_tree_model_get_iter (GTK_TREE_MODEL(c->sess_store), &iter, path);

		gchar *roomjid;   
		gchar *owner;
		gchar *session;
		gchar *id;
		
		gtk_tree_model_get(GTK_TREE_MODEL(c->sess_store), &iter, 0, &owner, 1, &id, 2, &session, 3, &roomjid, -1);
		
		
		char buf[20];
		WokXMLTag MenuXML(NULL, "menu");
		sprintf(buf, "%d", event->button);
		MenuXML.AddAttr("button", buf);
		sprintf(buf, "%d", event->time);
		MenuXML.AddAttr("time", buf);
		MenuXML.AddTag("item").AddAttr("signal", "GUI P2PRandMonitor Menu");
		WokXMLTag &data = MenuXML.AddTag("data");
		data.AddAttr("roomjid", roomjid);
		data.AddAttr("owner", owner);
		data.AddAttr("session", session);
		data.AddAttr("id", id);
		
		c->wls->SendSignal("Jabber GUI JIDMenu", &MenuXML);
		
		gtk_tree_path_free(path);
		g_free(roomjid);
		g_free(owner);
		g_free(session);
		g_free(id);
		
		return true;
	}		
	return false;
}

