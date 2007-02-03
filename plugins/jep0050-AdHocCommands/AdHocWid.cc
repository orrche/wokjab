/***************************************************************************
 *  Copyright (C) 2006  Kent Gustavsson <nedo80@gmail.com>
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
// Class: AdHocWid
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sun Oct 15 21:24:22 2006
//

#include "AdHocWid.h"
#include "AdHocSession.h"
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


AdHocWid::AdHocWid(WLSignal *wls, std::string session, std::string jid, std::string node) : WLSignalInstance(wls),
session(session)
{
	WokXMLTag msgtag(NULL,"message");
	msgtag.AddAttr("session", session);
	WokXMLTag &tag = msgtag.AddTag("iq");
	tag.AddAttr("to", jid );
	tag.AddAttr("type", "get");
	WokXMLTag *query;
	query = &tag.AddTag("query");
	query->AddAttr("xmlns", "http://jabber.org/protocol/disco#info");
	query->AddAttr("node", "http://jabber.org/protocol/commands");

	wls->SendSignal("Jabber XML IQ Send", &msgtag);
	EXP_SIGHOOK("Jabber XML IQ ID " + tag.GetAttr("id"), &AdHocWid::Set, 1000);
	
	xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/AdHocWid.glade", NULL, NULL);
	g_signal_connect (G_OBJECT (glade_xml_get_widget(xml, "window")), "destroy",
				G_CALLBACK (AdHocWid::Destroy), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget(xml, "close_button")), "clicked",
					G_CALLBACK (AdHocWid::CloseButton), this);
	GtkCellRenderer *renderer;

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (glade_xml_get_widget(xml, "command_view")),
        -1, "Command", renderer, "markup", NAME_COLUMN, NULL);

	model = gtk_list_store_new (NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	gtk_tree_view_set_model (GTK_TREE_VIEW
				 (glade_xml_get_widget(xml,"command_view")), GTK_TREE_MODEL (model));
				 
	g_signal_connect (G_OBJECT (glade_xml_get_widget(xml, "exec_button")), "clicked",
					G_CALLBACK (AdHocWid::ExecButton), this);
}


AdHocWid::~AdHocWid()
{
	g_object_unref(xml);
}

void
AdHocWid::Destroy(GtkWidget *widget, AdHocWid *c)
{
	delete c;
}

void
AdHocWid::CloseButton(GtkButton *button, AdHocWid *c)
{
	gtk_widget_destroy(glade_xml_get_widget(c->xml, "window"));
}

void
AdHocWid::ExecButton(GtkButton *button, AdHocWid *c)
{
	GtkTreeIter       iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(glade_xml_get_widget(c->xml, "command_view")));
	
	if(gtk_tree_selection_get_selected(selection,  NULL, &iter) != FALSE)
	{
		gchar *command, *jid;
		gtk_tree_model_get(GTK_TREE_MODEL(c->model), &iter, COMMAND_COLUMN, &command, JID_COLUMN, &jid, -1);
		
		WokXMLTag msgtag(NULL,"message");
		msgtag.AddAttr("session", c->session);
		WokXMLTag &tag = msgtag.AddTag("iq");
		tag.AddAttr("to", jid );
		tag.AddAttr("type", "set");
		WokXMLTag *query;
		query = &tag.AddTag("command");
		query->AddAttr("xmlns", "http://jabber.org/protocol/commands");
		query->AddAttr("node", command);
		query->AddAttr("action", "execute");

		c->wls->SendSignal("Jabber AdHoc Execute", &msgtag);
	
		g_free(jid);
		g_free(command);
	}
}
  
int
AdHocWid::Set(WokXMLTag *tag)
{
	gtk_list_store_clear (GTK_LIST_STORE(model));
	
	std::list<WokXMLTag *>::iterator iter;
	std::list<WokXMLTag *> &list = tag->GetFirstTag("iq").GetFirstTag("query").GetTagList("item");
	
	
	for( iter = list.begin(); iter != list.end() ; iter++)
	{
		GtkTreeIter treeiter;
		gtk_list_store_append(GTK_LIST_STORE(model), &treeiter);
		gtk_list_store_set(GTK_LIST_STORE(model), &treeiter, 
						NAME_COLUMN , (*iter)->GetAttr("name").c_str(),
						COMMAND_COLUMN, (*iter)->GetAttr("node").c_str(),
						JID_COLUMN, tag->GetFirstTag("iq").GetAttr("from").c_str(), -1);
	}

	return 1;
}

