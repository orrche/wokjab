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
#include "../include/xdatabase.h"


xdatabase::xdatabase(WLSignal *wls, WLSignalData* wlsd) : WLSignalInstance(wls)
{
	tag_field = static_cast <WokXMLTag*>(wlsd);
	
/*	if(tag_field->GetFirstTag("required"))
		required = true;
	else
		required = false;
	*/
	required = false;
	
	var = tag_field->GetAttr("var");
	if(tag_field->GetAttr("label").size())
		labelwid = gtk_label_new(tag_field->GetAttr("label").c_str());
	else
		labelwid = gtk_label_new(var.c_str());
	gtk_label_set_line_wrap(GTK_LABEL(labelwid), true);
	
	
}


xdatabase::~xdatabase()
{
	
}

std::string
xdatabase::GetData()
{
	return "";
}

std::string
xdatabase::GetVar()
{
	return var;
}

bool
xdatabase::Ready()
{
	return true;	
}

int 
xdatabase::xml(WLSignalData *wlsd)
{
	return 1;	
}
