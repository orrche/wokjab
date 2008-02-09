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
	EXP_SIGHOOK("Jabber GroupChat GetJIDMenu", &GroupChat::Menu, 1000);
	
	EXP_SIGHOOK("Jabber GroupChat SetRole", &GroupChat::SetRole, 500);
	
	EXP_SIGHOOK("Jabber GroupChat Menu SetRole none", &GroupChat::Menu_none, 1000);
	EXP_SIGHOOK("Jabber GroupChat Menu SetRole visitor", &GroupChat::Menu_visitor, 1000);
	EXP_SIGHOOK("Jabber GroupChat Menu SetRole participant", &GroupChat::Menu_participant, 1000);
	EXP_SIGHOOK("Jabber GroupChat Menu SetRole moderator", &GroupChat::Menu_mod, 1000);
}

GroupChat::~GroupChat()
{

}


int
GroupChat::SetRole(WokXMLTag *tag)
{
	wls->SendSignal("Jabber GroupChat SetRole " + tag->GetAttr("role"), tag);
	
	if ( tag->GetAttr("send") != "false" )
	{
		WokXMLTag mtag("message");
		mtag.AddAttr("session", tag->GetAttr("session"));
		
		WokXMLTag &iq = mtag.AddTag("iq");
		iq.AddAttr("to", tag->GetAttr("roomjid"));
		iq.AddAttr("type", "set");
		
		WokXMLTag &query = iq.AddTag("query", "http://jabber.org/protocol/muc#admin");
		WokXMLTag &item = query.AddTag("item");
		item.AddAttr("nick", tag->GetAttr("nick"));
		item.AddAttr("role", tag->GetAttr("role"));
		
		wls->SendSignal("Jabber XML IQ Send", mtag);
	}
	return 1;	
}

int
GroupChat::Menu_none(WokXMLTag *tag)
{
	WokXMLTag none("role");
	none.AddAttr("nick", tag->GetAttr("jid").substr(tag->GetAttr("jid").rfind("/")+1));
	none.AddAttr("roomjid", tag->GetAttr("jid").substr(0, tag->GetAttr("jid").rfind("/")));
	none.AddAttr("session", tag->GetAttr("session"));
	none.AddAttr("role", "none");
	
	wls->SendSignal("Jabber GroupChat SetRole", none);
	return 1;
}

int
GroupChat::Menu_visitor(WokXMLTag *tag)
{
	WokXMLTag visitor("role");
	visitor.AddAttr("nick", tag->GetAttr("jid").substr(tag->GetAttr("jid").rfind("/")+1));
	visitor.AddAttr("roomjid", tag->GetAttr("jid").substr(0, tag->GetAttr("jid").rfind("/")));
	visitor.AddAttr("session", tag->GetAttr("session"));
	visitor.AddAttr("role", "visitor");
	
	wls->SendSignal("Jabber GroupChat SetRole", visitor);
	return 1;
}

int
GroupChat::Menu_participant(WokXMLTag *tag)
{
	WokXMLTag participant("role");
	participant.AddAttr("nick", tag->GetAttr("jid").substr(tag->GetAttr("jid").rfind("/")+1));
	participant.AddAttr("roomjid", tag->GetAttr("jid").substr(0, tag->GetAttr("jid").rfind("/")));
	participant.AddAttr("session", tag->GetAttr("session"));
	participant.AddAttr("role", "participant");
	
	wls->SendSignal("Jabber GroupChat SetRole", participant);
	return 1;
}

int
GroupChat::Menu_mod(WokXMLTag *tag)
{
	WokXMLTag mod("role");
	mod.AddAttr("nick", tag->GetAttr("jid").substr(tag->GetAttr("jid").rfind("/")+1));
	mod.AddAttr("roomjid", tag->GetAttr("jid").substr(0, tag->GetAttr("jid").rfind("/")));
	mod.AddAttr("session", tag->GetAttr("session"));
	mod.AddAttr("role", "moderator");
	
	wls->SendSignal("Jabber GroupChat SetRole", mod);
	return 1;
}

int
GroupChat::Menu(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator iter;
	WokXMLTag *admintag = NULL;
	for ( iter = tag->GetTagList("item").begin() ; iter != tag->GetTagList("item").end() ; iter++)
	{
		if ( (*iter)->GetAttr("name") == "Admin")
		{
			admintag = *iter;
			break;
		}
	}
	if( !admintag )
	{
		admintag = &tag->AddTag("item");
		admintag->AddAttr("name", "Admin");
	}
	
	WokXMLTag *setroletag = NULL;
	for ( iter = admintag->GetTagList("item").begin() ; iter != admintag->GetTagList("item").end() ; iter++)
	{
		if ( (*iter)->GetAttr("name") == "Set Role")
		{
			setroletag = *iter;
			break;
		}
	}
	if( !setroletag )
	{
		setroletag = &admintag->AddTag("item");
		setroletag->AddAttr("name", "Set Role");
	}
	
	WokXMLTag &none = setroletag->AddTag("item");
	none.AddAttr("name", "none (kick)");
	none.AddAttr("signal", "Jabber GroupChat Menu SetRole none");
	
	WokXMLTag &visitor = setroletag->AddTag("item");
	visitor.AddAttr("name", "visitor (devoice)");
	visitor.AddAttr("signal", "Jabber GroupChat Menu SetRole visitor");
	
	WokXMLTag &ptag = setroletag->AddTag("item");
	ptag.AddAttr("name", "participant (voice)");
	ptag.AddAttr("signal", "Jabber GroupChat Menu SetRole participant");
	
	WokXMLTag &mod = setroletag->AddTag("item");
	mod.AddAttr("name", "moderator (OP)");
	mod.AddAttr("signal", "Jabber GroupChat Menu SetRole moderator");
	
	/*
	
	WokXMLTag &kick = admintag->AddTag("item");
	kick.AddAttr("name", "Ban");
	tag->AddAttr("signal", "Jabber GroupChat Menu Ban");
	
	WokXMLTag &kick = admintag->AddTag("item");
	kick.AddAttr("name", "UnBan");
	tag->AddAttr("signal", "Jabber GroupChat Menu UnBan");
	*/
	
	return 1;
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
