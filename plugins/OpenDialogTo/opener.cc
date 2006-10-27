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
// Class: Opener
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Jan 24 17:12:23 2006
//

#include "opener.h"
#include "dopener.h"

Opener::Opener(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("GetMenu", &Opener::Menu, 999);
	EXP_SIGHOOK("Jabber GUI DialogOpener", &Opener::DialogOpener, 999);
}

Opener::~Opener()
{
	std::list<DOpener*>::iterator iter;
	for( iter = window.begin() ; iter != window.end() ; iter = window.begin())
	{
		delete *iter;	
	}
}

int
Opener::Menu(WokXMLTag *xml)
{
	WokXMLTag *tag;
	tag = &xml->AddTag("item");
	tag->AddAttr("name", "Send Message To");
	tag->AddAttr("signal", "Jabber GUI DialogOpener");
	
	return 1;
}

int
Opener::DialogOpener(WokXMLTag *tag)
{
	window.push_back(new DOpener(wls, tag, this));
	return 1;
}

void
Opener::DialogOpenerRemove( DOpener *dop )
{
	std::list<DOpener*>::iterator iter;
	
	iter = std::find(window.begin(), window.end(), dop );
	
	if( iter != window.end() )
	{
		window.erase(iter);
	}
}
