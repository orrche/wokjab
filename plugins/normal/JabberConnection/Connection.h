/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <nedo80@gmail.com>
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

#ifndef __WOKJAB_CONNECTION_H
#define __WOKJAB_CONNECTION_H

#include <openssl/ssl.h>

#ifdef __WIN32
#else
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#endif

#include <errno.h>
#include <unistd.h>  // close
#include <string>


class Connection;


#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>

#include "SSL.h"
#include "XML_Input.h"
#include "XML_Output.h"

#include "Jabber.h"

using namespace Woklib;

class Connection : public WLSignalInstance
{
      public:
	Connection (WLSignal *wls, std::string in_username, std::string in_password, std::string in_server, std::string in_host, std::string resource, int in_port, int type, std::string session);
	~Connection();
	int get_socket();

	XML_Input *xmlinput;
	XML_Output *xmloutput;

	int GetUserVariables(WokXMLTag *tag);
	int ReadData(WokXMLTag *tag);
	int GetSocket(WokXMLTag *tag);
	int GetMyNick(WokXMLTag *tag);
	int Reset(WokXMLTag *tag);
	int SetJID(WokXMLTag *tag);
	
	void sendinit();

	void StartSSL();
	void SetSSL(::SSL *s);
	std::string GetConID();
	void SetConID(const std::string &id);

	std::string username;
	std::string password;
	std::string server;
	std::string host;
	std::string resource;
	std::string session;
	std::string ip;
	std::string conid;
	private:

	int openconnection();

	int port;
	int socket_nr;
	int type;

	Woklib::SSL *ssl;
};

#endif //__WOKJAB_CONNECTION_H
