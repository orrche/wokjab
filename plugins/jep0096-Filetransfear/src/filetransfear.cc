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

#include "filetransfear.h"
#include "filetransfearwid.h"

#include <sys/stat.h>
#include <iostream>
#include <iomanip>
#include "filepicker.h"
#include <sstream>

#include <vector>

#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#include "../../../gettext.h"
#define _(string) gettext (string)


jep96::jep96(WLSignal *wls):
WoklibPlugin(wls)
{
	config = new WokXMLTag("config");
	gxml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/streams.glade", NULL, NULL);
	GtkWidget *fileview = glade_xml_get_widget (gxml, "fileview");
	filewindow = glade_xml_get_widget(gxml, "filewindow");
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	
	g_signal_connect (G_OBJECT (filewindow), "delete_event",
			G_CALLBACK (jep96::Delete), this);	
	g_signal_connect (G_OBJECT (glade_xml_get_widget(gxml, "closebutton")), "clicked",
			G_CALLBACK (jep96::CloseWindow), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget(gxml, "removebutton")), "clicked",
			G_CALLBACK (jep96::RemoveStream), this);	
	g_signal_connect (G_OBJECT (fileview), "drag_data_get", 
			G_CALLBACK (jep96::DragGet), this);
	
	file_store = gtk_list_store_new (NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, 
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_BOOLEAN, G_TYPE_INT, G_TYPE_STRING);
  	gtk_tree_view_set_model (GTK_TREE_VIEW(fileview), GTK_TREE_MODEL(file_store));
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("File/ID", renderer, "text", FILE_ID_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (fileview), column);

	renderer = gtk_cell_renderer_progress_new ();
	column = gtk_tree_view_column_new_with_attributes ("Data Transferd", renderer, "text", DATA_TRANSFEARED_COLUMN , "value" , 9, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (fileview), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Status", renderer, "text", STATUS_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (fileview), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Sender/Receiver", renderer, "markup", SENDER_RECEIVER_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (fileview), column);

	enum
    {
      TARGET_STRING,
      TARGET_URL
    };
	static GtkTargetEntry target_entry[] =
    {      
	  { "text/uri-list",        0, TARGET_URL },	
    };
	
	gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(fileview), (GdkModifierType) GDK_BUTTON1_MASK, target_entry, 1, (GdkDragAction) (GDK_ACTION_COPY));
	
	
	EXP_SIGHOOK("Jabber XML IQ New si set xmlns:http://jabber.org/protocol/si", &jep96::Wid, 999);
	EXP_SIGHOOK("Jabber Stream File Send", &jep96::SendFile, 999);
	EXP_SIGHOOK("Jabber Stream File Menu", &jep96::JidMenuActivated, 999);
	EXP_SIGHOOK("Jabber Stream File Show", &jep96::Show, 999);
	EXP_SIGHOOK("Jabber GUI Roster GetJIDMenu", &jep96::JidMenu, 999);
	EXP_SIGHOOK("Jabber Disco Info Get", &jep96::DiscoInfo, 500);
	EXP_SIGHOOK("GetMenu", &jep96::MainMenu,999);
	
	EXP_SIGHOOK("Jabber Stream File Status Position", &jep96::Position, 500);
	EXP_SIGHOOK("Jabber Stream File Status Connected", &jep96::Connected, 500);
	EXP_SIGHOOK("Jabber Stream File Status Terminated", &jep96::Terminated, 500);
	EXP_SIGHOOK("Jabber Stream File Status Finished", &jep96::Finnished, 500);
	EXP_SIGHOOK("Jabber Stream File Status Rejected", &jep96::Rejected, 500);
	EXP_SIGHOOK("Jabber Stream File Status Accepted", &jep96::Accepted, 500);
	
	EXP_SIGHOOK("Jabber Stream Event Finished Ignore", &jep96::FinishIgnore, 1000);
	EXP_SIGHOOK("Jabber Stream Event Finished OpenFolder", &jep96::FinishOpen, 1000);
	EXP_SIGHOOK("Jabber Stream File Incomming", &jep96::Incomming,1000);
	EXP_SIGHOOK("Jabber Stream File Incomming", &jep96::NoHandleIncomming,1100);
	EXP_SIGHOOK("Jabber Stream RequestAuthorisation", &jep96::FileAuth, 1000);
	sidnum = 0;
	
	EXP_SIGHOOK("Config XML Change /filetransfer", &jep96::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/filetransfer");
	wls->SendSignal("Config XML Trigger", &conftag);
 
				
	speedcalctimerid = g_timeout_add (1000, (gboolean (*)(void *)) (jep96::SpeedCalc), this);
}

jep96::~jep96()
{	 
	g_source_destroy(g_main_context_find_source_by_id(NULL,speedcalctimerid));
	std::map <std::string, WokXMLTag*>::iterator iter;
	
	for( iter = sessions.begin() ; iter != sessions.end() ; iter++)
		delete iter->second;
	
	gtk_widget_destroy(filewindow);
	g_object_unref(gxml);

	delete config;
}

void
jep96::DragGet(GtkWidget *wgt, GdkDragContext *context, GtkSelectionData *selection, guint info, guint time, jep96 *c)
{
	GtkTreeIter iter;
	std::vector <std::string> data;
	
	GtkTreeSelection* select =  gtk_tree_view_get_selection(GTK_TREE_VIEW(glade_xml_get_widget(c->gxml, "fileview")));
	GList *list =  gtk_tree_selection_get_selected_rows(select, (GtkTreeModel**) &c->file_store);
	int listlen = g_list_length(list);
	gchar *uris[listlen+1];
	
	for(;list; list = list->next)
	{
		gchar *sid;
		gboolean sending;
		
		gtk_tree_model_get_iter (GTK_TREE_MODEL(c->file_store), &iter, (GtkTreePath *) list->data);
		gtk_tree_model_get(GTK_TREE_MODEL(c->file_store), &iter, SID_COLUMN, &sid, SENDING_COLUMN, &sending, -1);
		
		
		std::string file = c->sessions[sid]->GetAttr("file");
		
		if ( !file.empty() )
			data.push_back("file://" + file);
		
		g_free(sid);
	}
	
	int i;
	for(i = 0 ; i<data.size(); i++)
	{
		uris[i] = static_cast<gchar*>(g_malloc(sizeof(char)*data[i].size()));
		strcpy(uris[i], data[i].c_str());
	}
	uris[i] = NULL;
	
	gtk_selection_data_set_uris(selection, uris);
	
	for(i = 0; uris[i] ; i++ )
	{
		g_free(uris[i]);
	}
	
	g_list_foreach (list, ( void(*)(void*, void*)) gtk_tree_path_free, NULL);
	g_list_free (list);
}

gboolean 
jep96::SpeedCalc (jep96 * c)
{
	std::map <std::string, WokXMLTag *>::iterator iter;
	
	for( iter = c->sessions.begin() ; iter != c->sessions.end() ; iter++)
	{
		if( iter->second->GetAttr("lastpos").empty() )
		{
			iter->second->AddAttr("speed", c->PrettySize(atoll(iter->second->GetAttr("position").c_str())));
		}
		else
		{
			unsigned long long progress;
			progress = atoll(iter->second->GetAttr("position").c_str()) - atoll(iter->second->GetAttr("lastpos").c_str());
			iter->second->AddAttr("speed", c->PrettySize(progress));
		}
		iter->second->AddAttr("lastpos", iter->second->GetAttr("position"));
	}
	
	
	
	return TRUE;
}


int
jep96::NoHandleIncomming(WokXMLTag *tag)
{
	if ( tag->GetAttr("handled") != "true")
	{
		tag->AddAttr("strsize", PrettySize (atoll(tag->GetFirstTag("iq").GetFirstTag("si", "http://jabber.org/protocol/si").GetFirstTag("file", "http://jabber.org/protocol/si/profile/file-transfer").GetAttr("size").c_str())));
		tag->AddAttr("size", tag->GetFirstTag("iq").GetFirstTag("si", "http://jabber.org/protocol/si").GetFirstTag("file", "http://jabber.org/protocol/si/profile/file-transfer").GetAttr("size"));
		new jep96Widget(wls, tag, tag->GetFirstTag("filetransfer").GetAttr("lsid"));
		sessions[tag->GetFirstTag("filetransfer").GetAttr("lsid")] = new WokXMLTag (*tag);
	}
	
	return 0;
}

int
jep96::FileAuth(WokXMLTag *tag)
{
	
	std::map <std::string, WokXMLTag *>::iterator iter;
	for( iter = sessions.begin() ; iter != sessions.end() ; iter++)
	{
		if ( tag->GetAttr("sid") == iter->second->GetFirstTag("iq").GetFirstTag("si", "http://jabber.org/protocol/si").GetAttr("id") &&
			tag->GetAttr("initiator") == iter->second->GetFirstTag("iq").GetAttr("from") && 
			tag->GetAttr("session") == iter->second->GetAttr("session") &&
			iter->second->GetAttr("autoaccept") == "true" )
		{
			tag->GetFirstTag("file").AddAttr("lsid", iter->first);
			tag->GetFirstTag("file").AddAttr("name", iter->second->GetAttr("file"));			
		}
			
	}		
	return 1;
}

int
jep96::Incomming(WokXMLTag *tag)
{
	if ( config->GetFirstTag("userfolder_root").GetAttr("data").empty() || tag->GetAttr("handled") == "true")
		return 1;
	
	std::string jid = tag->GetFirstTag("iq").GetAttr("from");
	if ( jid.find("/") != std::string::npos )
		jid = jid.substr(0, jid.find("/"));
	
	std::list <WokXMLTag *>::iterator jidlist;

	for( 	jidlist = config->GetFirstTag("auto_accept").GetFirstTag("list").GetTagList("item").begin();
			jidlist != config->GetFirstTag("auto_accept").GetFirstTag("list").GetTagList("item").end();
			jidlist++)
	{
		if ( (*jidlist)->GetAttr("data") == jid )
		{
			std::string filename = config->GetFirstTag("userfolder_root").GetAttr("data") + "/" + jid + "/" + tag->GetFirstTag("iq").GetFirstTag("si", "http://jabber.org/protocol/si").GetFirstTag("file").GetAttr("name");
			
			WokXMLTag msgtag(NULL, "message");
			msgtag.AddAttr("session", tag->GetAttr("session"));
			WokXMLTag &iqtag = msgtag.AddTag("iq");
			
			iqtag.AddAttr("type", "result");
			
			iqtag.AddAttr("to", tag->GetFirstTag("iq").GetAttr("from"));
			iqtag.AddAttr("id", tag->GetFirstTag("iq").GetAttr("id"));
			WokXMLTag &si_tag = iqtag.AddTag("si", "http://jabber.org/protocol/si");
			WokXMLTag &feature_tag = si_tag.AddTag("feature", "http://jabber.org/protocol/feature-neg");
			
			WokXMLTag &x_tag = feature_tag.AddTag("x", "jabber:x:data");
			x_tag.AddAttr("type", "submit");
			WokXMLTag &field_tag = x_tag.AddTag("field");
			field_tag.AddAttr("var", "stream-method");
			field_tag.AddTag("value").AddText("http://jabber.org/protocol/bytestreams");
			
			wls->SendSignal("Jabber XML Send", &msgtag);
			tag->AddAttr("handled", "true");
			tag->AddAttr("autoaccept", "true");
			tag->AddAttr("file", filename);
			tag->AddAttr("strsize", PrettySize (atoll(tag->GetFirstTag("iq").GetFirstTag("si", "http://jabber.org/protocol/si").GetFirstTag("file", "http://jabber.org/protocol/si/profile/file-transfer").GetAttr("size").c_str())));
			tag->AddAttr("size", tag->GetFirstTag("iq").GetFirstTag("si", "http://jabber.org/protocol/si").GetFirstTag("file", "http://jabber.org/protocol/si/profile/file-transfer").GetAttr("size").c_str());
			sessions[tag->GetFirstTag("filetransfer").GetAttr("lsid")] = new WokXMLTag (*tag);
			
			WokXMLTag eventtag ("event");
			eventtag.AddAttr("type", "jep0096 AutoAcceptFile");
			
			WokXMLTag &item = eventtag.AddTag("item");
			item.AddAttr("icon", PACKAGE_DATA_DIR"/wokjab/filetransfer.png");
			item.AddAttr("session", tag->GetAttr("session"));
			item.AddAttr("timeout", "5000");
			
			std::string filen = filename;

			if ( filen.find("/") != std::string::npos )
			{
				filen = filen.substr(filen.rfind("/"));
			}

			item.AddTag("description").AddText(_("File ") + std::string(filename) + _(" autoaccepted  from ") + tag->GetFirstTag("iq").GetAttr("from"));
			item.AddAttr("jid", tag->GetFirstTag("iq").GetAttr("from"));
		
			/*
			WokXMLTag &commands = item.AddTag("commands");
			{
				WokXMLTag &command = commands.AddTag("command");
				command.AddAttr("name", _("Close"));
				WokXMLTag &signal = command.AddTag("signal");
				signal.AddAttr("name", "Jabber Stream Event Finished Ignore");
				signal.GetFirstTag("data").AddTag("sid").AddAttr("name", fintag->GetAttr("sid"));
			}*/
			
			wls->SendSignal("Jabber Event Add", eventtag);
			
			return 0;
		}
	}
	
	return 1;
}

int
jep96::ReadConfig(WokXMLTag *tag)
{
	tag->GetFirstTag("config").GetFirstTag("auto_accept").AddAttr("label", _("Auto accept from"));
	tag->GetFirstTag("config").GetFirstTag("auto_accept").AddAttr("type", "jidlist");
	
	
	tag->GetFirstTag("config").GetFirstTag("openfoler_with").AddAttr("type", "string");
	tag->GetFirstTag("config").GetFirstTag("openfoler_with").AddAttr("label", _("Open folder with"));
	openwith = tag->GetFirstTag("config").GetFirstTag("openfoler_with").GetAttr("data");
	
	tag->GetFirstTag("config").GetFirstTag("userfolder_root").AddAttr("type", "string");
	tag->GetFirstTag("config").GetFirstTag("userfolder_root").AddAttr("label", _("Root for userfolders"));
	
	tag->GetFirstTag("config").GetFirstTag("popup_transfear_wid").AddAttr("type", "bool");
	tag->GetFirstTag("config").GetFirstTag("popup_transfear_wid").AddAttr("label", _("Popup transfer window"));
	popup_ft_wid = tag->GetFirstTag("config").GetFirstTag("popup_transfear_wid").GetAttr("data");
	
	
	if ( !tag->GetFirstTag("config").GetFirstTag("proxy").GetTagList("item").empty() )
	{
		autoproxy = tag->GetFirstTag("config").GetFirstTag("proxy").GetFirstTag("item").GetBody();
		autoproxytype = tag->GetFirstTag("config").GetFirstTag("proxy").GetFirstTag("item").GetAttr("type");
	}
	else
		autoproxy = "";
	
	delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));
	return 1;
}

gboolean 
jep96::Delete( GtkWidget *widget, GdkEvent *event, jep96 *c)
{
	gtk_widget_hide(c->filewindow);
	
	return true;
}

void
jep96::CloseWindow(GtkButton *button, jep96 *c)
{
	gtk_widget_hide(c->filewindow);
}

void
jep96::RemoveStream(GtkButton *button, jep96 *c)
{
	GtkTreeIter       iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(glade_xml_get_widget (c->gxml, "fileview")));
	
	if(gtk_tree_selection_get_selected(selection,  NULL, &iter))
	{
		gchar *sid;
		gtk_tree_model_get(GTK_TREE_MODEL(c->file_store), &iter, 3, &sid, -1);
		
		if ( c->rows.find(sid) != c->rows.end() )
		{
			// g_object_unref(c->rows[sid]); <-- dont get why I shouldn't clean this up
			c->rows.erase(sid);
		}
		if ( c->sessions.find(sid) != c->sessions.end() )
		{	
			WokXMLTag nodata(NULL, "nodata");
			c->wls->SendSignal("Jabber Stream File Send Abort " + c->sessions[sid]->GetFirstTag("iq").GetFirstTag("si").GetAttr("id"), nodata);
		
			delete (c->sessions[sid]);
			c->sessions.erase(sid);
		}
		
		if(gtk_list_store_remove(c->file_store, &iter) == TRUE)
		{
			gtk_tree_selection_select_iter(selection, &iter);	
		}
		g_free(sid);
	}
}

int
jep96::Wid(WokXMLTag *xml)
{

	std::stringstream sslsid;
	sslsid << "jep96-" << sidnum++;
	
	GtkTreeIter iter;
	
	gtk_list_store_append (file_store, &iter);  /* Acquire a top-level iterator */
	gtk_list_store_set (file_store, &iter, 0, (xml->GetFirstTag("iq").GetFirstTag("si").GetFirstTag("file").GetAttr("name")).c_str(),
			1, "--",
			2, "Negotiating" , 
			3, sslsid.str().c_str(), 
			4, ("<b>Sender:</b>" + xml->GetFirstTag("iq").GetAttr("from") + "\n<b>Receiver:</b>" + xml->GetFirstTag("iq").GetAttr("to")).c_str(),
			6, xml->GetAttr("session").c_str(), 
			7, NULL,
			8, FALSE, 
			-1);
	
	if ( popup_ft_wid != "false" )
	{
		
		
	}
//	gtk_widget_show(filewindow);
//	gtk_window_present (GTK_WINDOW(filewindow));
	
	rows[sslsid.str()] = gtk_tree_row_reference_new(GTK_TREE_MODEL(file_store),gtk_tree_model_get_path(GTK_TREE_MODEL(file_store), &iter));

	xml->AddTag("filetransfer").AddAttr("lsid", sslsid.str());
	
	std::stringstream signal;
	signal << "Jabber Stream File Incomming " << XMLisize(xml->GetFirstTag("iq").GetFirstTag("si").GetAttr("id"));
	
	wls->SendSignal(signal.str(), xml);
	if ( xml->GetAttr("handled") != "true")
		wls->SendSignal("Jabber Stream File Incomming", xml);
	
	return 1;
}

int
jep96::JidMenu(WokXMLTag *xml)
{
	WokXMLTag &option = xml->AddTag("item");
	option.AddAttr("name", "Send File");
	option.AddAttr("signal", "Jabber Stream File Menu");
	
	return 1;
}

int
jep96::MainMenu(WokXMLTag *tag)
{
	WokXMLTag *menu_item;
	
	menu_item = &tag->AddTag("item");
	menu_item->AddAttr("name", "Transfer");
	menu_item->AddAttr("signal", "Jabber Stream File Show");
	
	return 1;
}

int
jep96::Show(WokXMLTag *xml)
{
	gtk_widget_show_all(filewindow);
	gtk_window_present(GTK_WINDOW(filewindow));
	
	return 1;
}

int
jep96::JidMenuActivated(WokXMLTag *xml)
{
	std::string mto = xml->GetAttr("jid");
	
	if( mto.find("/") == std::string::npos )
	{
		WokXMLTag tag(NULL, "query");
		WokXMLTag &itemtag = tag.AddTag("item");
		itemtag.AddAttr("jid", mto);
		itemtag.AddAttr("session", xml->GetAttr("session"));
		
		wls->SendSignal("Jabber Roster GetResource", &tag);
		mto = mto + "/" + itemtag.GetFirstTag("resource").GetAttr("name");		
	}
	
	new filepicker(wls, xml->GetAttr("session"), mto);
	
	return 1;
}
/**
 * <send to="jid" name="path to file" popup="false" sid="id"/>
 * 
 *
 *
 */
int
jep96::SendFile(WokXMLTag *xml)
{
	unsigned long long size = 0;
	struct stat results;
	std::string filename;
	
	std::string to = xml->GetAttr("to");
	std::string file = xml->GetAttr("name");
	std::string sid;
	if ( xml->GetAttr("sid").empty() )
	{
		std::stringstream ssid;
		ssid << "jep96-" << sidnum++ << "-";
		sid = ssid.str();
		
		
		for(int i = 0; i < 10; i++)
		{
			sid+= rand() % 20 + 'a';
		}
		
		xml->AddAttr("sid", sid);
	}
	else
		sid = xml->GetAttr("sid");
		
	if (stat(file.c_str(), &results) == 0)
		size = results.st_size;
	else
		return 1;
	
	std::stringstream ssize;
	ssize << size;
	
	
	filename = file.substr(file.rfind("/")+1);
	
	GtkTreeIter iter;
	
	std::string myjid;
	WokXMLTag jiddata("session");
	jiddata.AddTag("item").AddAttr("session", xml->GetAttr("session"));
	wls->SendSignal("Jabber Connection GetUserData", jiddata);
	myjid = jiddata.GetFirstTag("item").GetFirstTag("username").GetBody() + "@" +
			jiddata.GetFirstTag("item").GetFirstTag("server").GetBody() + "/" +
			jiddata.GetFirstTag("item").GetFirstTag("resource").GetBody();
	
	
	gtk_list_store_append (file_store, &iter);
	gtk_list_store_set (file_store, &iter, 0, (filename).c_str(),
			1, "--",
			2, "Negotiating" , 
			3, sid.c_str(),
			4, ("<b>Sender:</b>" + myjid + "\n<b>Receiver:</b>" + to).c_str(),
			6, xml->GetAttr("session").c_str(), 
			7, NULL,
			8, TRUE, 
			-1);
	
	if ( popup_ft_wid != "false" && xml->GetAttr("popup") != "false" )
	{
		WokXMLTag popup("popup");
		wls->SendSignal("Jabber Stream File Show", popup);
	}
	
	rows[sid] = gtk_tree_row_reference_new(GTK_TREE_MODEL(file_store),gtk_tree_model_get_path(GTK_TREE_MODEL(file_store), &iter));
	
	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", xml->GetAttr("session"));
	
	WokXMLTag &iqtag = msgtag.AddTag("iq");
	iqtag.AddAttr("to", xml->GetAttr("to"));
	iqtag.AddAttr("type", "set");
	iqtag.AddAttr("xmlns", "jabber:client");
	
	WokXMLTag &si_tag = iqtag.AddTag("si");
	si_tag.AddAttr("xmlns", "http://jabber.org/protocol/si");
	si_tag.AddAttr("profile", "http://jabber.org/protocol/si/profile/file-transfer");
	si_tag.AddAttr("id", sid);
	
	WokXMLTag &file_tag = si_tag.AddTag("file");
	file_tag.AddAttr("xmlns", "http://jabber.org/protocol/si/profile/file-transfer");
	file_tag.AddAttr("name", filename);
	file_tag.AddAttr("size", ssize.str());
	file_tag.AddTag("range");
	
	WokXMLTag &feature_tag = si_tag.AddTag("feature");
	feature_tag.AddAttr("xmlns", "http://jabber.org/protocol/feature-neg");
	WokXMLTag &x_tag = feature_tag.AddTag("x");
	x_tag.AddAttr("xmlns","jabber:x:data");
	x_tag.AddAttr("type", "form");
	
	WokXMLTag &streammethod = x_tag.AddTag("field");
	streammethod.AddAttr("var", "stream-method");
	streammethod.AddAttr("type", "list-single");
	streammethod.AddTag("option").AddTag("value").AddText("http://jabber.org/protocol/bytestreams");
	
	wls->SendSignal("Jabber XML IQ Send", msgtag);
	
	WokXMLTag *data = new WokXMLTag (msgtag);
	if ( xml->GetAttr("proxy_type") == "auto")
	{
		data->AddAttr("proxy_type", autoproxytype );
		data->AddAttr("proxy", autoproxy );	
	}
	else
	{
		data->AddAttr("proxy_type", xml->GetAttr("proxy_type"));
		data->AddAttr("proxy", xml->GetAttr("proxy"));
	}
	
	data->AddAttr("rate", xml->GetAttr("rate"));
	data->AddAttr("file", file);
	data->AddAttr("strsize", PrettySize(size));
	data->AddAttr("size", ssize.str());
	data->AddAttr("popup", xml->GetAttr("popup"));
	data->AddAttr("event", xml->GetAttr("event"));
	data->AddTag(&file_tag);
	
	sessions[iqtag.GetAttr("id")] = data;
	sessions[sid] = new WokXMLTag (*data);
	EXP_SIGHOOK("Jabber XML IQ ID "  + iqtag.GetAttr("id"), &jep96::SendReply, 500);
	return 1;
}

int
jep96::FinishIgnore(WokXMLTag *tag)
{
	GtkTreeIter iter;
	if ( rows.find(tag->GetFirstTag("sid").GetAttr("name")) != rows.end() )
	{
		if( gtk_tree_model_get_iter(GTK_TREE_MODEL(file_store), &iter, gtk_tree_row_reference_get_path(rows[tag->GetFirstTag("sid").GetAttr("name")])))
		{
			WokXMLTag *xml;
			gtk_tree_model_get(GTK_TREE_MODEL(file_store), &iter, 7, &xml, -1);
			if ( xml )
			{
				wls->SendSignal("Jabber Event Remove", xml);
				gtk_list_store_set (file_store, &iter, 7, NULL , -1);
			
				delete xml;
			}
		}
	}
	return 1;	
}

int
jep96::FinishOpen(WokXMLTag *tag)
{
	int pid,status;
	switch (pid = fork()){
		case 0:
			switch (fork())
			{
				case 0:
					system((openwith + " " + tag->GetFirstTag("folder").GetAttr("name")).c_str());
					_exit(1);
			}
			_exit(1);
	}
	
	if(pid > 0)
		waitpid(pid, &status, 0);
	
	FinishIgnore(tag);
	return 1;
}

int
jep96::Finnished(WokXMLTag *fintag)
{
	GtkTreeIter iter;
	bool showevent = true;
		
	if ( sessions.find(fintag->GetAttr("sid")) != sessions.end() )
	{
		if ( ! fintag->GetAttr("filename").empty() )
			sessions[fintag->GetAttr("sid")]->AddAttr("file", fintag->GetAttr("filename"));
		if( sessions[fintag->GetAttr("sid")]->GetAttr("event") == "false" )
					showevent = false;
					
	}
	if ( rows.find(fintag->GetAttr("sid")) != rows.end() && showevent)
	{
		if( gtk_tree_model_get_iter(GTK_TREE_MODEL(file_store), &iter, gtk_tree_row_reference_get_path(rows[fintag->GetAttr("sid")])))
		{
			gchar *session, *filename;
			gboolean sending;
			
			WokXMLTag *eventtag;
			eventtag = new WokXMLTag("event");
			
			gtk_list_store_set (file_store, &iter, 2, "Finished", 7, eventtag, -1);
			gtk_tree_model_get(GTK_TREE_MODEL(file_store), &iter, 0, &filename, 6, &session, 8, &sending, -1);
			
			eventtag->AddAttr("type", "jep0096 FinishedFile");
			WokXMLTag &item = eventtag->AddTag("item");
			item.AddAttr("icon", PACKAGE_DATA_DIR"/wokjab/filetransfer.png");
			item.AddAttr("session", session);
			if ( sending == TRUE )
			{
				item.AddTag("description").AddText(_("File ") + std::string(filename) + _(" finished sending to ") + sessions[fintag->GetAttr("sid")]->GetFirstTag("iq").GetAttr("to"));
				item.AddAttr("jid", sessions[fintag->GetAttr("sid")]->GetFirstTag("iq").GetAttr("to"));
			}
			else
			{
				item.AddTag("description").AddText(_("File ") + std::string(filename) + _(" finished reciving from ") + sessions[fintag->GetAttr("sid")]->GetFirstTag("iq").GetAttr("from"));
				item.AddAttr("jid", sessions[fintag->GetAttr("sid")]->GetFirstTag("iq").GetAttr("from"));
			}
			
			WokXMLTag &commands = item.AddTag("commands");
			{
				WokXMLTag &command = commands.AddTag("command");
				command.AddAttr("name", _("Close"));
				WokXMLTag &signal = command.AddTag("signal");
				signal.AddAttr("name", "Jabber Stream Event Finished Ignore");
				signal.GetFirstTag("data").AddTag("sid").AddAttr("name", fintag->GetAttr("sid"));
			}
			if ( !openwith.empty() && !fintag->GetAttr("filename").empty())
			{
				WokXMLTag &command = commands.AddTag("command");
				command.AddAttr("name", "Open Folder");
				WokXMLTag &signal = command.AddTag("signal");
				signal.AddAttr("name", "Jabber Stream Event Finished OpenFolder");
				signal.GetFirstTag("data").AddTag("sid").AddAttr("name", fintag->GetAttr("sid"));
				signal.GetFirstTag("data").AddTag("folder").AddAttr("name", fintag->GetAttr("filename").substr(0,fintag->GetAttr("filename").rfind("/")));
			}
			
			wls->SendSignal("Jabber Event Add", eventtag);
			g_free(filename);
			g_free(session);
		}
	}

	return 1;
}

int
jep96::Accepted(WokXMLTag *acctag)
{
	GtkTreeIter iter;
	GtkTreePath* path;
	
		
	std::cout << "Accepted data trigger " << *acctag << std::endl;
		
	if ( rows.find(acctag->GetAttr("sid")) != rows.end() )
	{
		if( ( path = gtk_tree_row_reference_get_path(rows[acctag->GetAttr("sid")]) ) != NULL )
		{
			if( gtk_tree_model_get_iter(GTK_TREE_MODEL(file_store), &iter, path ))
			{
				gchar *tt;
				gtk_tree_model_get (GTK_TREE_MODEL(file_store), &iter, TOOLTIP_COLUMN, &tt, -1);
				if ( tt )
				{
						gtk_list_store_set (file_store, &iter, 2, "Accepted", TOOLTIP_COLUMN, (tt + acctag->GetFirstTag("message").GetFirstTag("body").GetBody()).c_str()  , -1);

						g_free(tt);
				}
				else
						gtk_list_store_set (file_store, &iter, 2, "Accepted", TOOLTIP_COLUMN, acctag->GetFirstTag("message").GetFirstTag("body").GetBody().c_str()  , -1);
					
			}
		}
	}
	
	bool file_denied_popup = true;
	if ( !acctag->GetAttr("file").empty())
	{
		if ( sessions.find(acctag->GetAttr("sid")) != sessions.end() )
		{
			sessions[acctag->GetAttr("sid")]->AddAttr("file", acctag->GetAttr("file"));
			if ( sessions[acctag->GetAttr("sid")]->GetAttr("popup") == "false" )
						file_denied_popup = false;
		}
		else
			file_denied_popup = false;
	}
	
	if ( popup_ft_wid != "false" && acctag->GetAttr("popup") != "false" && !file_denied_popup )
	{
		WokXMLTag popup("popup");
		wls->SendSignal("Jabber Stream File Show", popup);
	}
	return 1;
}

int
jep96::Terminated(WokXMLTag *termtag)
{
	GtkTreeIter iter;
	
	if ( rows.find(termtag->GetAttr("sid")) != rows.end() )
	{
		if( gtk_tree_model_get_iter(GTK_TREE_MODEL(file_store), &iter, gtk_tree_row_reference_get_path(rows[termtag->GetAttr("sid")])))
		{
			gchar *tt;
			gtk_tree_model_get (GTK_TREE_MODEL(file_store), &iter, TOOLTIP_COLUMN, &tt, -1);
			if ( tt )
			{
				gtk_list_store_set (file_store, &iter, 2, "Terminated", TOOLTIP_COLUMN, (tt + termtag->GetFirstTag("message").GetFirstTag("body").GetBody()).c_str()  , -1);
			
				g_free(tt);
			}
			else
				gtk_list_store_set (file_store, &iter, 2, "Terminated", TOOLTIP_COLUMN, termtag->GetFirstTag("message").GetFirstTag("body").GetBody().c_str()  , -1);
		}
	}
	
	return 1;
}

std::string
jep96::PrettySize(unsigned long long size)
{
	unsigned long long divider;
	std::string ending;
	
	if( size > (unsigned long long)(1024) * 1024 * 1024 * 1024 )
	{
		divider = (unsigned long long)(1024) * 1024 * 1024 * 1024;
		ending = "TB";
	}
	else if( size > 1024 * 1024 * 1024 )
	{
		divider = 1024 * 1024 * 1024;
		ending = "GB";
	}
	else if ( size > 1024 * 1024 )
	{
		divider = 1024 * 1024;
		ending = "MB";
	}
	else if ( size > 1024 )	
	{
		divider = 1024;
		ending ="kB";
	}
	else
	{
		divider = 1;
		ending = "B";
	}
	
	std::stringstream msg;
	msg << std::fixed << std::setprecision(2) << double(size)/double(divider) << ending;
	
	return msg.str();
}

void
jep96::UpdateRowPosition(std::string sid)
{
	std::stringstream msg;
	msg << PrettySize(atol(sessions[sid]->GetAttr("position").c_str())) << "/" << sessions[sid]->GetAttr("strsize") << "\n" << sessions[sid]->GetAttr("speed") << "/s";
	
	GtkTreeIter iter;
	if( gtk_tree_model_get_iter(GTK_TREE_MODEL(file_store), &iter, gtk_tree_row_reference_get_path(rows[sid])))
	{
		long long int size = atoll(sessions[sid]->GetAttr("size").c_str());
		if ( size )
		{
			int progress = 100 * atoll(sessions[sid]->GetAttr("position").c_str()) / size;
			gtk_list_store_set (file_store, &iter, 1, msg.str().c_str() , 9, progress, -1);
		}
		else
			gtk_list_store_set (file_store, &iter, 1, msg.str().c_str() , 9, 100, -1);

	}
}

int
jep96::Position(WokXMLTag *postag)
{
	if ( sessions.find(postag->GetAttr("sid")) != sessions.end())
	{
		sessions[postag->GetAttr("sid")]->AddAttr("position", postag->GetAttr("pos"));
		UpdateRowPosition(postag->GetAttr("sid"));
	}
	
	return true;
}

int
jep96::Connected(WokXMLTag *contag)
{
	GtkTreeIter iter;
	
	if( rows.find(contag->GetAttr("sid")) != rows.end() )
	{
		if( gtk_tree_model_get_iter(GTK_TREE_MODEL(file_store), &iter, gtk_tree_row_reference_get_path(rows[contag->GetAttr("sid")])))
		{
			gtk_list_store_set (file_store, &iter, 2, "Connected" , -1);
		}
	}
	
	return true;
}

int
jep96::Rejected(WokXMLTag *rejtag)
{
	GtkTreeIter iter;
	if ( rows.find(rejtag->GetAttr("sid")) != rows.end() )
	{
		if( gtk_tree_model_get_iter(GTK_TREE_MODEL(file_store), &iter, gtk_tree_row_reference_get_path(rows[rejtag->GetAttr("sid")])))
		{
			gtk_list_store_set (file_store, &iter, 2, "Rejected" , -1);
		}
	}
	
	return true;
}

int
jep96::SendReply(WokXMLTag *msgtag)
{
	WokXMLTag &iqtag = msgtag->GetFirstTag("iq");
	std::string id = iqtag.GetAttr("id");
	
	
	if ( sessions.find(id) == sessions.end() )
		return true;
	
	std::string sid = sessions[id]->GetFirstTag("iq").GetFirstTag("si").GetAttr("id");
	
	if(iqtag.GetAttr("type") == "error")
	{
		// File not wanted by reciver..
		GtkTreeIter iter;
		if( gtk_tree_model_get_iter(GTK_TREE_MODEL(file_store), &iter, gtk_tree_row_reference_get_path(rows[sid])))
		{
			WokXMLTag contag(NULL, "Rejected");
			contag.AddAttr("sid", sid);
			wls->SendSignal("Jabber Stream File Status", &contag);
			wls->SendSignal("Jabber Stream File Status Rejected", &contag);
		}
	}
	else
	{
		GtkTreeIter iter;
		if( gtk_tree_model_get_iter(GTK_TREE_MODEL(file_store), &iter, gtk_tree_row_reference_get_path(rows[sid])))
		{
			WokXMLTag contag(NULL, "Accepted");
			contag.AddAttr("sid", sid);
			wls->SendSignal("Jabber Stream File Status", &contag);
			wls->SendSignal("Jabber Stream File Status Accepted", &contag);
		}
		
		std::string method = iqtag.GetFirstTag("si").GetFirstTag("feature").GetFirstTag("x").GetFirstTag("field").GetFirstTag("value").GetBody();
		if ( method.find(" ") == std::string::npos )
		{
			WokXMLTag filesend(NULL, "file");
			filesend.AddTag(&sessions[id]->GetFirstTag("file"));
			filesend.AddAttr("file", sessions[id]->GetAttr("file"));
			filesend.AddAttr("to", sessions[id]->GetFirstTag("iq").GetAttr("to"));
			filesend.AddAttr("rate", sessions[id]->GetAttr("rate"));
			filesend.AddAttr("proxy", sessions[id]->GetAttr("proxy"));
			filesend.AddAttr("proxy_type", sessions[id]->GetAttr("proxy_type"));
			filesend.AddAttr("sid", sid);
			filesend.AddAttr("session", msgtag->GetAttr("session"));
			filesend.AddTag(&iqtag);
			wls->SendSignal("Jabber Stream File Send Method " + method, filesend);
		}
	}
	
	delete sessions[id];
	sessions.erase(id);
	
	return 1;
}


int
jep96::DiscoInfo(WokXMLTag *tag)
{
	tag->AddTag("feature").AddAttr("var", "http://jabber.org/protocol/si");
	tag->AddTag("feature").AddAttr("var", "http://jabber.org/protocol/si/profile/file-transfer");

	return true;
}


