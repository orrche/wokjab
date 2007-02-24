/***************************************************************************
 *  Copyright (C) 2003-2007  Kent Gustavsson <nedo80@gmail.com>
 ****************************************************************************/
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
// Class: Idle
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Jan 16 17:31:52 2007
//

#include "Idle.h"


Idle::Idle(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber Connection Lost", &Idle::LoggedOut, 1);
	EXP_SIGHOOK("Jabber Connection Connect" , &Idle::SignIn, 1000);
}


Idle::~Idle()
{

}

int
Idle::LoggedOut(WokXMLTag *tag)
{
	if ( sessions.find(tag->GetAttr("session")) != sessions.end() )
	{
		delete sessions[tag->GetAttr("session")];
		sessions.erase(tag->GetAttr("session"));
	}

	return 1;
}

int
Idle::SignIn(WokXMLTag *tag)
{
	sessions[tag->GetAttr("session")] = new IdleSession(wls, tag->GetAttr("session"));

	return 1;
}
