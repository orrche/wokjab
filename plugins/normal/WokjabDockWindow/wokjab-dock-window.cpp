/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2008-2011 <nedo80@gmail.com>
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

WokjabDockWindow::WokjabDockWindow(WLSignal *wls, WokXMLTag *in_inittag, WokjabDockWindowMaster *parent) : WLSignalInstance(wls),
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
	parent->GetNotebook()->append_page(mainsock, label_box);

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

	if (event->state & GDK_MOD1_MASK)
	{
		if(event->keyval >= '0' && event->keyval <= '9') {
			int switchto = event->keyval - '1';
			if(switchto == -1)
				switchto = 9;
			
			c->parent->GetNotebook()->set_current_page(switchto);
		}
	}

	return FALSE;
}

void
WokjabDockWindow::Activate()
{
	parent->Raise();

}

void
WokjabDockWindow::SetUrgencyHint(WokXMLTag *tag)
{

	if ( tag->GetFirstTag("urgency").GetAttr("data") != "false" )
		parent->SetUrgency(true);
	else
		parent->SetUrgency(false);
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

std::string
WokjabDockWindow::GetID()
{
	return inittag->GetAttr("id");
}
