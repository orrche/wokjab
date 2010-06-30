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

#include <algorithm>
#include "group-manager.hpp"

GroupManager::GroupManager(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("GetMenu", &GroupManager::Menu, 999);
	EXP_SIGHOOK("Jabber GUI GroupManager", &GroupManager::DialogOpener, 999);
}

GroupManager::~GroupManager()
{
	std::list<GM_Widget*>::iterator iter;
	for( iter = window.begin() ; iter != window.end() ; iter++)
	{
		delete *iter;
	}
	
}

int
GroupManager::Menu(WokXMLTag *xml)
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
	tag->AddAttr("name", "Group Manager");
	tag->AddAttr("signal", "Jabber GUI GroupManager");
	
	return 1;
}


int
GroupManager::DialogOpener(WokXMLTag *tag)
{
	window.push_back( new GM_Widget(wls, this) );
	
	return 1;
}


void
GroupManager::DialogOpenerRemove( GM_Widget *gmw )
{
	std::list<GM_Widget*>::iterator iter;
	
	iter = std::find(window.begin(), window.end(), gmw );
	
	if( iter != window.end() )
	{
		delete *iter;
		window.erase(iter);
	}
}


