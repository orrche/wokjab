/***************************************************************************
 *  Copyright (C) 2005  Kent Gustavsson <oden@gmx.net>
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
// Class: GtkPBool
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sun Aug 14 20:49:48 2005
//

#include "GtkPBool.h"

GtkPBool::GtkPBool(WokXMLTag *tag) : GtkPCommon(tag)
{
	std::string name;
	if( tag->GetAttr("label").size() )
		name = tag->GetAttr("label");
	else
		name = tag->GetName();
		
	check = gtk_check_button_new_with_label (name.c_str());

	if ( tag->GetAttr("data") == "false")
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check) , FALSE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check) , TRUE);
}


GtkPBool::~GtkPBool()
{
	
}

GtkWidget *
GtkPBool::GetWidget()
{
	return check;
}

void
GtkPBool::Save()
{
	if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check)))
		conf->AddAttr("data", "true");
	else
		conf->AddAttr("data", "false");
}
