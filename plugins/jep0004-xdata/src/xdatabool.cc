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
#include "../include/xdatabool.h"


xdatabool::xdatabool(WLSignal *wls, WLSignalData* wlsd, GtkWidget *vbox) : xdatabase(wls, wlsd)
{
	checkbutton = gtk_check_button_new_with_label(tag_field->GetAttr("label").c_str());
	gtk_box_pack_start(GTK_BOX(vbox), checkbutton, false, false, 0 );
	
	if(tag_field->GetFirstTag("value").GetBody()[0] == '1')
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton), true);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton), false);
}


xdatabool::~xdatabool()
{
}

std::string
xdatabool::GetData()
{
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton)))
		return "1";
	return "0";
}
