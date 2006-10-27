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

#include "GroupChatJoinWindow.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

GroupChatJoinWindow::GroupChatJoinWindow (WLSignal * wls) : WLSignalInstance ( wls )
{
	GtkCellRenderer *renderer;
	
	xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/groupchatjoin.glade", NULL, NULL);
	GtkWidget *cbutton = glade_xml_get_widget (xml, "cancelbutton");
	GtkWidget *jbutton = glade_xml_get_widget (xml, "joinbutton");
	window = glade_xml_get_widget (xml, "window");
	sessionchooser = glade_xml_get_widget(xml, "sessionchooser");
	quickchooser = glade_xml_get_widget(xml, "quickchooser");
	nickentry = glade_xml_get_widget(xml, "nickentry");
	roomentry = glade_xml_get_widget(xml, "roomentry");
	serverentry = glade_xml_get_widget(xml, "serverentry");
	
	
	// Quick select
	quickmenu = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	gtk_combo_box_set_model(GTK_COMBO_BOX(quickchooser), GTK_TREE_MODEL(quickmenu));
	
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (quickchooser), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (quickchooser), renderer,
					"text", 0,
					NULL);
	
	// Session selector 
	sessionmenu = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
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

	GtkTreeIter treeiter;
	if( gtk_tree_model_get_iter_first(GTK_TREE_MODEL(sessionmenu), &treeiter) )
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX(sessionchooser), &treeiter);
	
	g_signal_connect (G_OBJECT (window), "destroy",
			  G_CALLBACK (GroupChatJoinWindow::Destroy), this);
	g_signal_connect (G_OBJECT (jbutton), "clicked",
			  G_CALLBACK (GroupChatJoinWindow::Join_Button),
			  this);
	g_signal_connect (G_OBJECT (cbutton), "clicked",
			  G_CALLBACK (GroupChatJoinWindow::Cancel_Button),
			  this);	
	g_signal_connect (G_OBJECT (quickchooser) , "changed", 
				G_CALLBACK (GroupChatJoinWindow::QuickChange),
				this);
				
	config = NULL;
			  
	EXP_SIGHOOK("Config XML Change /groupchat/joinwindow", &GroupChatJoinWindow::Config, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/groupchat/joinwindow");
	wls->SendSignal("Config XML Trigger", &conftag);
	EXP_SIGUNHOOK("Config XML Change /groupchat/joinwindow", &GroupChatJoinWindow::Config, 500);
}


GroupChatJoinWindow::~GroupChatJoinWindow ()
{
	if( config )
		delete config;
}

void
GroupChatJoinWindow::QuickChange(GtkComboBox *widget, GroupChatJoinWindow *c)
{
	std::cout << "Trying to change the world" << std::endl;
	GtkTreeIter treeiter;
	if( !gtk_combo_box_get_active_iter(GTK_COMBO_BOX(c->quickchooser), &treeiter) )
	{
		return;
	}
		
	gchar *server, *room, *nick;	
	gtk_tree_model_get (GTK_TREE_MODEL(c->quickmenu), &treeiter, 1, &server, 2, &room, 3, &nick, -1);
	gtk_entry_set_text(GTK_ENTRY(c->serverentry), server);
	gtk_entry_set_text(GTK_ENTRY(c->roomentry), room);
	gtk_entry_set_text(GTK_ENTRY(c->nickentry), nick);
	
}

void
GroupChatJoinWindow::Destroy (GtkWidget * widget, GroupChatJoinWindow *c)
{
	delete c;
}

void
GroupChatJoinWindow::Join_Button (GtkWidget * widget, GroupChatJoinWindow *c)
{
	GtkTreeIter treeiter;
	if( !gtk_combo_box_get_active_iter(GTK_COMBO_BOX(c->sessionchooser), &treeiter) )
	{
		return;
	}
		
	gchar *session;
	
	gtk_tree_model_get (GTK_TREE_MODEL(c->sessionmenu), &treeiter, 1, &session, -1);
	std::string server = gtk_entry_get_text(GTK_ENTRY(c->serverentry));
	std::string room = gtk_entry_get_text(GTK_ENTRY(c->roomentry));
	std::string nick = gtk_entry_get_text(GTK_ENTRY(c->nickentry));
	
	
	WokXMLTag jointag(NULL, "groupchat");
	jointag.AddAttr("nick", nick);
	jointag.AddAttr("room", room);
	jointag.AddAttr("server", server);
	jointag.AddAttr("session", session);
	
	c->wls->SendSignal("Jabber GroupChat Join", &jointag);
	
	
	c->config->AddAttr("path", "/groupchat/joinwindow");
	c->config->GetFirstTag("server").AddAttr("data", server);
	c->config->GetFirstTag("room").AddAttr("data", room);
	c->config->GetFirstTag("nick").AddAttr("data", nick);

	WokXMLTag ljtag(NULL, "item");
	ljtag.AddAttr("name", room + "@" + server + "/" + nick);
	ljtag.AddAttr("server", server);
	ljtag.AddAttr("room", room);
	ljtag.AddAttr("nick", nick);
	
	std::list <WokXMLTag *>::iterator iter;
	std::list <WokXMLTag *> removelist;
	
	while( c->config->GetFirstTag("lastjoins").GetTagList("item").size() > 10 )
		c->config->GetFirstTag("lastjoins").RemoveTag(*c->config->GetFirstTag("lastjoins").GetTagList("item").begin());

	for( iter = c->config->GetFirstTag("lastjoins").GetTagList("item").begin() ; iter != c->config->GetFirstTag("lastjoins").GetTagList("item").end() ; iter++)
		if ( ljtag == **iter )
			removelist.push_back(*iter);			
	
	for( iter = removelist.begin() ; iter != removelist.end() ; iter++)
		c->config->GetFirstTag("lastjoins").RemoveTag(*iter);
	
	
	c->config->GetFirstTag("lastjoins").AddTag(&ljtag);
	
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/groupchat/joinwindow");
	conftag.AddTag(c->config);
	
	c->wls->SendSignal("Config XML Store", &conftag);
	c->wls->SendSignal("Config XML Save", &conftag);
	
	gtk_widget_destroy (c->window);
}

void
GroupChatJoinWindow::Cancel_Button (GtkWidget * widget, GroupChatJoinWindow *c)
{
	gtk_widget_destroy (c->window);
}

int
GroupChatJoinWindow::Config(WokXMLTag *tag)
{
	if(config)
		delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));
	
	gtk_entry_set_text(GTK_ENTRY(serverentry), config->GetFirstTag("server").GetAttr("data").c_str());
	gtk_entry_set_text(GTK_ENTRY(nickentry), config->GetFirstTag("nick").GetAttr("data").c_str());
	gtk_entry_set_text(GTK_ENTRY(roomentry), config->GetFirstTag("room").GetAttr("data").c_str());
	
	std::list <WokXMLTag *>::iterator iter;
	
	for( iter = config->GetFirstTag("lastjoins").GetTagList("item").begin() ; iter != config->GetFirstTag("lastjoins").GetTagList("item").end() ; iter++)
	{
		GtkTreeIter treeiter;
		gtk_list_store_append(GTK_LIST_STORE(quickmenu), &treeiter);
		gtk_list_store_set(GTK_LIST_STORE(quickmenu), &treeiter, 0 , (*iter)->GetAttr("name").c_str(), 
						1, (*iter)->GetAttr("server").c_str(), 
						2, (*iter)->GetAttr("room").c_str(), 
						3, (*iter)->GetAttr("nick").c_str(), -1);
			
	}

	
	return true;
}
