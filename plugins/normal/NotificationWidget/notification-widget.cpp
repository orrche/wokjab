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

#include "notification-widget.hpp"


NotificationWidget::NotificationWidget(WLSignal *wls, WokXMLTag *tag) : WLSignalInstance(wls), 
origxml(new WokXMLTag(*tag))
{
	box = gtk_vbox_new(FALSE, 2);
	GtkWidget *label = gtk_label_new(tag->GetFirstTag("body").GetBody().c_str());
	gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
	
		
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);	
	gtk_widget_set_tooltip_text(box,tag->GetFirstTag("body").GetBody().c_str());
	
	std::list <WokXMLTag *> *com_list;
	std::list <WokXMLTag *>::iterator c_iter;
	
	com_list = &origxml->GetTagList("commands");
	for( c_iter = com_list->begin() ; c_iter != com_list->end() ; c_iter++ )
	{
		GtkWidget *bbox;
		GtkWidget *label;
		bbox = gtk_vbox_new(FALSE, 2);
		label = gtk_label_new((*c_iter)->GetAttr("name").c_str());
		
		gtk_box_pack_start(GTK_BOX(box), bbox, FALSE, FALSE, 0);
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
			
			g_signal_connect (button , "button-press-event",	G_CALLBACK (NotificationWidget::CommandExec),this);
		}
		
	}
	
}

gboolean
NotificationWidget::CommandExec(GtkWidget *button, GdkEventButton *event, NotificationWidget *c)
{
	
	WokXMLTag *tag = static_cast <WokXMLTag *> (g_object_get_data(G_OBJECT(button), "xml"));
	
	std::cout << "What the fuck.." << std::endl;
	
	if ( tag && !tag->GetFirstTag("signal").GetTags().empty() )
	{
		c->wls->SendSignal(tag->GetFirstTag("signal").GetAttr("name"), **tag->GetFirstTag("signal").GetTags().begin());
		return TRUE;
	}

	return FALSE;
}

std::string
NotificationWidget::GetId()
{
	return origxml->GetAttr("id");	
}


GtkWidget *
NotificationWidget::GetWidget()
{
	
	return box;
}
