/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2009 <nedo80@gmail.com>
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

#include <sstream>
#include "conn-window-manager.h"

ConnWindowManager::ConnWindowManager(WLSignal *wls) : WoklibPlugin(wls)
{

	winid = 0;
	EXP_SIGHOOK("GUIConnWindow Open", &ConnWindowManager::Open, 2);
	EXP_SIGHOOK("GUIConnWindow Close", &ConnWindowManager::Close, 2);
	
}



ConnWindowManager::~ConnWindowManager()
{
	std::map<std::string, GUIConnectWindow*>::iterator iter;

	for(iter = win.begin() ; iter != win.end() ; iter++)
	{
		delete iter->second;
	}

	win.clear();
}

int
ConnWindowManager::Open(WokXMLTag *tag)
{
	std::stringstream str;
	str << winid++;
	tag->AddAttr("id", str.str());
	
	win[str.str()] = new GUIConnectWindow(wls, str.str());


	
	return 1;
}

int
ConnWindowManager::Close(WokXMLTag *tag)
{
	if ( win.find(tag->GetAttr("id")) != win.end() )
	{
		delete win[tag->GetAttr("id")];
		win.erase(tag->GetAttr("id"));
	}
			

	return 1;   
}

