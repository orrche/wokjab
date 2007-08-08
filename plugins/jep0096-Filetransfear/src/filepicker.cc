/***************************************************************************
 *  Copyright (C) 2005  Kent Gustavsson <nedo80@gmail.com>
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


//
// Class: filepicker
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Aug 23 19:51:51 2005
//

#include "filepicker.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

filepicker::filepicker(WLSignal *wls, const std::string &session, const std::string &to) : WLSignalInstance(wls),
session(session),
to(to)
{
	gxml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/filepicker.glade", NULL, NULL);

	
	g_signal_connect_swapped (G_OBJECT (glade_xml_get_widget (gxml, "cancel_button")), "clicked",
			      G_CALLBACK (gtk_widget_destroy),
                              G_OBJECT (glade_xml_get_widget (gxml, "window")));
	g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "window")), "destroy",
					(void(*)())G_CALLBACK (filepicker::Destroy), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "ok_button")), "clicked",
					G_CALLBACK(filepicker::ButtonPress), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "button_add")), "clicked",
					G_CALLBACK(filepicker::ButtonAdd), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "button_delete")), "clicked",
					G_CALLBACK(filepicker::ButtonRemove), this);
	
	model = gtk_list_store_new(1, G_TYPE_STRING);
	
	GtkCellRenderer *renderer;  
	GtkTreeViewColumn *column;
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Address", renderer, "text", 0, NULL);		
	
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(gxml, "proxylist")), column);
	gtk_tree_view_set_model (GTK_TREE_VIEW (glade_xml_get_widget(gxml, "proxylist")), GTK_TREE_MODEL (model));
	
	config = NULL;
	EXP_SIGHOOK("Config XML Change /file-transfear/proxy", &filepicker::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/file-transfear/proxy");
	wls->SendSignal("Config XML Trigger", &conftag);
 
	
	GtkTreeIter tIter;
	if ( gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &tIter) == TRUE )
	{
		GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(glade_xml_get_widget(gxml, "proxylist")));
		gtk_tree_selection_select_iter(selection, &tIter);
	}
}


filepicker::~filepicker()
{
	g_object_unref(gxml);
}

int
filepicker::ReadConfig(WokXMLTag *tag)
{
	delete config;
	config = new WokXMLTag (tag->GetFirstTag("config"));
		
	gtk_list_store_clear(GTK_LIST_STORE(model));
	
	std::list <WokXMLTag *>::iterator listiter;
	
	for( listiter = config->GetFirstTag("proxy").GetTagList("item").begin() ; listiter != config->GetFirstTag("proxy").GetTagList("item").end() ; listiter++)
	{
		GtkTreeIter tIter;			 
		gtk_list_store_append (model, &tIter);
		gtk_list_store_set (model, &tIter,
			    0, (*listiter)->GetBody().c_str(),
			    -1);
	}
	
	return 1;	
}

void
filepicker::Destroy(GtkWidget * widget, filepicker *c)
{
	delete c;
}

void 
filepicker::ButtonAdd (GtkButton *button, filepicker *c)
{
	std::string proxy = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (c->gxml, "proxyentry")));
	
	c->config->GetFirstTag("proxy").AddTag("item").AddText(proxy);
	
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/file-transfear/proxy");
	conftag.AddTag(c->config);
	
	c->wls->SendSignal("Config XML Store", &conftag);
}

void
filepicker::ButtonRemove (GtkButton *button, filepicker *c)
{
	GtkTreeIter tIter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(glade_xml_get_widget(c->gxml, "proxylist")));
	
	if(gtk_tree_selection_get_selected(selection,  NULL, &tIter) == TRUE)
	{
		gchar *addy;
		gtk_tree_model_get(GTK_TREE_MODEL(c->model), &tIter, 0, &addy, -1);
		
		std::list <WokXMLTag *>::iterator listiter;
	
		for( listiter = c->config->GetFirstTag("proxy").GetTagList("item").begin() ; listiter != c->config->GetFirstTag("proxy").GetTagList("item").end() ; listiter++)
		{
			if ( (*listiter)->GetBody() == addy )
			{
				c->config->GetFirstTag("proxy").RemoveTag(*listiter);
				break;
			}
		}
	
		WokXMLTag conftag(NULL, "config");
		conftag.AddAttr("path", "/file-transfear/proxy");
		conftag.AddTag(c->config);
		
		c->wls->SendSignal("Config XML Store", &conftag);
		g_free(addy);
	}
}

void 
filepicker::ButtonPress (GtkButton *button, filepicker *c)
{
	WokXMLTag tag(NULL, "send");
	tag.AddAttr("to", c->to);
	tag.AddAttr("session", c->session);
	
	tag.AddAttr("name", gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(glade_xml_get_widget (c->gxml, "chooser"))));
	std::string rate = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (c->gxml, "rateentry")));
	if ( atoi(rate.c_str()) > 0 )
		tag.AddAttr("rate", rate + "000");
	
	std::string proxy("");
	GtkTreeIter tIter;
	
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(glade_xml_get_widget(c->gxml, "proxylist")));
	
	if(gtk_tree_selection_get_selected(selection,  NULL, &tIter) == TRUE && &tIter)
	{
		gchar *addy;
		gtk_tree_model_get(GTK_TREE_MODEL(c->model), &tIter, 0, &addy, -1);
		proxy = addy;
		g_free(addy);
	}
	
	tag.AddAttr("proxy", proxy);
	
	c->wls->SendSignal("Jabber Stream File Send", &tag);
	gtk_widget_destroy(glade_xml_get_widget (c->gxml, "window"));
}
