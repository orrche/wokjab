/***************************************************************************
 *  Copyright (C) 2005  Kent Gustavsson <nedo80@gmail.com>
 ****************************************************************************/
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


//
// Class: Auth
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Aug  9 12:59:33 2005
//

#include "auth.h"


Auth::Auth(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber GUI Roster GetJIDMenu", &Auth::Menu, 1000);
	EXP_SIGHOOK("Jabber Roster Presence Request", &Auth::ReqAuthAction, 1000);
}


Auth::~Auth()
{
	// TODO: put destructor code here
}

int
Auth::Menu(WokXMLTag *tag)
{
	WokXMLTag &item = tag->AddTag("item");
	item.AddAttr("name", "Req Auth");
	item.AddAttr("signal", "Jabber Roster Presence Request");
	return true;
}

int
Auth::ReqAuthAction(WokXMLTag *tag)
{
	WokXMLTag msgtag(NULL, "message");
	msgtag.AddAttr("session", tag->GetAttr("session"));
	WokXMLTag &presencetag = msgtag.AddTag("presence");
	presencetag.AddAttr("type", "subscribe");
	
	std::string jid = tag->GetAttr("jid");
	if ( jid.find("/") != std::string::npos )
			jid = jid.substr(0, jid.find("/"));
			
	presencetag.AddAttr("to", jid);
	wls->SendSignal("Jabber XML Send", &msgtag);
	
	return true;
}
