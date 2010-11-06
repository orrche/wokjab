/***************************************************************************
 *  Copyright (C) 2006-2008  Kent Gustavsson <nedo80@gmail.com>
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
// Class: AdHoc
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sun Oct 15 01:03:03 2006
//

#include "AdHoc.h"
#include "AdHocWid.h"
#include "AdHocSession.h"

AdHoc::AdHoc(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber disco Feature http://jabber.org/protocol/commands", &AdHoc::Feature, 500);
	EXP_SIGHOOK("Jabber GUI GetJIDMenu", &AdHoc::Menu, 1000);
	EXP_SIGHOOK("Jabber AdHoc Start", &AdHoc::Start, 1000);
	EXP_SIGHOOK("Jabber AdHoc Execute", &AdHoc::Exec, 1000);
}


AdHoc::~AdHoc()
{


}

int
AdHoc::Feature(WokXMLTag *tag)
{
	new AdHocWid(wls, *tag);
	return 1;
}

int
AdHoc::Start(WokXMLTag *tag)
{
	new AdHocWid(wls, *tag);
	return 1;
}

int
AdHoc::Menu(WokXMLTag *tag)
{
	std::string jid = tag->GetAttr("jid");

	WokXMLTag &item = tag->AddTag("item");
	if ( jid.find("/") == std::string::npos )
	{
		WokXMLTag xml(NULL, "query");
		WokXMLTag &itag = xml.AddTag("item");
		itag.AddAttr("jid", jid);
		itag.AddAttr("session", tag->GetAttr("session"));
		wls->SendSignal("Jabber Roster GetResource", xml);
		std::string resource = itag.GetFirstTag("resource").GetAttr("name");
		
		if ( resource.empty() )
			item.AddAttr("name", "AdHoc Command");
		else
		{
			item.AddAttr("name", "AdHoc Command (" + resource + ")");
			WokXMLTag &dtag = item.AddTag("data");
			dtag.AddAttr("jid", jid+"/"+resource);
			dtag.AddAttr("session", tag->GetAttr("session"));
		}
	}
	else
		item.AddAttr("name", "AdHoc Command");
	item.AddAttr("signal", "Jabber AdHoc Start");


	return 1;
}

int
AdHoc::Exec(WokXMLTag *tag)
{
	wls->SendSignal("Jabber XML IQ Send", tag);
	EXP_SIGHOOK("Jabber XML IQ ID " + tag->GetFirstTag("iq").GetAttr("id"), &AdHoc::ExecResponse, 1000);
	
	return 1;
}

int
AdHoc::ExecResponse(WokXMLTag *tag)
{
	new AdHocSession(wls,tag);
	return 1;
}
