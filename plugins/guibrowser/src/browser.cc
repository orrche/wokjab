/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <nedo80@gmail.com>
 ****************************************************************************/
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include "browser.h"
#include "browserwid.h"
#include <algorithm>

Browser::Browser(WLSignal *wls):
WoklibPlugin(wls)
{
	EXP_SIGHOOK("Get Main Menu", &Browser::MainMenu, 999);
	EXP_SIGHOOK("GUI Browser Open", &Browser::BrowserWid, 999);
}

Browser::~Browser()
{	
	std::list<BrowserWidget*>::iterator iter;
	for( iter = window.begin() ; iter != window.end() ; iter = window.begin())
	{
		delete *iter;	
	}
}


int
Browser::MainMenu(WokXMLTag *xml)
{
	WokXMLTag *tag;
	tag = &xml->AddTag("item");
	tag->AddAttr("name", "Browser");
	tag->AddAttr("signal", "GUI Browser Open");
	
	return 1;
}

int
Browser::BrowserWid(WokXMLTag *xml)
{
	window.push_back(new BrowserWidget(wls, this));
	return 1;
}

void
Browser::BrowserWidRemove(BrowserWidget *wid)
{
	std::list<BrowserWidget*>::iterator iter;
	
	iter = std::find(window.begin(), window.end(), wid );
	
	if( iter != window.end() )
	{
		window.erase(iter);
	}
}
