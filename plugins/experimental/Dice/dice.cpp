/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2008 <nedo80@gmail.com>
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

#include "dice.hpp"
#include <sstream>
#include <fstream>
#include <vector>

static void
set_cell_color (GtkCellLayout   *cell_layout,
		GtkCellRenderer *cell,
		GtkTreeModel    *tree_model,
		GtkTreeIter     *iter,
		gpointer         data)
{
  GdkColor color;
  guint32 pixel = 0;
  GdkPixbuf *pixbuf;

  if (gdk_color_parse ("red", &color))
    pixel =
      (color.red   >> 8) << 24 |
      (color.green >> 8) << 16 |
      (color.blue  >> 8) << 8;

  pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, 24, 24);
  gdk_pixbuf_fill (pixbuf, pixel);

  g_object_set (cell, "pixbuf", pixbuf, NULL);

  g_object_unref (pixbuf);
}

Dice::Dice(WLSignal *wls): WoklibPlugin(wls)
{
	EXP_SIGHOOK("Get Main Menu", &Dice::MainMenu, 999);
	EXP_SIGHOOK("GUI Dice Open", &Dice::DiceWid, 999);
	EXP_SIGHOOK("Jabber XML Message xmlns http://wokjab.sf.net/dice", &Dice::Message, 1000);
	EXP_SIGHOOK("Dice Remove Session", &Dice::RemoveSession, 1000);
	
	gxml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/dice.glade", "dice_window", NULL);
	collection_store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
	
	menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(glade_xml_get_widget (gxml, "new_session")), menu);
	
	g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "dice_window")), "delete-event", G_CALLBACK (Dice::DeleteEvent), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "new_session")), "activate", G_CALLBACK (Dice::NewSessionMenu), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "collection_view")), "drag_data_get", G_CALLBACK (Dice::DragGet), this);
	
    enum
    {
      TARGET_STRING,
      TARGET_URL
    };
	static GtkTargetEntry target_entry[] =
    {      
	  { "STRING",        0, TARGET_STRING },
      { "text/plain",    0, TARGET_STRING },
    };
	
	gtk_icon_view_enable_model_drag_source(GTK_ICON_VIEW(glade_xml_get_widget (gxml, "collection_view")), (GdkModifierType) 0, target_entry, 1, (GdkDragAction) (GDK_ACTION_COPY));
	
	config = new WokXMLTag ("config");
	EXP_SIGHOOK("Config XML Change /dice", &Dice::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/dice");
	wls->SendSignal("Config XML Trigger", &conftag);
	
	
	GtkCellRenderer *renderer;
	
	
	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (glade_xml_get_widget (gxml, "collection_view")), renderer, TRUE);
	gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (glade_xml_get_widget (gxml, "collection_view")), renderer, set_cell_color, NULL, NULL);
	
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 0.5, "yalign", 0.5, NULL);
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (glade_xml_get_widget (gxml, "collection_view")), renderer, TRUE);
	gtk_icon_view_set_model(GTK_ICON_VIEW(glade_xml_get_widget (gxml, "collection_view")), GTK_TREE_MODEL(collection_store));
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (glade_xml_get_widget (gxml, "collection_view")), renderer, "text", 0, NULL);
	
}

Dice::~Dice()
{
	g_object_unref(gxml);
}

gboolean
Dice::DeleteEvent( GtkWidget *widget, GdkEvent *event, Dice *c)
{
	//c->SaveConfig();
	gtk_widget_hide(glade_xml_get_widget (c->gxml, "dice_window"));
	return TRUE;
}

void
Dice::DragGet(GtkWidget *wgt, GdkDragContext *context, GtkSelectionData *selection, guint info, guint time, Dice *c)
{
	GtkTreeIter iter;
	
	std::vector <std::string> uris;
	WokXMLTag dice("dice");
	GList *list = gtk_icon_view_get_selected_items (GTK_ICON_VIEW(glade_xml_get_widget(c->gxml, "collection_view")));
	for(;list; list = list->next)
	{
		WokXMLTag *tag;
		gtk_tree_model_get_iter (GTK_TREE_MODEL(c->collection_store), &iter, (GtkTreePath *) list->data);
		gtk_tree_model_get(GTK_TREE_MODEL(c->collection_store), &iter, 2, &tag, -1);
		
		dice.AddTag(tag);
	}
	
	g_list_foreach (list, ( void(*)(void*, void*)) gtk_tree_path_free, NULL);
	g_list_free (list);
	
	std::string data;
	data = dice.GetStr();
	gtk_selection_data_set_text(selection, data.c_str(), data.size());
}

int
Dice::RemoveSession(WokXMLTag *tag)
{
	if ( session_dice.find(tag->GetAttr("session")) != session_dice.end() )
	{
		if ( session_dice[tag->GetAttr("session")].find(tag->GetAttr("roomjid")) != session_dice[tag->GetAttr("session")].end() )
		{
			delete session_dice[tag->GetAttr("session")][tag->GetAttr("roomjid")];
			session_dice[tag->GetAttr("session")].erase(tag->GetAttr("roomjid"));
			if ( session_dice[tag->GetAttr("session")].empty() )
				session_dice.erase(tag->GetAttr("session"));
		}
	}
	
	
	return 1;
}

void
Dice::NewSession(GtkMenuItem *menuitem, Dice *c)
{
	
	std::string roomjid = *static_cast <std::string*>(g_object_get_data(G_OBJECT(menuitem), "roomjid"));
	std::string session = *static_cast <std::string*>(g_object_get_data(G_OBJECT(menuitem), "session"));

	c->session_dice[session][roomjid] = new DiceSession(c->wls, session, roomjid, c->gxml);
}

void
Dice::MenuItemDestroy(GtkObject *object, Dice *c)
{
	delete(static_cast <std::string*>(g_object_get_data(G_OBJECT(object), "roomjid")));	
	delete(static_cast <std::string*>(g_object_get_data(G_OBJECT(object), "session")));	
}

void
Dice::NewSessionMenu(GtkMenuItem *menuitem, Dice *c)
{
	
	WokXMLTag groupchat("groupchat");
	c->wls->SendSignal("Jabber GroupChat GetRooms", groupchat);
	
	while(GTK_MENU_SHELL(c->menu)->children != NULL )
		gtk_widget_destroy(GTK_WIDGET(GTK_MENU_SHELL(c->menu)->children->data));
	
	std::list <WokXMLTag *>::iterator tagiter;
	for ( tagiter = groupchat.GetTagList("item").begin() ; tagiter != groupchat.GetTagList("item").end(); tagiter++)
	{
		GtkWidget *room_menu_item = gtk_menu_item_new_with_label(((*tagiter)->GetAttr("session") + ":" + (*tagiter)->GetAttr("roomjid")).c_str());
	
		g_object_set_data (G_OBJECT(room_menu_item), "roomjid", new std::string ((*tagiter)->GetAttr("roomjid")));
		g_object_set_data (G_OBJECT(room_menu_item), "session", new std::string ((*tagiter)->GetAttr("session")));
		gtk_menu_shell_append (GTK_MENU_SHELL (c->menu), room_menu_item);
		gtk_widget_show (room_menu_item);
		g_signal_connect (G_OBJECT (room_menu_item), "activate", G_CALLBACK (Dice::NewSession), c);
		g_signal_connect (G_OBJECT (room_menu_item), "destroy", G_CALLBACK (Dice::MenuItemDestroy), c);
	}
	
}

void
Dice::AddToCollection(WokXMLTag &xml)
{
	// tar -xjf foo.tar.bz2 -C bar/
	
	// Lazy
	
	GtkTreeIter iter;
	
	gtk_list_store_append (collection_store, &iter);
	
	gtk_list_store_set (collection_store, &iter, 0, (xml.GetFirstTag("name").GetBody() + " [" + xml.GetFirstTag("type", "common notation").GetBody() + "]").c_str() , 2 , new WokXMLTag (xml), -1);
}

int
Dice::Message(WokXMLTag *tag)
{		
	std::string session, roomjid;
	
	session = tag->GetAttr("session");
	roomjid = tag->GetFirstTag("message").GetAttr("from");
	roomjid = roomjid.substr(0, roomjid.find("/"));
	
	wls->SendSignal("Jabber Dice Message '" + XMLisize(session) + "' '" + XMLisize(roomjid) + "'",tag);
	
	return 1;
}

int
Dice::ReadConfig(WokXMLTag *tag)
{
	if ( config )
		delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));



	std::list <WokXMLTag *>::iterator die;

	gtk_list_store_clear(GTK_LIST_STORE(collection_store));
	for( die = config->GetTagList("die").begin() ; die != config->GetTagList("die").end() ; die++)
	{
		AddToCollection(**die);
	}
	
	
	
	/// Debug stuff...
	
	for( int x = 10 ; x ; x--)
	{
		std::stringstream d_type;
		d_type << "d" << 2*x;
		WokXMLTag die("die");
		die.AddTag("name").AddText("Dragon Dice");
		die.AddTag("type", "common notation").AddText(d_type.str());

		AddToCollection(die);
	}
	return 1;
}

int
Dice::MainMenu(WokXMLTag *tag)
{
	WokXMLTag &item = tag->AddTag("item");
	item.AddAttr("name", "Dice Manager");
	item.AddAttr("signal", "GUI Dice Open");
	
	return 1;
}

int
Dice::DiceWid(WokXMLTag *tag)
{
	CreateWid();
	
	return 1;
}

void
Dice::CreateWid()
{
	gtk_widget_show_all(glade_xml_get_widget (gxml, "dice_window"));
	gtk_window_present(GTK_WINDOW(glade_xml_get_widget (gxml, "dice_window")));
}


