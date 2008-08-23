/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <nedo80@gmail.com>
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

#include <fcntl.h> 

#include "recvsocket.h"
#include <sstream>

RecvSocket::RecvSocket(WLSignal *wls, int socket):
WLSignalInstance ( wls ),
socket(socket)
{
	char buf[20];
	int flags;
	
	if ((flags = fcntl(socket, F_GETFL, 0)) < 0)
	{
		woklib_error(wls, "Socks5 Connection couldn't get socket flags");
	}

	if (fcntl(socket, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		woklib_error(wls, "Socks5 Connection couldn't set non blocking socket");
	} 
	
	sprintf(buf, "%d", socket);
	WokXMLTag sigtag(NULL, "socket");
	sigtag.AddAttr("socket", buf);
	wls->SendSignal("Woklib Socket In Add", sigtag);
	EXP_SIGHOOK(sigtag.GetAttr("signal"), &RecvSocket::ReadData, 1000);
	
	pos = 0;
	stage = 0;
}

RecvSocket::~RecvSocket()
{
	if ( socket ) 
		close( socket );
}

int
RecvSocket::ReadData(WokXMLTag *xml)
{
	int len;
	len = recv (socket, data + pos, BUFFERSIZE - pos, 0);
	pos += len;

	if ( len <= 0 )
		perror(NULL);
	
	if( len == 0 )
	{
		xml->AddAttr("error","terminated");
		delete this;
		return 1;
	}
	
	if( pos > 260 )
	{
		/* Someone is just fucking with us .. close quit and die */
		xml->AddAttr("error","terminated");
		delete this;
		return 1; 
	}
	
	if( stage == 0 && pos > 1 && pos == data[1] + 2 )
	{
		bool noauthfound = false;
		for( int x = 2 ; x < data[1] + 2 ; x++)
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
		if( wls->SendSignal("Jabber Stream File Send Method http://jabber.org/protocol/bytestreams push hash:" + hash,sockettag) )
			socket = 0;
		
		xml->AddAttr("stop", "Not needed anymore");
		delete this;
	}
	
	return 1;
}

int
RecvSocket::SendData(char *data, uint len)
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
