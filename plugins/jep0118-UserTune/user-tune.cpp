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

#include "user-tune.hpp"


UserTune::UserTune(WLSignal *wls) : WoklibPlugin(wls)
{
	status = NULL;
	
	EXP_SIGHOOK("Jabber UserTune SetTune", &UserTune::SetTune, 1000);
	EXP_SIGHOOK("Jabber PubSub JID 'http://jabber.org/protocol/tune'", &UserTune::Message, 1000);
	EXP_SIGHOOK("Jabber UserActivityGet", &UserTune::ActivityLine, 1000);
	
	config = new WokXMLTag(NULL, "NULL");
	EXP_SIGHOOK("Config XML Change /pubsub/usertune", &UserTune::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/pubsub/usertune");
	wls->SendSignal("Config XML Trigger", &conftag);
}


UserTune::~UserTune()
{
	
	
	
}

int
UserTune::ActivityLine(WokXMLTag *tag)
{
	if ( user.find(tag->GetAttr("jid")) != user.end() )
	{
		WokXMLTag &item = tag->AddTag("item");
		item.AddAttr("type_name", "User Tune");
		WokXMLTag &line = item.AddTag("line");
					
		if ( status )
		{
			std::list <WokXMLObject *>::iterator oiter;
			for ( oiter = status->GetFirstTag("status").GetItemList().begin() ; oiter != status->GetFirstTag("status").GetItemList().end() ; oiter++)
			{
				switch ( (*oiter)->GetType() )
				{
					case 1:
						WokXMLTag *otag;
						otag = (WokXMLTag *)(*oiter);
						if ( otag->GetName() == "var" )
						{
							if ( otag->GetAttr("name") == "artist" )
								line.AddText(user[tag->GetAttr("jid")]->GetFirstTag("artist").GetBody());
							else if ( otag->GetAttr("name") == "length" )
								line.AddText(user[tag->GetAttr("jid")]->GetFirstTag("length").GetBody());
							else if ( otag->GetAttr("name") == "source")
								line.AddText(user[tag->GetAttr("jid")]->GetFirstTag("source").GetBody());
							else if ( otag->GetAttr("name") == "title")
								line.AddText(user[tag->GetAttr("jid")]->GetFirstTag("title").GetBody());
							else if ( otag->GetAttr("name") == "track")
								line.AddText(user[tag->GetAttr("jid")]->GetFirstTag("track").GetBody());
							else if ( otag->GetAttr("name") == "uri")
								line.AddText(user[tag->GetAttr("jid")]->GetFirstTag("uri").GetBody());
						}
						break;
					case 2:
						
						WokXMLText *tt;
						tt = (WokXMLText *)(*oiter);
						line.AddText(tt->GetText());
						break;
				}
			}
		}
	}
	return 1;	
}

int
UserTune::Message(WokXMLTag *tag)
{
	if ( user.find(tag->GetFirstTag("message").GetAttr("from")) != user.end() )
	{
		delete user[tag->GetFirstTag("message").GetAttr("from")];
	}
	
	WokXMLTag *tune = NULL;
	std::list <WokXMLTag *>::iterator eventiter;
	for ( eventiter = tag->GetFirstTag("message").GetTagList("event").begin() ; eventiter != tag->GetFirstTag("message").GetTagList("event").end() ; eventiter++)
	{
		if ( (*eventiter)->GetAttr("xmlns") == "http://jabber.org/protocol/pubsub#event" )
		{
			std::list <WokXMLTag *>::iterator itemiter;
			for ( itemiter = (*eventiter)->GetTagList("items").begin() ; itemiter != (*eventiter)->GetTagList("items").end() ; itemiter++ )
			{
				if ( (*itemiter)->GetAttr("node") == "http://jabber.org/protocol/tune" )
				{
					std::list <WokXMLTag *>::iterator tuneiter;
					for( tuneiter = (*itemiter)->GetFirstTag("item").GetTagList("tune").begin() ; tuneiter != (*itemiter)->GetFirstTag("item").GetTagList("tune").end() ; tuneiter++ )
					{
						
						if ( (*tuneiter)->GetAttr("xmlns") == "http://jabber.org/protocol/tune" )
						{
							tune = *tuneiter;
							break;
						}
					}
					if( tune ) break;
				}				
			}
			if( tune ) break;
		}
	}
	if ( tune )
	{
		user[tag->GetFirstTag("message").GetAttr("from")] = new WokXMLTag(*tune);
	
		wls->SendSignal("Jabber UserActivityUpdate " + tag->GetAttr("session") +
					" '" + XMLisize(tag->GetFirstTag("message").GetAttr("from")) + "'", tag);
	}
	else
		woklib_debug(wls, "One think there should be some kind of sane information here in the user tune ... ");
	
	return 1;
}

int
UserTune::ReadConfig(WokXMLTag *tag)
{
	
	tag->GetFirstTag("config").GetFirstTag("change_status").AddAttr("type", "bool");
	tag->GetFirstTag("config").GetFirstTag("change_status").AddAttr("label", "Influence status");
	tag->GetFirstTag("config").GetFirstTag("status").AddAttr("type", "text");
	tag->GetFirstTag("config").GetFirstTag("status").AddAttr("label", "Status string");
	if ( tag->GetFirstTag("config").GetFirstTag("status").GetBody() == "" )
	{
		tag->GetFirstTag("config").GetFirstTag("status").AddText("<status><var name=\"artist\"/></status>");
	}
	
	delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));
	
	
	delete status;
	status = new WokXMLTag ( "status" );
	status->Add(config->GetFirstTag("status").GetBody() );

	return 1;
	
}


int
UserTune::SetTune(WokXMLTag *tag)
{
	WokXMLTag sessions("sessions");
	wls->SendSignal("Jabber GetSessions", sessions);
	std::list <WokXMLTag *>::iterator session;
	
	
	for( session = sessions.GetTagList("item").begin() ; session != sessions.GetTagList("item").end() ; session++)
	{
		WokXMLTag msg("message");
		msg.AddAttr("session", (*session)->GetAttr("name"));
		WokXMLTag &iq = msg.AddTag("iq");
		iq.AddAttr("type", "set");
		WokXMLTag &pubsub = iq.AddTag("pubsub");
		pubsub.AddAttr("xmlns", "http://jabber.org/protocol/pubsub");
		WokXMLTag &publish = pubsub.AddTag("publish");
		publish.AddAttr("node", "http://jabber.org/protocol/tune");
		publish.AddTag(&tag->GetFirstTag("item"));
		wls->SendSignal("Jabber XML Send", msg);
		
		std::string statusmsg = "";
	
		if ( status && config->GetFirstTag("change_status").GetAttr("data") != "false" )
		{
			WokXMLTag *tune = NULL;;
			std::list<WokXMLTag*>::iterator tuneiter;
			for( tuneiter = tag->GetFirstTag("item").GetTagList("tune").begin() ; tuneiter != tag->GetFirstTag("item").GetTagList("tune").end() ; tuneiter++)
			{
				if ( (*tuneiter)->GetAttr("xmlns") == "http://jabber.org/protocol/tune" )
				{
					tune = (*tuneiter);
					break;
				}
				
			}
			if( tune )
			{
				std::list <WokXMLObject *>::iterator oiter;
				for ( oiter = status->GetFirstTag("status").GetItemList().begin() ; oiter != status->GetFirstTag("status").GetItemList().end() ; oiter++)
				{
					switch ( (*oiter)->GetType() )
					{
						case 1:
							WokXMLTag *tag;
							tag = (WokXMLTag *)(*oiter);
							if ( tag->GetName() == "var" )
							{
								if ( tag->GetAttr("name") == "artist" )
									statusmsg += tune->GetFirstTag("artist").GetBody();
								else if ( tag->GetAttr("name") == "length" )
									statusmsg += tune->GetFirstTag("length").GetBody();
								else if ( tag->GetAttr("name") == "source")
									statusmsg += tune->GetFirstTag("source").GetBody();
								else if ( tag->GetAttr("name") == "title")
									statusmsg += tune->GetFirstTag("title").GetBody();
								else if ( tag->GetAttr("name") == "track")
									statusmsg += tune->GetFirstTag("track").GetBody();
								else if ( tag->GetAttr("name") == "uri")
									statusmsg += tune->GetFirstTag("uri").GetBody();
							}
							break;
						case 2:
							
							WokXMLText *tt;
							tt = (WokXMLText *)(*oiter);
							statusmsg += tt->GetText();
							break;
					}
				}
				
				WokXMLTag mtag("message");
				mtag.AddAttr("session", (*session)->GetAttr("name"));
				mtag.AddTag("presence").AddTag("status").AddText(statusmsg);
                wls->SendSignal("Jabber XML Presence Send", mtag);
			}
		}
		
	}
}
