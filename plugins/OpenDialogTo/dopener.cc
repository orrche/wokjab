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
// Class: DOpener
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Jan 24 18:30:39 2006
//

#include "dopener.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

DOpener::DOpener(WLSignal *wls, WokXMLTag *tag, Opener *oper) : 
WLSignalInstance(wls),
oper(oper)
{
	GtkCellRenderer *renderer;

	xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/dialogopener.glade", NULL, NULL);
	
	GtkWidget *window = glade_xml_get_widget(xml, "window");
	GtkWidget *okbutton = glade_xml_get_widget(xml, "okbutton");
	GtkWidget *cancelbutton = glade_xml_get_widget(xml, "cancelbutton");
	
	g_signal_connect (G_OBJECT (window), "destroy_event",
			G_CALLBACK (DOpener::Delete), this);	
	g_signal_connect (G_OBJECT (okbutton), "clicked",
			G_CALLBACK (DOpener::OkButton), this);	
	g_signal_connect (G_OBJECT (cancelbutton), "clicked",
			G_CALLBACK (DOpener::CancelButton), this);
	
	sessionmenu = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	sessionchooser = glade_xml_get_widget(xml, "sessionchooser");
	
	gtk_combo_box_set_model(GTK_COMBO_BOX(sessionchooser), GTK_TREE_MODEL(sessionmenu));
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (sessionchooser), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (sessionchooser), renderer,
					"text", 0,
					NULL);
	
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
	

}


DOpener::~DOpener()
{
	GtkWidget *window = glade_xml_get_widget(xml, "window");
	g_object_unref(xml);
	xml = NULL;
	
	gtk_widget_destroy(window);
	
	oper->DialogOpenerRemove(this);
}

gboolean
DOpener::Delete( GtkWidget *widget, DOpener *c)
{	
	if ( c->xml )
		delete c;

	return TRUE;
}

gboolean
DOpener::OkButton( GtkWidget *widget, DOpener *c)
{
	GtkTreeIter treeiter;
	if( !gtk_combo_box_get_active_iter(GTK_COMBO_BOX(c->sessionchooser), &treeiter) )
	{
		return false;
	}
		
	gchar *session;
	gtk_tree_model_get (GTK_TREE_MODEL(c->sessionmenu), &treeiter, 1, &session, -1);
	
	WokXMLTag tag(NULL, "dialog");
	tag.AddAttr("jid", gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(c->xml, "jid_entry"))));
	tag.AddAttr("session", session);
	c->wls->SendSignal("Jabber GUI MessageDialog Open", &tag);
	gtk_widget_destroy(glade_xml_get_widget(c->xml, "window"));
	
	g_free (session);
	return false;
}

gboolean
DOpener::CancelButton( GtkWidget *widget, DOpener *c)
{
	gtk_widget_destroy(glade_xml_get_widget(c->xml, "window"));
	return false;
}
