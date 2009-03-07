/***************************************************************************
 *  Copyright (C) 2003-2009  Kent Gustavsson <nedo80@gmail.com>
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
// Class: Socks5Session
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Dec 19 22:18:23 2006
//

#include "Socks5Session.h"
#include <sstream>
#include <fcntl.h> 
#include <string.h>
#include <errno.h>

Socks5Session::Socks5Session(WLSignal *wls, WokXMLTag *tag, int id) : WLSignalInstance(wls),
socktag(new WokXMLTag(*tag)),
hostname(tag->GetAttr("hostname")),
port(atoi(tag->GetAttr("port").c_str())),
id(id)
{
	stage = 0;
	buffer = NULL;
	outbuffer = NULL;
	
	std::stringstream sstr_id;
	sstr_id << id;
	str_id = sstr_id.str();
	
	tag->AddAttr("established", "SOCKS5 Connection Established " + str_id);
	tag->AddAttr("data", "SOCKS5 Connection Data " + str_id);
	tag->AddAttr("fail", "SOCKS5 Connection Fail " + str_id);
	
	std::cout << "CON DATA:" << *tag << std::endl;
				
	if ( ! tag->GetTagList("connect").empty() )
	{
		std::list <WokXMLTag *>::iterator iter;
		for ( iter = tag->GetTagList("connect").begin() ; iter != tag->GetTagList("connect").end() ; iter++ )
		{
			WokXMLTag conntag(**iter);
			wls->SendSignal("Connect " + (*iter)->GetAttr("xmlns"), conntag);
			
			
			EXP_SIGHOOK(conntag.GetAttr("established"), &Socks5Session::Conn_Established, 1000);
			EXP_SIGHOOK(conntag.GetAttr("data"), &Socks5Session::Conn_Data, 1000);
			EXP_SIGHOOK(conntag.GetAttr("fail"), &Socks5Session::Conn_Fail, 1000);
		}
	}
	else
	{
		if ( tag->GetAttr("socket").empty() )
		{
			if ( OpenConnection() == -1 )
			{
				woklib_error(wls, "Couldn't open connection to the streamhost");
				tag->AddAttr("result", "error");
				delete this;
				return;
			}
		}
		else	
			socket_nr = atoi(tag->GetAttr("socket").c_str());
		
		
		Init();
		
	}
}


Socks5Session::~Socks5Session()
{
	delete socktag;
	delete [] buffer;
	delete [] outbuffer;
}

int
Socks5Session::Conn_Established(WokXMLTag *tag)
{
	socket_nr = atoi(tag->GetAttr("socket").c_str());
	Init();
	return 1;
}

int
Socks5Session::Conn_Fail(WokXMLTag *tag)
{
	WokXMLTag contag ( NULL, "connection");
	contag.AddAttr("id", str_id);
	contag.GetFirstTag("message").GetFirstTag("body").AddText("Previous connection failed - " + tag->GetFirstTag("message").GetFirstTag("body").GetBody());
	contag.AddAttr("error", tag->GetAttr("error"));
	
	wls->SendSignal("SOCKS5 Connection Failed " + str_id, contag);
	
	return 1;
}

int
Socks5Session::Conn_Data(WokXMLTag *tag)
{
	
	return 1;
}


void
Socks5Session::Init()
{
	std::stringstream sstr_socket;
	sstr_socket << socket_nr;
	str_socket = sstr_socket.str();
	outbuffer = new char[3];
	memcpy(outbuffer, "\x05\x01\x00", 3);
	outpos = 0;
	outsize = 3;
	
	WokXMLTag sigtag(NULL, "socket");
	sigtag.AddAttr("socket", str_socket);
	
	wls->SendSignal("Woklib Socket In Add", sigtag);
	signal_in = sigtag.GetAttr("signal");
	EXP_SIGHOOK(signal_in, &Socks5Session::Read, 1000);
	
	wls->SendSignal("Woklib Socket Out Add", sigtag);
	signal_out = sigtag.GetAttr("signal");
	EXP_SIGHOOK(signal_out, &Socks5Session::Ready, 1000);

}

void
Socks5Session::Request()
{
	char sbuf[] = {5,
				atoi(socktag->GetAttr("cmd").c_str()),
				0,
				atoi(socktag->GetAttr("atype").c_str()),
				socktag->GetAttr("dst.addr").size() };
	
	if ( outbuffer )
	{
		woklib_error(wls, "Sock5 Outputbuffer collition");
		delete this;
		return;
	}
	outbuffer = new char[socktag->GetAttr("dst.addr").size() + 7];
	memcpy(outbuffer, sbuf, 5);
	memcpy(outbuffer+5, socktag->GetAttr("dst.addr").c_str(), socktag->GetAttr("dst.addr").size());
	outbuffer[socktag->GetAttr("dst.addr").size() + 5] = atoi(socktag->GetAttr("dst.port").c_str())%256;
	outbuffer[socktag->GetAttr("dst.addr").size() + 6] = atoi(socktag->GetAttr("dst.port").c_str())/256;
	outpos = 0;
	outsize = socktag->GetAttr("dst.addr").size() + 7;
	
	WokXMLTag sigtag(NULL, "socket");
	sigtag.AddAttr("socket", str_socket);
	
	wls->SendSignal("Woklib Socket Out Add", sigtag);
	signal_out = sigtag.GetAttr("signal");
	EXP_SIGHOOK(signal_out, &Socks5Session::Ready, 1000);
}


int
Socks5Session::Authentication(std::string data)
{
	if ( data[0] == 5 && data[1] == 0 )
	{
		WokXMLTag contag ( NULL, "connection");
		contag.AddAttr("id", str_id);
		
		wls->SendSignal("SOCKS5 Connection Authenticated " + str_id, contag);
	}
	else
	{
		woklib_message(wls, "Proxy not accepting authentication");
		
		WokXMLTag contag ( NULL, "connection");
		contag.AddAttr("id", str_id);
		contag.GetFirstTag("message").GetFirstTag("body").AddText("Proxy not accepting authentication");
		
		wls->SendSignal("SOCKS5 Connection Failed " + str_id, contag);
		
		delete this;
		return 0;
	}
	
	return 1;
}


int
Socks5Session::Read(WokXMLTag *tag)
{
	if ( ! tag->GetAttr("error").empty() )
	{		
	
		WokXMLTag contag ( NULL, "connection");
		contag.AddAttr("id", str_id);
		contag.GetFirstTag("message").GetFirstTag("body").AddText("Socket closed unexpectedly - " + tag->GetAttr("error"));
		
		wls->SendSignal("SOCKS5 Connection Failed " + str_id, contag);
		close(socket_nr);
		delete this;
		return 1;
	}
	
	if ( stage < 2 )
	{
		if( !buffer )
		{
			buffer = new char[BUFFSIZE];
			pos = 0;
		}
			
		if ( BUFFSIZE - pos == 0 )
		{
			// Ohh what now ?
			close( socket_nr );
			delete this;
			return 1;		
		}
		
		std::cout << "socket nr " << socket_nr << " pos " << pos << " BUFFSIZE-pos " << BUFFSIZE-pos << std::endl;
		int len = recv (socket_nr, buffer+pos, BUFFSIZE-pos, 0);

		
		if ( stage == 0 )
		{
			if ( len + pos == 2 )
			{
				Authentication( std::string(buffer, 2));
			
				delete buffer;
				buffer = NULL;
				stage++;
				
				Request();
			}
			else if ( len + pos > 2 || len < 0)
			{
				close(socket_nr);
				tag->AddAttr("error", "closed");
				
				WokXMLTag contag ( NULL, "connection");
				contag.AddAttr("id", str_id);
				contag.GetFirstTag("message").GetFirstTag("body").AddText("Proxy protocol data parsing error: ");
				contag.GetFirstTag("message").GetFirstTag("body").AddText(strerror(errno));
				
		
				wls->SendSignal("SOCKS5 Connection Failed " + str_id, contag);
				delete this;
				
				return 1;
			}
		}
		if( stage == 1 )
		{
			if ( len + pos == socktag->GetAttr("dst.addr").size() + 7 )
			{
				stage++;
				
				if ( buffer[1] != 0 )
				{
					std::string errmsg = "";
					switch ( buffer[1] )
					{
						case 1:
							errmsg = "general SOCKS server failure";
							break;						
						case 2:
							errmsg = "connection not allowed by ruleset";
							break;						
						case 3:
							errmsg = "Network unreachable";
							break;						
						case 4:
							errmsg = "Host unreachable";
							break;						
						case 5:
							errmsg = "Connection refused";
							break;						
						case 6:
							errmsg = "TTL expired";
							break;						
						case 7:
							errmsg = "Command not supported";
							break;						
						case 8:
							errmsg = "Address type not supported";
							break;
						default:
							errmsg = "Undefined error";
					}
					close(socket_nr);
					delete this;
					return 1;
				}
				/*
		     o  X'00' succeeded
             o  X'01' general SOCKS server failure
             o  X'02' connection not allowed by ruleset
             o  X'03' Network unreachable
             o  X'04' Host unreachable
             o  X'05' Connection refused
             o  X'06' TTL expired
             o  X'07' Command not supported
             o  X'08' Address type not supported
             o  X'09' to X'FF' unassigned
				*/
				
				WokXMLTag contag ( NULL, "connection");
				contag.AddAttr("id", str_id);
				
				
				contag.AddAttr("socket", str_socket);
				wls->SendSignal("SOCKS5 Connection Established " + str_id, contag);
				
				delete buffer;
				delete outbuffer;
				
				buffer = outbuffer = NULL;
			}
		}

		if( len == 0 )
		{
			close(socket_nr);
			tag->AddAttr("error", "closed");
			
			WokXMLTag contag ( NULL, "connection");
			contag.AddAttr("id", str_id);
			contag.GetFirstTag("message").GetFirstTag("body").AddText("0 data recived .. does this really mean the connection is lost?");
		
			wls->SendSignal("SOCKS5 Connection Failed " + str_id, contag);
			delete this;
		}
	}
	else
	{
	
		if ( ! wls->SendSignal("SOCKS5 Connection Data " + str_id, tag) )
		{
			close(socket_nr);
			tag->AddAttr("stop", "closed");
			delete this;
		}
	
	}
	
	return 1;
}

int
Socks5Session::Ready(WokXMLTag *tag)
{
	// Version 5  Ways of authentication 1  Authentication method 0
	
	outpos += send (socket_nr, outbuffer , outsize - outpos, MSG_DONTWAIT);
	if ( outpos == outsize ) 
	{
		EXP_SIGUNHOOK(signal_out, &Socks5Session::Ready, 1000);
		tag->AddAttr("stop", "not needed anymore");
		
		delete [] outbuffer;
		outbuffer = NULL;
		
		if ( stage == 0 )
		{
			WokXMLTag contag ( NULL, "connection");
			contag.AddAttr("id", str_id);
			
			wls->SendSignal("SOCKS5 Connection Open " + str_id, contag);
		}
	}
	
	return 1;
}

int
Socks5Session::OpenConnection()
{	
	std::cout << "Using OpenConnection()" << std::endl;
	struct sockaddr_in sa;
	struct hostent *hp;
	
	if ((hp = gethostbyname (hostname.c_str())) == NULL)
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
	
	
	int flags;
	if ((flags = fcntl(socket_nr, F_GETFL, 0)) < 0)
	{
		woklib_error(wls, "Socks5 Connection couldn't get socket flags");
	}


	if (fcntl(socket_nr, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		woklib_error(wls, "Socks5 Connection couldn't set non blocking socket");
	} 
	
	
	if (connect (socket_nr, (struct sockaddr *) &sa, sizeof sa) < 0)
	{
		return 1;
		close (socket_nr);
		return (-1);
	}

	return 1;
}


