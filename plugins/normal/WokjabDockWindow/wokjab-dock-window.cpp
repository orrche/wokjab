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

WokjabDockWindow::WokjabDockWindow(WLSignal *wls, WokXMLTag *in_inittag, Gtk::Notebook *parent) : WLSignalInstance(wls),
inittag(new WokXMLTag(*in_inittag)),
parent( parent)
{
	//if (! inittag->GetAttr("labelid").empty())
	//{
	//}

	label_cbutton.set_label("x");
	label_cbutton.set_relief(Gtk::RELIEF_NONE);
	label_box.pack_start(labelsock);
	label_box.pack_start(label_cbutton);
	parent->append_page(mainsock, label_box);

	label_cbutton.signal_clicked().connect(sigc::mem_fun(*this,
              &WokjabDockWindow::on_cbutton_clicked));
	
	label_box.show_all();
	mainsock.show_all();
	
	mainsock.add_id(atoi(inittag->GetAttr("id").c_str()));
	labelsock.add_id(atoi(inittag->GetAttr("labelid").c_str()));
	
	Activate();
}

WokjabDockWindow::~WokjabDockWindow()
{	
	WokXMLTag closetag(NULL, "close");
	closetag.AddAttr("id", inittag->GetAttr("id"));

	wls->SendSignal("Wokjab DockWindow Close " + inittag->GetAttr("id"), &closetag);
}

void
WokjabDockWindow::on_cbutton_clicked()
{
  WokXMLTag deltag("delete");
  deltag.AddAttr("id", inittag->GetAttr("id"));
  wls->SendSignal("Wokjab DockWindow Destroy", deltag);
}

gboolean
WokjabDockWindow::key_press_handler(GtkWidget * widget, GdkEventKey * event,
			     WokjabDockWindow *c)
{
	/*
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
	 */
	return FALSE;
}

void
WokjabDockWindow::Activate()
{
	/*
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
	 */
}

void
WokjabDockWindow::SetUrgencyHint(WokXMLTag *tag)
{
	/*
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
	 */
}

bool
WokjabDockWindow::Visible()
{
	/*
	return GDL_DOCK_ITEM_ICONIFIED(win) == FALSE;
	*/
	return false;
}

void
WokjabDockWindow::Hide(WokXMLTag *tag)
{
	/*
	if ( !GDL_DOCK_ITEM_ICONIFIED(win) ) 
		gdl_dock_item_iconify_item(GDL_DOCK_ITEM(win));
	*/
}

void
WokjabDockWindow::Show(WokXMLTag *tag)
{

/*
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
	 */	
}

std::string
WokjabDockWindow::GetType()
{
	return inittag->GetAttr("type");
}


/*
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

*/
