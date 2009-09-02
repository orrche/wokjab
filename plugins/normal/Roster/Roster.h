/***************************************************************************
 *  Copyright (C) 2003-2006  Kent Gustavsson <nedo@gmail.com>
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

#ifndef __ROSTER_H
#define __ROSTER_H

class Roster;
	
#include <map>
#include <string>
#include <Woklib/WLSignal.h>

#include "User.h"
#include "Group.h"

	
class Roster : WoklibPlugin
{
	public:
	Roster(WLSignal *wls);
	~Roster();
	
	void AddUser(std::string session, User *usr);
	void DelUser(std::string session, User *usr);
	User *FindUserByJID(std::string session,  std::string jid);
	
	void AddUserToGroup(std::string session,  User *usr, std::string group);
	
	Group *FindGroup(std::string session,  std::string group );
	
	int SendRemoveUserFromGroup(std::string session,  const char *grp, const char *jid );
	int SendAddUserToGroup(std::string session,  const char *grp, const char *jid);
	int SendMsgToGroup(std::string session,  const char *grp, const char *msg);
	
	// Signal handlers
	int Execute (WokXMLTag *tag);

	int SignalGetResource(WokXMLTag *tag);
	int SignalAddUserToGroup(WokXMLTag *);
	int SignalRemoveUserFromGroup(WokXMLTag *);
	
	int SignalXMLGetGroups(WokXMLTag *);
	int SignalXMLGetAllGroups(WokXMLTag *);
	int SignalXMLGetMembers(WokXMLTag *);
	int SignalXMLGetRoster(WokXMLTag *);
	
	int SignalSendMsgToGroup(WokXMLTag *);
	int SignalGetRoster(WokXMLTag *);
	int SignalChangeUserNick(WokXMLTag *tag);
	int Presence(WokXMLTag *);
	int Disconnect(WokXMLTag *tag);
	private:
	std::map <std::string , std::map <std::string , Group *> > Groups;
	std::map <std::string , std::map <std::string , User *> > Users;
};

#endif // __ROSTER_H
