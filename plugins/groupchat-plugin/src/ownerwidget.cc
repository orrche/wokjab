/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <oden@gmx.net>
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

#include "ownerwidget.h"
#include <Woklib/WokXMLTag.h>

ownerwidget::ownerwidget(WLSignal *wls, const std::string &session, const std::string &roomjid, const std::string & hookid) : WLSignalInstance(wls),
session(session),
roomjid(roomjid)
{
	id = std::string("Jabber XML IQ ID ") + hookid;
	EXP_SIGHOOK(id.c_str(), &ownerwidget::xml, 1000);
}


ownerwidget::~ownerwidget()
{

}

int
ownerwidget::xml (WokXMLTag *tag)
{
	WokXMLTag *tag_iq;
	WokXMLTag *tag_x;
	tag_iq = &tag->GetFirstTag("iq");
	
	tag_x = &tag_iq->GetFirstTag("query").GetFirstTag("x");
	
	WokXMLTag msgtag( NULL, "message");
	msgtag.AddAttr("session", session);
	msgtag.AddTag(tag_x);
	
	wls->SendSignal("Jabber jabber:x:data Init", &msgtag);

	EXP_SIGHOOK(msgtag.GetAttr("signal"), &ownerwidget::answere, 1000);
	return 1;
}

int
ownerwidget::answere(WokXMLTag *tag)
{
	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", session);
	WokXMLTag &iqtag = msgtag.AddTag("iq");
	iqtag.AddAttr("type", "set");
	iqtag.AddAttr("to", roomjid);
#warning Really LAME
	WokXMLTag &querytag = iqtag.AddTag("query");
	querytag.AddAttr("xmlns", "http://jabber.org/protocol/muc#owner");
	querytag.AddText( tag->GetChildrenStr() );
	
	wls->SendSignal("Jabber XML IQ Send", &msgtag);
	return true;
}

