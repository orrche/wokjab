/***************************************************************************
 *  Copyright (C) 2003-2012  Kent Gustavsson <nedo80@gmail.com>
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
#include <sstream>

Jabber::Jabber(WLSignal *wls) : WoklibPlugin(wls)
{	
	session_nr = 0;
	
	EXP_SIGHOOK("Jabber Connection Connect", &Jabber::SignalConnect, 100);
	EXP_SIGHOOK("Jabber Connection TimedReconnect", &Jabber::ConnectionTimedReconnect, 1000);
	EXP_SIGHOOK("Jabber GetSessions", &Jabber::GetSessions, 1000);
	EXP_SIGHOOK("Jabber XML Send", &Jabber::SendXML, 1000);
	
	EXP_SIGHOOK("Jabber Connection Disconnect", &Jabber::SignalDisconnect, 1000);
	EXP_SIGHOOK("Jabber Connection Lost", &Jabber::ConnectionLost, 1000);
	
	EXP_SIGHOOK("Jabber Server GetMenu", &Jabber::ServerMenu, 1000);
	EXP_SIGHOOK("Jabber Server MenuLogout", &Jabber::ServerMenuLogout, 1000);
	authmanager = new IQAuthManager(wls);
}


Jabber::~Jabber()
{

}


std::string
Jabber::connect (std::string server, std::string host, std::string username,
		 std::string password, std::string resource, int port, int type)
{
	Connection *conn;
	char buf[20];
	sprintf(buf, "jabber%d", session_nr++);
	if( host == "" )
		host = server;
	conn = new Connection (wls, username, password, server, host, resource, port, type, buf);
	connections[buf] = conn;
	
	return buf;
}

int
Jabber::ServerMenuLogout(WokXMLTag *tag)
{
	WokXMLTag msg("disconnect");
	msg.AddAttr("session", tag->GetAttr("session"));
	
	wls->SendSignal("Jabber Connection Disconnect", msg);
	return 1;	
}

int
Jabber::ServerMenu(WokXMLTag *tag)
{
	WokXMLTag &item = tag->AddTag("item");
	item.AddAttr("name", "Logout");
	item.AddAttr("signal", "Jabber Server MenuLogout");
	
	return 1;
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
					tag->GetAttr("host"),
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
			wls->SendSignal("Jabber Connection Disconnect '" + XMLisize(session) + "'", &msgtag);
		}
		
	}
	else
	{
		std::string session = tag->GetAttr("session");
		WokXMLTag msgtag(NULL,"message");
		wls->SendSignal("Jabber Connection Disconnect '" + XMLisize(session) + "'", &msgtag);
	}
	
	return 1;
}

int 
Jabber::ConnectionTimedReconnect(WokXMLTag *tag)
{
	tag->AddAttr("stop", "true");

	wls->SendSignal("Jabber Connection Connect", tag->GetFirstTag("connect"));

	std::cout << *tag << std::endl;

	return 1;
}


int
Jabber::ConnectionLost(WokXMLTag *tag)
{
	std::map<std::string, Connection *>::iterator conn_iter = connections.find(tag->GetAttr("session"));
	
	if( conn_iter != connections.end() )
	{
		Connection *connection = conn_iter->second;
		

		if ( connection->reconnect )
		{
			tag->AddAttr("reconnecting", "true");
			std::stringstream ss_port;
			ss_port << connection->port;
			std::stringstream ss_type;
			ss_type << connection->type;

			WokXMLTag reconnectTimeout("timeout");
			reconnectTimeout.AddAttr("time", "15000");
			reconnectTimeout.AddAttr("signal", "Jabber Connection TimedReconnect");
			
			WokXMLTag &reconnectSignal = reconnectTimeout.GetFirstTag("data").AddTag("connect");
			reconnectSignal.AddAttr("server", connection->server);
			reconnectSignal.AddAttr("host", connection->host);
			reconnectSignal.AddAttr("username", connection->username);
			reconnectSignal.AddAttr("password", connection->password);
			reconnectSignal.AddAttr("resource", connection->resource);
			reconnectSignal.AddAttr("type", ss_type.str());
			reconnectSignal.AddAttr("port", ss_port.str());
			reconnectSignal.AddAttr("reconnecting", "true");
			
			wls->SendSignal("Woklib Timmer Add", reconnectTimeout);
		}


		delete connections[tag->GetAttr("session")];
		connections.erase(tag->GetAttr("session"));
	}

	return true;
}




