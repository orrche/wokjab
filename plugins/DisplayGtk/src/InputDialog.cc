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
// Class: InputDialog
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Fri Sep  9 15:21:46 2005
//

#include "InputDialog.h"
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sstream>

InputDialog::InputDialog(WLSignal *wls) : WLSignalInstance(wls)
{
	xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/displaygtk.inputdialog.glade", NULL, NULL);

	window = glade_xml_get_widget (xml, "inputwindow");
	GtkWidget *sendbutton = glade_xml_get_widget (xml, "SendButton");
	
	// Session chooser
	sessionmenu = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	GtkWidget *sessionchooser = glade_xml_get_widget (xml, "SessionSelector");
	
	gtk_combo_box_set_model(GTK_COMBO_BOX(sessionchooser) , GTK_TREE_MODEL(sessionmenu));
	
	
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
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

	
	g_signal_connect (G_OBJECT (window), "delete_event",
			G_CALLBACK (InputDialog::Delete), this);	
	g_signal_connect (G_OBJECT (sendbutton), "clicked",
			G_CALLBACK (InputDialog::SendButton), this);
}


InputDialog::~InputDialog()
{
	if ( xml )
	{
		g_object_unref(xml);
		xml = NULL;
	}
}

gboolean 
InputDialog::Delete(GtkWidget *widget, GdkEvent *event, InputDialog *c)
{
	delete c;
	return false;
}

gboolean
InputDialog::SendButton(GtkButton *button, InputDialog *c)
{
	GtkTreeIter treeiter;
	if( !gtk_combo_box_get_active_iter(GTK_COMBO_BOX(glade_xml_get_widget (c->xml, "SessionSelector")), &treeiter) )
	{
		return false;
	}
		
	gchar *session;
	
	gtk_tree_model_get (GTK_TREE_MODEL(c->sessionmenu), &treeiter, 1, &session, -1);

	GtkWidget *text = glade_xml_get_widget (c->xml, "InputView");
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
	std::stringstream str;
	GtkTextIter start_iter, end_iter;
	
	
	gtk_text_buffer_get_start_iter(buffer, &start_iter);
	gtk_text_buffer_get_end_iter(buffer, &end_iter);
		
	WokXMLTag tag(NULL,"message");
	tag.AddAttr("session", session);
	gchar *buftxt;
	buftxt = gtk_text_buffer_get_text(buffer,&start_iter, &end_iter, false);
	str << buftxt;
	g_object_unref(buftxt);
	str >> tag;
	
	c->wls->SendSignal("Jabber XML Send", &tag);
	
	g_object_unref(session);
	gtk_widget_destroy(c->window);
	return false;
}
