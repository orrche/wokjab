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

#include "event-manager.hpp"
#include <sstream>

EventManager::EventManager(WLSignal *wls) : WoklibPlugin(wls)
{
	id = 0;
	
	EXP_SIGHOOK("Jabber Event Add", &EventManager::Add, 10);
	EXP_SIGHOOK("Jabber Event Remove", &EventManager::Remove, 500);
}

EventManager::~EventManager()
{
	
	
	
	
}

int
EventManager::Remove(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator iter;
	
	for( iter = tag->GetTagList("item").begin() ; iter != tag->GetTagList("item").end() ; iter++)
	{
		if ( (*iter)->GetAttr("id") != "") 
		{
			wls->SendSignal("Jabber Event Remove " + (*iter)->GetAttr("id"), (*iter));
		}
	}
	
	return 1;
}


int
EventManager::Add(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator iter;
	
	for( iter = tag->GetTagList("item").begin() ; iter != tag->GetTagList("item").end() ; iter++)
	{
		if ( (*iter)->GetAttr("id").empty()) 
		{
			std::stringstream str;
			str << id++;
		
			(*iter)->AddAttr("id", "EventManager " + str.str());		
		}
		if ( !(*iter)->GetAttr("timeout").empty() )
		{
			WokXMLTag timer("timer");
			WokXMLTag &datatag = timer.AddTag("data");
			datatag.AddAttr("id", (*iter)->GetAttr("id"));
			timer.AddAttr("time", (*iter)->GetAttr("timeout"));
			
			wls->SendSignal("Woklib Timmer Add", timer);
			
			EXP_SIGHOOK(timer.GetAttr("signal"), &EventManager::Timeout, 1000);
		}
	}
	
	return 1;
}

int
EventManager::Timeout(WokXMLTag *tag)
{
	WokXMLTag remove("remove");
	remove.AddTag("item").AddAttr("id", tag->GetAttr("id"));
	
	wls->SendSignal("Jabber Event Remove", remove);
	return 1;	
}
