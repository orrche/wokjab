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

#include "../include/Group.h"

#include <string.h>
#include <iostream>

Group::Group (const char *name)
{
	this->name = name;
}

Group::~Group ()
{
}

void
Group::AddMember (User * usr)
{
	members[usr->GetJID ().c_str ()] = usr;
}

void
Group::RemoveMember (User * usr)
{
	members.erase (usr->GetJID ().c_str ());
}

std::string Group::GetName ()
{
	return (name);
}

bool Group::is (const char *comp)
{
	return (name == comp);
}

std::map < std::string,
User * >
Group::GetMembers ()
{
	return (members);
}

void
Group::PrintMembers ()		/// Debug
{
	std::map < std::string , User * >::iterator iter;

	for (iter = members.begin (); iter != members.end (); iter++)
	{
		std::cout << "Member of  group " << name << " : " << iter->second->
			GetJID () << std::endl;
	}
}
