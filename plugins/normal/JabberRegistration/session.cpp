/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2008 <nedo80@gmail.com>
 * 
 * wokjab is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * wokjab is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "session.hpp"


Session::Session(WLSignal *wls, WokXMLTag &in_origxml, JabberRegHandler *in_father) : WLSignalInstance(wls),
origxml(new WokXMLTag(in_origxml)),
father(in_father)
{	
}

Session::~Session()
{
	delete origxml;
}

int
Session::Finnish(WokXMLTag *tag)
{
	if ( tag->GetFirstTag("iq").GetAttr("type") == "error" )
	{
		woklib_error(wls, tag->GetFirstTag("iq").GetFirstTag("error").GetStr());
	}
	else
	{
		
	}
	
	WokXMLTag dctag("disconnect");
	dctag.AddAttr("session", origxml->GetAttr("session"));
	wls->SendSignal("Jabber Connection Disconnect", dctag);
	
	father->Remove(origxml->GetAttr("session"));
	return 1;
}

int
Session::RegTagResp(WokXMLTag *tag)
{
	if ( tag->GetFirstTag("iq").GetAttr("type") == "error" )
		return 1;
	
	std::list <WokXMLTag *>::iterator tagiter;

	WokXMLTag msgtag("message");
	msgtag.AddAttr("session", tag->GetAttr("session"));
	msgtag.AddTag(&tag->GetFirstTag("iq"));
	msgtag.GetFirstTag("iq").AddAttr("type", "set");
	msgtag.GetFirstTag("iq").RemoveAttr("from");
	msgtag.GetFirstTag("iq").RemoveAttr("id");
	
	for (tagiter = msgtag.GetFirstTag("iq").GetFirstTag("query").GetTags().begin() ; tagiter != 				msgtag.GetFirstTag("iq").GetFirstTag("query").GetTags().end(); tagiter++)
	{
		if ( !origxml->GetAttr((*tagiter)->GetName()).empty() )
		{
			(*tagiter)->AddText(origxml->GetAttr((*tagiter)->GetName()));
		}
	}
	
	wls->SendSignal("Jabber XML IQ Send", msgtag);
	EXP_SIGHOOK("Jabber XML IQ ID " + msgtag.GetFirstTag("iq").GetAttr("id"), &Session::Finnish, 1000);
	return 1;
}

int
Session::Connect(WokXMLTag *tag)
{
	
/*
	<iq type='get' id='reg1'>
  		<query xmlns='jabber:iq:register'/>
	</iq>
*/
	if ( origxml->GetAttr("username").empty() )
	{
		WokXMLTag regtag("register");
		regtag.AddAttr("session", origxml->GetAttr("session"));
		regtag.AddAttr("jid", origxml->GetAttr("server"));
		wls->SendSignal("Jabber disco Feature jabber:iq:register", regtag);
	}
	else
	{
		WokXMLTag msgtag("message");
		msgtag.AddAttr("session", origxml->GetAttr("session"));
		WokXMLTag &iqtag = msgtag.AddTag("iq");
		iqtag.AddAttr("type", "get");
		iqtag.AddTag("query", "jabber:iq:register");
		
		wls->SendSignal("Jabber XML IQ Send", msgtag);
		EXP_SIGHOOK("Jabber XML IQ ID " + msgtag.GetFirstTag("iq").GetAttr("id"), &Session::RegTagResp, 1000);
	}
	
	return 1;
}
