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
// Class: AdHoc
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Sun Oct 15 01:03:03 2006
//

#include "AdHoc.h"
#include "AdHocWid.h"

AdHoc::AdHoc(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber disco Feature http://jabber.org/protocol/commands", &AdHoc::Feature, 500);
	EXP_SIGHOOK("Jabber GUI GetJIDMenu", &AdHoc::Menu, 1000);
	EXP_SIGHOOK("Jabber AdHoc Start", &AdHoc::Start, 1000);
	EXP_SIGHOOK("Jabber XML IQ New command set xmlns:http://jabber.org/protocol/commands", &AdHoc::CommandExec, 1000);
}


AdHoc::~AdHoc()
{


}

int
AdHoc::Feature(WokXMLTag *tag)
{
	new AdHocWid(wls, tag->GetAttr("session"), tag->GetAttr("jid"), "");
	return 1;
}

int
AdHoc::Start(WokXMLTag *tag)
{
	new AdHocWid(wls, tag->GetAttr("session"), tag->GetAttr("jid"), "");
	return 1;
}

int
AdHoc::Menu(WokXMLTag *tag)
{
	WokXMLTag &item = tag->AddTag("item");
	item.AddAttr("name", "AdHoc Command");
	item.AddAttr("signal", "Jabber AdHoc Start");


	return 1;
}

int
AdHoc::CommandExec(WokXMLTag *tag)
{
	wls->SendSignal("Jabber AdHoc Command Exec " + tag->GetFirstTag("iq").GetFirstTag("command").GetAttr("node"), tag);
	return 1;
}

