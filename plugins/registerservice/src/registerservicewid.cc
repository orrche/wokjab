/***************************************************************************
 *  Copyright (C) 2003-2008  Kent Gustavsson <nedo80@gmail.com>
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

#include "registerservicewid.h"

RegisterServiceWidget::RegisterServiceWidget(WLSignal *wls, std::string jid, std::string session):
WLSignalInstance ( wls ),
jid(jid),
session(session)
{
	GtkWidget *button;
	
	instructions = gtk_text_view_new();
	scrollinstr = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrollinstr), instructions);
	
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "WokJab Register Service");
	hbox = gtk_vbox_new(FALSE, FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), scrollinstr, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (window), hbox);

	gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(instructions)), "Wait for data to get loaded", 27);
	
	
	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", session);
	WokXMLTag &tag = msgtag.AddTag("iq");
	tag.AddAttr("to", jid );
	tag.AddAttr("type", "get");
	tag.AddTag("query").AddAttr("xmlns", "jabber:iq:register");
	
	wls->SendSignal("Jabber XML IQ Send", &msgtag);

	signalid = tag.GetAttr("id");
	fetchsignal = "Jabber XML IQ ID " + tag.GetAttr("id");
	EXP_SIGHOOK(fetchsignal.c_str(), &RegisterServiceWidget::RegisterData, 500);
	
	gtk_widget_show_all(window);
	
	button = gtk_button_new_with_mnemonic("_Send");
	
	g_signal_connect (G_OBJECT (button), "clicked",
		      G_CALLBACK (RegisterServiceWidget::ButtonPress), this);
	g_signal_connect (G_OBJECT (window), "destroy",
					G_CALLBACK (RegisterServiceWidget::Destroy), this);
					
	gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, FALSE,0);
	
}

RegisterServiceWidget::~RegisterServiceWidget()
{
	EXP_SIGUNHOOK(fetchsignal.c_str(), &RegisterServiceWidget::RegisterData, 500);
}

void
RegisterServiceWidget::Destroy (GtkWidget * widget, RegisterServiceWidget *c)
{
	delete c;
}

void 
RegisterServiceWidget::ButtonPress (GtkButton *button, RegisterServiceWidget *c)
{
	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", c->session);
	WokXMLTag &response = msgtag.AddTag("iq");
	WokXMLTag &query = response.AddTag("query");
	response.AddAttr("to", c->jid);
	response.AddAttr("type", "set");
	response.AddAttr("id", c->signalid);
	query.AddAttr("xmlns", "jabber:iq:register");
	
	if ( c->xdata_id.empty() )
	{
		std::map<std::string, GtkWidget*>::iterator iter;
	
		for(iter = c->entryfields.begin() ; iter != c->entryfields.end() ; iter++)
		{
			query.AddTag(iter->first).AddText(gtk_entry_get_text(GTK_ENTRY(iter->second)));
		}
	}
	else
	{
		WokXMLTag xdata(NULL, "empty");
		c->wls->SendSignal("Jabber jabber:x:data Get " + c->xdata_id, xdata);
		query.AddTag(&xdata.GetFirstTag("x"));
	}
		
	c->wls->SendSignal("Jabber XML Send", &msgtag);
}

void
RegisterServiceWidget::RegisterField(std::string field, std::string value)
{
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *entry;
	
	vbox = gtk_hbox_new(FALSE, FALSE);
	entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), value.c_str());
	
	label = gtk_label_new(field.c_str());
		
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
	
	entryfields[field] = entry;
}

int
RegisterServiceWidget::RegisterData(WokXMLTag *tag)
{	
	WokXMLTag *querytag;
	std::list <WokXMLTag *> *list;
	std::list <WokXMLTag *>::iterator iter;
	
	if ( tag->GetFirstTag("iq").GetAttr("type") == "error")
	{
		woklib_error(wls, "Something went wrong with the registration\nError code:" + tag->GetFirstTag("iq").GetFirstTag("error").GetAttr("code") + " " + tag->GetFirstTag("iq").GetFirstTag("error").GetAttr("type") );
		return 1;
	}
	
	querytag = &tag->GetFirstTag("iq").GetFirstTag("query");
	
	list = &querytag->GetTagList("x", "jabber:x:data");
	
	if( !list->empty() )
	{
		WokXMLTag xdata("xdata");
		xdata.AddTag(*list->begin());
		xdata.AddTag("plug");
		wls->SendSignal("Jabber jabber:x:data Init", xdata);
		xdata_id = xdata.GetAttr("id");
		
		GtkWidget *socket;
		socket = gtk_socket_new();
	
		gtk_box_pack_start(GTK_BOX(hbox), socket, TRUE, TRUE, 0);
		
		gtk_socket_add_id(GTK_SOCKET(socket), atoi(xdata.GetFirstTag("plug").GetAttr("id").c_str()));
		gtk_widget_show_all(window);
		gtk_widget_hide(instructions);
		gtk_widget_hide(scrollinstr);
	}
	else
	{
		std::string instr = querytag->GetFirstTag("instructions").GetBody();
		gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(instructions)), instr.c_str(), instr.length());
	
		if( !querytag->GetTagList("username").empty() )
			RegisterField("username", querytag->GetFirstTag("username").GetBody());
		if( !querytag->GetTagList("password").empty() )
			RegisterField("password", querytag->GetFirstTag("password").GetBody());
		if( !querytag->GetTagList("email").empty() )
			RegisterField("email", querytag->GetFirstTag("email").GetBody());
		
		gtk_widget_show_all(window);
	}
	
	
	return true;
}
