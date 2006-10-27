/***************************************************************************
 *  Copyright (C) 2003-2006  Kent Gustavsson <oden@gmx.net>
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
// Class: WoklibPlugin
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Jan 31 09:50:18 2006
//

#include "MenuResource.h"


MenuResource::MenuResource(WLSignal *wls) : WoklibPlugin(wls)
{
		EXP_SIGHOOK("Jabber GUI GetJIDMenu", &MenuResource::Menu, 1000);
}


MenuResource::~MenuResource()
{

}

int
MenuResource::Menu(WokXMLTag *xml)
{
	if ( !xml->GetAttr("jid").size() || xml->GetAttr("jid").find("/") != std::string::npos)
		return true;
	
	WokXMLTag resourcetag(NULL, "query");
	WokXMLTag &rtag = resourcetag.AddTag("item");
	rtag.AddAttr("jid", xml->GetAttr("jid"));
	rtag.AddAttr("session", xml->GetAttr("session"));
	
	wls->SendSignal("Jabber Roster GetResource", &resourcetag);
	
	WokXMLTag &ritem = xml->AddTag("item");
	ritem.AddAttr("name", "Resources");
	ritem.AddAttr("jid", xml->GetAttr("jid"));
	ritem.AddAttr("session", xml->GetAttr("session"));
	
	
	std::list <WokXMLTag *>::iterator liter;
	for( liter = rtag.GetTagList("resource").begin() ; liter != rtag.GetTagList("resource").end() ; liter++)
	{
		WokXMLTag &resreq = ritem.AddTag("item");
		resreq.AddAttr("jid", xml->GetAttr("jid") + "/" + (*liter)->GetAttr("name"));
		resreq.AddAttr("session", xml->GetAttr("session"));
		resreq.AddAttr("name", (*liter)->GetAttr("name"));
		WokXMLTag &datatag = resreq.AddTag("data");
		datatag.AddAttr("jid", xml->GetAttr("jid") + "/" + (*liter)->GetAttr("name"));
		datatag.AddAttr("session", xml->GetAttr("session"));
		wls->SendSignal("Jabber GUI GetJIDMenu", &resreq);
		
	}

	return true;
}

