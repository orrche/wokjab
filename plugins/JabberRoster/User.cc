/***************************************************************************
 *  Copyright (C) 2003-2006  Kent Gustavsson <nedo80@gmail.com>
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
// Class: User
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sat Sep  2 14:39:43 2006
//

#include "User.h"
#include "glib.h"
#include <algorithm>

User::User(WLSignal *wls, WokXMLTag *tag, JabberSession *ses) : WLSignalInstance(wls),
ses(ses)
{
	usertag = NULL;
	visible = false;
	tickerpos = 0;
	currentticker = tickeritems.begin();
	UpdateTicketEntries(NULL);
	Update(tag);
	
	jid = tag->GetAttr("jid");
	
	EXP_SIGHOOK("Jabber XML Presence To " + ses->GetSession() + " " + tag->GetAttr("jid"), &User::Presence, 1000);
	EXP_SIGHOOK("Jabber Roster Update "  + ses->GetSession() + " " + tag->GetAttr("jid"), &User::UpdateXML, 1000);
	EXP_SIGHOOK("Jabber Roster Recheck", &User::Recheck, 1000);
	EXP_SIGHOOK("Jabber UserActivityUpdate " + ses->GetSession() + " '" + XMLisize(tag->GetAttr("jid"))+"'", &User::UpdateTicketEntries, 1000);
	EXP_SIGHOOK("Jabber Roster GUI Ticker Update", &User::TickerUpdate, 500);
}


User::~User()
{
	Hide();
	if (usertag)
		delete usertag;
}

int
User::UpdateTicketEntries(WokXMLTag *tag)
{
	WokXMLTag entries("entries");
	entries.AddAttr("jid", jid);
	entries.AddAttr("session", ses->GetSession());
	
	wls->SendSignal("Jabber UserActivityGet", entries);
	wls->SendSignal("Jabber UserActivityGet " + ses->GetSession() + " '" + XMLisize(jid) + "'", entries);
	
	for( std::list<std::string>::iterator iter = tickeritems.begin() ; iter != tickeritems.end() ;)
	{
		bool found = false;
		if ( *iter == statusmsg )
		{
			iter++;
			found = true;
		}
		else
		{
			std::list <WokXMLTag *>::iterator tagiter;
			for ( tagiter = entries.GetTagList("item").begin() ; tagiter != entries.GetTagList("item").end() ; tagiter++)
			{
				if ( *iter == (*tagiter)->GetFirstTag("line").GetBody() )
				{
					iter++;
					found = true;
					break;
				}
			}
		}
		
		if ( !found )
		{
			std::list<std::string>::iterator tmpiter;
			tmpiter = iter;

			iter++;
			if ( currentticker == tmpiter )
			{
				currentticker = tickeritems.begin();
				tickerpos = 0;
			}
			
			tickeritems.erase(tmpiter);
		}
	}
	std::list <WokXMLTag *>::iterator tagiter;
	for ( tagiter = entries.GetTagList("item").begin() ; tagiter != entries.GetTagList("item").end() ; tagiter++)
	{
		if ( find(tickeritems.begin(), tickeritems.end() , (*tagiter)->GetFirstTag("line").GetBody() ) == tickeritems.end())
		{
			tickeritems.push_back((*tagiter)->GetFirstTag("line").GetBody());
		}
	}
	
	if ( currentticker == tickeritems.end() ) 
		currentticker = tickeritems.begin();
	return 1;
}

int
User::TickerUpdate(WokXMLTag *tag)
{
	if ( !tickeritems.empty() );
	{
		if ( visible )
		{
			tickerpos++;
			UpdateRow();
		}
	}
}

int
User::UpdateXML(WokXMLTag *tag)
{
	UpdateEntry();
	return 1;
}

void
User::Update(WokXMLTag *tag)
{
	bool v = visible;
	
	if ( visible )
		Hide();
		
	name = tag->GetAttr("name");
	if ( name.empty() )
		name = tag->GetAttr("jid");
		
	if (usertag)
		delete usertag;
	
	usertag = new WokXMLTag (*tag);
	
	if ( v )
		Show();
}

int
User::Recheck(WokXMLTag *tag)
{
	UpdateEntry();
	
	return 1;
}

void
User::UpdateEntry()
{
	Hide();
	bool show;

	if (! statusmsg.empty() )
	{
		std::list <std::string>::iterator item = std::find(tickeritems.begin(), tickeritems.end(), statusmsg);
		if( item == currentticker )
		{
			currentticker++;
			if ( currentticker == tickeritems.end() )
				currentticker = tickeritems.begin();
			tickerpos = 0;
		}
		tickeritems.erase(item);
	}
	
	WokXMLTag querytag(NULL, "query");
	WokXMLTag &itemtag = querytag.AddTag("item");
	itemtag.AddAttr("jid", usertag->GetAttr("jid"));
	itemtag.AddAttr("session", ses->GetSession());
	
	wls->SendSignal("Jabber GUI Roster View", &querytag );
	wls->SendSignal("Jabber GUI GetIcon", &querytag );
	
	icon = querytag.GetFirstTag("item").GetFirstTag("resource").GetAttr("icon");
	statusmsg = querytag.GetFirstTag("item").GetFirstTag("resource").GetFirstTag("status").GetBody();
	showmsg = querytag.GetFirstTag("item").GetFirstTag("resource").GetFirstTag("show").GetBody();
	avatar = querytag.GetFirstTag("item").GetAttr("avatar");
	
	if ( !statusmsg.empty() )
	{
		tickeritems.push_back(statusmsg);
		if( tickeritems.size() == 1 )
		{
			currentticker = tickeritems.begin();
			tickerpos = 0;
		}
	}
	
	if( Events.empty() )
	{
		if( itemtag.GetAttr("view") == "true" )
			show = true;
		else
			show = false;
	}
	else
	{
		show = true;
		icon = (*Events.begin())->GetAttr("icon");
	}
		
	if ( show )
		Show();
}

int
User::Presence(WokXMLTag *tag)
{
	UpdateEntry();
	
	return 1;
}

void
User::UpdateRow()
{
	WokXMLTag itemtag(NULL, "item");
	WokXMLTag &columntag =  itemtag.AddTag("columns");
	WokXMLTag &texttag = columntag.AddTag("text");
	columntag.AddTag("pre_pix").AddText(icon);
	
	texttag.AddText(XMLisize(name));
			
	std::string::size_type pos;
	std::string stmsg = XMLisize(statusmsg);
	while((pos = stmsg.find("\n")) != std::string::npos)
		stmsg.erase(pos, 1);
	std::string shmsg = XMLisize(showmsg);
	while((pos = shmsg.find("\n")) != std::string::npos)
		shmsg.erase(pos, 1);
	
	if ( ! shmsg.empty() ) 
		texttag.AddText(" - <span style='italic' size='x-small' color='blue'>" + shmsg + "</span>");
	
	if ( ses->parent->config->GetFirstTag("ticker").GetAttr("data") != "false" && 
			( tickeritems.size() > 1 || ses->parent->config->GetFirstTag("ticker_single").GetAttr("data") != "false"))
	{
		if ( ! tickeritems.empty() )
		{
			if ( tickerpos > g_utf8_strlen (currentticker->c_str(), -1) + 4)
			{
				tickerpos = 0;
				currentticker++;
				
				if ( currentticker == tickeritems.end() )
					currentticker = tickeritems.begin();
			}
			
			texttag.AddText("\n<span style='italic' size='x-small'>");
			if ( tickerpos > g_utf8_strlen (currentticker->c_str(), -1) )
				texttag.AddText(std::string(" --- ").substr(-g_utf8_strlen (currentticker->c_str(), -1) + tickerpos));
			else
				texttag.AddText(g_utf8_offset_to_pointer(currentticker->c_str(), tickerpos));
			
			
			std::list <std::string>::iterator runner(currentticker);
			runner++;
			int n = 0;
			for (  ; runner != tickeritems.end() ; runner++ )
			{
				if ( n ||  tickerpos <= g_utf8_strlen (currentticker->c_str(), -1))
					texttag.AddText(" --- ");
				texttag.AddText(*runner);
				n++;
			}
			
			for( runner = tickeritems.begin() ; runner != currentticker ; runner++ )
			{
				if ( n ||  tickerpos <= g_utf8_strlen (currentticker->c_str(), -1))
					texttag.AddText(" --- ");
				texttag.AddText(*runner);
				n++;
			}
			texttag.AddText(" --- ");
			
			texttag.AddText(currentticker->substr(0, g_utf8_offset_to_pointer(currentticker->c_str(), tickerpos) - currentticker->c_str()));
			if ( tickerpos > g_utf8_strlen(currentticker->c_str(), -1) )
				texttag.AddText(std::string(" --- ").substr(0, tickerpos - g_utf8_strlen(currentticker->c_str(), -1)));
			
			texttag.AddText("</span>");
		}
	}
	else if ( !statusmsg.empty() )
	{
		texttag.AddText("\n<span style='italic' size='x-small'>");
		texttag.AddText(statusmsg);
		texttag.AddText("</span>");
	}
	else if ( tickeritems.size() == 1 )
	{
		texttag.AddText("\n<span style='italic' size='x-small'>");
		texttag.AddText(*tickeritems.begin());
		texttag.AddText("</span>");
	}
	
	columntag.AddTag("post_pix").AddText(avatar);
	
	std::map <std::string, std::string>::iterator iter;
	
	for( iter = id.begin(); iter != id.end() ; iter++ )
	{
		itemtag.AddAttr("id", iter->second);
		wls->SendSignal("GUIRoster UpdateItem", itemtag);
	}
}

void
User::Show()
{
	if ( visible ) 
		Hide();
	visible = true;
	
	WokXMLTag itemtag(NULL, "item");
	WokXMLTag &columntag =  itemtag.AddTag("columns");
	WokXMLTag &texttag = columntag.AddTag("text");
	columntag.AddTag("pre_pix").AddText(icon);
	
	texttag.AddText(XMLisize(name));
			
	std::string::size_type pos;
	std::string stmsg = XMLisize(statusmsg);
	while((pos = stmsg.find("\n")) != std::string::npos)
		stmsg.erase(pos, 1);
	std::string shmsg = XMLisize(showmsg);
	while((pos = shmsg.find("\n")) != std::string::npos)
		shmsg.erase(pos, 1);
	
	if ( ! shmsg.empty() ) 
		texttag.AddText(" - <span style='italic' size='x-small' color='blue'>" + shmsg + "</span>");
	
	if ( ses->parent->config->GetFirstTag("ticker").GetAttr("data") != "false" && 
			( tickeritems.size() > 1 || ses->parent->config->GetFirstTag("ticker_single").GetAttr("data") != "false"))
	{
		if ( ! tickeritems.empty() )
		{
			if ( tickerpos > g_utf8_strlen (currentticker->c_str(), -1) + 4)
			{
				tickerpos = 0;
				currentticker++;
				
				if ( currentticker == tickeritems.end() )
					currentticker = tickeritems.begin();
			}
			
			texttag.AddText("\n<span style='italic' size='x-small'>");
			if ( tickerpos > g_utf8_strlen (currentticker->c_str(), -1) )
				texttag.AddText(std::string(" --- ").substr(-g_utf8_strlen (currentticker->c_str(), -1) + tickerpos));
			else
				texttag.AddText(g_utf8_offset_to_pointer(currentticker->c_str(), tickerpos));
			
			
			std::list <std::string>::iterator runner(currentticker);
			runner++;
			int n = 0;
			for (  ; runner != tickeritems.end() ; runner++ )
			{
				if ( n ||  tickerpos <= g_utf8_strlen (currentticker->c_str(), -1))
					texttag.AddText(" --- ");
				texttag.AddText(*runner);
				n++;
			}
			
			for( runner = tickeritems.begin() ; runner != currentticker ; runner++ )
			{
				if ( n ||  tickerpos <= g_utf8_strlen (currentticker->c_str(), -1))
					texttag.AddText(" --- ");
				texttag.AddText(*runner);
				n++;
			}
			texttag.AddText(" --- ");
			
			texttag.AddText(currentticker->substr(0, g_utf8_offset_to_pointer(currentticker->c_str(), tickerpos) - currentticker->c_str()));
			if ( tickerpos > g_utf8_strlen(currentticker->c_str(), -1) )
				texttag.AddText(std::string(" --- ").substr(0, tickerpos - g_utf8_strlen(currentticker->c_str(), -1)));
			
			texttag.AddText("</span>");
		}
	}
	else if ( !statusmsg.empty() )
	{
		texttag.AddText("\n<span style='italic' size='x-small'>");
		texttag.AddText(statusmsg);
		texttag.AddText("</span>");
	}
	
	columntag.AddTag("post_pix").AddText(avatar);
	
	
	
	if ( usertag->GetTagList("group").size() )
	{
		std::list <WokXMLTag *>::iterator iter;
		for ( iter = usertag->GetTagList("group").begin() ; iter != usertag->GetTagList("group").end() ; iter++)
		{
			std::string groupid = ses->AddToGroup((*iter)->GetBody(), this);
			
			if ( !groupid.empty() )
			{
				itemtag.AddAttr("parant", groupid);

				wls->SendSignal("GUIRoster AddItem", itemtag);
				id[(*iter)->GetBody()] = itemtag.GetAttr("id");
			}
		}
	}
	else
	{
		itemtag.AddAttr("parant", ses->GetID());
			
		wls->SendSignal("GUIRoster AddItem", itemtag);
		id[""] = itemtag.GetAttr("id");
	}
	
	std::map <std::string, std::string>::iterator iter;
	
	for( iter = id.begin() ; iter != id.end() ; iter++)
	{
		EXP_SIGHOOK("GUIRoster Activate " + iter->second, &User::Activate, 500);
		EXP_SIGHOOK("GUIRoster RightButton " + iter->second, &User::RightButton, 500);
		EXP_SIGHOOK("GUIRoster Tooltip Set " + iter->second, &User::Tooltip, 500);
		EXP_SIGHOOK("GUIRoster Tooltip Reset " + iter->second, &User::TooltipReset, 500);
	}
}

void
User::Hide()
{
	if( !visible )
		return;
	visible = false;
	
	std::map <std::string, std::string>::iterator iter;
	
	for( iter = id.begin() ; iter != id.end() ; iter++)
	{
		WokXMLTag tag(NULL, "item");
		tag.AddAttr("id", iter->second);
		wls->SendSignal("GUIRoster RemoveItem", tag);
		
		if ( iter->first.size() )
			ses->RemoveFromGroup(iter->first);
	}
	
	for( iter = id.begin() ; iter != id.end() ; iter++)
	{
		EXP_SIGUNHOOK("GUIRoster Activate " + iter->second, &User::Activate, 500);
		EXP_SIGUNHOOK("GUIRoster RightButton " + iter->second, &User::RightButton, 500);
		EXP_SIGUNHOOK("GUIRoster Tooltip Set " + iter->second, &User::Tooltip, 500);
		EXP_SIGUNHOOK("GUIRoster Tooltip Reset " + iter->second, &User::TooltipReset, 500);
	}
	id.clear();
}

int
User::TooltipReset(WokXMLTag *tag)
{
	WokXMLTag tooltag(NULL, "tooltip");
	tooltag.AddAttr("session", ses->GetSession());
	tooltag.AddAttr("jid", usertag->GetAttr("jid"));
	
	wls->SendSignal("Jabber Tooltip Reset", &tooltag);
}

int
User::Tooltip(WokXMLTag *tag)
{
	WokXMLTag tooltag(NULL, "tooltip");
	tooltag.AddAttr("session", ses->GetSession());
	tooltag.AddAttr("jid", usertag->GetAttr("jid"));
	
	wls->SendSignal("Jabber Tooltip Set", &tooltag);
	return 1;
}

int
User::RightButton(WokXMLTag *tag)
{
	if (!usertag)
		return 1;
	
	char buf[20];
	WokXMLTag MenuXML(NULL, "menu");
	
	MenuXML.AddAttr("button", tag->GetAttr("button"));
	MenuXML.AddAttr("time", tag->GetAttr("time"));
	
	MenuXML.AddTag("item").AddAttr("signal", "Jabber GUI Roster GetJIDMenu");
	MenuXML.AddTag("item").AddAttr("signal", "Jabber GUI GetJIDMenu");
	WokXMLTag &data = MenuXML.AddTag("data");
	data.AddAttr("jid", usertag->GetAttr("jid"));
	data.AddAttr("session", ses->GetSession());
	wls->SendSignal("Jabber GUI JIDMenu", &MenuXML);
	
	return 1;
}

int
User::Activate(WokXMLTag *tag)
{
	if (!usertag)
		return 1;
	if ( !Events.empty() )
	{
	
		WokXMLTag eventtag(NULL, "event");
		eventtag.AddTag(Events.front());
		
		
		if(!(*Events.begin())->GetFirstTag("commands").GetFirstTag("command").GetFirstTag("signal").GetTags().empty())
			wls->SendSignal((*Events.begin())->GetFirstTag("commands").GetFirstTag("command").GetFirstTag("signal").GetAttr("name"), 
									*(*Events.begin())->GetFirstTag("commands").GetFirstTag("command").GetFirstTag("signal").GetTags().begin());
		
		wls->SendSignal("Jabber Event Remove", &eventtag);
	}
	else
	{
		WokXMLTag dialogtag(NULL,"dialog");
		dialogtag.AddAttr("jid", usertag->GetAttr("jid"));
		dialogtag.AddAttr("session", ses->GetSession());
		
		wls->SendSignal("Jabber GUI MessageDialog Open", &dialogtag);
	}
	
	return 1;
}

void
User::AddEvent(WokXMLTag *tag)
{
	Events.push_back(new WokXMLTag(*tag));
	UpdateEntry();
}

void
User::RemoveEvent(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator iter;
	std::list <WokXMLTag *>::iterator tmpiter;
	
	for( iter = Events.begin() ; iter != Events.end() ;)
	{
		if (tag->In(**iter))
		{
		
			tmpiter = iter;
			iter++;
			delete *tmpiter;
			Events.remove(*tmpiter);
		}
		else
		{
			iter++;
		}
	}
	UpdateEntry();
}
