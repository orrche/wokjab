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
// Class: JabberRoster
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sat Sep  2 11:29:22 2006
//

#include "JabberRoster.h"


JabberRoster::JabberRoster(WLSignal *wls) : WoklibPlugin(wls)
{
	config = new WokXMLTag("config");
	EXP_SIGHOOK("Config XML Change /main/window/jabber_roster", &JabberRoster::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/main/window/jabber_roster");
	wls->SendSignal("Config XML Trigger", &conftag);
	
	
	EXP_SIGHOOK("Jabber Event Add", &JabberRoster::AddItemEvent, 1000);
	EXP_SIGHOOK("Jabber Event Remove", &JabberRoster::RemoveItemEvent, 1);
	EXP_SIGHOOK("Jabber Connection Lost", &JabberRoster::LoggedOut, 1);
	EXP_SIGHOOK("Jabber Connection Connect" , &JabberRoster::SignIn, 1000);
	
	
	if ( config->GetFirstTag("ticker").GetAttr("data") != "false" )
	{
		WokXMLTag tag("timer");
		tag.AddAttr("time", "500");
		tag.AddAttr("signal", "Jabber Roster GUI Ticker Update");
		
		wls->SendSignal("Woklib Timmer Add",tag);
	}

	EXP_SIGHOOK("Jabber Roster GUI Ticker Update", &JabberRoster::KeepAlive, 1000);
}

JabberRoster::~JabberRoster()
{
	
}

int
JabberRoster::KeepAlive(WokXMLTag *tag)
{
	if ( config->GetFirstTag("ticker").GetAttr("data") == "false" )
	{
		tag->AddAttr("stop", "no longer needed");
		return 1;
	}
	return 1;	
}

int
JabberRoster::ReadConfig(WokXMLTag *tag)
{
	tag->GetFirstTag("config").GetFirstTag("ticker").AddAttr("type", "bool");
	tag->GetFirstTag("config").GetFirstTag("ticker").AddAttr("label", "Ticker");
	
	tag->GetFirstTag("config").GetFirstTag("ticker_single").AddAttr("type", "bool");
	tag->GetFirstTag("config").GetFirstTag("ticker_single").AddAttr("label", "Tick single items");
	
	tag->GetFirstTag("config").GetFirstTag("display_avatar").AddAttr("type", "bool");
	tag->GetFirstTag("config").GetFirstTag("display_avatar").AddAttr("label", "Display avatar in roster");
	
	if ( config->GetFirstTag("ticker").GetAttr("data") == "false" && tag->GetFirstTag("config").GetFirstTag("ticker").GetAttr("data") != "false" )
	{
		WokXMLTag tag("timer");
		tag.AddAttr("time", "500");
		tag.AddAttr("signal", "Jabber Roster GUI Ticker Update");
		
		wls->SendSignal("Woklib Timmer Add",tag);
	}
	
	delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));
	return 1;
}

int
JabberRoster::Presence(WokXMLTag *tag)
{
	


	return 1;
}

int
JabberRoster::AddItemEvent(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator iter;
	
	for ( iter = tag->GetTagList("item").begin() ; iter != tag->GetTagList("item").end() ; iter++)
	{
		if ( session.find((*iter)->GetAttr("session")) != session.end() )
			session[(*iter)->GetAttr("session")]->AddEvent(*iter);
	}

	return 1;
}

int
JabberRoster::RemoveItemEvent(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator iter;
	
	for ( iter = tag->GetTagList("item").begin() ; iter != tag->GetTagList("item").end() ; iter++)
	{
		session[(*iter)->GetAttr("session")]->RemoveEvent(*iter);
	}
	return 1;
}

int
JabberRoster::LoggedOut(WokXMLTag *tag)
{
	if ( session.find(tag->GetAttr("session") ) != session.end() )
	{
		delete session[tag->GetAttr("session")];
		session.erase(tag->GetAttr("session"));
	}

	return 1;
}

int
JabberRoster::SignIn(WokXMLTag *tag)
{	
	session[tag->GetAttr("session")] = new JabberSession(wls, tag, this);
	
	return 1;
}
