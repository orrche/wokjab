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

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#include <fcntl.h> 

#include "socks5-host-session.hpp"

Socks5HostSession::Socks5HostSession(WLSignal *wls, WokXMLTag &tag, int in_socket) : WLSignalInstance(wls), 
settings(tag),
socket(in_socket)
{
	char buf[20];
	int flags;
	
	if ((flags = fcntl(socket, F_GETFL, 0)) < 0)
	{
		woklib_error(wls, "Socks5 Connection couldn't get socket flags");
		delete this;
		return;
	}

	if (fcntl(socket, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		woklib_error(wls, "Socks5 Connection couldn't set non blocking socket");
		delete this;
		return;
	} 
	
	sprintf(buf, "%d", socket);
	WokXMLTag sigtag(NULL, "socket");
	sigtag.AddAttr("socket", buf);
	wls->SendSignal("Woklib Socket In Add", sigtag);
	EXP_SIGHOOK(sigtag.GetAttr("signal"), &Socks5HostSession::ReadData, 1000);
	
	pos = 0;
	stage = 0;
}

Socks5HostSession::~Socks5HostSession()
{
	if ( socket ) 
		close( socket );
}

int
Socks5HostSession::ReadData(WokXMLTag *xml)
{
	int len;
	len = recv (socket, data + pos, BUFFERSIZE - pos, 0);
	pos += len;

	if( len == 0 )
	{
		xml->AddAttr("error","terminated");
		return 1;
	}
	
	if( pos > 260 )
	{
		/* Someone is just fucking with us .. close quit and die */
		delete this;
		return 1;
	}
	
	if( stage == 0 && pos > 1 && pos == data[1] + 2 )
	{
		bool noauthfound = false;
		for( int x = 2 ; x < pos ; x++)
		{
			if ( data[x] == 0 )
			{
				noauthfound = true;
				break;
			}
		}
		
		if( !noauthfound )
		{
			/* no auth mood accaptibule */
			delete this;
			return 1;
		}
		else
		{
			SendData("\5\0", 2);
			stage = 1;
			pos = 0;
		}
		
	}
	if( stage == 1 && pos == 47)
	{
		std::string hash;
		for( int i = 5 ; i < 45 ; i++)
			hash += (char) data[i];
		
		char buf[20];
		sprintf(buf, "%d", socket);
	
		WokXMLTag sockettag(NULL, "socket");
		sockettag.AddAttr("socket", buf);
		
		/* If hook found dont close socket on class deletion */
		if( wls->SendSignal("SOCKS5 NewSession hash:" + hash,sockettag) )
			socket = 0;
		
		xml->AddAttr("stop", "Not needed anymore");
		delete this;
	}
	
	return 1;
	
	
	return 1;
}

int
Socks5HostSession::SendData(char *data, uint len)
{
	uint bcount;
	uint br;
	char *str;
	bcount = br = 0;

	str =(char *) data;
	while (bcount < len)
	{
		if ((br = send (socket, str, len - bcount, 0)) > 0)
		{
			bcount += br;
			str += br;
		}
		else if (br < 0)
			return (-1);
	}
	
	return(0);
}
