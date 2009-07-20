/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * wokjab
 * Copyright (C) Kent Gustavsson 2009 <nedo80@gmail.com>
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

#include "xdata-config-session.h"



xdataConfigSession::xdataConfigSession(WLSignal *wls, WokXMLTag &xml, std::string id) : WLSignalInstance(wls),
origxml(xml), data("config")
{
	myID = id;
	path = "/";

	
	SendResponse(origxml.GetFirstTag("iq").GetAttr("id"));
}

std::string
xdataConfigSession::GetID()
{
	return myID;


}


/**
 * Copy paste from the config plugin ... this kinda sucks ...
 */
WokXMLTag *
xdataConfigSession::GetPosition(WokXMLTag *place)
{
	std::string pospath;
	if ( path.size() )
		pospath = this->path.substr(1, this->path.size()-1);
	else
		pospath = "";

	while( pospath.size() )
	{
		std::string substr = pospath.substr(0, pospath.find("/"));
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
			
		if( pospath.size() == substr.size() )
			break;

		pospath = pospath.substr(substr.size() + 1, pospath.size());
	}

	return place;
}

void
xdataConfigSession::PopulateXML(WokXMLTag &xdata, WokXMLTag &config, std::string path)
{
    std::list <WokXMLTag *>::iterator taglist;

	for ( taglist = xdata.GetTags().begin(); 
         taglist != xdata.GetTags().end();
         taglist++)
    {
		if ( (*taglist)->GetAttr("type") == "string")
		{
			WokXMLTag &conf_field(config.AddTag("field"));
			conf_field.AddAttr("type", "text-single");
			conf_field.AddAttr("var", path + "/" + (*taglist)->GetName());
			conf_field.AddAttr("label", path + "/" + (*taglist)->GetAttr("label"));
			conf_field.AddTag("value").AddText((*taglist)->GetAttr("data"));
		}
		else if ( (*taglist)->GetAttr("type") == "password")
		{
			WokXMLTag &conf_field(config.AddTag("field"));
			conf_field.AddAttr("type", "text-private");
			conf_field.AddAttr("var", path + "/" + (*taglist)->GetName());
			conf_field.AddAttr("label", path + "/" + (*taglist)->GetAttr("label"));
			conf_field.AddTag("value").AddText((*taglist)->GetAttr("data"));
		}
		else if ( (*taglist)->GetAttr("type") == "text")
		{
			WokXMLTag &conf_field(config.AddTag("field"));
			conf_field.AddAttr("type", "text-multi");
			conf_field.AddAttr("var", path + "/" + (*taglist)->GetName());
			conf_field.AddAttr("label", path + "/" + (*taglist)->GetAttr("label"));
			conf_field.AddTag("value").AddText((*taglist)->GetBody());
		}	
		else if ( (*taglist)->GetAttr("type") == "bool")
		{
			WokXMLTag &conf_field(config.AddTag("field"));
			conf_field.AddAttr("type", "boolean");
			conf_field.AddAttr("var", path + "/" + (*taglist)->GetName());
			conf_field.AddAttr("label", path + "/" + (*taglist)->GetAttr("label"));
			
			if ( (*taglist)->GetAttr("data") ==  "false" )
				conf_field.AddTag("value").AddText("0");
			else
				conf_field.AddTag("value").AddText("1");
		}
		else if ( (*taglist)->GetAttr("type") == "jidlist")
		{
			WokXMLTag &conf_field(config.AddTag("field"));
			conf_field.AddAttr("type", "jid-multi");
			conf_field.AddAttr("var", path + "/" + (*taglist)->GetName());
			conf_field.AddAttr("label", path + "/" + (*taglist)->GetAttr("label"));

			std::list <WokXMLTag *>::iterator citer;
			for( citer = (*taglist)->GetFirstTag("list").GetTagList("item").begin(); citer !=  (*taglist)->GetFirstTag("list").GetTagList("item").end(); citer++)
			{
				conf_field.AddTag("value").AddText((*citer)->GetAttr("data"));
			}
		}
		else if ( !(*taglist)->GetTags().empty() )
		{
			PopulateXML(**taglist, config, path + "/" + (*taglist)->GetName());
		}
	}
}

void
xdataConfigSession::SendResponse(std::string iq_id)
{	 
	WokXMLTag pathtag(NULL, "configpath");
	wls->SendSignal("Config XML GetTree", &pathtag);

	WokXMLTag *configtag = GetPosition(&pathtag.GetFirstTag("config"));
	
	WokXMLTag mesg("message");
    mesg.AddAttr("session", origxml.GetAttr("session"));
    WokXMLTag &iq = mesg.AddTag("iq");
    iq.AddAttr("to", origxml.GetFirstTag("iq").GetAttr("from"));
    iq.AddAttr("type", "result");
    iq.AddAttr("id", iq_id);
    WokXMLTag &comm = iq.AddTag("command", "http://jabber.org/protocol/commands");
    comm.AddAttr("node", "configure");
    comm.AddAttr("sessionid", myID);
	comm.AddAttr("status", "executing");

    WokXMLTag &actions = comm.AddTag("actions");
    actions.AddAttr("execute", "next");
    actions.AddTag("next");

    if ( !(path == "" || path == "/" ))
    {
		actions.AddTag("prev");
	}
                 
	WokXMLTag &xtag = comm.AddTag("x");
	xtag.AddAttr("xmlns", "jabber:x:data");
	xtag.AddAttr("type", "form");
	xtag.AddTag("title").AddText("Remove Config");
	xtag.AddTag("instructions").AddText("Select sub-section");
						 
	WokXMLTag &field = xtag.AddTag("field");
	field.AddAttr("var", "subsection");
	field.AddAttr("label", "Subsections");
	field.AddAttr("type", "list-single" );

	WokXMLTag &option = field.AddTag("option");
	option.AddAttr("label", "Current position");
	option.AddTag("value").AddText(path);
    field.AddTag("value").AddText(path);
                                
	std::list <WokXMLTag *>::iterator taglist;
	for( taglist = configtag->GetTags().begin(); taglist != configtag->GetTags().end(); taglist++)
	{
		if ( (*taglist)->GetName() != "config")
		{
			WokXMLTag &option = field.AddTag("option");
			option.AddAttr("label", (*taglist)->GetName());
			option.AddTag("value").AddText(path + (*taglist)->GetName() + "/");
		}

	}
    data = configtag->GetFirstTag("config");
    PopulateXML(configtag->GetFirstTag("config").GetFirstTag("config"), xtag);
	wls->SendSignal("Jabber XML Send", mesg);
}

/** 
 * Finds the correct place in the config to set the value 
 */
void
xdataConfigSession::SetConfigParamRec(WokXMLTag &tag, const std::string &path, WokXMLTag &field)
{
	if ( path.find("/", 1) != std::string::npos )
	{
		SetConfigParamRec(tag.GetFirstTag(path.substr(1, path.find("/",1)-1)), path.substr(path.find("/",1)), field);
	}
	else
	{
		std::string paramname = path.substr(1);
		std::string type = tag.GetFirstTag(paramname).GetAttr("type");
		if ( type == "string" || type == "password")
		{
			tag.GetFirstTag(paramname).AddAttr("data", field.GetFirstTag("value").GetBody());
		}
		else if ( type == "text" )
		{
			tag.GetFirstTag(paramname).GetFirstTag("value").RemoveBody();
			tag.GetFirstTag(paramname).GetFirstTag("value").AddText(field.GetFirstTag("value").GetBody());
		}
		else if ( type == "bool" )
		{
			if ( field.GetFirstTag("value").GetBody() == "0" || field.GetFirstTag("value").GetBody() == "false" )
			{
				tag.GetFirstTag(paramname).AddAttr("data", "false");
			}
			else
				tag.GetFirstTag(paramname).AddAttr("data", "true");
		}
		else if ( type == "jidlist" )
		{
			while( !tag.GetFirstTag(paramname).GetFirstTag("list").GetTagList("item").empty() )
				tag.GetFirstTag(paramname).GetFirstTag("list").RemoveTag(
				          *tag.GetFirstTag(paramname).GetFirstTag("list").GetTagList("item").begin());

			std::list <WokXMLTag *>::iterator tagiter;
			for( tagiter = field.GetTagList("value").begin() ; tagiter != field.GetTagList("value").end() ; tagiter++)
			{
				std::cout << "Tagiter" << *tagiter << std::endl;
				tag.GetFirstTag(paramname).GetFirstTag("list").AddTag("item").AddAttr("data", (*tagiter)->GetBody());
			}
		}
				
	}

}

void
xdataConfigSession::Execute(WokXMLTag *tag)
{
	WokXMLTag &xtag = tag->GetFirstTag("iq").GetFirstTag("command", "http://jabber.org/protocol/commands").GetFirstTag("x", "jabber:x:data");

	std::list<WokXMLTag *>::iterator tagiter;


	
	/* Previous button used */                                                  
	if ( tag->GetFirstTag("iq").GetFirstTag("command", "http://jabber.org/protocol/commands").GetAttr("action") == "prev")
	{
		if ( path.rfind("/") == path.length()-1 )
			path = path.substr(0, path.length()-2);
		if ( path.find("/") != std::string::npos )
			path = path.substr(0, path.rfind("/"));
		else 
			path = "/";
		
	}
	else                            
	{
		if ( tag->GetFirstTag("iq").GetFirstTag("command", "http://jabber.org/protocol/commands").GetAttr("action") == "execute")
		{
			WokXMLTag config("config");
			config.AddAttr("path", path);
			wls->SendSignal("Config XML GetConfig", config);

			for(tagiter = xtag.GetTagList("field").begin(); tagiter != xtag.GetTagList("field").end(); tagiter++)
			{
				if ( (*tagiter)->GetAttr("var") == "subsection" )
					path = (*tagiter)->GetFirstTag("value").GetBody();
				if ( (*tagiter)->GetAttr("var")[0] == '/' )
				{
					SetConfigParamRec(config.GetFirstTag("config") ,(*tagiter)->GetAttr("var") , **tagiter);
				}
			}


			std::cout << "Config now looks like : " << config << std::endl;
			wls->SendSignal("Config XML Store", config);
			
		}
		else
		{	
			for(tagiter = xtag.GetTagList("field").begin(); tagiter != xtag.GetTagList("field").end(); tagiter++)
			{
				if ( (*tagiter)->GetAttr("var") == "subsection" )
					path = (*tagiter)->GetFirstTag("value").GetBody();
			}
		}
	}
	                                        
	SendResponse(tag->GetFirstTag("iq").GetAttr("id"));
}