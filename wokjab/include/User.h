/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <oden@gmx.net>
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


#ifndef __USER_H
#define __USER_H

class User;

#include "Group.h"
#include "Resource.h"
#include <map>
#include <string>
#include <list>

#include <Woklib/WokLib.h>
using namespace Woklib;

class User : WLSignalInstance
{
	public:
	User(WLSignal* wls,std::string jid, std::string nick);
	~User();
	
	void AddGroup(Group* grp);
	void RemoveAllGroups();
	
	std::string GetNick();
	void	SetNick(const std::string &name);
	bool is(const char* comp);
	bool MemberOf(const char* group);
	std::string GetJID();
	std::map <std::string, Group* > GetGroups();
	void Presence(WokXMLTag *tag);
	std::map <std::string, Resource> & GetResource();
	
	private:
	std::string nick;
	std::string jid;
	std::map <std::string, Group* > Groups;
	std::map <std::string, Resource> Resources;
};

#endif // __USER_H
