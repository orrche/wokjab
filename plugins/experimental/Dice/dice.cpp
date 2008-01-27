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
      { "text/uri-list", 0, TARGET_URL },
    };
	
	gtk_icon_view_enable_model_drag_source(GTK_ICON_VIEW(glade_xml_get_widget (gxml, "collection_view")),
                                                         (GdkModifierType) 0,
                                                         target_entry,
                                                         3,
                                                         (GdkDragAction) (GDK_ACTION_COPY));
	
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
	std::cout << "TFT" << std::endl;
//	gtk_selection_data_set_text         (selection, "unknown shit", -1);
	gtk_selection_data_set              (selection, GDK_TARGET_STRING, 8, (const guchar*)"unknown shit123", 12);
}

void
Dice::DataReceived(GtkWidget *wgt, GdkDragContext *context, int x, int y,
                        GtkSelectionData *seldata, guint info, guint time, Dice *c)
{
	std::cout << "Beawear" << std::endl;
	
	
}

void
Dice::NewSession(GtkMenuItem *menuitem, Dice *c)
{
	std::string roomjid = *static_cast <std::string*>(g_object_get_data(G_OBJECT(menuitem), "roomjid"));
	std::string session = *static_cast <std::string*>(g_object_get_data(G_OBJECT(menuitem), "session"));
		
	
	c->session_gxml[session][roomjid] = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/dice.glade", "mainbox", NULL);
	gtk_notebook_append_page(GTK_NOTEBOOK(glade_xml_get_widget (c->gxml, "notebook")), glade_xml_get_widget (c->session_gxml[session][roomjid], "mainbox"), gtk_label_new((session + ":" + roomjid).c_str()));
	
	GtkListStore *rollstore = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
	gtk_icon_view_set_model(GTK_ICON_VIEW(glade_xml_get_widget (c->session_gxml[session][roomjid], "current_roll")), GTK_TREE_MODEL(rollstore));
	g_object_unref(rollstore);

    enum
    {
      TARGET_STRING,
      TARGET_URL
    };
	static GtkTargetEntry target_entry[] =
    {
      { "STRING",        0, TARGET_STRING },
      { "text/plain",    0, TARGET_STRING },
      { "text/uri-list", 0, TARGET_URL },
    };
	gtk_drag_dest_set(glade_xml_get_widget (c->session_gxml[session][roomjid], "current_roll"), GTK_DEST_DEFAULT_ALL, target_entry, 3, (GdkDragAction) (GDK_ACTION_COPY|GDK_ACTION_MOVE|GDK_ACTION_LINK));
	//gtk_icon_view_enable_model_drag_dest(GTK_ICON_VIEW(glade_xml_get_widget (c->session_gxml[session][roomjid], "current_roll")), target_entry, 3, (GdkDragAction) (GDK_ACTION_COPY));
	g_signal_connect(glade_xml_get_widget (c->session_gxml[session][roomjid], "current_roll"), "drag_data_received", G_CALLBACK(Dice::DataReceived), c);
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

int
Dice::ReadConfig(WokXMLTag *tag)
{
	GtkTreeIter iter;
	if ( config )
		delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));



	std::list <WokXMLTag *>::iterator die;

	gtk_list_store_clear(GTK_LIST_STORE(collection_store));
	for( die = config->GetTagList("die").begin() ; die != config->GetTagList("die").end() ; die++)
	{
		gtk_list_store_append (collection_store, &iter);
		gtk_list_store_set (collection_store, &iter, 0, (*die)->GetAttr("name").c_str() , 3 , new WokXMLTag(**die), -1);
	}
	
	
	
	/// Debug stuff...
	gtk_list_store_append (collection_store, &iter);
	gtk_list_store_set (collection_store, &iter, 0, "donkey" , -1);
	gtk_list_store_append (collection_store, &iter);
	gtk_list_store_set (collection_store, &iter, 0, "wesnoth" ,-1);

	
	for( int x = 10 ; x ; x--)
	{
		std::stringstream str;
		
		str << "Dice " <<  x;
		gtk_list_store_append (collection_store, &iter);
		gtk_list_store_set (collection_store, &iter, 0, str.str().c_str() ,-1);
		
		
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


