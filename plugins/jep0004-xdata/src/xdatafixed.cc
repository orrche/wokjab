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

#include "../include/xdatafixed.h"
#include <iostream>

xdatafixed::xdatafixed(WLSignal *wls, WLSignalData* wlsd, GtkWidget *vbox) : xdatabase(wls, wlsd)
{
	std::string value = "Uhm strange .. ";
	
	value = tag_field->GetFirstTag("value").GetBody();
//	labelwid = gtk_label_new(value.c_str());
	gtk_label_set_text(GTK_LABEL(labelwid), value.c_str());
	gtk_label_set_line_wrap(GTK_LABEL(labelwid), true);
	gtk_box_pack_start(GTK_BOX(vbox) , labelwid, false, false, 0);
}


xdatafixed::~xdatafixed()
{
	
}
