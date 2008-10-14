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

WokjabDockWindow::WokjabDockWindow(WLSignal *wls, WokXMLTag *in_inittag, GtkWidget *in_topdock, WokjabDockWindow *relative, GdlDockLayout *in_layout) : WLSignalInstance(wls),
inittag(new WokXMLTag(*in_inittag)),
layout(in_layout),
topdock(in_topdock)
{
	GdlDockItemBehavior behavior = GDL_DOCK_ITEM_BEH_NORMAL;
	
	if ( inittag->GetAttr("handle") == "false" )
		behavior = (GdlDockItemBehavior) (behavior | GDL_DOCK_ITEM_BEH_NO_GRIP);
	
	win = gdl_dock_item_new (inittag->GetAttr("id").c_str(), inittag->GetAttr("identifier").c_str(), behavior);
	mainsock = gtk_socket_new();
	placeholder = gtk_event_box_new();
	
	gtk_signal_connect (GTK_OBJECT (win), "key_press_event",
	    GTK_SIGNAL_FUNC (WokjabDockWindow::key_press_handler), this);
	
	
	if (! inittag->GetAttr("labelid").empty() )
	{
		hiddenlabel = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		
		labelsock = gtk_socket_new();
		labelph = gtk_event_box_new();
		
		gtk_container_add(GTK_CONTAINER(hiddenlabel), labelsock);
		gtk_widget_destroy(gdl_dock_item_get_tablabel(GDL_DOCK_ITEM(win)));
		gdl_dock_item_set_tablabel (GDL_DOCK_ITEM(win), labelph);
		
		gtk_widget_show_all(labelph);
		gtk_widget_show_all(hiddenlabel);
		
		sig2lh = g_signal_connect(G_OBJECT(labelph), "unrealize", G_CALLBACK (WokjabDockWindow::LabelUnrealize), this);
		sig3lh = g_signal_connect(G_OBJECT(labelph), "realize", G_CALLBACK (WokjabDockWindow::LabelRealize), this);
		
		gtk_socket_add_id(GTK_SOCKET(labelsock), atoi(inittag->GetAttr("labelid").c_str()));
		gtk_widget_show_all(labelsock);
		gtk_widget_hide(hiddenlabel);

	}
	else
		labelsock = NULL;
	
	sig1h = g_signal_connect_after(G_OBJECT(win), "detach", G_CALLBACK (WokjabDockWindow::Destroy), this);
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

		GdlDockObject *dockto = gdl_dock_object_get_parent_object(GDL_DOCK_OBJECT(relative->win));
		if ( !dockto ) 
		{
			dockto = GDL_DOCK_OBJECT(relative->win);
		}
		
		gdl_dock_item_dock_to (GDL_DOCK_ITEM (win), GDL_DOCK_ITEM(dockto), placement, -1);
		
		dockto = gdl_dock_object_get_parent_object(GDL_DOCK_OBJECT(relative->win));
		if ( dockto ) 
		{
			/* Take advantage of a bug in gdl that lets you put the tabs at top by specifying horizontal
			 * and at the bottom by not touching it at all 
			 * Horizontal most be specified before vertical tho if you want it there hehe.
			 */
			if ( inittag->GetAttr("orientation") != "bottom" )
				g_object_set (dockto, "orientation", GTK_ORIENTATION_HORIZONTAL, NULL);
			
			if ( inittag->GetAttr("orientation") == "vertical" )
				g_object_set (dockto, "orientation", GTK_ORIENTATION_VERTICAL, NULL);
		}
		
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

	wls->SendSignal("Wokjab DockWindow Close " + inittag->GetAttr("id"), &closetag);
		
	g_signal_handler_disconnect (G_OBJECT(win), sig1h);
	g_signal_handler_disconnect (G_OBJECT (placeholder), sig2h);
	g_signal_handler_disconnect (G_OBJECT (placeholder), sig3h);
	
	if (! inittag->GetAttr("labelid").empty() )
	{
		g_signal_handler_disconnect (G_OBJECT (labelph), sig2lh);
		g_signal_handler_disconnect (G_OBJECT (labelph), sig3lh);
		gtk_widget_destroy(hiddenlabel);
	}
	gtk_widget_destroy(hiddenwindow);
}

gboolean
WokjabDockWindow::key_press_handler(GtkWidget * widget, GdkEventKey * event,
			     WokjabDockWindow *c)
{
	if (event->state & GDK_MOD1_MASK)
	{
		if(event->keyval >= '0' && event->keyval <= '9') {
			int switchto = event->keyval - '1';
			if(switchto == -1)
				switchto = 9;
			
			GdlDockObject *dockto = gdl_dock_object_get_parent_object(GDL_DOCK_OBJECT(c->win));
			if ( dockto ) 
			{
				GList *list = gdl_dock_get_named_items(GDL_DOCK(dockto));
				
				int i = 0;
				while(list)
				{
					list = list->next;
					i++;
				}
				if ( switchto < i )
					g_object_set (G_OBJECT(dockto), "page", switchto, NULL);
			}
		}
	}
	return FALSE;
}

void
WokjabDockWindow::Activate()
{
	if ( GDL_DOCK_ITEM_ICONIFIED(win) )
		Show(NULL);
	
	GdlDockObject *dock = GDL_DOCK_OBJECT(win);
	GdlDockObject *child = dock;
	
	while( dock && !GDL_IS_DOCK(dock) )
	{
		child = dock;
		dock = gdl_dock_object_get_parent_object(GDL_DOCK_OBJECT(dock));
	}
		
	gdl_dock_object_present (gdl_dock_object_get_parent_object(GDL_DOCK_OBJECT(win)), GDL_DOCK_OBJECT(win));	
	
	GtkWidget *window = win;
	
	while ( window && !GTK_IS_WINDOW(window) )
	{
		window = gtk_widget_get_parent(window);
	}
	if ( GTK_IS_WINDOW(window) )
	{	
		gtk_window_present (GTK_WINDOW(window));
	}
}

void
WokjabDockWindow::SetUrgencyHint(WokXMLTag *tag)
{
	GtkWidget *window = win;
	while ( window && !GTK_IS_WINDOW(window) )
	{
		window = gtk_widget_get_parent(window);
	}
	
	if ( GTK_IS_WINDOW(window) )
	{	
		if ( tag->GetFirstTag("urgency").GetAttr("data") != "false" )
			gtk_window_set_urgency_hint (GTK_WINDOW(window), TRUE);
		else
			gtk_window_set_urgency_hint (GTK_WINDOW(window), FALSE);
	}
}

bool
WokjabDockWindow::Visible()
{
	return GDL_DOCK_ITEM_ICONIFIED(win) == FALSE;
}

void
WokjabDockWindow::Hide(WokXMLTag *tag)
{
	if ( !GDL_DOCK_ITEM_ICONIFIED(win) ) 
		gdl_dock_item_iconify_item(GDL_DOCK_ITEM(win));
}

void
WokjabDockWindow::Show(WokXMLTag *tag)
{
	if ( GDL_DOCK_ITEM_ICONIFIED(win) )
	{
		gdl_dock_item_show_item(GDL_DOCK_ITEM (win));
		gtk_widget_show(win);
	}
	
	GtkWidget *window = win;
	while ( window && window != topdock )
	{
		window = gtk_widget_get_parent(window);
	}
	
	if ( window == topdock )
	{
		gdl_dock_item_dock_to (GDL_DOCK_ITEM (win), NULL, GDL_DOCK_FLOATING, -1);
	}
	
}

std::string
WokjabDockWindow::GetType()
{
	return inittag->GetAttr("type");
}


void
WokjabDockWindow::LabelUnrealize(GtkWidget *widget, WokjabDockWindow *c)  
{
	gtk_widget_reparent(c->labelsock, c->hiddenlabel);
}

void
WokjabDockWindow::LabelRealize(GtkWidget *widget, WokjabDockWindow *c)
{
	gtk_widget_reparent(c->labelsock, c->labelph);
	gtk_widget_show_all(c->labelsock);
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
		if ( !GDL_DOCK_ITEM_ICONIFIED(c->win) )
		{
			WokXMLTag destroy("window");
			destroy.AddAttr("id", c->inittag->GetAttr("id"));
			
			c->wls->SendSignal("Wokjab DockWindow Destroy", destroy);
		}
	}
}
