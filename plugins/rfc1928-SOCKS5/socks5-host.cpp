/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2008 <nedo80@gmail.com>
 * 
 * wokjab is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * wokjab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with wokjab.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#include <sstream>

#include "socks5-host.hpp"

#define MAXPENDING 5 

Socks5Host::Socks5Host(WLSignal *wls, WokXMLTag *tag) : WLSignalInstance(wls),
settings(*tag)
{
	struct sockaddr_in echoserver;
	
//	Maybe I should hook to this 
//	EXP_SIGHOOK("Jabber Disco Info Get", &jep65::DiscoInfo, 500);
	     	/* Create the TCP socket */
	if ((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		woklib_error(wls, "Failed at creating socket for Socks5 port " + settings.GetAttr("port"));
		delete this;
		return;
	}
	
	/* Construct the server sockaddr_in structure */
	memset(&echoserver, 0, sizeof(echoserver));	/* Clear struct */
	echoserver.sin_family = AF_INET;				/* Internet/IP */
	echoserver.sin_addr.s_addr = htonl(INADDR_ANY);	/* Incoming addr */
	echoserver.sin_port = htons(atoi(settings.GetAttr("port").c_str()));	/* server port */
	
	/* Bind the server socket */
	int i = 0; 
	if ( bind(serversock, (struct sockaddr *) &echoserver, sizeof(echoserver)) < 0 )
	{
		woklib_error(wls, "Couldn't not bind to designated SOCKS5 port " + settings.GetAttr("port"));
		delete this;
		return;
	}

	/* Listen on the server socket */
	if (listen(serversock, MAXPENDING) < 0) {
		woklib_error(wls, "Failed to listen on server socket");
		delete this;
		return;
	}
	
	std::stringstream buf;
	buf << serversock;
	WokXMLTag sigtag(NULL, "socket");
	sigtag.AddAttr("socket", buf.str());
	wls->SendSignal("Woklib Socket In Add", sigtag);
	EXP_SIGHOOK(sigtag.GetAttr("signal"), &Socks5Host::ReadData, 1000);
	
	
	/* Wait for client connection */
}

int
Socks5Host::ReadData(WokXMLTag *tag)
{
	
	
	return 1;
}

