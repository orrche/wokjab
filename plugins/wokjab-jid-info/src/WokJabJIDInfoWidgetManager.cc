/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <oden@gmx.net>
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

#include "include/WokJabJIDInfoWidgetManager.h"
#include "include/WokJabJIDInfoWidget.h"

WokJabJIDInfoWidgetManager::WokJabJIDInfoWidgetManager(WLSignal *wls):
WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber GUI GetJIDMenu", &WokJabJIDInfoWidgetManager::InfoMenu, 1);
	EXP_SIGHOOK("Jabber GUI Roster JIDInfo", &WokJabJIDInfoWidgetManager::NewWidget, 1);
}


WokJabJIDInfoWidgetManager::~WokJabJIDInfoWidgetManager()
{

}


int
WokJabJIDInfoWidgetManager::NewWidget(WokXMLTag *tag)
{
	std::string jid = tag->GetAttr("jid");
	if ( jid.find("/") != std::string::npos )
		jid = jid.substr(0, jid.find("/"));
	
	new WokJabJIDInfoWidget(wls, jid, tag->GetAttr("session"));
	return true;
}

int
WokJabJIDInfoWidgetManager::InfoMenu(WokXMLTag *tag)
{
	WokXMLTag *item;
	item = &tag->AddTag("item");
	item->AddAttr("name", "Constact Information");
	item->AddAttr("signal", "Jabber GUI Roster JIDInfo");
	
	return 1;
}
