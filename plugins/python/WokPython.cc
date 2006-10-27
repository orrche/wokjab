/***************************************************************************
 *  Copyright (C) 2003-2006  Kent Gustavsson <nedo80@gmail.com>
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
// Class: WokPython
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Mon Mar 13 01:50:33 2006
//

#include "WokPython.h"


WokPython::WokPython(WLSignal *wls) : WoklibPlugin(wls)
{
	Py_Initialize();
	PyEval_InitThreads();
	PyEval_ReleaseLock();
	EXP_SIGHOOK("Python Script Load", &WokPython::Load, 500);
	EXP_SIGHOOK("Python Script Unload", &WokPython::Unload, 500);
	EXP_SIGHOOK("Python Script GetList", &WokPython::GetScripts, 500);
	
	
	config = new WokXMLTag(NULL, "NULL");
	EXP_SIGHOOK("Config XML Change /script", &WokPython::Config, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/script");
	wls->SendSignal("Config XML Trigger", &conftag);
}

WokPython::~WokPython()
{
	config->GetFirstTag("scripts").RemoveChildrenTags();
	std::map <std::string, PyScript*>::iterator iter;
	for ( iter = script.begin() ; iter != script.end() ; iter++ )
	{
		config->GetFirstTag("scripts").AddTag("item").AddAttr("file", iter->first);
		delete (iter->second);
	}
	
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/script");
	conftag.AddTag(config);
	
	EXP_SIGUNHOOK("Config XML Change /script", &WokPython::Config, 500);
	wls->SendSignal("Config XML Store", &conftag);

	delete config;
}

int
WokPython::Config(WokXMLTag *tag) 
{
	if( tag )
	{
		std::list <WokXMLTag*>::iterator confiter;
		std::list <WokXMLTag*>::iterator olditer;
		std::list <WokXMLTag*> &list = tag->GetFirstTag("config").GetFirstTag("scripts").GetTagList("item");
		
		for( confiter = list.begin() ; confiter != list.end(); confiter++)
		{
			if (script.find((*confiter)->GetAttr("file")) == script.end() )
			{
				WokXMLTag script(NULL, "script");
				script.AddAttr("file", (*confiter)->GetAttr("file"));

				wls->SendSignal("Python Script Load", script);
			}
		}
		delete config;
		config = new WokXMLTag(tag->GetFirstTag("config"));
	}
	return true;
}

int
WokPython::Load(WokXMLTag *tag)
{
	if ( script.find(tag->GetAttr("file")) == script.end() )
		script[tag->GetAttr("file")] = new PyScript(wls, tag->GetAttr("file"));

		
	return 1;
}

int
WokPython::Unload(WokXMLTag *tag)
{
	delete script[tag->GetAttr("file")];
	script.erase(tag->GetAttr("file"));
	
	return 1;
}

int
WokPython::GetScripts(WokXMLTag *tag)
{
	std::map< std::string, PyScript *>::iterator iter;

	for( iter = script.begin() ; iter != script.end() ; iter++ )
	{
		WokXMLTag &item = tag->AddTag("item");
		item.AddAttr("file", iter->first);
	}
	
	return 1;
}

