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


#include "include/GUIConnectWindow.h"

#include <Woklib/WokLib.h>
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

GUIConnectWindow::GUIConnectWindow (int *feedback, WLSignal * wls) : WLSignalInstance(wls)
{
    xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/connect.window.glade", NULL, NULL);
	config = NULL;

	prio_entry = glade_xml_get_widget(xml,"prio_entry");
	username_entry = glade_xml_get_widget(xml,"username_entry");
	password_entry = glade_xml_get_widget(xml,"password_entry");
	server_entry = glade_xml_get_widget(xml,"server_entry");
	resource_entry = glade_xml_get_widget(xml,"resource_entry");
	port_entry = glade_xml_get_widget(xml,"port_entry");
	conn_win = glade_xml_get_widget(xml, "window");
	accounts = glade_xml_get_widget(xml, "accounts");

	accountlist = gtk_list_store_new(NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT);
	gtk_tree_view_set_model (GTK_TREE_VIEW(accounts), GTK_TREE_MODEL(accountlist));

	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("JID", renderer, "text", USER_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts), column);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Resource", renderer, "text", RESOURCE_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts), column);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Server", renderer, "text", SERVER_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts), column);
/*
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Password", renderer, "text", PASSWORD_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts), column);
*/
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Port", renderer, "text", PORT_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts), column);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Priority", renderer, "text", PRIO_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts), column);

	EXP_SIGHOOK("Config XML Change /connect/window", &GUIConnectWindow::Config, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/connect/window");
	wls->SendSignal("Config XML Trigger", &conftag);

	gtk_widget_show_all (conn_win);
	this->feedback = feedback;
	g_signal_connect (G_OBJECT (conn_win), "destroy",
			  G_CALLBACK (GUIConnectWindow::Destroy), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget(xml, "login_button")), "clicked",
				G_CALLBACK (GUIConnectWindow::Connect_Button), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget(xml, "cancel_button")), "clicked",
				G_CALLBACK (GUIConnectWindow::Connect_Button), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget(xml, "add_button")), "clicked",
				G_CALLBACK (GUIConnectWindow::Add_Button), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget(xml, "remove_button")), "clicked",
				G_CALLBACK (GUIConnectWindow::Remove_Button), this);
	g_signal_connect (G_OBJECT (accounts), "row-activated",
				G_CALLBACK (GUIConnectWindow::RowActivated), this);
}


GUIConnectWindow::~GUIConnectWindow ()
{
	*feedback = false;
	g_object_unref(xml);
}

void
GUIConnectWindow::Destroy (GtkWidget * widget, gpointer user_data)
{
	GUIConnectWindow *data;
	data = static_cast < GUIConnectWindow * >(user_data);
	delete data;
}

void
GUIConnectWindow::RowActivated(GtkTreeView *treeview, GtkTreePath *arg1, GtkTreeViewColumn *arg2, GUIConnectWindow *c)
{
	gchar *jid;
	gchar *server;
	gchar *resource;
	gchar *password;
	gint port;
	gint prio;
	GtkTreeIter iter;
	gtk_tree_model_get_iter(GTK_TREE_MODEL(c->accountlist), &iter, arg1);
	gtk_tree_model_get(GTK_TREE_MODEL(c->accountlist), &iter,
										USER_COLUMN, &jid,
										SERVER_COLUMN, &server,
										PASSWORD_COLUMN, &password,
										RESOURCE_COLUMN, &resource,
										PRIO_COLUMN, &prio,
										PORT_COLUMN, &port,
										-1);

	gtk_entry_set_text(GTK_ENTRY(c->username_entry), jid);
	gtk_entry_set_text(GTK_ENTRY(c->server_entry), server);
	gtk_entry_set_text(GTK_ENTRY(c->password_entry), password);
	gtk_entry_set_text(GTK_ENTRY(c->resource_entry), resource);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(c->prio_entry), prio);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(c->port_entry), port);

}

void
GUIConnectWindow::Add_Button (GtkWidget * widget, GUIConnectWindow *c)
{
	char buf[20];
	std::string nick,server,password,resource, port, prio;

	WokXMLTag &acc_tag = c->config->AddTag("account");

	server = gtk_entry_get_text (GTK_ENTRY (c->server_entry));
	nick = gtk_entry_get_text (GTK_ENTRY (c->username_entry));
	password = gtk_entry_get_text (GTK_ENTRY (c->password_entry));
	resource = gtk_entry_get_text (GTK_ENTRY (c->resource_entry));
	sprintf(buf, "%d", gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(c->port_entry)));
	port = buf;
	sprintf(buf, "%d", gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(c->prio_entry)));
	prio = buf;


	acc_tag.GetFirstTag("nick").AddAttr("data", nick);
	acc_tag.GetFirstTag("server").AddAttr("data", server);
	acc_tag.GetFirstTag("password").AddAttr("data", password);
	acc_tag.GetFirstTag("resource").AddAttr("data", resource);
	acc_tag.GetFirstTag("port").AddAttr("data", port);
	acc_tag.GetFirstTag("prio").AddAttr("data", prio);

	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/connect/window");
	conftag.AddTag(c->config);
	c->wls->SendSignal("Config XML Store", &conftag);

}

void
GUIConnectWindow::Remove_Button (GtkWidget * widget, GUIConnectWindow *c)
{
	GtkTreeIter       iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(c->accounts));

	if(gtk_tree_selection_get_selected(selection,  NULL, &iter));
	{
		gtk_list_store_remove(GTK_LIST_STORE(c->accountlist), &iter);

		c->config->RemoveChildrenTags();

		if ( gtk_tree_model_get_iter_first(GTK_TREE_MODEL(c->accountlist), &iter) )
		{
			gchar *jid;
			gchar *server;
			gchar *resource;
			gchar *password;
			gint port;
			gint prio;

			char buf[20];

			for(;;)
			{
				gtk_tree_model_get(GTK_TREE_MODEL(c->accountlist), &iter,
													USER_COLUMN, &jid,
													SERVER_COLUMN, &server,
													RESOURCE_COLUMN, &resource,
													PASSWORD_COLUMN, &password,
													PRIO_COLUMN, &prio,
													PORT_COLUMN, &port, -1);

				std::cout << "p" <<  port << "," << prio << std::endl;

				WokXMLTag &acc_tag = c->config->AddTag("account");
				acc_tag.GetFirstTag("nick").AddAttr("data", jid);
				acc_tag.GetFirstTag("server").AddAttr("data", server);
				acc_tag.GetFirstTag("password").AddAttr("data", password);
				acc_tag.GetFirstTag("resource").AddAttr("data", resource);
				sprintf(buf, "%d", prio);
				acc_tag.GetFirstTag("prio").AddAttr("data", buf);
				sprintf(buf, "%d", port);
				acc_tag.GetFirstTag("port").AddAttr("data", buf);

				if ( gtk_tree_model_iter_next(GTK_TREE_MODEL(c->accountlist), &iter) == FALSE)
					break;;
			}
		}
		WokXMLTag conftag(NULL, "config");
		conftag.AddAttr("path", "/connect/window");
		conftag.AddTag(c->config);
		c->wls->SendSignal("Config XML Store", &conftag);
	}

}

void
GUIConnectWindow::Connect_Button (GtkWidget * widget, gpointer sig_data)
{
	GUIConnectWindow *data;
	data = static_cast < GUIConnectWindow * >(sig_data);
	std::string nick,server,password,resource, port, prio;

	char buf[20];
	server = gtk_entry_get_text (GTK_ENTRY (data->server_entry));
	nick = gtk_entry_get_text (GTK_ENTRY (data->username_entry));
	password = gtk_entry_get_text (GTK_ENTRY (data->password_entry));
	resource = gtk_entry_get_text (GTK_ENTRY (data->resource_entry));
	sprintf(buf, "%d", gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(data->port_entry)));
	port = buf;
	sprintf(buf, "%d", gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(data->prio_entry)));
	prio = buf;

	WokXMLTag consig(NULL, "connect");
	consig.AddAttr("server", server);
	consig.AddAttr("username", nick);
	consig.AddAttr("password", password);
	consig.AddAttr("resource", resource);
	consig.AddAttr("port", port);
	consig.AddAttr("prio", prio);
	consig.AddAttr("type", "1");
	data->wls->SendSignal ("Jabber Connection Connect", &consig);

	gtk_widget_destroy (data->conn_win);
}

void
GUIConnectWindow::Cancel_Button (GtkWidget * widget, gpointer sig_data)
{
	GUIConnectWindow *data;
	data = static_cast < GUIConnectWindow * >(sig_data);

	gtk_widget_destroy (data->conn_win);
}

int
GUIConnectWindow::Config (WokXMLTag *tag)
{
	if ( config )
		delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));

	if( config->GetTagList("account").size() )
	{
		gtk_entry_set_text(GTK_ENTRY(username_entry), config->GetFirstTag("account").GetFirstTag("nick").GetAttr("data").c_str());
		gtk_entry_set_text(GTK_ENTRY(server_entry), config->GetFirstTag("account").GetFirstTag("server").GetAttr("data").c_str());
		gtk_entry_set_text(GTK_ENTRY(password_entry), config->GetFirstTag("account").GetFirstTag("password").GetAttr("data").c_str());
		gtk_entry_set_text(GTK_ENTRY(resource_entry), config->GetFirstTag("account").GetFirstTag("resource").GetAttr("data").c_str());

		gtk_spin_button_set_value(GTK_SPIN_BUTTON(prio_entry), atoi(config->GetFirstTag("account").GetFirstTag("prio").GetAttr("data").c_str()));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(port_entry), atoi(config->GetFirstTag("account").GetFirstTag("port").GetAttr("data").c_str()));
	}

	std::list <WokXMLTag *>::iterator iter;

	gtk_list_store_clear(GTK_LIST_STORE(accountlist));
	for( iter = config->GetTagList("account").begin() ; iter != config->GetTagList("account").end() ; iter++)
	{
		GtkTreeIter treeiter;
		gtk_list_store_append(GTK_LIST_STORE(accountlist), &treeiter);

		gtk_list_store_set(GTK_LIST_STORE(accountlist), &treeiter,
						SERVER_COLUMN , (**iter).GetFirstTag("server").GetAttr("data").c_str(),
						USER_COLUMN , (**iter).GetFirstTag("nick").GetAttr("data").c_str(),
						PASSWORD_COLUMN , (**iter).GetFirstTag("password").GetAttr("data").c_str(),
						RESOURCE_COLUMN , (**iter).GetFirstTag("resource").GetAttr("data").c_str(),
						PRIO_COLUMN , atoi((**iter).GetFirstTag("prio").GetAttr("data").c_str()),
						PORT_COLUMN , atoi((**iter).GetFirstTag("port").GetAttr("data").c_str()),
						-1);
	}
}
