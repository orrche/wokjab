/***************************************************************************
 *  Copyright (C) 2005  Kent Gustavsson <oden@gmx.net>
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


//
// Class: GtkPreference
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sun Jun 12 21:38:32 2005
//

#include "GtkPreference.h"
#include "GtkPString.h"
#include "GtkPText.h"
#include "GtkPBool.h"
#include "GtkPPassword.h"

#include <sstream>

GtkPreference::GtkPreference(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("GUI Preference", &GtkPreference::SigCreateWid, 1000);
	EXP_SIGHOOK("GetMenu", &GtkPreference::SigMenu , 1000);
	
	config = new WokXMLTag(NULL, "NULL");

	window = NULL;
	config = NULL;
}


GtkPreference::~GtkPreference()
{
	CleanWidgetList();
	
	delete config;
	config = NULL;
	
	if ( window ) 
	{
		gtk_widget_destroy(window);	
	}
}


void
GtkPreference::SaveConfig()
{	
	
	int width, height, pos_x, pos_y;
	std::stringstream sheight, swidth, spos_x, spos_y;

	gtk_window_get_size(GTK_WINDOW(window), &width, &height);
	gtk_window_get_position(GTK_WINDOW(window), &pos_x, &pos_y);
	sheight << height;
	swidth << width;
	spos_x << pos_x;
	spos_y << pos_y;

	config->GetFirstTag("width").AddAttr("data", swidth.str());
	config->GetFirstTag("height").AddAttr("data", sheight.str());
	config->GetFirstTag("pos_x").AddAttr("data", spos_x.str());
	config->GetFirstTag("pos_y").AddAttr("data", spos_y.str());

	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/preference");
	conftag.AddTag(config);

	EXP_SIGUNHOOK("Config XML Change /preference", &GtkPreference::ReadConfig, 500);
	wls->SendSignal("Config XML Store", &conftag);
	EXP_SIGHOOK("Config XML Change /preference", &GtkPreference::ReadConfig, 500);
	
}

int 
GtkPreference::ReadConfig(WokXMLTag *tag)
{
	if( tag )
	{
		delete config;
		config = new WokXMLTag(tag->GetFirstTag("config"));
	}
		
	int pos_x, pos_y,width,height;

	width = atoi( config->GetFirstTag("width").GetAttr("data").c_str());
	height = atoi( config->GetFirstTag("height").GetAttr("data").c_str());
	pos_x = atoi( config->GetFirstTag("pos_x").GetAttr("data").c_str());
	pos_y = atoi( config->GetFirstTag("pos_y").GetAttr("data").c_str());
		
	gtk_window_move (GTK_WINDOW(window), pos_x, pos_y);
	
	if(width == 0)
		width = 120;

	if(height == 0)
		height = 500;
		
	gtk_window_set_default_size(GTK_WINDOW(window), width, height);
	
	return 1;
}

void
GtkPreference::Destroy(GtkWidget *widget, GtkPreference *c)
{
}

gboolean
GtkPreference::Delete( GtkWidget *widget, GdkEvent *event, GtkPreference *c)
{
	c->InClassDestroy();
	
	return FALSE;
	
}

void
GtkPreference::InClassDestroy()
{
	SaveConfig();
	EXP_SIGUNHOOK("Config XML Change /preference", &GtkPreference::ReadConfig, 500);
	
	window = NULL;
	CleanWidgetList();
	
	delete config;
	config = NULL;
}

void
GtkPreference::CleanWidgetList()
{
	std::list <GtkPCommon *>::iterator iter;
	
	for( iter = widgets.begin(); iter != widgets.end() ; iter++)
		delete (*iter);
	
	widgets.clear();
}

void
GtkPreference::LocalActivate(std::string strpath)
{
	EXP_SIGHOOK(std::string("Config XML Change ") + strpath, &GtkPreference::CreateWindow, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", strpath);
	conf_path = strpath;
	wls->SendSignal("Config XML Trigger", &conftag);
	EXP_SIGUNHOOK(std::string("Config XML Change ") + strpath, &GtkPreference::CreateWindow, 500);
}

void
GtkPreference::SaveButton (GtkWidget * widget, GtkPreference *c)
{
	std::list <GtkPCommon *>::iterator iter;
	
	for( iter = c->widgets.begin(); iter != c->widgets.end() ; iter++)
		(*iter)->Save();
		
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", c->conf_path);
	conftag.AddTag(c->config);
	c->wls->SendSignal("Config XML Store", &conftag);
	c->wls->SendSignal("Config XML Save", &conftag);
}

gboolean
GtkPreference::Activate(GtkTreeView *tree_view, GdkEventButton *event, GtkPreference *c)
{
	GtkTreePath      *path;

	if (event->button == 1 && 
		gtk_tree_view_get_path_at_pos (tree_view, (int)event->x, (int)event->y, &path, NULL, NULL, NULL)) 
	{
		GtkTreeIter iter;
		gchar *strpath;
		
		gtk_tree_model_get_iter (GTK_TREE_MODEL(c->treestore), &iter, path);

		gtk_tree_model_get(GTK_TREE_MODEL(c->treestore), &iter, 1, &strpath, -1);
		
		if ( !strpath )
			return false;
			
		c->LocalActivate ( strpath );

	}
	return false;	
}

bool
GtkPreference::FindSanity(WokXMLTag &tag)
{
	std::list <WokXMLTag *>::iterator searcher;
	for ( searcher = tag.GetTags().begin() ; searcher != tag.GetTags().end() ; searcher++)
	{
		if (! (*searcher)->GetAttr("type").empty() )
			return true;
		else if ( FindSanity(**searcher) )
			return true;
	}
	
	return false;
}

int
GtkPreference::AddTreeItem(GtkTreeIter *parant, WokXMLTag &tag, std::string path)
{
	std::list <WokXMLTag*>::iterator tagiter;
	int m = 0;
	
	for( tagiter = tag.GetTags().begin(); tagiter != tag.GetTags().end(); tagiter++)
	{
		if((*tagiter)->GetName() != "config")
		{
			GtkTreeIter iter;
			
			gtk_tree_store_append (treestore, &iter, parant);
			gtk_tree_store_set (treestore, &iter,
						1, (path + "/" + (*tagiter)->GetName()).c_str(),
				    0, (*tagiter)->GetName().c_str(),
				    -1);
						
			if ( AddTreeItem(&iter, **tagiter, path + "/" + (*tagiter)->GetName()) == 0 )
				gtk_tree_store_remove(treestore, &iter);
			else
			{
				m++;
				
			}
		}
		else
		{
			if ( FindSanity(**tagiter) )
				m++;
		}
	}
	
	return m;
}

void
GtkPreference::CreateWid()
{
	GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
	
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW(window), "Preference");
																						 
	g_signal_connect ((gpointer) window, "destroy",
                    G_CALLBACK (GtkPreference::Destroy),
                    this);
	g_signal_connect ((gpointer) window, "delete_event",
					G_CALLBACK (GtkPreference::Delete), 
					this);
	
	GtkTreeIter iter;
	treeview = gtk_tree_view_new();
	treestore = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(treestore));
	GtkCellRenderer *renderer;
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview), -1,
							     "path",
							     renderer, "text",
							     0, 
							      NULL);
	

	gtk_tree_store_append (treestore, &iter, NULL);
	gtk_tree_store_set (treestore, &iter,
				    0, "/",
				    -1);

	
	GtkWidget *scrolltree = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add(GTK_CONTAINER(scrolltree), treeview);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolltree), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolltree), GTK_SHADOW_IN);

	configview = gtk_viewport_new (NULL, NULL);
	GtkWidget *scrollconf = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add(GTK_CONTAINER(scrollconf), configview);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollconf), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollconf), GTK_SHADOW_IN);
	
	GtkWidget *main_hpaned;
	main_hpaned = gtk_hpaned_new();
	gtk_paned_add1(GTK_PANED(main_hpaned), scrolltree);
	gtk_paned_add2(GTK_PANED(main_hpaned), scrollconf);
	gtk_paned_set_position (GTK_PANED(main_hpaned), 100);
	
	GtkWidget *savebutton;
	savebutton = gtk_button_new_with_mnemonic("_Save");
	GtkWidget *hbox;
	hbox = gtk_hbox_new(FALSE, 2);
	gtk_box_pack_end(GTK_BOX(hbox), savebutton, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), main_hpaned, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	g_signal_connect (G_OBJECT (savebutton), "clicked", 
				G_CALLBACK (GtkPreference::SaveButton), this);
	g_signal_connect (G_OBJECT (treeview), "button_press_event", 
			  G_CALLBACK (GtkPreference::Activate), this);
	
	WokXMLTag pathtag(NULL, "configpath");
	wls->SendSignal("Config XML GetTree", &pathtag);
	
	AddTreeItem(&iter, pathtag.GetFirstTag("config"), "");
	gtk_tree_view_expand_all(GTK_TREE_VIEW(treeview));
	
	EXP_SIGHOOK("Config XML Change /preference", &GtkPreference::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/preference");
	wls->SendSignal("Config XML Trigger", &conftag);	
	
	
	gtk_widget_show_all(window);
	
}

int
GtkPreference::CreateConfig(GtkWidget *parant, WokXMLTag *tag)
{
	int m = 0;
	GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
	
	std::list <WokXMLTag *>::iterator iter;
	std::list <WokXMLTag *> *list;
		
	list = &tag->GetTags();
	for( iter = list->begin() ; iter != list->end() ; iter++)
	{
		if((*iter)->GetAttr("type") == "string")
		{
			GtkPString *str = new GtkPString((*iter));
			gtk_box_pack_start(GTK_BOX(vbox), str->GetWidget(), FALSE, FALSE, 2);
			widgets.push_back(str);
			m++;
		}
		else if((*iter)->GetAttr("type") == "password")
		{
			GtkPPassword *pwd = new GtkPPassword((*iter));
			gtk_box_pack_start(GTK_BOX(vbox), pwd->GetWidget(), FALSE, FALSE, 2);
			widgets.push_back(pwd);
			m++;
		}
		else if((*iter)->GetAttr("type") == "text")
		{
			GtkPText *txt = new GtkPText((*iter));
			gtk_box_pack_start(GTK_BOX(vbox), txt->GetWidget(), FALSE, FALSE, 2);
			widgets.push_back(txt);
			m++;
		}
		else if((*iter)->GetAttr("type") == "bool")
		{
			GtkPBool *b = new GtkPBool((*iter));
			gtk_box_pack_start(GTK_BOX(vbox), b->GetWidget(), FALSE, FALSE, 2);
			widgets.push_back(b);
			m++;
		}
		
		if( (*iter)->GetTags().size() )
		{
			GtkWidget *frame = gtk_frame_new((*iter)->GetName().c_str());
			int n = CreateConfig(frame, (*iter));
			if ( n ) 
			{
				m += n;
				gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 2);
			}
			else
				gtk_widget_destroy(frame);
				
			
		}
	}
	
	gtk_container_add(GTK_CONTAINER(parant), vbox);
	return m;
}

int
GtkPreference::CreateWindow(WokXMLTag *tag)
{
	
	if(gtk_bin_get_child(GTK_BIN(configview)))
		gtk_widget_destroy(gtk_bin_get_child(GTK_BIN(configview)));
	
	CleanWidgetList();
	if(config)
		delete config;
	
	config = new WokXMLTag (tag->GetFirstTag("config"));
	CreateConfig(configview, config);
	
	gtk_widget_show_all(configview);
	return true;
}

int
GtkPreference::SigCreateWid(WokXMLTag *tag)
{
	if( !window )
		CreateWid();
	
	gtk_window_present (GTK_WINDOW(window));
	
	return 1;
}

int
GtkPreference::SigMenu(WokXMLTag *tag)
{
	WokXMLTag *menu_item;
	
	menu_item = &tag->AddTag("item");
	menu_item->AddAttr("name", "Preference");
	menu_item->AddAttr("signal", "GUI Preference");
	
	return 1;
}


