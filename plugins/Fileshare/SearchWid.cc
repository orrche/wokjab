/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <nedo80@gmail.com>
 ****************************************************************************/
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "SearchWid.h"


SearchWid::SearchWid(WLSignal *wls) : WLSignalInstance(wls)
{
	xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/SearchWid.glade", NULL, NULL);
	
	GtkCellRenderer *renderer;
/*
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (glade_xml_get_widget(xml,"resultview")),
        -1, "id", renderer, "text", 0, NULL);
*/
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (glade_xml_get_widget(xml,"resultview")),
        -1, "JID", renderer, "text", 1, NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (glade_xml_get_widget(xml,"resultview")),
        -1, "Name", renderer, "text", 2, NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (glade_xml_get_widget(xml,"resultview")),
        -1, "Size", renderer, "text", 3, NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (glade_xml_get_widget(xml,"resultview")),
        -1, "Hash", renderer, "text", 4, NULL);
	
	search_store = gtk_list_store_new (6, G_TYPE_STRING, G_TYPE_STRING, 
												G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	
	gtk_tree_view_set_model (GTK_TREE_VIEW (glade_xml_get_widget(xml,"resultview")), GTK_TREE_MODEL(search_store));
	
	gtk_tree_view_get_selection (GTK_TREE_VIEW (glade_xml_get_widget(xml,"resultview")));
	gtk_tree_selection_set_mode(GTK_TREE_SELECTION(gtk_tree_view_get_selection (GTK_TREE_VIEW (glade_xml_get_widget(xml,"resultview")))),
                                                         GTK_SELECTION_MULTIPLE);
	g_signal_connect (G_OBJECT (glade_xml_get_widget(xml,"resultview")), "button_press_event", 
				G_CALLBACK (SearchWid::popup_menu), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget(xml,"find_button")), "clicked",
				G_CALLBACK (SearchWid::Search), this);
				
	EXP_SIGHOOK("Jabber FileShare FileMenu", &SearchWid::Menu, 1000);
}


SearchWid::~SearchWid()
{
	

}

int
SearchWid::Menu(WokXMLTag *tag)
{
	WokXMLTag *item;
	item = & tag->AddTag("item");
	item->AddAttr("name", "Download");
	item->AddAttr("signal", "Jabber FileShare MenuDownload");
	
		item = & tag->AddTag("item");
	item->AddAttr("name", "----");
	return 1;
}

gboolean
SearchWid::popup_menu(GtkTreeView *tree_view, GdkEventButton *event, SearchWid *c)
{
	GtkTreePath      *path;
	if (event->button == 3 && 
		gtk_tree_view_get_path_at_pos (tree_view, (int)event->x, (int)event->y, &path, NULL, NULL, NULL)) 
	{
		GtkTreeIter iter;

		gtk_tree_model_get_iter (GTK_TREE_MODEL(c->search_store), &iter, path);

		gchar *file_id;   
		gchar *jid;
		gchar *session;
		gchar *filename;
		
		gtk_tree_model_get(GTK_TREE_MODEL(c->search_store), &iter,
        1, &jid, 0, &file_id, 2, &filename, 5, &session, -1);
		
		char buf[20];
		WokXMLTag MenuXML(NULL, "menu");
		sprintf(buf, "%d", event->button);
		MenuXML.AddAttr("button", buf);
		sprintf(buf, "%d", event->time);
		MenuXML.AddAttr("time", buf);
		MenuXML.AddTag("item").AddAttr("signal", "Jabber FileShare FileMenu");
		MenuXML.AddTag("item").AddAttr("signal", "Jabber GUI Roster GetJIDMenu");
		MenuXML.AddTag("item").AddAttr("signal", "Jabber GUI GetJIDMenu NoRoster");
		MenuXML.AddTag("item").AddAttr("signal", "Jabber GUI GetJIDMenu");

		WokXMLTag &data = MenuXML.AddTag("data");
		data.AddAttr("jid", jid);
		data.AddAttr("session", session);
		data.AddAttr("file_id", file_id);
		data.AddAttr("filename", filename);
		c->wls->SendSignal("Jabber GUI JIDMenu", &MenuXML);
		
		g_free(jid);
		g_free(file_id);
		g_free(session);
		g_free(filename);
		
		return true;
	}		
	return false;
}

int
SearchWid::IncommingResult(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator iter;
	std::list <WokXMLTag *> *list =  NULL;
	
	for( iter = tag->GetFirstTag("message").GetTagList("x").begin() ; iter != tag->GetFirstTag("message").GetTagList("x").end() ; iter++)
	{
		if ( (*iter)->GetAttr("xmlns") == "http://sf.wokjab.net/fileshare")
			list = &(*iter)->GetFirstTag("result").GetTagList("item");	
	}
	if ( list )
	{
		for( iter = list->begin() ; iter != list->end() ; iter++)
		{
			GtkTreeIter titer;
			
			gtk_list_store_append (search_store, &titer);
			gtk_list_store_set (search_store, &titer, 
										0, (*iter)->GetAttr("id").c_str(),
										1, tag->GetFirstTag("message").GetAttr("from").c_str(),
										2, (*iter)->GetAttr("name").c_str(),
										3, (*iter)->GetAttr("size").c_str(),
										4, (*iter)->GetAttr("hash").c_str(),
										5, tag->GetAttr("session").c_str(),
										-1 );
		}
	}
	return 1;
}

void
SearchWid::Hook(std::string thread)
{
	EXP_SIGHOOK("Jabber FileShare SearchResult " + thread, &SearchWid::IncommingResult, 500);
}

void
SearchWid::Search(GtkButton *button, SearchWid *c) 
{
	WokXMLTag query(NULL, "query");
	query.AddTag("condition").AddText(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(c->xml, "entry"))));

	c->wls->SendSignal("Jabber FileShare Search", query);
	
	c->Hook(query.GetAttr("thread"));
}
