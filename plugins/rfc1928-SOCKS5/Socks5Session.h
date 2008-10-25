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
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Dec 19 22:18:23 2006
//

#ifndef _SOCKS5SESSION_H_
#define _SOCKS5SESSION_H_



#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>

using namespace Woklib;
#define BUFFSIZE 8000

class Socks5Session : public WLSignalInstance
{
	public:
		Socks5Session(WLSignal *wls, WokXMLTag *tag, int id);
		 ~Socks5Session();
	
	int OpenConnection();
	void Request ();
	int Authentication(std::string data);
	int Ready(WokXMLTag *tag);
	int Read(WokXMLTag *tag);
	int Conn_Established(WokXMLTag *tag);
	int Conn_Fail(WokXMLTag *tag);
	int Conn_Data(WokXMLTag *tag);
	void Init();
		
	protected:
		WokXMLTag *socktag;
		std::string hostname;
		int port;
		int socket_nr;
		std::string str_socket;
		std::string str_id;
		std::string signal_out;
		std::string signal_in;
		int id;
		int stage;
		int pos;
		int outpos;
		int outsize;
		char *buffer;
		char *outbuffer;
};


#endif	//_SOCKS5SESSION_H_

