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
	EXP_SIGHOOK("Jabber Idle Tick Normal " + session, &IdleSession::Tick, 1000);
	EXP_SIGHOOK("Jabber Idle Tick DeathWatch " + session, &IdleSession::DeathWatch, 1000);

	t = 0;
}


IdleSession::~IdleSession()
{


}

int
IdleSession::DeathWatch(WokXMLTag *tag)
{
	if ( death )
	{
		woklib_error(wls, "Connection " + session + " seams to have timedout\"+
																				"But for now we are ignoring that");
		/*
		WokXMLTag mtag(NULL, "message");
		mtag.AddAttr("session", session);
		wls->SendSignal("Jabber Connection Lost", mtag);
		*/
	
	}
	
	tag->AddAttr("stop", "stop");
	return 1;
}

int
IdleSession::Tick(WokXMLTag *tag)
{
	std::cout << "T: " << t << std::endl;
	if ( t>=0)
		t--;
	if ( t == 0 )
	{
		WokXMLTag mtag(NULL, "message");
		mtag.AddAttr("session", session);
		mtag.AddText(" ");
		
		wls->SendSignal("Jabber XML Send", mtag);
		
		death = true;
		WokXMLTag timmertag(NULL, "timer");
		timmertag.AddAttr("time", "120000");
		timmertag.AddAttr("signal", "Jabber Idle Tick DeathWatch " + session);

		wls->SendSignal("Woklib Timmer Add",timmertag);
	}
	
	tag->AddAttr("stop", "stop");
	return 1;
}

int
IdleSession::Active(WokXMLTag *tag)
{
	death = false;
	t++;
	WokXMLTag timmertag(NULL, "timer");
	timmertag.AddAttr("time", "60000");
	timmertag.AddAttr("signal", "Jabber Idle Tick Normal " + session);

	wls->SendSignal("Woklib Timmer Add",timmertag);

	return 1;
}
