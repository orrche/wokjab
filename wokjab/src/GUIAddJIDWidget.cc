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


#include "../include/GUIAddJIDWidget.h"

GUIAddJIDWidget::GUIAddJIDWidget (WLSignal * wls):WLSignalInstance (wls)
{
	EXP_SIGHOOK("Get Main Menu", &GUIAddJIDWidget::AddMenu, 400);
	EXP_SIGHOOK("GUIRoster Add JID", &GUIAddJIDWidget::GUIAddJIDAction, 2);
	EXP_SIGHOOK("Jabber GUI GetJIDMenu NoRoster", &GUIAddJIDWidget::AddMenu, 2);
	
	widget_created = false;
	
}


GUIAddJIDWidget::~GUIAddJIDWidget ()
{
}

int
GUIAddJIDWidget::AddMenu (WokXMLTag * xml)
{
	WokXMLTag *item;
	item = & xml->AddTag("item");
	item->AddAttr("name", "Add Buddy");
	item->AddAttr("signal", "GUIRoster Add JID");

	return 1;
}

int
GUIAddJIDWidget::GUIAddJIDAction (WokXMLTag *tag)
{
	widget_created = true;

	if(tag)
	{
		std::string jid = tag->GetAttr("jid");
		if ( jid.find("/") != std::string::npos )
			jid = jid.substr(0, jid.find("/"));
		
		CreateWidget (jid);
	}
	else
		CreateWidget();
	return 1;
}

int
GUIAddJIDWidget::CreateWidget (std::string jid)
{
	GtkWidget *vbox1;
	GtkWidget *label4;
	GtkWidget *table1;
	GtkWidget *label1;
	GtkWidget *label2;
	GtkWidget *hbox1;
	GtkWidget *label3;
	GtkWidget *scrolledwindow2;
	GtkWidget *hbuttonbox1;
	GtkWidget *cencel_button;
	GtkWidget *ok_button;
	GtkCellRenderer *renderer;
		
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width (GTK_CONTAINER (window), 5);
	gtk_window_set_title (GTK_WINDOW (window), "Add Contact");

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (window), vbox1);

	// Session selector 
	sessionmenu = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	sessionchooser = gtk_combo_box_new_with_model(GTK_TREE_MODEL(sessionmenu));

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
	// Session selector end
	
	gtk_box_pack_start(GTK_BOX(vbox1), sessionchooser, FALSE, 0,0);

	
	
	label4 = gtk_label_new ("Add Contact");
	gtk_widget_show (label4);
	gtk_box_pack_start (GTK_BOX (vbox1), label4, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (label4), GTK_JUSTIFY_LEFT);

	table1 = gtk_table_new (2, 2, FALSE);
	gtk_widget_show (table1);
	gtk_box_pack_start (GTK_BOX (vbox1), table1, FALSE, FALSE, 5);
	gtk_table_set_row_spacings (GTK_TABLE (table1), 3);
	gtk_table_set_col_spacings (GTK_TABLE (table1), 3);

	label1 = gtk_label_new ("JID");
	gtk_widget_show (label1);
	gtk_table_attach (GTK_TABLE (table1), label1, 0, 1, 0, 1,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);

	label2 = gtk_label_new ("Nick");
	gtk_widget_show (label2);
	gtk_table_attach (GTK_TABLE (table1), label2, 0, 1, 1, 2,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);

	jid_entry = gtk_entry_new ();
	if(jid.size())
	{
		if(jid.find("/") == std::string::npos)
			gtk_entry_set_text(GTK_ENTRY(jid_entry), jid.c_str());
		else
			gtk_entry_set_text(GTK_ENTRY(jid_entry), jid.substr(0,jid.rfind("/")).c_str());
	}
			
	gtk_widget_show (jid_entry);
	gtk_table_attach (GTK_TABLE (table1), jid_entry, 1, 2, 0, 1,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);

	nick_entry = gtk_entry_new ();
	gtk_widget_show (nick_entry);
	gtk_table_attach (GTK_TABLE (table1), nick_entry, 1, 2, 1, 2,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 2);

	label3 = gtk_label_new ("Request Message: (ignored at the moment)");
	gtk_widget_show (label3);
	gtk_box_pack_start (GTK_BOX (hbox1), label3, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);

	scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow2);
	gtk_box_pack_start (GTK_BOX (vbox1), scrolledwindow2, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW
					     (scrolledwindow2),
					     GTK_SHADOW_IN);

	message_textview = gtk_text_view_new ();
	gtk_widget_show (message_textview);
	gtk_container_add (GTK_CONTAINER (scrolledwindow2), message_textview);
	gtk_text_buffer_set_text (gtk_text_view_get_buffer
				  (GTK_TEXT_VIEW (message_textview)),
				  "I would like to add you to my contact list.",
				  -1);

	hbuttonbox1 = gtk_hbutton_box_new ();
	gtk_widget_show (hbuttonbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox1, FALSE, TRUE, 5);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox1),
				   GTK_BUTTONBOX_END);
	gtk_box_set_spacing (GTK_BOX (hbuttonbox1), 10);

	cencel_button = gtk_button_new_from_stock ("gtk-cancel");
	gtk_widget_show (cencel_button);
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), cencel_button);
	GTK_WIDGET_SET_FLAGS (cencel_button, GTK_CAN_DEFAULT);

	ok_button = gtk_button_new_from_stock ("gtk-ok");
	gtk_widget_show (ok_button);
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), ok_button);
	GTK_WIDGET_SET_FLAGS (ok_button, GTK_CAN_DEFAULT);
	
	g_signal_connect (G_OBJECT (cencel_button), "clicked",
		G_CALLBACK (GUIAddJIDWidget::Cancel_Button), this);
	g_signal_connect (G_OBJECT (ok_button), "clicked",
		G_CALLBACK (GUIAddJIDWidget::OK_Button), this);
	g_signal_connect (G_OBJECT (window), "destroy",
		G_CALLBACK (GUIAddJIDWidget::Destroy), this);
		
	gtk_widget_show_all(window);	
	
	return true;
}

void
GUIAddJIDWidget::Cancel_Button(GtkWidget * widget, GUIAddJIDWidget *add_jid)
{
	gtk_widget_destroy(GTK_WIDGET(add_jid->window));
}

void
GUIAddJIDWidget::OK_Button(GtkWidget * widget, GUIAddJIDWidget *add_jid)
{
	
	GtkTreeIter treeiter;
	if( !gtk_combo_box_get_active_iter(GTK_COMBO_BOX(add_jid->sessionchooser), &treeiter) )
	{
		return;
	}
		
	gchar *session;
	
	gtk_tree_model_get (GTK_TREE_MODEL(add_jid->sessionmenu), &treeiter, 1, &session, -1);

	
	WokXMLTag msgtag(NULL, "message");
	WokXMLTag &iqtag = msgtag.AddTag("iq");
	
	iqtag.AddAttr("type", "set");
	WokXMLTag &query = iqtag.AddTag("query");
	query.AddAttr("xmlns","jabber:iq:roster");
	WokXMLTag &item = query.AddTag("item");
	item.AddAttr("name", gtk_entry_get_text(GTK_ENTRY(add_jid->nick_entry)));
	item.AddAttr("jid", gtk_entry_get_text(GTK_ENTRY(add_jid->jid_entry)));
	
	msgtag.AddAttr("session", session);
	add_jid->wls->SendSignal("Jabber XML Send", &msgtag);
	
	WokXMLTag msg2tag(NULL, "message");
	msg2tag.AddAttr("session", session);
	WokXMLTag &presence = msg2tag.AddTag("presence");
	presence.AddAttr("type", "subscribe");
	
	std::string jid = gtk_entry_get_text(GTK_ENTRY(add_jid->jid_entry));
	if ( jid.find("/") != std::string::npos )
			jid = jid.substr(0, jid.find("/"));
	presence.AddAttr("to", jid);
	add_jid->wls->SendSignal("Jabber XML Send", &msg2tag);
	
	gtk_widget_destroy(GTK_WIDGET(add_jid->window));
	
	g_free(session);
}

void
GUIAddJIDWidget::Destroy (GtkWidget * widget, GUIAddJIDWidget *add_jid)
{
	add_jid->widget_created = false;
}
