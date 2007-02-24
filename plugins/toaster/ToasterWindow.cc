/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <nedo80@gmail.com>
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
// Class: ToasterWindow
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Aug  8 16:29:20 2006
//

#include "ToasterWindow.h"


ToasterWindow::ToasterWindow(WLSignal *wls, WokXMLTag *xml, int x, int y) : WLSignalInstance(wls)
{
	orig = new WokXMLTag(*xml);
	window = gtk_window_new(GTK_WINDOW_POPUP);
	port = gtk_viewport_new(NULL, NULL);
	GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
	GtkWidget *label = gtk_label_new(xml->GetFirstTag("body").GetBody().c_str());
	gtk_label_set_line_wrap (GTK_LABEL(label), TRUE);
	
	gdk_color_parse ("red", &noticable_color);
	gtk_widget_modify_bg (port, GTK_STATE_NORMAL, &noticable_color);
																																																									
																																																									
	gtk_viewport_set_shadow_type (GTK_VIEWPORT (port), GTK_SHADOW_OUT);
	
	gtk_container_add(GTK_CONTAINER(window), port);
	gtk_container_add(GTK_CONTAINER(port), vbox);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 2);
		
	gtk_widget_show_all(window);

	int height,width;
	gtk_window_get_size(GTK_WINDOW(window), &width, &height);
	gtk_window_move(GTK_WINDOW(window), x - width, y - height);

	t = 10;
	g_timeout_add (200, (gboolean (*)(void *)) (ToasterWindow::Timeout), this);
	
	g_signal_connect (window , "button-press-event",	G_CALLBACK (ToasterWindow::button_press_event),this);
}


ToasterWindow::~ToasterWindow()
{
	delete orig;
	gtk_widget_destroy(window);
}

gboolean
ToasterWindow::button_press_event(GtkWidget *widget, GdkEventButton *event, ToasterWindow *c)
{
	std::cout << "Good this thing to work actually..." << std::endl;
	if ( c->orig->GetTagList("commands").empty() || c->orig->GetFirstTag("commands").GetTagList("command").empty() )
		return FALSE;
		
	c->wls->SendSignal(c->orig->GetFirstTag("commands").GetFirstTag("command").GetFirstTag("signal").GetAttr("name"), 
								**c->orig->GetFirstTag("commands").GetFirstTag("command").GetFirstTag("signal").GetTags().begin());
	
	return FALSE;
}


int
ToasterWindow::GetHeight()
{
	int height;
	
	gtk_window_get_size(GTK_WINDOW(window), NULL, &height);
	
	return height;
}

void
ToasterWindow::MoveTo(int x, int y)
{
	int height,width;
	gtk_window_get_size(GTK_WINDOW(window), &width, &height);
	
	gtk_window_move(GTK_WINDOW(window), x-width, y-height);
}

gboolean
ToasterWindow::Timeout(ToasterWindow *c)
{
	if ( c->t--%2)
	{
		gdk_color_parse ("red", &c->noticable_color);
	}
	else
	{
		gdk_color_parse ("blue", &c->noticable_color);
	}
	

	
	if ( !c->t )
	{
		gtk_widget_modify_bg (c->port, GTK_STATE_NORMAL, NULL);
		return FALSE;
	}
	
	gtk_widget_modify_bg (c->port, GTK_STATE_NORMAL, &c->noticable_color);
	return TRUE;
}
