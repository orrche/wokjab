/***************************************************************************
 *  Copyright (C) 2003-2005  Kent Gustavsson <oden@gmx.net>
 ****************************************************************************/
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#include <iostream>

#include <Woklib/WokLib.h>
#include <Woklib/WLSignal.h>
#include <Woklib/WokLibSignal.h>

#include "include/Roster.h"
#include "include/GUIWindow.h"
#include "include/main.h"
#include "../include/Initiat.h"

#include <gtk/gtk.h>
#include <dirent.h>

using std::string;
using std::cout;
using std::endl;

void LoadPlugin(WLSignal *wls, std::string filename)
{
	WokXMLTag tag(NULL, "add");
	tag.AddAttr("filename", filename);
	wls->SendSignal("Woklib Plugin Add", &tag);
}

void
pluginloader(WLSignal *wls, std::string path)
{
	DIR             *dip;
	struct dirent   *dit;
    std::cout << "Plugin loader " << path << std::endl;

	std::string filename = path;

	if ((dip = opendir(filename.c_str())) == NULL)
	{
    	perror("opendir");
		return;
	}

	while ((dit = readdir(dip)) != NULL)
	{
		std::string file = dit->d_name;

#ifdef __WIN32
		if(dit->d_name[0] != '.' && file.substr(file.size()-3) == "dll")
			LoadPlugin(wls, filename + '/' + file);
#else
		if(dit->d_name[0] != '.' && file.substr(file.size()-2) == "so")
			LoadPlugin(wls, filename + '/' + file);
#endif
	}

	if (closedir(dip) == -1)
	{
		perror("closedir");
		return;
	}
}

void
load_plugin_list(WLSignal *wls, GSList * plugins)
{
	/* run through the list */
	GSList *listrunner;
	listrunner = plugins;

	while (listrunner)
	{
		LoadPlugin(wls, ((const gchar *)listrunner->data));
		listrunner = listrunner->next;
	}
}

int
main (int argc, char **argv)
{
	WLSignal *wls;

	gtk_init(&argc, &argv);
	WokLib sj;
	wls = &sj.wls_main;

	Initiat client_init(&sj.wls_main, &sj, argc, argv);

	bool sysplug = true;
	bool normplug = true;

	if( argc > 1 )
	{
		for( int i = 1; i < (argc) ; ++i)
		{
			if ( std::string("-ns") == argv[i] )
				sysplug = false;
			if ( std::string("-nn") == argv[i] )
				normplug = false;
		}
	}

	if ( sysplug )
		pluginloader(&sj.wls_main, string(PACKAGE_PLUGIN_DIR) + "/system");

	WokXMLTag confinit(NULL, "init");
	confinit.AddTag("filename").AddAttr("data", std::string(g_get_home_dir()) + "/.wokjab/config.xml");
	wls->SendSignal("Config XML Init", &confinit);

    if ( normplug )
		pluginloader(&sj.wls_main, string(PACKAGE_PLUGIN_DIR) + "/normal");

    if( argc > 1 )
	{
		for(int i = 1 ; i < (argc-1) ; ++i)
		{
			if( std::string("-p") == argv[i] )
			{
				std::string pluginstr = argv[i+1];
				std::string::size_type startpos= 0;
				std::string::size_type endpos;

				while((endpos = pluginstr.find(",", startpos)) != std::string::npos)
				{
					LoadPlugin(wls, pluginstr.substr(startpos, endpos).c_str());
					startpos = endpos + 1;
				}
				endpos = pluginstr.length();
				LoadPlugin(wls, pluginstr.substr(startpos, endpos).c_str());
			}
		}
	}

	Roster roster(&sj.wls_main);
	GUIWindow win(&sj.wls_main);

	gtk_main();

	return (0);
}
