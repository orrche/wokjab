/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent 2007 <Gustavsson>
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

#include <algorithm>
#include "p-e-p-manager.hpp"


PEPManager::PEPManager(WLSignal *wls) : WoklibPlugin(wls)
{
	
	EXP_SIGHOOK("GetMenu", &PEPManager::Menu, 999);
	EXP_SIGHOOK("Jabber GUI PEPManager", &PEPManager::DialogOpener, 999);
	EXP_SIGHOOK("Jabber PubSub Registration GetJIDs", &PEPManager::AddRegJID, 999);
}



PEPManager::~PEPManager()
{
	std::list<PEP_Widget*>::iterator iter;
	for( iter = window.begin() ; iter != window.end() ; iter++)
	{
		delete *iter;
	}
}

int
PEPManager::AddRegJID(WokXMLTag *xml)
{
	WokXMLTag &item = xml->AddTag("item");
	
	WokXMLTag querytag(NULL, "query");
	WokXMLTag &itemtag = querytag.AddTag("item");
	itemtag.AddAttr("session", xml->GetAttr("session"));
	wls->SendSignal("Jabber Connection GetUserData", &querytag);
	std::string jid = itemtag.GetFirstTag("jid").GetBody();
	
	if( jid.find("/") != std::string::npos )
		jid = jid.substr(0, jid.find("/"));
	item.AddAttr("jid", jid);
	return 1;	
}

int
PEPManager::Menu(WokXMLTag *xml)
{
	
	std::list <WokXMLTag *>::iterator iter;
	WokXMLTag *settingstag = NULL;
	for ( iter = xml->GetTagList("item").begin() ; iter != xml->GetTagList("item").end() ; iter++)
	{
		if ( (*iter)->GetAttr("name") == "Settings")
		{
			settingstag = *iter;
			break;
		}
	}
	if( !settingstag )
	{
		settingstag = &xml->AddTag("item");
		settingstag->AddAttr("name", "Settings");
	}
	
	WokXMLTag *tag;
	tag = &settingstag->AddTag("item");
	tag->AddAttr("name", "PEP Manager");
	tag->AddAttr("signal", "Jabber GUI PEPManager");
	
	return 1;
}


int
PEPManager::DialogOpener(WokXMLTag *tag)
{
	window.push_back( new PEP_Widget(wls, this) );
	
	return 1;
}


void
PEPManager::DialogOpenerRemove( PEP_Widget *pepw )
{
	std::list<PEP_Widget*>::iterator iter;
	
	iter = std::find(window.begin(), window.end(), pepw );
	
	if( iter != window.end() )
	{
		delete *iter;
		window.erase(iter);
	}
}



