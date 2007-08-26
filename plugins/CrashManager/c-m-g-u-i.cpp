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

#include <dirent.h>

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
	
	DIR             *dip;
	struct dirent   *dit;
    
	std::string filename = std::string(g_get_home_dir()) + "/.wokjab";

	if ((dip = opendir(filename.c_str())) == NULL)
	{
    	perror("opendir");
		return;
	}

	while ((dit = readdir(dip)) != NULL)
	{
		std::string file = dit->d_name;

		if ( file.substr(file.size() - 4 ) == ".log" )
			std::cout << "file:" << file << std::endl;
	}

	if (closedir(dip) == -1)
	{
		perror("closedir");
		return;
	}
}

CMGUI::~CMGUI()
{
	
	
	
}

int
CMGUI::ReadConfig(WokXMLTag *tag)
{
	delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));
	return 1;
}
