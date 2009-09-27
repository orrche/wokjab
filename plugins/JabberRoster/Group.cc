/***************************************************************************
 *  Copyright (C) 2003-2006  Kent Gustavsson <nedo80@gmail.com>
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
// Class: Group
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sat Sep  2 15:49:31 2006
//

#include "Group.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


Group::Group(WLSignal *wls, std::string in_name, std::string in_sessionid, JabberSession *ses) : WLSignalInstance(wls),
name(in_name),
sessionid(in_sessionid),
ses(ses)
{	
	expanded = true;
	if ( name[0] == '.' )
	{
		id = "";
	}
	else
	{
		WokXMLTag itemtag("item");
		GenerateLine (itemtag);
		
		wls->SendSignal("GUIRoster AddItem", itemtag);
		id = itemtag.GetAttr("id");
		EXP_SIGHOOK("GUIRoster Activate " + id, &Group::Activate, 500);
	}

	
	usr = 0;
}

Group::~Group()
{
	if ( ! id.empty() )
	{
		WokXMLTag tag(NULL, "item");
		tag.AddAttr("id", id);
		
		wls->SendSignal("GUIRoster RemoveItem", tag);
	}
}

std::string 
Group::GetID()
{
	return id;
}

int
Group::Activate(WokXMLTag *tag)
{
	if ( expanded )
		expanded = false;
	else
		expanded = true;
	UpdateRow();
	return 1;
}

void
Group::GenerateLine(WokXMLTag &line)
{
	line.AddAttr("parant", sessionid);
	WokXMLTag &columntag =  line.AddTag("columns");
	WokXMLTag &texttag = columntag.AddTag("text");
	if ( expanded )
		columntag.AddTag("pre_pix").AddText(PACKAGE_DATA_DIR"/wokjab/group.png");
	else
		columntag.AddTag("pre_pix").AddText(PACKAGE_DATA_DIR"/wokjab/collapsed.group.png");
	

	WokXMLTag parse("parse");
	std::string markup = ses->parent->config->GetFirstTag("groupmarkup").GetBody();
	parse.Add("<markup>"+markup+"</markup>");

	
	WokXMLTag &vars = parse.AddTag("variables");
	vars.AddTag("name").AddText(XMLisize(name));
	
	wls->SendSignal("Wokjab XMLMarkup Parse", parse);
	
	texttag.AddText(parse.GetFirstTag("output").GetBody());
}

void
Group::UpdateRow()
{
	WokXMLTag itemtag(NULL, "item");
	GenerateLine(itemtag);
	
	std::map <std::string, std::string>::iterator iter;

	if ( expanded )
		itemtag.AddAttr("expanded", "true");
	else
		itemtag.AddAttr("expanded", "false");
		
	itemtag.AddAttr("id", id);
	wls->SendSignal("GUIRoster UpdateItem", itemtag);
	
}

void
Group::AddUser()
{
	usr++;
}

int
Group::RemoveUser()
{
	usr--;
	return usr;
}
