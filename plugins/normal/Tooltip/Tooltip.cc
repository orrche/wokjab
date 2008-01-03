/***************************************************************************
 *  Copyright (C) 2006  Kent Gustavsson <nedo80@gmail.com>
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
// Class: Tooltip
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sun Feb 12 20:06:04 2006
//

#include "Tooltip.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

Tooltip::Tooltip(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber Tooltip Set", &Tooltip::Set, 500);
	EXP_SIGHOOK("Jabber Tooltip Reset", &Tooltip::Reset, 500);
	
	xml = NULL;
	currenttag = NULL;
}


Tooltip::~Tooltip()
{
	if ( xml )
	{
		gtk_widget_destroy(glade_xml_get_widget (xml, "window"));
		g_object_unref ( xml );
	}
	if ( currenttag )
		delete currenttag;
	xml = NULL;
}

gboolean
Tooltip::Expose(GtkWidget *widget, GdkEventExpose *event, Tooltip *c)
{
	gtk_paint_flat_box(widget->style, widget->window, GTK_STATE_NORMAL, GTK_SHADOW_OUT,
					   NULL, widget, "tooltip", 0, 0, -1, -1);
	return FALSE;
}

gboolean 
Tooltip::DispWindow (Tooltip * c)
{
	if ( c->xml )
	{
		gtk_widget_destroy(glade_xml_get_widget (c->xml, "window"));
		g_object_unref ( c->xml );
	}
	c->xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/tooltip.glade", "window", NULL);
	
	{
		WokXMLTag querytag(NULL, "query");
		WokXMLTag &itemtag = querytag.AddTag("item");
		itemtag.AddAttr("session", c->currenttag->GetAttr("session"));
		c->wls->SendSignal("Jabber Connection GetUserData", &querytag);
	
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (c->xml, "session_label")), (itemtag.GetFirstTag("jid").GetBody() + " [" + c->currenttag->GetAttr("session") + "]").c_str());
	}
	gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (c->xml, "jid_label")), c->currenttag->GetAttr("jid").c_str());
	
	WokXMLTag querytag(NULL, "query");
	WokXMLTag &itemtag = querytag.AddTag("item");
	
	itemtag.AddAttr("jid", c->currenttag->GetAttr("jid"));
	itemtag.AddAttr("session", c->currenttag->GetAttr("session"));
	
	c->wls->SendSignal("Jabber GUI GetIcon", &querytag);
	
	
	gtk_image_set_from_file(GTK_IMAGE(glade_xml_get_widget (c->xml, "status_img")),itemtag.GetAttr("icon").c_str());
	if ( itemtag.GetAttr("avatar").size() )
		gtk_image_set_from_file(GTK_IMAGE(glade_xml_get_widget (c->xml, "avatar_img")),itemtag.GetAttr("avatar").c_str());
	else
		gtk_widget_hide(glade_xml_get_widget (c->xml, "avatar_img"));
	
	gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (c->xml, "nick_label")), itemtag.GetAttr("nick").c_str());
	gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (c->xml, "show_label")), 
											itemtag.GetFirstTag("resource").GetFirstTag("show").GetBody().c_str());
	
	std::string statusmsg = itemtag.GetFirstTag("resource").GetFirstTag("status").GetBody();
	
	WokXMLTag entries("entries");	
	entries.AddAttr("jid", c->currenttag->GetAttr("jid"));
	entries.AddAttr("session", c->currenttag->GetAttr("session"));
	c->wls->SendSignal("Jabber UserActivityGet", entries);
	c->wls->SendSignal("Jabber UserActivityGet " + c->currenttag->GetAttr("session") + " '" + XMLisize(c->currenttag->GetAttr("jid")) + "'", entries);
	
	gtk_table_resize(GTK_TABLE(glade_xml_get_widget (c->xml, "table") ), 5 + entries.GetTagList("item").size(), 2);
	
	std::list <WokXMLTag *>::iterator entry_iter;
	int n = 0;
	for( entry_iter = entries.GetTagList("item").begin() ; entry_iter != entries.GetTagList("item").end() ; entry_iter++)
	{
		/*
		statusmsg += "\n";
		statusmsg += (*entry_iter)->GetFirstTag("line").GetBody();	
		*/
		GtkWidget *line = gtk_label_new((*entry_iter)->GetFirstTag("line").GetBody().c_str());
		GtkWidget *type = gtk_label_new(((*entry_iter)->GetAttr("type_name")+":").c_str());
		
		gtk_table_attach_defaults(GTK_TABLE(glade_xml_get_widget (c->xml, "table") ),
                                                         type,
                                                         0,
                                                         1,
                                                         4+n,
                                                         5+n);
		gtk_table_attach_defaults(GTK_TABLE(glade_xml_get_widget (c->xml, "table") ),
                                                         line,
                                                         1,
                                                         2,
                                                         4+n,
                                                         5+n);
		n++;
	}
	gtk_widget_show_all(glade_xml_get_widget (c->xml, "table"));
	
	gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (c->xml, "status_label")), 
											statusmsg.c_str());
											
	c->id = 0;
	
	gint x,y,w,h;
	GtkWindow *window = GTK_WINDOW(glade_xml_get_widget (c->xml, "window"));
	gtk_window_get_size(window, &w, &h);
	gtk_window_get_position(window, &x, &y);
	y+=h/2+10;
	gtk_window_move(window, x, y);
	
	g_signal_connect(G_OBJECT(window), "expose_event",
			G_CALLBACK(Tooltip::Expose), c);
	
	gtk_widget_set_app_paintable(GTK_WIDGET(window), TRUE);
	gtk_widget_set_name(GTK_WIDGET(window), "gtk-tooltips");
	gtk_widget_ensure_style (GTK_WIDGET(window));
	
	gtk_widget_show(GTK_WIDGET(window));
	
	return FALSE;
}

int
Tooltip::Set(WokXMLTag &tag)
{
	if( id )
		g_source_remove(id);
	
	if ( currenttag ) 
		delete currenttag;
	currenttag = new WokXMLTag (tag);
	
	if( !xml )
		id = g_timeout_add (1000, (gboolean (*)(void *)) (Tooltip::DispWindow), this);
	else
		DispWindow(this);
	return true;
}

int
Tooltip::Reset(WokXMLTag &tag)
{
	if( id )
		g_source_remove(id);
	if ( xml )
	{
		gtk_widget_destroy(glade_xml_get_widget (xml, "window"));
		g_object_unref ( xml );
	}
	xml = NULL;
	return true;
}

