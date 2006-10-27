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

#include "rename.h"
#include "renamewidget.h"

Rename::Rename(WLSignal *wls):
WoklibPlugin(wls)
{
	this->wls = wls;
/*	GUIWindowAddMenuSignal sig_data("Join a groupchat", (void *)GroupChat::JoinGroupChat, this);
	wls->SendSignal("GUI Window add menu", &sig_data ); 
*/	
	EXP_SIGHOOK("Jabber GUI Roster GetJIDMenu", &Rename::JIDMenu, 999);
	EXP_SIGHOOK("GUI Roster Rename", &Rename::RenameWid, 999);
}

Rename::~Rename()
{
	EXP_SIGUNHOOK("GUI Roster Rename", &Rename::RenameWid, 999);
}


int
Rename::JIDMenu(WokXMLTag *xml)
{
	WokXMLTag *tag;
	tag = &xml->AddTag("item");
	tag->AddAttr("name", "Rename");
	tag->AddAttr("signal", "GUI Roster Rename");
	
	return 1;
}

int
Rename::RenameWid(WokXMLTag *xml)
{
	// Self destructs
	new RenameWidget(wls, xml->GetAttr("jid"), xml->GetAttr("session"));
	return 1;
}
