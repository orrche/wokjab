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

#include "plugin-loader.hpp"
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

void pluginloader(WLSignal *wls, std::string path);

PluginLoader::PluginLoader(WLSignal *wls) : WLSignalInstance(wls)
{
	EXP_SIGHOOK("Woklib Plugin Add", &PluginLoader::Add, 1000);
	EXP_SIGHOOK("Woklib Plugin Remove", &PluginLoader::Remove, 1000);
	
	config = new WokXMLTag(NULL, "NULL");
	EXP_SIGHOOK("Config XML Change /plugins", &PluginLoader::ReadConfig, 500);
	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/plugins");
	wls->SendSignal("Config XML Trigger", &conftag);

}

PluginLoader::~PluginLoader()
{
	SaveConfig();

}

void
PluginLoader::SaveConfig()
{
	config->RemoveTag(&config->GetFirstTag("plugins"));

	std::list <std::string>::iterator iter;

	for( iter = plugins.begin() ; iter != plugins.end() ; iter++)
	{
		config->GetFirstTag("plugins").AddTag("item").AddAttr("filename", *iter);
	}

	WokXMLTag conftag(NULL, "config");
	conftag.AddAttr("path", "/plugins");
	conftag.AddTag(config);

	EXP_SIGUNHOOK("Config XML Change /plugins", &PluginLoader::ReadConfig, 500);
	wls->SendSignal("Config XML Store", &conftag);
	EXP_SIGHOOK("Config XML Change /plugins", &PluginLoader::ReadConfig, 500);
}


int
PluginLoader::ReadConfig(WokXMLTag *tag)
{
	if ( !plugins.empty() )
		return 1;
	
	delete config;
	
	config = new WokXMLTag (tag->GetFirstTag("config"));

	std::list < WokXMLTag * >::iterator iter;
	std::list < WokXMLTag * > &list = config->GetFirstTag("plugins").GetTagList("item");

	for( iter = list.begin() ; iter != list.end() ; iter++)
	{		
		plugins.push_back((*iter)->GetAttr("filename"));
		WokXMLTag tag(NULL, "add");
		tag.AddAttr("filename", (*iter)->GetAttr("filename"));
		wls->SendSignal("Woklib Plugin Add", &tag);
	}

	if ( tag->GetAttr("inited") != "true" && plugins.empty())
	{
		tag->AddAttr("inited", "true");
		pluginloader(wls, std::string(PACKAGE_PLUGIN_DIR) + "/normal");
	}
	return 1;
}

int
PluginLoader::Add(WokXMLTag *tag)
{
	if ( std::find(plugins.begin(), plugins.end(), tag->GetAttr("filename")) == plugins.end() )
	{
		plugins.push_back(tag->GetAttr("filename"));
	}	
	return 1;
}

int
PluginLoader::Remove(WokXMLTag *tag)
{
	std::list<std::string>::iterator iter;
	
	if ( ( iter = std::find(plugins.begin(), plugins.end(), tag->GetAttr("filename"))) != plugins.end() )
		plugins.erase(iter);

	return 1;
}
