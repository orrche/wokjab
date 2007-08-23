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
// Class: Socks5Session
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Dec 19 22:18:23 2006
//

#include "Socks5Session.h"
#include <sstream>
#include <fcntl.h> 

Socks5Session::Socks5Session(WLSignal *wls, WokXMLTag *tag, int id) : WLSignalInstance(wls),
socktag(new WokXMLTag(*tag)),
hostname(tag->GetAttr("hostname")),
port(atoi(tag->GetAttr("port").c_str())),
id(id)
{
	stage = 0;
	buffer = NULL;
	outbuffer = NULL;
	
	if ( OpenConnection() == -1 )
	{
		woklib_error(wls, "Couldn't open connection to the streamhost");
		tag->AddAttr("result", "error");
		delete this;
		return;
	}

	std::stringstream sstr_socket;
	sstr_socket << socket_nr;
	str_socket = sstr_socket.str();
	
	std::stringstream sstr_id;
	sstr_id << id;
	str_id = sstr_id.str();
	
	tag->AddAttr("id", str_id);
	
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


Socks5Session::~Socks5Session()
{
	delete socktag;
	delete [] buffer;
	delete [] outbuffer;
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
			else if ( len + pos > 2 )
			{
				close(socket_nr);
				tag->AddAttr("error", "closed");
				
				WokXMLTag contag ( NULL, "connection");
				contag.AddAttr("id", str_id);
				
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
	std::cout << "Debug.." << std::endl;
	// Version 5  Ways of authentication 1  Authentication method 0

	outpos += send (socket_nr, outbuffer , outsize - outpos, MSG_DONTWAIT);
	if ( outpos == outsize ) 
	{
		EXP_SIGUNHOOK(signal_out, &Socks5Session::Ready, 1000);
		delete [] outbuffer;
		outbuffer = NULL;
		
		if ( stage == 0 )
		{
			WokXMLTag contag ( NULL, "connection");
			contag.AddAttr("id", str_id);
			
			wls->SendSignal("SOCKS5 Connection Open " + str_id, contag);
		}
	}
	
	
}

int
Socks5Session::OpenConnection()
{	
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
	
	std::cout << "Starting this shit.." << std::endl;
	if ((socket_nr = socket (hp->h_addrtype, SOCK_STREAM, 0)) < 0)
		return (-1);
	std::cout << "Connecting... this might maybe be done none blocking but not sure" << std::endl;
	
	
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
	std::cout << "And we are fucking connected" << std::endl;

	return 1;
}


