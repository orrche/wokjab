/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2007 <nedo80@gmail.com>
 * 
 * wokjab is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * wokjab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with wokjab.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#include "rand.hpp"


Rand::Rand(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber RandomNumber SessionCreate", &Rand::NewSession, 1000);
	EXP_SIGHOOK("Jabber XML Message xmlns RandomNumber", &Rand::Message, 1000);
}

Rand::~Rand()
{
	std::list <Session*>::iterator iter;
	
	for( iter = sessions.begin() ; iter != sessions.end() ; iter++ )
	{
		delete *iter;
	}	
}

int
Rand::NewSession(WokXMLTag *tag)
{
	Session *ses;
	ses = new Session(wls, tag);
	sessions.push_back(ses);
	return 1;
}

int
Rand::Message(WokXMLTag *tag)
{
	WokXMLTag &x = tag->GetFirstTag("message").GetFirstTag("x");
	std::string type = x.GetAttr("type");

	if ( type == "invite" )
	{
		
		
	}
	wls->SendSignal("Jabber RandomNumber Session '" + x.GetAttr("id") + "' '" + XMLisize(type) + "'", tag);
	
	return 1;
}
