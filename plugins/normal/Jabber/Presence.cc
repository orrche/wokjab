/***************************************************************************
 *  Copyright (C) 2003-2006  Kent Gustavsson <nedo80@gmail.com>
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


#include "Presence.h"
#include <iostream>

Presence::Presence(WLSignal *wls) :
WLSignalInstance ( wls )
{
	EXP_SIGHOOK("Jabber XML Object presence", &Presence::xml, 1000);
	EXP_SIGHOOK("Jabber XML Presence Send", &Presence::Send, 1000);
	EXP_SIGHOOK("Jabber XML Presence GetLast", &Presence::GetLast, 1000);
	
	last_presence = new WokXMLTag ("message");
}


Presence::~Presence()
{
	EXP_SIGUNHOOK("Jabber XML Object presence", &Presence::xml, 1000);
}

int
Presence::GetLast(WokXMLTag *tag)
{
	tag->AddTag(last_presence);
	
	return 1;
}

int
Presence::xml( WokXMLTag *tag)
{
	if( tag->GetFirstTag("presence").GetAttr("type") == "subscribe")
		wls->SendSignal("Jabber XML Presence SubReq", tag);
	else
		wls->SendSignal("Jabber XML Presence", tag);
	
	return 1;
}

int
Presence::Send(WokXMLTag *tag)
{
	delete last_presence;
	last_presence = new WokXMLTag(*tag);
	
	wls->SendSignal("Jabber XML Send", tag);
	return 1;
}
