/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <nedo80@gmail.com>
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

#include "include/GUIWindow.h"
#include "include/GUIRoster.h"

#include <iostream>
#include <sstream>
#include <algorithm>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

GUIWindow::GUIWindow(WLSignal *wls) :
WLSignalInstance(wls)
{
	xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/main.window.glade", "window", NULL);
	plugin_win_open = connect_win_open = false;
	visible = true;
	priotimeid = 0;

	g_signal_connect ((gpointer) glade_xml_get_widget(xml,"window"), "destroy",
					G_CALLBACK (GUIWindow::Destroy), this);
	g_signal_connect ((gpointer) glade_xml_get_widget(xml,"window"), "delete_event",
					G_CALLBACK (GUIWindow::Delete), this);
	g_signal_connect ((gpointer) glade_xml_get_widget(xml,"wokjabmenu"), "button_press_event",
                    G_CALLBACK (GUIWindow::MainMenu), this);
	
	config = new WokXMLTag(NULL, "NULL");
	EXP_SIGHOOK("Config XML Change /main/window", &GUIWindow::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/main/window");
	wls->SendSignal("Config XML Trigger", &conftag);

	EXP_SIGHOOK("GUI Window show", &GUIWindow::show, 1000);
	EXP_SIGHOOK("GUI Window hide", &GUIWindow::hide, 1000);
	EXP_SIGHOOK("GUI Window toggle", &GUIWindow::toggle, 1000);
	EXP_SIGHOOK("GUI Window AddWidget", &GUIWindow::AddWidget, 1000);
	EXP_SIGHOOK("Jabber Connection Authenticated", &GUIWindow::Loggedin, 1000);
	EXP_SIGHOOK("Jabber Connection Lost", &GUIWindow::Loggedout, 1000);
	EXP_SIGHOOK("GetMenu", &GUIWindow::GetMainMenu, 1000);
	EXP_SIGHOOK("MainMenu Connect", &GUIWindow::Connect, 1000);
	EXP_SIGHOOK("MainMenu Plugin", &GUIWindow::PluginWin, 1000);
	EXP_SIGHOOK("Quit Request", &GUIWindow::QuitRequest, 1000);
	EXP_SIGHOOK("Jabber XML Presence Send", &GUIWindow::SendingPresence, 10);
	EXP_SIGHOOK("GUI Window Init", &GUIWindow::GUIWindowInit, 550);	

	gmsghandler = new GUIMessageHandler(wls);
	new GUIRoster(wls);

	WokXMLTag empty(NULL, "empty");
	wls->SendSignal("GUI Window Init", empty);
}

GUIWindow::~GUIWindow()
{
	SaveConfig();
	EXP_SIGUNHOOK("Config XML Change /main/window", &GUIWindow::ReadConfig, 500);
	delete gmsghandler;
	
}

static gboolean
is_separator (GtkTreeModel *model,
	      GtkTreeIter  *iter,
	      gpointer      data)
{
  GtkTreePath *path;
  gboolean result;

  path = gtk_tree_model_get_path (model, iter);
  result = gtk_tree_path_get_indices (path)[0] == 5;
  gtk_tree_path_free (path);

  return result;
}

int
GUIWindow::GUIWindowInit(WokXMLTag *tag)
{
	GtkCellRenderer *renderer;
	preferencexml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/main.window.glade", "mainbox.presence", NULL);
	GtkWidget *main_vbox = glade_xml_get_widget (preferencexml, "mainbox.presence");
	char buf[40];

	mainwindowplug = gtk_plug_new(0);
	
	gtk_container_add(GTK_CONTAINER(mainwindowplug), main_vbox);
	
	gtk_widget_show_all(mainwindowplug);

	WokXMLTag contag(NULL, "connect");
	sprintf(buf, "%d", gtk_plug_get_id(GTK_PLUG(mainwindowplug)));
	WokXMLTag &widtag = contag.AddTag("widget");
	widtag.AddAttr("id", buf);
	widtag.AddAttr("expand", "false");
	widtag.AddAttr("fill", "true");
	contag.AddAttr("identifier", "Presence chooser");
	
	
	showmenu = gtk_list_store_new (5, G_TYPE_STRING, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	gtk_combo_box_set_model (GTK_COMBO_BOX(glade_xml_get_widget(preferencexml,"showentry")), GTK_TREE_MODEL(showmenu));
	
	renderer = gtk_cell_renderer_pixbuf_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (glade_xml_get_widget(preferencexml,"showentry")), renderer, FALSE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (glade_xml_get_widget(preferencexml,"showentry")), renderer, "pixbuf", 1,NULL);

	renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (glade_xml_get_widget(preferencexml,"showentry")), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (glade_xml_get_widget(preferencexml,"showentry")), renderer,"text", 0, NULL);
	   
	
	PopulateShowEntry();
	gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget(preferencexml,"showentry")), 4);

	g_signal_connect ((gpointer) glade_xml_get_widget(preferencexml,"prioentry"), "value-changed",
										G_CALLBACK (GUIWindow::SpinBtnPrio), this);
	g_signal_connect ((gpointer) glade_xml_get_widget(preferencexml,"statusentry"), "activate",
										G_CALLBACK (GUIWindow::EntryStatusActivate), this);
	g_signal_connect ((gpointer) glade_xml_get_widget(preferencexml,"statusentry"), "focus-out-event",
										G_CALLBACK (GUIWindow::EntryStatusLeft), this);
	g_signal_connect ((gpointer) glade_xml_get_widget(preferencexml,"showentry"), "changed",
                    G_CALLBACK (GUIWindow::MenuActivate), this);
    gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (glade_xml_get_widget(preferencexml,"showentry")), 
					  is_separator, NULL, NULL);
	
	wls->SendSignal("GUI Window AddWidget",&contag);
	std::stringstream sig;
	sig << "GUI Window Close " << gtk_plug_get_id(GTK_PLUG(mainwindowplug));
	EXP_SIGHOOK(sig.str(), &GUIWindow::PresenceClose, 500);
	return 1;
}

void
GUIWindow::PopulateShowEntry()
{
	//gtk_list_store_clear(showmenu);
	
	GtkTreeIter iter;
	GdkPixbuf *pixbuf;
	
	if ( gtk_tree_model_get_iter_first (GTK_TREE_MODEL(showmenu), &iter) == FALSE)
	{
		gtk_list_store_append (showmenu, &iter);
		pixbuf = gdk_pixbuf_new_from_file (PACKAGE_DATA_DIR"/wokjab/online.png", NULL);
		gtk_list_store_set (showmenu, &iter, 1, pixbuf, 0, "Online", -1);
		gtk_list_store_append (showmenu, &iter);
		pixbuf = gdk_pixbuf_new_from_file (PACKAGE_DATA_DIR"/wokjab/away.png", NULL);
		gtk_list_store_set (showmenu, &iter, 1, pixbuf, 0, "Away", -1);
		gtk_list_store_append (showmenu, &iter);
		pixbuf = gdk_pixbuf_new_from_file (PACKAGE_DATA_DIR"/wokjab/xa.png", NULL);
		gtk_list_store_set (showmenu, &iter, 1, pixbuf, 0, "Not Available", -1);
		gtk_list_store_append (showmenu, &iter);
		pixbuf = gdk_pixbuf_new_from_file (PACKAGE_DATA_DIR"/wokjab/dnd.png", NULL);
		gtk_list_store_set (showmenu, &iter, 1, pixbuf, 0, "Do Not Disturbe", -1);
		gtk_list_store_append (showmenu, &iter);
		pixbuf = gdk_pixbuf_new_from_file (PACKAGE_DATA_DIR"/wokjab/offline.png", NULL);
		gtk_list_store_set (showmenu, &iter, 1, pixbuf, 0, "Offline", -1);
	}
	
	if ( gtk_tree_model_get_iter_first (GTK_TREE_MODEL(showmenu), &iter) == TRUE )
	{
		gboolean cont = TRUE;
		
		for ( int i = 0 ; i < 5 ; i++ )
		{
			if ( cont == TRUE )
				cont = gtk_tree_model_iter_next(GTK_TREE_MODEL(showmenu), &iter);
		}
		
		
		if (cont)
			while ( gtk_list_store_remove (showmenu, &iter) == TRUE );
	}
	
	gtk_list_store_append (showmenu, &iter);
	gtk_list_store_set (showmenu, &iter,
			      0, "separator",
			      -1);
	
	std::list <WokXMLTag *>::iterator witer;
	if ( config ) 
	{
		for ( witer = config->GetFirstTag("stored_presence").GetTagList("item").begin() ; witer != config->GetFirstTag("stored_presence").GetTagList("item").end() ; witer++ )
		{
			gtk_list_store_insert (showmenu, &iter, 6);
			pixbuf = NULL;
			if ( (*witer)->GetAttr("show") == "xa" )
				pixbuf = gdk_pixbuf_new_from_file (PACKAGE_DATA_DIR"/wokjab/xa.png", NULL);
			else if ( (*witer)->GetAttr("show") == "away" )
				pixbuf = gdk_pixbuf_new_from_file (PACKAGE_DATA_DIR"/wokjab/away.png", NULL);
			else if ( (*witer)->GetAttr("show") == "dnd" )
				pixbuf = gdk_pixbuf_new_from_file (PACKAGE_DATA_DIR"/wokjab/dnd.png", NULL);
			else if ( (*witer)->GetAttr("show") == "" )
				pixbuf = gdk_pixbuf_new_from_file (PACKAGE_DATA_DIR"/wokjab/online.png", NULL);
			
			gtk_list_store_set (showmenu, &iter, 1, pixbuf, 0, (*witer)->GetAttr("status").c_str(), 
								2, (*witer)->GetAttr("status").c_str(),
								3, (*witer)->GetAttr("show").c_str(),
								4, (*witer)->GetAttr("prio").c_str(), -1);
		}
	}
}


int 
GUIWindow::PresenceClose(WokXMLTag *tag)
{
	gtk_widget_destroy(mainwindowplug);
	g_object_unref(xml);
	xml = NULL;

	return 1;
}

gboolean
GUIWindow::Delete( GtkWidget *widget, GdkEvent *event, GUIWindow *c)
{

	c->SaveConfig();

	if(c->config->GetFirstTag("exit_on_delete_event").GetAttr("data") == "false")
	{
		while( !c->Widgets.empty() )
		{
			WokXMLTag closetag(NULL, "close");
			std::stringstream buf;
			buf << "GUI Window Close " << *(c->Widgets.begin());
			c->Widgets.erase(c->Widgets.begin());
			
			closetag.AddAttr("id", buf.str().substr(21));
			c->wls->SendSignal(buf.str(), &closetag);
		}
		return FALSE;
	}
	c->hide(NULL);
	return TRUE;
}

/**
 * Function to add a socket widget into the main widget like the roster
 */
int
GUIWindow::AddWidget(WokXMLTag *tag)
{
	GtkWidget *sockwid;
	sockwid = gtk_socket_new();
	gboolean expand,fill;
	if ( tag->GetFirstTag("widget").GetAttr("expand") == "true" )
		expand = TRUE;
	else
		expand = FALSE;

	if ( tag->GetFirstTag("widget").GetAttr("fill") == "true" )
		fill = TRUE;
	else
		fill = FALSE;

	gtk_box_pack_start(GTK_BOX(glade_xml_get_widget(xml,"mainvbox")), sockwid, expand, fill, 0);
	gtk_socket_add_id(GTK_SOCKET(sockwid), atoi(tag->GetFirstTag("widget").GetAttr("id").c_str()));
	gtk_widget_show_all(sockwid);

	Widgets.push_back(atoi(tag->GetFirstTag("widget").GetAttr("id").c_str()));

	return true;
}

int
GUIWindow::SendingPresence(WokXMLTag *tag)
{
	WokXMLTag &ptag = tag->GetFirstTag("presence");
	if ( ptag.GetTagList("show").begin() == ptag.GetTagList("show").end() )
	{

		switch ( gtk_combo_box_get_active(GTK_COMBO_BOX(glade_xml_get_widget(preferencexml, "showentry"))) )
		{
			case 0:
				ptag.AddTag("show");
				break;
			case 1:
				ptag.AddTag("show").AddText("away");
				break;
			case 2:
				ptag.AddTag("show").AddText("xa");
				break;
			case 3:
				ptag.AddTag("show").AddText("dnd");
				break;
		}
	}
	else
	{
		if ( ptag.GetFirstTag("show").GetBody() == "" )
			gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget(preferencexml,"showentry")), 0);
		else if ( ptag.GetFirstTag("show").GetBody() == "away")
			gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget(preferencexml,"showentry")), 1);
		else if ( ptag.GetFirstTag("show").GetBody() == "xa")
			gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget(preferencexml,"showentry")), 2);
		else if ( ptag.GetFirstTag("show").GetBody() == "dnd")
			gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget(preferencexml,"showentry")), 3);

	}

	if ( ptag.GetTagList("status").begin() == ptag.GetTagList("status").end() )
		ptag.GetFirstTag("status").AddText(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(preferencexml,"statusentry"))));
	else
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget(preferencexml,"statusentry")), ptag.GetFirstTag("status").GetBody().c_str());

	if ( ptag.GetTagList("priority").begin() == ptag.GetTagList("priority").end() )
	{
		std::stringstream prio;
		prio << gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget(preferencexml,"prioentry")));
		ptag.GetFirstTag("priority").AddText(prio.str());
	}
	else
	{
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(preferencexml,"prioentry")), atoi(ptag.GetFirstTag("priority").GetBody().c_str()));
	}

	return true;
}

void
GUIWindow::SpinBtnPrio(GtkSpinButton *spinbutton, GUIWindow *c)
{
	if( c->priotimeid )
		g_source_remove(c->priotimeid);
	c->priotimeid = g_timeout_add (1000, (gboolean (*)(void *)) (GUIWindow::SetPresence), c);

}

gboolean
GUIWindow::StorePresence(GUIWindow *c)
{
	if ( c->current_status.empty() )
		return FALSE;
	
	std::stringstream prio;

	prio << gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(glade_xml_get_widget(c->preferencexml,"prioentry")));
	
	std::list <WokXMLTag*>::iterator witer;
	for ( witer = c->config->GetFirstTag("stored_presence").GetTagList("item").begin() ; witer != c->config->GetFirstTag("stored_presence").GetTagList("item").end() ; witer++)
	{
		if ( (*witer)->GetAttr("status") == c->current_status )
		{
			c->config->GetFirstTag("stored_presence").RemoveTag(*witer);
			witer = c->config->GetFirstTag("stored_presence").GetTagList("item").begin();
		}
	}
	
	WokXMLTag &item = c->config->GetFirstTag("stored_presence").AddTag("item");
	item.AddAttr("status", c->current_status);
	item.AddAttr("show", c->status_show);
	item.AddAttr("prio", prio.str());
	
	while( c->config->GetFirstTag("stored_presence").GetTagList("item").size() > 5 )
	{
		c->config->GetFirstTag("stored_presence").RemoveTag(*c->config->GetFirstTag("stored_presence").GetTagList("item").begin());
	}
	c->storetimeid = 0;
	
	c->PopulateShowEntry();
	return FALSE;
}

gboolean
GUIWindow::SetPresence(GUIWindow * c)
{
	std::stringstream prio;

	WokXMLTag msgtag (NULL, "message");
	WokXMLTag &ptag = msgtag.AddTag("presence");

	prio << gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(glade_xml_get_widget(c->preferencexml,"prioentry")));

	ptag.AddTag("priority").AddText(prio.str());

	std::list<std::string>::iterator iter;

	for( iter = c->ActiveSessions.begin() ; iter != c->ActiveSessions.end() ; iter++)
	{
		WokXMLTag sendtag(msgtag);
		sendtag.AddAttr("session", *iter);
		c->wls->SendSignal("Jabber XML Presence Send", &sendtag);
	}
	
	if( c->storetimeid )
		g_source_remove(c->storetimeid);
	c->storetimeid = g_timeout_add (15000, (gboolean (*)(void *)) (GUIWindow::StorePresence), c);

	
	return FALSE;
}

void
GUIWindow::SetStatusTo(const std::string &status)
{
	current_status = status;

	WokXMLTag msgtag (NULL, "message");
	WokXMLTag &ptag = msgtag.AddTag("presence");

	ptag.AddTag("status").AddText(status);

	std::list<std::string>::iterator iter;
	for( iter = ActiveSessions.begin() ; iter != ActiveSessions.end() ; iter++)
	{
		WokXMLTag sendtag(msgtag);
		sendtag.AddAttr("session", *iter);
		wls->SendSignal("Jabber XML Presence Send", &sendtag);
	}
	
	if( storetimeid )
		g_source_remove(storetimeid);
	storetimeid = g_timeout_add (15000, (gboolean (*)(void *)) (GUIWindow::StorePresence), this);
}

void
GUIWindow::EntryStatusActivate(GtkEntry *entry, GUIWindow *c)
{
	std::string status = gtk_entry_get_text(entry);
	
//	SavedStatusMsges[status] = gtk_combo_box_get_active(GTK_COMBO_BOX(glade_xml_get_widget(preferencexml, "showentry")));
	if ( c->current_status != status )
		c->SetStatusTo(status);
}

gboolean
GUIWindow::EntryStatusLeft(GtkWidget *widget, GdkEventFocus *event, GUIWindow *c)
{
	std::string status = gtk_entry_get_text(GTK_ENTRY(widget));
//	SavedStatusMsges[status] = gtk_combo_box_get_active(GTK_COMBO_BOX(glade_xml_get_widget(preferencexml, "showentry")));
	if ( c->current_status != status )
		c->SetStatusTo(status);
	return FALSE;
}

void
GUIWindow::MenuActivate (GtkComboBox *widget, GUIWindow *c)
{
	WokXMLTag msgtag(NULL, "message");
	WokXMLTag &ptag = msgtag.AddTag("presence");
	
	switch ( gtk_combo_box_get_active(GTK_COMBO_BOX(glade_xml_get_widget(c->preferencexml, "showentry"))) )
	{
		case 0:
			c->status_show = "";
			break;
		case 1:
			c->status_show = "away";
			break;
		case 2:
			c->status_show = "xa";
			break;
		case 3:
			c->status_show = "dnd";
			break;
		default:
			GtkTreeIter titer;
			if ( gtk_combo_box_get_active_iter(GTK_COMBO_BOX(glade_xml_get_widget(c->preferencexml, "showentry")), &titer) == TRUE )
			{
				gchar *status, *show, *prio;
				gtk_tree_model_get (GTK_TREE_MODEL(c->showmenu), &titer, 
									2, &status, 
									3, &show, 
									4, &prio, 
									-1);
				
				c->status_show = show;
				ptag.AddTag("status").AddText(status);
				ptag.AddTag("priority").AddText(prio);
				g_free(show);
				g_free(status);
				g_free(prio);
			}
	}
	ptag.AddTag("show").AddText(c->status_show);
	
	if ( c->ActiveSessions.empty() )
	{
		WokXMLTag connect("data");
		c->wls->SendSignal("MainMenu Connect", &connect);
		
		// For some odd reason this crashes wokjab, noo idea why that is
		//gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget(c->preferencexml, "showentry")), 4);
	}
	else
	{
		std::list<std::string>::iterator iter;
		for( iter = c->ActiveSessions.begin() ; iter != c->ActiveSessions.end() ; iter++)
		{
			WokXMLTag sendtag(msgtag);
			sendtag.AddAttr("session", *iter);
			c->wls->SendSignal("Jabber XML Presence Send", &sendtag);
		}
	}
}

void
GUIWindow::Destroy( GtkWidget *widget,
                     GUIWindow *c)
{
	if(c->visible)
		gtk_main_quit ();
}

int
GUIWindow::QuitRequest(WLSignalData *wlsd)
{
	gtk_main_quit();
}

int
GUIWindow::Connect (WLSignalData *wlsd)
{
	if(!connect_win_open)
	{
		connect_win_open = true;
		new GUIConnectWindow(&(connect_win_open), wls);
	}
}

int
GUIWindow::PluginWin (WLSignalData *wlsd)
{
	if(!plugin_win_open)
	{
		plugin_win_open = true;
		new GUIPluginWindow(&(plugin_win_open), wls);
	}
}

int
GUIWindow::GetMainMenu(WokXMLTag *menu_tag)
{
	WokXMLTag *menu_item;

	menu_item = &menu_tag->AddTag("item");
	menu_item->AddAttr("name", "Connect");
	menu_item->AddAttr("signal", "MainMenu Connect");

	menu_item = &menu_tag->AddTag("item");
	menu_item->AddAttr("name", "SignOut");
	menu_item->AddAttr("signal", "Jabber Connection Disconnect");

	menu_item = &menu_tag->AddTag("item");
	menu_item->AddAttr("name", "Plugin");
	menu_item->AddAttr("signal", "MainMenu Plugin");

	menu_item = &menu_tag->AddTag("item");
	menu_item->AddAttr("name", "Quit");
	menu_item->AddAttr("signal", "Quit Request");

	return (true);
}

gboolean
GUIWindow::MainMenu (GtkButton *button, GdkEventButton *event, GUIWindow *c)
{
	char buf[20];
	WokXMLTag MenuXML(NULL, "menu");
	sprintf(buf, "%d", event->button);
	MenuXML.AddAttr("button", buf);
	sprintf(buf, "%d", event->time);
	MenuXML.AddAttr("time", buf);
	MenuXML.AddTag("item").AddAttr("signal", "Get Main Menu");
	MenuXML.AddTag("item").AddAttr("signal", "GetMenu");
	WokXMLTag &data = MenuXML.AddTag("data");
	c->wls->SendSignal("Jabber GUI JIDMenu", &MenuXML);

	return false;
}

int
GUIWindow::ReadConfig(WokXMLTag *tag)
{
	if ( !visible )
		return 1;
	if( tag )
	{
		delete config;
		config = new WokXMLTag(tag->GetFirstTag("config"));
		tag->GetFirstTag("config").GetFirstTag("exit_on_delete_event").AddAttr("type", "bool");
		tag->GetFirstTag("config").GetFirstTag("exit_on_delete_event").AddAttr("label", "Minimize to systray on window exit");
		tag->GetFirstTag("config").GetFirstTag("dock").AddAttr("type", "bool");
		tag->GetFirstTag("config").GetFirstTag("dock").AddAttr("label", "Set window type to be a dock or panel");
	}

	// Getting size from the config
	int width, height, pos_y, pos_x;
	bool dock;

	width = atoi( config->GetFirstTag("width").GetAttr("data").c_str());
	height = atoi( config->GetFirstTag("height").GetAttr("data").c_str());
	pos_x = atoi( config->GetFirstTag("pos_x").GetAttr("data").c_str());
	pos_y = atoi( config->GetFirstTag("pos_y").GetAttr("data").c_str());
		
	gtk_window_move (GTK_WINDOW(glade_xml_get_widget(xml, "window")), pos_x, pos_y);
		
	if ( config->GetFirstTag("dock").GetAttr("data") != "false" )
		dock = true;
	else
		dock = false;

	if( visible && width && height)
		gtk_window_resize(GTK_WINDOW(glade_xml_get_widget(xml,"window")), width, height);
}

void
GUIWindow::SaveConfig()
{
	int width, height, pos_x, pos_y;
	std::stringstream sheight, swidth, spos_x, spos_y;

	gtk_window_get_size(GTK_WINDOW(glade_xml_get_widget(xml,"window")), &width, &height);
	gtk_window_get_position(GTK_WINDOW(glade_xml_get_widget(xml,"window")), &pos_x, &pos_y);
	sheight << height;
	swidth << width;
	spos_x << pos_x;
	spos_y << pos_y;

	
	config->GetFirstTag("width").AddAttr("data", swidth.str());
	config->GetFirstTag("height").AddAttr("data", sheight.str());
	config->GetFirstTag("pos_x").AddAttr("data", spos_x.str());
	config->GetFirstTag("pos_y").AddAttr("data", spos_y.str());

	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/main/window");
	conftag.AddTag(config);

	EXP_SIGUNHOOK("Config XML Change /main/window", &GUIWindow::ReadConfig, 500);
	wls->SendSignal("Config XML Store", &conftag);
	EXP_SIGHOOK("Config XML Change /main/window", &GUIWindow::ReadConfig, 500);
}


int
GUIWindow::show(WLSignalData *wlsd)
{
	if(visible)
	{
		gtk_window_present(GTK_WINDOW(glade_xml_get_widget(xml, "window")));
		gtk_window_deiconify (GTK_WINDOW(glade_xml_get_widget(xml, "window")));
		return true;
	}
	
	visible = true;
	ReadConfig(NULL);
	gtk_window_present(GTK_WINDOW(glade_xml_get_widget(xml, "window")));
	gtk_window_deiconify (GTK_WINDOW(glade_xml_get_widget(xml, "window")));
	return true;
}

int
GUIWindow::hide(WLSignalData *wlsd)
{
	if(!visible)
		return true;

	SaveConfig();
	gtk_widget_hide(glade_xml_get_widget(xml,"window"));

	visible = false;
	connected = 0;
	
	
	return true;
}

int
GUIWindow::Loggedin(WokXMLTag *tag)
{
	ActiveSessions.push_back(tag->GetAttr("session"));
	gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget(preferencexml,"showentry")), 0);
}

int
GUIWindow::Loggedout(WokXMLTag *tag)
{
	std::list <std::string>::iterator iter;
	iter = std::find(ActiveSessions.begin(), ActiveSessions.end(), tag->GetAttr("session"));
	if( iter == ActiveSessions.end() )
		return true;
	ActiveSessions.erase(iter);

	if(ActiveSessions.size() == 0)
		gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget(preferencexml,"showentry")), 4);

	return true;
}

int
GUIWindow::toggle(WLSignalData *wlsd)
{
	if(visible)
		hide(wlsd);
	else
		show(wlsd);

	return true;
}


gboolean
GUIWindow::WindowMove(GtkWidget *widget, GdkEventMotion *event, GUIWindow *c)
{
	c->UpdateStruts();
	return false;
}

void
GUIWindow::UpdateStruts()
{
	return;
#if 0
	gulong	struts[12] = { 0, };
	Display	*display;
	Window	xwindow;
	gint	width;
	gint	height;
	gint  swidth;
	gint root_x;
	gint root_y;
	int i = 0;
	GdkScreen *screen;

	if( !GTK_WIDGET_REALIZED(window) )
	{
		woklib_error(wls, "Window not yet realized for strut manipulation");
		return;
	}

	gtk_window_get_size(GTK_WINDOW(window), &width, &height);
	gtk_window_get_position(GTK_WINDOW(window), &root_x, &root_y);


	display = GDK_WINDOW_XDISPLAY(window->window);
	xwindow  = GDK_WINDOW_XWINDOW(window->window);
	screen = gtk_window_get_screen(GTK_WINDOW(window));

	swidth = gdk_screen_get_width (screen);
	net_wm_strut_partial = XInternAtom(display, "_NET_WM_STRUT_PARTIAL", False);
	net_wm_strut = XInternAtom(display, "_NET_WM_STRUT", False);

	if ( root_x + width/2 > swidth/2)
	{
		struts[STRUT_RIGHT] = swidth - root_x;
		struts[STRUT_RIGHT_START] = root_y;
		struts[STRUT_RIGHT_END] = root_y + height;
	}
	else
	{
		struts[STRUT_LEFT] = width + root_x;
		struts[STRUT_LEFT_START] = root_y;
		struts[STRUT_LEFT_END] = root_y + height;
	}

	gdk_error_trap_push();


	XChangeProperty (display, xwindow, net_wm_strut,
					XA_CARDINAL, 32, PropModeReplace,
					(guchar *) &struts, 4);

	XChangeProperty (display, xwindow, net_wm_strut_partial,
					XA_CARDINAL, 32, PropModeReplace,
					(guchar *) &struts, 12);

	gdk_error_trap_pop();
#endif
}
