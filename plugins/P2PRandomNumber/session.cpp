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

Session::Session(WLSignal *wls, WokXMLTag *xml, bool mine) : WLSignalInstance(wls),
origxml(*xml),
mine(mine)
{
	EXP_SIGHOOK("Jabber RandomNumber Session '" + XMLisize(origxml.GetAttr("session")) + "' '" + XMLisize(origxml.GetAttr("roomjid")) + "' Generate", &Session::Generate, 1000);
	
	if ( mine ) 
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
		WokXMLTag mtag("message");
		mtag.AddAttr("session", origxml.GetAttr("session"));
		WokXMLTag &message = mtag.AddTag("message");
		message.AddAttr("to", origxml.GetAttr("roomjid"));
		message.AddAttr("type", "groupchat");
		WokXMLTag &x = message.AddTag("x");
		x.AddAttr("xmlns", "RandomNumber");
		x.AddAttr("type", "generate request");
		x.AddAttr("id", "13"); // fix ..
		WokXMLTag &hash = x.AddTag("hash");
		char data[] = "123456789abcdef";
		std::string h;
		for( int x = 0 ; x < 16 ; x++ )
			h += data[16 * rand()/RAND_MAX];
		hash.AddText(h);
	}
	else
	{
#warning cant be programming with an english keyboard dont find anything...
		WokXMLTag mtag("message");
		mtag.AddAttr("session", origxml.GetAttr("session"));
	}
}

int
Session::Generate(WokXMLTag *tag)
{
	
	
	
	
	return 1;
}


