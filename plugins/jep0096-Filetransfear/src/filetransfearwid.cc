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

#include "filetransfearwid.h"
#include "fstream"

jep96Widget::jep96Widget(WLSignal *wls, WokXMLTag *xml, const std::string &lsid):
WLSignalInstance ( wls ),
from(xml->GetFirstTag("iq").GetAttr("from")),
id(xml->GetFirstTag("iq").GetAttr("id")),
session(xml->GetAttr("session")),
lsid(lsid)
{
	GtkWidget *main_vbox;
	GtkWidget *okbutton;
	GtkWidget *cancelbutton;
	GtkWidget *buttonbox;
	requested = false;
	origxml = new WokXMLTag(*xml);
	
	okbutton = gtk_button_new_with_mnemonic("_OK");
	cancelbutton = gtk_button_new_with_mnemonic("_Cancel");
	buttonbox = gtk_hbutton_box_new();
	
	main_vbox = gtk_vbox_new(false, false);
	chooser = gtk_file_chooser_widget_new(GTK_FILE_CHOOSER_ACTION_SAVE);
	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER(chooser), origxml->GetFirstTag("iq").GetFirstTag("si").GetFirstTag("file").GetAttr("name").c_str() );
	
	gtk_box_pack_start(GTK_BOX(buttonbox), okbutton, false, false, 2);
	gtk_box_pack_start(GTK_BOX(buttonbox), cancelbutton, false, false, 2);
	gtk_box_pack_start(GTK_BOX(main_vbox), chooser, true, true, 2);
	gtk_box_pack_start(GTK_BOX(main_vbox), buttonbox, false, false, 2);
	
	
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_add(GTK_CONTAINER(window), main_vbox);
	gtk_window_set_title (GTK_WINDOW (window), "File Transfear");
	
	g_signal_connect_swapped (G_OBJECT (cancelbutton), "clicked",
			      G_CALLBACK (gtk_widget_destroy),
                              G_OBJECT (window));
	g_signal_connect (G_OBJECT (window), "destroy",
					(void(*)())G_CALLBACK (jep96Widget::Destroy), this);
	g_signal_connect (G_OBJECT (okbutton), "clicked",
					G_CALLBACK(jep96Widget::ButtonPress), this);
	gtk_widget_show_all(window);
}

jep96Widget::~jep96Widget()
{
	if(!requested)
	{
		WokXMLTag msgtag(NULL, "message");
		msgtag.AddAttr("session", session);
		WokXMLTag &unauthorized = msgtag.AddTag("iq");
		unauthorized.AddAttr("type", "error");
		unauthorized.AddAttr("to", from);
		unauthorized.AddAttr("id", id);
		WokXMLTag &error = unauthorized.AddTag("error");
		error.AddAttr("code","406");
		error.AddAttr("type", "auth");
		error.AddTag("not-acceptable").AddAttr("xmlns","urn:ietf:params:xml:ns:xmpp-stanzas");
		wls->SendSignal("Jabber XML Send", &msgtag);

		WokXMLTag rejtag(NULL, "rejected");
		rejtag.AddAttr("sid", lsid);
		wls->SendSignal("Jabber Stream File Status", &rejtag);
		wls->SendSignal("Jabber Stream File Status Rejected", &rejtag);
	}
	delete origxml;
	
}

void
jep96Widget::Destroy (GtkWidget * widget, jep96Widget *c)
{
	delete c;
}

void
jep96Widget::Activate()
{
	filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(chooser));
	EXP_SIGHOOK("Jabber Stream RequestAuthorisation", &jep96Widget::FileAuth, 1000);
	
	WokXMLTag tag_x(origxml->GetFirstTag("iq").GetFirstTag("si").GetFirstTag("feature").GetFirstTag("x"));
	WokXMLTag jabberx(NULL, "data");
	
	
	jabberx.AddTag(&tag_x);
	wls->SendSignal("Jabber jabber:x:data Init", &jabberx);
	EXP_SIGHOOK(jabberx.GetAttr("signal"), &jep96Widget::xdataresp, 1000);
}

void 
jep96Widget::ButtonPress (GtkButton *button, jep96Widget *c)
{
	if ( strlen(gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(c->chooser))) == 0)
		return;
	
	std::ifstream fin;
	fin.open(gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(c->chooser)),std::ios::in);
	if( fin.is_open() )
	{
		fin.close();
		
		GtkWidget *dialog = gtk_dialog_new_with_buttons ("File Overwrite (yes/no)",
                                                  GTK_WINDOW(c->window),
                                                  (GtkDialogFlags) (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
                                                  GTK_STOCK_YES,
                                                  GTK_RESPONSE_ACCEPT,
                                                  GTK_STOCK_NO,
                                                  GTK_RESPONSE_REJECT,
                                                  NULL);
		GtkWidget *label = gtk_label_new("File already exists\nYou want to overwrite it?");
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox ) , label, false, false, 2);
		gtk_widget_show_all(dialog);
		
		gint result = gtk_dialog_run (GTK_DIALOG (dialog));
		switch (result)
		{
			case GTK_RESPONSE_ACCEPT:
				c->Activate();
				gtk_widget_destroy(dialog);
				break;
			case GTK_RESPONSE_REJECT:
				gtk_widget_destroy(dialog);
				break;
			default:
				break;
		}
	}
	else
		c->Activate();
	
	gtk_widget_hide(c->window);
}

int
jep96Widget::FileAuth(WokXMLTag *xml)
{
	if( xml->GetAttr("sid") == origxml->GetFirstTag("iq").GetFirstTag("si").GetAttr("id") &&
		xml->GetAttr("session") == origxml->GetAttr("session") && 
		xml->GetAttr("initiator") == origxml->GetFirstTag("iq").GetAttr("from"))
	{
		WokXMLTag &file = xml->AddTag("file");
		file.AddAttr("name", filename);
		file.AddAttr("lsid", lsid);
		delete this;
	}		
	
	return 1;	
}

int
jep96Widget::xdataresp(WokXMLTag *xml)
{
	if( xml->GetFirstTag("x").GetAttr("type") == "cancel")
	{
		delete this;
		return true;
	}

	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", session);
	WokXMLTag &tag = msgtag.AddTag("iq");
	
	tag.AddAttr("type", "result");
	
	tag.AddAttr("to", from);
	tag.AddAttr("id", id);
	WokXMLTag &si_tag = tag.AddTag("si");
	si_tag.AddAttr("xmlns", "http://jabber.org/protocol/si");
	WokXMLTag &feature_tag = si_tag.AddTag("feature");
	feature_tag.AddAttr("xmlns", "http://jabber.org/protocol/feature-neg");
	
	std::list <WokXMLObject *>::iterator iter;
	
	for( iter = xml->GetItemList().begin() ; iter != xml->GetItemList().end(); iter++)
		feature_tag.AddObject(*iter);
	
	wls->SendSignal("Jabber XML Send", &msgtag);
	
		
	WokXMLTag rejtag(NULL, "Accepted");
	rejtag.AddAttr("sid", lsid);
	wls->SendSignal("Jabber Stream File Status", &rejtag);
	wls->SendSignal("Jabber Stream File Status Accepted", &rejtag);

	return true;
	/*
	<iq type='result' to='sender@jabber.org/resource' id='offer1'>
	  <si xmlns='http://jabber.org/protocol/si'>
	    <feature xmlns='http://jabber.org/protocol/feature-neg'>
	      <x xmlns='jabber:x:data' type='submit'>
		<field var='stream-method'>
		  <value>http://jabber.org/protocol/bytestreams</value>
		</field>
	      </x>
	    </feature>
	  </si>
	</iq>
	*/
}

