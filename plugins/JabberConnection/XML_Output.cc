/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <oden@gmx.net>
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
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>  // close

XML_Output::XML_Output(WLSignal *wls):
wls(wls)
{
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
XML_Output::sendxml(std::string data)
{
	if(!socket_nr)
	{
		WokXMLTag sigtag(NULL, "message");
		sigtag.AddTag("body").AddText("Session not connected");
		wls->SendSignal("Display Error", sigtag);
		
		return(-1);
	}
	
	uint bcount;
	uint br;
	char *str;
	std::string msg;
	
#ifdef DEBUG
	{
		WokXMLTag sigtag(NULL, "message");
		sigtag.AddAttr("type", "out");
		sigtag.AddTag("body").AddText(data);
		wls->SendSignal("Display Socket", sigtag);
	}
#endif // DEBUG
	
	bcount = br = 0;

	str =(char *) data.c_str();
	while (bcount < data.size())
	{
		
		if ( ssl )
			br = SSL_write(ssl, str, data.size() - bcount);
		else
			br = send(socket_nr, str, data.size() - bcount , 0);

		if (br > 0)
		{
			bcount += br;
			str += br;
		}
		else if (br < 0)
		{
			char buf[50];
			strerror_r(br, buf, 50);
			WokXMLTag sigtag(NULL, "message");
			sigtag.AddTag("body").AddText(std::string("Error Socket Send: ") + buf);
			wls->SendSignal("Display Error", sigtag);
			
			return (-1);
		}
	}
	
	return(0);
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
