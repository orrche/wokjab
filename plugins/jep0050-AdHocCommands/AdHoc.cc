/***************************************************************************
 *  Copyright (C) 2006  Kent Gustavsson <nedo80@gmail.com>
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
// Class: AdHoc
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sun Oct 15 01:03:03 2006
//

#include "AdHoc.h"
#include "AdHocWid.h"
#include "AdHocSession.h"

AdHoc::AdHoc(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber disco Feature http://jabber.org/protocol/commands", &AdHoc::Feature, 500);
	EXP_SIGHOOK("Jabber GUI GetJIDMenu", &AdHoc::Menu, 2000);
	EXP_SIGHOOK("Jabber AdHoc Start", &AdHoc::Start, 1000);
	EXP_SIGHOOK("Jabber AdHoc Execute", &AdHoc::Exec, 1000);
	
	EXP_SIGHOOK("Jabber XML IQ New command set xmlns:http://jabber.org/protocol/commands", &AdHoc::CommandExec, 1000);
	EXP_SIGHOOK("Jabber XML Message Normal", &AdHoc::Message, 200);
}


AdHoc::~AdHoc()
{


}

int
AdHoc::Message(WokXMLTag *tag)
{
	std::list<WokXMLTag *>::iterator msg_iter;
	
	for( msg_iter = tag->GetTagList("message").begin() ; msg_iter != tag->GetTagList("message").end() ; msg_iter++)
	{
		std::list<WokXMLTag *>::iterator query_iter;
		for( query_iter = (*msg_iter)->GetTagList("query").begin() ; query_iter != (*msg_iter)->GetTagList("query").end() ; query_iter++)
		{
			if ( (*query_iter)->GetAttr("xmlns") == "http://jabber.org/protocol/disco#items" &&
							 (*query_iter)->GetAttr("node") == "http://jabber.org/protocol/commands")
			{
				WokXMLTag &coms_tag = tag->AddTag("command");
				coms_tag.AddAttr("name", "Ad-Hoc");
				
				std::list<WokXMLTag *>::iterator item_iter;
				for( item_iter = (*query_iter)->GetTagList("item").begin() ; item_iter != (*query_iter)->GetTagList("item").end() ; item_iter++)
				{
					if ( (*msg_iter)->GetAttr("from") != (*item_iter)->GetAttr("jid") )
					{
						woklib_message(wls, (*msg_iter)->GetAttr("from") + " is sending you adhoc messages with a diffrent return address");
						continue;
					}
					
					WokXMLTag &com_tag = coms_tag.AddTag("command");
					com_tag.AddAttr("name", (*item_iter)->GetAttr("name"));
					
					WokXMLTag &sig_tag = com_tag.AddTag("signal");
					sig_tag.AddAttr("name", "Jabber AdHoc Execute");
					WokXMLTag &msg_tag = sig_tag.AddTag("message");
					msg_tag.AddAttr("session", tag->GetAttr("session"));
					WokXMLTag &iq_tag = msg_tag.AddTag("iq");
					iq_tag.AddAttr("type", "set");
					iq_tag.AddAttr("to", (*item_iter)->GetAttr("jid"));
					WokXMLTag &command_tag = iq_tag.AddTag("command");
					command_tag.AddAttr("xmlns", "http://jabber.org/protocol/commands");
					command_tag.AddAttr("node", (*item_iter)->GetAttr("node"));
					command_tag.AddAttr("action", "execute");
				}
			}
		}
	}

	return 1;
}

int
AdHoc::Feature(WokXMLTag *tag)
{
	new AdHocWid(wls, tag->GetAttr("session"), tag->GetAttr("jid"), "");
	return 1;
}

int
AdHoc::Start(WokXMLTag *tag)
{
	new AdHocWid(wls, tag->GetAttr("session"), tag->GetAttr("jid"), "");
	return 1;
}

int
AdHoc::Menu(WokXMLTag *tag)
{
	std::string jid = tag->GetAttr("jid");

	WokXMLTag &item = tag->AddTag("item");
	if ( jid.find("/") == std::string::npos )
	{
		WokXMLTag xml(NULL, "query");
		WokXMLTag &itag = xml.AddTag("item");
		itag.AddAttr("jid", jid);
		itag.AddAttr("session", tag->GetAttr("session"));
		wls->SendSignal("Jabber Roster GetResource", xml);
		std::string resource = itag.GetFirstTag("resource").GetAttr("name");
		
		if ( resource.empty() )
			item.AddAttr("name", "AdHoc Command");
		else
		{
			item.AddAttr("name", "AdHoc Command (" + resource + ")");
			WokXMLTag &dtag = item.AddTag("data");
			dtag.AddAttr("jid", jid+"/"+resource);
			dtag.AddAttr("session", tag->GetAttr("session"));
		}
	}
	else
		item.AddAttr("name", "AdHoc Command");
	item.AddAttr("signal", "Jabber AdHoc Start");


	return 1;
}

int
AdHoc::CommandExec(WokXMLTag *tag)
{
	wls->SendSignal("Jabber AdHoc Command Exec " + tag->GetFirstTag("iq").GetFirstTag("command").GetAttr("node"), tag);
	return 1;
}

int
AdHoc::Exec(WokXMLTag *tag)
{
		wls->SendSignal("Jabber XML IQ Send", tag);
		EXP_SIGHOOK("Jabber XML IQ ID " + tag->GetFirstTag("iq").GetAttr("id"), &AdHoc::ExecResponse, 1000);
		
		return 1;
}

int
AdHoc::ExecResponse(WokXMLTag *tag)
{
	new AdHocSession(wls,tag);
	return 1;
}
