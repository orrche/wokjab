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


//
// Class: ScriptLoader
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Mar 21 20:32:31 2006
//

#include "ScriptLoader.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


ScriptLoader::ScriptLoader(WLSignal *wls) : WoklibPlugin(wls)
{
	xml = NULL;
	EXP_SIGHOOK("GetMenu", &ScriptLoader::Menu, 1000);
	EXP_SIGHOOK("Jabber ScriptLoader Show", &ScriptLoader::ShowWidget, 1000);
}


ScriptLoader::~ScriptLoader()
{
	if (xml)
		gtk_widget_destroy(glade_xml_get_widget (xml, "window"));
}

int
ScriptLoader::Menu(WokXMLTag *tag)
{
	WokXMLTag &item = tag->AddTag("item");
	item.AddAttr("signal", "Jabber ScriptLoader Show");
	item.AddAttr("name", "Scripts");
	
	return true;
}

void
ScriptLoader::UpdateList()
{
	gtk_list_store_clear (script_store);
	WokXMLTag scripts(NULL, "scripts");
	
	wls->SendSignal("Python Script GetList", scripts);
	std::list <WokXMLTag *>::iterator iter;
	std::list <WokXMLTag *> *list = &scripts.GetTagList("item");
	
	for ( iter = list->begin(); iter != list->end() ; iter++ )
	{
		GtkTreeIter treeiter;
		
		gtk_list_store_append (script_store, &treeiter);  /* Acquire a top-level iterator */
		gtk_list_store_set (script_store, &treeiter,
												0, (*iter)->GetAttr("file").c_str(),
												-1);
	}


}

int
ScriptLoader::ShowWidget(WokXMLTag *tag)
{
	if ( xml )
	{
		gtk_window_present (GTK_WINDOW(glade_xml_get_widget (xml, "window")));
	}
	else
	{
		GtkCellRenderer *renderer;
		GtkTreeViewColumn *column;

		xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/scriptloader.glade", "window", NULL);
		g_signal_connect (G_OBJECT(glade_xml_get_widget (xml, "window")), "destroy", G_CALLBACK (ScriptLoader::Destroy), this);
		g_signal_connect (G_OBJECT(glade_xml_get_widget (xml, "button_refresh")), "clicked", G_CALLBACK (ScriptLoader::Reload), this);
		g_signal_connect (G_OBJECT(glade_xml_get_widget (xml, "button_remove")), "clicked", G_CALLBACK (ScriptLoader::Unload), this);
		g_signal_connect (G_OBJECT(glade_xml_get_widget (xml, "button_add")), "clicked", G_CALLBACK (ScriptLoader::Load), this);
		
		script_store = gtk_list_store_new (1, G_TYPE_STRING);
		gtk_tree_view_set_model (GTK_TREE_VIEW(glade_xml_get_widget (xml, "scriptlist")), GTK_TREE_MODEL(script_store));
	
		renderer = gtk_cell_renderer_text_new ();
		column = gtk_tree_view_column_new_with_attributes ("Script", renderer, "text", 0, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget (xml, "scriptlist")), column);
	
	}

	UpdateList();
	
	return true;
}

void
ScriptLoader::Destroy( GtkWidget *widget, ScriptLoader *c )
{
	g_object_unref ( c->xml );
	c->xml = NULL;
}

void
ScriptLoader::Reload(GtkButton *button, ScriptLoader *c)
{
	GtkTreeIter iter;
	GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW(glade_xml_get_widget (c->xml, "scriptlist")));
	
	if( !gtk_tree_selection_get_selected (sel, NULL , &iter))
		return;
		
	gchar *file;
	gtk_tree_model_get (GTK_TREE_MODEL(c->script_store), &iter, 0, &file, -1);

	WokXMLTag script(NULL, "script");
	script.AddAttr("file", file);
	
	c->wls->SendSignal("Python Script Unload", script);
	c->wls->SendSignal("Python Script Load", script);
	
	g_free(file);
}

void
ScriptLoader::Unload(GtkButton *button, ScriptLoader *c)
{
	GtkTreeIter iter;
	GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW(glade_xml_get_widget (c->xml, "scriptlist")));
	
	if( !gtk_tree_selection_get_selected (sel, NULL , &iter))
		return;
		
	gchar *file;
	gtk_tree_model_get (GTK_TREE_MODEL(c->script_store), &iter, 0, &file, -1);

	WokXMLTag script(NULL, "script");
	script.AddAttr("file", file);
	
	c->wls->SendSignal("Python Script Unload", script);
	
	g_free(file);
	c->UpdateList();
}

void
ScriptLoader::LoadButton(GtkButton *button, ScriptLoader *c)
{
	const gchar *file;
	file = gtk_file_selection_get_filename (GTK_FILE_SELECTION (c->file_selector));
		
	WokXMLTag script(NULL, "script");
	script.AddAttr("file", file);
	
	c->wls->SendSignal("Python Script Load", script);
	c->UpdateList();
}

void
ScriptLoader::Load(GtkButton *button, ScriptLoader *c)
{
   c->file_selector = gtk_file_selection_new ("Select script to be loaded.");
   
   g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (c->file_selector)->ok_button),
                     "clicked",
                     G_CALLBACK (ScriptLoader::LoadButton),
                     c );
   			   
   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (c->file_selector)->ok_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy), 
                             (gpointer) c->file_selector); 

   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (c->file_selector)->cancel_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) c->file_selector); 
   
   gtk_widget_show (c->file_selector);
}
