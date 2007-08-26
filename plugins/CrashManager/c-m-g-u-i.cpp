/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2007 <nedo80@gmail.com>
 * 
 * wokjab is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * wokjab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with wokjab.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#include "c-m-g-u-i.hpp"
#include <sstream>
#include <dirent.h>
#include <fstream>

CMGUI::CMGUI(WLSignal *wls, CrashManager *parant) : WLSignalInstance(wls), 
parant(parant)
{
	config = new WokXMLTag(NULL, "NULL");
	EXP_SIGHOOK("Config XML Change /CrashManager/window", &CMGUI::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/CrashManager/window");
	wls->SendSignal("Config XML Trigger", &conftag);
	
	xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/cmgui.glade", NULL, NULL);
		
	GtkWidget *sessionchooser;
	GtkCellRenderer *renderer;
	
	sessionmenu = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	
	sessionchooser = glade_xml_get_widget(xml, "session");
	gtk_combo_box_set_model (GTK_COMBO_BOX(sessionchooser), GTK_TREE_MODEL(sessionmenu) );
	
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
	
	
	if ( querytag.GetTagList("item").empty() )
	{
		woklib_error(wls, "You need to be connected");	
	}
	else
	{
		DIR             *dip;
		struct dirent   *dit;
		
		std::string filename = std::string(g_get_home_dir()) + "/.wokjab";

		if ((dip = opendir(filename.c_str())) == NULL)
		{
			perror("opendir");
			return;
		}

		std::string message;
		std::stringstream my_log_file;
		my_log_file << "sig." << getpid() << ".log";
		
		while ((dit = readdir(dip)) != NULL)
		{
			std::string file = dit->d_name;
			if ( my_log_file.str() == file )
				continue;
			if ( file.size() > 4 )
			{
				if ( file.substr(file.size() - 4 ) == ".log" && file.substr(0,4) == "sig.")
				{
					std::ifstream f;
					f.open((filename + "/" + file).c_str());
					if ( f.is_open() )
					{
						while(! f.eof() )
						{
							std::string line;
							std::getline(f,line);
							message+= line+ "\n";
						}
					}
					
					message += "\n\n\n";
					unlink((filename + "/" + file).c_str());
				}
			}
		}
		
		gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget(xml, "debug_data"))), message.c_str(), -1);

		if (closedir(dip) == -1)
		{
			perror("closedir");
			return;
		}
		
		g_signal_connect (G_OBJECT (glade_xml_get_widget(xml, "send")), "clicked",
			G_CALLBACK (CMGUI::SendButton), this);	
		g_signal_connect (G_OBJECT (glade_xml_get_widget(xml, "chat")), "clicked",
			G_CALLBACK (CMGUI::ChatButton), this);	
	}
	g_signal_connect (G_OBJECT (glade_xml_get_widget(xml, "window")), "delete-event", 
		G_CALLBACK (CMGUI::Delete), this);
	gtk_widget_show_all(glade_xml_get_widget(xml, "window"));
}

CMGUI::~CMGUI()
{
	gtk_widget_destroy(glade_xml_get_widget(xml, "window"));
	
	
}

gboolean
CMGUI::Delete( GtkWidget *widget, GdkEvent *event, CMGUI *c)
{
	c->parant->Remove(c);
	return TRUE;
}

int
CMGUI::ReadConfig(WokXMLTag *tag)
{
	delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));
	return 1;
}

void
CMGUI::SendButton(GtkButton *widget, CMGUI *c)
{
	WokXMLTag mesgtag("message");
	
	GtkTreeIter treeiter;
	if( gtk_combo_box_get_active_iter(GTK_COMBO_BOX(glade_xml_get_widget(c->xml, "session")), &treeiter) != TRUE )
	{
		return;
	}
		
	gchar *session;
	gtk_tree_model_get (GTK_TREE_MODEL(c->sessionmenu), &treeiter, 1, &session, -1);
	
	WokXMLTag tag(NULL, "dialog");
	tag.AddAttr("jid", "nedo@jabber.se");
	tag.AddAttr("session", session);
	c->wls->SendSignal("Jabber GUI MessageDialog Open", &tag);
	
	mesgtag.AddAttr("session", session);
	g_free (session);
	
	GtkTextIter start_iter, end_iter;
	gtk_text_buffer_get_start_iter(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget(c->xml, "debug_data"))), &start_iter);
	gtk_text_buffer_get_end_iter(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget(c->xml, "debug_data"))), &end_iter);

	std::string debug_msg = gtk_text_buffer_get_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget(c->xml, "debug_data"))),&start_iter, &end_iter, false);
	
	gtk_text_buffer_get_start_iter(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget(c->xml, "comment"))), &start_iter);
	gtk_text_buffer_get_end_iter(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget(c->xml, "comment"))), &end_iter);

	std::string comment_msg = gtk_text_buffer_get_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget(c->xml, "comment"))),&start_iter, &end_iter, false);
	
	
	WokXMLTag &message = mesgtag.AddTag("message");
	message.AddAttr("to","nedo@jabber.se");
	WokXMLTag &body = message.AddTag("body");
	body.AddText("Wokjab Crash Debug message ..\n");
	body.AddText(debug_msg.c_str());
	body.AddText("Comment:\n");
	body.AddText(comment_msg.c_str());
	body.AddText("\n");
	if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget(c->xml, "response_allowed"))) == TRUE )
		body.AddText("Responce allowed");
	else
		body.AddText("Responce denied");
		
	c->wls->SendSignal("Jabber XML Message Send", mesgtag);
	
	c->parant->Remove(c);
}

void
CMGUI::ChatButton(GtkButton *widget, CMGUI *c)
{
	GtkTreeIter treeiter;
	if( gtk_combo_box_get_active_iter(GTK_COMBO_BOX(glade_xml_get_widget(c->xml, "session")), &treeiter) != TRUE )
	{
		return;
	}
		
	gchar *session;
	gtk_tree_model_get (GTK_TREE_MODEL(c->sessionmenu), &treeiter, 1, &session, -1);
	
	WokXMLTag tag(NULL, "dialog");
	tag.AddAttr("jid", "nedo@jabber.se");
	tag.AddAttr("session", session);
	c->wls->SendSignal("Jabber GUI MessageDialog Open", &tag);
	
	g_free (session);
}
