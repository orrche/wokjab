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


filepicker::filepicker(WLSignal *wls, const std::string &session, const std::string &to) : WLSignalInstance(wls),
session(session),
to(to)
{
	GtkWidget *main_vbox;
	GtkWidget *okbutton;
	GtkWidget *cancelbutton;
	GtkWidget *buttonbox;

	okbutton = gtk_button_new_with_mnemonic("_OK");
	cancelbutton = gtk_button_new_with_mnemonic("_Cancel");
	buttonbox = gtk_hbutton_box_new();
	
	main_vbox = gtk_vbox_new(false, false);
	chooser = gtk_file_chooser_widget_new(GTK_FILE_CHOOSER_ACTION_OPEN);
	
	gtk_box_pack_start(GTK_BOX(buttonbox), okbutton, false, false, 2);
	gtk_box_pack_start(GTK_BOX(buttonbox), cancelbutton, false, false, 2);
	gtk_box_pack_start(GTK_BOX(main_vbox), chooser, true, true, 2);
	gtk_box_pack_start(GTK_BOX(main_vbox), buttonbox, false, false, 2);
	
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_add(GTK_CONTAINER(window), main_vbox);
	gtk_window_set_title (GTK_WINDOW (window), "File to send");
	
	g_signal_connect_swapped (G_OBJECT (cancelbutton), "clicked",
			      G_CALLBACK (gtk_widget_destroy),
                              G_OBJECT (window));
	g_signal_connect (G_OBJECT (window), "destroy",
					(void(*)())G_CALLBACK (filepicker::Destroy), this);
	g_signal_connect (G_OBJECT (okbutton), "clicked",
					G_CALLBACK(filepicker::ButtonPress), this);
					
	gtk_window_set_default_size(GTK_WINDOW(window), 500, 300);
	
	gtk_widget_show_all(window);
}


filepicker::~filepicker()
{
	
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
	std::cout << tag << std::endl;
	
	tag.AddAttr("name", gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(c->chooser)));
	
	std::cout << tag << std::endl;
	
	c->wls->SendSignal("Jabber Stream File Send", &tag);
	gtk_widget_destroy(c->window);
}
