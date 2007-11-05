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

#include "html-link.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

HtmlLink::HtmlLink(WLSignal *wls) : WoklibPlugin(wls)
{
	EXP_SIGHOOK("Jabber XML Message Normal", &HtmlLink::Message, 200);
	EXP_SIGHOOK("HtmlLink RunBrowser", &HtmlLink::RunBrowser, 200);
		
	config = new WokXMLTag(NULL, "NULL");
	EXP_SIGHOOK("Config XML Change /external/web_browser", &HtmlLink::Config, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/external/web_browser");
	wls->SendSignal("Config XML Trigger", &conftag);

}


HtmlLink::~HtmlLink()
{
	
	
	
	
}

int
HtmlLink::Message(WokXMLTag *tag)
{
	std::list <WokXMLTag *>::iterator bodyiter;
	WokXMLTag *command = NULL;
	bool inited = false;
	
	for ( bodyiter = tag->GetFirstTag("message").GetTagList("body").begin() ; 
		 bodyiter != tag->GetFirstTag("message").GetTagList("body").end() ; bodyiter++)
	{
		std::string body = (*bodyiter)->GetBody();
		size_t pos = 0;
		while ( (pos = body.find("http://", pos)) != std::string::npos )
		{
			std::string link;
			size_t endlink = body.size();
			size_t tmp;
			
			if ( ( tmp = body.find(" ", pos )) != std::string::npos && tmp < endlink )
				endlink = tmp;
			if ( ( tmp = body.find("\n", pos )) != std::string::npos && tmp < endlink )
				endlink = tmp;
			if ( ( tmp = body.find("\t", pos )) != std::string::npos && tmp < endlink )
				endlink = tmp;
				
			if ( endlink == body.size() )
			{
				link = body.substr(pos);
				body = body.substr(0, pos);
			}
			else
			{
				link = body.substr(pos, endlink - pos );
				body = body.substr(0, pos) + body.substr(endlink);	
			}
			
			if ( !inited )
			{
				command = &tag->AddTag("command");
			
				command->AddAttr("name", "HTTP:");
				inited = true;
			}
			
			WokXMLTag &com_tag = command->AddTag("command");
			com_tag.AddAttr("name", link);
			
			WokXMLTag &sig_tag = com_tag.AddTag("signal");
			sig_tag.AddAttr("name", "HtmlLink RunBrowser");
			WokXMLTag &data_tag = sig_tag.AddTag("data");
			WokXMLTag &link_tag = data_tag.AddTag("link");
			link_tag.AddAttr("url", link);
		}
		pos = 0;
		while ( (pos = body.find("www.", pos)) != std::string::npos )
		{
			std::string link;
			size_t endlink = body.size();
			size_t tmp;
			
			if ( ( tmp = body.find(" ", pos )) != std::string::npos && tmp < endlink )
				endlink = tmp;
			if ( ( tmp = body.find("\n", pos )) != std::string::npos && tmp < endlink )
				endlink = tmp;
			if ( ( tmp = body.find("\t", pos )) != std::string::npos && tmp < endlink )
				endlink = tmp;
				
			if ( endlink == body.size() )
			{
				link = body.substr(pos);
				body = body.substr(0, pos);
			}
			else
			{
				link = body.substr(pos, endlink - pos );
				body = body.substr(0, pos) + body.substr(endlink);	
			}
			
			if ( !inited )
			{
				command = &tag->AddTag("command");
			
				command->AddAttr("name", "HTTP:");
				inited = true;
			}
			
			WokXMLTag &com_tag = command->AddTag("command");
			com_tag.AddAttr("name", "http://" + link);
			
			WokXMLTag &sig_tag = com_tag.AddTag("signal");
			sig_tag.AddAttr("name", "HtmlLink RunBrowser");
			WokXMLTag &data_tag = sig_tag.AddTag("data");
			WokXMLTag &link_tag = data_tag.AddTag("link");
			link_tag.AddAttr("url", "http://" + link);
		}
		
	}	
	
	return 1;	
}

int
HtmlLink::RunBrowser(WokXMLTag *tag)
{
	int pid,status;
	switch (pid = fork()){
		case 0:
			switch (fork())
			{
				case 0:
					system((config->GetFirstTag("browser").GetAttr("data") + " " + tag->GetFirstTag("link").GetAttr("url")).c_str());
					_exit(1);
			}
			_exit(1);
	}
	
	if(pid > 0)
		waitpid(pid, &status, 0);
	
	return 1;
}

int
HtmlLink::Config(WokXMLTag *tag)
{

	delete config;
	config = new WokXMLTag(tag->GetFirstTag("config"));
	
	tag->GetFirstTag("config").GetFirstTag("browser").AddAttr("name", "Browser to use");
	tag->GetFirstTag("config").GetFirstTag("browser").AddAttr("type", "string");

	return 1;
}	
