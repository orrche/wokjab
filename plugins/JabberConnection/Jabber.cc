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

#include "Jabber.h"

Jabber::Jabber(WLSignal *wls) : WoklibPlugin(wls)
{	
	session_nr = 0;
	
	EXP_SIGHOOK("Jabber Connection Connect", &Jabber::SignalConnect, 100);
	EXP_SIGHOOK("Jabber GetSessions", &Jabber::GetSessions, 1000);
	EXP_SIGHOOK("Jabber XML Send", &Jabber::SendXML, 1000);
	
	EXP_SIGHOOK("Jabber Connection Disconnect", &Jabber::SignalDisconnect, 1000);
	EXP_SIGHOOK("Jabber Connection Lost", &Jabber::ConnectionLost, 1000);
	
	authmanager = new IQAuthManager(wls);
}


Jabber::~Jabber()
{

}


std::string
Jabber::connect (std::string server, std::string username,
		 std::string password, std::string resource, int port, int type)
{
	Connection *conn;
	char buf[20];
	sprintf(buf, "jabber%d", session_nr++);
	
	conn = new Connection (wls, username, password, server,resource, port, type, buf);
	connections[buf] = conn;
	
	return buf;
}

int
Jabber::GetSessions(WokXMLTag *tag)
{
	std::map<std::string, Connection *>::iterator iter;
	
	for ( iter = connections.begin() ; iter != connections.end() ; iter++)
		tag->AddTag("item").AddAttr("name", iter->first);

	return 1;
}


int
Jabber::SendXML(WokXMLTag *tag)
{
	if( connections.find(tag->GetAttr("session")) != connections.end() )
		connections[tag->GetAttr("session")]->xmloutput->sendxml(tag->GetChildrenStr().c_str());
	return 1;
}

int
Jabber::SignalConnect(WokXMLTag *tag)
{
	tag->AddAttr("session", connect( tag->GetAttr("server"),
					tag->GetAttr("username"),
					tag->GetAttr("password"),
					tag->GetAttr("resource"),
					atoi(tag->GetAttr("port").c_str()),
					atoi(tag->GetAttr("type").c_str())) );
	
	return 1;
}

int
Jabber::SignalDisconnect(WokXMLTag *tag)
{
	if( !tag || tag->GetAttr("session") == "" ) 
	{
		std::map<std::string, Connection *>::iterator iter;
		
		for ( iter = connections.begin() ; iter != connections.end() ; iter ++)
		{
			std::string session = iter->first;
			WokXMLTag msgtag(NULL,"message");
			msgtag.AddAttr("session", session);
			msgtag.AddText("</stream:stream>");
			wls->SendSignal("Jabber XML Send", &msgtag);
		}
		
	}
	else
	{
		std::string session = tag->GetAttr("session");
		WokXMLTag msgtag(NULL,"message");
		msgtag.AddAttr("session", session);
		msgtag.AddText("</stream:stream>");
		wls->SendSignal("Jabber XML Send", &msgtag);
	}
	
	return 1;
}

int
Jabber::ConnectionLost(WokXMLTag *tag)
{
	if( connections.find(tag->GetAttr("session")) != connections.end() )
	{
		delete connections[tag->GetAttr("session")];
		connections.erase(tag->GetAttr("session"));
	}
	
	return true;
}




