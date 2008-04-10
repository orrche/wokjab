/***************************************************************************
 *  Copyright (C) 2003-2008  Kent Gustavsson <nedo80@gmail.com>
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

#include <string>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#include "bytestream.h"
#include "bytestreamsession.h"
#include "jep65send.h"
#include "recvsocket.h"



#include <sstream>

#define MAXPENDING 5    /* Max connection requests */
#define BUFFSIZE 32

void Die(char *mess) { perror(mess); }
	  
jep65::jep65(WLSignal *wls):
WoklibPlugin(wls)
{
	struct sockaddr_in echoserver;
	sport = "8111";
	
	EXP_SIGHOOK("Jabber XML IQ New query set xmlns:http://jabber.org/protocol/bytestreams", &jep65::Session, 999);
	EXP_SIGHOOK("Jabber Stream File Send Method http://jabber.org/protocol/bytestreams", &jep65::Send, 999);
	EXP_SIGHOOK("Jabber Disco Info Get", &jep65::DiscoInfo, 500);
	     	/* Create the TCP socket */
	if ((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		Die("Failed to create socket");
	}
	
	/* Construct the server sockaddr_in structure */
	memset(&echoserver, 0, sizeof(echoserver));	/* Clear struct */
	echoserver.sin_family = AF_INET;				/* Internet/IP */
	echoserver.sin_addr.s_addr = htonl(INADDR_ANY);	/* Incoming addr */
	echoserver.sin_port = htons(atoi(sport.c_str()));	/* server port */
	
	/* Bind the server socket */
	int i = 0; 
	while (bind(serversock, (struct sockaddr *) &echoserver,
			sizeof(echoserver)) < 0) 
	{
		echoserver.sin_port = htons(atoi(sport.c_str()) + i++);	/* server port */
		//woklib_error(wls, "Failed to bind server socket");
		//Die("Failed to bind the server socket");
	}
	if ( i > 1 )
	{
		i--;
		std::stringstream str;
		str << 8111 + i;
		sport = str.str();
		
		woklib_debug(wls, "New listening port " + sport);
	}
	/* Listen on the server socket */
	if (listen(serversock, MAXPENDING) != 0) {
		woklib_error(wls, "Failed to listen on server socket");
		return;
		//Die("Failed to listen on server socket");
	}
	
	std::stringstream buf;
	buf << serversock;
	WokXMLTag sigtag(NULL, "socket");
	sigtag.AddAttr("socket", buf.str());
	wls->SendSignal("Woklib Socket In Add", sigtag);
	EXP_SIGHOOK(sigtag.GetAttr("signal"), &jep65::ReadData, 1000);
	
	
	/* Wait for client connection */
}

jep65::~jep65()
{
	close(serversock);
}

int
jep65::Session(WokXMLTag *xml)
{
	new jep65Session(wls, xml);
	return 1;
}

/**
 * Used for transmitting files 
	*/
int
jep65::Send(WokXMLTag *xml)
{
	new jep65send(wls, xml, sport);
	return 1;
}

int 
jep65::ReadData(WokXMLTag *xml)
{
	int clientsock;
	struct sockaddr_in echoclient;
	unsigned int clientlen = sizeof(echoclient);
	
	if ((clientsock = accept(serversock, (struct sockaddr *) &echoclient, &clientlen)) < 0)
	{
		woklib_error(wls, "Failed to accept client connection");
//		Die("Failed to accept client connection");
		return 1;
	}

	new RecvSocket(wls, clientsock);
	
	return 1;
}

int
jep65::DiscoInfo(WokXMLTag *tag)
{
	tag->AddTag("feature").AddAttr("var", "http://jabber.org/protocol/bytestreams");

	return true;
}

