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


#include "IQauth.h"
#include <Woklib/WLSignal.h>
#include <Woklib/WoklibPlugin.h>
#include <Woklib/WokXMLTag.h>
#include <iostream>

#include "openssl/sha.h"

using std::cout;
using std::endl;

IQauth::IQauth (WLSignal *wls, std::string con_id, std::string session):
WLSignalInstance ( wls ),
con_id(con_id),
session(session)
{
	WokXMLTag querytag(NULL,"variables");
	WokXMLTag &itemtag = querytag.AddTag("item");
	itemtag.AddAttr("session", session);
	
	/* Gets the connection variables .. from Connection in woklib */
	wls->SendSignal("Jabber Connection GetUserData", &querytag);
	
	username = itemtag.GetFirstTag("username").GetBody();
	resource = itemtag.GetFirstTag("resource").GetBody();
	password = itemtag.GetFirstTag("password").GetBody();
	con_type = atoi(itemtag.GetFirstTag("type").GetBody().c_str());
	
	//switch (con_type)
	switch(ClearTextUser)
	{
		case ClearTextUser:
			InitClearTextUser();
			break;
		case SHA1User:
			InitSHA1UserStage1();
			break;
		case HandshakeComponent:
			InitHandshakeComponent();
			break;
		default:
			InitSHA1UserStage1();
	}
}

IQauth::~IQauth ()
{

}

void
IQauth::InitHandshakeComponent()
{
	unsigned char buffer[30];
	std::string iqmsg;
	
	iqmsg = "<handshake>";
	
	woklib_debug(wls,"Init handshake auth");
	std::string digest = con_id + password;
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
	
	iqmsg += "</handshake>";
	
	WokXMLTag msgtag(NULL,"message");
	msgtag.AddAttr("session", session);
	msgtag.AddText(iqmsg);
	wls->SendSignal("Jabber Send XML Send" , &msgtag);
}

void
IQauth::InitClearTextUser()
{
	woklib_debug(wls,"Init clear text auth");
	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", session);
	WokXMLTag &iqtag = msgtag.AddTag("iq");
	iqtag.AddAttr("type", "set");
	WokXMLTag &querytag = iqtag.AddTag("query");
	querytag.AddAttr("xmlns", "jabber:iq:auth");
	querytag.AddTag("username").AddText(username);
	querytag.AddTag("resource").AddText(resource);
	querytag.AddTag("password").AddText(password);
	
	wls->SendSignal("Jabber XML IQ Send", &msgtag);
	signal = std::string("Jabber XML IQ ID ") + iqtag.GetAttr("id");
	
	EXP_SIGHOOK(signal, &IQauth::xmlClearTextUser, 1000);
}

void
IQauth::InitSHA1UserStage1()
{
	woklib_debug(wls,"Init sha1 auth");
	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", session);
	WokXMLTag &iqtag = msgtag.AddTag("iq");
	iqtag.AddAttr("type", "get");
	WokXMLTag &querytag = iqtag.AddTag("query");
	querytag.AddAttr("xmlns", "jabber:iq:auth");
	querytag.AddTag("username").AddText(username);
	
	wls->SendSignal("Jabber XML IQ Send", &msgtag);
	signal = std::string("Jabber XML IQ ID ") + iqtag.GetAttr("id");
	
	EXP_SIGHOOK(signal, &IQauth::InitSHA1UserStage2 , 1000);
	
}

void
IQauth::InitSHA1UserStage2(WokXMLTag *tag)
{
	WokXMLTag* tag_iq;
	unsigned char buffer[30];
	tag_iq = &tag->GetFirstTag("iq");
	std::string iqmsg;
	
	EXP_SIGUNHOOK(signal, &IQauth::InitSHA1UserStage2, 1000);
	
	if(tag_iq->GetAttr("type") == "error")
	{
		/* FIX ME do something sane here ... */
		woklib_error(wls, "Authentication failed");
		delete this;
		return;		
	}
		
	WokXMLTag msgtag(NULL,"message");
	msgtag.AddAttr("session", session);
	WokXMLTag &iqtag = msgtag.AddTag("iq");
	iqtag.AddAttr("type", "set");
	WokXMLTag &querytag = iqtag.AddTag("query");
	querytag.AddAttr("xmlns", "jabber:iq:auth");
	querytag.AddTag("username").AddText(username);
	querytag.AddTag("resource").AddText(resource);
	
	iqmsg = "";
	std::string digest = con_id + password;
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
	
	querytag.AddTag("digest").AddText(iqmsg);
	
	wls->SendSignal("Jabber XML IQ Send", &msgtag);
	signal = std::string("Jabber XML IQ ID ") + iqtag.GetAttr("id");
	
	EXP_SIGHOOK(signal, &IQauth::xmlClearTextUser, 1000);
}


int
IQauth::xmlClearTextUser (WokXMLTag *tag)
{
	EXP_SIGHOOK(signal, &IQauth::xmlClearTextUser, 1000);
	
	
	WokXMLTag *tag_iq;
	tag_iq = &tag->GetFirstTag("iq");
	
	if(tag_iq->GetAttr("type") == "result")
	{
		WokXMLTag msgtag(NULL, "message");
		msgtag.AddAttr("session", tag->GetAttr("session"));
		wls->SendSignal("Jabber Connection Authenticated", &msgtag);
	}
	else if ( tag_iq->GetAttr("type") == "error")
	{
		woklib_error(wls, "Authentication failed");
		WokXMLTag tag(NULL, "message");
		tag.AddAttr("session", session);
		wls->SendSignal("Jabber Connection Disconnect", &tag);
	}
	delete this;
	return 1;
}
