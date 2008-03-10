/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2007 <nedo80@gmail.com>
 * 
 * wokjab is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * wokjab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with wokjab.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#include "g-p-genc.hpp"
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#include "../../gettext.h"
#define _(string) gettext (string)



std::string rungpg ( const std::string &data, const std::string &arg, const std::string &password)
{
	int fd[3][2];
	
	pipe ( fd[0] );
	pipe ( fd[1] );
	pipe ( fd[2] );
	
	int pid = fork();
	if ( pid == 0 )
	{
		std::stringstream str;
		str << fd[2][0];
		
		std::string cmd = "gpg -q --armor --batch --always-trust --passphrase-fd " + str.str() + " " + arg;
		
		close ( fd[1][1] );
		close ( fd[0][0] );
		close ( fd[2][1] );
		
		dup2( fd[0][1], fileno(stdout) );
		dup2( fd[1][0], fileno(stdin) );
		
		system(cmd.c_str());
		
		close ( fd[1][0] );
		close ( fd[0][1] );
		close ( fd[2][0] );
		
		exit(1);
	}
	if ( pid < 0 )
	{
		std::cout << "Fork problem" << std::endl;
		return "";
	}

	close ( fd[1][0] );
	close ( fd[0][1] );
	close ( fd[2][0] );
		
	
	int status;
	write(fd[2][1], (password + "\n").c_str(), password.size()+1);
	close(fd[2][1]);
	write(fd[1][1], data.c_str(), data.size());
	close(fd[1][1]);
	
	waitpid(pid, &status, 0);

#define BUFLEN 200
	int len;
	char buf[BUFLEN+1];
	std::string ret;
	
	while ( ( len = read(fd[0][0], buf, BUFLEN) ) != 0 )
	{
	
		if ( len > 0 )
		{
			buf[len] = 0;
			ret += buf;
		}
		else 
			break;
	}

	close ( fd[0][0] );
	return ret;
}


GPGenc::GPGenc(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Program Start", &GPGenc::ProgramStart, 1500);
	EXP_SIGHOOK("Jabber GUI GetJIDMenu", &GPGenc::Menu, 1500);
	EXP_SIGHOOK("GPGenc AssignKey", &GPGenc::AssignKey, 1000);
	EXP_SIGHOOK("GPGenc AssignKey Data", &GPGenc::AssignKeyData, 1000);
	
	config = new WokXMLTag ("config");
	EXP_SIGHOOK("Config XML Change /GPGenc", &GPGenc::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/GPGenc");
	wls->SendSignal("Config XML Trigger", &conftag);
}

GPGenc::~GPGenc()
{
	delete config;	
	SaveConfig();
	
}

int
GPGenc::ProgramStart(WokXMLTag *tag)
{
	WokXMLTag form("form");
	WokXMLTag &x = form.AddTag("x");
	x.AddAttr("xmlns", "jabber:x:data");
	x.AddAttr("type", "submit");
	
	WokXMLTag &ppfield = x.AddTag("field");
	ppfield.AddAttr("type", "text-private");
	ppfield.AddAttr("var", "passphrase");
	
	WokXMLTag &keyfield = x.AddTag("field");
	keyfield.AddAttr("type", "list-single");
	keyfield.AddAttr("var", "key");
	
	WokXMLTag &option = keyfield.AddTag("option");
	option.AddTag("value").AddText("default");
	
	wls->SendSignal("Jabber jabber:x:data Init", form);
	EXP_SIGHOOK(form.GetAttr("signal"), &GPGenc::Setup, 500);
	return 1;	
}

int
GPGenc::StoreKey(WokXMLTag *tag)
{
	WokXMLTag &key = tag->GetFirstTag("config").GetFirstTag("key");
	tag->GetFirstTag("config").AddAttr("path", "/jid/" + jid_store + "/gpgkey");
	key.AddAttr("type", "string");
	key.AddAttr("data", key_store);
	
	return 1;
}

int
GPGenc::AssignKeyData(WokXMLTag *tag)
{
	std::string jid,key;
	
	std::list <WokXMLTag *>::iterator iter;
	
	for(  iter = tag->GetFirstTag("x", "jabber:x:data").GetTagList("field").begin() ;
		  iter != tag->GetFirstTag("x", "jabber:x:data").GetTagList("field").end() ;
		  iter++)
	{
		if ( (*iter)->GetAttr("var") == "jid")
			jid = (*iter)->GetFirstTag("value").GetBody();
		if ( (*iter)->GetAttr("var") == "key")
			key = (*iter)->GetFirstTag("value").GetBody();
	}
	

				
				
	if ( (!key.empty()) && (!jid.empty()) )
	{
		jid_store = jid;
		key_store = key;
		
		EXP_SIGHOOK("Config XML Change /jid/" + jid + "/gpgkey", &GPGenc::StoreKey, 500);
		WokXMLTag conftag(NULL, "config");
		conftag.AddAttr("path", "/jid/" + jid + "/gpgkey");
		wls->SendSignal("Config XML Trigger", &conftag);			
		EXP_SIGUNHOOK("Config XML Change /jid/" + jid + "/gpgkey", &GPGenc::StoreKey, 500);
	}			
	/*
	<data>
		<x type='submit' xmlns='jabber:x:data'>
			<field var='jid'><value>schmidtm524@googlemail.com</value></field>
			<field var='key'><value>B6CD 75FC 2F40 D626 C6E8  165A F95E 36C5 6D7F 2E7C</value></field>
		</x>
	</data>
	*/
	return 1;
}

int
GPGenc::ReadConfig(WokXMLTag *tag)
{
	if ( config )
		delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));
	
	
	return 1;
}

void
GPGenc::SaveConfig()
{	
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/GPGenc");
	conftag.AddTag(config);

	EXP_SIGUNHOOK("Config XML Change /GPGenc", &GPGenc::ReadConfig, 500);
	wls->SendSignal("Config XML Store", &conftag);
	EXP_SIGHOOK("Config XML Change /GPGenc", &GPGenc::ReadConfig, 500);
	
}

int
GPGenc::AssignKey(WokXMLTag *tag)
{
	WokXMLTag form("form");
	WokXMLTag &x = form.AddTag("x");
	x.AddAttr("xmlns", "jabber:x:data");
	x.AddAttr("type", "submit");
	
	WokXMLTag &jidlabel = x.AddTag("field");
	jidlabel.AddAttr("type", "fixed");
	jidlabel.AddTag("value").AddText(_("JID:") + tag->GetAttr("jid"));
	
	
	std::string noresource = tag->GetAttr("jid");
	if ( noresource.find("/") != std::string::npos )
		noresource.substr(0, noresource.find("/"));
	
	WokXMLTag &jid = x.AddTag("field");
	jid.AddAttr("type", "hidden");
	jid.AddAttr("var", "jid");
	jid.AddTag("value").AddText(tag->GetAttr("jid"));
	
	
	WokXMLTag &keyfield = x.AddTag("field");
	keyfield.AddAttr("type", "list-single");
	keyfield.AddAttr("var", "key");
	
	std::string data = rungpg("", "--fingerprint -k", "");
	for(;;)
	{
		std::string fp;
		std::string name;
		
		std::size_t pos;
		pos = data.find("pub");
		
		if ( pos == std::string::npos ) break;
		pos = data.find("\n", pos);
		if ( pos == std::string::npos ) break;
		pos = data.find(" = ", pos);
		if ( pos == std::string::npos ) break;
		pos += 3;
		
		std::size_t endline;
		endline = data.find("\n", pos);
		if ( endline == std::string::npos ) break;

		fp = data.substr(pos, endline-pos);
		pos = endline + 1;
//		if( data.substr(pos, pos+3) == "uid" )
		{
			name = data.substr(pos+3, data.find("\n", pos+3)-pos-3);
			unsigned int i = 0;
			
			while ( name[i] == ' ' && name.size() > i )
				i++;
			
			name = name.substr(i);
		}
			
		pos = data.find("\n\n", pos);
		if ( pos == std::string::npos ) break;
		
		data = data.substr(pos);
		
		WokXMLTag &option = keyfield.AddTag("option");
		option.AddTag("value").AddText(fp);
		option.AddAttr("label", name + "\n" + fp);
	}
	
	WokXMLTag &option = keyfield.AddTag("option");
	option.AddTag("value").AddText("default");
	
	
	form.AddAttr("signal", "GPGenc AssignKey Data");
	wls->SendSignal("Jabber jabber:x:data Init", form);
	
	
	return 1;
}

int
GPGenc::Menu(WokXMLTag *tag)
{
	WokXMLTag &item = tag->AddTag("item");
	item.AddAttr("name", "Assign GPG key");
	item.AddAttr("signal" , "GPGenc AssignKey");

	return 1;
}

int
GPGenc::Setup(WokXMLTag *tag)
{	
	std::list <WokXMLTag *>::iterator xiter;
	
	for( xiter = tag->GetFirstTag("x").GetTagList("field").begin() ; xiter != tag->GetFirstTag("x").GetTagList("field").end() ; xiter++)
	{
		if ( (*xiter)->GetAttr("var") == "passphrase")
		{
			passphrase = (*xiter)->GetFirstTag("value").GetBody();
		}
		if ( (*xiter)->GetAttr("var") == "key" )
		{
			
		}
	}
	
	EXP_SIGHOOK("Jabber XML Presence Send", &GPGenc::Presence, 950);
	EXP_SIGHOOK("Jabber XML Message Send", &GPGenc::OutMessage, 950);
	EXP_SIGHOOK("Jabber XML Object message", &GPGenc::Message, 1);
	EXP_SIGHOOK("Jabber XML Presence", &GPGenc::InPresence, 1);
	
	return 1;
}

int
GPGenc::OutMessage(WokXMLTag *tag)
{
	std::string body = tag->GetFirstTag("message").GetFirstTag("body").GetBody();
	std::string jid = tag->GetFirstTag("message").GetAttr("to");
	if ( jid.find("/") != std::string::npos )
		jid = jid.substr(0, jid.find("/"));
	
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/jid/" + jid + "/gpgkey");
	wls->SendSignal("Config XML GetConfig", &conftag);			
	
	
	if ( !conftag.GetFirstTag("config").GetTagList("key").empty() )
	{
		std::string key = conftag.GetFirstTag("config").GetFirstTag("key").GetAttr("data");
		
		if ( key.empty() ) 
			return 1;
		
		while( key.find(" ") != std::string::npos )
			key.erase(key.find(" "), 1);
		std::string encdata = rungpg(body, "-ae -r " + key, passphrase);
		
		
		if ( ! encdata.empty() )
		{
			if ( encdata.find("\n\n") == std::string::npos )
				return 1;
			encdata = encdata.substr(encdata.find("\n\n")+2);
	
			if ( encdata.find("\n-----END PGP MESSAGE-----\n") == std::string::npos )
				return 1;
			encdata = encdata.substr(0, encdata.rfind("\n-----END PGP MESSAGE-----\n"));
			
			WokXMLTag &xtag = tag->GetFirstTag("message").AddTag("x", "jabber:x:encrypted");
			xtag.AddText(encdata);
			tag->GetFirstTag("message").GetFirstTag("body").RemoveBody();
			tag->GetFirstTag("message").GetFirstTag("body").AddText("This message is encrypted");
		}
	}
	return 1;
}

int
GPGenc::InPresence(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator xiter;
	
	for( xiter = tag->GetFirstTag("presence").GetTagList("x").begin() ; xiter != tag->GetFirstTag("presence").GetTagList("x").end() ; xiter++)
	{
		if ( (*xiter)->GetAttr("xmlns") == "jabber:x:signed")
		{
#if 0
			gpgme_error_t err;
			gpgme_data_t sig, text;
			gpgme_verify_result_t result;

			std::string sig_data = "-----BEGIN PGP MESSAGE-----\nVersion: GnuPG v2.0.7 (GNU/Linux)\n\n" + (*xiter)->GetBody()+ "-----END PGP MESSAGE-----\n";
			std::string sig_text = tag->GetFirstTag("presence").GetFirstTag("status").GetChildrenStr();
			
			/* Checking a valid message.  */
			err = gpgme_data_new_from_mem (&text, sig_text.data(), sig_text.size(), 0);
			fail_if_err (err);
			err = gpgme_data_new_from_mem (&sig, sig_data.data(), sig_data.size(), 0);
			fail_if_err (err);
			err = gpgme_op_verify (ctx, sig, text, NULL);
			fail_if_err (err);
			result = gpgme_op_verify_result (ctx);
			
			fingerprints[tag->GetAttr("session")][tag->GetFirstTag("presence").GetAttr("from")] = result->signatures->fpr;
//			check_result (result, 0, "A0FF4590BB6122EDEF6E3C542D727CC768697734", GPG_ERR_NO_ERROR, 1);
#endif
		}
	}
	
	return 1;	
}

int
GPGenc::Presence(WokXMLTag *tag)
{
/*
#warning need to read up on how to check for errors here ...
	gpgme_error_t err;
	gpgme_data_t in, out;
	gpgme_sign_result_t result;
	
	
	std::string childstr = tag->GetFirstTag("presence").GetFirstTag("status").GetChildrenStr();
	
	err = gpgme_data_new_from_mem (&in, childstr.data(), childstr.size(), 0);
	fail_if_err(err);
	
	err = gpgme_data_new (&out);
	fail_if_err (err);
	err = gpgme_op_sign (ctx, in, out, GPGME_SIG_MODE_DETACH);
	fail_if_err (err);
	result = gpgme_op_sign_result (ctx);
	check_result (result, GPGME_SIG_MODE_DETACH);
	WokXMLTag &xtag = tag->GetFirstTag("presence").AddTag("x");
	xtag.AddAttr("xmlns", "jabber:x:signed");
	xtag.AddText(print_data(out));
	
	gpgme_data_release (out);
	
*/
	std::string childstr = tag->GetFirstTag("presence").GetFirstTag("status").GetChildrenStr();
	std::string sign = rungpg(childstr, " -s ", passphrase );
	
	if ( sign.find("\n\n") == std::string::npos )
		return 1;
	sign = sign.substr(sign.find("\n\n")+2);
	
	if ( sign.find("\n-----END PGP MESSAGE-----\n") == std::string::npos )
		return 1;
	sign = sign.substr(0, sign.rfind("\n-----END PGP MESSAGE-----\n"));
	
	if ( !sign.empty() )
		tag->GetFirstTag("presence").AddTag("x", "jabber:x:signed").AddText(sign);
	
	return 1;
}


int
GPGenc::Message(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator xiter;
	
	for( xiter = tag->GetFirstTag("message").GetTagList("x", "jabber:x:encrypted").begin() ; xiter != tag->GetFirstTag("message").GetTagList("x", "jabber:x:encrypted").end() ; xiter++)
	{
		std::string body = "-----BEGIN PGP MESSAGE-----\nVersion: GnuPG v2.0.7 (GNU/Linux)\n\n" + (*xiter)->GetBody()+ "\n-----END PGP MESSAGE-----\n";
		
		std::string decrypt = rungpg(body, "-d", passphrase);
		
		tag->GetFirstTag("message").GetFirstTag("body").RemoveBody();
		tag->GetFirstTag("message").GetFirstTag("body").AddText(decrypt);
	}
	return 1;
}
