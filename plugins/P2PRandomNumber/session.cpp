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

#include "session.hpp"
#include <sstream>

Session::Session(WLSignal *wls, WokXMLTag *xml, std::string id) : WLSignalInstance(wls),
origxml(*xml),
id(id)
{
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
	
	owner = origxml.GetAttr("roomjid") + "/" + mynick;
	
	WokXMLTag mtag("message");
	mtag.AddAttr("session", origxml.GetAttr("session"));
	WokXMLTag &message = mtag.AddTag("message");
	message.AddAttr("to", origxml.GetAttr("roomjid"));
	message.AddAttr("type", "groupchat");
	WokXMLTag &x = message.AddTag("x");
	x.AddAttr("xmlns", "RandomNumber");
	x.AddAttr("type", "generate request");
	x.AddAttr("id", id); 
	x.AddAttr("owner", owner);  
	WokXMLTag &hash = x.AddTag("hash");
	char data[] = "0123456789abcdef";
	std::string h;
	for( int x = 0 ; x < 16 ; x++ )
		h += data[rand()/(RAND_MAX/16)];
	hash.AddText(h);
	
	wls->SendSignal("Jabber XML Send", mtag);
	
	EXP_SIGHOOK("Jabber RandomNumber Session '" + XMLisize(origxml.GetAttr("session")) + "' '" + XMLisize(x.GetAttr("owner")) + "' '" + x.GetAttr("id") + "'", &Session::Message, 1000);
}


Session::Session(WLSignal *wls, WokXMLTag *xml) : WLSignalInstance(wls),
origxml(*xml)
{
	WokXMLTag whoami("whoami");
	wls->SendSignal("Jabber GroupChat Whoami '" + XMLisize(origxml.GetAttr("session")) + "' '" + XMLisize(origxml.GetAttr("roomjid")) + "'", whoami);
	mynick = whoami.GetAttr("nick");
	
	id = xml->GetAttr("id");
	owner = xml->GetAttr("owner");
	
	WokXMLTag mtag("message");
	mtag.AddAttr("session", origxml.GetAttr("session"));
	WokXMLTag &message = mtag.AddTag("message");
	message.AddAttr("to", origxml.GetAttr("roomjid"));
	message.AddAttr("type", "groupchat");
	WokXMLTag &x = message.AddTag("x");
	x.AddAttr("xmlns", "RandomNumber");
	x.AddAttr("type", "participant seed");
	x.AddAttr("id", id);
	x.AddAttr("owner", xml->GetAttr("owner"));
	WokXMLTag &hash = x.AddTag("hash");
	char data[] = "0123456789abcdef";
	std::string h;
	for( int x = 0 ; x < 16 ; x++ )
		h += data[rand()/(RAND_MAX/16)];
	hash.AddText(h);
	
	wls->SendSignal("Jabber XML Send", mtag);
}

int
Session::Message(WokXMLTag *tag)
{	
	std::string type = tag->GetFirstTag("message").GetFirstTag("x", "RandomNumber").GetAttr("type");
	if ( type == "cancel" )
	{
		
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
	}
	
	hashes[tag->GetFirstTag("message").GetAttr("from")] = tag->GetFirstTag("message").GetFirstTag("x", "RandomNumber").GetFirstTag("hash").GetBody();
	
	if ( hashes.size() == requiredusers.size() )
	{
		unsigned long int r = 0;
		std::cout << "We now have the requried data to generate a random number..." << std::endl;
		
		std::map <std::string, std::string>::iterator hashiter;
		for ( hashiter = hashes.begin() ; hashiter != hashes.end() ; hashiter++)
		{
			unsigned long int x = strtoull(("0x" + hashiter->second).c_str(), NULL, 16);
			r ^= x;
			
		}
		
		std::cout << "we now have the random number.." << r << std::endl;
		std::stringstream str;
		str << std::hex << r;
		WokXMLTag number ("number");
		number.AddAttr("num", str.str());
		wls->SendSignal("Jabber RandomNumber Generated '" + XMLisize(origxml.GetAttr("session")) + "' '" + XMLisize(owner) + "' '" + XMLisize(id) + "'", number);
	}
	return 1;
}

