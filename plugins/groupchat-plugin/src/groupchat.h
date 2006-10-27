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

#ifndef __GROUPCHAT_H
#define __GROUPCHAT_H

#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <iostream>
#include <gtk/gtk.h>
#include <map>
#include <vector>

class GroupChat;

#include "GroupChatWidget.h"
#include "GroupChatRoster.h"
#include "GroupChatTooltip.h"

using namespace Woklib;

class GroupChat : public WoklibPlugin
{
	public:
		GroupChat(WLSignal *wls);
		~GroupChat();
		
		void InitGC();
		
		int GetMainMenu(WLSignalData *wlsd);
		int JoinGroupChatSig(WokXMLTag *tag);
		int Join(WokXMLTag *tag);
		int Message(WokXMLTag *tag);
		int CloseDialog(WokXMLTag *tag);
		int Presence(WokXMLTag *tag);
		
		void Remove(std::string session, std::string jid, const GroupChatWidget *wid);
	protected:
		std::map <std::string, std::map<std::string , GroupChatWidget *> > rooms;
		std::map <std::string, std::map<std::string , GroupChatRoster *> > rosterrooms;
		std::map <std::string , std::vector<std::string> > roomids;
		
		GroupChatTooltip *gct;
};

#endif // __GROUPCHAT_H
