/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2008 <nedo80@gmail.com>
 * 
 * wokjab is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * wokjab is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "wokjab-dock-window.hpp"

WokjabDockWindow::WokjabDockWindow(WLSignal *wls, WokXMLTag *in_inittag, GtkWidget *topdock, WokjabDockWindow *relative) : WLSignalInstance(wls),
inittag(new WokXMLTag(*in_inittag))
{
	std::cout << "Inittag: " << *inittag << std::endl;
	GdlDockItemBehavior behavior = GDL_DOCK_ITEM_BEH_CANT_ICONIFY;
	
	if ( inittag->GetAttr("handle") == "false" )
		behavior = (GdlDockItemBehavior) (behavior | GDL_DOCK_ITEM_BEH_NO_GRIP);
	
	win = gdl_dock_item_new (inittag->GetAttr("id").c_str(), inittag->GetAttr("identifier").c_str(), behavior);
	mainsock = gtk_socket_new();
	placeholder = gtk_event_box_new();
	
	sig1h = g_signal_connect(G_OBJECT(win), "detach", G_CALLBACK (WokjabDockWindow::Destroy), this);
	sig2h = g_signal_connect(G_OBJECT(placeholder), "unrealize", G_CALLBACK (WokjabDockWindow::Unrealize), this);
	sig3h = g_signal_connect(G_OBJECT(placeholder), "realize", G_CALLBACK (WokjabDockWindow::Realize), this);
	
	hiddenwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_realize(hiddenwindow);
	
	gtk_container_add (GTK_CONTAINER (win), placeholder);
	
	gdl_dock_add_item (GDL_DOCK (topdock), GDL_DOCK_ITEM (win), GDL_DOCK_TOP);
	if( relative )
	{
		
		GdlDockPlacement placement = GDL_DOCK_CENTER;
		if ( inittag->GetAttr("placement") == "bottom" )
			placement = GDL_DOCK_BOTTOM;
		else if ( inittag->GetAttr("placement") == "center" )
			placement = GDL_DOCK_CENTER;
		
		GdlDockObject *dockto = GDL_DOCK_OBJECT(relative->win);
		
		while( dockto && !GDL_DOCK_OBJECT(dockto) )
			dockto = gdl_dock_object_get_parent_object(GDL_DOCK_OBJECT(dockto));
		
		std::cout << "1:" << GDL_IS_DOCK(relative->win) << std::endl;
		std::cout << "2:" << GDL_IS_DOCK(gdl_dock_object_get_parent_object(GDL_DOCK_OBJECT(relative->win))) << std::endl;
		std::cout << "3:" << GDL_IS_DOCK(gdl_dock_object_get_toplevel(GDL_DOCK_OBJECT(relative->win))) << std::endl;
		//
		if ( dockto ) 
			gdl_dock_item_dock_to (GDL_DOCK_ITEM (win), GDL_DOCK_ITEM(dockto), placement, -1);
		else
			gdl_dock_item_dock_to (GDL_DOCK_ITEM (win), GDL_DOCK_ITEM(gdl_dock_object_get_toplevel(GDL_DOCK_OBJECT(relative->win))), placement, -1);
	}
	else
		gdl_dock_item_dock_to (GDL_DOCK_ITEM (win), NULL, GDL_DOCK_FLOATING, -1);
	
	gtk_container_add(GTK_CONTAINER(hiddenwindow), mainsock);
	gtk_socket_add_id(GTK_SOCKET(mainsock), atoi(inittag->GetAttr("id").c_str()));

	
	gtk_widget_hide(hiddenwindow);
	gtk_widget_show_all(win);
}

WokjabDockWindow::~WokjabDockWindow()
{
	WokXMLTag closetag(NULL, "close");
	closetag.AddAttr("id", inittag->GetAttr("id"));

	wls->SendSignal("GUI WindowDock Close " + inittag->GetAttr("id"), &closetag);
		
	g_signal_handler_disconnect (G_OBJECT(win), sig1h);
	g_signal_handler_disconnect (G_OBJECT (placeholder), sig2h);
	g_signal_handler_disconnect (G_OBJECT (placeholder), sig3h);
	
	gtk_widget_destroy(hiddenwindow);
	
	//gdl_dock_item_unbind(GDL_DOCK_ITEM(win));
}

std::string
WokjabDockWindow::GetType()
{
	return inittag->GetAttr("type");
}

void
WokjabDockWindow::Unrealize(GtkWidget *widget, WokjabDockWindow *c)  
{
	gtk_widget_reparent(c->mainsock, c->hiddenwindow);
}

void
WokjabDockWindow::Realize(GtkWidget *widget, WokjabDockWindow *c)
{
	gtk_widget_reparent(c->mainsock, c->placeholder);
	gtk_widget_show_all(c->placeholder);
}

void
WokjabDockWindow::Destroy(GdlDockObject *gdldockobject, gboolean arg1, WokjabDockWindow *c)
{
	if ( arg1 == 1 )
	{
		WokXMLTag destroy("window");
		destroy.AddAttr("id", c->inittag->GetAttr("id"));
		
		c->wls->SendSignal("Wokjab DockWindow Destroy", destroy);
	}	
}
