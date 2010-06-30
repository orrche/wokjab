/***************************************************************************
 *  Copyright (C) 2003-2009  Kent Gustavsson <nedo80@gmail.com>
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


#include <sstream>
#include <gtk/gtkprivate.h>
#include <algorithm>
#include <Woklib/WokXMLTag.h>

#include "GUIWindowDock.h"

using std::list;
using std::string;

GUIWindowDock::GUIWindowDock(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("GUI WindowDock RemoveWidet", &GUIWindowDock::RemovePage, 1000);
	EXP_SIGHOOK("GUI WindowDock AddWidget", &GUIWindowDock::AppendPlugPage, 1000);
	EXP_SIGHOOK("GUI WindowDock Activate", &GUIWindowDock::Activate, 1000);
	EXP_SIGHOOK("GUI WindowDock HideWidget", &GUIWindowDock::HidePage, 1000);
	EXP_SIGHOOK("GUI WindowDock ShowWidget", &GUIWindowDock::ShowPage, 1000);
	window = NULL;
	config = new WokXMLTag(NULL, "NULL");
	EXP_SIGHOOK("Config XML Change /dock/window", &GUIWindowDock::Config, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/dock/window");
	wls->SendSignal("Config XML Trigger", &conftag);

	hiddenwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	hiddenvbox = gtk_vbox_new(FALSE, FALSE);
	gtk_container_add(GTK_CONTAINER(hiddenwindow), hiddenvbox);
	
	gtk_widget_realize(hiddenwindow);
	gtk_widget_realize(hiddenvbox);
}


GUIWindowDock::~GUIWindowDock()
{
}

void
GUIWindowDock::SaveConfig()
{
	int width, height;
	std::stringstream swidth, sheight;
	
	gtk_window_get_size(GTK_WINDOW(window), &width, &height);
	swidth << width;
	sheight << height;
	
	config->GetFirstTag("width").AddAttr("data", swidth.str());
	config->GetFirstTag("height").AddAttr("data", sheight.str());
	
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/dock/window");
	conftag.AddTag(config);
	
	EXP_SIGUNHOOK("Config XML Change /dock/window", &GUIWindowDock::Config, 500);
	wls->SendSignal("Config XML Store", &conftag);
	EXP_SIGHOOK("Config XML Change /dock/window", &GUIWindowDock::Config, 500);
	

}

gboolean GUIWindowDock::widget_destroy(GtkWidget *widget, GdkEvent *event, GUIWindowDock *c)
{
	
	c->SaveConfig();
	c->window = NULL;
	while( !c->Widgets.empty() )
	{
		WokXMLTag closetag(NULL, "close");
		std::stringstream buf;
		buf << "GUI WindowDock Close " << *(c->Widgets.begin());
		c->Widgets.erase(c->Widgets.begin());
		
		closetag.AddAttr("id", buf.str().substr(21));
		c->wls->SendSignal(buf.str(), &closetag);
	}
	
	return false;
}

void
GUIWindowDock::page_delete(GtkWidget *widget, GUIWindowDock *c )
{	
	WokXMLTag remove(NULL, "remove");

	std::stringstream str;
	str << c->Widgets[gtk_notebook_get_current_page(GTK_NOTEBOOK(c->notebook))];
	remove.AddAttr("widget", str.str());

	c->RemovePage(&remove);
}

GtkWidget *
GUIWindowDock::GetWidget()
{
	return notebook;
}

gboolean
GUIWindowDock::key_press_handler(GtkWidget * widget, GdkEventKey * event,
			     gpointer data)
{
	GUIWindowDock *obj;
	obj = static_cast < GUIWindowDock * > ( data );
	if (event->state & GDK_MOD1_MASK)
	{
		if(event->keyval >= '0' && event->keyval <= '9') {
			int switchto = event->keyval - '1';
			if(switchto == -1)
				switchto = 9;
			
			gtk_notebook_set_current_page(GTK_NOTEBOOK(obj->notebook), switchto);
		}
	}
	return FALSE;
}

void
GUIWindowDock::CreateWidget()
{
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	
	notebook = gtk_notebook_new();
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
	gtk_container_add (GTK_CONTAINER (window), notebook);
	
	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
		GTK_SIGNAL_FUNC (GUIWindowDock::widget_destroy), this);
	gtk_signal_connect (GTK_OBJECT (window), "key_press_event",
		    GTK_SIGNAL_FUNC (GUIWindowDock::key_press_handler),
			    this);
	
	gtk_window_set_title(GTK_WINDOW(window), std::string ("Message Dock ver2").c_str());
	
	g_signal_connect (G_OBJECT (notebook), "switch-page",
				G_CALLBACK (GUIWindowDock::SwitchPage), this);
	
	Config(NULL);
	gtk_widget_show_all(window);
}

void
GUIWindowDock::SwitchPage(GtkNotebook *notebook, GtkNotebookPage *note_page,  guint page_num, GUIWindowDock *c)
{
	GtkWidget *page = gtk_notebook_get_nth_page (GTK_NOTEBOOK(c->notebook), page_num);
	GtkWidget *bin = GTK_WIDGET(page);
	GtkWidget *socket = gtk_bin_get_child(GTK_BIN(bin));
	
	std::map <int, GtkWidget *>::iterator socket_search;
	
	for ( socket_search = c->sockets.begin() ; socket_search != c->sockets.end() ; socket_search++)
	{
		if ( socket_search->second == socket )
		{
			gtk_window_set_title(GTK_WINDOW(c->window), (c->identifier[socket_search->first] + " - Wokjab").c_str());
		}
	}
	
}

int
GUIWindowDock::Config(WokXMLTag *tag)
{
	if ( tag )
	{
		delete config;
		config = new WokXMLTag(tag->GetFirstTag("config"));
	}
	if (!window)
		return true;

	int width, height;
	
	width = atoi( config->GetFirstTag("width").GetAttr("data").c_str());
	height = atoi( config->GetFirstTag("height").GetAttr("data").c_str());
	
	if ( width && height && window)
		gtk_window_set_default_size(GTK_WINDOW(window), width, height);

	return true;
}

int
GUIWindowDock::HidePage(WokXMLTag *tag)
{	
	int mainid = atoi(tag->GetAttr("widget").c_str());
	if ( find(Widgets.begin(), Widgets.end() , mainid) == Widgets.end() )
		return 1;
	Widgets.erase(find(Widgets.begin(), Widgets.end(), atoi(tag->GetAttr("widget").c_str())));
		
	HiddenWidgets.push_back(mainid);
	
	GtkWidget *parent = gtk_widget_get_parent(sockets[mainid]);
	gtk_widget_reparent(labelsockets[mainid], hiddenvbox);
	gtk_widget_reparent(sockets[mainid], hiddenvbox);
	gtk_widget_destroy(parent);
	
	if(Widgets.empty())
		gtk_widget_destroy(window);
		
	return 1;
}

int
GUIWindowDock::ShowPage(WokXMLTag *tag)
{
	int mainid = atoi(tag->GetAttr("widget").c_str());
	
	if ( find(HiddenWidgets.begin(), HiddenWidgets.end() , mainid) == HiddenWidgets.end() )
		return 1;
	
	HiddenWidgets.erase(find(HiddenWidgets.begin(), HiddenWidgets.end(), mainid));
	
	if(Widgets.empty())
		CreateWidget();
	Widgets.push_back(mainid);
	
	GtkWidget *mainsock;
	GtkWidget *labelsock;
	
	mainsock = sockets[mainid];
	labelsock = labelsockets[mainid];
	
	GtkWidget *box = gtk_hbox_new(FALSE, 2);
	GtkWidget *close_button = gtk_button_new_with_label("x");
	GtkTooltips *tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, close_button, "Closes this tab", "Closes this tab");
	gtk_signal_connect( GTK_OBJECT(close_button), "clicked",
		GTK_SIGNAL_FUNC (GUIWindowDock::page_delete), this);
	
	GtkWidget *bin = gtk_event_box_new();
	gint n = gtk_notebook_append_page(GTK_NOTEBOOK(notebook), bin, box);
	
	gtk_widget_show_all(box);
	gtk_widget_show_all(window);
	
	gtk_widget_reparent(mainsock, bin);
	gtk_widget_reparent(labelsock, box);
	gtk_box_pack_end(GTK_BOX(box), close_button, FALSE, FALSE, 0);

		
	gtk_widget_show_all(mainsock);
	gtk_widget_show_all(labelsock);
	gtk_widget_show_all(box);
	
	gtk_window_present (GTK_WINDOW(window));
	gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), n);
	
	return 1;
}

int
GUIWindowDock::AppendPlugPage(WokXMLTag *tag)
{
	GtkWidget *mainsock;
	GtkWidget *labelsock;
	
	mainsock = gtk_socket_new();
	labelsock = gtk_socket_new();
	
	GtkWidget *box;
	GtkWidget *close_button;
	GtkTooltips *tooltips;

	box = gtk_hbox_new(FALSE, 2);
	close_button = gtk_button_new_with_label("x");
	tooltips = gtk_tooltips_new();
	gtk_tooltips_set_tip(tooltips, close_button, "Closes this tab", "Closes this tab");
	gtk_signal_connect( GTK_OBJECT(close_button), "clicked",
		GTK_SIGNAL_FUNC (GUIWindowDock::page_delete), this);
	
	gtk_box_pack_start(GTK_BOX(box), labelsock, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(box), close_button, FALSE, FALSE, 0);
	
	GtkWidget *bin = gtk_event_box_new();
	gtk_box_pack_start(GTK_BOX (hiddenvbox), bin, FALSE, FALSE,0);
	gtk_box_pack_start(GTK_BOX (hiddenvbox), box, FALSE, FALSE,0);
	gtk_container_add(GTK_CONTAINER(bin), mainsock);
	
	gtk_widget_show_all(box);
	gtk_widget_show_all(bin);
	gtk_widget_show_all(labelsock);
	gtk_socket_add_id(GTK_SOCKET(mainsock), atoi(tag->GetAttr("mainwidget").c_str()));
	gtk_socket_add_id(GTK_SOCKET(labelsock), atoi(tag->GetAttr("labelwidget").c_str()));
	
	HiddenWidgets.push_back(atoi(tag->GetAttr("mainwidget").c_str()));
	sockets[atoi(tag->GetAttr("mainwidget").c_str())] = mainsock;
	labelsockets[atoi(tag->GetAttr("mainwidget").c_str())] = labelsock;
	identifier[atoi(tag->GetAttr("mainwidget").c_str())] = tag->GetAttr("identifier");
	
	if ( tag->GetAttr("minimize") == "true" )
	{
	}
	else
	{
		WokXMLTag show_page(NULL, "hide");
		show_page.AddAttr("widget", tag->GetAttr("mainwidget"));

		ShowPage(&show_page);
	}
	
	return true;

}

int
GUIWindowDock::RemovePage(WokXMLTag *tag)
{
#warning check....
	
	WokXMLTag closetag(NULL, "close");
	closetag.AddAttr("id", tag->GetAttr("widget"));
	std::stringstream buf;

	GtkWidget *parent = gtk_widget_get_parent(sockets[atoi(tag->GetAttr("widget").c_str())]);
	
	buf << "GUI WindowDock Close " << tag->GetAttr("widget");
	wls->SendSignal(buf.str(), &closetag);
	
	Widgets.erase(find(Widgets.begin(), Widgets.end(), atoi(tag->GetAttr("widget").c_str())));
	
	gtk_widget_destroy(parent);
	if(Widgets.empty())
		gtk_widget_destroy(window);
	
	return true;
}

int
GUIWindowDock::Activate(WokXMLTag *tag)
{
	gtk_window_present (GTK_WINDOW(window));
	if ( find(Widgets.begin(), Widgets.end() , atoi(tag->GetAttr("id").c_str())) != Widgets.end() )
	{
		gint n = gtk_notebook_page_num(GTK_NOTEBOOK(notebook), gtk_widget_get_parent(sockets[atoi(tag->GetAttr("id").c_str())]));
		gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), n);
	}
	return true;
}
