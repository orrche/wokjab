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


#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>

#include <sstream>
#include "openssl/sha.h"

#include "jep65send.h"



jep65send::jep65send(WLSignal *wls,  WokXMLTag *msgtag, std::string sport):
WLSignalInstance ( wls ),
session(msgtag->GetAttr("session")),
sid(msgtag->GetAttr("sid")),
to(msgtag->GetAttr("to")),
sport(sport)
{
	EXP_SIGHOOK("Jabber Stream File Send Abort " + sid, &jep65send::Abort, 1000);
	listening = false;
	WokXMLTag querytag(NULL, "query");
	WokXMLTag &itemtag = querytag.AddTag("item");
	itemtag.AddAttr("session", session);
	wls->SendSignal("Jabber Connection GetUserData", &querytag);
	me = itemtag.GetFirstTag("jid").GetBody();

	unsigned char buffer[50];
	WokXMLTag &filetag = msgtag->GetFirstTag("file");
	data = new WokXMLTag(*msgtag);
	
	fsize = size = atoi(filetag.GetAttr("size").c_str());
	file = msgtag->GetAttr("file");
	sid = msgtag->GetAttr("sid");
	
	if( (baserate = atoi(msgtag->GetAttr("rate").c_str())) > 0 )
	{
		throttled = true;
		rate = baserate;
		
		
		WokXMLTag time(NULL, "timer");
		time.AddAttr("time","1000");
		time.AddAttr("signal", "Jabber Stream File Send Method http://jabber.org/protocol/bytestreams TimeOut "+sid);
		wls->SendSignal("Woklib Timmer Add", &time);
		
		EXP_SIGHOOK("Jabber Stream File Send Method http://jabber.org/protocol/bytestreams TimeOut " + sid, &jep65send::Timeout, 1000);
	}
	else
	{
		throttled = false;
	}
	
	fbpos = 0;
	
	socket = 0;
	hash = "";
	std::string digest = sid + me +  to;
	SHA1((unsigned char *)digest.c_str(), digest.size(), buffer);
	for( int i = 0 ; i < 20 ; i++)
	{
		char buf2[3];
		if(buffer[i] < 16)
			sprintf(buf2, "0%x", buffer[i]);
		else
			sprintf(buf2, "%x", buffer[i]);
		hash += buf2;
	}
	
	EXP_SIGHOOK("Jabber Stream File Send Method http://jabber.org/protocol/bytestreams push hash:" + hash, &jep65send::FileTransfear, 1000);
	
	proxy = msgtag->GetAttr("proxy");
	if ( proxy == "" )
	{
		SendInitiat(false);
	}
	else
	{
		InitProxy();
	}
	
}

jep65send::~jep65send()
{
	if ( socket ) 
		close ( socket);
	delete data;
}

void
jep65send::InitProxy()
{
	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", session);
	WokXMLTag &iqtag = msgtag.AddTag("iq");
	iqtag.AddAttr("type", "get");
	iqtag.AddAttr("to", proxy);
	WokXMLTag &querytag = iqtag.AddTag("query");
	querytag.AddAttr("xmlns", "http://jabber.org/protocol/bytestreams");
	
	wls->SendSignal("Jabber XML IQ Send", &msgtag);
	
	EXP_SIGHOOK("Jabber XML IQ ID "  + iqtag.GetAttr("id"), &jep65send::InitProxyReply, 500);
	
/*
	<iq type='get' 
    from='initiator@host1/foo' 
    to='proxy.host3' 
    id='discover'>
  <query xmlns='http://jabber.org/protocol/bytestreams'/>
</iq>
*/
}

int
jep65send::Abort(WokXMLTag *tag)
{
	close(socket);
	delete this;
	return 1;
}

int
jep65send::InitProxyReply(WokXMLTag *tag)
{
	if ( tag->GetFirstTag("iq").GetAttr("type") == "result" )
	{
		std::cout << "tag: " << tag << std::endl;

		
		pjid = tag->GetFirstTag("iq").GetFirstTag("query").GetFirstTag("streamhost").GetAttr("jid");
		phost = tag->GetFirstTag("iq").GetFirstTag("query").GetFirstTag("streamhost").GetAttr("host");
		pzeroconf = tag->GetFirstTag("iq").GetFirstTag("query").GetFirstTag("streamhost").GetAttr("zeroconf");
		pport = tag->GetFirstTag("iq").GetFirstTag("query").GetFirstTag("streamhost").GetAttr("port");
		std::cout << "PJID: " << pjid << " PHost: " << phost << " zconf: " << pzeroconf << std::endl;
		
		
		
		
	
		WokXMLTag msgtag(NULL, "message");
		msgtag.AddAttr("session", session);
		WokXMLTag &iqtag = msgtag.AddTag("iq");
		iqtag.AddAttr("type", "set");
		iqtag.AddAttr("to", to);
		WokXMLTag &querytag = iqtag.AddTag("query");
		querytag.AddAttr("xmlns", "http://jabber.org/protocol/bytestreams");
		querytag.AddAttr("sid", sid);
		querytag.AddAttr("mode", "tcp");
		WokXMLTag &streamhost = querytag.AddTag("streamhost");
		streamhost.AddAttr("port", pport);
		streamhost.AddAttr("host", phost);
		streamhost.AddAttr("jid", proxy);
		
		wls->SendSignal("Jabber XML IQ Send", &msgtag);
		EXP_SIGHOOK("Jabber XML IQ ID "  + iqtag.GetAttr("id"), &jep65send::InitProxyConnect, 500);
	
	}
	else
	{
		woklib_error(wls, "Proxy didn't agree!!");
	}

	return 1;
/*
<iq type='result' 
    from='proxy.host3' 
    to='initiator@host1/foo' 
    id='discover'>
  <query xmlns='http://jabber.org/protocol/bytestreams'>
    <streamhost 
        jid='proxy.host3' 
        host='24.24.24.1' 
        zeroconf='_jabber.bytestreams'/>
  </query>
</iq>
*/
}

int
jep65send::InitProxyConnect(WokXMLTag *tag)
{
	WokXMLTag sockettag ( NULL, "socket");
	sockettag.AddAttr("hostname", phost);
	sockettag.AddAttr("port", pport);
	sockettag.AddAttr("cmd", "1");
	sockettag.AddAttr("atype", "3");
	sockettag.AddAttr("dst.addr", hash);
	sockettag.AddAttr("dst.port", "0");
	
	wls->SendSignal("SOCKS5 Connect", sockettag);
	
	EXP_SIGHOOK("SOCKS5 Connection Established " + sockettag.GetAttr("id"), &jep65send::SOCKS_Established, 1000);
	EXP_SIGHOOK("SOCKS5 Connection Data " + sockettag.GetAttr("id"), &jep65send::SOCKS_Data, 1000);
	
	return 1;
}

int
jep65send::SOCKS_Established(WokXMLTag *tag)
{
	socket = atoi(tag->GetAttr("socket").c_str());
	ReqProxy();
	return 1;
}

int
jep65send::SOCKS_Data(WokXMLTag *tag)
{
	// If we get here the connection is most likely closed
	char buffer[10];
	int len;
	
	len = recv (socket, buffer, 10, 0);
	if ( len == 0 )
	{
		delete this;
	}
	return 1;
}

void
jep65send::ReqProxy()
{
	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", session);
	WokXMLTag &iqtag = msgtag.AddTag("iq");
	iqtag.AddAttr("type", "set");
	iqtag.AddAttr("to", proxy);
	WokXMLTag &querytag = iqtag.AddTag("query");
	querytag.AddAttr("xmlns", "http://jabber.org/protocol/bytestreams");
	querytag.AddAttr("sid", sid);
	querytag.AddTag("activate").AddText(to);
	
	wls->SendSignal("Jabber XML IQ Send", &msgtag);
	
	EXP_SIGHOOK("Jabber XML IQ ID "  + iqtag.GetAttr("id"), &jep65send::ProxyReply, 500);
}

int 
jep65send::ProxyReply(WokXMLTag *tag)
{
	if ( tag->GetFirstTag("iq").GetAttr("type") == "result" )
	{
		
			
		WokXMLTag contag(NULL, "connected");
		contag.AddAttr("sid", sid);
		wls->SendSignal("Jabber Stream File Status", &contag);
		wls->SendSignal("Jabber Stream File Status Connected", &contag);
		
		if (  ! listening ) 
		{
			std::stringstream sstr_socket;
			sstr_socket << socket;
			WokXMLTag sigtag(NULL, "socket");
			sigtag.AddAttr("socket", sstr_socket.str());
			listening = true;
			wls->SendSignal("Woklib Socket Out Add", sigtag);
			EXP_SIGHOOK(sigtag.GetAttr("signal"), &jep65send::SocketAvailibule, 1000);
		}
		ffile.open(file.c_str(), std::ios::in);
	}
	else
	{
		delete this;
		// Damn no proxy
	}

	return 1;
}

void
jep65send::SendInitiat( bool use_proxy)
{
	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", session);
	WokXMLTag &iqtag = msgtag.AddTag("iq");
	iqtag.AddAttr("type", "set");
	iqtag.AddAttr("to", to);
	WokXMLTag &querytag = iqtag.AddTag("query");
	querytag.AddAttr("xmlns", "http://jabber.org/protocol/bytestreams");
	querytag.AddAttr("sid", sid);
	querytag.AddAttr("mode", "tcp");
	WokXMLTag &streamhost = querytag.AddTag("streamhost");
	streamhost.AddAttr("port", sport);
	
	WokXMLTag userinfo(NULL, "userinfo");
	WokXMLTag &itemtag = userinfo.AddTag("item");
	itemtag.AddAttr("session", session);
	wls->SendSignal("Jabber Connection GetUserData", &userinfo);
	streamhost.AddAttr("host", itemtag.GetFirstTag("ip").GetBody());
	streamhost.AddAttr("jid", itemtag.GetFirstTag("jid").GetBody());
	
	wls->SendSignal("Jabber XML IQ Send", &msgtag);
	EXP_SIGHOOK("Jabber XML IQ ID " + iqtag.GetAttr("id"), &jep65send::TransfearStart, 1000);
}



const std::string &
jep65send::GetHash()
{
	return hash;
}

/* Opening file and getting ready to send data 
 * Jabber XML IQ ID *
 */
int
jep65send::TransfearStart(WokXMLTag *tag)
{
	EXP_SIGUNHOOK("Jabber Stream File Send Method http://jabber.org/protocol/bytestreams push hash:" + hash, &jep65send::FileTransfear, 1000);
	if ( socket ) 
	{
		if ( ! listening ) 
		{
			WokXMLTag sigtag(NULL, "socket");
			sigtag.AddAttr("socket", tag->GetAttr("socket"));
			listening = true;
			wls->SendSignal("Woklib Socket Out Add", sigtag);
			EXP_SIGHOOK(sigtag.GetAttr("signal"), &jep65send::SocketAvailibule, 1000);
		}
		ffile.open(file.c_str(), std::ios::in);
	}	
	return 1;
}

/* Sending start sequense 
 * Jabber Stream File Send Method http://jabber.org/protocol/bytestreams push hash:*
 */
int
jep65send::FileTransfear(WokXMLTag *tag)
{
	char buff[200];
	if(!recv (socket, buff, 200, 0))
	{
		WokXMLTag termtag(NULL, "terminated");
		termtag.AddAttr("sid", sid);
		wls->SendSignal("Jabber Stream File Status", &termtag);
		wls->SendSignal("Jabber Stream File Status Terminated", &termtag);
		
		tag->AddAttr("error", "terminated");
	}

	if( !socket )
	{
		socket = atoi(tag->GetAttr("socket").c_str());
		std::string data;
		
		char buf[5+hash.size() + 2];

		buf[0] = 5;
		buf[1] = 0;
		buf[2] = 0;
		buf[3] = 3;
		buf[4] = hash.size();
		strcpy(buf+5, hash.c_str());
		buf[5+hash.size()] = 0;
		buf[5+hash.size()+1] = 0;
		
		SendData(buf, 5 + 2 + hash.size());
	}
	
	WokXMLTag contag(NULL, "connected");
	contag.AddAttr("sid", sid);
	wls->SendSignal("Jabber Stream File Status", &contag);
	wls->SendSignal("Jabber Stream File Status Connected", &contag);
	
	return 1;
}

int
jep65send::Timeout(WokXMLTag *tag)
{
	if( !throttled )
		return false;
		
	if( rate < 0 )
	{
		
		std::stringstream sock;
		rate += baserate;
		
		if(!listening && rate >= 0)
		{
			sock << socket;
			listening = true;
			
			WokXMLTag sigtag(NULL, "socket");
			sigtag.AddAttr("socket", sock.str());
			wls->SendSignal("Woklib Socket Out Add", sigtag);
			EXP_SIGHOOK(sigtag.GetAttr("signal"), &jep65send::SocketAvailibule, 1000);
		}
	}
	/*
	else
		rate = baserate;
	*/
	return true;
}

/* Sending the file
 * -->Woklib Socket Out Add
 */
int
jep65send::SocketAvailibule( WokXMLTag *tag)
{
	int sent = 0;
	int maxsize;
	std::cout << "Availibule" << std::endl;
	if ( tag->GetAttr("error").size() )
	{
		WokXMLTag termtag(NULL, "terminated");
		termtag.AddAttr("sid", sid);
		wls->SendSignal("Jabber Stream File Status", &termtag);
		wls->SendSignal("Jabber Stream File Status Terminated", &termtag);
		delete this;
		return 1;	
	}
	
	
	if( SHUNKSIZE > baserate && throttled )
		maxsize = baserate;
	else
		maxsize = SHUNKSIZE;

	
	if(size > 0 )
	{
		if( fbpos != fbend )
		{
			sent = send ( socket, filebuf + fbpos, fbend - fbpos, MSG_DONTWAIT);
			if ( sent == -1 )
				return 1;
			fbpos += sent;
		}
		else
		{
			if( ffile.read(filebuf, maxsize) )
				fbend = maxsize;
			else
				fbend = ffile.gcount();
			sent = send ( socket, filebuf, fbend, 0 );
			if ( sent == -1 )
			{
				fbpos = 0;
				return 1;
			}
			fbpos = sent;
		}
	}
	
	std::cout << "Sent: " << sent << std::endl;
	
	if( fbpos == fbend)
	{
		size -= fbend;
		//fbpos = 0;
	}
	if( throttled )
	{
		rate-= sent;
		if(rate < 0)
		{
			listening = false;
			tag->AddAttr("stop", "finished");
		}
	}
	
	
	WokXMLTag postag(NULL, "position");
	std::stringstream pos;
	pos << fsize - size;
	postag.AddAttr("sid", sid);
	postag.AddAttr("pos", pos.str());
	wls->SendSignal("Jabber Stream File Status", &postag);
	wls->SendSignal("Jabber Stream File Status Position", &postag);
	
	
	if(size <= 0)
	{
		WokXMLTag fintag(NULL, "finished");
		fintag.AddAttr("sid", sid);
		wls->SendSignal("Jabber Stream File Status", &fintag);
		wls->SendSignal("Jabber Stream File Status Finished", &fintag);
	
		listening = false;
		tag->AddAttr("stop", "finished");
		delete this;
	}
	return 1;
}

/* Sending any data */
int
jep65send::SendData(char *data, uint len)
{
	std::cout << "We are sending data this way ... that wasn't the point..." << std::endl;

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
		{
			std::cout << "shit fatal error" << std::endl;
			return (-1);
		}
	}
	
	return(0);
}
