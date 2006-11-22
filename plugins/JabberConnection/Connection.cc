/***************************************************************************
 *  Copyright (C) 2003-2006  Kent Gustavsson <oden@gmx.net>
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


#include "Connection.h"
#include "IQauth.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

#include <iostream>
#include <sstream>
#include <string.h>

Connection::Connection (WLSignal *wls,std::string in_username, std::string in_password, std::string in_server, std::string resource,  int in_port, int type, std::string session)
: WLSignalInstance(wls),
username(in_username),
password(in_password),
server(in_server),
resource(resource),
port(in_port),
type(type),
session(session)
{
	EXP_SIGHOOK("Jabber Connection GetUserData", &Connection::GetUserVariables, 1);
	EXP_SIGHOOK("Jabber Connection GetSocket", &Connection::GetSocket, 500);
	EXP_SIGHOOK("Jabber Connection Reset " +session, &Connection::Reset, 500);
	
	EXP_SIGHOOK("Jabber GetMyNick", &Connection::GetMyNick, 1);
	socket_nr = 0;
	ssl = NULL;

	xmlinput = new XML_Input(this, wls, session);
	xmloutput = new XML_Output(wls);
	
	openconnection();
	
	char buf[20];
	sprintf(buf, "%d", get_socket());
	WokXMLTag sigtag(NULL, "socket");
	sigtag.AddAttr("socket", buf);
	wls->SendSignal("Woklib Socket In Add", sigtag);
	EXP_SIGHOOK(sigtag.GetAttr("signal"), &Connection::ReadData, 1000);

	ssl = NULL;
	StartSSL();
	if(server != "")
		sendinit();
}

Connection::~Connection()
{
	if(ssl)
		delete ssl;
	close (socket_nr);
	delete xmlinput;
	delete xmloutput;
}

int
Connection::Reset(WokXMLTag *tag)
{
//	delete xmlinput;
	xmlinput = new XML_Input(this, wls, session);
	if ( ssl )
		xmlinput->SetSSL(ssl->ssl);
	sendinit();
}

int
Connection::ReadData(WokXMLTag *tag)
{
	XML_Input *xi;
	
	xi = xmlinput;
	if (!xmlinput->read_data (get_socket ()))
		tag->AddAttr("error", "connection lost");
	
	if ( xi != xmlinput )
		delete xi;
	
	return 1;
}

int
Connection::GetMyNick(WokXMLTag *tag)
{
	if( tag->GetAttr("session") == session )
		tag->AddAttr("nick", username);
		
	return true;
}

int
Connection::GetUserVariables(WokXMLTag *tag)
{
	std::stringstream buf;
	std::list <WokXMLTag *>::iterator iter;
	
	for( iter = tag->GetTagList("item").begin() ; iter != tag->GetTagList("item").end() ; iter++)
	{
		if( (*iter)->GetAttr("session") == session )
		{
			(*iter)->AddTag("username").AddText(username);
			(*iter)->AddTag("resource").AddText(resource);
			(*iter)->AddTag("server").AddText(server);
			(*iter)->AddTag("password").AddText(password);
			buf << type;
			(*iter)->AddTag("type").AddText(buf.str());
			(*iter)->AddTag("jid").AddText(username + "@" + server + "/" + resource);
			(*iter)->AddTag("ip").AddText(ip);
		}		
	}
	
	return 1;
}

int
Connection::openconnection()
{
	struct sockaddr_in sa;
	struct hostent *hp;
	
	if ((hp = gethostbyname (server.c_str())) == NULL)
	{
		errno = ECONNREFUSED;
		return (-1);
	}

	
	memset (&sa, 0, sizeof (sa));
	memcpy ((char *) &sa.sin_addr, hp->h_addr, hp->h_length);

	sa.sin_family = hp->h_addrtype;
	sa.sin_port = htons ((u_short) port);
		
	if ((socket_nr = socket (hp->h_addrtype, SOCK_STREAM, 0)) < 0)
		return (-1);
	if (connect (socket_nr, (struct sockaddr *) &sa, sizeof sa) < 0)
	{
		close (socket_nr);
		return (-1);
	}

	struct sockaddr myip;
	socklen_t len = sizeof( struct sockaddr_in );
	if ( !getsockname(socket_nr, (sockaddr*)&sa, &len))
		ip = inet_ntoa(sa.sin_addr);
	else
		ip = "";
	
	xmloutput->set_socket(socket_nr);
		
	return (1);
}

void Connection::sendinit()
{
	std::string init;
	
	init = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?><stream:stream to=\"";
	init += server;
	init += "\" xmlns=\"jabber:";
	switch(type)
	{
		case IQauth::HandshakeComponent:
			init+= "component:accept";
			break;
		default:
			init+= "client";
	}
	init += "\" xmlns:stream=\"http://etherx.jabber.org/streams\" version='1.0'>";
	
	xmloutput->sendxml(init.c_str());
}

int Connection::get_socket()
{
	return(socket_nr);
}

int Connection::GetSocket(WokXMLTag *tag)
{
	if( tag->GetAttr("session") == session) 
	{
		std::stringstream buf;
		buf << socket_nr;
		tag->AddAttr("socket", buf.str());
		std::cout << "Sent socknr " << socket_nr << std::endl;
		std::cout << "XML : " << *tag << std::endl;
	}
	
	return true;
}

void
Connection::StartSSL()
{
	ssl = new Woklib::SSL(wls, this);
}

void
Connection::SetSSL(::SSL *s)
{
	xmlinput->SetSSL(s);
	xmloutput->SetSSL(s);
}

std::string
Connection::GetConID()
{

	return conid;
}

void
Connection::SetConID(const std::string &id)
{
	conid = id;
}
