/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <oden@gmx.net>
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

#include <dirent.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

GUIPluginWindow::GUIPluginWindow(int *feedback, WLSignal *wls)
{
	xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/plugin.glade", NULL, NULL);
	
	filexml = NULL;
	
	this->feedback = feedback;
	this->wls = wls;

	g_signal_connect (glade_xml_get_widget(xml, "close_button"), "clicked",
		G_CALLBACK (GUIPluginWindow::Cancel_Button), this);
	g_signal_connect (glade_xml_get_widget(xml, "delete_button"), "clicked",
	      G_CALLBACK (GUIPluginWindow::Remove_Button), this);
	g_signal_connect (glade_xml_get_widget(xml, "add_button"), "clicked",
	      G_CALLBACK (GUIPluginWindow::Add_Button), this);
	g_signal_connect (glade_xml_get_widget(xml, "revert_button"), "clicked",
	      G_CALLBACK (GUIPluginWindow::Reload_Button), this);
	g_signal_connect (glade_xml_get_widget(xml, "window"), "destroy",
	   G_CALLBACK (GUIPluginWindow::Destroy), this);
	
	GtkCellRenderer *renderer;  
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_toggle_new ();
	g_signal_connect (renderer, "toggled",
	    G_CALLBACK (GUIPluginWindow::Toggled), this);

	column = gtk_tree_view_column_new_with_attributes("Loaded",
								renderer,
								"active", 3,
								NULL);
	gtk_tree_view_column_set_sort_column_id(column, 3);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(xml, "plugin_view")), column);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("info",
							     renderer, "text",
							     0, NULL);
	gtk_tree_view_column_set_sort_column_id(column, 0);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(xml, "plugin_view")), column);
	column = gtk_tree_view_column_new_with_attributes("version",
							     renderer, "text",
							     1, NULL);
	gtk_tree_view_column_set_sort_column_id(column, 1);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(xml, "plugin_view")), column);
	column = gtk_tree_view_column_new_with_attributes("filename",
							     renderer, "text",
							     2, NULL);
	gtk_tree_view_column_set_sort_column_id(column, 2);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(xml, "plugin_view")), column);
	g_object_set (G_OBJECT (renderer), "xalign", 0.0, NULL);

	model = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);
	gtk_tree_view_set_model (GTK_TREE_VIEW
		(glade_xml_get_widget(xml, "plugin_view")), GTK_TREE_MODEL (model));
	
	DisplayPlugins();
}


GUIPluginWindow::~GUIPluginWindow()
{
	*feedback = false;
}

void
GUIPluginWindow::Toggled (GtkCellRendererToggle *cell, gchar *path_str, GUIPluginWindow *c)
{
	GtkTreeIter  iter;
	GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
	gboolean fixed;
	gchar *filename;

	/* get toggled iter */
	gtk_tree_model_get_iter (GTK_TREE_MODEL(c->model), &iter, path);
	gtk_tree_model_get (GTK_TREE_MODEL(c->model), &iter, 3, &fixed, 2, &filename, -1);
	
	if ( fixed == FALSE )
	{
		WokXMLTag tag(NULL, "add");
		tag.AddAttr("filename", filename);
		c->wls->SendSignal("Woklib Plugin Add", &tag);
	}
	else
	{
		WokXMLTag tag(NULL, "remove");
		tag.AddAttr("filename", filename);
		c->wls->SendSignal("Woklib Plugin Remove", &tag);

	}
	/* do something with the value */
	fixed ^= 1;

	/* set new value */
	gtk_list_store_set (GTK_LIST_STORE (c->model), &iter, 3, fixed, -1);

	/* clean up */
	gtk_tree_path_free (path);
	g_free(filename);
}

void
GUIPluginWindow::DisplayPlugins()
{
	gtk_list_store_clear(model);
	WokXMLTag tag(NULL, "signal");
	wls->SendSignal("Woklib Plugin Get",tag);
	
	std::list<std::string> listed_plugins;
	std::list<WokXMLTag *>::iterator iter;
	for( iter = tag.GetTagList("plugin").begin(); iter != tag.GetTagList("plugin").end(); iter++ )
	{
		std::string filename, info, version;

		filename = (*iter)->GetFirstTag("filename").GetBody();
		info = (*iter)->GetFirstTag("info").GetBody();
		version = (*iter)->GetFirstTag("version").GetBody();
		
		listed_plugins.push_back(filename);
		GtkTreeIter RosterIter;			 
		gtk_list_store_append (model, &RosterIter);
		gtk_list_store_set (model, &RosterIter,
				    2, filename.c_str(),
				    0, info.c_str(),
					1, version.c_str(),
					3, TRUE,
				    -1);
	}

	DIR             *dip;
	struct dirent   *dit;

	if ((dip = opendir(PACKAGE_PLUGIN_DIR"/normal")) == NULL)
	{
    	perror("opendir");
		return;
	}

	while ((dit = readdir(dip)) != NULL)
	{
		std::string file = dit->d_name;
		if ( std::find(listed_plugins.begin(), listed_plugins.end(), PACKAGE_PLUGIN_DIR"/normal/"+file) == listed_plugins.end() )
		{
			if ( file.size() > 3 && file.substr(file.size() - 3) ==  ".so")
			{
				GtkTreeIter RosterIter;			 
				gtk_list_store_append (model, &RosterIter);
				gtk_list_store_set (model, &RosterIter,
						    2,	(PACKAGE_PLUGIN_DIR"/normal/"+file).c_str(),
							3, FALSE,
						    -1);
			}
		}
	}
}

void
GUIPluginWindow::Destroy( GtkWidget *widget, GUIPluginWindow *c)
{
	if ( c->filexml )
		gtk_widget_destroy(glade_xml_get_widget(c->filexml, "window"));
	delete c;
}

void
GUIPluginWindow::load_destroy( GtkWidget *widget, GUIPluginWindow * c )
{
	g_object_unref(c->filexml);
	c->filexml = NULL;
}


void
GUIPluginWindow::Cancel_Button( GtkWidget *widget, GUIPluginWindow *c )
{	
	gtk_widget_destroy( glade_xml_get_widget(c->xml, "window") );
}
void 
GUIPluginWindow::load_plugin (GtkFileSelection *file_selector, GUIPluginWindow *c )
{
	gchar *selected_filename;
	selected_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (glade_xml_get_widget(c->filexml, "chooser")));
	if ( selected_filename == NULL )
		return;
		
	WokXMLTag tag(NULL, "add");
	tag.AddAttr("filename", selected_filename);
	c->wls->SendSignal("Woklib Plugin Add", &tag);
	
	c->DisplayPlugins();
	
	g_free(selected_filename);
}

void
GUIPluginWindow::Remove_Button( GtkWidget *widget, GUIPluginWindow *c )
{
	GtkTreeIter       iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(glade_xml_get_widget(c->xml, "plugin_view")));
	
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
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(glade_xml_get_widget(c->xml, "plugin_view")));
	
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
GUIPluginWindow::Add_Button( GtkWidget *widget, GUIPluginWindow *c )
{
	if(c->filexml)
		return;

	c->filexml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/fileloadplugin.glade", NULL, NULL);
	
	GtkFileFilter *filter = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter, "*.so");
	gtk_file_filter_set_name (filter,"Plugins");
	
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(glade_xml_get_widget(c->filexml, "chooser")), GTK_FILE_FILTER(filter));
	
	g_signal_connect (G_OBJECT (glade_xml_get_widget(c->filexml, "open_button")), "clicked",
						G_CALLBACK(GUIPluginWindow::load_plugin), c);
	g_signal_connect (G_OBJECT (glade_xml_get_widget(c->filexml, "window")), "destroy",
						G_CALLBACK(GUIPluginWindow::load_destroy), c);
						
	g_signal_connect_swapped (G_OBJECT (glade_xml_get_widget(c->filexml, "close_button")), "clicked",
			      G_CALLBACK (gtk_widget_destroy),
            G_OBJECT (glade_xml_get_widget(c->filexml, "window")));
	g_signal_connect_swapped (G_OBJECT (glade_xml_get_widget(c->filexml, "open_button")), "clicked",
			      G_CALLBACK (gtk_widget_destroy),
            G_OBJECT (glade_xml_get_widget(c->filexml, "window")));
}
