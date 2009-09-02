/***************************************************************************
 *  Copyright (C) 2003-2009  Kent Gustavsson <oden@gmx.net>
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

#include "User.h"
#include <string.h>
#include <iostream>
#include <Woklib/WokXMLTag.h>

User::User(WLSignal *wls,std::string jid, std::string nick):
WLSignalInstance (wls)
{
	this->jid = jid;
	this->nick = nick;
	
	std::string signal;
}

User::~User()
{
	RemoveAllGroups();
}

void 
User::AddGroup(Group *grp)
{
	Groups[grp->GetName()] = grp;
}

void 
User::RemoveAllGroups()
{
	std::map < std::string, Group * >::iterator iter;
	for (iter = Groups.begin (); iter != Groups.end (); iter++)
		iter->second->RemoveMember(this);
	Groups.erase(Groups.begin(),Groups.end());
}

bool
User::MemberOf(const char *group)
{
	if (Groups.find(group) != Groups.end())
		return true;
	return false;
}

std::map <std::string, Group *>
User::GetGroups()
{
	return(Groups);
}

std::map <std::string, Resource> &
User::GetResource()
{
	return Resources;
}


bool
User::is(const char *comp)
{
	return( jid == comp );
}

std::string
User::GetJID()
{
	return(jid);
}

std::string
User::GetNick()
{
	return(nick);
}

void
User::SetNick( const std::string &name)
{
	nick = name;
}

void
User::Presence(WokXMLTag* tag)
{
	std::string resource;
	WokXMLTag *tag_presence;
	tag_presence = &tag->GetFirstTag("presence");
	
	if( tag_presence->GetAttr("from").find("/") == std::string::npos)
		resource = "";
	else
		resource = tag_presence->GetAttr("from").substr(tag_presence->GetAttr("from").find("/") + 1);
	
	if(tag_presence->GetAttr("type") == "unavailable")
	{
		Resources.erase(resource);
		return;
	}
	
	/* Why the hell have I done it this way ?!? */
	Resources[resource].SetStatus(tag_presence->GetFirstTag("status").GetBody());
	Resources[resource].SetPriority(atoi(tag_presence->GetFirstTag("priority").GetBody().c_str()));
	Resources[resource].SetResource(resource);
	Resources[resource].SetShow(tag_presence->GetFirstTag("show").GetBody());
}
