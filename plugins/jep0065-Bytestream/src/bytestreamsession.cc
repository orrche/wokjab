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
	EXP_SIGHOOK("Jabber Stream File Send Abort " + sid, &jep65Session::Abort, 1000);
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
		lsid = reqauth.GetFirstTag("file").GetAttr("lsid");
		
		std::list <WokXMLTag*>::iterator hostiter;
		std::string s5id = "";
		
		for ( hostiter = xml->GetFirstTag("iq").GetFirstTag("query").GetTagList("streamhost").begin() ;
								hostiter != xml->GetFirstTag("iq").GetFirstTag("query").GetTagList("streamhost").end() ;
								hostiter++)
		{
			WokXMLTag sockettag ( NULL, "socket");
			sockettag.AddAttr("hostname", (*hostiter)->GetAttr("host"));
			sockettag.AddAttr("port", (*hostiter)->GetAttr("port"));
			sockettag.AddAttr("cmd", "1");
			sockettag.AddAttr("atype", "3");
			sockettag.AddAttr("dst.addr", sha1);
			sockettag.AddAttr("dst.port", "0");
			
			wls->SendSignal("SOCKS5 Connect", sockettag);
			if ( sockettag.GetAttr("result") != "error")
			{
				s5id = sockettag.GetAttr("id");
				usedstreamhost = (*hostiter)->GetAttr("jid");
			}
		}
		if ( !s5id.empty() )
		{
			EXP_SIGHOOK("SOCKS5 Connection Established " + s5id, &jep65Session::SOCKS_Established, 1000);
			EXP_SIGHOOK("SOCKS5 Connection Data " + s5id, &jep65Session::SOCKS_Data, 1000);
			EXP_SIGHOOK("SOCKS5 Connection Failed " + s5id, &jep65Session::SOCKS_Fail, 1000);
		}
		else
			delete this;
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

jep65Session::~jep65Session()
{
	if(file)
		file.close();
	delete orig;
}

#define BUFFSIZE 300000

int
jep65Session::SOCKS_Fail(WokXMLTag *tag)
{
	socket_nr = atoi(tag->GetAttr("socket").c_str());
		
	WokXMLTag contag(NULL, "Terminated");
	contag.AddAttr("sid", lsid);
	wls->SendSignal("Jabber Stream File Status", &contag);
	wls->SendSignal("Jabber Stream File Status Terminated", &contag);
		
	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", session);
	WokXMLTag &repiq = msgtag.AddTag("iq");
	repiq.AddAttr("type", "error");
	repiq.AddAttr("to", orig->GetFirstTag("iq").GetAttr("from"));
	repiq.AddAttr("id", orig->GetFirstTag("iq").GetAttr("id"));
	WokXMLTag &errortag = repiq.AddTag("error");
	errortag.AddAttr("code", "406");
	errortag.AddAttr("type", "auth");
	errortag.AddTag("not-acceptable").AddAttr("xmlns", "urn:ietf:params:xml:ns:xmpp-stanzas");
	
	wls->SendSignal("Jabber XML Send", &msgtag);
	
	
	/*
	<iq type='error' 
    from='target@example.org/bar' 
    to='initiator@example.com/foo' 
    id='initiate'>
  <error code='406' type='auth'>
    <not-acceptable xmlns='urn:ietf:params:xml:ns:xmpp-stanzas'/>
  </error>
</iq>
	*/
	
	delete this;
	return 1;	
}


int
jep65Session::Abort(WokXMLTag *tag)
{
	close(socket_nr);
	delete this;	
	
	return 1;
}

int
jep65Session::SOCKS_Data(WokXMLTag *tag)
{
	if( pos == 0 )
	{
		if ( filename.find("/") != std::string::npos )	
		{
			if (g_mkdir_with_parents(filename.substr(0, filename.rfind("/")).c_str(), 0777) == -1 )
			{
				woklib_error(wls, "Couldn't create directory " + filename.substr(0, filename.rfind("/") + 1));
				close ( socket_nr );
				delete this;
				return 1;
			}
		}

		file.open(filename.c_str(), std::ios::out);
		if ( !file )
		{
			woklib_error(wls, "Coldn't open file " + filename);
			close ( socket_nr );
			delete this;
			return 1;
		}
	}
	
		
	char buffer[BUFFSIZE];
	int len;
	
	len = recv (socket_nr, buffer, BUFFSIZE, 0);

	file.write(buffer, len);
	pos += len;
	
	WokXMLTag postag(NULL, "position");
	std::stringstream sspos;
	sspos << pos;
	postag.AddAttr("sid", lsid);
	postag.AddAttr("pos", sspos.str());
	wls->SendSignal("Jabber Stream File Status", &postag);
	wls->SendSignal("Jabber Stream File Status Position", &postag);
	
	file.flush();
	if ( len == 0 )
	{
		WokXMLTag fintag(NULL, "finished");
		fintag.AddAttr("sid", lsid);
		fintag.AddAttr("filename", filename);
		wls->SendSignal("Jabber Stream File Status", &fintag);
		wls->SendSignal("Jabber Stream File Status Finished", &fintag);
	
		close(socket_nr);
		tag->AddAttr("error", "closed");
		
		delete this;
	}
	
	return 1;
}

int 
jep65Session::SOCKS_Established(WokXMLTag *tag)
{
	socket_nr = atoi(tag->GetAttr("socket").c_str());
		
	WokXMLTag contag(NULL, "Connected");
	contag.AddAttr("sid", lsid);
	wls->SendSignal("Jabber Stream File Status", &contag);
	wls->SendSignal("Jabber Stream File Status Connected", &contag);
		
	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", session);
	WokXMLTag &repiq = msgtag.AddTag("iq");
	repiq.AddAttr("type", "result");
	repiq.AddAttr("to", orig->GetFirstTag("iq").GetAttr("from"));
	repiq.AddAttr("id", orig->GetFirstTag("iq").GetAttr("id"));
	WokXMLTag &querytag = repiq.AddTag("query");
	querytag.AddAttr("xmlns", "http://jabber.org/protocol/bytestreams");
	querytag.AddTag("streamhost-used").AddAttr("jid",usedstreamhost);
	wls->SendSignal("Jabber XML Send", &msgtag);
	return 1;
}


