/***************************************************************************
 *  Copyright (C) 2003-2008  Kent Gustavsson <oden@gmx.net>
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

#include "groupchat.h"
#include "GroupChatJoinWindow.h"

GroupChat::GroupChat(WLSignal *wls):
WoklibPlugin(wls)
{	
	EXP_SIGHOOK("Jabber GroupChat Join", &GroupChat::Join, 999);
	EXP_SIGHOOK("Jabber GroupChat Presence", &GroupChat::Presence, 999);

	EXP_SIGHOOK("Jabber XML Message GroupChat", &GroupChat::Message, 999);
	//EXP_SIGHOOK("close_muc_dialog", &GroupChat::CloseDialog, 999);
	EXP_SIGHOOK("GetMenu", &GroupChat::GetMainMenu, 1000);
	EXP_SIGHOOK("GroupChat Join Activate", &GroupChat::JoinGroupChatSig, 1000);
	
	gct = new GroupChatTooltip(wls);
}

GroupChat::~GroupChat()
{
	delete gct;
}

void
GroupChat::InitGC()
{
	EXP_SIGHOOK("Jabber GroupChat Join", &GroupChat::Join, 999);
	EXP_SIGHOOK("Jabber GroupChat Presence", &GroupChat::Presence, 999);

	EXP_SIGHOOK("Jabber XML Message GroupChat", &GroupChat::Message, 999);
	//EXP_SIGHOOK("close_muc_dialog", &GroupChat::CloseDialog, 999);
	EXP_SIGHOOK("GetMenu", &GroupChat::GetMainMenu, 1000);
	EXP_SIGHOOK("GroupChat Join Activate", &GroupChat::JoinGroupChatSig, 1000);

}

int
GroupChat::GetMainMenu(WLSignalData *wlsd)
{
	WokXMLTag *menu_tag;
	WokXMLTag *menu_item;
	menu_tag = static_cast<WokXMLTag *> (wlsd);
	
	menu_item = &menu_tag->AddTag("item");
	menu_item->AddAttr("name", "Join a groupchat");
	menu_item->AddAttr("signal", "GroupChat Join Activate");
	
	return 1;
}

int
GroupChat::JoinGroupChatSig(WokXMLTag *tag)
{
	new GroupChatJoinWindow( wls );
	return true;
}
/*
void
GroupChat::JoinGroupChat(GtkMenuItem *menuitem,gpointer user_data)
{
	GroupChat *data;
	data = static_cast < GroupChat * > ( user_data );
	new GroupChatJoinWindow( data->wls );
}
*/
int
GroupChat::Join(WokXMLTag *tag)
{
	GroupChatWidget *wid;
	std::string roomjid = std::string(tag->GetAttr("room") + '@' + tag->GetAttr("server"));
	wid = new GroupChatWidget(wls, tag->GetAttr("session"), roomjid, tag->GetAttr("nick"), this);
	
	char buf[40];
	
	WokXMLTag contag(NULL, "connect");
	contag.AddAttr("identifier", roomjid);
	contag.AddAttr("session", tag->GetAttr("session"));
	sprintf(buf, "%d", wid->GetWidget());
	contag.AddAttr("mainwidget", buf);
	sprintf(buf, "%d", wid->GetLabel());
	contag.AddAttr("labelwidget", buf);
	contag.AddAttr("close_signal", "close_muc_dialog");
	contag.AddAttr("minimize", tag->GetAttr("minimize"));

	wls->SendSignal("GUI WindowDock AddWidget",&contag);

	EXP_SIGHOOK(std::string("GUI WindowDock Close ") + contag.GetAttr("mainwidget"), &GroupChat::CloseDialog, 500);
//	EXP_SIGHOOK(std::string("GUI WindowDock ReConnect ") + contag.GetAttr("mainwidget"), &GroupChat::ReConnect, 500);
	
	rooms[tag->GetAttr("session")][roomjid] = wid;
	rosterrooms[tag->GetAttr("session")][roomjid] = new GroupChatRoster(wls, roomjid, contag.GetAttr("mainwidget"), tag->GetAttr("nick"));
	roomids[contag.GetAttr("mainwidget")].push_back(tag->GetAttr("session"));
	roomids[contag.GetAttr("mainwidget")].push_back(roomjid);
	
	
	return 1;
}

int
GroupChat::Message(WokXMLTag *tag)
{
	WokXMLTag &msgtag = tag->GetFirstTag("message");
	
	std::string roomjid = msgtag.GetAttr("from").substr(0, msgtag.GetAttr("from").find("/"));
	
	if( rooms[tag->GetAttr("session")].find(roomjid) != rooms[tag->GetAttr("session")].end() )
	{
		rosterrooms[tag->GetAttr("session")][roomjid]->Message(&msgtag);
		rooms[tag->GetAttr("session")][roomjid]->Message(msgtag);
	}
	else
		std::cout << "Strange got message not assosiated with a room" << std::endl;
	
	return 1;	
}

int
GroupChat::CloseDialog(WokXMLTag *tag)
{
	std::string id = tag->GetAttr("id");
	std::string session = roomids[id][0];
	std::string jid = roomids[id][1];
	
	if(rooms[session].find(jid) != rooms[session].end())
	{
		delete(rooms[session][jid]);
		rooms[tag->GetAttr("session")].erase(jid);
		roomids.erase(id);
		
		delete(rosterrooms[session][jid]);
		rosterrooms[session].erase(jid);
	}

	return 1;
}

int
GroupChat::Presence(WokXMLTag *tag)
{
	WokXMLTag *tag_presence = &tag->GetFirstTag("presence");
	
	std::string roomjid = tag_presence->GetAttr("from").substr(0, tag_presence->GetAttr("from").find("/"));
	
	if(rooms[tag->GetAttr("session")].find(roomjid) != rooms[tag->GetAttr("session")].end())
		rooms[tag->GetAttr("session")][roomjid]->Presence(tag);
		
	
	return 1;
}

void
GroupChat::Remove(std::string session, std::string jid, const GroupChatWidget *wid)
{
	rooms[session].erase(jid);
	if ( rooms[session].size() == 0 )
		rooms.erase(session);
		
	/*
	delete rosterrooms[session][jid];
	rosterrooms[session].erase(jid);
	*/
}
