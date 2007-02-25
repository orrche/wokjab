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
// Class: DisplayWidget
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Thu Aug 11 10:46:23 2005
//

#include "DisplayWidget.h"
#include "InputDialog.h"
#include "SignalGenDialog.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

DisplayWidget::DisplayWidget(WLSignal *wls) : WLSignalInstance(wls)
{
	Create();
}


DisplayWidget::~DisplayWidget()
{
	gtk_widget_destroy(glade_xml_get_widget (xml, "messagewindow"));
	g_object_unref(xml);
}

void
DisplayWidget::Create()
{
  xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/displaygtk.glade", NULL, NULL);

	GtkWidget *window = glade_xml_get_widget (xml, "messagewindow");
	GtkWidget *databutton = glade_xml_get_widget (xml, "databutton");
	GtkWidget *inputbutton = glade_xml_get_widget (xml, "inputbutton");
	signal_treeview = glade_xml_get_widget(xml,"signaltreeview");
	message_treeview = glade_xml_get_widget(xml,"messagetreeview");
	debug_treeview = glade_xml_get_widget(xml,"debugtreeview");
	inout_treeview = glade_xml_get_widget(xml,"inouttreeview");
	error_treeview = glade_xml_get_widget(xml,"errortreeview");
		
	
	signal_store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
  gtk_tree_view_set_model (GTK_TREE_VIEW(signal_treeview), GTK_TREE_MODEL(signal_store));
	message_store = gtk_list_store_new (1, G_TYPE_STRING);
  gtk_tree_view_set_model (GTK_TREE_VIEW(message_treeview), GTK_TREE_MODEL(message_store));
	inout_store = gtk_list_store_new (1, G_TYPE_STRING);
  gtk_tree_view_set_model (GTK_TREE_VIEW(inout_treeview), GTK_TREE_MODEL(inout_store));
	error_store = gtk_list_store_new (1, G_TYPE_STRING);
  gtk_tree_view_set_model (GTK_TREE_VIEW(error_treeview), GTK_TREE_MODEL(error_store));
	debug_store = gtk_list_store_new (1, G_TYPE_STRING);
  gtk_tree_view_set_model (GTK_TREE_VIEW(debug_treeview), GTK_TREE_MODEL(debug_store));


	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Signal", renderer, "text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (signal_treeview), column);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Signal", renderer, "text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (message_treeview), column);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Signal", renderer, "text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (inout_treeview), column);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Signal", renderer, "text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (error_treeview), column);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Signal", renderer, "text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (debug_treeview), column);
	
	
	g_signal_connect (G_OBJECT (window), "delete_event",
			G_CALLBACK (DisplayWidget::Delete), this);	
	g_signal_connect (G_OBJECT (databutton), "clicked",
			G_CALLBACK (DisplayWidget::ShowDataButton), this);	
	g_signal_connect (G_OBJECT (inputbutton), "clicked",
			G_CALLBACK (DisplayWidget::InputButton), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (xml, "signalbutton")), "clicked",
			G_CALLBACK (DisplayWidget::SignalButton), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (xml, "SignalsClear")), "clicked",
			G_CALLBACK (DisplayWidget::SignalClear), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (xml, "DebugClear")), "clicked",
			G_CALLBACK (DisplayWidget::DebugClear), this);
}

void
DisplayWidget::Show()
{
	gtk_widget_show(glade_xml_get_widget (xml, "messagewindow"));
	gtk_window_present (GTK_WINDOW(glade_xml_get_widget (xml, "messagewindow")));
}

void
DisplayWidget::InputButton (GtkButton *button, DisplayWidget *c)
{
	new InputDialog(c->wls);
}

void
DisplayWidget::SignalButton (GtkButton *button, DisplayWidget *c)
{
	new SignalGenDialog(c->wls);
}

void
DisplayWidget::SignalClear (GtkButton *button, DisplayWidget *c)
{
	std::list <WokXMLTag *>::iterator iter;
	for ( iter = c->signal_data.begin() ; iter != c->signal_data.end() ; iter++)
	{
		delete *iter;
	}
	c->signal_data.clear();
	
	gtk_list_store_clear(c->signal_store);

}

gboolean
DisplayWidget::ShowDataButton (GtkButton *button, DisplayWidget *c)
{
	GtkTreeIter iter;
	GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW(c->signal_treeview));
	
	if( !gtk_tree_selection_get_selected (sel, NULL , &iter))
		return false;
	
	GtkTreePath *path = gtk_tree_model_get_path (GTK_TREE_MODEL(c->signal_store), &iter);
	
	std::list<WokXMLTag *>::iterator listiter;
	listiter = c->signal_data.begin();
	for( int i = atoi(gtk_tree_path_to_string (path)); i ; i--)
		listiter++;
	c->CreateXMLViewer(*listiter);
	gtk_tree_path_free(path);
	
	return false;
}

gboolean 
DisplayWidget::Delete( GtkWidget *widget, GdkEvent *event, DisplayWidget *c)
{
	gtk_widget_hide(glade_xml_get_widget (c->xml, "messagewindow"));
	
	return true;
}


void
DisplayWidget::Error(WokXMLTag *tag)
{
	GtkTreeIter iter;
	int lines;
	
	gtk_list_store_append (error_store, &iter);  /* Acquire a top-level iterator */
	gtk_list_store_set (error_store, &iter,
											0, tag->GetFirstTag("body").GetBody().c_str(),
											-1);
	
	
	if( (lines = gtk_tree_model_iter_n_children (GTK_TREE_MODEL(error_store), NULL)) > 100)
	{
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(error_store), &iter);
		gtk_list_store_remove(error_store, &iter);
	}
	Show();
	gtk_notebook_set_current_page   (GTK_NOTEBOOK(glade_xml_get_widget(xml,"notebook")), 2);
}

void
DisplayWidget::InOut(WokXMLTag *tag)
{
	GtkTreeIter iter;
	
	gtk_list_store_append (inout_store, &iter);  /* Acquire a top-level iterator */
	gtk_list_store_set (inout_store, &iter,
											0, tag->GetFirstTag("body").GetBody().c_str(),
											-1);
	
	
	if( gtk_tree_model_iter_n_children (GTK_TREE_MODEL(inout_store), NULL) > 100)
	{
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(inout_store), &iter);
		gtk_list_store_remove(inout_store, &iter);
	}

}

void
DisplayWidget::Debug(WokXMLTag *tag)
{
	GtkTreeIter iter;
	int lines;
	
	gtk_list_store_append (debug_store, &iter);  /* Acquire a top-level iterator */
	gtk_list_store_set (debug_store, &iter,
											0, tag->GetFirstTag("body").GetBody().c_str(),
											-1);
	
	
	if( (lines = gtk_tree_model_iter_n_children (GTK_TREE_MODEL(debug_store), NULL)) > 100)
	{
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(debug_store), &iter);
		gtk_list_store_remove(debug_store, &iter);
	}

}

void
DisplayWidget::DebugClear(GtkButton *button, DisplayWidget *c)
{
	gtk_list_store_clear(c->debug_store);
}

void
DisplayWidget::Message(WokXMLTag *tag)
{
	GtkTreeIter iter;
	int lines;
	
	gtk_list_store_append (message_store, &iter);  /* Acquire a top-level iterator */
	gtk_list_store_set (message_store, &iter,
											0, tag->GetFirstTag("body").GetBody().c_str(),
											-1);
	
	
	if( (lines = gtk_tree_model_iter_n_children (GTK_TREE_MODEL(message_store), NULL)) > 100)
	{
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(message_store), &iter);
		gtk_list_store_remove(message_store, &iter);
	}
	
	gtk_notebook_set_current_page   (GTK_NOTEBOOK(glade_xml_get_widget(xml,"notebook")), 0);
	Show();
}

void
DisplayWidget::Signal(WokXMLTag *tag)
{
	GtkTreeIter iter;
	int lines;
	
	gtk_list_store_append (signal_store, &iter);  /* Acquire a top-level iterator */
	gtk_list_store_set (signal_store, &iter,
											0, tag->GetAttr("name").c_str(),
											-1);
	
	signal_data.push_back(new WokXMLTag(*tag));
	
	if( (lines = gtk_tree_model_iter_n_children (GTK_TREE_MODEL(signal_store), NULL)) > 100)
	{
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(signal_store), &iter);
		gtk_list_store_remove(signal_store, &iter);
		delete *signal_data.begin();
		signal_data.pop_front();
		//lines = 99;
	}
}

void
DisplayWidget::CreateXMLViewer(WokXMLTag *tag)
{
  GtkWidget *xmlviewer;
  GtkWidget *dialog_vbox1;
  GtkWidget *scrolledwindow1;
  GtkWidget *textview1;
  GtkWidget *dialog_action_area1;
  GtkWidget *okbutton1;

  xmlviewer = gtk_dialog_new ();
	gtk_window_set_default_size (GTK_WINDOW (xmlviewer), 350, 200);

  gtk_window_set_title (GTK_WINDOW (xmlviewer), _("Signal XML"));
  gtk_window_set_position (GTK_WINDOW (xmlviewer), GTK_WIN_POS_MOUSE);
  gtk_window_set_type_hint (GTK_WINDOW (xmlviewer), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox1 = GTK_DIALOG (xmlviewer)->vbox;
  gtk_widget_show (dialog_vbox1);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), scrolledwindow1, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_SHADOW_IN);

  textview1 = gtk_text_view_new ();
  gtk_widget_show (textview1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), textview1);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (textview1), FALSE);
  gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (textview1), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview1), GTK_WRAP_WORD);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textview1), FALSE);
  gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview1)), tag->GetStr().c_str(), -1);

  dialog_action_area1 = GTK_DIALOG (xmlviewer)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  okbutton1 = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (okbutton1);
  //gtk_dialog_add_action_widget (GTK_DIALOG (xmlviewer), okbutton1, GTK_RESPONSE_CLOSE);
  GTK_WIDGET_SET_FLAGS (okbutton1, GTK_CAN_DEFAULT);
	
	gtk_widget_show_all(xmlviewer);
}

