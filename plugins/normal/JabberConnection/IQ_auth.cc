/***************************************************************************
 *  Copyright (C) 2003-2009  Kent Gustavsson <nedo80@gmail.com>
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

#include <openssl/sha.h>
#include <openssl/md5.h>

IQauth::IQauth (WLSignal *wls, std::string session, int in_con_type):
WLSignalInstance ( wls ),
session(session),
con_type(in_con_type)
{
	EXP_SIGHOOK("Jabber XML Object challenge", &IQauth::SD_Challange, 1000);
	EXP_SIGHOOK("Jabber XML Object success", &IQauth::SD_Success, 1000);
	EXP_SIGHOOK("Jabber XML Object failure", &IQauth::SD_Failure, 1000);
	
	WokXMLTag querytag(NULL,"variables");
	WokXMLTag &itemtag = querytag.AddTag("item");
	itemtag.AddAttr("session", session);
	
	/* Gets the connection variables .. from Connection in woklib */
	wls->SendSignal("Jabber Connection GetUserData", &querytag);
	
	username = itemtag.GetFirstTag("username").GetBody();
	resource = itemtag.GetFirstTag("resource").GetBody();
	password = itemtag.GetFirstTag("password").GetBody();
	server = itemtag.GetFirstTag("server").GetBody();
	con_id = itemtag.GetFirstTag("conid").GetBody();

	switch (con_type)
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
		case SASLDIGESTMD5:
			InitSASLDIGESTMD5();
			break;
		case SASLPLAIN:
			InitSASLPLAIN();
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
	
	iqmsg = "";
	
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
	
	
	WokXMLTag msgtag(NULL,"message");
	msgtag.AddAttr("session", session);
	msgtag.AddTag("handshake").AddText(iqmsg);
	wls->SendSignal("Jabber XML Send" , &msgtag);
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

std::string
IQauth::SD_A1(std::string username, std::string realm, std::string passwd, std::string nonce, std::string cnonce, std::string authzid)
{	
	std::string ret;
	std::string md1 = username + ":" + realm + ":" + passwd;
	std::string md2;
	if ( authzid.empty() )
		md2 = ":" + nonce + ":" + cnonce;
	else
		md2 = ":" + nonce + ":" + cnonce + ":" + authzid;
	
	ret.append(H(md1));
	ret.append(md2);
	
	return ret;
}

std::string
IQauth::SD_A2(std::string digest_uri_value)
{
	return std::string("AUTHENTICATE:") + digest_uri_value;
}

std::string
IQauth::H(std::string data)
{
	std::string ret;
 char buf[160];
	MD5( (unsigned char*) data.c_str(), data.size(), (unsigned char*)buf );
	ret.append(buf, 16);
	
	return ret;
}

std::string
IQauth::HEX(std::string data)
{
	std::string myhash;
	
	for( int i = 0 ; i < data.size() ; i++)
	{
		char buf2[3];
		if((unsigned char)data[i] < 16)
			sprintf(buf2, "0%x", (unsigned char)data[i]);
		else
			sprintf(buf2, "%x", (unsigned char)data[i]);
		myhash += buf2;
	}
	return myhash;
}

int
IQauth::SD_Challange(WokXMLTag *tag)
{
	if ( tag->GetAttr("session") != session )
		return 1;
	
	
	char buffer[500];
	tag->GetFirstTag("challenge").GetBodyAsBase64(buffer,500);
	std::string chal(buffer);
	std::map<std::string, std::string> data;
	
	for ( int i = 0 ; i < chal.size() ; )
	{
		std::string value = chal.substr(chal.find("=", i)+1, chal.find(",",i)-chal.find("=",i)-1);
		if( value[0] == '"' )
			value = value.substr(1,value.size()-2);
		data[chal.substr(i,chal.find("=",i)-i)] = value;
		if ( chal.find(",", i) == std::string::npos )
			break;
			
		i = chal.find(",",i)+1;
	}
	
	std::map<std::string, std::string>::iterator iter;
	
	if ( !data["rspauth"].empty() )
	{
		WokXMLTag message(NULL, "message");
		message.AddAttr("session", session);
		WokXMLTag &resptag = message.AddTag("response");
		resptag.AddAttr("xmlns", "urn:ietf:params:xml:ns:xmpp-sasl");
		
		wls->SendSignal("Jabber XML Send", message);
				
		return 1;
	}
	
	const char* base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	std::string cnonce;
	for( int i = 43 ; i ; i-- )
	{
		cnonce += base64char[rand()%strlen(base64char)];
	}
	cnonce += "=";
		
	if ( data["realm"] == "" )
		data["realm"] = "jabber";

	std::cout << "username: " << username << " password: " << password << std::endl;
	std::cout << data["realm"] << ", " << data["nonce"] << ", " << cnonce << ", " << data["digest-uri"] << std::endl;
	std::string resp = HEX(H( HEX(H(SD_A1(username, data["realm"], password, data["nonce"], cnonce, ""))) + ":" + data["nonce"] + ":00000001:" +
                   cnonce + ":auth:" + HEX(H(SD_A2("xmpp/"+ server +"")))));
																			
	std::string response="charset=utf-8,username=\"" + username + "\",realm=\""+ data["realm"] + "\",nonce=\"" + data["nonce"] + "\",nc=00000001,cnonce=\"" + cnonce + "\",digest-uri=\"xmpp/"+ server +"\",response=" +
								resp + ",qop=auth";
								
	WokXMLTag message(NULL, "message");
	message.AddAttr("session", session);
	WokXMLTag &resptag = message.AddTag("response");
	resptag.AddData(response);
	resptag.AddAttr("xmlns", "urn:ietf:params:xml:ns:xmpp-sasl");
	
	wls->SendSignal("Jabber XML Send", message);
	
	return 1;
}

int
IQauth::SD_Failure(WokXMLTag *tag)
{
	if ( tag->GetAttr("session") != session )
		return 1;
	
	WokXMLTag message(NULL, "message");
	message.AddAttr("session", session);
	
	wls->SendSignal("Jabber Connection Disconnect", message);
	woklib_error(wls, "Could not authenticate with the server: " + server + " using the username " + username);
	
	delete this;
	return 1;
}

int
IQauth::SD_Success(WokXMLTag *tag)
{
	if ( tag->GetAttr("session") != session )
		return 1;
	
	WokXMLTag message(NULL, "message");
	message.AddAttr("session", session);
	
	wls->SendSignal("Jabber Connection Reset " + session, message);

	delete this;
	return 1;
}

void
IQauth::InitSASLDIGESTMD5()
{
	WokXMLTag message(NULL, "message");
	message.AddAttr("session", session);
	WokXMLTag &authtag = message.AddTag("auth");
	authtag.AddAttr("xmlns", "urn:ietf:params:xml:ns:xmpp-sasl");
	authtag.AddAttr("mechanism", "DIGEST-MD5");
	
	wls->SendSignal("Jabber XML Send", &message);
}

void
IQauth::InitSASLPLAIN()
{
	WokXMLTag message(NULL, "message");
	message.AddAttr("session", session);
	WokXMLTag &authtag = message.AddTag("auth");
	authtag.AddAttr("xmlns", "urn:ietf:params:xml:ns:xmpp-sasl");
	authtag.AddAttr("mechanism", "PLAIN");
	
	std::string authstr = username + "@" + server + std::string("",1) + username;
	authstr += std::string("",1);
	authstr += password;
	
	authtag.AddData(authstr);
	
	wls->SendSignal("Jabber XML Send", &message);

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
		woklib_error(wls, "Authentication failed server: " + server + " using the username " + username);
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
