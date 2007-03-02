/***************************************************************************
 *  Copyright (C) 2003-2006  Kent Gustavsson <nedo80@gmail.com>
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
#include "../include/xdatamultitext.h"

xdatamultitext::xdatamultitext(WLSignal *wls, WLSignalData* wlsd, GtkWidget *vbox) : xdatabase(wls, wlsd)
{
	GtkWidget *hbox;
	GtkWidget *scroll1;
	
	std::string value = tag_field->GetFirstTag("value").GetBody();
	
	
	hbox = gtk_hbox_new(FALSE, 1);
	textview = gtk_text_view_new();
	scroll1 = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scroll1), textview);
	
	gtk_box_pack_start(GTK_BOX(hbox), labelwid, FALSE, FALSE, 1);
	gtk_box_pack_start(GTK_BOX(hbox), scroll1, TRUE, TRUE, 1);
	
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 1);
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(textview));
	
	gtk_text_buffer_set_text(buffer, value.c_str(), value.size());
	required = false;
}


xdatamultitext::~xdatamultitext()
{
	
}

void
xdatamultitext::GetData(WokXMLTag &tag)
{
	GtkTextIter start_iter, end_iter;
		
	gtk_text_buffer_get_start_iter(buffer, &start_iter);
	gtk_text_buffer_get_end_iter(buffer, &end_iter);
		
	tag.AddTag("value").AddText(gtk_text_buffer_get_text(buffer,&start_iter, &end_iter, false));
	return;
}

bool
xdatamultitext::Ready()
{
	GtkTextIter start_iter, end_iter;
		
	gtk_text_buffer_get_start_iter(buffer, &start_iter);
	gtk_text_buffer_get_end_iter(buffer, &end_iter);
	return ( ! ( gtk_text_buffer_get_text(buffer,&start_iter, &end_iter, false) == "" && required));
}
