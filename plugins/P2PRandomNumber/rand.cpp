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

#include "rand.hpp"
#include <sstream>

Rand::Rand(WLSignal *wls) : WoklibPlugin(wls)
{
	id = 0;
	EXP_SIGHOOK("Jabber RandomNumber RegisterSesseion", &Rand::RegisterSession, 1000);
	EXP_SIGHOOK("Jabber RandomNumber SessionCreate", &Rand::NewSession, 1000);
	EXP_SIGHOOK("Jabber XML Message xmlns RandomNumber", &Rand::Message, 1000);
}

Rand::~Rand()
{
	std::list <Session*>::iterator iter;
	
	for( iter = sessions.begin() ; iter != sessions.end() ; iter++ )
	{
		delete *iter;
	}	
}


int
Rand::RegisterSession(WokXMLTag *tag)
{
	std::stringstream str;
	str << id++;
	
	tag->AddAttr("id", str.str());
	
	return 1;
}
/*
<tag session="jabber0" roomjid="test@conference.jabber.se" id="13"/>
*/
int
Rand::NewSession(WokXMLTag *tag)
{

	Session *ses;
	ses = new Session(wls, tag, tag->GetAttr("id"));
	sessions.push_back(ses);
	return 1;
}

/*
<message session="jabber0">
  <message to="nedo@jabber.se" from="rand@conference.jabber.se/basil" type="groupchat">
    <x xmlns="RandomNumber" owner="rand@conference.jabber.se/basi" id="13" type="genereate request">
      <hash>862e127cfea6b543</hash>
    </x>
  </message>
</message>

<message session="jabber0">
  <message to="nedo@jabber.se" from="rand@conference.jabber.se/basil" type="groupchat">
    <x xmlns="RandomNumber" owner="rand@conference.jabber.se/nedo", id="4" type="participant seed">
      <hash>862e127cfea6b543</hash>
    </x>
  </message>
</message>

<message session="jabber0">
  <message to="nedo@jabber.se" from="rand@conference.jabber.se/basil" type="groupchat">
    <x xmlns="RandomNumber" rand owner="rand@conference.jabber.se/basil", id="13" type="cancel" />
  </message>
</message>
*/

int
Rand::Message(WokXMLTag *tag)
{
	std::cout << "hum XML: " << *tag << std::endl;
	
	WokXMLTag &x = tag->GetFirstTag("message").GetFirstTag("x", "RandomNumber");
	std::cout << "X: " << x << std::endl;
	
	std::string type = x.GetAttr("type");
	std::string roomjid = tag->GetFirstTag("message").GetAttr("from").substr(0, tag->GetFirstTag("message").GetAttr("from").find("/"));
	
	if ( wls->SendSignal("Jabber RandomNumber Session '" + XMLisize(tag->GetAttr("session")) + "' '" + XMLisize(x.GetAttr("owner")) + "' '"+ XMLisize(x.GetAttr("id")) + "'", tag) == 0 )
	{
		if ( x.GetAttr("type") == "generate request" )
		{			
			WokXMLTag sessiontag("session");
			sessiontag.AddAttr("roomjid", tag->GetFirstTag("message").GetAttr("from").substr(0, tag->GetFirstTag("message").GetAttr("from").find("/")));
			sessiontag.AddAttr("session", tag->GetAttr("session"));
			sessiontag.AddAttr("owner", tag->GetFirstTag("message").GetAttr("from"));
			sessiontag.AddAttr("id", x.GetAttr("id"));
			Session *ses;
			
			ses = new Session(wls, &sessiontag);
			sessions.push_back(ses);
							   
			wls->SendSignal("Jabber RandomNumber Session '" + XMLisize(tag->GetAttr("session")) + "' '" + XMLisize(x.GetAttr("owner")) + "' '"+ XMLisize(x.GetAttr("id")) + "'", tag);
		}
	}
	
	return 1;
}
