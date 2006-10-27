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

#include "bytestreamsession.h"

#include "openssl/sha.h"
#include "vector"
#include <sstream>

jep65Session::jep65Session(WLSignal *wls, WokXMLTag *xml):
WLSignalInstance ( wls ),
initiator(xml->GetFirstTag("iq").GetAttr("from")),
target(xml->GetFirstTag("iq").GetAttr("to")),
sid(xml->GetFirstTag("iq").GetFirstTag("query").GetAttr("sid")),
session(xml->GetAttr("session"))
{
	unsigned char buffer[50];
	std::string iqmsg;
	pos = 0;
	stage = 0;
	iqmsg = "";
	
	orig = new WokXMLTag(*xml);
	
	std::string digest = sid + initiator +  target;

	SHA1((unsigned char *)digest.c_str(), digest.size(), buffer);
	for( int i = 0 ; i < 20 ; i++)
	{
		char buf2[3];
		if(buffer[i] < 16)
			sprintf(buf2, "0%x", buffer[i]);
		else
			sprintf(buf2, "%x", buffer[i]);
		iqmsg += buf2;
	}
	
	sha1 = iqmsg;
	
	WokXMLTag reqauth(NULL, "reqauth");
	reqauth.AddAttr("sid", sid);
	reqauth.AddAttr("initiator", initiator);
	reqauth.AddAttr("session", session);
	
	
	wls->SendSignal("Jabber Stream RequestAuthorisation", reqauth);
	if( reqauth.GetTagList("file").begin() != reqauth.GetTagList("file").end() )
	{	
		filename = reqauth.GetFirstTag("file").GetAttr("name");
		OpenConnection(xml->GetFirstTag("iq").GetFirstTag("query").GetFirstTag("streamhost").GetAttr("host"), 
									atoi(xml->GetFirstTag("iq").GetFirstTag("query").GetFirstTag("streamhost").GetAttr("port").c_str()));
		lsid = reqauth.GetFirstTag("file").GetAttr("lsid");
		
		char buf[20];
		sprintf(buf, "%d", socket_nr);
		WokXMLTag sigtag(NULL, "socket");
		sigtag.AddAttr("socket", buf);
		wls->SendSignal("Woklib Socket In Add", sigtag);
		EXP_SIGHOOK(sigtag.GetAttr("signal"), &jep65Session::ReadData, 1000);
		SendInitData();

		WokXMLTag contag(NULL, "Connected");
		contag.AddAttr("sid", lsid);
		wls->SendSignal("Jabber Stream File Status", &contag);
		wls->SendSignal("Jabber Stream File Status Connected", &contag);
	}
	else
	{
		WokXMLTag msgtag(NULL, "message");
		msgtag.AddAttr("session", session);
		WokXMLTag &unauthorized = msgtag.AddTag("iq");
		unauthorized.AddAttr("type", "error");
		unauthorized.AddAttr("to", xml->GetAttr("from"));
		unauthorized.AddAttr("id", xml->GetAttr("id"));
		WokXMLTag &error = unauthorized.AddTag("error");
		error.AddAttr("code","406");
		error.AddAttr("type", "auth");
		error.AddTag("not-acceptable").AddAttr("xmlns","urn:ietf:params:xml:ns:xmpp-stanzas");
		wls->SendSignal("Jabber XML Send", unauthorized);
		
		woklib_message(wls,"Unouthorized file transfear");
		delete this;
	}
}

int 
jep65Session::GetSocket()
{
	return socket_nr;
}

jep65Session::~jep65Session()
{
	if(file)
		file.close();
	delete orig;
}

int
jep65Session::OpenConnection(std::string host, int port)
{
	struct sockaddr_in sa;
	struct hostent *hp;
	
	if ((hp = gethostbyname (host.c_str())) == NULL)
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
	if (connect (socket_nr, (struct sockaddr *) &sa, sizeof sa) < 0)
	{
		close (socket_nr);
		return (-1);
	}

	return 1;
}

int
jep65Session::SendInitData()
{
	char data[50];
	data[0] = 5;
	data[1] = 1;
	data[2] = 0;
	
	senddata(data, 3);
	return 1;
}

#define BUFFSIZE 300000
int 
jep65Session::ReadData(WokXMLTag *xml)
{
	std::vector <std::string> proxy;
	char buffer[BUFFSIZE+1];
	char *buf;
	int len;
	
	len = recv (socket_nr, buffer, BUFFSIZE, 0);
	buf = buffer;
	
	while(buf - buffer < len )
	{
		if( pos == 1 && *buf == 0 && stage == 0)
		{
			stage = 1;
			pos = 0;
			char sbuf[] = {5,1,0,3,sha1.size() };
			senddata(sbuf, 5);
			senddata((char *)sha1.c_str(), 40);
			senddata("\0\0", 2);
		}
		else if( pos == 47 && stage == 1)
		{
			WokXMLTag msgtag(NULL, "message");
			msgtag.AddAttr("session", session);
			WokXMLTag &repiq = msgtag.AddTag("iq");
			repiq.AddAttr("type", "result");
			repiq.AddAttr("to", orig->GetFirstTag("iq").GetAttr("from"));
			repiq.AddAttr("id", orig->GetFirstTag("iq").GetAttr("id"));
			WokXMLTag &querytag = repiq.AddTag("query");
			querytag.AddAttr("xmlns", "http://jabber.org/protocol/bytestreams");
			querytag.AddTag("streamhost-used").AddAttr("jid",orig->GetFirstTag("iq").GetFirstTag("query").GetFirstTag("streamhost").GetAttr("jid"));
			wls->SendSignal("Jabber XML Send", &msgtag);
			stage = 2;
			pos = 0;
			break;
		}
		else if( stage == 2 )
		{
			if( pos == 0 )
			{
				file.open(filename.c_str(), std::ios::out);
			}
			
			
			file.write(buf, len);
			pos += len;
			
			WokXMLTag postag(NULL, "position");
			std::stringstream sspos;
			sspos << pos;
			postag.AddAttr("sid", lsid);
			postag.AddAttr("pos", sspos.str());
			wls->SendSignal("Jabber Stream File Status", &postag);
			wls->SendSignal("Jabber Stream File Status Position", &postag);
	
			
			break;
		}
		
		buf++;
		pos++;
		
	}
	
	/*
	Dont know this yet ...
	if( stage == 2 && pos > atoi(orig->GetFirstTag("iq").GetAttr("size").c_str()) )
	{
		std::cout << "Pos " << pos << " should be " << atoi(orig->GetFirstTag("iq").GetAttr("size").c_str()) << std::endl;
		close(socket_nr);
		xml->AddAttr("error", "oversize");
		WokXMLTag message(NULL, "message");
		message.AddTag("body").AddText("File transfear exceded file size\n");
		wls->SendSignal("Display Error", message);
	}
	*/
	
	if( len == 0 )
	{
		close(socket_nr);
		xml->AddAttr("error", "closed");
		delete this;
	}
	
	return true;
}


int
jep65Session::senddata(char *data, uint len)
{
	uint bcount;
	uint br;
	char *str;
	bcount = br = 0;

	str =(char *) data;
	while (bcount < len)
	{
		if ((br = send (socket_nr, str, len - bcount, 0)) > 0)
		{
			bcount += br;
			str += br;
		}
		else if (br < 0)
			return (-1);
	}
	
	return(0);
}

