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

/* This is one of the sader classes in this library
 * Horribule horribule code will follow here after
 */

#include "XML_Output.h"
#include <iostream>
#ifdef __WIN32
#include <winsock.h>
typedef unsigned int uint;
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#endif
#include <errno.h>
#include <unistd.h>  // close
#include <sstream>

XML_Output::XML_Output(WLSignal *wls, std::string session): WLSignalInstance(wls),
session(session)
{	
	EXP_SIGHOOK("Jabber Connection Disconnect '" + XMLisize(session) + "'", &XML_Output::SignalDisconnect, 1000);
	transmitting = false;
	this->socket_nr = 0;
	ssl = NULL;
}

XML_Output::~XML_Output()
{

}

void
XML_Output::set_socket(int socket_nr)
{
	this->socket_nr = socket_nr;
}

int
XML_Output::SignalDisconnect(WokXMLTag *tag)
{
	sendxml("<stream::stream/>");
	
}

int
XML_Output::sendxml(std::string data)
{
	if(!socket_nr)
	{
		WokXMLTag sigtag(NULL, "message");
		sigtag.AddTag("body").AddText("Session not connected");
		wls->SendSignal("Display Error", sigtag);
				
		WokXMLTag msg(NULL, "message");
		msg.AddAttr("session", session);
		wls->SendSignal("Jabber Connection Lost", &msg);
		
		return(-1);
	}

	buffer += data;
	
	if ( !transmitting )
	{
		transmitting = true;
		
		std::stringstream sstr_socket;
		std::string str_socket;
		
		sstr_socket << socket_nr;
		str_socket = sstr_socket.str();
		
		WokXMLTag sigtag(NULL, "socket");
		sigtag.AddAttr("socket", str_socket);

		wls->SendSignal("Woklib Socket Out Add", sigtag);
		signal_out = sigtag.GetAttr("signal");
		EXP_SIGHOOK(signal_out, &XML_Output::SocketAvailibule, 1000);
	}
}


int
XML_Output::SocketAvailibule(WokXMLTag *tag)
{
	int bcount;
	int br;
	char *str;

	WokXMLTag activetag (NULL, "active");
	activetag.AddAttr("session", session);
	wls->SendSignal("Jabber Session IO Active", activetag);
	wls->SendSignal("Jabber Session IO Active " + session, activetag);
	
#ifdef DEBUG
	{
		WokXMLTag sigtag(NULL, "message");
		sigtag.AddAttr("type", "out");
		sigtag.AddTag("body").AddText(buffer);
		sigtag.AddAttr("session", session);
		wls->SendSignal("Display Socket", sigtag);
	}
#endif // DEBUG

	bcount = br = 0;
	str =(char *) buffer.c_str();

	if ( ssl )
	{
		// No idea at all why this is nessesary
		if ( buffer.size() > 5000 )
			br = SSL_write(ssl, str, 5000);
		else
			br = SSL_write(ssl, str, buffer.size());
	}
	else
		br = send(socket_nr, str, buffer.size() , 0);

	if (br > 0)
	{
		buffer = buffer.substr(br);
	}
	else if (br < 0)
	{
#ifdef __WIN32
#else
		char buf[50];
		strerror_r(br, buf, 50);
		WokXMLTag sigtag(NULL, "message");
		sigtag.AddTag("body").AddText(std::string("Error Socket Send: ") + buf);
		wls->SendSignal("Display Error", sigtag);
#endif
		socket_nr = 0;
	}
	
	if ( buffer.size() == 0 )
	{
	
		EXP_SIGUNHOOK(signal_out, &XML_Output::SocketAvailibule, 1000);
		transmitting = false;
		tag->AddAttr("stop","no more data");
	}
	return(1);
}

int
XML_Output::sendxml(const char *data)
{
	std::string xml_data;
	int ret;

	xml_data = data;
	ret = sendxml(xml_data);

	return(ret);
}

void
XML_Output::SetSSL(SSL * s)
{
	ssl = s;
}
