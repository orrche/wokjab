/***************************************************************************
 *  Copyright (C) 2005  Kent Gustavsson <nedo80@gmail.com>
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
// Class: filepicker
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Aug 23 19:51:51 2005
//

#include "filepicker.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

filepicker::filepicker(WLSignal *wls, const std::string &session, const std::string &to) : WLSignalInstance(wls),
session(session),
to(to)
{
	gxml = glade_xml_new (PACKAGE_GLADE_DIR"/wokjab/filepicker.glade", NULL, NULL);

	
	g_signal_connect_swapped (G_OBJECT (glade_xml_get_widget (gxml, "cancel_button")), "clicked",
			      G_CALLBACK (gtk_widget_destroy),
                              G_OBJECT (glade_xml_get_widget (gxml, "window")));
	g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "window")), "destroy",
					(void(*)())G_CALLBACK (filepicker::Destroy), this);
	g_signal_connect (G_OBJECT (glade_xml_get_widget (gxml, "ok_button")), "clicked",
					G_CALLBACK(filepicker::ButtonPress), this);
					
}


filepicker::~filepicker()
{
	g_object_unref(gxml);
}

void
filepicker::Destroy(GtkWidget * widget, filepicker *c)
{
	delete c;
}

void 
filepicker::ButtonPress (GtkButton *button, filepicker *c)
{
	WokXMLTag tag(NULL, "send");
	tag.AddAttr("to", c->to);
	tag.AddAttr("session", c->session);
	
	tag.AddAttr("name", gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(glade_xml_get_widget (c->gxml, "chooser"))));
	std::string rate = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (c->gxml, "rateentry")));
	if ( atoi(rate.c_str()) > 0 )
		tag.AddAttr("rate", rate + "000");
	tag.AddAttr("proxy", gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (c->gxml, "proxyentry"))));
	
	c->wls->SendSignal("Jabber Stream File Send", &tag);
	gtk_widget_destroy(glade_xml_get_widget (c->gxml, "window"));
}
