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

#include "include/GUIPluginWindow.h"
#include <Woklib/WokLibSignal.h>
#include <Woklib/WokXMLTag.h>

GUIPluginWindow::GUIPluginWindow(int *feedback, WLSignal *wls)
{
	GtkWidget *hbox1;
	GtkWidget *scrolledwindow1;
	GtkWidget *vbuttonbox1;
	GtkWidget *button3;
	GtkWidget *button4;
	GtkWidget *button7;
	GtkWidget *button_reload;

	this->feedback = feedback;
	this->wls = wls;

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Plugin Manager");

	hbox1 = gtk_hbox_new (FALSE, 3);
	gtk_widget_show (hbox1);
	gtk_container_add (GTK_CONTAINER (window), hbox1);

	scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow1);
	gtk_box_pack_start (GTK_BOX (hbox1), scrolledwindow1, TRUE, TRUE, 3);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	treeview1 = gtk_tree_view_new ();
	gtk_widget_show (treeview1);
	gtk_container_add (GTK_CONTAINER (scrolledwindow1), treeview1);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview1), FALSE);

	vbuttonbox1 = gtk_vbutton_box_new ();
	gtk_widget_show (vbuttonbox1);
	gtk_box_pack_start (GTK_BOX (hbox1), vbuttonbox1, FALSE, FALSE, 3);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (vbuttonbox1), GTK_BUTTONBOX_SPREAD);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (vbuttonbox1), 3);

	button3 = gtk_button_new_with_mnemonic ("Add");
	gtk_widget_show (button3);
	gtk_container_add (GTK_CONTAINER (vbuttonbox1), button3);
	GTK_WIDGET_SET_FLAGS (button3, GTK_CAN_DEFAULT);

	button_reload = gtk_button_new_with_mnemonic ("Reload");
	gtk_widget_show (button_reload);
	gtk_container_add (GTK_CONTAINER (vbuttonbox1), button_reload);
	GTK_WIDGET_SET_FLAGS (button_reload, GTK_CAN_DEFAULT);
	
	button4 = gtk_button_new_with_mnemonic ("Remove");
	gtk_widget_show (button4);
	gtk_container_add (GTK_CONTAINER (vbuttonbox1), button4);
	GTK_WIDGET_SET_FLAGS (button4, GTK_CAN_DEFAULT);

	button7 = gtk_button_new_with_mnemonic ("Close");
	gtk_widget_show (button7);
	gtk_container_add (GTK_CONTAINER (vbuttonbox1), button7);
	GTK_WIDGET_SET_FLAGS (button7, GTK_CAN_DEFAULT);
		
	g_signal_connect (G_OBJECT (button7), "clicked",
		G_CALLBACK (GUIPluginWindow::Cancel_Button), this);
	g_signal_connect (G_OBJECT (button4), "clicked",
	      G_CALLBACK (GUIPluginWindow::Remove_Button), this);
	g_signal_connect (G_OBJECT (button3), "clicked",
	      G_CALLBACK (GUIPluginWindow::Add_Button), this);
	g_signal_connect (G_OBJECT (button_reload), "clicked",
	      G_CALLBACK (GUIPluginWindow::Reload_Button), this);
	g_signal_connect (G_OBJECT (window), "destroy",
	   G_CALLBACK (GUIPluginWindow::Destroy), this);
	
	GtkCellRenderer *renderer;
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview1), -1,
							     "info",
							     renderer, "text",
							     0, NULL);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview1), -1,
							     "version",
							     renderer, "text",
							     1, NULL);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview1), -1,
							     "filename",
							     renderer, "text",
							     2, NULL);
	g_object_set (G_OBJECT (renderer), "xalign", 0.0, NULL);
	
	model = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model (GTK_TREE_VIEW
		(treeview1), GTK_TREE_MODEL (model));
	
	DisplayPlugins();
	
	gtk_window_set_default_size(GTK_WINDOW(window), 500, 300);
	gtk_widget_show(window);
}


GUIPluginWindow::~GUIPluginWindow()
{
	*feedback = false;
}


void
GUIPluginWindow::DisplayPlugins()
{
	gtk_list_store_clear(model);
	WokXMLTag tag(NULL, "signal");
	wls->SendSignal("Woklib Plugin Get",tag);
	
	std::list<WokXMLTag *>::iterator iter;
	for( iter = tag.GetTagList("plugin").begin(); iter != tag.GetTagList("plugin").end(); iter++ )
	{
		std::string filename, info, version;

		filename = (*iter)->GetFirstTag("filename").GetBody();
		info = (*iter)->GetFirstTag("info").GetBody();
		version = (*iter)->GetFirstTag("version").GetBody();
		
		GtkTreeIter RosterIter;			 
		gtk_list_store_append (model, &RosterIter);
		gtk_list_store_set (model, &RosterIter,
				    2, filename.c_str(),
				    0, info.c_str(),
					1, version.c_str(),
				    -1);
	}
}

void
GUIPluginWindow::Destroy( GtkWidget *widget, gpointer   user_data )
{
	GUIPluginWindow *data;
	data = static_cast < GUIPluginWindow * > ( user_data );
	delete data;
}

void
GUIPluginWindow::Cancel_Button( GtkWidget *widget, gpointer sig_data )
{
	GUIPluginWindow *data;
	data = static_cast < GUIPluginWindow *> ( sig_data);
	
	gtk_widget_destroy( data->window );
}
void 
GUIPluginWindow::load_plugin (GtkFileSelection *file_selector, gpointer sig_data )
{
	GUIPluginWindow *data;
	data = static_cast < GUIPluginWindow *> ( sig_data);

	const gchar *selected_filename;
	selected_filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (data->file_selector));
	
	WokXMLTag tag(NULL, "add");
	tag.AddAttr("filename", selected_filename);
	data->wls->SendSignal("Woklib Plugin Add", &tag);
	
	data->DisplayPlugins();
}

void
GUIPluginWindow::Remove_Button( GtkWidget *widget, GUIPluginWindow *c )
{
	GtkTreeIter       iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(c->treeview1));
	
	if(gtk_tree_selection_get_selected(selection,  NULL, &iter));
	{
		gchar *filename;
		gtk_tree_model_get(GTK_TREE_MODEL(c->model), &iter, 2, &filename, -1);
		
		WokXMLTag tag(NULL, "remove");
		tag.AddAttr("filename", filename);
		c->wls->SendSignal("Woklib Plugin Remove", &tag);
		
		g_free(filename);
	}
	
	c->DisplayPlugins();
	
}

void
GUIPluginWindow::Reload_Button( GtkWidget *widget, GUIPluginWindow *c )
{
	GtkTreeIter       iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(c->treeview1));
	
	if(gtk_tree_selection_get_selected(selection,  NULL, &iter));
	{
		gchar *filename;
		gtk_tree_model_get(GTK_TREE_MODEL(c->model), &iter, 2, &filename, -1);
		
		WokXMLTag unloadtag(NULL, "remove");
		unloadtag.AddAttr("filename", filename);
		c->wls->SendSignal("Woklib Plugin Remove", &unloadtag);
		
		WokXMLTag loadtag(NULL, "remove");
		loadtag.AddAttr("filename", filename);
		c->wls->SendSignal("Woklib Plugin Add", &loadtag);
		
		
		g_free(filename);
	}
	
	c->DisplayPlugins();
	
}


void
GUIPluginWindow::Add_Button( GtkWidget *widget, gpointer sig_data )
{
	GUIPluginWindow *data;
	data = static_cast < GUIPluginWindow *> ( sig_data);


   data->file_selector = gtk_file_selection_new ("Select plugin to be loaded.");
   
   g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (data->file_selector)->ok_button),
                     "clicked",
                     G_CALLBACK (GUIPluginWindow::load_plugin),
                     sig_data );
   			   
   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (data->file_selector)->ok_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy), 
                             (gpointer) data->file_selector); 

   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (data->file_selector)->cancel_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) data->file_selector); 
   
   gtk_widget_show (data->file_selector);
}
