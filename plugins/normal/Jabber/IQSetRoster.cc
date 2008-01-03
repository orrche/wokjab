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


#include "IQSetRoster.h"

#include <iostream>
#include <list>

/* This class both take cares of sending the IQ request for the roster and 
 * to handle the iq when contacts gets added/removoed/renamed
 * sends signal to a roster class to build a roster
 */

IQSetRoster::IQSetRoster (WLSignal *wls):
WLSignalInstance ( wls )
{
	EXP_SIGHOOK("Jabber XML IQ New query set xmlns:jabber:iq:roster", &IQSetRoster::xml, 1000);
	EXP_SIGHOOK("Jabber Request Roster", &IQSetRoster::Req, 500);
}

IQSetRoster::~IQSetRoster ()
{
}

int
IQSetRoster::Req( WokXMLTag *tag )
{
	WokXMLTag msgtag(NULL,"message");
	msgtag.AddAttr("session", tag->GetAttr("session"));
	WokXMLTag &iqtag = msgtag.AddTag("iq");
	iqtag.AddAttr("type", "get");
	iqtag.AddTag("query").AddAttr("xmlns", "jabber:iq:roster");
	wls->SendSignal("Jabber XML IQ Send", &msgtag);

	std::string id = iqtag.GetAttr("id");

	signal = std::string("Jabber XML IQ ID ") + id;
	EXP_SIGHOOK(signal.c_str(), &IQSetRoster::xml, 1000);
}

int
IQSetRoster::xml( WokXMLTag *tag )
{
	WokXMLTag *tag_x;
	WokXMLTag *tag_iq;
	std::list < WokXMLTag *	>::iterator tag_iter;
	tag_iq = &tag->GetFirstTag("iq");
	tag_x = &tag_iq->GetFirstTag("query");
	
	for(tag_iter = tag_x->GetTagList("item").begin(); tag_iter != tag_x->GetTagList("item").end(); tag_iter++)
	{
		WokXMLTag msgtag (NULL, "message");
		msgtag.AddTag(*tag_iter);
		msgtag.AddAttr("session", tag->GetAttr("session"));
		wls->SendSignal("Jabber Roster Update", &msgtag);
		wls->SendSignal("Jabber Roster Update " + tag->GetAttr("session"), &msgtag);
	}
	
	return 1;
}
