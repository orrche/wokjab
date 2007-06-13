/***************************************************************************
 *  Copyright (C) 2003-2006  Kent Gustavsson <nedo80@gmail.com>
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

#include "include/Roster.h"


#include <string.h>
#include <map>
#include <list>
#include <string>
#include <iostream>

Roster::Roster (WLSignal *wls):
WLSignalInstance (wls)
{
	EXP_SIGHOOK("Jabber Roster Update", &Roster::Execute, 200);
	EXP_SIGHOOK("Jabber Connection Authenticated",&Roster::SignalGetRoster, 1);
	EXP_SIGHOOK("Jabber Roster GetResource", &Roster::SignalGetResource, 1000);
	EXP_SIGHOOK("Roster Add User To Group", &Roster::SignalAddUserToGroup, 1000);
	EXP_SIGHOOK("Roster Remove User From Group", &Roster::SignalRemoveUserFromGroup, 1000);
	EXP_SIGHOOK("Roster Change JID Nickname", &Roster::SignalChangeUserNick, 1000);
	EXP_SIGHOOK("Roster Send Message To Group", &Roster::SignalSendMsgToGroup, 1000);

	EXP_SIGHOOK("Jabber Roster GetGroups", &Roster::SignalXMLGetGroups, 1000);
	EXP_SIGHOOK("Roster Get Members", &Roster::SignalXMLGetMembers, 1000);
	EXP_SIGHOOK("Roster Get All Groups", &Roster::SignalXMLGetAllGroups, 1000);
	EXP_SIGHOOK("Roster Get Roster", &Roster::SignalXMLGetRoster, 1000);
	
	EXP_SIGHOOK("Jabber XML Presence", &Roster::Presence, 100);
	EXP_SIGHOOK("Jabber Connection Lost", &Roster::Disconnect, 1000);
}

Roster::~Roster ()
{
}

int
Roster::SignalGetResource(WokXMLTag *tag)
{
	std::list <WokXMLTag*>::iterator tagiter;
	
	for( tagiter = tag->GetTagList("item").begin() ; tagiter != tag->GetTagList("item").end() ; tagiter++)
	{
		User *usr;
		usr = FindUserByJID((*tagiter)->GetAttr("session"), (*tagiter)->GetAttr("jid"));
		
		
		if(usr == NULL)
			(*tagiter)->AddAttr("presence", "unavailable");
		else
		{

			(*tagiter)->AddAttr("nick", usr->GetNick());
			std::map <std::string, Resource>::iterator riter;
			if(usr->GetResource().begin() == usr->GetResource().end() )
			{
				(*tagiter)->AddAttr("presence", "unavailable");
			}
			else
			{
				if ( (*tagiter)->GetAttr("jid").find("/") == std::string::npos )
				{
					/* This is the way you shouldn't use std::map right ? 
					*/
					std::map <int, Resource*> toorder;
					
					for( riter = usr->GetResource().begin() ; riter != usr->GetResource().end() ; riter++)
					{
						toorder[riter->second.GetPriority()] = &riter->second;
					}
					
					
					std::map <int, Resource*>::iterator rrunner;
					for( rrunner = toorder.end() ; rrunner != toorder.begin() ; )
					{	
						--rrunner;
							
						char buf[20];
						WokXMLTag &rtag = (*tagiter)->AddTag("resource");
						rtag.AddAttr("name", rrunner->second->GetResource());
						
						sprintf(buf, "%d", rrunner->second->GetPriority());
						rtag.AddAttr("priority", buf);
						rtag.AddTag("show").AddText(rrunner->second->GetShow());
						rtag.AddTag("status").AddText(rrunner->second->GetStatus());
						
					}
				}
				else
				{
					std::string res = (*tagiter)->GetAttr("jid").substr((*tagiter)->GetAttr("jid").find("/")+1);
					
					for( riter = usr->GetResource().begin() ; riter != usr->GetResource().end() ; riter++)
					{
						if ( res != riter->second.GetResource() )
							continue;
						char buf[20];
						WokXMLTag &rtag = (*tagiter)->AddTag("resource");
						rtag.AddAttr("name", riter->second.GetResource());
						
						sprintf(buf, "%d", riter->second.GetPriority());
						rtag.AddAttr("priority", buf);
						rtag.AddTag("show").AddText(riter->second.GetShow());
						rtag.AddTag("status").AddText(riter->second.GetStatus());
						
					}					
				}
			}
		}
	}
	
	return 1;
	
}

int
Roster::Disconnect(WokXMLTag *tag)
{
	std::string session = tag->GetAttr("session");
	
	{
	std::map <std::string, User *>::iterator iter;
	for ( iter = Users[session].begin() ; iter != Users[session].end() ; iter++)
		delete (iter->second);
	}
	{
	std::map <std::string, Group *>::iterator iter;
	for ( iter = Groups[session].begin() ; iter != Groups[session].end() ; iter++)
		delete (iter->second);
	}
	Users[session].clear();
	Groups[session].clear();
	
	Users.erase(session);
	Groups.erase(session);
}

void
Roster::AddUser (std::string session, User * usr)
{
	Users[session][usr->GetJID ().c_str ()] = usr;
}

void
Roster::DelUser (std::string session, User * usr)
{
	Users[session].erase (usr->GetJID ().c_str ());
	delete usr;
}

void
Roster::AddUserToGroup (std::string session, User * usr, std::string group)
{
	Group *grp;

	grp = FindGroup (session, group);

	if (grp == NULL)
	{
		grp = new Group (group.c_str ());
		Groups[session][group] = grp;
	}
	grp->AddMember (usr);
	usr->AddGroup (grp);
}

Group *
Roster::FindGroup (std::string session, std::string group)
{
	if(Groups[session].find(group) == Groups[session].end())
		return NULL;
	return (Groups[session][group]);
}

User *
Roster::FindUserByJID (std::string session, std::string jid)
{
	std::string str;
	std::string::size_type pos;

	pos = jid.find ("/", 0);
	str = jid.substr (0, pos);
	
	if(Users[session].find(str) == Users[session].end())
		return NULL;
	return (Users[session][str]);
}

int
Roster::SendRemoveUserFromGroup (std::string session, const char *grp_name, const char *jid)
{
	User *usr;
	Group *fgrp;
	std::map < std::string, Group * >grps;
	std::map < std::string, Group * >::iterator iter;
	std::string str;

	fgrp = FindGroup (session, grp_name);
	usr = FindUserByJID (session, jid);

	if (!usr && !fgrp)
		return -1;

	if (!usr->MemberOf (grp_name))
		return 0;

	grps = usr->GetGroups ();

	WokXMLTag message(NULL, "message");
	message.AddAttr("session", session);
	WokXMLTag &iqtag = message.AddTag("iq");
	iqtag.AddAttr("type","set");
	WokXMLTag &querytag = iqtag.AddTag("query");
	querytag.AddAttr("xmlns", "jabber:iq:roster");
	WokXMLTag &itemtag = querytag.AddTag("item");
	itemtag.AddAttr("jid", usr->GetJID());
	itemtag.AddAttr("name", usr->GetNick ());
	
	for (iter = grps.begin (); iter != grps.end (); iter++)
		if (iter->second != fgrp)
			itemtag.AddTag("group").AddText(iter->second->GetName());

	
	wls->SendSignal("Jabber XML Send", &message);
	return 1;
}

int
Roster::SendAddUserToGroup (std::string session, const char *grp_name, const char *jid)
{
	User *usr;
	std::map < std::string, Group * >grps;
	std::map < std::string, Group * >::iterator iter;
	std::string str;

	usr = FindUserByJID (session, jid);
	if (!usr)
		return -1;

	grps = usr->GetGroups ();


	WokXMLTag message(NULL, "message");
	message.AddAttr("session", session);
	WokXMLTag &iqtag = message.AddTag("iq");
	iqtag.AddAttr("type","set");
	WokXMLTag &querytag = iqtag.AddTag("query");
	querytag.AddAttr("xmlns", "jabber:iq:roster");
	WokXMLTag &itemtag = querytag.AddTag("item");
	itemtag.AddAttr("jid", usr->GetJID());
	itemtag.AddAttr("name", usr->GetNick ());
	
	for (iter = grps.begin (); iter != grps.end (); iter++)
		itemtag.AddTag("group").AddText(iter->second->GetName());

	itemtag.AddTag("group").AddText(grp_name);

	wls->SendSignal("Jabber XML Send", &message);
	return 1;
}

int
Roster::SendMsgToGroup (std::string session, const char *grp_name, const char *msg)
{
	std::map < std::string, User * >members;
	std::map < std::string, User * >::iterator iter;
	Group *grp;

	grp = FindGroup (session, grp_name);
	if (!grp)
		return -1;

	members = grp->GetMembers ();

	for (iter = members.begin (); iter != members.end (); iter++)
	{
		WokXMLTag msgtag(NULL, "message");
		msgtag.AddAttr("session", session);
		WokXMLTag &messagetag = msgtag.AddTag("message");
		messagetag.AddAttr("to", iter->second->GetJID());
		messagetag.AddTag("body").AddText(msg);
		
		wls->SendSignal("Jabber XML Send", &msgtag);
	}
	return 1;
}

int
Roster::Execute (WokXMLTag *tag)
{
	User *usr;
	std::string session;
	session = tag->GetAttr("session");
	WokXMLTag *tag_item;
	std::list <WokXMLTag *>::iterator runner;

	tag_item = &tag->GetFirstTag("item");
	if ((usr = FindUserByJID (session, tag_item->GetAttr("jid"))) == NULL)
	{
		usr = new User (wls, tag_item->GetAttr("jid"), tag_item->GetAttr("name"));
		AddUser (session, usr);
	}
	else
	{
		usr->SetNick(tag_item->GetAttr("name"));
	}
	usr->RemoveAllGroups ();
	
	for(runner = tag_item->GetTagList("group").begin() ; runner != tag_item->GetTagList("group").end() ; runner++)
		AddUserToGroup (session, usr, (*runner)->GetBody());
		
	return 1;
}

int
Roster::SignalXMLGetRoster(WokXMLTag *tag)
{
	std::map<std::string , std::map <std::string , User *> >::iterator sesiter(Users.begin());
	std::map<std::string , User *>::iterator useriter;
	std::map <std::string, Group *>::iterator groupiter;
	
	for(;sesiter != Users.end() ; sesiter++)
	{
		WokXMLTag &sestag = tag->AddTag("session");
		sestag.AddAttr("name", sesiter->first);
		
		for( useriter = sesiter->second.begin(); useriter != sesiter->second.end() ; useriter++)
		{
			WokXMLTag &usertag = sestag.AddTag("user");
			usertag.AddAttr("jid", useriter->first);
			usertag.AddAttr("nick", useriter->second->GetNick());
			
			std::map <std::string, Group *> list(useriter->second->GetGroups());
			
			for ( groupiter = list.begin(); groupiter != list.end() ; groupiter++)
			{
				WokXMLTag &grouptag = usertag.AddTag("group");
				grouptag.AddText(groupiter->first);
			}
		}
	}

	return true;
}

int
Roster::SignalXMLGetAllGroups(WokXMLTag *tag)
{
	std::map <std::string, Group *>::iterator iter;
	std::string session = tag->GetAttr("session");
	
	for( iter = Groups[session].begin() ; iter != Groups[session].end() ; iter++)
	{
		tag->AddTag("group").AddText(iter->first);
	}
	
	return true;
}

int
Roster::SignalXMLGetGroups(WokXMLTag *tag)
{
	std::map <std::string, Group *>::iterator iter;
	std::map <std::string, Group *> grp;
	std::string session = tag->GetAttr("session");
	
	User *usr;
	usr = FindUserByJID(session, tag->GetAttr("jid"));
	if( usr == NULL )
		return 1;
	grp = usr->GetGroups();
	
	for( iter = grp.begin() ; iter != grp.end() ; iter++)
	{
		tag->AddTag("group").AddText(iter->first);
	}
}

int
Roster::SignalXMLGetMembers(WokXMLTag *tag)
{
	std::map <std::string, User *>::iterator iter;
	std::map <std::string, User* > usr;
	std::string session = tag->GetAttr("session");
	
	Group *grp;
	grp = FindGroup(session, tag->GetAttr("group"));
	if ( grp )
	{
		usr = grp->GetMembers();
		for( iter = usr.begin() ; iter != usr.end() ; iter++)
		{
			tag->AddTag("jid").AddText(iter->first);
		}
	}
}


int
Roster::SignalChangeUserNick(WokXMLTag *tag)
{
	User *usr;
	Group *fgrp;
	std::string session = tag->GetAttr("session");
	
	std::map < std::string, Group * >grps;
	std::map < std::string, Group * >::iterator iter;
	
	std::string str;

	usr = FindUserByJID (session, tag->GetAttr("jid"));

	if (!usr && !fgrp)
		return 1;

	grps = usr->GetGroups ();

	WokXMLTag message(NULL, "message");
	message.AddAttr("session", session);
	WokXMLTag &iqtag = message.AddTag("iq");
	WokXMLTag &querytag = iqtag.AddTag("query");
	iqtag.AddAttr("type","set");
	querytag.AddAttr("xmlns", "jabber:iq:roster");
	WokXMLTag &itemtag = querytag.AddTag("item");
	itemtag.AddAttr("jid", usr->GetJID());
	itemtag.AddAttr("name", tag->GetAttr("nickname"));
	
	for (iter = grps.begin (); iter != grps.end (); iter++)
		itemtag.AddTag("group").AddText(iter->second->GetName());

	wls->SendSignal("Jabber XML Send", &message);
}

int
Roster::SignalAddUserToGroup(WokXMLTag *tag)
{
	std::string session = tag->GetAttr("session");
	SendAddUserToGroup(session, tag->GetAttr("group").c_str() , tag->GetAttr("jid").c_str());
	return 1;
}

int
Roster::SignalRemoveUserFromGroup(WokXMLTag *tag)
{
	std::string session = tag->GetAttr("session");
	
	SendRemoveUserFromGroup(session, tag->GetAttr("group").c_str() , tag->GetAttr("jid").c_str());
	return 1;
}

int
Roster::SignalSendMsgToGroup(WokXMLTag *tag)
{
	std::string session = tag->GetAttr("session");
	
	SendMsgToGroup(session, tag->GetAttr("group").c_str() , tag->GetBody().c_str());
	return 1;
}

int
Roster::SignalGetRoster(WokXMLTag *tag)
{
	WokXMLTag msgtag ( NULL, "rosterreq");
	msgtag.AddAttr("session", tag->GetAttr("session"));
	wls->SendSignal("Jabber Request Roster", msgtag);
	
	return 1;
}

int
Roster::Presence(WokXMLTag *tag)
{
	WokXMLTag *tag_presence;
	tag_presence = &tag->GetFirstTag("presence");
	std::string type = tag_presence->GetAttr("type");
	
	if( type == "" || type == "unavailable")
	{
		std::string jid;
		std::string session = tag->GetAttr("session");
		
		jid = tag_presence->GetAttr("from").substr(0, tag_presence->GetAttr("from").find("/"));

		if(Users[session].find(jid) != Users[session].end())
			Users[session][jid]->Presence(tag);
	}
	return 1;
}
