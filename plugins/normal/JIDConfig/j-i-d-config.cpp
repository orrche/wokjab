/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2008 <nedo80@gmail.com>
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

#include "j-i-d-config.hpp"


JIDConfig::JIDConfig(WLSignal *wls) : WoklibPlugin(wls)
{
	config = NULL;
	EXP_SIGHOOK("Jabber JIDConfig Get", &JIDConfig::Get, 999);
	EXP_SIGHOOK("Jabber JIDConfig Store", &JIDConfig::Store, 999);
	EXP_SIGHOOK("Jabber JIDConfig Trigger", &JIDConfig::Trigger, 999);
	
	/*
	EXP_SIGHOOK("Config XML Save", &Config::Save, 999);
	EXP_SIGHOOK("Config XML GetTree", &Config::GetTree, 999);
	*/
	
	EXP_SIGHOOK("Config XML Change /jidconfig", &JIDConfig::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/jidconfig");
	wls->SendSignal("Config XML Trigger", &conftag);
}

int
JIDConfig::ReadConfig(WokXMLTag *tag)
{
	delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));
	
	jidconfig.clear();
	std::list <WokXMLTag *>::iterator tagiter;
	for ( tagiter = config->GetTagList("config").begin() ; tagiter != config->GetTagList("config").end() ; tagiter++)
	{
		jidconfig[(*tagiter)->GetAttr("name")] = *tagiter;
		wls->SendSignal("Jabber JIDConfig Change " + (*tagiter)->GetAttr("name"), jidconfig[(*tagiter)->GetAttr("name")]);
	}
	
	
	return 1;
}

void
JIDConfig::SaveConfig()
{	
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/jidconfig");
	conftag.AddTag(config);

	EXP_SIGUNHOOK("Config XML Change /jidconfig", &JIDConfig::ReadConfig, 500);
	wls->SendSignal("Config XML Store", &conftag);
	EXP_SIGHOOK("Config XML Change /jidconfig", &JIDConfig::ReadConfig, 500);
}

WokXMLTag *
JIDConfig::GetPosition(std::string path, std::string jid)
{
	if  ( jidconfig.find(jid) == jidconfig.end() )
	{
		jidconfig[jid] = &config->AddTag("config");
		jidconfig[jid]->AddAttr("name", jid);
	}
		
	WokXMLTag *place = jidconfig[jid];
	path = path.substr(1, path.size()-1);

	while( path.size() )
	{
		std::string substr = path.substr(0, path.find("/"));
		std::string folder;
				
		for( unsigned int i = 0 ; i < substr.size() ; i++)
		{
			if( substr[i] == '@' )
				folder += "_a";
			else if( substr[i] == '_' )
				folder += "__";
			else
				folder += substr[i];
		}				
		
		if ( place->GetTagList(folder).empty() )
		{
			WokXMLTag *pre = place;
			place = &place->GetFirstTag(folder);
			pre->AddText("\n");
		}
		else
			place = &place->GetFirstTag(folder);
			
		if( path.size() == substr.size() )
			break;

		path = path.substr(substr.size() + 1, path.size());
	}

	return place;
}


int 
JIDConfig::Get(WokXMLTag *tag)
{
	std::string path = tag->GetAttr("path");
	std::string jid = tag->GetAttr("name");
	
	WokXMLTag *place = GetPosition(path, jid);
	tag->AddTag(&place->GetFirstTag("config").GetFirstTag("config"));
	
	return 1;
}

int
JIDConfig::Store(WokXMLTag *tag)
{
	std::string path = tag->GetAttr("path");
	if(path.empty())
		return 1;
	WokXMLTag *place;
	WokXMLTag *parant = GetPosition(path, tag->GetAttr("name"));
	
	place = &parant->GetFirstTag("config");

	std::string name = parant->GetName();
	parant->RemoveTag(place);

	delete place;

	parant->AddTag(tag);

	wls->SendSignal("Jabber JIDConfig Change " + tag->GetAttr("name") + " " + path, tag);

	SaveConfig();
	
	return 1;
}

int
JIDConfig::Trigger(WokXMLTag *tag)
{
	std::string path = tag->GetAttr("path");
	WokXMLTag *place = GetPosition(path, tag->GetAttr("name"));	
	wls->SendSignal("Jabber JIDConfig Change " + tag->GetAttr("name") + " " + path, tag);


	return 1;
}
