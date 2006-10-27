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
// Class: GroupChatTooltip
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Thu Oct 12 14:00:25 2006
//

#include "GroupChatTooltip.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

GroupChatTooltip::GroupChatTooltip(WLSignal *wls) : WLSignalInstance(wls)
{
	EXP_SIGHOOK("Jabber GroupChatTooltip Set", &GroupChatTooltip::Set, 500);
	EXP_SIGHOOK("Jabber GroupChatTooltip Reset", &GroupChatTooltip::Reset, 500);
	
	xml = NULL;
	currenttag = NULL;
}


GroupChatTooltip::~GroupChatTooltip()
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
GroupChatTooltip::Expose(GtkWidget *widget, GdkEventExpose *event, GroupChatTooltip *c)
{
	gtk_paint_flat_box(widget->style, widget->window, GTK_STATE_NORMAL, GTK_SHADOW_OUT,
					   NULL, widget, "tooltip", 0, 0, -1, -1);
	return FALSE;
}

gboolean 
GroupChatTooltip::DispWindow (GroupChatTooltip * c)
{
	if ( c->xml )
	{
		gtk_widget_destroy(glade_xml_get_widget (c->xml, "window"));
		g_object_unref ( c->xml );
	}
	c->xml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/groupchattooltip.glade", "window", NULL);
		
	c->id = 0;
	
	gint x,y,w,h;
	GtkWindow *window = GTK_WINDOW(glade_xml_get_widget (c->xml, "window"));
	std::string data;
	data = "<span>" + c->currenttag->GetAttr("room") + "</span>";
	
	std::list <WokXMLTag *>::iterator tagiter;
	for(tagiter = c->currenttag->GetFirstTag("message").GetTagList("body").begin() ;
			tagiter != c->currenttag->GetFirstTag("message").GetTagList("body").end() ;
			tagiter++)
	{
		data += (*tagiter)->GetBody();
	}
	gtk_label_set_markup(GTK_LABEL(glade_xml_get_widget(c->xml, "mainlabel")), data.c_str());
	
	gtk_window_get_size(window, &w, &h);
	gtk_window_get_position(window, &x, &y);
	y+=h/2+10;
	gtk_window_move(window, x, y);
	
	g_signal_connect(G_OBJECT(window), "expose_event",
			G_CALLBACK(GroupChatTooltip::Expose), c);
	
	gtk_widget_set_app_paintable(GTK_WIDGET(window), TRUE);
	gtk_widget_set_name(GTK_WIDGET(window), "gtk-tooltips");
	gtk_widget_ensure_style (GTK_WIDGET(window));
	
	gtk_widget_show(GTK_WIDGET(window));
	
	return FALSE;
}

int
GroupChatTooltip::Set(WokXMLTag &tag)
{
	if( id )
		g_source_remove(id);
	
	if ( currenttag ) 
		delete currenttag;
	currenttag = new WokXMLTag (tag);
	
	if( !xml )
		id = g_timeout_add (1000, (gboolean (*)(void *)) (GroupChatTooltip::DispWindow), this);
	else
		DispWindow(this);
	return true;
}

int
GroupChatTooltip::Reset(WokXMLTag &tag)
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

