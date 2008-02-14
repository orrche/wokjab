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
// Class: GUIRoster
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Thu Aug 31 12:39:29 2006
//

#include "GUIRoster.h"
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sstream>

GUIRoster::GUIRoster(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("GUIRoster AddItem", &GUIRoster::AddItem, 500);
	EXP_SIGHOOK("GUIRoster UpdateItem", &GUIRoster::UpdateItem, 500);
	EXP_SIGHOOK("GUIRoster RemoveItem", &GUIRoster::RemoveItem, 500);
	EXP_SIGHOOK("GUI Window Init", &GUIRoster::GUIWindowInit, 500);
	inittag = NULL;
	hoverid = NULL;
	xml = NULL;
	CreateWid();
}


GUIRoster::~GUIRoster()
{
	if ( xml ) 
		g_object_unref(xml);
}

void
GUIRoster::CreateWid()
{
	xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/GUIRoster.glade", "mainbox", NULL);
	GtkWidget *main_vbox = glade_xml_get_widget (xml, "mainbox");

	GtkCellRenderer *renderer;
	GtkCellRenderer *renderer_pix;
	
	
	pre_pix_column = gtk_tree_view_column_new();
	
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(pre_pix_column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(pre_pix_column, renderer,
                                             "text", INDENT_COLUMN, NULL);
	
	
	renderer_pix = gtk_cell_renderer_pixbuf_new ();
	g_object_set(renderer_pix, "xalign", 0.0, "yalign", 0.0, "ypad", 0, "xpad", 0, NULL);
	gtk_tree_view_column_pack_start(pre_pix_column, renderer_pix, FALSE);
	gtk_tree_view_column_set_attributes(pre_pix_column, renderer_pix,
                                             "pixbuf", PRE_PIX_COLUMN, NULL);

	
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 0.0, "yalign", 0.0, "ypad", 0, NULL);
	gtk_tree_view_column_pack_start(pre_pix_column, renderer, TRUE);
	gtk_tree_view_column_add_attribute(pre_pix_column, renderer,
                                             "markup", TEXT_COLUMN);
																						 
	//gtk_tree_view_column_set_min_width(text_column, 1);
	
	renderer_pix = gtk_cell_renderer_pixbuf_new ();
	g_object_set(renderer_pix, "xalign", 1.0, "yalign", 0.0, "ypad", 0, NULL);
	gtk_tree_view_column_pack_end(pre_pix_column, renderer_pix, FALSE);
	gtk_tree_view_column_set_attributes(pre_pix_column, renderer_pix,
                                             "pixbuf", POST_PIX_COLUMN, NULL);

	
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(xml,"view_roster")),
				     GTK_TREE_VIEW_COLUMN (pre_pix_column));
	
/*
	pre_pix_column = gtk_tree_view_column_new();
	
	renderer_pix = gtk_cell_renderer_pixbuf_new ();
	g_object_set(renderer_pix, "xalign", 0.0, "yalign", 0.0, "ypad", 0, "xpad", 0, NULL);
	gtk_tree_view_column_pack_start(pre_pix_column, renderer_pix, FALSE);
	gtk_tree_view_column_set_attributes(pre_pix_column, renderer_pix,
                                             "pixbuf", PRE_PIX_COLUMN,
																						 NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(xml,"view_roster")),
				     GTK_TREE_VIEW_COLUMN (pre_pix_column));
	
	text_column = gtk_tree_view_column_new();
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 0.0, "yalign", 0.0, "ypad", 0, NULL);
	gtk_tree_view_column_pack_start(text_column, renderer, TRUE);
	gtk_tree_view_column_add_attribute(text_column, renderer,
                                             "markup", TEXT_COLUMN);
																						 
	gtk_tree_view_column_set_min_width(text_column, 1);
	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(xml,"view_roster")), GTK_TREE_VIEW_COLUMN (text_column));		 

	post_pix_column = gtk_tree_view_column_new();
	renderer_pix = gtk_cell_renderer_pixbuf_new ();
	g_object_set(renderer_pix, "xalign", 1.0, "yalign", 0.0, "ypad", 0, NULL);
	gtk_tree_view_column_pack_end(post_pix_column, renderer_pix, FALSE);
	gtk_tree_view_column_set_attributes(post_pix_column, renderer_pix,
                                             "pixbuf", POST_PIX_COLUMN,
																						 NULL);

	gtk_tree_view_append_column (GTK_TREE_VIEW (glade_xml_get_widget(xml,"view_roster")),
				     GTK_TREE_VIEW_COLUMN (post_pix_column));
	*/					 
						 
	GtkTreeStore *model = gtk_tree_store_new (NUM_COLUMNS, GDK_TYPE_PIXBUF,
				    G_TYPE_STRING, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING);
	
	
	gtk_tree_view_set_model (GTK_TREE_VIEW
				 (glade_xml_get_widget(xml,"view_roster")), GTK_TREE_MODEL (model));
	mainwindowplug = gtk_plug_new(0);
	
	gtk_container_add(GTK_CONTAINER(mainwindowplug), main_vbox);
	
	gtk_widget_show_all(mainwindowplug);
	
	
	g_signal_connect (G_OBJECT(glade_xml_get_widget(xml,"view_roster")), "row_activated", 
		  G_CALLBACK (GUIRoster::row_activated), this);
	g_signal_connect (G_OBJECT(glade_xml_get_widget(xml,"view_roster")), "button_press_event", 
		  G_CALLBACK (GUIRoster::popup_menu), this);
	g_signal_connect (G_OBJECT(glade_xml_get_widget(xml,"view_roster")), "motion-notify-event",
			G_CALLBACK (GUIRoster::MouseMotion), this);
	g_signal_connect (G_OBJECT(glade_xml_get_widget(xml,"view_roster")), "leave-notify-event",
			G_CALLBACK (GUIRoster::MouseLeave), this);
	g_signal_connect (G_OBJECT(glade_xml_get_widget(xml,"view_roster")), "size_allocate",
			G_CALLBACK (GUIRoster::SizeChange), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (xml, "view_roster")), "drag_data_get", 
			G_CALLBACK (GUIRoster::DragGet), this);
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
	
	gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(glade_xml_get_widget(xml,"view_roster")), (GdkModifierType) GDK_BUTTON1_MASK , target_entry, 2, (GdkDragAction) (GDK_ACTION_COPY));
	
	char buf[200];
	WokXMLTag contag(NULL, "connect");
	sprintf(buf, "%d", gtk_plug_get_id(GTK_PLUG(mainwindowplug)));
	WokXMLTag &widtag = contag.AddTag("widget");
	widtag.AddAttr("id", buf);
	widtag.AddAttr("expand", "true");
	widtag.AddAttr("fill", "true");
	
	if ( !wls->SendSignal("GUI Window AddWidget",&contag) )
	{
		inittag = new WokXMLTag (contag);
	}
	else
	{
		std::stringstream sig;
		sig << "GUI Window Close " << gtk_plug_get_id(GTK_PLUG(mainwindowplug));
		EXP_SIGHOOK(sig.str(), &GUIRoster::Close, 500);
	}    
	

	
	
}

void
GUIRoster::DragGet(GtkWidget *wgt, GdkDragContext *context, GtkSelectionData *selection, guint info, guint time, GUIRoster *c)
{
	GtkTreeIter iter;
	
	WokXMLTag data("dnd", "dnd");
	GtkTreeModel *model;
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(glade_xml_get_widget(c->xml, "view_roster")));
	
	GtkTreeSelection* select =  gtk_tree_view_get_selection(GTK_TREE_VIEW(glade_xml_get_widget(c->xml, "view_roster")));
	GList *list =  gtk_tree_selection_get_selected_rows(select, &model);
	for(;list; list = list->next)
	{
		gchar *id;
		gtk_tree_model_get_iter(GTK_TREE_MODEL(model), &iter, (GtkTreePath *) list->data);
		gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, ID_COLUMN, &id, -1);
	
		WokXMLTag itemtag("item");
		if ( c->wls->SendSignal(std::string("GUIRoster DragGet ") + id, itemtag) )
			data.AddTag(&itemtag);
		
		g_free(id);
	}
	
	g_list_foreach (list, ( void(*)(void*, void*)) gtk_tree_path_free, NULL);
	g_list_free (list);
	
	std::string data_str;
	data_str = data.GetStr();
	gtk_selection_data_set_text(selection, data_str.c_str(), data_str.size());
}

void
GUIRoster::SizeChange(GtkWidget *widget, GtkAllocation *requisition, GUIRoster *c)
{	
	int width = requisition->width - requisition->x - AVATAR_SIZE - 
			gtk_tree_view_column_get_width(c->pre_pix_column);
	/*		
	gtk_tree_view_column_set_max_width(c->text_column, width);
	gtk_tree_view_column_set_min_width(c->text_column, width);*/
}

gboolean
GUIRoster::popup_menu(GtkTreeView *tree_view, GdkEventButton *event, GUIRoster *c)
{
	GtkTreePath      *path;

	if (event->button == 3 && 
		gtk_tree_view_get_path_at_pos (tree_view, (int)event->x, (int)event->y, &path, NULL, NULL, NULL)) 
	{		
		GtkTreeSelection *selection;
		GtkTreeIter iter;
		gchar *id;
		GtkTreeModel *model;
		
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(glade_xml_get_widget(c->xml, "view_roster")));
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(glade_xml_get_widget(c->xml,"view_roster")));
		gtk_tree_selection_unselect_all (selection);
		gtk_tree_selection_select_path (selection, path);
		gtk_tree_model_get_iter (GTK_TREE_MODEL(model), &iter, path);

		gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, ID_COLUMN, &id, -1);
	
	
		int res;
		char buf[20];
		WokXMLTag emptytag(NULL, "emptytag");
		sprintf(buf, "%d", event->button);
		emptytag.AddAttr("button", buf);
		sprintf(buf, "%d", event->time);
		emptytag.AddAttr("time", buf);
		res = c->wls->SendSignal(std::string("GUIRoster RightButton ") + id, emptytag);
	
		g_free(id);
		gtk_tree_path_free (path);
		
		
		return TRUE;
	}
	return FALSE;
}


void
GUIRoster::SetHover (gchar *hid)
{
	if ( hoverid )
	{

		if(!strcmp(hoverid, hid ))
		{
			g_free (hid);
			return;
		}
		else
		{
			WokXMLTag tag1(NULL, "tooltip");
			wls->SendSignal(std::string("GUIRoster Tooltip Reset ") + hoverid, &tag1);
		}
		g_free(hoverid);
	}
	
	hoverid = g_strdup(hid);
	
	WokXMLTag tag2(NULL, "tooltip");
	tag2.AddAttr("id", hid);
	
	wls->SendSignal(std::string("GUIRoster Tooltip Set ") + hid, &tag2);
	wls->SendSignal("GUIRoster Tooltip Set", &tag2);
	
	g_free(hid);
}

void
GUIRoster::CleanHover ()
{
	if ( hoverid )
	{
		WokXMLTag tag(NULL, "tooltip");
		wls->SendSignal(std::string("GUIRoster Tooltip Reset ") + hoverid, &tag);
		
		g_free(hoverid);
		hoverid = NULL;
	}
}

gboolean 
GUIRoster::MouseMotion (GtkWidget *treeview, GdkEventMotion *event, GUIRoster *c)
{
	GtkTreePath *path;
	GtkTreeModel *model;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(glade_xml_get_widget(c->xml, "view_roster")));

	if( gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), (gint)event->x, (gint)event->y, 
																		&path, NULL, NULL, NULL) == FALSE)
	{

		gtk_tree_path_free(path);
		c->CleanHover();
		return FALSE;
	}
	
	GtkTreeIter iter;
	gtk_tree_model_get_iter(GTK_TREE_MODEL(model), &iter, path);
	gtk_tree_path_free(path);
	
	gchar *hid;
	
	gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, ID_COLUMN, &hid, -1);

	if( hid ) 
		c->SetHover(hid);
	else
		c->CleanHover();
	
	return FALSE;
}

gboolean
GUIRoster::MouseLeave(GtkWidget *treeview, GdkEventCrossing *event, GUIRoster *c)
{
	c->CleanHover();
	return FALSE;
}


int
GUIRoster::row_activated (GtkTreeView *treeview, GtkTreePath *arg1, GtkTreeViewColumn *arg2, GUIRoster *c)
{
	gchar *id;
	GtkTreeIter iter;
	GtkTreeModel *model;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(glade_xml_get_widget(c->xml, "view_roster")));
	
	gtk_tree_model_get_iter(GTK_TREE_MODEL(model), &iter, arg1);
	gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, ID_COLUMN, &id, -1);
	
	WokXMLTag emptytag(NULL, "tag");
	c->wls->SendSignal(std::string("GUIRoster Activate ") + id, emptytag);
	
	g_free(id);
	
	return 1;
}

int
GUIRoster::AddItem(WokXMLTag *tag)
{
	std::string letters = "ABCDEFGHIJKLMNOPQRSTUVXYZabcdefghijklmnopqrstuvxy";
	std::string id;
	
	while ( id.size() == 0 )
	{
		for( int i = 0 ; i < 10 ; i++ )
		{
			id += letters[rand() % letters.size()];
		}
		if(  item.find(id) != item.end() )
			id = "";
	}
	
	RosterItem *parant = NULL;
	if( tag->GetAttr("parant").size() )
		if ( item.find(tag->GetAttr("parant")) != item.end() )
			parant = item[tag->GetAttr("parant")];

	item[id] = new RosterItem(wls, xml, id, tag, parant);
	tag->AddAttr("id", id);
	return 1;
}

int
GUIRoster::UpdateItem(WokXMLTag *tag)
{
	item[tag->GetAttr("id")]->Update(tag);
	return 1;
}

int
GUIRoster::RemoveItem(WokXMLTag *tag)
{

	delete item[tag->GetAttr("id")];
	item.erase(tag->GetAttr("id"));
	return 1;
}
 
int
GUIRoster::GUIWindowInit(WokXMLTag *tag)
{
	if( inittag )
	{
		wls->SendSignal("GUI Window AddWidget",inittag);
		delete inittag;
		inittag = NULL;
		
		std::stringstream sig;
		sig << "GUI Window Close " << gtk_plug_get_id(GTK_PLUG(mainwindowplug));
		EXP_SIGHOOK(sig.str(), &GUIRoster::Close, 500);
	}
	return 1;
}

int
GUIRoster::Close(WokXMLTag *tag)
{
	gtk_widget_destroy(mainwindowplug);
	g_object_unref(xml);
	xml = NULL;
	
	return 1;
}
