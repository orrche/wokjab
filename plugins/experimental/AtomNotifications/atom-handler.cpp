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
	
	EXP_SIGHOOK("GetMenu", &AtomHandler::Menu, 1000);
	
}

int 
AtomHandler::Menu(WokXMLTag *tag)
{
	WokXMLTag &menuitem = tag->AddTag("item");
	menuitem.AddAttr("name", "Atom Feeds");
	
	std::list<WokXMLTag *>::iterator item;
	
	for ( item = atomitems.begin(); item != atomitems.end(); item++)
	{
//		string body;
	
		WokXMLTag &entry = (*item)->GetFirstTag("entry", "http://www.w3.org/2005/Atom");

//		body = entry.GetFirstTag("source").GetFirstTag("title").GetBody());
		//body.AddText(">\n" +entry.GetFirstTag("title").GetBody() +"\n");
		//body.AddTag("i").AddText(entry.GetFirstTag("summary").GetBody());

		std::list <WokXMLTag *>::iterator feediter;
//		for( feediter = menuitem->GetTagList("item").begin(); feediter != menuitem->GetTagList("item").end(); feediter++)
	//	{
		//	if ( (*feediter)->GetAttr("name") == entry.
		
	//	}

		WokXMLTag &subitem = menuitem.AddTag("item");
		subitem.AddAttr("name", entry.GetFirstTag("title").GetBody());
		
		WokXMLTag &summaryitem = subitem.AddTag("item");
		summaryitem.AddAttr("name", entry.GetFirstTag("summary").GetBody());
		
		std::list <WokXMLTag *>::iterator linkiter;
		for(linkiter = entry.GetTagList("link").begin(); linkiter != entry.GetTagList("link").end(); linkiter++)
		{
			WokXMLTag &cmditem = subitem.AddTag("item");
			
			if ( (*linkiter)->GetAttr("rel").empty() )
				cmditem.AddAttr("name", (*linkiter)->GetAttr("href"));
			else
				cmditem.AddAttr("name", (*linkiter)->GetAttr("rel"));
			
			cmditem.AddAttr("signal", "Jabber PubSub Atom Link");
			WokXMLTag &sig = cmditem.AddTag("data");
			sig.AddTag(*linkiter);
			sig.GetFirstTag("link").AddAttr("session", (*item)->GetFirstTag("session", "wokjab").GetAttr("session"));
		
		}
	}
	
	return 1;
}

int
AtomHandler::Link(WokXMLTag *tag)
{
	if ( tag->GetFirstTag("link").GetAttr("href").substr(0,7) == "http://" )
	{
		WokXMLTag run("run");
		run.AddTag("link").AddAttr("url", tag->GetFirstTag("link").GetAttr("href"));
		
		wls->SendSignal("HtmlLink RunBrowser", run);
	}
	if ( tag->GetFirstTag("link").GetAttr("href").substr(0,5) == "xmpp:")
	{
		WokXMLTag uri("uri");
		uri.AddAttr("uri", tag->GetFirstTag("link").GetAttr("href"));
		uri.AddAttr("session", tag->GetAttr("session"));
		
		wls->SendSignal("Jabber URI", uri);
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
		WokXMLTag *itemdata = new WokXMLTag(**item);
		itemdata->AddTag("session", "wokjab").AddAttr("session", tag->GetAttr("session"));
		atomitems.push_back(itemdata);
		
		while ( atomitems.size() > 20 )
			atomitems.pop_front();
		continue;
		
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
				if ( (*linkiter)->GetAttr("rel").empty() )
					command.AddAttr("name", (*linkiter)->GetAttr("href"));
				else
					command.AddAttr("name", (*linkiter)->GetAttr("rel"));
				
				WokXMLTag &sig = command.AddTag("signal");
				sig.AddAttr("name" , "Jabber PubSub Atom Link");
				sig.GetFirstTag("link").AddTag(*linkiter);
				sig.GetFirstTag("link").AddAttr("session", tag->GetAttr("session"));
			
			}
		}
		
		wls->SendSignal("Notification Add", notify);
	}

	
	
	return 1;	
}
