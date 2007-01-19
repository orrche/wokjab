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
// Class: JabberSession
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sat Sep  2 14:20:43 2006
//

#include "JabberSession.h"


JabberSession::JabberSession(WLSignal *wls, WokXMLTag *tag) : WLSignalInstance(wls),
session(tag->GetAttr("session"))
{
	EXP_SIGHOOK("Jabber Roster Update " + session , &JabberSession::UpdateRoster, 1000);
	
	std::string name = tag->GetAttr("username") + "@" + tag->GetAttr("server") + "/" + tag->GetAttr("resource");
	
	WokXMLTag itemtag(NULL, "item");
	WokXMLTag &columntag =  itemtag.AddTag("columns");
	WokXMLTag &texttag = columntag.AddTag("text");
	texttag.AddText(name);
		
	wls->SendSignal("GUIRoster AddItem", itemtag);
	id = itemtag.GetAttr("id");
}


JabberSession::~JabberSession()
{
		WokXMLTag tag(NULL, "item");
		tag.AddAttr("id", id);
		wls->SendSignal("GUIRoster RemoveItem", tag);
}

int
JabberSession::UpdateRoster(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator iter;
	
	for ( iter = tag->GetTagList("item").begin() ; iter != tag->GetTagList("item").end() ; iter++ )
	{
		if ( user.find((*iter)->GetAttr("jid") ) != user.end() )
			user[(*iter)->GetAttr("jid")]->Update(*iter);
		else
			user[(*iter)->GetAttr("jid")] = new User(wls, *iter, this);
	}
	return 1;
}

std::string
JabberSession::GetID()
{
	return id;
}

std::string
JabberSession::GetSession()
{
	return session;
}

std::string
JabberSession::AddToGroup(std::string groupname, User *usr)
{
	if (group.find(groupname) == group.end() )
	{
		group[groupname] = new Group(wls, groupname, id);
	}
	group[groupname]->AddUser();
	return group[groupname]->GetID();
}

void
JabberSession::RemoveFromGroup(std::string groupname)
{
	if ( group[groupname]->RemoveUser() == 0 )
	{
		delete group[groupname];
		group.erase(groupname);
	}
}

void
JabberSession::AddEvent(WokXMLTag *tag)
{
	std::string jid = tag->GetAttr("jid").substr(0, tag->GetAttr("jid").find("/"));
	
	if ( user.find(jid) != user.end() )
	{
		user[jid]->AddEvent(tag);
	}
}

void
JabberSession::RemoveEvent(WokXMLTag *tag)
{
	std::string jid = tag->GetAttr("jid").substr(0, tag->GetAttr("jid").find("/"));
	
	if ( user.find(jid) != user.end() )
	{
		user[jid]->RemoveEvent(tag);
	}
}
