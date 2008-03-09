/***************************************************************************
 *  Copyright (C) 2005-2007  Kent Gustavsson <nedo80@gmail.com>
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
// Class: Config
//
// Created by: Kent Gustavsson <nedo80@gmail.com>
// Created on: Fri Jun 10 02:25:01 2005
//

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "Config.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef __WIN32
#include <io.h>
#endif
Config::Config(WLSignal *wls):
WoklibPlugin(wls)
{
	EXP_SIGHOOK("Config XML Init", &Config::Init, 999);
}

Config::~Config()
{
	Save(NULL);
	delete truebasetag;
}

/**
 * Initiates the config so diffrent config files can be read for diffrent programs
 */
int
Config::Init(WokXMLTag *tag)
{
	EXP_SIGHOOK("Config XML Store", &Config::Store, 999);
	EXP_SIGHOOK("Config XML Trigger", &Config::Trigger, 999);
	EXP_SIGHOOK("Config XML Save", &Config::Save, 999);
	EXP_SIGHOOK("Config XML GetTree", &Config::GetTree, 999);

	filename = tag->GetFirstTag("filename").GetAttr("data");

	std::string::size_type pos = filename.find("/");
	while( pos != std::string::npos )
	{
#ifdef __WIN32
            mkdir(filename.substr(0, pos).c_str());
#else
			mkdir(filename.substr(0, pos).c_str(), 0700);
#endif
			pos = filename.find("/", pos + 1);
	}

//	mkdir((std::string(std::getenv("HOME")) + "/.wokjab").c_str(), 0700);

	truebasetag = new WokXMLTag(NULL, "config");
	std::ifstream file;
	file.open(filename.c_str(),std::ios::in);
	bool fromdefaultfile = false;
	if(!file.is_open())
	{
		file.clear();
		std::string datadir = PACKAGE_DATA_DIR;
		datadir += "/wokjab/defaultconfig.xml";
		file.open(datadir.c_str() ,std::ios::in);
		fromdefaultfile = true;
	}

	file >> *truebasetag;
	file.close();
	basetag = &truebasetag->GetFirstTag("config");

	if ( fromdefaultfile ) 
		Save(basetag);
	
	return true;
}

WokXMLTag *
Config::GetPosition(std::string path)
{
	WokXMLTag *place = basetag;
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
							
		place = &place->GetFirstTag(folder);
		if( path.size() == substr.size() )
			break;

		path = path.substr(substr.size() + 1, path.size());
	}

	return place;
}

int
Config::GetTree(WokXMLTag *tag)
{
	tag->AddTag(basetag);

	return 1;
}

int
Config::Save(WokXMLTag *tag)
{
	std::ofstream file(filename.c_str(), std::ios::out);
	file << *basetag << std::endl;
	file.close();
	return 1;
}

int
Config::Store(WokXMLTag *tag)
{
	std::string path = tag->GetAttr("path");
	if(!path.size())
		return true;
	WokXMLTag *place;
	WokXMLTag *parant = GetPosition(path);

	place = &parant->GetFirstTag("config");

	std::string name = parant->GetName();
	parant->RemoveTag(place);

	delete place;

	parant->AddTag(tag);

	wls->SendSignal("Config XML Change " + path, tag);

	Save(NULL);
	return 1;
}


int
Config::Trigger(WokXMLTag *tag)
{
	std::string path = tag->GetAttr("path");
	WokXMLTag *place = GetPosition(path);
	wls->SendSignal("Config XML Change " + path, place->GetFirstTag("config"));


	return 1;
}

