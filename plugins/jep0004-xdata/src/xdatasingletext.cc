/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <oden@gmx.net>
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

#include "../include/xdatasingletext.h"

xdatasingletext::xdatasingletext(WLSignal *wls, WLSignalData* wlsd, GtkWidget *vbox) : xdatabase(wls, wlsd)
{
	GtkWidget *hbox;
	WokXMLTag* tag_value;
	
	hbox = gtk_hbox_new(FALSE, 1);

	entry = gtk_entry_new();
	if((tag_value = &tag_field->GetFirstTag("value")))
		gtk_entry_set_text(GTK_ENTRY(entry), tag_value->GetBody().c_str());
	
	gtk_box_pack_start(GTK_BOX(hbox), labelwid, FALSE, FALSE, 1);
	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 1);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 1);
}


xdatasingletext::~xdatasingletext()
{
	
}

std::string
xdatasingletext::GetData()
{
	return (gtk_entry_get_text(GTK_ENTRY(entry)));
}

bool
xdatasingletext::Ready()
{
	return ( ! ( gtk_entry_get_text(GTK_ENTRY(entry)) == "" && required));
}
