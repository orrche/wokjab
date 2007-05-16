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
#include "../include/xdatamultilist.h"



xdatamultilist::xdatamultilist(WLSignal *wls, WLSignalData* wlsd, GtkWidget *vbox)
: xdatabase(wls, wlsd)
{
	GtkWidget *hbox;
	GtkWidget *bbox;
	
	GtkWidget *add_btn;
	GtkWidget *del_btn;
	GtkWidget *up_btn;
	GtkWidget *down_btn;
	GtkWidget *optionscroll;
	GtkWidget *selectedscroll;
	
	hbox = gtk_hbox_new(FALSE, FALSE);
	optionlist = gtk_tree_view_new();
	selectedlist = gtk_tree_view_new();
	bbox = gtk_vbox_new(FALSE, FALSE);
	add_btn = gtk_button_new_with_label("Add");
	del_btn = gtk_button_new_with_label("Del");
	up_btn = gtk_button_new_with_label("Up");
	down_btn = gtk_button_new_with_label("Down");
	
	optionstore = gtk_list_store_new(1, G_TYPE_STRING);
	selectedstore = gtk_list_store_new(1, G_TYPE_STRING);
	optionscroll = gtk_scrolled_window_new(NULL, NULL);
	selectedscroll = gtk_scrolled_window_new(NULL, NULL);
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(optionlist), GTK_TREE_MODEL(optionstore));
	gtk_tree_view_set_model(GTK_TREE_VIEW(selectedlist), GTK_TREE_MODEL(selectedstore));
			
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(optionscroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(selectedscroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(optionscroll), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(selectedscroll), GTK_SHADOW_ETCHED_IN);
	
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("value", renderer, "text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (optionlist), column);
	
		renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("value", renderer, "text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (selectedlist), column);
	
	gtk_box_pack_start(GTK_BOX(bbox), add_btn, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(bbox), del_btn, FALSE, FALSE, 2);
	
	/*
	These where tricker then I thought
	
	gtk_box_pack_start(GTK_BOX(bbox), up_btn, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(bbox), down_btn, FALSE, FALSE, 2);
	*/
	
	gtk_container_add(GTK_CONTAINER(optionscroll), optionlist);
	gtk_container_add(GTK_CONTAINER(selectedscroll), selectedlist);
	
	
	gtk_box_pack_start(GTK_BOX(hbox), optionscroll, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), bbox, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), selectedscroll, TRUE, TRUE, 2);
	
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 2);

	std::list <WokXMLTag *> *list;
	std::list <WokXMLTag *>::iterator iter;
	list = &tag_field->GetTagList("value");
	for( iter = list->begin() ; iter != list->end() ; iter++)
	{
		GtkTreeIter titer;
		
		gtk_list_store_append(optionstore, &titer);
		gtk_list_store_set (optionstore, &titer, 0, (*iter)->GetBody().c_str(), -1);
	}
	
	g_signal_connect (G_OBJECT (add_btn), "clicked",
		G_CALLBACK (xdatamultilist::Add), this);
	g_signal_connect (G_OBJECT (del_btn), "clicked",
		G_CALLBACK (xdatamultilist::Del), this);
	g_signal_connect (G_OBJECT (up_btn), "clicked",
		G_CALLBACK (xdatamultilist::Up), this);
	g_signal_connect (G_OBJECT (down_btn), "clicked",
		G_CALLBACK (xdatamultilist::Down), this);
	
	gtk_widget_show_all(hbox);
	
}

xdatamultilist::~xdatamultilist()
{

}

bool
xdatamultilist::Ready()
{
	return true;
}

void
xdatamultilist::GetData(WokXMLTag &tag)
{
	GtkTreeIter iter;
	if ( gtk_tree_model_get_iter_first(GTK_TREE_MODEL(selectedstore), &iter) == TRUE )
	{
		gchar *value;
		gtk_tree_model_get(GTK_TREE_MODEL(selectedstore), &iter, 0, &value, -1);
		tag.AddTag("value").AddText(value);
		g_free(value);
		
		while ( gtk_tree_model_iter_next (GTK_TREE_MODEL(selectedstore), &iter) == TRUE )
		{		
			gtk_tree_model_get(GTK_TREE_MODEL(selectedstore), &iter, 0, &value, -1);
			tag.AddTag("value").AddText(value);
			g_free(value);
		}
	}
}

void
xdatamultilist::Add(GtkWidget *button, xdatamultilist *c)
{
	GtkTreeIter iter;
	GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW(c->optionlist));
	
	if( !gtk_tree_selection_get_selected (sel, NULL , &iter))
		return;
			
	gchar *value;
	gtk_tree_model_get(GTK_TREE_MODEL(c->optionstore), &iter, 0, &value, -1);
	gtk_list_store_remove(c->optionstore, &iter);
	
	gtk_list_store_append(c->selectedstore, &iter);
	gtk_list_store_set (c->selectedstore, &iter, 0, value, -1);
	
	g_free(value);
}

void
xdatamultilist::Del(GtkWidget *button, xdatamultilist *c)
{
	GtkTreeIter iter;
	GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW(c->selectedlist));
	
	if( !gtk_tree_selection_get_selected (sel, NULL , &iter))
		return;

	gchar *value;
	gtk_tree_model_get(GTK_TREE_MODEL(c->selectedstore), &iter, 0, &value, -1);
	gtk_list_store_remove(c->selectedstore, &iter);
	
	gtk_list_store_append(c->optionstore, &iter);
	gtk_list_store_set (c->optionstore, &iter, 0, value, -1);
	
	g_free(value);
}

void
xdatamultilist::Up(GtkWidget *button, xdatamultilist *c)
{


}

void
xdatamultilist::Down(GtkWidget *button, xdatamultilist *c)
{


}


