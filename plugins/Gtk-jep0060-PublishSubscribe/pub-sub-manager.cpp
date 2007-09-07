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

#include "pub-sub-manager.hpp"


PubSubManager::PubSubManager(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("GetMenu", &PubSubManager::Menu, 999);
	EXP_SIGHOOK("Jabber GUI PubSubManager", &PubSubManager::DialogOpener, 999);
	
	EXP_SIGHOOK("Jabber disco Identity pubsub", &PubSubManager::DiscoOpener, 999);
	
}


PubSubManager::~PubSubManager()
{
	
	
	
}

int
PubSubManager::DiscoOpener(WokXMLTag *tag)
{
	WokXMLTag opener("open");
	WokXMLTag &data = opener.AddTag("data");
	data.AddTag("jid").AddText(tag->GetAttr("jid"));
	data.AddTag("node").AddText(tag->GetAttr("node"));
	
	wls->SendSignal("Jabber GUI PubSubManager", opener);
	
	return 1;
}

int
PubSubManager::Menu(WokXMLTag *xml)
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
	tag->AddAttr("name", "Publish Subscribe");
	tag->AddAttr("signal", "Jabber GUI PubSubManager");
	
	return 1;
}



int
PubSubManager::DialogOpener(WokXMLTag *tag)
{
	window.push_back( new PubSub_Widget(wls, tag, this) );
	
	return 1;
}


void
PubSubManager::DialogOpenerRemove( PubSub_Widget *pepw )
{
	std::list<PubSub_Widget*>::iterator iter;
	
	iter = std::find(window.begin(), window.end(), pepw );
	
	if( iter != window.end() )
	{
		delete *iter;
		window.erase(iter);
	}
}

