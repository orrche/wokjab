/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <nedo80@gmail.com>
 ****************************************************************************/
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#include "renamewidget.h"


RenameWidget::RenameWidget(WLSignal *wls, const std::string &jid, const std::string &session):
jid(jid),
session(session),
wls(wls)
{
	GtkWidget *jid_label;
	GtkWidget *button;
	GtkWidget *vbox;
	
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Rename JID");

	vbox = gtk_vbox_new(FALSE, FALSE);

	jid_label = gtk_label_new_with_mnemonic(jid.c_str());
	button = gtk_button_new_with_label ("OK");
	alias_entry = gtk_entry_new();
	
	gtk_box_pack_start(GTK_BOX(vbox), jid_label, FALSE, 0,0);
	gtk_box_pack_start(GTK_BOX(vbox), alias_entry, FALSE, 0,0);
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, 0,0);
	
	gtk_container_add (GTK_CONTAINER (window), vbox);
	
	g_signal_connect (G_OBJECT (button), "clicked",
		      G_CALLBACK (RenameWidget::ButtonPress), this);
	g_signal_connect (G_OBJECT (window), "destroy",
					G_CALLBACK (RenameWidget::Destroy), this);
	gtk_widget_show_all(window);
}

RenameWidget::~RenameWidget()
{
	
}

void
RenameWidget::Destroy (GtkWidget * widget, RenameWidget *c)
{
	delete c;
}

void
RenameWidget::ButtonPress(GtkWidget *widget, RenameWidget *c)
{
	WokXMLTag tag(NULL, "changeto");
	tag.AddAttr("jid", c->jid);
	tag.AddAttr("session", c->session);
	tag.AddAttr("nickname", gtk_entry_get_text(GTK_ENTRY(c->alias_entry)));
	c->wls->SendSignal("Roster Change JID Nickname", tag);
	
	gtk_widget_destroy (c->window);
}
