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

#include "u-r-i-handler.hpp"


URIHandler::URIHandler(WLSignal *wls ) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber URI", &URIHandler::Handle, 1000);
	
	
	
}


int
URIHandler::Handle(WokXMLTag *tag)
{
	if (tag->GetAttr("uri").substr(0, 5) == "xmpp:")
	{
		std::string uri = tag->GetAttr("uri").substr(5);
		
		
		std::string::size_type pos;
		std::string jid;
		if ((pos = uri.find("?"))==std::string::npos)
			return 1;
		
		jid = uri.substr(0,pos);
		
		std::string action = uri.substr(pos+1);
		
		std::map <std::string, std::string> values;
		std::string::size_type value_pos = 0;
		while( (value_pos = action.find(";", value_pos ) ) != std::string::npos)
		{
			value_pos++;
			std::string::size_type nameend;
			if ( ( nameend = action.find("=", value_pos) ) == std::string::npos )
				break;
			
			std::string::size_type valueend;
			if ( ( valueend = action.find("=", nameend) ) == std::string::npos )
				values[action.substr(value_pos, nameend-value_pos)] = "";
			else
				values[action.substr(value_pos, nameend-value_pos)] = action.substr(nameend+1, valueend - nameend - 1);
			
		}
		
		if(action.size() >= 4 && action.substr(0, 4) == "join")
		{
			std::cout << "join" << std::endl;
		}
		if (action.size() >= 7 &&  action.substr(0, 7) == "message")
		{
				
			std::cout << "message" << std::endl;
			WokXMLTag dialog("dialog");
			dialog.AddAttr("jid", jid);
			dialog.AddAttr("session", tag->GetAttr("session"));
			if ( values.find("body") != values.end() )
			{
				WokXMLTag &body = dialog.AddTag("body");
				body.AddText(values["body"]);
			}
			wls->SendSignal("Jabber GUI MessageDialog Open", dialog);
		}
		if ( action.size() >= 7 && action.substr(0, 7) == "command")
		{
			if ( values.find("node") != values.end())
			{
				WokXMLTag adhoc("adhoc");
				adhoc.AddAttr("jid", jid);
				adhoc.AddAttr("session", tag->GetAttr("session"));
				
				WokXMLTag &node = adhoc.AddTag("node");
				node.AddAttr("node", values["node"]);
				
				wls->SendSignal("Jabber AdHoc Start", adhoc);
			}
			
			
			
			
		}
		
	}
	
	
	return 1;
}


