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

#include <openssl/sha.h>
#include "session.hpp"
#include <sstream>

Session::Session(WLSignal *wls, WokXMLTag *xml) : WLSignalInstance(wls),
origxml(*xml),
id(xml->GetAttr("id"))
{
	deleted = false;
	sent_plain = false;
	char data[] = "0123456789abcdef";
	std::string h;
	for( int x = 0 ; x < 16 ; x++ )
		myplain += data[rand()/(RAND_MAX/16)];
	
	unsigned char buffer[30];
	SHA1((unsigned char *)myplain.c_str(), myplain.size(), buffer);
	
	std::string myhash;
	for( int i = 0 ; i < 20 ; i++)
	{
		char buf2[3];
		if(buffer[i] < 16)
			sprintf(buf2, "0%x", buffer[i]);
		else
			sprintf(buf2, "%x", buffer[i]);
		myhash += buf2;
	}
	/*
	<message session="jabber0">
	  <message to="nedo@jabber.se" from="rand@conference.jabber.se/basil" type="groupchat">
		<x xmlns="RandomNumber">
		  <rand owner="rand@conference.jabber.se/basi" id="13" type="genereate request">
			<hash>ce47d07243bb6eaf5e1322c81baf9bbf</hash>
		  </rand>
		</x>
	  </message>
	</message>	
*/	
	WokXMLTag whoami("whoami");
	wls->SendSignal("Jabber GroupChat Whoami '" + XMLisize(origxml.GetAttr("session")) + "' '" + XMLisize(origxml.GetAttr("roomjid")) + "'", whoami);
	mynick = whoami.GetAttr("nick");
	

	
	WokXMLTag mtag("message");
	mtag.AddAttr("session", origxml.GetAttr("session"));
	WokXMLTag &message = mtag.AddTag("message");
	message.AddAttr("to", origxml.GetAttr("roomjid"));
	message.AddAttr("type", "groupchat");
	WokXMLTag &x = message.AddTag("x");
	x.AddAttr("xmlns", "RandomNumber");
	
	if ( origxml.GetAttr("owner").empty() )
	{
		owner = origxml.GetAttr("roomjid") + "/" + mynick;
		xml->AddAttr("owner", owner);
		
		x.AddAttr("type", "generate request");
	}
	else
	{
		owner = xml->GetAttr("owner");
		x.AddAttr("type", "participant seed");
	}

	x.AddAttr("owner", owner);  
	x.AddAttr("id", id); 
	WokXMLTag &hash = x.AddTag("hash");

	hash.AddText(myhash);
	
	wls->SendSignal("Jabber XML Send", mtag);
	
	EXP_SIGHOOK("Jabber RandomNumber Session '" + XMLisize(origxml.GetAttr("session")) + "' '" + XMLisize(x.GetAttr("owner")) + "' '" + id + "'", &Session::Message, 1000);
	
	
	WokXMLTag timer("time");
	timer.AddAttr("time", "120000");
	wls->SendSignal("Woklib Timmer Add", timer);
	
	EXP_SIGHOOK(timer.GetAttr("signal"), &Session::Timeout, 1000);
}

Session::~Session()
{
	if ( !deleted && owner == origxml.GetAttr("roomjid") + "/" + mynick )
	{
		WokXMLTag mtag("message");
		mtag.AddAttr("session", origxml.GetAttr("session"));
		WokXMLTag &message = mtag.AddTag("message");
		message.AddAttr("to", origxml.GetAttr("roomjid"));
		message.AddAttr("type", "groupchat");
		WokXMLTag &x = message.AddTag("x");
		x.AddAttr("xmlns", "RandomNumber");
		x.AddAttr("type", "cancel");
		x.AddAttr("id", id);
		x.AddAttr("owner", owner);
		
		wls->SendSignal("Jabber XML Send", mtag);
		
		WokXMLTag number ("number");
		number.AddAttr("id", id);
		number.AddAttr("owner", owner);
		number.AddAttr("session", origxml.GetAttr("session"));
		wls->SendSignal("Jabber RandomNumber Cancel '" + XMLisize(origxml.GetAttr("session")) + "' '" + XMLisize(owner) + "' '" + XMLisize(id) + "'", number);
		wls->SendSignal("Jabber RandomNumber Cancel", number);
	}
	
	
}

bool
Session::Is(const std::string &in_owner, const std::string &in_id, const std::string &session, const std::string &roomjid)
{
	return ( in_owner == owner && in_id == id && session == origxml.GetAttr("session") && roomjid == origxml.GetAttr("roomjid"));
}

void
Session::Cancel()
{
	if ( deleted )
		return;
	
	WokXMLTag number ("number");
	number.AddAttr("id", id);
	number.AddAttr("owner", owner);
	number.AddAttr("session", origxml.GetAttr("session"));
	wls->SendSignal("Jabber RandomNumber Cancel '" + XMLisize(origxml.GetAttr("session")) + "' '" + XMLisize(owner) + "' '" + XMLisize(id) + "'", number);
	wls->SendSignal("Jabber RandomNumber Cancel", number);
	
	WokXMLTag i_am_tag ("session");
	i_am_tag.AddAttr("owner", owner);
	i_am_tag.AddAttr("id", id);
	i_am_tag.AddAttr("roomjid", origxml.GetAttr("roomjid"));
	i_am_tag.AddAttr("session", origxml.GetAttr("session"));

	deleted = true;
	
	wls->SendSignal("Jabber RandomNumber RemoveSession", i_am_tag);
}

int
Session::Timeout(WokXMLTag *tag)
{
	if ( owner == origxml.GetAttr("roomjid") + "/" + mynick )
	{
		WokXMLTag mtag("message");
		mtag.AddAttr("session", origxml.GetAttr("session"));
		WokXMLTag &message = mtag.AddTag("message");
		message.AddAttr("to", origxml.GetAttr("roomjid"));
		message.AddAttr("type", "groupchat");
		WokXMLTag &x = message.AddTag("x");
		x.AddAttr("xmlns", "RandomNumber");
		x.AddAttr("type", "cancel");
		x.AddAttr("id", id);
		x.AddAttr("owner", owner);
		
		wls->SendSignal("Jabber XML Send", mtag);
	}
	
	
	Cancel();

	return 1;
}

void
Session::GetData(WokXMLTag &tag)
{
	tag.AddAttr("owner", owner);
	tag.AddAttr("id", id);
	tag.AddAttr("session", origxml.GetAttr("session"));
	tag.AddAttr("roomjid", origxml.GetAttr("roomjid"));	
	
	
	{
		std::stringstream str;
		str << requiredusers.size();
		tag.AddAttr("requiredusers", str.str());
	}
	{
		std::stringstream str;
		str << hashes.size();
		tag.AddAttr("hashes", str.str());
	}
	{
		std::stringstream str;
		str << plain.size();
		tag.AddAttr("plains", str.str());
	}
	
	
}


int
Session::Message(WokXMLTag *tag)
{	
	bool progress = false;
	
	std::string type = tag->GetFirstTag("message").GetFirstTag("x", "RandomNumber").GetAttr("type");
	if ( type == "cancel" )
	{
		if( owner  == tag->GetFirstTag("message").GetAttr("from") )
		{
			Cancel();
			return 1;
		}
		else
			woklib_message(wls, tag->GetFirstTag("message").GetAttr("from") + " is messing around trying to cancel sessions his not the owner of");
	}
	else if ( type == "generate request" )
	{
		WokXMLTag occupants("occupants");
		wls->SendSignal("Jabber GroupChat GetOccupants '" + origxml.GetAttr("session") + "' '" + origxml.GetAttr("roomjid") +"'", occupants);
		
		std::list <WokXMLTag *>::iterator useriter;
		
		for ( useriter = occupants.GetTagList("item").begin() ; useriter != occupants.GetTagList("item").end() ; useriter++) 
		{
			std::string role = (*useriter)->GetFirstTag("message").GetFirstTag("presence").GetFirstTag("x","http://jabber.org/protocol/muc#user").GetFirstTag("item").GetAttr("role");
			if ( role == "participant" || role == "moderator" )
			{
				requiredusers.push_back((*useriter)->GetAttr("nick"));
			}
		}
		hashes[tag->GetFirstTag("message").GetAttr("from")] = tag->GetFirstTag("message").GetFirstTag("x", "RandomNumber").GetFirstTag("hash").GetBody();
		progress = true;
	}
	else if ( type == "participant seed" )
	{
		hashes[tag->GetFirstTag("message").GetAttr("from")] = tag->GetFirstTag("message").GetFirstTag("x", "RandomNumber").GetFirstTag("hash").GetBody();
		progress = true;
	}
	else if ( type == "participant plain" )
	{
		std::string this_plain = tag->GetFirstTag("message").GetFirstTag("x", "RandomNumber").GetFirstTag("plain").GetBody();
		plain[tag->GetFirstTag("message").GetAttr("from")] = this_plain;
		progress = true;
		
		unsigned char buffer[30];
		SHA1((unsigned char *)this_plain.c_str(), this_plain.size(), buffer);
	
		std::string thehash;
		for( int i = 0 ; i < 20 ; i++)
		{
			char buf2[3];
			if(buffer[i] < 16)
				sprintf(buf2, "0%x", buffer[i]);
			else
				sprintf(buf2, "%x", buffer[i]);
			thehash += buf2;
		}
		
		if ( thehash != hashes[tag->GetFirstTag("message").GetAttr("from")] )
		{
			woklib_message(wls, "Something fishy is going on with this random number... got " + hashes[tag->GetFirstTag("message").GetAttr("from")] + " from " + tag->GetFirstTag("message").GetAttr("from") + " and my computed is " + thehash);
			if ( owner == origxml.GetAttr("roomjid") + "/" + mynick )
			{
				WokXMLTag mtag("message");
				mtag.AddAttr("session", origxml.GetAttr("session"));
				WokXMLTag &message = mtag.AddTag("message");
				message.AddAttr("to", origxml.GetAttr("roomjid"));
				message.AddAttr("type", "groupchat");
				WokXMLTag &x = message.AddTag("x");
				x.AddAttr("xmlns", "RandomNumber");
				x.AddAttr("type", "cancel");
				x.AddAttr("id", id);
				x.AddAttr("owner", owner);

				wls->SendSignal("Jabber XML Send", mtag);
			}
			Cancel();
			return 1;
		}
	}
	if ( progress )
	{
		WokXMLTag item("item");
		GetData(item);
		wls->SendSignal("Jabber RandomNumber Progress '" + XMLisize(origxml.GetAttr("session")) + "' '" + XMLisize(owner) + "' '" + XMLisize(id) + "'", item);
		wls->SendSignal("Jabber RandomNumber Progress", item);
	}
	if ( hashes.size() == requiredusers.size() && !sent_plain )
	{
		WokXMLTag mtag("message");
		mtag.AddAttr("session", origxml.GetAttr("session"));
		WokXMLTag &message = mtag.AddTag("message");
		message.AddAttr("to", origxml.GetAttr("roomjid"));
		message.AddAttr("type", "groupchat");
		WokXMLTag &x = message.AddTag("x");
		x.AddAttr("xmlns", "RandomNumber");
		x.AddAttr("type", "participant plain");
		x.AddAttr("id", id); 
		x.AddAttr("owner", owner);  
		WokXMLTag &hash = x.AddTag("plain");

		hash.AddText(myplain);
		
		wls->SendSignal("Jabber XML Send", mtag);
		sent_plain = true;
	}
	if ( plain.size() == requiredusers.size() )
	{
		unsigned long long int r = 0;
		
		std::map <std::string, std::string>::iterator plainiter;
		for ( plainiter = plain.begin() ; plainiter != plain.end() ; plainiter++)
		{
			unsigned long long int x = strtoull(("0x" + plainiter->second).c_str(), NULL, 16);
			r ^= x;
			
		}
		
		std::stringstream str;
		str << std::hex << r;
		WokXMLTag number ("number");
		number.AddAttr("num", str.str());
		number.AddAttr("id", id);
		number.AddAttr("owner", owner);
		number.AddAttr("session", origxml.GetAttr("session"));
		wls->SendSignal("Jabber RandomNumber Generated '" + XMLisize(origxml.GetAttr("session")) + "' '" + XMLisize(owner) + "' '" + XMLisize(id) + "'", number);
		wls->SendSignal("Jabber RandomNumber Generated", number);
		
		WokXMLTag i_am_tag ("session");
		i_am_tag.AddAttr("owner", owner);
		i_am_tag.AddAttr("id", id);
		i_am_tag.AddAttr("roomjid", origxml.GetAttr("roomjid"));
		i_am_tag.AddAttr("session", origxml.GetAttr("session"));
		
		
		deleted = true;
		
		wls->SendSignal("Jabber RandomNumber RemoveSession", i_am_tag);
	}
	return 1;
}

