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

#include "pub-sub.hpp"


PubSub::PubSub(WLSignal *wls):WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber XML Object message", &PubSub::Message, 10);
	
	
}


PubSub::~PubSub()
{
	
	
	
}


int
PubSub::Message(WokXMLTag *tag)
{
	if( tag->GetFirstTag("message").GetTagList("event").empty() )
		return 1;

	std::list <WokXMLTag *>::iterator iter;
	
	for( iter = tag->GetFirstTag("message").GetTagList("event").begin() ; iter != tag->GetFirstTag("message").GetTagList("event").end() ; iter++)
	{
		if( (*iter)->GetAttr("xmlns") == "http://jabber.org/protocol/pubsub#event")
		{
			std::list <WokXMLTag *>::iterator item;
			for ( item = (*iter)->GetTagList("items").begin() ; item != (*iter)->GetTagList("items").end() ; item++ )
			{
				wls->SendSignal("Jabber PubSub JID '" + XMLisize(tag->GetFirstTag("message").GetAttr("from")) + "' '" + XMLisize((*item)->GetAttr("node")) + "'", tag); 				
				wls->SendSignal("Jabber PubSub JID '" + XMLisize((*item)->GetAttr("node")) + "'", tag);
			}
			
		}
		
		
	}
	
	return 1;
}
