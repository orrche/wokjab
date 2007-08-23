/***************************************************************************
 *  Copyright (C) 2005  Kent Gustavsson <oden@gmx.net>
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
// Class: EntCap
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Fri Aug 26 22:37:01 2005
//

#include "EntCap.h"
#include <sstream>
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

EntCap::EntCap(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber XML Presence Send", &EntCap::Presence, 500);
}


EntCap::~EntCap()
{
	// TODO: put destructor code here
}

int
EntCap::Presence(WokXMLTag *tag)
{
	WokXMLTag caps(NULL, "cap");
	wls->SendSignal("Jabber jep115 GetCap", &caps);
	
	WokXMLTag &captag = tag->GetFirstTag("presence").AddTag("c");
	captag.AddAttr("node", "http://wokjab.sf.net/caps");
	captag.AddAttr("version", VERSION);
	captag.AddAttr("xmlns", "http://jabber.org/protocol/caps");
	if( caps.GetTagList("cap").size() )
	{
		std::stringstream buf;
		std::list <WokXMLTag *>::iterator iter;
		for( iter = caps.GetTagList("cap").begin() ; iter != caps.GetTagList("cap").end() ; iter++)
		{
			buf << ' ' << (*iter)->GetBody();		
		}
		captag.AddAttr("ext", buf.str().substr(1));
	}
	
	return true;
}
