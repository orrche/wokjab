/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <nedo80@gmail.com>
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


#include "AutoJoin.h"


AutoJoin::AutoJoin(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Get Main Menu", &AutoJoin::Menu, 1000);
	EXP_SIGHOOK("AutoGroupChatJoinerGUI Preference", &AutoJoin::CreateWid, 1000);
}


AutoJoin::~AutoJoin()
{
	std::list <AutoJoinWid*>::iterator iter;
	for ( iter = widlist.begin() ; iter != widlist.end() ; iter++)
	{
		delete *iter;
	}
}

void 
AutoJoin::RemoveWid(AutoJoinWid *wid)
{
	widlist.erase(find(widlist.begin(), widlist.end(), wid));
}

int
AutoJoin::CreateWid(WokXMLTag *xml)
{
	new AutoJoinWid(wls, xml, this);
	
}

int
AutoJoin::Menu(WokXMLTag *xml)
{
	WokXMLTag &settingstag = xml->AddTag("item");
	settingstag.AddAttr("name", "Settings");
	
	WokXMLTag &autojoin = settingstag.AddTag("item");
	autojoin.AddAttr("name", "Auto Joiner");
	// This I haven't got working yet atleast in the menu but when I do it should start working
	autojoin.AddAttr("signal", "AutoGroupChatJoinerGUI Preference");
			
	WokXMLTag querytag(NULL,"session");
	wls->SendSignal("Jabber GetSessions", &querytag);
				
	std::list <WokXMLTag *>::iterator iter;
	
	for( iter = querytag.GetTagList("item").begin() ; iter != querytag.GetTagList("item").end() ; iter++)
	{
		WokXMLTag &session = autojoin.AddTag("item");
		session.AddAttr("name", (*iter)->GetAttr("name"));
		session.AddAttr("signal", "AutoGroupChatJoinerGUI Preference");
		session.AddTag("data").AddTag("session").AddText((*iter)->GetAttr("name"));
	}
				
	return true;
}

