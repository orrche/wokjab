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

#include "filetransfear.h"
#include "filetransfearwid.h"

#include <sys/stat.h>
#include "iostream"
#include "filepicker.h"
#include <sstream>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

jep96::jep96(WLSignal *wls):
WoklibPlugin(wls)
{
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
	
	file_store = gtk_list_store_new (4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
  gtk_tree_view_set_model (GTK_TREE_VIEW(fileview), GTK_TREE_MODEL(file_store));
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("File/ID", renderer, "text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (fileview), column);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Data Transfeard", renderer, "text", 1, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (fileview), column);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Status", renderer, "text", 2, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (fileview), column);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("sid", renderer, "text", 3, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (fileview), column);
	
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
	EXP_SIGHOOK("Jabber Stream File Status Finnished", &jep96::Finnished, 500);
	EXP_SIGHOOK("Jabber Stream File Status Rejected", &jep96::Rejected, 500);
	EXP_SIGHOOK("Jabber Stream File Status Accepted", &jep96::Accepted, 500);
	filetag = NULL;
	sidnum = 0;
}

jep96::~jep96()
{
	if(filetag)
		delete filetag;
	
	std::map <std::string, WokXMLTag*>::iterator iter;
	
	for( iter = sessions.begin() ; iter != sessions.end() ; iter++)
		delete iter->second;
	
	gtk_widget_destroy(filewindow);
	g_object_unref(gxml);
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
	
	if(gtk_tree_selection_get_selected(selection,  NULL, &iter));
	{
		gchar *sid;
		
		gtk_tree_model_get(GTK_TREE_MODEL(c->file_store), &iter, 3, &sid, -1);
		
		std::cout << "Sid: " << sid << std::endl;
		
		if ( c->rows.find(sid) != c->rows.end() )
		{
			g_free(c->rows[sid]);
			c->rows.erase(sid);
			std::cout << "Row size: " << c->rows.size() << std::endl;
		}
		if ( c->sessions.find(sid) != c->sessions.end() )
		{
			delete (c->sessions[sid]);
			c->sessions.erase(sid);
			std::cout << "Session size: " << c->sessions.size() << std::endl;
		}
		
		gtk_list_store_remove(c->file_store, &iter);
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
	gtk_list_store_set (file_store, &iter, 0, xml->GetFirstTag("iq").GetFirstTag("si").GetFirstTag("file").GetAttr("name").c_str(),
											1, "--",
											2, "Negotiating" , 
											3, sslsid.str().c_str(), -1);
	gtk_widget_show(filewindow);
	gtk_window_present (GTK_WINDOW(filewindow));
	rows[sslsid.str()] = gtk_tree_row_reference_new(GTK_TREE_MODEL(file_store),gtk_tree_model_get_path(GTK_TREE_MODEL(file_store), &iter));

	
	new jep96Widget(wls, xml, sslsid.str());
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
	menu_item->AddAttr("name", "Transfear");
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

int
jep96::SendFile(WokXMLTag *xml)
{
	int size = 0;
	struct stat results;
	std::string filename;
	std::string ssize;
	char buff[20];
	
	std::string to = xml->GetAttr("to");
	std::string file = xml->GetAttr("name");
	sprintf(buff,"%d", sidnum++);
	std::string sid = std::string("jep96-") + buff + "-";
	
	for(int i = 0; i < 10; i++)
	{
		sid+= rand() % 20 + 'a';
	}
	
	if (stat(file.c_str(), &results) == 0)
		size = results.st_size;
	else
		return 1;
	
	sprintf(buff, "%d", size);
	ssize = buff;
	
	
	filename = file.substr(file.rfind("/")+1);
	
	GtkTreeIter iter;
	
	gtk_list_store_append (file_store, &iter);  /* Acquire a top-level iterator */
	gtk_list_store_set (file_store, &iter, 0, filename.c_str(),
											1, "--",
											2, "Negotiating" , 
											3, sid.c_str(), -1);
	gtk_widget_show(filewindow);
	gtk_window_present (GTK_WINDOW(filewindow));
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
	file_tag.AddAttr("size", ssize);
	file_tag.AddTag("range");
	filetag = new WokXMLTag (file_tag);
	
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
	data->AddAttr("file", file);
	data->AddAttr("strsize", PrettySize(size));
	sessions[iqtag.GetAttr("id")] = data;
	
	EXP_SIGHOOK("Jabber XML IQ ID "  + iqtag.GetAttr("id"), &jep96::SendReply, 500);
	
	return 1;
}

int
jep96::Finnished(WokXMLTag *fintag)
{
	GtkTreeIter iter;
	if( gtk_tree_model_get_iter(GTK_TREE_MODEL(file_store), &iter, gtk_tree_row_reference_get_path(rows[fintag->GetAttr("sid")])))
	{
		gtk_list_store_set (file_store, &iter, 2, "Finnished" , -1);
	}


	return true;
}

int
jep96::Accepted(WokXMLTag *acctag)
{
	GtkTreeIter iter;
	GtkTreePath* path;
	
	if( ( path = gtk_tree_row_reference_get_path(rows[acctag->GetAttr("sid")]) ) != NULL )
	{
		if( gtk_tree_model_get_iter(GTK_TREE_MODEL(file_store), &iter, path ))
		{
			gtk_list_store_set (file_store, &iter, 2, "Accepted" , -1);
		}
	}

	return true;
}

int
jep96::Terminated(WokXMLTag *termtag)
{
	GtkTreeIter iter;
	if( gtk_tree_model_get_iter(GTK_TREE_MODEL(file_store), &iter, gtk_tree_row_reference_get_path(rows[termtag->GetAttr("sid")])))
	{
		gtk_list_store_set (file_store, &iter, 2, "Connected" , -1);
	}

	return true;
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
		divider = 1024 * 10245;
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
	msg << size/divider << ending;
	
	return msg.str();
}

int
jep96::Position(WokXMLTag *postag)
{
	std::string sid = postag->GetAttr("sid");
	
	std::stringstream msg;
	msg << PrettySize(atol(postag->GetAttr("pos").c_str()));// << sessions[sid]->GetAttr("strsize");
	if ( sessions.find(sid) != sessions.end() )
		msg << "/" << sessions[sid]->GetAttr("strsize");
	
	GtkTreeIter iter;
	if( gtk_tree_model_get_iter(GTK_TREE_MODEL(file_store), &iter, gtk_tree_row_reference_get_path(rows[sid])))
	{
		gtk_list_store_set (file_store, &iter, 1, msg.str().c_str() , -1);
	}

	return true;
}

int
jep96::Connected(WokXMLTag *contag)
{
	GtkTreeIter iter;
	if( gtk_tree_model_get_iter(GTK_TREE_MODEL(file_store), &iter, gtk_tree_row_reference_get_path(rows[contag->GetAttr("sid")])))
	{
		gtk_list_store_set (file_store, &iter, 2, "Connected" , -1);
	}

	return true;
}

int
jep96::Rejected(WokXMLTag *rejtag)
{
	GtkTreeIter iter;
	if( gtk_tree_model_get_iter(GTK_TREE_MODEL(file_store), &iter, gtk_tree_row_reference_get_path(rows[rejtag->GetAttr("sid")])))
	{
		gtk_list_store_set (file_store, &iter, 2, "Rejected" , -1);
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
			filesend.AddTag(filetag);
			filesend.AddAttr("file", sessions[id]->GetAttr("file"));
			filesend.AddAttr("to", sessions[id]->GetFirstTag("iq").GetAttr("to"));
			filesend.AddAttr("sid", sid);
			filesend.AddAttr("session", msgtag->GetAttr("session"));
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


