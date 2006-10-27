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
// Class: Disco
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Wed Aug 24 12:22:12 2005
//

#include "Disco.h"


Disco::Disco(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber XML IQ New query get xmlns:http://jabber.org/protocol/disco#info", &Disco::RequestInfo, 500);
}


Disco::~Disco()
{
	// TODO: put destructor code here
}

int
Disco::RequestInfo(WokXMLTag *tag)
{
	WokXMLTag discoitems(NULL, "disco");
	discoitems.AddAttr("jid", tag->GetFirstTag("iq").GetAttr("from"));
	discoitems.AddAttr("session", tag->GetAttr("session"));
	
	std::string node = tag->GetFirstTag("iq").GetFirstTag("query").GetAttr("node");
	if ( node.empty() )
		wls->SendSignal("Jabber Disco Info Get", &discoitems);
	else
		wls->SendSignal("Jabber Disco Info Get Node " + node, &discoitems);
	

	WokXMLTag message(NULL, "message");
	message.AddAttr("session", tag->GetAttr("session"));
	WokXMLTag &iqtag= message.AddTag("iq");
	WokXMLTag &querytag = iqtag.AddTag("query");
	iqtag.AddAttr("type", "result");
	iqtag.AddAttr("to", tag->GetFirstTag("iq").GetAttr("from"));
	iqtag.AddAttr("id", tag->GetFirstTag("iq").GetAttr("id"));
	querytag.AddAttr("xmlns", "http://jabber.org/protocol/disco#info");
	
	std::list < WokXMLTag * > ::iterator iter;
	std::list < WokXMLTag * > *list;
	
	list = &discoitems.GetTags();
	for( iter = list->begin(); iter != list->end() ; iter++)
		querytag.AddTag(*iter);
	
	wls->SendSignal("Jabber XML Send", &message);
	return true;
}
