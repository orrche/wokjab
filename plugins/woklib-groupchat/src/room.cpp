/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2007 <nedo80@gmail.com>
 * 
 * wokjab is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * wokjab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with wokjab.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#include "room.hpp"


Room::Room(WLSignal *wls, WokXMLTag *xml) : WLSignalInstance(wls), 
origxml(new WokXMLTag(*xml))
{
	EXP_SIGHOOK("Jabber GroupChat Presence '" + XMLisize(origxml->GetAttr("session")) + "' '" + XMLisize(origxml->GetAttr("room") + '@' + origxml->GetAttr("server")) + "'", &Room::Presence, 1000);
	EXP_SIGHOOK("Jabber GroupChat GetOccupants '" + XMLisize(origxml->GetAttr("session")) + "' '" + XMLisize(origxml->GetAttr("room") + '@' + origxml->GetAttr("server")) + "'", &Room::GetOccupants, 1000);
	EXP_SIGHOOK("Jabber GroupChat Whoami '" + XMLisize(origxml->GetAttr("session")) + "' '" + XMLisize(origxml->GetAttr("room") + '@' + origxml->GetAttr("server")) + "'", &Room::Whoami, 1000);
}

int
Room::Whoami(WokXMLTag *tag)
{
	tag->AddAttr("nick", origxml->GetAttr("nick"));
	return 1;
}

int
Room::GetOccupants(WokXMLTag *tag)
{
	std::map <std::string, User* >::iterator useriter;
	
	for( useriter = users.begin() ; useriter != users.end() ; useriter++)
	{
		WokXMLTag &item = tag->AddTag("item");
		item.AddAttr("nick", useriter->first);
		item.AddTag(&(useriter->second->GetUserData()));
	}
	
	return 1;
}

int
Room::Presence(WokXMLTag *tag)
{
	std::string from = tag->GetFirstTag("presence").GetAttr("from");
	std::string nick = from.substr(from.find("/")+1);
	
	if ( tag->GetFirstTag("presence").GetAttr("type") == "unavailable" )
	{
		if ( users.find(nick) != users.end())
		{
			delete users[nick];
			users.erase(nick);
		}
	}	
	else if ( users.find(nick) == users.end())
	{
		users[nick] = new User(wls, tag);
	}
	else
		users[nick]->Update(tag);
	
	return 1;
}

void
Room::SetAffiliation(std::string username, std::string affiliation)
{
	
	
}
