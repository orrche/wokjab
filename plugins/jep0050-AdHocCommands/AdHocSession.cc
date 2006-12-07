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
// Class: AdHocSession
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Mon Oct 16 15:18:51 2006
//

#include "AdHocSession.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

AdHocSession::AdHocSession(WLSignal *wls, WokXMLTag *tag) : WLSignalInstance(wls),
sessionid(tag->GetFirstTag("iq").GetFirstTag("command").GetAttr("sessionid")),
session(tag->GetAttr("session")),
node(tag->GetFirstTag("iq").GetFirstTag("command").GetAttr("node")),
jid(tag->GetFirstTag("iq").GetAttr("from"))
{
	xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/AdHocSession.glade", NULL, NULL);
	g_signal_connect (G_OBJECT (glade_xml_get_widget(xml, "cancel_button")), "clicked",
					G_CALLBACK (AdHocSession::CancelButton), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget(xml, "close_button")), "clicked",
					G_CALLBACK (AdHocSession::CloseButton), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget(xml, "window")), "destroy",
					G_CALLBACK (AdHocSession::Destroy), this);
	ExecResponse(tag);
}


AdHocSession::~AdHocSession()
{
	g_object_unref(xml);
}

void
AdHocSession::Destroy(GtkWidget *widget, AdHocSession *c)
{
	delete c;
}

void
AdHocSession::CloseButton(GtkButton *button, AdHocSession *c)
{
	gtk_widget_destroy(glade_xml_get_widget(c->xml, "window"));
}

void
AdHocSession::CancelButton(GtkButton *button, AdHocSession *c)
{
	WokXMLTag msgtag(NULL,"message");
	msgtag.AddAttr("session", c->session);
	WokXMLTag &tag = msgtag.AddTag("iq");
	tag.AddAttr("to", c->jid );
	tag.AddAttr("type", "set");
	WokXMLTag *query;
	query = &tag.AddTag("command");
	query->AddAttr("xmlns", "http://jabber.org/protocol/commands");
	query->AddAttr("node", c->node);
	query->AddAttr("sessionid", c->sessionid);
	query->AddAttr("action", "cancel");

	c->wls->SendSignal("Jabber XML IQ Send", &msgtag);
	
	gtk_widget_destroy(glade_xml_get_widget(c->xml, "window"));
}

void
AdHocSession::Button(GtkButton *button, AdHocSession *c)
{
		WokXMLTag xdata(NULL, "empty");
	c->wls->SendSignal("Jabber jabber:x:data Get " + c->xdataid, xdata);
	c->XDataResponse(&xdata, c->buttons[GTK_WIDGET(button)]);
	
	/*
	if ( c->buttons[GTK_WIDGET(button)] == "complete" )
		gtk_widget_destroy(glade_xml_get_widget(c->xml, "window"));	
	*/
}

int
AdHocSession::ExecResponse(WokXMLTag *tag)
{
	WokXMLTag *tag_iq;
	WokXMLTag *tag_x;
	tag_iq = &tag->GetFirstTag("iq");
		
	tag_x = &tag_iq->GetFirstTag("command").GetFirstTag("x");
	
	WokXMLTag msgtag( NULL, "message");
	msgtag.AddAttr("session", tag->GetAttr("session"));
	msgtag.AddTag(tag_x);
	
	msgtag.AddTag("plug");
	wls->SendSignal("Jabber jabber:x:data Init", &msgtag);
	xdataid = msgtag.GetAttr("id");
	
	GtkWidget *socket;
	socket = gtk_socket_new();
		
	
	
	std::list <WokXMLTag *>::iterator iter_note;
	std::list <WokXMLTag *> &list_note = tag_iq->GetFirstTag("command").GetTagList("note");

	for ( iter_note = list_note.begin() ; iter_note != list_note.end() ; iter_note++ )
	{
		std::string note_msg;
		if ( (*iter_note)->GetAttr("type") == "" )
			note_msg = "Info: ";
		else
			note_msg = (*iter_note)->GetAttr("type") + ": ";
		
		note_msg += (*iter_note)->GetBody();
			
		GtkWidget *note_label = gtk_label_new(note_msg.c_str());
		gtk_box_pack_start(GTK_BOX(glade_xml_get_widget(xml, "mainvbox")), note_label, FALSE, FALSE, 0);
	}
	
	GtkWidget *scroll_win = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy  (GTK_SCROLLED_WINDOW(scroll_win), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(scroll_win), socket);
	gtk_box_pack_start(GTK_BOX(glade_xml_get_widget(xml, "mainbox")), scroll_win, TRUE, TRUE, 0);

	gtk_socket_add_id(GTK_SOCKET(socket), atoi(msgtag.GetFirstTag("plug").GetAttr("id").c_str()));
	status = tag_iq->GetFirstTag("command").GetAttr("status");
	std::list<WokXMLTag *>::iterator list_iter;
	std::list<WokXMLTag *> &list = tag_iq->GetFirstTag("command").GetFirstTag("actions").GetTags();


	
	while ( !buttons.empty() )
	{
		gtk_widget_destroy(buttons.begin()->first);
		buttons.erase(buttons.begin());
	}
	for( list_iter = list.begin() ; list_iter != list.end() ; list_iter++)
	{
		std::string btnname = (*list_iter)->GetName();
		std::string stock;
		
		if( btnname == "next")
			stock = "gtk-go-forward";
		else if (btnname == "prev")
			stock = "gtk-go-back";
		else if (btnname == "complete")
			stock = "gtk-ok";
		else
			stock = btnname;
				
		GtkWidget *btn = gtk_button_new_from_stock(stock.c_str());
		buttons[btn] = btnname;
		gtk_box_pack_start(GTK_BOX(glade_xml_get_widget(xml, "buttonbox")), btn, FALSE, FALSE, 0);
		
		g_signal_connect (G_OBJECT (btn), "clicked",
					G_CALLBACK (AdHocSession::Button), this);
	}
	
	gtk_widget_show_all(glade_xml_get_widget(xml, "window"));
	if ( status == "completed")
	{
		gtk_widget_show(glade_xml_get_widget(xml, "close_button"));
		gtk_widget_hide(glade_xml_get_widget(xml, "cancel_button"));
	}
	else
	{
		gtk_widget_show(glade_xml_get_widget(xml, "cancel_button"));
		gtk_widget_hide(glade_xml_get_widget(xml, "close_button"));
	}
	
	EXP_SIGHOOK(msgtag.GetAttr("signal"), &AdHocSession::XDataResponse, 1000);
	
	return 1;
}

int
AdHocSession::XDataResponse(WokXMLTag *xdata, std::string button)
{
	WokXMLTag msgtag(NULL,"message");
	msgtag.AddAttr("session", session);
	WokXMLTag &tag = msgtag.AddTag("iq");
	tag.AddAttr("to", jid );
	tag.AddAttr("type", "set");
	WokXMLTag *query;
	query = &tag.AddTag("command");
	query->AddAttr("xmlns", "http://jabber.org/protocol/commands");
	query->AddAttr("node", node);
	query->AddAttr("sessionid", sessionid);
	query->AddAttr("action", button);
	query->AddTag(&xdata->GetFirstTag("x"));
	
	wls->SendSignal("Jabber XML IQ Send", &msgtag);
	EXP_SIGHOOK("Jabber XML IQ ID " + tag.GetAttr("id"), &AdHocSession::ExecResponse, 1000);
	
	return 1;
}
