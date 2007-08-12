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
	
	if ( config->GetFirstTag("change_status").GetAttr("data") != "false" )
	{
		delete status;
		status = new WokXMLTag ( "status" );
		status->Add(config->GetFirstTag("status").GetBody() );
	}
	else if ( status != NULL )
	{
		delete status;
		status = NULL;
	}
	
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
	
		if ( status )
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
