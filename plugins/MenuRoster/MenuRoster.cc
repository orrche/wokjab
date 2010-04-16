/***************************************************************************
 *  Copyright (C) 2003-2006  Kent Gustavsson <oden@gmx.net>
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
// Class: MenuRoster
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Jan 31 18:41:35 2006
//

#include "MenuRoster.h"


MenuRoster::MenuRoster(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber Menu Roster", &MenuRoster::CreateRoster, 1000);
	EXP_SIGHOOK("Get Main Menu", &MenuRoster::CreateRoster, 1000);
}


MenuRoster::~MenuRoster()
{

}

void
MenuRoster::AddUserEntry(WokXMLTag *pos, WokXMLTag *user, std::string session)
{
	WokXMLTag querytag(NULL, "query");
	WokXMLTag &itemtag = querytag.AddTag("item");
	itemtag.AddAttr("jid", user->GetAttr("jid"));
	itemtag.AddAttr("session", session);

	wls->SendSignal("Jabber GUI Roster View", &querytag );
	if( itemtag.GetAttr("view") != "true" )
		return;
	wls->SendSignal("Jabber GUI GetIcon", &querytag );

	WokXMLTag &usertag = pos->AddTag("item");
	if ( user->GetAttr("nick").size () )
		usertag.AddAttr("name", user->GetAttr("nick"));
	else
		usertag.AddAttr("name", user->GetAttr("jid"));
	usertag.AddAttr("icon", itemtag.GetAttr("icon"));
	usertag.AddAttr("jid", user->GetAttr("jid"));
	usertag.AddAttr("session", session);
	
	WokXMLTag &datatag = usertag.AddTag("data");
	datatag.AddAttr("jid", user->GetAttr("jid"));
	datatag.AddAttr("session", session);
	
	wls->SendSignal("Jabber GUI Roster GetJIDMenu", &usertag);
	wls->SendSignal("Jabber GUI GetJIDMenu", &usertag);
}

int 
MenuRoster::CreateRoster(WokXMLTag *tag)
{
	std::map<std::string, std::map<std::string, WokXMLTag *> > groupitem;

	WokXMLTag &item = tag->AddTag("item");
	item.AddAttr("name", "Roster");
	
	WokXMLTag roster(NULL, "roser");
	wls->SendSignal("Roster Get Roster", &roster);
	
	std::list<WokXMLTag *> list = roster.GetTagList("session");
	std::list<WokXMLTag *>::iterator listiter;
	
	for( listiter = list.begin() ; listiter != list.end(); listiter++)
	{
		WokXMLTag &sestag = item.AddTag("item");

		WokXMLTag querytag(NULL, "query");
		WokXMLTag &itemtag = querytag.AddTag("item");
		itemtag.AddAttr("session", (**listiter).GetAttr("name"));
		wls->SendSignal("Jabber Connection GetUserData", &querytag);
		
		sestag.AddAttr("name", itemtag.GetFirstTag("jid").GetBody());
		
		std::list<WokXMLTag *> userlist = (**listiter).GetTagList("user");
		std::list<WokXMLTag *>::iterator useriter(userlist.begin());
		for ( ; useriter != userlist.end() ; useriter++)
		{
			if( (**useriter).GetTagList("group").size () == 0 )
				AddUserEntry(&sestag, *useriter, (**listiter).GetAttr("name"));
			else
			{
				std::list<WokXMLTag *> grouplist = (**useriter).GetTagList("group");
				std::list<WokXMLTag *>::iterator groupiter(grouplist.begin());


				for( ;groupiter != grouplist.end(); groupiter++)
				{
					if( groupitem[(**listiter).GetAttr("name")].find( (**groupiter).GetBody() ) == groupitem[(**listiter).GetAttr("name")].end() )
					{
						groupitem[(**listiter).GetAttr("name")][ (**groupiter).GetBody() ] = &sestag.AddTag("item");
						groupitem[(**listiter).GetAttr("name")][ (**groupiter).GetBody() ]->AddAttr("name", (**groupiter).GetBody());
					}
					
					AddUserEntry(groupitem[(**listiter).GetAttr("name")][(**groupiter).GetBody()], *useriter, (**listiter).GetAttr("name"));
				}
			}
		}
		
	}
	
	return true;
}


