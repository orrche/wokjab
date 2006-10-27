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
// Class: GtkPPassword
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Fri Sep  9 18:03:32 2005
//

#include "GtkPPassword.h"


GtkPPassword::GtkPPassword(WokXMLTag *tag) : GtkPCommon(tag)
{
	std::string name;
	if( tag->GetAttr("label").size() )
		name = tag->GetAttr("label");
	else
		name = tag->GetName();
		
	hbox = gtk_hbox_new(FALSE, 2);
	GtkWidget *label = gtk_label_new(name.c_str());
	entry = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
	gtk_entry_set_text(GTK_ENTRY(entry), tag->GetAttr("data").c_str());
	
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);
}


GtkPPassword::~GtkPPassword()
{
	// TODO: put destructor code here
}

GtkWidget *
GtkPPassword::GetWidget()
{
	return hbox;
}

void
GtkPPassword::Save()
{
	conf->AddAttr("data", gtk_entry_get_text(GTK_ENTRY(entry)));
}

