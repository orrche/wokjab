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
// Class: IdleSession
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Tue Jan 16 17:34:15 2007
//

#include "IdleSession.h"


IdleSession::IdleSession(WLSignal *wls, std::string session) : WLSignalInstance(wls),
session(session)
{
	EXP_SIGHOOK("Jabber Session IO Active " + session, &IdleSession::Active, 1000);
	EXP_SIGHOOK("Jabber Idle Tick " + session, &IdleSession::Tick, 1000);
	
	WokXMLTag timmertag(NULL, "timer");
	timmertag.AddAttr("time", "1000");
	timmertag.AddAttr("signal", "Jabber Idle Tick " + session);

	wls->SendSignal("Woklib Timmer Add",timmertag);
	t = 0;
}


IdleSession::~IdleSession()
{


}

int
IdleSession::Tick(WokXMLTag *tag)
{
	t++;
	if ( t > 180 )
	{
		WokXMLTag mtag(NULL, "message");
		mtag.AddAttr("session", session);
		mtag.AddText(" ");
		
		wls->SendSignal("Jabber XML Send", mtag);
	}
	if ( t > 180*2 )
	{
		woklib_error(wls, "Connection " + session + " seams to have timedout");
		WokXMLTag mtag(NULL, "message");
		mtag.AddAttr("session", session);
		wls->SendSignal("Jabber Connection Lost", mtag);
	}
	return 1;
}

int
IdleSession::Active(WokXMLTag *tag)
{
	t = 0;


	return 1;
}
