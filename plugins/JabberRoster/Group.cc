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


Group::Group(WLSignal *wls, std::string name, std::string sessionid) : WLSignalInstance(wls)
{	
	WokXMLTag itemtag(NULL, "item");
	itemtag.AddAttr("parant", sessionid);
	WokXMLTag &columntag =  itemtag.AddTag("columns");
	WokXMLTag &texttag = columntag.AddTag("text");
	texttag.AddText(name);
		
	wls->SendSignal("GUIRoster AddItem", itemtag);
	id = itemtag.GetAttr("id");
	
	usr = 0;
}

Group::~Group()
{
	WokXMLTag tag(NULL, "item");
	tag.AddAttr("id", id);
	
	wls->SendSignal("GUIRoster RemoveItem", tag);
}

std::string 
Group::GetID()
{
	return id;
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
