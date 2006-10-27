/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <nedo80@gmail.com>
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


#include "include/GUIPresentReqWidget.h"
#include <Woklib/WokXMLTag.h>
#include <iostream>

GUIPresentReqWidget::GUIPresentReqWidget (WLSignal * wls):WLSignalInstance (wls)
{
	EXP_SIGHOOK("Jabber XML Presence SubReq", &GUIPresentReqWidget::Presence, 999);
	widget_visible = false;
}


GUIPresentReqWidget::~GUIPresentReqWidget ()
{
}


int
GUIPresentReqWidget::Presence (WokXMLTag * msgintag)
{
	WokXMLTag *tag_presence;
	
	tag_presence = &msgintag->GetFirstTag("presence");
	
	std::string jid = tag_presence->GetAttr("from").substr(0, tag_presence->GetAttr("from").find("/"));

	WokXMLTag *item = new WokXMLTag(NULL, "item");
	item->AddAttr("nick", jid);
	item->AddAttr("session", msgintag->GetAttr("session"));
	
	items[jid] = item;
	
	if (widget_visible)
		update_list ();
	else
		create_widget ();
	
	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", msgintag->GetAttr("session"));
	WokXMLTag &iq_tag = msgtag.AddTag("iq");
	WokXMLTag *vcard;
	iq_tag.AddAttr("to", jid);
	iq_tag.AddAttr("type", "get");
	vcard = &iq_tag.AddTag("vCard");
	vcard->AddAttr("xmlns", "vcard-temp");
	vcard->AddAttr("prodid", "-//HandGen//NONSGML vGen v1.0//EN");
	vcard->AddAttr("version", "2.0");
	wls->SendSignal("Jabber XML IQ Send", &msgtag);
	std::string id_vcard;
	id_vcard = std::string("Jabber XML IQ ID ") + iq_tag.GetAttr("id");
	
	EXP_SIGHOOK(id_vcard.c_str(), &GUIPresentReqWidget::vcard, 1000);
	
	return true;
}

int
GUIPresentReqWidget::vcard(WokXMLTag *tag)
{
	std::string from;
	
	from = tag->GetFirstTag("iq").GetAttr("from");
	WokXMLTag &vcard = tag->GetFirstTag("iq").GetFirstTag("vCard");
	WokXMLTag *tmptag;
	
	if( (tmptag=&vcard.GetFirstTag("NICKNAME")))
	{
		items[from]->AddAttr("nick", tmptag->GetBody());
		update_list();
	}
	
}

void
GUIPresentReqWidget::create_widget ()
{
	GtkWidget *vbox1;
	GtkWidget *label3;
	GtkWidget *scrolledwindow2;
	GtkWidget *hbuttonbox1;
	GtkWidget *close_button;
	GtkWidget *delete_button;
	GtkWidget *add_button;
	GtkWidget *addall_button;
	
	window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width (GTK_CONTAINER (window1), 5);
	gtk_window_set_title (GTK_WINDOW (window1), "Subscription Request");
	gtk_window_set_default_size (GTK_WINDOW (window1), 300, 200);

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (window1), vbox1);

	label3 = gtk_label_new ("Subscription Requesters:");
	gtk_widget_show (label3);
	gtk_box_pack_start (GTK_BOX (vbox1), label3, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);

	scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow2);
	gtk_box_pack_start (GTK_BOX (vbox1), scrolledwindow2, TRUE, TRUE, 5);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW
					     (scrolledwindow2),
					     GTK_SHADOW_IN);

	jid_treeview = gtk_tree_view_new ();
	gtk_widget_show (jid_treeview);
	gtk_container_add (GTK_CONTAINER (scrolledwindow2), jid_treeview);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (jid_treeview),
					   TRUE);

	hbuttonbox1 = gtk_hbutton_box_new ();
	gtk_widget_show (hbuttonbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox1, FALSE, FALSE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox1),
				   GTK_BUTTONBOX_SPREAD);

	close_button = gtk_button_new_from_stock ("gtk-close");
	gtk_widget_show (close_button);
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), close_button);
	GTK_WIDGET_SET_FLAGS (close_button, GTK_CAN_DEFAULT);

	delete_button = gtk_button_new_from_stock ("gtk-delete");
	gtk_widget_show (delete_button);
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), delete_button);
	GTK_WIDGET_SET_FLAGS (delete_button, GTK_CAN_DEFAULT);

	add_button = gtk_button_new_from_stock ("gtk-add");
	gtk_widget_show (add_button);
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), add_button);
	GTK_WIDGET_SET_FLAGS (add_button, GTK_CAN_DEFAULT);
	
	addall_button = gtk_button_new_with_mnemonic  ("Add A_ll");
	gtk_widget_show(addall_button);
	gtk_container_add (GTK_CONTAINER(hbuttonbox1), addall_button);
	GTK_WIDGET_SET_FLAGS(addall_button, GTK_CAN_DEFAULT);
	
	g_signal_connect (G_OBJECT (close_button), "clicked",
		G_CALLBACK (GUIPresentReqWidget::Close_Button), this);
	g_signal_connect (G_OBJECT (add_button), "clicked",
		G_CALLBACK (GUIPresentReqWidget::Add_Button), this);
	g_signal_connect (G_OBJECT (addall_button), "clicked",
		G_CALLBACK (GUIPresentReqWidget::AddAll_Button), this);
	g_signal_connect (G_OBJECT (window1), "destroy",
		G_CALLBACK (GUIPresentReqWidget::Destroy), this);
	
	// Initiating the chart..
	GtkCellRenderer *renderer;
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (jid_treeview), -1,
							     "Nick",
							     renderer, "text",
							     0, NULL);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (jid_treeview), -1,
							     "JID",
							     renderer, "text",
							     1, NULL);
	g_object_set (G_OBJECT (renderer), "xalign", 0.0, NULL);
	
	model = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model (GTK_TREE_VIEW
		(jid_treeview), GTK_TREE_MODEL (model));
		
	update_list();
	
	widget_visible = true;
	gtk_widget_show_all(window1);
}

void
GUIPresentReqWidget::Close_Button(GtkWidget * widget, GUIPresentReqWidget *req_widget)
{
	gtk_widget_destroy(GTK_WIDGET(req_widget->window1));
}

void
GUIPresentReqWidget::AddJID(std::string jid, std::string nick, std::string session)
{
	std::string xml_data;
		
	//Should...
	//Checking if user is already in roster then dont put
	//him/her there again ( loses the nick and group info)
	{
		WokXMLTag msgtag(NULL, "message");
		msgtag.AddAttr("session", session);
		WokXMLTag &iqtag = msgtag.AddTag("iq");
		iqtag.AddAttr("type", "set");
		WokXMLTag &query = iqtag.AddTag("query");
		query.AddAttr("xmlns", "jabber:iq:roster");
		WokXMLTag &item = query.AddTag("item");
		item.AddAttr("jid", jid);
		item.AddAttr("name", nick);
		
		wls->SendSignal("Jabber XML Send", &msgtag);
	}
	
	//Sending accept answere on request.	
	WokXMLTag msgtag2(NULL, "message");
	msgtag2.AddAttr("session", session);
	WokXMLTag &ptag = msgtag2.AddTag("presence");
	ptag.AddAttr("to", jid);
	ptag.AddAttr("type", "subscribed");
	
	wls->SendSignal("Jabber XML Send", &msgtag2);
	
	// Removing jid from list
	delete items[jid];
	items.erase(jid);
}

void
GUIPresentReqWidget::AddAll_Button(GtkWidget * widget, GUIPresentReqWidget *req_widget)
{
	GtkTreeIter iter;
	
	if(!gtk_tree_model_get_iter_first   (GTK_TREE_MODEL(req_widget->model), &iter))
		return;
	
	gchar *jid;
	gchar *nick;
	gchar *session;
	
	gtk_tree_model_get(GTK_TREE_MODEL(req_widget->model), &iter, 1, &jid, 0, &nick, 2, &session, -1);
	req_widget->AddJID(jid, nick, session);
	
	while(gtk_tree_model_iter_next(GTK_TREE_MODEL(req_widget->model), &iter))
	{
		gtk_tree_model_get(GTK_TREE_MODEL(req_widget->model), &iter, 1, &jid, 0, &nick, 2, &session, -1);
		req_widget->AddJID(jid, nick, session);
	}
	gtk_widget_destroy(GTK_WIDGET(req_widget->window1));
}

void
GUIPresentReqWidget::Add_Button(GtkWidget * widget, GUIPresentReqWidget *req_widget)
{
	GtkTreeIter       iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(req_widget->jid_treeview));
	
	if(gtk_tree_selection_get_selected(selection,  NULL, &iter));
	{
		gchar *jid;
		gchar *nick;
		gchar *session;
		gtk_tree_model_get(GTK_TREE_MODEL(req_widget->model), &iter, 1, &jid, 0, &nick, 2, &session, -1);
		req_widget->AddJID(jid, nick,session);
		
		// If the list is empty remove the window else update the list
		if( req_widget->items.empty() )
			gtk_widget_destroy(GTK_WIDGET(req_widget->window1));
		else
			req_widget->update_list ();
	}
}

void
GUIPresentReqWidget::Destroy(GtkWidget * widget, GUIPresentReqWidget *req_widget)
{
	req_widget->widget_visible = false;
}

void
GUIPresentReqWidget::update_list()
{
	gtk_list_store_clear(model);
	
	std::map <std::string, WokXMLTag *>::iterator iter;

	for(iter = items.begin() ; iter != items.end() ; iter++)
	{
		GtkTreeIter RosterIter;			 
		gtk_list_store_append (model, &RosterIter);
		gtk_list_store_set (model, &RosterIter,
				    0, iter->second->GetAttr("nick").c_str(),
				    1, iter->first.c_str(),
						2, iter->second->GetAttr("session").c_str(),
				    -1);
	}
}
