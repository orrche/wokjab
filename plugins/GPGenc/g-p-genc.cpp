/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2007-2008 <nedo80@gmail.com>
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



std::string rungpg ( const std::string &data, const std::string &arg, const std::string &password, std::string *status = NULL)
{
	int fd[4][2];
	
	pipe ( fd[0] );
	pipe ( fd[1] );
	pipe ( fd[2] );
	pipe ( fd[3] );
	
	int pid = fork();
	if ( pid == 0 )
	{
		std::stringstream pfd;
		pfd << fd[2][0];
		
		std::stringstream sfd;
		sfd << fd[3][1];
		
		std::string cmd = "gpg -q --armor --batch --always-trust --passphrase-fd " + pfd.str() + " --status-fd " + sfd.str() + " " + arg;
		
		close ( fd[0][0] );
		close ( fd[1][1] );
		close ( fd[2][1] );
		close ( fd[3][0] );
		
		dup2( fd[0][1], fileno(stdout) );
		dup2( fd[1][0], fileno(stdin) );
		
		system(cmd.c_str());
		
		close ( fd[0][1] );
		close ( fd[1][0] );
		close ( fd[2][0] );
		close ( fd[3][1] );
		
		exit(1);
	}
	if ( pid < 0 )
	{
		std::cout << "Fork problem" << std::endl;
		return "";
	}

	close ( fd[0][1] );
	close ( fd[1][0] );
	close ( fd[2][0] );
	close ( fd[3][1] );	
	
	write(fd[2][1], (password + "\n").c_str(), password.size()+1);
	close(fd[2][1]);
	if ( data.size() != write(fd[1][1], data.c_str(), data.size())) 
		std::cout << "ohh ..." << std::endl;
	close(fd[1][1]);
	
	int pid_status;
	waitpid(pid, &pid_status, 0);

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
	
	if ( status )
	{
		while ( ( len = read(fd[3][0], buf, BUFLEN) ) != 0 )
		{
		
			if ( len > 0 )
			{
				buf[len] = 0;
				*status += buf;
			}
			else 
				break;
		}
	}
	
	close ( fd[3][0] );
	return ret;
}


GPGenc::GPGenc(WLSignal *wls) : WoklibPlugin(wls)
{
	cought = false;
	
	EXP_SIGHOOK("Program Start", &GPGenc::ProgramStart, 1500);
	EXP_SIGHOOK("Jabber GUI GetJIDMenu", &GPGenc::Menu, 1500);
	EXP_SIGHOOK("GPGenc AssignKey", &GPGenc::AssignKey, 1000);
	EXP_SIGHOOK("GPGenc AssignKey Data", &GPGenc::AssignKeyData, 1000);
	EXP_SIGHOOK("Jabber AutoConnect", &GPGenc::AutoConnectInhibiter, 100);
	EXP_SIGHOOK("Jabber XML Presence", &GPGenc::InPresence, 1);
	
	config = new WokXMLTag ("config");
	EXP_SIGHOOK("Config XML Change /GPGenc", &GPGenc::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/GPGenc");
	wls->SendSignal("Config XML Trigger", &conftag);
}

GPGenc::~GPGenc()
{
	SaveConfig();
	delete config;	
}

int
GPGenc::AutoConnectInhibiter(WokXMLTag *tag)
{
	cought = true;	
	return 0;
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
					
	if ( !jid.empty() )
	{
		WokXMLTag conftag(NULL, "config");
		conftag.AddAttr("path", "/gpgkey");
		conftag.AddAttr("name", jid);
		wls->SendSignal("Jabber JIDConfig Get", &conftag);
		conftag.GetFirstTag("config").GetFirstTag("key").AddAttr("data",key);
		wls->SendSignal("Jabber JIDConfig Store", &conftag);
		
		
		WokXMLTag testtag("config");
		testtag.AddAttr("path", "/gpgkey");
		testtag.AddAttr("name", jid);
		wls->SendSignal("Jabber JIDConfig Get", testtag);
	}		
	
	return 1;
}

int
GPGenc::ReadConfig(WokXMLTag *tag)
{		
	if ( tag->GetFirstTag("config").GetTagList("send_warning_message_mismatch").empty() )
	{
		tag->GetFirstTag("config").GetFirstTag("send_warning_message_mismatch").AddAttr("type", "bool");
		tag->GetFirstTag("config").GetFirstTag("send_warning_message_mismatch").AddAttr("label", _("Warn on key mismatch"));
		tag->GetFirstTag("config").GetFirstTag("send_warning_message_mismatch").AddAttr("tooltip", _("Warn if your encrypting to diffrent key\nthen annaunced in users presence"));
		
		tag->GetFirstTag("config").GetFirstTag("send_message_message_mismatch").AddAttr("type", "bool");
		tag->GetFirstTag("config").GetFirstTag("send_message_message_mismatch").AddAttr("label", _("Send unencrypted on key mismatch"));
		tag->GetFirstTag("config").GetFirstTag("send_message_message_mismatch").AddAttr("tooltip", _("Send unencrypted on key mismatch\nYou are really suposed to KNOW WHAT YOUR DOING if you disable this"));
	}
	
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
	std::string jid = tag->GetAttr("jid");
	
	if ( jid.find("/"))
		jid = jid.substr(0, jid.find("/"));
	
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/gpgkey");
	conftag.AddAttr("name", jid);
	wls->SendSignal("Jabber JIDConfig Get", &conftag);	
	
	std::string key = conftag.GetFirstTag("config").GetFirstTag("key").GetAttr("data");
	
	WokXMLTag form("form");
	WokXMLTag &x = form.AddTag("x");
	x.AddAttr("xmlns", "jabber:x:data");
	x.AddAttr("type", "submit");
	
	WokXMLTag &jidlabel = x.AddTag("field");
	jidlabel.AddAttr("type", "fixed");
	jidlabel.AddTag("value").AddText(_("JID:") + jid);
		
	WokXMLTag &f_jid = x.AddTag("field");
	f_jid.AddAttr("type", "hidden");
	f_jid.AddAttr("var", "jid");
	f_jid.AddTag("value").AddText(jid);
	
	
	WokXMLTag &keyfield = x.AddTag("field");
	keyfield.AddAttr("type", "list-single");
	keyfield.AddAttr("var", "key");
	keyfield.AddTag("value").AddText(key);
	
	WokXMLTag holder("holder");
	
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
		
		bool found_default = false;
		
		std::string key = fp;
		while( key.find(" ") != std::string::npos )
			key.erase(key.find(" "), 1);
		
		std::map<std::string, std::map < std::string, std::string > >::iterator sessiter;
		for ( sessiter = fingerprints.begin() ; sessiter != fingerprints.end() ; sessiter++)
		{
			if ( sessiter->second.find(jid) != sessiter->second.end() )
			{
				if ( key == sessiter->second[jid] )
				{
					found_default = true;
					WokXMLTag &option = keyfield.AddTag("option");
					option.AddTag("value").AddText(fp);
					option.AddAttr("label", name + "\n" + fp);
				}
			}
		}
		
		if ( !found_default )
		{
			WokXMLTag &option = holder.AddTag("option");
			option.AddTag("value").AddText(fp);
			option.AddAttr("label", name + "\n" + fp);
		}
	}
	
	if ( !keyfield.GetTagList("option").empty() )
	{
		WokXMLTag &option = keyfield.AddTag("option");
		option.AddTag("value").AddText("---------------------------");
	}
	WokXMLTag &option = keyfield.AddTag("option");
	option.AddTag("value").AddText("");
	option.AddAttr("label", "No key");
	
	if ( !holder.GetTagList("option").empty())
	{
		WokXMLTag &option = keyfield.AddTag("option");
		option.AddTag("value").AddText("---------------------------");
	
		std::list <WokXMLTag *>::iterator tagiter;
		
		for ( tagiter = holder.GetTagList("option").begin() ; tagiter != holder.GetTagList("option").end() ; tagiter++ )
			keyfield.AddTag(*tagiter);			
	}
		
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
	if ( tag->GetFirstTag("x", "jabber:x:data").GetAttr("type") == "cancel") 
	{
		return 1;		
	}
	
	std::list <WokXMLTag *>::iterator xiter;
	
	for( xiter = tag->GetFirstTag("x", "jabber:x:data").GetTagList("field").begin() ; xiter != tag->GetFirstTag("x", "jabber:x:data").GetTagList("field").end() ; xiter++)
	{
		if ( (*xiter)->GetAttr("var") == "passphrase")
		{
			passphrase = (*xiter)->GetFirstTag("value").GetBody();
		}
		if ( (*xiter)->GetAttr("var") == "key" )
		{
			
		}
	}
	
	std::string status;
	rungpg("", " -bs ", passphrase, &status);
		
	if ( status.find("\n[GNUPG:] GOOD_PASSPHRASE") != std::string::npos )
	{	
		EXP_SIGHOOK("Jabber XML Presence Send", &GPGenc::Presence, 960);
		EXP_SIGHOOK("Jabber XML Message Send", &GPGenc::OutMessage, 960);
		EXP_SIGHOOK("Jabber XML Object message", &GPGenc::Message, 1);
		
		EXP_SIGUNHOOK("Jabber AutoConnect", &GPGenc::AutoConnectInhibiter, 100);
		
		WokXMLTag sesstag("sessions");
		wls->SendSignal("Jabber GetSessions", sesstag);
		
		std::list <WokXMLTag *>::iterator sessiter;
		for( sessiter = sesstag.GetTagList("item").begin() ; sessiter != sesstag.GetTagList("item").end() ; sessiter++)
		{
			WokXMLTag msgp("message");
			msgp.AddAttr("session", (*sessiter)->GetAttr("name"));
			msgp.AddTag("presence");
			
			wls->SendSignal("Jabber XML Presence Send", msgp);
		}
				
		if ( cought )
		{
			WokXMLTag dummy("xml");
			wls->SendSignal("Jabber AutoConnect", dummy);
		}
	}
	else
		woklib_error(wls, "Bad passphrase");
	
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
	conftag.AddAttr("path", "/gpgkey");
	conftag.AddAttr("name", jid);
	wls->SendSignal("Jabber JIDConfig Get", &conftag);			
	
	
	if ( !conftag.GetFirstTag("config").GetTagList("key").empty() )
	{
		std::string key = conftag.GetFirstTag("config").GetFirstTag("key").GetAttr("data");
		while( key.find(" ") != std::string::npos )
			key.erase(key.find(" "), 1);
		
		if ( key.empty() )
			return 1;

		if ( fingerprints[tag->GetAttr("session")][jid] != key )
		{
			if ( config->GetFirstTag("send_warning_message_mismatch").GetAttr("data") != "false" )
				woklib_message(wls, _("Key mismatch while sending message to ") + jid);
			if ( config->GetFirstTag("send_message_message_mismatch").GetAttr("data") != "false" )
			{
				woklib_message(wls, _("Sending unencrypted messages to ") + jid);
				return 1;
			}
			
		}
	
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
	
	std::string jid = tag->GetFirstTag("presence").GetAttr("from");
	if ( jid.find("/") != std::string::npos )
	{
		jid = jid.substr(0, jid.find("/"));	
	}

	bool remove_fp = false;
	for( xiter = tag->GetFirstTag("presence").GetTagList("x").begin() ; xiter != tag->GetFirstTag("presence").GetTagList("x").end() ; xiter++)
	{
		
		if ( (*xiter)->GetAttr("xmlns") == "jabber:x:signed")
		{
			std::string sig_text = tag->GetFirstTag("presence").GetFirstTag("status").GetChildrenStr();
			std::string sig_data = "\n-----BEGIN PGP SIGNATURE-----\nVersion: GnuPG v2.0.7 (GNU/Linux)\n\n" + (*xiter)->GetBody()+ "\n-----END PGP SIGNATURE-----\n";
			
			char buf[20];
			strcpy(buf, "/tmp/woksig.XXXXXX");
			int fd = mkstemp(buf);
			if ( fd > 0 )
			{
				write(fd, sig_text.c_str(), sig_text.size());
				lseek(fd,SEEK_SET, 0 );
				
				std::stringstream test;
				test << fd;
				
				std::string status;
				std::string data = rungpg(sig_data, std::string(" --verify --enable-special-filenames - -\\&") + test.str(), passphrase,  &status);
				
				std::string fp = "";
				bool goodsig = false;;
				if ( status.find("\n[GNUPG:] GOODSIG") != std::string::npos )
				{
					std::size_t pos;
					if ( ( pos = status.find("\n[GNUPG:] VALIDSIG ")) != std::string::npos )
					{
						pos += 19;
						if ( status.size() - pos - 40 > 0 )	
							fp = status.substr(pos, 40);
						
						goodsig = true;
					}
				}
				close(fd);
				unlink(buf);
				
				
				if ( !goodsig )
				{
					woklib_debug(wls, _("GPG bad signature for ") + tag->GetFirstTag("presence").GetAttr("from"));
					remove_fp = false;
				}
				else
					fingerprints[tag->GetAttr("session")][jid] = fp;
				
				WokXMLTag conftag(NULL, "config");
				conftag.AddAttr("path", "/gpgkey");
				conftag.AddAttr("name", jid);
				wls->SendSignal("Jabber JIDConfig Get", &conftag);			
				
				std::string key;
				if ( !conftag.GetFirstTag("config").GetTagList("key").empty() )
				{
					key = conftag.GetFirstTag("config").GetFirstTag("key").GetAttr("data");
					
					while( key.find(" ") != std::string::npos )
						key.erase(key.find(" "), 1);
				}
				
				if ( key != fp )
					woklib_debug(wls, _("GPG key mismatch for ") + tag->GetFirstTag("presence").GetAttr("from"));
				
			}
		}
	}
	if ( tag->GetFirstTag("presence").GetAttr("type") == "unavailable" )
		remove_fp = true;

	if ( remove_fp )
	{
		fingerprints[tag->GetAttr("session")].erase(jid);
		if ( fingerprints[tag->GetAttr("session")].empty())
		{
			fingerprints.erase(tag->GetAttr("session"));
		}
	}
	
	return 1;	
}

int
GPGenc::Presence(WokXMLTag *tag)
{
	std::string childstr = tag->GetFirstTag("presence").GetFirstTag("status").GetChildrenStr();
	std::string sign = rungpg(childstr, " -bs ", passphrase );
		
	if ( sign.find("\n\n") == std::string::npos )
		return 1;
	sign = sign.substr(sign.find("\n\n")+2);
	
	if ( sign.find("\n-----END PGP SIGNATURE-----\n") == std::string::npos )
		return 1;
	sign = sign.substr(0, sign.rfind("\n-----END PGP SIGNATURE-----\n"));
	
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
		
		std::string status;
		std::string decrypt = rungpg(body, "-d", passphrase, &status);
		
		if ( status.find("\n[GNUPG:] DECRYPTION_OKAY") != std::string::npos )
		{
			tag->GetFirstTag("message").GetFirstTag("body").RemoveBody();
			tag->GetFirstTag("message").GetFirstTag("body").AddText(decrypt);
		}
	}
	return 1;
}
