/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <nedo80@gmail.com>
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
#include "../include/xdatasinglelist.h"


xdatasinglelist::xdatasinglelist(WLSignal *wls, WLSignalData* wlsd, GtkWidget *vbox)
: xdatabase(wls, wlsd)
{
	std::string value;
	std::list <WokXMLTag*>::iterator iter;
	int selected_item=0;
	
	singlist = gtk_option_menu_new();
	menu = gtk_menu_new ();
	
	value = tag_field->GetFirstTag("value").GetBody();

	int i= 0;
	for(iter = tag_field->GetTagList("option").begin();iter != tag_field->GetTagList("option").end();iter++)
	{
		std::string menu_value;
		std::string menu_label;
		
		menu_value = (*iter)->GetFirstTag("value").GetBody();
		if(menu_value.size() == 0)
			menu_value = (*iter)->GetAttr("label");
		
		menu_label = (*iter)->GetAttr("label");
		if(menu_label.size() == 0)
			menu_label = (*iter)->GetFirstTag("value").GetBody();
		
		
		GtkWidget *menu_item;
		menu_item = gtk_menu_item_new_with_label (menu_label.c_str());
		gtk_container_add (GTK_CONTAINER (menu), menu_item);
			
		
		if ( value == menu_value ) 
			selected_item = i;
		menu_map[i] = menu_value;
		i++;
		
	}
	
	gtk_option_menu_set_menu (GTK_OPTION_MENU (singlist), menu);
	gtk_box_pack_start(GTK_BOX(vbox), labelwid, false, false, 0);
	gtk_box_pack_start(GTK_BOX(vbox), singlist, false, false, 0);
	gtk_option_menu_set_history(GTK_OPTION_MENU(singlist), selected_item);
}


xdatasinglelist::~xdatasinglelist()
{

}

std::string
xdatasinglelist::GetData()
{
	return(menu_map[gtk_option_menu_get_history(GTK_OPTION_MENU(singlist))]);	
}
