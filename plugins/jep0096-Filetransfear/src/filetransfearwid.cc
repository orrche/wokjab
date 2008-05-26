/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <nedo80@gmail.com>
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

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#ifdef __WIN32
#include <io.h>
#endif

jep96Widget::jep96Widget(WLSignal *wls, WokXMLTag *xml, const std::string &lsid):
WLSignalInstance ( wls ),
from(xml->GetFirstTag("iq").GetAttr("from")),
id(xml->GetFirstTag("iq").GetAttr("id")),
session(xml->GetAttr("session")),
lsid(lsid)
{
	config = new WokXMLTag(NULL, "NULL");
	EXP_SIGHOOK("Config XML Change /filetransfer", &jep96Widget::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/filetransfer");
	wls->SendSignal("Config XML Trigger", &conftag);
	
	requested = false;
	origxml = new WokXMLTag(*xml);
	default_question = false;
	if ( origxml->GetFirstTag("iq").GetFirstTag("si").GetFirstTag("feature").GetFirstTag("x").GetTagList("field").size() == 1 )
	{
		if ( origxml->GetFirstTag("iq").GetFirstTag("si").GetFirstTag("feature").GetFirstTag("x").GetFirstTag("field").GetAttr("var") == "stream-method" )
		{
			std::list <WokXMLTag *>::iterator iter;
			
			for( iter = origxml->GetFirstTag("iq").GetFirstTag("si").GetFirstTag("feature").GetFirstTag("x").GetFirstTag("field").GetTagList("option").begin() ;
				 iter != origxml->GetFirstTag("iq").GetFirstTag("si").GetFirstTag("feature").GetFirstTag("x").GetFirstTag("field").GetTagList("option").end() ;
				iter++)
			{
				if ( (*iter)->GetFirstTag("value").GetBody() == "http://jabber.org/protocol/bytestreams" )
				{
					default_question = true;
					break;
				}				
			}
		}
	}
	
	if ( default_question && ! config->GetFirstTag("userfolder_root").GetAttr("data").empty() )
	{
		
	}
	
	eventtag = new WokXMLTag("event");
	eventtag->AddAttr("type", "jep0096 IncommingFile");
	WokXMLTag &item = eventtag->AddTag("item");
	item.AddAttr("icon", "/usr/local/share/wokjab/filetransfer.png");
	item.AddAttr("jid", from);
	item.AddAttr("session", session);
	item.AddTag("description").AddText(from + " sends you the file ");
	WokXMLTag &commands = item.AddTag("commands");
	{
		WokXMLTag &command = commands.AddTag("command");
		command.AddAttr("name", "Recive File");
		WokXMLTag &signal = command.AddTag("signal");
		signal.AddAttr("name", "Jabber Stream ReciveWid Open " + id);
		signal.AddTag("empty"); // We need to send some thing :) 
	}
	{
		WokXMLTag &command = commands.AddTag("command");
		command.AddAttr("name", "To Desktop");
		WokXMLTag &signal = command.AddTag("signal");
		signal.AddAttr("name", "Jabber Stream ReciveWid Open " + id);
		WokXMLTag &settings = signal.AddTag("settings"); 
		WokXMLTag &destfolder = settings.AddTag("destination_folder");
		destfolder.AddText(std::string(g_get_home_dir()) + "/Desktop");
	}
	{
		if ( ! config->GetFirstTag("userfolder_root").GetAttr("data").empty() )
		{
			WokXMLTag &command = commands.AddTag("command");
			command.AddAttr("name", "To User Folder");
			WokXMLTag &signal = command.AddTag("signal");
			signal.AddAttr("name", "Jabber Stream ReciveWid Open " + id);
			WokXMLTag &settings = signal.AddTag("settings"); 
			WokXMLTag &destfolder = settings.AddTag("destination_folder");
			if ( from.find("/") == std::string::npos )
				destfolder.AddText(config->GetFirstTag("userfolder_root").GetAttr("data") + "/" + from);
			else
				destfolder.AddText(config->GetFirstTag("userfolder_root").GetAttr("data") + "/" + from.substr(0, from.find("/")));
		}
	}

	EXP_SIGHOOK("Jabber Stream ReciveWid Open " + id, &jep96Widget::Open, 1000);
	
	wls->SendSignal("Jabber Event Add", eventtag);
		
	/*
	<signal level='3' name='Jabber Event Add'>
		<event>
			<item icon='/usr/local/share/wokjab/msg.png' jid='nedo@jabber.se/wokjab' session='jabber0'>
				<description>nedo@jabber.se/wokjab</description>
				<commands>
					<command name='Open Dialog'>
						<signal name='Jabber GUI MessageDialog Open'>
							<item jid='nedo@jabber.se/wokjab' session='jabber0'></item>
						</signal>
					</command>
				</commands>
			</item>
		</event>
	</signal>
	*/
}

jep96Widget::~jep96Widget()
{
	if(!requested)
	{
		woklib_debug(wls, "File is not requested");
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

int
jep96Widget::ReadConfig (WokXMLTag *tag)
{
	delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));
	return 1;
}

int
jep96Widget::Open(WokXMLTag *tag)
{
	if ( !eventtag ) 
		return 1;
//	EXP_SIGUNHOOK("Jabber Stream ReciveWid Open " + id, &jep96Widget::Open, 1000);
	wls->SendSignal("Jabber Event Remove", eventtag);
	delete eventtag;
	eventtag = NULL;
	
	if ( (!tag->GetFirstTag("destination_folder").GetBody().empty()) && default_question )
	{
		requested = true;		
		
		{
			std::string filename = tag->GetFirstTag("destination_folder").GetBody()+"/";
			std::string::size_type pos = filename.find("/");
			while( pos != std::string::npos )
			{
		#ifdef __WIN32
					mkdir(filename.substr(0, pos).c_str());
		#else
					mkdir(filename.substr(0, pos).c_str(), 0700);
		#endif
					pos = filename.find("/", pos + 1);
			}
		}
		filename = tag->GetFirstTag("destination_folder").GetBody() + "/" + origxml->GetFirstTag("iq").GetFirstTag("si").GetFirstTag("file").GetAttr("name");
		EXP_SIGHOOK("Jabber Stream RequestAuthorisation", &jep96Widget::FileAuth, 1000);

		WokXMLTag msgtag(NULL, "message");
		msgtag.AddAttr("session", session);
		WokXMLTag &tag = msgtag.AddTag("iq");
		
		tag.AddAttr("type", "result");
		
		tag.AddAttr("to", from);
		tag.AddAttr("id", id);
		WokXMLTag &si_tag = tag.AddTag("si", "http://jabber.org/protocol/si");
		WokXMLTag &feature_tag = si_tag.AddTag("feature", "http://jabber.org/protocol/feature-neg");
		
		WokXMLTag &x_tag = feature_tag.AddTag("x", "jabber:x:data");
		x_tag.AddAttr("type", "submit");
		WokXMLTag &field_tag = x_tag.AddTag("field");
		field_tag.AddAttr("var", "stream-method");
		field_tag.AddTag("value").AddText("http://jabber.org/protocol/bytestreams");
		/*
		<x xmlns='jabber:x:data' type='submit'>
        	<field var='stream-method'>
          		<value>http://jabber.org/protocol/bytestreams</value>
        	</field>
      	</x>
		*/
		
		wls->SendSignal("Jabber XML Send", &msgtag);
		
			
		WokXMLTag rejtag(NULL, "Accepted");
		rejtag.AddAttr("sid", lsid);
		wls->SendSignal("Jabber Stream File Status", &rejtag);
		wls->SendSignal("Jabber Stream File Status Accepted", &rejtag);
	}
	else
	{
		GtkWidget *main_vbox;
		GtkWidget *okbutton;
		GtkWidget *cancelbutton;
		GtkWidget *buttonbox;
		GtkWidget *jid_label;
		GtkWidget *socket;
		
		socket = gtk_socket_new();
		
		okbutton = gtk_button_new_with_mnemonic("_OK");
		cancelbutton = gtk_button_new_with_mnemonic("_Cancel");
		buttonbox = gtk_hbutton_box_new();
		
		std::string filename = tag->GetFirstTag("destination_folder").GetBody()+"/";
		std::string::size_type pos = filename.find("/");
		while( pos != std::string::npos )
		{
	#ifdef __WIN32
				mkdir(filename.substr(0, pos).c_str());
	#else
				mkdir(filename.substr(0, pos).c_str(), 0700);
	#endif
				pos = filename.find("/", pos + 1);
		}
		
		main_vbox = gtk_vbox_new(false, false);
		chooser = gtk_file_chooser_widget_new(GTK_FILE_CHOOSER_ACTION_SAVE);
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER(chooser), origxml->GetFirstTag("iq").GetFirstTag("si").GetFirstTag("file").GetAttr("name").c_str() );
		if( ! tag->GetFirstTag("destination_folder").GetBody().empty() )
			gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER(chooser), ("file://" + tag->GetFirstTag("destination_folder").GetBody()+"/").c_str());
		

		
		jid_label = gtk_label_new(origxml->GetFirstTag("iq").GetAttr("from").c_str());
		
		gtk_box_pack_start(GTK_BOX(buttonbox), okbutton, false, false, 2);
		gtk_box_pack_start(GTK_BOX(buttonbox), cancelbutton, false, false, 2);
		gtk_box_pack_start(GTK_BOX(main_vbox), jid_label, false, false, 2);
		gtk_box_pack_start(GTK_BOX(main_vbox), chooser, true, true, 2);
		gtk_box_pack_start(GTK_BOX(main_vbox), socket, false, false, 2);
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
		
		WokXMLTag tag_x(origxml->GetFirstTag("iq").GetFirstTag("si").GetFirstTag("feature").GetFirstTag("x"));
		WokXMLTag jabberx(NULL, "data");
		
		jabberx.AddTag(&tag_x);
		jabberx.AddTag("plug");
		wls->SendSignal("Jabber jabber:x:data Init", &jabberx);
		xdataid = jabberx.GetAttr("id");
		gtk_socket_add_id(GTK_SOCKET(socket), atoi(jabberx.GetFirstTag("plug").GetAttr("id").c_str()));
		
	}
	
	return 1;
}

void
jep96Widget::Destroy (GtkWidget * widget, jep96Widget *c)
{
	delete c;
}

void
jep96Widget::Activate()
{
	requested = true;
	filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(chooser));
	EXP_SIGHOOK("Jabber Stream RequestAuthorisation", &jep96Widget::FileAuth, 1000);

	WokXMLTag xdata(NULL, "empty");
	wls->SendSignal("Jabber jabber:x:data Get " + xdataid, xdata);
	xdataresp(&xdata);
	/*
	WokXMLTag tag_x(origxml->GetFirstTag("iq").GetFirstTag("si").GetFirstTag("feature").GetFirstTag("x"));
	WokXMLTag jabberx(NULL, "data");
	
	
	jabberx.AddTag(&tag_x);
	wls->SendSignal("Jabber jabber:x:data Init", &jabberx);
	EXP_SIGHOOK(jabberx.GetAttr("signal"), &jep96Widget::xdataresp, 1000);
	*/
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
	rejtag.AddAttr("file", filename);
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

