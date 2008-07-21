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
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sat Sep  2 14:20:43 2006
//

#ifndef _JABBERSESSION_H_
#define _JABBERSESSION_H_

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

using namespace Woklib;
class JabberSession;

#include <map>
#include "User.h"
#include "Group.h"
#include "JabberRoster.h"


class JabberSession : public WLSignalInstance
{
	public:
		JabberSession(WLSignal *wls, WokXMLTag *tag, JabberRoster *c);
		 ~JabberSession();

		void RemoveFromGroup(std::string groupname);
		void AddEvent(WokXMLTag *tag);
		std::string AddToGroup(std::string group, User *usr);
		std::string GetID();
		std::string GetSession();
		int UpdateAll();
		
		int UpdateRoster(WokXMLTag *tag);
		int RightButton(WokXMLTag *tag);
	
		JabberRoster *parent;
	protected:
		std::string id,session,serverjid;
		std::map <std::string, User*> user;
		std::map <std::string, Group *> group;
};


#endif	//_JABBERSESSION_H_

