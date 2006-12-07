/***************************************************************************
 *  Copyright (C) 2005-2006  Kent Gustavsson <nedo80@gmail.com>
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
// Class: GtkMenu
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Aug  9 14:20:55 2005
//

#include "gtkmenu.h"


GtkJabberMenu::GtkJabberMenu(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber GUI JIDMenu", &GtkJabberMenu::Create, 10);
}


GtkJabberMenu::~GtkJabberMenu()
{

}

gboolean
GtkJabberMenu::MenuActivate(GtkMenuItem *menuitem,GtkJabberMenu *c)
{
	g_assert( c->MenuSignals.find(GTK_WIDGET(menuitem)) != c->MenuSignals.end());
	if ( c->MenuSignals[GTK_WIDGET(menuitem)] != "" )
		c->wls->SendSignal(c->MenuSignals[GTK_WIDGET(menuitem)], c->MenuData[GTK_WIDGET(menuitem)]);
	
	c->CleanDataStore();
	
	return true;
}

void
GtkJabberMenu::AddItem(GtkWidget *menu, WokXMLTag *tag, WokXMLTag *olddata)
{
	GtkWidget *menu_item;
	
	std::list<WokXMLTag *> list = tag->GetTagList("item");
	std::list<WokXMLTag *>::iterator listiter;
	for( listiter = list.begin() ; listiter != list.end() ; listiter++)
	{
		if((*listiter)->GetAttr("icon").size())
		{
			menu_item = gtk_image_menu_item_new_with_mnemonic((*listiter)->GetAttr("name").c_str());
			GtkWidget *image = gtk_image_new_from_file((*listiter)->GetAttr("icon").c_str());
			gtk_image_menu_item_set_image( GTK_IMAGE_MENU_ITEM(menu_item), image);
		}
		else
			menu_item = gtk_menu_item_new_with_mnemonic ((*listiter)->GetAttr("name").c_str());
		gtk_widget_show (menu_item);
  	WokXMLTag *curdata;
		if(!(*listiter)->GetTagList("data").empty())
		{
			curdata = new WokXMLTag ((*listiter)->GetFirstTag("data"));
			DataStore.push_back(curdata);
		}
		else
			curdata = olddata;
		
		MenuData[menu_item] = curdata;
		
		gtk_container_add (GTK_CONTAINER (menu), menu_item);
		if ( (*listiter)->GetTagList("item").size() == 0 )
		{
			gtk_signal_connect (GTK_OBJECT (menu_item), "activate",
											 GTK_SIGNAL_FUNC (GtkJabberMenu::MenuActivate),
											 this);
			
			MenuSignals[menu_item] = (*listiter)->GetAttr("signal");
		}
		else
		{
			GtkWidget *new_menu = gtk_menu_new();
			AddItem(new_menu, *listiter, curdata);
			gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), new_menu);
		}
	}
}

void
GtkJabberMenu::CleanDataStore()
{
	while ( !DataStore.empty() )
	{
		delete *DataStore.begin();
		DataStore.pop_front();
	}
	
	MenuSignals.clear();
	MenuData.clear();
}

int
GtkJabberMenu::Create(WokXMLTag *tag)
{
	GtkWidget *menuwid;
	WokXMLTag menu(NULL, "menu");
	
	if( DataStore.size() )
		CleanDataStore();
	
	DataStore.push_back(new WokXMLTag(tag->GetFirstTag("data")));
	
	menu.AddAttr("jid", tag->GetFirstTag("data").GetAttr("jid"));
	menu.AddAttr("session", tag->GetFirstTag("data").GetAttr("session"));
	
	std::list<WokXMLTag *> list = tag->GetTagList("item");
	std::list<WokXMLTag *>::iterator listiter;
	for( listiter = list.begin() ; listiter != list.end() ; listiter++)
		wls->SendSignal((*listiter)->GetAttr("signal"), &menu);

	menuwid = gtk_menu_new ();
	
	
	AddItem(menuwid, &menu, *DataStore.begin());
	
	gtk_menu_popup (GTK_MENU(menuwid), NULL, NULL, NULL, NULL,
        		atoi(tag->GetAttr("button").c_str()), atoi(tag->GetAttr("time").c_str()));
		
	
	return true;
}
