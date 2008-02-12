/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <nedo80@gmail.com>
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


ToasterWindow::ToasterWindow(WLSignal *wls, WokXMLTag *config, WokXMLTag *xml, int x, int y) : WLSignalInstance(wls)
{
	orig = new WokXMLTag(*xml);
	window = gtk_window_new(GTK_WINDOW_POPUP);
	port = gtk_viewport_new(NULL, NULL);
	GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
	GtkWidget *label = gtk_label_new(xml->GetFirstTag("body").GetBody().c_str());
	gtk_label_set_line_wrap (GTK_LABEL(label), TRUE);
	
	gdk_color_parse (config->GetFirstTag("color1").GetAttr("data").c_str(), &noticable_color1);
	gdk_color_parse (config->GetFirstTag("color2").GetAttr("data").c_str(), &noticable_color2);
	
	gtk_widget_modify_bg (port, GTK_STATE_NORMAL, &noticable_color1);
																																																									
																																																									
	gtk_viewport_set_shadow_type (GTK_VIEWPORT (port), GTK_SHADOW_OUT);
	
	gtk_container_add(GTK_CONTAINER(window), port);
	gtk_container_add(GTK_CONTAINER(port), vbox);
	
	
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 2);
	std::list <WokXMLTag *> *com_list;
	std::list <WokXMLTag *>::iterator c_iter;
	
	com_list = &orig->GetTagList("commands");
	for( c_iter = com_list->begin() ; c_iter != com_list->end() ; c_iter++ )
	{
		GtkWidget *bbox;
		GtkWidget *label;
		bbox = gtk_hbox_new(FALSE, 2);
		label = gtk_label_new((*c_iter)->GetAttr("name").c_str());
		
		gtk_box_pack_start(GTK_BOX(vbox), bbox, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(bbox), label, FALSE, FALSE, 0);
		
		std::list <WokXMLTag *> *list = &(*c_iter)->GetTagList("command");
		std::list <WokXMLTag *>::iterator iter;
		for ( iter = list->begin(); iter != list->end(); iter++)
		{
			GtkWidget *button = gtk_event_box_new();
			GtkWidget *label = gtk_label_new((*iter)->GetAttr("name").c_str());
			gtk_container_add(GTK_CONTAINER(button), label);
			
			gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
			g_object_set_data(G_OBJECT(button), "xml", *iter);
			
			g_signal_connect (button , "button-press-event",	G_CALLBACK (ToasterWindow::CommandExec),this);
		}
		
	}
	gtk_widget_show_all(window);

	int height,width;
	gtk_window_get_size(GTK_WINDOW(window), &width, &height);
	gtk_window_move(GTK_WINDOW(window), x - width, y - height);

	t = atoi(config->GetFirstTag("flashes").GetAttr("data").c_str());
	timeoutid = g_timeout_add (atoi(config->GetFirstTag("fspeed").GetAttr("data").c_str()), (gboolean (*)(void *)) (ToasterWindow::Timeout), this);
		
	int delay = 0;
	if ( ( delay = atoi(config->GetFirstTag("delay").GetAttr("data").c_str())) < 1 )
		delay = 15;
	timeoutremoveid = g_timeout_add (delay*1000, (gboolean (*)(void *)) (ToasterWindow::TimeoutRemove), this);
	
	
	g_signal_connect (window , "button-press-event",	G_CALLBACK (ToasterWindow::button_press_event),this);
}


ToasterWindow::~ToasterWindow()
{
	delete orig;
	
	if ( timeoutremoveid )
		g_source_remove(timeoutremoveid);
	if ( timeoutid ) 
		g_source_remove(timeoutid);

	gtk_widget_destroy(window);
}

std::string
ToasterWindow::GetID()
{
	return orig->GetAttr("id");
}

gboolean
ToasterWindow::button_press_event(GtkWidget *widget, GdkEventButton *event, ToasterWindow *c)
{
	if ( c->orig->GetTagList("commands").empty() || c->orig->GetFirstTag("commands").GetTagList("command").empty() )
		return FALSE;
		
	if (! c->orig->GetFirstTag("commands").GetFirstTag("command").GetFirstTag("signal").GetTags().empty() )
	{
		c->wls->SendSignal(c->orig->GetFirstTag("commands").GetFirstTag("command").GetFirstTag("signal").GetAttr("name"), 
								**c->orig->GetFirstTag("commands").GetFirstTag("command").GetFirstTag("signal").GetTags().begin());
		
		WokXMLTag removesig(NULL, "remove");
		removesig.AddAttr("id", c->GetID() );
		c->wls->SendSignal("Toaster Remove", removesig);
		return TRUE;
	}
	
	return FALSE;
}

gboolean
ToasterWindow::CommandExec(GtkWidget *button, GdkEventButton *event, ToasterWindow *c)
{
	WokXMLTag *tag = static_cast <WokXMLTag *> (g_object_get_data(G_OBJECT(button), "xml"));
	if ( tag && !tag->GetFirstTag("signal").GetTags().empty() )
	{
		c->wls->SendSignal(tag->GetFirstTag("signal").GetAttr("name"), **tag->GetFirstTag("signal").GetTags().begin());
		return TRUE;
	}

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
ToasterWindow::TimeoutRemove(ToasterWindow *c)
{	
	WokXMLTag removesig(NULL, "remove");
	removesig.AddAttr("id", c->GetID() );
	
	c->timeoutremoveid = 0;
	c->wls->SendSignal("Toaster Remove", removesig);
	
	return FALSE;
}

gboolean
ToasterWindow::Timeout(ToasterWindow *c)
{
	GdkColor *color;

	if ( c->t--%2)
		color = &c->noticable_color1;
	else
		color = &c->noticable_color2;
	

	
	if ( !c->t )
	{
		gtk_widget_modify_bg (c->port, GTK_STATE_NORMAL, NULL);
		c->timeoutid = 0;
		return FALSE;
	}
	
	gtk_widget_modify_bg (c->port, GTK_STATE_NORMAL, color);
	return TRUE;
}
