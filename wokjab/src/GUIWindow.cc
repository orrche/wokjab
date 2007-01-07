/***************************************************************************
 *  Copyright (C) 2003-2006  Kent Gustavsson <nedo80@gmail.com>
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
using std::cout;
using std::endl;

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

GUIWindow::GUIWindow(WLSignal *wls) :
WLSignalInstance(wls)
{
	xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/main.window.glade", NULL, NULL);
	plugin_win_open = connect_win_open = false;
	visible = true;
	priotimeid = 0;

	gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget(xml,"showentry")), 4);

	//gtk_box_pack_start(GTK_BOX(glade_xml_get_widget(xml,"mainvbox")), roster->GetWidget(), TRUE, TRUE, 0 );

	g_signal_connect ((gpointer) glade_xml_get_widget(xml,"prioentry"), "value-changed",
										G_CALLBACK (GUIWindow::SpinBtnPrio), this);
	g_signal_connect ((gpointer) glade_xml_get_widget(xml,"statusentry"), "activate",
										G_CALLBACK (GUIWindow::EntryStatusActivate), this);
	g_signal_connect ((gpointer) glade_xml_get_widget(xml,"statusentry"), "focus-out-event",
										G_CALLBACK (GUIWindow::EntryStatusLeft), this);
	g_signal_connect ((gpointer) glade_xml_get_widget(xml,"window"), "destroy",
										G_CALLBACK (GUIWindow::Destroy), this);
	g_signal_connect ((gpointer) glade_xml_get_widget(xml,"window"), "delete_event",
										G_CALLBACK (GUIWindow::Delete), this);

	g_signal_connect ((gpointer) glade_xml_get_widget(xml,"wokjabmenu"), "button_press_event",
                    G_CALLBACK (GUIWindow::MainMenu), this);
	g_signal_connect ((gpointer) glade_xml_get_widget(xml,"showentry"), "changed",
                    G_CALLBACK (GUIWindow::MenuActivate), this);

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

	gmsghandler = new GUIMessageHandler(wls);
	roster = new GUIRoster(wls);

	WokXMLTag empty(NULL, "empty");
	wls->SendSignal("GUI Window Init", empty);
}

GUIWindow::~GUIWindow()
{
	EXP_SIGUNHOOK("Config XML Change /main/window", &GUIWindow::ReadConfig, 500);
	delete gmsghandler;
	delete roster;
}

gboolean
GUIWindow::Delete( GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GUIWindow *data;
	data = static_cast <GUIWindow *> ( user_data);

	data->SaveConfig();

	if(data->config->GetFirstTag("exit_on_delete_event").GetAttr("data") != "false")
		return false;

	data->hide(NULL);
	return true;
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

//	Widgets.push_back(atoi(tag->GetFirstTag("widget").GetAttr("id").c_str()));

	return true;
}

int
GUIWindow::SendingPresence(WokXMLTag *tag)
{
	WokXMLTag &ptag = tag->GetFirstTag("presence");
	if ( ptag.GetTagList("show").begin() == ptag.GetTagList("show").end() )
	{

		switch ( gtk_combo_box_get_active(GTK_COMBO_BOX(glade_xml_get_widget(xml, "showentry"))) )
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
			gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget(xml,"showentry")), 0);
		else if ( ptag.GetFirstTag("show").GetBody() == "away")
			gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget(xml,"showentry")), 1);
		else if ( ptag.GetFirstTag("show").GetBody() == "xa")
			gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget(xml,"showentry")), 2);
		else if ( ptag.GetFirstTag("show").GetBody() == "dnd")
			gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget(xml,"showentry")), 3);

	}

	if ( ptag.GetTagList("status").begin() == ptag.GetTagList("status").end() )
		ptag.GetFirstTag("status").AddText(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget(xml,"statusentry"))));
	else
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget(xml,"statusentry")), ptag.GetFirstTag("status").GetBody().c_str());

	if ( ptag.GetTagList("priority").begin() == ptag.GetTagList("priority").end() )
	{
		std::stringstream prio;
		prio << gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"prioentry")));
		ptag.GetFirstTag("priority").AddText(prio.str());
	}
	else
	{
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(xml,"prioentry")), atoi(ptag.GetFirstTag("priority").GetBody().c_str()));
	}

	return true;
}

gboolean
GUIWindow::SetPresence(GUIWindow * c)
{
	std::stringstream prio;

	WokXMLTag msgtag (NULL, "message");
	WokXMLTag &ptag = msgtag.AddTag("presence");

	prio << gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(glade_xml_get_widget(c->xml,"prioentry")));

	ptag.AddTag("priority").AddText(prio.str());

	std::list<std::string>::iterator iter;
	for( iter = c->ActiveSessions.begin() ; iter != c->ActiveSessions.end() ; iter++)
	{
		WokXMLTag sendtag(msgtag);
		sendtag.AddAttr("session", *iter);
		c->wls->SendSignal("Jabber XML Presence Send", &sendtag);
	}

	return FALSE;
}

void
GUIWindow::SpinBtnPrio(GtkSpinButton *spinbutton, GUIWindow *c)
{
	if( c->priotimeid )
		g_source_remove(c->priotimeid);
	c->priotimeid = g_timeout_add (1000, (gboolean (*)(void *)) (GUIWindow::SetPresence), c);

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
}

void
GUIWindow::EntryStatusActivate(GtkEntry *entry, GUIWindow *c)
{
	std::string status = gtk_entry_get_text(entry);
	if ( c->current_status != status )
		c->SetStatusTo(status);
}

gboolean
GUIWindow::EntryStatusLeft(GtkWidget *widget, GdkEventFocus *event, GUIWindow *c)
{
	std::string status = gtk_entry_get_text(GTK_ENTRY(widget));
	if ( c->current_status != status )
		c->SetStatusTo(status);
	return FALSE;
}

void
GUIWindow::MenuActivate (GtkComboBox *widget, GUIWindow *c)
{
	WokXMLTag msgtag(NULL, "message");
	WokXMLTag &ptag = msgtag.AddTag("presence");

	switch ( gtk_combo_box_get_active(GTK_COMBO_BOX(glade_xml_get_widget(c->xml, "showentry"))) )
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

	std::list<std::string>::iterator iter;
	for( iter = c->ActiveSessions.begin() ; iter != c->ActiveSessions.end() ; iter++)
	{
		WokXMLTag sendtag(msgtag);
		sendtag.AddAttr("session", *iter);
		c->wls->SendSignal("Jabber XML Presence Send", &sendtag);
	}
}

void
GUIWindow::Destroy( GtkWidget *widget,
                     gpointer   user_data )
{
	GUIWindow *data;
	data = static_cast <GUIWindow *> ( user_data);
	if(data->visible)
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
		tag->GetFirstTag("config").GetFirstTag("exit_on_delete_event").AddAttr("label", "Turn off program on window exit");
		tag->GetFirstTag("config").GetFirstTag("dock").AddAttr("type", "bool");
		tag->GetFirstTag("config").GetFirstTag("dock").AddAttr("label", "Set window type to be a dock or panel");
	}

	// Getting size from the config
	int width, height;
	bool dock;

	width = atoi( config->GetFirstTag("width").GetAttr("data").c_str());
	height = atoi( config->GetFirstTag("height").GetAttr("data").c_str());
	if ( config->GetFirstTag("dock").GetAttr("data") != "false" )
		dock = true;
	else
		dock = false;

	if(width == 0)
		width = 120;

	if(height == 0)
		height = 500;

	if( visible )
	{
	// More struts...
#if 0
		if( dock && !connected )
			connected = g_signal_connect ((gpointer) window, "configure-event",
											G_CALLBACK (GUIWindow::WindowMove),
											this);
		else if ( !dock && connected )
			g_signal_handler_disconnect ((gpointer) window, connected);
#endif
		gtk_window_set_default_size(GTK_WINDOW(glade_xml_get_widget(xml,"window")), width, height);
	}


	//SaveConfig();

}

void
GUIWindow::SaveConfig()
{
	int width, height;
	std::stringstream sheight, swidth;

	gtk_window_get_size(GTK_WINDOW(glade_xml_get_widget(xml,"window")), &width, &height);
	sheight << height;
	swidth << width;

	config->GetFirstTag("width").AddAttr("data", swidth.str());
	config->GetFirstTag("height").AddAttr("data", sheight.str());

	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/main/window");
	conftag.AddTag(config);

	EXP_SIGUNHOOK("Config XML Change /main/window", &GUIWindow::ReadConfig, 500);
	wls->SendSignal("Config XML Store", &conftag);
	EXP_SIGHOOK("Config XML Change /main/window", &GUIWindow::ReadConfig, 500);
	//roster->SaveConfig();

}


int
GUIWindow::show(WLSignalData *wlsd)
{
	if(visible)
		return true;

	gtk_widget_show_all (glade_xml_get_widget(xml,"window"));
	gtk_window_present(GTK_WINDOW(glade_xml_get_widget(xml, "window")));

	visible = true;
	ReadConfig(NULL);

	return true;
}

int
GUIWindow::hide(WLSignalData *wlsd)
{
	if(!visible)
		return true;

	SaveConfig();
	gtk_widget_hide_all(glade_xml_get_widget(xml,"window"));

	visible = false;
	connected = 0;
	return true;
}

int
GUIWindow::Loggedin(WokXMLTag *tag)
{
	ActiveSessions.push_back(tag->GetAttr("session"));
	gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget(xml,"showentry")), 0);
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
		gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget(xml,"showentry")), 4);

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
	// Fucks up kde at the moment
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
