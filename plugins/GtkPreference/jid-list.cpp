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

#include "jid-list.hpp"


JIDList::JIDList(WokXMLTag *tag) : GtkPCommon(tag)
{
	std::string name;
	if( tag->GetAttr("label").size() )
		name = tag->GetAttr("label");
	else
		name = tag->GetName();
	
	gxml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/preference.jidlist.glade", "mainbox", NULL);
	
	gtk_label_set_text(GTK_LABEL(glade_xml_get_widget(gxml, "label")), name.c_str());
	
	GtkListStore *store;
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(glade_xml_get_widget(gxml, "list")), GTK_TREE_MODEL(store));
	gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW(glade_xml_get_widget(gxml, "list"))), GTK_SELECTION_MULTIPLE);	
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	/*
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("nick",
							     renderer, "text",
							     0, NULL);
	gtk_tree_view_column_set_sort_column_id(column, 0);
	gtk_tree_view_append_column (GTK_TREE_VIEW(glade_xml_get_widget(gxml, "list")), column);
	*/
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("jid",
							     renderer, "text",
							     1, NULL);
	gtk_tree_view_column_set_sort_column_id(column, 1);
	gtk_tree_view_append_column (GTK_TREE_VIEW(glade_xml_get_widget(gxml, "list")), column);
	
	GtkTreeIter iter;
	std::list <WokXMLTag *>::iterator citer;
	for( citer = conf->GetFirstTag("list").GetTagList("item").begin(); citer != conf->GetFirstTag("list").GetTagList("item").end(); citer++)
	{
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, 1, (*citer)->GetAttr("data").c_str(), -1);
	}
	
	
	enum
    {
      TARGET_STRING,
      TARGET_URL
    };
	static GtkTargetEntry target_entry[] =
    {
      { "STRING",        0, TARGET_STRING },
      { "text/plain",    0, TARGET_STRING },
    };
	gtk_drag_dest_set(glade_xml_get_widget (gxml, "list"), GTK_DEST_DEFAULT_ALL, target_entry, 2, (GdkDragAction) (GDK_ACTION_COPY|GDK_ACTION_MOVE|GDK_ACTION_LINK));
	g_signal_connect(glade_xml_get_widget (gxml, "list"), "drag_data_received", G_CALLBACK(JIDList::DataReceived), this);
	
	g_signal_connect(glade_xml_get_widget (gxml, "delete"), "clicked", G_CALLBACK(JIDList::Remove), this);
}


JIDList::~JIDList()
{
	
}

void
JIDList::Remove(GtkButton *button, JIDList *c)
{
	GtkTreeIter iter;
	GtkTreeModel *model;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(glade_xml_get_widget(c->gxml, "list")));
	
	for(;;)
	{
		GtkTreeSelection* select =  gtk_tree_view_get_selection(GTK_TREE_VIEW(glade_xml_get_widget(c->gxml, "list")));
		GList *list =  gtk_tree_selection_get_selected_rows(select, &model);
		if ( list )
		{
			gtk_tree_model_get_iter(GTK_TREE_MODEL(model), &iter, (GtkTreePath *) list->data);
			gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
		}
		else
			break;
		

		g_list_foreach (list, ( void(*)(void*, void*)) gtk_tree_path_free, NULL);
		g_list_free (list);
	}
}

gboolean
JIDList::DragMotion(GtkWidget *widget, GdkDragContext *drag_context, gint x, gint y, guint time, JIDList *c)
{
	std::cout << "Motion: " << x << ", " << y << std::endl;
	return TRUE;
}

void
JIDList::DataReceived(GtkWidget *wgt, GdkDragContext *context, int x, int y,
                        GtkSelectionData *seldata, guint info, guint time, JIDList *c)
{	
	WokXMLTag data("data");
	data.Add((const char*)gtk_selection_data_get_text (seldata));
	
	GtkListStore *store;
	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(glade_xml_get_widget(c->gxml, "list"))));
	
	std::list <WokXMLTag *>::iterator jiditer;
	GtkTreeIter iter;
	for(  jiditer = data.GetFirstTag("dnd", "dnd").GetTagList("item", "xmpp:jid").begin() ; jiditer != data.GetFirstTag("dnd", "dnd").GetTagList("item", "xmpp:jid").end() ; jiditer++)
	{
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, 1, (*jiditer)->GetAttr("data").c_str(), -1);
	}
	
	gtk_drag_finish (context, TRUE, FALSE, time);
}

GtkWidget *
JIDList::GetWidget()
{
	return glade_xml_get_widget(gxml, "mainbox");
}	

void
JIDList::Save()
{
	GtkTreeIter iter;
	GtkListStore *store;
	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(glade_xml_get_widget(gxml, "list"))));
	
	while ( conf->GetTagList("list").begin() != conf->GetTagList("list").end() )
		conf->RemoveTag(*conf->GetTagList("list").begin());

	WokXMLTag &list = conf->AddTag("list");
	if ( gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter) == TRUE )
	{
		do 
		{
			gchar *jid;
			gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, 1, &jid, -1);
			if(jid)
				list.AddTag("item").AddAttr("data", jid);
			
			g_free(jid);
		} while ( gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter) != FALSE);
	}
}

