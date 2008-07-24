/***************************************************************************
 *  Copyright (C) 2003-2008  Kent Gustavsson <nedo80@gmail.com>
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
// Class: GroupChatRoster
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Wed Sep  6 18:30:45 2006
//

#include "GroupChatRoster.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

GroupChatRoster::GroupChatRoster(WLSignal *wls, std::string room, std::string mainwid, std::string mynick) : WLSignalInstance(wls),
room(room),
mainwid(mainwid),
mynick(mynick)
{
	WokXMLTag itemtag(NULL, "item");
	WokXMLTag &columntag =  itemtag.AddTag("columns");
	WokXMLTag &texttag = columntag.AddTag("text");
	columntag.AddTag("pre_pix").AddText(PACKAGE_DATA_DIR"/wokjab/groupchat/wokjab/gicon.png");
	texttag.AddText(room);
		
	wls->SendSignal("GUIRoster AddItem", itemtag);
	id = itemtag.GetAttr("id");
	EXP_SIGHOOK("GUIRoster Activate " + id, &GroupChatRoster::Activate, 1000);
	EXP_SIGHOOK("GUIRoster Tooltip Set " + id, &GroupChatRoster::Tooltip, 1000);
	EXP_SIGHOOK("GUIRoster Tooltip Reset " + id, &GroupChatRoster::TooltipReset, 1000);
}


GroupChatRoster::~GroupChatRoster()
{
	WokXMLTag tag(NULL, "item");
	tag.AddAttr("id", id);
	
	wls->SendSignal("GUIRoster RemoveItem", tag);
}

void
GroupChatRoster::Display()
{

	WokXMLTag itemtag(NULL, "item");
	itemtag.AddAttr("id", id);
	
	wls->SendSignal("GUIRoster RemoveItem", itemtag);

	WokXMLTag &columntag =  itemtag.AddTag("columns");
	WokXMLTag &texttag = columntag.AddTag("text");
	columntag.AddTag("pre_pix").AddText(PACKAGE_DATA_DIR"/wokjab/groupchat/wokjab/gicon.png");
	texttag.AddText(room);

	std::list<std::string>::iterator iter;
	int i=0;
	for ( iter = message.begin() ; iter != message.end() ; iter++)
	{
		if ( message.size() - i++ < 5 )
			texttag.AddText(*iter);
	}
	
	wls->SendSignal("GUIRoster AddItem", itemtag);
	EXP_SIGUNHOOK("GUIRoster Activate " + id, &GroupChatRoster::Activate, 1000);
	EXP_SIGUNHOOK("GUIRoster Tooltip Set " + id, &GroupChatRoster::Tooltip, 1000);
	EXP_SIGUNHOOK("GUIRoster Tooltip Reset " + id, &GroupChatRoster::TooltipReset, 1000);
	id = itemtag.GetAttr("id");
	EXP_SIGHOOK("GUIRoster Activate " + id, &GroupChatRoster::Activate, 1000);
	EXP_SIGHOOK("GUIRoster Tooltip Set " + id, &GroupChatRoster::Tooltip, 1000);
	EXP_SIGHOOK("GUIRoster Tooltip Reset " + id, &GroupChatRoster::TooltipReset, 1000);
}

int
GroupChatRoster::Activate(WokXMLTag *tag)
{
	WokXMLTag widget(NULL, "widget");
	widget.AddAttr("id", mainwid);
	
	wls->SendSignal("Wokjab DockWindow Activate", widget);
	return 1;
}

int
GroupChatRoster::Message(WokXMLTag *tag)
{
	if ( tag->GetTagList("body").empty() )
		return 1;
	
	std::string body = tag->GetFirstTag("body").GetBody();
	if ( body.empty() )
		return 1;
	
	while ( body.find("\n") != std::string::npos )
	{
		body.replace(body.find("\n"), 1, " ");
	}
	
	std::string msg;
	
	if ( tag->GetAttr("from").find("/") == std::string::npos )
	{
		// Status message
		msg = "\n <span size='x-small' color='blue'>" + XMLisize(body) + "</span>";
	}
	else
	{
		// Normal message
		std::string color = "green";
		if ( mynick == tag->GetAttr("from").substr(tag->GetAttr("from").find("/")+1) )
			color = "gray";
		
		if ( body.substr(0,4) == "/me " )
			msg = "\n <span size='x-small' color='" + color + "'>*" + XMLisize(tag->GetAttr("from").substr(tag->GetAttr("from").find("/")+1)) + " " + XMLisize(body.substr(4)) + "</span>";
		else
			msg = "\n <span size='x-small' color='" + color + "'>&lt;" + XMLisize(tag->GetAttr("from").substr(tag->GetAttr("from").find("/")+1)) + "&gt;</span><span size='x-small'>" + XMLisize(body) + "</span>";
		
	}


	message.push_back(msg);
	while ( message.size() > 15 )
		message.pop_front();
		
	Display();
	return 1;
}

int
GroupChatRoster::Tooltip(WokXMLTag *tag)
{
	WokXMLTag tooltag(NULL, "tooltip");
	tooltag.AddAttr("room", room);
	std::list<std::string>::iterator iter;
	
	for( iter = message.begin(); iter != message.end() ; iter++)
	{
		tooltag.GetFirstTag("message").AddTag("body").AddText(*iter);
	}
	
	wls->SendSignal("Jabber GroupChatTooltip Set", &tooltag);

	return 1;
}


int
GroupChatRoster::TooltipReset(WokXMLTag *tag)
{
	WokXMLTag tooltag(NULL, "tooltip");
	
	wls->SendSignal("Jabber GroupChatTooltip Reset", &tooltag);
	
	return 1;
}
