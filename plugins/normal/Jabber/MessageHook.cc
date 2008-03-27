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

#include <iostream>

#include "MessageHook.h"

MessageHook::MessageHook (WLSignal * wls):
WLSignalInstance (wls)
{
	EXP_SIGHOOK("Jabber XML Object message", &MessageHook::xml, 1000);
	EXP_SIGHOOK("Jabber XML Message Send", &MessageHook::send, 1000);
	EXP_SIGHOOK("Jabber XML Message Send", &MessageHook::devide, 500);
}

MessageHook::~MessageHook ()
{	
}

int
MessageHook::xml(WokXMLTag *data)
{
	std::string type;
	std::list<WokXMLTag *> tag_list;
	std::list<WokXMLTag *>::iterator iter;
	
	type = data->GetFirstTag("message").GetAttr("type");
	
	WokXMLTag *body = &data->GetFirstTag("message").GetFirstTag("body");
	if (body->GetBody()[0] == '!' )
	{
		std::string command = body->GetBody().substr(1,body->GetBody().find(" ")-1);
		wls->SendSignal("Jabber XML Message Command " + command, data);
	}
	if (type.length() == 0 || type== "chat" || type== "error" || type== "normal")
		wls->SendSignal("Jabber XML Message Normal", data);
	else if ( type =="groupchat")
		wls->SendSignal("Jabber XML Message GroupChat", data);
	
	tag_list = data->GetFirstTag("message").GetTagList("x");
		
	for(iter = tag_list.begin(); iter != tag_list.end(); iter++)
		wls->SendSignal("Jabber XML Message xmlns " + (*iter)->GetAttr("xmlns"), data);
	
	return 1;
}

int
MessageHook::devide(WokXMLTag *tag)
{
	wls->SendSignal("Jabber XML Message To '" + XMLisize(tag->GetAttr("session")) + "' '" + XMLisize(tag->GetFirstTag("message").GetAttr("to")) + "'", tag);
	return 1;	
}

int
MessageHook::send(WokXMLTag *tag)
{
	if ( tag->GetAttr("send") != "false" )
	{
		WokXMLTag tosend("message");
		tosend.AddAttr("session", tag->GetAttr("session"));
		tosend.AddTag(& tag->GetFirstTag("message"));
		wls->SendSignal("Jabber XML Send", tosend);
	}
	return 1;
}
