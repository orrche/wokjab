/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2008-2011 <nedo80@gmail.com>
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

#include "wokjab-dock-window-handler.hpp"
#include <sstream>
#include <utility>

WokjabDockWindowHandler::WokjabDockWindowHandler(WLSignal *wls) : WoklibPlugin(wls)
{
	// What happens if this is done elsewhere to? Feels bad.
	Gtk::Main kit(0, NULL);

	
	EXP_SIGHOOK("Wokjab DockWindow Add", &WokjabDockWindowHandler::Add, 1000);
	EXP_SIGHOOK("Wokjab DockWindow Destroy", &WokjabDockWindowHandler::Destroy, 1000);
	EXP_SIGHOOK("Wokjab DockWindow Activate", &WokjabDockWindowHandler::Activate, 1000);
	EXP_SIGHOOK("Wokjab DockWindow SetUrgencyHint", &WokjabDockWindowHandler::SetUrgency, 1000);
	
	EXP_SIGHOOK("Wokjab DockWindow Hide", &WokjabDockWindowHandler::Hide, 1000);
	EXP_SIGHOOK("Wokjab DockWindow Show", &WokjabDockWindowHandler::Show, 1000);
}

int
WokjabDockWindowHandler::SetUrgency(WokXMLTag *tag)
{
	if ( windows.find(tag->GetAttr("id"))  != windows.end() ) 
	{
		windows[tag->GetAttr("id")]->SetUrgencyHint(tag);
	}
	
	return 1;
}


int
WokjabDockWindowHandler::Hide(WokXMLTag *tag)
{
	if ( windows.find(tag->GetAttr("id"))  != windows.end() ) 
	{
		windows[tag->GetAttr("id")]->Hide(tag);
	}
	return 1;
}

int
WokjabDockWindowHandler::Show(WokXMLTag *tag)
{
	if ( windows.find(tag->GetAttr("id"))  != windows.end() ) 
	{
		windows[tag->GetAttr("id")]->Show(tag);
	}
	return 1;
}

int
WokjabDockWindowHandler::Activate(WokXMLTag *tag)
{
	if ( windows.find(tag->GetAttr("id"))  != windows.end() ) 
	{
		windows[tag->GetAttr("id")]->Activate();
	}
	return 1;
}

int
WokjabDockWindowHandler::Add(WokXMLTag *tag)
{
	std::map<std::string, WokjabDockWindow* >::iterator iter;
	
	if ( masterlist.find(tag->GetAttr("type")) == masterlist.end() )
	{
		WokjabDockWindowMaster *master = new WokjabDockWindowMaster(tag->GetAttr("type"), this);
		
		masterlist[tag->GetAttr("type")] = master;
	}
	windows[tag->GetAttr("id")] = new WokjabDockWindow(wls, tag, masterlist[tag->GetAttr("type")]);
		
	return 1;
}

void
WokjabDockWindowHandler::RemoveWindow(WokjabDockWindowMaster *wdwm)
{
	std::map<std::string, WokjabDockWindow*>::iterator iter;
	std::list<WokjabDockWindow*> deletelist;
	std::list<WokjabDockWindow*>::iterator deliter;

	for ( iter = windows.begin() ; iter != windows.end() ; iter++ )
	{
		if ( iter->second->GetType() == wdwm->getType() )
		{
			deletelist.push_back(iter->second);
		}
	}

	for( deliter = deletelist.begin() ; deliter != deletelist.end() ; deliter++) 
	{

		WokXMLTag tag("destroy");
		tag.AddAttr("id", (*deliter)->GetID());
		wls->SendSignal("Wokjab DockWindow Destroy", tag);
		std::cout << "should remove a window" << std::endl;
	}
}

int
WokjabDockWindowHandler::Destroy(WokXMLTag *tag)
{
	if ( windows.find(tag->GetAttr("id"))  != windows.end() ) 
	{
		WokjabDockWindow *tmp = windows[tag->GetAttr("id")];
		windows.erase(tag->GetAttr("id"));

		std::string type = tmp->GetType();
		delete tmp;
		WokjabDockWindowMaster *dwm = masterlist[type];
		Gtk::Notebook *nb = dwm->GetNotebook();

		std::cout << "Pages: " << nb->get_n_pages() << std::endl;

		if( nb->get_n_pages() == 0 )
		{
			delete dwm;
			masterlist.erase(type);
		}
	}
	
	return 1;
}
