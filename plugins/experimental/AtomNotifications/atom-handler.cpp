/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2008 <nedo80@gmail.com>
 * 
 * wokjab is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * wokjab is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "atom-handler.hpp"


AtomHandler::AtomHandler(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber PubSub Item xmlns 'entry' 'http://www.w3.org/2005/Atom'", &AtomHandler::NewAtom, 1000);
	EXP_SIGHOOK("Jabber PubSub Atom Link", &AtomHandler::Link, 1000);
}

int
AtomHandler::Link(WokXMLTag *tag)
{
	std::cout << "XML: " << *tag << std::endl;
	
	if ( tag->GetAttr("href").substr(0,7) == "http://" )
	{
		WokXMLTag run("run");
		run.AddTag("link").AddAttr("url", tag->GetAttr("href"));
		
		wls->SendSignal("HtmlLink RunBrowser", run);
	}
		
		
	return 1;
}

int
AtomHandler::NewAtom(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator item;
	
	for ( item = tag->GetFirstTag("message").GetFirstTag("event", "http://jabber.org/protocol/pubsub#event").GetFirstTag("items").GetTagList("item").begin() ; 
		item != tag->GetFirstTag("message").GetFirstTag("event", "http://jabber.org/protocol/pubsub#event").GetFirstTag("items").GetTagList("item").end() ; item++)
	{
		WokXMLTag &entry = (*item)->GetFirstTag("entry", "http://www.w3.org/2005/Atom");
		WokXMLTag notify("notification");
		WokXMLTag &itemtag = notify.AddTag("item");
		WokXMLTag &body = itemtag.AddTag("body");
		body.AddText("Atom <");
		body.AddTag("b").AddText(entry.GetFirstTag("source").GetFirstTag("title").GetBody());
		body.AddText(">\n" +entry.GetFirstTag("title").GetBody() +"\n");
		body.AddTag("i").AddText(entry.GetFirstTag("summary").GetBody());

		if ( !entry.GetTagList("link").empty() )
		{
			WokXMLTag &commands = itemtag.GetFirstTag("commands");
			
			std::list <WokXMLTag *>::iterator linkiter;
			for(linkiter = entry.GetTagList("link").begin(); linkiter != entry.GetTagList("link").end(); linkiter++)
			{
				WokXMLTag &command = commands.AddTag("command");
				if ( (*linkiter)->GetBody().empty() )
					command.AddAttr("name", (*linkiter)->GetAttr("href"));
				else
					command.AddAttr("name", (*linkiter)->GetBody());
				
				WokXMLTag &sig = command.AddTag("signal");
				sig.AddAttr("name" , "Jabber PubSub Atom Link");
				sig.AddTag(*linkiter);
			
			}
		}
		
		wls->SendSignal("Notification Add", notify);
	}

	
	
	return 1;	
}
