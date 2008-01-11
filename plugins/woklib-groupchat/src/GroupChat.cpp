/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <oden@gmx.net>
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

#include "GroupChat.hpp"
#include <Woklib/WokXMLTag.h>
#include <iostream>

GroupChat::GroupChat(WLSignal *wls) : WoklibPlugin (wls)
{
	EXP_SIGHOOK("Jabber XML Presence", &GroupChat::Presence, 998);
	EXP_SIGHOOK("Jabber GroupChat Join", &GroupChat::Join, 998);
	EXP_SIGHOOK("Jabber GroupChat Leave", &GroupChat::Part, 998);
	EXP_SIGHOOK("Jabber GroupChat BanUser", &GroupChat::Ban, 998);
	EXP_SIGHOOK("Jabber GroupChat GetRooms", &GroupChat::GetRooms, 100);
}

GroupChat::~GroupChat()
{

}

int
GroupChat::GetRooms(WokXMLTag *tag)
{
	
	std::map< std::string, std::map< std::string, Room* > >::iterator sesiter;
	
	for( sesiter = rooms.begin() ; sesiter != rooms.end() ; sesiter++)
	{
		std::map< std::string, Room* >::iterator roomiter;
		for ( roomiter = sesiter->second.begin() ; roomiter != sesiter->second.end() ; roomiter++)
		{
			WokXMLTag &item = tag->AddTag("item");
			item.AddAttr("session", sesiter->first);
			item.AddAttr("roomjid", roomiter->first);			
		}		
	}
	
	return 1;	
}

int
GroupChat::Presence(WokXMLTag *tag)
{
	WokXMLTag *tag_presence;
	tag_presence = &tag->GetFirstTag("presence");
	
	int pos1,pos2;
	
	std::string server;
	std::string session;
	std::string nick;
	std::string room;
	std::string from;
	
	from = tag_presence->GetAttr("from");
	pos1 = from.find("@");
	pos2 = from.find("/");
	
	session = tag->GetAttr("session");
	server = from.substr(pos1+1, pos2-pos1-1);
	nick = from.substr(pos2+1, from.length() - pos2 - 1);
	room = from.substr(0, pos1);
	
	tag->AddAttr("nick", nick);
	tag->AddAttr("server", server);
	tag->AddAttr("room", room);
	
	
	if( rooms[session].find(room + "@" + server) != rooms[session].end() )
	{
		wls->SendSignal("Jabber GroupChat Presence", tag);
		wls->SendSignal("Jabber GroupChat Presence '" + XMLisize(tag->GetAttr("session")) + "' '" + XMLisize(room + "@" + server) + "'", tag);
		wls->SendSignal("Jabber GroupChat Presence '" + XMLisize(tag->GetAttr("session")) + "' '" + XMLisize(tag_presence->GetAttr("from")) + "'", tag);
	}
	return 1;
}

int
GroupChat::Join(WokXMLTag *tag)
{
	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", tag->GetAttr("session"));
	WokXMLTag &presencetag = msgtag.AddTag("presence");
	presencetag.AddAttr("to", tag->GetAttr("room") + '@' + tag->GetAttr("server") + '/' + tag->GetAttr("nick"));
	WokXMLTag &xtag = presencetag.AddTag("x");
	xtag.AddAttr("xmlns", "http://jabber.org/protocol/muc");
	
	wls->SendSignal("Jabber XML Send", msgtag);
	if ( rooms[tag->GetAttr("session")].find(tag->GetAttr("room") + '@' + tag->GetAttr("server")) == rooms[tag->GetAttr("session")].end())
		rooms[tag->GetAttr("session")][tag->GetAttr("room") + '@' + tag->GetAttr("server")] = new Room(wls, tag);
	else
		woklib_message(wls,"Great now your joining a room your already in like you haven't been causing enough problems for today already");
	
	return 1;
}

int
GroupChat::Part(WokXMLTag *tag)
{
	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", tag->GetAttr("session"));
	WokXMLTag &presencetag = msgtag.AddTag("presence");
	presencetag.AddAttr("to", tag->GetAttr("room") + '@' + tag->GetAttr("server") + '/' + tag->GetAttr("nick"));
	presencetag.AddAttr("type", "unavailable");
	
	wls->SendSignal("Jabber XML Send", msgtag);
	
	if ( rooms[tag->GetAttr("session")].find(tag->GetAttr("room") + '@' + tag->GetAttr("server")) == rooms[tag->GetAttr("session")].end())
		woklib_message(wls,"And how did you manage to trick the client into leaving a room you aint in ?");
	else
	{
		delete rooms[tag->GetAttr("session")][tag->GetAttr("room") + '@' + tag->GetAttr("server")];
		rooms[tag->GetAttr("session")].erase(tag->GetAttr("room") + '@' + tag->GetAttr("server"));
		if ( rooms[tag->GetAttr("session")].empty() )
			rooms.erase(tag->GetAttr("session"));

	}
	
	return 1;
}

int
GroupChat::Ban(WokXMLTag *tag)
{
	return 1;
}
