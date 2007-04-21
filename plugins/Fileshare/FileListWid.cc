/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <nedo80@gmail.com>
 ****************************************************************************/
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#include "FileListWid.h"
#include "File.h"
#include "Download.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sstream>

#include "zlib.h"

FileListWid::FileListWid(WLSignal *wls, WokXMLTag *tag, WokXMLTag *config) : WLSignalInstance(wls),
config ( new WokXMLTag (*config))
{
	sid = tag->GetFirstTag("iq").GetFirstTag("si").GetAttr("id");
	filelist = new WokXMLTag (NULL, "filelist");
	session = tag->GetAttr("session");
	jid = tag->GetFirstTag("iq").GetAttr("from");
	
	xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/FileListWid.glade", NULL, NULL);
	gtk_label_set_text(GTK_LABEL(glade_xml_get_widget(xml,"jid")), tag->GetFirstTag("iq").GetAttr("to").c_str());
	gtk_window_set_title(GTK_WINDOW(glade_xml_get_widget(xml,"window")), (tag->GetFirstTag("iq").GetAttr("to") + " filelist").c_str());
	
	GtkCellRenderer *renderer;
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (glade_xml_get_widget(xml,"folder")),
        -1, "Name", renderer, "text", 0, NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (glade_xml_get_widget(xml,"folder")),
        -1, "Size", renderer, "text", 1, NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (glade_xml_get_widget(xml,"folder")),
        -1, "ID", renderer, "text", 2, NULL);
								
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (glade_xml_get_widget(xml,"tree")),
        -1, "Name", renderer, "text", 0, NULL);
	
	path_store = gtk_tree_store_new (2, G_TYPE_STRING, G_TYPE_INT);
	folder_store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	
	gtk_tree_view_set_model (GTK_TREE_VIEW (glade_xml_get_widget(xml,"tree")), GTK_TREE_MODEL(path_store));
	gtk_tree_view_set_model (GTK_TREE_VIEW (glade_xml_get_widget(xml,"folder")), GTK_TREE_MODEL(folder_store));
	
	g_signal_connect (G_OBJECT (glade_xml_get_widget(xml,"tree")), "cursor-changed",
					G_CALLBACK (FileListWid::OpenFolder), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget(xml,"download")), "clicked",
					G_CALLBACK (FileListWid::Download), this);
					
	EXP_SIGHOOK("Jabber Stream RequestAuthorisation", &FileListWid::Auth, 1000);
	EXP_SIGHOOK("Jabber Stream File Status Finished", &FileListWid::Finished, 1000);
}


FileListWid::~FileListWid()
{
	if(xml)
	{
		gtk_widget_destroy( glade_xml_get_widget (xml, "window") );
		g_object_unref(xml);
		xml = NULL;
	}

	delete config;
	delete filelist;
}

void
FileListWid::Download(GtkButton *button, FileListWid *c) 
{
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW (glade_xml_get_widget(c->xml,"folder")));

	if(gtk_tree_selection_get_selected(selection,  NULL, &iter) != FALSE)
	{
		gchar *id;
		gchar *name;
		gchar *size;
		gtk_tree_model_get(GTK_TREE_MODEL(c->folder_store), &iter, 0, &name, 1, &size, 2, &id, -1);

		if ( !strcmp(size, "<folder>" ) )
		{
			GtkTreeSelection *selection;
			GtkTreeIter iter;
			
			selection = gtk_tree_view_get_selection(GTK_TREE_VIEW (glade_xml_get_widget(c->xml,"tree")));

			if(gtk_tree_selection_get_selected(selection,  NULL, &iter) != FALSE)
			{
				WokXMLTag *tag;
				gtk_tree_model_get(GTK_TREE_MODEL(c->path_store), &iter, 1, &tag, -1);
				
				std::list <WokXMLTag *>::iterator titer;
				
				for( titer = tag->GetTagList("item").begin(); titer != tag->GetTagList("item").end() ; titer++)
				{
					if ( (*titer)->GetAttr("name").substr((*titer)->GetAttr("name").rfind("/")+1) == name )
					{
						new DownloadFolder(c->wls, *titer, c->jid, c->session, c->config->GetFirstTag("download_path").GetAttr("data"));
					}
				}
			}
		}
		else
		{
			if ( strlen(id) == 0 ) 
				return;
		
			WokXMLTag msg(NULL, "message");
			msg.AddAttr("session", c->session);
			WokXMLTag &iq = msg.AddTag("iq");
			iq.AddAttr("to", c->jid);
			iq.AddAttr("type", "get");
			WokXMLTag &fileshare = iq.AddTag("fileshare");
			fileshare.AddAttr("xmlns", "http://sf.wokjab.net/fileshare");
			WokXMLTag &file = fileshare.AddTag("file");
			file.AddAttr("id", id);
			
			c->wls->SendSignal("Jabber XML IQ Send", msg);
			
			new File(c->wls, msg, name, c->config->GetFirstTag("download_path").GetAttr("data"));
		}
		g_free(id);
		g_free(name);
		g_free(size);
	}
}

void
FileListWid::OpenFolder (GtkTreeView *tree_view, FileListWid *c)
{
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW (glade_xml_get_widget(c->xml,"tree")));

	if(gtk_tree_selection_get_selected(selection,  NULL, &iter) != FALSE)
	{
	
		WokXMLTag *tag;
		gtk_tree_model_get(GTK_TREE_MODEL(c->path_store), &iter, 1, &tag, -1);
		
		gtk_list_store_clear(GTK_LIST_STORE(c->folder_store));
		std::list <WokXMLTag *>::iterator iter;
		for( iter = tag->GetTagList("item").begin() ; iter != tag->GetTagList("item").end() ; iter++)
		{
			std::string size;
			if( (*iter)->GetAttr("type") == "folder")
				size = "<folder>";
			else
				size = (*iter)->GetAttr("size");
				
			GtkTreeIter titer;
			
			gtk_list_store_append (c->folder_store, &titer);
			gtk_list_store_set (c->folder_store, &titer, 
										0, (*iter)->GetAttr("name").c_str(), 
										1, size.c_str(), 
										2, (*iter)->GetAttr("id").c_str(), -1 );
		}
	}
}
																																																								
																																																								
void
FileListWid::PopulateTree(WokXMLTag *tag, GtkTreeIter *piter)
{
	if ( tag->GetAttr("type") != "folder" )
		return;
	
	GtkTreeIter titer;
	
	gtk_tree_store_append (path_store, &titer, piter);
	gtk_tree_store_set (path_store, &titer, 0, tag->GetAttr("name").c_str(), 1, tag, -1 );
	if ( tag->GetAttr("type") == "folder") 
	{
		std::list <WokXMLTag *>::iterator iter;
		for( iter = tag->GetTagList("item").begin() ; iter != tag->GetTagList("item").end() ; iter++)
		{
			PopulateTree(*iter, &titer);
		}
	}
}

int
FileListWid::Finished(WokXMLTag *tag)
{
	if ( tag->GetAttr("sid") != lsid )
		return 1;
	
 int err;
	
	char buf[2000];
		
	gzFile file;
	file = gzopen ("/tmp/filelist.xml.gz", "rb");
	while ( ( err = gzread (file, buf, 2000) ) > 0 )
	{
		buf[err] = 0;
		filelist->Add(buf);
	}
	gzclose(file);
	
	if ( err == -1 )
	{
		woklib_error(wls, "Somekinda problem reading the filelist");
		delete this;
		return 1;
	}	
	
	GtkTreeIter titer;
	
	gtk_tree_store_append (path_store, &titer, NULL);
	gtk_tree_store_set (path_store, &titer, 0, "/", 1, &filelist->GetFirstTag("filelist"), -1 );
	std::list <WokXMLTag *>::iterator iter;
	for( iter = filelist->GetFirstTag("filelist").GetTagList("item").begin() ; iter != filelist->GetFirstTag("filelist").GetTagList("item").end() ; iter++)
	{
		PopulateTree(*iter, &titer);
	}
	
	return 1;
}

int
FileListWid::Auth(WokXMLTag *tag)
{
	std::cout << "2XML: " << *tag << std::endl;
	if ( tag->GetAttr("sid") == sid )
	{
		lsid = "local:" + sid;
		WokXMLTag &file = tag->AddTag("file");
		file.AddAttr("lsid", lsid);
		file.AddAttr("name", "/tmp/filelist.xml.gz");	
	}
	/*
	if(  len(xml.GetTagList("file")) == 0 ):
		print "no file tag"
		global id
		id += 1
		
		file = xml.AddTagName("file")
		file.AddAttr("lsid", "jep96.autoreciver-" + str(id))
		file.AddAttr("name", "/home/nedo/test.mp3")
	*/

	return 1;
}

