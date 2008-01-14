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


Rand::Rand(WLSignal *wls) : WoklibPlugin(wls)
{
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


/*
<tag session="jabber0" roomjid="test@conference.jabber.se"/>
*/
int
Rand::NewSession(WokXMLTag *tag)
{
	Session *ses;
	ses = new Session(wls, tag, true);
	sessions.push_back(ses);
	return 1;
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

<message session="jabber0">
  <message to="nedo@jabber.se" from="rand@conference.jabber.se/basil" type="groupchat">
    <x xmlns="RandomNumber">
      <rand owner="rand@conference.jabber.se/nedo", id="4" type="participant seed">
        <hash>5892587866cdf0d229b6b3e3305d997b</hash>
      </rand>
    </x>
  </message>
</message>

<message session="jabber0">
  <message to="nedo@jabber.se" from="rand@conference.jabber.se/basil" type="groupchat">
    <x xmlns="RandomNumber">
      <rand owner="rand@conference.jabber.se/basil", id="13" type="cancel" />
    </x>
  </message>
</message>
*/

int
Rand::Message(WokXMLTag *tag)
{
	WokXMLTag &x = tag->GetFirstTag("message").GetFirstTag("x");
	std::string type = x.GetAttr("type");

	if ( wls->SendSignal("Jabber RandomNumber Session '" + tag->GetAttr("session") + "' '" + x.GetFirstTag("rand").GetAttr("owner") + "'", tag) == 0 )
	{
		if ( tag->GetFirstTag("message").GetFirstTag("x").GetFirstTag("rand").GetAttr("type") == "generate request" )
		{
			WokXMLTag sessiontag("session");
			sessiontag.AddAttr("roomjid", tag->GetFirstTag("message").GetAttr("from").substr(0, tag->GetFirstTag("message").GetAttr("from").find("/")));
			sessiontag.AddAttr("session", tag->GetAttr("session"));
							   
			Session *ses;
			/* Not sure this is entirely correct.. need to think this through maybe later */
			ses = new Session(wls, &sessiontag, false);
			sessions.push_back(ses);
							   
			wls->SendSignal("Jabber RandomNumber Session '" + tag->GetAttr("session") + "' '" + x.GetFirstTag("rand").GetAttr("owner") + "'", tag);
		}
	}
	
	return 1;
}
